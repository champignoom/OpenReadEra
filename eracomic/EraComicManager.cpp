/*
 * Copyright (C) 2013-2021 READERA LLC
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
 * Developers: ReadEra Team (2013-2021), Playful Curiosity (2013-2021),
 * Tarasus (2018-2021).
 */
//
// Created by Tarasus on 4/1/2021.
//

#include "EraComicManager.h"
#include "../orebridge/include/ore_log.h"
#include "../jpeg-turbo/turbojpeg.h"
#include "../orelibpng/png.h"
#include "../easybmp/EasyBMP.h"
#include "../libwebp/src/webp/decode.h"

// DCT filter example. Produces a negative of the image. */
int inverseFilter(short *coeffs, tjregion arrayRegion, tjregion planeRegion, int componentIndex, int transformIndex, tjtransform *transform)
{
    int i;
    for (i = 0; i < arrayRegion.w * arrayRegion.h; i++)
        coeffs[i] = -coeffs[i];
    return 0;
}

void toLowercase( std::string &str)
{
    for (char & ch : str) {ch = std::tolower(ch);}
}

imgFormat ComicManager::validateName(std::string name)
{
    int dot = name.find_last_of('.');
    if(dot == std::string::npos)
    {
        return FORMAT_UNKNOWN;
    }
    std::string format = name.substr(dot + 1);
    if (format == "jpg" || format == "jpeg" || format == "JPG" || format == "JPEG")
    {
        return FORMAT_JPG;
    }
    else if (format == "png" || format == "PNG")
    {
        return FORMAT_PNG;
    }
    else if (format == "bmp" || format == "BMP")
    {
        return FORMAT_BMP;
    }
    else if (format == "webp" || format == "WEBP")
    {
        return FORMAT_WEBP;
    }
    return FORMAT_UNKNOWN;
}

bool ComicManager::freeAllPages()
{
    for (int page_index = 0; page_index < pagecount; page_index++)
    {
        auto it = indexmap.find(page_index);
        if( it == indexmap.end())
        {
            continue;
        }
        freeFile(it->second);
    }
    return true;
}

bool ComicManager::freeFile(uint32_t index)
{
    if(files.at(index)!=NULL)
    {
        files.at(index)->free_page_buf();
        files.at(index) = NULL;
        return true;
    }
    return false;
}

ComicFile *ComicManager::getPage(uint32_t raw_index)
{
    auto it = indexmap.find(raw_index);
    if ( it == indexmap.end()) {
        LE("No page %d found", raw_index);
        return NULL;
    }
    int file_index = it->second;

    ComicFile * file = files.at(file_index);
    if(file == NULL)
    {
        //LE("file = NULL, redecoding");
        file = loadFile(file_index);
    }
    return file;
}

imgFormat ComicManager::checkImageFormat(unsigned char* buf)
{
    const unsigned char jpg[3]        = {0xFF,0xD8,0xFF};
    const unsigned char png[8]        = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};
    const unsigned char bmp[2]        = {0x42,0x4D};
    const unsigned char webp_start[4] = {0x52,0x49,0x46,0x46};
    const unsigned char webp_end[4]   = {0x57,0x45,0x42,0x50};

    if(strncmp((char*)buf,(char*)jpg,3) == 0) return FORMAT_JPG;
    if(strncmp((char*)buf,(char*)png,8) == 0) return FORMAT_PNG;
    if(strncmp((char*)buf,(char*)bmp,2) == 0) return FORMAT_BMP;
    if(strncmp((char*)buf,(char*)webp_start,4) == 0)
    {
        if(strncmp((char*)buf+8,(char*)webp_end,4) == 0)
            return FORMAT_WEBP;
    }

    char log[100];
    sprintf(log,"0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
    LE("unknown image signature: [%s]",log);
    return FORMAT_UNKNOWN;
}

bool ComicManager::getImageInfo(std::string filename, unsigned char *inBuf, unsigned long inSize, int *w, int *h)
{
    imgFormat bin_format  = ComicManager::checkImageFormat(inBuf);

#ifdef OREDEBUG
    imgFormat name_format = ComicManager::validateName(filename);
    if(name_format!=bin_format)
    {
        LE("getImageInfo: %s name_format is %d but file signature tells %d",filename.c_str(),name_format,bin_format);
    }
#endif

    if(bin_format == FORMAT_UNKNOWN)
    {
        return false;
    }

    bool result = false;
    switch (bin_format)
    {
        case FORMAT_JPG:
            result = getJpgInfo(inBuf, inSize, w, h);
            break;
        case FORMAT_PNG:
            result = getPngInfo(inBuf, inSize, w, h);
            break;
        case FORMAT_BMP:
            result = getBmpInfo(inBuf, inSize, w, h);
            break;
        case FORMAT_WEBP:
            result = getWebpInfo(inBuf, inSize, w, h);
            break;
        case FORMAT_UNKNOWN:
        default:
            LE("UNKNOWN FORMAT %d",(int)bin_format);
    }
    if(!result)
    {
        LE("failed getting info for file [%s]",filename.c_str());
    }
    return result;
}

bool ComicManager::getJpgInfo(unsigned char* inBuf, unsigned long inSize, int* w, int *h)
{
    tjhandle tjInstance = NULL;

    if ((tjInstance = tjInitDecompress()) == NULL)
    {
        LE("error initializing decompressor");
        return false;
    }

    int inSubsamp;
    int inColorspace;

    if (tjDecompressHeader3_imp(tjInstance, inBuf, inSize, w, h, &inSubsamp, &inColorspace, 0) < 0)
    {
        LE("Error reading JPEG header at size %lu [%s]",inSize ,tjGetErrorStr());
        if (*w < 1 || *h < 1)
        {
            tjDestroy(tjInstance);
            tjInstance = NULL;
            return false;
        }
        //But we can deal with it because we got needed width and height anyway
    }
    tjDestroy(tjInstance);
    tjInstance = NULL;
    return true;
}

bool ComicManager::getPngInfo(unsigned char* inBuf, unsigned long inSize, int* w, int *h)
{
    png_image image;
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;

    if (png_image_begin_read_from_memory(&image,inBuf,inSize))
    {
        image.format = PNG_FORMAT_RGBA;
        *w = image.width;
        *h = image.height;

        png_image_free(&image);
        return true;
    }
    LE("getPngInfo: of size [%lu] error: %s",inSize, image.message);
    return false;
}

bool ComicManager::getBmpInfo(unsigned char* inBuf, unsigned long inSize, int* w, int *h)
{
    BMP bmp;
    if(!bmp.GetBmpInfoFromStream(inBuf,inSize,w,h))
    {
        LE("getBmpInfo failed for size [%lu]",inSize);
        return false;
    }
    return true;
}

bool ComicManager::getWebpInfo(unsigned char* inBuf, unsigned long inSize, int* w, int *h)
{
    return WebPGetInfo(inBuf,inSize,w,h);
}

bool ComicManager::decodeImageBuf(unsigned char *inBuf, unsigned long inSize, ComicFile *comicFile)
{
    //LE("invert = %d ,decode file %s",this->config_invert_images,comicFile->name.c_str());
    switch (comicFile->format)
    {
        case FORMAT_JPG: return decodeJpgBuf(inBuf, inSize, comicFile);
        case FORMAT_PNG: return decodePngBuf(inBuf, inSize, comicFile);
        case FORMAT_BMP: return decodeBmpBuf(inBuf, inSize, comicFile);
        case FORMAT_WEBP: return decodeWebpBuf(inBuf, inSize, comicFile);
        case FORMAT_UNKNOWN:
        default:
            LE("UNKNOWN FORMAT %d",(int)comicFile->format);
            return false;
    }
}

bool ComicManager::decodeJpgBuf(unsigned char* inBuf, unsigned long inSize, ComicFile * comicFile)
{
    tjhandle tjInstance = NULL;

    tjtransform xform;
    memset(&xform, 0, sizeof(tjtransform));

    if(this->config_invert_images)
    {
        xform.customFilter = inverseFilter;
        auto dstBuf = (unsigned char *) malloc(inSize);
        unsigned long dstSize = 0;

        if ((tjInstance = tjInitTransform()) == NULL)
        {
            LE("error initializing transformer");
            return false;
        }
        xform.options |= TJXOPT_TRIM;
        if (tjTransform(tjInstance, inBuf, inSize, 1, &dstBuf, &dstSize, &xform, 0) < 0)
        {
            LE("error transforming input image: %s",tjGetErrorStr());
            return false;
        }
        tjFree(inBuf);
        inBuf = dstBuf;
        inSize = dstSize;
    }
    else
    {
        if ((tjInstance = tjInitDecompress()) == NULL)
        {
            LE("error initializing decompressor");
            return false;
        }
    }


    int orig_width = 0;
    int orig_height = 0;
    int inSubsamp = 0;
    int inColorspace = 0;

    if (tjDecompressHeader3(tjInstance, inBuf, inSize, &orig_width, &orig_height, &inSubsamp, &inColorspace) < 0)
    {
        LE("Error reading JPEG header %s", tjGetErrorStr());
        if( orig_width < 1 || orig_height < 1 )
        {
            LE("Retried, but failed!");
            tjDestroy(tjInstance);
            tjInstance = NULL;
            return false;
        }
        LE("But we can deal with it");
    }

    //LE("Input Image:  %d x %d pixels, %s subsampling", orig_width, orig_height, subsampName[inSubsamp]);

    int targetSize = orig_width * orig_height * tjPixelSize[TJPF_RGBA];

    if(!comicFile->alloc_page_buf(targetSize))
    {
        LE("Error allocating uncompressed image buffer");
        tjDestroy(tjInstance);
        tjInstance = NULL;
        return false;
    }
    if (tjDecompress2(tjInstance, inBuf, inSize, comicFile->page_buf, orig_width, 0, orig_height, TJPF_RGBA, 0) < 0)
    {
        LE("Error decompressing JPEG image %s", tjGetErrorStr());
        tjDestroy(tjInstance);
        tjInstance = NULL;
        return false;
    }
    comicFile->orig_width = orig_width;
    comicFile->orig_height = orig_height;

    tjDestroy(tjInstance);
    tjInstance = NULL;
    return true;
}

bool ComicManager::decodePngBuf(unsigned char* inBuf, unsigned long inSize, ComicFile * comicFile)
{
    png_image image;
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;

    if (png_image_begin_read_from_memory(&image, inBuf,inSize))
    {
        image.format = PNG_FORMAT_RGBA;
        if (!comicFile->alloc_page_buf(PNG_IMAGE_SIZE(image)))
        {
            return false;
        }

        if (png_image_finish_read(&image, NULL, comicFile->page_buf, 0, NULL))
        {
            comicFile->orig_width = image.width;
            comicFile->orig_height = image.height;
        }
        const int RGBA_NUM = 4;
        const int size = image.width * image.height * RGBA_NUM;

        png_image_free(&image);

        //invert images for nightmode
        if(this->config_invert_images)
        {
            for (int i = 0; i < size; i++)
            {
                comicFile->page_buf[i] = 0xFF - comicFile->page_buf[i];
            }
        }
        return true;
    }
    LE("decodePngBuf: error: %s\n", image.message);
    return false;
}

bool ComicManager::decodeBmpBuf(unsigned char* inBuf, unsigned long inSize, ComicFile * comicFile)
{
    BMP bmp;
    if(!bmp.ReadFromBuffer(inBuf,inSize))
    {
        LE("decodeBmpBuf failed reading file %s",comicFile->name.c_str());
        return false;
    }
    const int w = bmp.TellWidth();
    const int h = bmp.TellHeight();
    const int RGBA_NUM = 4;
    const int size = w * h * RGBA_NUM;
    comicFile->orig_width  = w;
    comicFile->orig_height = h;

    if (!comicFile->alloc_page_buf(size))
    {
        LE("decodeBmpBuf failed allocating %d bytes",size);
        return false;
    }
    if (!bmp.WriteToBuffer(comicFile->page_buf,size))
    {
        LE("decodeBmpBuf failed copying %d bytes to page buffer",size);
        return false;
    }
    //invert images for nightmode
    if(this->config_invert_images)
    {
        for (int i = 0; i < size; i++)
        {
            comicFile->page_buf[i] = 0xFF - comicFile->page_buf[i];
        }
    }
    return true;
}

bool ComicManager::decodeWebpBuf(unsigned char* inBuf, unsigned long inSize, ComicFile * comicFile)
{
    int w = 0;
    int h = 0;
    if(!WebPGetInfo(inBuf,inSize,&w,&h))
    {
        LE("Unable to get webp dimensions for [%s]",comicFile->name.c_str());
        return false;
    }

    const int RGBA_NUM = 4;
    const int size = w * h * RGBA_NUM;

    if (!comicFile->alloc_page_buf(size))
    {
        LE("decodeWebpBuf failed allocating %d bytes",size);
        return false;
    }

    comicFile->page_buf = WebPDecodeRGBA(inBuf, inSize, &w, &h);
    comicFile->orig_width  = w;
    comicFile->orig_height = h;
    //invert images for nightmode
    if(this->config_invert_images)
    {
        for (int i = 0; i < size; i++)
        {
            comicFile->page_buf[i] = 0xFF - comicFile->page_buf[i];
        }
    }
    return true;
}


std::string getDigitsPrefix(const std::string &in_string)
{
    std::locale loc;
    std::string res;
    for (char i : in_string)
    {
        if(!std::isdigit(i,loc))
        {
            break;
        }
        res += i;
    }
    return res;
}

std::string getDigitsSuffix(const std::string &in_string)
{
    std::locale loc;
    std::string res;
    std::string::size_type i=0;
    for (i = in_string.length()-1; i >=0 ; i--)
    {
        char ch = in_string[i];
        if(!std::isdigit(ch,loc))
        {
            break;
        }
        res += ch;
    }
    std::reverse(res.begin(),res.end());
    return res;
}

bool endsWith(const std::string& str, const std::string& suffix)
{
    return ((str.size() >= suffix.size()) && (0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix)));
}

bool startsWith(const std::string& str, const std::string& prefix)
{
    return ((str.size() >= prefix.size()) && (0 == str.compare(0, prefix.size(), prefix)));
}

std::string stripPrefix(const std::string &s, const std::string& prefix)
{
    if (s.empty() || prefix.empty()) {
        return s;
    }
    if (startsWith(s, prefix)) {
        return s.substr(prefix.length(),s.length()-prefix.length());
    }
    return s;
}

std::string stripSuffix(const std::string &s, const std::string& suffix)
{
    if (s.empty() || suffix.empty()) {
        return s;
    }
    if (endsWith(s,suffix)) {
        return s.substr(0, s.length() - suffix.length());
    }
    return s;
}

int compareNullGreater(long x, long y)
{
    // NULL, NOT ZERO.
    //if (x == 0 && y == 0) return 0;
    //if (x == 0)           return 1;
    //if (y == 0)           return -1;
    if (x == y)           return 0;
    if (x <  y)           return -1;
    return 1;
}

int compareNullLesser(long x, long y)
{
    // NULL, NOT ZERO.
    //if (x == 0 && y == 0) return 0;
    //if (x == 0)           return -1;
    //if (y == 0)           return 1;
    if (x == y)           return 0;
    if (x <  y)           return -1;
    return 1;
}

bool StrComparator(std::pair<std::string, int> &x, std::pair<std::string, int> &y)
{
    if (x.first.empty() && y.first.empty())
    {
        return true;
    }
    if (x.first.empty())
    {
        return true;
    }
    if (y.first.empty())
    {
        return false;
    }
    int dotx = x.first.find_last_of('.');
    if(dotx == std::string::npos)
    {
        return true;
    }
    int doty = y.first.find_last_of('.');
    if(doty == std::string::npos)
    {
        return false;
    }

    std::string stripped_x = x.first.substr(0,dotx);
    std::string stripped_y = y.first.substr(0,doty);
    //LE("StrComparator \nx = [%s], \ny = [%s]",stripped_x.c_str(),stripped_y.c_str());

    toLowercase(stripped_x);
    toLowercase(stripped_y);

    //LE("Lowercase \nx = [%s], \ny = [%s]",stripped_x.c_str(),stripped_y.c_str());

    std::string prefixStringX = getDigitsPrefix(stripped_x);
    std::string prefixStringY = getDigitsPrefix(stripped_y);

    //LE("StrComparator prefix x = [%s], prefix y = [%s]",prefixStringX.c_str(),prefixStringY.c_str());

    long prefixX = (prefixStringX.empty())? 0 : std::strtol(prefixStringX.c_str(), NULL, 10);
    long prefixY = (prefixStringY.empty())? 0 : std::strtol(prefixStringY.c_str(), NULL, 10);

    int comparePrefix = compareNullGreater(prefixX,prefixY);
    if (comparePrefix != 0)
    {
        //LW("StrComparator prefix compare %ld to %ld = (%d)",prefixX,prefixY,(comparePrefix == -1));
        return (comparePrefix < 0);
    }

    stripped_x = stripPrefix(stripped_x, prefixStringX);
    stripped_y = stripPrefix(stripped_y, prefixStringY);

    //std::string suffixStringX = getDigitsSuffix(stripped_x);
    //std::string suffixStringY = getDigitsSuffix(stripped_y);
    //LE("StrComparator suffix x = [%s], suffix y = [%s]", suffixStringX.c_str(),suffixStringY.c_str());

    //Cut (from left) all identical characters until we reach digits.
    //Then collect all next digits, and compare them.
    //If they are not equal - return their comparison.
    //Else cut the digits, and continue comparing chars.
    if(stripped_x != stripped_y)
    {
        char a = stripped_x[0];
        char b = stripped_y[0];
        while (a == b)
        {
            if(stripped_x.length() == 0)
            {
                return true;
            }
            else if (stripped_y.length() == 0)
            {
                return false;
            }
            stripped_x= stripped_x.substr(1,stripped_x.length()-1);
            stripped_y= stripped_y.substr(1,stripped_y.length()-1);
            a = stripped_x[0];
            b = stripped_y[0];
            if(isdigit(a) && isdigit(b) )
            {
                std::string a_str;
                std::string b_str;
                for (char ch : stripped_x)
                {
                    if(!isdigit(ch))
                        break;
                    a_str+=ch;
                }
                for (char ch : stripped_y)
                {
                    if (!isdigit(ch))
                        break;
                    b_str += ch;
                }
                int a_digital = atoi(a_str.c_str());
                int b_digital = atoi(b_str.c_str());
                if(a_digital != b_digital)
                {
                    //LW("StrComparator compare digital 1 a = [%d], b = [%d]",a_digital,b_digital);
                    //LW("    StrComparator compare digital 1 a_str = [%s], b_str = [%s]",a_str.c_str(),b_str.c_str());
                    return a_digital < b_digital;
                }
                else
                {
                    //LW("StrComparator compare digital 2 a == b == [%d]",a_digital);
                    //LW("    StrComparator compare digital 2 a_str = [%s], b_str = [%s]",a_str.c_str(),b_str.c_str());
                    stripped_x = stripped_x.substr(a_str.length(),stripped_x.length() - a_str.length() );
                    stripped_y = stripped_y.substr(b_str.length(),stripped_y.length() - b_str.length() );
                }
            }
        }
    }
    //LW("StrComparator compare middle \nx = [%s], \ny = [%s]",stripped_x.c_str(),stripped_y.c_str());
    int compareMiddle = stripped_x.compare(stripped_y);
    if (compareMiddle != 0)
    {
        //LW("StrComparator sort middle \nx = [%s], \ny = [%s] = (%d)",stripped_x.c_str(),stripped_y.c_str(),compareMiddle);
        return (compareMiddle < 0);
    }
    //LW("StrComparator middles SAME (return false)");
    return false;
}