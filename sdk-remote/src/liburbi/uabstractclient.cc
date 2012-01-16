/*
 * Copyright (C) 2005-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file liburbi/uabstractclient.cc

#include <algorithm>
#include <libport/cassert>
#include <libport/cerrno>
#include <libport/cmath>
#include <libport/cstdlib>
#include <fstream>
#include <iostream>

#include <libport/format.hh>
#include <libport/io-stream.hh>
#include <libport/lexical-cast.hh>

#include <libport/cstdio>
#include <libport/cstring>
#include <libport/containers.hh>
#include <libport/debug.hh>
#include <libport/escape.hh>
#include <libport/lexical-cast.hh>
#include <libport/lockable.hh>
#include <libport/sys/stat.h>
#include <libport/unistd.h>
#include <libport/windows.hh>

#include <urbi/uabstractclient.hh>
#include <urbi/uconversion.hh>
#include <urbi/umessage.hh>
#include <urbi/utag.hh>

#include <liburbi/compatibility.hh>

GD_CATEGORY(Urbi.Client.Abstract);

namespace urbi
{

  /// Fake tag to treat error message with tags.
  const char* tag_error = "[error]";
  /// Fake tag to catch all the messages.
  const char* tag_wildcard = "[wildcard]";

  std::ostream&
  default_stream()
  {
    return (getDefaultClient()
	    ? ((UAbstractClient*)getDefaultClient())->stream_get()
	    : std::cerr);
  }


  /*-------------.
  | UCallbacks.  |
  `-------------*/

  static UCallbackID nextId;

  class UCallbackWrapperCB: public UCallbackWrapper
  {
    UCallback cb;
  public:
    UCallbackWrapperCB(UCallback cb)
      : cb(cb)
    {
    }
    virtual UCallbackAction operator()(const UMessage& msg)
    {
      return cb(msg);
    }
  };


  class UCallbackWrapperCCB: public UCallbackWrapper
  {
    UCustomCallback cb;
    void * data;
  public:
    UCallbackWrapperCCB(UCustomCallback cb, void* data)
      : cb(cb)
      , data(data)
    {
    }
    virtual UCallbackAction operator()(const UMessage& msg)
    {
      return cb(data, msg);
    }
  };


  /*-------------------.
  | UClientStreambuf.  |
  `-------------------*/

  class UClientStreambuf: public libport::StreamBuffer
  {
  public:
    UClientStreambuf(UAbstractClient* cl)
      : client_(cl)
    {}

  protected:
     virtual size_t read(char* buffer, size_t size);
     virtual void write(char* buffer, size_t size);

  private:
    UAbstractClient* client_;
  };

  void
  UClientStreambuf::write(char* buffer, size_t size)
  {
    client_->effective_send(buffer, size);
  }

  size_t
  UClientStreambuf::read(char*, size_t)
  {
    return 0;
  }

  /*------------------.
  | UAbstractClient.  |
  `------------------*/


  const char* UAbstractClient::CLIENTERROR_TAG = "client_error";

  void
  UAbstractClient::bins_clear()
  {
    for (/* nothing */; !bins.empty(); bins.pop_front())
      bins.front().clear();
  }

  inline
  bool
  matching_tag(const UMessage& msg, const char* tag)
  {
    return
      msg.tag == tag
      || (libport::streq(tag, tag_error) && msg.type == MESSAGE_ERROR)
      // The wild card does not match tags starting with
      // TAG_PRIVATE_PREFIX.
      || (libport::streq(tag, tag_wildcard)
          && msg.tag.compare(0,
                             sizeof TAG_PRIVATE_PREFIX - 1,
                             TAG_PRIVATE_PREFIX));
  }

  void
  UAbstractClient::notifyCallbacks(const UMessage& msg)
  {
    libport::BlockLock bl(listLock);
    bool inc = true;
    for (callbacks_type::iterator it = callbacks_.begin();
         it != callbacks_.end();
         inc ? it++ : it, inc = true)
      if (matching_tag(msg, it->tag))
      {
        UCallbackAction ua = it->callback(msg);
        if (ua == URBI_REMOVE)
        {
          delete &it->callback;
          it = callbacks_.erase(it);
          inc = false;
        }
      }
  }

  UAbstractClient::UAbstractClient(const std::string& host,
				   unsigned port,
				   size_t buflen,
				   bool server)
    : LockableOstream(new UClientStreambuf(this))
    , closed_ (false)
    , listLock()
    , host_(host)
    , port_(port)
    , server_(server)
    , sendBufSize(buflen)
    , recvBufSize(buflen)
    , rc(0)

    , recvBuffer(new char[buflen])
    , recvBufferPosition(0)
    , sendBuffer(new char[buflen])

    , kernelMajor_(-1)
    , kernelMinor_(-1)
    , binaryBuffer(0)
    , parsePosition(0)
    , inString(false)
    , nBracket(0)
    , binaryMode(false)
    , system(false)
    , init_(true)
    , counter_(0)
    , stream_(this)
  {
    exceptions(std::ostream::eofbit | std::ostream::failbit |
               std::ostream::badbit);
    recvBuffer[0] = 0;
    sendBuffer[0] = 0;
  }

  UAbstractClient::~UAbstractClient()
  {
    // No more default client if delete.
    if ((void*)getDefaultClient() == (void*)this)
      setDefaultClient(0);
    delete [] recvBuffer;
    delete [] sendBuffer;
  }

  /*! In threaded environnments, this function lock()s the send buffer so that
   * only the calling thread can call the send functions. Otherwise do
   * nothing.
   */
  UAbstractClient::error_type
  UAbstractClient::startPack()
  {
    sendBufferLock.lock();
    return 0;
  }

  UAbstractClient::error_type
  UAbstractClient::endPack()
  {
    error_type res = effective_send(sendBuffer);
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    return res;
  }

  UAbstractClient::error_type
  UAbstractClient::send(const char* command, ...)
  {
    if (rc)
      return -1;
    va_list arg;
    va_start(arg, command);
    sendBufferLock.lock();
    rc = vpack(command, arg);
    va_end(arg);
    if (rc < 0)
    {
      sendBufferLock.unlock();
      return rc;
    }
    return rc = endPack();
  }

  UAbstractClient::error_type
  UAbstractClient::send(const std::string& s)
  {
    return send("%s", s.c_str());
  }

  UAbstractClient::error_type
  UAbstractClient::send(const UValue& v)
  {
    switch (v.type)
    {
    // Bounce to UValue operator << for those types.
    case DATA_DOUBLE:
    case DATA_SLOTNAME:
    case DATA_STRING:
      return send(string_cast(v));
      break;

    // Use our own sendBinary for binary, who knows how to talk to k1 and k2.
    case DATA_BINARY:
      if (v.binary->type != BINARY_NONE
          && v.binary->type != BINARY_UNKNOWN)
        v.binary->buildMessage();
      return sendBinary(v.binary->common.data, v.binary->common.size,
                        v.binary->message);
      break;

    // Lists can contain binary, so recurse using this function.
    case DATA_LIST:
      send("[");
      foreach (const UValue* u, *v.list)
        send(libport::format("%s,", u));
      return send("]");
      break;

    case DATA_DICTIONARY:
      send("[");
      if (v.dictionary->empty())
        send("=>");
      else
        foreach (const UDictionary::value_type& d, *v.dictionary)
          send(libport::format("\"%s\"=>%s,",
                               libport::escape(d.first), d.second));
      return send("]");
      break;

    case DATA_VOID:
      break;
    };
    return 0;
  }

  UAbstractClient::error_type
  UAbstractClient::send(std::istream& is)
  {
    if (rc)
      return -1;
    sendBufferLock.lock();
    while (is.good() && !rc)
    {
      is.read(sendBuffer, sendBufSize);
      rc = effective_send(sendBuffer, is.gcount());
    }
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    return rc;
  }



  /*! This function must only be called between a startPack()
    and the corresponding endPack(). Data is queued in the send
    buffer, and sent when endPack() is called.
  */
  UAbstractClient::error_type
  UAbstractClient::pack(const char* command, ...)
  {
    if (rc)
      return -1;
    va_list arg;
    va_start(arg, command);
    rc = vpack(command, arg);
    va_end(arg);
    return rc;
  }


  UAbstractClient::error_type
  UAbstractClient::vpack(const char* command, va_list arg)
  {
    if (rc)
      return -1;
    sendBufferLock.lock();
    if (command)
    {
      // Don't print if we overflow the buffer.  It would be nice to
      // rely on the behavior of the GNU LibC which accepts 0 as
      // destination buffer to query the space needed.  But it is not
      // portable (e.g., segv on OS X).  So rather, try to vsnprintf,
      // and upon failure, revert the buffer in its previous state.
      size_t slen = strlen(sendBuffer);
      size_t msize = sendBufSize - slen;
      int r = vsnprintf(sendBuffer + slen, msize, command, arg);
      // vsnprintf returns the number of characters to write.  Check
      // that it fits.  Don't forget the ending '\0' that it does not
      // count, but wants to add.
      if (r < 0 || static_cast<int>(msize) <= r)
      {
        // Don't produce partial input.
        sendBuffer[slen] = 0;
        rc = -1;
      }
    }
    sendBufferLock.unlock();
    return rc;
  }


  UAbstractClient::error_type
  UAbstractClient::sendFile(const std::string& f)
  {
    if (f == "/dev/stdin")
      return send(std::cin);
    else
    {
      std::ifstream is(f.c_str(), std::ios::binary);
      if (is.fail())
        return -1;
      else
        return send(is);
    }
  }


  UAbstractClient::error_type
  UAbstractClient::sendBin(const void* buffer, size_t len)
  {
    return sendBin(buffer, len, 0);
  }


  UAbstractClient::error_type
  UAbstractClient::sendBin(const void* buffer, size_t len,
			   const char* header, ...)
  {
    if (rc)
      return -1;
    sendBufferLock.lock();
    if (header)
    {
      va_list arg;
      va_start(arg, header);
      vpack(header, arg);
      va_end(arg);
      effective_send(sendBuffer);
    }

    error_type res = effective_send(buffer, len);
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    return res;
  }

  UAbstractClient::error_type
  UAbstractClient::sendBinary(const void* data, size_t len,
                              const std::string& header)
  {
    if (kernelMajor() < 2)
      return sendBin(data, len, "BIN %lu %s;",
                     static_cast<unsigned long>(len), header.c_str());
    else
    {
      sendBufferLock.lock();
      *this << libport::format("Global.Binary.new(\"%s\", \"\\B(%s)(",
                               libport::escape(header), len);
      flush();
      effective_send(data, len);
      *this << ")\")";
      sendBufferLock.unlock();
      return rc;
    }
  }

  struct sendSoundData
  {
    char* buffer;
    int bytespersec;
    size_t length;
    size_t pos;
    char* device;
    char* tag;
    char formatString[50];
    USoundFormat format;
    UAbstractClient* uc;
    bool startNotify;
  };

  static UCallbackAction sendSound_(void* cb, const UMessage &msg)
  {
    //the idea is to cut the sound into small chunks,
    //add a header and send each chunk separately

    //create the header.
    static const size_t CHUNK_SIZE = 32 * 8*60;
    sendSoundData* s = (sendSoundData*)cb;
    //handle next chunk
    if (s->format == SOUND_WAV && s->pos==0)
      s->pos = sizeof (wavheader);
    size_t tosend = std::min(CHUNK_SIZE, s->length - s->pos);

    //int playlength = tosend *1000 / s->bytespersec;
    std::string header = ((s->format == SOUND_WAV) ? "wav " : "raw ")
      + (std::string)s->formatString;

    s->uc->send("%s.val = Global.Binary.new(\"%s\", \"\\B(%lu)(",
		s->device,
                header.c_str(),
		static_cast<unsigned long>
                (tosend + ((s->format == SOUND_WAV) ? sizeof (wavheader) : 0))
		);

    if (s->format == SOUND_WAV)
    {
      wavheader wh;
      memcpy(&wh, s->buffer, sizeof wh);
      wh.datalength=tosend;
      wh.length=tosend+44-8;
      s->uc->sendBin(&wh, sizeof wh);
    }
    s->uc->sendBin(s->buffer+s->pos, tosend);
    ///TODO: make this constant modifiable
    s->uc->send(")\")|;waituntil(%s.remain < 1000);\n"
		" %s << 1;\n", s->device, msg.tag.c_str());
    s->pos += tosend;
    if (s->pos >= s->length)
    {
      const char* dev = s->device ? s->device : "speaker";
      s->uc->send("%s.val->blend = %s.sendsoundsaveblend;", dev, dev);

      if (s->tag && s->tag[0])
        s->uc->send("Channel.new(\"%s\") << 1;\n", s->tag);
      delete[] s->buffer;
      free(s->tag);
      free(s->device);
      delete s;
      return URBI_REMOVE;
    }
    return URBI_CONTINUE;
  }

  /*!
    If tag is set, an URBI system "stop" message with this tag will be
    generated when the sound has been played.  The sound data is copied in
    case of asynchronous send, and may be safely deleted as soon as this
    function returns.
  */
  UAbstractClient::error_type
  UAbstractClient::sendSound(const char* device, const USound& sound,
			     const char* tag)
  {
    switch (sound.soundFormat)
    {
    case SOUND_MP3:
    case SOUND_OGG:
      // We don't handle chunking for these formats.
      return sendBin(sound.data, sound.size,
		     "%s +report:  %s.val = BIN %lu %s;",
		     tag, device, static_cast<unsigned long>(sound.size),
                     sound.soundFormat == SOUND_MP3 ? "mp3" : "ogg");
      break;

    case SOUND_WAV:
    case SOUND_RAW:
    {
      const char* dev = device ? device : "speaker";
      send("%s.removeSlot(\"sendsoundsaveblend\") |"
           "var %s.sendsoundsaveblend = %s.val->blend;"
           "%s.val->blend=\"queue\";",
           dev, dev, dev, dev);
      sendSoundData* s = new sendSoundData();
      s->bytespersec = sound.channels * sound.rate * (sound.sampleSize / 8);
      s->uc = this;
      s->buffer = new char[sound.size];
      memcpy(s->buffer, sound.data, sound.size);
      s->length = sound.size;
      s->tag = tag ? strdup(tag) : 0;
      s->device = strdup(device);
      s->pos = 0;
      s->format = sound.soundFormat;
      if (sound.soundFormat == SOUND_RAW)
	sprintf(s->formatString, "%zd %zd %zd %d",
		sound.channels, sound.rate, sound.sampleSize,
		sound.sampleFormat);
      else
	s->formatString[0] = 0;
      s->startNotify = false;
      std::string utag = fresh();
      (*this) << "var " + utag +" = Channel.new(\"" << utag << "\");";
      UCallbackID cid = setCallback(sendSound_, s, utag.c_str());
      // Invoke it 2 times to queue sound.
      if (sendSound_(s, UMessage(*this, 0, utag, "*** stop",
				 binaries_type()))
          == URBI_CONTINUE)
      {
	if (sendSound_(s, UMessage(*this, 0, utag, "*** stop",
				   binaries_type()))
            == URBI_REMOVE)
	  deleteCallback(cid);
      }
      else
	deleteCallback(cid);
      return 0;
    }

    default:
      // Unrecognized format.
      return 1;
    }
  }

  UCallbackID
  UAbstractClient::setCallback(UCallback cb, const char* tag)
  {
    return addCallback(tag, *new UCallbackWrapperCB(cb));
  }

  UCallbackID
  UAbstractClient::setCallback(UCustomCallback cb,
			       void* cbData,
			       const char* tag)
  {
    return addCallback(tag, *new UCallbackWrapperCCB(cb, cbData));
  }


  int
  UAbstractClient::getAssociatedTag(UCallbackID id, char* tag)
  {
    listLock.lock();
    callbacks_type::iterator it =
      std::find(callbacks_.begin(), callbacks_.end(), id);
    if (it == callbacks_.end())
    {
      listLock.unlock();
      return 0;
    }
    strcpy(tag, it->tag);
    listLock.unlock();
    return 1;
  }


  int
  UAbstractClient::deleteCallback(UCallbackID id)
  {
    listLock.lock();
    callbacks_type::iterator it =
      std::find(callbacks_.begin(), callbacks_.end(), id);
    if (it == callbacks_.end())
    {
      listLock.unlock();
      return 0;
    }
    delete &(it->callback);
    callbacks_.erase(it);
    listLock.unlock();
    return 1;
  }

  UCallbackID
  UAbstractClient::sendCommand(UCallback cb, const char* cmd, ...)
  {
    std::string tag = fresh();
    std::string mcmd = tag + " << " + cmd;
    UCallbackID res = setCallback(cb, tag.c_str());
    sendBufferLock.lock();
    va_list arg;
    va_start(arg, cmd);
    vpack(mcmd.c_str(), arg);
    va_end(arg);
    if (endPack())
    {
      deleteCallback(res);
      return UINVALIDCALLBACKID;
    }
    return res;
  }

  UCallbackID
  UAbstractClient::sendCommand(UCustomCallback cb, void *cbData,
			       const char* cmd, ...)
  {
    std::string tag = fresh();
    std::string mcmd = tag + " << " + cmd;
    UCallbackID res = setCallback(cb, cbData, tag.c_str());
    sendBufferLock.lock();
    va_list arg;
    va_start(arg, cmd);
    vpack(mcmd.c_str(), arg);
    va_end(arg);
    if (endPack())
    {
      deleteCallback(res);
      return UINVALIDCALLBACKID;
    }
    return res;
  }

  UAbstractClient::error_type
  UAbstractClient::putFile(const char* localName, const char* remoteName)
  {
    sendBufferLock.lock();
    if (!remoteName)
      remoteName = localName;
    send("save(\"%s\", \"", remoteName);
    error_type res = sendFile(localName);
    send("\");");
    sendBufferLock.unlock();
    return res;
  }

  UAbstractClient::error_type
  UAbstractClient::putFile(const void* buffer, size_t length,
			   const char* remoteName)
  {
    send("save(\"%s\", \"", remoteName);
    sendBin(buffer, length);
    send("\");");
    sendBufferLock.unlock();
    return 0;
  }

  std::string
  UAbstractClient::fresh()
  {
    static boost::format fmt("URBI_%s");
    return str(fmt % ++counter_);
  }

  void
  UAbstractClient::makeUniqueTag(char* tag)
  {
    strcpy(tag, fresh().c_str());
  }

  bool
  UAbstractClient::process_recv_buffer_binary_()
  {
    //Receiving binary. Append to binaryBuffer;
    size_t len =
      std::min(recvBufferPosition - endOfHeaderPosition,
               binaryBufferLength - binaryBufferPosition);
    if (binaryBuffer)
      memcpy (static_cast<char*> (binaryBuffer) + binaryBufferPosition,
              recvBuffer + endOfHeaderPosition, len);
    binaryBufferPosition += len;

    if (binaryBufferPosition == binaryBufferLength)
    {
      //Finished receiving binary.
      //append
      BinaryData bd;
      bd.size = binaryBufferLength;
      bd.data = binaryBuffer;
      bins << bd;
      binaryBuffer = 0;

      if (nBracket == 0)
      {
        //end of command, send
        //dumb listLock.lock();
        UMessage msg(*this, currentTimestamp, currentTag, currentCommand,
                     bins);
        notifyCallbacks(msg);
        //unlistLock.lock();

        bins_clear();

        //flush
        parsePosition = 0;
        //Move the extra we received
        recvBufferPosition -= len + endOfHeaderPosition;
        memmove(recvBuffer,
                recvBuffer + endOfHeaderPosition + len,
                recvBufferPosition);
      }
      else
      {
        // not over yet
        //leave parseposition where it is
        //move the extra (parsePosition = endOfHeaderPosition)
        recvBufferPosition -= len;
        memmove(recvBuffer + parsePosition,
                recvBuffer + endOfHeaderPosition + len,
                recvBufferPosition - endOfHeaderPosition);
      }
      binaryBuffer = 0;
      binaryMode = false;

      // Reenter loop.
      return true;
    }
    else
    {
      // Not finished receiving binary.
      recvBufferPosition = endOfHeaderPosition;
      return false;
    }
  }

  bool
  UAbstractClient::process_recv_buffer_text_()
  {
    // Not in binary mode.
    char* endline =
      static_cast<char*> (memchr(recvBuffer+parsePosition, '\n',
                                 recvBufferPosition - parsePosition));
    if (!endline)
      return false; //no new end of command/start of binary: wait

    if (parsePosition == 0) // parse header
    {
      // Ignore empty lines.
      if (endline == recvBuffer)
      {
        memmove(recvBuffer, recvBuffer+1, recvBufferPosition - 1);
        recvBufferPosition--;
        return true;
      }

      if (2 != sscanf(recvBuffer, "[%d:%64[A-Za-z0-9_.]]",
                      &currentTimestamp, currentTag))
      {
        if (1 == sscanf(recvBuffer, "[%d]", &currentTimestamp))
          currentTag[0] = 0;
        else
        {
          // failure
          GD_FERROR("read, error parsing header: '%s'", recvBuffer);
          currentTimestamp = 0;
          strcpy(currentTag, "UNKNWN");
          //listLock.lock();
          UMessage msg(*this, 0, tag_error,
                       "!!! UAbstractClient::read, fatal error parsing header",
                       binaries_type());
          notifyCallbacks(msg);
          //unlistLock.lock();
        }
      }

      currentCommand = strstr(recvBuffer, "]");
      if (!currentCommand)
      {
        //reset all
        nBracket = 0;
        inString = false;
        parsePosition = 0;
        recvBufferPosition = 0;
        return false;
      }

      ++currentCommand;
      while (*currentCommand == ' ')
        ++currentCommand;
      system = (*currentCommand == '!' || *currentCommand == '*');
      parsePosition = (long) currentCommand - (long) recvBuffer;

      //reinit just to be sure:
      nBracket = 0;
      inString = false;
    }

    for (/* nothing */; parsePosition < recvBufferPosition; ++parsePosition)
    {
      if (inString)
        switch (recvBuffer[parsePosition])
        {
        case '\\':
          if (parsePosition == recvBufferPosition-1)
            //we cant handle the '\\'
            return false;
          ++parsePosition; //ignore next character
          continue;
        case '"':
          inString = false;
          continue;
        }
      else
      {
        switch (recvBuffer[parsePosition])
        {
        case '"':
          inString = true;
          continue;
        case '[':
          ++nBracket;
          continue;
        case ']':
          --nBracket;
          continue;
        case '\n':
          // FIXME: handle '[' in echoed messages or errors nBracket == 0.
          //
          // end of command
          recvBuffer[parsePosition] = 0;
          //listLock.lock();
          UMessage msg(*this, currentTimestamp, currentTag,
                       currentCommand,
                       bins);
          notifyCallbacks(msg);
          //unlistLock.lock();
          //prepare for next read, copy the extra
          memmove(recvBuffer, recvBuffer + parsePosition + 1,
                  recvBufferPosition - parsePosition - 1);
          // copy beginning of next cmd
          recvBufferPosition = recvBufferPosition - parsePosition - 1;
          recvBuffer[recvBufferPosition] = 0;
          parsePosition = 0;
          bins_clear();
          goto line_finished; //restart
        }

        if (!system && !strncmp(recvBuffer+parsePosition-3, "BIN ", 4))
        {
          //very important: scan starts below current point
          //compute length
          char* endLength;
          binaryBufferLength =
            strtol(recvBuffer+parsePosition+1, &endLength, 0);
          if (endLength == recvBuffer+parsePosition+1)
          {
            GD_ERROR("read, error parsing bin data length.");
            recvBufferPosition = 0;
            return false;
          }
          //go to end of header
          while (recvBuffer[parsePosition] !='\n')
            ++parsePosition; //we now we will find a \n
          ++parsePosition;
          endOfHeaderPosition = parsePosition;
          binaryMode = true;
          binaryBuffer = malloc(binaryBufferLength);
          binaryBufferPosition = 0;
          break; //restart in binarymode to handle binary
        }
      }
    }
  line_finished:
    // Either we ate all characters, or we were asked to restart.
    return parsePosition != recvBufferPosition;
  }

  /*!
    As long as this function has not returned, neither recvBuffer
    nor recvBufferPos may be modified.
  */
  void
  UAbstractClient::processRecvBuffer()
  {
    while (binaryMode
           ? process_recv_buffer_binary_()
           : process_recv_buffer_text_())
      continue;
  }

  UCallbackID
  UAbstractClient::setWildcardCallback(UCallbackWrapper& callback)
  {
    return addCallback(tag_wildcard, callback);
  }

  UCallbackID
  UAbstractClient::setErrorCallback(UCallbackWrapper& callback)
  {
    return addCallback(tag_error, callback);
  }

  UCallbackID
  UAbstractClient::setClientErrorCallback(UCallbackWrapper& callback)
  {
    return addCallback(CLIENTERROR_TAG, callback);
  }

  UCallbackID
  UAbstractClient::setCallback(UCallbackWrapper& callback,
                               const char* tag)
  {
    return addCallback(tag, callback);
  }

  UCallbackID
  UAbstractClient::addCallback(const char* tag,
                               UCallbackWrapper& w)
  {
    listLock.lock();
    UCallbackInfo ci(w);
    strncpy(ci.tag, tag, URBI_MAX_TAG_LENGTH-1);
    ci.tag[URBI_MAX_TAG_LENGTH-1]=0;
    ci.id = ++nextId;
    callbacks_.push_front(ci);
    listLock.unlock();
    return ci.id;
  }

  void
  UAbstractClient::clientError(std::string message, int erc)
  {
    // Like in UMessage's constructor, skip the possible "!!! "
    // prefix.
    const char prefix[] = "!!! ";
    if (message.substr(0, sizeof prefix - 1) == prefix)
      message.erase(0, sizeof prefix - 1);

    if (erc)
    {
      message += message.empty() ? "" : ": ";
      message += libport::strerror(erc);
    }

    UMessage m(*this);
    m.type = MESSAGE_ERROR;
    // rawMessage is incorrect but we don't care.
    m.message = m.rawMessage = message;
    m.timestamp = 0;
    m.tag = CLIENTERROR_TAG;
    notifyCallbacks(m);
  }

  void
  UAbstractClient::clientError(const char* message, int erc)
  {
    return clientError(std::string(message ? message : ""), erc);
  }

  void
  UAbstractClient::onConnection()
  {
# define VERSION_TAG TAG_PRIVATE_PREFIX "__version"
    setCallback(*this, &UAbstractClient::setVersion, VERSION_TAG);
    // We don't know our kernel version yet.
    send(SYNCLINE_WRAP(
           "{\n"
           "  var __ver__ = 2;\n"
           "  {var __ver__ = 1};\n"
           "  var " VERSION_TAG ";\n"
           "  if (__ver__ == 1)\n"
           "    " VERSION_TAG " << system.version\n"
           "  else\n"
           "  {\n"
           "    " VERSION_TAG " = Channel.new(\"" VERSION_TAG "\");\n"
           "    " VERSION_TAG " << System.version;\n"
           "  };\n"
           "};\n"));
# undef VERSION_TAG
  }

  UCallbackAction
  UAbstractClient::setConnectionID(const UMessage& msg)
  {
    GD_FINFO_TRACE("setConnectionId for client %p", this);
    if (msg.type == MESSAGE_DATA && msg.value)
    {
      std::string id(*msg.value);
      if (!id.empty())
      {
        libport::BlockLock bl(sendBufferLock);
	connectionID_ = id;
	return URBI_REMOVE;
      }
    }
    return URBI_CONTINUE;
  }

  UCallbackAction
  UAbstractClient::setVersion(const UMessage& msg)
  {
    GD_FINFO_TRACE("setVersion for client %p", this);
    libport::BlockLock bl(sendBufferLock);
    if (msg.type != MESSAGE_DATA)
      return URBI_CONTINUE;
    aver_eq(msg.value->type, DATA_STRING);
    kernelVersion_ = *msg.value->stringValue;
    size_t sep = kernelVersion_.find_first_of('.');
    try
    {
      kernelMajor_ = boost::lexical_cast<int>(kernelVersion_.substr(0, sep));
      size_t sep2 = kernelVersion_.find_first_of('.', sep+1);
      if (sep2 != kernelVersion_.npos)
        kernelMinor_ =
          boost::lexical_cast<int>(kernelVersion_.substr(sep+1,
                                                         sep2-sep-1));
      else
        kernelMinor_ = 0;
    }
    catch (boost::bad_lexical_cast&)
    {
      kernelMajor_ = 2;
      kernelMinor_ = 0;
      GD_FWARN("failed to parse kernel version string: '%s', assuming %s.%s.",
               kernelVersion_,
               kernelMajor_,
               kernelMinor_);
    }
    // Set the kernel version of our associated stream.
    ::urbi::kernelMajor(*this) = kernelMajor_;

    // Have the connectionId sent on __ident.
# define IDENT_TAG TAG_PRIVATE_PREFIX "__ident"
    setCallback(*this, &UAbstractClient::setConnectionID, IDENT_TAG);
    if (kernelMajor_ < 2)
      send(IDENT_TAG " << local.connectionID;\n");
    else
      send(SYNCLINE_WRAP("Channel.new(\"" IDENT_TAG "\")"
                         " << connectionTag.name;\n"));
    return URBI_REMOVE;
# undef IDENT_TAG
  }

  int
  UAbstractClient::getCurrentTimestamp() const
  {
    return currentTimestamp;
  }

  const std::string&
  UAbstractClient::connectionID() const
  {
    libport::BlockLock bl(sendBufferLock);
    return connectionID_;
  }

  std::string
  getClientConnectionID(const UAbstractClient* cli)
  {
    if (!cli)
      return "";
    return cli->connectionID();
  }


  /*-----------------.
  | Default client.  |
  `-----------------*/

  UClient* defaultClient = 0;

  UClient* getDefaultClient()
  {
    return defaultClient;
  }

  UClient& get_default_client()
  {
    return *getDefaultClient();
  }

  void setDefaultClient(UClient* cl)
  {
    defaultClient = cl;
  }


  std::ostream&
  unarmorAndSend(const char* a, UAbstractClient* where)
  {
    aver(a);
    aver(where);
    std::ostream& s = *where;
    if (strlen(a)>2)
    {
      if (a[0]=='(' && a[strlen(a)-1]==')')
	s.rdbuf()->sputn(a+1, strlen(a)-2);
      else
	s << a; //this is baaad, user forgot the parenthesis but was lucky
    }
    return s;
  }

} // namespace urbi
