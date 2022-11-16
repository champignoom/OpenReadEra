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

#include "include/RectHelper.h"
#include "include/crconfig.h"

void RectHelper::Invalidate()
{
    finalNode_ = NULL;
    absRect_ = lvRect();
    NodeIndex_     = -1;
    LineIndex_     =  0;
    NodeLineIndex_ =  0;
    NodeIsInvisible_ = false;
}

void RectHelper::Init(ldomXRange *range)
{
    ldomNode *  Node = range->getStartNode();
    Init(Node);
}

void RectHelper::Init(ldomNode *Node)
{
    if(Node == Node_)
    {
        return;
    }
    Node_ = Node;
    ldomNode * finalNode = GetFinalNode();


    if (finalNode != finalNode_)
    {
        #if DEBUG_GETRECT_LOGS
        CRLog::warn("Init FinalNode");
        #endif
        InitFinalNode(finalNode);
    }
    #if DEBUG_GETRECT_LOGS
    CRLog::warn("Update node");
    #endif
    InitNode(Node_);
    #if DEBUG_GETRECT_LOGS
    //CRLog::error("NEW FINALNODE = [%s]",LCSTR(finalNode_->getXPath()));
    //CRLog::error("Node_ Text = %s",LCSTR(Node_->getText()));
    #endif
}

bool RectHelper::NodeIsInvisible(ldomNode *node)
{
    if (node == NULL)
    {
        return false;
    }
    if (node->getRendMethod() == erm_invisible)
    {
        #if DEBUG_GETRECT_LOGS
        CRLog::error("INVISIBLE NODE FOUND");
        CRLog::error("node = [%s]", LCSTR(node->getXPath()));
        #endif
        return true;
    }
    return false;
}

ldomNode *RectHelper::GetFinalNode()
{
    ldomNode * p = Node_->isElement() ? Node_: Node_->getParentNode() ;
    ldomNode * mainNode = p->getCrDom()->getRootNode();
    ldomNode * finalNode = NULL;
    for (; p; p = p->getParentNode())
    {
        int rm = p->getRendMethod();
        if (rm == erm_final || rm == erm_list_item)
        {
            finalNode = p; // found final block
        }
        else if (p->getRendMethod() == erm_invisible)
        {
            return p;
            //return false; // invisible !!!
        }
        if (p == mainNode)
        {
            break;
        }
    }
    return finalNode;
}

void RectHelper::InitFinalNode(ldomNode *finalNode)
{
    if (isInit)
    {
        Invalidate();
    }
    isInit = true;
    NodeIsInvisible_ = NodeIsInvisible(finalNode);

    finalNode_ = finalNode;
    if (finalNode == NULL)
    {
       #if DEBUG_GETRECT_LOGS
        CRLog::error("INITFINALNODE NULL");
       #endif
        return;
    }
    finalNode_->getAbsRect(absRect_);
    RenderRectAccessor r(finalNode_);
    finalNode_->renderFinalBlock(txtform_, &r, r.getWidth());
    #if DEBUG_GETRECT_LOGS
    //CRLog::error("txtform_->GetLineCount() = %d", txtform_->GetLineCount());
    //CRLog::error("finalnode = [%s]", LCSTR(finalNode_->getXPath()));
    //CRLog::error("finalnode childcount = %d", finalNode_->getChildCount());
    #endif
}

void RectHelper::InitNode(ldomNode *Node)
{
    // text node
    srcIndex_   = -1;
    srcLen_     = -1;
    lastIndex_  = -1;
    lastLen_    = -1;
    lastOffset_ = -1;

    if (finalNode_ == NULL || NodeIsInvisible_)
    {
        return;
    }

    int index = FindNodeIndex(Node, NodeIndex_);
    NodeIndex_ = index;

    int count = txtform_->GetSrcCount();
    if (index > 0)
    {
        const src_text_fragment_t *src = txtform_->GetSrcInfo(index);
        bool isObject = (src->flags & LTEXT_SRC_IS_OBJECT) != 0;
        srcIndex_ = index;
        srcLen_ = isObject ? 0 : src->t.len;

        lastIndex_ = index - 1;
        const src_text_fragment_t *src2 = txtform_->GetSrcInfo(lastIndex_);
        bool isObject2 = (src2->flags & LTEXT_SRC_IS_OBJECT) != 0;
        lastLen_ = isObject2 ? 0 : src2->t.len;
        lastOffset_ = isObject2 ? 0 : src2->t.offset;
    }
    else if (index == 0)
    {
        srcIndex_ = index;
        const src_text_fragment_t *src = txtform_->GetSrcInfo(index);
        bool isObject = (src->flags & LTEXT_SRC_IS_OBJECT) != 0;
        srcLen_ = isObject ? 0 : src->t.len;
    }
    else if (count > 0) // srcindex < 0
    {
        lastIndex_ = (FindLastIndexEnable_) ? FindLastIndex(Node) : count - 1;
        const src_text_fragment_t *src = txtform_->GetSrcInfo(lastIndex_);
        bool isObject = (src->flags & LTEXT_SRC_IS_OBJECT) != 0;
        srcIndex_ = lastIndex_;
        srcLen_ = isObject ? 0 : src->t.len;
        lastLen_ = srcLen_;
        lastOffset_ = isObject ? 0 : src->t.offset;
    }
    else
    {
        #if DEBUG_GETRECT_LOGS
        CRLog::error("GetSrcCount: Final node contains no text nodes!");
        #endif
    }
    LineIndex_ = FindLineIndex(LineIndex_);
    NodeLineIndex_ = LineIndex_ ;
}

bool RectHelper::ifnull(ldomXPointerEx xpointer, lvRect &rect)
{
    // no base final node, using blocks
    //lvRect rc;
    ldomNode *node = xpointer.getNode();
    //CRLog::error("NEW NODE = [%s]",LCSTR(node->getXPath()));
    //CRLog::error("Node Text = %s",LCSTR(node->getText()));

    int offset = xpointer.getOffset();
    if (offset < 0 || node->getChildCount() == 0)
    {
        CRLog::error("Ifnull 1");
        node->getAbsRect(rect);
        return true;
        //return rc.topLeft();
    }
    if (offset < node->getChildCount())
    {
        CRLog::error("Ifnull 2");
        node->getChildNode(offset)->getAbsRect(rect);
        return true;
        //return rc.topLeft();
    }
    CRLog::error("Ifnull 3");
    node->getChildNode(node->getChildCount() - 1)->getAbsRect(rect);
    return true;
    //return rc.bottomRight();
}

//get line where current node starts
int RectHelper::FindLineIndex(int start)
{
   #if DEBUG_GETRECT_LOGS
    CRLog::error("start = %d, linecount = %d", start, txtform_->GetLineCount());
   #endif
    int l = 0;
    int w = 0;
    bool found = false;
    for (l = start; l < txtform_->GetLineCount(); l++)
    {
        const formatted_line_t *frmline = txtform_->GetLineInfo(l);
        for (w = 0; w < (int) frmline->word_count; w++)
        {
            const formatted_word_t *word = &frmline->words[w];
            if (word->src_text_index == srcIndex_)
            {
                #if DEBUG_GETRECT_LOGS
                CRLog::error("Found Line Index1 = %d", l);
                #endif
                found = true;
                break;
            }
        }
        if (found)
        {
            break;
        }
    }
    if (found == false)
    {
        #if DEBUG_GETRECT_LOGS
        CRLog::error("NOT Found Line Index = 0");
        #endif
        return 0;
    }
    if (start > 0 && l == start)
    {
        for (l = start; l >= 0; l--)
        {
            const formatted_line_t *frmline = txtform_->GetLineInfo(l);
            w = (l == start) ? w - 1 : (int) frmline->word_count;
            for (; w >= 0; w--)
            {
                formatted_word_t frmword = frmline->words[w];
                if (frmword.src_text_index != srcIndex_)
                {
                    #if DEBUG_GETRECT_LOGS
                    CRLog::error("Found Line Index2 = %d", l);
                    #endif
                    return l;
                }
            }
        }
    }
    else
    {
        return l;
    }
    return 0;
}

int RectHelper::FindNodeIndex(ldomNode *node, int start)
{

    start = (start < 0) ? 0 : start;
    int count = txtform_->GetSrcCount();
    for (int i = start; i < count; i++)
    {
        const src_text_fragment_t *src = txtform_->GetSrcInfo(i);
        if (src->object == node)
        {
            return i;
        }
    }
    //RLog::error("start = %d, count = %d",start,count);
    #if DEBUG_GETRECT_LOGS
    CRLog::error("NOT FOUND NODE INDEX RETRYING");
    #endif
    //nodes in txtform appear to be able not to be in order,
    // so we retry search cycle from zero, to find node index again
    if (start > 0)
    {
        #if DEBUG_GETRECT_LOGS
        CRLog::error("start = %d, count = %d", start, count);
        #endif
        return FindNodeIndex(node, 0);
    }
    return -1;
}


int RectHelper::FindLastIndex(ldomNode *node)
{

    // ПЕРЕНЕСЕНО ИЗ СТАРОЙ РЕАЛИЗАЦИИ
    // Проблемный участок, от которого одновременно зависит
    // генерация оглавлений (Гаррисон), и генерация хитбоксов в некоторых случаях (Storia della mafia)
    // К функции GetRect добавлен параметр forlvpoint, который при вызове в генерации оглавлений, включает этот кусок.

    int count = txtform_->GetSrcCount();
    if (count <= 0)
    {
        return -1;
    }
    ldomXPointerEx xp1(node, 0);
    for (int i = 0; i < count; i++)
    {
        const src_text_fragment_t *src = txtform_->GetSrcInfo(i);
        bool isObject = (src->flags & LTEXT_SRC_IS_OBJECT) != 0;
        int offset = isObject ? 0 : src->t.offset;

        ldomXPointerEx xp2((ldomNode *) src->object, offset);
        if (xp2.compare(xp1) > 0)
        {
            //CRLog::error("FindLastIndex = %d",i);
            return i;
        }
    }
    #if DEBUG_GETRECT_LOGS
    CRLog::error("FindLastIndex Not found = %d", count - 1);
    #endif

    return count - 1;
}
void RectHelper::ResetLineIndex()
{
    LineIndex_ = NodeLineIndex_;
}

lvRect RectHelper::getRect(ldomWord word, bool init)
{
    if(init)
    {
        Init(word.getNode());
    }
    return getRect(word);
}

lvRect RectHelper::getRect(ldomXPointer xPointer)
{
    Init(xPointer.getNode());
    lvRect rect;
    processRect(xPointer,rect);
    return rect;
}

lvRect RectHelper::getRect(ldomWord word)
{
    #if DEBUG_GETRECT_LOGS
    CRLog::trace("GetRect START");
    #endif
    lvRect rect;
    if (word.isNull())
    {
        #if DEBUG_GETRECT_LOGS
        CRLog::error("word is null STOP");
        #endif
        return rect;
    }
    // get start and end rects
    lvRect rc1;
    lvRect rc1old;
    lvRect rc2;
    lvRect rc2old;
    ldomXPointerEx xp1 = word.getStartXPointer();
    ldomXPointerEx xp2 = word.getEndXPointer();
    #if DEBUG_GETRECT_LOGS
    CRLog::error("Nodetext = [%s] , LETTER = [%s]", LCSTR(word.getNode()->getText()), LCSTR(word.getText()));
    #endif

    //bool a1 = false;
    //bool a2 = false;
    //a1 = this->processRect(xp1, rc1);
    //a2 = this->processRect(xp2, rc2);
    //xp1.getRect(rc1old,false);
    //xp2.getRect(rc2old,false);

    #if DEBUG_GETRECT_LOGS
    CRLog::trace("GetRect STOP1");
    #endif
    //if(rc1 != rc1old)
    //{
    //    CRLog::warn("RC1 New rect != old rect! ^ [%d:%d][%d:%d] != [%d:%d][%d:%d]",rc1.left,rc1.right,rc1.top,rc1.bottom,rc1old.left,rc1old.right,rc1old.top,rc1old.bottom);
    //}
    //if (rc2 != rc2old)
    //{
    //    CRLog::warn("RC2 New rect != old rect! ^ [%d:%d][%d:%d] != [%d:%d][%d:%d]",rc2.left,rc2.right,rc2.top,rc2.bottom,rc2old.left,rc2old.right,rc2old.top,rc2old.bottom);
    //}

    //if (!xp1.getRect(rc1,false) || !xp2.getRect(rc2,false)) //OLD OLD
    //if (!xp1.getRect(rc1,false) || !this->processRect(xp2, rc2)) //OLD NEW
    if (!this->processRect(xp1, rc1) || !this->processRect(xp2, rc2)) //NEW NEW
    //if (!a1 || !a2)
    {
        return rect;
    }
    if (rc1.top == rc2.top && rc1.bottom == rc2.bottom)
    {
        // on same line
        rect.left = rc1.left;
        rect.top = rc1.top;
        rect.right = rc2.right;
        rect.bottom = rc2.bottom;
        #if DEBUG_GETRECT_LOGS
        CRLog::trace("GetRect STOP2");
        #endif
        return rect;
    }
    // on different lines
    ldomXRange range(xp1, xp2);
    ldomNode *parent = range.getNearestCommonParent();
    if (!parent)
    {
        #if DEBUG_GETRECT_LOGS
        CRLog::trace("GetRect STOP3");
        #endif
        return rect;
    }
    parent->getAbsRect(rect);
    rect.top = rc1.top;
    rect.bottom = rc2.bottom;
    rect.left = rc1.left < rc2.left ? rc1.left : rc2.left;
    rect.right = rc1.right > rc2.right ? rc1.right : rc2.right;
    return rect;

}

bool RectHelper::processRect(ldomXPointerEx xpointer, lvRect &rect)
{

    if (NodeIsInvisible_)
    {
        return false;
    }
    //return xpointer.getRect(rect, false);

    if (finalNode_ == NULL)
    {
        bool res = ifnull(xpointer, rect);
        #if DEBUG_GETRECT_LOGS
        CRLog::error("Rect ifnull = [%d:%d][%d:%d]", rect.left, rect.right, rect.top, rect.bottom);
        #endif
        return res;
    }

    lvRect rc;
    finalNode_->getAbsRect(rc);
    //CRLog::error("absrect = [%d:%d][%d:%d]",rc.left,rc.right,rc.top,rc.bottom);
    if (rc.height() == 0 && rc.width() > 0)
    {
        rect = rc;
        rect.bottom++;
        #if DEBUG_GETRECT_LOGS
        CRLog::error("1");
        #endif
        return true;
    }
    if (NodeIndex_ < 0 && lastIndex_ < 0)
    {
        //CRLog::error("LastIndex < 0");
        return false;
    }
    ldomNode *node = xpointer.getNode();
    #if DEBUG_GETRECT_LOGS
    CRLog::error("NEW FINAL NODE = [%s]", LCSTR(finalNode_->getXPath()));
    // CRLog::error("FINAL NODE TEXT = [%s]",LCSTR(finalNode_->getText()));
    // CRLog::error("NODE      = [%s]",LCSTR(Node_->getXPath()));
    // CRLog::error("NODE TEXT = [%s]",LCSTR(Node_->getText()));
    #endif
    int offset = xpointer.getOffset();
    //CRLog::error("NEW offset = %d",offset);
    //CRLog::error("NEW lastoffset = %d",lastOffset_);
    if (NodeIndex_ < 0)
    {
        #if DEBUG_GETRECT_LOGS
        CRLog::error("NEW replacing offset with last offset");
        #endif
        offset = lastOffset_;
    }
    #if DEBUG_GETRECT_LOGS
    CRLog::error("srcIndex = %d srcLen = %d lastIndex = %d lastLen = %d lastOffset = %d nodeIndex =%d ", srcIndex_, srcLen_, lastIndex_, lastLen_, lastOffset_, NodeIndex_);
    #endif
    css_style_rec_t * style = finalNode_->getStyle().get();
    int padding_top = 0;
    bool isObject = (srcLen_ == 0);
    if(style && style->padding->type == css_val_px && !isObject)
    {
        padding_top = style->padding[2].value;
    }

    int count = txtform_->GetLineCount();
    int start = LineIndex_;
    for (int l = start; l < count; l++)
    {
        const formatted_line_t *frmline = txtform_->GetLineInfo(l);
        if(padding_top>0)
        {
            padding_top = frmline->baseline;
        }
        for (int w = 0; w < (int) frmline->word_count; w++)
        {
            const formatted_word_t *word = &frmline->words[w];
            bool lastWord = (l == txtform_->GetLineCount() - 1 && w == frmline->word_count - 1);
            if (word->src_text_index >= srcIndex_ || lastWord)
            {
                #if DEBUG_GETRECT_LOGS
                CRLog::error("l = %d, w = %d lastword = %d line_index = %d", l, w, (lastWord) ? 1 : 0, LineIndex_);
                //CRLog::error("word->src_text_index > srcIndex || offset <= word->t.start");
                //CRLog::error("%d>%d || %d <= %d",word->src_text_index,srcIndex_,offset,word->t.start);
                //CRLog::error("(offset < word->t.start + word->t.len) || (offset == srcLen && offset == word->t.start + word->t.len)");
                //CRLog::error("(%d < %d + %d) || (%d == %d && %d == %d + %d)",offset, word->t.start , word->t.len,offset , srcLen_ ,offset , word->t.start , word->t.len);
                #endif
                // found word from same src line
                if (word->src_text_index > srcIndex_ || offset <= word->t.start)
                {

                    // before this word
                    rect.left = word->x + rc.left + frmline->x;
                    //rect.top = word->y + rc.top + frmline->y + frmline->baseline;
                    rect.top = rc.top + frmline->y + padding_top;
                    rect.right = rect.left + 1;
                    rect.bottom = rect.top + frmline->height;
                    LineIndex_ = l;
                    #if DEBUG_GETRECT_LOGS
                    CRLog::error("word->x = %d, rc.left = %d, frmline->x = %d", word->x, rc.left, frmline->x);
                    CRLog::error("Rect1 = [%d:%d][%d:%d]", rect.left, rect.right, rect.top, rect.bottom);
                    //CRLog::error("LINEINDEX END = %d",LineIndex_);
                    #endif
                    return true;
                }
                else if ((offset < word->t.start + word->t.len) || (offset == srcLen_ && offset == word->t.start + word->t.len))
                {
                    // pointer inside this word
                    LVFont *font = (LVFont *) txtform_->GetSrcInfo(srcIndex_)->t.font;
                    lUInt16 widths[512];
                    lUInt8 flg[512];
                    lString16 str = node->getText();
                    font->measureText(str.c_str() + word->t.start, offset - word->t.start, widths, flg, word->width + 50, '?', txtform_->GetSrcInfo(srcIndex_)->letter_spacing);
                    int chx = widths[offset - word->t.start - 1];
                    rect.left = word->x + rc.left + frmline->x + chx;
                    //rect.top = word->y + rc.top + frmline->y + frmline->baseline;
                    rect.top = rc.top + frmline->y + padding_top;
                    rect.right = rect.left + 1;
                    rect.bottom = rect.top + frmline->height;
                    LineIndex_ = l;
                    #if DEBUG_GETRECT_LOGS
                    CRLog::error("Rect2 = [%d:%d][%d:%d]", rect.left, rect.right, rect.top, rect.bottom);
                    //CRLog::error("LINEINDEX END = %d",LineIndex_);
                    #endif


                    return true;
                }
                else if (lastWord)
                {
                    // after last word
                    rect.left = word->x + rc.left + frmline->x + word->width;
                    //rect.top = word->y + rc.top + frmline->y + frmline->baseline;
                    rect.top = rc.top + frmline->y + padding_top;
                    rect.right = rect.left + 1;
                    rect.bottom = rect.top + frmline->height;
                    LineIndex_ = l;
                    #if DEBUG_GETRECT_LOGS
                    CRLog::error("Rect3 = [%d:%d][%d:%d]", rect.left, rect.right, rect.top, rect.bottom);
                    //CRLog::error("LINEINDEX END = %d",LineIndex_);
                    #endif
                    return true;
                }
            }
        }
    }
    #if DEBUG_GETRECT_LOGS
    //CRLog::error("new getrect return false");
    #endif
    return false;
}