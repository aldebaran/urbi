/// \file urbi/uproperty.hh

// This file is part of UObject Component Architecture
// Copyright (c) 2008 Gostai S.A.S.
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//
// For more information, comments, bug reports: http://www.urbiforge.com

#ifndef URBI_UPROPERTY_HH
# define URBI_UPROPERTY_HH

# include <libport/cstring>
# include <libport/assert.hh>

namespace urbi
{

  /// URBI properties associated to a variable
  enum UProperty
  {
    PROP_RANGEMIN,
    PROP_RANGEMAX,
    PROP_SPEEDMIN,
    PROP_SPEEDMAX,
    PROP_BLEND,
    PROP_DELTA,
  };

  namespace
  {
    const char* UPropertyNames[]=
    {
      "rangemin",
      "rangemax",
      "speedmin",
      "speedmax",
      "blend",
      "delta",
    };
  }

  // FIXME: This is needed by urbi-sdk/uvar.cc.
  inline
  bool
  is_propertytype (int i)
  {
    return (static_cast<int> (PROP_RANGEMAX) <= i
	    && i <= static_cast<int> (PROP_DELTA));
  }

  // values for enum-like properties
  inline
  const char*
  name (UProperty u)
  {
    return UPropertyNames[static_cast <int> (u)];
  }

  inline
  UProperty
  uproperty (const char* cp)
  {
    for (int i = 0; is_propertytype (i); ++i)
      if (STREQ (UPropertyNames[i], cp))
	return static_cast<UProperty> (i);
    pabort ("unknown uproperty: " << cp);
  }

}

#endif
