#include <ast/all.hh>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>
#include <object/symbols.hh>
#include <parser/ast-factory.hh>
#include <parser/parse.hh>
#include <parser/parse-result.hh>
#include <rewrite/rewrite.hh>

namespace std
{
  ostream&
  operator<< (ostream& o, const parser::case_type& c)
  {
    return o << "/* " << (const void*) &c << " */ "
             << "case "
             << libport::deref << c.first
             << " => "
             << libport::deref << c.second;
  }

  ostream&
  operator<< (ostream& o, const parser::cases_type& cs)
  {
    o << "/* " << (const void*) &cs << " */ "
      << "{" << endl;
    foreach (const parser::case_type& c, cs)
      o << "  " << c << endl;
    return o << "}";
  }

}

namespace parser
{
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
      PARAMETRIC_AST(desugar,
                     "var '$at' = persist(%exp:1, %exp:2) |"
                     "at ('$at') %exp:3 onleave %exp:4");

      return exp(desugar % cond % duration % at % onleave);
    }
    else
    {
      PARAMETRIC_AST(desugar,
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

    PARAMETRIC_AST(desugar,
      "detach("
      "{"
      "  %exp:1.onEvent(closure ('$at')"
      "  {"
      "    var '$pattern' = Pattern.new(%exp:2) |"
      "    if ('$pattern'.match('$at'.payload))"
      "    {"
      "      %exp: 3"
      "    }"
      "  })"
      "})");

    PARAMETRIC_AST(desugar_body,
      "      %exp:1 |"
      "      waituntil(!'$at'.active) |"
      "      %exp:2");


    ast::rExp body = exp(desugar_body % at % onleave);
    return exp(desugar % event % payload % rewrite::pattern_bind(payload, body));
  }

  ast::rExp
  ast_waituntil_event(const ast::loc& loc,
                      ast::rExp event,
                      ast::exps_type* payload)
  {
    PARAMETRIC_AST
      (desugar,
       "var '$pattern' = Pattern.new(%exp:1) |"
       "%exp:2.'waituntil'('$pattern') |");

    ast::rList d_payload = new ast::List(loc, payload);
    return rewrite::pattern_bind(
      d_payload,
      exp(desugar % d_payload % event),
      false);
  }

  ast::rExp
  ast_whenever_event(const ast::loc& loc,
                     ast::rExp event, ast::rExp payload,
                     ast::rExp body, ast::rExp onleave)
  {
    if (!onleave)
      onleave = new ast::Noop(loc, 0);

    PARAMETRIC_AST(desugar,
      "detach("
      "{"
      "%exp:1.onEvent("
      "  closure ('$whenever')"
      "  {"
      "    var '$pattern' = Pattern.new(%exp:2);"
      "    if ('$pattern'.match('$whenever'.payload))"
      "      detach("
      "        {"
      "          %exp:3"
      "        })"
      "  })"
      "})");

    PARAMETRIC_AST(desugar_body,
      "while (true)"
      "{"
      "  %exp:1 |"
      "  if(!'$whenever'.active)"
      "    break"
      "} |"
      "%exp:2");

    body = exp(desugar_body % body % onleave);

    return exp(desugar % event
               % payload % rewrite::pattern_bind(payload, body));
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
    return new ast::Call(l, args, target, method);
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
    PARAMETRIC_AST(a, "closure () { %exp:1 }");
    return exp(a % value);
  }


  /// Build a for loop.
  // The increment is included directly in the condition to make sure
  // it is executed on `continue'.
  ast::rExp
  ast_for (const yy::location&, ast::flavor_type,
           ast::rExp init, ast::rExp test, ast::rExp inc,
           ast::rExp body)
  {
    // FIXME: for| is handled as a simple for
    PARAMETRIC_AST(desugar,

      "{"
      "  %exp:1 |"
      "  var '$tmp-for-first' = true |"
      "  while ({ if ('$tmp-for-first') '$tmp-for-first' = false else %exp:2 | %exp:3})"
      "    %exp:4 |"
      "}"
      );

    desugar
      % init
      % inc
      % test
      % body;

    return exp(desugar);
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
  ast_switch(const yy::location& l, ast::rExp cond,
             const cases_type& cases, ast::rExp def)
  {
    (void) l;

    PARAMETRIC_AST(nil, "nil");
    ast::rExp inner = def ? def : exp(nil);
    rforeach (const case_type& c, cases)
    {
      PARAMETRIC_AST(a,
                     "var '$pattern' = Pattern.new(%exp:1) |"
                     "if ('$pattern'.match('$switch')) %exp:2 else %exp:3");
      ast::rExp body = rewrite::pattern_bind(c.first,
                                             c.second);
      a % c.first
        % body
        % inner;
      inner = ast::exp(a);
    }

    PARAMETRIC_AST(sw, "{ var '$switch' = %exp:1 | %exp:2 }");
    return exp(sw % cond % inner);
  }

  ast::rLValue ast_lvalue_once(const ast::rLValue& lvalue)
  {
    ast::rCall tmp = ast_call(lvalue->location_get(), SYMBOL(DOLLAR_tmp));

    if (lvalue->call()->target_implicit())
      return lvalue.get();
    else
      return ast_call(lvalue->location_get(), tmp, lvalue->call()->name_get());
  }

  ast::rExp ast_lvalue_wrap(const ast::rLValue& lvalue, const ast::rExp& e)
  {
    PARAMETRIC_AST(wrap,
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
