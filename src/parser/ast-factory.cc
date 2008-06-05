#include <ast/all.hh>
#include <object/symbols.hh>
#include <parser/ast-factory.hh>

namespace parser
{

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
      case ast::flavor_and:
        res = new ast::And (l, lhs, rhs);
        break;
      case ast::flavor_pipe:
        res = new ast::Pipe (l, lhs, rhs);
        break;
      default:
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
    ast::rCall res = new ast::Call(l, target, method, args);
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
    passert (op, op == ast::flavor_pipe || op == ast::flavor_semicolon);
    assert (init);
    assert (test);
    assert (inc);
    assert (body);

    // BODY | INC.
    ast::rExp loop_body = ast_nary (l, ast::flavor_pipe, body, inc);

    // WHILE OP (TEST) { BODY | INC }.
    ast::While *while_loop =
      new ast::While(l, op, test, ast_scope(l, loop_body));

    // { INIT OP WHILE OP (TEST) { BODY | INC } }.
    return ast_scope(l, ast_nary (l, op, init, while_loop));
  }


  /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
  /// \param op can be any of the four cases.
  ast::rExp
  ast_nary(const yy::location& l,
           ast::flavor_type op, ast::rExp lhs, ast::rExp rhs)
  {
    switch (op)
    {
      case ast::flavor_and:
      case ast::flavor_pipe:
        return ast_bin(l, op, lhs, rhs);

      case ast::flavor_comma:
      case ast::flavor_semicolon:
      {
        ast::rNary res = new ast::Nary(l);
        res->push_back(lhs, op);
        res->push_back(rhs);
        return res;
      }
      default:
        pabort(op);
    }
  }


  /// Return \a e in a ast::Scope unless it is already one.
  ast::rAbstractScope
  ast_scope(const yy::location& l, ast::rExp target, ast::rExp e)
  {
    if (ast::rAbstractScope res = e.unsafe_cast<ast::AbstractScope>())
      return res;
    else if (target)
      return new ast::Do(l, e, target);
    else
      return new ast::Scope(l, e);
  }

  ast::rAbstractScope
  ast_scope(const yy::location& l, ast::rExp e)
  {
    return ast_scope(l, 0, e);
  }

  /*-----------------.
  | Changing slots.  |
  `-----------------*/

  /// Factor slot_set, slot_update, and slot_remove.
  /// \param l        source location.
  /// \param lvalue   object and slot to change.  This object is
  ///                 destroyed by this function: its contents is
  ///                 stolen, and its counter is decreased.  So the
  ///                 comments in the code for details.
  /// \param change   the Urbi method to invoke.
  /// \param value    optional assigned value.
  /// \param modifier optional time modifier object.
  /// \return The AST node calling the slot assignment.
  static
  inline
  ast::rExp
  ast_slot_change(const yy::location& l,
                  ast::rCall lvalue, libport::Symbol change,
                  ast::rExp value)
  {
    ast::rExp res = 0;
    bool implicit = lvalue->target_implicit();

    if (implicit && change == SYMBOL(updateSlot))
      res = new ast::Assignment(l, lvalue->name_get(), value, 0);
    else if (implicit && change == SYMBOL(setSlot))
      res = new ast::Declaration(l, lvalue->name_get(), value);
    else
    {
      ast::rCall call =
        ast_call(l,
                 lvalue->target_get(), change,
                 ast::rString(new ast::String(lvalue->location_get(),
                                              lvalue->name_get())));
      if (value)
        call->arguments_get()->push_back(value);
      res = call;
    }

    // Our parser stack does not obey the C++ semantics and it does
    // not decrement the counters of the shared pointers.  So help it.
    lvalue->counter_dec();
    return res;
  }

  ast::rExp
  ast_slot_set(const yy::location& l, ast::rCall lvalue,
               ast::rExp value)
  {
    return ast_slot_change(l, lvalue, SYMBOL(setSlot), value);
  }

  ast::rExp
  ast_slot_update(const yy::location& l, ast::rCall lvalue,
                  ast::rExp value)
  {
    return ast_slot_change(l, lvalue, SYMBOL(updateSlot), value);
  }

  ast::rExp
  ast_slot_remove(const yy::location& l, ast::rCall lvalue)
  {
    return ast_slot_change(l, lvalue, SYMBOL(removeSlot), 0);
  }



}
