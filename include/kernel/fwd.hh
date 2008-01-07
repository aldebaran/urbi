/// \file   kernel/fwd.hh
/// \brief  Forward declarations.

#ifndef KERNEL_FWD_HH
# define KERNEL_FWD_HH

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
};

class UCommandQueue;
class UConnection;
class UContext;

class UGenericCallback;
class UGhostConnection;
class UImage;
class UList;
class UMonitor;
class UMultiEventInstance;
class UParser;
class UQueue;
class UServer;
class USound;
class UString;
class UTest;
class UWaitCounter;

// FIXME: Should not be here, but we don't want to export ast/ either.
namespace ast
{
  class Ast;
}

#endif // !KERNEL_FWD_HH
