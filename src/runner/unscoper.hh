#ifndef RUNNER_UNSCOPER_HH
# define RUNNER_UNSCOPER_HH

# include <scheduler/tag.hh>

namespace runner
{

  class Unscoper : public scheduler::Tag
  {
  public:
    explicit Unscoper(libport::Symbol name);
    virtual void apply_tag(scheduler::tags_type& tags,
			   libport::Finally* finally);
  };

} // namespace runner

#endif // RUNNER_UNSCOPER_HH
