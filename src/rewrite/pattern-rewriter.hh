#ifndef REWRITE_PATTERN_REWRITER_HH
# define REWRITE_PATTERN_REWRITER_HH

# include <object/object.hh>
# include <ast/cloner.hh>

namespace rewrite
{
  class PatternRewriter: public ast::Cloner
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
    /// Construct a \c PatternRewriter.
    explicit PatternRewriter();

    /// Destroy a \c PatternRewriter.
    virtual ~PatternRewriter();
    /// \}

    /// Import visit from DefaultVisitor.
    using super_type::visit;

  protected:
    CONST_VISITOR_VISIT_NODES((Binding)
                              (Nary));

  private:
    /// Extracted variables declarations.
    typedef std::set<libport::Symbol> declarations_type;
    typedef std::vector<declarations_type> declarations_stack_type;
    declarations_stack_type decs_;
  };
}

#endif
