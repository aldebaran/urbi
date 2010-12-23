/*
 * Copyright (C) 2007, 2008, 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/uvar.hh
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
     provides access to the variable properties, and reading/writing
     the value to/from all known types.  */
  class URBI_SDK_API UVar: public UContext
  {
  public:
    /// Creates an unbound UVar. Call init() to bind it.
    UVar();
    UVar(const std::string&, impl::UContextImpl* = 0);
    UVar(const std::string&, const std::string&, impl::UContextImpl* = 0);
    UVar(UObject&, const std::string&, impl::UContextImpl* = 0);
    UVar(const UVar&);
  private:
    UVar& operator=(const UVar&);
  public:
    ~UVar();

    // Bind to \a object.slot.
    void init(const std::string& varname, impl::UContextImpl* = 0);
    void init(const std::string& object, const std::string& slot,
              impl::UContextImpl* = 0);
    void setOwned();

    /// The type of the current content.
    UDataType type() const;

    /// Request the current value, wait until it is available.
    void syncValue();

    /// Keep this UVar synchronized with kernel value.
    void keepSynchronized();

    void reset (ufloat);

    UVar& operator=(ufloat);
    UVar& operator=(const std::string&);
    /// Deep copy.
    UVar& operator=(const UBinary&);
    /// Deep copy.
    UVar& operator=(const UImage& i);
    /// Deep copy.
    UVar& operator=(const USound& s);

    UVar& operator=(const UList& l);

    UVar& operator=(const UDictionary& d);

    UVar& operator=(const UValue& v);

    template<typename T>
    UVar& operator=(const T&);

    template<typename T>
    bool operator ==(const T&) const;

    /// Cast operator taking a dummy value of the target type.
    template<typename T> T as(T*) const;
    /// Generic cast operator using the extensible uvalue_cast mechanism.
    template<typename T> T as() const;
    /// Conveniance wrapper on as().
    template<typename T> T& fill(T&) const;

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
    operator UDictionary() const;

    /// Deactivate all callbacks associated with this UVar and stop synchro.
    void unnotify();

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
    UProp constant;

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

    /// Use RTP mode to transmit this variable if possible.
    void useRTP(bool enable=true);
    impl::UVarImpl* impl_;
    const UValue& val() const;
    libport::utime_t timestamp() const;


    enum RtpMode {
      RTP_DEFAULT, ///< Use RTP if it is the default mode
      RTP_YES,     ///< Force RTP
      RTP_NO       ///< Do not use RTP
    };

     /// Check that impl_ is set or throw a runtime error.
    void check() const;
  private:
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
    Type get_ ##  Name () const		        \
    {						\
      return Name;				\
    }						\
    void set_ ##  Name (const Type& v)		\
    {						\
      Name = v;					\
    }						\
  private:					\
    Type Name;

    /// Full name of the variable as seen in URBI.
    PRIVATE(std::string, name)

    /// True if the variable is a temporary and must not be stored in callbacks
    PRIVATE(bool, temp);

    PRIVATE(RtpMode, rtp);

    /// If set, this variable will never export its content remotely.
    PRIVATE(bool, local);
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
  blend(*this, PROP_BLEND),                     \
  constant(*this, PROP_CONSTANT)

  /// Report \a u on \a o for debugging.
  URBI_SDK_API
  std::ostream& operator<< (std::ostream& o, const urbi::UVar& u);

} // end namespace urbi

# include <urbi/uvar.hxx>

#endif // ! URBI_UVAR_HH
