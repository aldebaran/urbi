#ifndef OBJECT_EXECUTABLE_HH
# define OBJECT_EXECUTABLE_HH

# include <object/cxx-object.hh>

namespace object
{
  class Executable: public CxxObject
  {
  public:
    virtual rObject operator() (object::objects_type args) = 0;
  };
}

# include <object/cxx-object.hxx>

#endif
