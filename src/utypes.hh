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

#ifndef UTYPE_HH
# define UTYPE_HH

# include "libport/cstring"
# include <list>
# ifdef _MSC_VER
#  define snprintf _snprintf
#  define vsnprintf _vsnprintf
# endif

# include "libport/hash.hh"
# include "libport/ufloat.h"

# include "fwd.hh"

extern  int   usedMemory; //< Keeps track of how much memory has been used for
			  //< commands, buffers, etc...
extern  int   availableMemory; //< Total amount of free memory in the system

# define ADDMEM(x)   {usedMemory += ((int)(x*1.15));}
# define FREEMEM(x)  {usedMemory -= ((int)(x*1.15));}
# define ADDOBJ(x)   {usedMemory += ((int)(sizeof(x)*1.15));}
# define FREEOBJ(x)  {usedMemory -= ((int)(sizeof(x)*1.15));}

# define LIBERATE(x) if (x && x->liberate()==0) delete (x)
# define DD ::urbiserver->debug("Check point %s, %d\n", __FILE__, __LINE__);
# define DDD(x) ::urbiserver->debug("Check point [%s] on %s, %d\n", \
				    x, __FILE__, __LINE__);

# define ASSERT(test) if (!(test)) \
  ::urbiserver->debug("ASSERT FAILED: %s in %s %d\n", \
  # test, __FILE__, __LINE__); else

typedef unsigned long IPAdd;

//! Return code values
enum UErrorValue
{
  USUCCESS,
  UFAIL,
  UMEMORYFAIL
};

//! Return code for variable Update
enum UVarSet
{
  UOK,
  USPEEDMAX
};

//! Type of Errors
enum UErrorCode
{
  UERROR_CRITICAL,
  UERROR_SYNTAX,
  UERROR_DIVISION_BY_ZERO,
  UERROR_RECEIVE_BUFFER_FULL,
  UERROR_MEMORY_OVERFLOW,
  UERROR_SEND_BUFFER_FULL,
  UERROR_RECEIVE_BUFFER_CORRUPTED,
  UERROR_MEMORY_WARNING,
  UERROR_CPU_OVERLOAD
};

//! Type of Bind modes
enum UBindMode
{
  UEXTERNAL,
  UINTERNAL
};

//! Type of binding
enum UBindType
{
  UBIND_FUNCTION,
  UBIND_VAR,
  UBIND_EVENT,
  UBIND_OBJECT
};

//! Type of defs in UCommand_DEF
enum UDefType
{
  UDEF_FUNCTION,
  UDEF_VAR,
  UDEF_VARS,
  UDEF_EVENT,
  UDEF_QUERY
};



//! Type of Warnings
enum UWarningCode
{
  UWARNING_MEMORY
};

//! Type of Derivative
enum UDeriveType
{
  UNODERIV,
  UDERIV,
  UDERIV2,
  UTRUEDERIV,
  UTRUEDERIV2
};


//! Results of a test
enum UTestResult
{
  UFALSE,
  UTRUE,
  UTESTFAIL
};

//! The different types for a UCommand
enum UCommandType
{
  CMD_GENERIC,
    CMD_TREE,
    CMD_ASSIGN_VALUE,
    CMD_ASSIGN_PROPERTY,
    CMD_ASSIGN_BINARY,
    CMD_EXPR,
    CMD_RETURN,
    CMD_ECHO,
    CMD_NEW,
    CMD_ALIAS,
    CMD_INHERIT,
    CMD_GROUP,
    CMD_WAIT,
    CMD_WAIT_TEST,
    CMD_INCREMENT,
    CMD_DECREMENT,
    CMD_DEF,
    CMD_CLASS,
    CMD_IF,
    CMD_EVERY,
    CMD_TIMEOUT,
    CMD_STOPIF,
    CMD_FREEZEIF,
    CMD_AT,
    CMD_AT_AND,
    CMD_WHILE,
    CMD_WHILE_AND,
    CMD_WHILE_PIPE,
    CMD_WHENEVER,
    CMD_LOOP,
    CMD_LOOPN,
    CMD_LOOPN_PIPE,
    CMD_LOOPN_AND,
    CMD_FOREACH,
    CMD_FOREACH_PIPE,
    CMD_FOREACH_AND,
    CMD_FOR,
    CMD_FOR_PIPE,
    CMD_FOR_AND,
    CMD_NOOP,
    CMD_EMIT,
    CMD_LOAD
  };

//! Possible status for a UCommand
enum UCommandStatus
{
  UONQUEUE,
  URUNNING,
  UCOMPLETED,
  UBACKGROUND,
  UMORPH
};

//! The different types for a UExpression.
enum UExpressionType
{
  EXPR_VALUE,
  EXPR_VARIABLE,
  EXPR_LIST,
  EXPR_GROUP,
  EXPR_ADDR_VARIABLE,
  EXPR_FUNCTION,
  EXPR_PLUS,
  EXPR_MINUS,
  EXPR_MULT,
  EXPR_DIV,
  EXPR_MOD,
  EXPR_EXP,
  EXPR_NEG,
  EXPR_COPY,
  EXPR_PROPERTY,
  EXPR_EVENT,

  EXPR_TEST_EQ,
  EXPR_TEST_REQ,
  EXPR_TEST_PEQ,
  EXPR_TEST_DEQ,
  EXPR_TEST_NE,
  EXPR_TEST_GT,
  EXPR_TEST_GE,
  EXPR_TEST_LT,
  EXPR_TEST_LE,
  EXPR_TEST_BANG,
  EXPR_TEST_AND,
  EXPR_TEST_OR
};

enum UEventCompoundType
{
  EC_MATCH,
  EC_AND,
  EC_OR,
  EC_BANG
};

//! The different Data types
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

//! Blending mode
enum UBlend
{
  UMIX,
  UADD,
  UDISCARD,
  UQUEUE,
  UCANCEL,
  UNORMAL
};

//! Node type for a UCommand_TREE
enum UNodeType
{
  UAND,
  UPIPE,
  USEMICOLON,
  UCOMMA
};

//! Runlevel type for a binary tree exploration
enum URunlevel
{
  UWAITING,
  UEXPLORED,
  UTERMINATED
};

//! Return code values for the setDeviceVar method
enum UReport
{
  UDONE,
  UCONTINUE
};


typedef unsigned char ubyte;

//static const urbi::ufloat TRUE = urbi::ufloat(1);
//static const urbi::ufloat FALSE = urbi::ufloat(0);
# define ABSF(x)     (((x)>0)? (x) : (-(x)) )


//! URefPt is used to make references to pointers.
template <class T>
class URefPt
{
public:

  URefPt(T* p_)
  {
    p = p_;
    cpt = 1;
  }

  ~URefPt()
  {
    delete p;
  }

  int   cpt;
  T* p;

  T* ref()
  {
    return p;
  }

  int liberate()
  {
    cpt--;
    if (cpt<=0)
    {
      delete p;
      p=0;
    }
    return cpt;
  }

  URefPt *copy()
  {
    cpt++;
    return this;
  }
};





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
typedef  urbi::hash_map_type<const char*, UVariable*>::type HMvariabletab ;
typedef  urbi::hash_map_type<const char*, UFunction*>::type HMfunctiontab ;
typedef  urbi::hash_map_type<const char*, UObj*>::type HMobjtab ;
typedef  urbi::hash_map_type<const char*, UGroup*>::type HMgrouptab ;
typedef  urbi::hash_map_type<const char*, UString*>::type HMaliastab ;
typedef  urbi::hash_map_type<const char*, UEventHandler*>::type HMemittab ;
typedef  urbi::hash_map_type<const char*, UBinder*>::type HMbindertab ;
typedef  urbi::hash_map_type<const char*, UWaitCounter*>::type HMobjWaiting;
typedef  urbi::hash_map_type<std::string, TagInfo>::type HMtagtab;

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
