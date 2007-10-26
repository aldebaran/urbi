/*! \file userver.hh
 *******************************************************************************

 File: userver.h\n
 Definition of the USystem class.

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

#ifndef USERVER_HH
# define USERVER_HH

# include <cstdarg>
# include <sstream>

# include "libport/config.h"
# if ! defined LIBPORT_URBI_ENV_AIBO
#  include <boost/thread.hpp>
# endif

# include "libport/fwd.hh"
# include "libport/compiler.hh"
# include "libport/lockable.hh"

# include "kernel/fwd.hh"
# include "kernel/ustring.hh"
# include "kernel/utypes.hh"
# include "kernel/tag-info.hh"

extern  const char* EXTERNAL_MESSAGE_TAG;
extern  const char* DISPLAY_FORMAT;
extern  const char* DISPLAY_FORMAT1;
extern  const char* DISPLAY_FORMAT2;

extern  const char* MAINDEVICE;

extern  const char* UNKNOWN_TAG;
extern  class UServer   *urbiserver; // Global variable for the server


//! UServer class: handles all URBI system processing.
/*! There must be one UServer defined in the program and it must be overloaded
    to make it specific to the particular robot.

    UServer is used to store the UConnection list and the UDevice list.
    This object does all the internal processing of URBI and handles the pool
    of UCommand's.
*/
class UServer: public libport::Lockable
{
public:
  //! UServer constructor.
  /*! UServer constructor

   Unlike UConstructor, it is not required that you handle the memory
   management task when you create the robot-specific sub class. The
   difference in memory between your class and the UServer class is
   considered as neglectible and included in the security margin. If you
   don't understand this point, ignore it.

   \param frequency gives the value in msec of the server update,
   which are the calls to the "work" function. These calls must be done at
   a fixed, precise, real-time frequency to let the server computer motor
   trajectories between two "work" calls.

   \param freeMemory indicates the biggest malloc possible on the system
   when the server has just started. It is used to determine a high
   limit of memory allocation, thus avoiding later to run out of memory
   during a new or malloc.
   */
  UServer(ufloat frequency, int freeMemory, const char* mainName);

  virtual ~UServer();

public:
  //! Initialization of the server. Displays the header message & init stuff
  /*! This function must be called once the server is operational and
   able to print messages. It is a requirement for URBI compliance to print
   the header at start, so this function *must* be called. Beside, it also
   do initalization work for the devices and system variables.
   */
  void initialize();
  /// Obsolete name for initialize().
  void initialization () { initialize(); }

  //! Main processing loop of the server
  /*! This function must be called every "frequency_" msec to ensure the proper
   functionning of the server. It will call the command execution, the
   connection message sending when they are delayed, etc...

   "frequency_" is a parameter of the server, given in the constructor.
   */
  void work();

  /// Set the system.args list in URBI.
  void main (int argc, const char* argv[]);


  /// Package information about this server.
  static const libport::PackageInfo& package_info ();

  void error (const char* s, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));
  void echo (const char* s, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));


  //! Display a formatted message, with a key.
  /*! This function uses the virtual URobot::display() function to make the
   message printing robot-specific.
   It formats the output in a standard URBI way by adding a key between
   brackets at the end. This key can be "" or NULL.It can be used to
   visually extract information from the flux of messages printed by
   the server.
   \param key is the message key. Maxlength = 5 chars.
   \param s   is the formatted string containing the message.
   */
  void vecho_key (const char* key, const char* s, va_list args)
    __attribute__ ((__format__ (__printf__, 3, 0)));
  void echoKey (const char* key, const char* s, ...)
    __attribute__ ((__format__ (__printf__, 3, 4)));

  /// Send debugging data.
  void vdebug (const char* s, va_list args)
    __attribute__ ((__format__ (__printf__, 2, 0)));
  void debug (const char* s, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));

  void              isolate         ();
  void              deIsolate       ();
  bool              isIsolated      ();

  virtual ufloat    getTime         () = 0;
  virtual ufloat    getPower        () = 0;

  //! Overload this function to return a specific header for your URBI server
  /*! Used to give some information specific to your server in the standardized
   header which is displayed on the server output at start and in the
   connection when a new connection is created.\n
   Typical custom header should be like:

   *** URBI version xx.xx for \<robotname\> robot\\n\n
   *** (c) Copyright \<year\> \<name\>\\n

   The function should return in header the line corresponding to 'line'
   or an empty string (not NULL!) when there is no line any more.
   Each line is supposed to end with a carriage return \\n and each line should
   start with three empty spaces. This complicated method is necessary to allow
   the connection to stamp every line with the standard URBI prefix [time:tag].

   \param line is the requested line number
   \param header the custom header
   \param maxlength the maximum length allowed for the header (the parameter
   has been malloc'ed for that size). Typical size is 1024 octets and
   should be enough for any reasonable header.
   */
  virtual void      getCustomHeader (int line, char* header,
				     int maxlength) = 0;

  /// A list of directory names.
  typedef std::list<std::string> path_type;
  /// Where to look for files to load.
  // Should eventually become an Urbi variable.
  // Should probably be changeable with an envvar.
  // By default, empty (not even ".").  We rely on find_file to
  // return at least the file name.
  path_type path;

  /// Return the full file name, handle paths.
  /// Return \a f on failure.
  virtual std::string find_file (const char* f);

  /// Load a file into the connection.
  /// Returns UFAIL if anything goes wrong, USUCCESS otherwise.
  virtual UErrorValue loadFile (const char *filename,
				UCommandQueue* loadQueue);

  /// Save content to a file
  /// This function must be redefined by the robot-specific server.
  /// Returns UFAIL if anything goes wrong, USUCCESS otherwise.
  virtual UErrorValue saveFile (const char *filename,
				const char * content) = 0;

  //! Check if there is enough free memory to run
  /*! Every time there is a new, a malloc, a delete or free or a strdup in the
   server, the global variable "usedMemory" is updated. The "memoryCheck"
   function checks that the currently used memory is less than the maximum
   availableMemory declared at the the server initialization.
   If it is more than the maximum, an memoryOverflow is raised and the
   server enter isolation mode.

   \sa isIsolated()
   */
 void              memoryCheck     ();

  //! Evaluate how much memory is available for a malloc
  /*! This function tries to evaluate how much memory is available for a malloc,
   using brute force dichotomic allocation. This is the only known way to get
   this information on most systems (like OPENR).
   */
  // FIXME: Why is this a member function?
  size_t memory () const;

  UVariable*        getVariable     ( const char *device,
				      const char *property);


  ufloat            getFrequency    ();
  void              mark            (UString *stopTag);
  virtual void      reboot          () = 0;
  virtual void      shutdown        () = 0;
  virtual void      beforeWork      ();
  virtual void      afterWork       ();

  /// Display a message on the robot console
  void              display         (const char*);
  /// Display a set of messages on the robot console.
  void              display         (const char**);

  ufloat            lastTime        ();
  void              updateTime      ();
  void              addConnection   (UConnection* connection);
  void              removeConnection(UConnection* connection);
  inline int        getUID          ()
  {
    libport::BlockLock bl(this);
    return ++uid;
  }
  int               addAlias        (const char* id, const char* variablename);

  // A usual connection to stop dependencies.
  UConnection& getGhostConnection ();

  void              freeze          (const std::string &tag);
  void              unfreeze        (const std::string &tag);
  void              block           (const std::string &tag);
  void              unblock         (const std::string &tag);
  /// List of active connections: includes one UGhostConnection.
  std::list<UConnection*>  connectionList;

  /// Hash of variable values.
  HMvariabletab            variabletab;

  /// Hash of variable that have both an access and change notify
  std::list<UVariable*>    access_and_change_varlist;

  /// Hash of function definition.
  HMfunctiontab            functiontab;
  /// Hash of functions definition markers.
  HMfunctiontab            functiondeftab;
  /// Hash of objects hierarchy.
  HMobjtab                 objtab;
  /// Hash of alias definitions.
  HMaliastab               aliastab;
  /// Hash of obj alias definitions.
  HMaliastab               objaliastab;
  /// Hash of group definitions.
  HMgrouptab               grouptab;
  /// Hash of events, one entry per (name,nbArgs).
  HMemittab                emittab;
  /// Hash of events, one entry per name.
  HMemit2tab               emit2tab;
  /// Hash of function binders.
  HMbindertab              functionbindertab;
  /// Hash of event binders.
  HMbindertab              eventbindertab;
  /// Hash of obj name waiting for a remote new.
  HMobjWaiting             objWaittab;
  /// Hash of all tags currently 'instanciated'
  HMtagtab                 tagtab;
  /// Array of list of UObjects registered for a system messages
  /// The system message type is the index.
  std::vector<std::list<urbi::USystem*> > systemObjects;

  /// Variables to reinit (nbAverage=0).
  std::list<UVariable*>         reinitList;
  /// List of variables to delete after a reset.
  std::list<UVariable*>         resetList;
  /// True when the server is in the process of resting.
  bool reseting;
  /// Reseting stage.
  int stage;
  /// List of variables to delete in a reset command.
  std::list<UVariable*> varToReset;

  /// Flag used to signal a memory overflow.
  bool memoryOverflow;

  /// Shows debug or not.
  bool debugOutput;

# if ! defined LIBPORT_URBI_ENV_AIBO
  /// Used to synchronize message reception.
  boost::recursive_mutex mutex;
# endif

private:
  /// Name of the main device.
  UString mainName_;

public:
  /// True after a stop command.
  bool somethingToDelete;
  /// True after the initialization phase: all vars are uservar then.
  bool uservarState;

  /// Cpu load expressed as a number between 0 and 1.
  ufloat                   cpuload;
  /// True when there is a cpu overload.
  bool                     cpuoverload;
  /// A signal must be sent to every connection.
  bool                     signalcpuoverload;
  /// Nb of recent cpu overloads.
  int                      cpucount;
  /// Threshold for cpu overload alert.
  ufloat                   cputhreshold;
  /// True when the server is paranoid on def checking.
  bool                     defcheck;
  ufloat                   previous2Time,
			   previous3Time,
			   currentTime,
			   previousTime,
			   latestTime; ///< used to detect cpu overload
  /// Stops all commands in all connections.
  bool                     stopall;
  /// False inside parsing, true otherwise for commands created by the
  /// kernel.
  bool                     systemcommands;

  /// Urbi TCP Port..
  enum { TCP_PORT = 54000 };

protected:
  virtual void     effectiveDisplay         (const char*) = 0;

private:
  void mark (TagInfo*);
  /// Used by echo()& error().
  enum { MAXSIZE_INTERNALMESSAGE = 1024 };
  /// Amount of security mem.
  enum { SECURITY_MEMORY_SIZE = 100000 };

  /// Frequency of the calls to work().
  ufloat           frequency_;
  /// Stores memory for emergency use..
  void*            securityBuffer_;
  /// Is the server isolated.
  bool             isolate_;
  /// Store the time on the last call to updateTime();.
  ufloat           lastTime_;
  /// The ghost connection used for URBI.INI.
  UGhostConnection* ghost_;

  /// unique id source
  int               uid;
};

//! Accessor for frequency_.
inline ufloat
UServer::getFrequency()
{
  return frequency_;
}

//! Accessor for lastTime_.
inline ufloat
UServer::lastTime()
{
  return lastTime_;
}


inline
int unic()
{
  /// Unique identifier to create new references.
  static int cnt = 10000;
  return ++cnt;
}

// Return an identifier starting with \a prefix, ending with a unique int.
inline
std::string unic (const char* prefix)
{
  std::ostringstream o;
  o << prefix << unic();
  return o.str();
}


/*-------------------------.
| Freestanding functions.  |
`-------------------------*/

/// Send debugging messages via ::urbiserver.
void debug (const char* fmt, va_list args)
  __attribute__ ((__format__ (__printf__, 1, 0)));

/// Send debugging messages via ::urbiserver.
void debug (const char* fmt, ...)
  __attribute__ ((__format__ (__printf__, 1, 2)));

/// Send debugging messages indented with \a t spaces, via ::urbiserver.
void debug (unsigned t, const char* fmt, ...)
  __attribute__ ((__format__ (__printf__, 2, 3)));

// Send debugging messages.
#if URBI_DEBUG
// Must be invoked with two pairs of parens.
# define DEBUG(Msg)  debug Msg
#else
# define DEBUG(Msg) ((void) 0)
#endif

#endif

// Local Variables:
// mode: c++
// End:
