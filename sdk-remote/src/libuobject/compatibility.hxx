#include <urbi/uobject.hh>

namespace urbi
{
  namespace compatibility
  {
    inline
    std::string
    emit(const std::string& event)
    {
      return 2 <= kernelMajor() ? event + "!" : "emit " + event;
    }

    inline
    std::string
    isvoid(const std::string& exp)
    {
      return 2 <= kernelMajor() ?
        "(" + exp + ").isVoid" : "isvoid(" + exp + ")";
    }
  }
}
