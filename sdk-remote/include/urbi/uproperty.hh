/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/uproperty.hh

#ifndef URBI_UPROPERTY_HH
# define URBI_UPROPERTY_HH

# include <libport/cstring>
# include <libport/cassert>

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
    PROP_CONSTANT,
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
      "constant",
    };
  }

  // FIXME: This is needed by urbi-sdk/uvar.cc.
  inline
  bool
  is_propertytype (int i)
  {
    return (static_cast<int> (PROP_RANGEMAX) <= i
	    && i <= static_cast<int> (PROP_CONSTANT));
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
      if (libport::streq (UPropertyNames[i], cp))
	return static_cast<UProperty> (i);
    pabort ("unknown uproperty: " << cp);
  }

}

#endif
