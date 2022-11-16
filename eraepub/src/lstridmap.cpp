/*******************************************************

   CoolReader Engine DOM Tree 

   LDOMNodeIdMap.cpp:  Name to Id map

   (c) Vadim Lopatin, 2000-2006
   This source code is distributed under the terms of
   GNU General Public License
   See LICENSE file for details

*******************************************************/

#include "include/lstridmap.h"
#include "include/dtddef.h"

CrStrIntPair::CrStrIntPair(lUInt16 _id, const lString16& _value,
		const css_elem_def_props_t* _data)
		: id(_id),
		  value(_value) {
	if (_data) {
        data = new css_elem_def_props_t();
		*data = *_data;
	} else
		data = NULL;
}

CrStrIntPair::CrStrIntPair(CrStrIntPair& item) : id(item.id), value(item.value) {
	if (item.data) {
		data = new css_elem_def_props_t();
		*data = *item.data;
	} else {
		data = NULL;
	}
}

CrStrIntPair::~CrStrIntPair()
{
	if (data)
		delete data;
}

LvDomNameIdMap::LvDomNameIdMap(lUInt16 maxId)
{
    m_size = maxId+1;
    m_count = 0;
    m_by_id = new CrStrIntPair* [m_size];
    memset(m_by_id, 0, sizeof(CrStrIntPair*) * m_size);
    m_by_name = new CrStrIntPair* [m_size];
    memset(m_by_name, 0, sizeof(CrStrIntPair*) * m_size);
    m_sorted = true;
    m_changed = false;
}

/// Copy constructor
LvDomNameIdMap::LvDomNameIdMap(LvDomNameIdMap & map)
{
    m_changed = false;
    m_size = map.m_size;
    m_count = map.m_count;
    m_by_id   = new CrStrIntPair * [m_size];
    int i;
    for ( i=0; i<m_size; i++ ) {
        if ( map.m_by_id[i] )
            m_by_id[i] = new CrStrIntPair( *map.m_by_id[i] );
        else
            m_by_id[i] = NULL;
    }
    m_by_name = new CrStrIntPair * [m_size];
    for ( i=0; i<m_size; i++ ) {
        if ( map.m_by_name[i] )
            m_by_name[i] = new CrStrIntPair( *map.m_by_name[i] );
        else
            m_by_name[i] = NULL;
    }
    m_sorted = map.m_sorted;
}

LvDomNameIdMap::~LvDomNameIdMap()
{
    Clear();
    delete[] m_by_name;
    delete[] m_by_id;
}

static int compare_items( const void * item1, const void * item2 )
{
    return (*((CrStrIntPair **)item1))->value.compare( (*((CrStrIntPair **)item2))->value );
}

void LvDomNameIdMap::Sort()
{
    if (m_count>1)
        qsort( m_by_name, m_count, sizeof(CrStrIntPair*), compare_items );
    m_sorted = true;
}

const CrStrIntPair* LvDomNameIdMap::FindPair(const lChar16* name)
{
    if (m_count==0 || !name || !*name)
        return NULL;
    if (!m_sorted)
        Sort();
    lUInt16 a, b, c;
    int r;
    a = 0;
    b = m_count;
    for (;;) {
        c = (a + b)>>1;
        r = lStr_cmp( name, m_by_name[c]->value.c_str() );
        if (r == 0)
            return m_by_name[c]; // found
        if (b==a+1)
            return NULL; // not found
        if (r>0) {
            a = c;
        } else {
            b = c;
        }
    }
}

const CrStrIntPair* LvDomNameIdMap::FindPair(const lChar8* name)
{
    if (m_count==0 || !name || !*name)
        return NULL;
    if (!m_sorted)
        Sort();
    lUInt16 a, b, c;
    int r;
    a = 0;
    b = m_count;
    for (;;) {
        c = (a + b)>>1;
        r = lStr_cmp(name, m_by_name[c]->value.c_str());
        if (r == 0)
            return m_by_name[c]; // found
        if (b == a + 1)
            return NULL; // not found
        if (r > 0) {
            a = c;
        } else {
            b = c;
        }
    }
}

void LvDomNameIdMap::AddItem(CrStrIntPair* item)
{
    if (item == NULL)
        return;
    if (item->id == 0) {
        delete item;
        return;
    }
    //CRLog::debug("LDOMNameIdMap::AddItem %s is %d", LCSTR(item->value), item->id);
    if (item->id >= m_size)
    {
        // Reallocate storage
        lUInt16 newsize = item->id+16;
        m_by_id = (CrStrIntPair **)realloc( m_by_id, sizeof(CrStrIntPair *)*newsize );
        m_by_name = (CrStrIntPair **)realloc( m_by_name, sizeof(CrStrIntPair *)*newsize );
        for (lUInt16 i = m_size; i<newsize; i++)
        {
            m_by_id[i] = NULL;
            m_by_name[i] = NULL;
        }
        m_size = newsize;
    }
    if (m_by_id[item->id] != NULL)
    {
        delete item;
        return; // already exists
    }
    m_by_id[item->id] = item;
    m_by_name[m_count++] = item;
    m_sorted = false;
    if (!m_changed) {
        m_changed = true;
        //CRLog::debug("New ID for %s is %d", LCSTR(item->value), item->id);
    }
}

void LvDomNameIdMap::AddItem(lUInt16 id, const lString16& value, const css_elem_def_props_t* data)
{
    if (id == 0)
        return;
    CrStrIntPair* item = new CrStrIntPair(id, value, data);
    AddItem(item);
}

void LvDomNameIdMap::Clear()
{
    for (lUInt16 i = 0; i<m_count; i++)
    {
        if (m_by_name[i])
            delete m_by_name[i];
    }
    memset( m_by_id, 0, sizeof(CrStrIntPair *)*m_size);
    m_count = 0;
}

void LvDomNameIdMap::dumpUnknownItems( FILE * f, int start_id )
{
    for (int i=start_id; i<m_size; i++)
    {
        if (m_by_id[i] != NULL)
        {
            lString8 s8( m_by_id[i]->value.c_str() );
            fprintf( f, "%d %s\n", m_by_id[i]->id, s8.c_str() );
        }
    }
}