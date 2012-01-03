/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <kernel/server-timer.hh>

#ifndef COMPILATION_MODE_SPEED

# include <libport/debug.hh>
# include <libport/lexical-cast.hh>
# include <libport/tokenizer.hh>

GD_CATEGORY(Urbi);

namespace kernel
{
  libport::timer server_timer;

  static void timer_log()
  {
    // We _need_ an intermediate string variable here.
    // See libport/tokenizer.hh.
    std::string s = string_cast(server_timer);
    foreach (const std::string& line, libport::lines(s))
      GD_INFO_TRACE(line);
  }

  void timer_log_on_destruction()
  {
    server_timer.destruction_hook(timer_log);
  }

}


#endif
