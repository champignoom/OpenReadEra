#ifndef HIST_H_INCLUDED
#define HIST_H_INCLUDED

#include "lvptrvec.h"

enum bmk_type {
    bmkt_lastpos,
    bmkt_pos,
    bmkt_comment,
    bmkt_correction
};

class CRBookmark {
private:
    lString16 _startpos;
    lString16 _endpos;
    double _ratio;
    int _type;
    lString16 _postext;
    lString16 _titletext;
public:
	static lString16 getChapterName(ldomXPointer p);

    // fake bookmark for range
    CRBookmark(lString16 startPos,lString16 endPos)
			: _startpos(startPos)
			, _endpos(endPos)
			, _ratio(0)
			, _type(0)
			, _postext(lString16::empty_str)
			, _titletext(lString16::empty_str)
    {}

    CRBookmark(const CRBookmark & v )
			: _startpos(v._startpos)
			, _endpos(v._endpos)
			, _ratio(v._ratio)
			, _type(v._type)
			, _postext(v._postext)
			, _titletext(v._titletext)
    {}

    CRBookmark & operator = (const CRBookmark & v )
    {
        _startpos = v._startpos;
        _endpos = v._endpos;
        _ratio = v._ratio;
        _type = v._type;
        _postext = v._postext;
        _titletext = v._titletext;
        return *this;
    }

    CRBookmark()
    		: _ratio(0)
    		, _type(0)
    {}

    CRBookmark (ldomXPointer ptr);

    lString16 getStartPos() { return _startpos; }
    lString16 getEndPos() { return _endpos; }
    lString16 getPosText() { return _postext; }
    lString16 getTitleText() { return _titletext; }
    int getType() { return _type; }
    double getRatio() { return _ratio; }
    void setStartPos(const lString16 & s ) { _startpos = s; }
    void setEndPos(const lString16 & s ) { _endpos = s; }
    void setPosText(const lString16 & s ) { _postext= s; }
    void setTitleText(const lString16 & s ) { _titletext = s; }
    void setType( int n ) { _type = n; }
    void setRatio(double n) {_ratio = n;}
};

#endif //HIST_H_INCLUDED
