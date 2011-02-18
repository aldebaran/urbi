/*
 * Copyright (C) 2004, 2006-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/usyncclient.hh
/// \brief Definition of the URBI interface class


#ifndef URBI_USYNCCLIENT_HH
# define URBI_USYNCCLIENT_HH

# include <libport/finally.hh>
# include <libport/fwd.hh>
# include <libport/lockable.hh>
# include <libport/semaphore.hh>
# include <libport/utime.hh>
# include <libport/pthread.h>

# include <urbi/uclient.hh>

namespace urbi
{
  /*! Format in which image requested with syncGetSound are transmitted*/
  enum UTransmitFormat
  {
    /// Transmit images compressed in JPEG.
    URBI_TRANSMIT_JPEG,
    /// Transmit raw YCbCr images.
    URBI_TRANSMIT_YCbCr
  };

  /// UClient linux implementation with support for synchronous extra
  /// functions.
  /*! This class provides extra functions to synchronously request
     values. These functions can safely be called frow within a
     callback function.

     All callbacks will be called in a separate thread created in the
     constructor.
     When one of those callbacks calls a synchronous function, new incoming
     messages are kept on hold until the response from the synchronous call
     is received.

     If you want to call these callbacks in a different
     thread, call stopCallbackThread(), then regularly call
     processEvents(). Each call will call callbacks for all pending
     messages in the current thread.  */
  class URBI_SDK_API USyncClient: public UClient
  {
  public:
    typedef boost::function1<void, USyncClient*> connect_callback_type;
    typedef UClient super_type;
    typedef super_type::error_type error_type;

    /// Option structure for USyncClient construction.
    ///
    /// In addition to UClient::options, provides:
    /// - startCallbackThread
    /// - connectCallback.
    struct URBI_SDK_API options
      : public super_type::options
    {
      options();
      /// If true, create a thread dedicated to the processing of
      /// incoming messages. If false, it is the responsibility of the
      /// user to regularly call processEvents().  Defaults to true.
      UCLIENT_OPTION(bool, startCallbackThread);
      /// Ignore host and port if set, do not connect or listen.
      /// Called when connection is established.
      UCLIENT_OPTION(connect_callback_type, connectCallback);
    };

    /** Create a new connection to an Urbi Server. Blocks until the connection
     * is established.
     *
     *  \param host    The host to connect to.
     *  \param port    the port number to connect to, defaults to URBI_PORT.
     *  \param buflen  Size of reception buffer, defaults to URBI_BUFLEN.
     *  \param opt     Parameters of this USyncClient.
     */
    USyncClient(const std::string& host,
		unsigned port = URBI_PORT,
		size_t buflen = URBI_BUFLEN,
                const USyncClient::options& opt = options());

    ~USyncClient();

    /// Options for send(), rather than multiplying the overloads.
    struct send_options
    {
      send_options();

      send_options& timeout(libport::utime_t);
      send_options& tag(const char*, const char* = 0);

      /// Timeout in microseconds
      libport::utime_t timeout_;
      const char* mtag_;
      const char* mmod_;
      static const send_options default_options;
    };

  protected:
    virtual error_type onClose ();

  protected:
    /** Synchronously ask the server for the value of an expression.
     * \param expression
     *   the Urbi expression to evaluate.
     *   It must be a single expression and must not start with a tag.
     *   It's a printf-like format string.
     * \param arg
     *   the arguments for the expression
     * \param options
     *   what tag to use and so forth.
     * \return
     *   the resulting message, or 0 in case of error.
     */
    UMessage*
    syncGet_(const char* expression, va_list& arg,
	     const send_options& options = send_options::default_options);

  public:
    /// Synchronously evaluate an Urbi expression. The expression must
    /// not start with a tag or channel.
    ATTRIBUTE_PRINTF(2, 3)
    UMessage* syncGet(const char* expression, ...);

    /// Synchronously evaluate an Urbi expression. The expression must
    /// not start with a tag or channel.
    UMessage* syncGet(const std::string& exp);

    /// Likewise, with a timeout.
    ATTRIBUTE_PRINTF(3, 4)
    UMessage* syncGet(libport::utime_t useconds,
                      const char* expression, ...);
    /// Synchronously evaluate an Urbi expression, specifying the tag
    /// and modifiers to prepend to it.
    ATTRIBUTE_PRINTF(2, 5)
    UMessage* syncGetTag(const char* expression,
                         const char* mtag, const char* mmod, ...);
    /// Likewise, with timeout.
    ATTRIBUTE_PRINTF(3, 6)
    UMessage* syncGetTag(libport::utime_t useconds,
                         const char* expression,
                         const char* mtag, const char* mmod, ...);

    /// Send given buffer without copying it.
    int syncSend(const void* buffer, size_t length);

    /// Get an image in a synchronous way.
    /// \return 1 on success, 0 on failure.
    int syncGetImage(const char* cameraDevice, void* buffer,
                     size_t& buffersize,
		     int format, int transmitFormat,
                     size_t& width, size_t& height,
                     libport::utime_t useconds = 0);

    /// Get the value of any device in a synchronous way.
    /// \return 1 on success, 0 on failure.
    int syncGetValue(const char* valName, UValue& val,
		     libport::utime_t useconds = 0);
    int syncGetValue(const char* tag, const char* valName, UValue& val,
		     libport::utime_t useconds = 0);

    /// Get the value of device.val in a synchronous way.
    /// \return 1 on success, 0 on failure.
    int syncGetDevice(const char* device, ufloat &val,
		      libport::utime_t useconds = 0);

    /// Execute an URBI command, return the resulting double
    /// value.
    /// \return 1 on success, 0 on failure.
    int syncGetResult(const char* command, ufloat &val,
		      libport::utime_t useconds = 0);

    /// Get the normalized value of a device in a synchronous
    /// way.
    /// \return 1 on success, 0 on failure.
    int syncGetNormalizedDevice(const char* device, ufloat &val,
				libport::utime_t useconds = 0);

    /// Get a field of a device in a synchronous way.
    /// \return 1 on success, 0 on failure.
    int syncGetDevice(const char* device, const char* field, ufloat &val,
		      libport::utime_t useconds = 0);

    /// Get sound for duration milliseconds in buffer.
    int syncGetSound(const char* device, int duration, USound &sound,
		     libport::utime_t useconds = 0);

    /// Wait until a message with specified tag is received. Returned
    /// message must be deleted.
    UMessage* waitForTag(const std::string& tag, libport::utime_t useconds = 0);

    /// Must be called once before sending message associated with waitForTag
    void lockQueue();

    /// Overriding UAbstractclient implementation
    virtual void notifyCallbacks(const UMessage &msg);

    /**
     * Check message queue for pending messages, notify callbacks synchronously.
     * @param timeout If different -1 process events for at most @a timeout
     *                microseconds. This is useful if you don't want
     *                processEvents() to take to much time if there are many
     *                many pending messages.
     * @return true if at least one message was processed, false otherwise.
     * Callbacks functions are called synchronously in the caller thread.
     */
    bool processEvents(libport::utime_t timeout = -1);

    /**
     *  Stop the callback processing thread.
     *  The user is responsible for calling processEvents() regularily
     *  once this function has been called.
     */
    void stopCallbackThread();
    void callbackThread();

    /** Enable synchronous transmission of messages to the callbacks.
     *  When enabled, the message processing thread is bypassed, and callbacks
     *  are called synchronously by the network thread.
     *  The result is a lower latency, but some configurations of synchronous
     *  calls will not work anymore.
     */
    void setSynchronous(bool enable);
    /**
     * Block until kernel version is available, or an error occurrs.
     * @param hasProcessingThread true if a processing thread is running, false
     * if processEvents must be called while waiting.
     */
    using super_type::waitForKernelVersion;
    void waitForKernelVersion(bool hasProcessingThread);

    void setDefaultOptions(const USyncClient::send_options& opt);
    const USyncClient::send_options&
      getOptions(const USyncClient::send_options& opt =
                 USyncClient::send_options::default_options) const;

    /**
     * Listen on the specified host:port pair. Bind an USyncClient on each
     * connection, and call \b connectCallback. Socket ownership is transfered
     * to the callback.
     * @return a handle on the listening socket. Destroy it to stop listening.
     */
    static boost::shared_ptr<libport::Finally>
    listen(const std::string& host, const std::string& port,
           boost::system::error_code& erc,
           connect_callback_type connectCallback,
           size_t buflen = URBI_BUFLEN,
           bool startCallbackThread = true);

    virtual void onConnect();

    /// @return true if the current thread is the callback thread.
    bool isCallbackThread() const;
  protected:
    int joinCallbackThread_();

    static libport::Socket* onAccept(connect_callback_type l, size_t buflen,
                                     bool startThread);
    // Incremented at each queue push, decremented on pop.
    libport::Semaphore sem_;
    // Semaphore to delay execution of callback thread until ctor finishes.
    libport::Semaphore callbackSem_;

    // The list of incoming messages waiting to be processed.
    std::list<UMessage*> queue;
    libport::Lockable queueLock_;

    /// When locked waiting for a specific tag, notifyCallbacks will
    /// store the received message here, and waitForTag will get it
    /// there.
    UMessage* message_;
    libport::Semaphore syncLock_;
    std::string syncTag;

    send_options default_options_;

    bool stopCallbackThread_;
    pthread_t cbThread;
    // Used to block until the callback thread is realy stopped.
    libport::Semaphore stopCallbackSem_;
    connect_callback_type connectCallback_;
    // True if waitForTag is waiting from the polling thread.
    bool waitingFromPollThread_;
    // If set, bypass callback thread and send messages synchronously.
    bool synchronous_;
  };

} // namespace urbi

#endif // ! URBI_USYNCCLIENT_HH
