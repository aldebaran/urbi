/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef REWRITE_RESCOPER_HH
# define REWRITE_RESCOPER_HH

# include <ast/analyzer.hh>
# include <urbi/object/object.hh>

namespace rewrite
{
  /**
   *  The rescoper:
   *
   *  - inserts closures around '&' and ',' operands,
   *  - extracts variable declarations to unscope them.
   *
   *  That is, rewrite:
   *
   *  code1() & var x = code2();
   *
   *  to:
   *
   *  var x; closure () { code1() } & closure () { x = code2() };
   *
   */
  class Rescoper: public ast::Analyzer
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef ast::Cloner super_type;
    /// Import rObject
    typedef urbi::object::rObject rObject;
    /// \}

    /// \name Ctor & dtor.
    /// \{
    /// Construct a \c Rescoper.
    explicit Rescoper();

    /// Destroy a \c Rescoper.
    virtual ~Rescoper();
    /// \}

    /// Import visit from Cloner.
    using super_type::visit;

  protected:
    CONST_VISITOR_VISIT_NODES(
      (And)
      (Nary)
      (While)
      );

  private:
    // Helpers
    ast::rExp
    make_declaration(const ast::loc& l, ast::rConstLValue what);
    ast::rExp
    make_assignment(const ast::loc& l, ast::rConstLValue what,
                    ast::rConstExp value);
    ast::rExp
    unscope(ast::rExp subject, ast::rNary nary);
  };
}

#endif
