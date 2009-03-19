#ifndef PARSER_AST_FACTORY_HH
# define PARSER_AST_FACTORY_HH

# include <list>

# include <ast/exps-type.hh>
# include <ast/flavor.hh>
# include <ast/fwd.hh>
# include <libport/fwd.hh>
# include <parser/fwd.hh>

namespace parser
{
  /// at (%cond ~ %duration) {%body} onleave {%onleave}
  ast::rExp
  ast_at(const yy::location& loc,
         ast::rExp cond,
         ast::rExp body, ast::rExp onleave = 0,
         ast::rExp duration = 0);

  /// at (?(%event)(%payload) {%body} onleave {%onleave}
  ast::rExp
  ast_at_event(const ast::loc& loc,
               EventMatch& event,
               ast::rExp body, ast::rExp onleave = 0);

  /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
  /// \param op can be any of the four cases.
  ast::rExp
  ast_bin(const yy::location& l,
          ast::flavor_type op, ast::rExp lhs, ast::rExp rhs);

  /// "<method>"
  ast::rCall
  ast_call(const yy::location& l,
           libport::Symbol method);


  /// "<target> . <method> (args)".
  ast::rCall
  ast_call(const yy::location& l,
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


  /// "class" lvalue protos block
  ast::rExp
  ast_class(const yy::location& l,
            ast::rCall lvalue, ast::exps_type* protos, ast::rExp block);


  /// closure () { <value> }
  ast::rExp
  ast_closure(ast::rExp value);


  ast::rExp
  ast_event_catcher(const ast::loc& loc,
                    EventMatch& event,
                    ast::rExp body, ast::rExp onleave);

  /// \param iffalse can be 0.
  ast::rExp
  ast_if(const yy::location& l,
         ast::rExp cond, ast::rExp iftrue, ast::rExp iffalse);

  /// Build a for loop.
  // Since we don't have "continue", for is really a sugared
  // while:
  //
  // "for OP ( INIT; TEST; INC ) BODY";::rExp
  ast::rExp
  ast_for(const yy::location& l, ast::flavor_type op,
          ast::rExp init, ast::rExp test, ast::rExp inc,
          ast::rExp body);

  /** Use these functions to avoid CPP-like problem when referring
   *  several times to an lvalue.  For instance, do not desugar
   *
   *  f(x).val += 1
   *
   *  as
   *
   *  f(x).val = f(x).val + 1
   *
   *  but as
   *
   *  var tmp = f(x) | tmp.val = tmp.val + 1
   *
   * 1/ Use ast_lvalue_once to get the actual slot owner to use.
   * 2/ Transform your code
   * 3/ Use ast_lvalue_wrap to potentially define the cached owner.
   *
   */
  // We need two separate functions. This could be improved if
  // ParametricAst where able to pick the same variable several times,
  // which should pose no problem now that ast are refcounted.
  ast::rLValue ast_lvalue_once(const ast::rLValue& lvalue);
  ast::rExp ast_lvalue_wrap(const ast::rLValue& lvalue, const ast::rExp& e);

  /// Return the ast for "nil".
  ast::rExp ast_nil();

  /// Return \a e in a ast::Scope unless it is already one.
  ast::rScope
  ast_scope(const yy::location& l, ast::rExp target, ast::rExp e);

  ast::rScope
  ast_scope(const yy::location& l, ast::rExp e);

  ast::rExp
  ast_string(const yy::location& l, libport::Symbol s);


  /*--------.
  | Switch  |
  `--------*/

  typedef std::pair<ast::rMatch, ast::rNary> case_type;
  typedef std::list<case_type> cases_type;

  ast::rExp
  ast_switch(const yy::location& l, ast::rExp cond,
             const cases_type& cases, ast::rExp def);


  /// timeout(duration) body
  ast::rExp ast_timeout(const ast::rExp& duration, const ast::rExp& body);

  /// waituntil (cond ~ duration);
  /// \param duration can be 0.
  ast::rExp ast_waituntil(const yy::location& loc,
                          const ast::rExp& cond, ast::rExp duration);

  /// waituntil (?(%event)(%payload))
  ast::rExp
  ast_waituntil_event(const ast::loc& loc,
                      ast::rExp event,
                      ast::exps_type* payload);

  /// whenever (%cond ~ %duration) {%body} onleave {%else_stmt}
  ast::rExp
  ast_whenever(const yy::location& loc,
               ast::rExp cond,
               ast::rExp body, ast::rExp else_stmt = 0,
               ast::rExp duration = 0);

  /// whenever (?(%event)(%payload) {%body} onleave {%onleave}
  ast::rExp
  ast_whenever_event(const ast::loc& loc,
                     EventMatch& event,
                     ast::rExp body, ast::rExp onleave = 0);
}

namespace std
{
  // The structure (list) live in std, that's where Koening look-up
  // will look for them.
  ostream& operator<< (ostream& o, const parser::case_type& c);
  ostream& operator<< (ostream& o, const parser::cases_type& c);
}

# include <parser/ast-factory.hxx>

#endif // !PARSER_AST_FACTORY_HH
