/*! \file urbi/usyncclient.hh
****************************************************************************
 *
 * Definition of the URBI interface class
 *
 * Copyright (C) 2004, 2006, 2007, 2008 Jean-Christophe Baillie.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**********************************************************************/

#ifndef URBI_USYNCCLIENT_HH
# define URBI_USYNCCLIENT_HH

# include <libport/fwd.hh>
# include <libport/lockable.hh>
# include <libport/semaphore.hh>
# include <libport/utime.hh>

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

     All callback will be called in a separate thread created in the
     constructor.  If you want to call these callbacks in a different
     thread, call @stopCallbackThread, then regularly call
     @processEvents. Each call will call callbacks for all pending
     messages in the current thread.  */
  class URBI_SDK_API USyncClient: public UClient
  {
  public:
    /** Create a new connection to an Urbi Server.
     *
     *  \param host    The host to connect to.
     *  \param port    the port number to connect to, defaults to URBI_PORT.
     *  \param buflen  Size of reception buffer, defaults to URBI_BUFLEN.
     *  \param server  If true, listen for an incoming connection from an
     *                  urbi server instead of connecting.
     *  \param startCallbackThread Create a thread didacated to the processing
     *    of incoming messages. If false, it is the responsibility of the user
     *    to regularly call processEvents().
     */
    USyncClient(const std::string& host,
		int port = URBI_PORT,
		int buflen = URBI_BUFLEN,
		bool server = false,
		bool startCallbackThread = true,
		int semListenInc = 2);

    ~USyncClient ();

  protected:
    /** Synchronously ask the server for the value of an expression.
     * \param expression the URBI expression to evaluate.
     *         It must be a single expression and must not start with a tag.
     * \param mtag tag to use, or 0 to generate one.
     * \param mmod modifier to use on the tag, or 0 for none.
     * \return the resulting message, or 0 in case of error.
     */
    UMessage*
    syncGet_ (const char* expression, const char* mtag,
              const char* mmod, va_list& arg);

  public:
    /// Synchronously evaluate an Urbi expression. The expression must
    /// not start with a tag or channel.
    UMessage *syncGet (const char* expression, ...);
    /// Synchronously evaluate an Urbi expression, specifying the tag
    /// and modifiers to prepend to it.
    UMessage *syncGetTag (const char* expression,
                          const char* mtag, const char* mmod, ...);



    /// Send given buffer without copying it.
    int syncSend(const void * buffer, int length);

    /// Get an image in a synchronous way. Returns 1 on success, 0 on failure.
    int syncGetImage(const char* cameraDevice, void* buffer, int &buffersize,
		     int format, int transmitFormat, int &width, int &height);

    /// Get the value of any device in a synchronous way. Returns 1 on
    /// success, 0 on failure.
    int syncGetValue(const char* valName, UValue& val);
    int syncGetValue(const char* tag, const char* valName, UValue& val);

    /// Get the value of device.val in a synchronous way. Returns 1 on
    /// success, 0 on failure.
    int syncGetDevice(const char* device, double &val);

    /// Execute an URBI command, return the resulting double
    /// value. Returns 1 on success, 0 on failure.
    int syncGetResult(const char* command, double &val);

    /// Get the normalized value of a device in a synchronous
    /// way. Returns 1 on success, 0 on failure.
    int syncGetNormalizedDevice(const char* device, double &val);

    /// Get a field of a device in a synchronous way. Returns 1 on
    /// success, 0 on failure.
    int syncGetDevice(const char* device, const char * field, double &val);

    /// Get sound for duration milliseconds in buffer.
    int syncGetSound(const char * device, int duration, USound &sound);

    /// Wait until a message with specified tag is received. Returned
    /// message must be deleted.
    UMessage * waitForTag(const char * tag);
    /// Overriding UAbstractclient implementation
    virtual void notifyCallbacks(const UMessage &msg);

    /**
     * Check message queue for pending messages, notify callbacks synchronously.
     * @param timeout If different -1 process events for at most @a timeout
     *                microseconds. This is useful if you don't want
     *                processEvents() to take to much time if there are many
     *                many pending messages.
     * Callbacks functions are called synchronously in the caller thread.
     */
    void processEvents(const libport::utime_t timeout = -1);

    /**
     *  Stop the callback processing thread.
     *  The user is responsible for calling processEvents() regularily
     *  once this function has been called.
     */
    void stopCallbackThread();
    void callbackThread();

  protected:
    int joinCallbackThread_ ();

    // Incremented at each queue push, decremented on pop.
    libport::Semaphore sem_;
    // Semaphore to delay execution of callback thread until ctor finishes.
    libport::Semaphore callbackSem_;
    std::list<UMessage*> queue;
    libport::Lockable queueLock_;
    UMessage* msg;
    libport::Semaphore syncLock_;
    std::string syncTag;

    bool stopCallbackThread_;
    void* cbThread;
    // Used to block until the callback thread is realy stopped.
    libport::Semaphore stopCallbackSem_;
  };

} // namespace urbi

#endif // ! URBI_USYNCCLIENT_HH
