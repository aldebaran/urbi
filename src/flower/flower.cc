/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/finally.hh>

#include <ast/cloner.hxx>
#include <ast/parametric-ast.hh>

#include <flower/flower.hh>

#include <parser/parse.hh>
#include <parser/parser-impl.hh>

#include <rewrite/rewrite.hh>

DECLARE_LOCATION_FILE;

namespace flower
{
  using libport::Finally;
  using libport::scoped_set;

  Flower::Flower()
    : in_catch_(false)
    , in_function_(false)
    , in_loop_(false)
  {}

  void
  Flower::err(const ast::loc& loc, const std::string& msg)
  {
    errors_.err(loc, msg, "syntax error");
  }

  void
  Flower::visit(const ast::Break* b)
  {
    if (!in_loop_)
      err(b->location_get(), "`break' not within a loop");

    has_break_ = true;

    PARAMETRIC_AST(res, "'$loopBreakTag'.stop");
    result_ = exp(res);
    result_->original_set(b);
  }

  void
  Flower::visit(const ast::Continue* c)
  {
    if (!in_loop_)
      err(c->location_get(), "`continue' not within a loop");

    has_continue_ = true;

    PARAMETRIC_AST(res, "'$loopContinueTag'.stop");
    result_ = exp(res);
    result_->original_set(c);
  }

  static inline
  ast::rExp
  brk(ast::rExp e)
  {
    PARAMETRIC_AST(brk,
      "{var '$loopBreakTag' = Tag.newFlowControl(\"loopBreakTag\") |"
      "'$loopBreakTag': %exp:1}");

    return exp(brk % e);
  }

  static inline
  ast::rExp
  cont(ast::rExp e)
  {
    PARAMETRIC_AST(cont,
      "{var '$loopContinueTag' = Tag.newFlowControl(\"loopContinueTag\") |"
      "'$loopContinueTag': %exp:1}");

    return exp(cont % e);
  }

  void
  Flower::visit(const ast::While* code)
  {
    Finally finally;
    finally << scoped_set(in_loop_, true)
            << scoped_set(has_break_, false)
            << scoped_set(has_continue_, false);

    ast::rExp res = code->body_get()->body_get();
    // FIXME: how come res can be null?
    res = res ? recurse(res) : new ast::Noop(code->location_get(), 0);
    if (has_continue_)
      res = cont(res.get());

    PARAMETRIC_AST(whle, "while (%exp:1) %exp:2");
    res = exp(whle % recurse(code->test_get()) % res);
    res.unchecked_cast<ast::While>()->flavor_set(code->flavor_get());

    if (has_break_)
      res = brk(res);

    result_ = res;
    result_->original_set(code);
  }

  void
  Flower::visit(const ast::Foreach* code)
  {
    Finally finally;
    finally << scoped_set(in_loop_, true)
            << scoped_set(has_break_, false)
            << scoped_set(has_continue_, false);

    PARAMETRIC_AST(each, "%exp:1 . %id:2 (%exp:3)");
    ast::rExp target = recurse(code->list_get());

    ast::rExp body = recurse(code->body_get());
    if (has_continue_)
      body = cont(body);

    // 'fillme' is a placeholder filled later. Parametric ASTs can't
    // parametrize formal arguments for now.
    PARAMETRIC_AST(closure, "closure (fillme) {%exp:1}");
    ast::rRoutine c = (closure % body).result<ast::Routine>();
    // Rename the 'fillme' closure formal argument
    c->formals_get()->front()->what_set(code->index_get()->what_get());

    each % target;
    switch (code->flavor_get())
    {
    case ast::flavor_none:
    case ast::flavor_semicolon:
      each % SYMBOL(each);
      break;
    case ast::flavor_pipe:
      each % SYMBOL(each_PIPE);
      break;
    case ast::flavor_and:
      each % SYMBOL(each_AMPERSAND);
      break;
    case ast::flavor_comma:
      pabort("invalid comma flavor");
    }
    each % c;

    result_ = has_break_ ? brk(exp(each)) : exp(each);
    result_->original_set(code);
  }

  void
  Flower::visit(const ast::Routine* code)
  {
    if (code->closure_get())
    {
      super_type::visit(code);
      return;
    }
    Finally finally;
    finally << scoped_set(in_function_, true)
            << scoped_set(has_return_, false);
    super_type::visit(code);
    if (has_return_)
    {
      PARAMETRIC_AST_DESUGAR(a,
                             "var '$returnTag' ="
                             "  Tag.newFlowControl(\"returnTag\") | "
                             "'$returnTag': %exp:1");
      ast::rScope copy = result_.unsafe_cast<ast::Routine>()->body_get();
      copy->body_set(exp(a % copy->body_get()));
    }
    result_->original_set(code);
  }

  void
  Flower::visit(const ast::Return* ret)
  {
    if (!in_function_)
      err(ret->location_get(), "return: outside a function");

    has_return_ = true;

    if (ast::rExp e = ret->value_get())
    {
      PARAMETRIC_AST(a, "'$returnTag'.stop(%exp:1)");
      result_ = exp(a % e);
    }
    else
    {
      PARAMETRIC_AST(a, "'$returnTag'.stop");
      result_ = exp(a);
    }

    result_->original_set(ret);
  }

  void
  Flower::visit(const ast::Try* code)
  {
    Finally finally(scoped_set(has_general_catch_, false));
    super_type::visit(code);
  }

  void
  Flower::visit(const ast::Catch* code)
  {
    if (has_general_catch_)
      err(code->location_get(),
		    "catch: exception already caught by a previous clause");
    has_general_catch_ = !code->match_get();

    Finally finally(scoped_set(in_catch_, true));
    super_type::visit(code);
  }

  void
  Flower::visit(const ast::Throw* code)
  {
    if (!code->value_get() && !in_catch_)
      err(code->location_get(),
		    "throw: argumentless throw outside of a catch block");
    super_type::visit(code);
  }

} // namespace flower
