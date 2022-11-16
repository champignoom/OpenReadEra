/*******************************************************
   CoolReader Engine

   lvtinydom.cpp: fast and compact XML DOM tree

   (c) Vadim Lopatin, 2000-2011
   Copyright (C) 2013-2020 READERA LLC

   This source code is distributed under the terms of
   GNU General Public License
   See LICENSE file for details
*******************************************************/

#include <map>
#include <stdlib.h>
#include <zlib.h>
#include <eraepub/include/crcss.h>
#include <erapdf/freetype/include/internal/psaux.h>
#include "openreadera.h"
#include "include/lvstring.h"
#include "include/lvtinydom.h"
#include "include/fb2def.h"
#include "include/lvrend.h"
#include "include/crconfig.h"
#include "include/RectHelper.h"
#include "include/rtlhandler.h"
#include "include/charProps.h"

/// Data compression level (0=no compression, 1=fast compressions, 3=normal compression)
#ifndef DOC_DATA_COMPRESSION_LEVEL
#define DOC_DATA_COMPRESSION_LEVEL 1
#endif

/// Define to store new text nodes as persistent text, instead of mutable
#define USE_PERSISTENT_TEXT 1

/// Default buffer size, was 0xA00000 in pyre crengine
#ifndef DOC_BUFFER_SIZE
#define DOC_BUFFER_SIZE 0x1000000
#endif

#define TEXT_CACHE_UNPACKED_SPACE (25*DOC_BUFFER_SIZE/100)
#define TEXT_CACHE_CHUNK_SIZE     0x008000 // 32K
#define ELEM_CACHE_UNPACKED_SPACE (45*DOC_BUFFER_SIZE/100)
#define ELEM_CACHE_CHUNK_SIZE     0x004000 // 16K
#define RECT_CACHE_UNPACKED_SPACE (15*DOC_BUFFER_SIZE/100)
#define RECT_CACHE_CHUNK_SIZE     0x008000 // 32K
#define STYLE_CACHE_UNPACKED_SPACE (10*DOC_BUFFER_SIZE/100)
#define STYLE_CACHE_CHUNK_SIZE    0x00C000 // 48K

//#define TRACE_AUTOBOX
#define RECT_DATA_CHUNK_ITEMS_SHIFT 11
#define STYLE_DATA_CHUNK_ITEMS_SHIFT 12

#define PACK_BUF_SIZE 0x10000
#define UNPACK_BUF_SIZE 0x40000

#define RECT_DATA_CHUNK_ITEMS (1<<RECT_DATA_CHUNK_ITEMS_SHIFT)
#define RECT_DATA_CHUNK_SIZE (RECT_DATA_CHUNK_ITEMS*sizeof(lvdomElementFormatRec))
#define RECT_DATA_CHUNK_MASK (RECT_DATA_CHUNK_ITEMS-1)

#define STYLE_DATA_CHUNK_ITEMS (1<<STYLE_DATA_CHUNK_ITEMS_SHIFT)
#define STYLE_DATA_CHUNK_SIZE (STYLE_DATA_CHUNK_ITEMS*sizeof(ldomNodeStyleInfo))
#define STYLE_DATA_CHUNK_MASK (STYLE_DATA_CHUNK_ITEMS-1)

#define STYLE_HASH_TABLE_SIZE     512
#define FONT_HASH_TABLE_SIZE      256

static int NextDomIndex = 0;
CrDom* ldomNode::_domInstances[MAX_DOM_INSTANCES] = {NULL};

/// adds document to list, returns ID of allocated document, -1 if no space in instance array
int ldomNode::registerDom(CrDom* doc)
{
    for (int i = 0; i < MAX_DOM_INSTANCES; i++) {
        if (NextDomIndex < 0 || NextDomIndex >= MAX_DOM_INSTANCES) {
            NextDomIndex = 0;
        }
        if (_domInstances[NextDomIndex] == NULL) {
            _domInstances[NextDomIndex] = doc;
            return NextDomIndex++;
        }
        NextDomIndex++;
    }
    return -1;
}

/// removes document from list
void ldomNode::unregisterDom(CrDom* doc)
{
    for (int i = 0; i < MAX_DOM_INSTANCES; i++) {
        if (_domInstances[i] == doc) {
            _domInstances[i] = NULL;
        }
    }
}

/// mutable text node
class ldomTextNode
{
    lUInt32 _parentIndex;
    lString8 _text;
public:

    lUInt32 getParentIndex()
    {
        return _parentIndex;
    }

    void setParentIndex( lUInt32 n )
    {
        _parentIndex = n;
    }

    ldomTextNode( lUInt32 parentIndex, const lString8 & text )
    : _parentIndex(parentIndex), _text(text)
    {
    }

    lString8 getText()
    {
        return _text;
    }

    lString16 getText16()
    {
        return Utf8ToUnicode(_text);
    }

    void setText( const lString8 & s )
    {
        _text = s;
    }

    void setText( const lString16 & s )
    {
        _text = UnicodeToUtf8(s);
    }
};

/// pack data from _buf to _compbuf
bool ldomPack( const lUInt8 * buf, int bufsize, lUInt8 * &dstbuf, lUInt32 & dstsize );
/// unpack data from _compbuf to _buf
bool ldomUnpack( const lUInt8 * compbuf, int compsize, lUInt8 * &dstbuf, lUInt32 & dstsize  );

lUInt32 calcGlobalSettingsHash(int documentId)
{
    lUInt32 hash = 0;
    if (fontMan->getKerning()) {
        hash += 127365;
    }
    hash = hash * 31 + fontMan->GetFontListHash(documentId);
    hash = hash * 31 + (int) fontMan->GetHintingMode();
    if (gFlgFloatingPunctuationEnabled) {
        hash = hash * 75 + 1761;
    }
    if (HyphMan::getSelectedDictionary() == NULL) {
        hash = hash * 31 + 123;
    } else {
        hash = hash * 31 + HyphMan::getSelectedDictionary()->getHash();
    }
    return hash;
}

static void dumpRendMethods( ldomNode * node, lString16 prefix )
{
    lString16 name = prefix;
    if ( node->isText() )
        name << node->getText();
    else
        name << "<" << node->getNodeName() << ">   " << fmt::decimal(node->getRendMethod());
    CRLog::trace( "%s ",LCSTR(name) );
    for ( int i=0; i<node->getChildCount(); i++ ) {
        dumpRendMethods( node->getChildNode(i), prefix + "   ");
    }
}

// BLOB storage

class ldomBlobItem {
    int _storageIndex;
    lString16 _name;
    int _size;
    lUInt8 * _data;
public:
    ldomBlobItem( lString16 name ) : _storageIndex(-1), _name(name), _size(0), _data(NULL) {

    }
    ~ldomBlobItem() {
        if ( _data )
            delete[] _data;
    }
    int getSize() { return _size; }
    int getIndex() { return _storageIndex; }
    lUInt8 * getData() { return _data; }
    lString16 getName() { return _name; }
    void setIndex(int index, int size) {
        if ( _data )
            delete[] _data;
        _data = NULL;
        _storageIndex = index;
        _size = size;
    }
    void setData( const lUInt8 * data, int size ) {
        if ( _data )
            delete[] _data;
        if (data && size>0) {
            _data = new lUInt8[size];
            memcpy(_data, data, size);
            _size = size;
        } else {
            _data = NULL;
            _size = -1;
        }
    }
};

ldomBlobCache::ldomBlobCache() : _changed(false)
{

}

bool ldomBlobCache::addBlob(const lUInt8 * data, int size, lString16 name)
{
    ldomBlobItem* item = new ldomBlobItem(name);
    item->setData(data, size);
    _list.add(item);
    _changed = true;
    return true;
}

LVStreamRef ldomBlobCache::getBlob( lString16 name )
{
    ldomBlobItem* item = NULL;
    for (int i=0; i<_list.length(); i++) {
        if (_list[i]->getName() == name) {
            item = _list[i];
            break;
        }
    }
    if (item && item->getData()) {
        // RAM
        return LVCreateMemoryStream(item->getData(), item->getSize(), true);
    }
    return LVStreamRef();
}

//#define DEBUG_RENDER_RECT_ACCESS
#ifdef DEBUG_RENDER_RECT_ACCESS
  static signed char render_rect_flags[200000]={0};
  static void rr_lock( ldomNode * node )
  {
    int index = node->getDataIndex()>>4;
    CRLog::trace("RenderRectAccessor(%d) lock", index );
    if ( render_rect_flags[index] )
        crFatalError(123, "render rect accessor: cannot get lock");
    render_rect_flags[index] = 1;
  }
  static void rr_unlock( ldomNode * node )
  {
    int index = node->getDataIndex()>>4;
    CRLog::trace("RenderRectAccessor(%d) lock", index );
    if ( !render_rect_flags[index] )
        crFatalError(123, "render rect accessor: unlock w/o lock");
    render_rect_flags[index] = 0;
  }
#endif

RenderRectAccessor::RenderRectAccessor( ldomNode * node )
: _node(node), _modified(false), _dirty(false)
{
#ifdef DEBUG_RENDER_RECT_ACCESS
    rr_lock( _node );
#endif
    _node->getRenderData(*this);
}

RenderRectAccessor::~RenderRectAccessor()
{
    if ( _modified )
        _node->setRenderData(*this);
#ifdef DEBUG_RENDER_RECT_ACCESS
    if ( !_dirty )
        rr_unlock( _node );
#endif
}

void RenderRectAccessor::push()
{
    if ( _modified ) {
        _node->setRenderData(*this);
        _modified = false;
        _dirty = true;
#ifdef DEBUG_RENDER_RECT_ACCESS
            rr_unlock( _node );
#endif
    }
}

void RenderRectAccessor::setX( int x )
{
    if ( _dirty ) {
        _dirty = false;
        _node->getRenderData(*this);
#ifdef DEBUG_RENDER_RECT_ACCESS
        rr_lock( _node );
#endif
    }
    if ( _x != x ) {
        _x = x;
        _modified = true;
    }
}
void RenderRectAccessor::setY( int y )
{
    if ( _dirty ) {
        _dirty = false;
        _node->getRenderData(*this);
#ifdef DEBUG_RENDER_RECT_ACCESS
        rr_lock( _node );
#endif
    }
    if ( _y != y ) {
        _y = y;
        _modified = true;
    }
}
void RenderRectAccessor::setWidth( int w )
{
    if ( _dirty ) {
        _dirty = false;
        _node->getRenderData(*this);
#ifdef DEBUG_RENDER_RECT_ACCESS
        rr_lock( _node );
#endif
    }
    if ( _width != w ) {
        _width = w;
        _modified = true;
    }
}
void RenderRectAccessor::setHeight( int h )
{
    if ( _dirty ) {
        _dirty = false;
        _node->getRenderData(*this);
#ifdef DEBUG_RENDER_RECT_ACCESS
        rr_lock( _node );
#endif
    }
    if ( _height != h ) {
        _height = h;
        _modified = true;
    }
}

int RenderRectAccessor::getX()
{
    if ( _dirty ) {
        _dirty = false;
        _node->getRenderData(*this);
#ifdef DEBUG_RENDER_RECT_ACCESS
        rr_lock( _node );
#endif
    }
    return _x;
}
int RenderRectAccessor::getY()
{
    if ( _dirty ) {
        _dirty = false;
        _node->getRenderData(*this);
#ifdef DEBUG_RENDER_RECT_ACCESS
        rr_lock( _node );
#endif
    }
    return _y;
}
int RenderRectAccessor::getWidth()
{
    if ( _dirty ) {
        _dirty = false;
        _node->getRenderData(*this);
#ifdef DEBUG_RENDER_RECT_ACCESS
        rr_lock( _node );
#endif
    }
    return _width;
}
int RenderRectAccessor::getHeight()
{
    if ( _dirty ) {
        _dirty = false;
        _node->getRenderData(*this);
#ifdef DEBUG_RENDER_RECT_ACCESS
        rr_lock( _node );
#endif
    }
    return _height;
}
void RenderRectAccessor::getRect( lvRect & rc )
{
    if ( _dirty ) {
        _dirty = false;
        _node->getRenderData(*this);
#ifdef DEBUG_RENDER_RECT_ACCESS
        rr_lock( _node );
#endif
    }
    rc.left = _x;
    rc.top = _y;
    rc.right = _x + _width;
    rc.bottom = _y + _height;
}

class ldomPersistentText;
class ldomPersistentElement;

/// common header for data storage items
struct DataStorageItemHeader {
    /// item type: LXML_TEXT_NODE, LXML_ELEMENT_NODE, LXML_NO_DATA
    lUInt16 type;
    /// size of item / 16
    lUInt16 sizeDiv16;
    /// data index of this node in document
    lInt32 dataIndex;
    /// data index of parent node in document, 0 means no parent
    lInt32 parentIndex;
};

/// text node storage implementation
struct TextDataStorageItem : public DataStorageItemHeader {
    /// utf8 text length, characters
    lUInt16 length;
    /// utf8 text, w/o zero
    lChar8 text[2]; // utf8 text follows here, w/o zero byte at end
    /// return text
    inline lString16 getText() { return Utf8ToUnicode( text, length ); }
    inline lString8 getText8() { return lString8( text, length ); }
};

/// element node data storage
struct ElementDataStorageItem : public DataStorageItemHeader {
    lUInt16 id;
    lUInt16 nsid;
    lInt16  attrCount;
    lUInt8  rendMethod;
    lUInt8  reserved8;
    lInt32  childCount;
    lInt32  children[1];
    lUInt16 * attrs() { return (lUInt16 *)(children + childCount); }

    lxmlAttribute* attr(int index)
    {
        return (lxmlAttribute*) &(((lUInt16*) (children + childCount))[index * 3]);
    }

    lUInt16 getAttrValueId( lUInt16 ns, lUInt16 id )
    {
        lUInt16 * a = attrs();
        for ( int i=0; i<attrCount; i++ ) {
            lxmlAttribute * attr = (lxmlAttribute *)(&a[i*3]);
            if ( !attr->compare( ns, id ) )
                continue;
            return  attr->index;
        }
        return LXML_ATTR_VALUE_NONE;
    }
    lxmlAttribute * findAttr( lUInt16 ns, lUInt16 id )
    {
        lUInt16 * a = attrs();
        for ( int i=0; i<attrCount; i++ ) {
            lxmlAttribute * attr = (lxmlAttribute *)(&a[i*3]);
            if ( attr->compare( ns, id ) )
                return attr;
        }
        return NULL;
    }
    // TODO: add items here
    //css_style_ref_t _style;
    //font_ref_t      _font;
};

CrDomBase::CrDomBase()
		: _textCount(0),
		  _textNextFree(0),
		  _elemCount(0)
		, _elemNextFree(0)
		, _styles(STYLE_HASH_TABLE_SIZE)
		, _fonts(FONT_HASH_TABLE_SIZE)
		, _tinyElementCount(0)
		, _itemCount(0)
		, _renderedBlockCache( 32 )
		, _mapped(false)
		, _maperror(false)
		, _mapSavingStage(0)
		, _minSpaceCondensingPercent(DEF_MIN_SPACE_CONDENSING_PERCENT)
		 // persistent text node data storage
		, _textStorage(this, 't', TEXT_CACHE_UNPACKED_SPACE, TEXT_CACHE_CHUNK_SIZE )
		 // persistent element data storage
		, _elemStorage(this, 'e', ELEM_CACHE_UNPACKED_SPACE, ELEM_CACHE_CHUNK_SIZE )
		// element render rect storage
		, _rectStorage(this, 'r', RECT_CACHE_UNPACKED_SPACE, RECT_CACHE_CHUNK_SIZE )
		// element style info storage
		, _styleStorage(this, 's', STYLE_CACHE_UNPACKED_SPACE, STYLE_CACHE_CHUNK_SIZE )
		,_docProps(LVCreatePropsContainer())
		,_docFlags(DOC_FLAG_EMBEDDED_STYLES | DOC_FLAG_ENABLE_FOOTNOTES | DOC_FLAG_EMBEDDED_FONTS)
		,_fontMap(113)
{
    memset( _textList, 0, sizeof(_textList) );
    memset( _elemList, 0, sizeof(_elemList) );
    _docIndex = ldomNode::registerDom((CrDom*) this);
}

void CrDomBase::clearNodeStyle( lUInt32 dataIndex )
{
    ldomNodeStyleInfo info;
    _styleStorage.getStyleData( dataIndex, &info );
    _styles.release( info._styleIndex );
    _fonts.release( info._fontIndex );
    info._fontIndex = info._styleIndex = 0;
    _styleStorage.setStyleData( dataIndex, &info );
}

void CrDomBase::setNodeStyleIndex( lUInt32 dataIndex, lUInt16 index )
{
    ldomNodeStyleInfo info;
    _styleStorage.getStyleData( dataIndex, &info );
    if ( info._styleIndex!=index ) {
        info._styleIndex = index;
        _styleStorage.setStyleData( dataIndex, &info );
    }
}

void CrDomBase::setNodeFontIndex( lUInt32 dataIndex, lUInt16 index )
{
    ldomNodeStyleInfo info;
    _styleStorage.getStyleData( dataIndex, &info );
    if ( info._fontIndex!=index ) {
        info._fontIndex = index;
        _styleStorage.setStyleData( dataIndex, &info );
    }
}

lUInt16 CrDomBase::getNodeStyleIndex( lUInt32 dataIndex )
{
    ldomNodeStyleInfo info;
    _styleStorage.getStyleData( dataIndex, &info );
    return info._styleIndex;
}

css_style_ref_t CrDomBase::getNodeStyle( lUInt32 dataIndex )
{
    ldomNodeStyleInfo info;
    _styleStorage.getStyleData( dataIndex, &info );
    css_style_ref_t res =  _styles.get( info._styleIndex );
    return res;
}

font_ref_t CrDomBase::getNodeFont( lUInt32 dataIndex )
{
    ldomNodeStyleInfo info;
    _styleStorage.getStyleData( dataIndex, &info );
    return _fonts.get( info._fontIndex );
}

void CrDomBase::setNodeStyle( lUInt32 dataIndex, css_style_ref_t & v )
{
    ldomNodeStyleInfo info;
    _styleStorage.getStyleData( dataIndex, &info );
    _styles.cache( info._styleIndex, v );
    _styleStorage.setStyleData( dataIndex, &info );
}

void CrDomBase::setNodeFont( lUInt32 dataIndex, font_ref_t & v )
{
    ldomNodeStyleInfo info;
    _styleStorage.getStyleData( dataIndex, &info );
    _fonts.cache( info._fontIndex, v );
    _styleStorage.setStyleData( dataIndex, &info );
}

lUInt16 CrDomBase::getNodeFontIndex( lUInt32 dataIndex )
{
    ldomNodeStyleInfo info;
    _styleStorage.getStyleData( dataIndex, &info );
    return info._fontIndex;
}

/// get ldomNode instance pointer
ldomNode * CrDomBase::getTinyNode( lUInt32 index )
{
    if ( !index )
        return NULL;
    if ( index & 1 ) // element
        return &(_elemList[index>>TNC_PART_INDEX_SHIFT][(index>>4)&TNC_PART_MASK]);
    else // text
        return &(_textList[index>>TNC_PART_INDEX_SHIFT][(index>>4)&TNC_PART_MASK]);
}

/// allocate new tiny node
ldomNode * CrDomBase::allocTinyNode( int type )
{
    ldomNode * res;
    if ( type & 1 ) {
        // allocate Element
        if ( _elemNextFree ) {
            // reuse existing free item
            int index = (_elemNextFree << 4) | type;
            res = getTinyNode(index);
            res->_handle._dataIndex = index;
            _elemNextFree = res->_data._nextFreeIndex;
        } else {
            // create new item
            _elemCount++;
            ldomNode * part = _elemList[_elemCount >> TNC_PART_SHIFT];
            if ( !part ) {
                part = (ldomNode*)malloc( sizeof(ldomNode) * TNC_PART_LEN );
                memset( part, 0, sizeof(ldomNode) * TNC_PART_LEN );
                _elemList[ _elemCount >> TNC_PART_SHIFT ] = part;
            }
            res = &part[_elemCount & TNC_PART_MASK];
            res->setDocumentIndex( _docIndex );
            res->_handle._dataIndex = (_elemCount << 4) | type;
        }
        _itemCount++;
    } else {
        // allocate Text
        if ( _textNextFree ) {
            // reuse existing free item
            int index = (_textNextFree << 4) | type;
            res = getTinyNode(index);
            res->_handle._dataIndex = index;
            _textNextFree = res->_data._nextFreeIndex;
        } else {
            // create new item
            _textCount++;
            ldomNode * part = _textList[_textCount >> TNC_PART_SHIFT];
            if ( !part ) {
                part = (ldomNode*)malloc( sizeof(ldomNode) * TNC_PART_LEN );
                memset( part, 0, sizeof(ldomNode) * TNC_PART_LEN );
                _textList[ _textCount >> TNC_PART_SHIFT ] = part;
            }
            res = &part[_textCount & TNC_PART_MASK];
            res->setDocumentIndex( _docIndex );
            res->_handle._dataIndex = (_textCount << 4) | type;
        }
        _itemCount++;
    }
    return res;
}

void CrDomBase::recycleTinyNode( lUInt32 index )
{
    if ( index & 1 ) {
        // element
        index >>= 4;
        ldomNode * part = _elemList[index >> TNC_PART_SHIFT];
        ldomNode * p = &part[index & TNC_PART_MASK];
        p->_handle._dataIndex = 0; // indicates NULL node
        p->_data._nextFreeIndex = _elemNextFree;
        _elemNextFree = index;
        _itemCount--;
    } else {
        // text
        index >>= 4;
        ldomNode * part = _textList[index >> TNC_PART_SHIFT];
        ldomNode * p = &part[index & TNC_PART_MASK];
        p->_handle._dataIndex = 0; // indicates NULL node
        p->_data._nextFreeIndex = _textNextFree;
        _textNextFree = index;
        _itemCount--;
    }
}

CrDomBase::~CrDomBase()
{
    // clear all elem parts
    for ( int partindex = 0; partindex<=(_elemCount>>TNC_PART_SHIFT); partindex++ ) {
        ldomNode * part = _elemList[partindex];
        if ( part ) {
            int n0 = TNC_PART_LEN * partindex;
            for ( int i=0; i<TNC_PART_LEN && n0+i<=_elemCount; i++ )
                part[i].onCollectionDestroy();
            free(part);
            _elemList[partindex] = NULL;
        }
    }
    // clear all text parts
    for ( int partindex = 0; partindex<=(_textCount>>TNC_PART_SHIFT); partindex++ ) {
        ldomNode * part = _textList[partindex];
        if ( part ) {
            int n0 = TNC_PART_LEN * partindex;
            for ( int i=0; i<TNC_PART_LEN && n0+i<=_textCount; i++ )
                part[i].onCollectionDestroy();
            free(part);
            _textList[partindex] = NULL;
        }
    }
    ldomNode::unregisterDom((CrDom*)this);
}

/// get chunk pointer and update usage data
ldomTextStorageChunk * ldomDataStorageManager::getChunk( lUInt32 address )
{
    ldomTextStorageChunk* chunk = _chunks[address>>16];
    if ( chunk!=_recentChunk ) {
        if ( chunk->_prevRecent )
            chunk->_prevRecent->_nextRecent = chunk->_nextRecent;
        if ( chunk->_nextRecent )
            chunk->_nextRecent->_prevRecent = chunk->_prevRecent;
        chunk->_prevRecent = NULL;
        if (((chunk->_nextRecent = _recentChunk)))
            _recentChunk->_prevRecent = chunk;
        _recentChunk = chunk;
    }
    return chunk;
}

/// get or allocate space for element style data item
void ldomDataStorageManager::getStyleData(lUInt32 elemDataIndex, ldomNodeStyleInfo* dst)
{
    // assume storage has raw data chunks
    int index = elemDataIndex >> 4; // element sequential index
    int chunkIndex = index >> STYLE_DATA_CHUNK_ITEMS_SHIFT;
    while (_chunks.length() <= chunkIndex) {
        //if ( _chunks.length()>0 )
        //    _chunks[_chunks.length()-1]->compact();
        _chunks.add(new ldomTextStorageChunk(STYLE_DATA_CHUNK_SIZE, this, _chunks.length()));
        getChunk((_chunks.length() - 1) << 16);
        compact(0);
    }
    ldomTextStorageChunk* chunk = getChunk(chunkIndex << 16);
    int offsetIndex = index & STYLE_DATA_CHUNK_MASK;
    chunk->getRaw(offsetIndex * sizeof(ldomNodeStyleInfo),
            sizeof(ldomNodeStyleInfo),
            (lUInt8*) dst);
}

/// set element style data item
void ldomDataStorageManager::setStyleData(lUInt32 elemDataIndex, const ldomNodeStyleInfo* src)
{
    // assume storage has raw data chunks
    int index = elemDataIndex >> 4; // element sequential index
    int chunkIndex = index >> STYLE_DATA_CHUNK_ITEMS_SHIFT;
    while (_chunks.length() < chunkIndex) {
        //if ( _chunks.length()>0 )
        //    _chunks[_chunks.length()-1]->compact();
        _chunks.add(new ldomTextStorageChunk(STYLE_DATA_CHUNK_SIZE, this, _chunks.length()));
        getChunk((_chunks.length() - 1) << 16);
        compact(0);
    }
    ldomTextStorageChunk* chunk = getChunk(chunkIndex << 16);
    int offsetIndex = index & STYLE_DATA_CHUNK_MASK;
    chunk->setRaw(offsetIndex * sizeof(ldomNodeStyleInfo),
            sizeof(ldomNodeStyleInfo),
            (const lUInt8*) src);
}

/// get or allocate space for rect data item
void ldomDataStorageManager::getRendRectData(lUInt32 elemDataIndex, lvdomElementFormatRec* dst)
{
    // assume storage has raw data chunks
    int index = elemDataIndex >> 4; // element sequential index
    int chunkIndex = index >> RECT_DATA_CHUNK_ITEMS_SHIFT;
    while (_chunks.length() <= chunkIndex) {
        //if ( _chunks.length()>0 )
        //    _chunks[_chunks.length()-1]->compact();
        _chunks.add(new ldomTextStorageChunk(RECT_DATA_CHUNK_SIZE, this, _chunks.length()));
        getChunk((_chunks.length() - 1) << 16);
        compact(0);
    }
    ldomTextStorageChunk* chunk = getChunk(chunkIndex << 16);
    int offsetIndex = index & RECT_DATA_CHUNK_MASK;
    chunk->getRaw(offsetIndex * sizeof(lvdomElementFormatRec),
            sizeof(lvdomElementFormatRec),
            (lUInt8*) dst);
}

/// set rect data item
void ldomDataStorageManager::setRendRectData(
        lUInt32 elemDataIndex,
        const lvdomElementFormatRec* src)
{
    // assume storage has raw data chunks
    int index = elemDataIndex >> 4; // element sequential index
    int chunkIndex = index >> RECT_DATA_CHUNK_ITEMS_SHIFT;
    while (_chunks.length() < chunkIndex) {
        //if ( _chunks.length()>0 )
        //    _chunks[_chunks.length()-1]->compact();
        _chunks.add(new ldomTextStorageChunk(RECT_DATA_CHUNK_SIZE, this, _chunks.length()));
        getChunk((_chunks.length() - 1) << 16);
        compact(0);
    }
    ldomTextStorageChunk* chunk = getChunk(chunkIndex << 16);
    int offsetIndex = index & RECT_DATA_CHUNK_MASK;
    chunk->setRaw(offsetIndex * sizeof(lvdomElementFormatRec),
            sizeof(lvdomElementFormatRec),
            (const lUInt8*) src);
}

lUInt32 ldomDataStorageManager::allocText(
        lUInt32 dataIndex,
        lUInt32 parentIndex,
        const lString8& text)
{
    if (!_activeChunk) {
        _activeChunk = new ldomTextStorageChunk(this, _chunks.length());
        _chunks.add(_activeChunk);
        getChunk((_chunks.length() - 1) << 16);
        compact(0);
    }
    int offset = _activeChunk->addText(dataIndex, parentIndex, text);
    if (offset < 0) {
        // no space in current chunk, add one more chunk
        //_activeChunk->compact();
        _activeChunk = new ldomTextStorageChunk(this, _chunks.length());
        _chunks.add(_activeChunk);
        getChunk((_chunks.length() - 1) << 16);
        compact(0);
        offset = _activeChunk->addText(dataIndex, parentIndex, text);
        if (offset < 0) {
            crFatalError(1001, "Unexpected error while allocation of text");
        }
    }
    return offset | (_activeChunk->getIndex() << 16);
}

lUInt32 ldomDataStorageManager::allocElem(lUInt32 dataIndex,
        lUInt32 parentIndex,
        int childCount,
        int attrCount)
{
    if (!_activeChunk) {
        _activeChunk = new ldomTextStorageChunk(this, _chunks.length());
        _chunks.add(_activeChunk);
        getChunk((_chunks.length() - 1) << 16);
        compact(0);
    }
    int offset = _activeChunk->addElem(dataIndex, parentIndex, childCount, attrCount);
    if (offset < 0) {
        // no space in current chunk, add one more chunk
        //_activeChunk->compact();
        _activeChunk = new ldomTextStorageChunk(this, _chunks.length());
        _chunks.add(_activeChunk);
        getChunk((_chunks.length() - 1) << 16);
        compact(0);
        offset = _activeChunk->addElem(dataIndex, parentIndex, childCount, attrCount);
        if (offset < 0) {
            crFatalError(1002, "Unexpected error while allocation of element");
        }
    }
    return offset | (_activeChunk->getIndex() << 16);
}

/// call to invalidate chunk if content is modified
void ldomDataStorageManager::modified( lUInt32 addr )
{
    ldomTextStorageChunk * chunk = getChunk(addr);
    chunk->modified();
}

/// change node's parent
bool ldomDataStorageManager::setParent( lUInt32 address, lUInt32 parent )
{
    ldomTextStorageChunk * chunk = getChunk(address);
    return chunk->setParent(address&0xFFFF, parent);
}

/// free data item
void ldomDataStorageManager::freeNode( lUInt32 addr )
{
    ldomTextStorageChunk * chunk = getChunk(addr);
    chunk->freeNode(addr&0xFFFF);
}

lString8 ldomDataStorageManager::getText( lUInt32 address )
{
    ldomTextStorageChunk * chunk = getChunk(address);
    return chunk->getText(address&0xFFFF);
}

/// get pointer to element data
ElementDataStorageItem * ldomDataStorageManager::getElem( lUInt32 addr )
{
    ldomTextStorageChunk * chunk = getChunk(addr);
    return chunk->getElem(addr&0xFFFF);
}

/// returns node's parent by address
lUInt32 ldomDataStorageManager::getParent( lUInt32 addr )
{
    ldomTextStorageChunk * chunk = getChunk(addr);
    return chunk->getElem(addr&0xFFFF)->parentIndex;
}

void ldomDataStorageManager::compact(int reservedSpace)
{
    if (_uncompressedSize + reservedSpace
        > _maxUncompressedSize + _maxUncompressedSize / 10) { // allow +10% overflow
        // do compacting
        int sumsize = reservedSpace;
        for (ldomTextStorageChunk* p = _recentChunk; p; p = p->_nextRecent) {
            if (p->_bufsize >= 0) {
                if ((int) p->_bufsize + sumsize < _maxUncompressedSize
                    || (p == _activeChunk && reservedSpace < 0xFFFFFFF)) {
                    // fits
                    sumsize += p->_bufsize;
                }
            }
        }
    }
}

ldomDataStorageManager::ldomDataStorageManager(
        CrDomBase* owner, char type, int maxUnpackedSize, int chunkSize)
        : _owner(owner),
          _activeChunk(NULL),
          _recentChunk(NULL),
          _uncompressedSize(0),
          _maxUncompressedSize(maxUnpackedSize),
          _chunkSize(chunkSize),
          _type(type)
{
}

ldomDataStorageManager::~ldomDataStorageManager() { }

ldomTextStorageChunk::ldomTextStorageChunk(
        int preAllocSize,
        ldomDataStorageManager * manager,
        lUInt16 index)
	: _manager(manager)
	, _nextRecent(NULL)
	, _prevRecent(NULL)
        /// buffer for uncompressed data
	, _buf(NULL)
        /// _buf (uncompressed) area size, bytes
	, _bufsize(preAllocSize)
        /// _buf (uncompressed) data write position (for appending of new data)
	, _bufpos(preAllocSize)
        /// ? index of chunk in storage
	, _index(index)
	, _type( manager->_type )
{
    _buf = (lUInt8*)malloc(preAllocSize);
    memset(_buf, 0, preAllocSize);
    _manager->_uncompressedSize += _bufsize;
}

ldomTextStorageChunk::ldomTextStorageChunk(ldomDataStorageManager * manager, lUInt16 index)
	: _manager(manager)
	, _nextRecent(NULL)
	, _prevRecent(NULL)
	, _buf(NULL)   /// buffer for uncompressed data
	, _bufsize(0)    /// _buf (uncompressed) area size, bytes
	, _bufpos(0)     /// _buf (uncompressed) data write position (for appending of new data)
	, _index(index)      /// ? index of chunk in storage
	, _type( manager->_type )
{
}

ldomTextStorageChunk::~ldomTextStorageChunk()
{
    setunpacked(NULL, 0);
}

/// get raw data bytes
void ldomTextStorageChunk::getRaw( int offset, int size, lUInt8 * buf )
{
#ifdef _DEBUG
    if ( !_buf || offset+size>(int)_bufpos || offset+size>(int)_bufsize )
        crFatalError(123, "ldomTextStorageChunk: Invalid raw data buffer position");
#endif
    memcpy( buf, _buf+offset, size );
}

/// set raw data bytes
void ldomTextStorageChunk::setRaw( int offset, int size, const lUInt8 * buf )
{
#ifdef _DEBUG
    if ( !_buf || offset+size>(int)_bufpos || offset+size>(int)_bufsize )
        crFatalError(123, "ldomTextStorageChunk: Invalid raw data buffer position");
#endif
    if (memcmp(_buf+offset, buf, size) != 0) {
        memcpy(_buf+offset, buf, size);
        modified();
    }
}

/// returns free space in buffer
int ldomTextStorageChunk::space()
{
    return _bufsize - _bufpos;
}

/// returns free space in buffer
int ldomTextStorageChunk::addText( lUInt32 dataIndex, lUInt32 parentIndex, const lString8 & text )
{
    int itemsize = (sizeof(TextDataStorageItem)+text.length()-2 + 15) & 0xFFFFFFF0;
    if ( !_buf ) {
        // create new buffer, if necessary
        _bufsize = _manager->_chunkSize > itemsize ? _manager->_chunkSize : itemsize;
        _buf = (lUInt8*)malloc(sizeof(lUInt8) * _bufsize);
        memset(_buf, 0, _bufsize );
        _bufpos = 0;
        _manager->_uncompressedSize += _bufsize;
    }
    if ( (int)_bufsize - (int)_bufpos < itemsize )
        return -1;
    TextDataStorageItem * p = (TextDataStorageItem*)(_buf + _bufpos);
    p->sizeDiv16 = (lUInt16)(itemsize >> 4);
    p->dataIndex = dataIndex;
    p->parentIndex = parentIndex;
    p->type = LXML_TEXT_NODE;
    p->length = (lUInt16)text.length();
    memcpy(p->text, text.c_str(), p->length);
    int res = _bufpos >> 4;
    _bufpos += itemsize;
    return res;
}

/// adds new element item to buffer, returns offset inside chunk of stored data
int ldomTextStorageChunk::addElem(lUInt32 dataIndex, lUInt32 parentIndex, int childCount, int attrCount)
{
    int itemsize = (sizeof(ElementDataStorageItem) + attrCount * sizeof(lUInt16) * 3
                    + childCount * sizeof(lUInt32) - sizeof(lUInt32) + 15) & 0xFFFFFFF0;
    if ( !_buf ) {
        // create new buffer, if necessary
        _bufsize = _manager->_chunkSize > itemsize ? _manager->_chunkSize : itemsize;
        _buf = (lUInt8*)malloc(sizeof(lUInt8) * _bufsize);
        memset(_buf, 0, _bufsize );
        _bufpos = 0;
        _manager->_uncompressedSize += _bufsize;
    }
    if ( _bufsize - _bufpos < (unsigned)itemsize )
        return -1;
    ElementDataStorageItem *item = (ElementDataStorageItem *)(_buf + _bufpos);
    if ( item ) {
        item->sizeDiv16 = (lUInt16)(itemsize >> 4);
        item->dataIndex = dataIndex;
        item->parentIndex = parentIndex;
        item->type = LXML_ELEMENT_NODE;
        item->parentIndex = parentIndex;
        item->attrCount = (lUInt16)attrCount;
        item->childCount = childCount;
    }
    int res = _bufpos >> 4;
    _bufpos += itemsize;
    return res;
}

/// set node parent by offset
bool ldomTextStorageChunk::setParent( int offset, lUInt32 parentIndex )
{
    offset <<= 4;
    if ( offset>=0 && offset<(int)_bufpos ) {
        TextDataStorageItem * item = (TextDataStorageItem *)(_buf+offset);
        if ( (int)parentIndex!=item->parentIndex ) {
            item->parentIndex = parentIndex;
            modified();
            return true;
        } else
            return false;
    }
    CRLog::error("Offset %d is out of bounds (%d) for storage chunk %c%d, chunkCount=%d",
            offset, this->_bufpos, this->_type, this->_index, _manager->_chunks.length() );
    return false;
}

/// get text node parent by offset
lUInt32 ldomTextStorageChunk::getParent( int offset )
{
    offset <<= 4;
    if ( offset>=0 && offset<(int)_bufpos ) {
        TextDataStorageItem * item = (TextDataStorageItem *)(_buf+offset);
        return item->parentIndex;
    }
    CRLog::error("Offset %d is out of bounds (%d) for storage chunk %c%d, chunkCount=%d",
            offset, this->_bufpos, this->_type, this->_index, _manager->_chunks.length() );
    return 0;
}

/// get pointer to element data
ElementDataStorageItem * ldomTextStorageChunk::getElem( int offset  )
{
    offset <<= 4;
    if ( offset>=0 && offset<(int)_bufpos ) {
        ElementDataStorageItem * item = (ElementDataStorageItem *)(_buf+offset);
        return item;
    }
    CRLog::error("Offset %d is out of bounds (%d) for storage chunk %c%d, chunkCount=%d",
            offset, this->_bufpos, this->_type, this->_index, _manager->_chunks.length() );
    return NULL;
}

/// call to invalidate chunk if content is modified
void ldomTextStorageChunk::modified()
{
    if ( !_buf ) {
        CRLog::error("Modified is called for node which is not in memory");
    }
}

/// free data item
void ldomTextStorageChunk::freeNode( int offset )
{
    offset <<= 4;
    if ( offset>=0 && offset<(int)_bufpos ) {
        TextDataStorageItem * item = (TextDataStorageItem *)(_buf+offset);
        if ( (item->type==LXML_TEXT_NODE || item->type==LXML_ELEMENT_NODE) && item->dataIndex ) {
            item->type = LXML_NO_DATA;
            item->dataIndex = 0;
            modified();
        }
    }
}

/// get text item from buffer by offset
lString8 ldomTextStorageChunk::getText( int offset )
{
    offset <<= 4;
    if ( offset>=0 && offset<(int)_bufpos ) {
        TextDataStorageItem * item = (TextDataStorageItem *)(_buf+offset);
        return item->getText8();
    }
    return lString8::empty_str;
}

/// pack data from _buf to _compbuf
bool ldomPack( const lUInt8 * buf, int bufsize, lUInt8 * &dstbuf, lUInt32 & dstsize )
{
    lUInt8 tmp[PACK_BUF_SIZE]; // 64K buffer for compressed data
    int ret;
    z_stream z;
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;
    ret = deflateInit( &z, DOC_DATA_COMPRESSION_LEVEL );
    if (ret != Z_OK)
        return false;
    z.avail_in = bufsize;
    z.next_in = (unsigned char *)buf;
    z.avail_out = PACK_BUF_SIZE;
    z.next_out = tmp;
    ret = deflate( &z, Z_FINISH );
    int have = PACK_BUF_SIZE - z.avail_out;
    deflateEnd(&z);
    if ( ret != Z_STREAM_END || have==0 || have>=PACK_BUF_SIZE || z.avail_in!=0 ) {
        // some error occured while packing, leave unpacked
        //setpacked( buf, bufsize );
        return false;
    }
    dstsize = have;
    dstbuf = (lUInt8 *)malloc(have);
    memcpy( dstbuf, tmp, have );
    return true;
}

/// unpack data from _compbuf to _buf
bool ldomUnpack( const lUInt8 * compbuf, int compsize, lUInt8 * &dstbuf, lUInt32 & dstsize  )
{
    lUInt8 tmp[UNPACK_BUF_SIZE]; // 64K buffer for compressed data
    int ret;
    z_stream z;
    memset( &z, 0, sizeof(z) );
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;
    ret = inflateInit( &z );
    if ( ret != Z_OK )
        return false;
    z.avail_in = compsize;
    z.next_in = (unsigned char *)compbuf;
    z.avail_out = UNPACK_BUF_SIZE;
    z.next_out = tmp;
    ret = inflate( &z, Z_FINISH );
    int have = UNPACK_BUF_SIZE - z.avail_out;
    inflateEnd(&z);
    if ( ret!=Z_STREAM_END || have==0 || have>=UNPACK_BUF_SIZE || z.avail_in!=0 ) {
        // some error occured while unpacking
        return false;
    }
    dstsize = have;
    dstbuf = (lUInt8 *)malloc(have);
    memcpy( dstbuf, tmp, have );
    return true;
}

void ldomTextStorageChunk::setunpacked( const lUInt8 * buf, int bufsize )
{
    if ( _buf ) {
        _manager->_uncompressedSize -= _bufsize;
        free(_buf);
        _buf = NULL;
        _bufsize = 0;
    }
    if ( buf && bufsize ) {
        _bufsize = bufsize;
        _bufpos = bufsize;
        _buf = (lUInt8 *)malloc( sizeof(lUInt8) * bufsize );
        _manager->_uncompressedSize += _bufsize;
        memcpy( _buf, buf, bufsize );
    }
}

/// fastDOM, moved to .cpp to hide implementation
class ldomAttributeCollection
{
private:
    lUInt16 _len;
    lUInt16 _size;
    lxmlAttribute * _list;
public:
    ldomAttributeCollection()
    : _len(0), _size(0), _list(NULL)
    {
    }
    ~ldomAttributeCollection()
    {
        if (_list)
            free(_list);
    }
    lxmlAttribute * operator [] (int index) { return &_list[index]; }
    const lxmlAttribute * operator [] (int index) const { return &_list[index]; }
    lUInt16 length() const
    {
        return _len;
    }
    lUInt16 get( lUInt16 nsId, lUInt16 attrId ) const
    {
        for (lUInt16 i=0; i<_len; i++)
        {
            if (_list[i].compare( nsId, attrId ))
                return _list[i].index;
        }
        return LXML_ATTR_VALUE_NONE;
    }
    void set( lUInt16 nsId, lUInt16 attrId, lUInt16 valueIndex )
    {
        // find existing
        for (lUInt16 i=0; i<_len; i++)
        {
            if (_list[i].compare( nsId, attrId ))
            {
                _list[i].index = valueIndex;
                return;
            }
        }
        // add
        if (_len>=_size)
        {
            _size += 4;
            _list = cr_realloc( _list, _size );
        }
        _list[ _len++ ].setData(nsId, attrId, valueIndex);
    }
    void add( lUInt16 nsId, lUInt16 attrId, lUInt16 valueIndex )
    {
        // find existing
        if (_len>=_size)
        {
            _size += 4;
            _list = cr_realloc( _list, _size );
        }
        _list[ _len++ ].setData(nsId, attrId, valueIndex);
    }
    void add( const lxmlAttribute * v )
    {
        // find existing
        if (_len>=_size)
        {
            _size += 4;
            _list = cr_realloc( _list, _size );
        }
        _list[ _len++ ] = *v;
    }
};

CrDomXml::CrDomXml()
		: _elementNameTable(MAX_ELEMENT_TYPE_ID),
		  _attrNameTable(MAX_ATTRIBUTE_TYPE_ID),
		  _nsNameTable(MAX_NAMESPACE_TYPE_ID),
		  _nextUnknownElementId(UNKNOWN_ELEMENT_TYPE_ID),
		  _nextUnknownAttrId(UNKNOWN_ATTRIBUTE_TYPE_ID),
		  _nextUnknownNsId(UNKNOWN_NAMESPACE_TYPE_ID),
		  _attrValueTable( DOC_STRING_HASH_SIZE ),
		  _idNodeMap(8192),
		  _urlImageMap(1024),
		  _idAttrId(0),
		  _nameAttrId(0),
		  //_keepData(false),
		  //_mapped(false)
		  _pagesData(8192)
{
    // create and add one data buffer
   stylesheet_.setDocument(this);
}

CrDomXml::~CrDomXml() {}

void CrDomXml::onAttributeSet( lUInt16 attrId, lUInt16 valueId, ldomNode * node )
{
    if ( _idAttrId==0 )
        _idAttrId = _attrNameTable.IntByStr("id");
    if ( _nameAttrId==0 )
        _nameAttrId = _attrNameTable.IntByStr("name");
    if (attrId == _idAttrId) {
        _idNodeMap.set( valueId, node->getDataIndex() );
    } else if ( attrId==_nameAttrId ) {
        lString16 nodeName = node->getNodeName();
        if (nodeName == "a")
            _idNodeMap.set( valueId, node->getDataIndex() );
    }
}

lUInt16 CrDomXml::getNsNameIndex( const lChar16 * name )
{
    const CrStrIntPair * item = _nsNameTable.FindPair( name );
    if (item)
        return item->id;
    _nsNameTable.AddItem( _nextUnknownNsId, lString16(name), NULL );
    return _nextUnknownNsId++;
}

lUInt16 CrDomXml::getNsNameIndex( const lChar8 * name )
{
    const CrStrIntPair * item = _nsNameTable.FindPair( name );
    if (item)
        return item->id;
    _nsNameTable.AddItem( _nextUnknownNsId, lString16(name), NULL );
    return _nextUnknownNsId++;
}

lUInt16 CrDomXml::getAttrNameIndex(const lChar16* name) {
    const CrStrIntPair* item = _attrNameTable.FindPair(name);
    if (item) {
    	//CRLog::warn("Found item: %s", LCSTR(lString16(name)));
        return item->id;
    }
    _attrNameTable.AddItem(_nextUnknownAttrId, lString16(name), NULL);
    //CRLog::warn("Added item: %s", LCSTR(lString16(name)));
    return _nextUnknownAttrId++;
}

lUInt16 CrDomXml::getAttrNameIndex(const lChar8* name) {
    const CrStrIntPair* item = _attrNameTable.FindPair(name);
    if (item) {
    	//CRLog::warn("Found item: %s", LCSTR(lString16(name)));
        return item->id;
    }
    _attrNameTable.AddItem(_nextUnknownAttrId, lString16(name), NULL);
    //CRLog::warn("Added item: %s", LCSTR(lString16(name)));
    return _nextUnknownAttrId++;
}

lUInt16 CrDomXml::getElementNameIndex( const lChar16 * name )
{
    const CrStrIntPair * item = _elementNameTable.FindPair( name );
    if (item)
        return item->id;
    _elementNameTable.AddItem( _nextUnknownElementId, lString16(name), NULL );
    return _nextUnknownElementId++;
}

lUInt16 CrDomXml::findElementNameIndex( const lChar8 * name )
{
    const CrStrIntPair * item = _elementNameTable.FindPair( name );
    if (item)
        return item->id;
    return 0;
}

lUInt16 CrDomXml::getElementNameIndex( const lChar8 * name )
{
    const CrStrIntPair * item = _elementNameTable.FindPair( name );
    if (item)
        return item->id;
    _elementNameTable.AddItem( _nextUnknownElementId, lString16(name), NULL );
    return _nextUnknownElementId++;
}

/// create formatted text object with options set
LFormattedText * CrDomXml::createFormattedText()
{
    LFormattedText * p = new LFormattedText();
    p->setImageScalingOptions(&_imgScalingOptions);
    p->setMinSpaceCondensingPercent(_minSpaceCondensingPercent);
    p->setHighlightOptions(&_highlightOptions);
    return p;
}

/// returns main element (i.e. FictionBook for FB2)
ldomNode * CrDomXml::getRootNode()
{
    return getTinyNode(17);
}

CrDom::CrDom()
: m_toc(this)
, _last_docflags(0)
, _page_height(0)
, _page_width(0)
, _rendered(false)
, lists(100)
, cfg_txt_indent(1)
, cfg_txt_margin(0)
, cfg_txt_indent_margin_override(false)
, force_render(false)
{
    allocTinyElement(NULL, 0, 0);
    //new ldomElement( this, NULL, 0, 0, 0 );
    //assert( _instanceMapCount==2 );
}

static void writeNode( LVStream * stream, ldomNode * node, bool treeLayout )
{
    int level = 0;
    if ( treeLayout ) {
        level = node->getNodeLevel();
        for (int i=0; i<level; i++ )
            *stream << "  ";
    }
    if ( node->isText() )
    {
        lString8 txt = node->getText8();
        txt = lString8("[") + txt + lString8("]");
        *stream << txt;
        if ( treeLayout )
            *stream << "\n";
    }
    else if (  node->isElement() )
    {
        lString8 elemName = UnicodeToUtf8(node->getNodeName());
        lString8 elemNsName = UnicodeToUtf8(node->getNodeNsName());
        if (!elemNsName.empty())
            elemName = elemNsName + ":" + elemName;
        if (!elemName.empty())
            *stream << "<" << elemName;
        int i;
        for (i=0; i<(int)node->getAttrCount(); i++)
        {
            const lxmlAttribute * attr = node->getAttribute(i);
            if (attr)
            {
                lString8 attrName( UnicodeToUtf8(node->getCrDom()->getAttrName(attr->id)) );
                lString8 nsName( UnicodeToUtf8(node->getCrDom()->getNsName(attr->nsid)) );
                lString8 attrValue( UnicodeToUtf8(node->getCrDom()->getAttrValue(attr->index)) );
                *stream << " ";
                if ( nsName.length() > 0 )
                    *stream << nsName << ":";
                *stream << attrName << "=\"" << attrValue << "\"";
            }
        }
        if ( node->getChildCount() == 0 ) {
            if (!elemName.empty())
            {
                if ( elemName[0] == '?' )
                    *stream << "?>";
                else
                    *stream << "/>";
            }
            if ( treeLayout )
                *stream << "\n";
        } else {
            if (!elemName.empty())
                *stream << ">";
            if ( treeLayout )
                *stream << "\n";
            for (i=0; i<(int)node->getChildCount(); i++)
            {
                writeNode( stream, node->getChildNode(i), treeLayout );
            }
            if ( treeLayout ) {
                for (int i=0; i<level; i++ )
                    *stream << "  ";
            }
            if (!elemName.empty())
                *stream << "</" << elemName << ">";
            if ( treeLayout )
                *stream << "\n";
        }
    }
}

bool CrDom::saveToStream(LVStreamRef stream, const char *, bool treeLayout)
{
    if (!stream || !getRootNode()->getChildCount())
        return false;
    *stream.get() << UnicodeToLocal(cs16(L"\xFEFF"));
    writeNode(stream.get(), getRootNode(), treeLayout);
    return true;
}

CrDom::~CrDom()
{
	stylesheet_.clear();
    fontMan->UnregisterDocumentFonts(_docIndex);
}

class LVImportStylesheetParser
{
private:
    CrDom  *cre_dom_;
    lString16Collection in_progress_;
    int nesting_level_;
public:
    LVImportStylesheetParser(CrDom* cre_dom) : cre_dom_(cre_dom), nesting_level_(0) { }

    ~LVImportStylesheetParser()
    {
        in_progress_.clear();
    }

    bool Parse(lString16 cssFile)
    {
        bool ret = false;
        if ( cssFile.empty() )
            return ret;

        lString16 codeBase = cssFile;
        LVExtractLastPathElement(codeBase);
        LVStreamRef css_stream = cre_dom_->getDocParentContainer()->OpenStream(
                cssFile.c_str(),
                LVOM_READ);
        if (!css_stream.isNull()) {
            lString16 css;
            css << LVReadCssText( css_stream );
            int offset = in_progress_.add(cssFile);
            ret = Parse(codeBase, css) || ret;
            in_progress_.erase(offset, 1);
        }
        return ret;
    }

    bool Parse(lString16 codeBase, lString16 css)
    {
        bool ret = false;
        if (css.empty()) {
            return ret;
        }
        lString8 css8 = UnicodeToUtf8(css);
        const char * s = css8.c_str();

        nesting_level_ += 1;
        while (nesting_level_ < 11) { //arbitrary limit
            lString8 import_file;
            if (LVProcessStyleSheetImport(s, import_file)) {
                lString16 importFilename = LVCombinePaths( codeBase, Utf8ToUnicode(import_file) );
                if ( !importFilename.empty() && !in_progress_.contains(importFilename) ) {
                    ret = Parse(importFilename) || ret;
                }
            } else {
                break;
            }
        }
        nesting_level_ -= 1;
        return (cre_dom_->getStylesheet()->parse(s) || ret);
    }

};

/// renders (formats) document in memory
bool CrDom::setRenderProps(int width, int height, font_ref_t def_font, int interline_space)
{
    bool changed = false;
    _renderedBlockCache.clear();

    //TODO pass here image zoom factor based on device dpi
    // mode: 0=disabled, 1=integer scaling factors, 2=free scaling
    // scale: 0=auto based on font size, 1=no zoom, 2=scale up to *2, 3=scale up to *3
 /*   if (_imgScalingOptions.zoom_in_inline.mode != DEF_IMAGE_SCALE_ZOOM_IN_MODE) {
        _imgScalingOptions.zoom_in_inline.mode = (img_scaling_mode_t) DEF_IMAGE_SCALE_ZOOM_IN_MODE;
        changed = true;
    }
    if (_imgScalingOptions.zoom_in_inline.max_scale != DEF_IMAGE_SCALE_ZOOM_IN_SCALE) {
        _imgScalingOptions.zoom_in_inline.max_scale = DEF_IMAGE_SCALE_ZOOM_IN_SCALE;
        changed = true;
    }
    if (_imgScalingOptions.zoom_in_block.mode != DEF_IMAGE_SCALE_ZOOM_IN_MODE) {
        _imgScalingOptions.zoom_in_block.mode = (img_scaling_mode_t) DEF_IMAGE_SCALE_ZOOM_IN_MODE;
        changed = true;
    }
    if (_imgScalingOptions.zoom_in_block.max_scale != DEF_IMAGE_SCALE_ZOOM_IN_SCALE) {
        _imgScalingOptions.zoom_in_block.max_scale = DEF_IMAGE_SCALE_ZOOM_IN_SCALE;
        changed = true;
    }

    if (_imgScalingOptions.zoom_out_inline.mode != DEF_IMAGE_SCALE_ZOOM_OUT_MODE) {
        _imgScalingOptions.zoom_out_inline.mode = (img_scaling_mode_t) DEF_IMAGE_SCALE_ZOOM_OUT_MODE;
        changed = true;
    }
    if (_imgScalingOptions.zoom_out_inline.max_scale != DEF_IMAGE_SCALE_ZOOM_OUT_SCALE) {
        _imgScalingOptions.zoom_out_inline.max_scale = DEF_IMAGE_SCALE_ZOOM_OUT_SCALE;
        changed = true;
    }
    if (_imgScalingOptions.zoom_out_block.mode != DEF_IMAGE_SCALE_ZOOM_OUT_MODE) {
        _imgScalingOptions.zoom_out_block.mode = (img_scaling_mode_t) DEF_IMAGE_SCALE_ZOOM_OUT_MODE;
        changed = true;
    }
    if (_imgScalingOptions.zoom_out_block.max_scale != DEF_IMAGE_SCALE_ZOOM_OUT_SCALE) {
        _imgScalingOptions.zoom_out_block.max_scale = DEF_IMAGE_SCALE_ZOOM_OUT_SCALE;
        changed = true;
    }*/

    css_style_ref_t s( new css_style_rec_t );
    s->display = css_d_block;
    s->white_space = css_ws_normal;
    s->text_align = css_ta_left;
    s->text_align_last = css_ta_left;
    s->text_decoration = css_td_none;
    s->hyphenate = css_hyph_auto;
    s->color.type = css_val_unspecified;
    s->color.value = 0x000000;
    s->background_color.type = css_val_unspecified;
    s->background_color.value = 0xFFFFFF;
    //_def_style->background_color.type = color;
    //_def_style->background_color.value = 0xFFFFFF;
    s->page_break_before = css_pb_auto;
    s->page_break_after = css_pb_auto;
    s->page_break_inside = css_pb_auto;
    s->list_style_type = css_lst_disc;
    s->list_style_position = css_lsp_outside;
    s->vertical_align = css_va_baseline;
    s->font_family = def_font->getFontFamily();
    s->font_size.type = css_val_px;
    s->font_size.value = def_font->getSize();
    s->font_name = def_font->getTypeFace();
    s->font_weight = css_fw_400;
    s->font_style = css_fs_normal;
    s->text_indent.type = css_val_px;
    s->text_indent.value = 0;
    s->line_height.type = css_val_percent;
    s->line_height.value = interline_space;
    //lUInt32 defStyleHash = (((stylesheet_.getHash() * 31) + calcHash(_def_style))
    //          *31 + calcHash(_def_font));
    //defStyleHash = defStyleHash * 31 + getDocFlags();
    if ( _last_docflags != getDocFlags() ) {
        //CRLog::trace("ldomDocument::setRenderProps() - doc flags changed");
        _last_docflags = getDocFlags();
        changed = true;
    }
    if ( calcHash(_def_style) != calcHash(s) ) {
        //CRLog::trace("ldomDocument::setRenderProps() - style is changed");
        _def_style = s;
        changed = true;
    }
    if ( calcHash(_def_font) != calcHash(def_font)) {
        //CRLog::trace("ldomDocument::setRenderProps() - font is changed");
        _def_font = def_font;
        changed = true;
    }
    if ( _page_height != height ) {
        //CRLog::trace("ldomDocument::setRenderProps() - page height is changed");
        _page_height = height;
        changed = true;
    }
    if ( _page_width != width ) {
        //CRLog::trace("ldomDocument::setRenderProps() - page width is changed");
        _page_width = width;
        changed = true;
    }
    //{
    //    lUInt32 styleHash = calcStyleHash();
    //    styleHash = styleHash * 31 + calcGlobalSettingsHash();
    //    CRLog::trace("Style hash before set root style: %x", styleHash);
    //}
    //getRootNode()->setFont( _def_font );
    //getRootNode()->setStyle( _def_style );
    //{
    //    lUInt32 styleHash = calcStyleHash();
    //    styleHash = styleHash * 31 + calcGlobalSettingsHash();
    //    CRLog::trace("Style hash after set root style: %x", styleHash);
    //}
    return changed;
}

void CrDomBase::dropStyles()
{
    _styles.clear(-1);
    _fonts.clear(-1);
    resetNodeNumberingProps();

    int count = ((_elemCount+TNC_PART_LEN-1) >> TNC_PART_SHIFT);
    for ( int i=0; i<count; i++ ) {
        int offs = i*TNC_PART_LEN;
        int sz = TNC_PART_LEN;
        if ( offs + sz > _elemCount+1 ) {
            sz = _elemCount+1 - offs;
        }
        ldomNode * buf = _elemList[i];
        for ( int j=0; j<sz; j++ ) {
            if ( buf[j].isElement() ) {
                setNodeStyleIndex( buf[j]._handle._dataIndex, 0 );
                setNodeFontIndex( buf[j]._handle._dataIndex, 0 );
            }
        }
    }
}

int CrDomBase::calcFinalBlocks()
{
    int cnt = 0;
    int count = ((_elemCount+TNC_PART_LEN-1) >> TNC_PART_SHIFT);
    for ( int i=0; i<count; i++ ) {
        int offs = i*TNC_PART_LEN;
        int sz = TNC_PART_LEN;
        if ( offs + sz > _elemCount+1 ) {
            sz = _elemCount+1 - offs;
        }
        ldomNode * buf = _elemList[i];
        for ( int j=0; j<sz; j++ ) {
            if ( buf[j].isElement() ) {
                int rm = buf[j].getRendMethod();
                if ( rm==erm_final )
                    cnt++;
            }
        }
    }
    return cnt;
}

void CrDom::applyDocStylesheet()
{
    /*
     * Stylesheet stuff tips:
     *
     * inline LVStyleSheet* getStylesheet() { return &_stylesheet; }
     * LVLoadStylesheetFile
     * LVStyleSheet::parse
     * void CrDomBase::dropStyles()
     * bool CrDomBase::updateLoadedStyles( bool enabled )
     * DISABLE_STYLESHEET_REL
    */
    if (!getDocFlag(DOC_FLAG_EMBEDDED_STYLES)) {
        return;
    }
    if (!stylesheet_file_name_.empty()) {
        if (getDocParentContainer().isNull()) {
            return;
        }
        if (parseStyleSheet(stylesheet_file_name_)) {
        	CRLog::trace("applyDocStylesheet() using stylesheet from link/stylesheet: %s",
        			LCSTR(stylesheet_file_name_));
        }
        return;
    }
    ldomXPointer ss = createXPointer(cs16("/FictionBook/stylesheet"));
    if (ss.isNull()) {
        CRLog::trace("applyDocStylesheet() /FictionBook/stylesheet not found");
        return;
    }
    lString16 css = ss.getText('\n');
    if (css.empty()) {
        CRLog::trace("applyDocStylesheet() /FictionBook/stylesheet is empty");
        return;
    }
    if(css.pos("{")==-1)
    {
        CRLog::trace("applyDocStylesheet() /FictionBook/stylesheet is invalid");
        return;
    }
    CRLog::trace("applyDocStylesheet() using /FictionBook/stylesheet:\n%s", LCSTR(css));
    stylesheet_.parse(LCSTR(css));
}

bool CrDom::parseStyleSheet(lString16 codeBase, lString16 css)
{
    LVImportStylesheetParser parser(this);
    return parser.Parse(codeBase, css);
}

bool CrDom::parseStyleSheet(lString16 cssFile)
{
    LVImportStylesheetParser parser(this);
    return parser.Parse(cssFile);
}

/// save document formatting parameters after render
void CrDom::updateRenderContext()
{
    int dx = _page_width;
    int dy = _page_height;
    lUInt32 styleHash = calcStyleHash();
    lUInt32 stylesheetHash = (((stylesheet_.getHash() * 31) + calcHash(_def_style))
                              * 31 + calcHash(_def_font));
    //calcStyleHash( getRootNode(), styleHash );
    _hdr.render_style_hash = styleHash;
    _hdr.stylesheet_hash = stylesheetHash;
    _hdr.render_dx = dx;
    _hdr.render_dy = dy;
    _hdr.render_docflags = _docFlags;
    /*
    CRLog::trace(
            "updateRenderContext: styleHash: %x, stylesheetHash: %x docflags: %x, w: %x, h: %x",
            _hdr.render_style_hash,
            _hdr.stylesheet_hash,
            _hdr.render_docflags,
            _hdr.render_dx,
            _hdr.render_dy);
    */
}

/// check document formatting parameters before render - whether we need to reformat;
/// returns false if render is necessary
bool CrDom::checkRenderContext()
{
    bool res = true;
    ldomNode* node = getRootNode();
    if (node != NULL && node->getFont().isNull()) {
        CRLog::trace("checkRenderContext: Style is not set for root node");
        res = false;
    }
    lUInt32 styleHash = calcStyleHash();
    lUInt32 stylesheetHash = (((stylesheet_.getHash() * 31) + calcHash(_def_style)) * 31
                              + calcHash(_def_font));
    //calcStyleHash( getRootNode(), styleHash );
    if ( styleHash != _hdr.render_style_hash ) {
        CRLog::trace("checkRenderContext: Style hash doesn't match %x!=%x",
                styleHash, _hdr.render_style_hash);
        res = false;
    } else if (stylesheetHash != _hdr.stylesheet_hash) {
        CRLog::trace("checkRenderContext: Stylesheet hash doesn't match %x!=%x",
                stylesheetHash, _hdr.stylesheet_hash);
        res = false;
    } else if (_docFlags != _hdr.render_docflags) {
        CRLog::trace("checkRenderContext: Doc flags don't match %x!=%x",
                _docFlags, _hdr.render_docflags);
        res = false;
    } else if (_page_width != (int)_hdr.render_dx) {
        CRLog::trace("checkRenderContext: Width doesn't match %x!=%x",
                _page_width, (int)_hdr.render_dx);
        res = false;
    } else if (_page_height != (int)_hdr.render_dy) {
        CRLog::trace("checkRenderContext: Page height doesn't match %x!=%x",
                _page_height, (int)_hdr.render_dy);
        res = false;
    }
    return res;
}

int CrDom::render(LVRendPageList* pages,
        int width,
        int dy,
		bool showCover,
		int y0,
		font_ref_t def_font,
		int interline_space)
{
    CRLog::info("CrDom::render w=%d, h=%d, fontFace=%s, docFlags=%d",
    		width, dy, def_font->getTypeFace().c_str(), getDocFlags());
    setRenderProps(width, dy, def_font, interline_space);
    // update styles
    //if (getRootNode()->getStyle().isNull() || getRootNode()->getFont().isNull()
    //        || _docFlags != _hdr.render_docflags
    //        || width!=_hdr.render_dx
    //        || dy!=_hdr.render_dy || defStyleHash!=_hdr.stylesheet_hash ) {
    //    CRLog::trace("init format data...");
    //    getRootNode()->recurseElements( initFormatData );
    //} else {
    //    CRLog::trace("reusing existing format data...");
    //}

    this->ApplyEmbeddedStyles();

    if (!checkRenderContext() || force_render) {
        //CRLog::info("CrDom::checkRenderContext FORMATTING");
        dropStyles();
        //CRLog::trace("stylesheet_.push()");

        stylesheet_.push();
        applyDocStylesheet();
        //CRLog::info("initNodeStyleRecursive()");
        getRootNode()->initNodeStyleRecursive();
        //CRLog::trace("stylesheet_.pop()");
        stylesheet_.pop();
        //CRLog::trace("Init render method");
        getRootNode()->initNodeRendMethodRecursive();
        //getRootNode()->setFont(_def_font);
        //getRootNode()->setStyle(_def_style);
        updateRenderContext();
        //lUInt32 styleHash = calcStyleHash();
        //styleHash = styleHash * 31 + calcGlobalSettingsHash();
        _rendered = false;
        force_render = false;
    }
    if (!_rendered) {
        pages->clear();
        if (showCover) {
        	pages->add(new LVRendPageInfo(_page_height));
        }
        LVRendPageContext context(pages, _page_height);
        int numFinalBlocks = calcFinalBlocks();
        CRLog::trace("Final block count: %d", numFinalBlocks);
        //updateStyles();
        int height = renderBlockElement( context, getRootNode(), 0, y0, width ) + y0;
        _rendered = true;
        gc();
        //CRLog::trace("finalizing... fonts.length=%d", _fonts.length());
        context.Finalize();
        updateRenderContext();
        _pagesData.reset();
        pages->serialize( _pagesData );
        return height;
    } else {
        CRLog::trace("rendering context is not changed, no render");
        if (_pagesData.pos()) {
            _pagesData.setPos(0);
            pages->deserialize( _pagesData );
        }
        CRLog::trace("%d rendered pages found", pages->length());
        return getFullHeight();
    }
}

void CrDomXml::setNodeTypes( const elem_def_t * node_scheme )
{
    if (!node_scheme)
        return;
    for (; node_scheme && node_scheme->id != 0; ++node_scheme)
    {
        _elementNameTable.AddItem(
        		node_scheme->id,
        		lString16(node_scheme->name),
        		&node_scheme->props
        );
    }
}

// set attribute types from table
void CrDomXml::setAttributeTypes( const attr_def_t * attr_scheme )
{
    if ( !attr_scheme )
        return;
    for ( ; attr_scheme && attr_scheme->id != 0; ++attr_scheme )
    {
        _attrNameTable.AddItem(
            attr_scheme->id,               // ID
            lString16(attr_scheme->name),  // Name
            NULL);
    }
    _idAttrId = _attrNameTable.IntByStr("id");
}

// set namespace types from table
void CrDomXml::setNameSpaceTypes( const ns_def_t * ns_scheme )
{
    if ( !ns_scheme )
        return;
    for ( ; ns_scheme && ns_scheme->id != 0; ++ns_scheme )
    {
        _nsNameTable.AddItem(
            ns_scheme->id,                 // ID
            lString16(ns_scheme->name),    // Name
            NULL);
    }
}

void CrDomXml::dumpUnknownEntities( const char * fname )
{
    FILE * f = fopen( fname, "wt" );
    if ( !f )
        return;
    fprintf(f, "Unknown elements:\n");
    _elementNameTable.dumpUnknownItems(f, UNKNOWN_ELEMENT_TYPE_ID);
    fprintf(f, "-------------------------------\n");
    fprintf(f, "Unknown attributes:\n");
    _attrNameTable.dumpUnknownItems(f, UNKNOWN_ATTRIBUTE_TYPE_ID);
    fprintf(f, "-------------------------------\n");
    fprintf(f, "Unknown namespaces:\n");
    _nsNameTable.dumpUnknownItems(f, UNKNOWN_NAMESPACE_TYPE_ID);
    fprintf(f, "-------------------------------\n");
    fclose(f);
}

static bool IsEmptySpace(const lChar16* text, int len)
{
   for (int i=0; i<len; i++)
      if ( text[i]!=' ' && text[i]!='\r' && text[i]!='\n' && text[i]!='\t')
         return false;
   return true;
}

static bool IS_FIRST_BODY = false;

ldomElementWriter::ldomElementWriter(CrDom* document,
        lUInt16 nsid,
        lUInt16 id,
        ldomElementWriter* parent,
        lUInt32 flags)
        : _parent(parent),
          _document(document),
          _tocItem(NULL),
          _isBlock(true),
          _isSection(false),
          stylesheet_IsSet(false),
          _bodyEnterCalled(false)
{
    //logfile << "{c";
    _typeDef = _document->getElementTypePtr( id );
    _flags = flags;
    if ( (_typeDef && _typeDef->white_space==css_ws_pre) || (_parent && _parent->getFlags()&TXTFLG_PRE) )
        _flags |= TXTFLG_PRE;
    _isSection = (id==el_section);
    _allowText = _typeDef ? _typeDef->allow_text : (_parent?true:false);
    if (_parent)
        _element = _parent->getElement()->insertChildElement( (lUInt32)-1, nsid, id );
    else
        _element = _document->getRootNode(); //->insertChildElement( (lUInt32)-1, nsid, id );
    if (IS_FIRST_BODY && id==el_body) {
        _tocItem = _document->getToc();
        //_tocItem->clear();
        IS_FIRST_BODY = false;
    }
    //logfile << "}";
}

lUInt32 ldomElementWriter::getFlags()
{
    return _flags;
}

static bool isBlockNode( ldomNode * node )
{
    if ( !node->isElement() )
        return false;
    switch ( node->getStyle()->display )
    {
    case css_d_block:
    case css_d_list_item:
    case css_d_table:
    case css_d_table_row:
    case css_d_inline_table:
    case css_d_table_row_group:
    case css_d_table_header_group:
    case css_d_table_footer_group:
    case css_d_table_column_group:
    case css_d_table_column:
    case css_d_table_cell:
    case css_d_table_caption:
        return true;

    case css_d_inherit:
    case css_d_inline:
    case css_d_run_in:
    case css_d_compact:
    case css_d_marker:
    case css_d_none:
        break;
    }
    return false;
}

static bool isInlineNode( ldomNode * node )
{
    if ( node->isText() )
        return true;
    //int d = node->getStyle()->display;
    //return ( d==css_d_inline || d==css_d_run_in );
    int m = node->getRendMethod();
    return m==erm_inline || m==erm_runin;
}

static std::vector<ldomNode*> getSectionHeaderNodes( ldomNode * section )
{
    std::vector<ldomNode*> res;
    if (!section || section->getChildCount() == 0)
    {
        return res;
    }
    for (int i = 0; i < section->getChildCount(); i++)
    {
        ldomNode * child = section->getChildNode(i);
        if ( !child || child->isText())
        {
            continue;
        }
        if(child->getNodeName() == "title")
        {
            res.emplace_back(child);
        }
    }
    return res;
}

/*
 * deprecated
static lString16 getSectionHeader( ldomNode * section )
{
    lString16 header = lString16("-");
    if ( !section || section->getChildCount() == 0 )
        return header;
    ldomNode * child = section->getChildElementNode(0, L"title");
    if ( !child )
        return header;
    header = child->getText(L' ', 1024);
    return header;
}
*/

lString16 ldomElementWriter::getPath()
{
    if (!_path.empty() || _element->isRoot()) {
        return _path;
    }
    _path = _parent->getPath() + "/" + _element->getXPathSegment();
    return _path;
}

void ldomElementWriter::updateTocItem()
{
    if ( !_isSection )
        return;
    // TODO: update item
    if ( _parent && _parent->_tocItem ) {
        ldomNode *body = _element->getParentNode("body");
        const lString16 &bodyName = body->getAttributeValue(attr_name);
        if(!bodyName.empty() && (bodyName == lString16("notes_hidden") || bodyName == lString16("notes")))
        {
            _isSection = false;
            return;
        }
        std::vector<ldomNode *> titlenodes = getSectionHeaderNodes(_element);

        for (auto titlenode : titlenodes)
        {
            lString16 title = titlenode->getText(L' ', 1024);

            //CRLog::trace("TOC ITEM: %s", LCSTR(title));
            _tocItem = _parent->_tocItem->addChild(title, ldomXPointer(titlenode, 0), getPath());
        }
    }
    _isSection = false;
}

void ldomElementWriter::onBodyEnter()
{
    _bodyEnterCalled = true;
    if (_document->isDefStyleSet()) {
#if 0
        CRLog::trace("onBodyEnter() for node %04x %s",
                _element->getDataIndex(),
                LCSTR(_element->getNodeName()));
#endif
        _element->initNodeStyle();
#ifdef OREDEBUG
        if (_element->getStyle().isNull()) {
            CRLog::error("element style init error %x %s",
                    _element->getNodeIndex(),
                    LCSTR(_element->getNodeName()));
        }
#endif
        _isBlock = isBlockNode(_element);
    }
    if (_isSection) {
        if (_parent && _parent->_isSection) {
            _parent->updateTocItem();
        }
    }
}

void ldomNode::removeChildren( int startIndex, int endIndex )
{
    for ( int i=endIndex; i>=startIndex; i-- ) {
        removeChild(i)->destroy();
    }
}

void ldomNode::autoboxChildren( int startIndex, int endIndex )
{
    if ( !isElement() )
        return;
    css_style_ref_t style = getStyle();
    bool pre = ( style->white_space==css_ws_pre );
    int firstNonEmpty = startIndex;
    int lastNonEmpty = endIndex;

    bool hasInline = pre;
    if ( !pre ) {
        while ( firstNonEmpty<=endIndex && getChildNode(firstNonEmpty)->isText() ) {
            lString16 s = getChildNode(firstNonEmpty)->getText();
            if ( !IsEmptySpace(s.c_str(), s.length() ) )
                break;
            firstNonEmpty++;
        }
        while ( lastNonEmpty>=endIndex && getChildNode(lastNonEmpty)->isText() ) {
            lString16 s = getChildNode(lastNonEmpty)->getText();
            if ( !IsEmptySpace(s.c_str(), s.length() ) )
                break;
            lastNonEmpty--;
        }

        for ( int i=firstNonEmpty; i<=lastNonEmpty; i++ ) {
            ldomNode * node = getChildNode(i);
            if ( isInlineNode( node ) )
                hasInline = true;
        }
    }
    if (hasInline) { //&& firstNonEmpty<=lastNonEmpty
#ifdef TRACE_AUTOBOX
        CRLog::trace("Autobox children %d..%d of node <%s>  childCount=%d", firstNonEmpty,
                     lastNonEmpty, LCSTR(getNodeName()), getChildCount());
        for (int i = firstNonEmpty; i <= lastNonEmpty; i++) {
            ldomNode* node = getChildNode(i);
            if (node->isText()) {
                CRLog::trace("    text: %d '%s'", node->getDataIndex(), LCSTR(node->getText()));
            } else {
                CRLog::trace("    elem: %d <%s> rendMode=%d  display=%d", node->getDataIndex(),
                             LCSTR(node->getNodeName()), node->getRendMethod(),
                             node->getStyle()->display);
            }
        }
#endif
        // remove starting empty
        removeChildren(lastNonEmpty+1, endIndex);
        // inner inline
        ldomNode * abox = insertChildElement( firstNonEmpty, LXML_NS_NONE, el_autoBoxing );
        abox->initNodeStyle();
        abox->setRendMethod( erm_final );
        moveItemsTo( abox, firstNonEmpty+1, lastNonEmpty+1 );
        // remove trailing empty
        removeChildren(startIndex, firstNonEmpty-1);
    } else {
        // only empty items: remove them instead of autoboxing
        removeChildren(startIndex, endIndex);
    }
}

static void resetRendMethodToInline( ldomNode * node )
{
    if (node->getStyle().get()->display == css_d_none)
    {
        return;
    }
    node->setRendMethod(erm_inline);
}

static void resetRendMethodToInvisible( ldomNode * node )
{
    node->setRendMethod(erm_invisible);
}

static void detectChildTypes( ldomNode * parent, bool & hasBlockItems, bool & hasInline )
{
    hasBlockItems = false;
    hasInline = false;
    int len = parent->getChildCount();
    for ( int i=len-1; i>=0; i-- ) {
        ldomNode * node = parent->getChildNode(i);
        if ( !node->isElement() ) {
            // text
            hasInline = true;
        } else {
            // element
            int d = node->getStyle()->display;
            int m = node->getRendMethod();
            if ( d==css_d_none || m==erm_invisible )
                continue;
            if ( m==erm_inline || m==erm_runin) { //d==css_d_inline || d==css_d_run_in
                hasInline = true;
            } else {
                hasBlockItems = true;
            }
        }
    }
}

// init table element render methods
// states: 0=table, 1=colgroup, 2=rowgroup, 3=row, 4=cell
// returns table cell count
int initTableRendMethods( ldomNode * enode, int state )
{
    //main node: table
    if ( state==0 && enode->getStyle()->display==css_d_table )
        enode->setRendMethod( erm_table ); // for table
    int cellCount = 0;
    int cnt = enode->getChildCount();
    int i;
    for (i=0; i<cnt; i++)
    {
        ldomNode * child = enode->getChildNode( i );
        if ( child->isElement() )
        {
            switch( child->getStyle()->display )
            {
            case css_d_table_caption:
                if ( state==0 ) {
                    child->setRendMethod( erm_table_caption );
                } else {
                    child->setRendMethod( erm_invisible );
                }
                break;
            case css_d_inline:
                {
                }
                break;
            case css_d_table_row_group:
                if ( state==0 ) {
                    child->setRendMethod( erm_table_row_group );
                    cellCount += initTableRendMethods( child, 2 );
                } else {
                    child->setRendMethod( erm_invisible );
                }
                break;
            case css_d_table_header_group:
                if ( state==0 ) {
                    child->setRendMethod( erm_table_header_group );
                    cellCount += initTableRendMethods( child, 2);
                } else {
                    child->setRendMethod( erm_invisible );
                }
                break;
            case css_d_table_footer_group:
                if ( state==0 ) {
                    child->setRendMethod( erm_table_footer_group );
                    cellCount += initTableRendMethods( child, 2 );
                } else {
                    child->setRendMethod( erm_invisible );
                }
                break;
            case css_d_table_row:
                if ( state==0 || state==2 ) {
                    child->setRendMethod( erm_table_row );
                    cellCount += initTableRendMethods( child, 3 );
                } else {
                    child->setRendMethod( erm_invisible );
                }
                break;
            case css_d_table_column_group:
                if ( state==0 ) {
                    child->setRendMethod( erm_table_column_group );
                    cellCount += initTableRendMethods( child, 1 );
                } else {
                    child->setRendMethod( erm_invisible );
                }
                break;
            case css_d_table_column:
                if ( state==0 || state==1 ) {
                    child->setRendMethod( erm_table_column );
                } else {
                    child->setRendMethod( erm_invisible );
                }
                break;
            case css_d_table_cell:
                if ( state==3 ) {
                    child->setRendMethod( erm_table_cell );
                    cellCount++;
                    // will be translated to block or final below
                    //child->initNodeRendMethod();
                    child->initNodeRendMethodRecursive();
                    //child->setRendMethod( erm_table_cell );
                    //initRendMethod( child, true, true );
                } else {
                    child->setRendMethod( erm_invisible );
                }
                break;
            default:
                // ignore
                break;
            }
        }
    }
//    if ( state==0 ) {
//        dumpRendMethods( enode, cs16("   ") );
//    }
    return cellCount;
}

bool hasInvisibleParent( ldomNode * node )
{
    for ( ; !node->isRoot(); node = node->getParentNode() )
        if ( node->getStyle()->display==css_d_none )
            return true;
    return false;
}

bool checkHasBlockChilrden(ldomNode * node)
{
    //this method checks whether element node has element node children that are NOT el_span node
    if(!node->isElement())
    {
        return false;
    }

    for (int i = 0; i < node->getChildCount(); i++)
    {
        ldomNode * child = node->getChildNode(i);
        if(child->isText())
        {
            continue;
        }
        int rm = child->getRendMethod();
        if(rm == erm_inline || rm == erm_runin)
        {
            continue;
        }
        else
        {
            //LE("erm = %d for node [%s]",rm, LCSTR(node->getXPath()));
            return true;
        }
    }
    return false;
}

void ldomNode::initNodeRendMethod()
{
    if (!isElement())
        return;
    if ( isRoot() ) {
        setRendMethod(erm_block);
        return;
    }

    // DEBUG TEST
    // if ( getParentNode()->getChildIndex( getDataIndex() )<0 ) {
    //     CRLog::error("Invalid parent->child relation for nodes %d->%d",
    //     getParentNode()->getDataIndex(), getDataIndex() ); }
    //if ( getNodeName() == "image" ) { CRLog::trace("Init log for image");}

    int d = getStyle()->display;

    switch (d)
    {
        case css_d_none:
            setRendMethod(erm_invisible);
            recurseElements(resetRendMethodToInvisible);
            //} else if (hasInvisibleParent(this)) {
            //    // invisible
            //    //recurseElements( resetRendMethodToInvisible );
            //    setRendMethod(erm_invisible); }
            break;
        case css_d_inline:
            if (this->getNodeId() == el_span && checkHasBlockChilrden(this))
            {
                //LV("css_d_inline initNodeRendMethodDefault for [%s]",LCSTR(getXPath()));
                initNodeRendMethodDefault();
            }
            else
            {
                //LV("switch all children elements of [%s] to inline", LCSTR(getXPath()));
                recurseElements(resetRendMethodToInline);
            }
            break;
        case css_d_run_in:
            //CRLog::trace("switch all children elements of <%s> to inline", LCSTR(getNodeName()));
            recurseElements( resetRendMethodToInline );
            setRendMethod(erm_runin);
            break;
        case css_d_list_item:
            setRendMethod(erm_list_item);
            break;
        case css_d_table:
            initTableRendMethods( this, 0 );
            break;
        default:
        {
            // block or final
            initNodeRendMethodDefault();
        }
    }
}

void ldomNode::initNodeRendMethodDefault()
{
    // remove last empty space text nodes
    bool hasBlockItems = false;
    bool hasInline = false;
    detectChildTypes(this, hasBlockItems, hasInline);
    const css_elem_def_props_t *ntype = getElementTypePtr();
    if (ntype && ntype->is_object){
        int d = getStyle()->display;
        switch (d)
        {
            case css_d_block:
            case css_d_inline:
            case css_d_run_in:
                setRendMethod(erm_final); break;
            default:
                //setRendMethod( erm_invisible );
                recurseElements(resetRendMethodToInvisible);
                break;
        }
    }
    else if (hasBlockItems && !hasInline){setRendMethod(erm_block) ; } // only blocks inside
    else if (!hasBlockItems && hasInline){setRendMethod(erm_final) ; }
    else if (!hasBlockItems && !hasInline){setRendMethod(erm_block);  /* setRendMethod( erm_invisible ); */}
    else if (hasBlockItems && hasInline)
    {
        if (getParentNode()->getNodeId() == el_autoBoxing){ setRendMethod(erm_final);} // already autoboxed
        else
        {
            // cleanup or autobox
            int i = getChildCount() - 1;
            for (; i >= 0; i--)
            {
                ldomNode *node = getChildNode(i);

                // DEBUG TEST
                // if ( getParentNode()->getChildIndex( getDataIndex() )<0 ) {
                //     CRLog::error("Invalid parent->child relation for nodes %d->%d",
                //           getParentNode()->getDataIndex(), getDataIndex() );
                // }

                if (isInlineNode(node))
                {
                    int j = i - 1;
                    for (; j >= 0; j--)
                    {
                        node = getChildNode(j);
                        if (!isInlineNode(node))
                        {
                            break;
                        }
                    }
                    j++;
                    // j..i are inline
                    if (j > 0 || i < (int) getChildCount() - 1)
                    {
                        autoboxChildren(j, i);
                    }
                    i = j;
                }
                else if (i > 0)
                {
                    ldomNode *prev = getChildNode(i - 1);
                    if (prev->isElement() && prev->getRendMethod() == erm_runin)
                    {
                        // autobox run-in
                        if (getChildCount() != 2)
                        {
                            //CRLog::trace("Autoboxing run-in items");
                            autoboxChildren(i - 1, i);
                        }
                        i--;
                    }
                }
            }
            // check types after autobox
            detectChildTypes(this, hasBlockItems, hasInline);
            if (hasInline){ setRendMethod(erm_final); } // Final
            else{ setRendMethod(erm_block); } // Block
        }
    }
}

void ldomElementWriter::onBodyExit()
{
    if ( _isSection )
        updateTocItem();

    if ( !_document->isDefStyleSet() )
        return;
    if ( !_bodyEnterCalled ) {
        onBodyEnter();
    }
//    if ( _element->getStyle().isNull() ) {
//        lString16 path;
//        ldomNode * p = _element->getParentNode();
//        while (p) {
//            path = p->getNodeName() + L"/" + path;
//            p = p->getParentNode();
//        }
//        CRLog::error("style not initialized for element 0x%04x %s path %s",
//             _element->getDataIndex(), LCSTR(_element->getNodeName()), LCSTR(path));
//        crFatalError();
//    }
    _element->initNodeRendMethod();

    if ( stylesheet_IsSet )
        _document->getStylesheet()->pop();
}

void ldomElementWriter::onText( const lChar16 * text, int len, lUInt32 )
{
    //logfile << "{t";
    {
        // normal mode: store text copy
        // add text node, if not first empty space string of block node
        if ( !_isBlock || _element->getChildCount()!=0 || !IsEmptySpace( text, len ) || (_flags&TXTFLG_PRE) || (_flags&TXTFLG_KEEP_SPACES) ) {
            lString8 s8 = UnicodeToUtf8(text, len);
            _element->insertChildText(s8);
        } else {
            //CRLog::trace("ldomElementWriter::onText: Ignoring first empty space of block item");
        }
    }
    //logfile << "}";
}

//#define DISABLE_STYLESHEET_REL
/// if stylesheet file name is set, and file is found, set stylesheet to its value
bool ldomNode::applyNodeStylesheet()
{
#ifndef DISABLE_STYLESHEET_REL
	//CRLog::trace("ldomNode::applyNodeStylesheet()");
	if (!getCrDom()->getDocFlag(DOC_FLAG_EMBEDDED_STYLES) || getNodeId() != el_DocFragment) {
		return false;
	}
	//CRLog::trace("applyNodeStylesheet DOC_FLAG_EMBEDDED_STYLES=true");
    if (getCrDom()->getDocParentContainer().isNull()) {
        return false;
    }
    bool stylesheetChanged = false;
    if ( hasAttribute(attr_StyleSheet) ) {
        getCrDom()->stylesheet_.push();
        stylesheetChanged = getCrDom()->parseStyleSheet(getAttributeValue(attr_StyleSheet));
        if ( !stylesheetChanged )
            getCrDom()->stylesheet_.pop();
    }
    if ( getChildCount() > 0 ) {
        ldomNode *styleNode = getChildNode(0);

        if ( styleNode && styleNode->getNodeId()==el_stylesheet ) {
            if ( false == stylesheetChanged) {
                getCrDom()->stylesheet_.push();
            }
            if (getCrDom()->parseStyleSheet(styleNode->getAttributeValue(attr_href),
                                                styleNode->getText()) ) {
                stylesheetChanged = true;
            } else if (false == stylesheetChanged) {
                getCrDom()->stylesheet_.pop();
            }
        }
    }
    return stylesheetChanged;
#endif
    return false;
}

void ldomElementWriter::addAttribute(lUInt16 nsid, lUInt16 id, const wchar_t* value)
{
    getElement()->setAttributeValue(nsid, id, value);
    if (id == attr_StyleSheet) {
       stylesheet_IsSet = _element->applyNodeStylesheet();
    }
}

ldomElementWriter * LvDomWriter::pop( ldomElementWriter * obj, lUInt16 id )
{
    //logfile << "{p";
    ldomElementWriter * tmp = obj;
    for ( ; tmp; tmp = tmp->_parent )
    {
        //logfile << "-";
        if (tmp->getElement()->getNodeId() == id)
            break;
    }
    //logfile << "1";
    if (!tmp)
    {
        //logfile << "-err}";
        return obj; // error!!!
    }
    ldomElementWriter * tmp2 = NULL;
    //logfile << "2";
    for ( tmp = obj; tmp; tmp = tmp2 )
    {
        //logfile << "-";
        tmp2 = tmp->_parent;
        bool stop = (tmp->getElement()->getNodeId() == id);
        ElementCloseHandler( tmp->getElement() );
        delete tmp;
        if ( stop )
            return tmp2;
    }
    /*
    logfile << "3 * ";
    logfile << (int)tmp << " - " << (int)tmp2 << " | cnt=";
    logfile << (int)tmp->getElement()->childCount << " - "
            << (int)tmp2->getElement()->childCount;
    */
    //logfile << "}";
    return tmp2;
}

ldomElementWriter::~ldomElementWriter()
{
    //CRLog::trace("~ldomElementWriter for element 0x%04x %s",
    //     _element->getDataIndex(), LCSTR(_element->getNodeName()));
    //getElement()->persist();
    onBodyExit();
}

void LvDomWriter::OnStart(LVFileFormatParser * parser)
{
    //logfile << "LvDomWriter::OnStart()\n";
    // add document root node
    //CRLog::trace("LvDomWriter::OnStart()");
    if ( !_headerOnly )
        _stopTagId = 0xFFFE;
    else {
        _stopTagId = doc_->getElementNameIndex(L"description");
        //CRLog::trace( "LvDomWriter() : header only, tag id=%d", _stopTagId );
    }
    LvXMLParserCallback::OnStart( parser );
    _currNode = new ldomElementWriter(doc_, 0, 0, NULL,_flags);
}

void LvDomWriter::OnStop()
{
    //logfile << "LvDomWriter::OnStop()\n";
    while (_currNode)
        _currNode = pop( _currNode, _currNode->getElement()->getNodeId() );
}

/// called after > of opening tag (when entering tag body)
void LvDomWriter::OnTagBody()
{
    // init element style
    if ( _currNode ) {
        _currNode->onBodyEnter();
    }
}

ldomNode* LvDomWriter::OnTagOpen(const lChar16* nsname, const lChar16* tagname)
{
    //CRLog::trace("OnTagOpen(%s)", UnicodeToUtf8(lString16(tagname)).c_str());
    lUInt16 id = doc_->getElementNameIndex(tagname);
    lUInt16 nsid = (nsname && nsname[0]) ? doc_->getNsNameIndex(nsname) : 0;

    //if ( id==_stopTagId ) {
        //CRLog::trace("stop tag found, stopping...");
    //    _parser->Stop();
    //}
    _currNode = new ldomElementWriter(doc_, nsid, id, _currNode,_flags);
    _flags = _currNode->getFlags();
    //logfile << " !o!\n";
    //return _currNode->getElement();
    return _currNode->getElement();
}

void LvDomWriter::OnTagClose(const lChar16*, const lChar16* tagname) {
    //logfile << "LvDomWriter::OnTagClose() [" << nsname << ":" << tagname << "]";
    if (!_currNode)
    {
        _errFlag = true;
        //logfile << " !c-err!\n";
        return;
    }
    if (tagname[0] == 'l' && _currNode && !lStr_cmp(tagname, "link")) {
        // link node
        if (_currNode && _currNode->getElement() &&
            _currNode->getElement()->isNodeName("link") &&
            _currNode->getElement()->getParentNode() &&
            _currNode->getElement()->getParentNode()->isNodeName("head") &&
            _currNode->getElement()->getAttributeValue("rel") == "stylesheet" &&
            _currNode->getElement()->getAttributeValue("type") == "text/css"
            )
        {
            lString16 href = _currNode->getElement()->getAttributeValue("href");
            lString16 stylesheetFile = LVCombinePaths(doc_->getCodeBase(), href);
            //CRLog::trace("Internal stylesheet file: %s", LCSTR(stylesheetFile));
            if(doc_->stylesheet_file_name_ != stylesheetFile)
            {
                doc_->stylesheet_file_name_ = stylesheetFile;
                doc_->applyDocStylesheet();
            }
        }
    }

    bool isStyleSheetTag = !lStr_cmp(tagname, "stylesheet");
    if (isStyleSheetTag) {
        ldomNode* parentNode = _currNode->getElement()->getParentNode();
        if (parentNode && parentNode->isNodeName("DocFragment")) {
            if (doc_->getDocFlag(DOC_FLAG_EMBEDDED_STYLES)) {
                doc_->parseStyleSheet(
                        _currNode->getElement()->getAttributeValue(attr_href),
                        _currNode->getElement()->getText());
            }
            isStyleSheetTag = false;
        }
    }

    lUInt16 id = doc_->getElementNameIndex(tagname);
    //lUInt16 nsid = (nsname && nsname[0]) ? _document->getNsNameIndex(nsname) : 0;
    _errFlag |= (id != _currNode->getElement()->getNodeId());
    _currNode = pop( _currNode, id );

    if ( _currNode )
        _flags = _currNode->getFlags();

    if ( id==_stopTagId ) {
        //CRLog::trace("stop tag found, stopping...");
        _parser->Stop();
    }

    if ( isStyleSheetTag ) {
        //CRLog::trace("</stylesheet> found");
        if ( !_popStyleOnFinish ) {
            //CRLog::trace("saving current stylesheet before applying of document stylesheet");
            doc_->getStylesheet()->push();
            _popStyleOnFinish = true;
            doc_->applyDocStylesheet();
        }
    }
}

void
LvDomWriter::OnAttribute(const lChar16* nsname, const lChar16* attrname, const lChar16* attr_val)
{
    lUInt16 attr_ns = (nsname && nsname[0]) ? doc_->getNsNameIndex(nsname) : 0;
    lUInt16 attr_id = (attrname && attrname[0]) ? doc_->getAttrNameIndex(attrname) : 0;
    _currNode->addAttribute(attr_ns, attr_id, attr_val);
}

void LvDomWriter::OnText(const lChar16 * text, int len, lUInt32 flags) {
    if (_currNode)
    {
        if ( (_flags & XML_FLAG_NO_SPACE_TEXT)
             && IsEmptySpace(text, len)  && !(flags & TXTFLG_PRE))
             return;
        if (_currNode->_allowText || _currNode->_element->isNodeName("annotation"))
        {
            if (RTL_DISPLAY_ENABLE && lString16(text,len).CheckRTL())
            {
                gDocumentRTL = 1;
                ldomElementWriter * node = _currNode;
                node->addAttribute(0, node->_document->getAttrNameIndex("dir"), L"rtl");
                while (!node->_isBlock)
                {
                    node = node->_parent;
                    node->addAttribute(0, node->_document->getAttrNameIndex("dir"), L"rtl");
                }
                this->setRTLflag(true);
            }

            if (this->RTLflag_ && RTL_DISPLAY_ENABLE && gDocumentFormat != DOC_FORMAT_DOCX)
            {
                _currNode->onText(lString16(text).PrepareRTL().c_str(), len, flags);
            }
            else
            {
                _currNode->onText(text, len, flags);
            }
        }
    }
}

void LvDomWriter::OnEncoding(const lChar16 *, const lChar16 *) {}

LvDomWriter::LvDomWriter(CrDom* document, bool headerOnly)
    	: doc_(document),
    	  _currNode(NULL),
    	  _errFlag(false),
    	  _headerOnly(headerOnly),
    	  _popStyleOnFinish(false),
    	  _flags(0) {
    _stopTagId = 0xFFFE;
    IS_FIRST_BODY = true;
    if (doc_->isDefStyleSet()) {
#ifdef OREDEBUG
        //CRLog::trace("LvDomWriter::LvDomWriter()");
#endif
        doc_->getRootNode()->initNodeStyle();
        doc_->getRootNode()->setRendMethod(erm_block);
    }
}

LvDomWriter::~LvDomWriter()
{
    while (_currNode) {
        _currNode = pop(_currNode, _currNode->getElement()->getNodeId());
    }
    if (doc_->isDefStyleSet()) {
        if (_popStyleOnFinish) {
            doc_->getStylesheet()->pop();
        }
#ifdef OREDEBUG
        //CRLog::trace("LvDomWriter::~LvDomWriter()");
#endif
        doc_->getRootNode()->initNodeStyle();
        doc_->getRootNode()->initNodeFont();
        doc_->updateRenderContext();
    }
}

bool FindNextNode(ldomNode*& node, ldomNode* root)
{
    if (node->getChildCount() > 0) {
        // first child
        node = node->getChildNode(0);
        return true;
    }
    if (node->isRoot() || node == root )
        return false; // root node reached
    int index = node->getNodeIndex();
    ldomNode * parent = node->getParentNode();
    while (parent)
    {
        if ( index < (int)parent->getChildCount()-1 ) {
            // next sibling
            node = parent->getChildNode( index + 1 );
            return true;
        }
        if (parent->isRoot() || parent == root )
            return false; // root node reached
        // up one level
        index = parent->getNodeIndex();
        parent = parent->getParentNode();
    }
    //if ( node->getNodeType() == LXML_TEXT_NODE )
    return false;
}

// base64 decode table
static const signed char base64_decode_table[] = {
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0..15
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //16..31   10
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63, //32..47   20
   52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1, //48..63   30
   -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14, //64..79   40
   15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1, //80..95   50
   -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40, //INDEX2..111  60
   41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1  //112..127 70
};

#define BASE64_BUF_SIZE 128
class LVBase64NodeStream : public LVNamedStream
{
private:
    ldomNode *  m_elem;
    ldomNode *  m_curr_node;
    lString16   m_curr_text;
    int         m_text_pos;
    lvsize_t    m_size;
    lvpos_t     m_pos;

    int         m_iteration;
    lUInt32     m_value;

    lUInt8      m_bytes[BASE64_BUF_SIZE];
    int         m_bytes_count;
    int         m_bytes_pos;

    int readNextBytes()
    {
        int bytesRead = 0;
        bool flgEof = false;
        while ( bytesRead == 0 && !flgEof )
        {
            while ( m_text_pos >= (int)m_curr_text.length() )
            {
                if ( !findNextTextNode() )
                    return bytesRead;
            }
            int len = m_curr_text.length();
            const lChar16 * txt = m_curr_text.c_str();
            for ( ; m_text_pos<len && m_bytes_count < BASE64_BUF_SIZE - 3; m_text_pos++ )
            {
                lChar16 ch = txt[ m_text_pos ];
                if ( ch < 128 )
                {
                    if ( ch == '=' )
                    {
                        // end of stream
                        if ( m_iteration == 2 )
                        {
                            m_bytes[m_bytes_count++] = (lUInt8)((m_value>>4) & 0xFF);
                            bytesRead++;
                        }
                        else if ( m_iteration == 3 )
                        {
                            m_bytes[m_bytes_count++] = (lUInt8)((m_value>>10) & 0xFF);
                            m_bytes[m_bytes_count++] = (lUInt8)((m_value>>2) & 0xFF);
                            bytesRead += 2;
                        }
                        // stop!!!
                        //m_text_pos--;
                        m_iteration = 0;
                        flgEof = true;
                        break;
                    }
                    else
                    {
                        int k = base64_decode_table[ch];
                        if ( !(k & 0x80) ) {
                            // next base-64 digit
                            m_value = (m_value << 6) | (k);
                            m_iteration++;
                            if (m_iteration==4)
                            {
                                //
                                m_bytes[m_bytes_count++] = (lUInt8)((m_value>>16) & 0xFF);
                                m_bytes[m_bytes_count++] = (lUInt8)((m_value>>8) & 0xFF);
                                m_bytes[m_bytes_count++] = (lUInt8)((m_value>>0) & 0xFF);
                                m_iteration = 0;
                                m_value = 0;
                                bytesRead+=3;
                            }
                        } else {
                            //m_text_pos++;
                        }
                    }
                }
            }
        }
        return bytesRead;
    }

    bool findNextTextNode()
    {
        while ( FindNextNode( m_curr_node, m_elem ) ) {
            if ( m_curr_node->isText() ) {
                m_curr_text = m_curr_node->getText();
                m_text_pos = 0;
                return true;
            }
        }
        return false;
    }

    int bytesAvailable() { return m_bytes_count - m_bytes_pos; }

    bool rewind()
    {
        m_curr_node = m_elem;
        m_pos = 0;
        m_bytes_count = 0;
        m_bytes_pos = 0;
        m_iteration = 0;
        m_value = 0;
        return findNextTextNode();
    }

    bool skip( lvsize_t count )
    {
        while ( count )
        {
            if ( m_bytes_pos >= m_bytes_count )
            {
                m_bytes_pos = 0;
                m_bytes_count = 0;
                int bytesRead = readNextBytes();
                if ( bytesRead == 0 )
                    return false;
            }
            int diff = (int) (m_bytes_count - m_bytes_pos);
            if (diff > (int)count)
                diff = (int)count;
            m_pos += diff;
            count -= diff;
        }
        return true;
    }

public:
    virtual ~LVBase64NodeStream() { }
    LVBase64NodeStream( ldomNode * element )
        : m_elem(element), m_curr_node(element), m_size(0), m_pos(0)
    {
        // calculate size
        rewind();
        m_size = bytesAvailable();
        for (;;) {
            int bytesRead = readNextBytes();
            if ( !bytesRead )
                break;
            m_bytes_count = 0;
            m_bytes_pos = 0;
            m_size += bytesRead;
        }
        // rewind
        rewind();
    }
    virtual bool Eof()
    {
        return m_pos >= m_size;
    }
    virtual lvsize_t  GetSize()
    {
        return m_size;
    }

    virtual lvpos_t GetPos()
    {
        return m_pos;
    }

    virtual lverror_t GetPos( lvpos_t * pos )
    {
        if (pos)
            *pos = m_pos;
        return LVERR_OK;
    }

    virtual lverror_t Seek(lvoffset_t offset, lvseek_origin_t origin, lvpos_t* newPos)
    {
        lvpos_t npos = 0;
        lvpos_t currpos = GetPos();
        switch (origin) {
        case LVSEEK_SET:
            npos = offset;
            break;
        case LVSEEK_CUR:
            npos = currpos + offset;
            break;
        case LVSEEK_END:
            npos = m_size + offset;
            break;
        }
        if (npos > m_size)
            return LVERR_FAIL;
        if ( npos != currpos )
        {
            if (npos < currpos)
            {
                if ( !rewind() || !skip(npos) )
                    return LVERR_FAIL;
            }
            else
            {
                skip( npos - currpos );
            }
        }
        if (newPos)
            *newPos = npos;
        return LVERR_OK;
    }
    virtual lverror_t Write(const void*, lvsize_t, lvsize_t*)
    {
        return LVERR_NOTIMPL;
    }
    virtual lverror_t Read(void* buf, lvsize_t size, lvsize_t* pBytesRead)
    {
        lvsize_t bytesRead = 0;
        //fprintf( stderr, "Read()\n" );

        lUInt8 * out = (lUInt8 *)buf;

        while (size>0)
        {
            int sz = bytesAvailable();
            if (!sz) {
                m_bytes_pos = m_bytes_count = 0;
                sz = readNextBytes();
                if (!sz) {
                    if ( !bytesRead || m_pos!=m_size) //
                        return LVERR_FAIL;
                    break;
                }
            }
            if (sz>(int)size)
                sz = (int)size;
            for (int i=0; i<sz; i++)
                *out++ = m_bytes[m_bytes_pos++];
            size -= sz;
            bytesRead += sz;
            m_pos += sz;
        }

        if (pBytesRead)
            *pBytesRead = bytesRead;
        //fprintf( stderr, "    %d bytes read...\n", (int)bytesRead );
        return LVERR_OK;
    }
    virtual lverror_t SetSize(lvsize_t)
    {
        return LVERR_NOTIMPL;
    }
};

img_scaling_option_t::img_scaling_option_t()
{
    // Mode: 0=disabled, 1=integer scaling factors, 2=free scaling
    // Scale: 0=auto based on font size, 1=no zoom, 2=scale up to *2, 3=scale up to *3
    mode = IMG_FREE_SCALING;
    max_scale = 3;
    //mode = IMG_NO_SCALE; //IMG_FREE_SCALING // IMG_INTEGER_SCALING
    //max_scale = 1;
}

img_scaling_options_t::img_scaling_options_t()
{
    img_scaling_option_t option;
    img_scaling_option_t option_inline;
    //option_inline.mode=IMG_NO_SCALE;
    zoom_in_inline = option_inline;
    zoom_in_block = option;
    zoom_out_inline = option_inline;
    zoom_out_block = option;
}

xpath_step_t ParseXPathStep( const lChar16 * &path, lString16 & name, int & index )
{
    int pos = 0;
    const lChar16 * s = path;
    //int len = path.GetLength();
    name.clear();
    index = -1;
    int flgPrefix = 0;
    if (s && s[pos]) {
        lChar16 ch = s[pos];
        // prefix: none, '/' or '.'
        if (ch=='/') {
            flgPrefix = 1;
            ch = s[++pos];
        } else if (ch=='.') {
            flgPrefix = 2;
            ch = s[++pos];
        }
        int nstart = pos;
        if (ch>='0' && ch<='9') {
            // node or point index
            pos++;
            while (s[pos]>='0' && s[pos]<='9')
                pos++;
            if (s[pos] && s[pos!='/'] && s[pos]!='.')
                return xpath_step_error;
            lString16 sindex( path+nstart, pos-nstart );
            index = sindex.atoi();
            if (index<((flgPrefix==2)?0:1))
                return xpath_step_error;
            path += pos;
            return (flgPrefix==2) ? xpath_step_point : xpath_step_nodeindex;
        }
        while (s[pos] && s[pos]!='[' && s[pos]!='/' && s[pos]!='.')
            pos++;
        if (pos==nstart)
            return xpath_step_error;
        name = lString16( path+ nstart, pos-nstart );
        if (s[pos]=='[') {
            // index
            pos++;
            int istart = pos;
            while (s[pos] && s[pos]!=']' && s[pos]!='/' && s[pos]!='.')
                pos++;
            if (!s[pos] || pos==istart)
                return xpath_step_error;

            lString16 sindex( path+istart, pos-istart );
            index = sindex.atoi();
            pos++;
        }
        if (!s[pos] || s[pos]=='/' || s[pos]=='.') {
            path += pos;
            return (name == "text()") ? xpath_step_text : xpath_step_element; // OK!
        }
        return xpath_step_error; // error
    }
    return xpath_step_error;
}

/// get pointer for relative path
ldomXPointer ldomXPointer::relative( lString16 relativePath )
{
    return _data->getDocument()->createXPointer( getNode(), relativePath );
}

ldomXPointer CrDom::convertXPointerFromMuPdf(const lString16 & xPointerStr )
{
    lString16 newstr;
    ldomXPointer result;
    //("/page[%d]/block[%d]/line[%d]/char[%d]", pagenum, blocknum, linenum, charnum);
    int pagestart  = xPointerStr.pos("[") + 1;
    int pageend    = xPointerStr.pos("]");

    int blockstart = xPointerStr.pos("[",pageend) + 1;
    int blockend   = xPointerStr.pos("]",pageend+1);

    int linestart = xPointerStr.pos("[",blockend) + 1;
    int lineend   = xPointerStr.pos("]",blockend+1);

    int charstart = xPointerStr.pos("[",lineend) + 1;
    int charend   = xPointerStr.pos("]",lineend+1);

    //CRLog::error("ps = %d, pe = %d",pagestart,pageend);
    //CRLog::error("bs = %d, be = %d",blockstart,blockend);
    //CRLog::error("ls = %d, le = %d",linestart,lineend);
    //CRLog::error("cs = %d, ce = %d",charstart,charend);

    lString16 linestr = xPointerStr.substr(linestart ,lineend - linestart  );
    lString16 chstr   = xPointerStr.substr(charstart ,charend - charstart  );

    int page  = atoi(LCSTR( xPointerStr.substr(pagestart ,pageend - pagestart  )));
    int block = atoi(LCSTR( xPointerStr.substr(blockstart,blockend- blockstart )));
    int ch    = atoi(LCSTR(chstr));

    page+=1; // xml element numeration starts from 1
    block+=1; // xml element numeration starts from 1

    newstr.append(L"/FictionBook/body/section[").append(page).append(L"]/p[").append(block).append(L"]");
    ldomXPointer xp = createXPointer( getRootNode(), newstr );

    ldomNode * blocknode = xp.getNode();
    if(blocknode == nullptr)
    {
        CRLog::error("blocknode creation failed: newstr = %s",LCSTR(newstr));
        return ldomXPointer();
    }
    for (int i = 0; i < blocknode->getChildCount(); i++)
    {
        ldomNode * child = blocknode->getChildNode(i);
        if(child->getAttributeValue("data-line") == linestr)
        {
            int datastart = atoi(LCSTR(child->getAttributeValue("data-start")));
            int dataend = datastart + child->getText().length();
            if (ch > datastart && ch < dataend)
            {
                int newch = ch - datastart;
                newstr.append("/"+child->getXPathSegment()).append("/text().").append(newch);
                //CRLog::error("newstr = %s",LCSTR(newstr));
                result = createXPointer(newstr);
                result.setNode(child);
                result.setOffset(newch);
                return result;
            }
        }
    }
    CRLog::error("not found");
    return result;
}

int countOffsetRecursiveDeeper(ldomNode *node, ldomNode *stopnode)
{
    int sum = 0;
    for (int i = 0; i < node->getChildCount(); i++)
    {
        ldomNode *child = node->getChildNode(i);

        if (child == stopnode)
        {
            return sum;
        }
        sum += countOffsetRecursiveDeeper(child, stopnode);
    }
    return sum;
}

int countOffsetRecursiveBlock(ldomNode * node, ldomNode* stopnode, int searchline)
{
    int sum = 0;
    for (int i = 0; i < node->getChildCount(); i++)
    {
        ldomNode* child = node->getChildNode(i);
        int line = atoi(LCSTR(child->getAttributeValue("data-line")));
        if(line != searchline)
            continue;
        sum += countOffsetRecursiveDeeper(child, stopnode);
    }
    return sum;
}

lString16 CrDom::convertXPointerToMuPdf(ldomXPointer xp)
{
    ldomNode* orignode = xp.getNode();
    ldomNode* node = xp.getNode();
    //assert(node != NULL);
    if(node == NULL)
    {
        LE("node == NULL!");
        return lString16::empty_str;
    }

    int ch = xp.getOffset();

    while (node->getAttributeValue("data-line") == lString16::empty_str  && node != nullptr)
    {
        node = node->getParentNode();
    }
    int line = atoi(LCSTR(node->getAttributeValue("data-line")));

    ldomNode *blocknode;
    while (node->getAttributeValue("data-block") == lString16::empty_str && node != nullptr)
    {
        node = node->getParentNode();
        blocknode = node;
    }

    ch = ch + countOffsetRecursiveBlock(blocknode,orignode, line);

    int block = atoi(LCSTR(node->getAttributeValue("data-block")));
    while (node->getAttributeValue("data-page") == lString16::empty_str  && node != nullptr)
    {
        node = node->getParentNode();
    }
    int page = atoi(LCSTR(node->getAttributeValue("data-page")));

    lString16 newstr;

    newstr.append(L"/page[").append(page).append(L"]/block[").append(block).append(L"]/line[").append(line).append(L"]/char[").append(ch).append(L"]");
    return newstr;
}



/// create xpointer from pointer string
ldomXPointer CrDom::createXPointer(const lString16 & xPointerStr)
{
    if ( xPointerStr[0]=='#' ) {
        lString16 id = xPointerStr.substr(1);
        lUInt16 idid = getAttrValueIndex(id.c_str());
        lInt32 nodeIndex;
        if ( _idNodeMap.get(idid, nodeIndex) ) {
            ldomNode * node = getTinyNode(nodeIndex);
            if ( node && node->isElement() ) {
                return ldomXPointer(node, -1);
            }
        }
        return ldomXPointer();
    }
    return createXPointer( getRootNode(), xPointerStr );
}

/// return parent final node, if found
ldomNode * ldomXPointer::getFinalNode() const
{
    ldomNode * node = getNode();
    for (;;) {
        if ( !node )
            return NULL;
        if ( node->getRendMethod()==erm_final )
            return node;
        node = node->getParentNode();
    }
}

/// create xpointer from doc point
ldomXPointer CrDom::createXPointer(lvPoint pt, int direction)
{
    ldomXPointer ptr;
    if (!getRootNode()) {
        return ptr;
    }
    ldomNode* finalNode = getRootNode()->elementFromPoint(pt, direction);
    if (!finalNode) {
        if (pt.y >= getFullHeight()) {
            ldomNode* node = getRootNode()->getLastTextChild();
            return ldomXPointer(node, node ? node->getText().length() : 0);
        } else if (pt.y <= 0) {
            ldomNode* node = getRootNode()->getFirstTextChild();
            return ldomXPointer(node, 0);
        }
        CRLog::trace("createXPointer not final node");
        return ptr;
    }
    lvRect rc;
    finalNode->getAbsRect(rc);
    //CRLog::trace("CrDom::createXPointer point = (%d, %d), finalNode %08X rect = (%d,%d,%d,%d)",
    //    pt.x, pt.y, (lUInt32)finalNode, rc.left, rc.top, rc.right, rc.bottom );
    pt.x -= rc.left;
    pt.y -= rc.top;
    //if ( !r )
    //    return ptr;
    if (finalNode->getRendMethod() != erm_final && finalNode->getRendMethod() != erm_list_item) {
        // not final, use as is
        if (pt.y < (rc.bottom + rc.top) / 2) {
            return ldomXPointer(finalNode, 0);
        } else {
            return ldomXPointer(finalNode, finalNode->getChildCount());
        }
    }
    // final, format and search
    LFormattedTextRef txtform;
    {
        RenderRectAccessor r(finalNode);
        finalNode->renderFinalBlock(txtform, &r, r.getWidth());
    }
    int lcount = txtform->GetLineCount();
    for (int l = 0; l < lcount; l++) {
        const formatted_line_t* frmline = txtform->GetLineInfo(l);
        if (pt.y >= (int) (frmline->y + frmline->height) && l < lcount - 1) {
            continue;
        }
        //CRLog::trace("  point (%d, %d) line found [%d]: (%d..%d)",
        //    pt.x, pt.y, l, frmline->y, frmline->y+frmline->height);
        // found line, searching for word
        int wc = (int) frmline->word_count;
        int x = pt.x - frmline->x;
        for (int w = 0; w < wc; w++) {
            const formatted_word_t* word = &frmline->words[w];
            if (x < word->x + word->width || w == wc - 1) {
                const src_text_fragment_t* src = txtform->GetSrcInfo(word->src_text_index);
                //CRLog::trace(" word found [%d]: x=%d..%d, start=%d, len=%d  %08X",
                //    w, word->x, word->x + word->width, word->t.start, word->t.len, src->object);
                // found word, searching for letters
                ldomNode* node = (ldomNode*) src->object;
                if (!node) {
                    continue;
                }
                if (src->flags & LTEXT_SRC_IS_OBJECT) {
                    // object (image)
#if 1
                    // return image object itself
                    return ldomXPointer(node, 0);
#else
                    return ldomXPointer( node->getParentNode(),
                        node->getNodeIndex() + (( x < word->x + word->width/2 ) ? 0 : 1) );
#endif
                }
                LVFont* font = (LVFont*) src->t.font;
                lUInt16 w[512];
                lUInt8 flg[512];
                lString16 str = node->getText();
                font->measureText(
                        str.c_str() + word->t.start,
                        word->t.len,
                        w,
                        flg,
                        word->width + 50,
                        '?',
                        src->letter_spacing);
                for (int i = 0; i < word->t.len; i++) {
                    int xx = (i > 0) ? (w[i - 1] + w[i]) / 2 : w[i] / 2;
                    if (x < word->x + xx) {
                        return ldomXPointer(node, src->t.offset + word->t.start + i);
                    }
                }
                return ldomXPointer(node, src->t.offset + word->t.start + word->t.len);
            }
        }
    }
    return ptr;
}

/// returns coordinates of pointer inside formatted document
lvPoint ldomXPointer::toPoint() const
{
    lvRect rc;
    //к GetRect добавлен параметр, который отвечает за включение проблемного участка кода.
    if (!getRect(rc, true))
        return lvPoint(-1, -1);
    return rc.topLeft();

    //New implementation
    /*
    ldomNode * node = this->getNode();
    int offset = this->getOffset();
    ldomXPointerEx xp = ldomXPointerEx(node,offset);
    RectHelper rh;
    rh.FindLastIndexEnable_ = true;
    rh.Init(node);
    if(!rh.processRect(xp,rc))
        return lvPoint(-1, -1);
    return rc.topLeft();
    */
}

/// returns caret rectangle for pointer inside formatted document
bool ldomXPointer::getRect(lvRect &rect, bool forlvpoint) const
{
    #if DEBUG_GETRECT_LOGS
    CRLog::debug("OLD getrect Start");
    //CRLog::trace("ldomXPointer::getRect()");
    #endif
    if (isNull())
    {
        #if DEBUG_GETRECT_LOGS
        CRLog::error("OLD XPOINTER IS NULL RETURN");
        #endif
        return false;
    }
    ldomNode *p = isElement() ? getNode() : getNode()->getParentNode();
    ldomNode *p0 = p;
    ldomNode *finalNode = NULL;
    if (!p)
    {
        //CRLog::trace("ldomXPointer::getRect() - p==NULL");
    }
    //printf("getRect( p=%08X type=%d )\n", (unsigned)p, (int)p->getNodeType() );
    if (!p->getCrDom())
    {
        //CRLog::trace("ldomXPointer::getRect() - p->getCrDom()==NULL");
    }
    ldomNode *mainNode = p->getCrDom()->getRootNode();
    for (; p; p = p->getParentNode())
    {
        int rm = p->getRendMethod();
        if (rm == erm_final || rm == erm_list_item)
        {
            finalNode = p; // found final block
        }
        else if (p->getRendMethod() == erm_invisible)
        {
            //CRLog::error("OLD INVISIBLE NODE RETURN");
            return false; // invisible !!!
        }
        if (p == mainNode)
        {
            break;
        }
    }

    if (finalNode == NULL)
    {
        lvRect rc;
        p0->getAbsRect(rc);
        // Это очень сильно спамит в лог
        //CRLog::trace("node w/o final parent: %d..%d", rc.top, rc.bottom);
    }

    if (finalNode != NULL)
    {

        lvRect rc;
        finalNode->getAbsRect(rc);
        if (rc.height() == 0 && rc.width() > 0)
        {
            rect = rc;
            rect.bottom++;
            return true;
        }
        RenderRectAccessor r(finalNode);
        //if ( !r )
        //    return false;
        LFormattedTextRef txtform;
        finalNode->renderFinalBlock(txtform, &r, r.getWidth());

        ldomNode *node = getNode();

        int offset = getOffset();

        // text node
        int srcIndex = -1;
        int srcLen = -1;
        int lastIndex = -1;
        int lastLen = -1;
        int lastOffset = -1;
        ldomXPointerEx xp(node, offset);
        #if DEBUG_GETRECT_LOGS
        //CRLog::error("OLD FINAL NODE = [%s]",LCSTR(finalNode->getXPath()));
        //CRLog::error("OLD FINAL NODE TEXT = [%s]",LCSTR(finalNode_->getText()));
        //CRLog::error("OLD NODE      = [%s]",LCSTR(node->getXPath()));
        //CRLog::error("OLD NODE TEXT = [%s]",LCSTR(node->getText()));
        #endif
        for (int i = 0; i < txtform->GetSrcCount(); i++)
        {
            const src_text_fragment_t *src = txtform->GetSrcInfo(i);
            bool isObject = (src->flags & LTEXT_SRC_IS_OBJECT) != 0;
            if (src->object == node)
            {
                srcIndex = i;
                srcLen = isObject ? 0 : src->t.len;
                break;
            }
            lastIndex = i;
            lastLen = isObject ? 0 : src->t.len;
            lastOffset = isObject ? 0 : src->t.offset;
            // Проблемный участок, от которого одновременно зависит
            // генерация оглавлений (Гаррисон), и генерация хитбоксов в некоторых случаях (Storia della mafia)
            // К функции GetRect добавлен параметр forlvpoint, который при вызове в генерации оглавлений, включает этот кусок.
            if (forlvpoint )
            {
                ldomXPointerEx xp2((ldomNode *) src->object, lastOffset);
                if(xp2.compare(xp) > 0)
                {
                    srcIndex = i;
                    srcLen = lastLen;
                    offset = lastOffset;
                    //CRLog::error("forlvpoint!!!!!!!!!! break");
                    break;
                }
            }
            //конец проблемного участка кода
        }

        if (srcIndex == -1)
        {
            if (lastIndex < 0)
            {
                return false;
                #if DEBUG_GETRECT_LOGS
                //CRLog::error("LAST INDEX < 0 RETURN");
                #endif
            }
           #if DEBUG_GETRECT_LOGS
            //CRLog::error("OLD offset = %d",offset);
            //CRLog::error("OLD lastoffset = %d",lastOffset);
            //CRLog::error("OLD replacing offset with last offset");
           #endif
            srcIndex = lastIndex;
            srcLen = lastLen;
            offset = lastOffset;
        }
        #if DEBUG_GETRECT_LOGS
        CRLog::error("srcIndex = %d srcLen = %d lastIndex = %d lastLen = %d lastOffset = %d", srcIndex, srcLen, lastIndex, lastLen, lastOffset);
        #endif
        for (int l = 0; l < txtform->GetLineCount(); l++)
        {
            const formatted_line_t *frmline = txtform->GetLineInfo(l);
            for (int w = 0; w < (int) frmline->word_count; w++)
            {
                const formatted_word_t *word = &frmline->words[w];
                bool lastWord = (l == txtform->GetLineCount() - 1 && w == frmline->word_count - 1);
                if (word->src_text_index >= srcIndex || lastWord)
                {
                    #if DEBUG_GETRECT_LOGS
                    //CRLog::error("word->src_text_index > srcIndex || offset <= word->t.start");
                    //CRLog::error("%d>%d || %d <= %d",word->src_text_index,srcIndex,offset,word->t.start);
                    //CRLog::error("(offset < word->t.start + word->t.len) || (offset == srcLen && offset == word->t.start + word->t.len)");
                    //CRLog::error("(%d < %d + %d) || (%d == %d && %d == %d + %d)",offset, word->t.start , word->t.len,offset , srcLen ,offset , word->t.start , word->t.len);

                    #endif
                    // found word from same src line
                    if (word->src_text_index > srcIndex || offset <= word->t.start)
                    {
                        // before this word
                        rect.left = word->x + rc.left + frmline->x;
                        //rect.top = word->y + rc.top + frmline->y + frmline->baseline;
                        rect.top = rc.top + frmline->y;
                        rect.right = rect.left + 1;
                        rect.bottom = rect.top + frmline->height;
                        #if DEBUG_GETRECT_LOGS
                        //CRLog::error("word->x = %d, rc.left = %d, frmline->x = %d",word->x,rc.left,frmline->x);
                        CRLog::error("l = %d, w = %d lastword = %d",l,w,(lastWord)?1:0);
                        CRLog::error("old Rect1 = [%d:%d][%d:%d]",rect.left,rect.right,rect.top,rect.bottom);
                        #endif
                        return true;
                    }
                    else if ((offset < word->t.start + word->t.len) || (offset == srcLen && offset == word->t.start + word->t.len))
                    {
                      // pointer inside this word
                        LVFont *font = (LVFont *) txtform->GetSrcInfo(srcIndex)->t.font;
                        lUInt16 widths[512];
                        lUInt8 flg[512];
                        lString16 str = node->getText();
                        font->measureText(str.c_str() + word->t.start, offset - word->t.start, widths, flg, word->width + 50, '?', txtform->GetSrcInfo(srcIndex)->letter_spacing);
                        int chx = widths[offset - word->t.start - 1];
                        rect.left = word->x + rc.left + frmline->x + chx;
                        //rect.top = word->y + rc.top + frmline->y + frmline->baseline;
                        rect.top = rc.top + frmline->y;
                        rect.right = rect.left + 1;
                        rect.bottom = rect.top + frmline->height;
                        #if DEBUG_GETRECT_LOGS
                        CRLog::error("l = %d, w = %d lastword = %d",l,w,(lastWord)?1:0);
                        CRLog::error("old Rect2 = [%d:%d][%d:%d]",rect.left,rect.right,rect.top,rect.bottom);
                        #endif
                        return true;
                    }
                    else if (lastWord)
                    {
                        // after last word
                        rect.left = word->x + rc.left + frmline->x + word->width;
                        //rect.top = word->y + rc.top + frmline->y + frmline->baseline;
                        rect.top = rc.top + frmline->y;
                        rect.right = rect.left + 1;
                        rect.bottom = rect.top + frmline->height;
                        #if DEBUG_GETRECT_LOGS
                        CRLog::error("l = %d, w = %d lastword = %d",l,w,(lastWord)?1:0);
                        CRLog::error("old Rect3 = [%d:%d][%d:%d]",rect.left,rect.right,rect.top,rect.bottom);
                        #endif
                        return true;
                    }
                }
            }
        }
        return false;
    }
    else
    {
        // no base final node, using blocks
        //lvRect rc;
        ldomNode *node = getNode();
        //CRLog::error("OLD NODE = [%s]",LCSTR(node->getXPath()));
        int offset = getOffset();
        if (offset < 0 || node->getChildCount() == 0)
        {
            #if DEBUG_GETRECT_LOGS
            //CRLog::error("offset = %d",offset);
            //CRLog::error("childcoount = %d",node->getChildCount());

            CRLog::error("OLD Ifnull 1");
            #endif
            node->getAbsRect(rect);
            return true;
            //return rc.topLeft();
        }
        if (offset < (int) node->getChildCount())
        {
            node->getChildNode(offset)->getAbsRect(rect);
            #if DEBUG_GETRECT_LOGS
            CRLog::error("OLD Ifnull 2");
            #endif
            return true;
            //return rc.topLeft();
        }
        #if DEBUG_GETRECT_LOGS
        CRLog::error("OLD Ifnull 3");
        #endif
        node->getChildNode(node->getChildCount() - 1)->getAbsRect(rect);
        return true;
        //return rc.bottomRight();
    }
}

/// create xpointer from relative pointer string
ldomXPointer CrDom::createXPointer( ldomNode * baseNode, const lString16 & xPointerStr )
{
    //CRLog::trace( "CrDom::createXPointer(%s)", UnicodeToUtf8(xPointerStr).c_str() );
    if ( xPointerStr.empty() )
        return ldomXPointer();
    const lChar16 * str = xPointerStr.c_str();
    int index = -1;
    ldomNode * currNode = baseNode;
    lString16 name;
    lString8 ptr8 = UnicodeToUtf8(xPointerStr);
    //const char * ptr = ptr8.c_str();
    xpath_step_t step_type;

    while ( *str ) {
        //CRLog::trace( "    %s", UnicodeToUtf8(lString16(str)).c_str() );
        step_type = ParseXPathStep( str, name, index );
        //CRLog::trace( "        name=%s index=%d", UnicodeToUtf8(lString16(name)).c_str(), index );
        switch (step_type ) {
        case xpath_step_error:
            // error
            //CRLog::trace("    xpath_step_error");
            return ldomXPointer();
        case xpath_step_element:
            // element of type 'name' with 'index'        /elemname[N]/
            {
                lUInt16 id = getElementNameIndex( name.c_str() );
                ldomNode * foundItem = currNode->findChildElement(
                        LXML_NS_ANY,
                        id,
                        index > 0 ? index - 1 : -1);
                if (foundItem == NULL && currNode->getChildCount() == 1) {
                    // make saved pointers work properly even after moving of some part of path
                    // one element deeper
                    foundItem = currNode->getChildNode(0)->findChildElement(
                            LXML_NS_ANY,
                            id,
                            index > 0 ? index - 1 : -1);
                }
//                int foundCount = 0;
//                for (unsigned i=0; i<currNode->getChildCount(); i++) {
//                    ldomNode * p = currNode->getChildNode(i);
//                    CRLog::trace( "node[%d]: %d %s", i, p->getNodeId(), LCSTR(p->getNodeName()));
//                    if ( p && p->isElement() && p->getNodeId()==id ) {
//                        foundCount++;
//                        if ( foundCount==index || index==-1 ) {
//                            foundItem = p;
//                            break; // DON'T CHECK WHETHER OTHER ELEMENTS EXIST
//                        }
//                    }
//                }
//                if ( foundItem==NULL || (index==-1 && foundCount>1) ) {
//                    //CRLog::trace("    Element %d is not found. foundCount=%d", id, foundCount);
//                    return ldomXPointer(); // node not found
//                }
                if (foundItem == NULL) {
                    //CRLog::trace("    Element %d is not found. foundCount=%d", id, foundCount);
                    return ldomXPointer(); // node not found
                }
                // found element node
                currNode = foundItem;
            }
            break;
        case xpath_step_text:
            // text node with 'index'                     /text()[N]/
            {
                ldomNode * foundItem = NULL;
                int foundCount = 0;
                for (int i=0; i<currNode->getChildCount(); i++) {
                    ldomNode * p = currNode->getChildNode(i);
                    if ( p->isText() ) {
                        foundCount++;
                        if ( foundCount==index || index==-1 ) {
                            foundItem = p;
                        }
                    }
                }
                if ( foundItem==NULL || (index==-1 && foundCount>1) )
                    return ldomXPointer(); // node not found
                // found text node
                currNode = foundItem;
            }
            break;
        case xpath_step_nodeindex:
            // node index                                 /N/
            if ( index<=0 || index>(int)currNode->getChildCount() )
                return ldomXPointer(); // node not found: invalid index
            currNode = currNode->getChildNode( index-1 );
            break;
        case xpath_step_point:
            // point index                                .N
            if (*str)
                return ldomXPointer(); // not at end of string
            if ( currNode->isElement() ) {
                // element point
                if ( index<0 || index>(int)currNode->getChildCount() )
                    return ldomXPointer();
                return ldomXPointer(currNode, index);
            } else {
                // text point
                if ( index<0 || index>(int)currNode->getText().length() )
                    return ldomXPointer();
                return ldomXPointer(currNode, index);
            }
            break;
        }
    }
    return ldomXPointer( currNode, -1 ); // XPath: index==-1
}

/// returns XPath segment for this element relative to parent element (e.g. "p[10]")
lString16 ldomNode::getXPathSegment()
{
    if ( isNull() || isRoot() )
        return lString16::empty_str;
    ldomNode * parent = getParentNode();
    int cnt = parent->getChildCount();
    int index = 0;
    if ( isElement() ) {
        int id = getNodeId();
        for ( int i=0; i<cnt; i++ ) {
            ldomNode * node = parent->getChildNode(i);
            if ( node == this ) {
                return getNodeName() + "[" + fmt::decimal(index+1) + "]";
            }
            if ( node->isElement() && node->getNodeId()==id )
                index++;
        }
    } else {
        for ( int i=0; i<cnt; i++ ) {
            ldomNode * node = parent->getChildNode(i);
            if ( node == this ) {
                return "text()[" + lString16::itoa(index+1) + "]";
            }
            if ( node->isText() )
                index++;
        }
    }
    return lString16::empty_str;
}

lString16 ldomNode::getPath()
{
    if(this==NULL)
    {
        return lString16("NULL_NODE");
    }
    lString16 result;
    ldomNode * node = this;
    result = node->getXPathSegment();
    node = node->getParentNode();
    while (node!=NULL)
    {
        result = node->getXPathSegment() + "/" + result;
        node = node->getParentNode();
    }
    return result;
}


lString16 ldomNode::getXPath()
{
if(this==NULL)
{
    CRLog::error("GetXPath: NODE IS NULL");
    return lString16("NULLNODE");
}
    lString16 result;
    lString16 brackets = lString16("[" + lString16::itoa(this->getNodeIndex()+1) + "]");

    if (this->isText())
    {
        result.append(lString16("Text") + brackets);
    }
    else
    {
        result.append(this->getXPathSegment());
    }
    ldomNode *parent = this->getParentNode();
    while (parent != NULL)
    {
        result = parent->getXPathSegment() + "/" + result;
        parent = parent->getParentNode();
    }
    return result;
}
lString16 ldomXPointer::toString()
{
    lString16 path;
    if (isNull()) {
        return path;
    }
    ldomNode* node = getNode();
    int offset = getOffset();
    if (offset >= 0) {
        path << "." << fmt::decimal(offset);
    }
    ldomNode* p = node;
    ldomNode* mainNode = node->getCrDom()->getRootNode();
    while (p && p != mainNode) {
        ldomNode* parent = p->getParentNode();
        if (p->isElement()) {
            // element
            lString16 name = p->getNodeName();
            lUInt16 id = p->getNodeId();
            if (!parent) {
                return "/" + name + path;
            }
            int index = -1;
            int count = 0;
            for (int i = 0; i < parent->getChildCount(); i++) {
                ldomNode* node = parent->getChildElementNode(i, id);
                if (node) {
                    count++;
                    if (node == p) {
                        index = count;
                    }
                }
            }
            if (count > 1) {
                path = cs16("/") + name + "[" + fmt::decimal(index) + "]" + path;
            } else {
                path = cs16("/") + name + path;
            }
        } else {
            // text
            if (!parent) {
                return cs16("/text()") + path;
            }
            int index = -1;
            int count = 0;
            for (int i = 0; i < parent->getChildCount(); i++) {
                ldomNode* node = parent->getChildNode(i);
                if (node->isText()) {
                    count++;
                    if (node == p) {
                        index = count;
                    }
                }
            }
            if (count > 1) {
                path = cs16("/text()") + "[" + fmt::decimal(index) + "]" + path;
            } else {
                path = "/text()" + path;
            }
        }
        p = parent;
    }
    return path;
}

int CrDom::getFullHeight()
{
    RenderRectAccessor rd( this->getRootNode() );
    return rd.getHeight() + rd.getY();
}


lString16 ExtractDocAuthors(CrDom* dom, lString16 delimiter)
{
    if (delimiter.empty()) {
        delimiter = ", ";
    }
    lString16 authors;
    for (int i = 0; i < 64; i++) {
        lString16 path =
                cs16("/FictionBook/description/title-info/author[") + fmt::decimal(i + 1) + "]";
        ldomXPointer pauthor = dom->createXPointer(path);
        if (!pauthor) {
            //CRLog::trace( "xpath not found: %s", UnicodeToUtf8(path).c_str() );
            break;
        }
        lString16 firstName = metaSanitize(pauthor.relative(L"/first-name").getText());
        lString16 lastName = metaSanitize(pauthor.relative(L"/last-name").getText());
        lString16 middleName = metaSanitize(pauthor.relative(L"/middle-name").getText());
        lString16 author = firstName;
        if (!author.empty()) {
            author += " ";
        }
        if (!middleName.empty()) {
            author += middleName;
        }
        if (!lastName.empty() && !author.empty()) {
            author += " ";
        }
        author += lastName;
        if(author.empty())
        {
            continue;
        }
        if (!authors.empty()) {
            authors += delimiter;
        }
        authors += author;
    }
    return metaSanitize(authors);
}

lString16 ExtractDocTitle(CrDom* dom)
{
    return metaSanitize(dom->createXPointer(L"/FictionBook/description/title-info/book-title").getText());
}

lString16 ExtractDocGenres(CrDom *dom, lString16 delimiter)
{
    if (delimiter.empty())
    {
        delimiter = "|";
    }
    lString16 genres;
    for (int i = 0; i < 16; i++)
    {
        lString16 path = cs16("/FictionBook/description/title-info/genre[") + fmt::decimal(i + 1) + "]";
        ldomXPointer node = dom->createXPointer(path);
        if (!node)
        {
            //CRLog::trace( "xpath not found: %s", UnicodeToUtf8(path).c_str() );
            break;
        }
        genres += node.getText();
        genres += delimiter;
    }
    bool replace = genres.replace(L"_",L" ");
    while (replace)
    {
        replace = genres.replace(L"_",L" ");
    }
    genres = genres.substr(0, genres.length() - 1);
    return metaSanitize(genres);
}

lString16 ExtractDocAnnotation(CrDom* dom)
{
    lString16 annotation = dom->createXPointer(L"/FictionBook/description/title-info/annotation").getText();
    if(annotation.empty())
    {
        annotation = dom->createXPointer(L"/FictionBook/description/src-title-info/annotation").getText();
    }
    if(annotation.empty())
    {
        annotation = dom->createXPointer(L"/FictionBook/description/publish-info/annotation").getText();
    }
    return metaSanitize(annotation);
}

lString16 ExtractDocLanguage(CrDom* doc)
{
    return metaSanitize(doc->createXPointer(L"/FictionBook/description/title-info/lang").getText());
}

lString16 ExtractDocSeries(CrDom* dom, int* p_series_number)
{
    lString16 res;
    ldomNode* series = dom->createXPointer(L"/FictionBook/description/title-info/sequence").getNode();
    lString16 sname;
    lString16 snumber;
    if (series) {
        sname = lString16(series->getAttributeValue(attr_name)).trim();
        snumber = series->getAttributeValue(attr_number);
    }

    if(sname.empty())
    {
        series =  dom->createXPointer(L"/FictionBook/description/publish-info/sequence").getNode();
        if(series)
        {
            sname = lString16(series->getAttributeValue(attr_name)).trim();
            snumber = series->getAttributeValue(attr_number);
        }
    }

    if (!sname.empty()) {
        if (p_series_number != NULL)
        {
            *p_series_number = strtol(LCSTR(snumber),NULL,10);
            res = sname;
        }
        else
        {
            //nowhere to write snumber so we write it into series name
            // "(sname #snumber)"
            res << "(" << sname;
            if (!snumber.empty())
                res << " #" << snumber << ")";
        }
        return metaSanitize(res);
    }
    return lString16::empty_str;
}

lString16 ExtractDocThumbImageName(CrDom* doc)
{
    lString16 result = doc->createXPointer(L"/FictionBook/description/title-info/coverpage/image").getImgHRef();
    if (result.startsWith("#"))
    {
        result = result.substr(1);
    }
    return result;
}

void ldomXPointerEx::initIndex()
{
    int m[MAX_DOM_LEVEL];
    ldomNode * p = getNode();
    _level = 0;
    while ( p ) {
        m[_level] = p->getNodeIndex();
        _level++;
        p = p->getParentNode();
    }
    for ( int i=0; i<_level; i++ ) {
        _indexes[ i ] = m[ _level - i - 1 ];
    }
}

/// move to sibling #
bool ldomXPointerEx::sibling( int index )
{
    if ( _level <= 1 )
        return false;
    ldomNode * p = getNode()->getParentNode();
    if ( !p || index < 0 || index >= (int)p->getChildCount() )
        return false;
    setNode( p->getChildNode( index ) );
    setOffset(0);
    _indexes[ _level-1 ] = index;
    return true;
}

/// move to next sibling
bool ldomXPointerEx::nextSibling()
{
    return sibling( _indexes[_level-1] + 1 );
}

/// move to previous sibling
bool ldomXPointerEx::prevSibling()
{
    if ( _level <= 1 )
        return false;
    return sibling( _indexes[_level-1] - 1 );
}

/// move to next sibling element
bool ldomXPointerEx::nextSiblingElement()
{
    if ( _level <= 1 )
        return false;
    ldomNode * node = getNode();
    ldomNode * p = node->getParentNode();
    for ( int i=_indexes[_level-1] + 1; i<(int)p->getChildCount(); i++ ) {
        if ( p->getChildNode( i )->isElement() )
            return sibling( i );
    }
    return false;
}

/// move to previous sibling element
bool ldomXPointerEx::prevSiblingElement()
{
    if ( _level <= 1 )
        return false;
    ldomNode * node = getNode();
    ldomNode * p = node->getParentNode();
    for ( int i=_indexes[_level-1] - 1; i>=0; i-- ) {
        if ( p->getChildNode( i )->isElement() )
            return sibling( i );
    }
    return false;
}

/// move to parent
bool ldomXPointerEx::parent()
{
    if ( _level<=1 )
        return false;
    setNode( getNode()->getParentNode() );
    setOffset(0);
    _level--;
    return true;
}

/// move to child #
bool ldomXPointerEx::child( int index )
{
    if ( _level >= MAX_DOM_LEVEL )
        return false;
    int count = getNode()->getChildCount();
    if ( index<0 || index>=count )
        return false;
    _indexes[ _level++ ] = index;
    setNode( getNode()->getChildNode( index ) );
    setOffset(0);
    return true;
}

/// compare two pointers, returns -1, 0, +1
int ldomXPointerEx::compare( const ldomXPointerEx& v ) const
{
    int i;
    for ( i=0; i<_level && i<v._level; i++ ) {
        if ( _indexes[i] < v._indexes[i] )
            return -1;
        if ( _indexes[i] > v._indexes[i] )
            return 1;
    }
    if ( _level < v._level ) {
        return -1;
//        if ( getOffset() < v._indexes[i] )
//            return -1;
//        if ( getOffset() > v._indexes[i] )
//            return 1;
//        return -1;
    }
    if ( _level > v._level ) {
        if ( _indexes[i] < v.getOffset() )
            return -1;
        if ( _indexes[i] > v.getOffset() )
            return 1;
        return 1;
    }
    if ( getOffset() < v.getOffset() )
        return -1;
    if ( getOffset() > v.getOffset() )
        return 1;
    return 0;
}

/// calls specified function recursively for all elements of DOM tree
void ldomXPointerEx::recurseElements( void (*pFun)( ldomXPointerEx & node ) )
{
    if ( !isElement() )
        return;
    pFun( *this );
    if ( child( 0 ) ) {
        do {
            recurseElements( pFun );
        } while ( nextSibling() );
        parent();
    }
}

/// calls specified function recursively for all nodes of DOM tree
void ldomXPointerEx::recurseNodes( void (*pFun)( ldomXPointerEx & node ) )
{
    if ( !isElement() )
        return;
    pFun( *this );
    if ( child( 0 ) ) {
        do {
            recurseElements( pFun );
        } while ( nextSibling() );
        parent();
    }
}

/// returns true if this interval intersects specified interval
bool ldomXRange::checkIntersection( ldomXRange & v )
{
    if ( isNull() || v.isNull() )
        return false;
    if ( _end.compare( v._start ) < 0 )
        return false;
    if ( _start.compare( v._end ) > 0 )
        return false;
    return true;
}

/// create list by filtering existing list, to get only values which intersect filter range
ldomXRangeList::ldomXRangeList( ldomXRangeList & srcList, ldomXRange & filter )
{
    for ( int i=0; i<srcList.length(); i++ ) {
        if ( srcList[i]->checkIntersection( filter ) )
            LVPtrVector<ldomXRange>::add( new ldomXRange( *srcList[i] ) );
    }
}

/// copy constructor of full node range
ldomXRange::ldomXRange( ldomNode * p )
: _start( p, 0 ), _end( p, p->isText() ? p->getText().length() : p->getChildCount() ), _flags(1)
{
}

static const ldomXPointerEx & _max( const ldomXPointerEx & v1,  const ldomXPointerEx & v2 )
{
    int c = v1.compare( v2 );
    if ( c>=0 )
        return v1;
    else
        return v2;
}

static const ldomXPointerEx & _min( const ldomXPointerEx & v1,  const ldomXPointerEx & v2 )
{
    int c = v1.compare( v2 );
    if ( c<=0 )
        return v1;
    else
        return v2;
}

/// create intersection of two ranges
ldomXRange::ldomXRange( const ldomXRange & v1,  const ldomXRange & v2 )
    : _start( _max( v1._start, v2._start ) ), _end( _min( v1._end, v2._end ) )
{
}

/// create list splittiny existing list into non-overlapping ranges
ldomXRangeList::ldomXRangeList( ldomXRangeList & srcList, bool splitIntersections )
{
    if ( srcList.empty() )
        return;
    int i;
    if ( splitIntersections ) {
        ldomXRange * maxRange = new ldomXRange( *srcList[0] );
        for ( i=1; i<srcList.length(); i++ ) {
            if ( srcList[i]->getStart().compare( maxRange->getStart() ) < 0 )
                maxRange->setStart( srcList[i]->getStart() );
            if ( srcList[i]->getEnd().compare( maxRange->getEnd() ) > 0 )
                maxRange->setEnd( srcList[i]->getEnd() );
        }
        maxRange->setFlags(0);
        add( maxRange );
        for ( i=0; i<srcList.length(); i++ )
            split( srcList[i] );
        for ( int i=length()-1; i>=0; i-- ) {
            if ( get(i)->getFlags()==0 )
                erase( i, 1 );
        }
    } else {
        for ( i=0; i<srcList.length(); i++ )
            add( new ldomXRange( *srcList[i] ) );
    }
}

/// split into subranges using intersection
void ldomXRangeList::split( ldomXRange * r )
{
    int i;
    for ( i=0; i<length(); i++ ) {
        if ( r->checkIntersection( *get(i) ) ) {
            ldomXRange * src = remove( i );
            int cmp1 = src->getStart().compare( r->getStart() );
            int cmp2 = src->getEnd().compare( r->getEnd() );
            //TODO: add intersections
            if (cmp1 < 0 && cmp2 < 0) {
                //   0====== src ======0
                //        X======= r=========X
                //   1111122222222222222
                ldomXRange* r1 = new ldomXRange(src->getStart(), r->getStart(), src->getFlags());
                ldomXRange* r2 = new ldomXRange(r->getStart(),
                        src->getEnd(),
                        src->getFlags() | r->getFlags());
                insert(i++, r1);
                insert(i, r2);
                delete src;
            } else if (cmp1 > 0 && cmp2 > 0) {
                //           0====== src ======0
                //     X======= r=========X
                //           2222222222222233333
                ldomXRange* r2 = new ldomXRange(src->getStart(),
                        r->getEnd(),
                        src->getFlags() | r->getFlags());
                ldomXRange* r3 = new ldomXRange(r->getEnd(), src->getEnd(), src->getFlags());
                insert(i++, r2);
                insert(i, r3);
                delete src;
            } else if (cmp1 < 0 && cmp2 > 0) {
                // 0====== src ================0
                //     X======= r=========X
                ldomXRange* r1 = new ldomXRange(src->getStart(), r->getStart(), src->getFlags());
                ldomXRange* r2 = new ldomXRange(r->getStart(),
                        r->getEnd(),
                        src->getFlags() | r->getFlags());
                ldomXRange* r3 = new ldomXRange(r->getEnd(), src->getEnd(), src->getFlags());
                insert(i++, r1);
                insert(i++, r2);
                insert(i, r3);
                delete src;
            } else if (cmp1 == 0 && cmp2 > 0) {
                //   0====== src ========0
                //   X====== r=====X
                ldomXRange* r1 = new ldomXRange(src->getStart(),
                        r->getEnd(),
                        src->getFlags() | r->getFlags());
                ldomXRange* r2 = new ldomXRange(r->getEnd(), src->getEnd(), src->getFlags());
                insert(i++, r1);
                insert(i, r2);
                delete src;
            } else if (cmp1 < 0 && cmp2 == 0) {
                //   0====== src =====0
                //      X====== r=====X
                ldomXRange* r1 = new ldomXRange(src->getStart(), r->getStart(), src->getFlags());
                ldomXRange* r2 = new ldomXRange(r->getStart(),
                        r->getEnd(),
                        src->getFlags() | r->getFlags());
                insert(i++, r1);
                insert(i, r2);
                delete src;
            } else {
                //        0====== src =====0
                //   X============== r===========X
                //
                //        0====== src =====0
                //   X============== r=====X
                //
                //   0====== src =====0
                //   X============== r=====X
                //
                //   0====== src ========0
                //   X========== r=======X
                src->setFlags( src->getFlags() | r->getFlags() );
                insert( i, src );
            }
        }
    }
}

bool CrDom::findText(lString16 pattern, bool caseInsensitive, bool reverse, int minY, int maxY,
        LVArray<ldomWord>& words, int maxCount, int maxHeight)
{
    if (minY < 0) {
        minY = 0;
    }
    int fh = getFullHeight();
    if (maxY <= 0 || maxY > fh) {
        maxY = fh;
    }
    ldomXPointer start = createXPointer(lvPoint(0, minY), reverse ? -1 : 1);
    ldomXPointer end = createXPointer(lvPoint(10000, maxY), reverse ? -1 : 1);
    if (start.isNull() || end.isNull()) {
        return false;
    }
    ldomXRange range(start, end);
    CRLog::trace("ldomDocument::findText() for Y %d..%d, range %d..%d",
            minY,
            maxY,
            start.toPoint().y,
            end.toPoint().y);
    if (range.getStart().toPoint().y == -1) {
        range.getStart().nextVisibleText();
        CRLog::trace("ldomDocument::findText() updated range %d..%d",
                range.getStart().toPoint().y,
                range.getEnd().toPoint().y);
    }
    if (range.getEnd().toPoint().y == -1) {
        range.getEnd().prevVisibleText();
        CRLog::trace("ldomDocument::findText() updated range %d..%d",
                range.getStart().toPoint().y,
                range.getEnd().toPoint().y);
    }
    if (range.isNull()) {
        CRLog::trace("No text found: Range is empty");
        return false;
    }
    return range.findText(pattern, caseInsensitive, reverse, words, maxCount, maxHeight);
}

static bool findText( const lString16 & str, int & pos, const lString16 & pattern )
{
    int len = pattern.length();
    if ( pos < 0 || pos + len > (int)str.length() )
        return false;
    const lChar16 * s1 = str.c_str() + pos;
    const lChar16 * s2 = pattern.c_str();
    int nlen = str.length() - pos - len;
    for ( int j=0; j<nlen; j++ ) {
        bool matched = true;
        for ( int i=0; i<len; i++ ) {
            if ( s1[i] != s2[i] ) {
                matched = false;
                break;
            }
        }
        if ( matched )
            return true;
        s1++;
        pos++;
    }
    return false;
}

static bool findTextRev( const lString16 & str, int & pos, const lString16 & pattern )
{
    int len = pattern.length();
    if ( pos+len>(int)str.length() )
        pos = str.length()-len;
    if ( pos < 0 )
        return false;
    const lChar16 * s1 = str.c_str() + pos;
    const lChar16 * s2 = pattern.c_str();
    int nlen = pos - len;
    for ( int j=nlen-1; j>=0; j-- ) {
        bool matched = true;
        for ( int i=0; i<len; i++ ) {
            if ( s1[i] != s2[i] ) {
                matched = false;
                break;
            }
        }
        if ( matched )
            return true;
        s1--;
        pos--;
    }
    return false;
}

/// searches for specified text inside range
bool ldomXRange::findText(lString16 pattern, bool caseInsensitive, bool reverse,
                          LVArray<ldomWord>& words, int maxCount, int maxHeight,
                          bool checkMaxFromStart)
{
    if ( caseInsensitive )
        pattern.lowercase();
    words.clear();
    if ( pattern.empty() )
        return false;
    if ( reverse ) {
        // reverse search
        if ( !_end.isText() ) {
            _end.prevVisibleText();
            lString16 txt = _end.getNode()->getText();
            _end.setOffset(txt.length());
        }
        int firstFoundTextY = -1;
        while ( !isNull() ) {

            lString16 txt = _end.getNode()->getText();
            int offs = _end.getOffset();

            if ( firstFoundTextY!=-1 && maxHeight>0 ) {
                ldomXPointer p( _start.getNode(), offs );
                int currentTextY = p.toPoint().y;
                if ( currentTextY<firstFoundTextY-maxHeight )
                    return words.length()>0;
            }

            if ( caseInsensitive )
                txt.lowercase();

            while ( ::findTextRev( txt, offs, pattern ) ) {
                if ( !words.length() && maxHeight>0 ) {
                    ldomXPointer p( _end.getNode(), offs );
                    firstFoundTextY = p.toPoint().y;
                }
                words.add( ldomWord(_end.getNode(), offs, offs + pattern.length() ) );
                offs--;
            }
            if ( !_end.prevVisibleText() )
                break;
            txt = _end.getNode()->getText();
            _end.setOffset(txt.length());
            if ( words.length() >= maxCount )
                break;
        }
    } else {
        // direct search
        if ( !_start.isText() )
            _start.nextVisibleText();
        int firstFoundTextY = -1;
        if (checkMaxFromStart) {
			ldomXPointer p( _start.getNode(), _start.getOffset() );
			firstFoundTextY = p.toPoint().y;
		}
        while ( !isNull() ) {
            int offs = _start.getOffset();

            if ( firstFoundTextY!=-1 && maxHeight>0 ) {
                ldomXPointer p( _start.getNode(), offs );
                int currentTextY = p.toPoint().y;
                if ( (checkMaxFromStart && currentTextY>=firstFoundTextY+maxHeight) ||
					currentTextY>firstFoundTextY+maxHeight )
                    return words.length()>0;
            }

            lString16 txt = _start.getNode()->getText();
            if ( caseInsensitive )
                txt.lowercase();

            while ( ::findText( txt, offs, pattern ) ) {
                if ( !words.length() && maxHeight>0 ) {
                    ldomXPointer p( _start.getNode(), offs );
                    int currentTextY = p.toPoint().y;
                    if (checkMaxFromStart) {
						if ( currentTextY>=firstFoundTextY+maxHeight )
							return words.length()>0;
					} else
						firstFoundTextY = currentTextY;
                }
                words.add( ldomWord(_start.getNode(), offs, offs + pattern.length() ) );
                offs++;
            }
            if ( !_start.nextVisibleText() )
                break;
            if ( words.length() >= maxCount )
                break;
        }
    }
    return words.length() > 0;
}

/// fill marked ranges list
void ldomXRangeList::getRanges( ldomMarkedRangeList &dst )
{
    dst.clear();
    if ( empty() )
        return;
    for ( int i=0; i<length(); i++ ) {
        ldomXRange * range = get(i);
        lvPoint ptStart = range->getStart().toPoint();
        lvPoint ptEnd = range->getEnd().toPoint();
#if 0
        CRLog::trace("selectRange( %d,%d : %d,%d : %s, %s )",
                     ptStart.x,
                     ptStart.y,
                     ptEnd.x,
                     ptEnd.y,
                     LCSTR(range->getStart().toString()),
                     LCSTR(range->getEnd().toString()) );
#endif
        ldomMarkedRange * item = new ldomMarkedRange( ptStart, ptEnd, range->getFlags() );
        if ( !item->empty() )
            dst.add( item );
        else
            delete item;
    }
}

/// fill text selection list by splitting text into monotonic flags ranges
void ldomXRangeList::splitText( ldomMarkedTextList &dst, ldomNode * textNodeToSplit )
{
    lString16 text = textNodeToSplit->getText();
    if ( length()==0 ) {
        dst.add( new ldomMarkedText( text, 0, 0 ) );
        return;
    }
    ldomXRange textRange( textNodeToSplit );
    ldomXRangeList ranges;
    ranges.add( new ldomXRange(textRange) );
    int i;
    for ( i=0; i<length(); i++ ) {
        ranges.split( get(i) );
    }
    for ( i=0; i<ranges.length(); i++ ) {
        ldomXRange * r = ranges[i];
        int start = r->getStart().getOffset();
        int end = r->getEnd().getOffset();
        if ( end>start )
            dst.add( new ldomMarkedText( text.substr(start, end-start), r->getFlags(), start ) );
    }
    /*
    if (dst.length()) {
        CRLog::trace(" splitted: ");
        for (int k = 0; k < dst.length(); k++) {
            CRLog::trace("    (%d, %d) %s",
                    dst[k]->offset,
                    dst[k]->flags,
                    UnicodeToUtf8(dst[k]->text).c_str());
        }
    }
    */
}

/// returns rectangle (in doc coordinates) for range. Returns true if found.
bool ldomXRange::getRect(lvRect& rect)
{
    if (isNull()) {
        return false;
    }
    // get start and end rects
    lvRect rc1;
    lvRect rc2;
    if (!getStart().getRect(rc1) || !getEnd().getRect(rc2)) {
        return false;
    }
    if (rc1.top == rc2.top && rc1.bottom == rc2.bottom) {
        // on same line
        rect.left = rc1.left;
        rect.top = rc1.top;
        rect.right = rc2.right;
        rect.bottom = rc2.bottom;
        return !rect.isEmpty();
    }
    // on different lines
    ldomNode* parent = getNearestCommonParent();
    if (!parent) {
        return false;
    }
    parent->getAbsRect(rect);
    rect.top = rc1.top;
    rect.bottom = rc2.bottom;
    rect.left = rc1.left < rc2.left ? rc1.left : rc2.left;
    rect.right = rc1.right > rc2.right ? rc1.right : rc2.right;
    return !rect.isEmpty();
}

/// sets range to nearest word bounds, returns true if success
bool ldomXRange::getWordRange( ldomXRange & range, ldomXPointer & p )
{
    ldomNode * node = p.getNode();
    if ( !node->isText() )
        return false;
    int pos = p.getOffset();
    lString16 txt = node->getText();
    if ( pos<0 )
        pos = 0;
    if ( pos>(int)txt.length() )
        pos = txt.length();
    int endpos = pos;
    for (;;) {
        lChar16 ch = txt[endpos];
        if ( ch==0 || ch==' ' )
            break;
        endpos++;
    }
    /*
    // include trailing space
    for (;;) {
        lChar16 ch = txt[endpos];
        if ( ch==0 || ch!=' ' )
            break;
        endpos++;
    }
    */
    for ( ;; ) {
        if ( pos==0 )
            break;
        if ( txt[pos]!=' ' )
            break;
        pos--;
    }
    for ( ;; ) {
        if ( pos==0 )
            break;
        if ( txt[pos-1]==' ' )
            break;
        pos--;
    }
    ldomXRange r( ldomXPointer( node, pos ), ldomXPointer( node, endpos ) );
    range = r;
    return true;
}

/// returns true if intersects specified line rectangle
bool ldomMarkedRange::intersects( lvRect & rc, lvRect & intersection )
{
    if ( start.y>=rc.bottom )
        return false;
    if ( end.y<rc.top )
        return false;
    intersection = rc;
    if ( start.y>=rc.top && start.y<rc.bottom ) {
        if ( start.x > rc.right )
            return false;
        intersection.left = rc.left > start.x ? rc.left : start.x;
    }
    if ( end.y>=rc.top && end.y<rc.bottom ) {
        if ( end.x < rc.left )
            return false;
        intersection.right = rc.right < end.x ? rc.right : end.x;
    }
    return true;
}

/// create bounded by RC list, with (0,0) coordinates at left top corner
ldomMarkedRangeList::ldomMarkedRangeList( const ldomMarkedRangeList * list, lvRect & rc )
{
    if ( !list || list->empty() )
        return;
//    if ( list->get(0)->start.y>rc.bottom )
//        return;
//    if ( list->get( list->length()-1 )->end.y < rc.top )
//        return;
    for ( int i=0; i<list->length(); i++ ) {
        ldomMarkedRange * src = list->get(i);
        if ( src->start.y>=rc.bottom || src->end.y<rc.top )
            continue;
        add( new ldomMarkedRange(
            lvPoint(src->start.x-rc.left, src->start.y-rc.top ),
            lvPoint(src->end.x-rc.left, src->end.y-rc.top ),
            src->flags ) );
    }
}

/// returns nearest common element for start and end points
ldomNode * ldomXRange::getNearestCommonParent()
{
    ldomXPointerEx start(getStart());
    ldomXPointerEx end(getEnd());
    while ( start.getLevel() > end.getLevel() && start.parent() )
        ;
    while ( start.getLevel() < end.getLevel() && end.parent() )
        ;
    while ( start.getIndex()!=end.getIndex() && start.parent() && end.parent() )
        ;
    if ( start.getNode()==end.getNode() )
        return start.getNode();
    return NULL;
}

/// searches path for element with specific id,
/// returns level at which element is founs, 0 if not found
int ldomXPointerEx::findElementInPath( lUInt16 id )
{
    if ( !ensureElement() )
        return 0;
    ldomNode * e = getNode();
    for ( ; e!=NULL; e = e->getParentNode() ) {
        if ( e->getNodeId()==id ) {
            return e->getNodeLevel();
        }
    }
    return 0;
}

bool ldomXPointerEx::ensureFinal()
{
    if ( !ensureElement() )
        return false;
    int cnt = 0;
    int foundCnt = -1;
    ldomNode * e = getNode();
    for ( ; e!=NULL; e = e->getParentNode() ) {
        if ( e->getRendMethod() == erm_final ) {
            foundCnt = cnt;
        }
        cnt++;
    }
    if ( foundCnt<0 )
        return false;
    for ( int i=0; i<foundCnt; i++ )
        parent();
    // curent node is final formatted element (e.g. paragraph)
    return true;
}

/// ensure that current node is element (move to parent, if not - from text node to element)
bool ldomXPointerEx::ensureElement()
{
    ldomNode * node = getNode();
    if ( !node )
        return false;
    if ( node->isText()) {
        if (!parent())
            return false;
        node = getNode();
    }
    if ( !node || !node->isElement() )
        return false;
    return true;
}

/// move to first child of current node
bool ldomXPointerEx::firstChild()
{
    return child(0);
}

/// move to last child of current node
bool ldomXPointerEx::lastChild()
{
    int count = getNode()->getChildCount();
    if ( count <=0 )
        return false;
    return child( count - 1 );
}

/// move to first element child of current node
bool ldomXPointerEx::firstElementChild()
{
    ldomNode * node = getNode();
    int count = node->getChildCount();
    for ( int i=0; i<count; i++ ) {
        if ( node->getChildNode( i )->isElement() )
            return child( i );
    }
    return false;
}

/// move to last element child of current node
bool ldomXPointerEx::lastElementChild()
{
    ldomNode * node = getNode();
    int count = node->getChildCount();
    for ( int i=count-1; i>=0; i-- ) {
        if ( node->getChildNode( i )->isElement() )
            return child( i );
    }
    return false;
}

/// forward iteration by elements of DOM three
bool ldomXPointerEx::nextElement()
{
    if ( !ensureElement() )
        return false;
    if ( firstElementChild() )
        return true;
    for (;;) {
        if ( nextSiblingElement() )
            return true;
        if ( !parent() )
            return false;
    }
}

/// returns true if current node is visible element with render method == erm_final
bool ldomXPointerEx::isVisibleFinal()
{
    if ( !isElement() )
        return false;
    int cnt = 0;
    int foundCnt = -1;
    ldomNode * e = getNode();
    for ( ; e!=NULL; e = e->getParentNode() ) {
        switch ( e->getRendMethod() ) {
        case erm_final:
            foundCnt = cnt;
            break;
        case erm_invisible:
            foundCnt = -1;
            break;
        default:
            break;
        }
        cnt++;
    }
    if ( foundCnt != 0 )
        return false;
    // curent node is visible final formatted element (e.g. paragraph)
    return true;
}

/// move to next visible text node
bool ldomXPointerEx::nextVisibleText( bool thisBlockOnly )
{
    ldomXPointerEx backup;
    if ( thisBlockOnly )
        backup = *this;
    while ( nextText(thisBlockOnly) ) {
        if ( isVisible() )
            return true;
    }
    if ( thisBlockOnly )
        *this = backup;
    return false;
}

/// returns true if current node is visible element or text
bool ldomXPointerEx::isVisible()
{
    ldomNode * p;
    ldomNode * node = getNode();
    if ( node && node->isText() )
        p = node->getParentNode();
    else
        p = node;
    while ( p ) {
        if ( p->getRendMethod() == erm_invisible )
            return false;
        p = p->getParentNode();
    }
    return true;
}

/// move to next text node
bool ldomXPointerEx::nextText( bool thisBlockOnly )
{
    ldomNode * block = NULL;
    if ( thisBlockOnly )
        block = getThisBlockNode();
    setOffset( 0 );
    while ( firstChild() ) {
        if ( isText() )
            return (!thisBlockOnly || getThisBlockNode()==block);
    }
    for (;;) {
        while ( nextSibling() ) {
            if ( isText() )
                return (!thisBlockOnly || getThisBlockNode()==block);
            while ( firstChild() ) {
                if ( isText() )
                    return (!thisBlockOnly || getThisBlockNode()==block);
            }
        }
        if ( !parent() )
            return false;
    }
}

/// move to previous text node
bool ldomXPointerEx::prevText( bool thisBlockOnly )
{
    ldomNode * block = NULL;
    if ( thisBlockOnly )
        block = getThisBlockNode();
    setOffset( 0 );
    for (;;) {
        while ( prevSibling() ) {
            if ( isText() )
                return  (!thisBlockOnly || getThisBlockNode()==block);
            while ( lastChild() ) {
                if ( isText() )
                    return (!thisBlockOnly || getThisBlockNode()==block);
            }
        }
        if ( !parent() )
            return false;
    }
}

/// move to previous visible text node
bool ldomXPointerEx::prevVisibleText( bool thisBlockOnly )
{
    ldomXPointerEx backup;
    if ( thisBlockOnly )
        backup = *this;
    while ( prevText( thisBlockOnly ) )
        if ( isVisible() )
            return true;
    if ( thisBlockOnly )
        *this = backup;
    return false;
}

// TODO: implement better behavior
inline bool IsUnicodeSpace( lChar16 ch )
{
    return ch==' ';
}

// TODO: implement better behavior
inline bool IsUnicodeSpaceOrNull( lChar16 ch )
{
    return ch==0 || ch==' ';
}

inline bool canWrapWordBefore( lChar16 ch ) {
    return ch>=0x2e80 && ch<0xa640;
}

inline bool canWrapWordAfter( lChar16 ch ) {
    return ch>=0x2e80 && ch<0xa640;
}

/// move to previous visible word beginning
bool ldomXPointerEx::prevVisibleWordStart( bool thisBlockOnly )
{
    if ( isNull() )
        return false;
    ldomNode * node = NULL;
    lString16 text;
    int textLen = 0;
    for ( ;; ) {
        if ( !isText() || !isVisible() || _data->getOffset()==0 ) {
            // move to previous text
            if ( !prevVisibleText(thisBlockOnly) )
                return false;
            node = getNode();
            text = node->getText();
            textLen = text.length();
            _data->setOffset( textLen );
        } else {
            node = getNode();
            text = node->getText();
            textLen = text.length();
        }
        bool foundNonSpace = false;
        while ( _data->getOffset() > 0 && IsUnicodeSpace(text[_data->getOffset()-1]) )
            _data->addOffset(-1);
        while ( _data->getOffset()>0 ) {
            if ( IsUnicodeSpace(text[ _data->getOffset()-1 ]) )
                break;
            foundNonSpace = true;
            _data->addOffset(-1);
        }
        if ( foundNonSpace )
            return true;
    }
}

/// move to previous visible word end
bool ldomXPointerEx::prevVisibleWordEnd( bool thisBlockOnly )
{
    if ( isNull() )
        return false;
    ldomNode * node = NULL;
    lString16 text;
    int textLen = 0;
    bool moved = false;
    for ( ;; ) {
        if ( !isText() || !isVisible() || _data->getOffset()==0 ) {
            // move to previous text
            if ( !prevVisibleText(thisBlockOnly) )
                return false;
            node = getNode();
            text = node->getText();
            textLen = text.length();
            _data->setOffset( textLen );
            moved = true;
        } else {
            node = getNode();
            text = node->getText();
            textLen = text.length();
        }
        // skip spaces
        while ( _data->getOffset() > 0 && IsUnicodeSpace(text[_data->getOffset()-1]) ) {
            _data->addOffset(-1);
            moved = true;
        }
        if ( moved && _data->getOffset()>0 )
            return true; // found!
        // skip non-spaces
        while ( _data->getOffset()>0 ) {
            if ( IsUnicodeSpace(text[ _data->getOffset()-1 ]) )
                break;
            _data->addOffset(-1);
        }
        // skip spaces
        while ( _data->getOffset() > 0 && IsUnicodeSpace(text[_data->getOffset()-1]) ) {
            _data->addOffset(-1);
            moved = true;
        }
        if ( moved && _data->getOffset()>0 )
            return true; // found!
    }
}

/// move to next visible word beginning
bool ldomXPointerEx::nextVisibleWordStart( bool thisBlockOnly )
{
    if ( isNull() )
        return false;
    ldomNode * node = NULL;
    lString16 text;
    int textLen = 0;
    bool moved = false;
    for ( ;; ) {
        if ( !isText() || !isVisible() ) {
            // move to previous text
            if ( !nextVisibleText(thisBlockOnly) )
                return false;
            node = getNode();
            text = node->getText();
            textLen = text.length();
            _data->setOffset( 0 );
            moved = true;
        } else {
            for (;;) {
                node = getNode();
                text = node->getText();
                textLen = text.length();
                if ( _data->getOffset() < textLen )
                    break;
                if ( !nextVisibleText(thisBlockOnly) )
                    return false;
                _data->setOffset( 0 );
            }
        }
        // skip spaces
        while ( _data->getOffset()<textLen && IsUnicodeSpace(text[ _data->getOffset() ]) ) {
            _data->addOffset(1);
            moved = true;
        }
        if ( moved && _data->getOffset()<textLen )
            return true;
        // skip non-spaces
        while ( _data->getOffset()<textLen ) {
            if ( IsUnicodeSpace(text[ _data->getOffset() ]) )
                break;
            moved = true;
            _data->addOffset(1);
        }
        // skip spaces
        while ( _data->getOffset()<textLen && IsUnicodeSpace(text[ _data->getOffset() ]) ) {
            _data->addOffset(1);
            moved = true;
        }
        if ( moved && _data->getOffset()<textLen )
            return true;
    }
}

/// move to end of current word
bool ldomXPointerEx::thisVisibleWordEnd(bool thisBlockOnly)
{
    CR_UNUSED(thisBlockOnly);
    if ( isNull() )
        return false;
    ldomNode * node = NULL;
    lString16 text;
    int textLen = 0;
    bool moved = false;
    if ( !isText() || !isVisible() )
        return false;
    node = getNode();
    text = node->getText();
    textLen = text.length();
    if ( _data->getOffset() >= textLen )
        return false;
    // skip spaces
    while ( _data->getOffset()<textLen && IsUnicodeSpace(text[ _data->getOffset() ]) ) {
        _data->addOffset(1);
        //moved = true;
    }
    // skip non-spaces
    while ( _data->getOffset()<textLen ) {
        if ( IsUnicodeSpace(text[ _data->getOffset() ]) )
            break;
        moved = true;
        _data->addOffset(1);
    }
    return moved;
}

/// move to next visible word end
bool ldomXPointerEx::nextVisibleWordEnd( bool thisBlockOnly )
{
    if ( isNull() )
        return false;
    ldomNode * node = NULL;
    lString16 text;
    int textLen = 0;
    //bool moved = false;
    for ( ;; ) {
        if ( !isText() || !isVisible() ) {
            // move to previous text
            if ( !nextVisibleText(thisBlockOnly) )
                return false;
            node = getNode();
            text = node->getText();
            textLen = text.length();
            _data->setOffset( 0 );
            //moved = true;
        } else {
            for (;;) {
                node = getNode();
                text = node->getText();
                textLen = text.length();
                if ( _data->getOffset() < textLen )
                    break;
                if ( !nextVisibleText(thisBlockOnly) )
                    return false;
                _data->setOffset( 0 );
            }
        }
        bool nonSpaceFound = false;
        // skip non-spaces
        while ( _data->getOffset()<textLen ) {
            if ( IsUnicodeSpace(text[ _data->getOffset() ]) )
                break;
            nonSpaceFound = true;
            _data->addOffset(1);
        }
        if ( nonSpaceFound )
            return true;
        // skip spaces
        while ( _data->getOffset()<textLen && IsUnicodeSpace(text[ _data->getOffset() ]) ) {
            _data->addOffset(1);
            //moved = true;
        }
        // skip non-spaces
        while ( _data->getOffset()<textLen ) {
            if ( IsUnicodeSpace(text[ _data->getOffset() ]) )
                break;
            nonSpaceFound = true;
            _data->addOffset(1);
        }
        if ( nonSpaceFound )
            return true;
    }
}

/// returns true if current position is visible word beginning
bool ldomXPointerEx::isVisibleWordStart()
{
   if ( isNull() )
        return false;
    if ( !isText() || !isVisible() )
        return false;
    ldomNode * node = getNode();
    lString16 text = node->getText();
    int textLen = text.length();
    int i = _data->getOffset();
    lChar16 currCh = i<textLen ? text[i] : 0;
    lChar16 prevCh = i<textLen && i>0 ? text[i-1] : 0;
    if (canWrapWordBefore(currCh) || (IsUnicodeSpaceOrNull(prevCh) && !IsUnicodeSpace(currCh)))
        return true;
    return false;
 }

/// returns true if current position is visible word end
bool ldomXPointerEx::isVisibleWordEnd()
{
    if ( isNull() )
        return false;
    if ( !isText() || !isVisible() )
        return false;
    ldomNode * node = getNode();
    lString16 text = node->getText();
    int textLen = text.length();
    int i = _data->getOffset();
    lChar16 currCh = i>0 ? text[i-1] : 0;
    lChar16 nextCh = i<textLen ? text[i] : 0;
    if (canWrapWordAfter(currCh) || (!IsUnicodeSpace(currCh) && IsUnicodeSpaceOrNull(nextCh)))
        return true;
    return false;
}

/// returns block owner node of current node (or current node if it's block)
ldomNode * ldomXPointerEx::getThisBlockNode()
{
    if ( isNull() )
        return NULL;
    ldomNode * node = getNode();
    if ( node->isText() )
        node = node->getParentNode();
    for (;;) {
        if ( !node )
            return NULL;
        lvdom_element_render_method rm = node->getRendMethod();
        switch ( rm ) {
        case erm_runin: // treat as separate block
        case erm_block:
        case erm_final:
        case erm_mixed:
        case erm_list_item:
        case erm_table:
        case erm_table_row_group:
        case erm_table_row:
        case erm_table_caption:
            return node;
        default:
            break; // ignore
        }
        node = node->getParentNode();
    }
}

/// returns true if points to last visible text inside block element
bool ldomXPointerEx::isLastVisibleTextInBlock()
{
    if ( !isText() )
        return false;
    ldomXPointerEx pos(*this);
    return !pos.nextVisibleText(true);
}

/// returns true if points to first visible text inside block element
bool ldomXPointerEx::isFirstVisibleTextInBlock()
{
    if ( !isText() )
        return false;
    ldomXPointerEx pos(*this);
    return !pos.prevVisibleText(true);
}

// sentence navigation

/// returns true if points to beginning of sentence
bool ldomXPointerEx::isSentenceStart()
{
    if ( isNull() )
        return false;
    if ( !isText() || !isVisible() )
        return false;
    ldomNode * node = getNode();
    lString16 text = node->getText();
    int textLen = text.length();
    int i = _data->getOffset();
    lChar16 currCh = i<textLen ? text[i] : 0;
    lChar16 prevCh = i>0 ? text[i-1] : 0;
    lChar16 prevNonSpace = 0;
    for ( ;i>0; i-- ) {
        lChar16 ch = text[i-1];
        if ( !IsUnicodeSpace(ch) ) {
            prevNonSpace = ch;
            break;
        }
    }
    if ( !prevNonSpace ) {
        ldomXPointerEx pos(*this);
        while ( !prevNonSpace && pos.prevVisibleText(true) ) {
            lString16 prevText = pos.getText();
            for ( int j=prevText.length()-1; j>=0; j-- ) {
                lChar16 ch = prevText[j];
                if ( !IsUnicodeSpace(ch) ) {
                    prevNonSpace = ch;
                    break;
                }
            }
        }
    }

    if ( !IsUnicodeSpace(currCh) && IsUnicodeSpaceOrNull(prevCh) ) {
        switch (prevNonSpace) {
        case 0:
        case '.':
        case '?':
        case '!':
        case L'\x2026': // horizontal ellypsis
            return true;
        default:
            return false;
        }
    }
    return false;
}

/// returns true if points to end of sentence
bool ldomXPointerEx::isSentenceEnd()
{
    if ( isNull() )
        return false;
    if ( !isText() || !isVisible() )
        return false;
    ldomNode * node = getNode();
    lString16 text = node->getText();
    int textLen = text.length();
    int i = _data->getOffset();
    lChar16 currCh = i<textLen ? text[i] : 0;
    lChar16 prevCh = i>0 ? text[i-1] : 0;
    if ( IsUnicodeSpaceOrNull(currCh) ) {
        switch (prevCh) {
        case 0:
        case '.':
        case '?':
        case '!':
        case L'\x2026': // horizontal ellypsis
            return true;
        default:
            break;
        }
    }
    // word is not ended with . ! ?
    // check whether it's last word of block
    ldomXPointerEx pos(*this);
    //return !pos.nextVisibleWordStart(true);
    return !pos.thisVisibleWordEnd(true);
}

/// move to beginning of current visible text sentence
bool ldomXPointerEx::thisSentenceStart()
{
    if ( isNull() )
        return false;
    if ( !isText() && !nextVisibleText() && !prevVisibleText() )
        return false;
    for (;;) {
        if ( isSentenceStart() )
            return true;
        if ( !prevVisibleWordStart(true) )
            return false;
    }
}

/// move to end of current visible text sentence
bool ldomXPointerEx::thisSentenceEnd()
{
    if ( isNull() )
        return false;
    if ( !isText() && !nextVisibleText() && !prevVisibleText() )
        return false;
    for (;;) {
        if ( isSentenceEnd() )
            return true;
        if ( !nextVisibleWordEnd(true) )
            return false;
    }
}

/// move to beginning of next visible text sentence
bool ldomXPointerEx::nextSentenceStart()
{
    if ( !isSentenceStart() && !thisSentenceEnd() )
        return false;
    for (;;) {
        if ( !nextVisibleWordStart() )
            return false;
        if ( isSentenceStart() )
            return true;
    }
}

/// move to beginning of prev visible text sentence
bool ldomXPointerEx::prevSentenceStart()
{
    if ( !thisSentenceStart() )
        return false;
    for (;;) {
        if ( !prevVisibleWordStart() )
            return false;
        if ( isSentenceStart() )
            return true;
    }
}

/// move to end of next visible text sentence
bool ldomXPointerEx::nextSentenceEnd()
{
    if ( !nextSentenceStart() )
        return false;
    return thisSentenceEnd();
}

/// move to end of next visible text sentence
bool ldomXPointerEx::prevSentenceEnd()
{
    if ( !thisSentenceStart() )
        return false;
    for (;;) {
        if ( !prevVisibleWordEnd() )
            return false;
        if ( isSentenceEnd() )
            return true;
    }
}

/// if start is after end, swap start and end
void ldomXRange::sort()
{
    if ( _start.isNull() || _end.isNull() )
        return;
    if ( _start.compare(_end) > 0 ) {
        ldomXPointer p1( _start );
        ldomXPointer p2( _end );
        _start = p2;
        _end = p1;
    }
}

/// backward iteration by elements of DOM three
bool ldomXPointerEx::prevElement()
{
    if ( !ensureElement() )
        return false;
    for (;;) {
        if ( prevSiblingElement() ) {
            while ( lastElementChild() )
                ;
            return true;
        }
        if ( !parent() )
            return false;
        return true;
    }
}

/// move to next final visible node (~paragraph)
bool ldomXPointerEx::nextVisibleFinal()
{
    for ( ;; ) {
        if ( !nextElement() )
            return false;
        if ( isVisibleFinal() )
            return true;
    }
}

/// move to previous final visible node (~paragraph)
bool ldomXPointerEx::prevVisibleFinal()
{
    for ( ;; ) {
        if ( !prevElement() )
            return false;
        if ( isVisibleFinal() )
            return true;
    }
}

ldomNode* ldomXRange::getEndNode()
{
    if (_endnode == NULL)
    {
        _endnode = this->_end.getNode();
    }
    return _endnode;
}

ldomNode* ldomXRange::getStartNode()
{
    if (_startnode == NULL)
    {
        _startnode = this->_start.getNode();
    }
    return _startnode;
}

ldomNode* ldomXRange::getAncestor(ldomNode* n1, ldomNode* n2)
{
    typedef std::map< int, ldomNode*> nodemap;
    nodemap m;

    ldomNode * a;
    ldomNode * b;
    ldomNode * p;

    if (n1 == NULL || n2 == NULL)
    {
        return NULL;
    }
    ldomNode *p1 = n1->getParentNode();
    ldomNode *p2 = n2->getParentNode();

    if (p1 == NULL || p2 == NULL) return NULL;
    if (n1 == n2) return p1;
    if (n1 == p2) return p1;
    if (n2 == p1) return p2;

    if(n1->getNodeLevel() > n2->getNodeLevel())
    {
        a = n1; b = n2;
    }
    else
    {
        a = n2; b = n1;
    }

    p = a;
    while (p->getParentNode() != NULL)
    {
        p = p->getParentNode();
        int key = p->getNodeLevel();
        m[key]= p;
    }

    p = b;
    while (p->getParentNode() != NULL)
    {
        p = p->getParentNode();
        int key = p->getNodeLevel();
        if (m.find(key)!= m.end())
        {
            if(p == m.at(key))
            {
                return p;
            }
        }
    }
    return NULL;
}


/// run callback for each node in range
void ldomXRange::forEach2(ldomNodeCallback *callback)
{
    if ( _start.isNull() || _end.isNull() )
    {
        return;
    }

    ldomNode *start = getStartNode();
    ldomNode *end   = getEndNode();
    ldomNode *common_parent   = getAncestor(start,end);
    int level = (common_parent != NULL) ? common_parent->getNodeLevel() : 1;

    //CRLog::error("start  path = %s",LCSTR(start->getXPath()));
    //CRLog::error("end    path = %s",LCSTR(end->getXPath()));
    //CRLog::error("parent path = %s",LCSTR(common_parent->getXPath()));
    //CRLog::error("parent lvl  = %d",common_parent->getNodeLevel());

    ldomNode *node = start;
    ldomNode *parent = node->getParentNode();

    int index = node->getNodeIndex();

    bool endfound = false;
    while (parent->getNodeLevel() >= level)
    {
        for (int i = index; i < parent->getChildCount(); i++)
        {
            node = parent->getChildNode(i);

            if (node->isText())
            {
                //CRLog::error("TEXT nodepath = %s",LCSTR(node->getXPath()));
                callback->processText(node, this);
            }
            else
            {
                //CRLog::error("ELEMENT nodepath = %s",LCSTR(node->getXPath()));
                if (callback->onElement(node))
                {
                    endfound = callback->processElement(node, this);
                }
            }

            endfound = endfound || (node == this->getEndNode());
            if (endfound)
            {
                //CRLog::error("FOREACH FINISH nodepath = %s",LCSTR(node->getXPath()));
                return;
            }
        }
        index = parent->getNodeIndex() + 1;
        parent = parent->getParentNode();
        //CRLog::error("parentpath = %s",LCSTR(parent->getXPath()));
        if (parent == end)
        {
            //CRLog::error("PARENT EXIT");
            return;
        }
        if (parent->getNodeLevel() == level)
        {
            //CRLog::error("WE'RE IN COMMON PARENT!!!!!");
        }
    }
}

void ldomNodeCallback::processText(ldomNode* node, ldomXRange * range)
{
    ldomXRange noderange = ldomXRange(node);

    if (node == range->getEndNode())
    {
        noderange.setEnd(range->getEnd());
    }
    if(node == range->getStartNode() )
    {
        noderange.setStart(range->getStart());
    }
    this->onText(&noderange);
    /*
    lString16 text = node->getText().substr(noderange.getStart().getOffset(),noderange.getEnd().getOffset());
    CRLog::error("full  text = %s",LCSTR(node->getText()));
    CRLog::error("substrtext = %s",LCSTR(text));
    */
}

bool ldomNodeCallback::processElement(ldomNode* node, ldomXRange * range)
{
    for (lUInt32 i = 0; i < node->getChildCount(); i++)
    {
        ldomNode *child = node->getChildNode(i);

        if (child->isText())
        {
            //CRLog::error("_______TEXT nodepath = %s",LCSTR(child->getXPath()));
            processText(child,range);
        }
        else
        {
            //CRLog::error("_______ELEMENT nodepath = %s",LCSTR(child->getXPath()));

            if (this->onElement(child))
            {
                if(processElement(child, range))
                {
                    return true;
                }
            }
        }

        if (child == range->getEndNode())
        {
            return true;
        }
    }
    return false;
}

/// get all words from specified range
void ldomXRange::getRangeWords(LVArray<ldomWord>& words_list) {
    class WordsCollector : public ldomNodeCallback {
        LVArray<ldomWord>& list_;
    public:
        WordsCollector(LVArray<ldomWord>& list) : list_(list) {}
        /// called for each found text fragment in range
        virtual void onText(ldomXRange* nodeRange) {
            ldomNode* node = nodeRange->getStart().getNode();
            lString16 text = node->getText();
            int len = text.length();
            int end = nodeRange->getEnd().getOffset();
            if (len > end) {
                len = end;
            }
            int beginOfWord = -1;
            int TRFLAGS = 0 ;
            TRFLAGS |= CH_PROP_ALPHA;
            TRFLAGS |= CH_PROP_DIGIT;
            TRFLAGS |= CH_PROP_PUNCT;
            TRFLAGS |= CH_PROP_HYPHEN;
            TRFLAGS |= CH_PROP_VOWEL;
            TRFLAGS |= CH_PROP_CONSONANT;
            TRFLAGS |= CH_PROP_SIGN;
            TRFLAGS |= CH_PROP_ALPHA_SIGN;
            TRFLAGS |= CH_PROP_DASH;

            for (int i = nodeRange->getStart().getOffset(); i <= len; i++) {
                int alpha = lGetCharProps(text[i]) & TRFLAGS; //  words check here
                if (alpha && beginOfWord < 0) {
                    beginOfWord = i;
                }
                if (!alpha && beginOfWord >= 0) {
                    list_.add(ldomWord(node, beginOfWord, i));
                    beginOfWord = -1;
                }
            }
        }
        /// called for each found node in range
        virtual bool onElement(ldomXPointerEx* ptr) {
            ldomNode* elem = ptr->getNode();
            if (elem->getRendMethod() == erm_invisible) {
                return false;
            }
            return true;
        }
    };
    WordsCollector collector(words_list);
    forEach2(&collector);
}

/// get all words from specified range
void ldomXRange::getRangeChars(LVArray<TextRect>& words_list,int clip_width, bool rtl_enable, bool rtl_space ) {
    class WordsCollector : public ldomNodeCallback {
    public:
        LVArray<TextRect> list_;
        bool contains_rtl = false;
        RectHelper* rectHelper_;

        WordsCollector()  { rectHelper_ = new RectHelper();}

        ~WordsCollector() { delete rectHelper_; }

        bool AllowTextNodeShift(ldomNode* node)
        {
            ldomNode* parentnode = node->getParentNode();
            css_style_rec_t *style = parentnode->getStyle().get();
            if (style == nullptr)
            {
                return false;
            }
            int whitespace = style->white_space;
            if (style->white_space != css_ws_normal)
            {
                return false;
            }
            while (parentnode!=NULL)
            {
                if (parentnode == NULL)
                {
                    return false;
                }
                lString16 name = parentnode->getNodeName();
                if(name == "a")
                {
                    return false;
                }
                int index = node->getNodeIndex();
                if(name == "style" && index !=0)
                {
                    return false;
                }
                if(name == "style" && index ==0)
                {
                    return true;
                }
                if(name == "body" || name == "section")
                {
                    return false;
                }
                if(name == "font")
                {
                    return false;
                }
                if(name == "span")
                {
                    return false;
                }
                if(name == "p" && index == 0)
                {
                    return true;
                }
                if(name == "p" && index == 1 )
                {
                    if(parentnode->getChildNode(0)->getText().empty()) return true;
                }
                if(name == "blockquote" && index == 0)
                {
                    return true;
                }
                node = parentnode;
                parentnode = parentnode->getParentNode();
            }
            return false;
        };
        /// called for each found text fragment in range
        virtual void onText(ldomXRange* nodeRange) {
            ldomNode* node = nodeRange->getStart().getNode();
            bool flag_in_ruby = false;
            if(!CheckNodeInvisibility(node))
            {
                return;
            }
            if (gDocumentRTL==1 && node->isRTL())
            {
                this->contains_rtl = true;
            }
            ldomNode * parent_node = node->getParentNode();

            if(parent_node == nullptr)
            {
                return;
            }
            if(parent_node->getNodeName() == "ruby" )
            {
                flag_in_ruby = true;
            }
            lString16 rubytext;
            if(flag_in_ruby)
            {
                ldomNode *ruby = node->getParentNode("ruby");
                if (ruby != NULL)
                {
                    for (int i = 0; i < ruby->getChildCount(); i++)
                    {
                        ldomNode *child = ruby->getChildNode(i);
                        if (child->getNodeName() == "rt")
                        {
                            rubytext.append(L"\u0083"); //for selection not to stop on left bracket
                            rubytext.append("(" + child->getText() + ")");
                        }
                    }
                }
            }
            lString16 text = node->getText();
            //CRLog::error("node [%s]",LCSTR(node->getXPath()));
            //CRLog::error("Text[%s]",LCSTR(text));
            //CRLog::error("Textlen = %d",text.length());
            rectHelper_->Init(nodeRange);
            //CRLog::debug("==========================================");


            int pos = nodeRange->getStart().getOffset();
            const int start = pos;
            int len = text.length();
            int end = nodeRange->getEnd().getOffset();
            if (len > end)
            {
                len = end;
            }

            int leftshift = 0;
            int shift =0;

            for (; pos < len; pos++)
            {
                ldomWord word = ldomWord(node, pos, pos + 1);
                lvRect rect = rectHelper_->getRect(word);
                lString16 string = word.getText();
                if (text[pos] == ' ' || text[pos] == '\t' || string == L"\u200B" )
                {
                    leftshift = leftshift + (rect.right - rect.left);
                    continue;
                }
                break;
            }
            rectHelper_->ResetLineIndex(); // offsetting lineIndex back to the beginning of node
            if (leftshift > 0 && AllowTextNodeShift(node) && pos == 0)
            {
                // alone_space_in_node shift prohibition
                if ( pos == len && pos == 1 && (text == " " || text == "\t") )
                {
                    leftshift = 0;
                }

                for (; pos < len - 1; pos++)
                {
                    ldomWord word = ldomWord(node, pos, pos + 1);
                    lvRect rect = rectHelper_->getRect(word);
                    lString16 string = word.getText();
                    rect.left = rect.left - leftshift + gTextLeftShift;
                    rect.right = rect.right - leftshift + gTextLeftShift -1;
                    if(flag_in_ruby && pos == start)
                    {
                        rect.left -= parent_node->getStyle()->padding[0].value;
                    }
                    list_.add(TextRect(node, rect, string, word));
                }
                //last char zero width fix
                ldomWord word = ldomWord(node, len - 1, len);
                lvRect rect = rectHelper_->getRect(word);
                lString16 string = word.getText();
                rect.left = rect.left - leftshift + gTextLeftShift;
                rect.right = rect.right + gTextLeftShift-1;
                int rubyHitboxOffset = rect.width() * 0.1;

                if (flag_in_ruby)
                {
                    rect.right += parent_node->getStyle()->padding[2].value;
                    rect.right -= rubyHitboxOffset;
                }
                list_.add(TextRect(node, rect, string,word));
                if(flag_in_ruby && rubytext.length()>3)
                {
                    rect.left = rect.right;
                    rect.right += rubyHitboxOffset;
                    //LE("add rubyrect 1 for [%s] [%d:%d][%d:%d], [%s]",LCSTR(rubytext),rect.left,rect.right,rect.top,rect.bottom,LCSTR(node->getXPath()));
                    list_.add(TextRect(node,rect,rubytext,word));
                }
            }
            else
            {
                pos = nodeRange->getStart().getOffset();
                for (; pos < len; pos++)
                {
                    ldomWord word = ldomWord(node, pos, pos + 1);
                    lvRect rect = rectHelper_->getRect(word);
                    lString16 string = word.getText();
                    if (text[pos] == ' ' || text[pos] == '\t' || string == L"\u200B" || string == L"\u00A0" )
                    {
                        continue;
                    }
                    break;
                }
                if (pos == len)// && node->getNodeIndex() == 0)
                {
                    css_style_rec_t *style = parent_node->getStyle().get();
                    if (style->display == css_d_block || parent_node->getNodeName() == "br")
                    {
                        return;
                    }
                }

                pos = nodeRange->getStart().getOffset();
                rectHelper_->ResetLineIndex();
                lvRect rect;
                ldomWord word;
                int rubyHitboxOffset = 0;

                for (; pos < len ; pos++)
                {
                    word = ldomWord(node, pos, pos + 1);
                    rect = rectHelper_->getRect(word);
                    lString16 string = word.getText();
                    rect.left = rect.left + gTextLeftShift;
                    rect.right = rect.right + gTextLeftShift-1;
                    if (flag_in_ruby)
                    {
                        if (pos == start)
                        {
                            rect.left -= parent_node->getStyle()->padding[0].value;
                        }
                        if (pos == len - 1)
                        {
                            rubyHitboxOffset = rect.width() * 0.1;
                            rect.right += parent_node->getStyle()->padding[2].value;
                            rect.right -= rubyHitboxOffset;
                        }
                    }
                    list_.add(TextRect(node, rect, string, word));
                }
                if(flag_in_ruby && rubytext.length()>3)
                {
                    rect.left = rect.right;
                    rect.right += rubyHitboxOffset;
                    //LE("add rubyrect 2 for [%s] [%d:%d][%d:%d], [%s]",LCSTR(rubytext),rect.left,rect.right,rect.top,rect.bottom,LCSTR(node->getXPath()));
                    list_.add(TextRect(node,rect,rubytext,word));
                }
            }
        }
        /// called for each found node in range
        virtual bool onElement(ldomXPointerEx* ptr) {
            ldomNode* elem = ptr->getNode();
            if (elem->getRendMethod() == erm_invisible) {
                return false;
            }
            return true;
        }

        bool CheckNodeInvisibility(ldomNode *node)
        {
            ldomNode * p = node;
            while (p->getParentNode()!=NULL)
            {
                p = p->getParentNode();
                if (p->getRendMethod() == erm_invisible)
                {
                    return false;
                }
            }
            return true;
        }
    };
    WordsCollector collector;
    forEach2(&collector);
    if( RTL_DISPLAY_ENABLE && collector.contains_rtl && rtl_enable )
    {
        words_list = RTL_mix(collector.list_, clip_width, rtl_space);
    }
    else
    {
        words_list = collector.list_;
    }
    return;
}

/*ALPHACHECK*/
/*
int TRFLAGS = 0;
TRFLAGS |= CH_PROP_ALPHA;
TRFLAGS |= CH_PROP_DIGIT;
TRFLAGS |= CH_PROP_PUNCT;
TRFLAGS |= CH_PROP_SPACE;
TRFLAGS |= CH_PROP_HYPHEN;
TRFLAGS |= CH_PROP_VOWEL;
TRFLAGS |= CH_PROP_CONSONANT;
TRFLAGS |= CH_PROP_SIGN;
TRFLAGS |= CH_PROP_ALPHA_SIGN;
TRFLAGS |= CH_PROP_DASH;
TRFLAGS |= CH_PROP_HIEROGLYPH;

//int alpha = lGetCharProps(text[pos]) & TRFLAGS; //  words check here
if (alpha)
{
 do stuff here
}
*/


/// adds all visible words from range, returns number of added words
int ldomWordExList::addRangeWords( ldomXRange & range, bool /*trimPunctuation*/ ) {
    LVArray<ldomWord> list;
    range.getRangeWords( list );
    for ( int i=0; i<list.length(); i++ )
        add( new ldomWordEx(list[i]) );
    init();
    return list.length();
}

lvPoint ldomMarkedRange::getMiddlePoint() {
    if ( start.y==end.y ) {
        return lvPoint( ((start.x + end.x)>>1), start.y );
    } else {
        return start;
    }
}

/// returns distance (dx+dy) from specified point to middle point
int ldomMarkedRange::calcDistance( int x, int y, MoveDirection dir ) {
    lvPoint middle = getMiddlePoint();
    int dx = middle.x - x;
    int dy = middle.y - y;
    if ( dx<0 ) dx = -dx;
    if ( dy<0 ) dy = -dy;
    switch (dir) {
    case DIR_LEFT:
    case DIR_RIGHT:
        return dx + dy;
    case DIR_UP:
    case DIR_DOWN:
        return dx + dy*100;
    case DIR_ANY:
        return dx + dy;
    }
    return dx + dy;
}

/// select word
void ldomWordExList::selectWord( ldomWordEx * word, MoveDirection dir )
{
    selWord = word;
    if ( selWord ) {
        lvPoint middle = word->getMark().getMiddlePoint();
        if ( x==-1 || (dir!=DIR_UP && dir!=DIR_DOWN) )
            x = middle.x;
        y = middle.y;
    } else {
        x = y = -1;
    }
}

/// select next word in specified direction
ldomWordEx * ldomWordExList::selectNextWord( MoveDirection dir, int moveBy )
{
    if ( !selWord )
        return selectMiddleWord();
    pattern.clear();
    for ( int i=0; i<moveBy; i++ ) {
        ldomWordEx * word = findNearestWord( x, y, dir );
        if ( word )
            selectWord( word, dir );
    }
    return selWord;
}

/// select middle word in range
ldomWordEx * ldomWordExList::selectMiddleWord() {
    if ( minx==-1 )
        init();
    ldomWordEx * word = findNearestWord( (maxx+minx)/2, (miny+maxy)/2, DIR_ANY );
    selectWord(word, DIR_ANY);
    return word;
}

ldomWordEx * ldomWordExList::findWordByPattern()
{
    ldomWordEx * lastBefore = NULL;
    ldomWordEx * firstAfter = NULL;
    bool selReached = false;
    for ( int i=0; i<length(); i++ ) {
        ldomWordEx * item = get(i);
        if ( item==selWord )
            selReached = true;
        lString16 text = item->getText();
        text.lowercase();
        bool flg = true;
        for ( int j=0; j<pattern.length(); j++ ) {
            if ( j>=text.length() ) {
                flg = false;
                break;
            }
            lString16 chars = pattern[j];
            chars.lowercase();
            bool charFound = false;
            for ( int k=0; k<chars.length(); k++ ) {
                if ( chars[k]==text[j] ) {
                    charFound = true;
                    break;
                }
            }
            if ( !charFound ) {
                flg = false;
                break;
            }
        }
        if ( !flg )
            continue;
        if ( selReached ) {
            if ( firstAfter==NULL )
                firstAfter = item;
        } else {
            lastBefore = item;
        }
    }
    if ( firstAfter )
        return firstAfter;
    else
        return lastBefore;
}

/// try append search pattern and find word
ldomWordEx * ldomWordExList::appendPattern(lString16 chars)
{
    pattern.add(chars);
    ldomWordEx * foundWord = findWordByPattern();

    if ( foundWord ) {
        selectWord(foundWord, DIR_ANY);
    } else {
        pattern.erase(pattern.length()-1, 1);
    }
    return foundWord;
}

/// remove last character from pattern and try to search
ldomWordEx * ldomWordExList::reducePattern()
{
    if ( pattern.length()==0 )
        return NULL;
    pattern.erase(pattern.length()-1, 1);
    ldomWordEx * foundWord = findWordByPattern();

    if ( foundWord )
        selectWord(foundWord, DIR_ANY);
    return foundWord;
}

/// find word nearest to specified point
ldomWordEx * ldomWordExList::findNearestWord( int x, int y, MoveDirection dir ) {
    if ( !length() )
        return NULL;
    int bestDistance = -1;
    ldomWordEx * bestWord = NULL;
    ldomWordEx * defWord = (dir==DIR_LEFT || dir==DIR_UP) ? get(length()-1) : get(0);
    int i;
    if ( dir==DIR_LEFT || dir==DIR_RIGHT ) {
        int thisLineY = -1;
        int thisLineDy = -1;
        for ( i=0; i<length(); i++ ) {
            ldomWordEx * item = get(i);
            lvPoint middle = item->getMark().getMiddlePoint();
            int dy = middle.y - y;
            if ( dy<0 ) dy = -dy;
            if ( thisLineY==-1 || thisLineDy>dy ) {
                thisLineY = middle.y;
                thisLineDy = dy;
            }
        }
        ldomWordEx * nextLineWord = NULL;
        for ( i=0; i<length(); i++ ) {
            ldomWordEx * item = get(i);
            if ( dir!=DIR_ANY && item==selWord )
                continue;
            ldomMarkedRange * mark = &item->getMark();
            lvPoint middle = mark->getMiddlePoint();
            switch ( dir ) {
            case DIR_LEFT:
                if ( middle.y<thisLineY )
                    nextLineWord = item; // last word of prev line
                if ( middle.x>=x )
                    continue;
                break;
            case DIR_RIGHT:
                if ( nextLineWord==NULL && middle.y>thisLineY )
                    nextLineWord = item; // first word of next line
                if ( middle.x<=x )
                    continue;
                break;
            case DIR_UP:
            case DIR_DOWN:
            case DIR_ANY:
                // none
                break;
            }
            if ( middle.y!=thisLineY )
                continue;
            int dist = mark->calcDistance(x, y, dir);
            if ( bestDistance==-1 || dist<bestDistance ) {
                bestWord = item;
                bestDistance = dist;
            }
        }
        if ( bestWord!=NULL )
            return bestWord; // found in the same line
        if ( nextLineWord!=NULL  )
            return nextLineWord;
        return defWord;
    }
    for ( i=0; i<length(); i++ ) {
        ldomWordEx * item = get(i);
        if ( dir!=DIR_ANY && item==selWord )
            continue;
        ldomMarkedRange * mark = &item->getMark();
        lvPoint middle = mark->getMiddlePoint();
        if ( dir==DIR_UP && middle.y >= y )
            continue;
        if ( dir==DIR_DOWN && middle.y <= y )
            continue;

        int dist = mark->calcDistance(x, y, dir);
        if ( bestDistance==-1 || dist<bestDistance ) {
            bestWord = item;
            bestDistance = dist;
        }
    }
    if ( bestWord!=NULL )
        return bestWord;
    return defWord;
}

void ldomWordExList::init()
{
    if ( !length() )
        return;
    for ( int i=0; i<length(); i++ ) {
        ldomWordEx * item = get(i);
        lvPoint middle = item->getMark().getMiddlePoint();
        if ( i==0 || minx > middle.x )
            minx = middle.x;
        if ( i==0 || maxx < middle.x )
            maxx = middle.x;
        if ( i==0 || miny > middle.y )
            miny = middle.y;
        if ( i==0 || maxy < middle.y )
            maxy = middle.y;
    }
}


/// returns href attribute of <A> element, null string if not found
lString16 ldomXPointer::getHRef()
{
    if ( isNull() )
        return lString16::empty_str;
    ldomNode * node = getNode();
    while ( node && !node->isElement() )
        node = node->getParentNode();
    while ( node && node->getNodeId()!=el_a )
        node = node->getParentNode();
    if ( !node )
        return lString16::empty_str;
    lString16 ref = node->getAttributeValue( LXML_NS_ANY, attr_href );
    if (!ref.empty() && ref[0] != '#')
        ref = DecodeHTMLUrlString(ref);
    return ref;
}

lString16 ldomXPointer::getImgHRef()
{
    if (isNull())
    {
        //CRLog::error("emptystr");
        return lString16::empty_str;
    }
    ldomNode *node = getNode();
    while (node && !node->isElement())
    {
        //CRLog::error("node is not element");
        node = node->getParentNode();
    }
    if (!node)
    {
        //CRLog::error("!node");
        return lString16::empty_str;}

    lString16 ref = node->getAttributeValue(LXML_NS_ANY, attr_href);
    if (!ref.empty())
        ref = DecodeHTMLUrlString(ref);
    return ref;
}

/// returns href attribute of <A> element, null string if not found
lString16 ldomXRange::getHRef()
{
    if ( isNull() )
        return lString16::empty_str;
    return _start.getHRef();
}


/// returns text between two XPointer positions
lString16 ldomXRange::GetRangeText(lChar16 blockDelimiter, int maxTextLen)
{
    if (isNull())
    {
        return lString16::empty_str;
    }
    class TextKeeper : public ldomNodeCallback
    {
        LVArray<lvRect> para_rect_array;
        bool endnode_found_ = false;
        int unused_;
        lString16 text_;

    public:

        TextKeeper(int &unused) : unused_(unused)	{	}

        // Called for each text fragment in range
        void processText(ldomNode *node, ldomXRange *range)
        {
            if (node->isNull())
            {
                return;
            }
            ldomNode* parent_node = node->getParentNode();
            bool flag_in_ruby = false;

            if(parent_node->getNodeName() == "ruby")
            {
                flag_in_ruby = true;
                if(text_.lastChar() == ' ')
                {
                    text_ = text_.substr(0, text_.length()-1);
                }
            }
            if( parent_node->getNodeName() == "rt" )
            {
                flag_in_ruby = true;
            }
            lString16 rubytext;

            ldomXRange noderange = ldomXRange(node);

            if (node == range->getEndNode())
            {
                noderange.setEnd(range->getEnd());
            }
            if(node == range->getStartNode() )
            {
                noderange.setStart(range->getStart());
            }

            int start = noderange.getStart().getOffset();
            int end = noderange.getEnd().getOffset();

            if(node->getParentNode("rt") != NULL)
            {
                text_ << "(" + node->getText().substr(start, end - start) + ")";
                return;
            }
            text_ << node->getText().substr(start, end - start);
            if(noderange.getEnd() != range->getEnd() && !flag_in_ruby)
            {
                text_ << " ";
            }
    }

        virtual bool onElement(ldomNode * node) {
            if(node->getNodeName() == "rt" && node->getParentNode("ruby") != NULL)
            {
                return true;
            }
            return node->getRendMethod() != erm_invisible;
        }

        lString16 getText() { return text_;}
    };

    int unused = 0;
    TextKeeper callback(unused);
    forEach2(&callback);
    return callback.getText();

}
CrDom* LVParseXMLStream(
        LVStreamRef stream,
		const elem_def_t* elem_table,
		const attr_def_t* attr_table,
		const ns_def_t* ns_table)
{
    if (stream.isNull())
        return NULL;
    bool error = true;
    CrDom * doc;
    doc = new CrDom();
    doc->setDocFlags( 0 );

    LvDomWriter writer(doc);
    doc->setNodeTypes( elem_table );
    doc->setAttributeTypes( attr_table );
    doc->setNameSpaceTypes( ns_table );

    /// FB2 format
    LVFileFormatParser * parser = new LvXmlParser(stream, &writer);
    if ( parser->CheckFormat() ) {
        if ( parser->Parse() ) {
            error = false;
        }
    }
    delete parser;
    if ( error ) {
        delete doc;
        doc = NULL;
    }
    return doc;
}

CrDom * LVParseHTMLStream(
        LVStreamRef stream,
        const elem_def_t * elem_table,
        const attr_def_t * attr_table,
        const ns_def_t * ns_table)
{
    if ( stream.isNull() )
        return NULL;
    bool error = true;
    CrDom * doc;
    doc = new CrDom();
    doc->setDocFlags( 0 );

    LvDomAutocloseWriter writerFilter(doc, false, HTML_AUTOCLOSE_TABLE);
    doc->setNodeTypes( elem_table );
    doc->setAttributeTypes( attr_table );
    doc->setNameSpaceTypes( ns_table );

    /// FB2 format
    LVFileFormatParser * parser = new LvHtmlParser(stream, &writerFilter);
    if ( parser->CheckFormat() ) {
        if ( parser->Parse() ) {
            error = false;
        }
    }
    delete parser;
    if ( error ) {
        delete doc;
        doc = NULL;
    }
    return doc;
}

lString16 LvDocFragmentWriter::convertId( lString16 id )
{
    if ( !codeBasePrefix.empty() ) {
        return codeBasePrefix + "_" + id;
    }
    return id;
}

lString16 LvDocFragmentWriter::convertHref( lString16 href )
{
    if ( href.pos("://")>=0 )
        return href; // fully qualified href: no conversion
    if ( href.pos("~")>=0 )
        return href.substr(1); // fully qualified href: no conversion
#if 0
    CRLog::trace("convertHref(%s, codeBase=%s, filePathName=%s)",
            LCSTR(href),
            LCSTR(codeBase),
            LCSTR(filePathName));
#endif
    if (href[0] == '#') {
        lString16 replacement = pathSubstitutions.get(filePathName);
        if (replacement.empty())
            return href;
        lString16 p = cs16("#") + replacement + "_" + href.substr(1);
        return p;
    }

    href = LVCombinePaths(codeBase, href);

    // resolve relative links
    lString16 p, id;
    if ( !href.split2(cs16("#"), p, id) )
        p = href;
    if ( p.empty() ) {
        //CRLog::trace("codebase = %s -> href = %s", LCSTR(codeBase), LCSTR(href));
        if ( codeBasePrefix.empty() )
            return href;
        p = codeBasePrefix;
    } else {
        lString16 replacement = pathSubstitutions.get(p);
        if(replacement.empty())
        {
            replacement =  pathSubstitutions.get(DecodeHTMLUrlString(p));
        }
        //CRLog::trace("href %s -> %s", LCSTR(p), LCSTR(replacement));
        if ( !replacement.empty() )
            p = replacement;
        else
            return href;
        //else
        //    p = codeBasePrefix;
        //p = LVCombinePaths( codeBase, p ); // relative to absolute path
    }
    if ( !id.empty() )
        p = p + "_" + id;

    p = cs16("#") + p;

    //CRLog::trace("converted href=%s to %s", LCSTR(href), LCSTR(p) );

    return p;
}

void LvDocFragmentWriter::setCodeBase( lString16 fileName )
{
    filePathName = fileName;
    codeBasePrefix = pathSubstitutions.get(fileName);
    codeBase = LVExtractPath(filePathName);
    if ( codeBasePrefix.empty() ) {
        //CRLog::trace("codeBasePrefix is empty for path %s", LCSTR(fileName));
        codeBasePrefix = pathSubstitutions.get(fileName);
    }
    stylesheetFile.clear();
}

/// called on attribute
void LvDocFragmentWriter::OnAttribute(const lChar16* nsname,
		const lChar16* attrname, const lChar16* attrvalue)
{
    if ( insideTag ) {
        if ( !lStr_cmp(attrname, "href") || !lStr_cmp(attrname, "src" )|| !lStr_cmp(attrname, "nref" ) ) {
            parent->OnAttribute(nsname, attrname, convertHref(lString16(attrvalue)).c_str() );
        } else if ( !lStr_cmp(attrname, "id") ) {
                parent->OnAttribute(nsname, attrname, convertId(lString16(attrvalue)).c_str() );
        } else if ( !lStr_cmp(attrname, "name") ) {
            //CRLog::trace("name attribute = %s", LCSTR(lString16(attrvalue)));
            if (lStr_cmp(attrvalue, "notes") != 0 ) //notes attribute avoiding
            {
                parent->OnAttribute(nsname, attrname, convertId(lString16(attrvalue)).c_str());
            }
            else
            {
                // pass [name="notes"] attribute safely
                parent->OnAttribute(nsname, attrname, attrvalue);
            }
        } else {
            parent->OnAttribute(nsname, attrname, attrvalue);
        }
    } else {
        if ( styleDetectionState ) {
            if ( !lStr_cmp(attrname, "rel") && !lStr_cmp(attrvalue, "stylesheet") )
                styleDetectionState |= 2;
            else if ( !lStr_cmp(attrname, "type") ) {
                if ( !lStr_cmp(attrvalue, "text/css") )
                    styleDetectionState |= 4;
                else
                    styleDetectionState = 0;  // text/css type supported only
            } else if ( !lStr_cmp(attrname, "href") ) {
                styleDetectionState |= 8;
                lString16 href = attrvalue;
                if ( stylesheetFile.empty() )
                	tmpStylesheetFile = LVCombinePaths( codeBase, href );
                else
                	tmpStylesheetFile = href;
            }
            if (styleDetectionState == 15) {
            	if (!stylesheetFile.empty())
            		stylesheetLinks.add(tmpStylesheetFile);
            	else
            		stylesheetFile = tmpStylesheetFile;

                styleDetectionState = 0;
                //CRLog::trace("CSS file href: %s", LCSTR(stylesheetFile));
            }
        }
    }
}

/// called on opening tag
ldomNode * LvDocFragmentWriter::OnTagOpen( const lChar16 * nsname, const lChar16 * tagname )
{
    if (insideTag) {
        return parent->OnTagOpen(nsname, tagname);
    } else {
        if ( !lStr_cmp(tagname, "link") )
            styleDetectionState = 1;
        if ( !lStr_cmp(tagname, "style") )
            headStyleState = 1;
    }
    /* These changes are made for docx footnotes to be parsed successfully. Needs tests.
     * Revert if these changes break something else.*/
    //todo : Test other document formats when docx scan will be available.
    if ( !insideTag /*&& baseTag==tagname*/ ) {
        insideTag = true;
        if ( !baseTagReplacement.empty() ) {
            baseElement = parent->OnTagOpen(L"", baseTagReplacement.c_str());
            lastBaseElement = baseElement;
            if (!stylesheetFile.empty()) {
                parent->OnAttribute(L"", L"StyleSheet", stylesheetFile.c_str());
                //CRLog::trace("Setting StyleSheet attribute to %s for document fragment",
                //        LCSTR(stylesheetFile));
            }
            if ( !codeBasePrefix.empty() )
                parent->OnAttribute(L"", L"id", codeBasePrefix.c_str() );
            parent->OnTagBody();
            if ( !headStyleText.empty() || stylesheetLinks.length() > 0 ) {
                parent->OnTagOpen(L"", L"stylesheet");
                parent->OnAttribute(L"", L"href", codeBase.c_str() );
                lString16 imports;
                for (int i = 0; i < stylesheetLinks.length(); i++) {
                    lString16 import("@import url(\"");
                    import << stylesheetLinks.at(i);
                    import << "\");\n";
                    imports << import;
                }
                stylesheetLinks.clear();
                lString16 styleText = imports + headStyleText.c_str();
                parent->OnTagBody();
                parent->OnText(styleText.c_str(), styleText.length(), 0);
                parent->OnTagClose(L"", L"stylesheet");
            }
            // add base tag, too (e.g., in CSS, styles are often defined for body tag"
            parent->OnTagOpen(L"", baseTag.c_str());
            if (parent->getFlags() & TXTFLG_IN_NOTES)
            {
                parent->OnAttribute(L"", L"name", L"notes");
            }
            parent->OnTagBody();
            return baseElement;
        }
    }
    return NULL;
}

/// called on closing tag
void LvDocFragmentWriter::OnTagClose( const lChar16 * nsname, const lChar16 * tagname )
{
    styleDetectionState = headStyleState = 0;
    if ( insideTag && baseTag==tagname ) {
        insideTag = false;
        if ( !baseTagReplacement.empty() ) {
            parent->OnTagClose(L"", baseTag.c_str());
            parent->OnTagClose(L"", baseTagReplacement.c_str());
        }
        baseElement = NULL;
        return;
    }
    if ( insideTag )
        parent->OnTagClose(nsname, tagname);
}

/// called after > of opening tag (when entering tag body)
/// or just before /> closing tag for empty tags
void LvDocFragmentWriter::OnTagBody()
{
    if ( insideTag ) {
        parent->OnTagBody();
    }
    if ( styleDetectionState == 11 ) {
        // incomplete <link rel="stylesheet", href="..." />; assuming type="text/css"
        if ( !stylesheetFile.empty() )
            stylesheetLinks.add(tmpStylesheetFile);
        else
            stylesheetFile = tmpStylesheetFile;
        styleDetectionState = 0;
    } else
        styleDetectionState = 0;
}

/** \brief callback object to fill DOM tree

    To be used with XML parser as callback object.

    Creates document according to incoming events.

    Autoclose HTML tags.
*/

void LvDomAutocloseWriter::setClass(const lChar16* className, bool overrideExisting)
{
    ldomNode * node = _currNode->_element;
    if ( _classAttrId==0 ) {
        _classAttrId = doc_->getAttrNameIndex(L"class");
    }
    if ( overrideExisting || !node->hasAttribute(_classAttrId) ) {
        node->setAttributeValue(LXML_NS_NONE, _classAttrId, className);
    }
}

void LvDomAutocloseWriter::appendStyle(const lChar16* style )
{
    ldomNode * node = _currNode->_element;
    if ( _styleAttrId==0 ) {
        _styleAttrId = doc_->getAttrNameIndex(L"style");
    }
    if (!doc_->getDocFlag(DOC_FLAG_EMBEDDED_STYLES)) {
        return;
    }
    lString16 oldStyle = node->getAttributeValue(_styleAttrId);
    if (!oldStyle.empty() && oldStyle.at(oldStyle.length() - 1) != ';') {
        oldStyle << "; ";
    }
    oldStyle << style;
    node->setAttributeValue(LXML_NS_NONE, _styleAttrId, oldStyle.c_str());
}

void LvDomAutocloseWriter::AutoClose(lUInt16 tag_id, bool open)
{
    lUInt16 * rule = _rules[tag_id];
    if ( !rule )
        return;
    if ( open ) {
        ldomElementWriter * found = NULL;
        ldomElementWriter * p = _currNode;
        while ( p && !found ) {
            lUInt16 id = p->_element->getNodeId();
            for ( int i=0; rule[i]; i++ ) {
                if ( rule[i]==id ) {
                    found = p;
                    break;
                }
            }
            p = p->_parent;
        }
        // found auto-close target
        if ( found != NULL ) {
            bool done = false;
            while ( !done && _currNode ) {
                if ( _currNode == found )
                    done = true;
                ldomNode * closedElement = _currNode->getElement();
                _currNode = pop( _currNode, closedElement->getNodeId() );
                //ElementCloseHandler( closedElement );
            }
        }
    } else {
        if ( !rule[0] )
            _currNode = pop( _currNode, _currNode->getElement()->getNodeId() );
    }
}

ldomNode* LvDomAutocloseWriter::OnTagOpen(const lChar16* nsname, const lChar16* tagname)
{
    if (!_tagBodyCalled) {
        CRLog::error("LvDomAutocloseWriter::OnTagOpen w/o parent's OnTagBody : %s",
        		LCSTR(lString16(tagname)));
        crFatalError();
    }
    _tagBodyCalled = false;
    //logfile << "lxmlDocumentWriter::OnTagOpen() [" << nsname << ":" << tagname << "]";
    //if ( nsname && nsname[0] )
    //    lStr_lowercase( const_cast<lChar16 *>(nsname), lStr_len(nsname) );
    //lStr_lowercase( const_cast<lChar16 *>(tagname), lStr_len(tagname) );

    // Patch for bad LIB.RU books - BR delimited paragraphs in "Fine HTML" format
    if ((tagname[0] == 'b' && tagname[1] == 'r' && tagname[2] == 0)
        || (tagname[0] == 'd' && tagname[1] == 'd' && tagname[2] == 0)) {
        // substitute to P
        tagname = L"p";
        _libRuParagraphStart = true; // to trim leading &nbsp;
    } else {
        _libRuParagraphStart = false;
    }

    lUInt16 id = doc_->getElementNameIndex(tagname);
    lUInt16 nsid = (nsname && nsname[0]) ? doc_->getNsNameIndex(nsname) : 0;
    AutoClose( id, true );
    _currNode = new ldomElementWriter( doc_, nsid, id, _currNode,_flags );
    _flags = _currNode->getFlags();
    if (_libRuDocumentDetected && (_flags & TXTFLG_PRE)) {
        // convert preformatted text into paragraphs
        _flags |= TXTFLG_PRE_PARA_SPLITTING | TXTFLG_TRIM;
    }
    //logfile << " !o!\n";
    //return _currNode->getElement();
    return _currNode->getElement();
}

void LvDomAutocloseWriter::OnTagBody()
{
    _tagBodyCalled = true;
    if (_currNode) _currNode->onBodyEnter();
}

bool isRightAligned(ldomNode * node) {
    lString16 style = node->getAttributeValue(attr_style);
    if (style.empty())
        return false;
    int p = style.pos("text-align: right", 0);
    return (p >= 0);
}

void LvDomAutocloseWriter::OnTagClose(const lChar16* /*nsname*/, const lChar16* tagname)
{
    if ( !_tagBodyCalled ) {
        CRLog::error("OnTagClose w/o parent's OnTagBody : %s", LCSTR(lString16(tagname)));
        crFatalError();
    }
    //logfile << "LvDomWriter::OnTagClose() [" << nsname << ":" << tagname << "]";
    //if ( nsname && nsname[0] )
    //    lStr_lowercase( const_cast<lChar16 *>(nsname), lStr_len(nsname) );
    //lStr_lowercase( const_cast<lChar16 *>(tagname), lStr_len(tagname) );
    if (!_currNode)
    {
        _errFlag = true;
        //logfile << " !c-err!\n";
        return;
    }

    if (tagname[0] == 'l' && _currNode && !lStr_cmp(tagname, "link")) {
        // link node
        if (_currNode &&
            _currNode->getElement() &&
            _currNode->getElement()->isNodeName("link") &&
            _currNode->getElement()->getParentNode() &&
            _currNode->getElement()->getParentNode()->isNodeName("head") &&
            _currNode->getElement()->getAttributeValue("rel") == "stylesheet" &&
            _currNode->getElement()->getAttributeValue("type") == "text/css"
            )
        {

            lString16 href = _currNode->getElement()->getAttributeValue("href");
            lString16 stylesheetFile = LVCombinePaths(doc_->getCodeBase(), href);
            //CRLog::trace("Internal stylesheet file: %s", LCSTR(stylesheetFile));
            doc_->stylesheet_file_name_ = stylesheetFile;
            if(doc_->stylesheet_file_name_ != stylesheetFile)
            {
                doc_->stylesheet_file_name_ = stylesheetFile;
                doc_->applyDocStylesheet();
            }
        }
    }
    lUInt16 id = doc_->getElementNameIndex(tagname);

    //======== START FILTER CODE ============
    AutoClose( _currNode->_element->getNodeId(), false );
    //======== END FILTER CODE ==============
    //lUInt16 nsid = (nsname && nsname[0]) ? doc_->getNsNameIndex(nsname) : 0;
    // save closed element
    ldomNode * closedElement = _currNode->getElement();
    _errFlag |= (id != closedElement->getNodeId());
    _currNode = pop( _currNode, id );

    if (_currNode) {
        _flags = _currNode->getFlags();
        if (_libRuDocumentDetected && (_flags & TXTFLG_PRE)) {
            // convert preformatted text into paragraphs
            _flags |= TXTFLG_PRE_PARA_SPLITTING | TXTFLG_TRIM;
        }
    }

    //=============================================================
    // LIB.RU patch: remove table of contents
    //ElementCloseHandler( closedElement );
    //=============================================================

    if ( id==_stopTagId ) {
        //CRLog::trace("stop tag found, stopping...");
        _parser->Stop();
    }
    //logfile << " !c!\n";
}

void LvDomAutocloseWriter::ElementCloseHandler(ldomNode* node)
{
    ldomNode* parent = node->getParentNode();
    lUInt16 id = node->getNodeId();
    if (parent) {
        if (parent->getLastChild() != node) {
            return;
        }
        if (id == el_table) {
            if (isRightAligned(node) && node->getAttributeValue(attr_width) == "30%") {
                // LIB.RU TOC detected: remove it
                //parent = parent->modify();
                //parent->removeLastChild();
            }
        } else if (id == el_pre && _libRuDocumentDetected) {
            // For LIB.ru - replace PRE element with DIV (section?)
            if (node->getChildCount() == 0) {
                //parent = parent->modify();
                // Remove empty PRE element
                //parent->removeLastChild();
            //} else if (node->getLastChild()->getNodeId() == el_div
            //&& node->getLastChild()->getChildCount()
            //&& ((ldomElement*) node->getLastChild())->getLastChild()->getNodeId() == el_form) {
                // Remove lib.ru final section
                //parent->removeLastChild();
            } else {
                node->setNodeId(el_div);
            }
        } else if (id == el_div) {
            //CRLog::trace("DIV attr align = %s", LCSTR(node->getAttributeValue(attr_align)));
            //CRLog::trace("DIV attr count = %d", node->getAttrCount());
            //int alignId = node->getCrDom()->getAttrNameIndex("align");
            //CRLog::trace("align= %d %d", alignId, attr_align);
            //for (int i = 0; i < node->getAttrCount(); i++)
            //    CRLog::trace("DIV attr %s", LCSTR(node->getAttributeName(i)));
            if (isRightAligned(node)) {
                ldomNode* child = node->getLastChild();
                if (child && child->getNodeId() == el_form) {
                    // LIB.RU form detected: remove it
                    //parent = parent->modify();
                    parent->removeLastChild();
                    _libRuDocumentDetected = true;
                }
            }
        }
    }
    if (!_libRuDocumentDetected) {
        node->persist();
    }
}

void LvDomAutocloseWriter::OnAttribute(const lChar16* nsname,
        const lChar16* attrname,
        const lChar16* attrvalue)
{
    //if ( nsname && nsname[0] )
    //    lStr_lowercase( const_cast<lChar16 *>(nsname), lStr_len(nsname) );
    //lStr_lowercase( const_cast<lChar16 *>(attrname), lStr_len(attrname) );

    if ( !lStr_cmp(attrname, "align") ) {
        if ( !lStr_cmp(attrvalue, "justify") )
            appendStyle( L"text-align: justify" );
        else if ( !lStr_cmp(attrvalue, "left") )
            appendStyle( L"text-align: left" );
        else if ( !lStr_cmp(attrvalue, "right") )
            appendStyle( L"text-align: right" );
        else if ( !lStr_cmp(attrvalue, "center") )
            appendStyle( L"text-align: center" );
       return;
    }
    lUInt16 attr_ns = (nsname && nsname[0]) ? doc_->getNsNameIndex( nsname ) : 0;
    lUInt16 attr_id = (attrname && attrname[0]) ? doc_->getAttrNameIndex( attrname ) : 0;

    _currNode->addAttribute( attr_ns, attr_id, attrvalue );
}

/// called on text
void LvDomAutocloseWriter::OnText( const lChar16 * text, int len, lUInt32 flags )
{
    //logfile << "lxmlDocumentWriter::OnText() fpos=" << fpos;
    if (_currNode)
    {
        AutoClose( _currNode->_element->getNodeId(), false );
        if ( (_flags & XML_FLAG_NO_SPACE_TEXT)
             && IsEmptySpace(text, len) && !(flags & TXTFLG_PRE))
             return;
        bool autoPara = _libRuDocumentDetected && (flags & TXTFLG_PRE);
        if (_currNode->_allowText) {
            if ( _libRuParagraphStart ) {
                bool cleaned = false;
                while ( *text==160 && len > 0 ) {
                    cleaned = true;
                    text++;
                    len--;
                    while ( *text==' ' && len > 0 ) {
                        text++;
                        len--;
                    }
                }
                if ( cleaned ) {
                    setClass(L"justindent");
                    //appendStyle(L"text-indent: 1.3em; text-align: justify");
                }
                _libRuParagraphStart = false;
            }
            int leftSpace = 0;
            const lChar16 * paraTag = NULL;
            bool isHr = false;
            if ( autoPara ) {
                while ( (*text==' ' || *text=='\t' || *text==160) && len > 0 ) {
                    text++;
                    len--;
                    leftSpace += (*text == '\t') ? 8 : 1;
                }
                paraTag = leftSpace > 8 ? L"h2" : L"p";
                lChar16 ch = 0;
                bool sameCh = true;
                for ( int i=0; i<len; i++ ) {
                    if ( !ch )
                        ch = text[i];
                    else if ( ch != text[i] ) {
                        sameCh = false;
                        break;
                    }
                }
                if ( !ch )
                    sameCh = false;
                if ( (ch=='-' || ch=='=' || ch=='_' || ch=='*' || ch=='#') && sameCh )
                    isHr = true;
            }
            if ( isHr ) {
                OnTagOpen( NULL, L"hr" );
                OnTagBody();
                OnTagClose( NULL, L"hr" );
            } else if ( len > 0 ) {
                if ( autoPara ) {
                    OnTagOpen( NULL, paraTag );
                    OnTagBody();
                }
                _currNode->onText( text, len, flags );
                if ( autoPara )
                    OnTagClose( NULL, paraTag );
            }
        }
    }
    //logfile << " !t!\n";
}

LvDomAutocloseWriter::LvDomAutocloseWriter(CrDom* document,
		bool headerOnly, const char *** rules)
		: LvDomWriter(document, headerOnly),
		  _libRuDocumentDetected(false),
		  _libRuParagraphStart(false),
		  _styleAttrId(0),
		  _classAttrId(0),
		  _tagBodyCalled(true) {
    lUInt16 i;
    for ( i=0; i<MAX_ELEMENT_TYPE_ID; i++ )
        _rules[i] = NULL;
    lUInt16 items[MAX_ELEMENT_TYPE_ID];
    for ( i=0; rules[i]; i++ ) {
        const char ** rule = rules[i];
        lUInt16 j;
        for ( j=0; rule[j] && j<MAX_ELEMENT_TYPE_ID; j++ ) {
            const char * s = rule[j];
            items[j] = doc_->getElementNameIndex( lString16(s).c_str() );
        }
        if ( j>=1 ) {
            lUInt16 id = items[0];
            _rules[ id ] = new lUInt16[j];
            for ( int k=0; k<j; k++ ) {
                _rules[id][k] = k==j-1 ? 0 : items[k+1];
            }
        }
    }
}

LvDomAutocloseWriter::~LvDomAutocloseWriter() {
    for (int i = 0; i < MAX_ELEMENT_TYPE_ID; i++) {
        if (_rules[i]) delete[] _rules[i];
    }
}

void CrDomBase::setDocFlag(lUInt32 mask, bool value)
{
    if (value)
        _docFlags |= mask;
    else
        _docFlags &= ~mask;
}

void CrDomBase::setDocFlags(lUInt32 value)
{
    _docFlags = value;
}

void CrDom::clear()
{
    clearRendBlockCache();
    _rendered = false;
    _urlImageMap.clear();
    _fontList.clear();
    fontMan->UnregisterDocumentFonts(_docIndex);
    //TODO: implement clear
    //_elemStorage.
}

lUInt32 CrDomBase::calcStyleHash()
{
    lUInt32 globalHash = calcGlobalSettingsHash(getFontContextDocIndex());
    return globalHash * 31 + getDocFlags();
}

lUInt32 CrDomBase::calcStyleHashFull()
{
    int count = ((_elemCount+TNC_PART_LEN-1) >> TNC_PART_SHIFT);
    lUInt32 res = 0; //_elemCount;
    lUInt32 globalHash = calcGlobalSettingsHash(getFontContextDocIndex());
    lUInt32 docFlags = getDocFlags();
    /*
    CRLog::trace("calcStyleHash: elemCount=%d, globalHash=%08x, docFlags=%08x",
            _elemCount,
            globalHash,
            docFlags);
    */
    for ( int i=0; i<count; i++ ) {
        int offs = i*TNC_PART_LEN;
        int sz = TNC_PART_LEN;
        if ( offs + sz > _elemCount+1 ) {
            sz = _elemCount+1 - offs;
        }
        ldomNode * buf = _elemList[i];
        for ( int j=0; j<sz; j++ ) {
            if ( buf[j].isElement() ) {
                css_style_ref_t style = buf[j].getStyle();
                lUInt32 sh = calcHash( style );
                res = res * 31 + sh;
                LVFontRef font = buf[j].getFont();
                lUInt32 fh = calcHash( font );
                res = res * 31 + fh;
            }
        }
    }
    /*
    CRLog::trace(
            "calcStyleHash: elemCount=%d, globalHash=%08x docFlags=%08x, nodeStyleHash: %08x",
            _elemCount,
            globalHash,
            docFlags,
            res);
    */
    res = res * 31 + _imgScalingOptions.getHash();
    res = res * 31 + _minSpaceCondensingPercent;
    res = (res * 31 + globalHash) * 31 + docFlags;
    return res;
}

static void validateChild( ldomNode * node )
{
    // DEBUG TEST
    if ( !node->isRoot() && node->getParentNode()->getChildIndex( node->getDataIndex() )<0 ) {
        CRLog::error(
                "Invalid parent->child relation for nodes %d->%d",
                node->getParentNode()->getDataIndex(),
                node->getParentNode()->getDataIndex());
    }
}

/// called on document loading end
bool CrDomBase::validateDocument()
{
    ((CrDom*) this)->getRootNode()->recurseElements(validateChild);
    int count = ((_elemCount + TNC_PART_LEN - 1) >> TNC_PART_SHIFT);
    bool res = true;
    for (int i = 0; i < count; i++) {
        int offs = i * TNC_PART_LEN;
        int sz = TNC_PART_LEN;
        if (offs + sz > _elemCount + 1) {
            sz = _elemCount + 1 - offs;
        }
        ldomNode* buf = _elemList[i];
        for (int j = 0; j < sz; j++) {
            buf[j].setDocumentIndex(_docIndex);
            if (!buf[j].isElement()) {
                continue;
            }
            lUInt16 style = getNodeStyleIndex(buf[j]._handle._dataIndex);
            lUInt16 font = getNodeFontIndex(buf[j]._handle._dataIndex);;
            if (!style) {
                if (!buf[j].isRoot()) {
                    CRLog::error("styleId=0 for node <%s> %d",
                            LCSTR(buf[j].getNodeName()),
                            buf[j].getDataIndex());
                    res = false;
                }
            } else if (_styles.get(style).isNull()) {
                CRLog::error("styleId!=0, but absent in cache for node <%s> %d",
                        LCSTR(buf[j].getNodeName()),
                        buf[j].getDataIndex());
                res = false;
            }
            if (!font) {
                if (!buf[j].isRoot()) {
                    CRLog::error("fontId=0 for node <%s>", LCSTR(buf[j].getNodeName()));
                    res = false;
                }
            } else if (_fonts.get(font).isNull()) {
                CRLog::error("fontId!=0, but absent in cache for node <%s>",
                        LCSTR(buf[j].getNodeName()));
                res = false;
            }
        }
    }
    return res;
}

bool CrDomBase::updateLoadedStyles(bool enabled)
{
    int count = ((_elemCount + TNC_PART_LEN - 1) >> TNC_PART_SHIFT);
    bool res = true;
    LVArray<css_style_ref_t>* list = _styles.getIndex();
    // style index to font index
    _fontMap.clear();
    for (int i = 0; i < count; i++) {
        int offs = i * TNC_PART_LEN;
        int sz = TNC_PART_LEN;
        if (offs + sz > _elemCount + 1) {
            sz = _elemCount + 1 - offs;
        }
        ldomNode* buf = _elemList[i];
        for (int j = 0; j < sz; j++) {
            buf[j].setDocumentIndex(_docIndex);
            if (!buf[j].isElement()) {
                continue;
            }
            lUInt16 style = getNodeStyleIndex(buf[j]._handle._dataIndex);
            if (!enabled || style == 0) {
                setNodeFontIndex(buf[j]._handle._dataIndex, 0);
                setNodeStyleIndex(buf[j]._handle._dataIndex, 0);
                //buf[j]._data._pelem._styleIndex = 0;
                //buf[j]._data._pelem._fontIndex = 0;
                continue;
            }
            css_style_ref_t s = list->get(style);
            if (!s.isNull()) {
                lUInt16 fntIndex = _fontMap.get(style);
                if (fntIndex == 0) {
                    LVFontRef fnt = getFont(s.get(), getFontContextDocIndex());
                    fntIndex = (lUInt16) _fonts.cache(fnt);
                    if (fnt.isNull()) {
                        CRLog::error("font not found for style!");
                    } else {
                        _fontMap.set(style, fntIndex);
                    }
                } else {
                    _fonts.addIndexRef(fntIndex);
                }
                if (fntIndex <= 0) {
                    CRLog::error("font caching failed for style!");
                    res = false;
                } else {
                    setNodeFontIndex(buf[j]._handle._dataIndex, fntIndex);
                    //buf[j]._data._pelem._fontIndex = fntIndex;
                }
            } else {
                CRLog::error("Loaded style index %d not found in style collection", (int) style);
                setNodeFontIndex(buf[j]._handle._dataIndex, 0);
                setNodeStyleIndex(buf[j]._handle._dataIndex, 0);
                //buf[j]._data._pelem._styleIndex = 0;
                //buf[j]._data._pelem._fontIndex = 0;
                res = false;
            }
        }
    }
#ifdef TODO_INVESTIGATE
    if ( enabled && res) {
        //_styles.setIndex( *list );
        // correct list reference counters
        for ( int i=0; i<list->length(); i++ ) {
            if ( !list->get(i).isNull() ) {
                // decrease reference counter
                // TODO:
                //_styles.release( list->get(i) );
            }
        }
    }
#endif
    delete list;
    //getRootNode()->setFont(_def_font);
    //getRootNode()->setStyle(_def_style);
    return res;
}

/// SHOULD BE CALLED ONLY AFTER setNodeTypes
void CrDomXml::setStylesheet(const char* css, bool replace)
{
    lUInt32 oldHash = stylesheet_.getHash();
    if (replace) {
       stylesheet_.clear();
    }
    if (css && *css) {
       stylesheet_.parse(css);
    }
#ifdef OREDEBUG
    lUInt32 newHash = stylesheet_.getHash();
    if (oldHash != newHash) {
        //CRLog::trace("New stylesheet hash: %08x", newHash);
    }
#endif
}

//=====================================================
// ldomElement declaration placed here to hide DOM implementation
// use ldomNode rich interface instead
class tinyElement
{
    friend class ldomNode;
private:
    CrDom* _document;
    ldomNode* _parentNode;
    lUInt16 _id;
    lUInt16 _nsid;
    LVArray<lInt32> _children;
    ldomAttributeCollection _attrs;
    lvdom_element_render_method _rendMethod;
public:
    tinyElement(CrDom* document, ldomNode* parentNode, lUInt16 nsid, lUInt16 id)
            : _document(document),
              _parentNode(parentNode),
              _id(id),
              _nsid(nsid),
              _rendMethod(erm_invisible)
    {
        _document->_tinyElementCount++;
    }
    ~tinyElement() { _document->_tinyElementCount--; }
};

//=====================================================

/// allocate new tinyElement
ldomNode * CrDomBase::allocTinyElement( ldomNode * parent, lUInt16 nsid, lUInt16 id )
{
    ldomNode * node = allocTinyNode( ldomNode::NT_ELEMENT );
    tinyElement * elem = new tinyElement( (CrDom*)this, parent, nsid, id );
    node->_data._elem_ptr = elem;
    return node;
}

static void readOnlyError()
{
    crFatalError(125, "Text node is persistent (read-only)! Call modify() to get r/w instance.");
}

//=====================================================

// shortcut for dynamic element accessor
#ifdef DEBUG_NODE_ALLOCATION
  #define ASSERT_NODE_NOT_NULL \
    if ( isNull() ) \
		crFatalError( 1313, "Access to null node" )
#else
  #define ASSERT_NODE_NOT_NULL
#endif

/// returns node level, 0 is root node
lUInt8 ldomNode::getNodeLevel() const
{
    const ldomNode * node = this;
    int level = 0;
    for ( ; node; node = node->getParentNode() )
        level++;
    return (lUInt8)level;
}

void ldomNode::onCollectionDestroy()
{
    if ( isNull() )
        return;
    //CRLog::trace("ldomNode::onCollectionDestroy(%d) type=%d", this->_handle._dataIndex, TNTYPE);
    switch ( TNTYPE ) {
    case NT_TEXT:
        delete _data._text_ptr;
        _data._text_ptr = NULL;
        break;
    case NT_ELEMENT:
        // ???
        getCrDom()->clearNodeStyle( _handle._dataIndex );
        delete _data._elem_ptr;
        _data._elem_ptr = NULL;
        break;
    case NT_PTEXT:      // immutable (persistent) text node
        // do nothing
        break;
    case NT_PELEMENT:   // immutable (persistent) element node
        // do nothing
        break;
    }
}

void ldomNode::destroy()
{
    if ( isNull() )
        return;
    //CRLog::trace("ldomNode::destroy(%d) type=%d", this->_handle._dataIndex, TNTYPE);
    switch ( TNTYPE ) {
    case NT_TEXT:
        delete _data._text_ptr;
        break;
    case NT_ELEMENT:
        {
            getCrDom()->clearNodeStyle(_handle._dataIndex);
            tinyElement * me = _data._elem_ptr;
            // delete children
            for ( int i=0; i<me->_children.length(); i++ ) {
                ldomNode * child = getCrDom()->getTinyNode(me->_children[i]);
                if ( child )
                    child->destroy();
            }
            delete me;
            _data._elem_ptr = NULL;
        }
        delete _data._elem_ptr;
        break;
    case NT_PTEXT:
        // disable removing from storage: to minimize modifications
        //doc_->_textStorage.freeNode( _data._ptext_addr._addr );
        break;
    case NT_PELEMENT:   // immutable (persistent) element node
        {
            ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
            for ( int i=0; i<me->childCount; i++ )
                getCrDom()->getTinyNode( me->children[i] )->destroy();
            getCrDom()->clearNodeStyle( _handle._dataIndex );
//            getCrDom()->_styles.release( _data._pelem._styleIndex );
//            getCrDom()->_fonts.release( _data._pelem._fontIndex );
//            _data._pelem._styleIndex = 0;
//            _data._pelem._fontIndex = 0;
        getCrDom()->_elemStorage.freeNode( _data._pelem_addr );
        }
        break;
    }
    getCrDom()->recycleTinyNode( _handle._dataIndex );
}

/// returns index of child node by dataIndex
int ldomNode::getChildIndex( lUInt32 dataIndex ) const
{
    dataIndex &= 0xFFFFFFF0;
    ASSERT_NODE_NOT_NULL;
    int parentIndex = -1;
    switch ( TNTYPE ) {
    case NT_ELEMENT:
        {
            tinyElement * me = _data._elem_ptr;
            for ( int i=0; i<me->_children.length(); i++ ) {
                if ( (me->_children[i] & 0xFFFFFFF0) == dataIndex ) {
                    // found
                    parentIndex = i;
                    break;
                }
            }
        }
        break;
    case NT_PELEMENT:
        {
            ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
            for ( int i=0; i<me->childCount; i++ ) {
                if ( (me->children[i] & 0xFFFFFFF0) == dataIndex ) {
                    // found
                    parentIndex = i;
                    break;
                }
            }
        }
        break;
    case NT_PTEXT:      // immutable (persistent) text node
    case NT_TEXT:
        break;
    }
    return parentIndex;
}

/// returns index of node inside parent's child collection
int ldomNode::getNodeIndex() const
{
    ASSERT_NODE_NOT_NULL;
    ldomNode * parent = getParentNode();
    if ( parent )
        return parent->getChildIndex( getDataIndex() );
    return 0;
}

/// returns true if node is document's root
bool ldomNode::isRoot() const
{
    ASSERT_NODE_NOT_NULL;
    switch ( TNTYPE ) {
    case NT_ELEMENT:
        return !_data._elem_ptr->_parentNode;
    case NT_PELEMENT:   // immutable (persistent) element node
        {
             ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
             return me->parentIndex==0;
        }
        break;
    case NT_PTEXT:      // immutable (persistent) text node
        {
            return getCrDom()->_textStorage.getParent( _data._ptext_addr )==0;
        }
    case NT_TEXT:
        return _data._text_ptr->getParentIndex()==0;
    }
    return false;
}

/// call to invalidate cache if persistent node content is modified
void ldomNode::modified()
{
    if ( isPersistent() ) {
        if ( isElement() )
            getCrDom()->_elemStorage.modified( _data._pelem_addr );
        else
            getCrDom()->_textStorage.modified( _data._ptext_addr );
    }
}

/// changes parent of item
void ldomNode::setParentNode( ldomNode * parent )
{
    ASSERT_NODE_NOT_NULL;
#ifdef TRACE_AUTOBOX
    if (getParentNode() != NULL && parent != NULL) {
        CRLog::trace("Changing parent of %d from %d to %d",
                getDataIndex(),
                getParentNode()->getDataIndex(),
                parent->getDataIndex());
    }
#endif
    switch ( TNTYPE ) {
    case NT_ELEMENT:
        _data._elem_ptr->_parentNode = parent;
        break;
    case NT_PELEMENT:   // immutable (persistent) element node
        {
            lUInt32 parentIndex = parent->_handle._dataIndex;
            ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
            if ( me->parentIndex != (int)parentIndex ) {
                me->parentIndex = parentIndex;
                modified();
            }
        }
        break;
    case NT_PTEXT:      // immutable (persistent) text node
        {
            lUInt32 parentIndex = parent->_handle._dataIndex;
            getCrDom()->_textStorage.setParent(_data._ptext_addr, parentIndex);
            //_data._ptext_addr._parentIndex = parentIndex;
            //doc_->_textStorage.setTextParent( _data._ptext_addr._addr, parentIndex );
        }
        break;
    case NT_TEXT:
        {
            lUInt32 parentIndex = parent->_handle._dataIndex;
            _data._text_ptr->setParentIndex( parentIndex );
        }
        break;
    }
}

/// returns dataIndex of node's parent, 0 if no parent
int ldomNode::getParentIndex() const
{
    ASSERT_NODE_NOT_NULL;
    switch ( TNTYPE ) {
    case NT_ELEMENT:
        return _data._elem_ptr->_parentNode ? _data._elem_ptr->_parentNode->getDataIndex() : 0;
    case NT_PELEMENT:
        {
            // immutable (persistent) element node
            ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
            return me->parentIndex;
        }
        break;
    case NT_PTEXT:
        // immutable (persistent) text node
        return getCrDom()->_textStorage.getParent(_data._ptext_addr);
    case NT_TEXT:
        return _data._text_ptr->getParentIndex();
    }
    return 0;
}

/// returns pointer to parent node, NULL if node has no parent
ldomNode * ldomNode::getParentNode() const
{
    ASSERT_NODE_NOT_NULL;
    int parentIndex = 0;
    switch ( TNTYPE ) {
    case NT_ELEMENT:
        return _data._elem_ptr->_parentNode;
    case NT_PELEMENT:
        {
            // immutable (persistent) element node
            ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
            parentIndex = me->parentIndex;
        }
        break;
    case NT_PTEXT:
        // immutable (persistent) text node
        parentIndex = getCrDom()->_textStorage.getParent(_data._ptext_addr);
        break;
    case NT_TEXT:
        parentIndex = _data._text_ptr->getParentIndex();
        break;
    }
    return parentIndex ? getTinyNode(parentIndex) : NULL;
}

//returns specified parent node from ancestor nodes. Returns NULL if not found.
ldomNode * ldomNode::getParentNode(const char * name)
{
    if(this->isNodeName(name))
        return this;
    ldomNode* node = this;
    while (node->getParentNode()!=NULL)
    {
        node = node->getParentNode();
        if(node->isNodeName(name))
        {
            return node;
        }
    }
    return NULL;
}

/// returns true child node is element
bool ldomNode::isChildNodeElement( lUInt32 index ) const
{
    ASSERT_NODE_NOT_NULL;
    if ( !isPersistent() ) {
        // element
        tinyElement * me = _data._elem_ptr;
        int n = me->_children[index];
        return ( (n & 1)==1 );
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        int n = me->children[index];
        return ( (n & 1)==1 );
    }
}

/// returns true child node is text
bool ldomNode::isChildNodeText( lUInt32 index ) const
{
    ASSERT_NODE_NOT_NULL;
    if ( !isPersistent() ) {
        // element
        tinyElement * me = _data._elem_ptr;
        int n = me->_children[index];
        return ( (n & 1)==0 );
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        int n = me->children[index];
        return ( (n & 1)==0 );
    }
}

/// returns child node by index, NULL if node with this index is not element
/// or nodeTag!=0 and element node name!=nodeTag
ldomNode * ldomNode::getChildElementNode( lUInt32 index, const lChar16 * nodeTag ) const
{
    lUInt16 nodeId = getCrDom()->getElementNameIndex(nodeTag);
    return getChildElementNode( index, nodeId );
}

/// returns child node by index, NULL if node with this index is not element
/// or nodeId!=0 and element node id!=nodeId
ldomNode * ldomNode::getChildElementNode( lUInt32 index, lUInt16 nodeId ) const
{
    ASSERT_NODE_NOT_NULL;
    ldomNode * res = NULL;
    if ( !isPersistent() ) {
        // element
        tinyElement * me = _data._elem_ptr;
        int n = me->_children[index];
        if ( (n & 1)==0 ) // not element
            return NULL;
        res = getTinyNode( n );
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        int n = me->children[index];
        if ( (n & 1)==0 ) // not element
            return NULL;
        res = getTinyNode( n );
    }
    if ( res && nodeId!=0 && res->getNodeId()!=nodeId )
        res = NULL;
    return res;
}

/// returns child node by index
ldomNode * ldomNode::getChildNode( lUInt32 index ) const
{
    ASSERT_NODE_NOT_NULL;
    if ( !isPersistent() ) {
        // element
        tinyElement * me = _data._elem_ptr;
        return getTinyNode( me->_children[index] );
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        return getTinyNode( me->children[index] );
    }
}

/// returns element child count
int ldomNode::getChildCount() const
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return 0;
    if ( !isPersistent() ) {
        // element
        tinyElement * me = _data._elem_ptr;
        return me->_children.length();
    } else {
        // persistent element
        {
            ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
            //if ( me==NULL ) { // DEBUG
            //    me = doc_->_elemStorage.getElem( _data._pelem_addr );
            //}
            return me->childCount;
        }
    }
}

/// returns element attribute count
int ldomNode::getAttrCount() const
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return 0;
    if ( !isPersistent() ) {
        // element
        tinyElement * me = _data._elem_ptr;
        return me->_attrs.length();
    } else {
        // persistent element
        {
            ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
            return me->attrCount;
        }
    }
}

bool ldomNode::isRTL()
{
    if (this == NULL)
    {
        return false;
    }
    ldomNode* node = this;
    while (node->getParentNode() != NULL)
    {
        if (node->getAttributeValue("dir") == "rtl" || node->getAttributeValue("class") == "rtl")
        {
            return true;
        }
        node = node->getParentNode();
    }
    return false;
}

/// returns attribute value by attribute name id and namespace id
const lString16 & ldomNode::getAttributeValue( lUInt16 nsid, lUInt16 id ) const
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return lString16::empty_str;
    if ( !isPersistent() ) {
        // element
        tinyElement * me = _data._elem_ptr;
        lUInt16 valueId = me->_attrs.get( nsid, id );
        if ( valueId==LXML_ATTR_VALUE_NONE )
            return lString16::empty_str;
        return getCrDom()->getAttrValue(valueId);
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        lUInt16 valueId = me->getAttrValueId( nsid, id );
        if ( valueId==LXML_ATTR_VALUE_NONE )
            return lString16::empty_str;
        return getCrDom()->getAttrValue(valueId);
    }
}

/// returns attribute value by attribute name and namespace
const lString16& ldomNode::getAttributeValue(const lChar16* nsName, const lChar16* attrName) const
{
    ASSERT_NODE_NOT_NULL;
    lUInt16 nsId = (nsName && nsName[0]) ? getCrDom()->getNsNameIndex(nsName) : LXML_NS_ANY;
    lUInt16 attrId = getCrDom()->getAttrNameIndex(attrName);
    return getAttributeValue(nsId, attrId);
}

/// returns attribute value by attribute name and namespace
const lString16& ldomNode::getAttributeValue(const lChar8* nsName, const lChar8* attrName) const
{
    ASSERT_NODE_NOT_NULL;
    lUInt16 nsId = (nsName && nsName[0]) ? getCrDom()->getNsNameIndex(nsName) : LXML_NS_ANY;
    lUInt16 attrId = getCrDom()->getAttrNameIndex(attrName);
    return getAttributeValue(nsId, attrId);
}

/// returns attribute by index
const lxmlAttribute * ldomNode::getAttribute( lUInt32 index ) const
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return NULL;
    if ( !isPersistent() ) {
        // element
        tinyElement * me = _data._elem_ptr;
        return me->_attrs[index];
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        return me->attr( index );
    }
}

/// returns true if element node has attribute with specified name id and namespace id
bool ldomNode::hasAttribute( lUInt16 nsid, lUInt16 id ) const
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return false;
    if ( !isPersistent() ) {
        // element
        tinyElement * me = _data._elem_ptr;
        lUInt16 valueId = me->_attrs.get( nsid, id );
        return ( valueId!=LXML_ATTR_VALUE_NONE );
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        return (me->findAttr( nsid, id ) != NULL);
    }
}

const lString16 & ldomNode::getAttributeName(lUInt32 index) const
{
    ASSERT_NODE_NOT_NULL;
    const lxmlAttribute * attr = getAttribute( index );
    if ( attr )
        return getCrDom()->getAttrName( attr->id );
    return lString16::empty_str;
}


void ldomNode::setAttribute(const lChar16* nsname, const lChar16* attrname, const lChar16* attr_val)
{
    lUInt16 attr_ns = (nsname && nsname[0]) ? getCrDom()->getNsNameIndex(nsname) : 0;
    lUInt16 attr_id = (attrname && attrname[0]) ? getCrDom()->getAttrNameIndex(attrname) : 0;
    setAttributeValue(attr_ns, attr_id, attr_val);
}

void ldomNode::setAttributeValue(lUInt16 nsid, lUInt16 id, const lChar16 * value)
{
    ASSERT_NODE_NOT_NULL;
    if (!isElement())
        return;
    lUInt16 valueIndex = getCrDom()->getAttrValueIndex(value);
    if (isPersistent()) {
        // persistent element
        ElementDataStorageItem* me = getCrDom()->_elemStorage.getElem(_data._pelem_addr);
        lxmlAttribute* attr = me->findAttr(nsid, id);
        if (attr) {
            attr->index = valueIndex;
            modified();
            return;
        }
        // else: convert to modifable and continue as non-persistent
        modify();
    }
    // element
    tinyElement* me = _data._elem_ptr;
    me->_attrs.set(nsid, id, valueIndex);
    if (nsid == LXML_NS_NONE)
        getCrDom()->onAttributeSet(id, valueIndex, this);
}

/// returns element type structure pointer if it was set in document for this element name
const css_elem_def_props_t * ldomNode::getElementTypePtr()
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return NULL;
    if ( !isPersistent() ) {
        // element
        const css_elem_def_props_t * res = getCrDom()->getElementTypePtr(_data._elem_ptr->_id);
//        if ( res && res->is_object ) {
//            CRLog::trace("Object found");
//        }
        return res;
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        const css_elem_def_props_t * res = getCrDom()->getElementTypePtr(me->id);
//        if ( res && res->is_object ) {
//            CRLog::trace("Object found");
//        }
        return res;
    }
}

/// returns element name id
lUInt16 ldomNode::getNodeId() const
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return 0;
    if ( !isPersistent() ) {
        // element
        return _data._elem_ptr->_id;
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        return me->id;
    }
}

/// returns element namespace id
lUInt16 ldomNode::getNodeNsId() const
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return 0;
    if ( !isPersistent() ) {
        // element
        return _data._elem_ptr->_nsid;
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        return me->nsid;
    }
}

/// replace element name id with another value
void ldomNode::setNodeId( lUInt16 id )
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return;
    if ( !isPersistent() ) {
        // element
        _data._elem_ptr->_id = id;
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        me->id = id;
        modified();
    }
}

/// returns element name
const lString16 & ldomNode::getNodeName() const
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return lString16::empty_str;
    if ( !isPersistent() ) {
        // element
        return getCrDom()->getElementName(_data._elem_ptr->_id);
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        return getCrDom()->getElementName(me->id);
    }
}

/// returns element name
bool ldomNode::isNodeName(const char * s) const
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return false;
    lUInt16 index = getCrDom()->findElementNameIndex(s);
    if (!index)
        return false;
    if ( !isPersistent() ) {
        // element
        return index == _data._elem_ptr->_id;
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        return index == me->id;
    }
}

/// returns element namespace name
const lString16 & ldomNode::getNodeNsName() const
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return lString16::empty_str;
    if ( !isPersistent() ) {
        // element
        return getCrDom()->getNsName(_data._elem_ptr->_nsid);
    } else {
        // persistent element
        ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
        return getCrDom()->getNsName(me->nsid);
    }
}

class DomBoundlString16 {
public:
    lString16 string_;
    LVArray<ldomNode*> pointers_;

    DomBoundlString16()
    {
        string_ = lString16::empty_str;
        pointers_.clear();
    }

    DomBoundlString16(ldomNode* node)
    {
        string_ = node->getText();
        for (int i = 0; i < string_.length(); i++)
        {
            pointers_.add(node);
        }
    }
    DomBoundlString16 & operator += ( DomBoundlString16 s ) { string_.append(s.string_); pointers_.add(s.pointers_); return *this;}
    int length() { return string_.length();}

    void printOut()
    {
        for (int i = 0; i < string_.length(); i++)
        {
            CRLog::error("[l %lc l] (0x%4x) ={ %s }",string_.at(i),string_.at(i),LCSTR(pointers_.get(i)->getXPath()));
        }
    }

    void applyNewString()
    {
        ldomNode* lastnode = NULL;
        lString16 orig_text;
        int pos = 0;
        for (int i = 0; i < pointers_.length()  ; i++)
        {
            ldomNode * node = pointers_.get(i);
            if(node == lastnode)
            {
                lChar16 ch = string_.at(i);
                lString16 chs(&ch,1);
                orig_text.replace(pos,1, chs);
                pos++;
            }
            else
            {
                if(lastnode!=NULL)
                {
                    lastnode->setText(orig_text);
                }

                lastnode = node;
                pos = 0;
                orig_text = node->getText();

                lChar16 ch = string_.at(i);
                lString16 chs(&ch,1);
                orig_text.replace(pos,1, chs);
                pos++;

            }
        }
        if(lastnode!=NULL && !orig_text.empty())
        {
            lastnode->setText(orig_text);
        }
    }

    bool checkNodesForRTL()
    {
        for (int i = 0; i < pointers_.length(); i++)
        {
            if(pointers_.get(i)->isRTL())
            {
                return true;
            }
        }
        return false;
    }

    void lamAlephFilter()
    {
        for (int i = 0; i < string_.length() ; i++)
        {
            lChar16 ch = string_.at(i);
            bool lamAlefFound = (ch == 0xFEFA || ch == 0xFEFB ||
                                 ch == 0xFEFC || ch == 0xFEF5 ||
                                 ch == 0xFEF6 || ch == 0xFEF7 ||
                                 ch == 0xFEF8 || ch == 0xFEF9) ;
            if(lamAlefFound)
            {
                pointers_.remove(i);
            }
        }
    }

    /*
    void PrepareIndic()
    {
        if(string_.length()<=1)
        {
            return;
        }
        //LE("in =  [%s]",LCSTR(in));
        lString16 result;

        //dvngLig reph("0x0930 0x094D.rphf");
        //lChar16 reph_unicode = findDvngLigRev(reph);
        lChar16 reph_unicode = 0xE02E;

        //dvngLig eyelash("0x0930 0x094D 0x200D.half");
        //lChar16 eyelash_unicode = findDvngLigRev(eyelash);
        lChar16 eyelash_unicode = 0xE04A;

        lString16Collection words;
        words.parse(string_,L" ",true);
        for (int w = 0; w < words.length(); w++)
        {
            lString16 word = words.at(w);

            for (int i = 0; i < word.length(); i++)
            {


                //if (i < word.length() - 3)
                //{
                    //ra E02E reph (ra + virama + consonant)
                    //NOW PROCESSED IN LvXmlParser::processIndicLigatures(); // NO SUCH METHOD
                    //*if (reph_unicode != 0 && word[i] == 0x930 && word[i + 1] == 0x94d)
                    //*{
                    //*    if (isDevanagari_consonant(word[i + 2]))
                    //*    {
                    //*        LE("reph");
                    //*        //reph
                    //*        word[i]   = word[i + 2];
                    //*        word[i+1] = reph_unicode;
                    //*        word.erase(i+2);
                    //*        pointers_.remove(i+2);
                    //*    }
                    //*}

                    //ra 0930 eyelash (ra + virama + ZWNJ + consonant)
                    //NOW PROCESSED IN LvXmlParser::processDvngLigatures();
                    //if (eyelash_unicode != 0 && word[i] == 0x0930 && word[i + 1] == 0x094d && word[i + 2] == 0x200d)
                    //{
                    //    if (isDevanagari_consonant(word[i + 3]))
                    //    {
                    //        word[i]   = eyelash_unicode;
                    //        word.erase(i+1);
                    //        pointers_.remove(i+1);
                    //    }
                    //}

                    //ra 0930 rakar ( consonant + virama + ra ) IS TAKEN CARE OF IN LIGATURE MODE
                //}

                //rra 0x0931 eyelash (0x0931 + consonant)
                if (i < word.length() - 2)
                {
                    if (eyelash_unicode != 0 && word[i] == 0x0931)
                    {
                        if (isDevanagari_consonant(word[i + 1]))
                        {
                            //eyelash
                            word[i] = eyelash_unicode;
                            //LE("rra eyelash");
                        }
                    }

                    // letter "I" + Nukta forms letter vocalic "L"
                    if (word[i] == 0x907)
                    {
                        if (word[i + 1] == 0x93c)
                        {
                            word[i] = 0x90c;
                            word.erase(i + 1);
                            pointers_.remove(i+1);
                        }
                    }

                    // vowel sign vocalic "R" + sign Nukta forms vowel sign vocalic "Rr"
                    if (word[i] == 0x943)
                    {
                        if (word[i + 1] == 0x93c)
                        {
                            word[i] = 0x944;
                            word.erase(i + 1);
                            pointers_.remove(i+1);
                        }
                    }

                    // Candrabindu + sign Nukta forms Om
                    if (word[i] == 0x901)
                    {
                        if (word[i + 1] == 0x93c)
                        {
                            word[i] = 0x950;
                            word.erase(i + 1);
                            pointers_.remove(i+1);
                        }
                    }

                    // letter vocalic "R" + sign Nukta forms letter vocalic "Rr"
                    if (word[i] == 0x90b)
                    {
                        if (word[i + 1] == 0x93c)
                        {
                            word[i] = 0x960;
                            word.erase(i + 1);
                            pointers_.remove(i+1);
                        }
                    }

                    // letter "Ii" + sign Nukta forms letter vocalic "LI"
                    if (word[i] == 0x908)
                    {
                        if (word[i + 1] == 0x93c)
                        {
                            word[i] = 0x961;
                            word.erase(i + 1);
                            pointers_.remove(i+1);
                        }
                    }

                    // vowel sign "I" + sign Nukta forms vowel sign vocalic "L"
                    if (word[i] == 0x93f)
                    {
                        if (word[i + 1] == 0x93c)
                        {
                            word[i] = 0x962;
                            word.erase(i + 1);
                            pointers_.remove(i+1);
                        }
                    }

                    // vowel sign "Ii" + sign Nukta forms vowel sign vocalic "LI"
                    if (word[i] == 0x940)
                    {
                        if (word[i + 1] == 0x93c)
                        {
                            word[i] = 0x963;
                            word.erase(i + 1);
                            pointers_.remove(i+1);
                        }
                    }

                    // Danda + sign Nukta forms sign Avagraha
                    if (word[i] == 0x964)
                    {
                        if (word[i + 1] == 0x93c)
                        {
                            word[i] = 0x93d;
                            word.erase(i + 1);
                            pointers_.remove(i+1);
                        }
                    }
                }

                // interchange the order of "i" vowel
                if (i >= 1 && word[i] == 0x93f)
                {
                    if(word[i-1] == 0xE02E) //ra reph form + i fix
                    {
                        //LE("ra reph form + i");
                        word[i]     = word[i - 1];
                        word[i - 1] = word[i - 2];
                        word[i - 2] = 0x93f;
                    }
                    else
                    {
                        //LE("i regular");
                        word[i] = word[i - 1];
                        word[i - 1] = 0x93f;
                    }

                }
            }
            result += word;
            result += ' ';
        }
        result = result.substr(0, result.length()-1);
        //LE("out = [%s]",LCSTR(result));

        this->string_ = result;
    }
     */
};

/// returns text node text as wide string
DomBoundlString16 ldomNode::getText_bound(int max_size)
{
    ASSERT_NODE_NOT_NULL;
    switch (TNTYPE)
    {
        case NT_PELEMENT:
        case NT_ELEMENT:
        {       DomBoundlString16 txt;
            int child_count = getChildCount();
            for (unsigned i = 0; i < child_count; i++)
            {
                ldomNode *child = getChildNode(i);
                txt += child->getText_bound(max_size);
                if (max_size != 0 && txt.length() > max_size)
                {
                    break;
                }
                if (i >= child_count - 1)
                {
                    break;
                }
            }
            return txt;
        }
            break;
        case NT_PTEXT:
        case NT_TEXT:
            return DomBoundlString16(this);
        default:
            return DomBoundlString16();
    }
    return DomBoundlString16();
}

/// returns text node text as wide string
lString16 ldomNode::getText(lChar16 block_delimiter, int max_size) const
{
    ASSERT_NODE_NOT_NULL;
    switch (TNTYPE) {
    case NT_PELEMENT:
    case NT_ELEMENT:
        {
            lString16 txt;
            int child_count = getChildCount();
            for (unsigned i = 0; i < child_count; i++) {
                ldomNode* child = getChildNode(i);
                txt += child->getText(block_delimiter, max_size);
                if (max_size != 0 && txt.length() > max_size)
                    break;
                if (i >= child_count - 1)
                    break;
                if (block_delimiter && child->isElement()) {
                    if (!child->getStyle().isNull() && child->getStyle()->display == css_d_block)
                        txt << block_delimiter;
                }
            }
            return txt;
        }
        break;
    case NT_PTEXT:
        return Utf8ToUnicode(getCrDom()->_textStorage.getText(_data._ptext_addr));
    case NT_TEXT:
        return _data._text_ptr->getText16();
    }
    return lString16::empty_str;
}

/// returns text node text as utf8 string
lString8 ldomNode::getText8( lChar8 blockDelimiter, int maxSize ) const
{
    ASSERT_NODE_NOT_NULL;
    switch ( TNTYPE ) {
    case NT_ELEMENT:
    case NT_PELEMENT:
        {
            lString8 txt;
            int cc = getChildCount();
            for (int i = 0; i < cc; i++) {
                ldomNode * child = getChildNode(i);
                txt += child->getText8(blockDelimiter, maxSize);
                if (maxSize != 0 && txt.length() > maxSize)
                    break;
                if (i >= getChildCount() - 1)
                    break;
                if ( blockDelimiter && child->isElement() ) {
                    if ( child->getStyle()->display == css_d_block )
                        txt << blockDelimiter;
                }
            }
            return txt;
        }
        break;
    case NT_PTEXT:
        return getCrDom()->_textStorage.getText( _data._ptext_addr );
    case NT_TEXT:
        return _data._text_ptr->getText();
    }
    return lString8::empty_str;
}

/// sets text node text as wide string
void ldomNode::setText( lString16 str )
{
    ASSERT_NODE_NOT_NULL;
    switch ( TNTYPE ) {
    case NT_ELEMENT:
        readOnlyError();
        break;
    case NT_PELEMENT:
        readOnlyError();
        break;
    case NT_PTEXT:
        {
            // convert persistent text to mutable
            lUInt32 parentIndex = getCrDom()->_textStorage.getParent(_data._ptext_addr);
            getCrDom()->_textStorage.freeNode( _data._ptext_addr );
            _data._text_ptr = new ldomTextNode( parentIndex, UnicodeToUtf8(str) );
            // change type from PTEXT to TEXT
            _handle._dataIndex = (_handle._dataIndex & ~0xF) | NT_TEXT;
        }
        break;
    case NT_TEXT:
        {
            _data._text_ptr->setText( str );
        }
        break;
    }
}

/// sets text node text as utf8 string
void ldomNode::setText8( lString8 utf8 )
{
    ASSERT_NODE_NOT_NULL;
    switch ( TNTYPE ) {
    case NT_ELEMENT:
        readOnlyError();
        break;
    case NT_PELEMENT:
        readOnlyError();
        break;
    case NT_PTEXT:
        {
            // convert persistent text to mutable
            lUInt32 parentIndex = getCrDom()->_textStorage.getParent(_data._ptext_addr);
            getCrDom()->_textStorage.freeNode( _data._ptext_addr );
            _data._text_ptr = new ldomTextNode( parentIndex, utf8 );
            // change type from PTEXT to TEXT
            _handle._dataIndex = (_handle._dataIndex & ~0xF) | NT_TEXT;
        }
        break;
    case NT_TEXT:
        {
            _data._text_ptr->setText( utf8 );
        }
        break;
    }
}

/// returns node absolute rectangle
void ldomNode::getAbsRect( lvRect & rect )
{
    ASSERT_NODE_NOT_NULL;
    ldomNode * node = this;
    RenderRectAccessor fmt( node );
    rect.left = fmt.getX();
    rect.top = fmt.getY();
    rect.right = fmt.getWidth();
    rect.bottom = fmt.getHeight();
    node = node->getParentNode();
    for (; node; node = node->getParentNode())
    {
        RenderRectAccessor fmt( node );
        rect.left += fmt.getX();
        rect.top += fmt.getY();
    }
    rect.bottom += rect.top;
    rect.right += rect.left;
}

/// returns render data structure
void ldomNode::getRenderData( lvdomElementFormatRec & dst)
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() ) {
        dst.clear();
        return;
    }
    getCrDom()->_rectStorage.getRendRectData(_handle._dataIndex, &dst);
}

/// sets new value for render data structure
void ldomNode::setRenderData( lvdomElementFormatRec & newData)
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return;
    getCrDom()->_rectStorage.setRendRectData(_handle._dataIndex, &newData);
}

/// sets node rendering structure pointer
void ldomNode::clearRenderData()
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return;
    lvdomElementFormatRec rec;
    getCrDom()->_rectStorage.setRendRectData(_handle._dataIndex, &rec);
}

/// calls specified function recursively for all elements of DOM tree, children before parent
void ldomNode::recurseElementsDeepFirst( void (*pFun)( ldomNode * node ) )
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return;
    int cnt = getChildCount();
    for (int i=0; i<cnt; i++)
    {
        ldomNode * child = getChildNode( i );
        if ( child->isElement() )
        {
            child->recurseElementsDeepFirst( pFun );
        }
    }
    pFun( this );
}

static void updateRendMethod( ldomNode * node )
{
    node->initNodeRendMethod();
}

/// init render method for the whole subtree
void ldomNode::initNodeRendMethodRecursive()
{
    recurseElementsDeepFirst( updateRendMethod );
}

static void updateStyleDataRecursive(ldomNode* node)
{
    if (!node->isElement()) {
        return;
    }
    bool styleSheetChanged = false;
    if (node->getNodeId() == el_DocFragment) {
        styleSheetChanged = node->applyNodeStylesheet();
    }
    node->initNodeStyle();
    int n = node->getChildCount();
    for (int i = 0; i < n; i++) {
        ldomNode* child = node->getChildNode(i);
        if (child->isElement()) {
            updateStyleDataRecursive(child);
        }
    }
    if (styleSheetChanged) {
        node->getCrDom()->getStylesheet()->pop();
    }
}

/// init render method for the whole subtree
void ldomNode::initNodeStyleRecursive()
{
    getCrDom()->_fontMap.clear();
    updateStyleDataRecursive( this );
    //recurseElements( updateStyleData );
}

/// calls specified function recursively for all elements of DOM tree
void ldomNode::recurseElements( void (*pFun)( ldomNode * node ) )
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return;
    pFun( this );
    int cnt = getChildCount();
    for (int i=0; i<cnt; i++)
    {
        ldomNode * child = getChildNode( i );
        if ( child->isElement() )
        {
            child->recurseElements( pFun );
        }
    }
}

/// calls specified function recursively for all nodes of DOM tree
void ldomNode::recurseNodes( void (*pFun)( ldomNode * node ) )
{
    ASSERT_NODE_NOT_NULL;
    pFun( this );
    if ( isElement() )
    {
        int cnt = getChildCount();
        for (int i=0; i<cnt; i++)
        {
            ldomNode * child = getChildNode( i );
            child->recurseNodes( pFun );
        }
    }
}

/// returns first text child element
ldomNode * ldomNode::getFirstTextChild(bool skipEmpty)
{
    ASSERT_NODE_NOT_NULL;
    if ( isText() ) {
        if ( !skipEmpty )
            return this;
        lString16 txt = getText();
        bool nonSpaceFound = false;
        for ( int i=0; i<txt.length(); i++ ) {
            lChar16 ch = txt[i];
            if ( ch!=' ' && ch!='\t' && ch!='\r' && ch!='\n' ) {
                nonSpaceFound = true;
                break;
            }
        }
        if ( nonSpaceFound )
            return this;
        return NULL;
    }
    for ( int i=0; i<(int)getChildCount(); i++ ) {
        ldomNode * p = getChildNode(i)->getFirstTextChild(skipEmpty);
        if (p)
            return p;
    }
    return NULL;
}

/// returns last text child element
ldomNode * ldomNode::getLastTextChild()
{
    ASSERT_NODE_NOT_NULL;
    if ( isText() )
        return this;
    else {
        for ( int i=(int)getChildCount()-1; i>=0; i-- ) {
            ldomNode * p = getChildNode(i)->getLastTextChild();
            if (p)
                return p;
        }
    }
    return NULL;
}

/// find node by coordinates of point in formatted document
ldomNode * ldomNode::elementFromPoint( lvPoint pt, int direction )
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return NULL;
    ldomNode * enode = this;
    RenderRectAccessor fmt( this );
    if ( enode->getRendMethod() == erm_invisible ) {
        return NULL;
    }
    if ( pt.y < fmt.getY() ) {
        if ( direction>0 && enode->getRendMethod() == erm_final )
            return this;
        return NULL;
    }
    if ( pt.y >= fmt.getY() + fmt.getHeight() ) {
        if ( direction<0 && enode->getRendMethod() == erm_final )
            return this;
        return NULL;
    }
    if ( enode->getRendMethod() == erm_final ) {
        return this;
    }
    int count = getChildCount();
    if ( direction>=0 ) {
        for ( int i=0; i<count; i++ ) {
            ldomNode * p = getChildNode( i );
            ldomNode * e = p->elementFromPoint( lvPoint( pt.x - fmt.getX(),
                    pt.y - fmt.getY() ), direction );
            if ( e )
                return e;
        }
    } else {
        for ( int i=count-1; i>=0; i-- ) {
            ldomNode * p = getChildNode( i );
            ldomNode * e = p->elementFromPoint( lvPoint( pt.x - fmt.getX(),
                    pt.y - fmt.getY() ), direction );
            if ( e )
                return e;
        }
    }
    return this;
}

/// find final node by coordinates of point in formatted document
ldomNode * ldomNode::finalBlockFromPoint( lvPoint pt )
{
    ASSERT_NODE_NOT_NULL;
    ldomNode * elem = elementFromPoint( pt, 0 );
    if ( elem && elem->getRendMethod() == erm_final )
        return elem;
    return NULL;
}

/// returns rendering method
lvdom_element_render_method ldomNode::getRendMethod()
{
    ASSERT_NODE_NOT_NULL;
    if ( isElement() ) {
        if ( !isPersistent() ) {
            return _data._elem_ptr->_rendMethod;
        } else {
            ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
            return (lvdom_element_render_method)me->rendMethod;
        }
    }
    return erm_invisible;
}

/// sets rendering method
void ldomNode::setRendMethod( lvdom_element_render_method method )
{
    ASSERT_NODE_NOT_NULL;
    if ( isElement() ) {
        if ( !isPersistent() ) {
            _data._elem_ptr->_rendMethod = method;
        } else {
            ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
            if ( me->rendMethod != method ) {
                me->rendMethod = (lUInt8)method;
                modified();
            }
        }
    }
}

/// returns element style record
css_style_ref_t ldomNode::getStyle()
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return css_style_ref_t();
    css_style_ref_t res = getCrDom()->getNodeStyle( _handle._dataIndex );
    if(res.isNull())
    {
        css_style_rec_t * empty = new css_style_rec_t;
        return css_style_ref_t(empty);
    }
    return res;
}

/// returns element font
font_ref_t ldomNode::getFont()
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return font_ref_t();
    return getCrDom()->getNodeFont( _handle._dataIndex );
}

/// sets element font
void ldomNode::setFont( font_ref_t font )
{
    ASSERT_NODE_NOT_NULL;
    if  ( isElement() ) {
        getCrDom()->setNodeFont( _handle._dataIndex, font );
    }
}

/// sets element style record
void ldomNode::setStyle( css_style_ref_t & style )
{
    ASSERT_NODE_NOT_NULL;
    if  ( isElement() ) {
        getCrDom()->setNodeStyle( _handle._dataIndex, style );
    }
}

bool ldomNode::initNodeFont()
{
    if ( !isElement() )
        return false;
    lUInt16 style = getCrDom()->getNodeStyleIndex( _handle._dataIndex );
    lUInt16 font = getCrDom()->getNodeFontIndex( _handle._dataIndex );
    lUInt16 fntIndex = getCrDom()->_fontMap.get( style );
    if ( fntIndex==0 ) {
        css_style_ref_t s = getCrDom()->_styles.get( style );
        if ( s.isNull() ) {
            CRLog::error("style not found for index %d", style);
            s = getCrDom()->_styles.get( style );
        }
        LVFontRef fnt = ::getFont(s.get(), getCrDom()->getFontContextDocIndex());
        fntIndex = (lUInt16) getCrDom()->_fonts.cache( fnt );
        if ( fnt.isNull() ) {
            CRLog::error("font not found for style!");
            return false;
        } else {
            getCrDom()->_fontMap.set(style, fntIndex);
        }
        if ( font != 0 ) {
            if ( font!=fntIndex ) // ???
                getCrDom()->_fonts.release(font);
        }
        getCrDom()->setNodeFontIndex( _handle._dataIndex, fntIndex);
        return true;
    } else {
        if ( font!=fntIndex )
            getCrDom()->_fonts.addIndexRef( fntIndex );
    }
    if ( fntIndex<=0 ) {
        CRLog::error("font caching failed for style!");
        return false;;
    } else {
        getCrDom()->setNodeFontIndex( _handle._dataIndex, fntIndex);
    }
    return true;
}

void ldomNode::initNodeStyle()
{
    // Assume all parent styles already initialized
    if (!getCrDom()->isDefStyleSet() || !isElement()) {
        return;
    }
    if (isRoot() || getParentNode()->isRoot()) {
        setNodeStyleRend(this, getCrDom()->getDefaultStyle(), getCrDom()->getDefaultFont());
    } else {
        ldomNode* parent = getParentNode();
#ifdef OREDEBUG
        if (parent->getChildIndex(getDataIndex()) < 0) {
            CRLog::error("Invalid parent->child relation for nodes %d->%d",
                         parent->getDataIndex(), getDataIndex());
        }
#endif
        css_style_ref_t style = parent->getStyle();
        LVFontRef font = parent->getFont();
        int counter = 0;
        while (style.isNull() && parent->getParentNode()!=NULL && counter < MAX_DOM_LEVEL)
        {
            parent = parent->getParentNode();
            style = parent->getStyle();
        }
        setNodeStyleRend(this, style, font);
    }
}

/// for display:list-item node, get marker
bool ldomNode::getNodeListMarker( int & counterValue, lString16 & marker, int & markerWidth )
{
    css_style_ref_t s = getStyle();
    marker.clear();
    markerWidth = 0;
    if ( s.isNull() )
        return false;
    css_list_style_type_t st = s->list_style_type;
    switch ( st ) {
    default:
        // treat default as disc
    case css_lst_disc:
        marker = L"\x2022"; //L"\x25CF"; // 25e6
        break;
    case css_lst_circle:
        marker = L"\x2022"; //L"\x25CB";
        break;
    case css_lst_square:
        marker = L"\x25A0";
        break;
    case css_lst_decimal:
    case css_lst_lower_roman:
    case css_lst_upper_roman:
    case css_lst_lower_alpha:
    case css_lst_upper_alpha:
        if ( counterValue<=0 ) {
            // calculate counter
            ldomNode * parent = getParentNode();
            counterValue = 0;
            for (int i = 0; i < parent->getChildCount(); i++) {
                ldomNode * child = parent->getChildNode(i);
                css_style_ref_t cs = child->getStyle();
                if ( cs.isNull() )
                    continue;
                switch ( cs->list_style_type ) {
                case css_lst_decimal:
                case css_lst_lower_roman:
                case css_lst_upper_roman:
                case css_lst_lower_alpha:
                case css_lst_upper_alpha:
                    counterValue++;
                    break;
                default:
                    // do nothing
                    ;
                }
                if ( child==this )
                    break;
            }
        } else {
            counterValue++;
        }
            static const char* lower_roman[] = {"i",
                                                "ii",
                                                "iii",
                                                "iv",
                                                "v",
                                                "vi",
                                                "vii",
                                                "viii",
                                                "ix",
                                                "x",
                                                "xi",
                                                "xii",
                                                "xiii",
                                                "xiv",
                                                "xv",
                                                "xvi",
                                                "xvii",
                                                "xviii",
                                                "xix",
                                                "xx",
                                                "xxi",
                                                "xxii",
                                                "xxiii"};
        if (counterValue > 0) {
            switch (st) {
            case css_lst_decimal:
                marker = lString16::itoa(counterValue);
                break;
            case css_lst_lower_roman:
                if (counterValue - 1 < (int)(sizeof(lower_roman) / sizeof(lower_roman[0])))
                    marker = lString16(lower_roman[counterValue-1]);
                else
                    marker = lString16::itoa(counterValue); // fallback to simple counter
                break;
            case css_lst_upper_roman:
                if (counterValue - 1 < (int)(sizeof(lower_roman) / sizeof(lower_roman[0])))
                    marker = lString16(lower_roman[counterValue-1]);
                else
                    marker = lString16::itoa(counterValue); // fallback to simple digital counter
                marker.uppercase();
                break;
            case css_lst_lower_alpha:
                if ( counterValue<=26 )
                    marker.append(1, (lChar16)('a' + counterValue - 1));
                else
                    marker = lString16::itoa(counterValue); // fallback to simple digital counter
                break;
            case css_lst_upper_alpha:
                if ( counterValue<=26 )
                    marker.append(1, (lChar16)('A' + counterValue - 1));
                else
                    marker = lString16::itoa(counterValue); // fallback to simple digital counter
                break;
            case css_lst_disc:
            case css_lst_circle:
            case css_lst_square:
            case css_lst_none:
            case css_lst_inherit:
                // do nothing
                break;
            }
        }
        break;
    }
    bool res = false;
    if (!marker.empty()) {
        LVFont* font = getFont().get();
        if (font) {
            markerWidth = font->getTextWidth((marker + "  ").c_str(),marker.length() + 2) + s->font_size.value / 8;
            res = true;
        } else {
            marker.clear();
        }
    }
    return res;
}

int ldomNode::getListLevel()
{
    int level = 0;
    ldomNode* parent = this;
    while (parent!=NULL)
    {
        if(parent->isNodeName("ul") || parent->isNodeName("ol")|| parent->isNodeName("list"))
        {
            level++;
        }
        parent = parent->getParentNode();
    }
    return level;
}

/// returns first child node
ldomNode * ldomNode::getFirstChild() const
{
    ASSERT_NODE_NOT_NULL;
    if  ( isElement() ) {
        if ( !isPersistent() ) {
            tinyElement * me = _data._elem_ptr;
            if ( me->_children.length() )
                return getCrDom()->getTinyNode(me->_children[0]);
        } else {
            ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
            if ( me->childCount )
                return getCrDom()->getTinyNode(me->children[0]);
        }
    }
    return NULL;
}

/// returns last child node
ldomNode * ldomNode::getLastChild() const
{
    ASSERT_NODE_NOT_NULL;
    if  ( isElement() ) {
        if ( !isPersistent() ) {
            tinyElement * me = _data._elem_ptr;
            if ( me->_children.length() )
                return getCrDom()->getTinyNode(me->_children[me->_children.length()-1]);
        } else {
            ElementDataStorageItem * me = getCrDom()->_elemStorage.getElem( _data._pelem_addr );
            if ( me->childCount )
                return getCrDom()->getTinyNode(me->children[me->childCount-1]);
        }
    }
    return NULL;
}

/// removes and deletes last child element
void ldomNode::removeLastChild()
{
    ASSERT_NODE_NOT_NULL;
    if ( hasChildren() ) {
        ldomNode * lastChild = removeChild( getChildCount() - 1 );
        lastChild->destroy();
    }
}

/// add child
void ldomNode::addChild( lInt32 childNodeIndex )
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return;
    if ( isPersistent() )
        modify(); // convert to mutable element
    tinyElement * me = _data._elem_ptr;
    me->_children.add( childNodeIndex );
}

/// move range of children startChildIndex to endChildIndex inclusively to specified element
void ldomNode::moveItemsTo( ldomNode * destination, int startChildIndex, int endChildIndex )
{
    ASSERT_NODE_NOT_NULL;
    if (!isElement()) {
        return;
    }
    if (isPersistent()) {
        modify();
    }
#ifdef TRACE_AUTOBOX
    CRLog::trace("moveItemsTo() invoked from %d to %d",
            getDataIndex(),
            destination->getDataIndex());
#endif

    int len = endChildIndex - startChildIndex + 1;
    tinyElement * me = _data._elem_ptr;
    for ( int i=0; i<len; i++ ) {
        ldomNode * item = getChildNode( startChildIndex );
        me->_children.remove( startChildIndex ); // + i
        item->setParentNode(destination);
        destination->addChild( item->getDataIndex() );
    }
    // TODO: renumber rest of children in necessary
}

/// find child element by tag id
ldomNode * ldomNode::findChildElement( lUInt16 nsid, lUInt16 id, int index )
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return NULL;
    ldomNode * res = NULL;
    int k = 0;
    int childCount = getChildCount();
    for ( int i=0; i<childCount; i++ )
    {
        ldomNode * p = getChildNode( i );
        if ( !p->isElement() )
            continue;
        if ( p->getNodeId() == id && ( (p->getNodeNsId() == nsid) || (nsid==LXML_NS_ANY) ) )
        {
            if ( k==index || index==-1 ) {
                res = p;
                break;
            }
            k++;
        }
    }
    if (!res) //  || (index==-1 && k>1)  // DON'T CHECK WHETHER OTHER ELEMENTS EXIST
        return NULL;
    return res;
}

/// find child element by id path
ldomNode * ldomNode::findChildElement( lUInt16 idPath[] )
{
    ASSERT_NODE_NOT_NULL;
    if ( !this || !isElement() )
        return NULL;
    ldomNode * elem = this;
    for ( int i=0; idPath[i]; i++ ) {
        elem = elem->findChildElement( LXML_NS_ANY, idPath[i], -1 );
        if ( !elem )
            return NULL;
    }
    return elem;
}

/// inserts child element
ldomNode * ldomNode::insertChildElement( lUInt32 index, lUInt16 nsid, lUInt16 id )
{
    ASSERT_NODE_NOT_NULL;
    if  ( isElement() ) {
        if ( isPersistent() )
            modify();
        tinyElement * me = _data._elem_ptr;
        if (index>(lUInt32)me->_children.length())
            index = me->_children.length();
        ldomNode * node = getCrDom()->allocTinyElement( this, nsid, id );
        me->_children.insert( index, node->getDataIndex() );
        return node;
    }
    readOnlyError();
    return NULL;
}

/// inserts child element
ldomNode * ldomNode::insertChildElement( lUInt16 id )
{
    ASSERT_NODE_NOT_NULL;
    if  ( isElement() ) {
        if ( isPersistent() )
            modify();
        ldomNode * node = getCrDom()->allocTinyElement( this, LXML_NS_NONE, id );
        _data._elem_ptr->_children.insert(
                _data._elem_ptr->_children.length(),
                node->getDataIndex() );
        return node;
    }
    readOnlyError();
    return NULL;
}

/// inserts child text
ldomNode * ldomNode::insertChildText( lUInt32 index, const lString16 & value )
{
    ASSERT_NODE_NOT_NULL;
    if  ( isElement() ) {
        if ( isPersistent() )
            modify();
        tinyElement * me = _data._elem_ptr;
        if (index>(lUInt32)me->_children.length())
            index = me->_children.length();
#if !defined(USE_PERSISTENT_TEXT)
        ldomNode * node = getCrDom()->allocTinyNode( NT_TEXT );
        lString8 s8 = UnicodeToUtf8(value);
        node->_data._text_ptr = new ldomTextNode(_handle._dataIndex, s8);
#else
        ldomNode * node = getCrDom()->allocTinyNode( NT_PTEXT );
        //node->_data._ptext_addr._parentIndex = _handle._dataIndex;
        lString8 s8 = UnicodeToUtf8(value);
        node->_data._ptext_addr = getCrDom()->_textStorage.allocText(
                node->_handle._dataIndex,
                _handle._dataIndex,
                s8 );
#endif
        me->_children.insert( index, node->getDataIndex() );
        return node;
    }
    readOnlyError();
    return NULL;
}

/// inserts child text
ldomNode * ldomNode::insertChildText( const lString16 & value )
{
    ASSERT_NODE_NOT_NULL;
    if  ( isElement() ) {
        if ( isPersistent() )
            modify();
        tinyElement * me = _data._elem_ptr;
#if !defined(USE_PERSISTENT_TEXT)
        ldomNode * node = getCrDom()->allocTinyNode( NT_TEXT );
        lString8 s8 = UnicodeToUtf8(value);
        node->_data._text_ptr = new ldomTextNode(_handle._dataIndex, s8);
#else
        ldomNode* node = getCrDom()->allocTinyNode(NT_PTEXT);
        lString8 s8 = UnicodeToUtf8(value);
        node->_data._ptext_addr = getCrDom()->_textStorage.allocText(
                node->_handle._dataIndex,
                _handle._dataIndex,
                s8);
#endif
        me->_children.insert( me->_children.length(), node->getDataIndex() );
        return node;
    }
    readOnlyError();
    return NULL;
}

/// inserts child text
ldomNode * ldomNode::insertChildText(const lString8 & s8)
{
    ASSERT_NODE_NOT_NULL;
    if  ( isElement() ) {
        if ( isPersistent() )
            modify();
        tinyElement * me = _data._elem_ptr;
#if !defined(USE_PERSISTENT_TEXT)
        ldomNode * node = getCrDom()->allocTinyNode( NT_TEXT );
        node->_data._text_ptr = new ldomTextNode(_handle._dataIndex, s8);
#else
        ldomNode* node = getCrDom()->allocTinyNode(NT_PTEXT);
        node->_data._ptext_addr = getCrDom()->_textStorage.allocText(
                node->_handle._dataIndex,
                _handle._dataIndex,
                s8);
#endif
        me->_children.insert( me->_children.length(), node->getDataIndex() );
        return node;
    }
    readOnlyError();
    return NULL;
}

/// remove child
ldomNode * ldomNode::removeChild( lUInt32 index )
{
    ASSERT_NODE_NOT_NULL;
    if  ( isElement() ) {
        if ( isPersistent() )
            modify();
        lUInt32 removedIndex = _data._elem_ptr->_children.remove(index);
        ldomNode * node = getTinyNode( removedIndex );
        return node;
    }
    readOnlyError();
    return NULL;
}

/// creates stream to read base64 encoded data from element
LVStreamRef ldomNode::createBase64Stream()
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return LVStreamRef();
#define DEBUG_BASE64_IMAGE 0
#if DEBUG_BASE64_IMAGE==1
    lString16 fname = getAttributeValue( attr_id );
    lString8 fname8 = UnicodeToUtf8( fname );
    LVStreamRef ostream = LVOpenFileStream(fname.empty() ? L"image.png" : fname.c_str(),LVOM_WRITE);
    printf("createBase64Stream(%s)\n", fname8.c_str());
#endif
    LVStream * stream = new LVBase64NodeStream( this );
    if ( stream->GetSize()==0 )
    {
#if DEBUG_BASE64_IMAGE==1
        printf("    cannot create base64 decoder stream!!!\n");
#endif
        delete stream;
        return LVStreamRef();
    }
    LVStreamRef istream( stream );
#if DEBUG_BASE64_IMAGE==1
    LVPumpStream( ostream, istream );
    istream->SetPos(0);
#endif
    return istream;
}

lString16 ldomNode::getHRef()
{
    if (isNull())
    {
        return lString16::empty_str;
    }
    ldomNode *node = this;
    while (node && !node->isElement())
        node = node->getParentNode();
    while (node && node->getNodeId() != el_a)
       node = node->getParentNode();

    if (!node)
    {
        return lString16::empty_str;
    }
    lString16 ref = node->getAttributeValue(LXML_NS_ANY, attr_href);
    if (!ref.empty() && ref[0] != '#')
    {
        ref = DecodeHTMLUrlString(ref);
    }
    return ref;
}

class NodeImageProxy : public LVImageSource
{
    ldomNode * _node;
    lString16 _refName;
    int _dx;
    int _dy;
public:
    NodeImageProxy( ldomNode * node, lString16 refName, int dx, int dy )
        : _node(node), _refName(refName), _dx(dx), _dy(dy)
    {

    }

    virtual ldomNode * GetSourceNode()
    {
        return NULL;
    }
    virtual LVStream * GetSourceStream()
    {
        return NULL;
    }

    virtual void   Compact() { }
    virtual int    GetWidth() { return _dx; }
    virtual int    GetHeight() { return _dy; }
    virtual bool   Decode( LVImageDecoderCallback * callback )
    {
        LVImageSourceRef img = _node->getCrDom()->getObjectImageSource(_refName);
        if ( img.isNull() )
            return false;
        return img->Decode(callback);
    }
    virtual ~NodeImageProxy()
    {

    }
};

/// returns object image ref name
lString16 ldomNode::getObjectImageRefName()
{
    if ( !this || !isElement() )
        return lString16::empty_str;
    //printf("ldomElement::getObjectImageSource() ... ");
    const css_elem_def_props_t * et = getCrDom()->getElementTypePtr(getNodeId());
    if (!et || !et->is_object)
        return lString16::empty_str;
    lUInt16 hrefId = getCrDom()->getAttrNameIndex("href");
    lUInt16 srcId = getCrDom()->getAttrNameIndex("src");
    lUInt16 recIndexId = getCrDom()->getAttrNameIndex("recindex");
    lString16 refName = getAttributeValue(getCrDom()->getNsNameIndex("xlink"),
        hrefId );

    if ( refName.empty() )
        refName = getAttributeValue(getCrDom()->getNsNameIndex("l"), hrefId );
    if ( refName.empty())
        refName = getAttributeValue( LXML_NS_ANY, srcId ); //LXML_NS_NONE
    if ( refName.empty() )
        refName = getAttributeValue( LXML_NS_ANY, hrefId ); //LXML_NS_NONE
    if (refName.empty()) {
        lString16 recindex = getAttributeValue( LXML_NS_ANY, recIndexId );
        if (!recindex.empty()) {
            int n;
            if (recindex.atoi(n)) {
                refName = lString16(MOBI_IMAGE_NAME_PREFIX) + fmt::decimal(n);
                //CRLog::trace("get mobi image %s", LCSTR(refName));
            }
        }
//        else {
//            for (int k=0; k<getAttrCount(); k++) {
//                CRLog::trace("attr %s=%s",
//                    LCSTR(getAttributeName(k)),
//                    LCSTR(getAttributeValue(getAttributeName(k).c_str())));
//            }
//        }
    }
    if ( refName.length()<2 )
        return lString16::empty_str;
    refName = DecodeHTMLUrlString(refName);
    return refName;
}

/// returns object video ref name
lString16 ldomNode::getObjectVideoRefName()
{
    if ( !this || !isElement() )
        return lString16::empty_str;

    const css_elem_def_props_t * et = getCrDom()->getElementTypePtr(getNodeId());
    if (!et || !et->is_object)
        return lString16::empty_str;

    lUInt16 posterId = getCrDom()->getAttrNameIndex("poster");
    lUInt16 srcId = getCrDom()->getAttrNameIndex("src");

    lString16 refName = getAttributeValue(getCrDom()->getNsNameIndex("xlink"), posterId );
    if ( refName.empty() )
        refName = getAttributeValue(getCrDom()->getNsNameIndex("l"), posterId );
    if ( refName.empty())
        refName = getAttributeValue( LXML_NS_ANY, posterId ); //LXML_NS_NONE
    if ( refName.empty() )
        refName = getAttributeValue( LXML_NS_ANY, srcId ); //LXML_NS_NONE
    if ( refName.length()<2 )
        return lString16::empty_str;
    refName = DecodeHTMLUrlString(refName);
    return refName;
}

/// returns object image stream
LVStreamRef ldomNode::getObjectImageStream()
{
    lString16 refName = getObjectImageRefName();
    if ( refName.empty() )
        return LVStreamRef();
    return getCrDom()->getObjectImageStream( refName );
}


/// returns object image source
LVImageSourceRef ldomNode::getObjectImageSource()
{
    lString16 refName;

    if(getNodeId() == el_video)
    {
        //return link to video "poster" image to show that there is a video
        refName = getObjectVideoRefName();
    }
    else
    {
        refName = getObjectImageRefName();
    }
    LVImageSourceRef ref;
    if (refName.empty())
        return ref;
    ref = getCrDom()->getObjectImageSource( refName );
    if (!ref.isNull()) {
        int dx = ref->GetWidth();
        int dy = ref->GetHeight();
        ref = LVImageSourceRef(new NodeImageProxy(this, refName, dx, dy));
    } else {
        CRLog::warn("ObjectImageSource cannot be opened by name:%s", LCSTR(refName));
    }
    getCrDom()->_urlImageMap.set(refName, ref);
    return ref;
}

/// register embedded document fonts in font manager, if any exist in document
void CrDom::registerEmbeddedFonts()
{
    if (_fontList.empty()) {
        return;
    }
    int list = _fontList.length();
    lString8 lastface = lString8("");
    for (int i = list; i > 0; i--) {
        LVEmbeddedFontDef* item = _fontList.get(i - 1);
        lString16 url = item->getUrl();
        lString8 face = item->getFace();
        if (face.empty()) { face = lastface; }
        else { lastface = face; }
        //CRLog::debug("url is %s\n", UnicodeToLocal(url).c_str());
        if (url.startsWithNoCase(lString16("res://")) ||
            url.startsWithNoCase(lString16("file://"))) {
            if (!fontMan->RegisterExternalFont(item->getUrl(),
                    item->getFace(),
                    item->getBold(),
                    item->getItalic())) {
                CRLog::error("Failed to register external font face: %s file: %s",
                        item->getFace().c_str(),
                        LCSTR(item->getUrl()));
            }
            continue;
        } else {
            if (!fontMan->RegisterDocumentFont(getDocIndex(),
                    _container,
                    item->getUrl(),
                    item->getFace(),
                    item->getBold(),
                    item->getItalic())) {
                CRLog::error("Failed to register document font face: %s file: %s",
                        item->getFace().c_str(),
                        LCSTR(item->getUrl()));
                lString16Collection flist;
                fontMan->getFaceList(flist);
                int cnt = flist.length();
                lString16 fontface = lString16("");
                CRLog::debug("fontlist has %d fontfaces\n", cnt);
                for (int j = 0; j < cnt; j = j + 1) {
                    fontface = flist[j];
                    do { (fontface.replace(lString16(" "), lString16("\0"))); }
                    while (fontface.pos(lString16(" ")) != -1);
                    if (fontface.lowercase().pos(url.lowercase()) != -1) {
                        CRLog::debug("****found %s\n", UnicodeToLocal(fontface).c_str());
                        fontMan->setalias(face,
                                UnicodeToLocal(flist[j]),
                                getDocIndex(),
                                item->getItalic(),
                                item->getBold());
                        break;
                    }
                }
            }
        }
    }
}

/// returns object image stream
LVStreamRef CrDom::getObjectImageStream( lString16 refName )
{
    LVStreamRef ref;
    if ( refName.startsWith(lString16(BLOB_NAME_PREFIX)) ) {
        return _blobCache.getBlob(refName);
    } if ( refName[0]!='#' ) {
        if ( !getDocParentContainer().isNull() ) {
            lString16 name = refName;
            if ( !getCodeBase().empty() )
                name = getCodeBase() + refName;
            ref = getDocParentContainer()->OpenStream(name.c_str(), LVOM_READ);
            /*
            if ( ref.isNull() ) {
                lString16 fname = getProps()->getStringDef( DOC_PROP_FILE_NAME, "" );
                fname = LVExtractFilenameWithoutExtension(fname);
                if ( !fname.empty() ) {
                    lString16 fn = fname + "_img";
                    lString16 name = fn + "/" + refName;
                    if ( !getCodeBase().empty() )
                        name = getCodeBase() + name;
                    ref = getDocParentContainer()->OpenStream(name.c_str(), LVOM_READ);
                }
            }
            */
            if ( ref.isNull() ) {
                CRLog::error("Cannot open stream by name %s", LCSTR(name));
                //retry and check if it's URL encoded
                ref = getDocParentContainer()->OpenStream(DecodeHTMLUrlString(name).c_str(), LVOM_READ);
            }
            if ( ref.isNull() ) {
                LE("Cannot open stream by name %s, retrying with different format cases", LCSTR(name));
                //retry and check if it's URL encoded
                int dotpos = name.rpos(".");
                if(dotpos !=-1)
                {
                    lString16 format = name.substr(dotpos);
                    lString16 name_stripped = name.substr(0,dotpos);
                    format = format.lowercase();
                    lString16 name_temp = name_stripped + format;
                    ref = getDocParentContainer()->OpenStream(name_temp.c_str(), LVOM_READ);
                    if ( ref.isNull() )
                    {
                        format = format.uppercase();
                        name_temp = name_stripped + format;
                        ref = getDocParentContainer()->OpenStream(name_temp.c_str(), LVOM_READ);
                    }
                }
            }
        }
        return ref;
    }
    lUInt16 refValueId = findAttrValueIndex( refName.c_str() + 1 );
    if ( refValueId == (lUInt16)-1 ) {
        return ref;
    }
    ldomNode * objnode = getNodeById( refValueId );
    if ( !objnode || !objnode->isElement())
        return ref;
    ref = objnode->createBase64Stream();
    return ref;
}

/// returns object image source
LVImageSourceRef CrDom::getObjectImageSource( lString16 refName )
{
    LVStreamRef stream = getObjectImageStream( refName );
    if (stream.isNull())
        return LVImageSourceRef();
    return LVCreateStreamImageSource( stream );
}

void CrDom::resetNodeNumberingProps()
{
    lists.clear();
}

ListNumberingPropsRef CrDom::getNodeNumberingProps( lUInt32 nodeDataIndex )
{
    return lists.get(nodeDataIndex);
}

void CrDom::setNodeNumberingProps( lUInt32 nodeDataIndex, ListNumberingPropsRef v )
{
    lists.set(nodeDataIndex, v);
}

void CrDom::ApplyEmbeddedStyles()
{
#if 0
    LE("ApplyEmbeddedStyles()");
    LW("{");
    lString16 s = stylesManager.as_string();
    lString16 buf;
    for (int i = 0; i < s.length() ; i++)
    {
        buf+= s.at(i);
        if(s.at(i) == '\n')
        {
            LW("%s",LCSTR(buf));
            buf.clear();
        }
    }
    LW("}");
#endif
    this->setStylesheet(LCSTR(this->stylesManager.as_string()), false);
}

/// formats final block
int ldomNode::renderFinalBlock(  LFormattedTextRef & frmtext, RenderRectAccessor * fmt, int width )
{
    ASSERT_NODE_NOT_NULL;
    if ( !isElement() )
        return 0;
    //LE("renderFinalBlock fallbackreset!");
    fontMan->FallbackReset();
    //CRLog::trace("renderFinalBlock()");
    CVRendBlockCache & cache = getCrDom()->getRendBlockCache();
    LFormattedTextRef f;
    lvdom_element_render_method rm = getRendMethod();
    if ( cache.get( this, f ) ) {
        frmtext = f;
        if ( rm != erm_final && rm != erm_list_item && rm != erm_table_caption )
            return 0;
        //RenderRectAccessor fmt( this );
        //CRLog::trace("Found existing formatted object for node #%08X", (lUInt32)this);
        return fmt->getHeight();
    }
    f = getCrDom()->createFormattedText();
    if ( (rm != erm_final && rm != erm_list_item && rm != erm_table_caption) )
        return 0;
    //RenderRectAccessor fmt( this );
    /// render whole node content as single formatted object

    //check if image is in table, forbid scaling
    if (getParentNode("td") != NULL)
    {
        img_scaling_options_t nozoom;
        nozoom.zoom_in_block.mode   = IMG_NO_SCALE;
        nozoom.zoom_in_inline.mode  = IMG_NO_SCALE;
        nozoom.zoom_out_block.mode  = IMG_NO_SCALE;
        nozoom.zoom_out_inline.mode = IMG_NO_SCALE;
        nozoom.zoom_in_block.max_scale   = 0;
        nozoom.zoom_in_inline.max_scale  = 0;
        nozoom.zoom_out_block.max_scale  = 0;
        nozoom.zoom_out_inline.max_scale = 0;
        f->setImageScalingOptions(&nozoom);
    }
    //LE("[%s] page_break_before %d",LCSTR(this->getNodeName()), getStyle()->page_break_before);

    int flags = styleToTextFmtFlags( getStyle(), 0 );
    ::renderFinalBlock( this, f.get(), fmt, flags, 0, 16 );
    int page_h = getCrDom()->getPageHeight();
    cache.set( this, f );
    int h = f->Format((lUInt16)width, (lUInt16)page_h);
    frmtext = f;
    //CRLog::trace("Created new formatted object for node #%08X", (lUInt32)this);
    return h;
}

/// formats final block again after change, returns true if size of block is changed
bool ldomNode::refreshFinalBlock()
{
    ASSERT_NODE_NOT_NULL;
    if ( getRendMethod() != erm_final )
        return false;
    // TODO: implement reformatting of one node
    CVRendBlockCache & cache = getCrDom()->getRendBlockCache();
    cache.remove( this );
    RenderRectAccessor fmt( this );
    lvRect oldRect, newRect;
    fmt.getRect( oldRect );
    LFormattedTextRef txtform;
    int width = fmt.getWidth();
    renderFinalBlock( txtform, &fmt, width );
    fmt.getRect( newRect );
    if ( oldRect == newRect )
        return false;
    // TODO: relocate other blocks
    return true;
}

/// replace node with r/o persistent implementation
ldomNode* ldomNode::persist()
{
    ASSERT_NODE_NOT_NULL;
    if (isPersistent()) {
        return this;
    }
    if (isElement()) {
        // ELEM->PELEM
        tinyElement* elem = _data._elem_ptr;
        int attrCount = elem->_attrs.length();
        int childCount = elem->_children.length();
        _handle._dataIndex = (_handle._dataIndex & ~0xF) | NT_PELEMENT;
        lUInt32 parentIndex = elem->_parentNode ? elem->_parentNode->_handle._dataIndex : 0;
        _data._pelem_addr = getCrDom()->_elemStorage.allocElem(
                _handle._dataIndex,
                parentIndex,
                elem->_children.length(),
                elem->_attrs.length());
        ElementDataStorageItem* data = getCrDom()->_elemStorage.getElem(_data._pelem_addr);
        data->nsid = elem->_nsid;
        data->id = elem->_id;
        lUInt16* attrs = data->attrs();
        int i;
        for (i = 0; i < attrCount; i++) {
            const lxmlAttribute* attr = elem->_attrs[i];
            attrs[i * 3] = attr->nsid;     // namespace
            attrs[i * 3 + 1] = attr->id;   // id
            attrs[i * 3 + 2] = attr->index;// value
        }
        for (i = 0; i < childCount; i++) {
            data->children[i] = elem->_children[i];
        }
        data->rendMethod = (lUInt8) elem->_rendMethod;
        delete elem;
    } else {
        // TEXT->PTEXT
        lString8 utf8 = _data._text_ptr->getText();
        delete _data._text_ptr;
        lUInt32 parentIndex = _data._text_ptr->getParentIndex();
        _handle._dataIndex = (_handle._dataIndex & ~0xF) | NT_PTEXT;
        _data._ptext_addr = getCrDom()->_textStorage.allocText(
                _handle._dataIndex,
                parentIndex,
                utf8);
        // change type
    }
    return this;
}

/// replace node with r/w implementation
ldomNode* ldomNode::modify()
{
    ASSERT_NODE_NOT_NULL;
    if (isPersistent()) {
        if (isElement()) {
            // PELEM->ELEM
            ElementDataStorageItem* data = getCrDom()->_elemStorage.getElem(_data._pelem_addr);
            tinyElement* elem = new tinyElement(getCrDom(),
                    getParentNode(),
                    data->nsid,
                    data->id);
            for (int i = 0; i < data->childCount; i++) {
                elem->_children.add(data->children[i]);
            }
            for (int i = 0; i < data->attrCount; i++) {
                elem->_attrs.add(data->attr(i));
            }
            _handle._dataIndex = (_handle._dataIndex & ~0xF) | NT_ELEMENT;
            elem->_rendMethod = (lvdom_element_render_method) data->rendMethod;
            getCrDom()->_elemStorage.freeNode(_data._pelem_addr);
            _data._elem_ptr = elem;
        } else {
            // PTEXT->TEXT
            // convert persistent text to mutable
            lString8 utf8 = getCrDom()->_textStorage.getText(_data._ptext_addr);
            lUInt32 parentIndex = getCrDom()->_textStorage.getParent(_data._ptext_addr);
            getCrDom()->_textStorage.freeNode(_data._ptext_addr);
            _data._text_ptr = new ldomTextNode(parentIndex, utf8);
            // change type
            _handle._dataIndex = (_handle._dataIndex & ~0xF) | NT_TEXT;
        }
    }
    return this;
}

lvRect ldomNode::getFullMargins()
{
    lvRect margins = lvRect(0,0,0,0);
    ldomNode* node = this;
    while(node != NULL && node->getParentNode()!=NULL)
    {
        if(!node->isText())
        {
            css_style_rec_t *style = node->getStyle().get();
            margins.left   += style->margin[0].value;
            margins.right  += style->margin[1].value;
            margins.top    += style->margin[2].value;

            if( getCrDom()->cfg_txt_indent_margin_override && getNodeId() == el_p)
            {
                margins.bottom += getCrDom()->cfg_txt_margin;
            }
            else
            {
                margins.bottom += style->margin[3].value;
            }

        }
        node = node->getParentNode();
    }
    return margins;
}

lString16 ldomNode::getMainParentName()
{
    lString16 result;
    ldomNode* node = this;
    while(node != NULL && node->getParentNode()!=NULL)
    {
        if(node->isNodeName("li")
           || node->isNodeName("poem")
           || node->isNodeName("stanza")
           || node->isNodeName("annotation")
           || node->isNodeName("blockquote")
           || node->isNodeName("td")
           || node->isNodeName("epigraph"))
        {
            return node->getNodeName();
        }
        node = node->getParentNode();
    }
    return lString16::empty_str;
}

void CrDomBase::dumpStatistics() {
//#define TINYNODECOLLECTION_DUMPSTATISTICS
#ifdef TINYNODECOLLECTION_DUMPSTATISTICS
    CRLog::trace("CrDomBase::dumpStatistics: totalNodes: %d (%d kB)\n"
    			"    elements: %d, textNodes: %d\n"
                "    ptext: %d (uncomp.), ptelems: %d (uncomp.)\n"
                "    rects: %d (uncomp.), nodestyles: %d (uncomp.), styles: %d\n"
    			"    fonts: %d, renderedNodes: %d\n"
                "    mutableElements: %d (~%d kB)",
                _itemCount,
                _itemCount*16/1024,
                _elemCount,
                _textCount,
                _textStorage.getUncompressedSize(),
                _elemStorage.getUncompressedSize(),
                _rectStorage.getUncompressedSize(),
                _styleStorage.getUncompressedSize(),
                _styles.length(),
                _fonts.length(),
                ((CrDom*)this)->_renderedBlockCache.length(),
                _tinyElementCount,
                _tinyElementCount * (sizeof(tinyElement) + 8 * 4) / 1024);
    CRLog::trace("Document memory usage:"
                " elements: %d,"
                " textNodes: %d,"
                " ptext=(%d uncompressed),"
                " ptelems=(%d uncompressed),"
                " rects=(%d uncompressed),"
                " nodestyles=(%d uncompressed),"
                " styles:%d, fonts:%d, renderedNodes:%d,"
                " totalNodes: %d(%dKb), mutableElements: %d(~%dKb)",
                _elemCount,
                 _textCount,
                _textStorage.getUncompressedSize(),
                _elemStorage.getUncompressedSize(),
                _rectStorage.getUncompressedSize(),
                _styleStorage.getUncompressedSize(),
                _styles.length(),
                 _fonts.length(),
                ((CrDom*)this)->_renderedBlockCache.length(),
                _itemCount,
                _itemCount*16/1024,
                _tinyElementCount,
                _tinyElementCount * (sizeof(tinyElement) + 8 * 4) / 1024);
#endif //TINYNODECOLLECTION_DUMPSTATISTICS
}

lvRect ldomWord::getRect()
{
    //CRLog::error("ldomWord getrect start");
    lvRect result;
    ldomXRange range(this->getStartXPointer(),this->getEndXPointer());
    range.getRect(result);
    //CRLog::error("ldomWord getrect end");
    return result;
}

void PrettyPrintDocx(ldomNode * node)
{
    if(!gDocumentRTL)
    {
        return;
    }

    for (int i = 0; i < node->getChildCount(); i++)
    {
        ldomNode * child = node->getChildNode(i);
        //rtl arabic fix for docx
        if(child->isText())
        {
            continue;
        }
        if (child->isNodeName("p"))
        {
            //bound.printOut();
            DomBoundlString16 bound = child->getText_bound();
            if (bound.checkNodesForRTL())
            {
                //CRLog::error("before str = %s",LCSTR(bound.string_));
                bound.string_ = bound.string_.PrepareRTL();
                bound.lamAlephFilter();
                bound.applyNewString();
            }
        }
        else
        {
            PrettyPrintDocx(child);
        }
    }
}

/*
void PrettyPrint_indic(ldomNode * node)
{
    if(gDocumentDvng == 0)
    {
        return;
    }
    for (int i = 0; i < node->getChildCount(); i++)
    {
        ldomNode * child = node->getChildNode(i);
        if(child->isText())
        {
            continue;
        }
        if (child->isNodeName("p"))
        {
            //bound.printOut();
            DomBoundlString16 bound = child->getText_bound();
            if (bound.string_.CheckDvng())
            {
                //CRLog::error("before str = %s",LCSTR(bound.string_));
                bound.PrepareIndic();
                bound.applyNewString();
            }
        }
        else
        {
            PrettyPrint_indic(child);
        }
    }
}
 */

void RecurseTOC(ldomNode * node,LvTocItem * toc, bool deepSearch, bool allow_title)
{
    if(node->isNodeName("body") && node->hasAttribute(attr_name) && node->getAttributeValue(attr_name)=="notes_hidden")
    {
        return;
    }
    if(node->isNodeName("img") || node->isNodeName("image") ||node->isNodeName("table")|| node->isNodeName("a")  )
    {
        return;
    }
    for (int i = 0; i < node->getChildCount(); i++)
    {
        ldomNode * child = node->getChildNode(i);
        if(!deepSearch)
        {
            if (child->isNodeName("h1") || child->isNodeName("h2") || child->isNodeName("h3"))
            {
                lString16 text = child->getText();
                if (text.empty() || text == "-" || text.DigitsOnly())
                {
                    continue;
                }
                if (text.length() > TOC_ITEM_LENGTH_MAX)
                {
                    text = text.substr(0, TOC_ITEM_LENGTH_MAX);
                    text = text + lString16("...");
                }

                int lvl = 1;
                if (child->isNodeName("h1") || child->isNodeName("title"))
                {
                    lvl = 1;
                }
                else if (child->isNodeName("h2"))
                {
                    lvl = 2;
                }
                else if (child->isNodeName("h3"))
                {
                    lvl = 3;
                }

                LvTocItem *item = new LvTocItem(ldomXPointer(child, 0), child->getPath(), text);
                toc->addItem(item, lvl);
                //CRLog::error("added toc [%s], path = [%s]",LCSTR(child->getText()),LCSTR(child->getXPath()));
            }
            else if (allow_title && child->isNodeName("title"))
            {
                lString16 text = child->getText();
                if (text.empty() || text == "-" || text.DigitsOnly())
                {
                    continue;
                }
                if (text.length() > TOC_ITEM_LENGTH_MAX)
                {
                    text = text.substr(0, TOC_ITEM_LENGTH_MAX);
                    text = text + lString16("...");
                }
                LvTocItem *item = new LvTocItem(ldomXPointer(child, 0), child->getPath(), text);
                toc->addItem(item, 1);
            }
            else if (child->isElement())
            {
                RecurseTOC(child,toc,deepSearch,allow_title);
            }
        }
        else
        {
            if (child->isNodeName("h4") || child->isNodeName("h5") || child->isNodeName("h6"))
            {
                lString16 text = child->getText();
                if (text.empty() || text == "-" || text.DigitsOnly())
                {
                    continue;
                }
                if (text.length() > TOC_ITEM_LENGTH_MAX)
                {
                    text = text.substr(0, TOC_ITEM_LENGTH_MAX);
                    text = text + lString16("...");
                }

                int lvl = 1;
                if (child->isNodeName("h4"))
                {
                    lvl = 1;
                }
                else if (child->isNodeName("h5"))
                {
                    lvl = 2;
                }
                else if (child->isNodeName("h6"))
                {
                    lvl = 3;
                }

                LvTocItem *item = new LvTocItem(ldomXPointer(child, 0), child->getPath(), text);
                toc->addItem(item, lvl);
                //CRLog::error("added toc [%s], path = [%s]",LCSTR(child->getText()),LCSTR(child->getXPath()));
            }
            else if (allow_title && child->isNodeName("title"))
            {
                lString16 text = child->getText();
                if (text.empty() || text == "-" || text.DigitsOnly())
                {
                    continue;
                }
                if (text.length() > TOC_ITEM_LENGTH_MAX)
                {
                    text = text.substr(0, TOC_ITEM_LENGTH_MAX);
                    text = text + lString16("...");
                }
                LvTocItem *item = new LvTocItem(ldomXPointer(child, 0), child->getPath(), text);
                toc->addItem(item, 1);
            }
            else if (child->isElement())
            {
                RecurseTOC(child,toc,deepSearch,allow_title);
            }
        }
    }
}

void GetTOC(CrDom * crDom, LvTocItem * toc, bool deepSearch, bool allow_title)
{
    ldomNode * root = crDom->getRootNode();
    for (int i = 0; i < root->getChildCount(); i++)
    {
        ldomNode * child = root->getChildNode(i);
        if(child->isNodeName("body") && child->hasAttribute(attr_name) && child->getAttributeValue(attr_name)=="notes_hidden")
        {
            continue;
        }
        if(child->isNodeName("img") || child->isNodeName("image")|| child->isNodeName("table") || child->isNodeName("a") )
        {
            continue;
        }
        if (child->isElement())
        {
            RecurseTOC(child,toc,deepSearch,allow_title);
        }
    }
}
ldomNode * ldomNode::getSibling (int index)
{
    ldomNode *p = getParentNode();
    if (!p || index < 0 || index >= (int) p->getChildCount())
    {
        return NULL;
    }
    return p->getChildNode(index);
}

ldomNode * ldomNode::getNextSibling ()
{
    int index = this->getNodeIndex() + 1;
    return  this->getSibling (index);
}

ldomNode * ldomNode::getPrevSibling ()
{
    int index = this->getNodeIndex() -1;
    return  this->getSibling (index);
}

void FixOdtSpaces(ldomNode * node)
{
    for (int i = 0; i < node->getChildCount(); i++)
    {
        ldomNode * child = node->getChildNode(i);
        if(child->isNull())
        {
            continue;
        }
        if(child->isNodeName("body") && child->hasAttribute(attr_name) && child->getAttributeValue(attr_name)=="notes_hidden")
        {
            continue;
        }
        if(child->isNodeName("img")||
        child->isNodeName("image") ||
        //child->isNodeName("table") ||
        child->isNodeName("a")||
        child->isNodeName("h1")||
        child->isNodeName("h2")||
        child->isNodeName("h3")||
        child->isNodeName("h4")||
        child->isNodeName("h5")||
        child->isNodeName("h6")||
        child->isNodeName("li") )
        {
            continue;
        }
        if (child->isElement())
        {
            FixOdtSpaces(child);
        }
        if(child->isText())
        {
            lString16 currtxt = child->getText();
            //CRLog::error("currtext = [%s]",LCSTR(currtxt));
            if(currtxt.length()==1)
            {
                continue;
            }
            if(currtxt.startsWith(" ") || currtxt.startsWith(".") || currtxt.startsWith(","))
            {
                continue;
            }
            ldomNode *s = child->getPrevSibling();
            if (s->isNull())
            {
                //CRLog::error("get sibling from upper node");
                ldomNode *p = child->getParentNode();
                while (s->isNull())
                {
                    p = p->getParentNode();
                    s = p->getLastTextChild();
                }
            }
            if(s->isNull())
            {
                continue;
            }
            lString16 txt = s->getText();
            if (txt.endsWith(" ") || txt.endsWith(".") || txt.endsWith(",") || txt.endsWith("!") || txt.endsWith("?"))
            {
                continue;
            }
            //CRLog::trace("sp added = [%s]",LCSTR(L" " + currtxt));
            child->setText(L" " + currtxt);
        }
    }
}