/// \file urbi/uprop.hh

// This file is part of UObject Component Architecture
// Copyright (c) 2007, 2008 Gostai S.A.S.
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//
// For more information, comments, bug reports: http://www.urbiforge.com

#ifndef URBI_UPROP_HH
# define URBI_UPROP_HH

# include <libport/export.hh>

# include <urbi/uvalue.hh>
# include <urbi/uproperty.hh>

namespace urbi
{

  //! Provides easy access to variable properties
  class URBI_SDK_API UProp
  {
  public:
    void operator =(const UValue& v);
    void operator =(const ufloat v);
    void operator =(const std::string& v);

    operator ufloat();
    operator std::string();
    operator UValue();

    UProp(UVar& owner, UProperty name);

  private:
    UVar& owner;
    UProperty name;

    // Disable copy ctor and assignment operator.
    UProp(const UProp&);
    UProp& operator =(const UProp&);
  };

} // end namespace urbi

#endif // ! URBI_UVAR_HH
