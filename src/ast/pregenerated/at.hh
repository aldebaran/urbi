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
 ** \file ast/at.hh
 ** \brief Declaration of ast::At.
 */

#ifndef AST_AT_HH
# define AST_AT_HH

# include <ast/exp.hh>
# include <ast/loc.hh>
# include <ast/flavored.hh>

namespace ast
{

  /// At.
  class At : public Flavored
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct an At node.
    At (const loc& location, const flavor_type& flavor,
        const loc& flavor_location, bool sync, const rExp& cond,
        const rExp& body, rExp onleave, rExp duration);
    /// Destroy an At node.
    virtual ~At ();
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
    At(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return flavor_location.
    const loc& flavor_location_get () const;
    /// Set flavor_location.
    void flavor_location_set (const loc&);
    /// Return sync.
    const bool& sync_get () const;
    /// Set sync.
    void sync_set (bool);
    /// Return cond.
    const rExp& cond_get () const;
    /// Return cond.
    rExp& cond_get ();
    /// Set cond.
    void cond_set (const rExp&);
    /// Return body.
    const rExp& body_get () const;
    /// Return body.
    rExp& body_get ();
    /// Set body.
    void body_set (const rExp&);
    /// Return onleave.
    const rExp& onleave_get () const;
    /// Return onleave.
    rExp& onleave_get ();
    /// Set onleave.
    void onleave_set (rExp);
    /// Return duration.
    const rExp& duration_get () const;
    /// Return duration.
    rExp& duration_get ();
    /// Set duration.
    void duration_set (rExp);
    /** \} */

  protected:
    /// flavor_location.
    loc flavor_location_;
    /// sync.
    bool sync_;
    /// cond.
    rExp cond_;
    /// body.
    rExp body_;
    /// onleave.
    rExp onleave_;
    /// duration.
    rExp duration_;
  };

} // namespace ast

# include <ast/at.hxx>

#endif // !AST_AT_HH
