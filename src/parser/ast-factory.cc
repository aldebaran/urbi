#include <ast/all.hh>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>
#include <object/symbols.hh>
#include <parser/ast-factory.hh>
#include <parser/parse.hh>
#include <parser/parse-result.hh>

namespace std
{
  ostream&
  operator<< (ostream& o, const parser::case_type& c)
  {
    return o << "/* " << (void*) &c << " */ "
             << "case "
             << libport::deref << c.first
             << " => "
             << libport::deref << c.second;
  }

  ostream&
  operator<< (ostream& o, const parser::cases_type& cs)
  {
    o << "/* " << (void*) &cs << " */ "
      << "{" << endl;
    foreach (const parser::case_type& c, cs)
      o << "  " << c << endl;
    return o << "}";
  }

}

namespace parser
{
  using ast::ParametricAst;

  ast::rExp
  desugar(::parser::Tweast& t)
  {
    ast::rExp res = ::parser::parse(t)->ast_get().unsafe_cast<ast::Exp>();
    if (!!getenv("DESUGAR"))
      LIBPORT_ECHO("res: " << get_pointer(res)
                   << ": " << libport::deref << res);
    return res;
  }

  ast::rExp
  ast_at(const yy::location& loc,
         ast::rExp cond,
         ast::rExp at, ast::rExp onleave,
         ast::rExp duration)
  {
    if (!onleave)
      onleave = new ast::Noop(loc, 0);

    if (duration)
    {
      static ast::ParametricAst desugar(
        "var '$at' = persist(%exp:1, %exp:2) |"
        "at ('$at') %exp:3 onleave %exp:4");

      return exp(desugar % cond % duration % at % onleave);
    }
    else
    {
      static ast::ParametricAst desugar(
        "Control.at_(%exp:1, detach(%exp:2), detach(%exp:3))");

      return exp(desugar % cond % at % onleave);
    }
  }

  ast::rExp
  ast_at_event(const ast::loc& loc,
               ast::rExp event, ast::rExp payload,
               ast::rExp at, ast::rExp onleave)
  {
    if (!onleave)
      onleave = new ast::Noop(loc, 0);

    static ast::ParametricAst desugar(
      "detach("
      "{"
      "  %exp:1.onEvent(closure ('$at')"
      "  {"
      "    if (Pattern.new(%exp:2).match('$at'.payload))"
      "    {"
      "      %exp:3 |"
      "      waituntil(!'$at'.active) |"
      "      %exp:4"
      "    }"
      "  })"
      "})");

    return exp(desugar % event % payload % at % onleave);
  }

  ast::rExp
  ast_whenever_event(const ast::loc& loc,
                     ast::rExp event, ast::rExp payload,
                     ast::rExp body, ast::rExp onleave)
  {
    if (!onleave)
      onleave = new ast::Noop(loc, 0);

    static ast::ParametricAst desugar(
      "detach("
      "{"
      "%exp:1.onEvent("
      "  closure ('$whenever')"
      "  {"
      "    if (Pattern.new(%exp:2).match('$whenever'.payload))"
      "      detach("
      "        {"
      "          while (true)"
      "          {"
      "            %exp:3 |"
      "            if(!'$whenever'.active)"
      "              break"
      "          } |"
      "          %exp:4"
      "        })"
      "  })"
      "})");

    return exp(desugar % event
               % payload
               % body % onleave);
  }

  /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
  /// \param op must be & or |.
  ast::rExp
  ast_bin(const yy::location& l,
          ast::flavor_type op, ast::rExp lhs, ast::rExp rhs)
  {
    ast::rExp res = 0;
    assert (lhs);
    assert (rhs);
    switch (op)
    {
    case ast::flavor_pipe:
    {
      ast::rPipe pipe;
      if (pipe = lhs.unsafe_cast<ast::Pipe>())
        pipe->children_get().push_back(rhs);
      else
      {
        pipe = new ast::Pipe(l, ast::exps_type());
        pipe->children_get().push_back(lhs);
        pipe->children_get().push_back(rhs);
      }
      res = pipe;
      break;
    }
    case ast::flavor_and:
    {
      ast::rAnd rand;
      if (rand = lhs.unsafe_cast<ast::And>())
        rand->children_get().push_back(rhs);
      else
      {
        rand = new ast::And(l, ast::exps_type());
        rand->children_get().push_back(lhs);
        rand->children_get().push_back(rhs);
      }
      res = rand;
      break;
    }
    case ast::flavor_comma:
    case ast::flavor_semicolon:
    {
      ast::rNary nary = new ast::Nary(l);
      nary->push_back(lhs, op);
      nary->push_back(rhs);
      res = nary;
      break;
    }
    case ast::flavor_none:
      pabort(op);
    }
    return res;
  }


  /// "<method> (args)".
  ast::rCall
  ast_call (const yy::location& l,
            libport::Symbol method)
  {
    return ast_call(l, new ast::Implicit(l), method);
  }


  /// "<target> . <method> (args)".
  ast::rCall
  ast_call (const yy::location& l,
            ast::rExp target, libport::Symbol method, ast::exps_type* args)
  {
    ast::rCall res = new ast::Call(l, args, target, method);
    return res;
  }

  /// "<target> . <method> ()".
  ast::rCall
  ast_call(const yy::location& l, ast::rExp target, libport::Symbol method)
  {
    return ast_call(l, target, method, 0);
  }

  /// "<target> . <method> (<arg1>)".
  ast::rCall
  ast_call(const yy::location& l,
           ast::rExp target, libport::Symbol method, ast::rExp arg1)
  {
    ast::rCall res = ast_call(l, target, method, new ast::exps_type);
    res->arguments_get()->push_back(arg1);
    return res;
  }


  /// "<target> . <method> (<arg1>, <arg2>)".
  /// "<target> . <method> (<arg1>, <arg2>, <arg3>)".
  ast::rCall
  ast_call(const yy::location& l,
           ast::rExp target, libport::Symbol method,
           ast::rExp arg1, ast::rExp arg2, ast::rExp arg3)
  {
    ast::rCall res = ast_call(l, target, method, new ast::exps_type);
    res->arguments_get()->push_back(arg1);
    res->arguments_get()->push_back(arg2);
    if (arg3)
      res->arguments_get()->push_back(arg3);
    return res;
  }


  ast::rExp
  ast_closure(ast::rExp value)
  {
    static ast::ParametricAst a("closure () { %exp:1 }");
    return exp(a % value);
  }


  /// Build a for loop.
  // Since we don't have "continue", for is really a sugared
  // while:
  //
  // "for OP ( INIT; TEST; INC ) BODY"
  //
  // ->
  //
  // "{ INIT OP WHILE OP (TEST) { BODY | INC } }"
  //
  // OP is either ";" or "|".
  ast::rExp
  ast_for (const yy::location& l, ast::flavor_type op,
           ast::rExp init, ast::rExp test, ast::rExp inc,
           ast::rExp body)
  {
    passert(op, op == ast::flavor_pipe || op == ast::flavor_semicolon);
    assert(init);
    assert(test);
    assert(inc);
    assert(body);

    // BODY | INC.
    ast::rExp loop_body = ast_bin(l, ast::flavor_pipe, body, inc);

    // WHILE OP (TEST) { BODY | INC }.
    ast::While *while_loop =
      new ast::While(l, op, test, ast_scope(l, loop_body));

    // { INIT OP WHILE OP (TEST) { BODY | INC } }.
    return ast_scope(l, ast_bin(l, op, init, while_loop));
  }

  /// Return \a e in a ast::Scope unless it is already one.
  ast::rScope
  ast_scope(const yy::location& l, ast::rExp target, ast::rExp e)
  {
    if (ast::rScope res = e.unsafe_cast<ast::Scope>())
      return res;
    else if (target)
      return new ast::Do(l, e, target);
    else
      return new ast::Scope(l, e);
  }

  ast::rScope
  ast_scope(const yy::location& l, ast::rExp e)
  {
    return ast_scope(l, 0, e);
  }

  ast::rExp
  ast_string(const yy::location& l, libport::Symbol s)
  {
    return new ast::String(l, s);
  }

  ast::rExp
  ast_switch(const yy::location& l, ast::rExp cond, const cases_type& cases)
  {
    (void) l;

    static ast::ParametricAst nil("nil");
    ast::rExp inner = exp(nil);
    rforeach (const case_type& c, cases)
    {
      static ast::ParametricAst a(
        "if (Pattern.new(%exp:1).match('$switch')) %exp:2 else %exp:3");
      a % c.first
        % ast_exp(c.second)
        % inner;
      inner = ast::exp(a);
    }

    static ParametricAst sw("{ var '$switch' = %exp:1 | %exp:2 }");
    return exp(sw % cond % inner);
  }

  ast::rLValue ast_lvalue_once(const ast::rLValue& lvalue)
  {
    ast::rCall tmp = ast_call(lvalue->location_get(), SYMBOL($tmp));

    if (lvalue->call()->target_implicit())
      return lvalue.get();
    else
      return ast_call(lvalue->location_get(), tmp, lvalue->call()->name_get());
  }

  ast::rExp ast_lvalue_wrap(const ast::rLValue& lvalue, const ast::rExp& e)
  {
    static ParametricAst wrap(
      "{"
      "var '$tmp' = %exp:1;"
      "%exp:2;"
      "}"
      );

    if (lvalue->call()->target_implicit())
      return e;
    else
    {
      wrap % lvalue->call()->target_get() % e;
      return exp(wrap);
    }
  }

}
