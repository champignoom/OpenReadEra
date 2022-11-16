/** \file props.cpp
    \brief properties container

    CoolReader Engine

    (c) Vadim Lopatin, 2000-2007
    This source code is distributed under the terms of
    GNU General Public License
    See LICENSE file for details
*/

#include "../include/props.h"
#include <stdio.h>


//============================================================================
// CRPropContainer declarations
//============================================================================

class CRPropItem
{
private:
    lString8 _name;
    lString16 _value;
public:
    CRPropItem( const char * name, const lString16 value )
    : _name(name), _value(value)
    { }
    CRPropItem( const CRPropItem& v )
    : _name( v._name )
    , _value( v._value )
    { }
    CRPropItem & operator = ( const CRPropItem& v )
    {
      _name = v._name;
      _value = v._value;
      return *this;
    }
    const char * getName() const { return _name.c_str(); }
    const lString16 & getValue() const { return _value; }
    void setValue(const lString16 &v) { _value = v; }
};

/// set contents from specified properties
void CRPropAccessor::set( const CRPropRef & v )
{
    clear();
    int sz = v->getCount();
    for ( int i=0; i<sz; i++ )
        setString( v->getName(i), v->getValue(i) );
}

/// calc props1 - props2
CRPropRef operator - ( CRPropRef props1, CRPropRef props2 )
{
    CRPropRef v = LVCreatePropsContainer();
    int cnt1 = props1->getCount();
    int cnt2 = props2->getCount();
    int p1 = 0;
    int p2 = 0;
    while ( (p1<=cnt1 && p2<=cnt2) && (p1<cnt1 || p2<cnt2) ) {
        if ( p1==cnt1 ) {
            break;
        } else if ( p2==cnt2 ) {
            v->setString( props1->getName( p1 ), props1->getValue( p1 ) );
            p1++;
        } else {
            int res = lStr_cmp( props1->getName( p1 ), props2->getName( p2 ) );
            if ( res<0 ) {
                v->setString( props1->getName( p1 ), props1->getValue( p1 ) );
                p1++;
            } else if ( res==0 ) {
                p1++;
                p2++;
            } else { // ( res>0 )
                p2++;
            }
        }
    }
    return v;
}
/// calc props1 | props2
CRPropRef operator | ( CRPropRef props1, CRPropRef props2 )
{
    CRPropRef v = LVCreatePropsContainer();
    int cnt1 = props1->getCount();
    int cnt2 = props2->getCount();
    int p1 = 0;
    int p2 = 0;
    while ( (p1<=cnt1 && p2<=cnt2) && (p1<cnt1 || p2<cnt2) ) {
        if ( p1==cnt1 ) {
            v->setString( props2->getName( p2 ), props2->getValue( p2 ) );
            p2++;
        } else if ( p2==cnt2 ) {
            v->setString( props1->getName( p1 ), props1->getValue( p1 ) );
            p1++;
        } else {
            int res = lStr_cmp( props1->getName( p1 ), props2->getName( p2 ) );
            if ( res<0 ) {
                v->setString( props1->getName( p1 ), props1->getValue( p1 ) );
                p1++;
            } else if ( res==0 ) {
                v->setString( props1->getName( p1 ), props1->getValue( p1 ) );
                p1++;
                p2++;
            } else { // ( res>0 )
                v->setString( props2->getName( p2 ), props2->getValue( p2 ) );
                p2++;
            }
        }
    }
    return v;
}
/// calc props1 & props2
CRPropRef operator & ( CRPropRef props1, CRPropRef props2 )
{
    CRPropRef v = LVCreatePropsContainer();
    int cnt1 = props1->getCount();
    int cnt2 = props2->getCount();
    int p1 = 0;
    int p2 = 0;
    while ( (p1<=cnt1 && p2<=cnt2) && (p1<cnt1 || p2<cnt2) ) {
        if ( p1==cnt1 ) {
            break;
        } else if ( p2==cnt2 ) {
            break;
        } else {
            int res = lStr_cmp( props1->getName( p1 ), props2->getName( p2 ) );
            if ( res<0 ) {
                p1++;
            } else if ( res==0 ) {
                v->setString( props1->getName( p1 ), props1->getValue( p1 ) );
                p1++;
                p2++;
            } else { // ( res>0 )
                p2++;
            }
        }
    }
    return v;
}

/// returns added or changed items of props2 compared to props1
CRPropRef operator ^ ( CRPropRef props1, CRPropRef props2 )
{
    CRPropRef v = LVCreatePropsContainer();
    int cnt1 = props1->getCount();
    int cnt2 = props2->getCount();
    int p1 = 0;
    int p2 = 0;
    while ( (p1<=cnt1 && p2<=cnt2) && (p1<cnt1 || p2<cnt2) ) {
        if ( p1==cnt1 ) {
            v->setString( props2->getName( p2 ), props2->getValue( p2 ) );
            p2++;
        } else if ( p2==cnt2 ) {
            break;
        } else {
            int res = lStr_cmp( props1->getName( p1 ), props2->getName( p2 ) );
            if ( res<0 ) {
                p1++;
            } else if ( res==0 ) {
                lString16 v1 = props1->getValue( p1 );
                lString16 v2 = props2->getValue( p2 );
                if ( v1!=v2 )
                    v->setString( props2->getName( p2 ), v2 );
                p1++;
                p2++;
            } else { // ( res>0 )
                v->setString( props2->getName( p2 ), props2->getValue( p2 ) );
                p2++;
            }
        }
    }
    return v;
}

class CRPropContainer : public CRPropAccessor
{
private:
    LVPtrVector<CRPropItem> _list;
protected:
    bool findItem( const char * name, int nameoffset, int start, int end, int & pos ) const;
    bool findItem( const char * name, int & pos ) const;
    void clear( int start, int end );
    CRPropContainer( const CRPropContainer & v )
    : _list( v._list )
    {
    }
public:
    /// returns true if specified property exists
    virtual bool hasProperty( const char * propName ) const
    {
        int pos;
        return findItem( propName, pos );
    }
    /// clear all items
    virtual void clear();
    /// returns property path in root container
    virtual const lString8 & getPath() const;
    /// returns property item count in container
    virtual int getCount() const;
    /// returns property name by index
    virtual const char * getName( int index ) const;
    /// returns property value by index
    virtual const lString16 & getValue( int index ) const;
    /// sets property value by index
    virtual void setValue( int index, const lString16 &value );
    /// get string property by name, returns false if not found
    virtual bool getString( const char * propName, lString16 &result ) const;
    /// set string property by name
    virtual void setString( const char * propName, const lString16 &value );
    /// constructor
    CRPropContainer();
    /// virtual destructor
    virtual ~CRPropContainer();
};

/// set string property by name, if it's not set already
void CRPropAccessor::setStringDef( const char * propName, const char * defValue )
{
    if ( !hasProperty( propName ) )
        setString( propName, Utf8ToUnicode( lString8( defValue ) ) );
}

/// set int property by name, if it's not set already
void CRPropAccessor::setIntDef( const char * propName, int value )
{
    if ( !hasProperty( propName ) )
        setInt( propName, value );
}

//============================================================================
// CRPropAccessor methods
//============================================================================

lString16 CRPropAccessor::getStringDef( const char * propName, const char * defValue ) const
{
    lString16 value;
    if ( !getString( propName, value ) )
        return lString16( defValue );
    else
        return value;
}

bool CRPropAccessor::getInt( const char * propName, int &result ) const
{
    lString16 value;
    if ( !getString( propName, value ) )
        return false;
    return value.atoi(result);
}

int CRPropAccessor::getIntDef( const char * propName, int defValue ) const
{
    int v = 0;
    if ( !getInt( propName, v ) )
        return defValue;
    else
        return v;
}

/// set int property as hex
void CRPropAccessor::setHex( const char * propName, int value )
{
    char s[16];
    sprintf(s, "0x%08X", value);
    setString( propName, Utf8ToUnicode(lString8(s)) );
}

void CRPropAccessor::setInt( const char * propName, int value )
{
    setString( propName, lString16::itoa( value ) );
}

bool CRPropAccessor::parseColor(lString16 value, lUInt32 & result) {
    int n = 0;
    if ( value.empty() || (value[0]!='#' && (value[0]!='0' || value[1]!='x')) ) {
        return false;
    }
    for ( int i=value[0]=='#' ? 1 : 2; i<value.length(); i++ ) {
        lChar16 ch = value[i];
        if ( ch>='0' && ch<='9' )
            n = (n << 4) | (ch - '0');
        else if ( ch>='a' && ch<='f' )
            n = (n << 4) | (ch - 'a' + 10);
        else if ( ch>='A' && ch<='F' )
            n = (n << 4) | (ch - 'A' + 10);
        else
            return false;
    }
    result = (lUInt32)n;
    return true;
}

/// get color (#xxxxxx) property by name, returns false if not found
bool CRPropAccessor::getColor( const char * propName, lUInt32 &result ) const
{
    lString16 value;
    if (!getString(propName, value)) {
        //CRLog::debug("%s is not found", propName);
        return false;
    }
    return parseColor(value, result);
}


/// get color (#xxxxxx) property by name, returns default value if not found
lUInt32 CRPropAccessor::getColorDef( const char * propName, lUInt32 defValue ) const
{
    lUInt32 v = 0;
//    CRLog::debug("getColorDef(%s), 0x%06x", propName, defValue);
    if ( !getColor( propName, v ) )
        return defValue;
    else
        return v;
}

/// set color (#xxxxxx) property by name
void CRPropAccessor::setColor( const char * propName, lUInt32 value )
{
    char s[12];
    sprintf( s, "#%06x", (int)value );
    setString( propName, lString16( s ) );
}

/// set argb color (#xxxxxx) property by name, if not set
void CRPropAccessor::setColorDef( const char * propName, lUInt32 defValue ) {
    lUInt32 v = 0;
    if (!getColor(propName, v))
        setColor(propName, defValue);
}

bool CRPropAccessor::getBool( const char * propName, bool &result ) const
{
    lString16 value;
    if (!getString(propName, value))
        return false;
    if (value == "true" || value == "TRUE" || value == "yes" || value == "YES" || value == "1") {
        result = true;
        return true;
    }
    if (value == "false" || value == "FALSE" || value == "no" || value == "NO" || value == "0") {
        result = false;
        return true;
    }
    return false;
}

bool CRPropAccessor::getBoolDef(const char * propName, bool defValue) const
{
    bool v = 0;
    if ( !getBool( propName, v ) )
        return defValue;
    else
        return v;
}

void CRPropAccessor::setBool( const char * propName, bool value )
{
    setString( propName, lString16( value ? "1" : "0" ) );
}

CRPropAccessor::~CRPropAccessor()
{
}

//============================================================================
// CRPropContainer methods
//============================================================================

CRPropContainer::CRPropContainer() { }

/// returns property path in root container
const lString8 & CRPropContainer::getPath() const
{
    return lString8::empty_str;
}

/// returns property item count in container
int CRPropContainer::getCount() const
{
    return _list.length();
}

/// returns property name by index
const char * CRPropContainer::getName( int index ) const
{
    return _list[index]->getName();
}

/// returns property value by index
const lString16 & CRPropContainer::getValue( int index ) const
{
    return _list[index]->getValue();
}

/// sets property value by index
void CRPropContainer::setValue( int index, const lString16 &value )
{
    _list[index]->setValue( value );
}

/// binary search
bool CRPropContainer::findItem( const char * name, int nameoffset, int start, int end, int & pos ) const
{
    int a = start;
    int b = end;
    while ( a < b ) {
        int c = (a + b) / 2;
        int res = lStr_cmp( name, _list[c]->getName() + nameoffset );
        if ( res == 0 ) {
            pos = c;
            return true;
        } else if ( res<0 ) {
            b = c;
        } else {
            a = c + 1;
        }
    }
    pos = a;
    return false;
}

/// binary search
bool CRPropContainer::findItem( const char * name, int & pos ) const
{
    return findItem( name, 0, 0, _list.length(), pos );
}

/// get string property by name, returns false if not found
bool CRPropContainer::getString( const char * propName, lString16 &result ) const
{
    int pos = 0;
    if ( !findItem( propName, pos ) )
        return false;
    result = _list[pos]->getValue();
    return true;
}

/// clear all items
void CRPropContainer::clear()
{
    _list.clear();
}

/// set string property by name
void CRPropContainer::setString( const char * propName, const lString16 &value )
{
    int pos = 0;
    if ( !findItem( propName, pos ) ) {
        _list.insert( pos, new CRPropItem( propName, value ) );
    } else {
        _list[pos]->setValue( value );
    }
}

/// virtual destructor
CRPropContainer::~CRPropContainer()
{
}

void CRPropContainer::clear( int start, int end )
{
    _list.erase( start, end-start );
}

/// factory function
CRPropRef LVCreatePropsContainer()
{
    return CRPropRef(new CRPropContainer());
}
