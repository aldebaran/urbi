#include <ast/all.hh>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>
#include <object/symbols.hh>
#include <parser/ast-factory.hh>
#include <parser/parse.hh>
#include <parser/parse-result.hh>
#include <parser/tweast.hh>

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


  ast::rCall
  ast_lvalue_once(ast::rCall lvalue, Tweast& tweast)
  {
    if (!lvalue->target_implicit())
    {
      libport::Symbol tmp = libport::Symbol::fresh(SYMBOL(__tmp__));
      const yy::location& l = lvalue->location_get();
      tweast << "var " << tmp << " = " << lvalue->target_get() << "|";
      lvalue = ast_call(l, ast_call(l, tmp), lvalue->name_get());
    }
    return lvalue;
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
                  ast::rExp value = 0,
                  ast::rExp modifier = 0)
  {
    ast::rExp res = 0;
    bool implicit = lvalue->target_implicit();

    /// FIXME: This is far from being enough, but it is hard to finish
    /// the implementation since the conversion to local vars
    /// vs. field is not finished yet.  In particular things might go
    /// wrong for the target (%exp:4) depending whether the assignment
    /// is to a local or to a field.
    if (modifier)
    {
      static ast::ParametricAst
        traj("TrajectoryGenerator"
             ".new(%exp:1, %exp:2, %exp:3)"
             ".run(%exp:4, %exp:5)");
      ast::rString name = new ast::String(l, lvalue->name_get());
      res = exp(traj
                %lvalue %value %modifier
                %new_clone(lvalue->target_get()) %name);
    }
    else
    {
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
    }
    return res;
  }

  ast::rExp
  ast_slot_set(const yy::location& l, ast::rCall lvalue,
               ast::rExp value,
               ast::rExp modifier)
  {
    return ast_slot_change(l, lvalue, SYMBOL(setSlot), value, modifier);
  }

  ast::rExp
  ast_slot_update(const yy::location& l, ast::rCall lvalue,
                  ast::rExp value,
                  ast::rExp modifier)
  {
    return ast_slot_change(l, lvalue, SYMBOL(updateSlot), value, modifier);
  }

  ast::rExp
  ast_slot_remove(const yy::location& l, ast::rCall lvalue)
  {
    return ast_slot_change(l, lvalue, SYMBOL(removeSlot));
  }

  ast::rExp
  ast_string(const yy::location& l, libport::Symbol s)
  {
    return new ast::String(l, s);
  }


}
