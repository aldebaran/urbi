#ifndef URBI_SDK_HH
# define URBI_SDK_HH

# include <object/cxx-conversions.hh>
# include <object/cxx-object.hh>
# include <object/object.hh>
# include <object/slot.hh>
# include <runner/call.hh>

namespace urbi
{
  /*-------------.
  | Import types |
  `-------------*/

  using object::Object;
  using object::CxxObject;
  using object::rObject;
  using object::Slot;

  /*----------------------.
  | Import call functions |
  `----------------------*/

  inline
  rObject call(rObject self,
               libport::Symbol msg,
               rObject arg1 = 0,
               rObject arg2 = 0,
               rObject arg3 = 0,
               rObject arg4 = 0,
               rObject arg5 = 0);

  // FIXME: function isn't ideal
  inline
  rObject global();
}

# include <urbi/sdk.hxx>

#endif
