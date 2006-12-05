/*! \file ucommand.hh
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

#ifndef UCOMMAND_HH
# define UCOMMAND_HH

# include <list>

# include "fwd.hh"

# include "ucopy.hh"
# include "utypes.hh"
# include "ustring.hh"
# include "ucommandqueue.hh"
# include "uasynccommand.hh"

# include "uexpression.hh"
# include "unamedparameters.hh"
# include "uvariablelist.hh"
# include "uvalue.hh"
# include "uobj.hh"
# include "ugroup.hh"
# include "uproperty.hh"
# include "uvariablename.hh"
# include "ubinary.hh"
# include "ucallid.hh"


// ****************************************************************************
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

  virtual void print(int);

  virtual UCommandStatus execute(UConnection*);
  virtual UCommand* copy();
  UErrorValue       copybase(UCommand *command);

  UCommand*         scanGroups(UVariableName** (UCommand::*refName)(), bool);
  virtual UVariableName** refVarName()  { return 0; };
  virtual UVariableName** refVarName2()  { return 0; };

  const std::string& getTag()  {return tag;}
  void setTag(const std::string& tag);
  void setTag(UCommand *b); //faster than the one above
  void unsetTag();

  bool isBlocked();
  bool isFrozen();

  /// Type of the command.
  UCommandType     type;
  /// Status of the command
  UCommandStatus   status;



  /// list of flags of tagged commands
  UNamedParameters *flags;

  /// the UCommand_TREE that owns the UCommand
  UCommand_TREE    *up;
  /// position in the owning UCommand_TREE
  UCommand         **position;
  /// stores the target UCommand in case of morphing
  UCommand         *morph;
  /// tells if the command should be deleted once it is UCOMPLETED (useful for loops)
  bool             persistant;
  /// true if the command has been marked for deletion in a stop command.
  bool             toDelete;
  /// used to put the whole tree in bg mode (after a morphing from a "at" or "whenever").
  bool             background;

  /// start time
  ufloat           startTime;
  /// expression used to store the flags params
  UExpression      *flagExpr1;
  /// expression used to store the flags params
  UExpression      *flagExpr2;
  /// expression used to store the flags params
  UExpression      *flagExpr4;
  /// in case of timeout or condout, stores the type of the flag (timeout:0), (condout:1).
  int              flagType;
  /// nb of times the flag test is true
  int              flag_nbTrue2;
  /// time of the last 'true' for "stop"
  ufloat           flag_startTrue2;
  /// nb of times the flag test is true
  int              flag_nbTrue4;
  /// time of the last 'true' for "freeze"
  ufloat           flag_startTrue4;
  /// true when the command is part of a morphed structure
  bool             morphed;

public:

  /// used by commands to build
  static const int MAXSIZE_TMPMESSAGE = 65536;
  /// internal
  HMvariabletab::iterator hmi;

  private:

  /// Command tag
  std::string      tag;
  /// Ptr to tag info concerning us
  TagInfo* tagInfo;
  std::list<UCommand *>::iterator tagInfoPtr; //for fast deletion


  /// Protection against copy
  UCommand (const UCommand &c);
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

  virtual UCommandStatus execute(UConnection*);
  virtual UCommand*      copy();

  ///D eletes sub commands marked for deletion
  void deleteMarked();

  /// Left side of the compound command.
  UCommand         *command1;
  /// Right side of the compound command.
  UCommand         *command2;
  /// context identificator for function calls
  UCallid          *callid;
  /// node type (AND, PIPE, ...)
  UNodeType        node;
  /// The state of execution of command1 and command2.
  URunlevel        runlevel1, runlevel2;
  /// belonging connection
  UConnection      *connection;
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
  virtual UVariableName** refVarName()  { return &variablename; };
  virtual UVariableName** refVarName2()  { return &expression->variablename; };



  /// variable name
  UVariableName    *variablename;
  /// associated variable
  UVariable        *variable;
  /// Expression
  UExpression      *expression;
  /// list of parameters
  UNamedParameters *parameters;
  /// method in the varname
  UString          *method;
  /// device in the varname
  UString          *devicename;

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

  /// stored temporary phase for cos modificator
  UExpression      *tmp_phase;
  /// stored temporary time=0 for direct assignment
  UExpression      *tmp_time;
  /// time limit in case of timeout modificator
  ufloat           endtime;
  /// start value for modificators
  ufloat           startval;
  /// target value for modificators
  ufloat           targetval;
  /// start time for modificators
  ufloat           starttime;
  /// target time for modificators
  ufloat           targettime;
  /// ideal value to reach for the next iteration. Used for +error
  ufloat           idealval;
  /// speed for modificators
  ufloat           speed;
  /// accel for the accel modificator
  ufloat           accel;
  /// minimal speed in a movement
  ufloat           speedmin;

  /// destination values
  ufloat           *valtmp;
  /// nb destinaton values
  int              nbval;

  /// true when the assign is finished
  bool             finished;
  /// true when the motion profile is done
  bool             profileDone;
  /// true for 'valn' type assignments
  bool             isvaln;
  /// true if +error is set for the command
  bool             errorFlag;
  /// true on the first passage
  bool             first;
  /// true when a nbAssign-- has to be done ondelete
  bool             assigned;
  /// is the def prefix used?
  bool             defkey;

private:

  UErrorValue      processModifiers(UConnection* connection,
				    ufloat currentTime);

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
  virtual UVariableName** refVarName()  { return &variablename; };


  /// variable name
  UVariableName    *variablename;
  /// associated variable
  UVariable        *variable;
  /// Binary container
  URefPt<UBinary>  *refBinary;

  /// method in the varname
  UString          *method;
  /// device in the varname
  UString          *devicename;
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
  virtual UVariableName** refVarName()  { return &variablename; };


  /// variable name
  UVariableName    *variablename;
  /// associated variable
  UVariable        *variable;
  /// Property operateur
  UString          *oper;
  /// assigned expression
  UExpression      *expression;

  /// method in the varname
  UString          *method;
  /// device in the varname
  UString          *devicename;
};

class UCommand_AUTOASSIGN : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_AUTOASSIGN (UVariableName* variablename,
			UExpression* expression,
			int assigntype);
  virtual ~UCommand_AUTOASSIGN();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection*);
  virtual UCommand*      copy();

  /// Name of the iterating variable
  UVariableName    *variablename;
  /// the list to iterate
  UExpression      *expression;
  /// is it +=(0) or -=(1)?
  int              assigntype;
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
  virtual UVariableName** refVarName()  { return &expression->variablename; };

  /// Expression
  UExpression      *expression;
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

  /// Expression
  UExpression      *expression;
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

  /// Expression
  UExpression      *expression;
  /// list of parameters
  UNamedParameters *parameters;
  /// tag of the connection to echo to.
  UString          *connectionTag;
};

class UCommand_NEW : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_NEW(UVariableName* varname,
	       UString* obj,
	       UNamedParameters *parameters,
	       bool noinit=false);
  virtual ~UCommand_NEW();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// Object name
  UString          *id;
  /// Identifier name
  UVariableName    *varname;
  /// Object
  UString          *obj;
  /// list of parameters
  UNamedParameters *parameters;
  /// tells if 'init' should be called
  bool             noinit;
  /// true when a remote new is waiting
  bool             remoteNew;
};

class UCommand_ALIAS : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_ALIAS (UVariableName* aliasname,
		  UVariableName* id,
		  bool eraseit=false);

  virtual ~UCommand_ALIAS();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// alias name
  UVariableName           *aliasname;
  /// identifier
  UVariableName           *id;
  /// unalias command
  bool                    eraseit;
};

class UCommand_INHERIT : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_INHERIT (UVariableName* subclass,
		    UVariableName* theclass,
		    bool eraseit=false);

  virtual ~UCommand_INHERIT();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// subclass that inherits
  UVariableName           *subclass;
  /// parent class
  UVariableName           *theclass;
  /// uninherit command
  bool                    eraseit;
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

  /// identifier
  UString              *id;
  /// list of group members
  UNamedParameters     *parameters;
  /// type of group command group/addgroup/delgroup
  int            grouptype;
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

  /// operator name
  UString          *oper;
  /// identifier
  UString          *id;
};

class UCommand_DEVICE_CMD : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_DEVICE_CMD  (UVariableName* device,
			ufloat *cmd);
  virtual ~UCommand_DEVICE_CMD();
  virtual UVariableName** refVarName()  { return &variablename; };

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// the device name embedded in a var name
  UVariableName    *variablename;
  /// the command (on, off, ...)
  ufloat           cmd;
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

  /// operator name
  UString          *oper;
  /// variable
  UVariableName    *variablename;
  /// cached variable, used by undef
  UVariable        *variable;
  /// cached function, used by undef
  UFunction        *fun;
};

class UCommand_BINDER : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_BINDER (UVariableName* objname,
		   UString* binder,
		   int type,
		   UVariableName* variablename,
		   int nbparam=0);
  virtual ~UCommand_BINDER();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// binder name "external" or "internal"
  UString          *binder;
  /// variable
  UVariableName    *variablename;
  /// name of the uobject controling the binding
  UVariableName    *objname;
  /// type of binding: 0:"function", 1:"var", 2:"event"
  int              type;
  /// nb of param in a function binding
  int              nbparam;
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

  /// operator name
  UString          *oper;
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

  /// Expression
  UExpression      *expression;

  /// time to stop waiting.
  ufloat           endtime;
};

class UCommand_EMIT : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_EMIT(UVariableName* eventname,
		UNamedParameters *parameters, UExpression *duration=0);
  virtual ~UCommand_EMIT();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// terminate and clean the event when the emit ends.
  void removeEvent ();

  /// Name of the event
  UVariableName      *eventname;
  /// list of parameters
  UNamedParameters   *parameters;
  UExpression        *duration;

  /// char* of the event name
  const char         *eventnamestr;
  /// true for the first execution
  bool               firsttime;
  /// time of the end of the signal
  ufloat             targetTime;
  /// the attached UEvent
  UEvent*            event;
  /// the associated UEventHandler
  UEventHandler*     eh;
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

  /// test
  UExpression      *test;
  /// nb of times the test is true
  int              nbTrue;
  /// time of the last 'true'
  ufloat           startTrue;
  /// true when the command has not been executed yet
  bool             firsttime;
  /// list of UMultiEvent candidates
  std::list<UAtCandidate*> candidates;

private:
  /// used for optimization
  bool reloop_;
};

class UCommand_INCDECREMENT : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_INCDECREMENT(UCommandType type, UVariableName *variablename);
  virtual ~UCommand_INCDECREMENT();
  virtual UVariableName** refVarName()  { return &variablename; };

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// variable
  UVariableName     *variablename;
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

  /// variable
  UVariableName    *variablename;
  /// list of parameters
  UNamedParameters *parameters;
  /// Command definition
  UCommand         *command;
  /// device name in a "def device {...}"
  UString          *device;
  /// list of variables in a multi def command
  UVariableList    *variablelist;
  /// type of definition (var, function, event)
  UDefType         deftype;
};


class UCommand_CLASS : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_CLASS (UString *object,
		  UNamedParameters *parameters);

  virtual ~UCommand_CLASS();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection*);
  virtual UCommand*      copy();

  /// class name
  UString          *object;
  /// list of parameters
  UNamedParameters *parameters;
};


class UCommand_IF : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_IF (UExpression *test,
	       UCommand* command1,
	       UCommand* command2);
  virtual ~UCommand_IF();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// test
  UExpression      *test;
  /// Command if
  UCommand         *command1;
  /// Command else (0 if no else)
  UCommand         *command2;
};

class UCommand_EVERY : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_EVERY (UExpression *duration,
		  UCommand* command);
  virtual ~UCommand_EVERY();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// duration
  UExpression      *duration;
  /// Command
  UCommand         *command;

  /// indicates the first time the command is run
  bool             firsttime;
  /// time of the previous pulse
  ufloat           starttime;
};

class UCommand_TIMEOUT : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_TIMEOUT (UExpression *duration,
		    UCommand* command);
  virtual ~UCommand_TIMEOUT();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection*);
  virtual UCommand*      copy();

  /// duration
  UExpression      *duration;
  /// Command
  UCommand         *command;
  /// ref of the tag to kill.
  UString          *tagRef;
};

class UCommand_STOPIF : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_STOPIF (UExpression *condition,
		   UCommand* command);
  virtual ~UCommand_STOPIF();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// condition
  UExpression      *condition;
  /// Command
  UCommand         *command;
  /// ref of the containing tag
  UString          *tagRef;
};

class UCommand_FREEZEIF : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_FREEZEIF (UExpression *condition,
		     UCommand* command);
  virtual ~UCommand_FREEZEIF();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection*);
  virtual UCommand*      copy();

  /// condition
  UExpression      *condition;
  /// Command
  UCommand         *command;
  /// ref of the containing tag
  UString          *tagRef;
};

class UCommand_AT : public UCommand, public UASyncCommand
{
public:
  MEMORY_MANAGED;

  UCommand_AT (UCommandType type,
	       UExpression *test,
	       UCommand* command1,
	       UCommand* command2);
  virtual ~UCommand_AT();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// test
  UExpression      *test;
  /// Command if
  UCommand         *command1;
  /// Command else (0 if no else)
  UCommand         *command2;
  /// true when the command has not been executed yet
  bool             firsttime;
  /// list of UMultiEvent candidates
  std::list<UAtCandidate*> candidates;

private:
  /// used for optimization
  bool reloop_;
};

class UCommand_WHILE : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_WHILE (UCommandType type,
		   UExpression *test,
		   UCommand* command);
  virtual ~UCommand_WHILE();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// test
  UExpression      *test;
  /// Command
  UCommand         *command;
};

class UCommand_WHENEVER : public UCommand, public UASyncCommand
{
public:
  MEMORY_MANAGED;

  UCommand_WHENEVER (UExpression *test,
		      UCommand* command1,
		      UCommand* command2);
  virtual ~UCommand_WHENEVER();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// test
  UExpression      *test;
  /// Command ok
  UCommand         *command1;
  /// Command onleave
  UCommand         *command2;
  /// true when the command has not been executed yet
  bool             firsttime;
  /// list of UMultiEvent candidates
  std::list<UAtCandidate*> candidates;

  void noloop()  {theloop_ = 0;};

private:
  /// used for optimization
  bool reloop_;
  /// true when 'whenever' has triggered and is still active
  bool active_;
  /// the "loop command1" command
  UCommand* theloop_;
};

class UCommand_LOOP : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_LOOP (UCommand* command);
  virtual ~UCommand_LOOP();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection*);
  virtual UCommand*      copy();

  /// Command
  UCommand         *command;
  /// non zero if the loop belongs to a whenever command.
  UCommand*        whenever_hook;
};

class UCommand_LOOPN : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_LOOPN (UCommandType type,
		   UExpression* expression,
		   UCommand* command);
  virtual ~UCommand_LOOPN();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// Expression
  UExpression      *expression;
  /// Command
  UCommand         *command;
};

class UCommand_FOREACH : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_FOREACH (UCommandType type,
		     UVariableName* variablename,
		     UExpression* expression,
		     UCommand* command);
  virtual ~UCommand_FOREACH();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// Name of the iterating variable
  UVariableName    *variablename;
  /// Command
  UCommand         *command;
  /// the list to iterate
  UExpression      *expression;
  /// index in the list
  UValue           *position;
  /// first execution of the command
  bool             firsttime;
};

class UCommand_FOR : public UCommand
{
public:
  MEMORY_MANAGED;

  UCommand_FOR (UCommandType type,
		UCommand* instr1,
		UExpression* test,
		UCommand* instr2,
		UCommand* command);
  virtual ~UCommand_FOR();

  virtual void print(int l);

  virtual UCommandStatus execute(UConnection *connection);
  virtual UCommand*      copy();

  /// 1st part
  UCommand         *instr1;
  /// 2nd part
  UCommand         *instr2;
  /// test
  UExpression      *test;
  /// Command
  UCommand         *command;
  /// true on the first passage
  bool             first;
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

  /// To load files.
  UCommandQueue       loadQueue;
private:
  /// To alternate exec/non-exec state (a kind of noop).
  bool ready;
  /// The load command.
  UCommand_TREE       *mainnode;
};

#endif
