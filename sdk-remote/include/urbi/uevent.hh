/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// \file urbi/uevent.hh
#ifndef URBI_UEVENT_HH
# define URBI_UEVENT_HH

# include <iosfwd>
# include <string>

# include <urbi/ucontext.hh>

namespace urbi
{
  /** UEvent class definition

     Each UEvent instance corresponds to one Urbi event. The class
     provides access to the event properties, and emitting/receiving
     events with payload.  */
  class URBI_SDK_API UEvent: public UContext
  {
  public:
    /// Creates an unbound UEvent. Call init() to bind it.
    UEvent();
    UEvent(const std::string& varname, urbi::impl::UContextImpl* impl = 0);
    UEvent(const std::string& objname, const std::string& varname,
           urbi::impl::UContextImpl* impl = 0);
    UEvent(urbi::UObject& obj, const std::string& varname,
           urbi::impl::UContextImpl* impl = 0);
    UEvent(const UEvent& e);

    // Bind to \a object.slot.
    void init(const std::string& object, const std::string& slot,
              urbi::impl::UContextImpl* = 0);

    // Emit the event.
    void emit(urbi::UAutoValue v1 = UAutoValue(),
              urbi::UAutoValue v2 = UAutoValue(),
              urbi::UAutoValue v3 = UAutoValue(),
              urbi::UAutoValue v4 = UAutoValue(),
              urbi::UAutoValue v5 = UAutoValue(),
              urbi::UAutoValue v6 = UAutoValue(),
              urbi::UAutoValue v7 = UAutoValue());

 private:
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
# undef PRIVATE
  };
} // end namespace urbi

# include <urbi/uevent.hxx>

#endif // ! URBI_UEVENT_HH
