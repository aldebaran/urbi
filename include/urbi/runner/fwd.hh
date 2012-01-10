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
 ** \file urbi/runner/fwd.hh
 ** \brief Forward declarations for the namespace runner.
 */

#ifndef URBI_RUNNER_FWD_HH
# define URBI_RUNNER_FWD_HH

# include <vector>
# include <libport/intrusive-ptr.hh>
# include <urbi/object/fwd.hh>

namespace runner
{

  class State;
  class Job;

  /// Smart pointer shorthand.
  typedef libport::intrusive_ptr<Job> rJob;

  /// Stack of Urbi tags.
  typedef std::vector<object::rTag> tag_stack_type;

} // namespace runner

#endif // !URBI_RUNNER_FWD_HH
