/*! \file kernel/userver.hh
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

#ifndef KERNEL_USERVER_HH
# define KERNEL_USERVER_HH

# include <cstdarg>
# include <sstream>

# include <libport/config.h>
# if ! defined LIBPORT_URBI_ENV_AIBO
#  include <boost/thread.hpp>
# endif

# include <libport/fwd.hh>
# include <libport/compiler.hh>
# include <libport/file-library.hh>
# include <libport/ufloat.h>
# include <libport/utime.hh>

# include "kernel/fwd.hh"
# include "kernel/ustring.hh"
# include "kernel/utypes.hh"

# include "runner/fwd.hh"

# include "scheduler/fwd.hh"

extern const char* EXTERNAL_MESSAGE_TAG;
extern const char* UNKNOWN_TAG;

extern const char* DISPLAY_FORMAT;
extern const char* DISPLAY_FORMAT1;
extern const char* DISPLAY_FORMAT2;

extern const char* MAINDEVICE;

/// Global variable for the server
extern class UServer* urbiserver;


//! UServer class: handles all URBI system processing.
/*! There must be one UServer defined in the program and it must be overloaded
    to make it specific to the particular robot.

    UServer is used to store the UConnection list and the UDevice list.
    This object does all the internal processing of URBI and handles the pool
    of UCommand's.
*/
class UServer
{
public:
  //! UServer constructor.
  /*! UServer constructor

   Unlike UConstructor, it is not required that you handle the memory
   management task when you create the robot-specific sub class. The
   difference in memory between your class and the UServer class is
   considered as neglectible and included in the security margin. If you
   don't understand this point, ignore it.

   \param period gives the value in msec of the server update,
   which are the calls to the "work" function. These calls must be done at
   a fixed, precise, real-time period to let the server computer motor
   trajectories between two "work" calls.
   */
  UServer(ufloat period, const char* mainName);

  virtual ~UServer ();

public:
  //! Initialization of the server. Displays the header message & init stuff
  /*! This function must be called once the server is operational and
   able to print messages. It is a requirement for URBI compliance to print
   the header at start, so this function *must* be called. Beside, it also
   do initalization work for the devices and system variables.
   */
  void initialize ();

  //! Main processing loop of the server
  /*! This function must be called every "period_" msec to ensure the proper
   functionning of the server. It will call the command execution, the
   connection message sending when they are delayed, etc...

   "period_" is a parameter of the server, given in the constructor.
   */
 libport::utime_t work ();

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
   \param args Arguments for the format string.
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

  virtual libport::utime_t getTime () = 0;
  virtual ufloat getPower () = 0;

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

  /// Path to explore when looking for .u files.
  libport::file_library search_path;

  /// Return the full file name, handle paths.
  /// Return \a f on failure.
  virtual std::string find_file (const libport::path& path);

  /// Type of UCommandQueue
  enum QueueType {
    /// The UComandQueue contains URBI code.
    QUEUE_URBI,
    /// THe UCommandQueu contains data, not to be messed with.
    QUEUE_DATA
  };

  /// Load a file into the connection.
  /// Returns UFAIL if anything goes wrong, USUCCESS otherwise.
  virtual UErrorValue loadFile (const std::string& file_name,
				UQueue& loadQueue,
				QueueType t=QUEUE_URBI);

  /// Load \a fn in the ghostqueue.
  UErrorValue load_init_file(const char* fn);

  /// Save content to a file
  /// This function must be redefined by the robot-specific server.
  /// Returns UFAIL if anything goes wrong, USUCCESS otherwise.
  virtual UErrorValue saveFile (const std::string& filename,
				const std::string& content) = 0;

  //! Accessor for period_.
  ufloat period_get();
  void mark (UString* stopTag);
  virtual void reboot () = 0;
  virtual void shutdown () = 0;
  //! Function called before work
  /*! Redefine this virtual function if you need to do pre-processing before
    the work function starts.
  */
  virtual void beforeWork ();
  //! Function called after work
  /*! Redefine this virtual function if you need to do post-processing
    before the work function ends.
  */
  virtual void afterWork ();

  /// Display a message on the robot console.
  void display (const char*);
  /// Display a set of messages on the robot console.
  void display (const char**);

  //! Accessor for lastTime_.
  libport::utime_t lastTime ();
  //! Update lastTime_ to current time.
  void updateTime ();
  void addConnection (UConnection& connection);
  /// Overload to support the legacy interface of k1.
  void addConnection (UConnection* connection);
  void removeConnection (UConnection& connection);
  int getUID ();

  // A usual connection to stop dependencies.
  UConnection& getGhostConnection ();

  void hasSomethingToDelete ();

  const scheduler::Scheduler& getScheduler () const;
  scheduler::Scheduler& getScheduler ();

  runner::Runner& getCurrentRunner () const;

protected:
  virtual void effectiveDisplay (const char*) = 0;

private:
  // Pointer to stop the header dependency.
  scheduler::Scheduler* scheduler_;

public: // FIXME remove from the public section.
  /// List of active connections: includes one UGhostConnection.
  // FIXME: This is meant to become a runner::Job and move out of this class.
  std::list<UConnection*> connectionList;

private:
  /// \{ Various parts of @c UServer::work.
  /// Scan currently opened connections for ongoing work
  void work_handle_connections_ ();
  /// Scan currently opened connections for deleting marked commands or
  /// killall order
  void work_handle_stopall_ ();
  void work_test_cpuoverload_ ();
  /// \}

public:
  /// Shows debug or not.
  bool debugOutput;

# if ! defined LIBPORT_URBI_ENV_AIBO
  /// Used to synchronize message reception.
  boost::recursive_mutex mutex;
# endif

private:
  /// Name of the main device.
  std::string mainName_;

public:
  /// True after a stop command.
  bool somethingToDelete;
  /// True after the initialization phase: all vars are uservar then.
  bool uservarState;

public:
  /// Stops all commands in all connections.
  bool stopall;

private:

  enum
  {
    /// Urbi TCP Port..
    TCP_PORT = 54000,
    /// Used by echo() & error().
    // FIXME: Because of this stupid hard limit, we can't produce
    // large outputs!  We should move to using C++.  Or some scheme
    // that is robust to the size of the message.
    MAXSIZE_INTERNALMESSAGE = 8192,
  };

  /// Frequency of the calls to work().
  ufloat period_;

  /// Store the time on the last call to updateTime();.
  libport::utime_t lastTime_;
  /// The ghost connection used for URBI.INI.
  UGhostConnection* ghost_;

  /// unique id source
  int               uid;
};

/// Unique identifier to create new references.
inline int unique ();

/// Return an identifier starting with \a prefix, ending with a unique int.
inline std::string unique (const std::string& prefix);

/*-------------------------.
| Freestanding functions.  |
`-------------------------*/

/// Send debugging messages via ::urbiserver.
void vdebug (const char* fmt, va_list args)
  __attribute__ ((__format__ (__printf__, 1, 0)));

/// Send debugging messages via ::urbiserver.
void debug (const char* fmt, ...)
  __attribute__ ((__format__ (__printf__, 1, 2)));

/// Send debugging messages indented with \a t spaces, via ::urbiserver.
void debug (unsigned t, const char* fmt, ...)
  __attribute__ ((__format__ (__printf__, 2, 3)));

// Send debugging messages.
# if URBI_DEBUG
// Must be invoked with two pairs of parens.
#  define DEBUG(Msg)  debug Msg
# else
#  define DEBUG(Msg) ((void) 0)
# endif

# include <kernel/userver.hxx>

#endif // !KERNEL_USERVER_HH
