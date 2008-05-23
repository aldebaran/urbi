/*! \file userver.cc
 *******************************************************************************

 File: userver.cc\n
 Implementation of the UServer class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */
//#define ENABLE_DEBUG_TRACES
#include <cassert>
#include <cstdlib>
#include <cstdarg>

#include <fstream>
#include <string>

#include <boost/format.hpp>
#if ! defined LIBPORT_URBI_ENV_AIBO
# include <boost/thread.hpp>
#endif

#include <libport/compiler.hh>
#include <libport/config.h>
#include <libport/cstdio>
#include <libport/finally.hh>
#include <libport/foreach.hh>
#include <libport/path.hh>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>

#include "urbi/uobject.hh"
#include "urbi/usystem.hh"

#include "kernel/userver.hh"
#include "kernel/utypes.hh"

#include "ast/ast.hh"
#include "ast/nary.hh"

#include "object/atom.hh"
#include "object/object-class.hh"
#include "object/primitives.hh"
#include "runner/runner.hh"
#include "scheduler/scheduler.hh"

#include "server-timer.hh"
#include "ubanner.hh"
#include "ughostconnection.hh"
#include "uobject.hh"
#include "uqueue.hh"

// Global server reference
UServer *urbiserver = 0;

// Formatting for the echo and error outputs.
const char* DISPLAY_FORMAT   = "[%ld] %-35s %s";
const char* DISPLAY_FORMAT1  = "[%ld] %-35s %s : %ld";
const char* DISPLAY_FORMAT2  = "[%d] %-35s %s : %d/%d";

UServer::UServer(const char* mainName)
  : search_path(),
    scheduler_ (new scheduler::Scheduler (boost::bind(&UServer::getTime,
						      boost::ref(*this)))),
    debugOutput (false),
    mainName_ (mainName),
    stopall (false)
{
#if ! defined NDEBUG
  server_timer.start ();
  server_timer.dump_on_destruction (std::cerr);
  TIMER_PUSH("server");
#endif
  ::urbiserver = this;
}

UErrorValue
UServer::load_init_file(const char* fn)
{
  DEBUG (("Loading %s...", fn));
  UErrorValue res = loadFile(fn, ghost_->recv_queue_get());
  if (res == USUCCESS)
  {
    DEBUG (("done\n"));
    ghost_->new_data_added_get() = true;
  }
  else
    DEBUG (("not found\n"));
  return res;
}

void
UServer::initialize()
{
  // Set the initial time to a valid value.
  updateTime();

  // Display the banner.
  {
    bool old_debugOutput = debugOutput;
    debugOutput = true;
    display(::HEADER_BEFORE_CUSTOM);

    int i = 0;
    char customHeader[1024];
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
      << libport::program_name << ": set IGNORE_URBI_U to ignore." << std::endl
      << libport::exit(EX_OSFILE);

  // Handle pluged UOBjects.
  // Create "uobject" in lobby where UObjects will be put.
  object::object_class->slot_set(SYMBOL(uobject_init),
                                 object::Primitive::fresh(&uobject_initialize));

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
# if ! defined LIBPORT_URBI_ENV_AIBO
  boost::recursive_mutex::scoped_lock lock(mutex);
# endif

  beforeWork ();

  work_handle_connections_ ();

  // To make sure that we get different times before and after every work
  // phase if we use a monotonic clock, update the time before and after
  // working.
  updateTime ();
  libport::utime_t next_time = scheduler_->work ();
  updateTime ();

  afterWork ();
  return next_time;
}

void
UServer::work_handle_connections_ ()
{
  // Scan currently opened connections for ongoing work
  foreach (UConnection* c, connections_)
    if (c->active_get())
    {
      if (!c->blocked_get())
	c->continue_send();

      c->error_check_and_send(UERROR_MEMORY_OVERFLOW);
      c->error_check_and_send(UERROR_MEMORY_WARNING);
      c->error_check_and_send(UERROR_SEND_BUFFER_FULL);
      c->error_check_and_send(UERROR_RECEIVE_BUFFER_FULL);
      c->error_check_and_send(UERROR_RECEIVE_BUFFER_CORRUPTED);

      if (c->new_data_added_get())
      {
	// used by loadFile and eval to
	// delay the parsing after the completion
	// of execute().
	c->new_data_added_get() = false;
	c->received("");
      }
    }
}

void
UServer::work_handle_stopall_ ()
{
  if (stopall)
  {
    foreach (UConnection* c, connections_)
      if (c->active_get() && c->has_pending_command ())
	c->drop_pending_commands ();
  }

  // Delete all connections with closing=true
  for (std::list<UConnection *>::iterator i = connections_.begin();
       i != connections_.end(); )
  {
    if ((*i)->closing_get())
    {
      delete *i;
      i = connections_.erase(i);
    }
    else
      i++;
  }

  stopall = false;
}

//! UServer destructor.
UServer::~UServer()
{
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

  char buf1[MAXSIZE_INTERNALMESSAGE];
  vsnprintf(buf1, sizeof buf1, s, args);
  char buf2[MAXSIZE_INTERNALMESSAGE];
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
  char buf[MAXSIZE_INTERNALMESSAGE];
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
  // FIXME: If shutdown is overriden in subclasses, this code is
  // not run.  Move it to the dtor?
  scheduler_->killall_jobs ();
}


void
UServer::updateTime()
{
  lastTime_ = getTime();
}

void
UServer::getCustomHeader (int, char* header, int)
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
UServer::loadFile (const std::string& base, UQueue& q, QueueType type)
{
  std::istream *is;
  bool isStdin = (base == std::string("/dev/stdin"));
  libport::Finally finally;
  if (isStdin)
    is = & std::cin;
  else
  {
    try
    {
      is = new std::ifstream(find_file (base).c_str (), std::ios::binary);
      finally << boost::bind(operator delete, is);
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
    q.push ((boost::format ("#push 1 \"%1%\"\n") % base).str().c_str());
    finally << boost::bind(&UQueue::push, &q, "#pop\n");
  }
  while (is->good ())
  {
    char buf[BUFSIZ];
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
    connections_.push_front (c);
}


void
UServer::connection_remove(UConnection* c)
{
  connections_.remove(c);
  echo(::DISPLAY_FORMAT1, long(this),
       __PRETTY_FUNCTION__,
       "Connection closed", long(c));
  delete c;
}


UConnection&
UServer::getGhostConnection ()
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
