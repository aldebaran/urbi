/**
 ** \file runner/job.hh
 ** \brief Definition of runner::Job.
 */

#ifndef RUNNER_JOB_HH
# define RUNNER_JOB_HH

# include <boost/utility.hpp>

namespace runner
{

  class Job : boost::noncopyable
  {
  public:
    Job ();
    virtual ~Job ();

  public:
    virtual void start ();
    virtual void run () = 0;
    virtual void stop ();

  private:
    // FIXME
  };

} // namespace runner

# include "runner/job.hxx"

#endif // !RUNNER_JOB_HH
