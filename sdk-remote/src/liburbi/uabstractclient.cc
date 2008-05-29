/*! \file uabstractclient.cc
****************************************************************************
 * Definition of the URBI interface class
 *
 * Copyright (C) 2004, 2006, 2007, 2008 Jean-Christophe Baillie.  All rights reserved.
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

#include "libport/windows.hh"

#include "libport/cstdio"
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include "libport/sys/stat.h"

#include <algorithm>
#include <iostream>

#include "libport/lockable.hh"

#include "urbi/uabstractclient.hh"

#include "libport/cstring"

#define URBI_ERROR_TAG "[error]"
#define URBI_WILDCARD_TAG "[wildcard]"

namespace urbi
{
  enum UCallbackType
  {
    UCB_,
    UCB_C,
  };

  static UCallbackID nextId;

  class UCallbackWrapperCB: public UCallbackWrapper
  {
    UCallback cb;
  public:
    UCallbackWrapperCB(UCallback cb): cb(cb)
    {
    }
    virtual UCallbackAction operator ()(const UMessage& msg)
    {
      return cb(msg);
    }
  };


  class UCallbackWrapperCCB: public UCallbackWrapper
  {
    UCustomCallback cb;
    void * data;
  public:
    UCallbackWrapperCCB(UCustomCallback cb, void * data): cb(cb), data(data)
    {
    }
    virtual UCallbackAction operator ()(const UMessage& msg)
    {
      return cb(data, msg);
    }
  };


  class UClientStreambuf: public std::streambuf
  {
  public:
    UClientStreambuf(UAbstractClient * cl)
      : client(cl)
    {
    }

  protected:
    virtual int overflow (int c = EOF);
    // Override std::basic_streambuf<_CharT, _Traits>::xsputn.
    virtual std::streamsize xsputn (const char* s, std::streamsize n);

  private:
    UAbstractClient* client;
  };

  int UClientStreambuf::overflow (int c )
  {
    if (c != EOF)
    {
      char ch = c;
      xsputn(&ch, 1);
    }
    return c;
  }

  std::streamsize UClientStreambuf::xsputn (const char* s, std::streamsize n)
  {
    client->sendBufferLock.lock();
    if (strlen(client->sendBuffer)+1+n > static_cast<unsigned>(client->buflen))
    {
      //error
      client->sendBufferLock.unlock();
      return 0;
    }
    int clen = strlen(client->sendBuffer);
    memcpy(client->sendBuffer+clen, s, n);
    client->sendBuffer[clen+n] = 0;
    if (strchr(client->sendBuffer, '&')
	|| strchr(client->sendBuffer, '|')
	|| strchr(client->sendBuffer, ';')
	|| strchr(client->sendBuffer, ','))
    {
      client->effectiveSend(client->sendBuffer, strlen(client->sendBuffer));
      client->sendBuffer[0] = 0;
    }
    client->sendBufferLock.unlock();
    return n;
  }


  const char * UAbstractClient::CLIENTERROR_TAG="client error";

  /*! Pass the given UMessage to all registered callbacks with the
   * corresponding tag, as if it were comming from the URBI server.
   */
  void
  UAbstractClient::notifyCallbacks(const UMessage &msg)
  {
    listLock.lock();
    bool inc=false;
    for (std::list<UCallbackInfo>::iterator it = callbackList.begin();
	 it!=callbackList.end(); inc?it:it++, inc=false)
    {
      if (STREQ(msg.tag.c_str(), it->tag)
	  || STREQ(it->tag, URBI_ERROR_TAG) && msg.type == MESSAGE_ERROR
	  || STREQ(it->tag, URBI_WILDCARD_TAG))
      {
	UCallbackAction ua = it->callback(msg);
	if (ua == URBI_REMOVE)
	{
	  delete &(it->callback);
	  it=callbackList.erase(it);
	  inc = true;
	}
      }
    }
    listLock.unlock();
  }

  /*! Initializes sendBuffer and recvBuffer, and copy _host and _port.
   \param _host IP address or name of the robot to connect to.
   \param _port TCP port to connect to.
   \param _buflen size of send and receive buffers.
   Implementations should establish the connection in their constructor.
   */
  UAbstractClient::UAbstractClient(const char *_host, int _port, int _buflen)
    : std::ostream(new UClientStreambuf(this)),
      // Irk, *new...
      sendBufferLock(*new libport::Lockable()),
      listLock(*new libport::Lockable()),
      host (NULL),
      port (_port),
      buflen (_buflen),
      rc (0),

      recvBuffer (NULL),
      recvBufferPosition (0),

      binaryBuffer (NULL),
      parsePosition (0),
      inString (false),
      nBracket (0),
      binaryMode(false),
      system(false),
      uid (0),
      stream(this)
  {
    getStream().setf(std::ios::fixed);
    host = new char[strlen(_host) + 1];
    if (!host)
    {
      rc = -1;
      return;
    }
    strcpy(host, _host);

    recvBuffer = new char[buflen];
    if (!recvBuffer)
    {
      rc = -1;
      //printf("UAbstractClient::UAbstractClient out of memory\n");
      return;
    }
    recvBuffer[0] = 0;

    sendBuffer = new char[buflen];
    if (!sendBuffer)
    {
      rc = -1;
      //printf("UAbstractClient::UAbstractClient out of memory\n");
      return;
    }
    sendBuffer[0] = 0;
  }

  UAbstractClient::~UAbstractClient()
  {
    delete [] host;
    delete [] recvBuffer;
    delete [] sendBuffer;
  }

  /*! In threaded environnments, this function lock()s the send buffer so that
   * only the calling thread can call the send functions. Otherwise do
   * nothing.
   */
  int
  UAbstractClient::startPack()
  {
    sendBufferLock.lock();
    return 0;
  }

  int
  UAbstractClient::endPack()
  {
    int retval = effectiveSend(sendBuffer, strlen(sendBuffer));
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    return retval;
  }


  /*! Multiple commands can be sent in one call.
   */
  int
  UAbstractClient::send(const char *command, ...)
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
    rc = effectiveSend(sendBuffer, strlen(sendBuffer));
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    return rc;
  }

  int UAbstractClient::send(UValue &v)
  {
    switch (v.type)
    {
      case DATA_DOUBLE:
	send("%lf", v.val);
	break;
      case DATA_STRING:
	send("\"%s\"", v.stringValue->c_str());
	break;
      case DATA_BINARY:
	if (v.binary->type != BINARY_NONE
	    && v.binary->type != BINARY_UNKNOWN)
	  v.binary->buildMessage();
	sendBin(v.binary->common.data, v.binary->common.size,
		"BIN %d %s;", v.binary->common.size,
		v.binary->message.c_str());
	break;
      case DATA_LIST:
      {
	send("[");
	int sz = v.list->size();
	for (int i = 0; i < sz; ++i)
	{
	  send((*v.list)[i]);
	  if (i != sz-1)
	    send(" , ");
	}
	send("]");
      }
      break;
      case DATA_OBJECT:
      {
	send("OBJ %s [", v.object->refName.c_str());
	int sz = v.object->size();
	for (int i = 0; i < sz; ++i)
	{
	  send("%s :", (*v.object)[i].name.c_str());
	  send(*((*v.object)[i].val) );
	  if (i != sz-1)
	    send(" , ");
	}
	send("]");
      }
      break;
      case DATA_VOID:
	break;
    };
    return 0;
  }



  /*! This function must only be called between a startPack()
   and the corresponding endPack(). Data is queued in the send
   buffer, and sent when endPack() is called.
   */
  int
  UAbstractClient::pack(const char *command, ...)
  {
    if (rc)
      return -1;
    va_list arg;
    va_start(arg, command);
    rc=vpack(command, arg);
    va_end(arg);
    return rc;
  }


  int
  UAbstractClient::vpack(const char *command, va_list arg)
  {
    //expand
    if (rc)
      return -1;
#if 0  //disabled, crashes
    int size = vsnprintf(NULL, 0, command, arg);
    va_end(arg);
    if (strlen(sendBuffer) + size + 1 > buflen)
      return -1;
    else
    {
#endif
      sendBufferLock.lock();
      vsprintf(&sendBuffer[strlen(sendBuffer)], command, arg);
      sendBufferLock.unlock();
      va_end(arg);
      return 0;
#if 0
    }
#endif
  }


  int
  UAbstractClient::sendFile(const char *name)
  {
    if (rc)
      return -1;
    FILE *fd;
    fd = fopen(name, "r");
    if (fd == NULL)
      return -1;
    int size;
    struct stat s;
    stat(name, &s);
    size = s.st_size;
    sendBufferLock.lock();
    if (!canSend(size))
    {
      sendBufferLock.unlock();
      return -1;
    }

    while (!feof(fd))
    {
      int res = fread(sendBuffer, 1, buflen, fd);
      effectiveSend(sendBuffer, res);
    }
    fclose(fd);
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    return 0;
  }


  int
  UAbstractClient::sendBin(const void *buffer, int len)
  {
    return sendBin(buffer, len, NULL, 0);
  }


  int
  UAbstractClient::sendBin(const void *buffer, int len,
			   const char *header, ...)
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
      if (!canSend(strlen(sendBuffer) + len))
      {
	sendBufferLock.unlock();
	return -1;
      }

      effectiveSend(sendBuffer, strlen(sendBuffer));
    }

    int res = effectiveSend(buffer, len);
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    return res;
  }

  struct sendSoundData
  {
    char * buffer;
    int bytespersec;
    int length;
    int pos;
    char * device;
    char * tag;
    char formatString[50];
    USoundFormat format;
    UAbstractClient * uc;
    bool startNotify;
  };

  struct wavheader
  {
    char riff[4];
    int length;
    char wave[4];
    char fmt[4];
    int lnginfo;
    short one;
    short channels;
    int freqechant;
    int bytespersec;
    short bytesperechant;
    short bitperchannel;
    char data[4];
    int datalength;
  };

  static UCallbackAction sendSound_(void * cb, const UMessage &msg)
  {
    //the idea is to cut the sound into small chunks,
    //add a header and send each chunk separately
    //create the header.
    // printf("sound message: %s %d\n", msg.systemValue, msg.type);
    static const int CHUNK_SIZE = 32 * 8*60;
    // static const int SUBCHUNK_SIZE = CHUNK_SIZE; //1024;


    sendSoundData *s=(sendSoundData *)cb;
    /*
     if (msg.type != MESSAGE_SYSTEM)
     return URBI_CONTINUE;
     if (strstr(msg.systemValue, "start") && s->startNotify==false)
     {
     s->startNotify = true;
     s->uc->notifyCallbacks(UMessage(*s->uc, 0, s->tag, "*** start"));
     }
     if (!strstr(msg.systemValue, "stop"))
     return URBI_CONTINUE;
     */
    /*wavheader wh =
     {
     {'R', 'I', 'F', 'F'},
     44-8,
     {'W', 'A', 'V', 'E'},
     {'f', 'm', 't', ' '},
     16, 1, 1, 16000, 32000, 2, 16,
     {'d', 'a', 't', 'a'},
     0}; // no comment...
     */
    //handle next chunk
    if (s->format == SOUND_WAV && s->pos==0)
      s->pos = sizeof (wavheader);
    int tosend = (s->length-s->pos > CHUNK_SIZE) ? CHUNK_SIZE:s->length-s->pos;

    //printf("%d start chunk of size %d at offset %d\n", 0, tosend, s->pos);
    int playlength = tosend *1000 / s->bytespersec;
    s->uc->send("%s.val = BIN %d %s %s;",
		s->device,
		(int)(tosend+ ((s->format == SOUND_WAV)?sizeof (wavheader):0)),
		(s->format == SOUND_WAV)?"wav":"raw",
		s->formatString
      );
    if (s->format == SOUND_WAV)
    {
      wavheader wh;
      memcpy(&wh, s->buffer, sizeof (wh));
      wh.datalength=tosend;
      wh.length=tosend+44-8;
      s->uc->sendBin(&wh, sizeof (wavheader));
    }


#if 0
    // this appears to be useless
    int msecpause = ((SUBCHUNK_SIZE / 32) * 10) / 14;
    int spos = 0;
    while (spos != tosend)
    {
      int ts = SUBCHUNK_SIZE;
      if (ts > tosend-spos)
	ts = tosend-spos;
      printf("%d chunk\n", mtime());
      s->uc->sendBin(s->buffer, ts);
      s->buffer += ts;
      spos += ts;
      usleep(msecpause);
    }
#endif

    s->uc->sendBin(s->buffer+s->pos, tosend);
    s->uc->send("wait(%s.remain < %d);"
		" %s << ping;", s->device, playlength / 2, msg.tag.c_str());
    // printf("%d end sending chunk\n", 0);
    s->pos += tosend;
    if (s->pos >= s->length )
    {
      //printf("over: %d %d\n", URBI_REMOVE, URBI_CONTINUE);
      //if (s->tag && s->tag[0])
      //  s->uc->notifyCallbacks(UMessage(*s->uc, 0, s->tag, "*** stop"));

      std::string rDevice = (s->device) ? s->device : "speaker";
      std::string message = rDevice + "->blend=" +
	rDevice + ".sendsoundsaveblend;";
      s->uc->send(message.c_str ());
      if (s->tag && s->tag[0])
	s->uc->send("%s << 1;", s->tag);
      free(s->buffer);
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
  int
  UAbstractClient::sendSound(const char* device, const USound& sound,
			     const char* tag)
  {
    if (sound.soundFormat == SOUND_MP3)
    {
      //we don't handle chunkuing for this format
      return sendBin(sound.data, sound.size,
		     "%s +report:  %s.val = BIN %d mp3;",
		     tag, device, sound.size);
    }
    if (sound.soundFormat == SOUND_OGG)
    {
      //we don't handle chunkuing for this format
      return sendBin(sound.data, sound.size,
		     "%s +report:  %s.val = BIN %d ogg;",
		     tag, device, sound.size);
    }

    if (sound.soundFormat == SOUND_WAV
	|| sound.soundFormat == SOUND_RAW)
    {
      std::string rDevice = (device) ? device : "speaker";
      std::string message = rDevice + ".sendsoundsaveblend = " +
	rDevice + "->blend;" + rDevice + "->blend=queue;";
      send(message.c_str ());
      sendSoundData *s = new sendSoundData();
      char utag[16];
      makeUniqueTag(utag);
      s->bytespersec = sound.channels * sound.rate * (sound.sampleSize / 8);
      s->uc = this;
      s->buffer = static_cast<char*> (malloc (sound.size));
      memcpy(s->buffer, sound.data, sound.size);
      s->length = sound.size;
      s->tag = tag ? strdup(tag) : 0;
      s->device = strdup(device);
      s->pos = 0;
      s->format = sound.soundFormat;
      if (sound.soundFormat == SOUND_RAW)
	sprintf(s->formatString, "%d %d %d %d",
		sound.channels, sound.rate, sound.sampleSize,
		(int) sound.sampleFormat);
      else
	s->formatString[0] = 0;
      s->startNotify = false;
      UCallbackID cid=setCallback(sendSound_, s, utag);
      //invoke it 2 times to queue sound
      if (sendSound_(s, UMessage(*this, 0, utag, "*** stop",
				 std::list<BinaryData>()))==URBI_CONTINUE)
      {
	if (sendSound_(s, UMessage(*this, 0, utag, "*** stop",
				   std::list<BinaryData>()))==URBI_REMOVE)
	{
	  deleteCallback(cid);
	}
      }
      else
	deleteCallback(cid);
      return 0;
    }
    //unrecognized format
    return 1;
  }

  UCallbackID
  UAbstractClient::setCallback(UCallback cb, const char *tag)
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

  /*! Returns 1 and fills tag on success, 0 on failure
   */
  int
  UAbstractClient::getAssociatedTag(UCallbackID id, char * tag)
  {
    listLock.lock();
    std::list<UCallbackInfo>:: iterator it = find(callbackList.begin(),
						  callbackList.end(), id);
    if (it == callbackList.end())
    {
      listLock.unlock();
      return 0;
    }
    strcpy(tag, it->tag);
    listLock.unlock();
    return 1;
  }


  /*! Returns 0 if no callback with this id was found, 1 otherwise.
   */
  int
  UAbstractClient::deleteCallback(UCallbackID callbackID)
  {
    listLock.lock();
    std::list<UCallbackInfo>:: iterator it = find(callbackList.begin(),
						  callbackList.end(),
						  callbackID);
    if (it == callbackList.end())
    {
      listLock.unlock();
      return 0;
    }
    delete &(it->callback);
    callbackList.erase(it);
    listLock.unlock();
    return 1;
  }

  UCallbackID
  UAbstractClient::sendCommand(UCallback cb, const char *cmd, ...)
  {
    char tag[16];
    makeUniqueTag(tag);
    char *mcmd = new char[strlen(cmd) + strlen(tag) + 5];
    sprintf(mcmd, "%s << %s", tag, cmd);
    UCallbackID cid = setCallback(cb, tag);
    sendBufferLock.lock();
    va_list arg;
    va_start(arg, cmd);
    vpack(mcmd, arg);
    va_end(arg);
    int retval = effectiveSend(sendBuffer, strlen(sendBuffer));
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    delete []  mcmd;
    if (retval)
    {
      deleteCallback(cid);
      return UINVALIDCALLBACKID;
    }
    return cid;
  }

  UCallbackID
  UAbstractClient::sendCommand(UCustomCallback cb, void *cbData,
			       const char *cmd, ...)
  {
    char tag[16];
    makeUniqueTag(tag);
    char *mcmd = new char[strlen(cmd) + strlen(tag) + 10];
    sprintf(mcmd, "%s << %s", tag, cmd);
    UCallbackID cid = setCallback(cb, cbData, tag);
    sendBufferLock.lock();
    va_list arg;
    va_start(arg, cmd);
    vpack(mcmd, arg);
    va_end(arg);
    int retval = effectiveSend(sendBuffer, strlen(sendBuffer));
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    delete []mcmd;
    if (retval)
    {
      deleteCallback(cid);
      return UINVALIDCALLBACKID;
    }
    return cid;

  }

  int
  UAbstractClient::putFile(const char * localName, const char * remoteName)
  {
    int len;
    struct stat st;
    if (stat(localName, &st) == -1)
      return 1;
    len = st.st_size;
    sendBufferLock.lock();
    if (!canSend(len + strlen(remoteName) + 20))
    {
      sendBufferLock.unlock();
      return -1;
    }

    if (!remoteName)
      remoteName = localName;
    send("save(\"%s\", \"", remoteName);
    int res = sendFile(localName);
    send("\");");
    sendBufferLock.unlock();
    return res;
  }

  int
  UAbstractClient::putFile(const void * buffer, int length,
			   const char * remoteName)
  {
    if (!canSend(length+strlen(remoteName)+ 20))
    {
      sendBufferLock.unlock();
      return -1;
    }
    send("save(\"%s\", \"", remoteName);
    sendBin(buffer, length);
    send("\");");
    sendBufferLock.unlock();
    return 0;
  }

  void
  UAbstractClient::makeUniqueTag(char* tag)
  {
    sprintf(tag, "URBI_%d", ++uid);
    return;
  }

  /*!
   As long as this function has not returned, neither recvBuffer
   nor recvBufferPos may be modified.
   */
  void
  UAbstractClient::processRecvBuffer()
  {
    while (true)
    {
      if (binaryMode)
      {
	//Receiving binary. Append to binaryBuffer;
	int len = std::min(recvBufferPosition - endOfHeaderPosition,
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
	  bins.push_back(bd);
	  binaryBuffer = 0;

	  if (nBracket == 0)
	  {
	    //end of command, send
	    //dumb listLock.lock();
	    UMessage msg(*this, currentTimestamp, currentTag, currentCommand,
			 bins);
	    notifyCallbacks(msg);
	    //unlistLock.lock();

	    while (!bins.empty())
	    {
	      free(bins.front().data);
	      bins.pop_front();
	    }
	    //flush
	    parsePosition = 0;
	    //Move the extra we received
	    memmove(recvBuffer,
		    recvBuffer + endOfHeaderPosition + len,
		    recvBufferPosition - len - endOfHeaderPosition);
	    recvBufferPosition = recvBufferPosition - len - endOfHeaderPosition;
	  }
	  else
	  {
	    // not over yet
	    //leave parseposition where it is
	    //move the extra (parsePosition = endOfHeaderPosition)
	    memmove(recvBuffer+parsePosition,
		    recvBuffer + endOfHeaderPosition + len,
		    recvBufferPosition - len - endOfHeaderPosition);
	    recvBufferPosition = recvBufferPosition - len;
	  }
	  binaryBuffer = 0;
	  binaryMode = false;

	  //Reenter loop.
	  continue;
	}
	else
	{
	  //Not finished receiving binary.
	  recvBufferPosition = endOfHeaderPosition;
	  return;
	}
      }
      else
      {
	// Not in binary mode.
	char* endline =
	  static_cast<char*> (memchr(recvBuffer+parsePosition, '\n',
				     recvBufferPosition - parsePosition));
	if (!endline)
	  return; //no new end of command/start of binary: wait

	if (parsePosition == 0) // parse header
	{
	  // Ignore empty lines.
	  if (endline == recvBuffer)
	  {
	    memmove(recvBuffer, recvBuffer+1, recvBufferPosition - 1);
	    recvBufferPosition--;
	    continue;
	  }
	  int found = sscanf(recvBuffer, "[%d:%64[A-Za-z0-9_.]]",
			     &currentTimestamp, currentTag);
	  if (found != 2)
	  {
	    found = sscanf(recvBuffer, "[%d]", &currentTimestamp);
	    if (found == 1)
	      currentTag[0] = 0;
	    else // failure
	    {
	      std::cerr << "UAbstractClient::read, fatal error parsing header: "
			<< recvBuffer << std::endl;
	      currentTimestamp = 0;
	      strcpy(currentTag, "UNKNWN");
	      //listLock.lock();
	      UMessage msg(*this, 0, URBI_ERROR_TAG,
			   "!!! UAbstractClient::read, fatal error parsing header",
			   std::list<BinaryData>());
	      notifyCallbacks(msg);
	      //unlistLock.lock();
	    }
	  }

	  currentCommand = strstr(recvBuffer, "]");
	  if (!currentCommand) {
	    //reset all
	    nBracket = 0;
	    inString = false;
	    parsePosition = 0;
	    recvBufferPosition = 0;
	    return;
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

	while (parsePosition < recvBufferPosition)
	{
	  if (inString)
	  {
	    if (recvBuffer[parsePosition]=='\\')
	    {
	      if (parsePosition == recvBufferPosition-1)
	      {
		//we cant handle the '\\'
		return;
	      }
	      parsePosition+=2; //ignore next character
	      continue;
	    }
	    if (recvBuffer[parsePosition]=='"')
	    {
	      inString = false;
	      ++parsePosition;
	      continue;
	    }
	  }
	  else
	  {
	    if (recvBuffer[parsePosition]=='"')
	    {
	      inString = true;
	      ++parsePosition;
	      continue;
	    }
	    if (recvBuffer[parsePosition]=='[')
	    {
	      ++nBracket;
	      ++parsePosition;
	      continue;
	    }
	    if (recvBuffer[parsePosition]==']')
	    {
	      --nBracket;
	      ++parsePosition;
	      continue;
	    }
	    if (recvBuffer[parsePosition]=='\n')
	    {
	      if (true /*XXX: handle '[' in echoed messages or errors nBracket == 0*/)
	      {
		//end of command
		recvBuffer[parsePosition]=0;
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
		while (!bins.empty())
		{
		  free(bins.front().data);
		  bins.pop_front();
		}
		break; //restart
	      }
	      //this should not happen: \n should have been handled by binary code below
	      std::cerr << "FATAL PARSE ERROR" << std::endl;
	    }
	    if (!system && !strncmp(recvBuffer+parsePosition-3, "BIN ", 4))
	    {
	      //very important: scan starts below current point
	      //compute length
	      char * endLength;
	      binaryBufferLength = strtol(recvBuffer+parsePosition+1, &endLength, 0);
	      if (endLength == recvBuffer+parsePosition+1)
	      {
		std::cerr << "UClient::read, error parsing bin data length."
			  << std::endl;
		recvBufferPosition = 0;
		return;
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
	  ++parsePosition;
	}
	//either we ate all characters, or we were asked to restart
	if (parsePosition == recvBufferPosition)
	  return;
	continue;
      }
    }
  }

  UCallbackID UAbstractClient::setWildcardCallback(UCallbackWrapper& callback)
  {
    return addCallback(URBI_WILDCARD_TAG, callback);
  }

  UCallbackID UAbstractClient::setErrorCallback(UCallbackWrapper& callback)
  {
    return addCallback(URBI_ERROR_TAG, callback);
  }

  UCallbackID UAbstractClient::setClientErrorCallback(UCallbackWrapper& callback)
  {
    return addCallback(CLIENTERROR_TAG, callback);
  }

  UCallbackID UAbstractClient::setCallback(UCallbackWrapper& callback,
					   const char* tag)
  {
    return addCallback(tag, callback);
  }

  UCallbackID UAbstractClient::addCallback(const char* tag,
					   UCallbackWrapper& w)
  {
    listLock.lock();
    UCallbackInfo ci(w);
    strncpy(ci.tag, tag, URBI_MAX_TAG_LENGTH-1);
    ci.tag[URBI_MAX_TAG_LENGTH-1]=0;
    ci.id = ++nextId;
    callbackList.push_front(ci);
    listLock.unlock();
    return ci.id;
  }

  /*! Generates a client error message and send it to listeners
  \param message an optional string describing the error
  \param erc an optional system error code on which strerror is called
  */
  void
  UAbstractClient::clientError(const char * message, int erc)
  {
    UMessage m(*this);
    m.type = MESSAGE_ERROR;
    std::string msg;
    if (message)
      msg = message;
    if (message && erc)
      msg += " : ";
    if (erc)
      msg += libport::strerror(erc);
    m.message = m.rawMessage = msg; //rawMessage is incorrect but we dont care
    m.timestamp = 0;
    m.tag = CLIENTERROR_TAG;
    notifyCallbacks(m);
  }

  UMessage::UMessage(UAbstractClient& client)
    : client(client), value(0)
  {
  }

  UMessage::UMessage(UAbstractClient& client, int timestamp,
		     const char *tag, const char *message,
		     std::list<BinaryData> bins)
    : client(client), timestamp(timestamp),  tag(tag), value(0)
  {
    rawMessage = std::string(message);
    while (message[0] ==' ')
      ++message;
    //parse non-value messages
    if (message[0] == '*')
    {
      //system message
      type = MESSAGE_SYSTEM;
      if (strlen(message) >= 4)
	this->message = (std::string)(message+4);
      return;
    }

    if (message[0] == '!')
    {
      //error message
      type = MESSAGE_ERROR;
      if (strlen(message) >= 4)
	this->message = (std::string)(message+4);
      return;
    }

    //value
    type = MESSAGE_DATA;
    value = new UValue();
    std::list<BinaryData>::iterator iter = bins.begin();
    int p = value->parse(message, 0, bins, iter);
    while (message[p] == ' ')
      ++p;
    /* no assertion can be made on message[p] because there is no terminator
     * for binaries */
    if (p < 0 || /*message[p] ||*/ iter != bins.end())
    {
      std::cerr << "PARSE ERROR in " << message << "at " << abs(p) << std::endl;
    }
  }

  UMessage::UMessage(const UMessage& b)
    : client(b.client)
  {
    rawMessage = b.rawMessage;
    timestamp = b.timestamp;
    tag = b.tag;
    type = b.type;
    value = 0;
    switch (type)
    {
      case MESSAGE_SYSTEM:
      case MESSAGE_ERROR:
	message = b.message;
	break;
      default:
	value = new UValue(*b.value);
	break;
    }
  }


  UMessage::~UMessage()
  {
    if (type != MESSAGE_SYSTEM && type != MESSAGE_ERROR && value)
      delete value;
  }

  std::ostream& operator <<(std::ostream &s, const UMessage &m)
  {
    s<<"["<<m.timestamp<<":"<<m.tag<<"] ";
    switch (m.type)
    {
      case MESSAGE_SYSTEM:
      case MESSAGE_ERROR:
	s<<m.message;
	break;
      default:
	s<<*m.value;
	break;
    }
    return s;
  }

  UClient * defaultClient=0;

  UClient * getDefaultClient()
  {
    return defaultClient;
  }

  void setDefaultClient(UClient * cl)
  {
    defaultClient = cl;
  }

  std::ostream& unarmorAndSend(const char * a)
  {
    std::ostream& s =
      (getDefaultClient()==0
       ? std::cerr
       : ((UAbstractClient*)getDefaultClient())->getStream());
    if (strlen(a)>2)
      if (a[0]=='(' && a[strlen(a)-1]==')')
	s.rdbuf()->sputn(a+1, strlen(a)-2);
      else
	s << a; //this is baaad, user forgot the parenthesis but was lucky
    return s;
  }

} // namespace urbi
