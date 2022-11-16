/** Font manager interface

    CoolReader Engine

    (c) Vadim Lopatin, 2000-2006
    Copyright (C) 2013-2020 READERA LLC

    This source code is distributed under the terms of
    GNU General Public License.

    See LICENSE file for details.
*/

#ifndef __LV_FNT_MAN_H_INCLUDED__
#define __LV_FNT_MAN_H_INCLUDED__

#include <stdlib.h>
#include "lvfnt.h"
#include "cssdef.h"
#include "lvstring.h"
#include "lvref.h"
#include "lvptrvec.h"
#include "hyphman.h"
#include "lvdrawbuf.h"

class LVDrawBuf;

struct LVFontGlyphCacheItem;

class LVFontGlobalGlyphCache
{
private:
    LVFontGlyphCacheItem * head;
    LVFontGlyphCacheItem * tail;
    int size;
    int max_size;
    void removeNoLock( LVFontGlyphCacheItem * item );
    void putNoLock( LVFontGlyphCacheItem * item );
public:
    LVFontGlobalGlyphCache( int maxSize )
        : head(NULL), tail(NULL), size(0), max_size(maxSize )
    {
    }
    ~LVFontGlobalGlyphCache()
    {
        clear();
    }
    void put( LVFontGlyphCacheItem * item );
    void remove( LVFontGlyphCacheItem * item );
    void refresh( LVFontGlyphCacheItem * item );
    void clear();
};

class LVFontLocalGlyphCache
{
private:
    LVFontGlyphCacheItem * head;
    LVFontGlyphCacheItem * tail;
    LVFontGlobalGlyphCache * global_cache;
    int size;
public:
    LVFontLocalGlyphCache( LVFontGlobalGlyphCache * globalCache )
        : head(NULL), tail(NULL), global_cache( globalCache )
    { }
    ~LVFontLocalGlyphCache()
    {
        clear();
    }
    void clear();
    LVFontGlyphCacheItem * get( lUInt16 ch );
    void put( LVFontGlyphCacheItem * item );
    void remove( LVFontGlyphCacheItem * item );
};

class LVFontGlyphCacheItem
{
public:
    LVFontGlyphCacheItem * prev_global;
    LVFontGlyphCacheItem * next_global;
    LVFontGlyphCacheItem * prev_local;
    LVFontGlyphCacheItem * next_local;
    LVFontLocalGlyphCache * local_cache;
    lChar16 ch;
    lUInt8 bmp_width;
    lUInt8 bmp_height;
    lInt8  origin_x;
    lInt8  origin_y;
    lUInt8 advance;
    lUInt8 v_advance;
    lUInt8 v_bearingX;
    lUInt8 v_bearingY;
    lUInt8 bmp[1];

    int getSize()
    {
        return sizeof(LVFontGlyphCacheItem)
            + (bmp_width * bmp_height - 1) * sizeof(lUInt8);
    }
    static LVFontGlyphCacheItem * newItem( LVFontLocalGlyphCache * local_cache, lChar16 ch, int w, int h )
    {
        LVFontGlyphCacheItem * item = (LVFontGlyphCacheItem *)malloc( sizeof(LVFontGlyphCacheItem)
            + (w*h - 1)*sizeof(lUInt8) );
        item->ch = ch;
        item->bmp_width   = (lUInt8)w;
        item->bmp_height  = (lUInt8)h;
        item->origin_x    = 0;
        item->origin_y    = 0;
        item->advance     = 0;
        item->v_advance   = 0;
        item->v_bearingX  = 0;
        item->v_bearingY  = 0;
        item->prev_global = NULL;
        item->next_global = NULL;
        item->prev_local  = NULL;
        item->next_local  = NULL;
        item->local_cache = local_cache;
        return item;
    }
    static void freeItem( LVFontGlyphCacheItem * item )
    {
        free( item );
    }
};

enum hinting_mode_t {
    HINTING_MODE_DISABLED,
    HINTING_MODE_BYTECODE_INTERPRETOR,
    HINTING_MODE_AUTOHINT
};

/// Base class for fonts implements single interface for font of any engine
class LVFont : public LVRefCounter
{
protected:
    int _visual_alignment_width;
    lString8 _typeface;
    css_font_family_t _family;
public:
    bool is_fallback_ = false;
    lUInt32 _hash;
    /// glyph properties structure
    struct glyph_info_t {
        lUInt8  blackBoxX;   ///< 0: width of glyph
        lUInt8  blackBoxY;   ///< 1: height of glyph black box
        lInt8   originX;     ///< 2: X origin for glyph
        lInt8   originY;     ///< 3: Y origin for glyph
        lUInt8  width;       ///< 4: full width of glyph
    };

    virtual unsigned int getCharIndex( lChar16 code, lChar16 def_char ) =0;// { return 0;};

    virtual LVFontGlyphCacheItem * getGlyphItem(lUInt16 ch, unsigned int ch_glyph_index) = 0 ;// {return NULL;};
    /// hyphenation character
    virtual lChar16 getHyphChar() { return UNICODE_SOFT_HYPHEN_CODE; }

    /// hyphen width
    virtual int getHyphenWidth() { return getCharWidth( getHyphChar() ); }

    /**
     * Max width of -/./,/!/? to use for visial alignment by width
     */
    virtual int getVisualAligmentWidth();

    /** \brief get glyph info
        \param glyph is pointer to glyph_info_t struct to place retrieved info
        \return true if glyh was found 
    */
    virtual bool getGlyphInfo(lUInt16 code, glyph_info_t* glyph, lChar16 def_char=0) = 0;

    virtual bool getGlyphInfoItem(int glyph_index, glyph_info_t * glyph) = 0 ;// { return false;};


    /** \brief measure text
        \param text is text string pointer
        \param len is number of characters to measure
        \param max_width is maximum width to measure line 
        \param def_char is character to replace absent glyphs in font
        \param letter_spacing is number of pixels to add between letters
        \return number of characters before max_width reached 
    */
    virtual lUInt16 measureText(const lChar16* text, int len, lUInt16* widths,
                        lUInt8* flags, int max_width, lChar16 def_char,
                        int letter_spacing=0, bool allow_hyphenation=true) = 0;
    /** \brief measure text
        \param text is text string pointer
        \param len is number of characters to measure
        \return width of specified string 
    */
    virtual lUInt32 getTextWidth(const lChar16 * text, int len) = 0;

//    /** \brief get glyph image in 1 byte per pixel format
//        \param code is unicode character
//        \param buf is buffer [width*height] to place glyph data
//        \return true if glyph was found
//    */


//    virtual bool getGlyphImage(lUInt16 code, lUInt8 * buf, lChar16 def_char=0) = 0;
    /** \brief get glyph item
        \param code is unicode character
        \return glyph pointer if glyph was found, NULL otherwise
    */
    virtual LVFontGlyphCacheItem * getGlyph(lUInt16 ch, lChar16 def_char=0) = 0;
    /// returns font baseline offset
    virtual int getBaseline() = 0;
    /// returns font height including normal interline space
    virtual int getHeight() const = 0;
    /// returns font character size
    virtual int getSize() const = 0;
    /// returns font weight
    virtual int getWeight() const = 0;
    /// returns italic flag
    virtual int getItalic() const = 0;
    /// returns char width
    virtual int getCharWidth( lChar16 ch, lChar16 def_char=0 ) = 0;
    /// retrieves font handle
    virtual void * GetHandle() = 0;
    /// returns font typeface name
    virtual lString8 getTypeFace() const { return _typeface; };
    /// returns font family id
    virtual css_font_family_t getFontFamily() const { return _family; };
    /// draws text string
    virtual void DrawTextString(LVDrawBuf *buf, int x, int y, const lChar16 *text, int len, lChar16 def_char,
            lUInt32 *palette, bool addHyphen, lUInt32 flags, int letter_spacing, bool reverse, int maxWidth) = 0;
    /// constructor
    LVFont() : _visual_alignment_width(-1), _hash(0) { }

    /// get bitmap mode (true=monochrome bitmap, false=antialiased)
    virtual bool getBitmapMode() { return false; }
    /// set bitmap mode (true=monochrome bitmap, false=antialiased)
    virtual void setBitmapMode( bool ) { }

    /// get kerning mode: true==ON, false=OFF
    virtual bool getKerning() const { return false; }
    /// get kerning mode: true==ON, false=OFF
    virtual void setKerning( bool ) { }

    /// sets current hinting mode
    virtual void setHintingMode(hinting_mode_t /*mode*/) { }
    /// returns current hinting mode
    virtual hinting_mode_t  getHintingMode() const { return HINTING_MODE_AUTOHINT; }

    /// returns true if font is empty
    virtual bool IsNull() const = 0;
    virtual bool operator ! () const = 0;
    virtual void Clear() = 0;
    virtual ~LVFont() { }

    virtual bool kerningEnabled() { return false; }
    virtual int getKerningOffset(lChar16 ch1, lChar16 ch2, lChar16 def_char) { CR_UNUSED3(ch1,ch2,def_char); return 0; }

    /// set fallback font for this font
    void setFallbackFont( LVProtectedFastRef<LVFont> font ) { CR_UNUSED(font); }
    /// get fallback font for this font
    LVFont * getFallbackFont() { return NULL; }
    LVFont * getFallbackFont(int size,int weight, bool italic) { return NULL; }
};

typedef LVProtectedFastRef<LVFont> LVFontRef;

enum font_antialiasing_t
{
    font_aa_none,
    font_aa_big,
    font_aa_all
};

class LVEmbeddedFontDef {
    lString16 _url;
    lString8 _face;
    bool _bold;
    bool _italic;
public:
    LVEmbeddedFontDef(lString16 url, lString8 face, bool bold, bool italic) :
        _url(url), _face(face), _bold(bold), _italic(italic)
    {
    }
    LVEmbeddedFontDef() : _bold(false), _italic(false) {
    }

    const lString16 & getUrl() { return _url; }
    const lString8 & getFace() { return _face; }
    bool getBold() { return _bold; }
    bool getItalic() { return _italic; }
    void setFace(const lString8 &  face) { _face = face; }
    void setBold(bool bold) { _bold = bold; }
    void setItalic(bool italic) { _italic = italic; }
    bool serialize(SerialBuf & buf);
    bool deserialize(SerialBuf & buf);
};

class LVEmbeddedFontList : public LVPtrVector<LVEmbeddedFontDef> {
public:
    LVEmbeddedFontDef * findByUrl(lString16 url);
    void add(LVEmbeddedFontDef * def) { LVPtrVector<LVEmbeddedFontDef>::add(def); }
    bool add(lString16 url, lString8 face, bool bold, bool italic);
    bool add(lString16 url) { return add(url, lString8::empty_str, false, false); }
    bool addAll(LVEmbeddedFontList & list);
    void set(LVEmbeddedFontList & list) { clear(); addAll(list); }
    bool serialize(SerialBuf & buf);
    bool deserialize(SerialBuf & buf);
};

/// font manager interface class
class LVFontManager
{
protected:
    int _antialiasMode;
    bool _allowKerning;
    hinting_mode_t _hintingMode;
public:
    int font_size_=24;
    virtual void FallbackFontFaceNext() { return;};
    virtual void FallbackFontFacePrevious() { return;};
    virtual int GetFallbackFontArraySize() { return 0 ;};

    virtual void CycleCounterIncr() =0;

    virtual bool AllowFallbackCycle() {return false;};

    /// garbage collector frees unused fonts
    virtual void gc() = 0;
    /// returns most similar font
    virtual LVFontRef GetFont(int size, int weight, bool italic, css_font_family_t family, lString8 typeface, int documentId = -1) = 0;
    /// set fallback font face (returns true if specified font is found)
    virtual bool SetFallbackFontFace( lString8 face ) { CR_UNUSED(face); return false; }

    /// set fallback font face (returns true if specified font is found)
    virtual bool AddFallbackFontFaceIntoArray(lString8 face) { CR_UNUSED(face); return false; }

    /// get fallback font face (returns empty string if no fallback font is set)
    virtual lString8 GetFallbackFontFace() { return lString8::empty_str; }

    /// get fallback font face (returns empty string if no fallback font is set)
    virtual lString8 GetFallbackFontFaceFromArray(int index) { return lString8::empty_str; }

    virtual void GetSystemFallbackFontsList(lString8Collection& list) = 0;

    virtual void InitFallbackFonts() =0;
    virtual void InitFallbackFontDefault() =0;
    virtual void FallbackReset() =0;

    /// returns fallback font for specified size
    virtual LVFontRef CreateFallbackFont(int size, int weight, bool italic) { return LVFontRef(); }
    /// registers font by name
    virtual lString8Collection RegisterFont(lString8 name, bool langcheck, bool fallback_mode = false) = 0;
    /// registers font by name and face
    virtual bool RegisterExternalFont(lString16 /*name*/, lString8 /*face*/, bool /*bold*/, bool /*italic*/) { return false; }
    /// registers document font
    virtual bool RegisterDocumentFont(int /*documentId*/, LVContainerRef /*container*/, lString16 /*name*/, lString8 /*face*/, bool /*bold*/, bool /*italic*/) { return false; }
    /// unregisters all document fonts
    virtual void UnregisterDocumentFonts(int /*documentId*/) { }
    /// initializes font manager
    virtual bool Init(lString8 path) = 0;
    /// get count of registered fonts
    virtual int GetFontCount() = 0;
    /// get hash of installed fonts and fallback font
    virtual lUInt32 GetFontListHash(int /*documentId*/) { return 0; }
    /// clear glyph cache
    virtual void clearGlyphCache() { }

    /// get antialiasing mode
    virtual int GetAntialiasMode() { return _antialiasMode; }
    /// set antialiasing mode
    virtual void SetAntialiasMode( int mode ) { _antialiasMode = mode; gc(); clearGlyphCache(); }

    /// get kerning mode: true==ON, false=OFF
    virtual bool getKerning() { return _allowKerning; }
    /// get kerning mode: true==ON, false=OFF
    virtual void setKerning( bool kerningEnabled ) { _allowKerning = kerningEnabled; gc(); clearGlyphCache(); }

    /// constructor
    LVFontManager() : _antialiasMode(font_aa_all), _allowKerning(false), _hintingMode(HINTING_MODE_AUTOHINT) { }
    /// destructor
    virtual ~LVFontManager() { }
    /// returns available typefaces
    virtual void getFaceList( lString16Collection & ) { }

    /// returns first found face from passed list, or return face for font found by family only
    virtual lString8 findFontFace(lString8 commaSeparatedFaceList, css_font_family_t fallbackByFamily);

    /// fills array with list of available gamma levels
    virtual void GetGammaLevels(LVArray<double> dst);
    /// returns current gamma level index
    virtual int  GetGammaIndex();
    /// sets current gamma level index
    virtual void SetGammaIndex( int gammaIndex );
    /// returns current gamma level
    virtual double GetGamma();
    /// sets current gamma level
    virtual void SetGamma( double gamma );

    /// sets current hinting mode
    virtual void SetHintingMode(hinting_mode_t /*mode*/) { }
    /// returns current hinting mode
    virtual hinting_mode_t  GetHintingMode() { return HINTING_MODE_AUTOHINT; }

    virtual bool setalias(lString8 alias,lString8 facename,int id,bool italic,bool bold){
        CR_UNUSED5(alias, facename, id, italic, bold);
        return false;
    }

    virtual void clear() {};
};

class LVBaseFont : public LVFont
{
protected:
    lString8 _typeface;
    css_font_family_t _family;
public:
    /// returns font typeface name
    virtual lString8 getTypeFace() const { return _typeface; }
    /// returns font family id
    virtual css_font_family_t getFontFamily() const { return _family; }
    /// draws text string
    virtual void DrawTextString(LVDrawBuf *buf, int x, int y, const lChar16 *text, int len, lChar16 def_char,
            lUInt32 *palette, bool addHyphen, lUInt32 flags, int letter_spacing, bool reverse, int avgWidth);
};

#define LVFONT_TRANSFORM_EMBOLDEN 1
/// create transform for font
LVFontRef LVCreateFontTransform( LVFontRef baseFont, int transformFlags );

/// current font manager pointer
extern LVFontManager* fontMan;

/// initializes font manager
bool InitFontManager( lString8 path );

/// deletes font manager
void ShutdownFontManager();

LVFontRef LoadFontFromFile(const char * fname);

/// to compare two fonts
bool operator == (const LVFont & r1, const LVFont & r2);

#endif //__LV_FNT_MAN_H_INCLUDED__
