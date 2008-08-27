#include <boost/bind.hpp>

#include <libport/foreach.hh>

#include <runner/unscoper.hh>

namespace runner
{

  Unscoper::Unscoper(libport::Symbol name)
    : scheduler::Tag(name)
  {
  }

  void
  Unscoper::apply_tag(scheduler::tags_type& tags, libport::Finally* finally)
  {
    scheduler::tags_type unscoped;
    foreach(scheduler::rTag& tag, tags)
      if (!tag->flow_control_get())
	unscoped.push_back(tag);
    if (finally)
      *finally << libport::restore(tags);
    tags = unscoped;
  }

} // namespace runner
