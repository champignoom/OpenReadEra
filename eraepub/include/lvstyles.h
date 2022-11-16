/**
    CSS Styles and node format

    (c) Vadim Lopatin, 2000-2006

    This source code is distributed under the terms of
    GNU General Public License. See LICENSE file for details.
*/

#if !defined(__LV_STYLES_H_INCLUDED__)
#define __LV_STYLES_H_INCLUDED__

#include "cssdef.h"
#include "lvrefcache.h"
#include "lvtextfm.h"
#include "lvfntman.h"

/// Element style record. Contains set of style properties.
typedef struct css_style_rec_tag {
    /// for reference counting
    int                  refCount;
    /// cache calculated hash value here
    lUInt32              hash;
    css_display_t        display;
    css_white_space_t    white_space;
    css_text_align_t     text_align;
    css_text_align_t     text_align_last;
    css_text_decoration_t text_decoration;
    css_vertical_align_t vertical_align;
    css_font_family_t    font_family;
    lString8             font_name;
    css_length_t         font_size;
    css_font_style_t     font_style;
    css_font_weight_t    font_weight;
    css_length_t         text_indent;
    css_length_t         line_height;
    css_length_t         width;
    css_length_t         height;
    /// -left, -right, -top, -bottom
    css_length_t         margin[4];
    /// -left, -right, -top, -bottom
    css_length_t         padding[4];
    css_length_t         color;
    css_length_t         background_color;
    css_length_t         letter_spacing;
    css_page_break_t     page_break_before;
    css_page_break_t     page_break_after;
    css_page_break_t     page_break_inside;
    css_hyphenate_t        hyphenate;
    css_list_style_type_t list_style_type;
    css_list_style_position_t list_style_position;
    css_style_rec_tag()
    : refCount(0)
    , hash(0)
    , display( css_d_inherit )
    , white_space(css_ws_inherit)
    , text_align(css_ta_inherit)
    , text_align_last(css_ta_inherit)
    , text_decoration (css_td_inherit)
    , vertical_align(css_va_inherit)
    , font_family(css_ff_inherit)
    , font_size(css_val_inherited, 0)
    , font_style(css_fs_inherit)
    , font_weight(css_fw_inherit)
    , text_indent(css_val_inherited, 0)
    , line_height(css_val_inherited, 0)
    , width(css_val_unspecified, 0)
    , height(css_val_unspecified, 0)
    , color(css_val_inherited, 0)
    , background_color(css_val_unspecified, 0)
    , letter_spacing(css_val_unspecified, 0)
    , page_break_before(css_pb_auto)
    , page_break_after(css_pb_auto)
    , page_break_inside(css_pb_auto)
    , hyphenate(css_hyph_inherit)
    , list_style_type(css_lst_inherit)
    , list_style_position(css_lsp_inherit)
    {
    }
    void AddRef() { refCount++; }
    int Release() { return --refCount; }
    int getRefCount() { return refCount; }
} css_style_rec_t;

/// style record reference type
typedef LVFastRef<css_style_rec_t> css_style_ref_t;
/// font reference type
typedef LVFontRef font_ref_t;

/// to compare two styles
bool operator == (const css_style_rec_t & r1, const css_style_rec_t & r2);

/// to copy style 2 into style 1
void copyStyle(const css_style_ref_t r1, const css_style_ref_t r2);

/// style hash table size
#define LV_STYLE_HASH_SIZE 0x100

/// style cache: allows to avoid duplicate style object allocation
class lvdomStyleCache : public LVRefCache<css_style_ref_t>
{
public:
    lvdomStyleCache(int size = LV_STYLE_HASH_SIZE) : LVRefCache<css_style_ref_t>(size) {}
};

/*
//print out hash-valuable style params
void printStyleHashFrags(css_style_ref_t style)
{
    css_style_rec_t* rec = style.get();

    CRLog::error("====================================");
    CRLog::error(" display             = %x",(lUInt32) rec->display);
    CRLog::error(" white_space         = %x",(lUInt32) rec->white_space);
    CRLog::error(" text_align          = %x",(lUInt32) rec->text_align);
    CRLog::error(" text_align_last     = %x",(lUInt32) rec->text_align_last);
    CRLog::error(" text_decoration     = %x",(lUInt32) rec->text_decoration);
    CRLog::error(" hyphenate           = %x",(lUInt32) rec->hyphenate);
    CRLog::error(" list_style_type     = %x",(lUInt32) rec->list_style_type);
    CRLog::error(" letter_spacing      = %x",(lUInt32) rec->letter_spacing.pack());
    CRLog::error(" list_style_position = %x",(lUInt32) rec->list_style_position);
    CRLog::error(" page_break_before   = %x",(lUInt32) (rec->page_break_before | (rec->page_break_before << 4) | (rec->page_break_before << 8)));
    CRLog::error(" vertical_align      = %x",(lUInt32) rec->vertical_align);
    CRLog::error(" font_size.type      = %x",(lUInt32) rec->font_size.type);
    CRLog::error(" font_size.value     = %x",(lUInt32) rec->font_size.value);
    CRLog::error(" font_style          = %x",(lUInt32) rec->font_style);
    CRLog::error(" font_weight         = %x",(lUInt32) rec->font_weight);
    CRLog::error(" line_height         = %x",(lUInt32) rec->line_height.pack());
    CRLog::error(" color               = %x",(lUInt32) rec->color.pack());
    CRLog::error(" background_color    = %x",(lUInt32) rec->background_color.pack());
    CRLog::error(" width               = %x",(lUInt32) rec->width.pack());
    CRLog::error(" height              = %x",(lUInt32) rec->height.pack());
    CRLog::error(" text_indent         = %x",(lUInt32) rec->text_indent.pack());
    CRLog::error(" margin              =[%x][%x][%x][%x]",(lUInt32) rec->margin[0].pack(),(lUInt32) rec->margin[1].pack(),(lUInt32) rec->margin[2].pack(),(lUInt32) rec->margin[3].pack());
    CRLog::error(" padding             =[%x][%x][%x][%x]",(lUInt32) rec->padding[0].pack(),(lUInt32) rec->padding[1].pack(),(lUInt32) rec->padding[2].pack(),(lUInt32) rec->padding[3].pack());
    CRLog::error(" font_family         = %x",(lUInt32) rec->font_family);
    CRLog::error(" font_name.getHash() = %x",(lUInt32) rec->font_name.getHash());
    CRLog::error("====================================");
};
*/

/// element rendering methods
enum lvdom_element_render_method
{
    /// invisible: don't render
    erm_invisible = 0,
    /// render as block element (render as containing other elements)
    erm_block,
    /// final element: render the whole it's content as single render block
    erm_final,
    /// inline element
    erm_inline,
    /// TODO: block and inline elements are mixed: autobox inline portions of nodes
    erm_mixed,
    /// render as block element as list item
    erm_list_item,
    /// table element: render as table
    erm_table,
    /// table row group
    erm_table_row_group,
    /// table header group
    erm_table_header_group,
    /// table footer group
    erm_table_footer_group,
    /// table row
    erm_table_row,
    /// table column group
    erm_table_column_group,
    /// table column
    erm_table_column,
    /// table cell
    erm_table_cell,
    /// table caption
    erm_table_caption,
    /// run-in
    erm_runin
};

/// node format record
class lvdomElementFormatRec {
protected:
    int  _x;
    int  _width;
    int  _y;
    int  _height;
public:
    lvdomElementFormatRec() : _x(0), _width(0), _y(0), _height(0) { }
    ~lvdomElementFormatRec() { }
    void clear()
    {
        _x = _width = _y = _height = 0;
    }
    bool operator == ( lvdomElementFormatRec & v )
    {
        return (_height==v._height && _y==v._y && _width==v._width && _x==v._x );
    }
    bool operator != ( lvdomElementFormatRec & v )
    {
        return (_height!=v._height || _y!=v._y || _width!=v._width || _x!=v._x );
    }
    int getX() const { return _x; }
    int getY() const { return _y; }
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
    void getRect( lvRect & rc ) const
    {
        rc.left = _x;
        rc.top = _y;
        rc.right = _x + _width;
        rc.bottom = _y + _height;
    }
    void setX( int x ) { _x = x; }
    void setY( int y ) { _y = y; }
    void setWidth( int w ) { _width = w; }
    void setHeight( int h ) { _height = h; }
};

/// calculate cache record hash
lUInt32 calcHash(css_style_rec_t & rec);
/// calculate font instance record hash
lUInt32 calcHash(font_ref_t & rec);
/// calculate cache record hash
inline lUInt32 calcHash(css_style_ref_t & rec) { return rec.isNull() ? 0 : calcHash( *rec.get() ); }

/// splits string like "Arial", Times New Roman, Courier; into list
/// returns number of characters processed
int splitPropertyValueList(const char* fontNames, lString8Collection& list);

/// joins list into string of comma separated quoted values
lString8 joinPropertyValueList(const lString8Collection& list);

#endif // __LV_STYLES_H_INCLUDED__