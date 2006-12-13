/// \file   fwd.hh
/// \brief  Forward declarations.

#ifndef KERNEL1_FWD_HH
# define KERNEL1_FWD_HH
namespace urbi
{
  class baseURBIStarter;
  class UGenericCallback;
}

class UASyncCommand;
class UASyncRegister;
class UAtCandidate;
class UBinary;
class UBinder;
class UCommandQueue;
class UConnection;
class UContext;
class UEvent;
class UEventCompound;
class UEventHandler;
class UEventHandler;
class UEventInstance;
class UEventMatch;
class UExpression;
class UFunction;
class UGenericCallback;
class UGhostConnection;
class UGroup;
class UImage;
class UList;
class UMonitor;
class UMultiEventInstance;
class UNamedParameters;
class UObj;
class UParser;
class UQueue;
class UServer;
class USound;
class UString;
class UTest;
class UValue;
class UVariable;
class UVariableList;
class UVariableName;

// From ucommand.hh.
class UCommand;
class UCommand_ALIAS;
class UCommand_ASSIGN_BINARY;
class UCommand_ASSIGN_PROPERTY;
class UCommand_ASSIGN_VALUE;
class UCommand_AT;
class UCommand_AUTOASSIGN;
class UCommand_BINDER;
class UCommand_CLASS;
class UCommand_DEF;
class UCommand_DEVICE_CMD;
class UCommand_ECHO;
class UCommand_EMIT;
class UCommand_EVERY;
class UCommand_EXPR;
class UCommand_FOR;
class UCommand_FOREACH;
class UCommand_FREEZEIF;
class UCommand_GROUP;
class UCommand_IF;
class UCommand_INCDECREMENT;
class UCommand_INHERIT;
class UCommand_LOOP;
class UCommand_LOOPN;
class UCommand_NEW;
class UCommand_NOOP;
class UCommand_OPERATOR;
class UCommand_OPERATOR_ID;
class UCommand_OPERATOR_VAR;
class UCommand_RETURN;
class UCommand_STOPIF;
class UCommand_TIMEOUT;
class UCommand_TREE;
class UCommand_WAIT;
class UCommand_WAIT_TEST;
class UCommand_WHENEVER;
class UCommand_WHILE;


#endif //! KERNEL1_FWD_HH
