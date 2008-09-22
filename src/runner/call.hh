#ifndef OBJECT_CALL_HH
# define OBJECT_CALL_HH

# include <libport/symbol.hh>
# include <object/fwd.hh>
# include <runner/fwd.hh>

namespace object
{
  /// Helpers to call Urbi functions from C++.

  // self.'msg'(args)
  rObject urbi_call(runner::Runner& r,
		    rObject self,
                    libport::Symbol msg,
                    objects_type& args);

  // self.'msg'(args) for up to five arguments
  rObject urbi_call(runner::Runner& r,
		    rObject self,
		    libport::Symbol msg,
		    rObject arg1 = 0,
		    rObject arg2 = 0,
		    rObject arg3 = 0,
		    rObject arg4 = 0,
		    rObject arg5 = 0);

  // owner.getSlot(msg).apply([self])
  rObject urbi_call_function(runner::Runner& r, rObject self,
                             rObject function_owner, libport::Symbol msg);
  // owner.getSlot(msg).apply([self, args])
  rObject urbi_call_function(runner::Runner& r,
                             rObject self,
                             rObject function_owner,
                             libport::Symbol msg,
                             objects_type& args);
}

#endif
