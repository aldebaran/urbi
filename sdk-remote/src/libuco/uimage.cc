/// \file libuco/uimage.cc

#include <urbi/uimage.hh>

namespace urbi
{

  const char*
  UImage::format_string() const
  {
    switch (imageFormat)
    {
    case IMAGE_RGB:
      return "rgb";
    case IMAGE_JPEG:
      return "jpeg";
    case IMAGE_YCbCr:
      return "YCbCr";
    case IMAGE_PPM:
      return "ppm";
    case IMAGE_UNKNOWN:
      return "unknown format";
    }
    // To pacify "warning: control reaches end of non-void function".
    // pabort(imageFormat);
    // FIXME: This should not abort. UImage should be initialized with IMAGE_UKNOWN.
    //        This is not done because data is stored in an union in UBinary and
    //        union members cannot have constructors.
    return "unknown format";
  }

} // namespace urbi
