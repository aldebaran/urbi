#include <runner/call.hh>

namespace object
{
  rObject
  urbi_call(runner::Runner& r,
            rObject self,
            libport::Symbol msg,
            objects_type& args)
  {
    return urbi_call_function(r, self, self, msg, args);
  }

#define CHECK_ARG(N)				\
  if (!arg ## N)				\
    goto done;					\
  args.push_back(arg ## N)

  rObject
  urbi_call(runner::Runner& r,
            rObject self,
            libport::Symbol msg,
            rObject arg1,
            rObject arg2,
            rObject arg3,
            rObject arg4,
            rObject arg5)
  {
    objects_type args;
    CHECK_ARG(1);
    CHECK_ARG(2);
    CHECK_ARG(3);
    CHECK_ARG(4);
    CHECK_ARG(5);
  done:
    return urbi_call_function(r, self, self, msg, args);
  }

#undef CHECK_ARG

  rObject
  urbi_call_function(runner::Runner& r,
                     rObject self,
                     rObject owner,
                     libport::Symbol msg)
  {
    objects_type args; // Call with no args.
    return urbi_call_function(r, self, owner, msg, args);
  }

  rObject
  urbi_call_function(runner::Runner& r,
                     rObject self,
                     rObject owner,
                     libport::Symbol msg,
                     objects_type& args)
  {
    assert(self);
    rObject message = owner->slot_get(msg);
    if (!message)
      throw LookupError(msg);
    rObject res = r.apply(self, message, msg, args);
    assert(res);
    return res;
  }
}
