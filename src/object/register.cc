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

      bind(SYMBOL(EQ_EQ),
           static_cast<bool (self_type::*)(const rObject&) const>
           (&self_type::operator==));
#define DECLARE(Name, Function)                 \
      bind(SYMBOL(Name), &String::Function)

      DECLARE(LT_EQ       , operator<=);
      DECLARE(PLUS        , plus);
      DECLARE(STAR        , star);
      DECLARE(asBool      , as_bool);
      DECLARE(asFloat     , as_float);
      DECLARE(asPrintable , as_printable);
      DECLARE(asString    , as_string);
      DECLARE(distance    , distance);
      DECLARE(empty       , empty);
#if !defined COMPILATION_MODE_SPACE
      DECLARE(format      , format);
#endif
      DECLARE(fresh       , fresh);
      DECLARE(fromAscii   , fromAscii);
      DECLARE(isAlnum     , is_alnum);
      DECLARE(isAlpha     , is_alpha);
      DECLARE(isCntrl     , is_cntrl);
      DECLARE(isDigit     , is_digit);
      DECLARE(isGraph     , is_graph);
      DECLARE(isLower     , is_lower);
      DECLARE(isPrint     , is_print);
      DECLARE(isPunct     , is_punct);
      DECLARE(isSpace     , is_space);
      DECLARE(isUpper     , is_upper);
      DECLARE(isXdigit    , is_xdigit);
      DECLARE(join        , join);
      DECLARE(replace     , replace);
      DECLARE(set         , set);
      DECLARE(size        , size);
      DECLARE(toAscii     , toAscii);
      DECLARE(toLower     , to_lower);
      DECLARE(toUpper     , to_upper);

#undef DECLARE
    }

#if !defined WIN32 && !defined COMPILATION_MODE_SPACE
    URBI_CXX_OBJECT_REGISTER(Process)
      : name_(libport::path("true").basename())
      , pid_(0)
      , binary_("/bin/true")
      , argv_()
      , status_(-1)
    {
      argv_ << binary_;
      bind(SYMBOL(asString), &Process::as_string);
      bind(SYMBOL(done), &Process::done);
      bind(SYMBOL(init), &Process::init);
      bind(SYMBOL(join), &Process::join);
      bind(SYMBOL(run),  &Process::run );
      bind(SYMBOL(runTo),  &Process::runTo );
      bind(SYMBOL(kill),  &Process::kill );
      bind(SYMBOL(status),  &Process::status );
    }
#endif

#if !defined COMPILATION_MODE_SPACE
    URBI_CXX_OBJECT_REGISTER(Regexp)
      : re_(".")
    {
#define DECLARE(Urbi, Cxx)                      \
      bind(SYMBOL(Urbi), &Regexp::Cxx)

      DECLARE(asPrintable, as_printable);
      DECLARE(asString, as_string);
      DECLARE(init, init);
      DECLARE(match, match);
      DECLARE(matches, matches);
#undef DECLARE
      bind(SYMBOL(SBL_SBR), &Regexp::operator[]);
    }

    URBI_CXX_OBJECT_REGISTER(Formatter)
    {
      bind(SYMBOL(init),    &Formatter::init);
      bind(SYMBOL(data),    &Formatter::data_get);
#define OPERATOR_PCT(Type)                                      \
    static_cast<std::string (Formatter::*)(const Type&) const>  \
    (&Formatter::operator%)
      bind(SYMBOL(PERCENT), OPERATOR_PCT(rObject));
#undef OPERATOR_PCT
    }

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
    {
      bind(SYMBOL(init),     &FormatInfo::init);
      bind(SYMBOL(asString), &FormatInfo::as_string);
      bind(SYMBOL(pattern),  &FormatInfo::pattern_get);

#define DECLARE(Name)                                     \
      bind(SYMBOL(Name), &FormatInfo::Name ##_get);       \
      property_set(SYMBOL(Name),                          \
                   SYMBOL(updateHook),                    \
                   primitive(&FormatInfo::update_hook))   \

      DECLARE(alignment);
      DECLARE(alt);
      DECLARE(group);
      DECLARE(pad);
      DECLARE(precision);
      DECLARE(prefix);
      DECLARE(spec);
      DECLARE(uppercase);
      DECLARE(width);

#undef DECLARE
    }
#endif

    URBI_CXX_OBJECT_REGISTER(UValue)
    {
      bind(SYMBOL(extract), &UValue::extract);
      bind(SYMBOL(extractAsToplevelPrintable),
           &UValue::extractAsToplevelPrintable);
      bind(SYMBOL(invalidate), &UValue::invalidate);
      bind(SYMBOL(put), (void (UValue::*)(rObject))&UValue::put);
    }


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

#define DECLARE(Name, Function)                 \
      bind(SYMBOL(Name), &Tag::Function)

      DECLARE(blocked, blocked);
      DECLARE(enter, enter);
      DECLARE(freeze, freeze);
      DECLARE(frozen, frozen);
      DECLARE(getParent, parent_get);
      DECLARE(leave, leave);
      DECLARE(name, name);
      DECLARE(priority, priority);
      DECLARE(scope, scope);
      DECLARE(setPriority, priority_set);
      DECLARE(unblock, unblock);
      DECLARE(unfreeze, unfreeze);
#undef DECLARE
    }

    URBI_CXX_OBJECT_REGISTER(Server)
      : libport::Socket(*object::Socket::get_default_io_service().get())
      , io_service_(object::Socket::get_default_io_service())
    {
      bind(SYMBOL(getIoService), &Server::getIoService);
      bind(SYMBOL(host),    &Server::host);
      bind(SYMBOL(listen),  &Server::listen);
      bind(SYMBOL(port),    &Server::port);
      bind(SYMBOL(sockets), &Server::sockets);
    }

    URBI_CXX_OBJECT_REGISTER(Socket)
      : libport::Socket(*get_default_io_service().get())
    {
      io_service_ = get_default_io_service();
#define DECLARE(Name)                           \
      bind(SYMBOL(Name), &Socket::Name)

      // Uncomment the line below when overloading will work.
      //bind(SYMBOL(connectSerial), (void (Socket::*)(const std::string&, unsigned int))&Socket::connectSerial);
      bind(SYMBOL(connectSerial), (void (Socket::*)(const std::string&, unsigned int, bool))&Socket::connectSerial);
      DECLARE(disconnect);
      DECLARE(host);
      DECLARE(init);
      DECLARE(isConnected);
      DECLARE(localHost);
      DECLARE(localPort);
      DECLARE(poll);
      DECLARE(port);
      DECLARE(read);
      DECLARE(write);
      DECLARE(syncWrite);
      DECLARE(getIoService);
#undef DECLARE
    }

    URBI_CXX_OBJECT_REGISTER(Semaphore)
    {
      bind(SYMBOL(new),             &Semaphore::_new);
      bind(SYMBOL(criticalSection), &Semaphore::criticalSection);
      bind(SYMBOL(acquire),         &Semaphore::acquire);
      bind(SYMBOL(release),         &Semaphore::release);
    }

    URBI_CXX_OBJECT_REGISTER(Code)
    {
      PARAMETRIC_AST(ast, "function () {}");
      ast_ = ast.result<const ast::Routine>();
      proto_add(Executable::proto);
      proto_remove(Object::proto);
      bind_variadic<rObject, Code>(SYMBOL(apply), &Code::apply);
      bind(SYMBOL(EQ_EQ),
           static_cast<bool (self_type::*)(const rObject&) const>
           (&self_type::operator==));
      bind(SYMBOL(asString), &Code::as_string);
      bind(SYMBOL(bodyString), &Code::body_string);
    }

    URBI_CXX_OBJECT_REGISTER(UVar)
      : Primitive(boost::function1<rObject, objects_type>(boost::bind(&UVar::accessor, this, _1)))
      , looping_(false)
      , inAccess_(false)
    {
      bind(SYMBOL(writeOwned), &UVar::writeOwned);
      bind(SYMBOL(update_timed), &UVar::update_timed);
      bind(SYMBOL(loopCheck), &UVar::loopCheck);
      bind(SYMBOL(accessor), &UVar::accessor);
    }

    URBI_CXX_OBJECT_REGISTER(Position)
      : pos_()
    {
      bind(SYMBOL(init), static_cast<void (Position::*)()>(&Position::init));
      bind(SYMBOL(init), static_cast<void (Position::*)(const value_type& pos)>(&Position::init));
      bind(SYMBOL(init), static_cast<void (Position::*)(unsigned from, unsigned to)>(&Position::init));
      bind(SYMBOL(init), static_cast<void (Position::*)(rObject file, unsigned from, unsigned to)>(&Position::init));
      typedef bool (Position::*comparison_type)(rPosition rhs) const;
      bind(SYMBOL(EQ_EQ), (comparison_type) &Position::operator ==);
      bind(SYMBOL(LT), (comparison_type) &Position::operator <);
      bind(SYMBOL(MINUS), &Position::operator -);
      bind(SYMBOL(PLUS), &Position::operator +);
      bind(SYMBOL(lines), &Position::lines);
      bind(SYMBOL(columns), &Position::columns);
      bind(SYMBOL(asString), &Position::as_string);

#define DECLARE(Name)                                     \
      bind(SYMBOL(Name), &Position::Name ##_get);         \
      property_set(SYMBOL(Name),                          \
                   SYMBOL(updateHook),                    \
                   primitive(&Position::Name ##_set))

      DECLARE(file);

#undef DECLARE
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
      bind(SYMBOL(EQ_EQ),
           (bool (Location::*)(rLocation rhs) const) &Location::operator ==);
      bind(SYMBOL(asString), &Location::as_string);
      bind(SYMBOL(isSystemLocation), &Location::is_system_location);
    }


    URBI_CXX_OBJECT_REGISTER(Lobby)
      : connection_(0)
    {
      bind(SYMBOL(send),
           static_cast<void (Lobby::*)(const std::string&)>(&Lobby::send));
      bind(SYMBOL(send),
           static_cast<void (Lobby::*)(const std::string&, const std::string&)>(&Lobby::send));
#define DECLARE(Name)                           \
      bind(SYMBOL(Name), &Lobby::Name)
      DECLARE(bytesSent);
      DECLARE(bytesReceived);
      DECLARE(create);
      DECLARE(lobby);
      DECLARE(quit);
      DECLARE(receive);
      DECLARE(write);
#undef DECLARE

      bind(SYMBOL(instances), &Lobby::instances_get);
    }

    URBI_CXX_OBJECT_REGISTER(List)
    {
      bind(SYMBOL(sort), static_cast<List::value_type (List::*)()>(&List::sort));
      bind(SYMBOL(sort), static_cast<List::value_type (List::*)(rObject)>(&List::sort));
#define DECLARE(Name, Function)                 \
      bind(SYMBOL(Name), &List::Function)

      DECLARE(asBool,         as_bool         );
      DECLARE(asString,       as_string       );
      DECLARE(back,           back            );
      DECLARE(clear,          clear           );
      DECLARE(each,           each            );
      DECLARE(each_AMPERSAND, each_and        );
      DECLARE(each_PIPE,      each_pipe       );
      DECLARE(eachi,          eachi           );
      DECLARE(empty,          empty           );
      DECLARE(front,          front           );
      DECLARE(SBL_SBR,        operator[]      );
      DECLARE(SBL_SBR_EQ,     set             );
      DECLARE(PLUS,           operator+       );
      DECLARE(PLUS_EQ,        operator+=      );
      DECLARE(insert,         insert          );
      DECLARE(insertBack,     insertBack      );
      DECLARE(insertFront,    insertFront     );
      DECLARE(removeBack,     removeBack      );
      DECLARE(removeFront,    removeFront     );
      DECLARE(removeById,     remove_by_id    );
      DECLARE(reverse,        reverse         );
      DECLARE(size,           size            );
      DECLARE(STAR,           operator*       );
      DECLARE(tail,           tail            );

#undef DECLARE
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
    {
      bind(SYMBOL(DOLLAR_backtrace), &Job::backtrace);
      bind(SYMBOL(name), &Job::name);
      bind(SYMBOL(setSideEffectFree), &Job::setSideEffectFree);
      bind(SYMBOL(status), &Job::status);
      bind(SYMBOL(tags), &Job::tags);
      bind(SYMBOL(terminate), &Job::terminate);
      bind(SYMBOL(timeShift), &Job::timeShift);
      bind(SYMBOL(waitForChanges), &Job::waitForChanges);
      bind(SYMBOL(waitForTermination), &Job::waitForTermination);
    }

    URBI_CXX_OBJECT_REGISTER(IoService)
    {
      bind(SYMBOL(pollFor), &IoService::pollFor);
      bind(SYMBOL(pollOneFor), &IoService::pollOneFor);
      bind(SYMBOL(poll), &IoService::poll);
      bind(SYMBOL(makeServer), &IoService::makeServer);
      bind(SYMBOL(makeSocket), &IoService::makeSocket);
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
    {
#define DECLARE(Name)                           \
      bind(SYMBOL(Name), &Finalizable::Name)

      DECLARE(__dec);
      DECLARE(__inc);
      DECLARE(__get);
#undef DECLARE
    }

    URBI_CXX_OBJECT_REGISTER(File)
      : path_(new Path("/"))
    {
      bind(SYMBOL(asList), &File::as_list);
      bind(SYMBOL(asPrintable), &File::as_printable);
      bind(SYMBOL(asString), &File::as_string);
      bind(SYMBOL(content), &File::content);
      bind(SYMBOL(create), &File::create);
      bind(SYMBOL(rename), &File::rename);
      bind(SYMBOL(remove), &File::remove);
    }

    OVERLOAD_TYPE(concat_bouncer, 1, 1,
                  Path, &Path::path_concat,
                  String, &Path::string_concat);

    URBI_CXX_OBJECT_REGISTER(Path)
      : path_(WIN32_IF("C:\\", "/"))
    {
      bind_variadic(SYMBOL(SLASH), concat_bouncer);

      bind(SYMBOL(EQ_EQ),
           static_cast<bool (Path::self_type::*)(const rObject&) const>
           (&Path::self_type::operator==));

#define DECLARE(Urbi, Cxx)                      \
      bind(SYMBOL(Urbi), &Path::Cxx)

      DECLARE(LT_EQ, operator<=);
      DECLARE(absolute, absolute);
      DECLARE(asList, as_list);
      DECLARE(asPrintable, as_printable);
      DECLARE(asString, as_string);
      DECLARE(basename, basename);
      DECLARE(cd, cd);
      DECLARE(cwd, cwd);
      DECLARE(dirname, dirname);
      DECLARE(exists, exists);
      DECLARE(init, init);
      DECLARE(isDir, is_dir);
      DECLARE(isReg, is_reg);
      DECLARE(open, open);
      DECLARE(readable, readable);
      DECLARE(writable, writable);
#undef DECLARE
    }

    URBI_CXX_OBJECT_REGISTER(Directory)
      : path_(new Path())
    {}

#if !defined COMPILATION_MODE_SPACE
    URBI_CXX_OBJECT_REGISTER(OutputStream)
      : Stream(STDOUT_FILENO, false)
    {
      bind(SYMBOL(LT_LT), &OutputStream::put    );
      bind(SYMBOL(close), &OutputStream::close  );
      bind(SYMBOL(flush), &OutputStream::flush  );
      bind(SYMBOL(init),  &OutputStream::init   );
      bind(SYMBOL(put),   &OutputStream::putByte);
    }

    URBI_CXX_OBJECT_REGISTER(InputStream)
      : Stream(STDIN_FILENO, false)
      , pos_(0)
      , size_(0)
    {
      bind(SYMBOL(close), &InputStream::close);
      bind(SYMBOL(get), &InputStream::get);
      bind(SYMBOL(getChar), &InputStream::getChar);
      bind(SYMBOL(getLine), &InputStream::getLine);
      bind(SYMBOL(init), &InputStream::init);
    }
#endif

    URBI_CXX_OBJECT_REGISTER(Dictionary)
    {
      bind(SYMBOL(asBool), &Dictionary::as_bool);
#define DECLARE(Name)                           \
      bind(SYMBOL(Name), &Dictionary::Name)

      DECLARE(clear);
      DECLARE(empty);
      DECLARE(erase);
      DECLARE(get);
      DECLARE(has);
      DECLARE(keys);
      DECLARE(set);
      DECLARE(size);
#undef DECLARE
    }


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
      bind(SYMBOL(EQ_EQ), &Date::operator ==);
      bind(SYMBOL(LT), (bool (Date::*)(rDate rhs) const)&Date::operator <);
      bind(SYMBOL(PLUS), &Date::operator +);
      bind(SYMBOL(asFloat), &Date::as_float);
      bind(SYMBOL(asString), &Date::as_string);
      bind(SYMBOL(epoch), &Date::epoch);
      bind(SYMBOL(init), &Date::init);
      bind(SYMBOL(now), &Date::now);
    }


    URBI_CXX_OBJECT_REGISTER(Barrier)
    {
      bind(SYMBOL(new),       &Barrier::_new);
      bind(SYMBOL(signal),    &Barrier::signal);
      bind(SYMBOL(signalAll), &Barrier::signalAll);
      bind(SYMBOL(wait),      &Barrier::wait);
    }
  }
}
