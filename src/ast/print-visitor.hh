//<<-
// Generated, do not edit by hand.
//->>

/**
 ** \file ast/print-visitor.hh
 ** \brief Definition of ast::PrintVisitor.
 */

#ifndef AST_PRINT_VISITOR_HH
# define AST_PRINT_VISITOR_HH

# include <functional>
# include <ostream>
# include "ast/visitor.hh"
# include "ast/fwd.hh"
# include "libport/select-const.hh"

namespace ast
{

  /** \brief Ast pretty-printer.
   **/

  class PrintVisitor : public GenVisitor<libport::constify_traits>
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Super class type.
    typedef GenVisitor<libport::constify_traits> super_type;

    /// Construct a PrintVisitor.
    PrintVisitor (std::ostream& s);

    /// Destroy a PrintVisitor.
    virtual ~PrintVisitor ();
    /** \} */

    using super_type::operator();

    virtual void operator() (const AliasExp&);
    virtual void operator() (const AndExp&);
    virtual void operator() (const ArrayExp&);
    virtual void operator() (const ArrowExp&);
    virtual void operator() (const AssignExp&);
    virtual void operator() (const Ast&);
    virtual void operator() (const AtExp&);
    virtual void operator() (const BinaryExp&);
    virtual void operator() (const CallExp&);
    virtual void operator() (const CommaExp&);
    virtual void operator() (const Dec&);
    virtual void operator() (const DeleteExp&);
    virtual void operator() (const DisinheritsExp&);
    virtual void operator() (const DotExp&);
    virtual void operator() (const EmitExp&);
    virtual void operator() (const EventDec&);
    virtual void operator() (const ExecExp&);
    virtual void operator() (const Exp&);
    virtual void operator() (const ExternalExp&);
    virtual void operator() (const FloatExp&);
    virtual void operator() (const FunctionDec&);
    virtual void operator() (const IdExp&);
    virtual void operator() (const IfExp&);
    virtual void operator() (const InheritsExp&);
    virtual void operator() (const ListExp&);
    virtual void operator() (const NameTy&);
    virtual void operator() (const NegOpExp&);
    virtual void operator() (const NewExp&);
    virtual void operator() (const OpExp&);
    virtual void operator() (const OpVarExp&);
    virtual void operator() (const PipeExp&);
    virtual void operator() (const RefExp&);
    virtual void operator() (const ReturnExp&);
    virtual void operator() (const ScopeExp&);
    virtual void operator() (const SemicolonExp&);
    virtual void operator() (const StringExp&);
    virtual void operator() (const TagDec&);
    virtual void operator() (const TagExp&);
    virtual void operator() (const TagOpExp&);
    virtual void operator() (const Ty&);
    virtual void operator() (const Var&);
    virtual void operator() (const VarDec&);
    virtual void operator() (const WaitExp&);
    virtual void operator() (const WaitUntilExp&);
    virtual void operator() (const WheneverExp&);

  private:
    std::ostream* stream_;
  };

} // namespace ast

// # include "ast/print-visitor.hxx"

#endif // !AST_PRINT_VISITOR_HH
