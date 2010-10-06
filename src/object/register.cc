/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <boost/assign.hpp>

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
      extend(nil);
      proto_add(Executable::proto);
      proto_remove(Object::proto);
    }

    // FIXME: kill this when overloading is fully supported
    typedef std::vector<std::string> strings_type;
    OVERLOAD_TYPE(
      split_overload, 4, 1,
      String,
      (strings_type (String::*)(const std::string&, int, bool, bool) const)
      &String::split,
      List,
      (strings_type (String::*)(const strings_type&, int, bool, bool) const)
      &String::split)

    // FIXME: kill this when overloading is fully supported
    static rObject split_bouncer(const objects_type& _args)
    {
      objects_type args = _args;
      static rPrimitive actual = new Primitive(split_overload);
      check_arg_count(args.size() - 1, 0, 4);
      switch (args.size())
      {
        case 1:
        {
          static strings_type seps =
            boost::assign::list_of(" ")("\t")("\r")("\n");
          args << to_urbi(seps)
               << new Float(-1)
               << object::false_class
               << object::false_class;
          break;
        }
        case 2:
          args << new Float(-1);
        case 3:
          args << object::false_class;
        case 4:
          args << object::true_class;
        default:
          break;
      }
      if (args[2] == object::nil_class)
        args[2] = new Float(-1);
      return (*actual)(args);
    }

    URBI_CXX_OBJECT_REGISTER(String)
    {
      bind_variadic(SYMBOL(split), split_bouncer);
    }

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
    {
      bind(SYMBOL(init),  static_cast<void (Tag::*)(                      )>(&Tag::init ));
      bind(SYMBOL(init),  static_cast<void (Tag::*)(const libport::Symbol&)>(&Tag::init ));
      bind(SYMBOL(stop),  static_cast<void (Tag::*)(                      )>(&Tag::stop ));
      bind(SYMBOL(stop),  static_cast<void (Tag::*)(rObject               )>(&Tag::stop ));
      bind(SYMBOL(block), static_cast<void (Tag::*)(                      )>(&Tag::block));
      bind(SYMBOL(block), static_cast<void (Tag::*)(rObject               )>(&Tag::block));
      bind_variadic(SYMBOL(newFlowControl), &Tag::new_flow_control);
    }

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
      bind_variadic<rObject, Code>(SYMBOL(apply), &Code::apply);
    }

    URBI_CXX_OBJECT_REGISTER(UVar)
      : Primitive(boost::function1<rObject, objects_type>(boost::bind(&UVar::accessor, this, _1)))
      , looping_(false)
      , inAccess_(false)
    {}

    URBI_CXX_OBJECT_REGISTER(Position)
      : pos_()
    {
      bind(SYMBOL(init), static_cast<void (Position::*)()>(&Position::init));
      bind(SYMBOL(init), static_cast<void (Position::*)(const value_type& pos)>(&Position::init));
      bind(SYMBOL(init), static_cast<void (Position::*)(unsigned from, unsigned to)>(&Position::init));
      bind(SYMBOL(init), static_cast<void (Position::*)(rObject file, unsigned from, unsigned to)>(&Position::init));
    }

    URBI_CXX_OBJECT_REGISTER(Location)
      : loc_()
    {
      // For some reason, cl.exe refuses "&Location::value_type::begin",
      // with error: function cannot access 'yy::location::begin'
      bind(SYMBOL(begin), &yy::location::begin);
      bind(SYMBOL(end),   &yy::location::end);
      bind(SYMBOL(init),  static_cast<void (Location::*)()>(&Location::init));
      bind(SYMBOL(init),  static_cast<void (Location::*)(const Position::value_type&)>(&Location::init));
      bind(SYMBOL(init),  static_cast<void (Location::*)(const Position::value_type&, const Position::value_type&)>(&Location::init));
    }


    URBI_CXX_OBJECT_REGISTER(Lobby)
      : connection_(0)
    {
      bind(SYMBOL(send),
           static_cast<void (Lobby::*)(const std::string&)>(&Lobby::send));
      bind(SYMBOL(send),
           static_cast<void (Lobby::*)(const std::string&, const std::string&)>(&Lobby::send));
    }

    URBI_CXX_OBJECT_REGISTER(List)
    {
      bind(SYMBOL(sort), static_cast<List::value_type (List::*)()>(&List::sort));
      bind(SYMBOL(sort), static_cast<List::value_type (List::*)(rObject)>(&List::sort));
    }

    URBI_CXX_OBJECT_REGISTER(Event)
    {
      typedef void (Event::*emit_type)(const objects_type& args);
      bind_variadic<void, Event>(SYMBOL(emit), static_cast<emit_type>(&Event::emit));
      bind(SYMBOL(hasSubscribers), &Event::hasSubscribers);
      bind(SYMBOL(localTrigger), &Event::localTrigger);
      typedef
      void (Event::*on_event_type)
      (rExecutable guard, rExecutable enter, rExecutable leave);
      bind(SYMBOL(onEvent), static_cast<on_event_type>(&Event::onEvent));
      bind(SYMBOL(stop), &Event::stop);
      bind_variadic<void, Event>(SYMBOL(syncEmit), &Event::syncEmit);
      bind_variadic<rEvent, Event>(SYMBOL(syncTrigger), &Event::syncTrigger);
      bind_variadic<rEvent, Event>(SYMBOL(trigger), &Event::trigger);
      bind(SYMBOL(waituntil), &Event::waituntil);
    }

    URBI_CXX_OBJECT_REGISTER(Job)
    {}

    URBI_CXX_OBJECT_REGISTER(IoService)
    {
    }

    URBI_CXX_OBJECT_REGISTER(Float)
      : value_(0)
    {
      bind(SYMBOL(PLUS),   static_cast<Float::value_type (Float::*)() const>(&Float::plus));
      bind(SYMBOL(PLUS),   static_cast<Float::value_type (Float::*)(Float::value_type) const>(&Float::plus));
      bind(SYMBOL(MINUS),  static_cast<Float::value_type (Float::*)() const>(&Float::minus));
      bind(SYMBOL(MINUS),  static_cast<Float::value_type (Float::*)(Float::value_type) const>(&Float::minus));
    }

    URBI_CXX_OBJECT_REGISTER(Duration)
      : Float(0)
    {
      proto_add(Float::proto);

      bind(SYMBOL(asPrintable), &Duration::asPrintable);
      bind(SYMBOL(asString),    &Duration::as_string);
      bind(SYMBOL(init),        static_cast<void (Duration::*)()>(&Duration::init));
      bind(SYMBOL(init),        static_cast<void (Duration::*)(const Duration::value_type&)>(&Duration::init));
      bind(SYMBOL(seconds),     &Duration::seconds);
    }

    URBI_CXX_OBJECT_REGISTER(Finalizable)
    {}

    URBI_CXX_OBJECT_REGISTER(File)
      : path_(new Path("/"))
    {}

    OVERLOAD_TYPE(concat_bouncer, 1, 1,
                  Path, &Path::path_concat,
                  String, &Path::string_concat);

    URBI_CXX_OBJECT_REGISTER(Path)
      : path_(WIN32_IF("C:\\", "/"))
    {
      bind_variadic(SYMBOL(SLASH), concat_bouncer);
    }

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


    /*-------.
    | Date.  |
    `-------*/

    // FIXME: kill this when overloading is fully supported
    OVERLOAD_TYPE_3(
      MINUS_overload, 1, 1,
      Date, (rDuration (Date::*)(rDate) const)     &Date::operator-,
      Duration, (rDate (Date::*)(const Date::duration_type&) const) &Date::operator-,
      Float, (rDate (Date::*)(const Date::duration_type&) const) &Date::operator-)

    // FIXME: kill this when overloading is fully supported
    static rObject MINUS(const objects_type& args)
    {
      check_arg_count(args.size() - 1, 0, 1);
      static rPrimitive actual = new Primitive(MINUS_overload);
      return (*actual)(args);
    }

    URBI_CXX_OBJECT_REGISTER(Date)
      : time_(boost::posix_time::microsec_clock::local_time())
    {
      bind_variadic(SYMBOL(MINUS), MINUS);
#define DECLARE(Unit)                                           \
  bind(libport::Symbol(#Unit),       &Date::Unit ## _get,       \
       libport::Symbol(#Unit "Set"), &Date::Unit ## _set)
      DECLARE(day);
      DECLARE(hour);
      DECLARE(minute);
      DECLARE(month);
      DECLARE(second);
      DECLARE(year);
#undef DECLARE
    }


    URBI_CXX_OBJECT_REGISTER(Barrier)
    {}
  }
}
