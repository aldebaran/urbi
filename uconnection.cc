/*! \file uconnection.cc
 *******************************************************************************

 File: uconnection.cc\n
 Implementation of the UConnection class.

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

#include <string.h>

#include "uconnection.h"
#include "userver.h"
#include "uqueue.h"
#include "ucommandqueue.h"
#include "parser/uparser.h"
#include "ucallid.h"
#include "uvariable.h"

char errorMessage[1024]; // Global variable (thanks bison...) to store the
                         // the error message when a parsing error occurs.

//! UConnection constructor.
/*! This constructor must be called by any sub class.

    Important note on memory management: you should call ADDOBJ(nameofsubclass)
    in the constructor of your UConnection sub class to maintain a valid 
    memory occupation evaluation. Symmetricaly, you must call
    FREEOBJ(nameofsubclass) in the UConnection destructor. "nameofsubclass" is
    the name of the sub class used (which will be evaluated with a sizeof 
    operator). ADDOBJ and FREEOBJ macros are in utypes.h

    \param userver is the server to which the connection belongs
    \param minSendBufferSize UConnection uses an adaptive dynamic queue (UQueue)
           to buffer sent data. This parameter sets the minimal and initial size
           of the queue. Suggested value is 4096.
    \param maxSendBufferSize The internal sending UQueue size can grow up to 
           maxSendBufferSize.A good strategy is to have here twice the size of
           the biggest data that could enter the queue, plus a small 10% 
           overhead. Typically, the size of the biggest image times two + 10%.
           Zero means "illimited" and is not advised if one wants to control
           the connection's size.
    \param packetSize is the maximal size of a packet sent in one shot via a 
           call to effectiveSend(). This should be the biggest as possible to
           help emptying the sending queue as fast as possible. Check the 
           capacity of your connection to know this limit.
    \param minRecvBufferSize UConnection uses an adaptive dynamic queue (UQueue)
           to buffer received data. This parameter sets the minimal and initial
           size of the queue. Suggested value is 4096.
    \param maxRecvBufferSize The internal receiving UQueue size can grow up to 
           maxRecvBufferSize.A good strategy is to have here twice the size of
           the biggest data that could enter the queue, plus a small 10% 
           overhead. Zero means "illimited". Note that binary data are handled
           on specific buffers and are not part of the receiving queue. The 
           "biggest size" here means the "biggest ascii command". For URBI 
           commands, a good choice is 65536.

    Note that all UQueues are, by default, adaptive queues (see the UQueue
    documentation for more details). This default kernel behavior can be 
    modified by changing the UConnection::ADAPTIVE constant or, individually by
    using the setSendAdaptive(int) and setReceiveAdaptive(int) method.

    When exiting, UError can have the following values:
    - USUCCESS: success
    - UFAIL   : memory allocation failed.

    \sa UQueue
*/
UConnection::UConnection  (UServer *userver, 
                           int minSendBufferSize,
                           int maxSendBufferSize,
                           int packetSize,
                           int minRecvBufferSize,
                           int maxRecvBufferSize)
{
  UError = USUCCESS;
  char tmpbuffer_connectionTag[50];
	
  server           = userver;
  packetSize_      = packetSize;
  sendAdaptive_    = UConnection::ADAPTIVE;
  recvAdaptive_    = UConnection::ADAPTIVE;

  // Create send queue
  sendQueue_       = new UQueue(minSendBufferSize,
                                maxSendBufferSize,
                                sendAdaptive_);
  if ((sendQueue_ == 0) || (sendQueue_->UError == UFAIL)) {
    UError = UFAIL;
    return;
  }
   
  // Create receive queue
  recvQueue_       = new UCommandQueue(minRecvBufferSize,
                                       maxRecvBufferSize,
                                       sendAdaptive_);
  if ((recvQueue_ == 0) || (recvQueue_->UError == UFAIL)) {
    UError = UFAIL;
    delete sendQueue_;
    sendQueue_ = 0;
    return;
  }

  // Initial state of the connection: unblocked, not receiving binary.
  blocked_             = false;
  receiveBinary_       = false;
  active_              = true;

  for (int i = 0; i < MAX_ERRORSIGNALS ; i++)
    errorSignals_[i] = false;

  // no active command and no last command at start:
  lastCommand = 0;
  activeCommand = 0;

  // initialize the connection tag used to reference local variables
  sprintf(tmpbuffer_connectionTag,"U%d",(int)this);
  connectionTag = new UString(tmpbuffer_connectionTag); 
  UVariable* cid = new UVariable(tmpbuffer_connectionTag,"connectionID",
                                 tmpbuffer_connectionTag);
  if (cid) cid->uservar = false;
  functionTag = 0;
  killall = false;
  closing = false;
  newDataAdded = false; 
  returnMode = false;
  obstructed = false;
  receiving  = false;
  inwork     = false;
  clientIP   = 0;
}

//! UConnection destructor.
UConnection::~UConnection() 
{
  delete recvQueue_;
  delete sendQueue_;

  if (connectionTag) {
    UVariable *vari = server->getVariable(connectionTag->str(),"connectionID");
    if (vari) delete vari;
    delete connectionTag;
  }
}

//! UConnection IP associated
/*! The robot specific part should call the function when the connection is active
    and transmit the IP address of the client, as a long int.
*/
void 
UConnection::setIP(IPAdd ip) 
{
  clientIP = ip;
}

//! UConnection close. Must be redefined by the robot-specific sub class.
/*! The implementation of this function must set 'closing' to true, to 
    tell the UConnection to stop sending data.
*/
UErrorValue 
UConnection::closeConnection() 
{
  closing = true;
  return (USUCCESS);
}

//! Initializes the connection, by sending the standard header for URBI
/*! This function must be called once the connection is operational and 
    able to send data. It is a requirement for URBI compliance to send
    the header at start, so this function must be called.
*/
void UConnection::initialize() 
{
  char customHeader[1024];
  int i;


  for (i = 0; i < ::NB_HEADER_BEFORE_CUSTOM; i++)
    send(::HEADER_BEFORE_CUSTOM[i],"start");

  i = 0;
  do {
    server->getCustomHeader(i,(char*)customHeader,1024);
    if (customHeader[0]!=0) send((const char*) customHeader,"start");
    i++;
  } while (customHeader[0]!=0);

  for (i = 0; i < ::NB_HEADER_AFTER_CUSTOM; i++)
    send(::HEADER_AFTER_CUSTOM[i],"start");
  sprintf(customHeader,"*** ID: %s\n",connectionTag->str());
  send(customHeader,"ident");

  sprintf(customHeader,"%s created",connectionTag->str());
  server->echo(::DISPLAY_FORMAT, (int)this,
               "UConnection::initialize",
               customHeader); 

  server->loadFile("CLIENT.INI",recvQueue_);
  newDataAdded = true;
}

//! Send a string through the connection.
/*! A tag is automatically added to output the message string and the 
    resulting string is sent via send(const ubyte*,int).
    \param s the string to send
    \param tag the tag of the message. Default is "notag" 
    \return 
            - USUCCESS: successful
            - UFAIL   : could not send the string
    \sa send(const ubyte*,int)
*/
UErrorValue      
UConnection::send (const char *s, const char* tag)
{
  static const int MAXSIZE_TMPBUFFER = 1024;
  static char tmpBuffer_[MAXSIZE_TMPBUFFER];

  if (tag == NULL)
    snprintf(tmpBuffer_,
             MAXSIZE_TMPBUFFER,
             "[%08d:%s] ", (int)server->lastTime(), ::UNKNOWN_TAG);
  else {
    snprintf(tmpBuffer_,
             MAXSIZE_TMPBUFFER-3,
             "[%08d:%s", (int)server->lastTime(), tag);
    strcat(tmpBuffer_,"] "); // This splitting method is used to truncate the
                             // tag if its size is too large.
  }
  
  send((const ubyte*)tmpBuffer_,strlen(tmpBuffer_));
  return send((const ubyte*)s,strlen(s));
}

//! Send a buffer through the connection.
/*! The function piles the buffer in the sending queue and calls continueSend()
    if the connection is not blocked (blocked means that the connection is not
    ready to send data). The server will try to send the data in the
    sending queue each time the "work" function is called and if the connection
    is not blocked. It is the job of the programmer to let the kernel know when
    the connection is blocked or not, using the "block()" function to block it
    or by calling continueSend() directly to unblock it.

    \param buffer the buffer to send
    \param length the length of the buffer
    \return 
            - USUCCESS: successful. The message is in the queue.
            - UFAIL   : could not send the buffer, not enough memory in the 
                        send queue.
    \sa send(const char*)
*/
UErrorValue     
UConnection::send (const ubyte *buffer, int length)
{
  if (closing) return USUCCESS;

  UErrorValue result = sendQueue_->push(buffer,length);
  if ( result != USUCCESS) {

    if (result == UMEMORYFAIL) {

      errorSignal(UERROR_SEND_BUFFER_FULL);
      server->memoryOverflow = true;
      server->isolate();
    }
    if (result == UFAIL) errorSignal(UERROR_SEND_BUFFER_FULL);
    return UFAIL;
  }

  if (!blocked_)
    continueSend();     

  return USUCCESS;
}

//! Returns the state of the connection: blocked or unblocked.
bool
UConnection::isBlocked ()
{
  return ( blocked_ );
}

//! Blocks the connection so that send() functions will not call continueSend.
/*! The normal behavior of the send() functions is to pile the data in the
    internal buffer and call continueSend(), except if the connection is
    blocked. */
void
UConnection::block ()
{
  blocked_ = true;
}


//! Send at most packetSize bytes in the connection, calling effectiveSend()
/*! Must be called when the system tells that the connection is ready to 
    accept new data for sending, in order to carry on the processing of the 
    sending queue stored in the internal buffer.
    Each call to continueSend sends packetSize bytes (at most) through the real
    connection until the internal buffer is empty.
    \return 
            - USUCCESS: successful
            - UFAIL   : effectiveSend() failed or not enough memory
*/ 
UErrorValue
UConnection::continueSend ()
{
  blocked_ = false;         // continueSend unblocks the connection.
  
  int toSend = sendQueue_->dataSize(); // nb of bytes to send
  if (toSend > packetSize_) toSend = packetSize_;
  if (toSend == 0) return USUCCESS;

  
  ubyte* popData = sendQueue_->virtualPop(toSend);

  if (popData != 0) {
    int wasSent = effectiveSend( (const ubyte*)popData, toSend );

    if (wasSent < 0) return UFAIL;
    else 
      if ((wasSent == 0) || (sendQueue_->pop(wasSent) != 0))
        return USUCCESS;    
  }
 
  server->memoryOverflow = true;
  server->isolate();
    
  return UFAIL;  
}

//! Handles an incoming string. 
/*! Must be called each time a string is received by the connection.
   \param s the incoming string
   \return UFAIL buffer overflow
   \return UMEMORYFAIL critical memory overflow
   \return USUCCESS otherwise
*/ 
UErrorValue
UConnection::received (const char *s) 
{
  return received( (const ubyte*) s, strlen(s) );
}

//! Handles a incoming buffer of data. 
/*! Must be called each time a buffer of data is received by the connection.
   \param buffer the incoming buffer
   \param length the length of the buffer
   \return UFAIL buffer overflow
   \return UMEMORYFAIL critical memory overflow
   \return USUCCESS otherwise
*/ 
UErrorValue          
UConnection::received (const ubyte *buffer, int length)
{ 
  if (server->memoryOverflow) {

    errorSignal(UERROR_RECEIVE_BUFFER_CORRUPTED);
    return UFAIL; // Block any new incoming command
                  // when the system is out of memory
  }

  if (receiveBinary_) // handles and try to finish the binary transfer
   
    if (length < binCommand->refBinary->ref()->bufferSize -
        transferedBinary_) {
      memcpy(binCommand->refBinary->ref()->buffer + transferedBinary_,
             buffer,
             length);
      transferedBinary_ += length;
      return USUCCESS;
    }
    else {
      memcpy(binCommand->refBinary->ref()->buffer + transferedBinary_,
             buffer,
             binCommand->refBinary->ref()->bufferSize - transferedBinary_);

      buffer += (binCommand->refBinary->ref()->bufferSize - 
                transferedBinary_);

      length -= (binCommand->refBinary->ref()->bufferSize - 
                transferedBinary_);
      receiveBinary_ = false;  

      append(binCommand->up);
    }
  
  UErrorValue result = recvQueue_->push( buffer, length);
  
  if ( result != USUCCESS) { // Handles memory errors.
    
    if ( result == UFAIL ) {
      
      errorSignal(UERROR_RECEIVE_BUFFER_FULL);
      errorSignal(UERROR_RECEIVE_BUFFER_CORRUPTED);
    }
    
    if ( result == UMEMORYFAIL ) {
      
      errorSignal(UERROR_RECEIVE_BUFFER_CORRUPTED);
      server->memoryOverflow = true;
      server->isolate();
    }
    return result;
  }

  // Starts processing
  receiving = true;
  server->updateTime();

  do {

    ubyte* command = recvQueue_->popCommand(length);

    if ((command == 0) && (length==-1)) {
      recvQueue_->clear();
      length = 0;
    }

    if (length !=0) {

      errorMessage[0] = 0; // no error at start (errorMessage string is empty)
      
      server->parser.commandTree = 0;
      server->systemcommands = false;
      int result = server->parser.process(command, length, this);    
      server->systemcommands = true;         
       
      if (result == -1) {
        
        server->isolate();
        server->memoryOverflow = true;
      }
      server->memoryCheck();
      
      // Xtrem memory recovery in case of anomaly
      if (server->memoryOverflow) 
        if (server->parser.commandTree) {
          delete server->parser.commandTree;                  
          server->parser.commandTree = 0;
        }
      
      // Error Message handling
      if ((errorMessage[0] != 0) &&
          (!server->memoryOverflow)) { // a parsing error occured 
        
        if (server->parser.commandTree) {
          delete server->parser.commandTree;
          server->parser.commandTree = 0;
        }
        
        send(errorMessage,"error");    
        
        errorMessage[ strlen(errorMessage) - 1 ] = 0; // remove '\n'
        errorMessage[ 42 ] = 0; // cut at 41 characters
        server->error(::DISPLAY_FORMAT,(int)this,
                      "UConnection::received",
                      errorMessage);      
      }
      else  
        if (server->parser.commandTree)
          if (server->parser.commandTree->command1) {
            // Process "commandTree"              
            
            // CMD_ASSIGN_BINARY: intercept and execute immediately
            if (server->parser.binaryCommand) {
              binCommand = ((UCommand_ASSIGN_BINARY*)
                            server->parser.commandTree->command1);
              
              ubyte* buffer = recvQueue_->pop(binCommand->refBinary->ref()->bufferSize);
              
              if (buffer) { // the binary was all in the queue
                memcpy(binCommand->refBinary->ref()->buffer,
                       buffer,
                       binCommand->refBinary->ref()->bufferSize);              
              }
              else { // not all was there, must set receiveBinary mode on
                
                transferedBinary_ = recvQueue_->dataSize();
                memcpy(binCommand->refBinary->ref()->buffer,
                       recvQueue_->pop(transferedBinary_),
                       transferedBinary_);
                receiveBinary_ = true;
              }
            }

            // Pile the command
            if (!receiveBinary_) {

              // immediate execution of simple commands
            
              if (!obstructed) {                
                server->parser.commandTree->up = 0;
                server->parser.commandTree->position = 0;                                    
                execute(server->parser.commandTree);
                if ((server->parser.commandTree) &&
                    (server->parser.commandTree->status == URUNNING))
                  obstructed = true;
              }

              if (server->parser.commandTree) 
                append(server->parser.commandTree); 

              server->parser.commandTree = 0;
            }
          } // command1
    }   
  } while ( (length != 0) &&
            (!receiveBinary_) &&
            (!server->memoryOverflow));  

  receiving = false;  
  server->parser.commandTree = 0;

  if (server->memoryOverflow) return UMEMORYFAIL; 

  return USUCCESS; 
}

//! Sends a buffer through the real connection (redefined in the sub class)
/*! Must be defined to implement the effective code that sends a buffer through 
    the connection.

    ATTENTION: The buffer received is a short lived buffer. There is no
    warranty whatsoever that it will survive once the function returns. You must
    make a copy of it if your sending method requires to work asynchronously on
    the buffer, after the function has returned.

    \return the number of bytes effectively sent. -1 means that there was an error.
*/
int  
UConnection::effectiveSend (const ubyte *buffer, int length)
{
  return length;
}

//! Send an error message based on the error number.
/*! This command sends an error message through the connection, and to the server
    output system, according to the error number n.

    \param n the error number. Use the UErrorCode enum. Can be:
            - 0 : Critical Error
            - 1 : Syntax Error (never used, bison handles it)
            - 2 : Division by zero
            - 3 : Receive buffer full
            - 4 : Out of memory
            - 5 : Send buffer full
            - 6 : Receive buffer corrupted
            - 7 : Memory Warning
*/
UErrorValue
UConnection::error (UErrorCode n)
{
  char errorString[80]; // Max error message = 80 chars

  switch (n) {

  case UERROR_CRITICAL            : strcpy(errorString,"!!! Critical error\n"); break;
  case UERROR_SYNTAX              : strcpy(errorString,"!!! Syntax error\n"); break;
  case UERROR_DIVISION_BY_ZERO    : strcpy(errorString,"!!! Division by zero\n"); break;
  case UERROR_RECEIVE_BUFFER_FULL : strcpy(errorString,"!!! Receive buffer full\n"); break;
  case UERROR_MEMORY_OVERFLOW     : strcpy(errorString,"!!! Out of memory\n"); break;
  case UERROR_SEND_BUFFER_FULL    : strcpy(errorString,"!!! Send buffer full\n"); break;
  case UERROR_CPU_OVERLOAD        : strcpy(errorString,"!!! CPU Overload\n"); break;
  case UERROR_RECEIVE_BUFFER_CORRUPTED : strcpy(errorString,"!!! Receive buffer corrupted\n"); break;
  default                         : strcpy(errorString,"!!! Unidentified Error\n");

  }

  UErrorValue result = send( errorString, "error");
  if (result == USUCCESS ) {
    errorString[strlen(errorString) - 1] = 0; // remove the '\n' at the end...
    server->error(::DISPLAY_FORMAT,(int)this,"UConnection::error",errorString);
  } 
  return( result );
}

//! Send a warning message based on the warning number.
/*! This command sends an warning message through the connection, and to the server
    output system, according to the warning number n.

    \param n the warning number. Use the UWarningCode enum. Can be:
            - 0 : Memory overflow warning
    \param complement is a complement string added at the end of the warning message.
*/
UErrorValue
UConnection::warning (UWarningCode n)
{
  char warningString[80]; // Max warning message = 80 chars

  switch (n) {

  case UWARNING_MEMORY            : strcpy(warningString,"!!! Memory overflow warning\n"); break;
  default                         : strcpy(warningString,"!!! Unidentified Warning\n");

  }

  UErrorValue result = send( warningString, "warning");
  if (result == USUCCESS ) {
    warningString[strlen(warningString) - 1] = 0; // remove the '\n' at the end...
    server->echoKey("WARNG",::DISPLAY_FORMAT,(int)this,"UConnection::warning",warningString);
  } 
  return( result );
}

//! Set a flag to insure the error will be send.
/*! This command sends an error message through the connection, and to the server
    output system, according to the error number n. The difference with the "error"
    function is that is does not actually send the message but set a flag so that the
    message will be sent. The flag is active as long as the message is not actually
    sent. So, using errorSignal is more robust since it guarantees that the message
    will be sent, at all costs.

    \param n the error number. Use the UErrorCode enum. Can be:
            - 0 : Critical Error
            - 1 : Syntax Error (never used, bison handles it)
            - 2 : Division by zero
            - 3 : Receive buffer full
            - 4 : Out of memory
            - 5 : Send buffer full
            - 6 : Receive buffer corrupted
            - 7 : Memory Warning
*/
void
UConnection::errorSignal (UErrorCode n)
{
  errorSignals_[(int)n] = true;
}

//! Check if the errorSignal is active and tries to effectively send the message
/*! If the message can be sent, the errorSignal is canceled, otherwise not.
 */
void
UConnection::errorCheck (UErrorCode n)
{
  if ( errorSignals_[(int)n] ) {
    
    if ( error(n) == USUCCESS )
      errorSignals_[(int)n] = false;
  }
}

//! Activate the connection
/*! The use of activation is related to OPENR. When a new connection is created
    in OPENR, it is not active immediately. One must wait until a listen call
    returns. For this reason, the connection is set to disactivate until this
    listen call returns. Then, it is activated and the connection can be
    "visible" from the kernel's point of view.

    In normal situations, just ignore this. For example, if your connection is 
    usable (send/receive) once it has been created, you can ignore this.
 */
void
UConnection::activate()
{
  active_ = true;
}

//! Disactivate the connection
/*! see UConnection::activate() for more details about activation.
 */
void
UConnection::disactivate()
{
  active_ = false;
}

//! Disactivate the connection
/*! see UConnection::activate() for more details about activation.
 */
bool
UConnection::isActive()
{
  return (active_);
}

//! Execute a command
/*! This function executes a regular command.
     
    \param command is the UCommand to execute.
*/                     
UCommand*
UConnection::processCommand(UCommand *&command,
                            URunlevel &rl,
                            bool &mustReturn)
{ 
  mustReturn = false;
  if (command == 0) return 0;
  if ((rl != UWAITING) ||
      (returnMode) ||
      (command->toDelete)) return (command);
 
  rl = UEXPLORED;
   
  // Handle blocked/freezed commands
  if ( (command->tag) &&
       (server->freezetab.find(command->tag->str())!=server->freezetab.end()) &&
       (server->freezetab[command->tag->str()]) )    
    return (command);
  

  if ( (command->tag) &&
       (server->blocktab.find(command->tag->str())!=server->blocktab.end()) &&
       (server->blocktab[command->tag->str()]) ) {
    
    if (command == lastCommand)
      lastCommand = command->up;
        
    delete command;
    return(0);    
  }

  UCommand         *morphed;  
  UCommand_TREE    *morphed_up;
  UCommand         **morphed_position;
  UNamedParameters *param;
  bool             stopit;

  while (1) {

    // timeout, stop , freeze and connection flags initialization

    if (command->startTime == -1) {
      command->startTime = server->lastTime();

      param = command->flags;
      while (param) {
      
        if (param->name) {

          if (param->name->equal("flagid")) {  
          
            param->name->update("noflag");    
            UValue* tmpID = param->expression->eval(command,this);
            if (tmpID) {

              if (tmpID->dataType == DATA_STRING)                                                             
                for (list<UConnection*>::iterator retr = ::urbiserver->connectionList.begin();
                     retr != ::urbiserver->connectionList.end();
                     retr++) 
                  if ((*retr)->isActive()) 
                    
                    if (((*retr)->connectionTag->equal(tmpID->str)) ||
                        (strcmp(tmpID->str->str(),"all") == 0) ||
                        ( ( strcmp(tmpID->str->str(),"other") == 0) &&
                          ( !(*retr)->connectionTag->equal(connectionTag)))) {
                      
                      UCommand_TREE* tohook = new UCommand_TREE( UAND,
                                                                 command->copy(),
                                                                 0);
                      (*retr)->append(tohook);
                    }
              
              delete tmpID;
            }
            delete command;
            return(0); 
          }

          
          if (param->name->equal("flagtimeout")) {
            command->flagType += 1;            
            command->flagExpr1 = param->expression; 
            send("!!! Warning: +timeout flag is obsolete. Use timeout(time) command instead.\n",command->tag->str());
          }
          if (param->name->equal("flagstop")) {            
            command->flagType += 2;        
            command->flagExpr2 = param->expression;
            send("!!! Warning: +stop flag is obsolete. Use stopif(test) command instead.\n",command->tag->str());
          } 
          if (param->name->equal("flagfreeze")) {            
            command->flagType += 4;        
            command->flagExpr4 = param->expression;
            send("!!! Warning: +freeze flag is obsolete. Use freezeif(test) command instead.\n",command->tag->str());
          }

          if ((param->name->equal("flag")) &&
              (param->expression) && 
              (param->expression->val == 10)) 
            command->flagType += 8;                      
          
          if ((param->name->equal("flag")) &&
              (param->expression) && 
              (!command->morphed) &&              
              (( param->expression->val == 4 ) || // 4 = +begin
               ( param->expression->val == 1 ) )) // 1 = +report
            send("*** begin\n",command->tag->str());
          
        }
      
        param = param->next;      
      }
    }
    
    stopit = false;

    // flag "+timeout"
    if (command->flagType&1) {
      UValue *value = command->flagExpr1->eval(command,this);
      if ((value) && 
          (value->dataType == DATA_NUM) &&
          (command->startTime + value->val <= server->lastTime()))
        stopit = true;
      if (value) delete value;
    }    

    // flag "+stop"

    if (command->flagType&2)  {
      UTestResult testres = booleval(command->flagExpr2->eval(command, this));
      
      if (testres == UTRUE) {
        if (command->flag_nbTrue2 == 0) 
          command->flag_startTrue2 = server->lastTime();
        command->flag_nbTrue2++;        
      }
      else 
        command->flag_nbTrue2 = 0;

      if ( ( (command->flagExpr2->softtest_time) && 
             (command->flag_nbTrue2 > 0) && 
             (server->lastTime() - command->flag_startTrue2 >= 
              command->flagExpr2->softtest_time->val)) ||
           
           ( (command->flag_nbTrue2 >0) &&              
             (command->flagExpr2->softtest_time==0)) )
        stopit = true;
    }

    // flag "+freeze"

    if (command->flagType&4) {
      UTestResult testres = booleval(command->flagExpr4->eval(command, this));
      
      if (testres == UTRUE) {
        if (command->flag_nbTrue4 == 0) 
          command->flag_startTrue4 = server->lastTime();
        command->flag_nbTrue4++;        
      }
      else 
        command->flag_nbTrue4 = 0;  

      if ( ( (command->flagExpr4->softtest_time) && 
             (command->flag_nbTrue4 > 0) && 
             (server->lastTime() - command->flag_startTrue4 >= 
              command->flagExpr4->softtest_time->val)) ||
           
           ( (command->flag_nbTrue4 >0) &&              
             (command->flagExpr4->softtest_time==0)) )
        return(command);
    }

    if (stopit) {
      param = command->flags;
      while (param) {
        
        if ((param->name) && 
            (param->name->equal("flag")) &&
            (param->expression) && 
            (!command->morphed) &&    
            (( param->expression->val == 3 ) || // 3 = +end
             ( param->expression->val == 1 ) )) // 1 = +report
          send("*** end\n",command->tag->str());
        
        param = param->next;
      }
      
      if (command == lastCommand)
        lastCommand = command->up;
      
      delete command;       
      return(0); 
    }

    // Regular command processing

    if (command->type == CMD_TREE) {
 
        mustReturn = true;
        return( command );
    }
    else { // != CMD_TREE
            
      morphed_up = command->up;
      morphed_position = command->position;
    
      switch (command->execute(this)) {
    
      case UCOMPLETED: 
       
        param = command->flags;
        while (param) {
          
          if ((param->name) && 
              (param->name->equal("flag")) &&
              (param->expression) && 
              (( param->expression->val == 3 ) || // 3 = +end
               ( param->expression->val == 1 ) )) // 1 = +report
            send("*** end\n",command->tag->str());
          
          param = param->next;
        }

        if (command == lastCommand)
          lastCommand = command->up;

        delete command;       
        return(0);        
        
      case UMORPH:
                       
        command->status = UONQUEUE;
        command->morphed = true;
        
        morphed = command->morph;     
        morphed->toDelete = command->toDelete;
        morphed->up = morphed_up;
        morphed->position = morphed_position;     
        if (command->flags)
          morphed->flags = command->flags->copy();

        if (!morphed->tag)
          if (command->tag)
            morphed->tag->update(command->tag->str());
          else
            morphed->tag = new UString("notag");
        //morphed->morphed = true;
        if (!command->persistant) delete command;  
        command = morphed;        
        break;
        
      default:

        // "+bg" flag
        if ((command->flagType&8) &&
            (command->status == URUNNING))
          command->status = UBACKGROUND;        

        return(command);
      }    
    }     
  }//while(1)
}
   
//! Execute a command tree
/*! This function executes a command tree and
    returns the next node of the tree to process..
     
    \param tree is the UCommand_TREE to execute.
*/                       
void
UConnection::execute(UCommand_TREE* &execCommand)
{     
  UCommand_TREE *oldtree;
  UCommand_TREE *tree;
  UVariable* vari;
  bool mustReturn;
  bool deletecommand;

  if ((execCommand == 0) || (closing)) return;

  tree = execCommand;

  while (tree) { 
 
    tree->status = URUNNING;

    // BLOCKED/FREEZED COMMANDS

    deletecommand = false;
    if ( (tree->tag) &&
         (server->freezetab.find(tree->tag->str())!=server->freezetab.end()) &&
         (server->freezetab[tree->tag->str()]) ) {

      tree = tree->up;
      continue;
    }      

    if ( (tree->tag) &&
         (server->blocktab.find(tree->tag->str())!=server->blocktab.end()) &&
         (server->blocktab[tree->tag->str()]) ) {
    
      tree->runlevel1 = UEXPLORED;
      tree->runlevel2 = UEXPLORED;
      deletecommand = true;      
    }      
            
    // COMMAND1
    
    if ((tree->callid) &&
        (tree->command1) &&
        (tree->runlevel1 == UWAITING))
      stack.push_front(tree->callid);
    
    tree->command1 = processCommand ( tree->command1,
                                      tree->runlevel1,
                                      mustReturn );   
    
    if (mustReturn)  { tree = (UCommand_TREE*) tree->command1; continue;}
    
    if (tree->callid) {
      stack.pop_front();       
      if (returnMode) {
        delete tree->command1;
        tree->command1 = 0;
        returnMode = false;
      }
    }

    // COMMAND2
    
    if ( (tree->node == UAND) ||
         (tree->node == UCOMMA) ||
         (tree->command1 == 0) || 
         (tree->command1->status == UBACKGROUND) ) {

      if (tree == lastCommand) 
        obstructed = false;
      
      tree->command2 = processCommand ( tree->command2,
                                        tree->runlevel2,
                                        mustReturn );
      
      if (mustReturn)  {tree = (UCommand_TREE*) tree->command2; continue;}
        
      if (tree->callid) { // arriving here means the function is finished
        vari  = ::urbiserver->getVariable(tree->callid->str(), "__result__");
        if (vari) delete vari;
      }    
    }

    // STATUS UPDATE

    if ( ((tree->command1 == 0) && 
          (tree->command2 == 0)) ||
         (deletecommand)) {
    
      if (tree == lastCommand)
        lastCommand = tree->up;
      if (tree == execCommand)
        execCommand = 0;

      if (tree->position)
        *(tree->position) = 0;
      oldtree = tree;
      
      UNamedParameters *param = tree->flags;
      while (param) {
        
        if ((param->name) && 
            (param->name->equal("flag")) &&
            (param->expression) && 
            (( param->expression->val == 3 ) || // 3 = +end
             ( param->expression->val == 1 ) )) // 1 = +report
          send("*** end\n",tree->tag->str());
        
        param = param->next;
      }

      tree = tree->up;    

      delete oldtree;
      continue;      
    }
    else
      if ( ( ((tree->command1 == 0) || 
              (tree->command1->status == UBACKGROUND)) &&

             ((tree->command2 == 0) || 
              (tree->command2->status == UBACKGROUND))) ||

           (tree->background == true) ||
           (tree->flagType&8))

        tree->status = UBACKGROUND;
    //   else
    //    tree->status = URUNNING;
    
    tree->runlevel1 = UWAITING;
    tree->runlevel2 = UWAITING;

    // REDUCTION

    if ((tree != lastCommand) &&
        (tree != execCommand) &&
        (!tree->toDelete) &&
        (tree->command1 == 0) &&
        (tree->command2 != 0)) { // left reduction 
      
      *(tree->position) = tree->command2;
      tree->command2->up = tree->up;
      tree->command2->position = tree->position;
      tree->command2->background = tree->background;
      tree->command2 = 0;
      oldtree = tree;
      tree = tree->up; // cannot be zero     
      
      delete oldtree;
      continue;
    }

    if ((tree != lastCommand) &&
        (tree != execCommand) &&
        (!tree->toDelete) &&
        (tree->command2 == 0) &&
        (tree->command1 != 0) && 
        (tree->command1->status != UBACKGROUND) ) { // right reduction 
                                          // the background hack is here to preserve {at()...} commands.
      
      *(tree->position) = tree->command1;
      tree->command1->up = tree->up;
      tree->command1->position = tree->position;
      tree->command1->background = tree->background;
      tree->command1 = 0;
      oldtree = tree;
      tree = tree->up; // cannot be zero    
    
      delete oldtree;
      continue;
    }

    // BACK UP
 
    tree = tree->up; 
  }

  if ( (execCommand) &&
       (execCommand->command1 == 0) &&
       (execCommand->command2 == 0)) {
    delete execCommand;
    execCommand = 0;
  } 

}

//! Append a command to the command queue
/*! This function appends a command to the command queue
    activeCommand is the command to process when the system
    wants to process next command. Commands are stored in 
    a tree structure, each branch being a ; , & or | command
    seprarator. The lastCommand always point to a ; or , 
    tree where the left side is empty, ready to receive the
    next command to append.
    See UConnection::execute for more details on the way
    commands are stored and processed.
    
    \param command is the UCommand to append.
*/                     
void
UConnection::append(UCommand_TREE *command)
{
  if (activeCommand == 0) {
    activeCommand = command;
    command->up = 0;
    command->position = 0;
  }
  else {
    lastCommand->command2 = command;   
    command->up = lastCommand;  
    command->position = &(lastCommand->command2);
  }
  
  lastCommand = command;
}

//! Returns how much space is available in the send queue
int
UConnection::availableSendQueue ()
{
  return(sendQueue_->bufferMaxFreeSpace());
}


//! Returns how many bytes are still in the send queue
int
UConnection::sendQueueRemain ()
{
  return(sendQueue_->dataSize());
}

//! Performs a variable prefix check for local storage in function calls
/*! When a new variable is created inside a connection, it is necessary to
    check if it is not a variable local to some function, in that case it
    must be added to the local stack of this function in order to destroy
    the variable once the function returns.
    This is done by localVariableCheck.
*/
void
UConnection::localVariableCheck (UVariable *variable)
{ 
  if (!stack.empty()) {

    UCallid* cid = stack.front();
    if (variable->devicename->equal(cid->str()))
      cid->store(variable);
  }   
}
