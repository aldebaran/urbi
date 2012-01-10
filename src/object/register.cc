/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <boost/assign.hpp>

#include <ast/parametric-ast.hh>
#include <libport/thread.hh>
#include <urbi/object/any-to-boost-function.hh>
#include <urbi/object/barrier.hh>
#include <urbi/object/centralized-slots.hh>
#include <object/code.hh>
#include <urbi/object/cxx-conversions.hh>
#include <urbi/object/cxx-object.hh>
#include <urbi/object/cxx-primitive.hh>
#include <urbi/object/date.hh>
#include <urbi/object/dictionary.hh>
#include <urbi/object/directory.hh>
#include <urbi/object/duration.hh>
#include <urbi/object/enumeration.hh>
#include <urbi/object/equality-comparable.hh>
#include <urbi/object/event.hh>
#include <urbi/object/event-handler.hh>
#include <urbi/object/file.hh>
#include <urbi/object/float.hh>
#include <urbi/object/fwd.hh>
#include <urbi/object/global.hh>
#include <urbi/object/hash.hh>
#include <urbi/object/job.hh>
#include <urbi/object/list.hh>
#include <urbi/object/lobby.hh>
#include <urbi/object/location.hh>
#include <urbi/object/object.hh>
#include <urbi/object/path.hh>
#include <urbi/object/position.hh>
#include <urbi/object/primitive.hh>
#include <urbi/object/slot.hh>
#include <urbi/object/string.hh>
#include <urbi/object/tag.hh>
#include <urbi/sdk.hh>

#include <object/uconnection.hh>
#include <object/finalizable.hh>
#include <object/ioservice.hh>
#include <object/profile.hh>
#include <object/semaphore.hh>
#include <object/server.hh>
#include <object/socket.hh>
#include <urbi/object/symbols.hh>
#include <object/system.hh>
#include <object/uvalue.hh>
#include <object/uvar.hh>

namespace urbi
{
  namespace object
  {
    URBI_CXX_OBJECT_REGISTER(Executable);
    URBI_CXX_OBJECT_REGISTER(Primitive);
    URBI_CXX_OBJECT_REGISTER(String);
    URBI_CXX_OBJECT_REGISTER(UValue);
    URBI_CXX_OBJECT_REGISTER(Tag);
    URBI_CXX_OBJECT_REGISTER(Server);
    URBI_CXX_OBJECT_REGISTER(Socket);
    URBI_CXX_OBJECT_REGISTER(Semaphore);
    URBI_CXX_OBJECT_REGISTER(Code);
    URBI_CXX_OBJECT_REGISTER(UVar);
    URBI_CXX_OBJECT_REGISTER(Position);
    URBI_CXX_OBJECT_REGISTER(Location);
    URBI_CXX_OBJECT_REGISTER(Lobby);
    URBI_CXX_OBJECT_REGISTER(List);
    URBI_CXX_OBJECT_REGISTER(Event);
    URBI_CXX_OBJECT_REGISTER(EventHandler);
    URBI_CXX_OBJECT_REGISTER(Job);
    URBI_CXX_OBJECT_REGISTER(IoService);
    URBI_CXX_OBJECT_REGISTER(Float);
    URBI_CXX_OBJECT_REGISTER(Duration);
    URBI_CXX_OBJECT_REGISTER(Finalizable);
    URBI_CXX_OBJECT_REGISTER(File);
    URBI_CXX_OBJECT_REGISTER(Path);
    URBI_CXX_OBJECT_REGISTER(Directory);
    URBI_CXX_OBJECT_REGISTER(Dictionary);
    URBI_CXX_OBJECT_REGISTER(Date);
    URBI_CXX_OBJECT_REGISTER(Barrier);
    URBI_CXX_OBJECT_REGISTER(Hash);
    URBI_CXX_OBJECT_REGISTER(UConnection);
    URBI_CXX_OBJECT_REGISTER(Profile);
    URBI_CXX_OBJECT_REGISTER(FunctionProfile);
  } // namespace object
} // namespace urbi


/*===============================================\
| Objects compiled only when not in space mode.  |
\===============================================*/

#if !defined COMPILATION_MODE_SPACE

# include <object/format-info.hh>
# include <object/formatter.hh>

namespace urbi
{
  namespace object
  {
    URBI_CXX_OBJECT_REGISTER(FormatInfo);
    URBI_CXX_OBJECT_REGISTER(Formatter);
  } // namespace object
} // namespace urbi
#endif
