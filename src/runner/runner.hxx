/**
 ** \file runner/runner.hxx
 ** \brief Inline implementation of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HXX
# define RUNNER_RUNNER_HXX

# include <libport/compilation.hh>

# include <object/lobby.hh>
# include <object/task.hh>

namespace runner
{

  LIBPORT_SPEED_INLINE
  Runner::Runner(rLobby lobby, scheduler::Scheduler& sched,
		 const libport::Symbol& name)
    : scheduler::Job(sched, name)
    , lobby_(lobby)
    , prio_(scheduler::UPRIO_DEFAULT)
  {
  }

  LIBPORT_SPEED_INLINE
  Runner::Runner(const Runner& model, const libport::Symbol& name)
    : scheduler::Job(model, name)
    , lobby_(model.lobby_)
    , prio_(scheduler::UPRIO_DEFAULT)
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

  LIBPORT_SPEED_INLINE const tags_type&
  Runner::tags_get() const
  {
    return tags_;
  }

  LIBPORT_SPEED_INLINE void
  Runner::tags_set(const tags_type& tags)
  {
    tags_ = tags;
    recompute_prio();
  }

  LIBPORT_SPEED_INLINE void
  Runner::tags_clear()
  {
    tags_.clear();
    recompute_prio();
  }

  LIBPORT_SPEED_INLINE scheduler::prio_type
  Runner::prio_get() const
  {
    return prio_;
  }

  LIBPORT_SPEED_INLINE void
  Runner::apply_tag(const scheduler::rTag& tag, libport::Finally* finally)
  {
    tags_.push_back(tag);
    if (finally)
      *finally << boost::bind(&tags_type::pop_back, boost::ref(tags_))
	       << boost::bind(&Runner::recompute_prio, this, boost::ref(*tag));
    recompute_prio(*tag);
  }

  LIBPORT_SPEED_INLINE bool
  Runner::has_tag(const object::rTag& tag) const
  {
    return has_tag(*(tag->value_get()));
  }

} // namespace runner

#endif // RUNNER_RUNNER_HXX
