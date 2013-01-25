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
 ** \file ast/local-assignment.hh
 ** \brief Declaration of ast::LocalAssignment.
 */

#ifndef AST_LOCAL_ASSIGNMENT_HH
# define AST_LOCAL_ASSIGNMENT_HH

# include <ast/local-declaration.hh>
# include <ast/local-write.hh>

namespace ast
{

  /// LocalAssignment.
  class LocalAssignment : public LocalWrite
  {
public:
  unsigned local_index_get() const;

    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a LocalAssignment node.
    LocalAssignment (const loc& location, const libport::Symbol& what,
                     const rExp& value, unsigned depth);
    /// Destroy a LocalAssignment node.
    virtual ~LocalAssignment ();
    /** \} */

    /// \name Visitors entry point.
    /// \{ */
  public:
    /// Accept a const visitor \a v.
    virtual void accept (ConstVisitor& v) const;
    /// Accept a non-const visitor \a v.
    virtual void accept (Visitor& v);
    /// Evaluate the node in AST interpreter \a r.
    virtual urbi::object::rObject
    eval (runner::Job& r) const;
    /// Return the node type
    virtual std::string node_type() const;
    /// \}

#if defined ENABLE_SERIALIZATION
    /// \name Serialization.
    /// \{ */
  public:
    template <typename T>
    LocalAssignment(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return the number of contexts to rewind to find the actual variable.
    const unsigned& depth_get () const;
    /// Return the number of contexts to rewind to find the actual variable.
    unsigned& depth_get ();
    /// Set the number of contexts to rewind to find the actual variable.
    void depth_set (unsigned);
    /// Return declaration.
    const rLocalDeclaration& declaration_get () const;
    /// Return declaration.
    rLocalDeclaration& declaration_get ();
    /// Set declaration.
    void declaration_set (const rLocalDeclaration&);
    /** \} */

  protected:
    /// the number of contexts to rewind to find the actual variable.
    unsigned depth_;
    /// declaration.
    rLocalDeclaration declaration_;
  };

} // namespace ast

# include <ast/local-assignment.hxx>

#endif // !AST_LOCAL_ASSIGNMENT_HH
