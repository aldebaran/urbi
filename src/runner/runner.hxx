/**
 ** \file runner/runner.hxx
 ** \brief Inline implementation of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HXX
# define RUNNER_RUNNER_HXX

# include <libport/compilation.hh>

# include <object/lobby.hh>
# include <object/tag.hh>
# include <object/task.hh>

namespace runner
{

  LIBPORT_SPEED_INLINE
  Runner::Runner(rLobby lobby, sched::Scheduler& sched,
		 const libport::Symbol& name)
    : sched::Job(sched, name)
    , lobby_(lobby)
    , prio_(sched::UPRIO_DEFAULT)
  {
  }

  LIBPORT_SPEED_INLINE
  Runner::Runner(const Runner& model, const libport::Symbol& name)
    : sched::Job(model, name)
    , lobby_(model.lobby_)
    , prio_(sched::UPRIO_DEFAULT)
  {
  }

  LIBPORT_SPEED_INLINE
  Runner::~Runner()
  {
  }

  LIBPORT_SPEED_INLINE
  const object::rLobby&
  Runner::lobby_get() const
  {
    return lobby_;
  }

  LIBPORT_SPEED_INLINE
  object::rLobby
  Runner::lobby_get()
  {
    return lobby_;
  }

  LIBPORT_SPEED_INLINE
  void
  Runner::lobby_set(rLobby lobby)
  {
    lobby_ = lobby;
  }

  LIBPORT_SPEED_INLINE sched::prio_type
  Runner::prio_get() const
  {
    return prio_;
  }

  LIBPORT_SPEED_INLINE void
  Runner::apply_tag(const object::rTag& tag, libport::Finally* finally)
  {
    tag_stack_.push_back(tag);
    if (finally)
      *finally << boost::bind(&tag_stack_type::pop_back, boost::ref(tag_stack_))
	       << boost::bind(&Runner::recompute_prio, this,
			      tag->value_get()->prio_get());
    recompute_prio(tag);
  }

  LIBPORT_SPEED_INLINE bool
  Runner::has_tag(const object::rTag& tag) const
  {
    return has_tag(*(tag->value_get()));
  }

  LIBPORT_SPEED_INLINE void
  Runner::tag_stack_clear()
  {
    tag_stack_.clear();
  }

  LIBPORT_SPEED_INLINE void
  Runner::tag_stack_set(const tag_stack_type& tag_stack)
  {
    tag_stack_ = tag_stack;
  }

  LIBPORT_SPEED_INLINE size_t
  Runner::tag_stack_size() const
  {
    return tag_stack_.size();
  }

} // namespace runner

#endif // RUNNER_RUNNER_HXX
