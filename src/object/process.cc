/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cerrno>
#include <libport/csignal>
#include <libport/sys/prctl.h>
#include <libport/sys/stat.h>

#include <libport/sys/types.h>
#include <libport/sys/wait.h>
#include <libport/fcntl.h>
#include <vector>

#include <libport/lockable.hh>
#include <libport/path.hh>
#include <libport/thread.hh>
#include <libport/unistd.h>

#include <object/input-stream.hh>
#include <object/output-stream.hh>
#include <object/process.hh>
#include <object/symbols.hh>
#include <urbi/sdk.hh>

#define XRUN(Function, Args)                    \
  do {                                          \
    if ((Function Args) == -1)                  \
      errnoabort(#Function);                    \
  } while (false)

#define XCLOSE(Fd)                              \
  XRUN(close, (Fd))

namespace urbi
{
  namespace object
  {
    typedef std::vector<rProcess> processes_type;
    static processes_type processes;
    static libport::Lockable mutex;

    void
    Process::monitor_children()
    {
      // FIXME: Go in sleep mode when there are no processes (with a condition)
      while (true)
      {
        {
          libport::BlockLock lock(mutex);
          processes_type::iterator it = processes.begin();
          while (it != processes.end())
          {
            switch (waitpid((*it)->pid_, &(*it)->status_, WNOHANG))
            {
            case -1:
              errnoabort("waitpid");
            case 0:
              ++it;
              break;
            default:
              it = processes.erase(it);
              break;
            }
          }
        }
        // For some reason, Valgrind 3.6.0, on OS X Snow Leopard, sees
        // errors in this sleep.
        //
        //  Syscall param __semwait_signal(ts) points to unaddressable byte(s)
        //     at 0x1002AAEB6: __semwait_signal (in /usr/lib/libSystem.B.dylib)
        //     by 0x1002F7B13: sleep (in /usr/lib/libSystem.B.dylib)
        //     by 0x100DE7579: urbi::object::Process::monitor_children() (process.cc:55)
        //     by 0x10004A95D: libport::_startThread(void*) (function_template.hpp:1013)
        //     by 0x1002A9455: _pthread_start (in /usr/lib/libSystem.B.dylib)
        //     by 0x1002A9308: thread_start (in /usr/lib/libSystem.B.dylib)
        //   Address 0x1 is not stack'd, malloc'd or (recently) free'd
        sleep(1);
      }
    }

    Process::Process(const std::string& binary,
                     const arguments_type& argv)
      : name_(libport::path(binary).basename())
      , pid_(0)
      , binary_(binary)
      , argv_(argv)
      , status_(-1)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Process::Process(rProcess model)
      : name_(model->name_)
      , pid_(0)
      , binary_(model->binary_)
      , argv_(model->argv_)
      , status_(-1)
    {
      proto_add(model);
    }

    void
    Process::init(const std::string& binary,
                  const arguments_type& argv)
    {
      name_ = libport::path(binary).basename();
      binary_ = binary;
      argv_ = argv;
    }

    Process::~Process()
    {
    }

    std::string
    Process::name() const
    {
      return name_;
    }

    std::string
    Process::as_string() const
    {
      return libport::format("Process %s", name_);
    }

    void Process::run()
    {
      run_();
    }

    void Process::runTo(const std::string& outFile)
    {
      run_(outFile);
    }

    void Process::run_(boost::optional<std::string> outFile)
    {
      if (pid_)
        RAISE("Process was already run");

      int stdin_fd[2];
      if (pipe(stdin_fd))
        FRAISE("pipe failed: %s", libport::strerror(errno));

      int stdout_fd[2];
      int stderr_fd[2];
      if (!outFile)
      {
        if (pipe(stdout_fd))
        {
          int err = errno;
          XCLOSE(stdin_fd[0]);
          XCLOSE(stdin_fd[1]);
          FRAISE("pipe failed: %s:", libport::strerror(err));
        }
        if (pipe(stderr_fd))
        {
          int err = errno;
          XCLOSE(stdout_fd[0]);
          XCLOSE(stdout_fd[1]);
          XCLOSE(stdin_fd[0]);
          XCLOSE(stdin_fd[1]);
          FRAISE("pipe failed: %s:", libport::strerror(err));
        }
      }
      else
        stdout_fd[0] = -1;
      pid_ = fork();

      if (pid_ < 0)
      {
        int err = errno;
        XCLOSE(stdin_fd[0]);
        XCLOSE(stdin_fd[1]);
        if (!outFile)
        {
          XCLOSE(stdout_fd[0]);
          XCLOSE(stdout_fd[1]);
          XCLOSE(stderr_fd[0]);
          XCLOSE(stderr_fd[1]);
        }
        pid_ = 0;
        FRAISE("fork failed: %s", libport::strerror(err));
      }

      if (pid_)
      {
        // Parent.
        XCLOSE(stdin_fd[0]);
        if (!outFile)
        {
          XCLOSE(stdout_fd[1]);
          XCLOSE(stderr_fd[1]);
          slot_set(SYMBOL(stdout), new InputStream(stdout_fd[0], true));
          slot_set(SYMBOL(stderr), new InputStream(stderr_fd[0], true));
        }
        slot_set(SYMBOL(stdin), new OutputStream(stdin_fd[1], true));

        {
          libport::BlockLock lock(mutex);
          processes.push_back(this);
        }
      }
      else
      {
        // Child
        // Ask to be killed when the parent dies.
        prctl(PR_SET_PDEATHSIG, SIGKILL);
        XCLOSE(stdin_fd[1]);
        if (!outFile)
        {
          XCLOSE(stdout_fd[0]);
          XCLOSE(stderr_fd[0]);
        }
        else
        {
          int fd = open(outFile.get().c_str(), O_WRONLY | O_APPEND | O_CREAT,
                        00600);
          if (fd == -1)
            errnoabort("open " + outFile.get());
          stdout_fd[1] = stderr_fd[1] = fd;
        }
        XRUN(dup2, (stdout_fd[1], STDOUT_FILENO));
        XRUN(dup2, (stderr_fd[1], STDERR_FILENO));
        XRUN(dup2, (stdin_fd[0],  STDIN_FILENO));

        try
        {
          arguments_type argv;
          argv << binary_ << argv_;
          libport::exec(argv, true);
        }
        catch (...)
        { /* nothing */ }
        errnoabort("exec");
      }
    }

#define XKILL(Signal)                                           \
    do {                                                        \
      if (::kill(pid_, Signal) && errno != ESRCH)               \
        FRAISE("cannot kill %s (%s): %s",                       \
               pid_, #Signal, libport::strerror(errno));        \
    } while (false)

    void
    Process::kill()
    {
      if (!done())
        XKILL(SIGKILL);
    }

#undef XKILL

    void
    Process::join() const
    {
      while (status_ == -1)
        urbi::yield_for(500000);
    }

    bool
    Process::done() const
    {
      return status_ != -1;
    }

    rObject
    Process::status() const
    {
      rObject res = new Object;
      res->proto_add(Object::proto);
      bool exited = false;
      bool signaled = false;
      bool done = false;
      if (status_ == -1)
      {
        res->slot_set(SYMBOL(asString),
                      to_urbi(pid_ ? "running" : "not started"));
      }
      else
      {
        done = true;
        // On OS X, casts are made to int, which results in failures on
        // status_ which is const here.
        int status = status_;
        if (WIFEXITED(status))
        {
          exited = true;
          int rv = WEXITSTATUS(status);
          res->slot_set(SYMBOL(exitStatus), to_urbi(rv));
          res->slot_set(SYMBOL(asString),
                        to_urbi(libport::format("exited with status %s", rv)));
        }
        else
        {
          aver(WIFSIGNALED(status));
          signaled = true;
          int sig = WTERMSIG(status);
          res->slot_set(SYMBOL(exitSignal), to_urbi(sig));
          res->slot_set(SYMBOL(asString),
                        to_urbi(libport::format("killed by signal %s", sig)));
        }
      }

      res->slot_set(SYMBOL(done), to_urbi(done));
      res->slot_set(SYMBOL(exited), to_urbi(exited));
      res->slot_set(SYMBOL(signaled), to_urbi(signaled));

      return res;
    }

    void
    Process::initialize(CxxObject::Binder<Process>& bind)
    {
      bind(SYMBOL(asString), &Process::as_string);
      bind(SYMBOL(done), &Process::done);
      bind(SYMBOL(init), &Process::init);
      bind(SYMBOL(join), &Process::join);
      bind.var(SYMBOL(name), &Process::name_);
      bind(SYMBOL(run),  &Process::run );
      bind(SYMBOL(runTo),  &Process::runTo );
      bind(SYMBOL(kill),  &Process::kill );
      bind(SYMBOL(status),  &Process::status );

      libport::startThread(boost::function0<void>(&Process::monitor_children));
    }

    URBI_CXX_OBJECT_REGISTER(Process)
      : name_(libport::path("true").basename())
      , pid_(0)
      , binary_("/bin/true")
      , argv_()
      , status_(-1)
    {
      argv_ << binary_;
    }
  }
}
