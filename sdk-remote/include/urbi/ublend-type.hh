/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_UBLEND_TYPE_HH
# define URBI_UBLEND_TYPE_HH

# include <libport/cstring>
# include <libport/cassert>

namespace urbi
{

  /*-------------.
  | UBlendType.  |
  `-------------*/

  //WARNING: synchronize with blendNames below, must be 0-based
  /// Possible blending mode for an UVar
  enum UBlendType
  {
    UMIX,
    UADD,
    UDISCARD,
    UQUEUE,
    UCANCEL,
    UNORMAL
  };

  namespace
  {
    const char* UBlendNames[]=
    {
      "mix",
      "add",
      "discard",
      "queue",
      "cancel",
      "normal",
    };
  }

  inline
  // FIXME: This is needed by urbi-sdk/uvar.cc.
  bool
  is_blendtype (int i)
  {
    return static_cast<int> (UMIX) <= i && i <= static_cast<int> (UNORMAL);
  }

  // values for enum-like properties
  inline
  const char*
  name (UBlendType u)
  {
    return UBlendNames[static_cast <int> (u)];
  }

  inline
  UBlendType
  ublendtype (const char* cp)
  {
    for (int i = 0; is_blendtype (i); ++i)
      if (libport::streq (UBlendNames[i], cp))
	return static_cast<UBlendType> (i);
    pabort ("unknown blendtype: " << cp);

  }

} // end namespace urbi

#endif // ! URBI_UBLEND_TYPE_HH
