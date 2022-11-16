/** \file lvimg.h
    \brief Image formats support

    CoolReader Engine C-compatible API

    (c) Vadim Lopatin, 2000-2006
    This source code is distributed under the terms of
    GNU General Public License.

    See LICENSE file for details.

*/

#ifndef __LVIMG_H_INCLUDED__
#define __LVIMG_H_INCLUDED__

#include "lvref.h"
#include "lvstream.h"

// Max unpacked size of skin image to hold in cache unpacked
#define MAX_SKIN_IMAGE_CACHE_ITEM_UNPACKED_SIZE 80*80*4

class LVImageSource;
class ldomNode;
class LVColorDrawBuf;

/// image decoding callback interface
class LVImageDecoderCallback
{
public:
    virtual ~LVImageDecoderCallback();
    virtual void OnStartDecode( LVImageSource * obj ) = 0;
    virtual bool OnLineDecoded( LVImageSource * obj, int y, lUInt32 * data ) = 0;
    virtual void OnEndDecode( LVImageSource * obj, bool errors ) = 0;
};

struct CR9PatchInfo {
	lvRect frame;
	lvRect padding;
	/// caclulate dst and src rectangles (src rect includes 1 pixel layout frame)
	void calcRectangles(const lvRect & dst,
                        const lvRect & src,
                        lvRect dstitems[9],
                        lvRect srcitems[9]) const;
	/// for each side, apply max(padding.C, dstPadding.C) to dstPadding
	void applyPadding(lvRect & dstPadding) const;
};


class LVImageSource
{
	CR9PatchInfo * _ninePatch;
public:
	virtual const CR9PatchInfo * GetNinePatchInfo() { return _ninePatch; }
	virtual CR9PatchInfo *  DetectNinePatch();
    virtual ldomNode * GetSourceNode() = 0;
    virtual LVStream * GetSourceStream() = 0;
    virtual void   Compact() = 0;
    virtual int    GetWidth() = 0;
    virtual int    GetHeight() = 0;
    virtual bool   Decode( LVImageDecoderCallback * callback ) = 0;
    LVImageSource() : _ninePatch(NULL) {}
    virtual ~LVImageSource();
};

typedef LVRef<LVImageSource> LVImageSourceRef;

/// type of image transform
enum ImageTransform {
    IMG_TRANSFORM_NONE,    // just draw w/o any resizing/tiling
    IMG_TRANSFORM_SPLIT,   // split at specified pixel, fill extra middle space with value of this pixel
    IMG_TRANSFORM_STRETCH, // stretch image proportionally to fill whole area
    IMG_TRANSFORM_TILE     // tile image
};

/// creates image which stretches source image by filling center with pixels at splitX, splitY
LVImageSourceRef LVCreateStretchFilledTransform(LVImageSourceRef src, int newWidth, int newHeight,
		ImageTransform hTransform=IMG_TRANSFORM_SPLIT, ImageTransform vTransform=IMG_TRANSFORM_SPLIT,
		int splitX=-1, int splitY=-1);
/// creates image which fills area with tiled copy
LVImageSourceRef LVCreateTileTransform(LVImageSourceRef src,
                                       int newWidth,
                                       int newHeight,
                                       int offsetX,
                                       int offsetY);
/// creates XPM image
LVImageSourceRef LVCreateXPMImageSource(const char* data[]);
LVImageSourceRef LVCreateNodeImageSource(ldomNode* node);
LVImageSourceRef LVCreateDummyImageSource(ldomNode* node, int width, int height);
/// creates image source from stream
LVImageSourceRef LVCreateStreamImageSource(LVStreamRef stream);
/// creates image source as memory copy of file contents
LVImageSourceRef LVCreateFileCopyImageSource(lString16 fname);
/// creates image source as memory copy of stream contents
LVImageSourceRef LVCreateStreamCopyImageSource(LVStreamRef stream);
/// creates decoded memory copy of image, if it's unpacked size is less than maxSize
/// (8 bit gray or 32 bit color)
LVImageSourceRef LVCreateUnpackedImageSource(LVImageSourceRef srcImage,
		int maxSize = MAX_SKIN_IMAGE_CACHE_ITEM_UNPACKED_SIZE, bool gray = false);
/// creates decoded memory copy of image, if it's unpacked size is less than maxSize;
/// bpp: 8,16,32 supported
LVImageSourceRef LVCreateUnpackedImageSource(LVImageSourceRef srcImage, int maxSize, int bpp);
/// creates image source based on draw buffer
LVImageSourceRef LVCreateDrawBufImageSource(LVColorDrawBuf* buf, bool own);

#define COLOR_TRANSFORM_BRIGHTNESS_NONE 0x808080
#define COLOR_TRANSFORM_CONTRAST_NONE 0x404040

/// Creates image source which transforms colors of another image source.
/// addRGB added first, then multiplyed by multiplyRGB fixed point components (0x20 is 1.0f)
LVImageSourceRef LVCreateColorTransformImageSource(LVImageSourceRef srcImage,
                                                   lUInt32 addRGB,
                                                   lUInt32 multiplyRGB);
/// Creates image source which applies alpha to another image source
/// (0 is no change, 255 is totally transparent)
LVImageSourceRef LVCreateAlphaTransformImageSource(LVImageSourceRef srcImage, int alpha);

class LVFont;
class LVDrawBuf;

#define IMAGE_SOURCE_FROM_BYTES(imgvar, bufvar) \
    extern unsigned char bufvar []; \
    extern int bufvar ## _size ; \
    LVImageSourceRef imgvar = LVCreateStreamImageSource( \
        LVCreateMemoryStream( bufvar , bufvar ## _size ) )
#endif
