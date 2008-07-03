#include <libport/finally.hh>

#include <ast/parametric-ast.hh>
#include <flower/flower.hh>
#include <object/urbi-exception.hh>
#include <parser/parse.hh>
#include <parser/parser-impl.hh>
#include <parser/tweast.hh>

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
    finally << scoped_set(in_loop_, true)
            << scoped_set(has_break_, false)
            << scoped_set(has_continue_, false);
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
    finally << scoped_set(in_loop_, true)
            << scoped_set(has_break_, false)
            << scoped_set(has_continue_, false);
    super_type::visit(code);

    parser::Tweast tweast;

    ast::rForeach copy = result_.unsafe_cast<ast::Foreach>();
    if (has_break_)
      tweast << "var loopBreakTag = new Tag | "
	     << "loopBreakTag:";

    tweast << copy->list_get();

    switch (code->flavor_get())
    {
    case ast::flavor_none:
    case ast::flavor_semicolon:
    case ast::flavor_pipe:
      tweast << ".each(";
      break;
    case ast::flavor_and:
      tweast << ".'each&'(";
      break;
    case ast::flavor_comma:
      pabort("Invalid flavor for 'for ... in': " << code->flavor_get());
    }

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
    finally << scoped_set(in_function_, true)
            << scoped_set(has_return_, false);
    super_type::visit(code);
    if (has_return_)
    {
      static ast::ParametricAst a("var returnTag = new Tag | "
                                  "returnTag: { %exp:1 }");
      ast::rAbstractScope copy =
        result_.unsafe_cast<ast::Function>()->body_get();
      copy->body_set(exp(a % copy->body_get()));
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

} // namespace flower
