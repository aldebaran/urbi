/*
 * Copyright (C) 2006-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cstdlib>
#include <libport/debug.hh>
#include <libport/cstdio>
#include <libport/format.hh>

#include <urbi/uconversion.hh>

GD_CATEGORY(Urbi.Convert);

// FIXME: we have alignment issues in this file.  That might be an
// issue on some architectures.

#ifndef NO_IMAGE_CONVERSION
# include <csetjmp>

// It would be nice to use jpeg/jpeglib.h, but this file includes
// jconfig.h, unqualified, which we might pick-up on the host.  So
// don't take gratuitous chances.
# include <jpeglib.h>

namespace urbi
{

  namespace
  {
    void*
    read_jpeg(const char* jpgbuffer, size_t jpgbuffer_size,
              bool RGB, size_t& output_size, size_t& w, size_t& h);

    int
    write_jpeg(const byte* src, size_t w, size_t h, bool ycrcb,
               byte* dst, size_t& sz, int quality);

    inline byte clamp(int v)
    {
      return (v < 0     ? 0
              : 255 < v ? 255
              :           v);
    }

    inline byte clamp(float v)
    {
      return (v < 0     ? 0
              : 255 < v ? 255
              :           (byte) v);
    }
  } // namespace

  int
  convertRGBtoYCbCr(const byte* in, size_t bufferSize,
		    byte* out)
  {
    for (size_t i = 0; i < bufferSize - 2; i += 3)
    {
      float r = in[i];
      float g = in[i + 1];
      float b = in[i + 2];
      /*
	Y  =      (0.257 * R) + (0.504 * G) + (0.098 * B) + 16
	Cr = V =  (0.439 * R) - (0.368 * G) - (0.071 * B) + 128
	Cb = U = -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128
      */
      out[i]     = clamp( 0.257f * r + 0.504f * g + 0.098f * b +  16.0f);
      out[i + 1] = clamp(-0.148f * r - 0.291f * g + 0.439f * b + 128.0f);
      out[i + 2] = clamp( 0.439f * r - 0.368f * g - 0.071f * b + 128.0f);
    }
    return 1;
  }

  int
  convertYCrCbtoYCbCr(const byte* in, size_t bufferSize,
		      byte* out)
  {
    for (size_t i = 0; i < bufferSize - 2; i += 3)
    {
      byte tmp; // If source == destination
      out[i]     = in[i];
      tmp        = in[i + 1];
      out[i + 1] = in[i + 2];
      out[i + 2] = tmp;
    }
    return 1;
  }


  int
  convertYCbCrtoRGB(const byte* in, size_t bufferSize,
		    byte* out)
  {
    // http://en.wikipedia.org/wiki/YUV#Converting_between_Y.27UV_and_RGB
    for (size_t i = 0; i < bufferSize - 2; i += 3)
    {
      int c = in[i]-16;
      int c298 = c * 298;
      int d = in[i+1] - 128;
      int e = in[i+2] - 128;
      out[i] = clamp((c298 + 409*e + 128) >> 8);
      out[i+1] = clamp((c298 + 100*d - 20*e + 128) >> 8);
      out[i+2] = clamp((c298 + 516*d + 128) >> 8);
      /* Float version, 5 times slower on p4.
         float y = in[i];
         float cb = in[i + 1];
         float cr = in[i + 2];
         out[i] = clamp(1.164 * (y - 16) + 1.596 * (cr - 128));
         out[i + 1] = clamp(1.164 * (y - 16) - 0.813 * (cr - 128) -
         0.392 * (cb - 128));
         out[i + 2] = clamp(1.164 * (y - 16) + 2.017 * (cb - 128));
      */
    }
    return 1;
  }


  /** Convert a buffer \a source, which contains a JPEG image, to a
      buffer for the \a dest, which will contain a RGB/YCrCb image
      (depending on \a dest_format).

      The \a source buffer is expected to be a pointer to a valid memory
      area of size equal to \a sourcelen.

      If \a dest buffer pointer is nul, then it will be changed to target
      the allocated RGB/YCrCb image of size \a size.  Otherwise a part of the
      RGB/YCrCb image will be copied in the buffer in the limit of the \a size
      argument.

      The property \a w and \a h respectively correspond to the width and
      the height that are retrieved during the convertion of the convertion
      of the data.

      \return 1 on success.
  */
  static
  int
  convert_jpeg_to(const byte* source, size_t sourcelen,
                  UImageFormat dest_format,
                  byte** dest, size_t& size, size_t& w, size_t& h)
  {
    passert(dest_format,
            dest_format == IMAGE_RGB || dest_format == IMAGE_YCbCr);
    if (!dest)
      return 0;

    size_t sz;
    void *destination = read_jpeg((const char*) source, sourcelen,
                                  dest_format == IMAGE_RGB, sz, w, h);
    if (!destination)
    {
      size = 0;
      return 0;
    }
    if (!*dest)
    {
      *dest = (byte*) destination;
      size = sz;
      return 1;
    }
    size_t cplen = std::min(sz, size);
    memcpy(*dest, destination, cplen);
    free(destination);
    size = sz;
    return 1;
  }

  int
  convertJPEGtoYCrCb(const byte* source, size_t sourcelen,
                     byte** dest, size_t& size, size_t& w, size_t& h)
  {
    return convert_jpeg_to(source, sourcelen,
                           IMAGE_YCbCr, dest, size, w, h);
  }

  int
  convertJPEGtoRGB(const byte* source, size_t sourcelen,
                   byte** dest, size_t& size, size_t& w, size_t& h)
  {
    return convert_jpeg_to(source, sourcelen,
                           IMAGE_RGB, dest, size, w, h);
  }


  int
  convertRGBtoJPEG(const byte* source,
                   size_t w, size_t h, byte* dest,
                   size_t& size, int quality)
  {
    return write_jpeg(source, w, h, false, dest, size, quality);
  }


  int
  convertYCrCbtoJPEG(const byte* source,
                     size_t w, size_t h, byte* dest,
                     size_t& size, int quality)
  {
    return write_jpeg(source, w, h, true, dest, size, quality);
  }

  int
  convertRGBtoGrey8_601(const byte* in, size_t bufferSize,
                        byte* out)
  {
    for (size_t j = 0, i = 0; i < bufferSize - 2; i += 3, j++)
    {
      float r = in[i];
      float g = in[i + 1];
      float b = in[i + 2];
      out[j]  = clamp( 0.299f * r + 0.587f * g + 0.114f * b);
    }
    return 1;
  }

  struct mem_source_mgr
  {
    struct jpeg_source_mgr pub;
    JOCTET eoi[2];
  };


  namespace
  {
    void init_source(j_decompress_ptr)
    {
    }

    boolean fill_input_buffer(j_decompress_ptr cinfo)
    {
      mem_source_mgr *src = (mem_source_mgr *) cinfo->src;
      if (src->pub.bytes_in_buffer != 0)
	return TRUE;
      src->eoi[0] = 0xFF;
      src->eoi[1] = JPEG_EOI;
      src->pub.bytes_in_buffer = 2;
      src->pub.next_input_byte = src->eoi;
      return TRUE;
    }

    void term_source(j_decompress_ptr)
    {
    }

    void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
    {
      mem_source_mgr* src = (mem_source_mgr*) cinfo->src;
      if (num_bytes <= 0)
	return;
      if (static_cast<unsigned long> (num_bytes) > src->pub.bytes_in_buffer)
	num_bytes = src->pub.bytes_in_buffer;
      src->pub.bytes_in_buffer -= num_bytes;
      src->pub.next_input_byte += num_bytes;
    }

  } // namespace

  struct urbi_jpeg_error_mgr
  {
    struct jpeg_error_mgr pub;	/* "public" fields */
    jmp_buf setjmp_buffer;	/* for return to caller */
  };

  METHODDEF(void)
  urbi_jpeg_error_exit (j_common_ptr cinfo)
  {
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    urbi_jpeg_error_mgr *  myerr = ( urbi_jpeg_error_mgr *) cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message) (cinfo);

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
  }


  struct mem_destination_mgr
  {
    struct jpeg_destination_mgr pub;
  };

  static void init_destination(j_compress_ptr)
  {
  }

  static boolean empty_output_buffer(j_compress_ptr)
  {
    return FALSE;
  }

  static void term_destination(j_compress_ptr)
  {
  }

  namespace
  {
    int
    write_jpeg(const byte* src, size_t w, size_t h, bool ycrcb,
	       byte* dst, size_t& sz, int quality)
    {
      struct jpeg_compress_struct cinfo;
      struct jpeg_error_mgr jerr;

      int row_stride;		/* physical row width in image buffer */

      cinfo.err = jpeg_std_error(&jerr);
      jpeg_create_compress(&cinfo);
      mem_destination_mgr *dest = (struct mem_destination_mgr *)
	(*cinfo.mem->alloc_small) ((j_common_ptr) & cinfo, JPOOL_PERMANENT,
				   sizeof (mem_destination_mgr));

      cinfo.dest = (jpeg_destination_mgr*)dest;
      dest->pub.init_destination=&init_destination;
      dest->pub.empty_output_buffer = &empty_output_buffer;
      dest->pub.term_destination = term_destination;
      dest->pub.free_in_buffer = sz;
      dest->pub.next_output_byte = dst;
      cinfo.image_width = w;
      cinfo.image_height = h;
      cinfo.input_components = 3;  // # of color components per pixel.
      /* colorspace of input image */
      cinfo.in_color_space = ycrcb ? JCS_YCbCr : JCS_RGB;

      jpeg_set_defaults(&cinfo);

      jpeg_set_quality(&cinfo,
		       quality, TRUE /* limit to baseline-JPEG values */);

      jpeg_start_compress(&cinfo, TRUE);

      row_stride = w * 3;	/* JSAMPLEs per row in image_buffer */

      while (cinfo.next_scanline < cinfo.image_height)
      {
	/* pointer to JSAMPLE row[s] */
        const JSAMPLE* row =
          (const JSAMPLE*) &src[cinfo.next_scanline * row_stride];
	jpeg_write_scanlines(&cinfo, const_cast<JSAMPLE**>(&row), 1);
      }

      jpeg_finish_compress(&cinfo);
      sz -= dest->pub.free_in_buffer ;
      jpeg_destroy_compress(&cinfo);

      return sz;
    }

    /*! Convert a jpeg image to YCrCb or RGB. Allocate the buffer with malloc.
     */
    void *read_jpeg(const char* jpgbuffer, size_t jpgbuffer_size, bool RGB,
		    size_t& output_size, size_t& w, size_t& h)
    {
      struct jpeg_decompress_struct cinfo;
      struct urbi_jpeg_error_mgr jerr;
      cinfo.err = jpeg_std_error(&jerr.pub);
      jerr.pub.error_exit = urbi_jpeg_error_exit;
      if (setjmp(jerr.setjmp_buffer))
      {
        /* If we get here, the JPEG code has signaled an error.  We
         * need to clean up the JPEG object, close the input file, and
         * return.
         */
	jpeg_destroy_decompress(&cinfo);
        GD_ERROR("JPEG error!");
	return 0;
      }
      jpeg_create_decompress(&cinfo);
      mem_source_mgr *source = (struct mem_source_mgr *)
	(*cinfo.mem->alloc_small) ((j_common_ptr) & cinfo, JPOOL_PERMANENT,
				   sizeof (mem_source_mgr));

      cinfo.src = (jpeg_source_mgr *) source;
      source->pub.skip_input_data = skip_input_data;
      source->pub.term_source = term_source;
      source->pub.init_source = init_source;
      source->pub.fill_input_buffer = fill_input_buffer;
      source->pub.resync_to_restart = jpeg_resync_to_restart;
      source->pub.bytes_in_buffer = jpgbuffer_size;
      source->pub.next_input_byte = (JOCTET *) jpgbuffer;
      cinfo.out_color_space = (RGB ? JCS_RGB : JCS_YCbCr);
      jpeg_read_header(&cinfo, TRUE);
      cinfo.out_color_space = (RGB ? JCS_RGB : JCS_YCbCr);
      jpeg_start_decompress(&cinfo);
      w = cinfo.output_width;
      h = cinfo.output_height;
      output_size =
        cinfo.output_width * cinfo.output_components * cinfo.output_height;
      void *buffer = malloc(output_size);

      while (cinfo.output_scanline < cinfo.output_height)
      {
	/* jpeg_read_scanlines expects an array of pointers to scanlines.
	 * Here the array is only one element long, but you could ask for
	 * more than one scanline at a time if that's more convenient.
	 */
	JSAMPLE* row =
          (JSAMPLE *) &((char*) buffer)[cinfo.output_scanline
                                        * cinfo.output_components
                                        * cinfo.output_width];
	jpeg_read_scanlines(&cinfo, &row, 1);
      }
      jpeg_finish_decompress(&cinfo);
      jpeg_destroy_decompress(&cinfo);

      return buffer;
    }



    //scale putting (scx, scy) at the center of destination image
    void scaleColorImage(byte* src, int sw, int sh,
			 int scx, int scy, byte* dst,
			 int dw, int dh, float sx, float sy)
    {
      for (int x = 0; x < dw; ++x)
	for (int y = 0; y < dh; ++y)
	{
	  //find the corresponding point in source image
	  float fsrcx = (float) (x-dw/2) / sx  + (float) scx;
	  float fsrcy = (float) (y-dh/2) / sy  + (float) scy;
	  int srcx = (int) fsrcx;
	  int srcy = (int) fsrcy;
	  if (srcx <= 0 || srcx >= sw - 1 || srcy <= 0 || srcy >= sh - 1)
	    memset(dst + (x + y * dw) * 3, 0, 3);
	  else //do the bilinear interpolation
	  {
	    float xfactor = fsrcx - (float) srcx;
	    float yfactor = fsrcy - (float) srcy;
	    for (int color = 0; color < 3; ++color)
	    {
	      float up = (float) src[(srcx + srcy * sw) * 3 + color]
		* (1.0 - xfactor)
		+ (float) src[(srcx + 1 + srcy * sw) * 3 + color] * xfactor;
	      float down = (float) src[(srcx + (srcy + 1) * sw) * 3 + color]
		* (1.0 - xfactor)
		+ (float) src[(srcx + 1 + (srcy + 1) * sw) * 3 + color]
		* xfactor;
	      float result = up * (1.0 - yfactor) + down * yfactor;
	      dst[(x + y * dw) * 3 + color] = (byte) result;
	    }
	  }
	}
    }

  } // anonymous namespace

  int convert(const UImage& src, UImage& dest)
  {
    enum FormatKind
    {
      RGB,
      YUV,
      COMPRESSED,
      UNSET
    };
    //step 1: uncompress source, to have raw uncompressed rgb or ycbcr
    bool allocated = false; // true if data must be freed

    // uncompressed data.
    byte* data = 0;
    size_t w, h;
    size_t usz;
    // Effective format of the source in 'data'.
    FormatKind format = UNSET;
    // Format we need the source in
    FormatKind targetformat = UNSET;

    switch (dest.imageFormat)
    {
    case IMAGE_RGB:
    case IMAGE_PPM:
    case IMAGE_GREY8:
      targetformat = RGB;
      break;
    case IMAGE_YCbCr:
    case IMAGE_NV12:
    case IMAGE_YUV411_PLANAR:
    case IMAGE_YUV420_PLANAR:
      targetformat = YUV;
      break;
    case IMAGE_JPEG:
      targetformat = COMPRESSED;
      break;
    default:
      GD_FERROR("Image conversion to format %s is not implemented",
                dest.format_string());
      return 0;
    }
    unsigned p = 0;
    int c = 0;

    // Avoid using src fields because JPEG file format embedded these
    // information in the data buffer.
    if (src.imageFormat != IMAGE_JPEG)
    {
      w = src.width;
      h = src.height;
      usz = w * h * 3;
    }

    switch (src.imageFormat)
    {
    case IMAGE_YCbCr:
      format = YUV;
      data = src.data;
      break;
    case IMAGE_RGB:
      format = RGB;
      data = src.data;
      break;
    case IMAGE_PPM:
      format = RGB;
      //locate header end
      p = 0;
      c = 0;
      while (c < 3 && p < src.size)
        if (src.data[p++] == '\n')
          ++c;
      data = src.data + p;
      break;
    case IMAGE_JPEG:
      // this image is allocated by the function convertJPEG* function.
      // w, h and usz are defined by these functions calls.
      allocated = true;
      if (targetformat == RGB)
      {
        convertJPEGtoRGB((byte*) src.data, src.size,
                         (byte**) &data, usz,
                         w, h);
        format = RGB;
      }
      else
      {
        convertJPEGtoYCrCb((byte*) src.data, src.size,
                           (byte**) &data, usz,
                           w, h);
        format = YUV;
      }
      break;
    case IMAGE_YUV422:
      format = YUV;
      data = (byte*)malloc(src.width * src.height * 3);
      allocated = true;
      for (unsigned i=0; i< src.width*src.height; i+=2)
      {
        data[i*3] = src.data[i*2];
        data[i*3 + 1] = src.data[i*2+1];
        data[i*3 + 2] = src.data[i*2+3];
        data[(i+1)*3] = src.data[i*2+2];
        data[(i+1)*3 + 1] = src.data[i*2+1];
        data[(i+1)*3 + 2] = src.data[i*2+3];
      }
      break;
    case IMAGE_YUV411_PLANAR:
    {
      format = YUV;
      data = (byte*)malloc(src.width * src.height * 3);
      allocated = true;
      unsigned char* cy = src.data;
      unsigned char* u = cy + w*h;
      unsigned char* v = u + w*h/4;
      int w = src.width;
      int h = src.height;
      for (int x=0; x<w;++x)
        for (int y=0; y<h; ++y)
        {
          data[(x+y*w)*3+0] = cy[x+y*w];
          data[(x+y*w)*3+1] = u[x/4 + y*w/4];
          data[(x+y*w)*3+2] = v[x/4 + y*w/4];
        }
    }
    break;
    case IMAGE_YUV420_PLANAR:
    {
      format = YUV;
      data = (byte*)malloc(src.width * src.height * 3);
      allocated = true;
      unsigned char* cy = src.data;
      unsigned char* u = cy + w*h;
      unsigned char* v = u + w*h/4;
      int w = src.width;
      int h = src.height;
      for (int x=0; x<w;++x)
        for (int y=0; y<h; ++y)
        {
          data[(x+y*w)*3+0] = cy[x+y*w];
          data[(x+y*w)*3+1] = u[x/2 + (y>>1)*w/2];
          data[(x+y*w)*3+2] = v[x/2 + (y>>1)*w/2];
        }
    }
    break;
    case IMAGE_NV12:
    {
      format = YUV;
      data = (byte*)malloc(src.width * src.height * 3);
      allocated = true;
      unsigned char* cy = src.data;
      unsigned char* uv = src.data + w*h;
      for (unsigned int x=0; x<w;++x)
        for (unsigned int y=0; y<h; ++y)
        {
          data[(x+y*w)*3+0] = cy[x+y*w];
          data[(x+y*w)*3+1] = uv[((x>>1) + (((y>>1)*w)>>1))*2];
          data[(x+y*w)*3+2] = uv[((x>>1) + (((y>>1)*w)>>1))*2 + 1];
        }
    }
    break;
    case IMAGE_GREY8:
      format = YUV;
      data = (byte*)malloc(src.width * src.height * 3);
      allocated = true;
      memset(data, 127, src.width * src.height * 3);
      for (unsigned i=0; i< src.width*src.height; ++i)
        data[i*3] = src.data[i];
      break;
    case IMAGE_GREY4:
      format = YUV;
      data = (byte*)malloc(src.width * src.height * 3);
      allocated = true;
      memset(data, 127, src.width * src.height * 3);
      for (unsigned i=0; i< src.width*src.height; i+=2)
      {
        data[i*3] = src.data[i/2] & 0xF0;
        data[(i+1)*3] = (src.data[i/2] & 0x0F) << 4;
      }
      break;
    case IMAGE_UNKNOWN:
      break;
    }

    if (dest.width == 0)
      dest.width = w;
    if (dest.height == 0)
      dest.height = h;

    //now resize if target size is different
    if (w != dest.width || h != dest.height)
    {
      void* scaled = malloc(dest.width * dest.height * 3);
      scaleColorImage(data, w, h, w/2, h/2,
		      (byte*) scaled, dest.width, dest.height,
		      (float) dest.width / (float) w,
		      (float) dest.height / (float) h);
      if (allocated)
        free(data);
      data = (byte*)scaled;
      allocated = true;
    }
    // Then factor YUV<->RGB conversion if necessary
    if ((format == RGB && targetformat == YUV)
        || (format == YUV && targetformat == RGB))
    {
      byte* src = data;
      if (!allocated)
      {
        allocated = true;
        data = (byte*)malloc(dest.width * dest.height * 3);
      }
      if (format == RGB)
        convertRGBtoYCbCr(src, dest.width * dest.height * 3, data);
      else
        convertYCbCrtoRGB(src, dest.width * dest.height * 3, data);
      format = targetformat;
    }
    //then convert to destination format
    dest.size = dest.width * dest.height * 3 + 20;
    if (dest.imageFormat == IMAGE_GREY8)
      dest.size = dest.width * dest.height + 20;
    dest.data = static_cast<byte*> (realloc(dest.data, dest.size));
    size_t dsz = dest.size;
    switch (dest.imageFormat)
    {
    case IMAGE_RGB:
      memcpy(dest.data, data, dest.width * dest.height * 3);
      break;
    case IMAGE_GREY8:
      assert(format == 0);
      convertRGBtoGrey8_601((byte*) data,
                            dest.width * dest.height * 3, (byte*) dest.data);
      break;
    case IMAGE_YCbCr:
      memcpy(dest.data, data, dest.width * dest.height * 3);
      break;
    case IMAGE_PPM:
      strcpy((char*) dest.data,
             libport::format("P6\n%s %s\n255\n",
                             dest.width, dest.height).c_str());
      memcpy(dest.data + strlen((char*) dest.data),
             data, dest.width * dest.height * 3);
      break;
    case IMAGE_JPEG:
      if (format == YUV)
        convertYCrCbtoJPEG((byte*) data,
                           dest.width ,dest.height,
                           (byte*) dest.data, dsz, 80);
      else
        convertRGBtoJPEG((byte*) data,
                         dest.width , dest.height,
                         (byte*) dest.data, dsz, 80);
      dest.size = dsz;
      break;
    case IMAGE_YUV411_PLANAR:
    {
      unsigned int plane = dest.width * dest.height;
      for (unsigned int i=0; i<plane;++i)
        dest.data[i] = data[i*3];
      for (unsigned int y=0; y<dest.height; y++)
        for (unsigned int x=0; x<dest.width; x+=4)
        {
          dest.data[plane + x/4 + y*dest.width/4]
            = data[(x+y*dest.width)*3+1];
          dest.data[plane+plane/4 + x/4 + y*dest.width/4]
            = data[(x+y*dest.width)*3+2];
        }
      break;
    }
    case IMAGE_YUV420_PLANAR:
    {
      unsigned int plane = dest.width * dest.height;
      for (unsigned int i=0; i<plane;++i)
        dest.data[i] = data[i*3];
      for (unsigned int y=0; y<dest.height/2; y++)
        for (unsigned int x=0; x<dest.width/2; x++)
        {
          dest.data[plane + x +y*dest.width/2]
            = data[(x*2+y*2*dest.width)*3+1];
          dest.data[plane + plane/4 + x +y*dest.width/2]
            = data[(x*2+y*2*dest.width)*3+2];
        }
      break;
    }
    case IMAGE_NV12:
    {
      unsigned int planeS = dest.width * dest.height;
      // y plane
      for (unsigned int p=0; p<planeS; ++p)
        dest.data[p] = data[p*3];
      // crcb interleaved plane
      for (unsigned int y=0; y<dest.height; y+=2)
        for (unsigned int x=0; x<dest.width; x+=2)
        {
          dest.data[planeS + x + y*dest.width/2]
            = data[(x+y*dest.width)*3+1];
          dest.data[planeS + x + y*dest.width/2 +1 ]
            = data[(x+y*dest.width)*3+2];
        }
      dest.size = planeS * 3 / 2;
    }
    break;
    default:
      GD_FERROR("Image conversion to format %s is not implemented",
                dest.format_string());
    }
    if (allocated)
      free(data);
    return 1;
  }

} // namespace urbi

#endif // !NO_IMAGE_CONVERSION

namespace urbi
{

  // FIXME: this is really debatable...
#if defined __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wcast-align"
#endif
  static
  void
  dup(unsigned short* dst, const unsigned short* src, size_t count)
  {
    unsigned int* idst = (unsigned int*)dst;
    const unsigned short* end = src + count;
    while (src != end)
    {
      *(idst++) = (unsigned int)(*src) << 16 | (unsigned int)(*src);
      src++;
    }
  }


  static
  void
  dup(byte* dst, const byte* src, size_t count)
  {
    unsigned short* idst = (unsigned short*)dst;
    const byte* end = src + count;
    while (src != end)
    {
      *(idst++) = (unsigned short)(*src) << 8 | (unsigned short)(*src);
      src++;
    }
  }
#if defined __clang__
# pragma GCC diagnostic pop
#endif

  template<typename D> void
  pud(D* dst, const D* src, int count)
  {
    for (int i=0; i<count/2; i++)
      dst[i] = src[i*2];
  }

  template<class S, class D>
  void copy(const S* src, D* dst,
	    int sc, int dc, int sr, int dr,
            size_t count, bool sf, bool df)
  {
    long shift = 8 * (sizeof (S) - sizeof (D));
    if (!shift && sc == dc && sr == dr && sf==df)
    {
      memcpy(dst, src, sizeof(S)*sc*count);
      return;
    }
    for (size_t i = 0; i < count; ++i)
    {
      float soffset = (float)i * ((float)sr / (float)dr);
      int so = (int)soffset;
      float factor = soffset - (float)so;
      S s1, s2;
      s1 = src[so * sc];
      if (i != count - 1)
	s2 = src[(so + 1) * sc];
      else
	s2 = s1; //nothing to interpolate with
      if (!sf)
      {
	s1 = s1 ^ (1<<(sizeof (S)*8-1));
	s2 = s2 ^ (1<<(sizeof (S)*8-1));
      }
      int v1 = (int) ((float)(s1)*(1.0-factor) + (float)(s2)*factor);
      int v2;
      if (sc==1)
	v2 = v1;
      else
      {
	s1 = src[so*sc+1];
	if (i != count - 1)
	  s2 = src[(so+1)*sc+1];
	else
	  s2 = s1; //nothing to interpolate with
	if (!sf)
	{
	  s1 = s1 ^ (1<<(sizeof (S)*8-1));
	  s2 = s2 ^ (1<<(sizeof (S)*8-1));
	}
	v2 = (int) ((float)(s1)*(1.0-factor) + (float)(s2)*factor);
      }
      D d1, d2;
      if (shift>=0)
      {
	d1 = (D)(v1 >>shift);
	d2 = (D)(v2 >>shift);
      }
      else
      {
	d1 = (D)(v1) * (1 << -shift);
	d2 = (D)(v2) * (1 << -shift);
      }
      if (!df)
      {
	d1 = d1 ^ (1<<(sizeof (D)*8-1));
	d2 = d2 ^ (1<<(sizeof (D)*8-1));
      }
      if (dc==2)
      {
	dst[i*2] = d1;
	dst[i*2+1] = d2;
      }
      else
	dst[i] = (D) (((int)d1+(int)d2) /2);
    }
  }


#if defined __clang__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wcast-align"
#endif
  int
  convert (const USound &source, USound &dest)
  {
    if ((source.soundFormat != SOUND_RAW
	 && source.soundFormat != SOUND_WAV)
	|| (dest.soundFormat != SOUND_RAW
	    && dest.soundFormat != SOUND_WAV))
      return 1; //conversion not handled yet
    /* phase one: calculate required buffer size, set destination unspecified
     * fields */
    size_t schannels, srate, ssampleSize;
    USoundSampleFormat ssampleFormat;
    if (source.soundFormat == SOUND_WAV)
    {
      wavheader * wh = (wavheader *)source.data;
      schannels = wh->channels;
      srate = wh->freqechant;
      ssampleSize = wh->bitperchannel;
      ssampleFormat = (ssampleSize>8)?SAMPLE_SIGNED:SAMPLE_UNSIGNED;
    }
    else
    {
      schannels = source.channels;
      srate = source.rate;
      ssampleSize = source.sampleSize;
      ssampleFormat = source.sampleFormat;
    }
    if (!dest.channels)
      dest.channels = schannels;
    if (!dest.rate)
      dest.rate = srate;
    if (!dest.sampleSize)
      dest.sampleSize = ssampleSize;
    if (!(int)dest.sampleFormat)
      dest.sampleFormat = ssampleFormat;
    if (dest.soundFormat == SOUND_WAV)
      dest.sampleFormat = dest.sampleSize > 8 ? SAMPLE_SIGNED
        : SAMPLE_UNSIGNED;
    // That's a big one!
    unsigned destSize =
      ((long long)(source.size
		   - ((source.soundFormat == SOUND_WAV)?44:0))
       * (long long)dest.channels
       * (long long)dest.rate
       * (long long)(dest.sampleSize/8))
      / ((long long)schannels
	 *(long long)srate
	 *(long long)(ssampleSize/8));
    if (dest.soundFormat == SOUND_WAV)
      destSize += sizeof (wavheader);
    if (dest.size<destSize)
      dest.data = static_cast<char*> (realloc (dest.data, destSize));
    dest.size = destSize;
    //write destination header if appropriate
    if (dest.soundFormat == SOUND_WAV)
    {
      wavheader* wh = (wavheader*) dest.data;
      memcpy(wh->riff, "RIFF", 4);
      wh->length = dest.size - 8;
      memcpy(wh->wave, "WAVE", 4);
      memcpy(wh->fmt, "fmt ", 4);
      wh->lnginfo = 16;
      wh->one = 1;
      wh->channels = dest.channels;
      wh->freqechant = dest.rate;
      wh->bytespersec = dest.rate * dest.channels * (dest.sampleSize/8);
      wh->bytesperechant = (dest.sampleSize/8)*dest.channels;
      wh->bitperchannel = dest.sampleSize;
      memcpy(wh->data, "data", 4);
      wh->datalength = destSize - sizeof (wavheader);
    }

    //do the conversion and write to dest.data
    char* sbuffer = source.data;
    if (source.soundFormat == SOUND_WAV)
      sbuffer += sizeof (wavheader);
    char* dbuffer = dest.data;
    if (dest.soundFormat == SOUND_WAV)
      dbuffer += sizeof (wavheader);
    int elementCount = dest.size - (dest.soundFormat == SOUND_WAV ?
				    sizeof (wavheader) : 0);
    elementCount /= (dest.channels * (dest.sampleSize / 8));
    switch (ssampleSize * 1000 + dest.sampleSize)
    {
    case 8008:
      if (srate == dest.rate && schannels == 1 && dest.channels == 2)
        dup((byte*)dbuffer, (byte*)sbuffer, elementCount);
      else if (srate == dest.rate && schannels == 2 && dest.channels == 1)
        pud(dbuffer, sbuffer, elementCount);
      else
        copy(dbuffer, sbuffer, schannels, dest.channels, srate, dest.rate,
	     elementCount, ssampleFormat==SAMPLE_SIGNED, dest.sampleFormat ==
	     SAMPLE_SIGNED);
      break;
    case 16008:
      copy((short*)sbuffer, dbuffer, schannels, dest.channels, srate,
           dest.rate, elementCount, ssampleFormat==SAMPLE_SIGNED,
           dest.sampleFormat == SAMPLE_SIGNED);
      break;
    case 16016: // Data is short, but convertions needs an unsigned short.
      if (srate == dest.rate && schannels == 1 && dest.channels == 2)
        dup((unsigned short*)dbuffer,
            (unsigned short*)sbuffer,
            elementCount);
      else if (srate == dest.rate && schannels == 2 && dest.channels == 1)
        pud((unsigned short*)dbuffer,
            (unsigned short*)sbuffer,
            elementCount);
      else
        copy((short*)sbuffer, (short*)dbuffer, schannels, dest.channels,
	     srate, dest.rate, elementCount, ssampleFormat==SAMPLE_SIGNED,
	     dest.sampleFormat == SAMPLE_SIGNED);
      break;
    case 8016:
      copy((char*)sbuffer, (short*)dbuffer, schannels, dest.channels,
           srate, dest.rate, elementCount, ssampleFormat==SAMPLE_SIGNED,
           dest.sampleFormat == SAMPLE_SIGNED);
      break;
    }
    return 0;
  }
#if defined __clang__
# pragma clang diagnostic pop
#endif
} // namespace urbi
