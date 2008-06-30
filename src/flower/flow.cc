#include <flower/flow.hh>
#include <flower/flower.hh>
#include <kernel/server-timer.hh>
#include <object/urbi-exception.hh>
#include <parser/parser-utils.hh>

namespace flower
{

  ast::rNary
  flow(ast::rConstNary a)
  {
    ast::rNary res;
    TIMER_PUSH("flow");
    Flower flow;
    try
    {
      flow(a);
      res = flow.result_get().unsafe_cast<ast::Nary>();
    }
    catch (const object::ParserError& pe)
    {
      ast::rMessage msg =
	new ast::Message(pe.location_get(),
			 parser::message_format(pe.location_get(),
						pe.msg_get()),
			 "error");
      ast::rNary nary = new ast::Nary(pe.location_get());
      nary->push_back(msg);
      res = nary;
    }
    TIMER_POP("flow");
    return res;
  }


} // namespace flower
