//
// Created by Tarasus on 08.09.2020.
//
#include <sys/mman.h>
#include <sys/stat.h>

#include <eraepub/include/indic/devanagariManager.h>
#include <eraepub/include/indic/banglaManager.h>
#include <eraepub/include/indic/malayalamManager.h>
#include <eraepub/include/indic/kannadaManager.h>
#include <eraepub/include/indic/tamilManager.h>
#include <eraepub/include/indic/teluguManager.h>
#include <eraepub/include/indic/gujaratiManager.h>
#include <eraepub/include/indic/oriyaManager.h>

LigMapManager gligMapManager;
LigMap      gCurrentLigMap;
LigMapRev   gCurrentLigMapRev;
FastLigMap  gCurrentFastLigMap;

#define LANGUAGE_DEVANAGARI 1
#define LANGUAGE_BANGLA     2
#define LANGUAGE_KANNADA    3
#define LANGUAGE_MALAYALAM  4
#define LANGUAGE_TAMIL      5
#define LANGUAGE_TELUGU     6
#define LANGUAGE_GUJARATI   7
#define LANGUAGE_ORIYA      8


std::vector<std::string> parse(const std::string &s, char delimeter)
{
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0, pos = 0;
    while((pos = s.find(delimeter, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );
        output.push_back(substring);
        prev_pos = ++pos;
    }
    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word
    return output;
}


LigMap parseFontIndexes(lString16 path, int language)
{
    switch (language)
    {
        case LANGUAGE_DEVANAGARI: if (gDvngLigMapRev.empty())     gDvngLigMapRev     = DevanagariLigMapReversed(); break;
        case LANGUAGE_BANGLA:     if (gBanglaLigMapRev.empty())   gBanglaLigMapRev   = BanglaLigMapReversed();     break;
        case LANGUAGE_MALAYALAM:  if (gMalayLigMapRev.empty())    gMalayLigMapRev    = MalayLigMapReversed();      break;
        case LANGUAGE_KANNADA:    if (gKannadaLigMapRev.empty())  gKannadaLigMapRev  = KannadaLigMapReversed();    break;
        case LANGUAGE_TAMIL:      if (gTamilLigMapRev.empty())    gTamilLigMapRev    = TamilLigMapReversed();      break;
        case LANGUAGE_TELUGU:     if (gTeluguLigMapRev.empty())   gTeluguLigMapRev   = TeluguLigMapReversed();     break;
        case LANGUAGE_GUJARATI:   if (gGujaratiLigMapRev.empty()) gGujaratiLigMapRev = GujaratiLigMapReversed();   break;
        case LANGUAGE_ORIYA:      if (gOriyaLigMapRev.empty())    gOriyaLigMapRev    = OriyaLigMapReversed();      break;
        default: LE("WRONG LANGUAGE : %d",language);
            break;
    }

    int fdin;

    char *src;
    struct stat statbuf;
    size_t fsize = 0;
    fdin = open(LCSTR(path), O_RDONLY);

    std::vector<std::pair<char*,int>> lines;
    std::map<lChar16 , dvngLig> result_map;

    if ( fstat(fdin, &statbuf) < 0 )
    {
        LE("fstat error");
        return result_map;
    }
    fsize = statbuf.st_size;
    if ((src = (char*)mmap(0, fsize, PROT_READ, MAP_SHARED, fdin, 0)) == MAP_FAILED )
    {
        LE("mmap err");
        return result_map;
    }

    char* found = src;
    int pos = 0;
    int lpos = 0;
    while (found!=NULL)
    {
        found = strchr(src+lpos, '\n');
        if(found==NULL)
        {
            std::pair<char*,int> pairB = std::make_pair(src+lpos,fsize-lpos);
            lines.push_back(pairB);
            break;
        }
        pos = found - src;
        std::pair<char*,int> pairA = std::make_pair(src+lpos,pos-lpos);
        lines.push_back(pairA);
        //LE("pos = %d, lpos = %d, len = %d",pos,lpos, pos-lpos);
        lpos = pos + 1;
    }

    for (auto & i : lines)
    {
        std::string line = std::string(i.first,i.second);
        //LE("Line = [%s]",line.c_str());
        int comment = line.find("//");
        if(comment!=std::string::npos)
        {
            line = line.substr(0,comment);
            int lastspace = line.find_last_not_of(' ');
            if(lastspace!=std::string::npos)
            {
                line = line.substr(0,lastspace+1);
            }
        }
        std::vector<std::string> words = parse(line,' ');
        dvngLig lig;
        lig.len = words.size()-1;

        wchar_t charcode = 0;
        std::string item;
        for (int i = 0; i < words.size(); i++)
        {
            item = words.at(i);
            if(item.empty())
            {
                continue;
            }
            switch (i)
            {
                case 0:  charcode = std::stoul("0x" + item, nullptr, 16); break;
                case 1:  lig.glyphindex = std::atoi(item.c_str()); break;
                case 2:  lig.a = std::stoul("0x" + item, nullptr, 16); break;
                case 3:  lig.b = std::stoul("0x" + item, nullptr, 16); break;
                case 4:  lig.c = std::stoul("0x" + item, nullptr, 16); break;
                case 5:  lig.d = std::stoul("0x" + item, nullptr, 16); break;
                case 6:  lig.e = std::stoul("0x" + item, nullptr, 16); break;
                case 7:  lig.f = std::stoul("0x" + item, nullptr, 16); break;
                case 8:  lig.g = std::stoul("0x" + item, nullptr, 16); break;
                case 9:  lig.h = std::stoul("0x" + item, nullptr, 16); break;
                case 10: lig.i = std::stoul("0x" + item, nullptr, 16); break;
                case 11: lig.j = std::stoul("0x" + item, nullptr, 16); break;
                default: LE("something's wrong, i = %d",i); break;
            }
        }

        char hashstr[50];
        sprintf(hashstr, "%X%X%X%X%X%X%X%X%X%X", lig.a,lig.b,lig.c,lig.d,lig.e,lig.f, lig.g, lig.h, lig.i, lig.j);
        lig.key_hash = lString16(hashstr,strlen(hashstr)).getHash();

        lChar16 ch =0;
        switch (language)
        {
            case LANGUAGE_DEVANAGARI:
                ch = findDvngLigRev(lig);
                break;
            case LANGUAGE_BANGLA:
                ch = findBanglaLigRev(lig);
                break;
            case LANGUAGE_MALAYALAM:
                ch = findMalayLigRev(lig);
                break;
            case LANGUAGE_KANNADA:
                ch = findKannadaLigRev(lig);
                break;
            case LANGUAGE_TAMIL:
                ch = findTamilLigRev(lig);
                break;
            case LANGUAGE_TELUGU:
                ch = findTeluguLigRev(lig);
                break;
            case LANGUAGE_GUJARATI:
                ch = findGujaratiLigRev(lig);
                break;
            case LANGUAGE_ORIYA:
                ch = findOriyaLigRev(lig);
                break;
            default: LE("WRONG LANGUAGE : %d",language);
                break;
        }
        if(ch != 0)
        {
            result_map.insert(std::make_pair(ch, lig));
        }

    }
    munmap(src,fsize);

    /*
    //debug printout
    LE("parseFontIndexes debug printout");
    for (auto it = result_map.begin(); it != result_map.end() ; it++)
    {
        dvngLig lig = it->second;

        char buf[100];
        switch (lig.len)
        {
            case 0: break;
            case 1: sprintf(buf,"0x%04X",lig.a);break;
            case 2: sprintf(buf,"0x%04X 0x%04X",lig.a,lig.b);break;
            case 3: sprintf(buf,"0x%04X 0x%04X 0x%04X",lig.a,lig.b,lig.c);break;
            case 4: sprintf(buf,"0x%04X 0x%04X 0x%04X 0x%04X",lig.a,lig.b,lig.c,lig.d);break;
            case 5: sprintf(buf,"0x%04X 0x%04X 0x%04X 0x%04X 0x%04X",lig.a,lig.b,lig.c,lig.d,lig.e);break;
            case 6: sprintf(buf,"0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X",lig.a,lig.b,lig.c,lig.d,lig.e,lig.f);break;
            case 7: sprintf(buf,"0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X",lig.a,lig.b,lig.c,lig.d,lig.e,lig.f,lig.g);break;
            case 8: sprintf(buf,"0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X",lig.a,lig.b,lig.c,lig.d,lig.e,lig.f,lig.g,lig.h);break;
            case 9: sprintf(buf,"0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X ",lig.a,lig.b,lig.c,lig.d,lig.e,lig.f,lig.g,lig.h,lig.i);break;
           case 10: sprintf(buf,"0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X ",lig.a,lig.b,lig.c,lig.d,lig.e,lig.f,lig.g,lig.h,lig.i,lig.j);break;
        }
        LE("[0x%04X] [%d] %s",it->first,lig.glyphindex,buf);
        memset(buf,0,100);
    }
    */
    return result_map;
}


lString16 lString16::processIndicText()
{
    bool unused;
    return this->processIndicText(&unused);
}

lString16 lString16::processIndicText(bool *indic)
{
    if(!gDocumentINDIC)
    {
        bool ind_suspect = false;
        for (int i = 0; i < this->length(); i++)
        {
            if (this->at(i) > 0x08FF)
            {
                ind_suspect = true;
                break;
            }
        }
        if (!ind_suspect)
        {
            return *this;
        }
    }
    if(DVNG_DISPLAY_ENABLE     == 1 && gDocumentDvng     == 0) {this->CheckDvng();}
    if(BANGLA_DISPLAY_ENABLE   == 1 && gDocumentBangla   == 0) {this->CheckBangla();}
    if(MALAY_DISPLAY_ENABLE    == 1 && gDocumentMalay    == 0) {this->CheckMalay();}
    if(KANNADA_DISPLAY_ENABLE  == 1 && gDocumentKannada  == 0) {this->CheckKannada();}
    if(TAMIL_DISPLAY_ENABLE    == 1 && gDocumentTamil    == 0) {this->CheckTamil();}
    if(TELUGU_DISPLAY_ENABLE   == 1 && gDocumentTelugu   == 0) {this->CheckTelugu();}
    if(GUJARATI_DISPLAY_ENABLE == 1 && gDocumentGujarati == 0) {this->CheckGujarati();}
    if(ORIYA_DISPLAY_ENABLE    == 1 && gDocumentOriya    == 0) {this->CheckOriya();}

    if(gDocumentDvng     == 0 &&
       gDocumentBangla   == 0 &&
       gDocumentMalay    == 0 &&
       gDocumentKannada  == 0 &&
       gDocumentTamil    == 0 &&
       gDocumentTelugu   == 0 &&
       gDocumentGujarati == 0 &&
       gDocumentOriya    == 0)
    {
        return *this;
    }

    lString16 res(*this);

    if(gDocumentDvng == 1)
    {
        res = res.processDvngText();
        *indic = true;
    }
    if(gDocumentBangla == 1)
    {
        res = res.processBanglaText();
        *indic = true;
    }
    if(gDocumentMalay == 1)
    {
        res = res.processMalayText();
        *indic = true;
    }
    if(gDocumentKannada == 1)
    {
        res = res.processKannadaText();
        *indic = true;
    }
    if(gDocumentTamil == 1)
    {
        res = res.processTamilText();
        *indic = true;
    }
    if(gDocumentTelugu == 1)
    {
        res = res.processTeluguText();
        *indic = true;
    }
    if(gDocumentGujarati == 1)
    {
        res = res.processGujaratiText();
        *indic = true;
    }
    if(gDocumentOriya == 1)
    {
        res = res.processOriyaText();
        *indic = true;
    }
    return res;
}

lString16 lString16::restoreIndicText()
{
    lString16 res;
    lString16Collection words;
    words.parse(*this, ' ', false);
    for (int i = 0; i < words.length(); ++i)
    {
        lString16 curr = words.at(i);
        if (DVNG_DISPLAY_ENABLE && gDocumentDvng)
        {
            curr = restoreDvngWord(curr);
        }
        if (BANGLA_DISPLAY_ENABLE && gDocumentBangla)
        {
            curr = restoreBanglaWord(curr);
        }
        if (MALAY_DISPLAY_ENABLE && gDocumentMalay)
        {
            curr = restoreMalayWord(curr);
        }
        if (KANNADA_DISPLAY_ENABLE && gDocumentKannada)
        {
            curr = restoreKannadaWord(curr);
        }
        if (TAMIL_DISPLAY_ENABLE && gDocumentTamil)
        {
            curr = restoreTamilWord(curr);
        }
        if (TELUGU_DISPLAY_ENABLE && gDocumentTelugu)
        {
            curr = restoreTeluguWord(curr);
        }
        if (GUJARATI_DISPLAY_ENABLE && gDocumentGujarati)
        {
            curr = restoreGujaratiWord(curr);
        }
        if (ORIYA_DISPLAY_ENABLE && gDocumentOriya)
        {
            curr = restoreOriyaWord(curr);
        }
        res += curr + " ";
    }
    res = res.substr(0, res.length() - 1);
    return res;
}

void lString16::checkCJK()
{
    if(gDocumentCJK)
    {
        return;
    }
    for (int i = 0; i < this->length(); i++)
    {
        int ch = this->at(i);
        #if CJK_PATCH
        if(isCJKIdeograph(ch))
        {
            gDocumentCJK = 1;
            return;
        }
        #else
        if (ch < UNICODE_CJK_PUNCTUATION_BEGIN)
            continue;
        if (ch >= UNICODE_CJK_IDEOGRAPHS_BEGIN && ch <= UNICODE_CJK_IDEOGRAPHS_END)
        {
            gDocumentCJK = 1;
            return;
        }
        if (ch >= UNICODE_CJK_PUNCTUATION_BEGIN && ch <= UNICODE_CJK_PUNCTUATION_END)
        {
            gDocumentCJK = 1;
            return;
        }
        if (ch >= UNICODE_CJK_PUNCTUATION_HALF_AND_FULL_WIDTH_BEGIN && ch <= UNICODE_CJK_PUNCTUATION_HALF_AND_FULL_WIDTH_END)
        {
            gDocumentCJK = 1;
            return;
        }
        #endif //CJK_PATCH
    }
}