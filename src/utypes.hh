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

/*--------------------.
| Memory allocation.  |
`--------------------*/

/// Keep track of how much memory has been used for commands, buffers,
/// etc.
extern  int   usedMemory;
/// Total amount of free memory in the system.
extern  int   availableMemory;

// FIXME: Why applying the 1.15 threshold here instead of where we
// consult usedMemory?
#if 0
# define ADDMEM(X)   usedMemory += (int) ((X) * 1.15)
#else
# define ADDMEM(x)   {usedMemory += ((int)(x*1.15));}
#endif

# define FREEMEM(X)  ADDMEM (-(X))

# define ADDOBJ(X)   ADDMEM (sizeof(X))
# define FREEOBJ(X)  FREEMEM (sizeof(X))

# define LIBERATE(X)				\
  do {						\
    if ((X) && (X)->liberate() == 0)		\
      delete X;					\
  } while (0)


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

/// Return code for variable Update
enum UVarSet
{
  UOK,
  USPEEDMAX
};

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

/// Type of defs in UCommand_DEF
enum UDefType
{
  UDEF_FUNCTION,
  UDEF_VAR,
  UDEF_VARS,
  UDEF_EVENT,
  UDEF_QUERY
};

/// Type of Derivative
enum UDeriveType
{
  UNODERIV,
  UDERIV,
  UDERIV2,
  UTRUEDERIV,
  UTRUEDERIV2
};


/// Results of a test
enum UTestResult
{
  UFALSE,
  UTRUE,
  UTESTFAIL
};

/// Possible status for a UCommand
enum UCommandStatus
{
  UONQUEUE,
  URUNNING,
  UCOMPLETED,
  UBACKGROUND,
  UMORPH
};


enum UEventCompoundType
{
  EC_MATCH,
  EC_AND,
  EC_OR,
  EC_BANG
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

// UBlendType.
using urbi::UBlendType;
using urbi::UMIX;
using urbi::UADD;
using urbi::UDISCARD;
using urbi::UQUEUE;
using urbi::UCANCEL;
using urbi::UNORMAL;

/// Node type for a UCommand_TREE
enum UNodeType
{
  UAND,
  UPIPE,
  USEMICOLON,
  UCOMMA
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

# define ABSF(x)     (((x)>0)? (x) : (-(x)) )

/** Class containing the number of pending call to a remote new for
 * a given class name (id).
 *  */
class UWaitCounter
{
  public:
    UWaitCounter(UString *id, int nb);
    ~UWaitCounter();

   UString* id; ///< class name
   int nb; ///< nb of waiting calls
};



class TagInfo;
typedef libport::hash_map_type<const char*, UVariable*>::type HMvariabletab;
typedef libport::hash_map_type<const char*, UFunction*>::type HMfunctiontab;
typedef libport::hash_map_type<const char*, UObj*>::type HMobjtab;
typedef libport::hash_map_type<const char*, UGroup*>::type HMgrouptab;
typedef libport::hash_map_type<const char*, UString*>::type HMaliastab;
typedef libport::hash_map_type<const char*, UEventHandler*>::type HMemittab;
typedef libport::hash_map_type<const char*, UBinder*>::type HMbindertab;
typedef libport::hash_map_type<const char*, UWaitCounter*>::type HMobjWaiting;
typedef libport::hash_map_type<std::string, TagInfo>::type HMtagtab;

/** Structure containing informations related to a tag.
 We have a hash table of those.
 An entry survives as long as a command has the tag, or if either froezen or
 blocked is set.
 Each entry is linked to parent entry (a.b ->a) and to all commands having the
 tag.
*/
class TagInfo
{
  public:
  TagInfo():frozen(false), blocked(false), parent(0)  {}
    bool frozen;
    bool blocked;
    std::list<UCommand*> commands; ///< All commands with this tag
    std::list<TagInfo *> subTags; ///< All tags with this one as direct parent
    TagInfo * parent;
    std::list<TagInfo*>::iterator parentPtr; ///< iterator in parent child list
    std::string name;

    /// Insert a Taginfo in map,link to parent creating if needed, recursively
    TagInfo * insert(HMtagtab & tab);
};


#endif
