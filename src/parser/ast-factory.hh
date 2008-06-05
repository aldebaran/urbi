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


  /// Build a for loop.
  // Since we don't have "continue", for is really a sugared
  // while:
  //
  // "for OP ( INIT; TEST; INC ) BODY";::rExp
  ast::rExp
  ast_for (const yy::location& l, ast::flavor_type op,
           ast::rExp init, ast::rExp test, ast::rExp inc,
           ast::rExp body);

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
               ast::rExp value);

  ast::rExp
  ast_slot_update(const yy::location& l, ast::rCall lvalue,
                  ast::rExp value  );

  ast::rExp
  ast_slot_remove(const yy::location& l, ast::rCall lvalue);

  ast::rExp
  ast_string(const yy::location& l, libport::Symbol s);

}
#endif // !PARSER_AST_FACTORY_HH
