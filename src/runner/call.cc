#include <libport/contract.hh>

#include <kernel/userver.hh>
#include <object/object.hh>
#include <runner/call.hh>
#include <runner/runner.hh>

namespace object
{
  rObject
  urbi_call(rObject self,
            libport::Symbol msg,
            objects_type& args)
  {
    return urbi_call_function(self, self, msg, args);
  }

#define CHECK_ARG(N)				\
  if (!arg ## N)				\
    goto done;					\
  args.push_back(arg ## N)

  rObject
  urbi_call(rObject self,
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
    return urbi_call_function(self, self, msg, args);
  }

#undef CHECK_ARG

  rObject
  urbi_call_function(rObject self,
                     rObject owner,
                     libport::Symbol msg)
  {
    objects_type args; // Call with no args.
    return urbi_call_function(self, owner, msg, args);
  }

  rObject
  urbi_call_function(rObject self,
                     rObject owner,
                     libport::Symbol msg,
                     objects_type& args)
  {
    runner::Runner& r = ::urbiserver->getCurrentRunner();

    assert(self);
    rObject res = r.apply(self, owner->slot_get(msg), msg, args);
    return iassertion(res);
  }
}
