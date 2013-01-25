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
 ** \file ast/cloner.hh
 ** \brief Declaration of ast::Cloner.
 */

#ifndef AST_CLONER_HH
# define AST_CLONER_HH

# include <boost/type_traits/remove_const.hpp>
# include <boost/optional.hpp>

# include "ast/default-visitor.hh"

namespace ast
{

  /// \brief Duplicate an Ast.
  class Cloner : public ast::DefaultConstVisitor
  {
  public:
    typedef ast::DefaultConstVisitor super_type;

    /// Create a Cloner.
    Cloner (bool map = false);
    /// Destroy a Cloner.
    virtual ~Cloner ();

    // Return the cloned Ast.
    ast::rAst result_get ();

    /// Visit entry point
    virtual void operator() (const Ast* e);

    /// Force the location of the cloned ast
    void location_set(const ast::loc& loc);

    // Visit methods.
  public:
    // Import overloaded virtual functions.
    using super_type::visit;

    CONST_VISITOR_VISIT_NODES(
                              (And)
                              (Assign)
                              (Assignment)
                              (At)
                              (Binding)
                              (Break)
                              (Call)
                              (CallMsg)
                              (Catch)
                              (Class)
                              (Continue)
                              (Declaration)
                              (Decrementation)
                              (Dictionary)
                              (Do)
                              (Emit)
                              (Event)
                              (Finally)
                              (Float)
                              (Foreach)
                              (If)
                              (Implicit)
                              (Incrementation)
                              (List)
                              (Local)
                              (LocalAssignment)
                              (LocalDeclaration)
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
                              (Unscope)
                              (Watch)
                              (While)
                             )


  protected:
    template <typename T>
    libport::intrusive_ptr<T> recurse (libport::intrusive_ptr<const T> t);

    template <typename T>
    libport::intrusive_ptr<T>
    recurse (libport::intrusive_ptr<T> t);

    std::pair<ast::rExp, ast::rExp>
    recurse(const std::pair<ast::rExp, ast::rExp>& p);

    template <typename T>
    boost::optional<T>
    recurse(const boost::optional<T>&);

    /** \brief Clone a collection object.

	Using overloading for this method is tempting, but it would
	lead to the same prototype than the first \a recurse method.

	A partial specialization for \a std::list<T> would work, but is
	not allowed by C++ standard. As a consequence, we are stuck to
	using different names.
     */
    template <typename CollectionType>
    CollectionType recurse_collection (const CollectionType& c);

    /** \brief Clone a collection object if not 0.

        It also returns a newly allocated pointer onto the copied
	collection.
    */
    template <typename CollectionType>
    CollectionType* maybe_recurse_collection(const CollectionType* c); 

    /// The cloned Ast.
    ast::rAst result_;

    /// The potential forced location
    boost::optional<ast::loc> loc_;

  private:
    bool use_map_;
    boost::unordered_map<long, ast::rAst> map_;
  };

} // namespace ast

#endif // !AST_CLONER_HH
