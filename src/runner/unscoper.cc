#include <boost/bind.hpp>

#include <libport/foreach.hh>

#include <runner/unscoper.hh>

namespace runner
{

  scheduler::tags_type
  remove_scope_tags(const scheduler::tags_type& tags)
  {
    scheduler::tags_type res;
    foreach(const scheduler::rTag& tag, tags)
      if (!tag->flow_control_get())
	res.push_back(tag);
    return res;
  }

  Unscoper::Unscoper(libport::Symbol name)
    : scheduler::Tag(name)
  {
  }

  void
  Unscoper::apply_tag(scheduler::tags_type& tags, libport::Finally* finally)
  {
    if (finally)
      *finally << libport::restore(tags);
    tags = remove_scope_tags(tags);
  }

} // namespace runner
