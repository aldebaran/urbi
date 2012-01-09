/*
 * Copyright (C) 2007-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file runner/runner.hxx
 ** \brief Inline implementation of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HXX
# define RUNNER_RUNNER_HXX

# include <libport/compilation.hh>

# include <urbi/object/event.hh>
# include <urbi/object/job.hh> // To destroy Runner::job_.
# include <urbi/object/lobby.hh>
# include <urbi/object/tag.hh>

namespace runner
{

  LIBPORT_SPEED_INLINE
  Runner::Runner(rLobby lobby, sched::Scheduler& sched,
		 const std::string& name)
    : sched::Job(sched, name)
    , redefinition_mode_(false)
    , void_error_(true)
    , lobby_(lobby)
    , prio_(sched::UPRIO_DEFAULT)
    , frozen_(false)
    , dependencies_log_(false)
    , dependencies_()
  {
  }

  LIBPORT_SPEED_INLINE
  Runner::Runner(const Runner& model, const std::string& name)
    : sched::Job(model, name)
    , redefinition_mode_(model.redefinition_mode_)
    , void_error_(true)
    , lobby_(model.lobby_)
    , prio_(sched::UPRIO_DEFAULT)
    , frozen_(false)
    , dependencies_log_(false)
    , dependencies_()
  {
  }

  LIBPORT_SPEED_INLINE
  Runner::~Runner()
  {
  }

  LIBPORT_SPEED_INLINE
  object::rLobby
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
    recompute_prio(tag->value_get()->prio_get());
  }

  LIBPORT_SPEED_INLINE bool
  Runner::has_tag(const object::rTag& tag) const
  {
    return has_tag(*tag->value_get());
  }

  LIBPORT_SPEED_INLINE void
  Runner::tag_stack_clear()
  {
    tag_stack_.clear();
  }

  LIBPORT_SPEED_INLINE const Runner::tag_stack_type&
  Runner::tag_stack_get_all() const
  {
    return tag_stack_;
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

  LIBPORT_SPEED_INLINE const Runner::dependencies_type&
  Runner::dependencies() const
  {
    return dependencies_;
  }

  LIBPORT_SPEED_INLINE void
  Runner::dependencies_log_set(bool v)
  {
    dependencies_log_ = v;
  }

  LIBPORT_SPEED_INLINE bool
  Runner::dependencies_log_get() const
  {
    return dependencies_log_;
  }
} // namespace runner

#endif // RUNNER_RUNNER_HXX
