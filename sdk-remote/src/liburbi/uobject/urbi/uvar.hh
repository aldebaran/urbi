/// \file urbi/uvar.hh

// This file is part of UObject Component Architecture
// Copyright (c) 2007 Gostai S.A.S.
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

# include "libport/fwd.hh"
# include "libport/ufloat.hh"
# include "urbi/uvalue.hh"

/// Define an attribute and its accessors.
# define PRIVATE(Type, Name)			\
  private:					\
    Type Name;					\
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
  private:

namespace urbi
{

  //! Provides easy access to variable properties
  class UProp
  {
  public:
    void operator =(const UValue& v);
    void operator =(const ufloat v);
    void operator =(const std::string& v);

    operator ufloat();
    operator std::string();
    operator UValue();

    UProp(UVar& owner, UProperty name)
      : owner(owner), name(name)
    {}

  private:
    UVar& owner;
    UProperty name;

    //disable copy ctor and equal operator
    UProp & operator =(const UProp &b);
    UProp(const UProp &b);
  };



  /** UVar class definition

     Each UVar instance corresponds to one URBI variable. The class
     provides access to the variable properties, and reading/writting
     the value to/from all known types.  */
  class UVar
  {
  public:
    UVar();
    UVar(UVar&);
    UVar(const std::string&);
    UVar(const std::string&, const std::string&);
    UVar(UObject&, const std::string&);
    ~UVar();

    void init(const std::string&, const std::string&);
    void setOwned();
    void syncValue ();

    void reset (ufloat);
    void operator = (ufloat);
    void operator = (const std::string&);
    /// Deep copy.
    void operator = (const UBinary &);
    /// Deep copy.
    void operator = (const UImage &i);
    /// Deep copy.
    void operator = (const USound &s);

    void operator = (const UList &l);

    void operator = (const UValue &v);

    operator int ();
    operator bool ()  {return (int)(*this);}

    /// Deep copy.
    operator UBinary ();

    /// Deep copy, binary will have to be deleted by the user.
    operator UBinary *();

    /// In plugin mode, gives direct access to the buffer, which may
    /// not be valid after the calling function returns. Changes to
    /// the other fields of the structure have no effect..
    operator UImage ();

    /// In plugin mode, gives direct access to the buffer, which may
    /// not be valid after the calling function returns. Changes to
    /// the other fields of the structure have no effect..
    operator USound();
    operator ufloat ();
    operator std::string ();
    operator UList();

    /// No effect in plugin mode. In remote mode, updates the value
    /// once asynchronously..
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

    // internal
    void __update(UValue&);

    void setZombie ();

  private:
    /// Xxx only works in softdevice mode.
    UValue& val()
    {
      return value;
    }

    /// Pointer to internal data specifics.
    UVardata  *vardata;
    void __init();

    /// Full name of the variable as seen in URBI.
    PRIVATE(std::string, name)
    /// The variable value on the softdevice's side.
    PRIVATE(UValue, value)

    // Check that the invariant of this class are verified.
    bool invariant () const;
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


  inline
  UVar::UVar()
    : owned (false),
      VAR_PROP_INIT,
      vardata (0), name ("noname")
  {}

  inline
  UVar::UVar(UVar&)
    : owned (false),
      VAR_PROP_INIT,
      vardata (0), name ("")
  {
    /// FIXME: This is really weird: a copy-ctor that does not use
    /// the lhs?
  }

  inline void UProp::operator =(const UValue& v)  {owner.setProp(name, v);}
  inline void UProp::operator =(const ufloat v)  {owner.setProp(name, v);}
  inline void UProp::operator =(const std::string& v){owner.setProp(name, v);}
  inline UProp::operator ufloat()  {return (ufloat)owner.getProp(name);}
  inline UProp::operator std::string()  {return owner.getProp(name);}
  inline UProp::operator UValue()  {return owner.getProp(name);}

} // end namespace urbi

/// Report \a u on \a o for debugging.
std::ostream& operator<< (std::ostream& o, const urbi::UVar& u);

# undef PRIVATE

#endif // ! URBI_UVAR_HH
