/*
 * Copyright (C) 2006-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file   urbi/kernel/fwd.hh
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
  class UServer;
  class ConnectionSet;
}

namespace urbi
{
  class UBinary;
  class UGenericCallback;
  class UImage;
  class UList;
  class USound;
  class UValue;
  class baseURBIStarter;
}

namespace urbi
{
  namespace object
  {
    class Lobby;
    typedef libport::intrusive_ptr<Lobby> rLobby;
  }
}

namespace parser
{
  class UParser;
}

namespace runner
{
  class Interpreter;
  class Runner;

  class Shell;
  typedef libport::intrusive_ptr<Shell> rShell;
}

namespace sched
{
  class Scheduler;
}

#endif // !KERNEL_FWD_HH
