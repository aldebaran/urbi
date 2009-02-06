#ifndef OBJECT_CALL_HH
# define OBJECT_CALL_HH

# include <libport/symbol.hh>
# include <object/fwd.hh>

namespace object
{
  /// Helpers to call Urbi functions from C++.

  // self.'msg'(args)
  URBI_SDK_API
  rObject urbi_call(libport::Symbol msg,
                    objects_type& args);

  // self.'msg'(args) for up to five arguments
  URBI_SDK_API
  rObject urbi_call(rObject self,
		    libport::Symbol msg,
		    rObject arg1 = 0,
		    rObject arg2 = 0,
		    rObject arg3 = 0,
		    rObject arg4 = 0,
		    rObject arg5 = 0);

  // owner.getSlot(msg).apply([self])
  URBI_SDK_API
  rObject urbi_call_function(rObject self,
                             rObject function_owner,
                             libport::Symbol msg);

  // owner.getSlot(msg).apply([self, args])
  URBI_SDK_API
  rObject urbi_call_function(rObject function_owner,
                             libport::Symbol msg,
                             objects_type& args);
}

#endif
