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

#ifndef _OPENREADERA_RECTHELPER_H_
#define _OPENREADERA_RECTHELPER_H_

#include "include/lvtinydom.h"
#include "include/lvtypes.h"

extern bool templogs;

class RectHelper
{
    ldomNode *Node_ = NULL;
    ldomNode *finalNode_ = NULL;
    LFormattedTextRef txtform_  = LFormattedTextRef();
    lvRect absRect_= lvRect();

    int srcIndex_      = -1;
    int srcLen_        = -1;
    int lastIndex_     = -1;
    int lastLen_       = -1;
    int lastOffset_    = -1;
    int NodeIndex_     = -1;
    int LineIndex_     =  0;
    int NodeLineIndex_ =  0;
    bool NodeIsInvisible_ = false;
    bool isInit           = false;

    ldomNode* GetFinalNode();

    void InitFinalNode(ldomNode *finalNode);

    void InitNode(ldomNode *Node);

    int FindNodeIndex(ldomNode *node, int start);

    int FindLineIndex(int start);

    int FindLastIndex(ldomNode *node);

    bool ifnull(ldomXPointerEx xpointer, lvRect &rect);

    void Invalidate();

    bool NodeIsInvisible(ldomNode *node);

public:

    RectHelper() {};

    /*~RectHelper() {
      free(Node_);
      free(finalNode_);
      free(txtform_.get());
      free(&absRect_);
    };*/

    void Init(ldomNode *Node);

    void Init(ldomXRange *range);

    lvRect getRect(ldomWord word);

    lvRect getRect(ldomWord word, bool init);

    lvRect getRect(ldomXPointer xPointer);

    bool processRect(ldomXPointerEx xpointer, lvRect &rect);

    //FOR TOC
    bool FindLastIndexEnable_ = false;

    void ResetLineIndex();
};


#endif //_OPENREADERA_RECTHELPER_H_
