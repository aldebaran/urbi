/// \file kernel/userver.hh
/// \brief Definition of the UServer class.

#ifndef KERNEL_USERVER_HH
# define KERNEL_USERVER_HH

# include <cstdarg>
# include <sstream>

# include <boost/ptr_container/ptr_list.hpp>

# include <libport/config.h>
# if ! defined LIBPORT_URBI_ENV_AIBO
#  include <boost/thread.hpp>
# endif

# include <libport/fwd.hh>
# include <libport/compiler.hh>
# include <libport/file-library.hh>
# include <libport/ufloat.h>
# include <libport/utime.hh>

# include <kernel/fwd.hh>
# include <kernel/utypes.hh>

// Do not include runner/fwd.hh etc. which are not public.
namespace runner    { class Runner; }
namespace scheduler { class Scheduler; }

extern const char* DISPLAY_FORMAT;
extern const char* DISPLAY_FORMAT1;
extern const char* DISPLAY_FORMAT2;


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
  UServer(const char* mainName);
  virtual ~UServer ();

public:
  //! Initialization of the server. Displays the header message & init stuff
  /*! This function must be called once the server is operational and
   able to print messages. It is a requirement for URBI compliance to print
   the header at start, so this function *must* be called. Beside, it also
   do initalization work for the devices and system variables.
   */
  void initialize ();

  /// Process the jobs.
  /// \return the time when should be called again.
  libport::utime_t work ();

  /// Set the system.args list in URBI.
  void main (int argc, const char* argv[]);


  /// Package information about this server.
  static const libport::PackageInfo& package_info ();

  //! Displays a formatted error message.
  /*! This function uses the virtual URobot::display() function to make the
   message printing robot-specific.

   It formats the output in a standard URBI way by adding an ERROR key
   between brackets at the end.
   */
  void error (const char* s, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));

  //! Displays a formatted message.
  /*! This function uses the virtual URobot::display() function to make the
   message printing robot-specific.

   It formats the output in a standard URBI way by adding an empty key
   between brackets at the end. If you want to specify a key, use the
   echoKey() function.
   \param s is the formatted string containing the message.
   \sa echoKey()
   */
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
  /*! This function uses the virtual URobot::display() function to make the
   message printing robot-specific.

   \param s is the formatted string containing the message
   \param args Arguments for the format string.
   */
  void vdebug (const char* s, va_list args)
    __attribute__ ((__format__ (__printf__, 2, 0)));
  void debug (const char* s, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));

  //! Overload this function to return the running time of the server.
  /*! The running time of the server must be in milliseconds.
   */
  virtual libport::utime_t getTime () = 0;

  //! Overload this function to return the remaining power of the robot
  /*! The remaining power is expressed as percentage. 0 for empty batteries
   and 1 for full power.
   */
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
  virtual UErrorValue load_file(const std::string& file_name,
                                UQueue& loadQueue,
                                QueueType t = QUEUE_URBI);

  /// Load \a fn in the ghostqueue.
  UErrorValue load_init_file(const char* fn);

  /// Save content to a file
  /// This function must be redefined by the robot-specific server.
  /// Returns UFAIL if anything goes wrong, USUCCESS otherwise.
  virtual UErrorValue save_file(const std::string& filename,
                                const std::string& content) = 0;

  //! Overload this function to specify how your system will reboot
  virtual void reboot () = 0;

  //! Overload this function to specify how your system will shutdown
  virtual void shutdown ();

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
  //! Update the server's time using the robot-specific implementation
  /*! It is necessary to have an update of the server time to
   increase the performance of successive calls to getTime.
   It allows also to see a whole processing session (like the
   processing of the command tree) as occuring AT the same time,
   from the server's point of view.
   */
  void updateTime ();


  /*--------------.
  | Connections.  |
  `--------------*/

  /// Add a new connection to the connection list.
  /// Take ownership on c. Also perform some error testing on the connection
  /// value and UError return code
  /// \precondition c != 0
  void connection_add(UConnection* c);

  /// Remove from the connection list.
  /// This function perform also some error testing on the connection
  /// value and UError return code
  /// Destroy \a c.
  void connection_remove(UConnection* c);

  // A usual connection to stop dependencies.
  UConnection& getGhostConnection();


  /*--------------------.
  | Scheduler, runner.  |
  `--------------------*/
public:
  const scheduler::Scheduler& getScheduler () const;
  scheduler::Scheduler& getScheduler ();

  runner::Runner& getCurrentRunner () const;

protected:
  //! Overload this function to specify how your robot is displaying messages.
  virtual void effectiveDisplay (const char*) = 0;

private:
  // Pointer to stop the header dependency.
  scheduler::Scheduler* scheduler_;


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
  /// Stops all commands in all connections.
  bool stopall;

  enum
  {
    /// Urbi TCP Port.
    TCP_PORT = 54000,
  };

private:
  /// Store the time on the last call to updateTime().
  libport::utime_t lastTime_;

  /// List of active connections: includes one UGhostConnection.
  // FIXME: This is meant to become a runner::Job and move out of this class.
  boost::ptr_list<UConnection> connections_;

  /// The ghost connection used for URBI.INI.
  UGhostConnection* ghost_;
};

/*-------------------------.
| Freestanding functions.  |
`-------------------------*/

/// Send debugging messages via ::urbiserver.
void vdebug (const char* fmt, va_list args)
  __attribute__ ((__format__ (__printf__, 1, 0)));

/// Send debugging messages via ::urbiserver.
void debug (const char* fmt, ...)
  __attribute__ ((__format__ (__printf__, 1, 2)));

// Send debugging messages.
# if URBI_DEBUG
// Must be invoked with two pairs of parens.
#  define DEBUG(Msg)  debug Msg
# else
#  define DEBUG(Msg) ((void) 0)
# endif

# include <kernel/userver.hxx>

#endif // !KERNEL_USERVER_HH
