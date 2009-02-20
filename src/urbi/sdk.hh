#ifndef URBI_SDK_HH
# define URBI_SDK_HH

# include <object/cxx-conversions.hh>
# include <object/cxx-object.hh>
# include <object/object.hh>
# include <object/slot.hh>

namespace urbi
{
  /*-------------.
  | Import types |
  `-------------*/

  using object::Object;
  using object::CxxObject;
  using object::rObject;
  using object::Slot;

  // FIXME: function isn't ideal
  inline
  rObject global();
}

# include <urbi/sdk.hxx>

#endif
