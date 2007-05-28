//<<-
// Generated, do not edit by hand.
//->>

# include "print-visitor.hh"
# include "all.hh"

namespace ast
{

  void PrintVisitor::operator() (const AliasExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const AndExp& node)
  {
    node.lhs_get().accept(*this);
    *stream_ << " & ";
    node.rhs_get().accept(*this);
  }

  void PrintVisitor::operator() (const ArrayExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const ArrowExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const AssignExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const Ast& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const AtExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const BinaryExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const CallExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const CommaExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const Dec& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const DeleteExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const DisinheritsExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const DotExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const EmitExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const EventDec& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const ExecExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const Exp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const ExpTag& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const ExternalExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const FloatExp& node)
  {
    *stream_ << node.value_get();
  }

  void PrintVisitor::operator() (const FunctionDec& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const IdExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const IfExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const InheritsExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const ListExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const NameTy& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const NegOpExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const NewExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const OpExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const OpVarExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const PipeExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const RefExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const ReturnExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const ScopeExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const SemicolonExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const StringExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const StringTag& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const Tag& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const TagDec& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const TagExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const TagOpExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const Ty& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const Var& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const VarDec& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const WaitExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const WaitUntilExp& )
  {
    /* No printer defined */
  }

  void PrintVisitor::operator() (const WheneverExp& )
  {
    /* No printer defined */
  }

} // namespace ast

