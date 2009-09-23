/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/// \file kernel/userver.cc

//#define ENABLE_DEBUG_TRACES
#include <libport/cassert>
#include <libport/compiler.hh>
#include <libport/csignal>
#include <libport/cstdlib>

#include <fstream>
#include <string>

// Include our header first to avoid duplicating some of its tricks.
#include <kernel/userver.hh>

#include <boost/assign/list_of.hpp>
#include <libport/bind.hh>
#include <boost/checked_delete.hpp>
#include <libport/format.hh>
#include <boost/algorithm/string.hpp>

#include <libport/asio.hh>
#include <libport/backtrace.hh>
#include <libport/compiler.hh>
#include <libport/config.h>
#include <libport/cstdio>
#include <libport/detect-win32.h>
#include <libport/finally.hh>
#include <libport/foreach.hh>
#include <libport/lexical-cast.hh>
#include <libport/path.hh>
#include <libport/package-info.hh>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>
#include <libport/tokenizer.hh>

#include <kernel/config.h>

#include <urbi/exit.hh>
#include <urbi/uobject.hh>
#include <urbi/usystem.hh>

#include <kernel/userver.hh>
#include <kernel/utypes.hh>

#include <ast/ast.hh>
#include <ast/nary.hh>

#include <object/lobby.hh>
#include <object/object.hh>
#include <object/primitive.hh>
#include <object/root-classes.hh>
#include <object/symbols.hh>
#include <object/system.hh>

#include <runner/runner.hh>
#include <runner/shell.hh>
#include <runner/sneaker.hh>

#include <sched/scheduler.hh>

#include <kernel/connection-set.hh>
#include <kernel/lock.hh>
#include <kernel/server-timer.hh>
#include <kernel/ubanner.hh>
#include <kernel/ughostconnection.hh>
#include <kernel/uobject.hh>
#include <kernel/uqueue.hh>

using libport::program_name;
using object::objects_type;
using object::rObject;

namespace kernel
{
  /*----------------.
  | Free standing.  |
  `----------------*/

  // Global server reference
  UServer *urbiserver = 0;

  static
  void
  init_error()
  {
    static bool ignore = getenv("IGNORE_URBI_U");
    if (!ignore)
      urbi::Exit(EX_OSFILE, "set IGNORE_URBI_U to ignore.");
  }



  /*----------.
  | UServer.  |
  `----------*/

  UServer::UServer(const char* mainName)
    : mode_(mode_kernel)
    , search_path(boost::assign::list_of
                  (std::string(libport::xgetenv("URBI_PATH")))
                  (std::string(libport::xgetenv("URBI_ROOT", URBI_ROOT))
                   + "/share/gostai"),
                  ":")
    , scheduler_(new sched::Scheduler(boost::bind(&UServer::getTime,
                                                  boost::ref(*this))))
    , mainName_(mainName)
    , stopall(false)
    , connections_(new kernel::ConnectionSet)
    , thread_id_(pthread_self())
    , io_(*new boost::asio::io_service())
  {
    lock_check(*this);
    TIMER_INIT();
    TIMER_PUSH("server");
    urbiserver = this;
#ifndef WIN32
    // Use line buffering even when stdout is not a TTY.
    setlinebuf(stdout);
#endif
  }

  UErrorValue
  UServer::load_init_file(const char* fn)
  {
    DEBUG(("Loading %s...", fn));
    UErrorValue res = load_file(fn, ghost_->recv_queue_get());
    if (res == USUCCESS)
    {
      DEBUG(("done\n"));
      ghost_->received("");
    }
    else
      DEBUG (("not found\n"));
    return res;
  }

  void
  UServer::xload_init_file(const char* fn)
  {
    if (load_init_file(fn) != USUCCESS)
    {
      std::cerr
        << program_name() << ": cannot load " << fn << "." << std::endl
        << program_name() << ": path: " << search_path << std::endl;
      init_error();
    }
  }

  void
  UServer::revision_check()
  {
    // Check that revision bw C++ and Urbi match.
    xload_init_file("urbi/package-info.u");

    // Force the processing until PackageInfo is defined.
    while (!object::system_class->slot_has(SYMBOL(PackageInfo)))
      work();
    object::rObject PackageInfo =
      object::system_class->slot_get(SYMBOL(PackageInfo));
    std::string urbi_rev =
      PackageInfo->slot_get(SYMBOL(revision)).get<std::string>();

    // C++ revision.
    std::string cxx_rev = package_info().get("revision");
    if (urbi_rev != cxx_rev)
    {
      std::cerr
        << program_name() << ": revison mismatch between C++ and Urbi.\n"
        << program_name() << ":   kernel revision: " << cxx_rev << "\n"
        << program_name() << ":   urbi.u revision: " << urbi_rev << std::endl;
      init_error();
    }
  }

#if !defined WIN32 && !defined _MSC_VER
  namespace
  {
    static void
    install_ice_catcher(void (*catcher)(int))
    {
      if (getenv("NO_ICE_CATCHER"))
        return;
      signal(SIGABRT, catcher);
      signal(SIGBUS,  catcher);
      signal(SIGSEGV, catcher);
    }

    ATTRIBUTE_NORETURN static void hard_ice(int i);

    static void hard_ice(int i)
    {
      std::cerr
        << program_name()
        << ": killed with signal " << i << " (" << strsignal(i)
        << ") while trying to debug" << std::endl;
      libport::signal(i, SIG_DFL);
      if (kill(getpid(), i))
        perror("kill");
      // Pacify noreturn.
      exit(EX_HARD);
    }

    static
    inline
    runner::Runner&
    operator<<(runner::Runner& r, const std::string& s)
    {
      // Avoid dynamic allocation, we're treating an ICE here.
      static const std::string notag = "";
      r.send_message(notag, s);
      return r;
    }

    static
    inline
    runner::Runner&
    operator<<(runner::Runner& r, const char* s)
    {
      return r << std::string(s);
    }

    template<typename Cont>
    static
    inline
    runner::Runner&
    operator<<(runner::Runner& r, const Cont& c)
    {
      foreach (const std::string& s, c)
        r << s;
      return r;
    }

    static void ice(int i)
    {
      install_ice_catcher(hard_ice);

      // If we have a job currently running, use it to signal the error,
      // otherwise try to use the sneaker which must have been created
      // (or we have an error very early on and we are in deep trouble).
      runner::Runner& r = dbg::runner_or_sneaker_get();

      // Display information from the least demanding, to the most
      // demanding.  For instance "ps" requires that the kernel is not
      // too broken, otherwise it will segv again.  Hence call it
      // last.
      r << ""
        << "    **********************"
        << "    *** INTERNAL ERROR ***"
        << "    **********************"
        << ""
        << ("The urbi kernel was killed by signal "
            + string_cast(i) + ": " + strsignal(i) + ".")
        << "Please report this bug to " PACKAGE_BUGREPORT " with this report,"
        << "core dump if any, and what code/situation triggered it."
        << ""
        << "Trying to give more information."
        << "Please include it in the report."
        << ""
        << "---------- VERSION ----------"
        << libport::lines(UServer::package_info().signature())
        << ""
        << "---------- CURRENT C++ BACKTRACE ----------"
        << libport::Backtrace()
        << ""
        << "---------- CURRENT URBI BACKTRACE ----------";
      static const std::string notag = "";
      r.show_backtrace(notag);
      if (object::system_class->slot_has(SYMBOL(ps)))
      {
        r << ""
          << "---------- PS ----------";
        object::system_class->call(SYMBOL(ps));
      }
      exit(EX_SOFTWARE);
    }
  }
#endif

  std::string
  UServer::banner_get() const
  {
    const std::string marker =
      "**********************************************************";
    std::string res =
      marker + "\n"
      + package_info().signature() + "\n"
      + "\n";

    const std::string& custom = custom_banner_get();
    if (!custom.empty())
      res +=
        custom + "\n"
        + "\n";

    res +=
      "URBI comes with ABSOLUTELY NO WARRANTY.\n"
      "This software can be used under certain conditions;\n"
      "see LICENSE file for details.\n"
      "\n"
      "See http://www.urbiforge.com for news and updates.\n"
      + marker + "\n";
    return res;
  }

  void
  UServer::initialize(bool interactive)
  {
#if !defined WIN32 && !defined _MSC_VER
# if !defined NDEBUG
    static bool catch_ices = !getenv("URBI_NO_ICE_CATCHER");
    if (catch_ices)
# endif
      install_ice_catcher(ice);
#endif
    // Set the initial time to a valid value.
    updateTime();


    /*---------.
    | Banner.  |
    `---------*/
    DEBUG((banner_get()));
    DEBUG(("Ready.\n"));

    /*--------.
    | Setup.  |
    `--------*/

    // The order is important: ghost connection, plugins, urbi.u.

    // Ghost connection
    DEBUG(("Setting up ghost connection..."));
    ghost_ = new UGhostConnection(*this, interactive);
    DEBUG(("done\n"));

    revision_check();
    xload_init_file("urbi/urbi.u");

    // Handle plugged UObjects.
    // Create "uobject" in lobby where UObjects will be put.
    object::Object::proto->slot_set(SYMBOL(uobject_init),
                                   new object::Primitive(&uobject_initialize));

    // Force processing of urbi.u.
    while (!object::Object::proto->slot_has(SYMBOL(loaded)))
      work();
    mode_ = mode_user;
    object::Object::proto->slot_remove(SYMBOL(loaded));
  }


  void
  UServer::main(int, const char*[])
  {
    // FIXME: Save argv into Urbi world.
  }

  void
  UServer::beforeWork()
  {
  }

  void
  UServer::afterWork()
  {
  }

  static object::rObject
  method_wrap(object::rObject tgt, libport::Symbol m, objects_type args,
              const objects_type&)
  {
    return tgt->call(m, args);
  }

  libport::utime_t
  UServer::work()
  {
    static bool report = getenv("REPORT");
    static int niter = 0;
    static libport::utime_t sumtime = 0, mintime = 10000000, maxtime = 0;
    static libport::utime_t rsumtime = 0, rmintime = 10000000, rmaxtime = 0;
    static unsigned int nzero = 0;

    beforeWork();

    // To make sure that we get different times before and after every work
    // phase if we use a monotonic clock, update the time before and after
    // working.
    libport::utime_t next_time;
    if (report)
    {
      updateTime();
      libport::utime_t ctime = libport::utime();
      next_time = scheduler_->work ();
      libport::utime_t rtime = next_time? std::max(0LL, next_time - ctime):0;
      ctime = libport::utime() - ctime;
      updateTime();
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
        std::cerr << "## work time(us)  min: " << mintime
                  << "   max: " << maxtime
                  << "   avg: " << sumtime/niter
                  << std::endl
                  << "  sched interval(us)  zero-ratio: "
                  << float(nzero) / float(niter)
                  << "   min: " << rmintime
                  << "   max: " << rmaxtime
                  << "   avg: " << rsumtime/(0.01 + niter-nzero)
                  << std::endl;
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
    else
    {
      updateTime();
      next_time = scheduler_->work ();
      updateTime();
    }
    {
      libport::BlockLock lock(_async_jobs_lock);
      foreach (AsyncJob& job, _async_jobs)
      {
        object::rPrimitive p = new object::Primitive
          (boost::bind(method_wrap, job.target, job.method, job.args, _1));
        sched::rJob interpreter =  new runner::Interpreter
          (*ghost_connection_get().shell_get(), p, job.method, job.args);
        scheduler_->add_job(interpreter);
      }
      _async_jobs.clear();
    }
    work_handle_stopall_();
    afterWork();
    return next_time;
  }

  void
  UServer::schedule(object::rObject o, libport::Symbol method,
                    const object::objects_type& args)
  {
    libport::BlockLock lock(_async_jobs_lock);
    _async_jobs.push_back(AsyncJob(o, method, args));
  }

  UServer::AsyncJob::AsyncJob(object::rObject t, libport::Symbol m,
                              const object::objects_type& a)
    : target(t)
    , method(m)
    , args(a)
  {}

  static void bounce_disconnection(UConnection* uc)
  {
    // This is executed from a job: we have a runner.
    uc->lobby_get()->disconnect();
    delete uc;
  }

  void
  UServer::work_handle_stopall_()
  {
    if (stopall)
    {
      foreach (UConnection* c, *connections_)
        if (c->active_get() && c->has_pending_command())
          c->drop_pending_commands();
    }

    // Call bounce_disconnection() inside GhostConnection's shell for each
    // closed connection.
    foreach (UConnection* c, *connections_)
    {
      if (c->closing_get())
      {
	ghost_->shell_get()->insert_oob_call(
	  boost::bind(&bounce_disconnection, c));
      }
    }

    connections_->connections_.remove_if(
     boost::bind(&UConnection::closing_get, _1));

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
  UServer::display(const char* s)
  {
    effectiveDisplay(s);
  }


  void
  UServer::shutdown()
  {
    scheduler_->killall_jobs();
  }


  void
  UServer::updateTime()
  {
    lastTime_ = getTime();
  }

  std::string
  UServer::custom_banner_get() const
  {
    return "";
  }

  std::string
  UServer::find_file(const libport::path& path) const
  {
    return search_path.find_file(path) / path.basename();
  }

  UErrorValue
  UServer::load_file(const std::string& base, UQueue& q, QueueType type)
  {
    std::istream *is;
    libport::Finally finally;
    if (base == "/dev/stdin")
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
      q.push((boost::format("//#push 1 \"%1%\"\n") % base).str().c_str());
      finally << boost::bind(&UQueue::push, &q, "//#pop\n");
    }
    while (is->good())
    {
      static char buf[BUFSIZ];
      is->read(buf, sizeof buf);
      q.push(buf, is->gcount());
    }
    return USUCCESS;
  }


  void
  UServer::connection_add(UConnection* c)
  {
    assert(c);
    if (c->uerror_ != USUCCESS)
      DEBUG(("UConnection constructor failed"));
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
  UServer::getCurrentRunner() const
  {
#ifndef SCHED_CORO_OSTHREAD
    if (thread_id_ != pthread_self())
      pabort("UObject API isn't thread safe. "
             "Do the last call within main thread.");
#endif
    return dynamic_cast<runner::Runner&> (scheduler_->current_job());
  }

  bool
  UServer::isAnotherThread() const
  {
    return thread_id_ != pthread_self();
  }
}
