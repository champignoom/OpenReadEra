/** CoolReader Engine

   lvstsheet.cpp:  style sheet implementation

   (c) Vadim Lopatin, 2000-2006
   Copyright (C) 2013-2020 READERA LLC

   This source code is distributed under the terms of
   GNU General Public License.

   See LICENSE file for details.
*/

#include "include/lvstsheet.h"
#include "include/lvtinydom.h"
#include "include/fb2def.h"

//#define DEBUG_CSS

#ifndef OREDEBUG
#undef DEBUG_CSS
#endif

enum css_decl_code {
    cssd_unknown,
    cssd_display,
    cssd_white_space,
    cssd_text_align,
    cssd_text_align_last,
    cssd_text_decoration,
    cssd_hyphenate, // hyphenate
    cssd_hyphenate2, // -webkit-hyphens
    cssd_hyphenate3, // adobe-hyphenate
    cssd_hyphenate4, // adobe-text-layout
    cssd_color,
    cssd_background_color,
    cssd_vertical_align,
    cssd_font_family, // id families like serif, sans-serif
    cssd_font_names,   // string font name like Arial, Courier
    cssd_font_size,
    cssd_font_style,
    cssd_font_weight,
    cssd_text_indent,
    cssd_line_height,
    cssd_letter_spacing,
    cssd_width,
    cssd_height,
    cssd_margin_left,
    cssd_margin_right,
    cssd_margin_top,
    cssd_margin_bottom,
    cssd_margin,
    cssd_padding_left,
    cssd_padding_right,
    cssd_padding_top,
    cssd_padding_bottom,
    cssd_padding,
    cssd_page_break_before,
    cssd_page_break_after,
    cssd_page_break_inside,
    cssd_list_style,
    cssd_list_style_type,
    cssd_list_style_position,
    cssd_list_style_image,
    cssd_stop
};

static const char * css_decl_name[] = {
    "",
    "display",
    "white-space",
    "text-align",
    "text-align-last",
    "text-decoration",
    "hyphenate",
    "-webkit-hyphens",
    "adobe-hyphenate",
    "adobe-text-layout",
    "color",
    "background-color",
    "vertical-align",
    "font-family",
    "$dummy-for-font-names$",
    "font-size",
    "font-style",
    "font-weight",
    "text-indent",
    "line-height",
    "letter-spacing",
    "width",
    "height",
    "margin-left",
    "margin-right",
    "margin-top",
    "margin-bottom",
    "margin",
    "padding-left",
    "padding-right",
    "padding-top",
    "padding-bottom",
    "padding",
    "page-break-before",
    "page-break-after",
    "page-break-inside",
    "list-style",
    "list-style-type",
    "list-style-position",
    "list-style-image",
    NULL
};

struct standard_color_t {
    const char* name;
    lUInt32 color;
};

standard_color_t standard_color_table[] = {
    {"black", 0x000000},
    {"green", 0x008000},
    {"silver", 0xC0C0C0},
    {"lime", 0x00FF00},
    {"gray", 0x808080},
    {"olive", 0x808000},
    {"white", 0xFFFFFF},
    {"yellow", 0xFFFF00},
    {"maroon", 0x800000},
    {"navy", 0x000080},
    {"red", 0xFF0000},
    {"blue", 0x0000FF},
    {"purple", 0x800080},
    {"teal", 0x008080},
    {"fuchsia", 0xFF00FF},
    {"aqua", 0x00FFFF},
    {NULL, 0}
};

static const char* css_d_names[] =
{
    "inherit",
    "inline",
    "block",
    "list-item", 
    "run-in", 
    "compact", 
    "marker", 
    "table", 
    "inline-table", 
    "table-row-group", 
    "table-header-group", 
    "table-footer-group", 
    "table-row", 
    "table-column-group", 
    "table-column", 
    "table-cell", 
    "table-caption", 
    "none", 
    NULL
};

static const char* css_ws_names[] =
{
    "inherit",
    "normal",
    "pre",
    "nowrap",
    NULL
};

static const char* css_ta_names[] =
{
    "inherit",
    "left",
    "right",
    "center",
    "justify",
    NULL
};

static const char* css_td_names[] =
{
    "inherit",
    "none",
    "underline",
    "overline",
    "line-through",
    "blink",
    NULL
};

static const char* css_hyph_names[] =
{
    "inherit",
    "none",
    "auto",
    NULL
};

static const char* css_hyph_names2[] =
{
    "inherit",
    "optimizeSpeed",
    "optimizeQuality",
    NULL
};

static const char* css_hyph_names3[] =
{
    "inherit",
    "none",
    "explicit",
    NULL
};

static const char* css_pb_names[] =
{
    "inherit",
    "auto",
    "always",
    "avoid",
    "left",
    "right",
    NULL
};

static const char* css_fs_names[] =
{
    "inherit",
    "normal",
    "italic",
    "oblique",
    NULL
};

static const char* css_fw_names[] =
{
    "inherit",
    "normal",
    "bold",
    "bolder",
    "lighter",
    "100",
    "200",
    "300",
    "400",
    "500",
    "600",
    "700",
    "800",
    "900",
    NULL
};
static const char* css_va_names[] =
{
    "inherit",
    "baseline", 
    "sub",
    "super",
    "top",
    "text-top",
    "middle",
    "bottom",
    "text-bottom",
    NULL
};

static const char* css_ti_attribute_names[] =
{
    "hanging",
    NULL
};

static const char* css_ff_names[] =
{
    "inherit",
    "serif",
    "sans-serif",
    "cursive",
    "fantasy",
    "monospace",
    NULL
};

static const char* css_lst_names[] =
{
    "inherit",
    "disc",
    "circle",
    "square",
    "decimal",
    "lower-roman",
    "upper-roman",
    "lower-alpha",
    "upper-alpha",
    "none",
    NULL
};

static const char* css_lsp_names[] =
{
    "inherit",
    "inside",
    "outside",
    NULL
};

static css_length_t read_length(int*& data)
{
    css_length_t len;
    len.type = (css_value_type_t) (*data++);
    len.value = (*data++);
    return len;
}

static lString16 GetNodeDesc(const ldomNode* node)
{
    //lUInt16 node_id = node->getNodeId();
    //lString16 desc = node->getCrDom()->getElementName(node_id);
    //lString16 desc = node->getNodeName();
    //desc << "/" << lString16::itoa(node_id);
    //desc << "[" << lString16::itoa(node->getNodeIndex()) << "]";
    ldomXPointer xptr = ldomXPointer((ldomNode*) node, 0);
    lString16 desc = xptr.toString();
    return desc;
}

void LVCssDeclaration::apply(const ldomNode* node, css_style_rec_t* style)
{
    if (!_data) {
#ifdef OREDEBUG
        //CRLog::info("CSS: no declaration for %s", LCSTR(GetNodeDesc(node)));
#endif
        return;
    }
#ifdef DEBUG_CSS
    //CRLog::trace("CSS: apply declaration [%s]", LCSTR(GetNodeDesc(node)));
#endif
    int* p = _data;
    for (;;) {
        switch (*p++) {
        case cssd_display:
            style->display = (css_display_t) *p++;
            break;
        case cssd_white_space:
            style->white_space = (css_white_space_t) *p++;
            break;
        case cssd_text_align:
            style->text_align = (css_text_align_t) *p++;
            break;
        case cssd_text_align_last:
            style->text_align_last = (css_text_align_t) *p++;
            break;
        case cssd_text_decoration:
            style->text_decoration = (css_text_decoration_t) *p++;
            break;
        case cssd_hyphenate:
            style->hyphenate = (css_hyphenate_t) *p++;
            break;
        case cssd_list_style_type:
            if(style->list_style_type == css_lst_inherit)
            {
                style->list_style_type = (css_list_style_type_t) *p;
            }
            *p++;
            break;
        case cssd_list_style_position:
            style->list_style_position = (css_list_style_position_t) *p++;
            break;
        case cssd_page_break_before:
            style->page_break_before = (css_page_break_t) *p++;
            break;
        case cssd_page_break_after:
            style->page_break_after = (css_page_break_t) *p++;
            break;
        case cssd_page_break_inside:
            style->page_break_inside = (css_page_break_t) *p++;
            break;
        case cssd_vertical_align:
            style->vertical_align = (css_vertical_align_t) *p++;
            break;
        case cssd_font_family:
            style->font_family = (css_font_family_t) *p++;
            break;
        case cssd_font_names: {
            lString8 names;
            names.reserve(64);
            int len = *p++;
            for (int i = 0; i < len; i++) {
                names << (lChar8) (*p++);
            }
            names.pack();
            style->font_name = names;
        }
            break;
        case cssd_font_style:
            style->font_style = (css_font_style_t) *p++;
            break;
        case cssd_font_weight:
            style->font_weight = (css_font_weight_t) *p++;
            break;
        case cssd_font_size:
            style->font_size = read_length(p);
            break;
        case cssd_text_indent:
            style->text_indent = read_length(p);
            break;
        case cssd_line_height:
            style->line_height = read_length(p);
            break;
        case cssd_letter_spacing:
            style->letter_spacing = read_length(p);
            break;
        case cssd_color:
            style->color = read_length(p);
            break;
        case cssd_background_color:
            style->background_color = read_length(p);
            break;
        case cssd_width:
            style->width = read_length(p);
            break;
        case cssd_height:
            style->height = read_length(p);
            break;
        case cssd_margin_left:
            style->margin[0] = read_length(p);
            break;
        case cssd_margin_right:
            style->margin[1] = read_length(p);
            break;
        case cssd_margin_top:
            style->margin[2] = read_length(p);
            break;
        case cssd_margin_bottom:
            style->margin[3] = read_length(p);
            break;
        case cssd_margin:
            style->margin[2] = read_length(p);
            style->margin[1] = read_length(p);
            style->margin[3] = read_length(p);
            style->margin[0] = read_length(p);
            break;
        case cssd_padding_left:
            style->padding[0] = read_length(p);
            break;
        case cssd_padding_right:
            style->padding[1] = read_length(p);
            break;
        case cssd_padding_top:
            style->padding[2] = read_length(p);
            break;
        case cssd_padding_bottom:
            style->padding[3] = read_length(p);
            break;
        case cssd_padding:
            style->padding[2] = read_length(p);
            style->padding[1] = read_length(p);
            style->padding[3] = read_length(p);
            style->padding[0] = read_length(p);
            break;
        case cssd_stop:
            return;
        }
    }
}

bool LVCssSelectorRule::check(const ldomNode*& node)
{
    switch (_type) {
    case cssrt_parent: {
        // E > F
        node = node->getParentNode();
        //CRLog::trace("        cssrt_parent: %d %d", node->getNodeId(), _id);
        if (node->isNull()) {
            CRLog::trace("        cssrt_parent_class node->isNull()");
            return false;
        }
        return node->getNodeId() == _id;
    }
        break;
    case cssrt_parent_class: {
        node = node->getParentNode();
        if (node->isNull()) {
            CRLog::trace("        cssrt_parent_class node->isNull()");
            return false;
        }
        lString16 val = node->getAttributeValue(attr_class);
        val.lowercase();
        //CRLog::trace("        cssrt_parent_class: [%s]==[%s]", LCSTR(val), LCSTR(_value));
        return val == _value;
    }
        break;
    case cssrt_ancessor: {
        // E F
        for (;;) {
            node = node->getParentNode();
            if (node->isNull()) {
                return false;
            }
            if (node->getNodeId() == _id) {
                return true;
            }
        }
    }
        break;
    case cssrt_predecessor: {
        // E + F
        int index = node->getNodeIndex();
        if (index <= 0) {
            return false;
        }
        ldomNode* elem = node->getParentNode()->getChildElementNode((lUInt32) (index - 1), _id);
        if (elem) {
            node = elem;
            //CRLog::trace("+ selector: found pred element");
            return true;
        }
        return false;
    }
        break;
    case cssrt_attrset: {
        // E[foo]
        if (!node->hasAttributes()) {
            return false;
        }
        return node->hasAttribute(_attrid);
    }
        break;
    case cssrt_attreq: {
        // E[foo="value"]
        lString16 val = node->getAttributeValue(_attrid);
        bool res = (val == _value);
        //if ( res )
        //    return true;
        //CRLog::trace("attreq: %s %s", LCSTR(val), LCSTR(_value) );
        return res;
    }
        break;
    case cssrt_attrhas: {
        // E[foo~="value"], one of space separated values
        lString16 val = node->getAttributeValue(_attrid);
        int p = val.pos(lString16(_value.c_str()));
        if (p < 0) {
            return false;
        }
        if (p > 0 && val[p - 1] != ' ') {
            return false;
        }
        if (p + _value.length() < val.length() && val[p + _value.length()] != ' ') {
            return false;
        }
        return true;
    }
        break;
    case cssrt_attrstarts: {
        // TODO E[foo|="value"]
        lString16 val = node->getAttributeValue(_attrid);
        if (_value.length() > val.length()) {
            return false;
        }
        val = val.substr(0, _value.length());
        return val == _value;
    }
        break;
    case cssrt_id: {
        // TODO E#id
        lString16 val = node->getAttributeValue(attr_id);
        if (_value.length() > val.length()) {
            return false;
        }
        return val == _value;
    }
        break;
    case cssrt_class: {
        // TODO E.class
        lString16 val = node->getAttributeValue(attr_class);
        val.lowercase();
        int pos = val.pos(" ");
        if(pos != -1)
        {
            int lastpos = 0;
            lString16 frag;
            while (pos != -1)
            {
                frag = val.substr(lastpos, pos - lastpos);
                if(frag == _value)
                {
                    return true;
                }
                lastpos = pos + 1;
                pos = val.pos(" ", lastpos );
            }
            frag = val.substr(lastpos, val.length() - lastpos); //last item
            return frag == _value;
        }
        return val == _value;
    }
        break;
    case cssrt_universal:
        // *
        return true;
    default:
        CRLog::error(" LVCssSelectorRule::check() rule type is undefined.");
        return false;
    }
    return true;
}

bool LVCssSelector::check(const ldomNode* node) const
{
    // Check main Id
    if (_id != 0 && node->getNodeId() != _id) {
#ifdef DEBUG_CSS
        CRLog::trace("    selector miss [%s] (reason: id)", name_.c_str());
#endif
        return false;
    }
    if (!_rules) {
#ifdef DEBUG_CSS
        CRLog::debug("    selector match [%s] (reason: no rules)", name_.c_str());
#endif
        return true;
    }
    if (node->isNull()) {
#ifdef OREDEBUG
        CRLog::info("    selector miss [%s] (reason: null node)", name_.c_str());
#endif
        return false;
    }
    if (node->isRoot()) {
#ifdef OREDEBUG
        //CRLog::trace("    selector miss [%s] (reason: root node)", name_.c_str());
#endif
        return false;
    }
    // Check additional rules
    const ldomNode* n = node;
    LVCssSelectorRule* rule = _rules;
    do {
        if (!rule->check(n)) {
#ifdef DEBUG_CSS
            CRLog::trace("    selector miss [%s] rule check", name_.c_str());
#endif
            return false;
        }
        rule = rule->getNext();
    } while (rule != NULL);
#ifdef DEBUG_CSS
    CRLog::debug("    selector match [%s]", name_.c_str());
#endif
    return true;
}

/* //NO NEED TO USE IT BECAUSE IT ONLY SLOWS DOWN

void LVStyleSheet::applyCssMulticlass(std::vector<lString16> coll, const ldomNode* node, css_style_rec_t* style)
{
    //LE("applyCssMulticlass for node %s", LCSTR(node->getNodeName()));

    lUInt16 id = node->getNodeId();
    LVCssSelector* selector_class = _selectors[0];
    LVCssSelector* selector_id = id > 0 && id < _selectors.length() ? _selectors[id] : NULL;

    for (int i = 0; i < coll.size(); ++i)
    {
        lString16 classname = coll.at(i);
        //LE("applyCssMulticlass classname = %s",LCSTR(classname));

        classname = L"." + classname;
        lString8 classname8 = UnicodeToUtf8(classname);
        //LE("applyCssMulticlass looking for %s",classname8.c_str());
        int hash = classname8.getHash();
        if(selectorMap_.find(hash) != selectorMap_.end())
        {
            selector_class = selectorMap_[hash];
            //selector_class->name_ = classname8;
            //LE("applyCssMulticlass [%s] found: %x",classname8.c_str(),selector_class);
        }
        else
        {
            //LE("applyCssMulticlass [%s] couldnt find class in map",LCSTR(classname));
            continue;
        }

        for (;;) {
            LVCssSelector *sel = NULL;
            if (selector_class != NULL){
                if (selector_id == NULL || selector_class->getSpecificity() < selector_id->getSpecificity())
                {
                    sel = selector_class;
                    selector_class = selector_class->getNext();
                } else {
                    sel = selector_id;
                    selector_id = selector_id->getNext();
                }
            } else if (selector_id != NULL) {
                sel = selector_id;
                selector_id = selector_id->getNext();
            }
            if (!sel) {
                break;
            }
            if (sel->check(node)) {
                sel->applyCss(node, style);
            }
        }
    }
}
*/

void LVStyleSheet::applyCss(const ldomNode* node, css_style_rec_t* style)
{
    if (!_selectors.length()) {
#ifdef OREDEBUG
        CRLog::info("LVStyleSheet::applyCss[%s]: selectors null", LCSTR(GetNodeDesc(node)));
#endif
        return;
    }
    bool applied = false;
    lUInt16 id = node->getNodeId();
    lString16 classname;
    LVCssSelector* selector_class = _selectors[0];
    LVCssSelector* selector_id = id > 0 && id < _selectors.length() ? _selectors[id] : NULL;

    if( node->hasAttribute(attr_class))
    {
        classname = node->getAttributeValue(attr_class);
        classname.lowercase();
        //lString16 classes = node->getAttributeValue(attr_class);
        //if(!classes.empty() && classes.pos(" ") != -1)
        //{
        //    std::vector<lString16> coll = classes.splitWithStd(' ');
        //    if (coll.size() > 1)
        //    {
        //        applyCssMulticlass(coll, node, style);
        //        return;
        //    }
        //}

        classname = L"." + classname;
        lString8 classname8 = UnicodeToUtf8(classname);
        //CRLog::error("looking for %s",classname8.c_str());
        if(selectorMap_.find(classname8.getHash()) != selectorMap_.end())
        {
            selector_class = selectorMap_[classname8.getHash()];
            //selector_class->setNext(NULL); //leads to map memory corruption due to cutting vector when found element
            //CRLog::error("%s found: %x",classname8.c_str(),selector_class);
        }
        else
        {
            //CRLog::error("couldnt find class %s in map",LCSTR(classname));
            //fallback for map errors
            selector_class = _selectors[0];
        }
    }
    else
    {
        selector_class = NULL;
    }

#ifdef DEBUG_CSS
    if (id == 0) {
        CRLog::info("LVStyleSheet::applyCss[%s]: node id==0", LCSTR(GetNodeDesc(node)));
    } else if (!selector_id) {
        CRLog::trace("LVStyleSheet::applyCss[%s]: !selector_id", LCSTR(GetNodeDesc(node)));
    } else {
        CRLog::trace("LVStyleSheet::applyCss[%s]", LCSTR(GetNodeDesc(node)));
    }
#endif
    for (;;) {
        LVCssSelector *sel = NULL;
        if (selector_class != NULL){
            if (selector_id == NULL || selector_class->getSpecificity() < selector_id->getSpecificity())
            {
                sel = selector_class;
                selector_class = selector_class->getNext();
            } else {
                sel = selector_id;
                selector_id = selector_id->getNext();
            }
        } else if (selector_id != NULL) {
            sel = selector_id;
            selector_id = selector_id->getNext();
        }
        if (!sel) {
            break;
        }
        if (sel->check(node)) {
            sel->applyCss(node, style);
            applied = true;
        }
    }
#ifdef DEBUG_CSS
    if (!applied) {
        //CRLog::debug("    selectors miss");
    }
#endif
}

inline bool css_is_alpha(char ch)
{
    return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch == '-') || (ch == '_'));
}

inline bool css_is_alnum(char ch)
{
    return (css_is_alpha(ch) || (ch >= '0' && ch <= '9'));
}

static int substr_compare(const char* sub, const char*& str)
{
    int j;
    for (j = 0; sub[j] == str[j] && sub[j] && str[j]; j++) {}
    if (!sub[j]) {
        //bool last_alpha = css_is_alpha( sub[j-1] );
        //bool next_alnum = css_is_alnum( str[j] );
        if (!css_is_alpha(sub[j - 1]) || !css_is_alnum(str[j])) {
            str += j;
            return j;
        }
    }
    return 0;
}

inline char toLower(char c)
{
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    }
    return c;
}

static int substr_icompare(const char* sub, const char*& str)
{
    int j;
    for (j = 0; toLower(sub[j]) == toLower(str[j]) && sub[j] && str[j]; j++) {}
    if (!sub[j]) {
        //bool last_alpha = css_is_alpha( sub[j-1] );
        //bool next_alnum = css_is_alnum( str[j] );
        if (!css_is_alpha(sub[j - 1]) || !css_is_alnum(str[j])) {
            str += j;
            return j;
        }
    }
    return 0;
}

/// Returns true if string not empty after skip
static bool skip_spaces_and_comments(const char*& str)
{
    const char* oldpos = str;
    for (;;) {
        while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
            str++;
        }
        if (*str == '/' && str[1] == '*') {
            // comment found
            while (*str && str[1] && (str[0] != '*' || str[1] != '/')) {
                str++;
            }
            if (*str == '*' && str[1] == '/') {
                str += 2;
            }
        }
        while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
            str++;
        }
        if (oldpos == str) {
            break;
        }
        if (*str == 0) {
            return false;
        }
        oldpos = str;
    }
    return *str != 0;
}

static css_decl_code parse_property_name(const char*& res)
{
    const char* str = res;
    for (int i = 1; css_decl_name[i]; i++) {
        if (substr_compare(css_decl_name[i], str)) {
            // found!
            skip_spaces_and_comments(str);
            if (substr_compare(":", str)) {
#ifdef DEBUG_CSS
                //CRLog::trace("property name: %s", lString8(res, str - res).c_str());
#endif
                skip_spaces_and_comments(str);
                res = str;
                return (css_decl_code) i;
            }
        }
    }
#ifdef OREDEBUG
    CRLog::debug("parse_property_name cssd_unknown");
#endif
    return cssd_unknown;
}

static int parse_name(const char*& str, const char** names, int def_value)
{
    for (int i = 0; names[i]; i++) {
        if (substr_compare(names[i], str)) {
            // found!
            return i;
        }
    }
    return def_value;
}

static bool next_property(const char*& str)
{
    while (*str && *str != ';' && *str != '}') {
        str++;
    }
    if (*str == ';') {
        str++;
    }
    return skip_spaces_and_comments(str);
}

static bool parse_number_value(const char*& str, css_length_t& value)
{
    value.type = css_val_unspecified;
    skip_spaces_and_comments(str);
    if (substr_compare("inherited", str)) {
        value.type = css_val_inherited;
        value.value = 0;
        return true;
    }
    int n = 0;
    if (*str != '.') {
        if (*str < '0' || *str > '9') {
            return false; // not a number
        }
        while (*str >= '0' && *str <= '9') {
            n = n * 10 + (*str - '0');
            str++;
        }
    }
    int frac = 0;
    int frac_div = 1;
    if (*str == '.') {
        str++;
        while (*str >= '0' && *str <= '9') {
            frac = frac * 10 + (*str - '0');
            frac_div *= 10;
            str++;
        }
    }
    if (substr_compare("em", str)) {
        value.type = css_val_em;
    } else if (substr_compare("pt", str)) {
        value.type = css_val_pt;
    } else if (substr_compare("ex", str)) {
        value.type = css_val_ex;
    } else if (substr_compare("px", str)) {
        value.type = css_val_px;
    } else if (substr_compare("in", str)) {
        value.type = css_val_in;
    } else if (substr_compare("cm", str)) {
        value.type = css_val_cm;
    } else if (substr_compare("mm", str)) {
        value.type = css_val_mm;
    } else if (substr_compare("pc", str)) {
        value.type = css_val_pc;
    } else if (substr_compare("%", str)) {
        value.type = css_val_percent;
    } else if (n == 0 && frac == 0) {
        value.type = css_val_px;
    } else {
        return false;
    }
    if (value.type == css_val_px || value.type == css_val_percent|| value.type == css_val_pt ) {
        value.value = n;                               // normal
    } else {
        value.value = n * 256 + 256 * frac / frac_div;
    } // *256
    return true;
}

static int hexDigit(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

/// parse color value like #334455, #345 or red
static bool parse_color_value(const char*& str, css_length_t& value)
{
    value.type = css_val_unspecified;
    skip_spaces_and_comments(str);
    if (substr_compare("inherited", str)) {
        value.type = css_val_inherited;
        value.value = 0;
        return true;
    }
    if (substr_compare("none", str)) {
        value.type = css_val_unspecified;
        value.value = 0;
        return true;
    }
    if (*str == '#') {
        // #rgb or #rrggbb colors
        str++;
        int nDigits = 0;
        for (; hexDigit(str[nDigits]) >= 0; nDigits++) {}
        if (nDigits == 3) {
            int r = hexDigit(*str++);
            int g = hexDigit(*str++);
            int b = hexDigit(*str++);
            value.type = css_val_color;
            value.value = (((r + r * 16) * 256) | (g + g * 16)) * 256 | (b + b * 16);
            return true;
        } else if (nDigits == 6) {
            int r = hexDigit(*str++) * 16;
            r += hexDigit(*str++);
            int g = hexDigit(*str++) * 16;
            g += hexDigit(*str++);
            int b = hexDigit(*str++) * 16;
            b += hexDigit(*str++);
            value.type = css_val_color;
            value.value = ((r * 256) | g) * 256 | b;
            return true;
        } else {
            return false;
        }
    }
    for (int i = 0; standard_color_table[i].name != NULL; i++) {
        if (substr_icompare(standard_color_table[i].name, str)) {
            value.type = css_val_color;
            value.value = standard_color_table[i].color;
            return true;
        }
    }
    return false;
}

bool LVCssDeclaration::parse(const char*& decl)
{
    if (!decl) {
        return false;
    }
    skip_spaces_and_comments(decl);
    if (*decl != '{') {
        return false;
    }
    // 512 - MAX_DECL_SIZE
    int buf[512];
    int buf_pos = 0;
    decl++;
    while (*decl && *decl != '}') {
        skip_spaces_and_comments(decl);
        css_decl_code prop_code = parse_property_name(decl);
        skip_spaces_and_comments(decl);
        if (prop_code == cssd_unknown) {
            next_property(decl);
            continue;
        }
        lString8 strValue;
        // parsed ok
        int n = -1;
        switch (prop_code) {
        case cssd_display:
            n = parse_name(decl, css_d_names, -1);
            break;
        case cssd_white_space:
            n = parse_name(decl, css_ws_names, -1);
            break;
        case cssd_text_align:
            n = parse_name(decl, css_ta_names, -1);
            break;
        case cssd_text_align_last:
            n = parse_name(decl, css_ta_names, -1);
            break;
        case cssd_text_decoration:
            n = parse_name(decl, css_td_names, -1);
            break;
        case cssd_hyphenate:
        case cssd_hyphenate2:
        case cssd_hyphenate3:
        case cssd_hyphenate4:
            prop_code = cssd_hyphenate;
            n = parse_name(decl, css_hyph_names, -1);
            if (n == -1) {
                n = parse_name(decl, css_hyph_names2, -1);
            }
            if (n == -1) {
                n = parse_name(decl, css_hyph_names3, -1);
            }
            break;
        case cssd_page_break_before:
            n = parse_name(decl, css_pb_names, -1);
            break;
        case cssd_page_break_inside:
            n = parse_name(decl, css_pb_names, -1);
            break;
        case cssd_page_break_after:
            n = parse_name(decl, css_pb_names, -1);
            break;
        case cssd_list_style_type:
            n = parse_name(decl, css_lst_names, -1);
            break;
        case cssd_list_style_position:
            n = parse_name(decl, css_lsp_names, -1);
            break;
        case cssd_vertical_align:
            n = parse_name(decl, css_va_names, -1);
            break;
        case cssd_font_family: {
            lString8Collection list;
            int processed = splitPropertyValueList(decl, list);
            decl += processed;
            n = -1;
            if (list.length()) {
                for (int i = list.length() - 1; i >= 0; i--) {
                    const char* name = list[i].c_str();
                    int nn = parse_name(name, css_ff_names, -1);
                    if (n == -1 && nn != -1) {
                        n = nn;
                    }
                    if (nn != -1) {
                        // remove family name from font list
                        list.erase(i, 1);
                    }
                }
                strValue = joinPropertyValueList(list);
            }
        }
            break;
        case cssd_font_style:
            n = parse_name(decl, css_fs_names, -1);
            break;
        case cssd_font_weight:
            n = parse_name(decl, css_fw_names, -1);
            break;
        case cssd_text_indent: {
            // read length
            css_length_t len;
            bool negative = false;
            if (*decl == '-') {
                decl++;
                negative = true;
            }
            if (parse_number_value(decl, len)) {
                // read optional "hanging" flag
                skip_spaces_and_comments(decl);
                int attr = parse_name(decl, css_ti_attribute_names, -1);
                if (attr == 0 || negative) {
                    len.value = -len.value;
                }
                // save result
                buf[buf_pos++] = prop_code;
                buf[buf_pos++] = len.type;
                buf[buf_pos++] = len.value;
            }
        }
            break;
        case cssd_line_height:
        case cssd_letter_spacing:
        case cssd_font_size:
        case cssd_width:
        case cssd_height:
        case cssd_margin_left:
        case cssd_margin_right:
        case cssd_margin_top:
        case cssd_margin_bottom:
        case cssd_padding_left:
        case cssd_padding_right:
        case cssd_padding_top:
        case cssd_padding_bottom: {
            css_length_t len;
            if (parse_number_value(decl, len)) {
                buf[buf_pos++] = prop_code;
                buf[buf_pos++] = len.type;
                buf[buf_pos++] = len.value;
            }
        }
            break;
        case cssd_margin:
        case cssd_padding: {
            css_length_t len[4];
            int i;
            for (i = 0; i < 4; ++i) {
                if (!parse_number_value(decl, len[i])) {
                    break;
                }
            }
            if (i) {
                switch (i) {
                case 1:
                    len[1] = len[0]; /* fall through */
                case 2:
                    len[2] = len[0]; /* fall through */
                case 3:
                    len[3] = len[1];
                }
                buf[buf_pos++] = prop_code;
                for (i = 0; i < 4; ++i) {
                    buf[buf_pos++] = len[i].type;
                    buf[buf_pos++] = len[i].value;
                }
            }
        }
            break;
        case cssd_color:
        case cssd_background_color: {
            css_length_t len;
            if (parse_color_value(decl, len)) {
                buf[buf_pos++] = prop_code;
                buf[buf_pos++] = len.type;
                buf[buf_pos++] = len.value;
            }
        }
            break;
        case cssd_stop:
        case cssd_unknown:
        default:
            break;
        }
        if (n != -1) {
            // add enum property
            buf[buf_pos++] = prop_code;
            buf[buf_pos++] = n;
        }
        if (!strValue.empty()) {
            // add string property
            if (prop_code == cssd_font_family) {
                // font names
                buf[buf_pos++] = cssd_font_names;
                buf[buf_pos++] = strValue.length();
                for (int i = 0; i < strValue.length(); i++) {
                    buf[buf_pos++] = strValue[i];
                }
            }
        }
        next_property(decl);
    }
    // store parsed result
    if (buf_pos) {
        buf[buf_pos++] = cssd_stop; // add end marker
        _data = new int[buf_pos];
        for (int i = 0; i < buf_pos; i++) {
            _data[i] = buf[i];
        }
    }
    // skip }
    skip_spaces_and_comments(decl);
    if (*decl == '}') {
        decl++;
        return true;
    }
    return false;
}

bool parse_attr_value(const char*& str, char* buf)
{
    int pos = 0;
    skip_spaces_and_comments(str);
    if (*str == '\"') {
        str++;
        for (; str[pos] && str[pos] != '\"'; pos++) {
            if (pos >= 64) {
                return false;
            }
        }
        if (str[pos] != '\"') {
            return false;
        }
        for (int i = 0; i < pos; i++) {
            buf[i] = str[i];
        }
        buf[pos] = 0;
        str += pos + 1;
        skip_spaces_and_comments(str);
        if (*str != ']') {
            return false;
        }
        str++;
        return true;
    } else {
        for (; str[pos] && str[pos] != ' ' && str[pos] != '\t' && str[pos] != ']'; pos++) {
            if (pos >= 64) {
                return false;
            }
        }
        if (str[pos] != ']') {
            return false;
        }
        for (int i = 0; i < pos; i++) {
            buf[i] = str[i];
        }
        buf[pos] = 0;
        str += pos;
        str++;
        return true;
    }
}

static bool parse_ident(const char*& str, char* ident)
{
    *ident = 0;
    skip_spaces_and_comments(str);
    if (!css_is_alpha(*str)) {
        return false;
    }
    int i;
    for (i = 0; css_is_alnum(str[i]); i++) {
        ident[i] = str[i];
    }
    ident[i] = 0;
    str += i;
    return true;
}

LVCssSelectorRule* parse_attr(const char*& str, CrDomXml* doc)
{
    char attrname[512];
    char attrvalue[512];
    LVCssSelectorRuleType st = cssrt_universal;
    if (*str == '.') {
        // E.class
        str++;
        skip_spaces_and_comments(str);
        if (!parse_ident(str, attrvalue)) {
            return NULL;
        }
        skip_spaces_and_comments(str);
        LVCssSelectorRule* rule = new LVCssSelectorRule(cssrt_class);
        lString16 s(attrvalue);
        s.lowercase();
        rule->setAttr(attr_class, s);
        return rule;
    } else if (*str == '#') {
        // E#id
        str++;
        skip_spaces_and_comments(str);
        if (!parse_ident(str, attrvalue)) {
            return NULL;
        }
        skip_spaces_and_comments(str);
        LVCssSelectorRule* rule = new LVCssSelectorRule(cssrt_id);
        lString16 s(attrvalue);
        rule->setAttr(attr_id, s);
        return rule;
    } else if (*str != '[') {
        return NULL;
    }
    str++;
    skip_spaces_and_comments(str);
    if (!parse_ident(str, attrname)) {
        return NULL;
    }
    skip_spaces_and_comments(str);
    attrvalue[0] = 0;
    if (*str == ']') {
        st = cssrt_attrset;
        str++;
    } else if (*str == '=') {
        str++;
        if (!parse_attr_value(str, attrvalue)) {
            return NULL;
        }
        st = cssrt_attreq;
    } else if (*str == '~' && str[1] == '=') {
        str += 2;
        if (!parse_attr_value(str, attrvalue)) {
            return NULL;
        }
        st = cssrt_attrhas;
    } else if (*str == '|' && str[1] == '=') {
        str += 2;
        if (!parse_attr_value(str, attrvalue)) {
            return NULL;
        }
        st = cssrt_attrstarts;
    } else {
        return NULL;
    }
    LVCssSelectorRule* rule = new LVCssSelectorRule(st);
    lString16 s(attrvalue);
    lUInt16 id = doc->getAttrNameIndex(lString16(attrname).c_str());
    rule->setAttr(id, s);
    return rule;
}

void LVCssSelector::insertRuleStart(LVCssSelectorRule* rule)
{
    rule->setNext(_rules);
    _rules = rule;
}

void LVCssSelector::insertRuleAfterStart(LVCssSelectorRule* rule)
{
    if (!_rules) {
        _rules = rule;
        return;
    }
    rule->setNext(_rules->getNext());
    _rules->setNext(rule);
}

bool LVCssSelector::parse(const char*& str, CrDomXml* dom)
{
    if (!str || !*str || strlen(str) == 0) {
#ifdef OREDEBUG
        CRLog::error("LVCssSelector::parse: empty string");
#endif
        return false;
    }
    if(!skip_spaces_and_comments(str))
    {
        CRLog::error("LVCssSelector::parse: empty string 2");
        return false;
    }
    char selector[512];
    int i;
    for (i = 0; str[i] != ',' && str[i] != '{' && i < strlen(str); i++) {
        selector[i] = str[i];
    }
    if (selector[i - 1] == ' ') {
        selector[i - 1] = '\0';
    } else {
        selector[i] = '\0';
    }
    name_ = lString8(selector);
    //CRLog::error("selector = %s",name_.c_str());
    for (;;) {
        skip_spaces_and_comments(str);
        if (*str == '*') {
            // universal selector
            str++;
            _id = 0;
            skip_spaces_and_comments(str);
        } else if (*str == '.') {
            // classname follows
            _id = 0;
        } else if (*str == '@') {
            // @-rule
            return true;
        } else if (css_is_alpha(*str)) {
            // identificator
            char ident[64];
            if (!parse_ident(str, ident)) {
#ifdef OREDEBUG
                CRLog::info("LVCssSelector::parse fail: !parse_ident");
#endif
                return false;
            }
            _id = dom->getElementNameIndex(lString16(ident).c_str());
            skip_spaces_and_comments(str);
        } else {
#ifdef OREDEBUG
            CRLog::info("LVCssSelector::parse fail: unknown selector type");
#endif
            return false;
        }
        if (*str == ',' || *str == '{') {
            return true;
        }
        // One or more attribute rules
        bool attr_rule = false;
        while (*str == '[' || *str == '.' || *str == '#') {
            LVCssSelectorRule* rule = parse_attr(str, dom);
            if (!rule) {
#ifdef OREDEBUG
                CRLog::info("LVCssSelector::parse fail: parse_attr");
#endif
                return false;
            }
            insertRuleStart(rule);
            /*
            insertRuleAfterStart(rule);
            if (_id != 0) {
                LVCssSelectorRule* rule = new LVCssSelectorRule(cssrt_parent);
                rule->setId(_id);
                insertRuleStart(rule);
                _id=0;
            }
            */
            skip_spaces_and_comments(str);
            attr_rule = true;
            //continue;
        }
        // element relation
        if (*str == '>') {
            str++;
            LVCssSelectorRule* rule;
            if (_rules && _rules->getType() == cssrt_class) {
                rule = new LVCssSelectorRule(cssrt_parent_class);
                rule->setAttr(_rules->getAttrId(), _rules->getValue());
            } else {
                rule = new LVCssSelectorRule(cssrt_parent);
            }
            rule->setId(_id);
            insertRuleStart(rule);
            _id = 0;
            continue;
        } else if (*str == '+') {
            str++;
            LVCssSelectorRule* rule = new LVCssSelectorRule(cssrt_predecessor);
            rule->setId(_id);
            insertRuleStart(rule);
            _id = 0;
            continue;
        } else if (css_is_alpha(*str)) {
            LVCssSelectorRule* rule = new LVCssSelectorRule(cssrt_ancessor);
            rule->setId(_id);
            insertRuleStart(rule);
            _id = 0;
            continue;
        }
        if (!attr_rule) {
#ifdef OREDEBUG
            CRLog::info("LVCssSelector::parse fail: !attr_rule");
#endif
            return false;
        } else if (*str == ',' || *str == '{') {
            return true;
        }
    }
}

bool LVStyleSheet::parse(const char* str)
{
    LVCssSelector* selector = NULL;
    LVCssSelector* prev_selector;
    int specifity = 1;
    int err_count = 0;
    int rule_count = 0;
    for (; *str;) {
        prev_selector = NULL;
        bool err = false;
        for (; *str;) {
            // In single iteration parses single selector or single rule
            // declaration block. Breaks after parsed list of selectors
            // related to single declaration block and that declaration block.
            selector = new LVCssSelector(specifity++);
            selector->setNext(prev_selector);
            if (!selector->parse(str, _doc)) {
                err = true;
                break;
            } else {
                if (*str == ',') {
                    str++;
                    prev_selector = selector;
                    // Parse next selector
                    continue;
                }
            }
            // parse declaration
            LVCssDeclRef decl(new LVCssDeclaration);
            if (!decl->parse(str)) {
#ifdef OREDEBUG
                CRLog::info("LVCssDeclaration::parse fail");
#endif
                err = true;
                err_count++;
            } else {
                // set decl to selectors
                for (LVCssSelector* p = selector; p; p = p->getNext()) {
                    p->setDeclaration(decl);
                }
                rule_count++;
            }
            // Fix preventing false error reporting on CSS trailing whitespace
            skip_spaces_and_comments(str);
            break;
        }
        if (err) {
            // Error: delete chain of selectors
            delete selector;
            // Skip until end of rule
//#ifdef OREDEBUG
            //if (skip_spaces_and_comments(str)) {
            //    lString8 css_error;
            //    while (*str && *str != '}') {
            //        css_error = css_error.append(lString8(str,strlen(str)));
            //        str++;
            //    }
            //    CRLog::info("LVStyleSheet::parse error: %s", css_error.c_str());
            //}
//#else
            while (*str && *str != '}') {
                str++;
            }
//#endif
            if (*str == '}') {
                str++;
            }
            continue;
        }
        // Ok: place rules to sheet
        for (LVCssSelector* nextItem = selector; nextItem;) {
            LVCssSelector* item = nextItem;
#if 0
            CRLog::debug("Selector: %d %s",
                    item->getElementNameId(),
                    item->name_.c_str());
#endif
            nextItem = item->getNext();
            lUInt16 id = item->getElementNameId();
            if (_selectors.length() <= id) {
                _selectors.set(id, NULL);
            }
            if (_selectors[id] == NULL
                || _selectors[id]->getSpecificity() > item->getSpecificity()) {
                // insert as first item
                item->setNext(_selectors[id]);
                _selectors[id] = item;
            } else {
                // insert as internal item
                for (LVCssSelector* p = _selectors[id]; p; p = p->getNext()) {
                    if (p->getNext() == NULL
                            || p->getNext()->getSpecificity() > item->getSpecificity()) {
                        item->setNext(p->getNext());
                        p->setNext(item);
                        break;
                    }
                }
            }
        }
    }
#if 0
    for (int i = 0; i < _selectors.length(); i++)
    {
        if (_selectors[i])
        {
            CRLog::error("Selector: %d %s {",
                    _selectors[i]->getElementNameId(),
                    _selectors[i]->name_.c_str());
            LVCssSelector *sel = _selectors[i];
            while (sel != NULL)
            {
                CRLog::debug("Selector: [%d] %d %s",
                        sel->getSpecificity(),
                        sel->getElementNameId(),
                        sel->name_.c_str());
                        sel = sel->getNext();
            }
            CRLog::error("}");
        }
    }
#endif
    this->GenerateClassMap();
    return _selectors.length() > 0;
}

/// extract @import filename from beginning of CSS
bool LVProcessStyleSheetImport(const char*& str, lString8& import_file)
{
    const char* p = str;
    import_file.clear();
    skip_spaces_and_comments(p);
    if (*p != '@') {
        return false;
    }
    p++;
    if (strncmp(p, "import", 6) != 0) {
        return false;
    }
    p += 6;
    skip_spaces_and_comments(p);
    bool in_url = false;
    char quote_ch = 0;
    if (!strncmp(p, "url", 3)) {
        p += 3;
        skip_spaces_and_comments(p);
        if (*p != '(') {
            return false;
        }
        p++;
        skip_spaces_and_comments(p);
        in_url = true;
    }
    if (*p == '\'' || *p == '\"') {
        quote_ch = *p++;
    }
    while (*p) {
        if (quote_ch && *p == quote_ch) {
            p++;
            break;
        }
        if (!quote_ch) {
            if (in_url && *p == ')') {
                break;
            }
            if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
                break;
            }
        }
        import_file << *p++;
    }
    skip_spaces_and_comments(p);
    if (in_url) {
        if (*p != ')') {
            return false;
        }
        p++;
    }
    if (import_file.empty()) {
        return false;
    }
    str = p;
    return true;
}

/// load stylesheet from file, with processing of import
bool LVLoadStylesheetFile(lString16 pathName, lString8& css)
{
    LVStreamRef file = LVOpenFileStream(pathName.c_str(), LVOM_READ);
    if (file.isNull()) {
        return false;
    }
    lString8 txt = UnicodeToUtf8(LVReadCssText(file));
    lString8 txt2;
    const char* s = txt.c_str();
    lString8 import_file;
    if (LVProcessStyleSheetImport(s, import_file)) {
        lString16 importFilename = LVMakeRelativeFilename(pathName, Utf8ToUnicode(import_file));
        //lString8 ifn = UnicodeToLocal(importFilename);
        //const char * ifns = ifn.c_str();
        if (!importFilename.empty()) {
            LVStreamRef file2 = LVOpenFileStream(importFilename.c_str(), LVOM_READ);
            if (!file2.isNull()) {
                txt2 = UnicodeToUtf8(LVReadCssText(file2));
            }
        }
    }
    if (!txt2.empty()) {
        txt2 << "\r\n";
    }
    css = txt2 + s;
    return !css.empty();
}

LVCssSelectorRule::LVCssSelectorRule(LVCssSelectorRule& v)
        : _type(v._type), _id(v._id), _attrid(v._attrid), _value(v._value), _next(NULL)
{
    if (v._next) {
        _next = new LVCssSelectorRule(*v._next);
    }
}

LVCssSelector::LVCssSelector(LVCssSelector& v)
        : _id(v._id),
          _decl(v._decl),
          _specificity(v._specificity),
          name_(v.name_),
          _next(NULL),
          _rules(NULL)
{
    if (v._next) {
        _next = new LVCssSelector(*v._next);
    }
    if (v._rules) {
        _rules = new LVCssSelectorRule(*v._rules);
    }
}

void LVStyleSheet::set(LVPtrVector<LVCssSelector>& v)
{
    _selectors.clear();
    if (!v.size()) {
        return;
    }
    _selectors.reserve(v.size());
    for (int i = 0; i < v.size(); i++) {
        LVCssSelector* selector = v[i];
        if (selector) {
            _selectors.add(new LVCssSelector(*selector));
        } else {
            _selectors.add(NULL);
        }
    }
}

LVStyleSheet::LVStyleSheet(LVStyleSheet& sheet) : _doc(sheet._doc)
{
    set(sheet._selectors);
    GenerateClassMap();
}

lString16 LVCssSelectorRule::ToString() const
{
    lString16 desc;
    return desc;
}

lString16 LVCssSelector::ToString(CrDomXml* dom) const
{
    lString16 desc;
    if (_id != 0) {
        desc << dom->getElementName(_id);
    }
    if (_rules) {
        LVCssSelectorRule* rule = _rules;
        do {
            desc << " " << rule->ToString();
            rule = rule->getNext();
        } while (rule != NULL);
    }
    return desc;
}

lUInt32 LVCssDeclaration::getHash()
{
    if (!_data) {
        return 0;
    }
    int* p = _data;
    lUInt32 hash = 0;
    for (; *p != cssd_stop; p++) {
        hash = hash * 31 + *p;
    }
    return hash;
}

lUInt32 LVCssSelectorRule::getHash()
{
    lUInt32 hash = 0;
    hash = ((((lUInt32) _type * 31
              + (lUInt32) _id) * 31)
            + (lUInt32) _attrid * 31)
           + ::getHash(_value);
    return hash;
}

lUInt32 LVCssSelector::getHash()
{
    lUInt32 hash = 0;
    lUInt32 nextHash = 0;
    if (_next) {
        nextHash = _next->getHash();
    }
    for (LVCssSelectorRule* p = _rules; p; p = p->getNext()) {
        lUInt32 ruleHash = p->getHash();
        hash = hash * 31 + ruleHash;
    }
    hash = hash * 31 + nextHash;
    if (!_decl.isNull()) {
        hash = hash * 31 + _decl->getHash();
    }
    return hash;
}

lUInt32 LVStyleSheet::getHash()
{
    lUInt32 hash = 0;
    for (int i = 0; i < _selectors.length(); i++) {
        if (_selectors[i]) {
            hash = hash * 31 + _selectors[i]->getHash() + i * 15324;
        }
    }
    return hash;
}

void LVStyleSheet::GenerateClassMap()
{
    selectorMap_.clear();
    LVCssSelector *sel = _selectors[0];
    while (sel != NULL)
    {
        //CRLog::error("sel add to map =  %s, %x",sel->name_.c_str(),sel);
        selectorMap_[sel->name_.getHash()] = sel;
        sel = sel->getNext();
    }
}
