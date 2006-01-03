/*! \file uabstractclient.cpp
****************************************************************************
 * $Id: uabstractclient.cpp,v 1.13 2005/09/30 17:48:00 nottale Exp $
 *
 * Definition of the URBI interface class
 *
 * Copyright (C) 2004 Jean-Christophe Baillie.  All rights reserved.
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

#ifdef WIN32
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <setjmp.h>
#include <algorithm>
#include <sys/stat.h>
#include <iostream>

extern "C"
{
#include "../../lib/jpeg-6b/jpeglib.h"
#include "../../lib/jpeg-6b/jerror.h"
}

#include "uabstractclient.h"
#define DEBUG 0
using std::min;
#if DEBUG
//
//#include <sys/time.h>
static int mtime()
{
  return 12;
  /*
  static int base = 0;
  timeval tv;
  gettimeofday(&tv, NULL);
  int tme = tv.tv_sec * 1000 + (tv.tv_usec / 1000);
  if (!base)
    base = tme;
    return tme - base;*/
}
#endif

//inline int min(int a, int b) {if (a<b) return a; else return b;}
#define URBI_ERROR_TAG "[error]"
#define URBI_WILDCARD_TAG "[wildcard]"
enum UCallbackType
{
  UCB_,
  UCB_C,
};



static UCallbackID nextId;

class UCallbackWrapperCB: public UCallbackWrapper {
  UCallback cb;
 public:
  UCallbackWrapperCB(UCallback cb): cb(cb) {}
  virtual UCallbackAction operator ()(const UMessage & msg) {
    return cb(msg);
  }
};


class UCallbackWrapperCCB: public UCallbackWrapper {
  UCustomCallback cb;
  void * data;
 public:
  UCallbackWrapperCCB(UCustomCallback cb, void * data): cb(cb), data(data) {}
  virtual UCallbackAction operator ()(const UMessage & msg) {
    return cb(data, msg);
  }
};

static void *read_jpeg(const char *jpgbuffer, int jpgbuffer_size, 
                       bool RGB, int &output_size);


class UClientStreambuf: public std::streambuf {
 public:
  UClientStreambuf(UAbstractClient * cl): client(cl) {}
 protected:
  virtual int overflow ( int c = EOF );
  virtual std::streamsize xsputn (char * s, std::streamsize n);
 private:
  UAbstractClient * client;
};

int UClientStreambuf::overflow ( int c ) {
  if (c != EOF) {
    char ch = c;
    xsputn(&ch, 1);
  }
  return c;
}

std::streamsize UClientStreambuf::xsputn (char * s, std::streamsize n) {
  client->lockSend();
  if (strlen(client->sendBuffer)+1+n > client->buflen) {
    //error
    client->unlockSend();
    return 0;
  }
  int clen = strlen(client->sendBuffer);
  memcpy(client->sendBuffer+clen, s, n);
  client->sendBuffer[clen+n] = 0;
  if (strchr(client->sendBuffer,'&') ||
      strchr(client->sendBuffer,'|') ||
      strchr(client->sendBuffer,';') ||
      strchr(client->sendBuffer,',')) {
    client->effectiveSend(client->sendBuffer, strlen(client->sendBuffer));
    client->sendBuffer[0] = 0;
  }
  client->unlockSend();
  return n;
}



/*! Pass the given UMessage to all registered callbacks with the corresponding tag, 
  as if it were comming from the URBI server.
 */
void 
UAbstractClient::notifyCallbacks(const UMessage &msg) {
  lockList();
  bool inc=false;
  for (list<UCallbackInfo>::iterator it = callbackList.begin(); it!=callbackList.end(); inc?it:it++, inc=false) {
    if ( 
	(!strcmp(msg.tag.c_str(), it->tag)) ||
	(!strcmp(it->tag, URBI_ERROR_TAG) && msg.type == MESSAGE_ERROR) ||
	(!strcmp(it->tag, URBI_WILDCARD_TAG)) 
	) {
      UCallbackAction ua = it->callback(msg);
      if (ua == URBI_REMOVE) {
	delete &(it->callback);
        it=callbackList.erase(it);
        inc = true;
      }
    }
  }
  unlockList();
}

/*! Initializes sendBuffer and recvBuffer, and copy _host and _port.
  \param _host IP address or name of the robot to connect to.
  \param _port TCP port to connect to.
  \param _buflen size of send and receive buffers.
  Implementations should establish the connection in their constructor.
 */
UAbstractClient::UAbstractClient(const char *_host, int _port, int _buflen) 
  :std::ostream(new UClientStreambuf(this)) {
    stream=this; 
    getStream().setf(std::ios::fixed); 
    rc = 0;
    uid = 0;
    host = NULL;
    recvBuffer = NULL;
    recvBufferPosition = 0;
    binaryBuffer = NULL;
    host = new char[strlen(_host) + 1];
    if (!host) {
      rc = -1;
      return;
    }
    strcpy(host, _host);
    port = _port;
    buflen = _buflen;
    
    
    recvBuffer = new char[buflen];
    if (!recvBuffer) {
      rc = -1;
      //printf("UAbstractClient::UAbstractClient out of memory\n");
      return;
    }
    recvBuffer[0] = 0;
    
    sendBuffer = new char[buflen];
    if (!sendBuffer) {
      rc = -1;
      //printf("UAbstractClient::UAbstractClient out of memory\n");
      return;
    }
    sendBuffer[0] = 0;
 

  }

UAbstractClient::~UAbstractClient()
{
  if (host) delete []  host;
  if (recvBuffer) delete [] recvBuffer;
  if (sendBuffer) delete [] sendBuffer;
}

/*! In threaded environnments, this function locks the send buffer so that only
  the calling thread can call the send functions. Otherwise do nothing.
 */
int
UAbstractClient::startPack()
{
  lockSend();
  return 0;
}

int
UAbstractClient::endPack()
{
  int retval = effectiveSend(sendBuffer, strlen(sendBuffer));
  sendBuffer[0] = 0;
  unlockSend();
  return retval;
}


/*! Multiple commands can be sent in one call.
 */
int
UAbstractClient::send(const char *command, ...)
{
  if (rc) return -1;
  va_list arg;
  va_start(arg, command);
  lockSend();
  rc = vpack(command, arg);
  va_end(arg);
  if (rc < 0) {
    unlockSend();
    return (rc);
  }
  rc = effectiveSend(sendBuffer, strlen(sendBuffer));
  sendBuffer[0] = 0;
  unlockSend();
  return rc;
}

/*! This function must only be called between a startPack() 
  and the corresponding endPack(). Data is queued in the send
  buffer, and sent when endPack() is called.
 */
int 
UAbstractClient::pack(const char *command, ...)
{
  if (rc) return -1;
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
  if (rc) return -1;
#if 0  //disabled, crashes
  int size = vsnprintf(NULL, 0, command, arg);
  va_end(arg);
  if (strlen(sendBuffer) + size + 1 > buflen)
    return (-1);
  else {
#endif
    lockSend();
    vsprintf(&sendBuffer[strlen(sendBuffer)], command, arg);
    unlockSend();
    va_end(arg);
    return (0);
#if 0
  }
#endif
}


int 
UAbstractClient::sendFile(const char *name)
{
  if (rc) return -1;
  FILE *fd;
  fd = fopen(name, "r");
  if (fd == NULL)
    return (-1);
  int size;	
  struct stat s;
  stat(name, &s);
  size = s.st_size;
  lockSend();
  if (!canSend(size)) {
	unlockSend();
	return -1;
  }

  while (!feof(fd)) {
    int res = fread( sendBuffer, 1, buflen, fd);
	effectiveSend(sendBuffer, res);
  }
  fclose(fd);
  sendBuffer[0] = 0;
  unlockSend();
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
  if (rc) return -1;
  lockSend();
  if (header) {
    va_list arg;
    va_start(arg, header);
    vpack(header, arg);
    va_end(arg);
	if (!canSend(strlen(sendBuffer) + len)) {
	  unlockSend();
	  return -1;
	}

	effectiveSend(sendBuffer, strlen(sendBuffer));
  }

  int res = effectiveSend(buffer, len);
  sendBuffer[0] = 0;
  unlockSend();
  return res;
}

struct sendSoundData {
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

struct wavheader {
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

 static UCallbackAction sendSound_(void * cb, const UMessage &msg) {
  //the idea is to cut the sound into small chunks,
  //add a header and send each chunk separately
  //create the header.
   // printf("sound message: %s %d\n", msg.systemValue, msg.type);
  static const int CHUNK_SIZE = 32 * 8*60;
  static const int SUBCHUNK_SIZE = CHUNK_SIZE; //1024;
  

   sendSoundData *s=(sendSoundData *)cb;
   /*
   if (msg.type != MESSAGE_SYSTEM)
     return URBI_CONTINUE;
   if (strstr(msg.systemValue,"start") && s->startNotify==false) {
     s->startNotify = true;
     s->uc->notifyCallbacks(UMessage(*s->uc, 0,s->tag, "*** start"));
   }
   if (!strstr(msg.systemValue,"stop"))
     return URBI_CONTINUE;
   */
/*wavheader wh = { 
	{'R','I','F','F'}, 
	44-8, 
	{'W','A','V','E'},
	{'f','m','t',' '},
	16,1,1, 16000, 32000, 2, 16, 
	{'d','a','t','a'},
	0}; // no comment...
 */
  //handle next chunk 
 if (s->format == SOUND_WAV && s->pos==0)
   s->pos = sizeof(wavheader);
  int tosend = (s->length-s->pos > CHUNK_SIZE) ? CHUNK_SIZE:s->length-s->pos;
 
  //printf("%d start chunk of size %d at offset %d\n", 0,tosend,s->pos);
  int playlength = tosend *1000 / s->bytespersec;
  s->uc->send("%s.val = BIN %d %s %s;", 
              s->device,
              (int)(tosend+ ((s->format == SOUND_WAV)?sizeof(wavheader):0)), 
              (s->format == SOUND_WAV)?"wav":"raw",
              s->formatString
              );
  if (s->format == SOUND_WAV) {
    wavheader wh;
    memcpy(&wh, s->buffer, sizeof(wh));
    wh.datalength=tosend;
    wh.length=tosend+44-8;
    s->uc->sendBin(&wh, sizeof(wavheader));
  }
 


  /* this appears to be useless
  int msecpause=((SUBCHUNK_SIZE/32)*10)/14;
  int spos=0;
  while (spos!=tosend) {
	int ts = SUBCHUNK_SIZE;
	if (ts>tosend-spos) ts=tosend-spos;
	printf("%d chunk\n",mtime());
	s->uc->sendBin(s->buffer, ts);
	s->buffer+=ts;
	spos+=ts;
	  usleep(msecpause);
  }
  */

  s->uc->sendBin(s->buffer+s->pos, tosend);
   s->uc->send("wait(%s.remain < %d); %s: ping;", s->device, playlength/2, msg.tag.c_str());
  // printf("%d end sending chunk\n", 0);
  s->pos+=tosend;
  if (s->pos >= s->length ) {
    
    //printf("over: %d %d\n",URBI_REMOVE,URBI_CONTINUE);
    //if (s->tag && s->tag[0]) 
    //  s->uc->notifyCallbacks(UMessage(*s->uc, 0,s->tag, "*** stop"));
    
    s->uc->send("speaker->blend=speaker.sendsoundsaveblend;");
    if (s->tag && s->tag[0]) 
      s->uc->send("%s: 1;", s->tag);
    free(s->buffer);
    if (s->tag)
      free(s->tag);
    free(s->device);
    delete s;   
    return URBI_REMOVE;
  }
  return URBI_CONTINUE;
}

/*! 
  If tag is set, an URBI system "stop" message with this tag will be generated when the sound has been played.  
  The sound data is copied in case of asynchronous send, and may be safely deleted as soon as this
  function returns.
 */
int 
UAbstractClient::sendSound(const char * device, const USound &sound, const char *tag) {
  
  if (sound.soundFormat ==  SOUND_MP3) {
    //we don't handle chunkuing for this format
    return sendBin(sound.data, sound.size, "%s +report:  %s.val = BIN %d mp3;", tag, device, sound.size);
  }
  if (sound.soundFormat ==  SOUND_OGG) {
    //we don't handle chunkuing for this format
    return sendBin(sound.data, sound.size, "%s +report:  %s.val = BIN %d ogg;", tag, device, sound.size);
 }

  if (sound.soundFormat == SOUND_WAV || sound.soundFormat == SOUND_RAW) {
    send("speaker.sendsoundsaveblend = speaker->blend;speaker->blend=queue;");
    sendSoundData *s=new sendSoundData();
    char utag[16];
    makeUniqueTag(utag);
    s->bytespersec = sound.channels * sound.rate * (sound.sampleSize/8);
    s->uc=this;
    s->buffer = (char *)malloc(sound.size);
    memcpy(s->buffer, sound.data, sound.size);
    s->length=sound.size;
    s->tag=strdup(tag);
    s->device = strdup(device);
    s->pos=0;
    s->format = sound.soundFormat;
    if ( sound.soundFormat == SOUND_RAW)
      sprintf(s->formatString,"%d %d %d %d", sound.channels, sound.rate, sound.sampleSize, (int)sound.sampleFormat);
    else
      s->formatString[0] = 0;
    s->startNotify = false;
    UCallbackID cid=setCallback(sendSound_, s,utag);
    //invoke it 2 times to queue sound
    if (sendSound_(s,UMessage(*this, 0, utag,"*** stop", list<BinaryData>()))==URBI_CONTINUE) {
      if ( sendSound_(s,UMessage(*this, 0, utag,"*** stop", list<BinaryData>()))==URBI_REMOVE) {
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
UAbstractClient::setCallback(UCustomCallback cb, void *cbData, const char *tag)
{
  return addCallback(tag, *new UCallbackWrapperCCB(cb, cbData));
}

/*! Returns 1 and fills tag on success, 0 on failure
 */
int
UAbstractClient::getAssociatedTag(UCallbackID id, char * tag) {
 lockList();
  list<UCallbackInfo>:: iterator it = find(callbackList.begin(), callbackList.end(), id);
  if (it == callbackList.end()) {
    unlockList();
    return 0;
  }
  strcpy(tag, it->tag);
  unlockList();
  return 1;
}


/*! Returns 0 if no callback with this id was found, 1 otherwise.
 */
int 
UAbstractClient::deleteCallback(UCallbackID callbackID)
{
  lockList();
  list<UCallbackInfo>:: iterator it = find(callbackList.begin(), callbackList.end(), callbackID);
  if (it == callbackList.end()) {
    unlockList();
    return 0;
  }
  delete &(it->callback);
  callbackList.erase(it);
  unlockList();
  return 1;
}

UCallbackID 
UAbstractClient::sendCommand(UCallback cb, const char *cmd, ...)
{
  char tag[16];
  makeUniqueTag(tag);
  char *mcmd = new char[strlen(cmd) + strlen(tag) + 5];
  sprintf(mcmd,"%s: %s",tag,cmd);
  UCallbackID cid = setCallback(cb, tag);
  lockSend();
  va_list arg;
  va_start(arg, cmd);
  vpack(mcmd, arg);
  va_end(arg);
  int retval = effectiveSend(sendBuffer, strlen(sendBuffer));
  sendBuffer[0] = 0;
  unlockSend();
  delete []  mcmd;
  if (retval) {
	deleteCallback(cid);
	return UINVALIDCALLBACKID;
  }
  return cid;
}

UCallbackID 
UAbstractClient::sendCommand(UCustomCallback cb, void *cbData, 
                             const char *cmd,...)
{
  char tag[16];
  makeUniqueTag(tag);
  char *mcmd = new char[strlen(cmd) + strlen(tag) + 10];
  sprintf(mcmd,"%s: %s",tag,cmd);
  UCallbackID cid = setCallback(cb, cbData, tag);
  lockSend();
  va_list arg;
  va_start(arg, cmd);
  vpack(mcmd, arg);
  va_end(arg);
  int retval = effectiveSend(sendBuffer, strlen(sendBuffer));
  sendBuffer[0] = 0;
  unlockSend();
  delete []mcmd;
  if (retval) {
	deleteCallback(cid);
	return UINVALIDCALLBACKID;
  }
  return cid;

}

int 
UAbstractClient::putFile(const char * localName, const char * remoteName) {
  int len;
  struct stat st;
  if (stat(localName,&st) == -1) return 1;
  len = st.st_size;
  lockSend();
  if (!canSend(len+strlen(remoteName)+ 20)) {
	unlockSend();
	return -1;
  }

  if (!remoteName) remoteName = localName;
  send("save(\"%s\", \"",remoteName);
  int res = sendFile(localName);
  send("\");");
  unlockSend();
  return res;
}

int 
UAbstractClient::putFile(const void * buffer, int length, 
                         const char * remoteName) {
  if (!canSend(length+strlen(remoteName)+ 20)) {
	unlockSend();
	return -1;
  }
  send("save(\"%s\", \"",remoteName);
  sendBin(buffer,length);
  send("\");");
  unlockSend();
  return 0;
}

void
UAbstractClient::makeUniqueTag(char *tag)
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
  static bool binaryMode = false;
  char *endline;
  char *endline2;
  while (true) {

	if (binaryMode) {
	  
	  //Receiving binary. Append to binaryBuffer;
	  int len = min( recvBufferPosition - endOfHeaderPosition, 
			 binaryBufferLength - binaryBufferPosition);
	  if (binaryBuffer)
	    memcpy( (char *)binaryBuffer + binaryBufferPosition, 
		    recvBuffer + endOfHeaderPosition, len);
	  binaryBufferPosition += len;
	  
	  if (binaryBufferPosition == binaryBufferLength) {
	    //Finished receiving binary.
	    //append
	    BinaryData bd;
	    bd.size = binaryBufferLength;
	    bd.data = binaryBuffer;
	    bins.push_back(bd);
	    binaryBuffer = 0;
	    
	    if (nBracket == 0) {
	      //end of command, send
	      lockList();
	      UMessage msg(*this, currentTimestamp, currentTag, currentCommand,
			   bins);
	      notifyCallbacks(msg); 
	      unlockList();
	      
	      while (!bins.empty()) {
		free(bins.front().data);
		bins.pop_front();
	      }
	    }
	    
	    binaryBuffer = 0;
	    binaryMode = false;
	    parsePosition = 0;
	    //Move the extra we received
	    memmove(recvBuffer, 
		    recvBuffer + endOfHeaderPosition + len,  
		    recvBufferPosition - len - endOfHeaderPosition);
	    recvBufferPosition = recvBufferPosition - len - endOfHeaderPosition;
		
	    //Reenter loop.
	    continue;
	  }
	  else {
	    //Not finished receiving binary.
		recvBufferPosition = endOfHeaderPosition;
		return;
	  }
	}
	
	else {
	  //Not in binary mode.
	  endline = (char *) memchr(recvBuffer+parsePosition, '\n', recvBufferPosition);
	  if (!endline) 
	    return; //no new end of command/start of binary: wait
	  
	  //check
	  /*
	    printf("ding %15s\n", recvBuffer);
	  endline2 = (char *) memchr(recvBuffer, 0, recvBufferPosition);
	  if ( (unsigned int)endline - (unsigned int)recvBuffer > 50)
	    printf("WARNING, header unexpectedly long\n");
	  if ( (unsigned int)endline > (unsigned int)endline2 && endline2)
	    printf("WARNING, 0 before newline\n");
	  */
	  

	  //parse the line
#if DEBUG
	  printf("%d parse line: --%s--\n", mtime(), recvBuffer);
#endif
	  if (parsePosition == 0) {//parse header
	    int found = sscanf(recvBuffer, "[%d:%64[A-Za-z0-9_]]", 
			       &currentTimestamp, currentTag);
	    if (found != 2) {
	      found = sscanf(recvBuffer, "[%d]", &currentTimestamp);
	      if (found == 1)
		  currentTag[0] = 0;
	      else {	//failure
		printf("UAbstractClient::read, fatal error parsing header");
		  printf(" line was '%s'\n", recvBuffer);
		  currentTimestamp = 0;
		  strcpy(currentTag, "UNKNWN");
		  lockList();
		  UMessage msg(*this, 0, URBI_ERROR_TAG, 
			       "!!! UAbstractClient::read, fatal error parsing header", 
			       list<BinaryData>());
		  notifyCallbacks(msg);
		  unlockList();
	      }
	    }
	    
	    currentCommand = strstr(recvBuffer, "]");
	    
	    currentCommand++;
	    parsePosition = (long)currentCommand - (long)recvBuffer;

	    //reinit just to be sure:
	    nBracket = 0;
	    inString = false;
	  }

	  while (parsePosition < recvBufferPosition) {
	    if (inString) {
	      if (recvBuffer[parsePosition]=='\\') {
		if (parsePosition == recvBufferPosition-1) {
		  //we cant handle the \\
		  return;
		}
		parsePosition+=2; //ignore next character
		continue;
	      }
	      if (recvBuffer[parsePosition]=='"') {
		inString = false;
		parsePosition++;
		continue;
	      }
	    }
	    else {
	      if (recvBuffer[parsePosition]=='"') {
		inString = true;
		parsePosition++;
		continue;
	      }
	      if (recvBuffer[parsePosition]=='[') {
		nBracket++;
		parsePosition++;
		continue;
	      }
	      if (recvBuffer[parsePosition]==']') {
		nBracket--;
		parsePosition++;
		continue;
	      }
	      if (recvBuffer[parsePosition]=='\n') {
		if (nBracket == 0) {
		  //end of command
		  recvBuffer[parsePosition]=0;
		  lockList();
		  UMessage msg(*this, currentTimestamp, currentTag, 
			       currentCommand, 
			       bins);
		  notifyCallbacks(msg);
		  unlockList();
		  //prepare for next read, copy the extra
		  memmove(recvBuffer, recvBuffer+parsePosition+1, recvBufferPosition-parsePosition-1);	//copy beginning of next cmd
		  recvBufferPosition = recvBufferPosition-parsePosition-1;
		  recvBuffer[recvBufferPosition] = 0;
		  parsePosition = 0;
		  while (!bins.empty()) {
		    free(bins.front().data);
		    bins.pop_front();
		  }
		  break; //restart
		}
		//this should not happen: \n should have been handled by binary code below
		fprintf(stderr,"FATAL PARSE ERROR\n");
	      }
	      if (!strncmp(recvBuffer+parsePosition-3, "BIN ", 4)) { //very important: scan starts below current point
		//compute length
		char * endLength;
		binaryBufferLength = strtol(recvBuffer+parsePosition+1,&endLength, 0);
		if (endLength == recvBuffer+parsePosition+1) {
		  printf("UClient::read, error parsing bin data length.\n");
		  recvBufferPosition = 0;
		  return;
		}
		//go to end of header
		while (recvBuffer[parsePosition] !='\n')
		  parsePosition++; //we now we will find a \n
		parsePosition++;
		endOfHeaderPosition = parsePosition;
		binaryMode = true;
		binaryBuffer = malloc(binaryBufferLength);
		binaryBufferPosition = 0;
		break; //restart in binarymode to handle binary
	      }
	    } //not in string mode
	  } //end while
	  //either we ate all characters, or we were asked to restart
	  if (parsePosition == recvBufferPosition)
	    return;
	  continue;	  
	} //en else (!binaryMode)
  }// end while
}

UCallbackID UAbstractClient::setWildcardCallback(UCallbackWrapper & callback) {
  return addCallback(URBI_WILDCARD_TAG, callback);
}

UCallbackID UAbstractClient::setErrorCallback(UCallbackWrapper & callback) {
  return addCallback(URBI_ERROR_TAG, callback);
}
UCallbackID UAbstractClient::setCallback(UCallbackWrapper & callback, const char * tag) {
  return addCallback(tag, callback);
}
UCallbackID UAbstractClient::addCallback(const char * tag, UCallbackWrapper &w) {
  lockList();
  UCallbackInfo ci(w);
  strncpy(ci.tag, tag, URBI_MAX_TAG_LENGTH-1);
  ci.tag[URBI_MAX_TAG_LENGTH-1]=0;
  ci.id = ++nextId;
  callbackList.push_front(ci);
  unlockList();
  return ci.id;
}


static inline unsigned char clamp(float v)
{
  if (v < 0)
    return 0;
  if (v > 255)
    return 255;
  return (unsigned char) v;
}


int 
convertRGBtoYCrCb(const byte * sourceImage, 
                  int bufferSize, 
                  byte * destinationImage)
{
  unsigned char *in = (unsigned char *) sourceImage;
  unsigned char *out = (unsigned char *) destinationImage;
  for (int i = 0; i < bufferSize - 2; i += 3) {
    float r = in[i];
    float g = in[i + 1];
    float b = in[i + 2];
    /*
      Y  =      (0.257 * R) + (0.504 * G) + (0.098 * B) + 16
      Cr = V =  (0.439 * R) - (0.368 * G) - (0.071 * B) + 128
      Cb = U = -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128
    */ 
    out[i] = clamp((0.257 * r) + (0.504 * g) + (0.098 * b) + 16);
    out[i + 1] = clamp((0.439 * r) - (0.368 * g) - (0.071 * b) + 128);
    out[i + 2] = clamp(-(0.148 * r) - (0.291 * g) + (0.439 * b) + 128);
  }
  return 1;
}

int 
convertYCrCbtoRGB(const byte * sourceImage, 
                  int bufferSize, 
                  byte * destinationImage)
{
  unsigned char *in = (unsigned char *) sourceImage;
  unsigned char *out = (unsigned char *) destinationImage;
  for (int i = 0; i < bufferSize - 2; i += 3) {
    float y = in[i];
    float cb = in[i + 1];
    float cr = in[i + 2];
    /*
       out[i+2]=clamp(y+1.403*cb);
       out[i+1]=clamp(y-0.344*cr-0.714*cb);
       out[i]=clamp(y+1.77*cr);
     */
    out[i] = clamp(1.164 * (y - 16) + 1.596 * (cr - 128));
    out[i + 1] = clamp(1.164 * (y - 16) - 0.813 * (cr - 128) - 
                       0.392 * (cb - 128));
    out[i + 2] = clamp(1.164 * (y - 16) + 2.017 * (cb - 128));
  }
  return 1;
}


int 
convertJPEGtoYCrCb(const byte * source, int sourcelen, byte * dest, 
                       int &size)
{
  int sz;
  void *destination = read_jpeg((const char *) source, sourcelen, false, sz);
  if (!destination) {
    size = 0;
    return 0;
  }
  int cplen = sz > size ? size : sz;
  memcpy(dest, destination, cplen);
  free(destination);
  size = sz;
  return 1;
}

int 
convertJPEGtoRGB(const byte * source, int sourcelen, byte * dest, int &size)
{
  int sz;
  void *destination = read_jpeg((const char *) source, sourcelen, true, sz);
  if (!destination) {
    size = 0;
    return 0;
  }
  int cplen = sz > size ? size : sz;
  memcpy(dest, destination, cplen);
  free(destination);
  size = sz;
  return 1;
}

struct mem_source_mgr
{
  struct jpeg_source_mgr pub;
  JOCTET eoi[2];
};

static void init_source(j_decompress_ptr cinfo)
{
}

static boolean fill_input_buffer(j_decompress_ptr cinfo)
{
  mem_source_mgr *src = (mem_source_mgr *) cinfo->src;
  if (src->pub.bytes_in_buffer != 0)
    return TRUE;
  src->eoi[0] = 0xFF;
  src->eoi[1] = JPEG_EOI;
  src->pub.bytes_in_buffer = 2;
  src->pub.next_input_byte = src->eoi;
  return TRUE;
}

static void term_source(j_decompress_ptr cinfo)
{
}

static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
  mem_source_mgr *src = (mem_source_mgr *) cinfo->src;
  if (num_bytes <= 0)
    return;
  if (num_bytes > src->pub.bytes_in_buffer)
    num_bytes = src->pub.bytes_in_buffer;
  src->pub.bytes_in_buffer -= num_bytes;
  src->pub.next_input_byte += num_bytes;
}


struct urbi_jpeg_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

METHODDEF(void)
urbi_jpeg_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  urbi_jpeg_error_mgr *  myerr = ( urbi_jpeg_error_mgr *) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}


/*! Convert a jpeg image to YCrCb or RGB. Allocate the buffer with malloc.
 */
static void *read_jpeg(const char *jpgbuffer, int jpgbuffer_size, bool RGB, 
                       int &output_size)
{
  struct jpeg_decompress_struct cinfo;
  struct urbi_jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = urbi_jpeg_error_exit;
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    printf( "JPEG error!\n"); 
    return 0;
  }
  jpeg_create_decompress(&cinfo);
  mem_source_mgr *source = (struct mem_source_mgr *)
    (*cinfo.mem->alloc_small) ((j_common_ptr) & cinfo, JPOOL_PERMANENT,
			       sizeof(mem_source_mgr));

  cinfo.src = (jpeg_source_mgr *) source;
  source->pub.skip_input_data = skip_input_data;
  source->pub.term_source = term_source;
  source->pub.init_source = init_source;
  source->pub.fill_input_buffer = fill_input_buffer;
  source->pub.resync_to_restart = jpeg_resync_to_restart;
  source->pub.bytes_in_buffer = jpgbuffer_size;
  source->pub.next_input_byte = (JOCTET *) jpgbuffer;
  cinfo.out_color_space = (RGB ? JCS_RGB : JCS_YCbCr);
  jpeg_read_header(&cinfo, TRUE);
  cinfo.out_color_space = (RGB ? JCS_RGB : JCS_YCbCr);
  jpeg_start_decompress(&cinfo);
  output_size = cinfo.output_width * 
    cinfo.output_components        * 
    cinfo.output_height;
  void *buffer = malloc(output_size);

  while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    JSAMPROW row_pointer[1];
    row_pointer[0] = (JOCTET *) & ((char *) buffer)[cinfo.output_scanline   * 
                                                    cinfo.output_components * 
                                                    cinfo.output_width];
    jpeg_read_scanlines(&cinfo, row_pointer, 1);
  }
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return buffer;
}


template<class S, class D> void copy(S* src, D* dst, int sc, int dc, int sr, int dr, int count, bool sf, bool df) {
  int shift = 8*(sizeof(S) - sizeof(D));
  for (int i=0;i<count;i++) {
    float soffset = (float)i* ((float)sr /(float)dr);
    int so = (int)soffset;
    float factor = soffset-(float)so;
    S s1, s2;
    s1 = src[so*sc];
    if (i != count - 1)
      s2 = src[(so+1)*sc]; 
    else
      s2 = s1; //nothing to interpolate with
    if (!sf) {
      s1 = s1 ^ (1<<(sizeof(S)*8-1));
      s2 = s2 ^ (1<<(sizeof(S)*8-1));   
    }
    int v1 = (int) ((float)(s1)*(1.0-factor) + (float)(s2)*factor);
    int v2;
    if (sc==1)
      v2 = v1;
    else {
      s1 = src[so*sc+1];
      if (i != count - 1)
	s2 = src[(so+1)*sc+1];
      else
	s2 = s1; //nothing to interpolate with
      if (!sf) {
        s1 = s1 ^ (1<<(sizeof(S)*8-1));
        s2 = s2 ^ (1<<(sizeof(S)*8-1));   
      }
       v2 = (int) ((float)(s1)*(1.0-factor) + (float)(s2)*factor);
    }
    D d1, d2;
    if (shift>=0) {
      d1 = (D)(v1 >>shift);
      d2 = (D)(v2 >>shift);
    }
    else {
      d1 = (D)(v1) *  (1<< (-shift));
      d2 = (D)(v2) * (1<< (-shift));
    }
    if (!df) {
      d1 = d1 ^ (1<<(sizeof(D)*8-1));
      d2 = d2 ^ (1<<(sizeof(D)*8-1));    
    }
    if (dc==2) {
      dst[i*2] = d1;
      dst[i*2+1] = d2;
    }
    else
      dst[i] = (D) (((int)d1+(int)d2) /2);
  }
}

/** Conversion between various sound formats.
    If any of destination,'s channel, sampleSize, rate or sampleFormat parameter is 0, values from source will be used.
    If the desitnation's datasize is too small, data will be realloc()ed, which means one can set data and datasize to zero, and let convert allocate the memory.
 */
int
convert (const USound &source, USound &dest) {
  if ( (source.soundFormat != SOUND_RAW && source.soundFormat != SOUND_WAV) ||
       (dest.soundFormat != SOUND_RAW && dest.soundFormat != SOUND_WAV))
    return 1; //conversion not handled yet
  //phase one: calculate required buffer size, set destination unspecified fields
  int schannels, srate, ssampleSize;
  USoundSampleFormat ssampleFormat;
  if (source.soundFormat == SOUND_WAV) {
    wavheader * wh = (wavheader *)source.data;
    schannels = wh->channels;
    srate = wh->freqechant;
    ssampleSize = wh->bitperchannel;
    ssampleFormat = (ssampleSize>8)?SAMPLE_SIGNED:SAMPLE_UNSIGNED;
  }
  else {
    schannels = source.channels;
    srate = source.rate;
    ssampleSize = source.sampleSize;
    ssampleFormat = source.sampleFormat;
  }
  if (!dest.channels) dest.channels = schannels;
  if (!dest.rate) dest.rate = srate;
  if (!dest.sampleSize) dest.sampleSize = ssampleSize;
  if (!(int)dest.sampleFormat) dest.sampleFormat = ssampleFormat;
  if (dest.soundFormat == SOUND_WAV) dest.sampleFormat = (dest.sampleSize>8)?SAMPLE_SIGNED:SAMPLE_UNSIGNED; 
  int destSize = (int) (( (long long)(source.size- ((source.soundFormat == SOUND_WAV)?44:0)) * (long long)dest.channels * (long long)dest.rate * (long long)(dest.sampleSize/8)) / ( (long long)schannels*(long long)srate*(long long)(ssampleSize/8)));
  if (dest.soundFormat == SOUND_WAV) destSize+= sizeof(wavheader);
  if (dest.size<destSize) 
    dest.data = (char *)realloc(dest.data, destSize);
  dest.size = destSize;
  //write destination header if appropriate
  if (dest.soundFormat == SOUND_WAV) {
    wavheader * wh = (wavheader *)dest.data;
    memcpy(wh->riff,"RIFF",4);
    wh->length = dest.size - 8;
    memcpy(wh->wave, "WAVE", 4);
    memcpy(wh->fmt, "fmt ", 4);
    wh->lnginfo = 16;
    wh->one = 1;
    wh->channels = dest.channels;
    wh->freqechant = dest.rate;
    wh->bytespersec = dest.rate * dest.channels * (dest.sampleSize/8);
    wh->bytesperechant = (dest.sampleSize/8)*dest.channels;
    wh->bitperchannel = dest.sampleSize;
    memcpy(wh->data, "data", 4);
    wh->datalength = destSize - sizeof(wavheader);
  }
  
  //do the conversion and write to dest.data
  char * sbuffer = source.data;
  if (source.soundFormat == SOUND_WAV)
    sbuffer += sizeof(wavheader);
  char * dbuffer = dest.data;
  if (dest.soundFormat == SOUND_WAV)
    dbuffer += sizeof(wavheader);
  int elementCount = dest.size - ((dest.soundFormat == SOUND_WAV)?sizeof(wavheader):0);
  elementCount /= (dest.channels * (dest.sampleSize/8));
  switch( ssampleSize*1000 + dest.sampleSize) {
  case 8008:
    copy(sbuffer, dbuffer, schannels, dest.channels, srate, dest.rate, elementCount, ssampleFormat==SAMPLE_SIGNED, dest.sampleFormat == SAMPLE_SIGNED);
    break;
  case 16008:
    copy((short *)sbuffer, dbuffer, schannels, dest.channels, srate, dest.rate, elementCount, ssampleFormat==SAMPLE_SIGNED, dest.sampleFormat == SAMPLE_SIGNED);
    break;
  case 16016:
    copy((short *)sbuffer, (short *)dbuffer, schannels, dest.channels, srate, dest.rate, elementCount, ssampleFormat==SAMPLE_SIGNED, dest.sampleFormat == SAMPLE_SIGNED);
    break;
  case 8016:
    copy((char *)sbuffer, (short *)dbuffer, schannels, dest.channels, srate, dest.rate, elementCount, ssampleFormat==SAMPLE_SIGNED, dest.sampleFormat == SAMPLE_SIGNED);
    break;
  }
  return 0;
}


//scale putting (scx,scy) at the center of destination image
static void scaleColorImage(unsigned char * src, int sw, int sh,  int scx, int scy, unsigned char * dst, int dw, int dh, float sx, float sy) {
  for (int x=0;x<dw;x++) 
    for (int y=0;y<dh;y++) {      
      //find the corresponding point in source image 
      float fsrcx = (float) (x-dw/2) / sx  + (float)scx;
      float fsrcy = (float) (y-dh/2) / sy  + (float)scy;
      int srcx = (int) fsrcx;
      int srcy = (int) fsrcy;
      if ( srcx<=0 || srcx>=sw-1 || srcy<=0 || srcy>=sh-1) 
	memset(dst+(x+y*dw)*3,0,3);
      else { //do the bilinear interpolation
	
	float xfactor = fsrcx-(float)srcx;
	float yfactor = fsrcy-(float)srcy;
	for (int color=0;color<3;color++) {
	  float up = (float)src[(srcx+srcy*sw)*3+color] * (1.0-xfactor) + (float)src[(srcx+1+srcy*sw)*3+color] * xfactor;
	  float down = (float)src[(srcx+(srcy+1)*sw)*3+color] * (1.0-xfactor) + (float)src[(srcx+1+(srcy+1)*sw)*3+color] * xfactor;
	  float result = up * (1.0-yfactor) + down * yfactor;
	  dst[(x+y*dw)*3+color] = (unsigned char)result;
	}
      }
    }
}




/** Convert between various image formats, takes care of everything
 */
int convert(const UImage & src, UImage & dest) {
  
  if (dest.width == 0)
    dest.width = src.width;
  if (dest.height == 0)
    dest.height = src.height;
  //step 1: uncompress source, to have raw uncompressed rgb or ycbcr
  void * uncompressedData=malloc(src.width*src.height*3);
  int usz = src.width*src.height*3;
  int format; //0 rgb 1 ycbcr
  int targetformat; //0 rgb 1 ycbcr 2 compressed
  
  switch(dest.imageFormat) {
  case IMAGE_RGB:
  case IMAGE_PPM:
    targetformat = 1;
    break;
  case IMAGE_YCbCr:
    targetformat = 0;
    break;
  case IMAGE_JPEG:
    targetformat = -1;
    break;
  }
  int p,c;
  switch(src.imageFormat) {
  case IMAGE_YCbCr:
    format = 1;
    memcpy(uncompressedData, src.data, src.width*src.height*3);
    break;
  case IMAGE_RGB:
    format = 0;
    memcpy(uncompressedData, src.data, src.width*src.height*3);
    break;
  case IMAGE_PPM:
    format = 0;
    //locate header end
    p=0;c=0;
    while(c<3) 
      if (src.data[p++]=='\n')
	c++;
    memcpy(src.data+p, uncompressedData, src.width*src.height*3);
    break;
  case IMAGE_JPEG:
    if (targetformat==0) {
      convertJPEGtoRGB((byte *)src.data,  src.size, (byte *)uncompressedData, usz);
      format = 0;
    }
    else {
      convertJPEGtoYCrCb((byte *)src.data,  src.size, (byte *)uncompressedData, usz);
      format = 1;
    }
    break;
  }


  //now resize if target size is different
  if (src.width != dest.width  ||  src.height != dest.height) {
    void * scaled = malloc(dest.width*dest.height*3);
    scaleColorImage((unsigned char *)uncompressedData, src.width, src.height, src.width/2, src.height/2,
		 (unsigned char *)scaled, dest.width, dest.height, 
		 (float)dest.width/(float)src.width, (float)dest.height/(float) src.height);
    free(uncompressedData);
    uncompressedData = scaled;
  }

  //then convert to destination format
  dest.data = (char *)realloc(dest.data, dest.width*dest.height*3+20);
  dest.size =  dest.width*dest.height*3+20;
  int dsz = dest.size;
  switch(dest.imageFormat) {
  case IMAGE_RGB:
    if (format == 1)
      convertYCrCbtoRGB((byte *)uncompressedData, dest.width*dest.height*3, (byte *)dest.data);
    else
      memcpy(dest.data, uncompressedData, dest.width*dest.height*3);
    break;
  case IMAGE_YCbCr:
    if (format == 0)
      convertRGBtoYCrCb((byte *)uncompressedData, dest.width*dest.height*3, (byte *)dest.data);
    else
      memcpy(dest.data, uncompressedData, dest.width*dest.height*3);
    break;
  case IMAGE_PPM:
    sprintf(dest.data, "P6\n%d %d\n255\n", dest.width, dest.height);
    if (format == 1)
      convertYCrCbtoRGB((byte *)uncompressedData, dest.width*dest.height*3, (byte *)dest.data+strlen(dest.data));
    else
      memcpy(dest.data+strlen(dest.data), uncompressedData, dest.width*dest.height*3);
    break;
  case IMAGE_JPEG:
    /*
      if (format == 1)
       convertYCrCbtoJPEG((byte *)uncompressedData, dest.width*dest.height*3, (byte *)dest.data, dsz);
     else 
     convertRGBtoJPEG((byte *)uncompressedData, dest.width*dest.height*3, (byte *)dest.data, dsz);
    */
    fprintf(stderr,"unsoported conversion requested: cant compress to jpeg\n");
    free(uncompressedData);
    return 0;
    break;
  }

  free(uncompressedData);
  return 1;
}


void unescape(string & data) {
  int src=0, dst=0;
  while (data[src]) {
    if (data[src]!='\\')
      data[dst]=data[src];
    else {
      switch(data[++src]) {
      case 'n':
	data[dst]='\n';
	break;
      case '\\':
	data[dst]='\\';
	break;
      case '"':
	data[dst]='"';
	break;
      default:
	data[dst]=data[src];
	break;
      }
    }
    src++;
    dst++;
  }
  data[dst] = 0;
  
}
void unescape(char * data) {
  char* src = data;
  char * dst = data;
  while (*src) {
    if (*src != '\\') 
      *dst = *src;
    
    else {
      switch (*(++src)) {
      case 'n':
	*dst = '\n';
	break;
      case '\\':
	*dst='\\';
	break;
      case '"':
	*dst = '"';
	break;
      default:
	*dst = *src;
      };
    }
    src++;
    dst++;
  }
  *dst = 0;
}

UMessage::UMessage(UAbstractClient & client, int timestamp,   char *tag, char *message, 
                    list<BinaryData> bins)
  : client(client), timestamp(timestamp),  tag(tag){
  while (message[0] ==' ') message++;
  //parse non-value messages
  if (message[0] == '*') {
      //system message
      type = MESSAGE_SYSTEM;
      this->message = message+3;
      return;
    }

  if (message[0] == '!') {
    //error message
    type = MESSAGE_ERROR;
    this->message = message+3;
    return;
  }

  //value
  type = MESSAGE_DATA;
  value = new UValue();
  list<BinaryData>::iterator iter = bins.begin();
  int p=value->parse(message,0, bins, iter);
  while (message[p]==' ') p++;
  if (p<0 || message[p] || iter != bins.end()) {
    std::cerr << "PARSE ERROR in "<<message<<"at "<<abs(p)<<std::endl;
  }
}


int UValue::parse(char * message, int pos, list<BinaryData> bins, list<BinaryData>::iterator &binpos) {
  while (message[pos]==' ')
    pos++;
  if (message[pos] == '"') {
    //string
    type = DATA_STRING;
    //get terminating '"'
    int p=pos+1;
    while (message[p] && message[p]!='"') {
      if (message[p]=='\\')
	p++;
      p++;
    }
    if (!message[p])
      return -p; //parse error

    stringValue = new string(message+pos+1, p-pos-1);
    unescape(*stringValue);
    return p+1;
  }

  if (message[pos] == '[') {
    //list message
    type = DATA_LIST;
    array = new UArray();
    pos++;
    while (message[pos]==' ') pos++;
    while (message[pos]) {
      while (message[pos]==' ') pos++;
      UValue *v = new UValue();
      int p = v->parse(message, pos, bins, binpos);
      if (p<0)
	return p;
      array->array.push_back(v);
      pos = p;
      while (message[pos]==' ') pos++;
      //expect , or rbracket
      if (message[pos]==']')
	break;
      if (message[pos]!=',')
	return -pos;
      pos++;
    }
    
    if (message[pos]!=']')
      return -pos;
    return pos+1;
  }

  //OBJ a [x:12, y:4]
  if (!strncmp(message+pos, "OBJ ", 4)) {
    //obj message
    pos+=4;
    type = DATA_OBJECT;
    object = new UNamedArray();

    //parse object name
    while (message[pos]==' ')
      pos++;
    
    int p = pos;
    while (message[p] && message[p]!=' ')
      p++;
    if (!message[p])
      return -p; //parse error
    object->refName = string(message+pos, p-pos);
    pos=p;

    
    while (message[pos]==' ')
      pos++;
    if (message[pos]!='[')
      return -pos;
    pos++;
    
    while (message[pos]) {
      while (message[pos]==' ') pos++;
      //parse name
      int p = pos;
      while (message[p] && message[p]!=':')
	p++;
      if (!message[p])
	return -p; //parse error
      p++;
      UNamedValue nv;
      nv.name = string(message+pos, p-pos-1);
      pos=p;
      while (message[pos]==' ') pos++;
      UValue *v = new UValue();
      p = v->parse(message, pos, bins, binpos);
      if (p<0)
	return p;
      nv.val = v;
      object->array.push_back(nv);
      pos = p;
      while (message[pos]==' ') pos++;
      //expect , or rbracket
      if (message[pos]==']')
	break;
      if (message[pos]!=',')
	return -pos;
      pos++;
    }
    
    if (message[pos]!=']')
      return -pos;
    return pos+1;
  }


      

  if (!strncmp(message+pos,"BIN ",4)) {
    //binary message: delegate
    type = DATA_BINARY;
    binary = new UBinary();
    pos +=4;
    //parsing type
    int p = binary->parse(message, pos, bins, binpos);
    return p;
  }
  
  //last attempt: double
  int p;
  int count = sscanf(message+pos, "%lf%n", &val, &p);
  if (!count) 
    return -pos;
  type = DATA_DOUBLE;
  pos +=p;
  return pos;

}


int UBinary::parse(char * message, int pos, list<BinaryData> bins, list<BinaryData>::iterator &binpos) {
  while (message[pos]==' ') pos++;
  //find end of header
 
  if( binpos == bins.end()) //no binary data available
    return -1;
  
  //validate size
  int ps,psize;
  int count = sscanf(message+pos,"%d%n",&psize,&ps);
  if (count!=1)
    return -pos;
  if (psize != binpos->size) {
    std::cerr <<"bin size inconsistency\n";
    return -pos;
  }
  pos +=ps;
  size = psize;
  data = malloc(psize);
  memcpy(data, binpos->data, size);
  binpos++;


 int p = pos;
  while (message[p] && message[p]!='\n')
    p++;
  if (!message[p])
    return -p; //parse error
  this->message = string(message+pos, p-pos);
  p++;

  //trying to parse header to find type
  char type[64];
  memset(type, 0, 64);
  int p1, p2, p3, p4, p5;
  count = sscanf(message+pos,"%63s %d %d %d %d", type, &p2, &p3, &p4, &p5);
  //DEBUG fprintf(stderr,"%s:  %d %s %d %d\n", message, p1, type, p2, p3);
  if (!strcmp(type, "jpeg")) {
    this->type = BINARY_IMAGE;
    image.size = size;
    image.width = p2;
    image.height = p3;
    image.imageFormat = IMAGE_JPEG;
    return p;
  }
 
  if (!strcmp(type, "YCbCr")) {
    this->type = BINARY_IMAGE;
    image.size = size;
    image.width = p2;
    image.height = p3;
    image.imageFormat = IMAGE_YCbCr;
    return p;
  }

  if (!strcmp(type, "rgb")) {
    this->type = BINARY_IMAGE;
    image.size = size;
    image.width = p2;
    image.height = p3;
    image.imageFormat = IMAGE_RGB;
    return p;
  }

  if (!strcmp(type, "raw")) {
    this->type = BINARY_SOUND;
    sound.soundFormat = SOUND_RAW;
    sound.size = size;
    sound.channels = p2;
    sound.rate = p3;
    sound.sampleSize = p4;
    sound.sampleFormat = (USoundSampleFormat) p5;
    return p;
  }

  if (!strcmp(type, "wav")) {
    this->type = BINARY_SOUND;
    sound.soundFormat = SOUND_WAV;
    sound.size = size;
    sound.channels = p2;
    sound.rate = p3;
    sound.sampleSize = p4;
    sound.sampleFormat = (USoundSampleFormat) p5;
    return p;
  }

  //unknown binary
  this->type = BINARY_UNKNOWN;
  return p;
 }


UMessage::UMessage(const UMessage &b)
  :client(b.client)
{

  timestamp = b.timestamp;
  tag = b.tag;
  type = b.type;
  
  switch(type) {
    
  case MESSAGE_SYSTEM:
  case MESSAGE_ERROR:
    message = b.message;
  
    break;
  default:
    value = new UValue(*b.value);
    break;
  }
}


UMessage::~UMessage() {
if (type != MESSAGE_SYSTEM && type != MESSAGE_ERROR)
  delete value;
}


UValue::UValue() : type(DATA_VOID) {}


UValue::UValue(double v) : val(v), type(DATA_DOUBLE)  {}
UValue::UValue(char * v) : stringValue(new string(v)), type(DATA_STRING)  {}
UValue::UValue(const string &v) : type(DATA_STRING), stringValue(new string(v)) {}
 
UValue::UValue(const UBinary &b) : type(DATA_BINARY){
  binary = new UBinary(b); 
}


UValue::~UValue() {
  switch(type) {
  case DATA_STRING:
    delete stringValue;
  case DATA_BINARY:
    if (binary)
      delete binary;
    break;
  case DATA_LIST:
    if (array)
      delete array;
    break;
  case DATA_OBJECT:
    if (object)
      delete object;
  }
}

UValue::operator double() {
  double v=0;
  switch( type) {
  case DATA_DOUBLE:
    return val;
    break;
  case DATA_STRING:
    sscanf(stringValue->c_str(),"%lf", &v);
    return v;
    break;
  };
  return 0;
};


UValue::operator string() {
  char str[254];
   switch( type) {
   case DATA_DOUBLE:
     sprintf(str,"%lf",val);
     return str;
     break;
   case DATA_STRING:
     return *stringValue;
     break;
   };
};

std::ostream & operator <<(std::ostream &s, const UValue &v) {
  switch( v.type) {
  case DATA_DOUBLE:
    s<< v.val;
    break;
  case DATA_STRING:
    s<< '"'<<*v.stringValue<<'"';
    break;
  case DATA_BINARY:
    s<<"BIN "<<v.binary->size<<" "<<v.binary->message<<";";
    s.write((char *)v.binary->data, v.binary->size);
    break;
  case DATA_LIST:
    {
      s<<"[";
      int sz = v.array->array.size();
      int p = 0;
      for (list<UValue * >::const_iterator it = v.array->array.begin(); it != v.array->array.end(); it++) {
	s << *(*it);
	if (++p != sz)
	  s<< " , ";
      }
      s<< "]";
    }
    break;
  case DATA_OBJECT:
    {
      s<<"OBJ "<<v.object->refName<<" [";
      int sz = v.object->array.size();
      int p = 0;
      for (list<UNamedValue>::const_iterator it = v.object->array.begin(); it != v.object->array.end(); it++) {
	s << it->name<<":"<< *(it->val);
	if (++p != sz)
	  s<< " , ";
      }
      s<< "]"; 
    }
  
    break;
  }
  return s;
}


void UValue::send(UAbstractClient *cl) {
  switch( type) {
  case DATA_DOUBLE:
    cl->send("%ld",val);
    break;
  case DATA_STRING:
    cl->send("%s",stringValue->c_str());
    break;
  case DATA_BINARY:
    cl->sendBin(binary->data, binary->size, "BIN %d %s;", binary->size, binary->message.c_str());      
    break;
  case DATA_LIST:
    {
      (*cl)<<"[";
      int sz = array->array.size();
      int p = 0;
      for (list<UValue * >::const_iterator it = array->array.begin(); it != array->array.end(); it++) {
	(*it)->send(cl);
	if (++p != sz)
	  (*cl)<< " , ";
      }
      (*cl)<< "]";
    }
    break;
  case DATA_OBJECT:
    {
      (*cl)<<"OBJ "<<object->refName<<" [";
      int sz = object->array.size();
      int p = 0;
      for (list<UNamedValue>::const_iterator it = object->array.begin(); it != object->array.end(); it++) {
	(*cl) << it->name<<":";
	it->val->send(cl);;
	if (++p != sz)
	  (*cl)<< " , ";
      }
      (*cl)<< "]"; 
    }  
    break;
  };
}


UValue& UValue::operator= (const UValue& v)
{ //TODO: optimize
  if (this == &v) return *this;
  switch(type) {
  case DATA_STRING:
    if (stringValue)
      delete stringValue;
  case DATA_BINARY:
    if (binary)
      delete binary;
    break;
  case DATA_LIST:
    if (array)
      delete array;
    break;
  case DATA_OBJECT:
    if (object)
      delete object;
  }
  
  type = v.type;
  switch (type) {  
  case DATA_DOUBLE:
    val = v.val;
    break;
  case DATA_STRING:
    stringValue = new string(*v.stringValue);
    break;
  case DATA_BINARY:
    binary = new UBinary(*v.binary); 
    break;
  case DATA_LIST:
    array = new UArray(*v.array);
    break;
  case DATA_OBJECT:
    object = new UNamedArray(*v.object);
    break;
  };
  return *this;
}


UValue::UValue(const UValue &v) {
  type = DATA_VOID;
  (*this) = v;
}


UBinary::UBinary() {
  data = 0;
}

UBinary::~UBinary() {
  if (data)
    free(data);
}

UBinary::UBinary(const UBinary &b) {
  type = BINARY_NONE;
  data = 0;
  (*this) = b;
}

UBinary & UBinary::operator = (const UBinary &b) {
  if (data)
    free(data);

  type = b.type;
  message = b.message;
  size = b.size;
  switch(type) {
  case BINARY_IMAGE:
    image = b.image;
    break;
  case BINARY_SOUND:
    sound = b.sound;
    break;
  }
  data = malloc(size);
  memcpy(data, b.data, b.size);
}


UArray::UArray() {}

UArray::UArray(const UArray &b) {
  (*this) = b;
}

UArray & UArray::operator = (const UArray &b) {
  while (!array.empty()) {
    delete array.front();
    array.pop_front();
  }

  for (list<UValue*>::const_iterator it= b.array.begin(); it !=b.array.end();it++)
    array.push_back(new UValue(**it));

  return (*this);
}

UArray::~UArray() {
  while (!array.empty()) {
    delete array.front();
    array.pop_front();
  }
}




UNamedArray::UNamedArray() {}

UNamedArray::UNamedArray(const UNamedArray &b) {
  (*this) = b;
}

UNamedArray & UNamedArray::operator = (const UNamedArray &b) {
  while (!array.empty()) {
    delete array.front().val;
    array.pop_front();
  }

  for (list<UNamedValue>::const_iterator it= b.array.begin(); it != b.array.end();it++)
    array.push_back(UNamedValue(it->name, new UValue(*(it->val))));

  return (*this);
}

UNamedArray::~UNamedArray() {
  while (!array.empty()) {
    delete array.front().val;
    array.pop_front();
  }
}



namespace urbi {
UClient * defaultClient=0;
}

UClient * urbi::getDefaultClient() {
  return urbi::defaultClient;
}

void urbi::setDefaultClient(UClient * cl) {
  urbi::defaultClient = cl;
}


std::ostream & urbi::unarmorAndSend(const char * a) {
  std::ostream & s = (urbi::getDefaultClient()==0)? (std::cerr) : (((UAbstractClient*)urbi::getDefaultClient())->getStream());
  if (strlen(a)>2)
    if (a[0]=='(' && a[strlen(a)-1]==')')
      s.rdbuf()->sputn(a+1, strlen(a)-2);
    else
      s << a; //this is baaad, user forgot the parenthesis but was lucky
  return s;
}

