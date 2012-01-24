/*
 * Copyright (C) 2005-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file kernel/userver.cc

#include <boost/date_time/posix_time/posix_time.hpp>

#include <libport/cassert>
#include <libport/csignal>
#include <libport/cstdlib>

#include <fstream>
#include <string>

#include <stdlib.h>
#include <fcntl.h>

// Include our header first to avoid duplicating some of its tricks.
#include <urbi/kernel/userver.hh>

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
#include <libport/program-name.hh>
#include <libport/sysexits.hh>
#include <libport/time.hh>
#include <libport/tokenizer.hh>

#include <kernel/config.h>

#include <urbi/exit.hh>
#include <urbi/uobject.hh>

#include <urbi/kernel/userver.hh>
#include <urbi/kernel/utypes.hh>

#include <ast/ast.hh>
#include <ast/nary.hh>

#include <libport/package-info.hh>
#include <urbi/package-info.hh>
#include <urbi/object/date.hh>
#include <urbi/object/global.hh>
#include <urbi/object/lobby.hh>
#include <urbi/object/object.hh>
#include <urbi/object/primitive.hh>
#include <urbi/object/tag.hh>
#include <object/root-classes.hh>
#include <object/socket.hh>
#include <urbi/object/symbols.hh>
#include <object/system.hh>

#include <runner/runner.hh>
#include <runner/shell.hh>
#include <runner/sneaker.hh>

#include <sched/scheduler.hh>

#include <kernel/connection-set.hh>
#include <kernel/server-timer.hh>
#include <kernel/ughostconnection.hh>
#include <kernel/uobject.hh>

using libport::program_name;
using object::objects_type;
using object::rObject;

GD_CATEGORY(Urbi.UServer);

namespace kernel
{
  // Global server reference
  UServer* urbiserver = 0;

  std::string current_function_name()
  {
    const ::runner::Interpreter::call_stack_type& bt =
      interpreter().call_stack_get();
    if (bt.size() > 1)
      return bt[bt.size() - 2].first.name_get();
    else
      return "<urbi-toplevel>";
  }

  ast::loc current_location()
  {
    if (const ast::Ast* ast = interpreter().innermost_node())
      return ast->location_get();
    else
      return ast::loc();
  }

  static
  void
  init_error()
  {
    const char* var = "URBI_IGNORE_URBI_U";
    static bool ignore = getenv(var);
    if (!ignore)
      throw urbi::Exit(EX_OSFILE,
                       libport::format("%s: set %s to ignore.",
                                       program_name(), var));
  }

  static
  size_t
  waker_socket_on_read(sched::Scheduler* sc, const void*, size_t sz)
  {
    sc->signal_world_change();
    return sz;
  }

  sched::Scheduler&
  scheduler()
  {
    return runner().scheduler_get();
  }


  /*----------.
  | UServer.  |
  `----------*/

  UServer::UServer(UrbiRoot& urbi_root)
    : mode_(mode_kernel)
    , search_path(boost::assign::list_of
                  (std::string(libport::xgetenv("URBI_PATH")))
                  (urbi_root.share_dir()),
                  ":")
    , opt_banner_(true)
    , fast_async_jobs_start_(false)
    , scheduler_(new sched::Scheduler(boost::bind(&UServer::getTime,
                                                  boost::ref(*this))))
    , stopall(false)
    , connections_(new kernel::ConnectionSet)
    , interactive_(true)
    , thread_id_(pthread_self())
    , urbi_root_(urbi_root)
  {
    TIMER_INIT();
    TIMER_PUSH("server");
    urbiserver = this;
    // If someone locks the big kernel lock and we're sleeping, wake
    // up.
    big_kernel_lock_.setOnLock(boost::bind(&UServer::wake_up, this));
#ifndef WIN32
    // Use line buffering even when stdout is not a TTY.
    setlinebuf(stdout);
#endif
    // Tell the scheduler we will handle job destruction ourself.
    scheduler_->keep_terminated_jobs_set(true);
    object::root_classes_initialize();
  }

  void
  UServer::xload_init_file(const char* fn)
  {
    if (load_file(fn, *ghost_) != USUCCESS)
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
        << libport::lines(::urbi::package_info().signature())
        << ""
        << "---------- CURRENT C++ BACKTRACE ----------"
        << libport::Backtrace()
        << ""
        << "---------- CURRENT Urbi BACKTRACE ----------";
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

  static void sigint_handler(int)
  {
    static libport::Time last_call = boost::posix_time::min_date_time;

    if (kernel::urbiserver->interactive_get()
        && libport::time::ms(1500) < libport::time::now() - last_call)
    {
      // First level interrupt.
      runner::rShell shell = urbiserver->ghost_connection_get().shell_get();
      shell->pending_commands_clear();
      shell->async_throw(sched::StopException(-1, object::void_class), true);
      last_call = libport::time::now();
      urbiserver->schedule(urbiserver->ghost_connection_get().lobby_get(),
                           SYMBOL(sigint_interrupt));
    }
    else
    {
      // Second level interrupt.
      // Restore the default handler in case there is a third SIGINT.
      signal(SIGINT, SIG_DFL);
      urbiserver->schedule(urbiserver->ghost_connection_get().lobby_get(),
                           SYMBOL(sigint_shutdown));
    }
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
#if defined WIN32
    // Globally disable text mode transformations for open() and pipe().
    // see http://msdn.microsoft.com/en-us/library/61dstksf%28v=VS.80%29.aspx
    _set_fmode(_O_BINARY);
#endif
    // Setup the handler for Ctrl+C.
    signal(SIGINT, sigint_handler);
    boost::posix_time::ptime
      now(boost::posix_time::microsec_clock::local_time());
    libport::utime_reference_set(libport::utime());

    // Set the initial time to a valid value.
    updateTime();

    // Setup wakeup mechanism when wake_up_pipe.second is written on.
    boost::asio::io_service& io = get_io_service();
    wake_up_pipe_ = std::make_pair(new libport::ConcreteSocket(io),
                                   new libport::Socket(io));
    libport::makePipe(wake_up_pipe_, io);
    dynamic_cast<libport::ConcreteSocket*>(wake_up_pipe_.first)
    ->onRead(boost::bind(&waker_socket_on_read, scheduler_, _1, _2));

    /*--------.
    | Setup.  |
    `--------*/

    // The order is important: ghost connection, plugins, urbi.u.

    // Ghost connection
    {
      GD_PUSH_TRACE("setting up ghost connection.");
      ghost_ = new UGhostConnection(*this, interactive);
    }


    /*--------------.
    | urbi/urbi.u.  |
    `--------------*/
    xload_init_file("urbi/urbi.u");

    // Handle plugged UObjects.
    // Create "uobject" in lobby where UObjects will be put.
    object::Object::proto->slot_set
      (SYMBOL(uobjectInit),
       new object::Primitive(&urbi::uobjects::uobject_initialize));

    // Force processing of urbi.u.
    {
      GD_PUSH_TRACE("going to work until urbi.u is processed.");
      while (!object::Object::proto->slot_has(SYMBOL(loaded)))
        work();
    }
    mode_ = mode_user;
    object::Object::proto->slot_remove(SYMBOL(loaded));
    GD_INFO_TRACE("urbi.u has been processed.");

    urbi::object::system_class
      ->setSlot(SYMBOL(timeReference),
                new urbi::object::Date(now));

    // urbiscript is up and running.  Send local.u and the banner to
    // the ghostconnection too.
    ghost_->initialize();

    fast_async_jobs_tag_ = new object::Tag(new sched::Tag("fastAsyncJob"));
    fast_async_jobs_job_ =
      new runner::Interpreter(ghost_->lobby_get(),
                               scheduler_get(),
                               boost::bind(&UServer::fast_async_jobs_run_,this),
                               rObject(ghost_->lobby_get()),
                               SYMBOL(FastAsyncJobs));
    scheduler_->add_job(fast_async_jobs_job_);
    sched::rJob poll =
      new runner::Interpreter(*ghost_->shell_get().get(),
                              object::system_class->slot_get(SYMBOL(pollLoop)),
                              SYMBOL(pollLoop));
    scheduler_->idle_job_set(poll);
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

  void
  UServer::async_jobs_process_()
  {
    std::vector<AsyncJob> jobs;
    {
      libport::BlockLock lock(async_jobs_lock_);
      std::swap(jobs, async_jobs_);
    }
    foreach (AsyncJob& job, jobs)
    {
      // Clone the shell to run asynchronous jobs.
      runner::Interpreter* interpreter = 0;
      if (job.target)
      {
        object::rPrimitive p = new object::Primitive
          (boost::bind(method_wrap, job.target, job.method, job.args, _1));
        interpreter = new runner::Interpreter
          (*ghost_connection_get().shell_get(), p, job.method, job.args);
      }
      else if (job.callback)
      {
        interpreter = new runner::Interpreter
          (ghost_connection_get().lobby_get(), *scheduler_, job.callback,
           ghost_connection_get().lobby_get(), job.method);
      }
      else
        pabort("Uninitialized AsyncJob in async_jobs");
      // Clean the tag stack because the shell could be frozen and
      // scheduled jobs have no reason to inherit frozen tags.
      interpreter->tag_stack_clear();

      // FIXME: clean the stack of the interpreter, this cause strange
      // issue where scheduled jobs inherit the stack of the primary
      // shell.
      scheduler_->add_job(sched::rJob(interpreter));
    }
    // FIXME: work around a scheduler bug when adding a job from
    // outside work.  It is supposed to work, but turns out the new
    // job is only scheduled after we force an extra work() run.
    if (!jobs.empty())
      wake_up();
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

    if (fast_async_jobs_start_)
      fast_async_jobs_tag_->as<object::Tag>()->unfreeze();

    dead_jobs_.clear();
    dead_jobs_ = scheduler_->terminated_jobs_get();
    scheduler_->terminated_jobs_clear(); // let refcounting do the job.

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
    if (!async_jobs_.empty())
      async_jobs_process_();
    work_handle_stopall_();
    afterWork();
    big_kernel_lock_.check();
    return next_time;
  }

  void
  UServer::schedule(object::rObject o, libport::Symbol method,
                    const object::objects_type& args)
  {
    schedule(AsyncJob(o, method, args));
  }

  void
  UServer::schedule(libport::Symbol method, boost::function0<void> callback)
  {
    schedule(AsyncJob(callback, method));
  }

  struct AsyncJobException : public sched::SchedulerException
  {
    COMPLETE_EXCEPTION(AsyncJobException);
  };

  void
  UServer::schedule_fast(boost::function0<void> j)
  {
    libport::BlockLock bl(fast_async_jobs_lock_);
    bool empty = fast_async_jobs_.empty();
    fast_async_jobs_ << j;
    if (empty)
    {
      // We cannot wake the job directly from here (an other thread), so
      // ask work() to do it for us.
      fast_async_jobs_start_ = true;
      wake_up();
    }
  }

  void
  UServer::fast_async_jobs_run_()
  {
    libport::Finally f;
    getCurrentRunner().apply_tag(fast_async_jobs_tag_->as<object::Tag>(), &f);
    unsigned count = 0;
    try{
    while (true)
    {
      fast_async_jobs_start_ = false;
      count++;
      GD_FINFO_TRACE("Fast async jobs running %s", count);
      {
        std::vector<boost::function0<void> > jobs;
        {
          libport::BlockLock bl(fast_async_jobs_lock_);
          std::swap(jobs, fast_async_jobs_);
        }
        GD_FINFO_TRACE("Fast async jobs processing %s operations",
                       jobs.size());
        foreach(boost::function0<void>& j, jobs)
          j();
      }
      GD_INFO_TRACE("Fast async job sleeping");
      fast_async_jobs_tag_->as<object::Tag>()->freeze();
      GD_INFO_TRACE("Fast async job waking up");
        // Wake up.
    }}
    catch(...)
    {
      GD_INFO_TRACE("Fast async job exiting");
      throw;
    }
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
    GD_INFO_TRACE("Shutting down: killing all jobs");
    scheduler_->killall_jobs();
    GD_INFO_TRACE("Shutting down: done");
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
  UServer::load_file(const std::string& base, UConnection& connection)
  {
    GD_FPUSH_DUMP("looking for %s", base);
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
        GD_FINFO_DUMP("loading %s", file);
      }
      catch (libport::file_library::Not_found&)
      {
        GD_FINFO_DUMP("file not found: %s", base);
        errno = ENOENT;
        return UFAIL;
      }
      if (!*is)
        return UFAIL;
    }
    connection.received(libport::format("//#push 1 \"%1%\"\n", base));
    typedef void(UConnection::*received_type)(const std::string&);
    finally << boost::bind
      (static_cast<received_type>(&UConnection::received),
       &connection, "//#pop\n");
    while (is->good())
    {
      static char buf[BUFSIZ];
      is->read(buf, sizeof buf);
      connection.received(buf, is->gcount());
    }
    return USUCCESS;
  }


  void
  UServer::connection_add(UConnection* c)
  {
    aver(c);
    if (c->uerror_ != USUCCESS)
      GD_INFO_TRACE("UConnection constructor failed");
    else
      connections_->add(c);
  }

  UConnection&
  UServer::ghost_connection_get()
  {
    return *ghost_;
  }

  bool
  UServer::isAnotherThread() const
  {
    return thread_id_ != pthread_self();
  }

  void
  UServer::wake_up()
  {
    // Do a synchronous write, which will wake the main loop up from poll.
    char data = 1;
    wake_up_pipe_.second->syncWrite(&data, 1);
  }

  boost::asio::io_service&
  UServer::get_io_service ()
  {
    return *object::Socket::get_default_io_service().get();
  }

  void
  UServer::connection_remove(UConnection& connection)
  {
    delete &connection;
    connections_->connections_.remove(&connection);
  }
}
