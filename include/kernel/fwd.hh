/// \file   kernel/fwd.hh
/// \brief  Forward declarations.

#ifndef KERNEL_FWD_HH
# define KERNEL_FWD_HH

# include <libport/intrusive-ptr.hh>

// Do not use headers such as ast/fwd.hh since they are not shipped.
// Duplicate forward declarations are therefore needed.

class UGenericCallback;

namespace ast
{
  class Nary;
  typedef libport::intrusive_ptr<Nary> rNary;
}

namespace kernel
{
  class UConnection;
  class UGhostConnection;
  class UQueue;
  class UServer;
  struct ConnectionSet;
}

namespace urbi
{
  class UBinary;
  class UGenericCallback;
  class UImage;
  class UList;
  class USound;
  class USystem;
  class UValue;
  class baseURBIStarter;
}

namespace object
{
  class Lobby;
  template<class T> class Atom;
  typedef libport::intrusive_ptr<Lobby> rLobby;
}

namespace parser
{
  class UParser;
}


namespace runner
{
  class Runner;

  class Shell;
  typedef libport::intrusive_ptr<Shell> rShell;
}

namespace sched
{
  class Scheduler;
}

#endif // !KERNEL_FWD_HH
