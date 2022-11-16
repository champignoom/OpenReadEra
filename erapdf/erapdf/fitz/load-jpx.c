#include "mupdf/fitz.h"

/* Without the definition of OPJ_STATIC, compilation fails on windows
 * due to the use of __stdcall. We believe it is required on some
 * linux toolchains too. */
#define OPJ_STATIC
#ifndef _MSC_VER
#define OPJ_HAVE_STDINT_H
#endif

#include <openjpeg.h>
#define MAXSIZE 512

static void fz_opj_error_callback(const char *msg, void *client_data)
{
	fz_context *ctx = (fz_context *)client_data;
	fz_warn(ctx, "openjpeg error: %s", msg);
}

static void fz_opj_warning_callback(const char *msg, void *client_data)
{
	fz_context *ctx = (fz_context *)client_data;
	fz_warn(ctx, "openjpeg warning: %s", msg);
}

static void fz_opj_info_callback(const char *msg, void *client_data)
{
	/* fz_warn("openjpeg info: %s", msg); */
}

typedef struct stream_block_s
{
	unsigned char *data;
	int size;
	int pos;
} stream_block;

static OPJ_SIZE_T fz_opj_stream_read(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data)
{
	stream_block *sb = (stream_block *)p_user_data;
	int len;

	len = sb->size - sb->pos;
	if (len < 0)
		len = 0;
	if (len == 0)
		return (OPJ_SIZE_T)-1; /* End of file! */
	if ((OPJ_SIZE_T)len > p_nb_bytes)
		len = p_nb_bytes;
	memcpy(p_buffer, sb->data + sb->pos, len);
	sb->pos += len;
	return len;
}

static OPJ_OFF_T fz_opj_stream_skip(OPJ_OFF_T skip, void * p_user_data)
{
	stream_block *sb = (stream_block *)p_user_data;

	if (skip > sb->size - sb->pos)
		skip = sb->size - sb->pos;
	sb->pos += skip;
	return sb->pos;
}

static OPJ_BOOL fz_opj_stream_seek(OPJ_OFF_T seek_pos, void * p_user_data)
{
	stream_block *sb = (stream_block *)p_user_data;

	if (seek_pos > sb->size)
		return OPJ_FALSE;
	sb->pos = seek_pos;
	return OPJ_TRUE;
}

static inline unsigned int read_value(const unsigned char *data, int bytes)
{
	unsigned int value = 0;
	for (; bytes > 0; bytes--)
		value = (value << 8) | *data++;
	return value;
}

void getJpxDims(fz_context *ctx, unsigned char *data, int size, int indexed, int *width, int *height, int *numcomps)
{
	opj_dparameters_t params;
	opj_codec_t *codec;
	opj_image_t *jpx;
	opj_stream_t *stream;
	unsigned char *p;
	OPJ_CODEC_FORMAT format;
	stream_block sb;

	if (size < 2)
		fz_throw(ctx, FZ_ERROR_GENERIC, "not enough data to determine image format");

	/* Check for SOC marker -- if found we have a bare J2K stream */
	if (data[0] == 0xFF && data[1] == 0x4F)
		format = OPJ_CODEC_J2K;
	else
		format = OPJ_CODEC_JP2;

	opj_set_default_decoder_parameters(&params);
	if (indexed)
		params.flags |= OPJ_DPARAMETERS_IGNORE_PCLR_CMAP_CDEF_FLAG;

	codec = opj_create_decompress(format);

	//opj_codec_set_threads(codec,4); // if not called - creates %cpu number% threads

	opj_set_info_handler(codec, fz_opj_info_callback, ctx);
	opj_set_warning_handler(codec, fz_opj_warning_callback, ctx);
	opj_set_error_handler(codec, fz_opj_error_callback, ctx);
	if (!opj_setup_decoder(codec, &params))
	{
		opj_destroy_codec(codec);
		fz_throw(ctx, FZ_ERROR_GENERIC, "j2k decode failed");
	}

	stream = opj_stream_default_create(OPJ_TRUE);
	sb.data = data;
	sb.pos = 0;
	sb.size = size;

	opj_stream_set_read_function(stream, fz_opj_stream_read);
	opj_stream_set_skip_function(stream, fz_opj_stream_skip);
	opj_stream_set_seek_function(stream, fz_opj_stream_seek);
	opj_stream_set_user_data(stream, &sb,NULL);
	/* Set the length to avoid an assert */
	opj_stream_set_user_data_length(stream, size);

	if (!opj_read_header(stream, codec, &jpx))
	{
		opj_stream_destroy(stream);
		opj_destroy_codec(codec);
		opj_image_destroy(jpx);
		fz_throw(ctx, FZ_ERROR_GENERIC, "Failed to read JPX header");
	}
	*width = jpx->x1 - jpx->x0;
	*height= jpx->y1 - jpx->y0;
	*numcomps = jpx->numcomps;
	opj_destroy_codec(codec);
	opj_stream_destroy(stream);
	opj_image_destroy(jpx);
}

fz_pixmap *
fz_load_jpx(fz_context *ctx, unsigned char *data, int size, fz_colorspace *defcs, int indexed)
{
	fz_pixmap *img;
	opj_dparameters_t params;
	opj_codec_t *codec;
	opj_image_t *jpx;
	opj_stream_t *stream;
	fz_colorspace *colorspace;
	unsigned char *p;
	OPJ_CODEC_FORMAT format;
	int a, n, w, h, depth, sgnd;
	int x, y, k, v;
	stream_block sb;

	if (size < 2)
		fz_throw(ctx, FZ_ERROR_GENERIC, "not enough data to determine image format");

	/* Check for SOC marker -- if found we have a bare J2K stream */
	if (data[0] == 0xFF && data[1] == 0x4F)
		format = OPJ_CODEC_J2K;
	else
		format = OPJ_CODEC_JP2;

	opj_set_default_decoder_parameters(&params);
	if (indexed)
		params.flags |= OPJ_DPARAMETERS_IGNORE_PCLR_CMAP_CDEF_FLAG;

	codec = opj_create_decompress(format);

	//opj_codec_set_threads(codec,4); // if not called - creates %cpu number% threads

	if(ctx->previewmode == 1)
	{
		//default vals
		params.cp_layer = 1;
		params.cp_reduce = 2;

		int width = 0;
		int height = 0;
		int temp_n = 0;
		getJpxDims(ctx,data,size,indexed,&width,&height,&temp_n);
		//LE("width = %d, height = %d",width,height);

		int maxdim = fz_maxi(width,height);
		unsigned int reducefactor = 0;
		int maxdim_temp = maxdim;
		while (maxdim_temp > MAXSIZE)
		{
			maxdim_temp = maxdim;
			reducefactor++;
			// + 0.5 is to fix double precision storage error
			// for example:
			// pow(2,5) might be stored as 24.9999999 or 25.0000000001 depending on compiler,
			// because the return type is double. When assigned to int,
			// 25.0000000001 becomes 25 but 24.9999999 will give output 24.
			maxdim_temp = maxdim_temp / (int)(pow(2,reducefactor)+0.5f);
		}

		params.cp_reduce = (reducefactor > 2) ? reducefactor: 2;

		int size = width*height*temp_n;
		if (ctx->preview_heavy_image == 0 && size > MAXSIZE * MAXSIZE * 3 )
		{
			ctx->preview_heavy_image = 1;
			/*
			   LE("new w = %d, new h = %d",
					width  / (int)(pow(2,params.cp_reduce)+0.5),
					height / (int)(pow(2,params.cp_reduce)+0.5));
			*/
		}
		else
		{
			//LE("JPG Image is not too big, so full decoding now : %dx%d*%d = %0.2f Kbytes",width,height,temp_n,size/1000.0);
			params.cp_layer = 0;
			params.cp_reduce = 0;
		}
	}
	else if (ctx->previewmode == 2) // SUPERFAST mode for page statistics extraction. All images are flat white.
	{
		int temp_w = 10;
		int temp_h = 10;
		unsigned char pixels[temp_w*temp_h*4];
		img = fz_new_pixmap_with_data(ctx, fz_device_rgb(ctx), temp_w, temp_h, pixels);
		fz_clear_pixmap_with_value(ctx, img, 0xff);
		return img;
	}

	opj_set_info_handler(codec, fz_opj_info_callback, ctx);
	opj_set_warning_handler(codec, fz_opj_warning_callback, ctx);
	opj_set_error_handler(codec, fz_opj_error_callback, ctx);
	if (!opj_setup_decoder(codec, &params))
	{
		opj_destroy_codec(codec);
		fz_throw(ctx, FZ_ERROR_GENERIC, "j2k decode failed");
	}

	stream = opj_stream_default_create(OPJ_TRUE);
	sb.data = data;
	sb.pos = 0;
	sb.size = size;

	opj_stream_set_read_function(stream, fz_opj_stream_read);
	opj_stream_set_skip_function(stream, fz_opj_stream_skip);
	opj_stream_set_seek_function(stream, fz_opj_stream_seek);
	opj_stream_set_user_data(stream, &sb,NULL);
	/* Set the length to avoid an assert */
	opj_stream_set_user_data_length(stream, size);

	if (!opj_read_header(stream, codec, &jpx))
	{
		opj_stream_destroy(stream);
		opj_destroy_codec(codec);
		fz_throw(ctx, FZ_ERROR_GENERIC, "Failed to read JPX header");
	}

	//LE("JPX x0 = %d x1 = %d y0 = %d y1 = %d",jpx->x0, jpx->x1, jpx->y0, jpx->y1)
	//opj_set_decode_area(codec, jpx, jpx->x0,jpx->y0,jpx->x1/2,jpx->y1/2);
	if (!(opj_decode(codec, stream, jpx) && opj_end_decompress(codec,stream)))
	{
		opj_stream_destroy(stream);
		opj_destroy_codec(codec);
		opj_image_destroy(jpx);
		fz_throw(ctx, FZ_ERROR_GENERIC, "Failed to decode JPX image");
	}

	opj_stream_destroy(stream);
	opj_destroy_codec(codec);

	/* jpx should never be NULL here, but check anyway */
	if (!jpx)
		fz_throw(ctx, FZ_ERROR_GENERIC, "opj_decode failed");

	for (k = 1; k < (int)jpx->numcomps; k++)
	{
		if (!jpx->comps[k].data)
		{
			opj_image_destroy(jpx);
			fz_throw(ctx, FZ_ERROR_GENERIC, "image components are missing data");
		}
		if (jpx->comps[k].w != jpx->comps[0].w)
		{
			opj_image_destroy(jpx);
			fz_throw(ctx, FZ_ERROR_GENERIC, "image components have different width");
		}
		if (jpx->comps[k].h != jpx->comps[0].h)
		{
			opj_image_destroy(jpx);
			fz_throw(ctx, FZ_ERROR_GENERIC, "image components have different height");
		}
		if (jpx->comps[k].prec != jpx->comps[0].prec)
		{
			opj_image_destroy(jpx);
			fz_throw(ctx, FZ_ERROR_GENERIC, "image components have different precision");
		}
	}

	n = jpx->numcomps;
	w = jpx->comps[0].w;
	h = jpx->comps[0].h;
	depth = jpx->comps[0].prec;
	sgnd = jpx->comps[0].sgnd;

	if (jpx->color_space == OPJ_CLRSPC_SRGB && n == 4) { n = 3; a = 1; }
	else if (jpx->color_space == OPJ_CLRSPC_SYCC && n == 4) { n = 3; a = 1; }
	else if (n == 2) { n = 1; a = 1; }
	else if (n > 4) { n = 4; a = 1; }
	else { a = 0; }

	if (defcs)
	{
		if (defcs->n == n)
		{
			colorspace = defcs;
		}
		else
		{
			fz_warn(ctx, "jpx file and dict colorspaces do not match");
			defcs = NULL;
		}
	}

	if (!defcs)
	{
		switch (n)
		{
		case 1: colorspace = fz_device_gray(ctx); break;
		case 3: colorspace = fz_device_rgb(ctx); break;
		case 4: colorspace = fz_device_cmyk(ctx); break;
		}
	}

	fz_try(ctx)
	{
		img = fz_new_pixmap(ctx, colorspace, w, h);
	}
	fz_catch(ctx)
	{
		opj_image_destroy(jpx);
		fz_rethrow_message(ctx, "out of memory loading jpx");
	}

	p = img->samples;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			for (k = 0; k < n + a; k++)
			{
				v = jpx->comps[k].data[y * w + x];
				if (sgnd)
					v = v + (1 << (depth - 1));
				if (depth > 8)
					v = v >> (depth - 8);
				*p++ = v;
			}
			if (!a)
				*p++ = 255;
		}
	}

	opj_image_destroy(jpx);

	if (a)
	{
		if (n == 4)
		{
			fz_pixmap *tmp = fz_new_pixmap(ctx, fz_device_rgb(ctx), w, h);
			fz_convert_pixmap(ctx, tmp, img);
			fz_drop_pixmap(ctx, img);
			img = tmp;
		}
		fz_premultiply_pixmap(ctx, img);
	}

	// EraPDF: patch from Sumatra PDF >>>
	/* SumatraPDF: extract image resolution (TODO: make openjpeg do this) */
	if (format == OPJ_CODEC_JP2)
	{
		unsigned char *base = data;
		int rest = size, ix = 0, level = 0;
		while (ix < rest - 8)
		{
			int lbox = read_value(base + ix, 4);
			unsigned int tbox = read_value(base + ix + 4, 4);
			if (lbox < 8 || lbox > rest - ix)
			{
				fz_warn(ctx, "impossibly small or large JP2 box (%x, %d)", tbox, lbox);
				break;
			}
			if (level == 0 && tbox == 0x6A703268 /* jp2h */ || level == 1 && tbox == 0x72657320 /* res  */)
			{
				base += ix + 8;
				rest = lbox - 8;
				ix = 0;
				level++;
			}
			else if (level == 2 && tbox == 0x72657363 /* resc */ && lbox == 18 && rest - ix >= 18)
			{
				int vrn = read_value((base += ix + 8), 2);
				int vrd = read_value(base + 2, 2);
				int hrn = read_value(base + 4, 2);
				int hrd = read_value(base + 6, 2);
				int vre = (char)base[8], hre = (char)base[9];
				img->xres = (int)((float)hrn / hrd * pow(10, hre - 2) * 2.54f);
				img->yres = (int)((float)vrn / vrd * pow(10, vre - 2) * 2.54f);
				if (img->xres <= 0 || img->yres <= 0)
				{
					fz_warn(ctx, "invalid image resolution (%d, %d)", img->xres, img->yres);
					img->xres = img->yres = 96;
				}
				break;
			}
			else
			{
				ix += lbox;
			}
		}
	}
	// EraPDF: patch from Sumatra PDF <<<

	return img;
}
