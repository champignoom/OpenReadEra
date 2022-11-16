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

void CreBridge::processPageLinks(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_LINKS;
    CmdDataIterator iter(request.first);
    uint32_t external_page = 0;
    iter.getInt(&external_page);
    if (!iter.isValid()) {
        CRLog::error("processPageLinks bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    auto page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    doc_view_->GoToPage(page);
    LVArray<Hitbox> pageLinks = doc_view_->GetPageLinks();

    for (int i = 0; i < pageLinks.length(); i++)
    {
        uint16_t target_page = 0;
        Hitbox curr_link = pageLinks.get(i);
        float l = curr_link.left_;
        float t = curr_link.top_;
        float r = curr_link.right_;
        float b = curr_link.bottom_;
        if(gJapaneseVerticalMode)
        {
            float t_t = t;
            float t_b = b;

            t_t -=0.5f;
            t_b -=0.5f;

            t_t = (t_t<0)? fabs(t_t): -t_t;
            t_b = (t_b<0)? fabs(t_b): -t_b;

            t_t +=0.5f;
            t_b +=0.5f;

            t = l;
            b = r;
            l = t_b;
            r = t_t;
        }

        lString16 href = curr_link.text_;

        if (href.length() > 1 && href[0] == '#')
        {
            lString16 ref = href.substr(1, href.length() - 1);
            lUInt16 id = doc_view_->GetCrDom()->getAttrValueIndex(ref.c_str());
            ldomNode* node = doc_view_->GetCrDom()->getNodeById(id);
            if (node) {
                ldomXPointer position(node, 0);
                target_page = (uint16_t) doc_view_->GetPageForBookmark(position);
                target_page = (uint16_t) ExportPage(doc_view_->GetColumns(), target_page);
                response.addWords(LINK_TARGET_PAGE, target_page);
                response.addFloat(l);
                response.addFloat(t);
                response.addFloat(r);
                response.addFloat(b);
                response.addFloat(.0F);
                response.addFloat(.0F);
            } else {
                responseAddLinkUnknown(response, href, l, t, r, b);
            }
        } else if (href.startsWith("http:") || href.startsWith("https:")) {
            response.addWords(LINK_TARGET_URI, 0);
            responseAddString(response, href);
            response.addFloat(l);
            response.addFloat(t);
            response.addFloat(r);
            response.addFloat(b);
        } else {
            responseAddLinkUnknown(response, href, l, t, r, b);
        }
    }

    LVArray<TextRect> fnoteslist = doc_view_->GetCurrentPageFootnotesLinks();
    if(fnoteslist.empty())
    {
        return;
    }
    float width = doc_view_->GetWidth();
    float height = doc_view_->GetHeight();

    for (int i = 0; i < fnoteslist.length(); i++)
    {
        uint16_t target_page = 0;
        TextRect curr_link = fnoteslist.get(i);
        float l = curr_link.getRect().left   / width  ;
        float t = curr_link.getRect().top    / height ;
        float r = curr_link.getRect().right  / width  ;
        float b = curr_link.getRect().bottom / height ;
        lString16 href = curr_link.getText();

        if (href.length() > 1 && href[0] == '#')
        {
            lString16 ref = href.substr(1, href.length() - 1);
            lUInt16 id = doc_view_->GetCrDom()->getAttrValueIndex(ref.c_str());
            ldomNode* node = doc_view_->GetCrDom()->getNodeById(id);
            if (node) {
                ldomXPointer position(node, 0);
                target_page = (uint16_t) doc_view_->GetPageForBookmark(position);
                target_page = (uint16_t) ExportPage(doc_view_->GetColumns(), target_page);
                response.addWords(LINK_TARGET_PAGE, target_page);
                response.addFloat(l);
                response.addFloat(t);
                response.addFloat(r);
                response.addFloat(b);
                response.addFloat(.0F);
                response.addFloat(.0F);
            } else {
                responseAddLinkUnknown(response, href, l, t, r, b);
            }
        } else if (href.startsWith("http:") || href.startsWith("https:")) {
            response.addWords(LINK_TARGET_URI, 0);
            responseAddString(response, href);
            response.addFloat(l);
            response.addFloat(t);
            response.addFloat(r);
            response.addFloat(b);
        } else {
            responseAddLinkUnknown(response, href, l, t, r, b);
        }
        //CRLog::error("ltrb = %f, %f, %f, %f , %s", l,t,r,b,LCSTR(href));
    }
}


