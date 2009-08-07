#ifndef URBI_SDK_HH
# define URBI_SDK_HH

# include <libport/utime.hh>

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

  void yield();
  void yield_until(libport::utime_t t);
  void yield_for(libport::utime_t t);
  void yield_for_fd(int fd);

  // FIXME: function isn't ideal
  inline
  rObject global();
}

# include <urbi/sdk.hxx>

#endif
