/// \file kernel/userver.cc

//#define ENABLE_DEBUG_TRACES
#include <cassert>
#include <libport/compiler.hh>
#include <libport/csignal>
#include <cstdlib>
#include <cstdarg>

#include <fstream>
#include <string>

// Include our header first to avoid duplicating some of its tricks.
#include <kernel/userver.hh>

#include <boost/bind.hpp>
#include <boost/checked_delete.hpp>
#include <boost/format.hpp>

#include <libport/compiler.hh>
#include <libport/config.h>
#include <libport/cstdio>
#include <libport/finally.hh>
#include <libport/foreach.hh>
#include <libport/lexical-cast.hh>
#include <libport/path.hh>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>

#include <kernel/config.h>

#include <urbi/uobject.hh>
#include <urbi/usystem.hh>

#include <kernel/userver.hh>
#include <kernel/utypes.hh>

#include <ast/ast.hh>
#include <ast/nary.hh>

#include <object/object.hh>
#include <object/primitive.hh>
#include <object/primitives.hh>
#include <object/root-classes.hh>
#include <object/system.hh>

#include <runner/call.hh>
#include <runner/runner.hh>
#include <runner/shell.hh>
#include <runner/sneaker.hh>

#include <scheduler/scheduler.hh>

#include <kernel/connection-set.hh>
#include <kernel/server-timer.hh>
#include <kernel/ubanner.hh>
#include <kernel/ughostconnection.hh>
#include <kernel/uobject.hh>
#include <kernel/uqueue.hh>

// Global server reference
UServer *urbiserver = 0;

// Formatting for the echo and error outputs.
const char* DISPLAY_FORMAT   = "[%ld] %-35s %s";
const char* DISPLAY_FORMAT1  = "[%ld] %-35s %s : %ld";
const char* DISPLAY_FORMAT2  = "[%d] %-35s %s : %d/%d";

// Buffers used to output data.
/// Used by echo() & error().
// FIXME: Because of this stupid hard limit, we can't produce
// large outputs!  We should move to using C++.  Or some scheme
// that is robust to the size of the message.
// NOTE: Do not *ever* create an object of this type on the stack.
// All instances must be marked as static. And the functions using
// them will not be reentrant. You have been warned.
typedef char buffer_type[8192];

static char* urbi_path = getenv("URBI_PATH");
static char* urbi_root = getenv("URBI_ROOT");

UServer::UServer(const char* mainName)
  : scheduler_(new scheduler::Scheduler(boost::bind(&UServer::getTime,
                                                    boost::ref(*this))))
  , debugOutput(false)
  , mainName_(mainName)
  , stopall(false)
  , connections_(new kernel::ConnectionSet)
{
  // The search path order is the URBI_PATH:URBI_ROOT/share/gostai:HARDCODED
  if (urbi_path)
    search_path.push_back(urbi_path, ":");
  if (urbi_root)
    search_path.push_back(libport::path(urbi_root) / "share" / "gostai", ":");
  else
    search_path.push_back(URBI_PATH, ":");
#if ! defined NDEBUG
  server_timer.start();
  server_timer.dump_on_destruction(std::cerr);
  TIMER_PUSH("server");
#endif
  ::urbiserver = this;
}

UErrorValue
UServer::load_init_file(const char* fn)
{
  DEBUG (("Loading %s...", fn));
  UErrorValue res = load_file(fn, ghost_->recv_queue_get());
  if (res == USUCCESS)
  {
    DEBUG (("done\n"));
    ghost_->new_data_added_get() = true;
  }
  else
    DEBUG (("not found\n"));
  return res;
}

#if !defined WIN32 && !defined _MSC_VER
static void
install_ice_catcher(void (*catcher)(int))
{
  signal(SIGABRT, catcher);
  signal(SIGBUS,  catcher);
  signal(SIGSEGV, catcher);
}

ATTRIBUTE_NORETURN static void hard_ice(int i);

static void hard_ice(int i)
{
  std::cerr << "Killed with signal " << i
            << " while trying to debug." << std::endl;
  libport::signal(i, SIG_DFL);
  if (kill(getpid(), i))
    perror("kill");
  // Pacify noreturn.
  exit(EX_HARD);
}

static void ice(int i)
{
  install_ice_catcher(hard_ice);

  // If we have a job currently running, use it to signal the error,
  // otherwise try to use the sneaker which must have been created
  // (or we have an error very early on and we are in deep trouble).
  runner::Runner& r = dbg::runner_or_sneaker_get();
  static const std::string tag = "";

  r.send_message(tag, "");
  r.send_message(tag, "    **********************");
  r.send_message(tag, "    *** INTERNAL ERROR ***");
  r.send_message(tag, "    **********************");
  r.send_message(tag, "");
  r.send_message(tag, "The urbi kernel was killed by signal "
                 + string_cast(i) + ": " + strsignal(i) + ".");
  r.send_message(tag, "Please report this bug to " PACKAGE_BUGREPORT
                 " with this report,");
  r.send_message(tag, "core dump if any, and what code/situation triggered it.");
  r.send_message(tag, "");
  r.send_message(tag, "Trying to give more information.");
  r.send_message(tag, "Please include it in the report.");
  r.send_message(tag, "");
  r.send_message(tag, "---------- CURRENT BACKTRACE ----------");
  r.show_backtrace(tag);
  r.send_message(tag, "");
  r.send_message(tag, "---------- PS ----------");
  urbi_call(r, object::system_class, SYMBOL(ps));
  r.send_message(tag, "");
  exit(EX_SOFTWARE);
}

#endif

void
UServer::initialize()
{
#if !defined WIN32 && !defined _MSC_VER
#if !defined NDEBUG
  static bool catch_ices = !getenv("URBI_NO_ICE_CATCHER");
  if (catch_ices)
#endif
    install_ice_catcher(ice);
#endif
  // Set the initial time to a valid value.
  updateTime();

  // Display the banner.
  {
    bool old_debugOutput = debugOutput;
    debugOutput = true;
    display(::HEADER_BEFORE_CUSTOM);

    unsigned int i = 0;
    static char customHeader[1024];
    do {
      getCustomHeader(i, customHeader, 1024);
      if (customHeader[0])
	display(customHeader);
      ++i;
    } while (customHeader[0] != 0);

    display(::HEADER_AFTER_CUSTOM);
    display("Ready.\n");

    debugOutput = old_debugOutput;
  }

  //The order is important: ghost connection, plugins, urbi.ini

  // Ghost connection
  DEBUG (("Setting up ghost connection..."));
  ghost_ = new UGhostConnection(*this);
  DEBUG (("done\n"));

  if (load_init_file("urbi/urbi.u") != USUCCESS
      && !getenv("IGNORE_URBI_U"))
    std::cerr
      << libport::program_name << ": cannot load urbi/urbi.u." << std::endl
      << libport::program_name << ": path: " << search_path << std::endl
      << libport::program_name << ": set IGNORE_URBI_U to ignore." << std::endl
      << libport::exit(EX_OSFILE);

  // Handle pluged UOBjects.
  // Create "uobject" in lobby where UObjects will be put.
  object::object_class->slot_set(SYMBOL(uobject_init),
                                 new object::Primitive(&uobject_initialize));

  // Force processing of urbi.u.
  while (!object::object_class->slot_locate(SYMBOL(loaded)))
    work();
  object::object_class->slot_remove(SYMBOL(loaded));
}


void
UServer::main (int, const char*[])
{
  // FIXME: Save argv into Urbi world.
}

void
UServer::beforeWork ()
{
}

void
UServer::afterWork ()
{
}

libport::utime_t
UServer::work()
{
  static bool report = getenv("REPORT");
  static int niter = 0;
  static libport::utime_t sumtime = 0, mintime = 10000000, maxtime = 0;
  static libport::utime_t rsumtime = 0, rmintime = 10000000, rmaxtime = 0;
  static unsigned int nzero = 0;

  beforeWork ();

  work_handle_connections_ ();

  // To make sure that we get different times before and after every work
  // phase if we use a monotonic clock, update the time before and after
  // working.
  updateTime ();
  libport::utime_t ctime = libport::utime();
  libport::utime_t next_time = scheduler_->work ();
  libport::utime_t rtime = next_time? std::max(0LL, next_time - ctime):0;
  ctime = libport::utime() - ctime;
  updateTime ();
  if (report)
  {
    if (!rtime)
      nzero++;
    else
    {
      rsumtime += rtime;
      rmintime = std::min(rmintime, rtime);
      rmaxtime = std::max(rmaxtime, rtime);
    }
    sumtime += ctime;
    mintime = std::min(mintime, ctime);
    maxtime = std::max(maxtime, ctime);
    niter++;
    if (niter == 1000)
    {
      std::cerr <<"## work time(us)  min: " << mintime <<"   max: " << maxtime
        << "   avg: " << sumtime/niter << std::endl;
      std::cerr <<"  sched interval(us)  zero-ratio: "
        << (float)nzero / (float)niter << "   min: " << rmintime <<"   max: "
        << rmaxtime << "   avg: " << rsumtime/(0.01 + niter-nzero) << std::endl;
      niter = 0;
      sumtime = 0;
      maxtime = 0;
      mintime = 1000000;
      rsumtime = 0;
      rmaxtime = 0;
      rmintime = 1000000;
      nzero = 0;
    }
  }
  work_handle_stopall_();
  afterWork ();
  return next_time;
}

void
UServer::work_handle_connections_ ()
{
  // Scan currently opened connections for ongoing work
  foreach (UConnection& c, *connections_)
    if (c.active_get())
    {
      if (!c.blocked_get())
	c.continue_send();

      if (c.new_data_added_get())
      {
	// used by load_file and eval to
	// delay the parsing after the completion
	// of execute().
	c.new_data_added_get() = false;
	c.received("");
      }
    }
}

void
UServer::work_handle_stopall_ ()
{
  if (stopall)
  {
    foreach (UConnection& c, *connections_)
      if (c.active_get() && c.has_pending_command ())
	c.drop_pending_commands ();
  }

  // Delete all connections with closing=true
  connections_->remove_closing();

  stopall = false;
}

//! UServer destructor.
UServer::~UServer()
{
  // When a connection gets destroyed, it indirectly calls the scheduler
  // in order to stop its associated connection tag. Since we are going
  // to destroy the scheduler, we must ensure that those actions are
  // carried out first.
  connections_->clear();
  delete scheduler_;

  object::cleanup_existing_objects();
}

void
UServer::vecho_key(const char* key, const char* s, va_list args)
{
  // This local declaration is rather unefficient but is necessary
  // to insure that the server could be made semi-reentrant.
  char key_[6];

  if (key == NULL)
    key_[0] = 0;
  else
  {
    strncpy(key_, key, 5);
    key_[5] = 0;
  }

  static buffer_type buf1;
  vsnprintf(buf1, sizeof buf1, s, args);
  static buffer_type buf2;
  snprintf(buf2, sizeof buf2, "%-90s [%5s]\n", buf1, key_);
  display(buf2);
}

void
UServer::echoKey(const char* key, const char* s, ...)
{
  va_list args;
  va_start(args, s);
  vecho_key(key, s, args);
  va_end(args);
}


void
UServer::error(const char* s, ...)
{
  va_list args;
  va_start(args, s);
  vecho_key("ERROR", s, args);
  va_end(args);
}


void
UServer::echo(const char* s, ...)
{
  va_list args;
  va_start(args, s);
  vecho_key("     ", s, args);
  va_end(args);
}


void
UServer::vdebug (const char* s, va_list args)
{
  static buffer_type buf;
  vsnprintf(buf, sizeof buf, s, args);
  effectiveDisplay(buf);
}


void
UServer::debug(const char* s, ...)
{
  va_list args;
  va_start(args, s);
  vdebug (s, args);
  va_end(args);
}


void
UServer::display(const char* s)
{
  if (debugOutput)
    effectiveDisplay(s);
}


void
UServer::display(const char** b)
{
  for (int i = 0; b[i]; ++i)
    display (b[i]);
}


void
UServer::shutdown()
{
  scheduler_->killall_jobs ();
}


void
UServer::updateTime()
{
  lastTime_ = getTime();
}

void
UServer::getCustomHeader (unsigned int, char* header, size_t)
{
  header[0] = 0; // empty string
}

namespace
{
  bool
  file_readable (const std::string& s)
  {
    std::ifstream is (s.c_str(), std::ios::binary);
    bool res = is;
    is.close();
    return res;
  }
}

std::string
UServer::find_file (const libport::path& path)
{
  return search_path.find_file(path) / path.basename();
}

UErrorValue
UServer::load_file(const std::string& base, UQueue& q, QueueType type)
{
  std::istream *is;
  bool isStdin = (base == std::string("/dev/stdin"));
  libport::Finally finally;
  if (isStdin)
    is = &std::cin;
  else
  {
    try
    {
      std::string file = find_file(base);
      is = new std::ifstream(file.c_str(), std::ios::binary);
      finally << boost::bind(boost::checked_delete<std::istream>, is);
    }
    catch (libport::file_library::Not_found&)
    {
      return UFAIL;
    }
    if (!*is)
      return UFAIL;
  }
  if (type == QUEUE_URBI)
  {
    q.push ((boost::format ("//#push 1 \"%1%\"\n") % base).str().c_str());
    finally << boost::bind(&UQueue::push, &q, "//#pop\n");
  }
  while (is->good ())
  {
    static char buf[BUFSIZ];
    is->read (buf, sizeof buf);
    q.push(buf, is->gcount());
  }
  return USUCCESS;
}


void
UServer::connection_add(UConnection* c)
{
  assert(c);
  if (c->uerror_ != USUCCESS)
    error(::DISPLAY_FORMAT1, (long)this,
	  __PRETTY_FUNCTION__,
	  "UConnection constructor failed");
  else
    connections_->add(c);
}

void
UServer::connection_remove(UConnection*)
{
  // Do not remove synchronously as it will destroy the object.
  // remove_closing will handle this.
}

UConnection&
UServer::ghost_connection_get()
{
  return *ghost_;
}

runner::Runner&
UServer::getCurrentRunner () const
{
  return dynamic_cast<runner::Runner&> (scheduler_->current_job());
}


/*--------------------------.
| Free standing functions.  |
`--------------------------*/

void
vdebug (const char* fmt, va_list args)
{
  ::urbiserver->vdebug (fmt, args);
}

void
debug (const char* fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  vdebug (fmt, args);
  va_end (args);
}
