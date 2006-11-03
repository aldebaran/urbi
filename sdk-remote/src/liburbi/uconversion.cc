#include "urbi/uconversion.hh"

#ifndef NO_IMAGE_CONVERSION
# include <setjmp.h>

# include "jpeg-6b/jpeglib.h"

namespace urbi
{

  namespace
  {
    void *read_jpeg(const char *jpgbuffer, int jpgbuffer_size,
		    bool RGB, int &output_size);

    int write_jpeg(const unsigned char* src, int w, int h, bool ycrcb,
		   unsigned char* dst, int &sz, int quality);


    inline unsigned char clamp(float v)
    {
      if (v < 0)
	return 0;
      if (v > 255)
	return 255;
      return (unsigned char) v;
    }
  } // namespace

  int
  convertRGBtoYCrCb(const byte * sourceImage,
		    int bufferSize,
		    byte * destinationImage)
  {
    unsigned char *in = (unsigned char *) sourceImage;
    unsigned char *out = (unsigned char *) destinationImage;
    for (int i = 0; i < bufferSize - 2; i += 3) {
      float r = in[i];
      float g = in[i + 1];
      float b = in[i + 2];
      /*
	Y  =      (0.257 * R) + (0.504 * G) + (0.098 * B) + 16
	Cr = V =  (0.439 * R) - (0.368 * G) - (0.071 * B) + 128
	Cb = U = -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128
      */
      out[i] = clamp((0.257 * r) + (0.504 * g) + (0.098 * b) + 16);
      out[i + 1] = clamp((0.439 * r) - (0.368 * g) - (0.071 * b) + 128);
      out[i + 2] = clamp(-(0.148 * r) - (0.291 * g) + (0.439 * b) + 128);
    }
    return 1;
  }

  int
  convertYCrCbtoRGB(const byte * sourceImage,
		    int bufferSize,
		    byte * destinationImage)
  {
    unsigned char *in = (unsigned char *) sourceImage;
    unsigned char *out = (unsigned char *) destinationImage;
    for (int i = 0; i < bufferSize - 2; i += 3) {
      float y = in[i];
      float cb = in[i + 1];
      float cr = in[i + 2];
      /*
	out[i+2]=clamp(y+1.403*cb);
	out[i+1]=clamp(y-0.344*cr-0.714*cb);
	out[i]=clamp(y+1.77*cr);
      */
      out[i] = clamp(1.164 * (y - 16) + 1.596 * (cr - 128));
      out[i + 1] = clamp(1.164 * (y - 16) - 0.813 * (cr - 128) -
			 0.392 * (cb - 128));
      out[i + 2] = clamp(1.164 * (y - 16) + 2.017 * (cb - 128));
    }
    return 1;
  }


  int
  convertJPEGtoYCrCb(const byte * source, int sourcelen, byte * dest,
		     int &size)
  {
    int sz;
    void *destination = read_jpeg((const char *) source, sourcelen, false, sz);
    if (!destination) {
      size = 0;
      return 0;
    }
    int cplen = sz > size ? size : sz;
    memcpy(dest, destination, cplen);
    free(destination);
    size = sz;
    return 1;
  }

  int
  convertJPEGtoRGB(const byte * source, int sourcelen, byte * dest, int &size)
  {
    int sz;
    void *destination = read_jpeg((const char *) source, sourcelen, true, sz);
    if (!destination) {
      size = 0;
      return 0;
    }
    int cplen = sz > size ? size : sz;
    memcpy(dest, destination, cplen);
    free(destination);
    size = sz;
    return 1;
  }

  int convertRGBtoJPEG(const byte* source,
		       int w, int h, byte* dest, int &size, int quality)
  {
    return write_jpeg(source, w, h, false, dest,size,quality);
  }


  int convertYCrCbtoJPEG(const byte* source,
			 int w, int h, byte* dest, int &size, int quality)
  {
    return write_jpeg(source, w, h, true, dest,size,quality);
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
      mem_source_mgr *src = (mem_source_mgr *) cinfo->src;
      if (num_bytes <= 0)
	return;
      if (num_bytes > src->pub.bytes_in_buffer)
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


  struct mem_destination_mgr{
    struct jpeg_destination_mgr pub;
  };

  void init_destination(j_compress_ptr) {
  }

  boolean empty_output_buffer(j_compress_ptr) {
    return FALSE;
  }

  void term_destination(j_compress_ptr) {
  }

  namespace
  {
    int
    write_jpeg(const unsigned char* src, int w, int h, bool ycrcb,
	       unsigned char* dst, int &sz, int quality)
    {
      struct jpeg_compress_struct cinfo;
      struct jpeg_error_mgr jerr;

      JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
      int row_stride;		/* physical row width in image buffer */

      cinfo.err = jpeg_std_error(&jerr);
      jpeg_create_compress(&cinfo);
      mem_destination_mgr *dest = (struct mem_destination_mgr *)
	(*cinfo.mem->alloc_small) ((j_common_ptr) & cinfo, JPOOL_PERMANENT,
				   sizeof(mem_destination_mgr));

      cinfo.dest = (jpeg_destination_mgr*)dest;
      dest->pub.init_destination=&init_destination;
      dest->pub.empty_output_buffer = &empty_output_buffer;
      dest->pub.term_destination = term_destination;
      dest->pub.free_in_buffer = sz;
      dest->pub.next_output_byte = dst;
      cinfo.image_width = w;
      cinfo.image_height = h;
      cinfo.input_components = 3;		/* # of color components per pixel */
      cinfo.in_color_space = ycrcb?JCS_YCbCr:JCS_RGB; /* colorspace of input image */

      jpeg_set_defaults(&cinfo);

      jpeg_set_quality(&cinfo,
		       quality, TRUE /* limit to baseline-JPEG values */);

      jpeg_start_compress(&cinfo, TRUE);

      row_stride = w * 3;	/* JSAMPLEs per row in image_buffer */

      while (cinfo.next_scanline < cinfo.image_height) {
	row_pointer[0] = (JSAMPLE *)& src[cinfo.next_scanline * row_stride];
	(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
      }

      jpeg_finish_compress(&cinfo);
      sz -= dest->pub.free_in_buffer ;
      jpeg_destroy_compress(&cinfo);

      return sz;
    }

    /*! Convert a jpeg image to YCrCb or RGB. Allocate the buffer with malloc.
     */
    void *read_jpeg(const char *jpgbuffer, int jpgbuffer_size, bool RGB,
		    int &output_size)
    {
      struct jpeg_decompress_struct cinfo;
      struct urbi_jpeg_error_mgr jerr;
      cinfo.err = jpeg_std_error(&jerr.pub);
      jerr.pub.error_exit = urbi_jpeg_error_exit;
      if (setjmp(jerr.setjmp_buffer)) {
	/* If we get here, the JPEG code has signaled an error.
	 * We need to clean up the JPEG object, close the input file, and return.
	 */
	jpeg_destroy_decompress(&cinfo);
	printf( "JPEG error!\n");
	return 0;
      }
      jpeg_create_decompress(&cinfo);
      mem_source_mgr *source = (struct mem_source_mgr *)
	(*cinfo.mem->alloc_small) ((j_common_ptr) & cinfo, JPOOL_PERMANENT,
				   sizeof(mem_source_mgr));

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
      output_size = cinfo.output_width *
	cinfo.output_components        *
	cinfo.output_height;
      void *buffer = malloc(output_size);

      while (cinfo.output_scanline < cinfo.output_height) {
	/* jpeg_read_scanlines expects an array of pointers to scanlines.
	 * Here the array is only one element long, but you could ask for
	 * more than one scanline at a time if that's more convenient.
	 */
	JSAMPROW row_pointer[1];
	row_pointer[0] = (JOCTET *) & ((char *) buffer)[cinfo.output_scanline   *
							cinfo.output_components *
							cinfo.output_width];
	jpeg_read_scanlines(&cinfo, row_pointer, 1);
      }
      jpeg_finish_decompress(&cinfo);
      jpeg_destroy_decompress(&cinfo);

      return buffer;
    }



    //scale putting (scx,scy) at the center of destination image
    void scaleColorImage(unsigned char * src, int sw, int sh,
			 int scx, int scy,
			 unsigned char * dst, int dw, int dh, float sx, float sy)
    {
      for (int x=0;x<dw;x++)
	for (int y=0;y<dh;y++) {
	  //find the corresponding point in source image
	  float fsrcx = (float) (x-dw/2) / sx  + (float)scx;
	  float fsrcy = (float) (y-dh/2) / sy  + (float)scy;
	  int srcx = (int) fsrcx;
	  int srcy = (int) fsrcy;
	  if ( srcx<=0 || srcx>=sw-1 || srcy<=0 || srcy>=sh-1)
	    memset(dst+(x+y*dw)*3,0,3);
	  else { //do the bilinear interpolation

	    float xfactor = fsrcx-(float)srcx;
	    float yfactor = fsrcy-(float)srcy;
	    for (int color=0;color<3;color++) {
	      float up = (float)src[(srcx+srcy*sw)*3+color] * (1.0-xfactor) + (float)src[(srcx+1+srcy*sw)*3+color] * xfactor;
	      float down = (float)src[(srcx+(srcy+1)*sw)*3+color] * (1.0-xfactor) + (float)src[(srcx+1+(srcy+1)*sw)*3+color] * xfactor;
	      float result = up * (1.0-yfactor) + down * yfactor;
	      dst[(x+y*dw)*3+color] = (unsigned char)result;
	    }
	  }
	}
    }

  } // anonymous namespace

  /** Convert between various image formats, takes care of everything
   */
  int convert(const UImage& src, UImage& dest)
  {
    if (dest.width == 0)
      dest.width = src.width;
    if (dest.height == 0)
      dest.height = src.height;
    //step 1: uncompress source, to have raw uncompressed rgb or ycbcr
    void * uncompressedData=malloc(src.width*src.height*3);
    int usz = src.width*src.height*3;
    int format; //0 rgb 1 ycbcr
    int targetformat; //0 rgb 1 ycbcr 2 compressed

    switch(dest.imageFormat)
      {
      case IMAGE_RGB:
      case IMAGE_PPM:
	targetformat = 1;
	break;
      case IMAGE_YCbCr:
	targetformat = 0;
	break;
      case IMAGE_JPEG:
	targetformat = -1;
	break;
      case IMAGE_UNKNOWN:
	break;
      }
    int p,c;
    switch(src.imageFormat)
      {
      case IMAGE_YCbCr:
	format = 1;
	memcpy(uncompressedData, src.data, src.width*src.height*3);
	break;
      case IMAGE_RGB:
	format = 0;
	memcpy(uncompressedData, src.data, src.width*src.height*3);
	break;
      case IMAGE_PPM:
	format = 0;
	//locate header end
	p=0;c=0;
	while(c<3)
	  if (src.data[p++]=='\n')
	    c++;
	memcpy(src.data+p, uncompressedData, src.width*src.height*3);
	break;
      case IMAGE_JPEG:
	if (targetformat==0) {
	  convertJPEGtoRGB((byte *)src.data,  src.size, (byte *)uncompressedData, usz);
	  format = 0;
	}
	else {
	  convertJPEGtoYCrCb((byte *)src.data,  src.size, (byte *)uncompressedData, usz);
	  format = 1;
	}
	break;
      case IMAGE_UNKNOWN:
	break;
      }


    //now resize if target size is different
    if (src.width != dest.width  ||  src.height != dest.height)
      {
	void * scaled = malloc(dest.width*dest.height*3);
	scaleColorImage((unsigned char *)uncompressedData, src.width, src.height, src.width/2, src.height/2,
			(unsigned char *)scaled, dest.width, dest.height,
			(float)dest.width/(float)src.width, (float)dest.height/(float) src.height);
	free(uncompressedData);
	uncompressedData = scaled;
      }

    //then convert to destination format
    dest.data = (unsigned char *)realloc(dest.data, dest.width*dest.height*3+20);
    dest.size =  dest.width*dest.height*3+20;

    switch(dest.imageFormat)
      {
      case IMAGE_RGB:
	if (format == 1)
	  convertYCrCbtoRGB((byte *)uncompressedData, dest.width*dest.height*3, (byte *)dest.data);
	else
	  memcpy(dest.data, uncompressedData, dest.width*dest.height*3);
	break;
      case IMAGE_YCbCr:
	if (format == 0)
	  convertRGBtoYCrCb((byte *)uncompressedData, dest.width*dest.height*3, (byte *)dest.data);
	else
	  memcpy(dest.data, uncompressedData, dest.width*dest.height*3);
	break;
      case IMAGE_PPM:
	sprintf((char*)dest.data, "P6\n%d %d\n255\n", dest.width, dest.height);
	if (format == 1)
	  convertYCrCbtoRGB((byte *)uncompressedData, dest.width*dest.height*3, (byte *)dest.data+strlen((char*)dest.data));
	else
	  memcpy(dest.data+strlen((char*)dest.data), uncompressedData, dest.width*dest.height*3);
	break;
      case IMAGE_JPEG:
	/*
	  if (format == 1)
	  convertYCrCbtoJPEG((byte *)uncompressedData, dest.width*dest.height*3, (byte *)dest.data, dsz);
	  else
	  convertRGBtoJPEG((byte *)uncompressedData, dest.width*dest.height*3, (byte *)dest.data, dsz);
	*/
	fprintf(stderr,"unsoported conversion requested: cant compress to jpeg\n");
	free(uncompressedData);
	return 0;
	break;
      case IMAGE_UNKNOWN:
	break;
      }

    free(uncompressedData);
    return 1;
  }

} // namespace urbi

#endif // !NO_IMAGE_CONVERSION

namespace urbi
{
  struct wavheader
  {
    char riff[4];
    int length;
    char wave[4];
    char fmt[4];
    int lnginfo;
    short one;
    short channels;
    int freqechant;
    int bytespersec;
    short bytesperechant;
    short bitperchannel;
    char data[4];
    int datalength;
  };

  template<class S, class D>
  void copy(S* src, D* dst,
	    int sc, int dc, int sr, int dr, int count, bool sf, bool df)
  {
    int shift = 8*(sizeof(S) - sizeof(D));
    for (int i=0;i<count;i++) {
      float soffset = (float)i* ((float)sr /(float)dr);
      int so = (int)soffset;
      float factor = soffset-(float)so;
      S s1, s2;
      s1 = src[so*sc];
      if (i != count - 1)
	s2 = src[(so+1)*sc];
      else
	s2 = s1; //nothing to interpolate with
      if (!sf) {
	s1 = s1 ^ (1<<(sizeof(S)*8-1));
	s2 = s2 ^ (1<<(sizeof(S)*8-1));
      }
      int v1 = (int) ((float)(s1)*(1.0-factor) + (float)(s2)*factor);
      int v2;
      if (sc==1)
	v2 = v1;
      else {
	s1 = src[so*sc+1];
	if (i != count - 1)
	  s2 = src[(so+1)*sc+1];
	else
	  s2 = s1; //nothing to interpolate with
	if (!sf) {
	  s1 = s1 ^ (1<<(sizeof(S)*8-1));
	  s2 = s2 ^ (1<<(sizeof(S)*8-1));
	}
	v2 = (int) ((float)(s1)*(1.0-factor) + (float)(s2)*factor);
      }
      D d1, d2;
      if (shift>=0) {
	d1 = (D)(v1 >>shift);
	d2 = (D)(v2 >>shift);
      }
      else {
	d1 = (D)(v1) *  (1<< (-shift));
	d2 = (D)(v2) * (1<< (-shift));
      }
      if (!df) {
	d1 = d1 ^ (1<<(sizeof(D)*8-1));
	d2 = d2 ^ (1<<(sizeof(D)*8-1));
      }
      if (dc==2) {
	dst[i*2] = d1;
	dst[i*2+1] = d2;
      }
      else
	dst[i] = (D) (((int)d1+(int)d2) /2);
    }
  }

  /** Conversion between various sound formats.
      If any of destination,'s channel, sampleSize, rate or sampleFormat parameter is 0, values from source will be used.
      If the desitnation's datasize is too small, data will be realloc()ed, which means one can set data and datasize to zero, and let convert allocate the memory.
  */
  int
  convert (const USound &source, USound &dest)
  {
    if ((source.soundFormat != SOUND_RAW
	 && source.soundFormat != SOUND_WAV)
	||
	(dest.soundFormat != SOUND_RAW
	 && dest.soundFormat != SOUND_WAV))
      return 1; //conversion not handled yet
    //phase one: calculate required buffer size, set destination unspecified fields
    int schannels, srate, ssampleSize;
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
      dest.sampleFormat = ((dest.sampleSize>8)
			   ? SAMPLE_SIGNED:SAMPLE_UNSIGNED);
    int destSize = (int) (( (long long)(source.size- ((source.soundFormat == SOUND_WAV)?44:0)) * (long long)dest.channels * (long long)dest.rate * (long long)(dest.sampleSize/8)) / ( (long long)schannels*(long long)srate*(long long)(ssampleSize/8)));
    if (dest.soundFormat == SOUND_WAV)
      destSize+= sizeof(wavheader);
    if (dest.size<destSize)
      dest.data = (char *)realloc(dest.data, destSize);
    dest.size = destSize;
    //write destination header if appropriate
    if (dest.soundFormat == SOUND_WAV) {
      wavheader * wh = (wavheader *)dest.data;
      memcpy(wh->riff,"RIFF",4);
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
      wh->datalength = destSize - sizeof(wavheader);
    }

    //do the conversion and write to dest.data
    char * sbuffer = source.data;
    if (source.soundFormat == SOUND_WAV)
      sbuffer += sizeof(wavheader);
    char * dbuffer = dest.data;
    if (dest.soundFormat == SOUND_WAV)
      dbuffer += sizeof(wavheader);
    int elementCount = dest.size - ((dest.soundFormat == SOUND_WAV)?sizeof(wavheader):0);
    elementCount /= (dest.channels * (dest.sampleSize/8));
    switch( ssampleSize*1000 + dest.sampleSize) {
    case 8008:
      copy(sbuffer, dbuffer, schannels, dest.channels, srate, dest.rate, elementCount, ssampleFormat==SAMPLE_SIGNED, dest.sampleFormat == SAMPLE_SIGNED);
      break;
    case 16008:
      copy((short *)sbuffer, dbuffer, schannels, dest.channels, srate, dest.rate, elementCount, ssampleFormat==SAMPLE_SIGNED, dest.sampleFormat == SAMPLE_SIGNED);
      break;
    case 16016:
      copy((short *)sbuffer, (short *)dbuffer, schannels, dest.channels, srate, dest.rate, elementCount, ssampleFormat==SAMPLE_SIGNED, dest.sampleFormat == SAMPLE_SIGNED);
      break;
    case 8016:
      copy((char *)sbuffer, (short *)dbuffer, schannels, dest.channels, srate, dest.rate, elementCount, ssampleFormat==SAMPLE_SIGNED, dest.sampleFormat == SAMPLE_SIGNED);
      break;
    }
    return 0;
  }

} // namespace urbi
