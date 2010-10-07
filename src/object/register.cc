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
#include <libport/thread.hh>
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

    /*------------.
    | Executable. |
    `------------*/

    URBI_CXX_OBJECT_REGISTER(Executable)
    {}

    /*-----------.
    | Primitive. |
    `-----------*/

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

      // Hack to avoid proto = 0,
      // proto will redefined after.
      proto = this;
      bind(SYMBOL(apply), &Primitive::apply);
    }

    /*--------.
    | String. |
    `--------*/

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
    static rObject string_split_bouncer(const objects_type& _args)
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

    OVERLOAD_2
    (string_sub_bouncer, 2,
     (std::string (String::*) (unsigned) const) (&String::sub),
     (std::string (String::*) (unsigned, unsigned) const) (&String::sub)
      );

    OVERLOAD_2
    (string_sub_eq_bouncer, 3,
     (std::string (String::*) (unsigned, const std::string&))
     (&String::sub_eq),
     (std::string (String::*) (unsigned, unsigned, const std::string&))
     (&String::sub_eq)
      );

    URBI_CXX_OBJECT_REGISTER(String)
    {
      bind_variadic(SYMBOL(split), string_split_bouncer);

      bind(SYMBOL(EQ_EQ),
           static_cast<bool (self_type::*)(const rObject&) const>
           (&self_type::operator==));

#define DECLARE(Name, Function)                 \
      bind(SYMBOL(Name), &String::Function)     \

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

      setSlot(SYMBOL(SBL_SBR), new Primitive(string_sub_bouncer));
      setSlot(SYMBOL(SBL_SBR_EQ), new Primitive(string_sub_eq_bouncer));
    }

    /*---------.
    | Process. |
    `---------*/

#if !defined WIN32
    URBI_CXX_OBJECT_REGISTER(Process)
      : name_(libport::path("true").basename())
      , pid_(0)
      , binary_("/bin/true")
      , argv_()
      , status_(-1)
    {
      argv_ << binary_;

# define DECLARE(Name, Function)             \
      bind(SYMBOL(Name), &Process::Function) \

      DECLARE(asString, as_string);
      DECLARE(done,     done);
      DECLARE(init,     init);
      DECLARE(join,     join);
      DECLARE(run,      run);
      DECLARE(runTo,    runTo);
      DECLARE(kill,     kill);
      DECLARE(status,   status);
      DECLARE(name,     name_);
# undef DECLARE

      libport::startThread(boost::function0<void>(&Process::monitor_children));
    }
#endif

    /*--------.
    | Regexp. |
    `--------*/

#if !defined COMPILATION_MODE_SPACE
    URBI_CXX_OBJECT_REGISTER(Regexp)
      : re_(".")
    {
# define DECLARE(Urbi, Cxx)            \
      bind(SYMBOL(Urbi), &Regexp::Cxx) \

      DECLARE(asPrintable, as_printable);
      DECLARE(asString,    as_string);
      DECLARE(init,        init);
      DECLARE(match,       match);
      DECLARE(matches,     matches);
      DECLARE(SBL_SBR,     operator[]);

# undef DECLARE
    }

    /*-----------.
    | Formatter. |
    `-----------*/

    URBI_CXX_OBJECT_REGISTER(Formatter)
    {
# define DECLARE(Urbi, Cxx)               \
      bind(SYMBOL(Urbi), &Formatter::Cxx) \

      DECLARE(init, init);
      DECLARE(data, data_get);

# undef DECLARE

# define OPERATOR_PCT(Type)                                       \
      static_cast<std::string (Formatter::*)(const Type&) const>  \
      (&Formatter::operator%)                                     \

      bind(SYMBOL(PERCENT), OPERATOR_PCT(rObject));

# undef OPERATOR_PCT
    }

    /*------------.
    | FormatInfo. |
    `------------*/

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

# define DECLARE(Name)                                    \
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

# undef DECLARE
    }
#endif

    /*--------.
    | UValue. |
    `--------*/

    URBI_CXX_OBJECT_REGISTER(UValue)
    {
#define DECLARE(Name)                   \
      bind(SYMBOL(Name), &UValue::Name) \

      DECLARE(extract);
      DECLARE(extractAsToplevelPrintable);
      DECLARE(invalidate);

#undef DECLARE

      bind(SYMBOL(put), (void (UValue::*)(rObject))&UValue::put);
    }

    /*-----.
    | Tag. |
    `-----*/

    URBI_CXX_OBJECT_REGISTER(Tag)
      : value_(new sched::Tag(libport::Symbol::make_empty()))
    {
#define DECLARE(Name, Cast)                                             \
      bind(SYMBOL(Name), static_cast<void (Tag::*)(Cast)>(&Tag::Name)); \

      DECLARE(init,                 );
      DECLARE(init,  libport::Symbol);
      DECLARE(stop,                 );
      DECLARE(stop,  rObject        );
      DECLARE(block,                );
      DECLARE(block, rObject        );

#undef DECLARE

      bind_variadic(SYMBOL(newFlowControl), &Tag::new_flow_control);

#define DECLARE(Name, Function)                 \
      bind(SYMBOL(Name), &Tag::Function)        \

      DECLARE(blocked,     blocked);
      DECLARE(enter,       enter);
      DECLARE(freeze,      freeze);
      DECLARE(frozen,      frozen);
      DECLARE(getParent,   parent_get);
      DECLARE(leave,       leave);
      DECLARE(name,        name);
      DECLARE(priority,    priority);
      DECLARE(scope,       scope);
      DECLARE(setPriority, priority_set);
      DECLARE(unblock,     unblock);
      DECLARE(unfreeze,    unfreeze);

#undef DECLARE
    }

    /*--------.
    | Server. |
    `--------*/

    URBI_CXX_OBJECT_REGISTER(Server)
      : libport::Socket(*object::Socket::get_default_io_service().get())
      , io_service_(object::Socket::get_default_io_service())
    {
#define DECLARE(Name, Cxx)             \
      bind(SYMBOL(Name), &Server::Cxx) \

      DECLARE(getIoService, getIoService);
      DECLARE(host,         host);
      DECLARE(listen,       listen);
      DECLARE(port,         port);
      DECLARE(sockets,      sockets);

#undef DECLARE
    }

    /*--------.
    | Socket. |
    `--------*/

    OVERLOAD_TYPE(
      socket_connect_overload, 2, 2,
      String,
      (void (Socket::*)(const std::string&, const std::string&))
      &Socket::connect,
      Float,
      (void (Socket::*)(const std::string&, unsigned))
      &Socket::connect)

    URBI_CXX_OBJECT_REGISTER(Socket)
      : libport::Socket(*get_default_io_service().get())
    {
      io_service_ = get_default_io_service();

#define DECLARE(Name)                           \
      bind(SYMBOL(Name), &Socket::Name)         \

      // Uncomment the line below when overloading will work.
      //bind(SYMBOL(connectSerial),
      //     (void (Socket::*)(const std::string&, unsigned int))
      //       &Socket::connectSerial);
      bind(SYMBOL(connectSerial),
           (void (Socket::*)(const std::string&, unsigned int, bool))
             &Socket::connectSerial);
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

      setSlot(SYMBOL(connect), new Primitive(socket_connect_overload));
    }

    /*-----------.
    | Semaphore. |
    `-----------*/

    URBI_CXX_OBJECT_REGISTER(Semaphore)
    {
#define DECLARE(Name, Cxx)                \
      bind(SYMBOL(Name), &Semaphore::Cxx) \

      DECLARE(new,             _new);
      DECLARE(criticalSection, criticalSection);
      DECLARE(acquire,         acquire);
      DECLARE(release,         release);

#undef DECLARE
    }

    /*------.
    | Code. |
    `------*/

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

#define DECLARE(Name, Cxx)           \
      bind(SYMBOL(Name), &Code::Cxx) \

      DECLARE(asString,   as_string);
      DECLARE(bodyString, body_string);

#undef DECLARE
    }

    /*------.
    | UVar. |
    `------*/

    static rObject
    uvar_update_bounce(objects_type args)
    {
      //called with self slotname slotval
      check_arg_count(args.size() - 1, 2);
      libport::intrusive_ptr<UVar> rvar =
        args.front()
        ->slot_get(libport::Symbol(args[1]->as<String>()->value_get())).value()
        .unsafe_cast<UVar>();
      if (!rvar)
        RAISE("UVar updatehook called on non-uvar slot");
      rvar->update_(args[2]);
      return void_class;
    }

    URBI_CXX_OBJECT_REGISTER(UVar)
      : Primitive(boost::function1<rObject, objects_type>
                    (boost::bind(&UVar::accessor, this, _1)))
      , looping_(false)
      , inAccess_(false)
    {
#define DECLARE(Name, Cxx)           \
      bind(SYMBOL(Name), &UVar::Cxx) \

      DECLARE(writeOwned,    writeOwned);
      DECLARE(update_timed,  update_timed);
      DECLARE(loopCheck,     loopCheck);
      DECLARE(accessor,      accessor);
      DECLARE(update_,       update_);
      DECLARE(update_timed_, update_timed_);

#undef DECLARE

      setSlot(SYMBOL(updateBounce), new Primitive(&uvar_update_bounce));
    }

    /*----------.
    | Position. |
    `----------*/

    URBI_CXX_OBJECT_REGISTER(Position)
      : pos_()
    {
      bind(SYMBOL(init), static_cast<void (Position::*)()>(&Position::init));
      bind(SYMBOL(init),
           static_cast<void (Position::*)(const value_type& pos)>
             (&Position::init));
      bind(SYMBOL(init),
           static_cast<void (Position::*)(unsigned from, unsigned to)>
             (&Position::init));
      bind(SYMBOL(init),
           static_cast
             <void (Position::*)(rObject file, unsigned from, unsigned to)>
               (&Position::init));
      typedef bool (Position::*comparison_type)(rPosition rhs) const;
      bind(SYMBOL(EQ_EQ), (comparison_type) &Position::operator ==);
      bind(SYMBOL(LT), (comparison_type) &Position::operator <);

#define DECLARE(Name, Cxx)               \
      bind(SYMBOL(Name), &Position::Cxx) \

      DECLARE(MINUS,    operator -);
      DECLARE(PLUS,     operator +);
      DECLARE(lines,    lines);
      DECLARE(columns,  columns);
      DECLARE(asString, as_string);

#undef DECLARE

#define DECLARE(Name)                                     \
      bind(SYMBOL(Name), &Position::Name ##_get);         \
      property_set(SYMBOL(Name),                          \
                   SYMBOL(updateHook),                    \
                   primitive(&Position::Name ##_set))     \

      DECLARE(file);

#undef DECLARE

      // For some reason, cl.exe refuses "&Position::value_type::line"
      // with error: function cannot access 'yy::position::line'
#define DECLARE(Name)                                   \
      bind(SYMBOL( Name ), &yy::position::Name) \

      DECLARE(line);
      DECLARE(column);

#undef DECLARE
    }

    /*----------.
    | Location. |
    `----------*/

    URBI_CXX_OBJECT_REGISTER(Location)
      : loc_()
    {
      bind(SYMBOL(init),
           static_cast<void (Location::*)()>(&Location::init));
      bind(SYMBOL(init),
           static_cast<void (Location::*)(const Position::value_type&)>
             (&Location::init));
      bind(SYMBOL(init),
           static_cast<void (Location::*)(const Position::value_type&,
                                          const Position::value_type&)>
             (&Location::init));
      bind(SYMBOL(EQ_EQ),
           (bool (Location::*)(rLocation rhs) const) &Location::operator ==);

      // For some reason, cl.exe refuses "&Location::value_type::begin"
      // with error: function cannot access 'yy::location::begin'
      bind(SYMBOL(begin), &yy::location::begin);
      bind(SYMBOL(end),   &yy::location::end);

#define DECLARE(Name, Cxx)                             \
      bind(SYMBOL( Name ), &Location::Cxx) \

      DECLARE(asString,         as_string);
      DECLARE(isSystemLocation, is_system_location);

#undef DECLARE
    }

    /*-------.
    | Lobby. |
    `-------*/

    URBI_CXX_OBJECT_REGISTER(Lobby)
      : connection_(0)
    {
      bind(SYMBOL(send),
           static_cast<void (Lobby::*)(const std::string&)>(&Lobby::send));
      bind(SYMBOL(send),
           static_cast<void (Lobby::*)(const std::string&, const std::string&)>
             (&Lobby::send));

#define DECLARE(Name)                  \
      bind(SYMBOL(Name), &Lobby::Name) \

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

    /*------.
    | List. |
    `------*/

    URBI_CXX_OBJECT_REGISTER(List)
    {
      bind(SYMBOL(sort),
           static_cast<List::value_type (List::*)()>(&List::sort));
      bind(SYMBOL(sort),
           static_cast<List::value_type (List::*)(rObject)>
             (&List::sort));

#define DECLARE(Name, Function)                 \
      bind(SYMBOL(Name), &List::Function)       \

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

    /*-------.
    | Event. |
    `-------*/

    URBI_CXX_OBJECT_REGISTER(Event)
    {
      typedef void (Event::*emit_type)(const objects_type& args);
      bind_variadic<void, Event>(SYMBOL(emit),
                                 static_cast<emit_type>(&Event::emit));
      bind(SYMBOL(hasSubscribers), &Event::hasSubscribers);
      bind(SYMBOL(localTrigger), &Event::localTrigger);
      typedef
      void (Event::*on_event_type)
      (rExecutable guard, rExecutable enter, rExecutable leave);
      bind(SYMBOL(onEvent), static_cast<on_event_type>(&Event::onEvent));
      bind_variadic<void, Event>(SYMBOL(syncEmit), &Event::syncEmit);
      bind_variadic<rEvent, Event>(SYMBOL(syncTrigger), &Event::syncTrigger);
      bind_variadic<rEvent, Event>(SYMBOL(trigger), &Event::trigger);

#define DECLARE(Name, Cxx)            \
      bind(SYMBOL(Name), &Event::Cxx) \

      DECLARE(stop,      stop);
      DECLARE(waituntil, waituntil);

#undef DECLARE
    }

    /*-----.
    | Job. |
    `-----*/

    URBI_CXX_OBJECT_REGISTER(Job)
    {
#define DECLARE(Name, Cxx)          \
      bind(SYMBOL(Name), &Job::Cxx) \

      DECLARE(DOLLAR_backtrace,   backtrace);
      DECLARE(name,               name);
      DECLARE(setSideEffectFree,  setSideEffectFree);
      DECLARE(status,             status);
      DECLARE(tags,               tags);
      DECLARE(terminate,          terminate);
      DECLARE(timeShift,          timeShift);
      DECLARE(waitForChanges,     waitForChanges);
      DECLARE(waitForTermination, waitForTermination);

#undef DECLARE
    }

    /*-----------.
    | IoService. |
    `-----------*/

    URBI_CXX_OBJECT_REGISTER(IoService)
    {
#define DECLARE(Name, Cxx)                \
      bind(SYMBOL(Name), &IoService::Cxx) \

      DECLARE(pollFor,    pollFor);
      DECLARE(pollOneFor, pollOneFor);
      DECLARE(poll,       poll);
      DECLARE(makeServer, makeServer);
      DECLARE(makeSocket, makeSocket);

#undef DECLARE
    }

    /*-------.
    | Float. |
    `-------*/

    URBI_CXX_OBJECT_REGISTER(Float)
      : value_(0)
    {
      bind(SYMBOL(PLUS),
           static_cast<Float::value_type (Float::*)() const>(&Float::plus));
      bind(SYMBOL(PLUS),
           static_cast<Float::value_type (Float::*)(Float::value_type) const>
            (&Float::plus));
      bind(SYMBOL(MINUS),
           static_cast<Float::value_type (Float::*)() const>(&Float::minus));
      bind(SYMBOL(MINUS),
           static_cast<Float::value_type (Float::*)(Float::value_type) const>
             (&Float::minus));
      bind(SYMBOL(EQ_EQ),
           static_cast<bool (self_type::*)(const rObject&) const>
                      (&self_type::operator==));

      limits = urbi::object::Object::proto->clone();

#define DECLARE(Urbi, Cxx)                    \
      limits->bind(SYMBOL(Urbi), &Float::Cxx) \

      DECLARE(digits, limit_digits);
      DECLARE(digits10, limit_digits10);
      DECLARE(min, limit_min);
      DECLARE(max, limit_max);
      DECLARE(epsilon, limit_epsilon);
      DECLARE(minExponent, limit_min_exponent);
      DECLARE(minExponent10, limit_min_exponent10);
      DECLARE(maxExponent, limit_max_exponent);
      DECLARE(maxExponent10, limit_max_exponent10);
      DECLARE(radix, limit_radix);

#undef DECLARE

#define DECLARE(Urbi, Cxx)            \
      bind(SYMBOL(Urbi), &Float::Cxx) \

      DECLARE(CARET,     operator^);
      DECLARE(GT_GT,     operator>>);
      DECLARE(LT_EQ,     operator<=);
      DECLARE(LT,        less_than);
      DECLARE(GT,        operator>);
      DECLARE(GT_EQ,     operator>=);
      DECLARE(BANG_EQ,   operator!=);
      DECLARE(LT_LT,     operator<<);
      DECLARE(PERCENT,   operator%);
      DECLARE(SLASH,     operator/);
      DECLARE(STAR,      operator*);
      DECLARE(STAR_STAR, pow);
      DECLARE(abs,       fabs);
      DECLARE(acos,      acos);
      DECLARE(asString,  as_string);
      DECLARE(asin,      asin);
      DECLARE(atan,      atan);
      DECLARE(atan2,     atan2);
      DECLARE(bitand,    operator&);
      DECLARE(bitor,     operator|);
      DECLARE(ceil,      ceil);
      DECLARE(compl,     operator~);
      DECLARE(cos,       cos);
      DECLARE(exp,       exp);
#if !defined COMPILATION_MODE_SPACE
      DECLARE(format, format);
#endif
      DECLARE(floor,   floor);
      DECLARE(inf,     inf);
      DECLARE(isInf,   is_inf);
      DECLARE(isNan,   is_nan);
      DECLARE(log,     log);
      DECLARE(nan,     nan);
      DECLARE(random,  random);
      DECLARE(srandom, srandom);
      DECLARE(round,   round);
      DECLARE(seq,     seq);
      DECLARE(sign,    sign);
      DECLARE(sin,     sin);
      DECLARE(sqrt,    sqrt);
      DECLARE(tan,     tan);
      DECLARE(trunc,  trunc);

#undef DECLARE

      // Hack to avoid proto = 0,
      // proto will redefined after.
      proto = this;

      setSlot("pi", new Float(M_PI));
      setSlot(SYMBOL(limits), Float::limits);
    }

    /*----------.
    | Duration. |
    `----------*/

    URBI_CXX_OBJECT_REGISTER(Duration)
      : Float(0)
    {
      proto_add(Float::proto);

      bind(SYMBOL(init),
           static_cast<void (Duration::*)()>(&Duration::init));
      bind(SYMBOL(init),
           static_cast<void (Duration::*)(const Duration::value_type&)>
             (&Duration::init));

#define DECLARE(Name, Cxx)               \
      bind(SYMBOL(Name), &Duration::Cxx) \

      DECLARE(asPrintable, asPrintable);
      DECLARE(asString,    as_string);
      DECLARE(seconds,     seconds);

#undef DECLARE
    }

    /*-------------.
    | Finalizable. |
    `-------------*/

    URBI_CXX_OBJECT_REGISTER(Finalizable)
    {
#define DECLARE(Name)                        \
      bind(SYMBOL(Name), &Finalizable::Name) \

      DECLARE(__dec);
      DECLARE(__inc);
      DECLARE(__get);

#undef DECLARE
    }

    /*------.
    | File. |
    `------*/

    OVERLOAD_TYPE(file_init_bouncer, 1, 1,
                  Path,
                  (void (File::*)(rPath)) &File::init,
                  String,
                  (void (File::*)(const std::string&)) &File::init);

    URBI_CXX_OBJECT_REGISTER(File)
      : path_(new Path("/"))
    {
#define DECLARE(Name, Cxx)           \
      bind(SYMBOL(Name), &File::Cxx) \

      DECLARE(asList,      as_list);
      DECLARE(asPrintable, as_printable);
      DECLARE(asString,    as_string);
      DECLARE(content,     content);
      DECLARE(create,      create);
      DECLARE(rename,      rename);
      DECLARE(remove,      remove);

#undef DECLARE

      setSlot(SYMBOL(init), new Primitive(&file_init_bouncer));
    }

    /*------.
    | Path. |
    `------*/

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

#define DECLARE(Urbi, Cxx)           \
      bind(SYMBOL(Urbi), &Path::Cxx) \

      DECLARE(LT_EQ,       operator<=);
      DECLARE(absolute,    absolute);
      DECLARE(asList,      as_list);
      DECLARE(asPrintable, as_printable);
      DECLARE(asString,    as_string);
      DECLARE(basename,    basename);
      DECLARE(cd,          cd);
      DECLARE(cwd,         cwd);
      DECLARE(dirname,     dirname);
      DECLARE(exists,      exists);
      DECLARE(init,        init);
      DECLARE(isDir,       is_dir);
      DECLARE(isReg,       is_reg);
      DECLARE(open,        open);
      DECLARE(readable,    readable);
      DECLARE(writable,    writable);

#undef DECLARE
    }

    /*-----------.
    | Directory. |
    `-----------*/

    OVERLOAD_TYPE(directory_init_bouncer, 1, 1,
                  Path,
                  (void (Directory::*)(rPath)) &Directory::init,
                  String,
                  (void (Directory::*)(const std::string&)) &Directory::init);

    URBI_CXX_OBJECT_REGISTER(Directory)
      : path_(new Path())
    {
#define DECLARE(Name, Cxx)                \
      bind(SYMBOL(Name), &Directory::Cxx) \

      DECLARE(asList,      list<&directory_mk_path>);
      DECLARE(content,     list<&directory_mk_string>);
      DECLARE(asPrintable, as_printable);
      DECLARE(asString,    as_string);

#undef DECLARE

      setSlot(SYMBOL(init), new Primitive(&directory_init_bouncer));
    }

    /*--------------.
    | OutputStream. |
    `--------------*/

#if !defined COMPILATION_MODE_SPACE
    URBI_CXX_OBJECT_REGISTER(OutputStream)
      : Stream(STDOUT_FILENO, false)
    {
#define DECLARE(Name, Cxx)                   \
      bind(SYMBOL(Name), &OutputStream::Cxx) \

      DECLARE(LT_LT, put);
      DECLARE(close, close);
      DECLARE(flush, flush);
      DECLARE(init,  init);
      DECLARE(put,   putByte);

#undef DECLARE
    }

    /*-------------.
    | InputStream. |
    `-------------*/

    URBI_CXX_OBJECT_REGISTER(InputStream)
      : Stream(STDIN_FILENO, false)
      , pos_(0)
      , size_(0)
    {
#define DECLARE(Name, Cxx)                  \
      bind(SYMBOL(Name), &InputStream::Cxx) \

      DECLARE(close,   close);
      DECLARE(get,     get);
      DECLARE(getChar, getChar);
      DECLARE(getLine, getLine);
      DECLARE(init,    init);

#undef DECLARE
    }
#endif

    /*------------.
    | Dictionary. |
    `------------*/

    URBI_CXX_OBJECT_REGISTER(Dictionary)
    {
      bind(SYMBOL(asBool), &Dictionary::as_bool);

#define DECLARE(Name)                       \
      bind(SYMBOL(Name), &Dictionary::Name) \

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

    /*------.
    | Date. |
    `------*/

    // FIXME: kill this when overloading is fully supported
    OVERLOAD_TYPE_3(
      MINUS_overload, 1, 1,
      Date,
      (rDuration (Date::*)(rDate) const)                      &Date::operator-,
      Duration,
      (rDate     (Date::*)(const Date::duration_type&) const) &Date::operator-,
      Float,
      (rDate     (Date::*)(const Date::duration_type&) const) &Date::operator-)

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

#define DECLARE(Unit)                                         \
      bind(libport::Symbol(#Unit),       &Date::Unit ## _get, \
           libport::Symbol(#Unit "Set"), &Date::Unit ## _set) \

      DECLARE(day);
      DECLARE(hour);
      DECLARE(minute);
      DECLARE(month);
      DECLARE(second);
      DECLARE(year);

#undef DECLARE

      bind(SYMBOL(LT), (bool (Date::*)(rDate rhs) const)&Date::operator <);

#define DECLARE(Name, Cxx)           \
      bind(SYMBOL(Name), &Date::Cxx) \

      DECLARE(EQ_EQ,    operator ==);
      DECLARE(PLUS,     operator +);
      DECLARE(asFloat,  as_float);
      DECLARE(asString, as_string);
      DECLARE(epoch,    epoch);
      DECLARE(init,     init);
      DECLARE(now,      now);

#undef DECLARE
    }

    /*---------.
    | Barrier. |
    `---------*/

    URBI_CXX_OBJECT_REGISTER(Barrier)
    {
#define DECLARE(Name, Cxx)              \
      bind(SYMBOL(Name), &Barrier::Cxx) \

      DECLARE(new,       _new);
      DECLARE(signal,    signal);
      DECLARE(signalAll, signalAll);
      DECLARE(wait,      wait);

#undef DECLARE
    }
  }
}
