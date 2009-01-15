#ifndef OBJECT_SLOTS_HH
# define OBJECT_SLOTS_HH

# include <boost/function.hpp>

# include <libport/symbol.hh>

# include <object/fwd.hh>
# include <urbi/export.hh>

namespace object
{
  class URBI_SDK_API Slots
  {
    public:
      /// The keys
      typedef libport::Symbol key_type;
      /// The values held by slots
      typedef object::rObject value_type;
      /// The slots
      typedef std::pair<const key_type, value_type> slot_type;
  };
}
#endif
