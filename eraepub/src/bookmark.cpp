#include "../include/lvtinydom.h"
#include "../include/bookmark.h"

lString16 CRBookmark::getChapterName(ldomXPointer ptr)
{
	lString16 chapter;
	int lastLevel = -1;
	bool foundAnySection = false;
    lUInt16 section_id = ptr.getNode()->getCrDom()->getElementNameIndex(L"section");
	if (!ptr.isNull())
	{
		ldomXPointerEx p(ptr);
		p.nextText();
		while (!p.isNull()) {
			if (!p.prevElement())
				break;
            bool foundSection = p.findElementInPath( section_id ) > 0;
            //(p.toString().pos("section") >=0 );
            foundAnySection = foundAnySection || foundSection;
            if (!foundSection && foundAnySection)
                continue;
			lString16 nname = p.getNode()->getNodeName();
            if (!nname.compare("title") || !nname.compare("h1") || !nname.compare("h2")  || !nname.compare("h3")) {
				if ( lastLevel!=-1 && p.getLevel()>=lastLevel )
					continue;
				lastLevel = p.getLevel();
				if (!chapter.empty())
                    chapter = " / " + chapter;
				chapter = p.getText(' ') + chapter;
				if (!p.parent())
					break;
			}
		}
	}
	return chapter;
}

CRBookmark::CRBookmark (ldomXPointer ptr)
: _startpos(lString16::empty_str)
, _endpos(lString16::empty_str)
, _ratio(0)
, _type(0)
, _postext(lString16::empty_str)
, _titletext(lString16::empty_str)
{
    if (ptr.isNull())
        return;
    lvPoint pt = ptr.toPoint();
    CrDom* doc = ptr.getNode()->getCrDom();
    int h = doc->getFullHeight();
    if (pt.y > 0 && h > 0) {
        if (pt.y < h) {
            _ratio = (double) pt.y / h;
        } else {
            _ratio = 1;
        }
    }
	setTitleText(CRBookmark::getChapterName(ptr));
    _startpos = ptr.toString();
    lvPoint endpt = pt;
    endpt.y += 100;
    ldomXPointer endptr = doc->createXPointer(endpt);
}
