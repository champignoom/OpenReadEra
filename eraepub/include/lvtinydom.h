/** \file lvtinydom.h
    \brief fast and compact XML DOM tree

    CoolReader Engine

    (c) Vadim Lopatin, 2000-2009
    Copyright (C) 2013-2020 READERA LLC

    This source code is distributed under the terms of
    GNU General Public License
    See LICENSE file for details

	Goal: make fast DOM implementation with small memory footprint.

    2009/04 : Introducing new storage model, optimized for mmap.
    All DOM objects are divided by 2 parts.
    1) Short RAM instance
    2) Data storage part, which could be placed to mmap buffer.

    Document object storage should handle object table and data buffer.
    Each object has DataIndex, index of entry in object table.
    Object table holds pointer to RAM instance and data storage for each object.
*/

#ifndef __LV_TINYDOM_H_INCLUDED__
#define __LV_TINYDOM_H_INCLUDED__

#include "lvstring.h"
#include "lstridmap.h"
#include "lvxml.h"
#include "dtddef.h"
#include "lvstyles.h"
#include "lvdrawbuf.h"
#include "lvstsheet.h"
#include "lvpagesplitter.h"
#include "lvptrvec.h"
#include "lvhashtable.h"
#include "lvimg.h"
#include "props.h"
#include "crconfig.h"
#include "openreadera.h"

#define LXML_NO_DATA       0 ///< to mark data storage record as empty
#define LXML_ELEMENT_NODE  1 ///< element node
#define LXML_TEXT_NODE     2 ///< text node

/// docFlag mask, enable internal stylesheet of document and style attribute of elements
#define DOC_FLAG_EMBEDDED_STYLES        1
/// docFlag mask, enable paperbook-like footnotes
#define DOC_FLAG_ENABLE_FOOTNOTES       2
/// docFlag mask, enable document embedded fonts (EPUB)
#define DOC_FLAG_EMBEDDED_FONTS         8

#define LXML_NS_NONE 0       ///< no namespace specified
#define LXML_NS_ANY  0xFFFF  ///< any namespace can be specified
#define LXML_ATTR_VALUE_NONE  0xFFFF  ///< attribute not found

#define DOC_STRING_HASH_SIZE  256
#define MAX_TYPE_ID           1024 // max of element, ns, attr
#define MAX_ELEMENT_TYPE_ID   1024
#define MAX_NAMESPACE_TYPE_ID 64
#define MAX_ATTRIBUTE_TYPE_ID 1024
#define UNKNOWN_ELEMENT_TYPE_ID   (MAX_ELEMENT_TYPE_ID>>1)
#define UNKNOWN_ATTRIBUTE_TYPE_ID (MAX_ATTRIBUTE_TYPE_ID>>1)
#define UNKNOWN_NAMESPACE_TYPE_ID (MAX_NAMESPACE_TYPE_ID>>1)

// document property names
#define DOC_PROP_CODE_BASE       "doc.file.code.base"
#define DOC_PROP_COVER_FILE      "doc.cover.file"

// 25 to 100
#define DEF_MIN_SPACE_CONDENSING_PERCENT 100

#define MAX_DOM_LEVEL 256

typedef enum {
    doc_format_none,
    doc_format_fb2,
    doc_format_txt,
    doc_format_rtf,
    doc_format_epub,
    doc_format_chm,
    doc_format_doc,
    doc_format_mobi,
    doc_format_html,
} doc_format_t;

/// final block cache
typedef LVRef<LFormattedText> LFormattedTextRef;
typedef LVCacheMap<ldomNode*, LFormattedTextRef> CVRendBlockCache;

/// XPath step kind
typedef enum {
	xpath_step_error = 0, // error
	xpath_step_element,   // element of type 'name' with 'index'        /elemname[N]/
	xpath_step_text,      // text node with 'index'                     /text()[N]/
	xpath_step_nodeindex, // node index                                 /N/
	xpath_step_point      // point index                                .N
} xpath_step_t;
xpath_step_t ParseXPathStep( const lChar8 * &path, lString8 & name, int & index );

// mode: 0=disabled, 1=integer scaling factors, 2=free scaling
// scale: 0=auto based on font size, 1=no zoom, 2=scale up to *2, 3=scale up to *3
#define DEF_IMAGE_SCALE_ZOOM_IN_MODE 0
#define DEF_IMAGE_SCALE_ZOOM_IN_SCALE 1

#define DEF_IMAGE_SCALE_ZOOM_OUT_MODE 1
#define DEF_IMAGE_SCALE_ZOOM_OUT_SCALE 3

/// type of image scaling
typedef enum {
    IMG_NO_SCALE,        /// scaling is disabled
    IMG_INTEGER_SCALING, /// integer multipier/divisor scaling -- *2, *3 only
    IMG_FREE_SCALING     /// free scaling, non-integer factor
} img_scaling_mode_t;

struct img_scaling_option_t
{
    img_scaling_mode_t mode;
    int max_scale;
    int getHash() { return (int) mode * 33 + max_scale; }
    img_scaling_option_t();
};

/// set of images scaling options for different kind of images
struct img_scaling_options_t
{
    img_scaling_option_t zoom_in_inline;
    img_scaling_option_t zoom_in_block;
    img_scaling_option_t zoom_out_inline;
    img_scaling_option_t zoom_out_block;
    /// returns hash value
    int getHash()
    {
        return (((zoom_in_inline.getHash() * 33 + zoom_in_block.getHash())
                * 33 + zoom_out_inline.getHash()) * 33 + zoom_out_block.getHash());
    }
    /// creates default options
    img_scaling_options_t();
};

struct DataStorageItemHeader;
struct TextDataStorageItem;
struct ElementDataStorageItem;
struct NodeItem;
class DataBuffer;

class ldomTextStorageChunk;
class ldomTextStorageChunkBuilder;
struct ElementDataStorageItem;
class CrDomBase;

struct ldomNodeStyleInfo
{
    lUInt16 _fontIndex;
    lUInt16 _styleIndex;
};

class ldomBlobItem;
#define BLOB_NAME_PREFIX L"@blob#"
#define MOBI_IMAGE_NAME_PREFIX L"mobi_image_"
class ldomBlobCache
{
    LVPtrVector<ldomBlobItem> _list;
    bool _changed;
public:
    ldomBlobCache();
    bool addBlob( const lUInt8 * data, int size, lString16 name );
    LVStreamRef getBlob( lString16 name );
};

class ldomDataStorageManager
{
    friend class ldomTextStorageChunk;
protected:
    CrDomBase* _owner;
    LVPtrVector<ldomTextStorageChunk> _chunks;
    ldomTextStorageChunk * _activeChunk;
    ldomTextStorageChunk * _recentChunk;
    int _uncompressedSize;
    int _maxUncompressedSize;
    int _chunkSize;
    char _type;       /// type, to show in log
    ldomTextStorageChunk * getChunk( lUInt32 address );
public:
    /// checks buffer sizes, compacts most unused chunks
    void compact( int reservedSpace );
    int getUncompressedSize() { return _uncompressedSize; }
    /// allocates new text node, return its address inside storage
    lUInt32 allocText( lUInt32 dataIndex, lUInt32 parentIndex, const lString8 & text );
    /// allocates storage for new element, returns address address inside storage
    lUInt32 allocElem( lUInt32 dataIndex, lUInt32 parentIndex, int childCount, int attrCount );
    /// get text by address
    lString8 getText( lUInt32 address );
    /// get pointer to element data
    ElementDataStorageItem * getElem( lUInt32 addr );
    /// change node's parent, returns true if modified
    bool setParent( lUInt32 address, lUInt32 parent );
    /// returns node's parent by address
    lUInt32 getParent( lUInt32 address );
    /// free data item
    void freeNode( lUInt32 addr );
    /// call to invalidate chunk if content is modified
    void modified( lUInt32 addr );

    /// get or allocate space for rect data item
    void getRendRectData( lUInt32 elemDataIndex, lvdomElementFormatRec * dst );
    /// set rect data item
    void setRendRectData( lUInt32 elemDataIndex, const lvdomElementFormatRec * src );

    /// get or allocate space for element style data item
    void getStyleData( lUInt32 elemDataIndex, ldomNodeStyleInfo * dst );
    /// set element style data item
    void setStyleData( lUInt32 elemDataIndex, const ldomNodeStyleInfo * src );

    ldomDataStorageManager(CrDomBase* owner,
            char type,
            int maxUnpackedSize,
            int chunkSize);

    ~ldomDataStorageManager();
};

/// class to store compressed/uncompressed text nodes chunk
class ldomTextStorageChunk
{
    friend class ldomDataStorageManager;
    ldomDataStorageManager * _manager;
    ldomTextStorageChunk * _nextRecent;
    ldomTextStorageChunk * _prevRecent;
    lUInt8 * _buf;     /// buffer for uncompressed data
    lUInt32 _bufsize;  /// _buf (uncompressed) area size, bytes
    lUInt32 _bufpos;  /// _buf (uncompressed) data write position (for appending of new data)
    lUInt16 _index;  /// ? index of chunk in storage
    char _type;       /// type, to show in log

    void setunpacked( const lUInt8 * buf, int bufsize );
    /// free data item
    void freeNode( int offset );

public:
    /// call to invalidate chunk if content is modified
    void modified();
    /// returns chunk index inside collection
    int getIndex() { return _index; }
    /// returns free space in buffer
    int space();
    /// adds new text item to buffer, returns offset inside chunk of stored data
    int addText( lUInt32 dataIndex, lUInt32 parentIndex, const lString8 & text );
    /// adds new element item to buffer, returns offset inside chunk of stored data
    int addElem( lUInt32 dataIndex, lUInt32 parentIndex, int childCount, int attrCount );
    /// get text item from buffer by offset
    lString8 getText( int offset );
    /// get node parent by offset
    lUInt32 getParent( int offset );
    /// set node parent by offset
    bool setParent( int offset, lUInt32 parentIndex );
    /// get pointer to element data
    ElementDataStorageItem * getElem( int offset );
    /// get raw data bytes
    void getRaw( int offset, int size, lUInt8 * buf );
    /// set raw data bytes
    void setRaw( int offset, int size, const lUInt8 * buf );
    /// create empty buffer
    ldomTextStorageChunk(ldomDataStorageManager * manager, lUInt16 index);
    /// create with preallocated buffer, for raw access
    ldomTextStorageChunk(int preAllocSize, ldomDataStorageManager * manager, lUInt16 index);
    ~ldomTextStorageChunk();
};

class ldomNode;

#define TNC_PART_COUNT 1024
#define TNC_PART_SHIFT 10
#define TNC_PART_INDEX_SHIFT (TNC_PART_SHIFT+4)
#define TNC_PART_LEN (1<<TNC_PART_SHIFT)
#define TNC_PART_MASK (TNC_PART_LEN-1)

/// Storage of ldomNode
class CrDomBase
{
    friend class ldomNode;
    friend class tinyElement;
    friend class CrDom;
private:
    int _textCount;
    lUInt32 _textNextFree;
    ldomNode * _textList[TNC_PART_COUNT];
    int _elemCount;
    lUInt32 _elemNextFree;
    ldomNode * _elemList[TNC_PART_COUNT];
    LVIndexedRefCache<css_style_ref_t> _styles;
    LVIndexedRefCache<font_ref_t> _fonts;
    int _tinyElementCount;
    int _itemCount;
    int _docIndex;
protected:
    /// final block cache
    CVRendBlockCache _renderedBlockCache;
    bool _mapped;
    bool _maperror;
    int  _mapSavingStage;
    img_scaling_options_t _imgScalingOptions;
    int  _minSpaceCondensingPercent;
    // Persistent text node data storage
    ldomDataStorageManager _textStorage;
    // Persistent element data storage
    ldomDataStorageManager _elemStorage;
    // Element render rect storage
    ldomDataStorageManager _rectStorage;
    // Element style storage (font & style indexes ldomNodeStyleInfo)
    ldomDataStorageManager _styleStorage;
    CRPropRef _docProps;
    lUInt32 _docFlags;
    LVStyleSheet stylesheet_;
    // Style index to font index
    LVHashTable<lUInt16, lUInt16> _fontMap;
    /// Checks buffer sizes, compacts most unused chunks
    ldomBlobCache _blobCache;

    int calcFinalBlocks();
    void dropStyles();
    bool updateLoadedStyles(bool enabled);
    lUInt32 calcStyleHash();
    void setNodeStyleIndex( lUInt32 dataIndex, lUInt16 index );
    void setNodeFontIndex( lUInt32 dataIndex, lUInt16 index );
    lUInt16 getNodeStyleIndex( lUInt32 dataIndex );
    lUInt16 getNodeFontIndex( lUInt32 dataIndex );
    css_style_ref_t getNodeStyle( lUInt32 dataIndex );
    font_ref_t getNodeFont( lUInt32 dataIndex );
    void setNodeStyle( lUInt32 dataIndex, css_style_ref_t & v );
    void setNodeFont( lUInt32 dataIndex, font_ref_t & v  );
    void clearNodeStyle( lUInt32 dataIndex );
    virtual void resetNodeNumberingProps() { }
public:
    CrDomBase();
    virtual ~CrDomBase();
    /// add named BLOB data to document
    bool addBlob(lString16 name, const lUInt8* data, int size)
    {
        return _blobCache.addBlob(data, size, name);
    }
    /// get BLOB by name
    LVStreamRef getBlob(lString16 name) { return _blobCache.getBlob(name); }
    inline bool getDocFlag( lUInt32 mask ) { return (_docFlags & mask) != 0; }
    void setDocFlag( lUInt32 mask, bool value );
    inline lUInt32 getDocFlags() { return _docFlags; }
    inline int getDocIndex() { return _docIndex; }
    inline int getFontContextDocIndex()
    {
        if (gEmbeddedStylesLVL > 1) return _docIndex;
        return (_docFlags & DOC_FLAG_EMBEDDED_FONTS) && (_docFlags & DOC_FLAG_EMBEDDED_STYLES)
                ? _docIndex
                : -1;
    }
    void setDocFlags( lUInt32 value );
    /// returns doc properties collection
    inline CRPropRef getProps() { return _docProps; }
    /// returns doc properties collection
    void setProps( CRPropRef props ) { _docProps = props; }
    /// get ldomNode instance pointer
    ldomNode * getTinyNode( lUInt32 index );
    /// allocate new ldomNode
    ldomNode * allocTinyNode( int type );
    /// allocate new tinyElement
    ldomNode * allocTinyElement( ldomNode * parent, lUInt16 nsid, lUInt16 id );
    /// recycle ldomNode on node removing
    void recycleTinyNode( lUInt32 index );
    bool validateDocument();
    /// dumps memory usage statistics to debug log
    void dumpStatistics();

    lUInt32 calcStyleHashFull();
};

class CrDom;
class tinyElement;
struct lxmlAttribute;

class RenderRectAccessor : public lvdomElementFormatRec
{
    ldomNode * _node;
    bool _modified;
    bool _dirty;
public:
    //RenderRectAccessor & operator -> () { return *this; }
    int getX();
    int getY();
    int getWidth();
    int getHeight();
    void getRect( lvRect & rc );
    void setX( int x );
    void setY( int y );
    void setWidth( int w );
    void setHeight( int h );
    void push();
    RenderRectAccessor( ldomNode * node );
    ~RenderRectAccessor();
};

/// compact 32bit value for node
struct ldomNodeHandle {
    unsigned _docIndex:8;   // index in ldomNode::_domInstances[MAX_DOM_INSTANCES];
    unsigned _dataIndex:24; // index of node in document's storage and type
};

/// max number which could be stored in ldomNodeHandle._docIndex
#define MAX_DOM_INSTANCES 256
class DomBoundlString16;
class ldomTextNode;
// no vtable, very small size (16 bytes)
// optimized for 32 bit systems
class ldomNode
{
    friend class CrDomBase;
    friend class RenderRectAccessor;
    friend class NodeImageProxy;
private:
    static CrDom* _domInstances[MAX_DOM_INSTANCES];
    /// adds document to list, returns ID of allocated document, -1 if no space in instance array
    static int registerDom(CrDom* doc);
    /// removes document from list
    static void unregisterDom(CrDom* doc);

    // types for _handle._type
    enum {
        NT_TEXT=0,       // mutable text node
        NT_ELEMENT=1,    // mutable element node
        NT_PTEXT=2,      // immutable (persistent) text node
        NT_PELEMENT=3   // immutable (persistent) element node
    };

    /// 0: packed 32bit data field
    ldomNodeHandle _handle; // _docIndex, _dataIndex, _type

    /// 4: misc data 4 bytes (8 bytes on x64)
    union {                    // [8] 8 bytes (16 bytes on x64)
        ldomTextNode * _text_ptr;   // NT_TEXT: mutable text node pointer
        tinyElement * _elem_ptr;    // NT_ELEMENT: mutable element pointer
        lUInt32 _pelem_addr;        // NT_PELEMENT: element storage address: chunk+offset
        lUInt32 _ptext_addr;        // NT_PTEXT: persistent text storage address: chunk+offset
        lUInt32 _nextFreeIndex;     // NULL for removed items
    } _data;

    /// sets document for node
    inline void setDocumentIndex( int index ) { _handle._docIndex = index; }

#define TNTYPE  (_handle._dataIndex&0x0F)
#define TNINDEX (_handle._dataIndex&(~0x0E))

    void onCollectionDestroy();

    inline ldomNode* getTinyNode(lUInt32 index) const
    {
        return ((CrDomBase*) getCrDom())->getTinyNode(index);
    }

    void operator delete(void *)
    {
        // Do nothing. Just to disable delete.
    }

    /// changes parent of item
    void setParentNode( ldomNode * newParent );
    /// add child
    void addChild( lInt32 childNodeIndex );
    /// call to invalidate cache if persistent node content is modified
    void modified();
    /// returns copy of render data structure
    void getRenderData( lvdomElementFormatRec & dst);
    /// sets new value for render data structure
    void setRenderData( lvdomElementFormatRec & newData);
    void autoboxChildren( int startIndex, int endIndex );
    void removeChildren( int startIndex, int endIndex );
public:
    /// if stylesheet file name is set, and file is found, set stylesheet to its value
    bool applyNodeStylesheet();
    bool initNodeFont();
    void initNodeStyle();
    /// init render method for this node only (children should already have rend method set)
    void initNodeRendMethod();
    /// default branch of initNodeRendMethod() moved into separate method
    void initNodeRendMethodDefault();
    /// init render method for the whole subtree
    void initNodeRendMethodRecursive();
    /// init render method for the whole subtree
    void initNodeStyleRecursive();
    /// remove node, clear resources
    void destroy();
    /// returns true for invalid/deleted node ot NULL this pointer
    inline bool isNull() const { return this == NULL || _handle._dataIndex==0; }
    /// returns true if node is stored in persistent storage
    inline bool isPersistent() const { return (_handle._dataIndex&2)!=0; }
    /// returns data index of node's registration in document data storage
    inline lInt32 getDataIndex() const { return TNINDEX; }
    /// returns pointer to document
    inline CrDom * getCrDom() const { return _domInstances[_handle._docIndex]; }
    /// returns pointer to parent node, NULL if node has no parent
    ldomNode * getParentNode() const;
    /// returns node type, either LXML_TEXT_NODE or LXML_ELEMENT_NODE
    inline lUInt8 getNodeType() const
    {
        return (_handle._dataIndex & 1) ? LXML_ELEMENT_NODE : LXML_TEXT_NODE;
    }
    /// returns node level, 0 is root node
    lUInt8 getNodeLevel() const;
    /// returns dataIndex of node's parent, 0 if no parent
    int getParentIndex() const;
    /// returns index of node inside parent's child collection
    int getNodeIndex() const;
    /// returns index of child node by dataIndex
    int getChildIndex( lUInt32 dataIndex ) const;
    /// returns true if node is document's root
    bool isRoot() const;
    /// returns true if node is text
    inline bool isText() const { return _handle._dataIndex && !(_handle._dataIndex&1); }
    /// returns true if node is image node
    inline bool isImage() const { return (this->isNodeName("img") || this->isNodeName("image") || this->isNodeName("video")); }
    /// returns true if node is element
    inline bool isElement() const { return _handle._dataIndex && (_handle._dataIndex&1); }
    /// returns true if node is and element that has children
    inline bool hasChildren() { return getChildCount()!=0; }
    /// returns true if node is element has attributes
    inline bool hasAttributes() const { return getAttrCount()!=0; }
    /// returns element child count
    int getChildCount() const;
    /// returns element attribute count
    int getAttrCount() const;
    /// returns attribute value by attribute name id and namespace id
    const lString16 & getAttributeValue( lUInt16 nsid, lUInt16 id ) const;
    /// returns attribute value by attribute name
    inline const lString16 & getAttributeValue( const lChar16 * attrName ) const
    {
        return getAttributeValue( NULL, attrName );
    }
    /// returns attribute value by attribute name
    inline const lString16 & getAttributeValue( const lChar8 * attrName ) const
    {
        return getAttributeValue( NULL, attrName );
    }
    /// returns attribute value by attribute name and namespace
    const lString16 & getAttributeValue( const lChar16 * nsName, const lChar16 * attrName ) const;
    /// returns attribute value by attribute name and namespace
    const lString16 & getAttributeValue( const lChar8 * nsName, const lChar8 * attrName ) const;
    /// returns attribute by index
    const lxmlAttribute * getAttribute( lUInt32 ) const;
    /// returns true if element node has attribute with specified name id and namespace id
    bool hasAttribute( lUInt16 nsId, lUInt16 attrId ) const;
    /// returns attribute name by index
    const lString16 & getAttributeName( lUInt32 ) const;
    /// sets attribute by string-defined nsname, attrname and value
    void setAttribute(const lChar16* nsname, const lChar16* attrname, const lChar16* attr_val);
    /// sets attribute value
    void setAttributeValue( lUInt16 , lUInt16 , const lChar16 * );
    /// returns attribute value by attribute name id
    inline const lString16& getAttributeValue(lUInt16 id) const
    {
        return getAttributeValue(LXML_NS_ANY, id);
    }
    /// returns true if element node has attribute with specified name id
    inline bool hasAttribute(lUInt16 id) const
    {
        return hasAttribute(LXML_NS_ANY, id);
    }
    /// returns element type structure pointer if it was set in document for this element name
    const css_elem_def_props_t * getElementTypePtr();
    /// returns element name id
    lUInt16 getNodeId() const;
    /// returns element namespace id
    lUInt16 getNodeNsId() const;
    /// replace element name id with another value
    void setNodeId( lUInt16 );
    /// returns element name
    const lString16 & getNodeName() const;
    /// compares node name with value, returns true if matches
    bool isNodeName(const char * name) const;
    /// returns element namespace name
    const lString16 & getNodeNsName() const;
    /// returns child node by index
    ldomNode * getChildNode( lUInt32 index ) const;
    /// returns true child node is element
    bool isChildNodeElement( lUInt32 index ) const;
    /// returns true child node is text
    bool isChildNodeText( lUInt32 index ) const;
    /// returns child node by index, NULL if node with this index is not element
    /// or nodeId!=0 and element node id!=nodeId
    ldomNode * getChildElementNode( lUInt32 index, lUInt16 nodeId=0 ) const;
    /// returns child node by index, NULL if node with this index is not element
    /// or nodeTag!=0 and element node name!=nodeTag
    ldomNode * getChildElementNode( lUInt32 index, const lChar16 * nodeTag ) const;
    /// returns text node text as wide string
    lString16 getText( lChar16 blockDelimiter = 0, int maxSize=0 ) const;
    DomBoundlString16 getText_bound(int maxSize=0 );
    /// returns text node text as utf8 string
    lString8 getText8( lChar8 blockDelimiter = 0, int maxSize=0 ) const;
    /// sets text node text as wide string
    void setText( lString16 );
    /// sets text node text as utf8 string
    void setText8( lString8 );
    /// returns node absolute rectangle
    void getAbsRect( lvRect & rect );
    /// sets node rendering structure pointer
    void clearRenderData();
    /// calls specified function recursively for all elements of DOM tree
    void recurseElements( void (*pFun)( ldomNode * node ) );
    /// calls specified function recursively for all elements of DOM tree, children before parent
    void recurseElementsDeepFirst( void (*pFun)( ldomNode * node ) );
    /// calls specified function recursively for all nodes of DOM tree
    void recurseNodes( void (*pFun)( ldomNode * node ) );
    /// returns first text child element
    ldomNode * getFirstTextChild( bool skipEmpty=false );
    /// returns last text child element
    ldomNode * getLastTextChild();
    /// find node by coordinates of point in formatted document
    ldomNode * elementFromPoint( lvPoint pt, int direction );
    /// find final node by coordinates of point in formatted document
    ldomNode * finalBlockFromPoint( lvPoint pt );
    // rich interface stubs for supporting Element operations
    /// returns rendering method
    lvdom_element_render_method getRendMethod();
    /// sets rendering method
    void setRendMethod( lvdom_element_render_method );
    /// returns element style record
    css_style_ref_t getStyle();
    /// returns element font
    font_ref_t getFont();
    /// sets element font
    void setFont( font_ref_t );
    /// sets element style record
    void setStyle( css_style_ref_t & );
    /// returns first child node
    ldomNode * getFirstChild() const;
    /// returns last child node
    ldomNode * getLastChild() const;
    /// removes and deletes last child element
    void removeLastChild();
    /// move range of children startChildIndex to endChildIndex inclusively to specified element
    void moveItemsTo( ldomNode *, int , int );
    /// find child element by tag id
    ldomNode * findChildElement( lUInt16 nsid, lUInt16 id, int index );
    /// find child element by id path
    ldomNode * findChildElement( lUInt16 idPath[] );
    /// inserts child element
    ldomNode * insertChildElement( lUInt32 index, lUInt16 nsid, lUInt16 id );
    /// inserts child element
    ldomNode * insertChildElement( lUInt16 id );
    /// inserts child text
    ldomNode * insertChildText( lUInt32 index, const lString16 & value );
    /// inserts child text
    ldomNode * insertChildText( const lString16 & value );
    /// inserts child text
    ldomNode * insertChildText(const lString8 & value);
    /// remove child
    ldomNode * removeChild( lUInt32 index );
    /// returns XPath segment for this element relative to parent element (e.g. "p[10]")
    lString16 getXPathSegment();
    // returns full XPath for node, can be used in navigation through domtree
    lString16 getPath();
    // returns XPath for node with addioinal info, used for logs
    lString16 getXPath();
    /// creates stream to read base64 encoded data from element
    LVStreamRef createBase64Stream();
    /// returns object image source
    LVImageSourceRef getObjectImageSource();
    /// returns object image ref name
    lString16 getObjectImageRefName();
    /// returns object video ref name
    lString16 getObjectVideoRefName();
    /// returns object image stream
    LVStreamRef getObjectImageStream();
    /// formats final block
    int renderFinalBlock(  LFormattedTextRef & frmtext, RenderRectAccessor * fmt, int width );
    /// formats final block again after change, returns true if size of block is changed
    bool refreshFinalBlock();
    /// replace node with r/o persistent implementation
    ldomNode * persist();
    /// replace node with r/w implementation
    ldomNode * modify();
    /// for display:list-item node, get marker
    bool getNodeListMarker( int & counterValue, lString16 & marker, int & markerWidth );
    /// returns href attribute of <A> element, null string if not found
    lString16 getHRef();
    //returns lvrect that contains accumulative sum of all margins of node and all parent nodes
    lvRect getFullMargins();
    //returns string name if any parent name is from white list
    lString16 getMainParentName();
    //returns specified parent node from ancestor nodes. Returns NULL if not found.
    ldomNode *getParentNode(const char *name);

    bool isRTL();

    ldomNode *getSibling(int index);

    ldomNode *getNextSibling();

    ldomNode *getPrevSibling();

    int getListLevel();
};

/**
    Base class for XML DOM documents

    Helps to decrease memory usage and increase performance for DOM implementations.
    Maintains Name<->Id maps for element names, namespaces and attributes.
    It allows to use short IDs instead of strings in DOM internals,
    and avoid duplication of string values. Manages data storage.
*/
class CrDomXml : public CrDomBase {
    friend class ldomNode;
	friend class ldomXPointer;
public:
    /// Default constructor
    CrDomXml();
    /// Destructor
    virtual ~CrDomXml();
    // Name <-> Id maps functions
    /// Get namespace name by id
    /**
        \param id is numeric value of namespace
        \return string value of namespace
    */
    inline const lString16 & getNsName( lUInt16 id )
    {
        return _nsNameTable.StrByInt( id );
    }
    /// Get namespace id by name
    /**
        \param name is string value of namespace
        \return id of namespace
    */
    lUInt16 getNsNameIndex( const lChar16 * name );
    /// Get namespace id by name
    /**
        \param name is string value of namespace (ASCII only)
        \return id of namespace
    */
    lUInt16 getNsNameIndex( const lChar8 * name );
    /// Get attribute name by id
    /**
        \param id is numeric value of attribute
        \return string value of attribute
    */
    inline const lString16 & getAttrName(lUInt16 id)
    {
        return _attrNameTable.StrByInt(id);
    }
    /// Get attribute id by name
    /**
        \param name is string value of attribute
        \return id of attribute
    */
    lUInt16 getAttrNameIndex( const lChar16 * name );
    /// Get attribute id by name
    /**
        \param name is string value of attribute (8bit ASCII only)
        \return id of attribute
    */
    lUInt16 getAttrNameIndex( const lChar8 * name );
    /// helper: returns attribute value
    inline const lString16 & getAttrValue( lUInt16 index ) const
    {
        return _attrValueTable[index];
    }
    /// helper: returns attribute value index
    inline lUInt16 getAttrValueIndex( const lChar16 * value )
    {
        return (lUInt16)_attrValueTable.add( value );
    }
    /// helper: returns attribute value index, 0xffff if not found
    inline lUInt16 findAttrValueIndex( const lChar16 * value )
    {
        return (lUInt16)_attrValueTable.find( value );
    }
    /// Get element name by id
    /**
        \param id is numeric value of element name
        \return string value of element name
    */
    inline const lString16 & getElementName( lUInt16 id )
    {
        return _elementNameTable.StrByInt( id );
    }
    /// Get element id by name
    /**
        \param name is string value of element name
        \return id of element
    */
    lUInt16 getElementNameIndex( const lChar16 * name );
    /// Get element id by name
    /**
        \param name is string value of element name (8bit ASCII only)
        \return id of element, allocates new ID if not found
    */
    lUInt16 getElementNameIndex( const lChar8 * name );
    /// Get element id by name
    /**
        \param name is string value of element name (8bit ASCII only)
        \return id of element, 0 if not found
    */
    lUInt16 findElementNameIndex( const lChar8 * name );
    /// Get element type properties structure by id
    /**
        \param id is element id
        \return pointer to elem_def_t structure containing type properties
        \sa elem_def_t
    */
    inline const css_elem_def_props_t * getElementTypePtr( lUInt16 id )
    {
        return _elementNameTable.dataById( id );
    }
    // set node types from table
    void setNodeTypes( const elem_def_t * node_scheme );
    // set attribute types from table
    void setAttributeTypes( const attr_def_t * attr_scheme );
    // set namespace types from table
    void setNameSpaceTypes( const ns_def_t * ns_scheme );
    // debug dump
    void dumpUnknownEntities( const char * fname );
    /// garbage collector
    virtual void gc() { fontMan->gc(); }
    inline LVStyleSheet* getStylesheet() { return &stylesheet_; }
    /// Sets style sheet, clears old content of css if arg replace is true
    void setStylesheet(const char* css, bool replace);
    void onAttributeSet( lUInt16 attrId, lUInt16 valueId, ldomNode * node );
    /// get element by id attribute value code
    inline ldomNode * getNodeById( lUInt16 attrValueId )
    {
        return getTinyNode( _idNodeMap.get( attrValueId ) );
    }
    /// get element by id attribute value
    inline ldomNode * getElementById( const lChar16 * id )
    {
        lUInt16 attrValueId = getAttrValueIndex( id );
        ldomNode * node = getNodeById( attrValueId );
        return node;
    }
    /// returns root element
    ldomNode * getRootNode();
    /// returns code base path relative to document container
    inline lString16 getCodeBase() { return getProps()->getStringDef(DOC_PROP_CODE_BASE, ""); }
    /// sets code base path relative to document container
    inline void setCodeBase(const lString16& codeBase)
    {
        getProps()->setStringDef(DOC_PROP_CODE_BASE, codeBase);
    }
    /// create formatted text object with options set
    LFormattedText * createFormattedText();
    void setHightlightOptions(text_highlight_options_t & options) {
        _highlightOptions = options;
    }
protected:
    struct DocFileHeader {
        lUInt32 render_dx;
        lUInt32 render_dy;
        lUInt32 render_docflags;
        lUInt32 render_style_hash;
        lUInt32 stylesheet_hash;

        DocFileHeader()
                : render_dx(0),
                  render_dy(0),
                  render_docflags(0),
                  render_style_hash(0),
                  stylesheet_hash(0) {}
    };
    DocFileHeader _hdr;
    text_highlight_options_t _highlightOptions;

    LvDomNameIdMap _elementNameTable;    // Element Name<->Id map
    LvDomNameIdMap _attrNameTable;       // Attribute Name<->Id map
    LvDomNameIdMap _nsNameTable;          // Namespace Name<->Id map
    lUInt16       _nextUnknownElementId; // Next Id for unknown element
    lUInt16       _nextUnknownAttrId;    // Next Id for unknown attribute
    lUInt16       _nextUnknownNsId;      // Next Id for unknown namespace
    lString16HashedCollection _attrValueTable;
    LVHashTable<lUInt16,lInt32> _idNodeMap; // id to data index map
    LVHashTable<lString16,LVImageSourceRef> _urlImageMap; // url to image source map
    lUInt16 _idAttrId; // Id for "id" attribute name
    lUInt16 _nameAttrId; // Id for "name" attribute name

    SerialBuf _pagesData;
};

struct lxmlAttribute
{
    lUInt16 nsid;
    lUInt16 id;
    lUInt16 index;
    inline bool compare( lUInt16 nsId, lUInt16 attrId )
    {
        return (nsId == nsid || nsId == LXML_NS_ANY) && (id == attrId);
    }
    inline void setData( lUInt16 nsId, lUInt16 attrId, lUInt16 valueIndex )
    {
        nsid = nsId;
        id = attrId;
        index = valueIndex;
    }
};

class CrDom;

/**
 * @brief XPointer/XPath object with reference counting.
 */
class ldomXPointer
{
protected:
	struct XPointerData {
	protected:
		CrDom * _doc;
		lInt32 _dataIndex;
		int _offset;
		int _refCount;
	public:
		inline void addRef() { _refCount++; }
		inline void release() { if ( (--_refCount)==0 ) delete this; }
		// create empty
		XPointerData() : _doc(NULL), _dataIndex(0), _offset(0), _refCount(1) { }
		// create instance
        XPointerData( ldomNode * node, int offset )
			: _doc(node ? node->getCrDom() : NULL)
			, _dataIndex(node ? node->getDataIndex() : 0)
			, _offset( offset )
			, _refCount( 1 ) {}
        // clone
        XPointerData(const XPointerData& v)
                : _doc(v._doc),
                  _dataIndex(v._dataIndex),
                  _offset(v._offset),
                  _refCount(1) {}
		inline CrDom * getDocument() { return _doc; }
        inline bool operator == (const XPointerData & v) const
		{
			return _doc==v._doc && _dataIndex == v._dataIndex && _offset == v._offset;
		}
		inline bool operator != (const XPointerData & v) const
		{
            return _doc != v._doc || _dataIndex != v._dataIndex || _offset != v._offset;
		}
        inline bool isNull()
        {
            return _dataIndex == 0;
        }
        inline ldomNode* getNode()
        {
            return _dataIndex > 0 ? ((CrDomXml*) _doc)->getTinyNode(_dataIndex) : NULL;
        }
        inline int getOffset()
        {
            return _offset;
        }
        inline void setNode( ldomNode * node )
		{
			if ( node ) {
				_doc = node->getCrDom();
				_dataIndex = node->getDataIndex();
			} else {
				_doc = NULL;
				_dataIndex = 0;
			}
		}
		inline void setOffset( int offset ) { _offset = offset; }
        inline void addOffset( int offset ) { _offset+=offset; }
        ~XPointerData() {}
	};
	/// node pointer
    //ldomNode * _node;
	/// offset within node for pointer, -1 for xpath
	//int _offset;
	// cloning constructor
	ldomXPointer(const XPointerData* data) : _data(new XPointerData(*data)) {}
public:
	XPointerData * _data;
    /// clear pointer (make null)
    void clear() { *this = ldomXPointer(); }
    /// returns node pointer
    inline ldomNode * getNode() const { return _data->getNode(); }
    /// return parent final node, if found
    ldomNode * getFinalNode() const;
    /// returns offset within node
	inline int getOffset() const { return _data->getOffset(); }
	/// set pointer node
    inline void setNode( ldomNode * node ) { _data->setNode( node ); }
	/// set pointer offset within node
	inline void setOffset( int offset ) { _data->setOffset( offset ); }
    /// default constructor makes NULL pointer
	ldomXPointer() : _data( new XPointerData() ) {}
	/// remove reference
	~ldomXPointer() { _data->release(); }
    /// copy constructor
	ldomXPointer( const ldomXPointer& v ) : _data(v._data) { _data->addRef(); }
    /// assignment operator
	ldomXPointer & operator =( const ldomXPointer& v )
	{
		if ( _data==v._data )
			return *this;
		_data->release();
		_data = v._data;
		_data->addRef();
        return *this;
	}
    /// constructor
    ldomXPointer( ldomNode * node, int offset )	: _data( new XPointerData( node, offset ) )	{}
    /// get pointer for relative path
    ldomXPointer relative( lString16 relativePath );
    /// get pointer for relative path
    ldomXPointer relative( const lChar16 * relativePath )
    {
        return relative( lString16(relativePath) );
    }
    /// returns true for NULL pointer
	bool isNull() const
	{
        return !this || !_data || _data->isNull();
	}
    /// returns true if object is pointer
	bool isPointer() const
	{
		return !_data->isNull() && getOffset()>=0;
	}
    /// returns true if object is path (no offset specified)
	bool isPath() const
	{
		return !_data->isNull() && getOffset()==-1;
	}
    /// returns true if pointer is NULL
	bool operator !() const
	{
		return _data->isNull();
	}
    /// returns true if pointers are equal
	bool operator == (const ldomXPointer & v) const
	{
		return *_data == *v._data;
	}
    /// returns true if pointers are not equal
	bool operator != (const ldomXPointer & v) const
	{
		return *_data != *v._data;
	}
    /// returns caret rectangle for pointer inside formatted document
    bool getRect(lvRect & rect, bool forlvpoint = false) const;
    /// returns coordinates of pointer inside formatted document
    lvPoint toPoint() const;
    /// converts to string
	lString16 toString();
    /// returns XPath node text
    lString16 getText(lChar16 blockDelimiter=0)
    {
        ldomNode * node = getNode();
        if ( !node )
            return lString16::empty_str;
        return node->getText( blockDelimiter );
    }
    /// returns href attribute of <A> element, null string if not found
    lString16 getHRef();
    lString16 getImgHRef();
	/// create a copy of pointer data
	ldomXPointer * clone()
	{
		return new ldomXPointer( _data );
	}
    /// returns true if current node is element
    inline bool isElement() const { return !isNull() && getNode()->isElement(); }
    /// returns true if current node is element
    inline bool isText() const { return !isNull() && getNode()->isText(); }
};

/// Xpointer optimized to iterate through DOM tree
class ldomXPointerEx : public ldomXPointer
{
protected:
    int _indexes[MAX_DOM_LEVEL];
    int _level;
    void initIndex();
public:
    /// returns bottom level index
    int getIndex() { return _indexes[_level-1]; }
    /// returns node level
    int getLevel() { return _level; }
    /// default constructor
    ldomXPointerEx() : ldomXPointer() { initIndex(); }
    /// constructor by node pointer and offset
    ldomXPointerEx(ldomNode* node, int offset) : ldomXPointer( node, offset )
    {
        initIndex();
    }
    /// copy constructor
    ldomXPointerEx( const ldomXPointer& v ) : ldomXPointer( v._data )
    {
        initIndex();
    }
    /// copy constructor
    ldomXPointerEx( const ldomXPointerEx& v ) : ldomXPointer( v._data )
    {
        _level = v._level;
        for ( int i=0; i<_level; i++ )
            _indexes[ i ] = v._indexes[i];
    }
    /// assignment operator
    ldomXPointerEx & operator =( const ldomXPointer& v )
    {
		if ( _data==v._data )
			return *this;
		_data->release();
		_data = new XPointerData( *v._data );
        initIndex();
        return *this;
    }
    /// assignment operator
    ldomXPointerEx & operator =( const ldomXPointerEx& v )
    {
        if ( _data==v._data )
            return *this;
        _data->release();
        _data = new XPointerData( *v._data );
        _level = v._level;
        for ( int i=0; i<_level; i++ )
            _indexes[ i ] = v._indexes[i];
        return *this;
    }
    /// returns true if ranges are equal
    bool operator==(const ldomXPointerEx& v) const
    {
        return _data->getDocument() == v._data->getDocument() &&
               _data->getNode() == v._data->getNode() && _data->getOffset() == v._data->getOffset();
    }
    /// searches path for element with specific id,
    /// returns level at which element is founs, 0 if not found
    int findElementInPath( lUInt16 id );
    /// compare two pointers, returns -1, 0, +1
    int compare( const ldomXPointerEx& v ) const;
    /// move to next sibling
    bool nextSibling();
    /// move to previous sibling
    bool prevSibling();
    /// move to next sibling element
    bool nextSiblingElement();
    /// move to previous sibling element
    bool prevSiblingElement();
    /// move to parent
    bool parent();
    /// move to first child of current node
    bool firstChild();
    /// move to last child of current node
    bool lastChild();
    /// move to first element child of current node
    bool firstElementChild();
    /// move to last element child of current node
    bool lastElementChild();
    /// move to child #
    bool child( int index );
    /// move to sibling #
    bool sibling( int index );
    /// ensure that current node is element (move to parent, if not - from text node to element)
    bool ensureElement();
    /// moves pointer to parent element with FINAL render method, returns true if success
    bool ensureFinal();
    /// returns true if current node is visible element with render method == erm_final
    bool isVisibleFinal();
    /// move to next final visible node (~paragraph)
    bool nextVisibleFinal();
    /// move to previous final visible node (~paragraph)
    bool prevVisibleFinal();
    /// returns true if current node is visible element or text
    bool isVisible();
    /// move to next text node
    bool nextText( bool thisBlockOnly = false );
    /// move to previous text node
    bool prevText( bool thisBlockOnly = false );
    /// move to next visible text node
    bool nextVisibleText( bool thisBlockOnly = false );
    /// move to previous visible text node
    bool prevVisibleText( bool thisBlockOnly = false );
    /// move to previous visible word beginning
    bool prevVisibleWordStart( bool thisBlockOnly = false );
    /// move to previous visible word end
    bool prevVisibleWordEnd( bool thisBlockOnly = false );
    /// move to next visible word beginning
    bool nextVisibleWordStart( bool thisBlockOnly = false );
    /// move to end of current word
    bool thisVisibleWordEnd( bool thisBlockOnly = false );
    /// move to next visible word end
    bool nextVisibleWordEnd( bool thisBlockOnly = false );
    /// move to beginning of current visible text sentence
    bool thisSentenceStart();
    /// move to end of current visible text sentence
    bool thisSentenceEnd();
    /// move to beginning of next visible text sentence
    bool nextSentenceStart();
    /// move to beginning of next visible text sentence
    bool prevSentenceStart();
    /// move to end of next visible text sentence
    bool nextSentenceEnd();
    /// move to end of prev visible text sentence
    bool prevSentenceEnd();
    /// returns true if points to beginning of sentence
    bool isSentenceStart();
    /// returns true if points to end of sentence
    bool isSentenceEnd();
    /// returns true if points to last visible text inside block element
    bool isLastVisibleTextInBlock();
    /// returns true if points to first visible text inside block element
    bool isFirstVisibleTextInBlock();
    /// returns block owner node of current node (or current node if it's block)
    ldomNode* getThisBlockNode();
    /// returns true if current position is visible word beginning
    bool isVisibleWordStart();
    /// returns true if current position is visible word end
    bool isVisibleWordEnd();
    /// forward iteration by elements of DOM three
    bool nextElement();
    /// backward iteration by elements of DOM three
    bool prevElement();
    /// calls specified function recursively for all elements of DOM tree
    void recurseElements( void (*pFun)( ldomXPointerEx & node ) );
    /// calls specified function recursively for all nodes of DOM tree
    void recurseNodes( void (*pFun)( ldomXPointerEx & node ) );
};

class ldomXRange;

/// callback for DOM tree iteration interface
class ldomNodeCallback {
public:
    /// destructor
    virtual ~ldomNodeCallback() { }
    /// called for each found text fragment in range
    virtual void onText(ldomXRange*) { }
    /// called for each found node in range
    virtual bool onElement(ldomNode*) { return true; }
    virtual void processText(ldomNode* node, ldomXRange* range);
    virtual bool processElement(ldomNode* node, ldomXRange* range);
};

/// range for word inside text node
class ldomWord
{
    ldomNode * _node;
    int _start;
    int _end;
public:
    lChar16 _text_cached = '@'; // search fix
    ldomWord( ) : _node(NULL), _start(0), _end(0) { }
    ldomWord( ldomNode * node, int start, int end ) : _node(node), _start(start), _end(end) { }

    //IF YOU ADD ANY FIELDS TO THIS CLASS DONT FORGET TO ADD THEM HERE for arrays to work properly!!!!! hrs wasted on this = 3
    ldomWord( const ldomWord & v ) : _node(v._node), _start(v._start), _end(v._end), _text_cached(v._text_cached) { }
    ldomWord & operator = ( const ldomWord & v )
    {
        _node = v._node;
        _start = v._start;
        _end = v._end;
        _text_cached = v._text_cached;
        //IF YOU ADD ANY FIELDS TO THIS CLASS DONT FORGET TO ADD THEM HERE for arrays to work properly!!!!! hrs wasted on this = 3
        return *this;
    }
    /// returns true if object doesn't point valid word
    bool isNull() { return _node==NULL || _start<0 || _end<=_start; }
    /// get word text node pointer
    ldomNode * getNode() const { return _node; }
    /// get word start offset
    int getStart() const { return _start; }
    /// get word end offset
    int getEnd() const { return _end; }
    /// get word start XPointer
    ldomXPointer getStartXPointer() const { return ldomXPointer( _node, _start ); }
    /// get word start XPointer
    ldomXPointer getEndXPointer() const { return ldomXPointer( _node, _end ); }
    /// get word text
    lString16 getText()
    {
        if ( isNull() )
            return lString16::empty_str;
        lString16 txt = _node->getText();
        return txt.substr( _start, _end-_start );
    }
    lvRect getRect();
    void setCachedText (lChar16 ch){ _text_cached = ch; }
    lChar16 getCachedText() { return _text_cached; }
};

class TextRect
{
private:
    ldomNode* node_;
    ldomWord word_;
    lvRect rect_;
    lString16 string_;
    int index_ = -1;
public:
    TextRect() :  word_(),node_(nullptr),rect_(lvRect(0,0,0,0)),string_(lString16::empty_str) {}
    TextRect(ldomNode* node,lvRect rect, lString16 string) :node_(node),rect_(rect)
    {
        string_ = string.ReplaceUnusualSpaces();
    }

    TextRect(ldomNode* node,lvRect rect, lString16 string, ldomWord word) :node_(node),rect_(rect),word_(word)
    {
        string_ = string.ReplaceUnusualSpaces();
    }


    lString16 getText(){ return string_;};
    lvRect getRect(){ return rect_;};
    ldomNode* getNode(){ return node_;};
    ldomWord getWord(){return word_;};
    int getIndex(){return index_;};
    void setRect(lvRect rect){ rect_ = rect;};
    void setText(lString16 string){ string_ = string;};
    void setIndex(int index){ index_ = index;};
    int getWidthRTL(LVFont * font);
};

class ImgRect
{
private:
    ldomNode* node_;
    lvRect rect_;
public:
    ImgRect() :  node_(nullptr),rect_(lvRect(0,0,0,0)) {}
    ImgRect(ldomNode* node,lvRect rect ) :node_(node),rect_(rect){}

    lvRect getRect(){ return rect_;};
    ldomNode* getNode(){ return node_;};
};


/// DOM range
class ldomXRange {
    ldomXPointerEx _start;
    ldomXPointerEx _end;
    lUInt32 _flags;
    ldomNode* _startnode = NULL;
    ldomNode* _endnode = NULL;
public:
    ldomXRange() : _flags(0) { }
    ldomXRange( const ldomXPointerEx & start, const ldomXPointerEx & end, lUInt32 flags=0 )
            : _start( start ),
              _end( end ),
              _flags(flags) {}
    ldomXRange( const ldomXPointer & start, const ldomXPointer & end )
            : _start( start ),
              _end( end ),
              _flags(0) {}
    /// copy constructor
    ldomXRange( const ldomXRange & v )
            : _start( v._start ),
              _end( v._end ),
              _flags(v._flags) {}
    ldomXRange( const ldomWord & word )
            : _start( word.getStartXPointer() ),
              _end( word.getEndXPointer() ),
              _flags(1) {}
    /// if start is after end, swap start and end
    void sort();
    /// create intersection of two ranges
    ldomXRange( const ldomXRange & v1,  const ldomXRange & v2 );
    /// copy constructor of full node range
    ldomXRange( ldomNode * p );
    /// copy assignment
    ldomXRange & operator = ( const ldomXRange & v )
    {
        _start = v._start;
        _end = v._end;
        return *this;
    }
    /// returns true if ranges are equal
    bool operator == ( const ldomXRange & v ) const
    {
        return _start == v._start && _end == v._end && _flags==v._flags;
    }
    /// returns true if interval is invalid or empty
    bool isNull()
    {
        if ( _start.isNull() || _end.isNull() )
            return true;
        if ( _start.compare( _end ) > 0 )
            return true;
        return false;
    }
    /// makes range empty
    void clear()
    {
        _start.clear();
        _end.clear();
        _flags = 0;
    }
    /// returns true if pointer position is inside range
    bool isInside( const ldomXPointerEx & p ) const
    {
        return ( _start.compare( p ) <= 0 && _end.compare( p ) >= 0 );
    }
    /// returns interval start point
    ldomXPointerEx & getStart() { return _start; }
    /// returns interval end point
    ldomXPointerEx & getEnd() { return _end; }
    /// sets interval start point
    void setStart( ldomXPointerEx & start ) { _start = start; }
    /// sets interval end point
    void setEnd( ldomXPointerEx & end ) { _end = end; }
    /// returns flags value
    lUInt32 getFlags() { return _flags; }
    /// sets new flags value
    void setFlags( lUInt32 flags ) { _flags = flags; }
    /// returns true if this interval intersects specified interval
    bool checkIntersection( ldomXRange & v );
    /// get all words from specified range
    void getRangeWords( LVArray<ldomWord> & list );

    void getRangeChars( LVArray<TextRect> & list ,int clip_width, bool rtl_enable = true, bool rtl_space = true);
    /// returns href attribute of <A> element, null string if not found
    lString16 getHRef();
    /// sets range to nearest word bounds, returns true if success
    static bool getWordRange( ldomXRange & range, ldomXPointer & p );
    /// run callback for each node in range REIMPLEMENTED FOR TEXT HITBOX EXTRACTION
    void forEach2( ldomNodeCallback * callback );
    /// returns rectangle (in doc coordinates) for range. Returns true if found.
    bool getRect( lvRect & rect );
    /// returns nearest common element for start and end points
    ldomNode* getNearestCommonParent();
    /// searches for specified text inside range
    bool findText(lString16 pattern, bool caseInsensitive, bool reverse, LVArray<ldomWord>& words,
                  int maxCount, int maxHeight, bool checkMaxFromStart = false);
    //foreach2 process text node
    void processText(ldomNode* node, ldomNodeCallback *callback);
    //foreach2 process element node
    bool processElement(ldomNode *node, ldomNodeCallback *callback);
    //foreach2 get node that contains xpointer of end of range
    ldomNode* getEndNode();
    //foreach2 get node that contains xpointer of start of range
    ldomNode* getStartNode();
    //get first common ancestor of nodes n1 and n2
    ldomNode* getAncestor(ldomNode* n1, ldomNode* n2);
    /// returns text between two XPointer positions
    lString16 GetRangeText( lChar16 blockDelimiter='\n', int maxTextLen=0 );

    void getRangeWordsNoRect(LVArray<ldomWord> &words_list);

    LVArray<ldomXPointer> getXpointerByOffsetInPageText(LVArray<int> pos_arr);

    ldomXPointer getXpointerByOffsetInPageText(int pos);
};

class ldomMarkedText
{
public:
    lString16 text;
    lUInt32   flags;
    int offset;
    ldomMarkedText(lString16 s, lUInt32 flg, int offs) : text(s), flags(flg), offset(offs) {}
    ldomMarkedText(const ldomMarkedText & v) : text(v.text), flags(v.flags), offset(0) {}
};

typedef LVPtrVector<ldomMarkedText> ldomMarkedTextList;

enum MoveDirection {
    DIR_ANY,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN
};

/// range in document, marked with specified flags
class ldomMarkedRange
{
public:
    /// start document point
    lvPoint   start;
    /// end document point
    lvPoint   end;
    /// flags
    lUInt32   flags;
    bool empty()
    {
        return ( start.y>end.y || ( start.y == end.y && start.x >= end.x ) );
    }
    /// returns mark middle point for single line mark, or start point for multiline mark
    lvPoint getMiddlePoint();
    /// returns distance (dx+dy) from specified point to middle point
    int calcDistance( int x, int y, MoveDirection dir );
    /// returns true if intersects specified line rectangle
    bool intersects( lvRect & rc, lvRect & intersection );
    /// constructor
    ldomMarkedRange( lvPoint _start, lvPoint _end, lUInt32 _flags )
            : start(_start),
              end(_end),
              flags(_flags) {}
    /// constructor
    ldomMarkedRange( ldomWord & word ) : flags(0)
    {
        ldomXPointer startPos(word.getNode(), word.getStart() );
        ldomXPointer endPos(word.getNode(), word.getEnd() );
        start = startPos.toPoint();
        end = endPos.toPoint();
    }
    /// copy constructor
    ldomMarkedRange( const ldomMarkedRange & v ) : start(v.start), end(v.end), flags(v.flags) {}
};

class ldomWordEx : public ldomWord
{
    ldomWord _word;
    ldomMarkedRange _mark;
    ldomXRange _range;
    lString16 _text;
public:
    ldomWordEx( ldomWord & word ) : _word(word), _mark(word), _range(word)
    {
        _text = _word.getText();
    }
    ldomWord & getWord() { return _word; }
    ldomXRange & getRange() { return _range; }
    ldomMarkedRange & getMark() { return _mark; }
    lString16 & getText() { return _text; }
};

/// list of extended words
class ldomWordExList : public LVPtrVector<ldomWordEx>
{
    int minx;
    int maxx;
    int miny;
    int maxy;
    int x;
    int y;
    ldomWordEx * selWord;
    lString16Collection pattern;
    void init();
    ldomWordEx * findWordByPattern();
public:
    ldomWordExList() : minx(-1), maxx(-1), miny(-1), maxy(-1), x(-1), y(-1), selWord(NULL) {}
    /// adds all visible words from range, returns number of added words
    int addRangeWords( ldomXRange & range, bool trimPunctuation );
    /// find word nearest to specified point
    ldomWordEx * findNearestWord( int x, int y, MoveDirection dir );
    /// select word
    void selectWord( ldomWordEx * word, MoveDirection dir );
    /// select next word in specified direction
    ldomWordEx * selectNextWord( MoveDirection dir, int moveBy = 1 );
    /// select middle word in range
    ldomWordEx * selectMiddleWord();
    /// get selected word
    ldomWordEx * getSelWord() { return selWord; }
    /// try append search pattern and find word
    ldomWordEx * appendPattern(lString16 chars);
    /// remove last character from pattern and try to search
    ldomWordEx * reducePattern();
};

/// list of marked ranges
class ldomMarkedRangeList : public LVPtrVector<ldomMarkedRange>
{
public:
    ldomMarkedRangeList() {}
    /// create bounded by RC list, with (0,0) coordinates at left top corner
    ldomMarkedRangeList( const ldomMarkedRangeList * list, lvRect & rc );
};

class ldomXRangeList : public LVPtrVector<ldomXRange>
{
public:
    /// add ranges for words
    void addWords( const LVArray<ldomWord> & words )
    {
        for ( int i=0; i<words.length(); i++ )
            LVPtrVector<ldomXRange>::add( new ldomXRange( words[i] ) );
    }
    ldomXRangeList( const LVArray<ldomWord> & words )
    {
        addWords( words );
    }
    /// create list splittiny existing list into non-overlapping ranges
    ldomXRangeList( ldomXRangeList & srcList, bool splitIntersections );
    /// create list by filtering existing list, to get only values which intersect filter range
    ldomXRangeList( ldomXRangeList & srcList, ldomXRange & filter );
    /// fill text selection list by splitting text into monotonic flags ranges
    void splitText( ldomMarkedTextList &dst, ldomNode * textNodeToSplit );
    /// fill marked ranges list
    void getRanges( ldomMarkedRangeList &dst );
    /// split into subranges using intersection
    void split( ldomXRange * r );
    /// default constructor for empty list
    ldomXRangeList() {};
};

class LvTocItem
{
private:
    LvTocItem*     	_parent;
    CrDom*  		_doc;
    lInt32          _level;
    lInt32          _index;
    lInt32          _page;
    lString16       _name;
    lString16       _path;
    ldomXPointer    _position;
    LVPtrVector<LvTocItem> _children;

    void addChild(LvTocItem* item)
    {
    	item->_level = _level + 1;
    	item->_parent = this;
    	item->_index = _children.length(),
    	item->_doc = _doc;
    	_children.add(item);
    }
public:
    LvTocItem(ldomXPointer pos, lString16 path, const lString16& name)
            : _parent(NULL),
            _doc(NULL),
            _level(0),
            _index(0),
            _page(0),
            _name(name),
            _path(path),
            _position(pos) { }

    void addItem(LvTocItem* item, int level)
    {
        item->_level = level;
        item->_parent = this;
        item->_index = _children.length(),
        item->_doc = _doc;
        _children.add(item);
    }

    void setPage( int n ) { _page = n; }
    /// get page number
    int getPage() { return _page; }
    /// returns parent node pointer
    LvTocItem * getParent() const { return _parent; }
    /// returns node level (0==root node, 1==top level)
    int getLevel() const { return _level; }
    /// returns node index
    int getIndex() const { return _index; }
    /// returns section title
    lString16 getName() const { return _name; }
    /// returns position pointer
    ldomXPointer getXPointer();
    /// returns position path
    lString16 getPath();
    /// returns Y position
    int getY();
    /// returns child node count
    int getChildCount() const { return _children.length(); }
    /// returns child node by index
    LvTocItem * getChild( int index ) const { return _children[index]; }
    /// add child TOC node
    LvTocItem * addChild( const lString16 & name, ldomXPointer ptr, lString16 path )
    {
        LvTocItem * item = new LvTocItem( ptr, path, name );
        addChild( item );
        return item;
    }
    void clear() { _children.clear(); }
    // root node constructor
    LvTocItem( CrDom * doc ) : _parent(NULL), _doc(doc), _level(0), _index(0), _page(0) {}
    ~LvTocItem() { clear(); }
};

class ListNumberingProps
{
public:
    int maxCounter;
    int maxWidth;
    ListNumberingProps(int c, int w) : maxCounter(c), maxWidth(w) {}
};
typedef LVRef<ListNumberingProps> ListNumberingPropsRef;

class CrDom : public CrDomXml
{
    friend class LvDomWriter;
    friend class LvDomAutocloseWriter;
private:
    LvTocItem m_toc;
    font_ref_t _def_font; // default font
    css_style_ref_t _def_style;
    lString16 stylesheet_file_name_;
    lUInt32 _last_docflags;
    int _page_height;
    int _page_width;
    bool _rendered;
    ldomXRangeList _selections;
    LVContainerRef _container;
    LVHashTable<lUInt32, ListNumberingPropsRef> lists;
    LVEmbeddedFontList _fontList;
protected:
    void applyDocStylesheet();
public:
    CrDom();
    virtual ~CrDom();

    int cfg_txt_indent;
    int cfg_txt_margin;
    bool cfg_txt_indent_margin_override;
    bool force_render;
    EpubStylesManager stylesManager;

    void ApplyEmbeddedStyles();

    void forceReinitStyles() {
        dropStyles();
        _hdr.render_style_hash = 0;
        _rendered = false;
    }
    ListNumberingPropsRef getNodeNumberingProps( lUInt32 nodeDataIndex );
    void setNodeNumberingProps( lUInt32 nodeDataIndex, ListNumberingPropsRef v );
    void resetNodeNumberingProps();
    /// returns object image stream
    LVStreamRef getObjectImageStream( lString16 refName );
    /// returns object image source
    LVImageSourceRef getObjectImageSource( lString16 refName );
    bool isDefStyleSet() { return !_def_style.isNull(); }
    /// return document's embedded font list
    LVEmbeddedFontList & getEmbeddedFontList() { return _fontList; }
    /// register embedded document fonts in font manager, if any exist in document
    void registerEmbeddedFonts();
    /// returns pointer to TOC root node
    LvTocItem * getToc() { return &m_toc; }
    /// save document formatting parameters after render
    void updateRenderContext();
    /// check document formatting parameters before render,
    /// whether we need to reformat; returns false if render is necessary
    bool checkRenderContext();
    LVContainerRef getDocParentContainer() { return _container; }
    void setDocParentContainer( LVContainerRef cont ) { _container = cont; }
    void clearRendBlockCache() { _renderedBlockCache.clear(); }
    void clear();
    /// return selections collection
    ldomXRangeList & getSelections() { return _selections; }
    /// get full document height
    int getFullHeight();
    /// returns page height setting
    int getPageHeight() { return _page_height; }
    /// saves document contents as XML to stream with specified encoding
    bool saveToStream( LVStreamRef stream, const char * codepage, bool treeLayout=false );
    /// get default font reference
    font_ref_t getDefaultFont() { return _def_font; }
    /// get default style reference
    css_style_ref_t getDefaultStyle() { return _def_style; }
    inline bool parseStyleSheet(lString16 codeBase, lString16 css);
    inline bool parseStyleSheet(lString16 cssFile);
    /// renders (formats) document in memory
    virtual int render(LVRendPageList* pages, int width, int dy,
    		bool showCover, int y0, font_ref_t def_font, int def_interline_space );
    /// renders (formats) document in memory
    virtual bool
    setRenderProps(int width, int height, font_ref_t def_font, int def_interline_space);
    /// create xpointer from pointer string
    ldomXPointer createXPointer( const lString16 & xPointerStr );
    /// create xpointer from pointer string
    ldomNode * nodeFromXPath( const lString16 & xPointerStr )
    {
        return createXPointer( xPointerStr ).getNode();
    }
    /// get element text by pointer string
    lString16 textFromXPath( const lString16 & xPointerStr )
    {
        ldomNode * node = nodeFromXPath( xPointerStr );
        if ( !node )
            return lString16::empty_str;
        return node->getText();
    }
    /// create xpointer from relative pointer string
    ldomXPointer createXPointer( ldomNode * baseNode, const lString16 & xPointerStr );
    /// create xpointer from doc point
    ldomXPointer createXPointer( lvPoint pt, int direction=0 );
    /// get rendered block cache object
    CVRendBlockCache & getRendBlockCache() { return _renderedBlockCache; }
    bool findText(lString16 pattern, bool caseInsensitive, bool reverse, int minY, int maxY,
                  LVArray<ldomWord>& words, int maxCount, int maxHeight);

    ldomXPointer convertXPointerFromMuPdf(const lString16 &xPointerStr);
    lString16    convertXPointerToMuPdf(ldomXPointer xp);

    void ResetEmbeddedTextIndent(int indent);
    void ResetEmbeddedParagraphMargin(int margin_bottom);
};

class LvDomWriter;

class ldomElementWriter
{
    ldomElementWriter * _parent;
    CrDom * _document;
    ldomNode * _element;
    LvTocItem * _tocItem;
    lString16 _path;
    const css_elem_def_props_t * _typeDef;
    bool _allowText;
    bool _isBlock;
    bool _isSection;
    bool stylesheet_IsSet;
    bool _bodyEnterCalled;
    lUInt32 _flags;
    lUInt32 getFlags();
    void updateTocItem();
    void onBodyEnter();
    void onBodyExit();
    ldomNode * getElement() { return _element; }
    lString16 getPath();
    void onText(const lChar16 * text, int len, lUInt32 flags);
    void addAttribute(lUInt16 nsid, lUInt16 id, const wchar_t * value);
    //lxmlElementWriter * pop( lUInt16 id );
    friend class LvDomWriter;
    friend class LvDomAutocloseWriter;
    //friend ldomElementWriter * pop( ldomElementWriter * obj, lUInt16 id );
    ldomElementWriter(CrDom * document, lUInt16 nsid, lUInt16 id, ldomElementWriter * parent, lUInt32 flags);
    ~ldomElementWriter();
};

/** \brief callback object to fill DOM tree

    To be used with XML parser as callback object.

    Creates document according to incoming events.
*/
class LvDomWriter : public LvXMLParserCallback
{
protected:
    CrDom* doc_;
    //ldomElement * _currNode;
    ldomElementWriter * _currNode;
    bool _errFlag;
    bool _headerOnly;
    bool _popStyleOnFinish;
    lUInt16 _stopTagId;
    lUInt32 _flags;
    bool RTLflag_ = false;
    virtual void ElementCloseHandler( ldomNode * node ) { node->persist(); }
public:
    void setRTLflag(bool RTLflag) {RTLflag_ = RTLflag;}

    bool getRTLflag() { return RTLflag_; }

    /// returns flags
    virtual lUInt32 getFlags() { return _flags; }
    /// sets flags
    virtual void setFlags( lUInt32 flags ) { _flags = flags; }
    // overrides
    /// called when encoding directive found in document
    virtual void OnEncoding( const lChar16 * name, const lChar16 * table );
    /// called on parsing start
    virtual void OnStart(LVFileFormatParser * parser);
    /// called on parsing end
    virtual void OnStop();
    /// called on opening tag
    virtual ldomNode* OnTagOpen(const lChar16* nsname, const lChar16* tagname);
    /// called after > of opening tag (when entering tag body)
    virtual void OnTagBody();
    /// called on closing tag
    virtual void OnTagClose(const lChar16* nsname, const lChar16* tagname);
    /// called on attribute
    virtual void OnAttribute(const lChar16* nsname,
    		const lChar16* attrname, const lChar16* attrvalue);
    /// close tags
    ldomElementWriter* pop(ldomElementWriter* obj, lUInt16 id);
    virtual void OnText(const lChar16* text, int len, lUInt32 flags);
    /// add named BLOB data to document
    virtual bool OnBlob(lString16 name, const lUInt8 * data, int size)
    {
    	return doc_->addBlob(name, data, size);
    }
    /// set document property
    virtual void OnDocProperty(const char * name, lString8 value)
    {
    	doc_->getProps()->setString(name, value);
    }
    LvDomWriter(CrDom* document, bool headerOnly=false);
    lString16 convertHref( lString16 href )
    {
        return href;
    }
    lString16 convertId( lString16 href )
    {
        return href;
    }
    virtual ~LvDomWriter();
};

/** \brief callback object to fill DOM tree

    To be used with XML parser as callback object.

    Creates document according to incoming events.

    Autoclose HTML tags.
*/
class LvDomAutocloseWriter : public LvDomWriter {
protected:
    bool _libRuDocumentDetected;
    bool _libRuParagraphStart;
    lUInt16 _styleAttrId;
    lUInt16 _classAttrId;
    lUInt16* _rules[MAX_ELEMENT_TYPE_ID];
    bool _tagBodyCalled;
    virtual void AutoClose(lUInt16 tag_id, bool open);
    virtual void ElementCloseHandler(ldomNode * elem);
    virtual void appendStyle(const lChar16 * style);
    virtual void setClass(const lChar16 * className, bool overrideExisting = false);

public:
    virtual void OnAttribute(const lChar16* nsname,
    		const lChar16* attrname, const lChar16* attrvalue);
    virtual ldomNode* OnTagOpen(const lChar16* nsname, const lChar16* tagname);
    /// called after > of opening tag (when entering tag body)
    virtual void OnTagBody();
    virtual void OnTagClose(const lChar16* nsname, const lChar16* tagname);
    /// called on text
    virtual void OnText(const lChar16* text, int len, lUInt32 flags);
    LvDomAutocloseWriter(CrDom* document, bool headerOnly, const char*** rules);
    virtual ~LvDomAutocloseWriter();
};

class LvDocFragmentWriter : public LvXMLParserCallback {
private:
    LvXMLParserCallback* parent;
    lString16 baseTag;
    lString16 baseTagReplacement;
    lString16 codeBase;
    lString16 filePathName;
    lString16 codeBasePrefix;
    lString16 stylesheetFile;
    lString16 tmpStylesheetFile;
    lString16Collection stylesheetLinks;
    bool insideTag;
    int styleDetectionState;
    LVHashTable<lString16, lString16> pathSubstitutions;
    ldomNode* baseElement;
    ldomNode* lastBaseElement;
    lString8 headStyleText;
    int headStyleState;
    bool RTLflag_;

public:
    void setRTLflag (bool RTLflag) {RTLflag_ = RTLflag; }
    bool getRTLflag () { return RTLflag_; }
    /// return content of html/head/style element
    lString8 getHeadStyleText() { return headStyleText; }

    ldomNode * getBaseElement() { return lastBaseElement; }

    lString16 convertId( lString16 id );
    lString16 convertHref( lString16 href );

    void addPathSubstitution( lString16 key, lString16 value )
    {
        pathSubstitutions.set(key, value);
    }

    virtual void setCodeBase( lString16 filePath );
    /// returns flags
    virtual lUInt32 getFlags() { return parent->getFlags(); }
    /// sets flags
    virtual void setFlags( lUInt32 flags ) { parent->setFlags(flags); }
    // overrides
    /// called when encoding directive found in document
    virtual void OnEncoding( const lChar16 * name, const lChar16 * table )
    { parent->OnEncoding( name, table ); }
    /// called on parsing start
    virtual void OnStart(LVFileFormatParser *)
    {
        insideTag = false;
        headStyleText.clear();
        headStyleState = 0;
    }
    /// called on parsing end
    virtual void OnStop()
    {
        if ( insideTag ) {
            insideTag = false;
            if ( !baseTagReplacement.empty() ) {
                parent->OnTagClose(L"", baseTagReplacement.c_str());
            }
            baseElement = NULL;
            return;
        }
        insideTag = false;
    }
    /// called on opening tag
    virtual ldomNode * OnTagOpen( const lChar16 * nsname, const lChar16 * tagname );
    /// called after > of opening tag (when entering tag body)
    virtual void OnTagBody();
    /// called on closing tag
    virtual void OnTagClose( const lChar16 * nsname, const lChar16 * tagname );
    /// called on attribute
    virtual void
    OnAttribute(const lChar16* nsname, const lChar16* attrname, const lChar16* attrvalue);
    /// called on text
    virtual void OnText( const lChar16 * text, int len, lUInt32 flags )
    {
        if (headStyleState == 1) {
            headStyleText << UnicodeToUtf8(lString16(text));
            return;
        }
        if ( insideTag )
        {
            if (this->RTLflag_ && RTL_DISPLAY_ENABLE && gDocumentFormat != DOC_FORMAT_DOCX)
            {
                parent->OnText(lString16(text).PrepareRTL().c_str(), len, flags);
            }
            else
            {
                parent->OnText(text, len, flags);
            }
        }
    }

    /// add named BLOB data to document
    virtual bool OnBlob(lString16 name, const lUInt8* data, int size)
    { return parent->OnBlob(name, data, size); }

    /// set document property
    virtual void OnDocProperty(const char* name, lString8 value)
    { parent->OnDocProperty(name, value); }

    LvDocFragmentWriter(LvXMLParserCallback* parentWriter,
    		lString16 baseTagName, lString16 baseTagReplacementName, lString16 fragmentFilePath)
    		: parent(parentWriter),
    		  baseTag(baseTagName),
    		  baseTagReplacement(baseTagReplacementName),
    		  insideTag(false),
    		  styleDetectionState(0),
    		  pathSubstitutions(100),
    		  baseElement(NULL),
    		  lastBaseElement(NULL),
    		  headStyleState(0)
    {
        setCodeBase(fragmentFilePath);
    }

    virtual ~LvDocFragmentWriter() { }
};

/// parse XML document from stream, returns NULL if failed
CrDom* LVParseXMLStream(
        LVStreamRef stream,
        const elem_def_t * elem_table=NULL,
        const attr_def_t * attr_table=NULL,
        const ns_def_t * ns_table=NULL);

/// parse XML document from stream, returns NULL if failed
CrDom* LVParseHTMLStream(
        LVStreamRef stream,
        const elem_def_t * elem_table=NULL,
        const attr_def_t * attr_table=NULL,
        const ns_def_t * ns_table=NULL);

/// extract authors from FB2 document, delimiter is lString16 by default
lString16 ExtractDocAuthors(CrDom* dom, lString16 delimiter = lString16::empty_str);
lString16 ExtractDocTitle(CrDom* dom);
lString16 ExtractDocLanguage(CrDom* dom);
/// returns "(Series Name #number)" if pSeriesNumber is NULL, separate name and number otherwise
lString16 ExtractDocSeries(CrDom* dom, int* pSeriesNumber=NULL);
lString16 ExtractDocThumbImageName(CrDom* dom);
lString16 ExtractDocGenres(CrDom *dom, lString16 delimiter);
lString16 ExtractDocAnnotation(CrDom* dom);

void RecurseTOC(ldomNode * node,LvTocItem * toc, bool deepSearch, bool allow_title = false);
void GetTOC(CrDom * crDom, LvTocItem * toc, bool deepSearch = false, bool allow_title = false);
void FixOdtSpaces(ldomNode *node);
void PrettyPrintDocx(ldomNode *node);
void PrettyPrint_indic(ldomNode *node);

#endif
