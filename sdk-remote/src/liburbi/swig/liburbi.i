// -*- C++ -*-

%module liburbi
%{
#include "uclient.h"

#ifdef SWIGPYTHON
# include "swig_python.h"
#endif /* !SWIGPYTHON */

#ifdef SWIGRUBY
# include "swig_ruby.h"
#endif /* !SWIGRUBY */

%}

namespace urbi
{
  typedef unsigned int UCallbackID;

  class UAbstractClient;
  class UCallback;
  class UCustomCallback;
  class UMessage;
  class UCallbackAction;
  class UCallbackWrapper;

  enum UMessageType {
    MESSAGE_SYSTEM,
    MESSAGE_ERROR,
    MESSAGE_DATA
  };

  class UMessage {
  public:

    /// Connection from which originated the message.
    //UAbstractClient    &client;
    /// Server-side timestamp.
    int                timestamp;
    /// Associated tag.
    std::string        tag;

    UMessageType       type;

    urbi::UValue       *value;
    std::string        message;
    /// Raw message without the binary data.
    std::string        rawMessage;

    /// Parser constructor
    UMessage(UAbstractClient & client, int timestamp,  const char *tag, const char *message, std::list<urbi::BinaryData> bins);

    /// If alocate is true, everything is copied, eles pointers are stolen
    UMessage(const UMessage &source);

    /// Free everything if data was copied, doesn't free anything otherwise
    ~UMessage();

    operator urbi::UValue& () {return *value;}
  };

  %extend UMessage
  {
    UMessage() {}
  }

};

#ifdef SWIGPYTHON
%include "swig_python.h"
#endif /* !SWIGPYTHON */

#ifdef SWIGRUBY
%include "swig_ruby.h"
#endif /* !SWIGRUBY */

namespace urbi
{
  class UAbstractClient : public std::ostream
  {
  public:
    /// Create a new instance and connect to the Urbi server.
    UAbstractClient(const char *_host,
		    int _port = URBI_PORT,
		    int _buflen = URBI_BUFLEN);

    virtual ~UAbstractClient();

    /// Return current error status, or zero if no error occurred.
    int error() {return(rc);};


    // Sending

    /// Function for backward compatibility. Will be removed in future versions.
    int send() { endPack(); return 0;}

    /// Send an Urbi command. The syntax is similar to the printf() function.
    int send(const char* format,...);

    ///send the value without any prefix or terminator
    int send(urbi::UValue& v);

    /// Send binary data.
    int sendBin(const void*, int len);

    /// Send an Urbi header followed by binary data.
    int sendBin(const void*, int len,const char * header,...);

    /// Lock the send buffer (for backward compatibility, will be removed in future versions).
    int startPack();

    /// Unlock the send buffer (for backward compatibility, will be removed in future versions).
    int endPack();

    /// Append Urbi commands to the send buffer (for backward compatibility, will be removed in future versions).
    int pack(const char*,...);

    /// va_list version of pack.
    int vpack(const char*,va_list args);

    /// Send urbi commands contained in a file.
    int sendFile(const char * filename);

    /// Send a command, prefixing it with a tag, and associate the given callback with this tag. */
    UCallbackID sendCommand(UCallback ,const char*,...);

    /// Send a command, prefixing it with a tag, and associate the given callback with this tag. */
    UCallbackID sendCommand(UCustomCallback ,void *,const char*,...);

    /// Send sound data to the robot for immediate playback.
    int sendSound(const char * device,
		  const urbi::USound &sound, const char * tag=0);

    /// Put a file on the robot's mass storage device.
    int putFile(const char * localName, const char * remoteName=0);

    /// Save a buffer to a file on the robot.
    int putFile(const void * buffer, int length, const char * remoteName);


    // Receiving

    /// Associate a callback function with a tag. new style
    UCallbackID setCallback(UCallbackWrapper & callback, const char * tag);

    /// Associate a callbaxk function with all error messages
    UCallbackID setErrorCallback(UCallbackWrapper & callback);

    /// Associate a callback with all messages
    UCallbackID setWildcardCallback(UCallbackWrapper & callback);

    /// OLD-style callbacks
    UCallbackID setCallback(UCallback ,const char* tag);

    /// Associate a callback function with a tag, specifiing a callback custom value that will be passed back to the callback function.
    UCallbackID setCallback(UCustomCallback ,void * callbackData,const char* tag);

    /// Callback to class member functions(old-style).
    template<class C>                                         UCallbackID setCallback(C& ref,
										      UCallbackAction (C::*)(                 const UMessage &),                 const char * tag);
    template<class C, class P1>                               UCallbackID setCallback(C& ref,
										      UCallbackAction (C::*)(P1 ,             const UMessage &), P1,             const char * tag);
    template<class C, class P1, class P2>                     UCallbackID setCallback(C& ref,
										      UCallbackAction (C::*)(P1 , P2,         const UMessage &), P1, P2,         const char * tag);
    template<class C, class P1, class P2, class P3>           UCallbackID setCallback(C& ref,
										      UCallbackAction (C::*)(P1 , P2, P3,     const UMessage &), P1, P2, P3,     const char * tag);
    template<class C, class P1, class P2, class P3, class P4> UCallbackID setCallback(C& ref,
										      UCallbackAction (C::*)(P1 , P2, P3, P4, const UMessage &), P1, P2, P3, P4, const char * tag);


    /// Get the tag associated with a registered callback.
    int getAssociatedTag(UCallbackID id, char * tag);

    /// Delete a callback.
    int deleteCallback(UCallbackID callBackID);

    /// Fill tag with a unique tag for this client.
    void makeUniqueTag(char * tag);

    /// Simulate an Urbi message.
    virtual void notifyCallbacks(const UMessage &msg);

    /// Notify of an error.
    virtual void printf(const char * format, ...)=0;

    /// Get time in milliseconds since an unspecified but constant reference time.
    virtual unsigned int getCurrentTime()=0;

    /// Return the server name or IP address.
    const char * getServerName() {return host;}

    /// Called each time new data is available in recvBuffer.
    void processRecvBuffer();

    std::ostream & getStream() { return *stream;}
  };

  %extend UAbstractClient
  {
#ifdef SWIGPYTHON
    UCallbackID setCallback(PyObject* obj, const char* tag)
      {
	if (!obj || !PyFunction_Check(obj) || !PyCallable_Check(obj))
	  return 0;
	return (*self).setCallback(*new urbi::Callback(obj),tag);
      }

    UCallbackID setErrorCallback(PyObject* obj)
      {
	if (!obj || !PyFunction_Check(obj) || !PyCallable_Check(obj))
	  return 0;
	return (*self).setErrorCallback(*new urbi::Callback(obj));
      }

    UCallbackID setWildcardCallback(PyObject* obj)
      {
	if (!obj || !PyFunction_Check(obj) || !PyCallable_Check(obj))
	  return 0;
	return (*self).setWildcardCallback(*new urbi::Callback(obj));
      }
#endif /* !SWIGPYTHON */

#ifdef SWIGRUBY
    UCallbackID setWildcardCallback(UCallbackWrapper & callback);

    UCallbackID setCallback(VALUE tag, VALUE proc)
      {
	Check_Type(tag, T_STRING);
	Check_Type(proc, T_DATA);

	return (*self).setCallback(*new urbi::Callback(proc),
				   StringValuePtr(tag));
      }

    UCallbackID setWildcardCallback(VALUE dummy, VALUE proc)
      {
	Check_Type(proc, T_DATA);
	return (*self).setWildcardCallback(*new urbi::Callback(proc));
      }
#endif /* !SWIGRUBY */

  };

  class UClient: public UAbstractClient
  {
  public:
    UClient(const char *_host,int _port = URBI_PORT,int _buflen = URBI_BUFLEN);

  virtual ~UClient();

    //! For compatibility with older versions of the library
    void start() {}

    //! For internal use.
    void listenThread();

    virtual void printf(const char * format, ...);
    virtual unsigned int getCurrentTime();
  };
};
