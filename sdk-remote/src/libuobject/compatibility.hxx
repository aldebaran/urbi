#include <urbi/uobject.hh>

namespace urbi
{

  inline
  std::string
  emit(const std::string& event)
  {
    return 2 <= kernelMajor() ? event + "!" : "emit " + event;
  }

}
