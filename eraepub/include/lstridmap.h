/** \file lstridmap.h
    \brief Name <-> Id map

   CoolReader Engine DOM Tree 

   Implements mapping between Name and Id

   (c) Vadim Lopatin, 2000-2006

   This source code is distributed under the terms of
   GNU General Public License.

   See LICENSE file for details.

*/

#ifndef __LSTR_ID_MAP_H__INCLUDED__
#define __LSTR_ID_MAP_H__INCLUDED__

#include <stdio.h>
#include "lvstring.h"

struct css_elem_def_props_t;

class CrStrIntPair {
    /// custom data pointer
    css_elem_def_props_t* data;
public:
    /// id
    lUInt16 id;
    /// value
    lString16 value;
	const css_elem_def_props_t* getData() const { return data; }
	/// constructor
	CrStrIntPair(lUInt16 _id, const lString16& _value, const css_elem_def_props_t* _data);
	/// copy constructor
	CrStrIntPair(CrStrIntPair& item);
	/// destructor
	~CrStrIntPair();
};

class LvDomNameIdMap {
private:
    CrStrIntPair** m_by_id;
    CrStrIntPair** m_by_name;
    lUInt16 m_count; // non-empty count
    lUInt16 m_size;  // max number of ids
    bool m_sorted;
    bool m_changed;
    void Sort();

public:
    /// Main constructor
    LvDomNameIdMap( lUInt16 maxId );
    /// Copy constructor
    LvDomNameIdMap( LvDomNameIdMap & map );
    ~LvDomNameIdMap();

    void Clear();

    void AddItem(lUInt16 id, const lString16& value, const css_elem_def_props_t* data);

    void AddItem(CrStrIntPair* item );

    const CrStrIntPair* findItem(lUInt16 id) const
    {
       return m_by_id[id];
    }

    const CrStrIntPair* FindPair(const lChar16* name);
    const CrStrIntPair* FindPair(const lChar8* name);
    const CrStrIntPair* FindPair(const lString16& name) { return FindPair(name.c_str()); }

    inline lUInt16 IntByStr(const lChar16* name)
    {
        const CrStrIntPair* item = FindPair(name);
        return item?item->id:0;
    }

    inline lUInt16 IntByStr(const lChar8* name)
    {
        const CrStrIntPair* item = FindPair(name);
        return item ? item->id : 0;
    }

    inline const lString16& StrByInt(lUInt16 id)
    { 
        if (id >= m_size)
            return lString16::empty_str;
        const CrStrIntPair* item = findItem(id);
        return item ? item->value : lString16::empty_str;
    }

    inline const css_elem_def_props_t* dataById(lUInt16 id)
    { 
        if (id >= m_size)
            return NULL;
        const CrStrIntPair* item = findItem(id);
        return item ? item->getData() : NULL;
    }

    // debug dump of all unknown entities
    void dumpUnknownItems(FILE * f, int start_id);
};

#endif
