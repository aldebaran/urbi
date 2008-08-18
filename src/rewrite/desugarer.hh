#ifndef REWRITE_DESUGAR_HH
# define REWRITE_DESUGAR_HH

# include <ast/cloner.hh>

namespace rewrite
{
  /// Desugar complex constructs to the core language
  class Desugarer: public ast::Cloner
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef ast::Cloner super_type;
    /// \}

  protected:
    /// Import visit from DefaultVisitor.
    using super_type::visit;
    /// Nodes to desugar
    CONST_VISITOR_VISIT_NODES(
      (Class)
      (Decrementation)
      (Delete)
      (Emit)
      (Incrementation)
      (OpAssignment)
      (PropertyRead)
      (PropertyWrite)
      );

  };
}

#endif
