/// \file urbi/uvar.hh

// This file is part of UObject Component Architecture
// Copyright (c) 2007, 2008, 2009 Gostai S.A.S.
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//
// For more information, comments, bug reports: http://www.urbiforge.com

#ifndef URBI_UVAR_HH
# define URBI_UVAR_HH

# include <iosfwd>
# include <string>

# include <libport/fwd.hh>
# include <libport/ufloat.hh>

# include <urbi/ucontext.hh>
# include <urbi/uvalue.hh>
# include <urbi/uprop.hh>
# include <urbi/uproperty.hh>

namespace urbi
{
  /** UVar class definition

     Each UVar instance corresponds to one URBI variable. The class
     provides access to the variable properties, and reading/writting
     the value to/from all known types.  */
  class URBI_SDK_API UVar: public UContext
  {
  public:
    /// Creates an unbound UVar. Call init() to bind it.
    UVar();
    UVar(const std::string&, impl::UContextImpl* = 0);
    UVar(const std::string&, const std::string&, impl::UContextImpl* = 0);
    UVar(UObject&, const std::string&, impl::UContextImpl* = 0);
    ~UVar();

    // Bind to \a object.slot.
    void init(const std::string& object, const std::string& slot,
              impl::UContextImpl* = 0);
    void setOwned();

    UDataType type() const;

    /// Request the current value, wait until it is available.
    void syncValue();

    /// Keep this UVar synchronized with kernel value.
    void keepSynchronized();

    void reset (ufloat);

    UVar& operator=(ufloat);
    UVar& operator=(const std::string&);
    /// Deep copy.
    UVar& operator=(const UBinary &);
    /// Deep copy.
    UVar& operator=(const UImage &i);
    /// Deep copy.
    UVar& operator=(const USound &s);

    UVar& operator=(const UList &l);

    UVar& operator=(const UValue &v);

    operator int() const;
    operator bool() const;

    // Cast to a UBinary to make copy through this operator.
    operator const UBinary&() const;

    /// Deep copy, binary will have to be deleted by the user.
    operator UBinary*() const;

    /// In plugin mode, gives direct access to the buffer, which may
    /// not be valid after the calling function returns. Changes to
    /// the other fields of the structure have no effect.
    operator UImage() const;

    /// In plugin mode, gives direct access to the buffer, which may
    /// not be valid after the calling function returns. Changes to
    /// the other fields of the structure have no effect.
    operator USound() const;

    operator ufloat() const;
    operator std::string() const;
    operator UList() const;

    /// No effect in plugin mode. In remote mode, updates the value
    /// once asynchronously.
    void requestValue();

    /// Kernel operators.
    ufloat& in();
    ufloat& out();

    /// Is the variable owned by the module?
    bool owned;

    /// Property accessors.
    UProp rangemin;
    UProp rangemax;
    UProp speedmin;
    UProp speedmax;
    UProp delta;
    UProp blend;

    UValue getProp(UProperty prop);
    void setProp(UProperty prop, const UValue &v);
    void setProp(UProperty prop, ufloat v);
    void setProp(UProperty prop, const char * v);
    void setProp(UProperty prop, const std::string& v)
    {
      setProp(prop, v.c_str());
    }

    /// Enable bypass-mode for this UVar. Plugin-mode only.
    /// In bypass mode, if the UVar contains binary data, the data is never
    /// copied. The consequence is that the data is only accessible from
    /// notifyChange callbacks (urbiScript or C++): it is invalidated as soon
    /// as all callbacks have returned.
    bool setBypass(bool enable=true);

    impl::UVarImpl* impl_;
    const UValue& val() const;
  private:
    /// Declared but not implemented: do not ever try to use it.
    UVar(const UVar&);

    /// Declared but not implemented: do not ever try to use it.
    UVar& operator=(const UVar&);

    /// Pointer to internal data specifics.
    UVardata* vardata;
    void __init();

/// Define an attribute and its accessors.
# define PRIVATE(Type, Name)			\
  public:					\
    Type get_ ##  Name ()			\
    {						\
      return Name;				\
    }						\
    const Type get_ ##  Name () const		\
    {						\
      return Name;				\
    }						\
    void set_ ##  Name (Type& v)		\
    {						\
      Name = v;					\
    }						\
  private:					\
    Type Name;


    /// Full name of the variable as seen in URBI.
    PRIVATE(std::string, name)

# undef PRIVATE

    // Check that the invariants of this class are verified.
    bool invariant() const;
    friend class impl::UVarImpl;
  };

  /*-------------------------.
  | Inline implementations.  |
  `-------------------------*/

  /// Helper macro to initialize UProps in UVar constructors.
#   define VAR_PROP_INIT			\
  rangemin(*this, PROP_RANGEMIN),		\
  rangemax(*this, PROP_RANGEMAX),		\
  speedmin(*this, PROP_SPEEDMIN),		\
  speedmax(*this, PROP_SPEEDMAX),		\
  delta(*this, PROP_DELTA),			\
  blend(*this, PROP_BLEND)

} // end namespace urbi

/// Report \a u on \a o for debugging.
URBI_SDK_API
std::ostream& operator<< (std::ostream& o, const urbi::UVar& u);

# include <urbi/uvar.hxx>

#endif // ! URBI_UVAR_HH
