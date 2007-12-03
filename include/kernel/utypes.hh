/*! \file utypes.hh
 *******************************************************************************

 File: utypes.h\n
 Definition of useful types in the URBI server kernel.

 This file must be included if someone wants to reuse in an other context some
 generic classes like the UStream circular buffer. Actually, from a design point
 of view, this is the only reason to be for utype.h :-)

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UTYPES_HH
# define UTYPES_HH

# include <list>
# include "libport/cstring"

# include "libport/hash.hh"
# include "libport/ufloat.h"

# include "fwd.hh"

# include "urbi/utypes-common.hh"

# include "kernel/fwd.hh"
# include "kernel/mem-track.hh"
# include "kernel/ustring.hh"


/*------------.
| Debugging.  |
`------------*/

# define DD								\
  ::urbiserver->debug("Check point %s, %d\n", __FILE__, __LINE__)

# define DDD(x)								\
  ::urbiserver->debug("Check point [%s] on %s, %d\n", x, __FILE__, __LINE__)

# define ASSERT(Test)					\
  if (!(Test))						\
    ::urbiserver->debug("ASSERT FAILED: %s in %s %d\n", \
			#Test, __FILE__, __LINE__);	\
  else



typedef unsigned long IPAdd;

/// Return code values
enum UErrorValue
{
  USUCCESS,
  UFAIL,
  UMEMORYFAIL
};

inline
std::ostream&
operator<< (std::ostream& o, UErrorValue v)
{
  switch (v)
  {
#define CASE(V) case V: o << #V; break;
    CASE(USUCCESS);
    CASE(UFAIL);
    CASE(UMEMORYFAIL);
#undef CASE
  }
  return o;
}

/// Type of Bind modes
enum UBindMode
{
  UEXTERNAL,
  UINTERNAL
};

/// Type of binding
enum UBindType
{
  UBIND_FUNCTION,
  UBIND_VAR,
  UBIND_EVENT,
  UBIND_OBJECT
};

/// Results of a test
enum UTestResult
{
  UFALSE,
  UTRUE,
  UTESTFAIL
};

/// The different Data types
enum UDataType
{
  DATA_UNKNOWN,
  DATA_NUM,
  DATA_STRING,
  DATA_FILE,
  DATA_BINARY,
  DATA_VOID,
  DATA_LIST,
  DATA_OBJ,
  DATA_FUNCTION,
  DATA_VARIABLE
};

/// Runlevel type for a binary tree exploration
enum URunlevel
{
  UWAITING,
  UEXPLORED,
  UTERMINATED
};

/// Return code values for the setDeviceVar method
enum UReport
{
  UDONE,
  UCONTINUE
};


typedef unsigned char ubyte;

# define ABSF(x)     (((x)>0)? (x) : (-(x)))

/// The number of pending call to a remote new for a given class name (id).
class UWaitCounter
{
public:
  UWaitCounter(const UString& id, int nb)
    : id(id),
      nb(nb)
  {
  }

  UString id; ///< class name
  int nb; ///< nb of waiting calls
};


typedef libport::hash_map<const char*, UVariable*> HMvariabletab;
typedef HMvariabletab::value_type HMvariable_type;

typedef libport::hash_map<const char*, UFunction*> HMfunctiontab;
typedef HMfunctiontab::value_type HMfunction_type;

typedef libport::hash_map<const char*, UObj*> HMobjtab;
typedef HMobjtab::value_type HMobj_type;

typedef libport::hash_map<const char*, UGroup*> HMgrouptab;
typedef HMgrouptab::value_type HMgroup_type;

typedef libport::hash_map<const char*, UString*> HMaliastab;
typedef HMaliastab::value_type HMalias_type;

typedef libport::hash_map<const char*, UEventHandler*> HMemittab;
typedef HMemittab::value_type HMemit_type;

typedef libport::hash_map<const char*, int> HMemit2tab;
typedef HMemit2tab::value_type HMemit2_type;

typedef libport::hash_map<const char*, UBinder*> HMbindertab;
typedef HMbindertab::value_type HMbinder_type;

typedef libport::hash_map<const char*, UWaitCounter*> HMobjWaiting;
typedef HMobjWaiting::value_type HMobjWait_type;

typedef libport::hash_map<std::string, TagInfo> HMtagtab;
// TagInfo is not defined yet, only fwd declared.  Cannot make the
// following definition.
// typedef HMtagtab::value_type HMtag_type;

#endif
