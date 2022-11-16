/**
    Implements CSS compiler for CoolReader Engine.

    Supports only subset of CSS.

    (c) Vadim Lopatin, 2000-2006
    Copyright (C) 2013-2020 READERA LLC

    This source code is distributed under the terms of
    GNU General Public License.

    See LICENSE file for details.
*/

#ifndef __LVSTSHEET_H_INCLUDED__
#define __LVSTSHEET_H_INCLUDED__

#include "cssdef.h"
#include "lvstyles.h"
#include <map>

class CrDomXml;
class ldomNode;

/**
    CSS property declaration. Currently supports only subset of properties.

    Properties supported:

    - display
    - white-space
    - text-align
    - vertical-align
    - font-family
    - font-size
    - font-style
    - font-weight
    - text-indent
    - line-height
    - width
    - height
    - margin-left
    - margin-right
    - margin-top
    - margin-bottom
    - margin
*/
class LVCssDeclaration {
private:
    int* _data;
public:
    void apply(const ldomNode* node, css_style_rec_t* style);
    bool empty() { return _data==NULL; }
    bool parse(const char*& decl);
    lUInt32 getHash();
    LVCssDeclaration() : _data(NULL) { }
    ~LVCssDeclaration()
    {
        if (_data) { delete[] _data; }
    }
};

typedef LVRef<LVCssDeclaration> LVCssDeclRef;

enum LVCssSelectorRuleType
{
    cssrt_universal,     // *
    cssrt_parent,        // E > F
    cssrt_parent_class,  // E.class > F
    cssrt_ancessor,      // E F
    cssrt_predecessor,   // E + F
    cssrt_attrset,       // E[foo]
    cssrt_attreq,        // E[foo="value"]
    cssrt_attrhas,       // E[foo~="value"]
    cssrt_attrstarts,    // E[foo|="value"]
    cssrt_id,            // E#id
    cssrt_class          // E.class
};

class LVCssSelectorRule
{
private:
    LVCssSelectorRuleType _type;
    lUInt16 _id;
    lUInt16 _attrid;
    lString16 _value;
    LVCssSelectorRule* _next;
public:
    LVCssSelectorRule(LVCssSelectorRuleType type)
    : _type(type), _id(0), _attrid(0), _next(NULL)
    { }
    LVCssSelectorRule(LVCssSelectorRule& v);
    LVCssSelectorRuleType getType() { return _type; };
    void setType(LVCssSelectorRuleType type) { _type = type; };
    void setId(lUInt16 id) { _id = id; }
    lUInt16 getAttrId() { return _attrid; }
    lString16 getValue() { return _value; }
    void setAttr(lUInt16 attr_id, lString16 value) { _attrid = attr_id; _value = value; }
    bool check(const ldomNode*& node);
    LVCssSelectorRule* getNext() { return _next; }
    void setNext(LVCssSelectorRule* next) { _next = next; }
    ~LVCssSelectorRule() { if (_next) delete _next; }
    lString16 ToString() const;
    lUInt32 getHash();
};

/**
    Simple CSS selector, currently supports only element name and universal selector.

    - * { } - universal selector
    - element-name { } - selector by element name
    - element1, element2 { } - several selectors delimited by comma
*/
class LVCssSelector {
private:
    lUInt16 _id;
    LVCssDeclRef _decl;
    int _specificity;
    LVCssSelector* _next;
    LVCssSelectorRule* _rules;
    void insertRuleStart(LVCssSelectorRule* rule);
    void insertRuleAfterStart(LVCssSelectorRule* rule);
public:
    LVCssSelector(int specifity) : _id(0), _specificity(specifity), _next(NULL), _rules(NULL) { }
    LVCssSelector(LVCssSelector& v);
    ~LVCssSelector()
    {
        if (_next) {
            delete _next;
        }
        if (_rules) {
            delete _rules;
        }
    }
    bool parse(const char* &str, CrDomXml* doc);
    lUInt16 getElementNameId() { return _id; }
    bool check(const ldomNode* node ) const;
    void applyCss(const ldomNode* node, css_style_rec_t* style) const
    {
        _decl->apply(node, style);
    }
    void setDeclaration(LVCssDeclRef decl) { _decl = decl; }
    int getSpecificity() { return _specificity; }
    LVCssSelector* getNext() { return _next; }
    void setNext(LVCssSelector* next) { _next = next; }
    lUInt32 getHash();
    lString8 name_;
    lString16 ToString(CrDomXml* dom) const;
};

typedef std::map<lUInt32,LVCssSelector*> CssSelectorMap;

/**
   Can parse stylesheet and apply compiled rules. Supports only subset of CSS features.
*/
class LVStyleSheet {
private:
    CrDomXml* _doc;
    LVPtrVector <LVCssSelector> _selectors;
    LVPtrVector <LVPtrVector <LVCssSelector> > _stack;

    LVPtrVector<LVCssSelector>* dup()
    {
        LVPtrVector<LVCssSelector>* res = new LVPtrVector<LVCssSelector>();
        for (int i = 0; i < _selectors.length(); i++) {
            LVCssSelector* selector = _selectors[i];
            if (selector) {
                res->add(new LVCssSelector(*selector));
            } else {
                res->add(NULL);
            }
        }
        return res;
    }

    void set(LVPtrVector<LVCssSelector>& v);

    CssSelectorMap selectorMap_;
public:
    /// save current state of stylesheet
    void push() { _stack.add(dup()); }
    /// restore previously saved state
    bool pop()
    {
        LVPtrVector<LVCssSelector>* v = _stack.pop();
        if (!v) {
            return false;
        }
        set(*v);
        GenerateClassMap();
        delete v;
        return true;
    }
    /// remove all rules from stylesheet
    void clear()
    {
        _selectors.clear();
        _stack.clear();
        selectorMap_.clear();
    }
    /// set document to retrieve ID values from
    void setDocument(CrDomXml* doc) { _doc = doc; }
    /// constructor
    LVStyleSheet(CrDomXml* doc = NULL ) : _doc(doc) { }
    /// copy constructor
    LVStyleSheet(LVStyleSheet& sheet);
    /// parse stylesheet, compile and add found rules to sheet
    bool parse(const char* str );
    /// apply stylesheet to node style
    void applyCss(const ldomNode* node, css_style_rec_t* style );

    /// calculate hash
    lUInt32 getHash();

    void GenerateClassMap();
};

/// extract @import filename from beginning of CSS
bool LVProcessStyleSheetImport(const char*& str, lString8& import_file);
/// load stylesheet from file, with processing of import
bool LVLoadStylesheetFile(lString16 pathName, lString8& css);

#endif // __LVSTSHEET_H_INCLUDED__