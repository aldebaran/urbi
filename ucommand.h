/*! \file ucommand.h
 *******************************************************************************

 File: ucommand.h\n
 Definition of the UCommand class.

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

#ifndef UCOMMAND_H_DEFINED
#define UCOMMAND_H_DEFINED

#include "utypes.h"
#include "ustring.h"
#include "ucommandqueue.h"
#include <list>

#include "uexpression.h"
#include "unamedparameters.h"
#include "uvariablelist.h"
#include "uvalue.h"
#include "uobj.h"
#include "ugroup.h"
#include "uproperty.h"
#include "uvariablename.h"
#include "ubinary.h"
#include "ucallid.h"

using namespace std;

class UCommand;
class UExpression;
class UConnection;
class UNamedParameters;
class UBinary;
class UVariableName;
class UVariableList;
class UVariable;
class UValue;
class UDevice;
class UServer;
class UCommand_TREE;
		

// *****************************************************************************
//! UCommand class stores URBI commands.
/*! UCommand class:   

    Specific commands are all derived from UCommand, but they contain
    command specific members.
*/
class UCommand
{
public:
  MEMORY_MANAGED;
  UCommand(UCommandType _type);          
  virtual ~UCommand();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand* copy();
  UErrorValue       copybase(UCommand *command);
  virtual void      mark(UString *stopTag);
  virtual void      deleteMarked();
  UCommand*         scanGroups(UVariableName** (UCommand::*refName)());
  virtual UVariableName** refVarName()  { return 0; };
  virtual UVariableName** refVarName2() { return 0; };



  
  UCommandType     type;        ///< Type of the command.
  UCommandStatus   status;      ///< Status of the command

  UString          *tag;        ///< Command tag
  UNamedParameters *flags;      ///< list of flags of tagged commands

  UCommand_TREE    *up;         ///< the UCommand_TREE that owns the UCommand
  UCommand         **position;  ///< position in the owning UCommand_TREE
  UCommand         *morph;      ///< stores the target UCommand in case of
                                ///< morphing
  bool             persistant;  ///< tells if the command should be deleted once
                                ///< it is UCOMPLETED (useful for loops)
  bool             toDelete;    ///< true if the command has been marked for 
                                ///< deletion in a stop command.
  bool             background;  ///< used to put the whole tree in bg mode (after
                                ///< a morphing from a "at" or "whenever").
  
  UFloat           startTime;   ///< start time
  UExpression      *flagExpr1;   ///< expression used to store the flags parameters
  UExpression      *flagExpr2;   ///< expression used to store the flags parameters
  UExpression      *flagExpr4;   ///< expression used to store the flags parameters
  int              flagType;    ///< in case of timeout or condout, stores the type
                                ///< of the flag (timeout:0), (condout:1). 
  int              flag_nbTrue2;      ///< nb of times the flag test is true
  UFloat           flag_startTrue2;   ///< time of the last 'true' for "stop"
  int              flag_nbTrue4;      ///< nb of times the flag test is true
  UFloat           flag_startTrue4;   ///< time of the last 'true' for "freeze"
  bool             morphed;     ///< true when the command is part of a morphed 
                                ///< structure  

public:

  static const int MAXSIZE_TMPMESSAGE = 65536; ///< used by commands to build
  HMvariabletab::iterator hmi;         ///< internal

};

extern char tmpbuffer[UCommand::MAXSIZE_TMPMESSAGE]; 


class UCommand_TREE : public UCommand
{
public:
  MEMORY_MANAGED;
  
  UCommand_TREE(UNodeType node,
                UCommand* command1, 
                UCommand* command2);
  virtual ~UCommand_TREE();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();
  virtual void           mark(UString *stopTag);
  virtual void           deleteMarked();

  UCommand         *command1;   ///< Left side of the compound command.
  UCommand         *command2;   ///< Right side of the compound command.
  UCallid          *callid; ///< context identificator for function calls
  UNodeType        node;  ///< node type (AND, PIPE, ...)
  URunlevel        runlevel1,
                   runlevel2;  ///< stores the state of execution of
                                ///< command1 and command2
  UConnection      *connection; ///< belonging connection
};

class UCommand_ASSIGN_VALUE : public UCommand
{
public:
  MEMORY_MANAGED;
  
  UCommand_ASSIGN_VALUE(UVariableName *variablename, 
                        UExpression* expression, 
                        UNamedParameters *parameters,
                        bool defkey = true);
  virtual ~UCommand_ASSIGN_VALUE();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();
  virtual UVariableName** refVarName() { return &variablename; };
  virtual UVariableName** refVarName2() { return &expression->variablename; };



  UVariableName    *variablename;   ///< variable name
  UVariable        *variable;  ///< associated variable
  UExpression      *expression; ///< Expression
  UNamedParameters *parameters; ///< list of parameters
  UString          *method;   ///< method in the varname 
  UString          *devicename; ///< device in the varname

  // Pointers to modificators to ease further processing
  // in the URUNNING mode.

  UExpression      *modif_time;
  UExpression      *modif_sin;
  UExpression      *modif_phase;
  UExpression      *modif_smooth;
  UExpression      *modif_speed;
  UExpression      *modif_accel;
  UExpression      *modif_ampli;
  UExpression      *modif_adaptive;
  UVariableName    *modif_getphase;
  UValue           *tmpeval;

  UExpression      *tmp_phase;  ///< stored temporary phase for cos modificator
  UExpression      *tmp_time;  ///< stored temporary time=0 for direct assignment
  UFloat           endtime;     ///< time limit in case of timeout modificator
  UFloat           startval;    ///< start value for modificators
  UFloat           targetval;   ///< target value for modificators
  UFloat           starttime;   ///< start time for modificators
  UFloat           targettime;  ///< target time for modificators
  UFloat           idealval;    ///< ideal value to reach for the next
                                ///< iteration. Used for +error
  UFloat           speed;       ///< speed for modificators
  UFloat           accel;       ///< accel for the accel modificator
  UFloat           speedmin;    ///< minimal speed in a movement
  UDevice          *dev;        ///< device related to the assigned variable
  
  UFloat           *valtmp;     ///< destination values 
  int              nbval;       ///< nb destinaton values 
  
  bool             finished;    ///< true when the assign is finished  
  bool             profileDone; ///< true when the motion profile is done
  bool             isvaln;      ///< true for 'valn' type assignments
  bool             errorFlag;   ///< true if +error is set for the command
  bool             first;       ///< true on the first passage
  bool             assigned;    ///< true when a nbAssign-- has to be done on delete
  bool             defkey;      ///< is the def prefix used?

private:

  UErrorValue      processModifiers(UConnection* connection, UFloat currentTime);

};

class UCommand_ASSIGN_BINARY : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_ASSIGN_BINARY(UVariableName *variablename, 
                         URefPt<UBinary> *refBinary);
  virtual ~UCommand_ASSIGN_BINARY();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();
  virtual UVariableName** refVarName() { return &variablename; };


  UVariableName    *variablename;   ///< variable name
  UVariable        *variable;  ///< associated variable
  URefPt<UBinary>  *refBinary;  ///< Binary container

  UString          *method;   ///< method in the varname 
  UString          *devicename; ///< device in the varname
};

class UCommand_ASSIGN_PROPERTY : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_ASSIGN_PROPERTY(UVariableName *variablename, 
                           UString *oper,
                           UExpression *expression);
  virtual ~UCommand_ASSIGN_PROPERTY();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();
  virtual UVariableName** refVarName() { return &variablename; };


  UVariableName    *variablename;   ///< variable name
  UVariable        *variable;       ///< associated variable
  UString          *oper;           ///< Property operateur
  UExpression      *expression;     ///< assigned expression

  UString          *method;   ///< method in the varname 
  UString          *devicename; ///< device in the varname
};

class UCommand_AUTOASSIGN : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_AUTOASSIGN ( UVariableName* variablename,
                        UExpression* expression,
                        int assigntype);                 
  virtual ~UCommand_AUTOASSIGN();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UVariableName    *variablename; ///< Name of the iterating variable
  UExpression      *expression;   ///< the list to iterate
  int              assigntype;    ///< is it +=(0) or -=(1)?
};


class UCommand_EXPR : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_EXPR(UExpression* expression); 
  virtual ~UCommand_EXPR();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy(); 
  virtual UVariableName** refVarName() { return &expression->variablename; };

  UExpression      *expression; ///< Expression
};

class UCommand_RETURN : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_RETURN(UExpression* expression); 
  virtual ~UCommand_RETURN();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UExpression      *expression; ///< Expression
};

class UCommand_ECHO : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_ECHO(UExpression* expression, 
                UNamedParameters *parameters,
                UString *connectionTag); 
  virtual ~UCommand_ECHO();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UExpression      *expression; ///< Expression
  UNamedParameters *parameters; ///< list of parameters
  UString          *connectionTag; ///< tag of the connection to echo to.
};

class UCommand_NEW : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_NEW(UString* id,  
               UString* obj, 
               UNamedParameters *parameters, 
	       bool noinit=false);  
  virtual ~UCommand_NEW();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UString          *id;         ///< Identifier
  UString          *obj;        ///< Object
  UNamedParameters *parameters; ///< list of parameters
  bool             noinit; ///< tells if 'init' should be called
};

class UCommand_ALIAS : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_ALIAS (UVariableName* aliasname,
                  UVariableName* id);  		

  virtual ~UCommand_ALIAS();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UVariableName           *aliasname; ///< alias name
  UVariableName           *id; ///< identifier
};

class UCommand_GROUP : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_GROUP(UString* id,
                 UNamedParameters *parameters,
		 int grouptype = 0);  

  virtual ~UCommand_GROUP();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();
 
  UString              *id; ///< identifier
  UNamedParameters     *parameters; ///< list of group members
  int                  grouptype; ///< type of group command group/addgroup/delgroup
};


class UCommand_OPERATOR_ID : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_OPERATOR_ID (UString* oper,
                        UString* id);  
  virtual ~UCommand_OPERATOR_ID();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UString          *oper; ///< operator name
  UString          *id; ///< identifier
};

class UCommand_DEVICE_CMD : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_DEVICE_CMD  (UString* device,
                        UString* cmd);  
  virtual ~UCommand_DEVICE_CMD();
  virtual UVariableName** refVarName() { return &variablename; };

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UVariableName    *variablename; ///< the device name embedded in a var name
  UString          *cmd;    ///< the command (on, off, ...)
};

class UCommand_OPERATOR_VAR : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_OPERATOR_VAR (UString* oper,
                         UVariableName* variablename);  
  virtual ~UCommand_OPERATOR_VAR();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UString          *oper;     ///< operator name
  UVariableName    *variablename;///< variable
  UVariable        *variable; ///< cached variable, used by undef
  UFunction        *fun; ///< cached function, used by undef
};

class UCommand_BINDER : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_BINDER (UString* binder,
                   int type,
                   UVariableName* variablename,
		   int nbparam=0);  
  virtual ~UCommand_BINDER();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UString          *binder;     ///< binder name "external" or "internal"
  UVariableName    *variablename;///< variable
  int              type; ///< type of binding: 0:"function", 1:"var", 2:"event"
  int              nbparam; ///< nb of param in a function binding
};


class UCommand_OPERATOR : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_OPERATOR (UString* oper);                       
  virtual ~UCommand_OPERATOR ();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UString          *oper; ///< operator name
};

class UCommand_WAIT : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_WAIT(UExpression* expression); 
  virtual ~UCommand_WAIT();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UExpression      *expression; ///< Expression
  
  UFloat           endtime; ///< time to stop waiting.
};

class UCommand_EMIT : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_EMIT(UVariableName* eventname, UNamedParameters *parameters, UExpression *duration=0); 
  virtual ~UCommand_EMIT();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UVariableName      *eventname;   ///< Name of the event
  UNamedParameters   *parameters;  ///< list of parameters  
  UExpression        *duration;

  const char         *eventnamestr; ///< char* of the event name
  bool               firsttime; ///< true for the first execution
  UFloat             targetTime; ///< time of the end of the signal
  int                eventid; ///< id used to uniquely identify the event
};

class UCommand_WAIT_TEST : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_WAIT_TEST(UExpression* test); 
  virtual ~UCommand_WAIT_TEST();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UExpression      *test;       ///< test
  int              nbTrue;      ///< nb of times the test is true
  UFloat           startTrue;   ///< time of the last 'true'
};

class UCommand_INCDECREMENT : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_INCDECREMENT(UCommandType type, UVariableName *variablename);                      
  virtual ~UCommand_INCDECREMENT();
  virtual UVariableName** refVarName() { return &variablename; };

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UVariableName     *variablename;   ///< variable
};

class UCommand_DEF : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_DEF (UDefType deftype,
                UVariableName *variablename,           
                UNamedParameters *parameters,
                UCommand* command); 
  UCommand_DEF (UDefType deftype,
                UString *device,           
                UNamedParameters *parameters);  
  UCommand_DEF (UDefType deftype,
                UVariableList *variablelist);                    

  virtual ~UCommand_DEF();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UVariableName    *variablename;   ///< variable
  UNamedParameters *parameters; ///< list of parameters
  UCommand         *command;    ///< Command definition
  UString          *device;     ///< device name in a "def device {...}"
  UVariableList    *variablelist; ///< list of variables in a multi def command
  UDefType         deftype;      ///< type of definition (var, function, event)
};


class UCommand_CLASS : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_CLASS (UString *object,
                  UNamedParameters *parameters);

  virtual ~UCommand_CLASS();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UString          *object;   ///< class name
  UNamedParameters *parameters; ///< list of parameters
};


class UCommand_IF : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_IF ( UExpression *test,
                UCommand* command1, 
                UCommand* command2);                 
  virtual ~UCommand_IF();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UExpression      *test;       ///< test
  UCommand         *command1;   ///< Command if
  UCommand         *command2;   ///< Command else (0 if no else)
};

class UCommand_EVERY : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_EVERY ( UExpression *duration,
                   UCommand* command);
  virtual ~UCommand_EVERY();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UExpression      *duration;   ///< duration
  UCommand         *command;    ///< Command 
  
  bool             firsttime; ///< indicates the first time the command is run
  UFloat           starttime; ///< time of the previous pulse
};

class UCommand_TIMEOUT : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_TIMEOUT ( UExpression *duration,
                   UCommand* command);
  virtual ~UCommand_TIMEOUT();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UExpression      *duration;   ///< duration
  UCommand         *command;    ///< Command 
  UString          *tagRef;     ///< ref of the tag to kill.
};

class UCommand_STOPIF : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_STOPIF ( UExpression *condition,
                    UCommand* command);
  virtual ~UCommand_STOPIF();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UExpression      *condition;   ///< condition
  UCommand         *command;     ///< Command 
  UString          *tagRef;      ///< ref of the containing tag
};

class UCommand_FREEZEIF : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_FREEZEIF ( UExpression *condition,
                    UCommand* command);
  virtual ~UCommand_FREEZEIF();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UExpression      *condition;   ///< condition
  UCommand         *command;     ///< Command 
  UString          *tagRef;      ///< ref of the containing tag
};

class UCommand_AT : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_AT ( UCommandType type,
                UExpression *test,
                UCommand* command1, 
                UCommand* command2);                 
  virtual ~UCommand_AT();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UExpression      *test;       ///< test
  UCommand         *command1;   ///< Command if
  UCommand         *command2;   ///< Command else (0 if no else)

  bool             mode;        ///< The command is activated 
                                ///< when the test switch to "mode"
  int              nbTrue;      ///< nb of times the test is true
  UFloat           startTrue;   ///< time of the last 'true'
};

class UCommand_WHILE : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_WHILE ( UCommandType type,
                   UExpression *test,
                   UCommand* command);                 
  virtual ~UCommand_WHILE();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UExpression      *test;       ///< test
  UCommand         *command;    ///< Command 
};

class UCommand_WHENEVER : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_WHENEVER ( UExpression *test,
                      UCommand* command1,
                      UCommand* command2);                 
  virtual ~UCommand_WHENEVER();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UExpression      *test;       ///< test
  UCommand         *command1;   ///< Command ok
  UCommand         *command2;   ///< Command onleave

  int              nbTrue;      ///< nb of times the test is true
  UFloat           startTrue;   ///< time of the last 'true'
  int              nbFalse;     ///< nb of times the test is false
  UFloat           startFalse;  ///< time of the last 'false'
};

class UCommand_LOOP : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_LOOP ( UCommand* command);                 
  virtual ~UCommand_LOOP();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UCommand         *command;    ///< Command 
};

class UCommand_LOOPN : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_LOOPN ( UCommandType type,
                   UExpression* expression,
                   UCommand* command);                 
  virtual ~UCommand_LOOPN();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UExpression      *expression; ///< Expression
  UCommand         *command;    ///< Command 
};

class UCommand_FOREACH : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_FOREACH ( UCommandType type,
                     UVariableName* variablename,
                     UExpression* expression,
                     UCommand* command);                 
  virtual ~UCommand_FOREACH();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UVariableName    *variablename; ///< Name of the iterating variable
  UCommand         *command;      ///< Command 
  UExpression      *expression;   ///< the list to iterate
  UValue           *position;     ///< index in the list
  bool             firsttime;     ///< first execution of the command
};

class UCommand_FOR : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_FOR ( UCommandType type,
                 UCommand* instr1,
                 UExpression* test,
                 UCommand* instr2,
                 UCommand* command);                 
  virtual ~UCommand_FOR();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UCommand         *instr1;     ///< 1st part
  UCommand         *instr2;     ///< 2nd part
  UExpression      *test;       ///< test
  UCommand         *command;    ///< Command 
  bool             first;       ///< true on the first passage
};

class UCommand_NOOP : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_NOOP(bool zerotime = false);                 
  virtual ~UCommand_NOOP();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();
};

class UCommand_LOAD : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_LOAD(UCommand_TREE *mainnode);                 
  virtual ~UCommand_LOAD();

  virtual void print(int l); 

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  UCommandQueue       *loadQueue;    ///< used to load files
  bool                ready; ///< used to alternate exec/non-exec state (a kind of noop)
  UCommand_TREE       *mainnode; ///< node that contains the load command
};

#endif
