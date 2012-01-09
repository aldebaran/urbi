/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cerrno>
#include <libport/containers.hh>
#include <libport/csignal>
#include <libport/debug.hh>
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

#include <urbi/object/job.hh>
#include <runner/runner.hh>
#include <object/urbi/input-stream.hh>
#include <object/urbi/output-stream.hh>
#include <object/urbi/process.hh>
#include <urbi/object/symbols.hh>
#include <urbi/sdk.hh>


#define XRUN(Function, Args)                    \
  do {                                          \
    if ((Function Args) == -1)                  \
      errnoabort(#Function);                    \
  } while (false)

#define XCLOSE(Fd)                              \
  XRUN(close, (Fd))

GD_CATEGORY(Urbi.Object.Process);

namespace urbi
{
  namespace object
  {

    /*-----------------.
    | Implementation.  |
    `-----------------*/

    typedef std::vector<rProcess> processes_type;
    static processes_type processes;

    void
    Process::monitor_child(Process* owner)
    {
      int status;
      int res = waitpid(owner->pid_, &status, 0);
      LIBPORT_USE(res);
      assert_gt(res, 0);
      {
        KERNEL_BLOCK_LOCK();
        owner->status_ = status;
        foreach (runner::Runner* job, owner->joiners_)
          // FIXME: Is there a risk we cancel a double freeze?
          job->frozen_set(false);
        // The Process no longer needs to be protected against
        // ref-counting.
        owner->ward_ = 0;
      }
    }

    Process::Process(const std::string& binary,
                     const arguments_type& argv)
      : handle_(0)
      , name_(libport::path(binary).basename())
      , pid_(0)
      , binary_(binary)
      , argv_(argv)
      , status_(-1)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Process::Process(rProcess model)
      : handle_(0)
      , name_(model->name_)
      , pid_(0)
      , binary_(model->binary_)
      , argv_(model->argv_)
      , status_(-1)
    {
      proto_add(model);
    }

    URBI_CXX_OBJECT_REGISTER_INIT(Process)
      : handle_(0)
      , name_(libport::path("true").basename())
      , pid_(0)
      , binary_("/bin/true")
      , argv_()
      , status_(-1)
    {
      argv_ << binary_;

#  define DECLARE(Name, Function)             \
      bind(SYMBOL_(Name), &Process::Function)

      DECLARE(asString, as_string);
      DECLARE(done,     done);
      DECLARE(init,     init);
      DECLARE(join,     join);
      DECLARE(run,      run);
      DECLARE(runTo,    runTo);
      DECLARE(kill,     kill);
      DECLARE(status,   status);
      DECLARE(name,     name_);
#  undef DECLARE
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
      if (handle_)
        GD_WARN("Memory leak: Did you forgot to join with a process?");
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
      if (pid_ || handle_)
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
          FRAISE("pipe failed: %s", libport::strerror(err));
        }
        if (pipe(stderr_fd))
        {
          int err = errno;
          XCLOSE(stdout_fd[0]);
          XCLOSE(stdout_fd[1]);
          XCLOSE(stdin_fd[0]);
          XCLOSE(stdin_fd[1]);
          FRAISE("pipe failed: %s", libport::strerror(err));
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

        // Make sure we stay alive until we're done.
        ward_ = this;
        handle_ =
          libport::startThread(boost::bind(Process::monitor_child, this));
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

    void
    Process::join_()
    {
      if (handle_ && done())
      {
        PTHREAD_RUN(pthread_join, handle_, 0);
        handle_ = 0;
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
    Process::join()
    {
      if (!done())
      {
        runner::rRunner self = &::kernel::runner();
        joiners_.push_back(self);
        self->frozen_set(true);
        // This line may throw and thus will not execute the join.
        self->yield();
      }
      join_();
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
  }
}
