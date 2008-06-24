#ifndef PARSER_AST_FACTORY_HH
# define PARSER_AST_FACTORY_HH

# include <ast/fwd.hh>
# include <libport/fwd.hh>
# include <parser/fwd.hh>

namespace parser
{
  /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
  /// \param op must be & or |.
  ast::rExp
  ast_bin(const yy::location& l,
          ast::flavor_type op, ast::rExp lhs, ast::rExp rhs);

  /// "<method>"
  ast::rCall
  ast_call (const yy::location& l,
            libport::Symbol method);


  /// "<target> . <method> (args)".
  ast::rCall
  ast_call (const yy::location& l,
            ast::rExp target, libport::Symbol method, ast::exps_type* args);

  /// "<target> . <method> ()".
  ast::rCall
  ast_call(const yy::location& l,
           ast::rExp target, libport::Symbol method);

  /// "<target> . <method> (<arg1>)".
  ast::rCall
  ast_call(const yy::location& l,
           ast::rExp target, libport::Symbol method, ast::rExp arg1);


  /// "<target> . <method> (<arg1>, <arg2>)".
  /// "<target> . <method> (<arg1>, <arg2>, <arg3>)".
  ast::rCall
  ast_call(const yy::location& l,
           ast::rExp target, libport::Symbol method,
           ast::rExp arg1, ast::rExp arg2, ast::rExp arg3 = 0);


  /// "class" lvalue block
  /// "class" lvalue
  ast::rExp
  ast_class(const yy::location& l,
            ast::rCall lvalue, ast::rExp block);

  /// To use to solve the ambiguities bw MetaVar::append_ and
  /// Tweast::append_ when we don't use exactly ast::rExp.
  inline
  ast::rExp
  ast_exp (ast::rExp e);

  /// Build a for loop.
  // Since we don't have "continue", for is really a sugared
  // while:
  //
  // "for OP ( INIT; TEST; INC ) BODY";::rExp
  ast::rExp
  ast_for (const yy::location& l, ast::flavor_type op,
           ast::rExp init, ast::rExp test, ast::rExp inc,
           ast::rExp body);

  /// If \c lvalue is composite, then store it in a local variable,
  /// and change \c lvalue to point to it.  Possibly store in \c
  /// tweast the initialization of the new \c lvalue.
  ///
  /// Use this function to avoid CPP-like problem when referring
  /// several times to an lvalue.  For instance, do not desugar
  ///
  /// f(x).val += 1
  ///
  /// as
  ///
  /// f(x).val = f(x).val + 1
  ///
  /// but as
  ///
  /// var tmp = f(x) | tmp.val = tmp.val + 1
  ///
  /// This function puts
  ///
  /// var tmp = f(x) |
  ///
  /// in \c tweast, and changes \c lvalue from
  ///
  /// f(x).val
  ///
  /// to
  ///
  /// tmp.
  ast::rCall ast_lvalue_once(ast::rCall lvalue, Tweast& tweast);


  /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
  /// \param op can be any of the four cases.
  ast::rExp
  ast_nary(const yy::location& l,
           ast::flavor_type op, ast::rExp lhs, ast::rExp rhs);

  /// Return \a e in a ast::Scope unless it is already one.
  ast::rAbstractScope
  ast_scope(const yy::location& l, ast::rExp target, ast::rExp e);

  ast::rAbstractScope
  ast_scope(const yy::location& l, ast::rExp e);

  /*-----------------.
  | Changing slots.  |
  `-----------------*/

  ast::rExp
  ast_slot_set(const yy::location& l, ast::rCall lvalue,
               ast::rExp value,
               ast::rExp modifier = 0);

  ast::rExp
  ast_slot_update(const yy::location& l, ast::rCall lvalue,
                  ast::rExp value,
                  ast::rExp modifier = 0);

  ast::rExp
  ast_slot_remove(const yy::location& l, ast::rCall lvalue);

  ast::rExp
  ast_string(const yy::location& l, libport::Symbol s);

}

# include <parser/ast-factory.hxx>

#endif // !PARSER_AST_FACTORY_HH
