#ifndef URBI_SDK_HH
# define URBI_SDK_HH

# include <object/cxx-object.hh>
# include <runner/call.hh>
# include <urbi/export.hh>

namespace urbi
{
  /*-------------.
  | Import types |
  `-------------*/

  using object::Object;
  using object::CxxObject;
  using object::rObject;


  /*----------------------.
  | Import call functions |
  `----------------------*/

  URBI_SDK_API
  inline
  rObject call(rObject self,
               libport::Symbol msg,
               rObject arg1 = 0,
               rObject arg2 = 0,
               rObject arg3 = 0,
               rObject arg4 = 0,
               rObject arg5 = 0);

  URBI_SDK_API
  inline
  rObject global();
}

# include <urbi/sdk.hxx>

#endif
