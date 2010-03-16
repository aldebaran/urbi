/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
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

#include <urbi/object/lobby.hh>
#include <urbi/object/object.hh>
#include <urbi/object/primitive.hh>
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
    static bool ignore = getenv("URBI_IGNORE_URBI_U");
    if (!ignore)
      urbi::Exit(EX_OSFILE, "set URBI_IGNORE_URBI_U to ignore.");
  }



  static size_t waker_socket_on_read(sched::Scheduler* sc, const void*,
                                     size_t sz)
  {
    sc->signal_world_change();
    return sz;
  }

  /*----------.
  | UServer.  |
  `----------*/

  UServer::UServer(UrbiRoot& urbi_root)
    : mode_(mode_kernel)
    , search_path(boost::assign::list_of
                  (std::string(libport::xgetenv("URBI_PATH")))
                  (urbi_root.share_path()),
                  ":")
    , opt_banner_(true)
    , scheduler_(new sched::Scheduler(boost::bind(&UServer::getTime,
                                                  boost::ref(*this))))
    , stopall(false)
    , connections_(new kernel::ConnectionSet)
    , thread_id_(pthread_self())
    , io_(*new boost::asio::io_service())
    , urbi_root_(urbi_root)
  {
    lock_check(*this);
    TIMER_INIT();
    TIMER_PUSH("server");
    urbiserver = this;
#ifndef WIN32
    // Use line buffering even when stdout is not a TTY.
    setlinebuf(stdout);
#endif
    // Setup wakeup mechanism when wake_up_pipe.second is written on.
    wake_up_pipe_ = std::make_pair(new libport::ConcreteSocket(io_),
                                   new libport::Socket(io_));
    libport::makePipe(wake_up_pipe_, io_);
    dynamic_cast<libport::ConcreteSocket*>(wake_up_pipe_.first)
    ->onRead(boost::bind(&waker_socket_on_read, scheduler_, _1, _2));
    synchronizer_.setOnLock(boost::bind(&UServer::wake_up, this));
    // Tell the scheduler we will handle job destruction ourself.
    scheduler_->keep_terminated_jobs_set(true);
  }

  UErrorValue
  UServer::load_init_file(const char* fn)
  {
    UErrorValue res = load_file(fn, ghost_->recv_queue_get());
    if (res == USUCCESS)
      ghost_->received("");
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

#if !defined WIN32 && !defined _MSC_VER
  namespace
  {
    static void
    install_ice_catcher(void (*catcher)(int))
    {
      if (getenv("URBI_NO_ICE_CATCHER"))
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


    /*--------.
    | Setup.  |
    `--------*/

    // The order is important: ghost connection, plugins, urbi.u.

    // Ghost connection
    DEBUG(("Setting up ghost connection..."));
    ghost_ = new UGhostConnection(*this, interactive);
    DEBUG(("done\n"));

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

    // urbiscript is up and running.  Send local.u and the banner to
    // the ghostconnection too.
    ghost_->initialize();
    object::rPrimitive p = new object::Primitive(
      object::Primitive::value_type(
        boost::bind(&UServer::handle_synchronizer_, this, _1)));
    sched::rJob interpreter =
    new runner::Interpreter(
                            *ghost_->shell_get().get(),
                            p->as<object::Object>(),
                            SYMBOL(handle_synchronizer),
                            object::objects_type());
    scheduler_->add_job(interpreter);
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
    static bool report = getenv("URBI_REPORT");
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
      libport::BlockLock lock(async_jobs_lock_);
      foreach (AsyncJob& job, async_jobs_)
      {
        object::rPrimitive p = new object::Primitive
          (boost::bind(method_wrap, job.target, job.method, job.args, _1));
        sched::rJob interpreter =  new runner::Interpreter
          (*ghost_connection_get().shell_get(), p, job.method, job.args);
        scheduler_->add_job(interpreter);
      }
      async_jobs_.clear();
    }
    work_handle_stopall_();
    afterWork();
    return next_time;
  }

  void
  UServer::schedule(object::rObject o, libport::Symbol method,
                    const object::objects_type& args)
  {
    libport::BlockLock lock(async_jobs_lock_);
    async_jobs_.push_back(AsyncJob(o, method, args));
  }

  UServer::AsyncJob::AsyncJob(object::rObject t, libport::Symbol m,
                              const object::objects_type& a)
    : target(t)
    , method(m)
    , args(a)
  {}

  static void bounce_disconnection(UConnection* uc)
  {
    // We cannot closed it yet because it is used to close all other
    // connections.
    if (&kernel::urbiserver->ghost_connection_get() == uc)
      return;

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
  UServer::find_file(const libport::path& path) const
  {
    return search_path.find_file(path) / path.basename();
  }

  UErrorValue
  UServer::load_file(const std::string& base, UQueue& q, QueueType type)
  {
    DEBUG(("Looking for %s...", base));
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
        DEBUG(("Loading %s...", file));
      }
      catch (libport::file_library::Not_found&)
      {
        DEBUG(("not found\n"));
        errno = ENOENT;
        return UFAIL;
      }
      if (!*is)
        return UFAIL;
    }
    if (type == QUEUE_URBI)
    {
      q.push(libport::format("//#push 1 \"%1%\"\n", base));
      finally
        << boost::bind(static_cast<void(UQueue::*)(const char*)>
                       (&UQueue::push),
                       &q, "//#pop\n");
    }
    while (is->good())
    {
      static char buf[BUFSIZ];
      is->read(buf, sizeof buf);
      q.push(buf, is->gcount());
    }
    DEBUG(("done\n"));
    return USUCCESS;
  }


  void
  UServer::connection_add(UConnection* c)
  {
    aver(c);
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
    // FIXME: check that main thread is currently in handle_synchronizer_().
    return dynamic_cast<runner::Runner&> (scheduler_->current_job());
  }

  runner::Runner*
  UServer::getCurrentRunnerOpt() const
  {
    sched::Job* j = scheduler_->current_job_opt();
    if (j)
      return dynamic_cast<runner::Runner*>(j);
    return 0;
  }

  bool
  UServer::isAnotherThread() const
  {
    return thread_id_ != pthread_self();
  }

  object::rObject
  UServer::handle_synchronizer_(const object::objects_type&)
  {
    runner::Runner& r = getCurrentRunner();
    sched::jobs_type dead_jobs;
    while (true)
    {
      // Handle job destruction with a one-cycle delay (The scheduler seems
      // to keep reference to dead jobs for one cycle).
      dead_jobs.clear();
      dead_jobs = scheduler_->terminated_jobs_get();
      scheduler_->terminated_jobs_clear(); // let refcounting do the job.

      r.side_effect_free_set(true);
      // We cannot yield within check, or an other OS thread will jump stack!
      r.non_interruptible_set(true);
      synchronizer_.check();
      r.non_interruptible_set(false);
      r.yield_until_things_changed();
    }
    return object::void_class;
  }

  void
  UServer::wake_up()
  {
    // Do a synchronous write, which will wake the main loop up from poll.
    char data = 1;
    wake_up_pipe_.second->syncWrite(&data, 1);
  }
}
