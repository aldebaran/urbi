#include <libport/finally.hh>

#include <ast/flavor.hh>
#include <flower/flower.hh>
#include <kernel/server-timer.hh>
#include <object/urbi-exception.hh>
#include <parser/tweast.hh>
#include <parser/parse.hh>
#include <parser/parser-impl.hh>
#include <parser/parser-utils.hh>

namespace flower
{

  using libport::Finally;
  using libport::scoped_set;

  static void
  error(const ast::rConstAst& node,
	const std::string keyword,
	const std::string msg)
  {
    throw object::ParserError(node->location_get(),
			      keyword + ": " + msg);
  }

  Flower::Flower()
  {
    in_function_ = in_loop_ = false;
  }

  void
  Flower::visit(ast::rConstBreak b)
  {
    if (!in_loop_)
      error(b, "break", "outside a loop");

    result_ = parser::parse("loopBreakTag.stop")->ast_get();
    has_break_ = true;
  }

  void
  Flower::visit(ast::rConstContinue c)
  {
    if (!in_loop_)
      error(c, "continue", "outside a loop");

    result_ = parser::parse("loopContinueTag.stop")->ast_get();
    has_continue_ = true;
  }

  void
  Flower::visit(ast::rConstWhile code)
  {
    Finally finally;
    finally << scoped_set(in_loop_, true);
    finally << scoped_set(has_break_, false);
    finally << scoped_set(has_continue_, false);
    super_type::visit(code);

    // Do nothing if neither break nor continue.
    if (!has_break_ && !has_continue_)
      return;

    ast::rWhile copy = result_.unsafe_cast<ast::While>();
    parser::Tweast tweast;

    if (has_break_)
      tweast << "var loopBreakTag = new Tag | "
	     << "loopBreakTag:";

    tweast << "while (" << copy->test_get() << ") {";

    if (has_continue_)
      tweast << "var loopContinueTag = new Tag | "
	     << "loopContinueTag: {"
	     << copy->body_get()
	     << "}";
    else
      tweast << copy->body_get();

    tweast << "}";

    result_ = parser::parse(tweast)->ast_get();
  }

  void
  Flower::visit(ast::rConstForeach code)
  {
    Finally finally;
    finally << scoped_set(in_loop_, true);
    finally << scoped_set(has_break_, false);
    finally << scoped_set(has_continue_, false);
    super_type::visit(code);

    parser::Tweast tweast;

    ast::rForeach copy = result_.unsafe_cast<ast::Foreach>();
    if (has_break_)
      tweast << "var loopBreakTag = new Tag | "
	     << "loopBreakTag:";

    tweast << copy->list_get() << ".each(";

    tweast << "closure(" << copy->index_get()->what_get() << ") {";

    if (has_continue_)
      tweast << "var loopContinueTag = new Tag | "
	     << "loopContinueTag: {";

    tweast << copy->body_get();

    if (has_continue_)
      tweast << "}";

    tweast << "})";

    result_ = parser::parse(tweast)->ast_get();
  }

  void
  Flower::visit(ast::rConstFunction code)
  {
    Finally finally;
    finally << scoped_set(in_function_, true);
    finally << scoped_set(has_return_, false);
    super_type::visit(code);
    if (has_return_)
    {
      ast::rAbstractScope copy = result_.unsafe_cast<ast::Function>()->body_get();
      parser::Tweast tweast;
      tweast << "var returnTag = new Tag | "
	     << "returnTag: {"
	     << copy->body_get()
	     << "}";
      copy->body_set(parser::parse(tweast)->ast_get());
    }
  }

  void
  Flower::visit(ast::rConstReturn ret)
  {
    if (!in_function_)
      error(ret, "return", "outside a function");

    parser::Tweast tweast;
    ast::rExp exp = ret->value_get();
    tweast << "returnTag.stop";
    if (exp)
      tweast << "(" << exp << ")";
    result_ = parser::parse(tweast)->ast_get();
    has_return_ = true;
  }

  ast::rAst
  flow(ast::rConstAst a)
  {
    ast::rAst res;
    TIMER_PUSH("flow");
    Flower flow;
    try
    {
      flow(a);
      res = flow.result_get();
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
