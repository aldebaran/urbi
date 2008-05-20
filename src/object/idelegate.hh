#ifndef OBJECT_IDELEGATE_HH
# define OBJECT_IDELEGATE_HH

#include "runner/runner.hh"

namespace object
{
  class IDelegate
  {
  public:
    virtual rObject operator() (runner::Runner&, objects_type) = 0;
    virtual ~IDelegate() {}
  };
}

#endif
