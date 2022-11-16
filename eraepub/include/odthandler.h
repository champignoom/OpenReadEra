/*
 * Copyright (C) 2013-2020 READERA LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Developers: ReadEra Team (2013-2020), Tarasus (2018-2020).
 */

#ifndef _OPENREADERA_ODTHANDLER_H_
#define _OPENREADERA_ODTHANDLER_H_


#include "lvstream.h"
#include "lvptrvec.h"

class OdtStyles
{
public:
    int setfields = 0;

    lString16 h1id_;
    lString16 h2id_;
    lString16 h3id_;
    lString16 h4id_;
    lString16 h5id_;
    lString16 h6id_;

    void addHeader(int num, lString16 id)
    {
        lString16 * selected;
        switch (num)
        {
            case 1: h1id_ = id; setfields++; break;
            case 2: h2id_ = id; setfields++; break;
            case 3: h3id_ = id; setfields++; break;
            case 4: h4id_ = id; setfields++; break;
            case 5: h5id_ = id; setfields++; break;
            case 6: h6id_ = id; setfields++; break;
            default: break;
        }
    }

    int GetHeaderById(const lString16 &id)
    {
             if (id == h1id_) return 1;
        else if (id == h2id_) return 2;
        else if (id == h3id_) return 3;
        else if (id == h4id_) return 4;
        else if (id == h5id_) return 5;
        else if (id == h6id_) return 6;
        else return 0;
    }
};

#endif //_OPENREADERA_ODTHANDLER_H_
