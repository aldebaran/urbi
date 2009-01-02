#ifndef REWRITE_RESCOPER_HH
# define REWRITE_RESCOPER_HH

# include <ast/analyzer.hh>
# include <object/object.hh>

namespace rewrite
{
  /**
   *  The roles of the rescoper are:
   *
   *  - Insert closures around '&' and ',' operands.
   *  - Extract variables declaration to unscope them.
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
    typedef object::rObject rObject;
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
