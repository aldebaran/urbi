#ifndef COMPILATION_MODE_SPEED

# include <libport/debug.hh>
# include <libport/tokenizer.hh>
# include <kernel/server-timer.hh>

namespace kernel
{
  libport::timer server_timer;

  void timer_log()
  {
    std::stringstream o;
    server_timer.dump(o);
    GD_CATEGORY(URBI);
    foreach (const std::string& line, libport::lines(o.str()))
      GD_INFO_TRACE(line);
  }

  void timer_log_on_destruction()
  {
    server_timer.destruction_hook(timer_log);
  }

}


#endif
