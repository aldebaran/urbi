/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <ast/parametric-ast.hh>
#include <urbi/object/any-to-boost-function.hh>
#include <urbi/object/barrier.hh>
#include <urbi/object/centralized-slots.hh>
#include <urbi/object/code.hh>
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
#include <urbi/object/file.hh>
#include <urbi/object/float.hh>
#include <urbi/object/fwd.hh>
#include <urbi/object/global.hh>
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

#include <object/finalizable.hh>
#include <object/format-info.hh>
#include <object/formatter.hh>
#include <object/input-stream.hh>
#include <object/ioservice.hh>
#include <object/output-stream.hh>
#include <object/process.hh>
#include <object/regexp.hh>
#include <object/semaphore.hh>
#include <object/server.hh>
#include <object/socket.hh>
#include <object/stream.hh>
#include <object/symbols.hh>
#include <object/system.hh>
#include <object/uvalue.hh>
#include <object/uvar.hh>


namespace urbi
{
  namespace object
  {

    URBI_CXX_OBJECT_REGISTER(Executable)
    {}

    static rObject nil(const objects_type&)
    {
      return void_class;
    }

    URBI_CXX_OBJECT_REGISTER(Primitive)
      : content_()
    {
      content_ << boost::function1<rObject, const objects_type&>(nil);
      proto_add(Executable::proto);
      proto_remove(Object::proto);
    }

    URBI_CXX_OBJECT_REGISTER(String)
    {}

#if !defined WIN32
    URBI_CXX_OBJECT_REGISTER(Process)
      : name_(libport::path("true").basename())
      , pid_(0)
      , binary_("/bin/true")
      , argv_()
      , status_(-1)
    {
      argv_ << binary_;
    }
#endif

#if !defined COMPILATION_MODE_SPACE
    URBI_CXX_OBJECT_REGISTER(Regexp)
      : re_(".")
    {}

    URBI_CXX_OBJECT_REGISTER(Formatter)
    {}

    URBI_CXX_OBJECT_REGISTER(FormatInfo)
      : alignment_(Align::RIGHT)
      , alt_(false)
      , consistent_(true)
      , group_("")
      , pad_(" ")
      , pattern_("%s")
      , precision_(6)
      , prefix_("")
      , spec_("s")
      , uppercase_(Case::UNDEFINED)
      , width_(0)
    {}
#endif

    URBI_CXX_OBJECT_REGISTER(UValue)
    {}


    URBI_CXX_OBJECT_REGISTER(Tag)
      : value_(new sched::Tag(libport::Symbol::make_empty()))
    {}

    URBI_CXX_OBJECT_REGISTER(Server)
      : libport::Socket(*object::Socket::get_default_io_service().get())
      , io_service_(object::Socket::get_default_io_service())
    {}

    URBI_CXX_OBJECT_REGISTER(Socket)
      : libport::Socket(*get_default_io_service().get())
    {
      io_service_ = get_default_io_service();
    }

    URBI_CXX_OBJECT_REGISTER(Semaphore)
    {}

    URBI_CXX_OBJECT_REGISTER(Code)
    {
      PARAMETRIC_AST(ast, "function () {}");
      ast_ = ast.result<const ast::Routine>();
      proto_add(Executable::proto);
      proto_remove(Object::proto);
    }

    URBI_CXX_OBJECT_REGISTER(UVar)
      : Primitive( boost::bind(&UVar::accessor, this))
      , looping_(false)
      , inAccess_(false)
    {}

    URBI_CXX_OBJECT_REGISTER(Position)
      : pos_()
    {}

    URBI_CXX_OBJECT_REGISTER(Location)
      : loc_()
    {}


    URBI_CXX_OBJECT_REGISTER(Lobby)
      : connection_(0)
    {}

    URBI_CXX_OBJECT_REGISTER(List)
    {}

    URBI_CXX_OBJECT_REGISTER(Event)
    {
      typedef void (Event::*emit_type)(const objects_type& args);
      bind(SYMBOL(emit), static_cast<emit_type>(&Event::emit));
      bind(SYMBOL(hasSubscribers), &Event::hasSubscribers);
      bind(SYMBOL(localTrigger), &Event::localTrigger);
      typedef
      void (Event::*on_event_type)
      (rExecutable guard, rExecutable enter, rExecutable leave);
      bind(SYMBOL(onEvent), static_cast<on_event_type>(&Event::onEvent));
      bind(SYMBOL(stop), &Event::stop);
      bind(SYMBOL(syncEmit), &Event::syncEmit);
      bind(SYMBOL(syncTrigger), &Event::syncTrigger);
      bind(SYMBOL(trigger), &Event::trigger);
      bind(SYMBOL(waituntil), &Event::waituntil);
    }

    URBI_CXX_OBJECT_REGISTER(Job)
    {}

    URBI_CXX_OBJECT_REGISTER(IoService)
    {
    }

    URBI_CXX_OBJECT_REGISTER(Float)
      : value_(0)
    {}

    URBI_CXX_OBJECT_REGISTER(Duration)
      : Float(0)
    {
      proto_add(Float::proto);

      bind(SYMBOL(asPrintable), &Duration::asPrintable);
      bind(SYMBOL(asString), &Duration::as_string);
      bind(SYMBOL(init), &Duration::init);
      bind(SYMBOL(seconds), &Duration::seconds);
    }

    URBI_CXX_OBJECT_REGISTER(Finalizable)
    {}

    URBI_CXX_OBJECT_REGISTER(File)
      : path_(new Path("/"))
    {}

    URBI_CXX_OBJECT_REGISTER(Path)
      : path_(WIN32_IF("C:\\", "/"))
    {}

    URBI_CXX_OBJECT_REGISTER(Directory)
      : path_(new Path())
    {}

#if !defined COMPILATION_MODE_SPACE
    URBI_CXX_OBJECT_REGISTER(OutputStream)
      : Stream(STDOUT_FILENO, false)
    {}

    URBI_CXX_OBJECT_REGISTER(InputStream)
      : Stream(STDIN_FILENO, false)
      , pos_(0)
      , size_(0)
    {}
#endif

    URBI_CXX_OBJECT_REGISTER(Dictionary)
    {}

    URBI_CXX_OBJECT_REGISTER(Date)
      : time_(boost::posix_time::microsec_clock::local_time())
    {}

    URBI_CXX_OBJECT_REGISTER(Barrier)
    {}
  }
}
