/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */

#include <ast/call.hh>
#include <kernel/uconnection.hh>
#include <runner/runner.hh>
#include <sstream>
#include <object/atom.hh>

namespace runner
{

  void Runner::show_backtrace(const call_stack_type& bt, const std::string& chan)
  {
    foreach (const ast::Call* c,
             boost::make_iterator_range(boost::rbegin(bt),
                                        boost::rend(bt)))
    {
      std::ostringstream o;
      o << "!!!    called from: " << c->location_get () << ": "
	<< c->name_get ();
      send_message_ (chan, o.str ());
    }
  }

  void
  Runner::send_message_ (const std::string& tag, const std::string& msg)
  {
    UConnection& c = lobby_->value_get().connection;
    c.send (msg.c_str(), tag.c_str());
    c.endline();
  }

} // namespace runner
