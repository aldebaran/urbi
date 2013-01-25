/*
 * Copyright (C) 2006-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// Generated, do not edit by hand.

/**
 ** \file ast/transformer.hh
 ** \brief Declaration of ast::Transformer.
 */

#ifndef AST_TRANSFORMER_HH
# define AST_TRANSFORMER_HH
# include <set>

# include <libport/cassert>
# include <libport/type-info.hh>

# include <ast/visitor.hh>
# include <ast/default-visitor.hh>

namespace ast
{
  class Transformer: public DefaultVisitor
  {
  public:
    typedef DefaultVisitor super_type;

    template <typename T>
    void
    transform(libport::intrusive_ptr<T>& ptr)
    {
      if (!ptr)
        return;
      operator()(ptr.get());
# ifndef NDEBUG
      aver(result_.unsafe_cast<T>(),
           "invalid transformation, expected a %s, got a %s",
           libport::TypeInfo(typeid(T)),
           libport::TypeInfo(typeid(*result_)));
# endif
      ptr = result_.unchecked_cast<T>();
    }

    template <typename K, typename V>
    void
    transform_collection(boost::unordered_map<K, V>* hash)
    {
      if (!hash)
        return;
      transform_collection(*hash);
    }

    template <typename K, typename V>
    void
    transform_collection(boost::unordered_map<K, V>& hash)
    {
      typedef typename boost::unordered_map<K, V>::value_type value_type;
      foreach (const value_type& v, hash)
      {
        V ast = v.second;
        transform(ast);
        hash[v.first] = ast;
      }
    }

    template <typename T>
    void
    transform_collection(std::vector<T>& v)
    {
      foreach (T& elt, v)
        transform(elt);
    }

    template <typename T>
    void
    transform_collection(std::vector<T>* v)
    {
      if (!v)
        return;
      transform_collection(*v);
    }

    void
    transform(std::pair<ast::rExp, ast::rExp>& p)
    {
      this->transform<ast::Exp>(p.first);
      this->transform<ast::Exp>(p.second);
    }

    using super_type::visit;
    VISITOR_VISIT_NODES(
      (And)
      (Assign)
      (Assignment)
      (Ast)
      (At)
      (Binding)
      (Break)
      (Call)
      (CallMsg)
      (Catch)
      (Class)
      (Composite)
      (Continue)
      (Declaration)
      (Decrementation)
      (Dictionary)
      (Do)
      (Emit)
      (Event)
      (Exp)
      (Finally)
      (Flavored)
      (Float)
      (Foreach)
      (If)
      (Implicit)
      (Incrementation)
      (LValue)
      (LValueArgs)
      (List)
      (Local)
      (LocalAssignment)
      (LocalDeclaration)
      (LocalWrite)
      (Match)
      (MetaArgs)
      (MetaCall)
      (MetaExp)
      (MetaId)
      (MetaLValue)
      (Nary)
      (Noop)
      (OpAssignment)
      (Pipe)
      (Property)
      (PropertyAction)
      (PropertyWrite)
      (Return)
      (Routine)
      (Scope)
      (Stmt)
      (String)
      (Subscript)
      (TaggedStmt)
      (This)
      (Throw)
      (Try)
      (Unary)
      (Unscope)
      (Watch)
      (While)
      (Write)
    );

    rAst
    result_get()
    {
      return result_;
    }

    virtual
    void
    operator() (ast::Ast* node)
    {
      if (has(seen_, node))
      {
        result_ = node;
        return;
      }
      seen_.insert(node);
      super_type::operator()(node);
    }

  protected:
    rAst result_;

  private:
    std::set<void*> seen_;
  };
}

#endif

