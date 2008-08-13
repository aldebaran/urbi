#include <ast/all.hh>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>
#include <object/symbols.hh>
#include <parser/ast-factory.hh>
#include <parser/parse.hh>
#include <parser/parse-result.hh>
#include <parser/tweast.hh>

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
      res = new ast::Pipe (l, lhs, rhs);
      break;
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


  /// "class" lvalue block
  /// "class" lvalue
  ast::rExp
  ast_class(const yy::location& l,
            ast::rCall lvalue, ast::exps_type* protos, ast::rExp block)
  {
    libport::Symbol name = lvalue->name_get();

    static ParametricAst desugar(
      "var %lvalue:1 ="
      "{"
      "  var '$tmp' = Object.clone |"
      "  %exp:2 |"
      "  '$tmp'.setSlot(\"protoName\", %exp:3) |"
      "  '$tmp'.setSlot(%exp:4, function () { this }) |"
      "  do '$tmp'"
      "  {"
      "    %exp:5 |"
      "  } |"
      "  '$tmp'"
      "}"
      );

    ast::rExp protos_set;
    if (protos)
    {
      static ParametricAst setProtos("'$tmp'.setProtos(%exp:1)");
      protos_set = exp(setProtos % new ast::List(l, protos));
    }
    else
      protos_set = new ast::Noop(l, 0);

    desugar % ast::rLValue(lvalue)
      % protos_set
      % ast_string(l, name)
      % ast_string(l, libport::Symbol("as" + name.name_get()))
      % ast_exp(block);

    ast::rExp res = exp(desugar);
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
    libport::Symbol switched = libport::Symbol::fresh("switched");

    static ast::ParametricAst nil("nil");
    ast::rExp inner = exp(nil);
    rforeach (const case_type& c, cases)
    {
      static ast::ParametricAst a(
        "if (Pattern.new(%exp:1).match(%exp:2)) %exp:3 else %exp:4");
      a % c.first
        % ast_exp(ast_call(l, switched))
        % ast_exp(c.second)
        % inner;
      inner = ast::exp(a);
    }

    ::parser::Tweast tweast;
    tweast << "var " << switched << " = " << cond << ";"
           << inner;
    ast::rExp res = ::parser::parse(tweast)->ast_get();
    return res;
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
