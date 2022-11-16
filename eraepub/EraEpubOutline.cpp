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
 * Developers: ReadEra Team (2013-2020), Playful Curiosity (2013-2020),
 * Tarasus (2018-2020).
 */

#include "EraEpubBridge.h"

void CreBridge::processOutline(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_OUTLINE;
    response.addInt((uint32_t) 0);
    int columns = doc_view_->GetColumns();
    LVPtrVector<LvTocItem, false> outline;
    doc_view_->GetOutline(outline);
    CRLog::trace("processOutline size: %d", outline.length());
#ifdef OREDEBUG
    #if 0
    for (int i = 0; i < 60000; i++) {
        response.addWords((uint16_t) OUTLINE_TARGET_XPATH, 1);
        response.addInt((uint32_t) 0);
        responseAddString(response, lString16("chapter ").appendDecimal(i));
        responseAddString(response, lString16("xpath"));
    }
    return;
#endif
#if 0
    for (int i = 0; i < outline.length(); i++) {
        LvTocItem* row = outline[i];
        uint16_t row_page = (uint16_t) ExportPage(columns, row->getPage());
        CRLog::trace("%s, %d, %d, %s",
                     LCSTR(row->getName()), row_page, row->getLevel() - 1, LCSTR(row->getPath()));
    }
#endif
#endif
    for (int i = 0; i < outline.length(); i++) {
        LvTocItem* row = outline[i];
        auto row_page = (uint16_t) ExportPage(columns, row->getPage());
        response.addWords((uint16_t) OUTLINE_TARGET_XPATH, row_page);
        // EraEPUB level is one-based while OpenReadEra is using zero-based levels
        response.addInt((uint32_t) row->getLevel() - 1);
        responseAddString(response, row->getName().restoreIndicText());
        responseAddString(response, row->getPath());
    }
}
