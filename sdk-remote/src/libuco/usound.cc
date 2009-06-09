#include <urbi/usound.hh>

namespace urbi
{
  bool
  USound::operator==(const USound &b) const
  {
    return !memcmp(this, &b, sizeof(USound));
  }
}
