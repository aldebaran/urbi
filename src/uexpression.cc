/*! \file uexpression.cc
 *******************************************************************************

 File: uexpression.cc\n
 Implementation of the UExpression class.

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

#if 0
# define ECHO(C) \
  std::cout << C << std::endl
#else
# define ECHO(C)
#endif

#define PING()					\
  ECHO (__FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__)


#include <cmath>
#include "libport/cstdio"

#include <sstream>

#include "libport/cstring"
#include "libport/ref-pt.hh"

#include "parser/uparser.hh"
#include "ubinary.hh"
#include "ucommand.hh"
#include "uasynccommand.hh"
#include "uconnection.hh"
#include "ucopy.hh"
#include "ueventhandler.hh"
#include "ueventcompound.hh"
#include "ueventinstance.hh"
#include "ueventmatch.hh"
#include "uexpression.hh"
#include "ugroup.hh"
#include "userver.hh"
#include "uvariable.hh"
#include "uvariablename.hh"
#include "uobj.hh"

// FIXME: This is code duplication with ucommand.cc, with a slight
// difference: expression do not have a tag, so we get the command's
// one.  It will be trivial to factor when we have cleaner asts.
namespace
{
  UErrorValue
  send_error (UConnection* c, const UCommand* cmd, const UExpression* e,
	      const char* fmt, va_list args)
    __attribute__ ((__format__ (__printf__, 4, 0)));

  /// Report an error, with "!!! " prepended, and "\n" appended.
  /// \param c     the connection to which the message is sent.
  /// \param cmd   the command whose tag will be used.
  /// \param e     the expression whose location will be used.
  /// \param fmt   printf-format string.
  /// \param args  its arguments.
  UErrorValue
  send_error (UConnection* c, const UCommand* cmd, const UExpression* e,
	      const char* fmt, va_list args)
  {
    std::ostringstream o;
    // FIXME: This is really bad if file names have %.  We need
    // something more robust (such using real C++ here instead of C
    // buffers).
    o << "!!! " << e->loc() << ": " << fmt << '\n';
    return c->sendf (cmd->getTag(), o.str ().c_str(), args);
  }

  UErrorValue
  send_error (UConnection* c, const UCommand* cmd, const UExpression* e,
	      const char* fmt, ...)
    __attribute__ ((__format__ (__printf__, 4, 5)));

  UErrorValue
  send_error (UConnection* c, const UCommand* cmd, const UExpression* e,
	      const char* fmt, ...)
  {
    va_list args;
    va_start(args, fmt);
    return send_error (c, cmd, e, fmt, args);
  }

}

MEMORY_MANAGER_INIT(UExpression);
// **************************************************************************
//! UExpression base constructor called by every specific constructor.
void UExpression::initialize()
{
  ADDOBJ(UExpression);

  expression1	   = 0;
  expression2	   = 0;
  str		   = 0;
  id		   = 0;
  variablename	   = 0;
  parameters	   = 0;
  firsteval	   = true;
  isconst	   = false;
  issofttest	   = false;
  staticcache	   = 0;
  tmp_value	   = 0;
  softtest_time = 0;
  dataType  = DATA_UNKNOWN;
}

//! UExpression constructor for numeric value.
/*! The parameter 'type' is required here only for the sake of uniformity
 between all the different constructors.
 */
UExpression::UExpression(const location& l,
			 UExpression::Type t, ufloat v)
 : UAst(l)
{
  assert (t == VALUE);
  initialize();
  val  = v;
  type = t;
  isconst = true;
  dataType   = DATA_NUM;
}


//! UExpression constructor for string value.
/*! The parameter 'type' is required here only for the sake of uniformity
 between all the different constructors.
 */
UExpression::UExpression(const location& l,
			 UExpression::Type t, UString *s)
 : UAst(l)
{
  assert (t == VALUE | t == GROUP);
  initialize();
  str = s;
  type = t;
  isconst = true;
  dataType   = DATA_STRING;
}

//! UExpression constructor for numeric value.
/*! The parameter 'type' is required here only for the sake of uniformity
 between all the different constructors.
 */
UExpression::UExpression(const location& l, UExpression::Type t, UValue *v)
 : UAst(l)
{
  initialize();
  assert (t == VALUE);
  type = t;
  isconst = true;
  dataType   = v->dataType;

  if (v->dataType == DATA_NUM)
    val  = v->val;
  else if (v->dataType == DATA_STRING)
    str = v->str->copy();
  else
  {
    tmp_value = v;
    dataType = DATA_UNKNOWN;
  }
}

//! UExpression constructor for functors
/*! The parameter 'type' is required here only for the sake of uniformity
 between all the different constructors.
 */
UExpression::UExpression(const location& l, UExpression::Type type,
			 UString *oper,
			 UVariableName *variablename)
 : UAst(l)
{
  initialize();
  this->str	     = oper;
  this->variablename = variablename;
  if (variablename)
    this->isconst    = variablename->isstatic;

  this->type	   = type; // should be PROPERTY
  dataType	   = DATA_UNKNOWN;
}

//! UExpression constructor for variable
/*! The parameter 'type' is required here only for the sake of uniformity
 between all the different constructors.
 */
UExpression::UExpression(const location& l,
			 UExpression::Type type, UVariableName* variablename)
 : UAst(l)
{
  initialize();
  this->type	 = type; // should be VARIABLE or
  //ADDR_VARIABLE or GROUPLIST
  if (type == ADDR_VARIABLE)
    dataType = DATA_STRING;
  this->variablename = variablename;
}

//! UExpression constructor for function
/*! The parameter 'type' is required here only for the sake of uniformity
 between all the different constructors.
 */
UExpression::UExpression(const location& l, UExpression::Type type,
			 UVariableName* variablename,
			 UNamedParameters *parameters)
 : UAst(l)
{
  initialize();
  this->type	     = type; // should be FUNCTION
  dataType	     = DATA_UNKNOWN;
  this->variablename = variablename;
  this->parameters   = parameters;
}

//! UExpression constructor for function
/*! The parameter 'type' is required here only for the sake of uniformity
 between all the different constructors.
 */
UExpression::UExpression(const location& l, UExpression::Type type,
			 UNamedParameters *parameters)
  : UAst(l)
{
  initialize();
  this->type	     = type; // should be LIST
  dataType	     = DATA_LIST;
  this->parameters   = parameters;
}

//! UExpression constructor for composed operation
UExpression::UExpression(const location& l, UExpression::Type type,
			 UExpression* expression1,
			 UExpression* expression2)
 : UAst(l)
{
  initialize();
  this->type	    = type;

  this->expression1 = expression1;
  this->expression2 = expression2;

  // Compile time calculus reduction
  if (expression1 && expression2
      && expression1->type == VALUE && expression1->dataType == DATA_NUM
      && expression2->type == VALUE && expression2->dataType == DATA_NUM
      && (type == PLUS || type == MINUS ||
	  type == MULT || type == DIV ||
	  type == EXP)
      && ! (type == DIV && expression2->val == 0))
  {
    switch (type)
    {
      case PLUS:
	val = expression1->val + expression2->val;
	break;
      case MINUS:
	val = expression1->val - expression2->val;
	break;
      case MULT:
	val = expression1->val * expression2->val;
	break;
      case DIV:
	val = expression1->val / expression2->val;
	break;
      case EXP:
	val = pow (expression1->val , expression2->val);
	break;
      default:
	// This case is not possible, but GCC does not seem to be
	// able to infer it.
	abort();
    }

    this->type = VALUE;
    this->isconst = true;
    dataType = DATA_NUM;
    delete expression1;
    this->expression1 = 0;
    delete expression2;
    this->expression2 = 0;
  }

  if (type == NEG
      && expression1
      && expression1->type == VALUE
      && expression1->dataType == DATA_NUM)
  {
    val = - expression1->val;

    this->type = VALUE;
    this->isconst = true;
    dataType = DATA_NUM;
    delete expression1;
    this->expression1 = 0;
  }
}

//! UExpression constructor.
UExpression::UExpression(const location& l,
			 UExpression::Type _type, UExpression* _expression1,
			 UExpression* _expression2,
			 UVariableName* _variablename,
			 UNamedParameters* _parameters, UString* _str,
			 UString* _id, UExpression* _softtest_time,
			 UValue* _staticcache, UDataType _dataType,
			 ufloat _val, bool _isconst,
			 bool _issofttest, bool _firsteval,
			 UValue* _tmp_value)
  : UAst(l),
    type (_type), dataType (_dataType), val (_val), str (_str),
    tmp_value (_tmp_value), id (_id), firsteval (_firsteval),
    isconst (_isconst), issofttest (_issofttest),staticcache (_staticcache),
    expression1 (_expression1), expression2 (_expression2),
    variablename (_variablename), parameters (_parameters),
    softtest_time (_softtest_time)
{
  ADDOBJ (UExpression);
}

//! UExpression destructor.
UExpression::~UExpression()
{
  FREEOBJ(UExpression);

  delete str;
  delete id;
  delete expression1;
  delete expression2;
  delete variablename;
  delete parameters;
  delete tmp_value;
}

//! UExpression hard copy function
UExpression*
UExpression::copy() const
{
  return new UExpression (loc(),
			  type, ucopy (expression1), ucopy (expression2),
			  ucopy (variablename), ucopy (parameters),
			  ucopy (str), ucopy (id), ucopy (softtest_time),
			  ucopy (staticcache), dataType, val, isconst,
			  issofttest, firsteval, tmp_value);
}

#define UDEBUG_EXPR(What)			\
  do {						\
    if (What)					\
    {						\
      ::urbiserver->debug(#What "= {");		\
      What->print();				\
      ::urbiserver->debug("} ");		\
    }						\
  } while (0)

#define UDEBUG_EXPR_I(What)			\
  do {						\
    if (What)					\
    {						\
      ::urbiserver->debug(#What "= {");		\
      What->print(t+3);				\
      ::urbiserver->debug("} ");		\
    }						\
  } while (0)

//! Print the expression
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UExpression::print(unsigned t)
{
  debug(t, "[Type:E%d ", type);
  if (isconst)
    debug("(const) ");
  if (type == VALUE && dataType == DATA_NUM)
  {
    std::ostringstream tstr;
    tstr << "val=" << val << " ";
    debug("%s", tstr.str().c_str());
  }
  if (str)
    debug("str='%s' ", str->str());
  if (id)
    debug("id='%s' ", id->str());
  UDEBUG_EXPR_I (expression1);
  UDEBUG_EXPR_I (expression2);
  UDEBUG_EXPR (variablename);
  UDEBUG_EXPR (parameters);
  debug("] ");
}

//! UExpression evaluation.
/*! The connection parameter is necessary to access the variable hash table for
 expressions who contain variables. It is also used to display error
 messages.
 The UCommand is used to retrieve a message tag if necessary.
 */
UValue*
UExpression::eval (UCommand *command,
		   UConnection *connection)
{
  // This is a hack to be backward compatible with existing code
  UEventCompound* ec = 0;
  UValue* v = eval (command, connection, ec);
  delete ec;
  return v;
}

// The following macro allow to factor a bit the following monster
// function.  Of course the actual fix is to factor the function for
// real, say using a visitor.

#define ENSURE_TYPES_1(Type1)			\
  do {						\
    if (!e1 || e1->dataType != Type1)		\
    {						\
      delete e1;				\
      return 0;					\
    }						\
  } while (0)

#define ENSURE_TYPES_2(Type1, Type2)		\
  do {						\
    if (!e1 || e1->dataType != Type1		\
	|| !e2 || e2->dataType != Type2)	\
    {						\
      delete e1;				\
      delete e2;				\
      return 0;					\
    }						\
  } while (0)

#define ENSURE_TYPES_3(Type1, Type2, Type3)	\
  do {						\
    if (!e1 || e1->dataType != Type1		\
	|| !e2 || e2->dataType != Type2		\
	|| !e3 || e3->dataType != Type3)	\
    {						\
      delete e1;				\
      delete e2;				\
      delete e3;				\
      return 0;					\
    }						\
  } while (0)

/// If Kind is non null, require that \a Lhs and \a Rhs be defined.
#define ENSURE_COMPARISON(Kind, Lhs, Rhs)				\
  do {									\
    if (*Kind								\
	&& !(Lhs && Lhs->dataType == DATA_NUM				\
	     && Rhs && Rhs->dataType == DATA_NUM))			\
    {									\
      send_error(connection, command, this,				\
		 Kind " comparisons must be between numerical values");	\
      delete Lhs;							\
      delete Rhs;							\
      return 0;								\
    }									\
  } while (0)


//! UExpression evaluation.
/*! The connection parameter is necessary to access the variable hash table for
 expressions who contain variables. It is also used to display error
 messages.
 The UCommand is used to retrieve a message tag if necessary.
 */
UValue*
UExpression::eval (UCommand *command,
		   UConnection *connection,
		   UEventCompound*& ec)
{
  if (issofttest && softtest_time)
  {
    UValue *v = softtest_time->eval(command, connection);
    if (v)
      softtest_time->val = v->val;
    else
      softtest_time->val = 0;
    delete v;
  }

  switch (type)
  {
    case LIST:
      return eval_LIST (command, connection);

    case GROUP:
      return eval_GROUP (command, connection);

    case VALUE:
    {
      if (tmp_value)
	return tmp_value->copy(); // hack to be able to handle complex
      // return types from function calls

      UValue* ret = new UValue();
      ret->dataType = dataType;
      if (dataType == DATA_NUM)
	ret->val = val;
      if (dataType == DATA_STRING)
	ret->str = new UString(str);
      return ret;
    }

    case ADDR_VARIABLE:
    {
      UValue* ret = new UValue();
      ret->dataType = DATA_STRING;
      // hack here to be able to use objects pointeurs

      ret->str = new UString (variablename->buildFullname(command,
							  connection));
      return ret;
    }

    case VARIABLE:
      return eval_VARIABLE (command, connection, ec);

    case PROPERTY:
    {
      UVariable *variable = variablename->getVariable(command, connection);
      if (!variablename->getFullname())
	return 0;
      if (!variable)
      {
	send_error(connection, command, this,
		   "Unknown identifier: %s",
		   variablename->getFullname()->str());
	return 0;
      }

      UValue* ret = new UValue();

      if (STREQ(str->str(), "rangemin"))
      {
	ret->dataType = DATA_NUM;
	ret->val = variable->rangemin;
	return ret;
      }

      if (STREQ(str->str(), "rangemax"))
      {
	ret->dataType = DATA_NUM;
	ret->val = variable->rangemax;
	return ret;
      }

      if (STREQ(str->str(), "speedmin"))
      {
	ret->dataType = DATA_NUM;
	ret->val = variable->speedmin;
	return ret;
      }

      if (STREQ(str->str(), "speedmax"))
      {
	ret->dataType = DATA_NUM;
	ret->val = variable->speedmax;
	return ret;
      }

      if (STREQ(str->str(), "delta"))
      {
	ret->dataType = DATA_NUM;
	ret->val = variable->delta;
	return ret;
      }

      if (STREQ(str->str(), "unit"))
      {
	ret->dataType = DATA_STRING;
	ret->str = new UString( variable->unit );
	return ret;
      }

      if (STREQ(str->str(), "blend"))
      {
	ret->dataType = DATA_STRING;
	ret->str = new UString(name (variable->blendType));
	return ret;
      }

      send_error(connection, command, this,
                 "Unknown property: %s", str->str());
      return 0;
    }

    case FUNCTION:
      return eval_FUNCTION (command, connection, ec);

    case PLUS:
    {
      UValue* e1 = expression1->eval(command, connection);
      UValue* e2 = expression2->eval(command, connection);

      if (e1==0 || e1->dataType == DATA_VOID ||
	  e2==0 || e2->dataType == DATA_VOID)
      {
	delete e1;
	delete e2;
	return 0;
      }
      UValue* ret = e1->add(e2);
      if (expression1->isconst && expression2->isconst)
	this->isconst = true;

      delete e1;
      delete e2;
      return ret;
    }

#define EVAL_ARITHMETICS(Value)				\
    {								\
      UValue* e1 = expression1->eval(command, connection);	\
      UValue* e2 = expression2->eval(command, connection);	\
      ENSURE_TYPES_2 (DATA_NUM, DATA_NUM);			\
      UValue* res = new UValue();				\
      res->dataType = DATA_NUM;					\
      res->val = Value;						\
      delete e1;						\
      delete e2;						\
      return res;						\
    }

    case MINUS:
      EVAL_ARITHMETICS(e1->val - e2->val);

    case MULT:
      EVAL_ARITHMETICS(e1->val * e2->val);

    case MOD:
      EVAL_ARITHMETICS(fmod(e1->val, e2->val));

    case EXP:
      EVAL_ARITHMETICS(pow(e1->val, e2->val));

#undef EVAL_ARITHMETICS


    case DIV:
    {
      UValue* e1 = expression1->eval(command, connection);
      UValue* e2 = expression2->eval(command, connection);
      ENSURE_TYPES_2 (DATA_NUM, DATA_NUM);

      if (e2->val == 0)
      {
	send_error(connection, command, this, "Division by zero");
	return 0;
      }

      UValue* ret = new UValue();
      ret->dataType = DATA_NUM;
      ret->val = e1->val / e2->val;
      delete e1;
      delete e2;
      return ret;
    }


    case NEG:
    {
      UValue* e1 = expression1->eval(command, connection);
      ENSURE_TYPES_1 (DATA_NUM);
      UValue* ret = new UValue();
      ret->dataType = DATA_NUM;
      ret->val = -e1->val;
      delete e1;
      return ret;
    }

    case COPY:
    {
      UValue* e1 = expression1->eval(command, connection);
      if (e1==0)
	return 0;
      UValue* ret = e1->copy();
      if (ret->dataType == DATA_BINARY)
      {
	UBinary *b = new UBinary(ret->refBinary->ref()->bufferSize, 0);
	if (!b || b->buffer == 0)
	  return 0;
	b->parameters = ret->refBinary->ref()->parameters->copy();

	libport::RefPt<UBinary> *ref = new libport::RefPt<UBinary>(b);
	if (!ref)
	  return 0;

	memcpy(b->buffer,
	       ret->refBinary->ref()->buffer,
	       ret->refBinary->ref()->bufferSize);

	LIBERATE(ret->refBinary);
	ret->refBinary = ref;
      }
      delete e1;
      return ret;
    }

    case TEST_REQ:
    {
      UValue* e1 = expression1->eval(command, connection);
      UValue* e2 = expression2->eval(command, connection);
      ENSURE_COMPARISON ("Approximate", e1, e2);
      UValue* ret = new UValue();
      ret->dataType = DATA_NUM;

      UVariable *variable =
	::urbiserver->getVariable(MAINDEVICE, "epsilontilde");
      if (variable)
	ret->val = (ABSF(e1->val - e2->val) <= variable->value->val );
      else
	ret->val = 0;

      delete e1;
      delete e2;
      return ret;
    }

    case TEST_DEQ:
    {
      UValue* e1 = expression1->eval(command, connection);
      UValue* e2 = expression2->eval(command, connection);
      ENSURE_COMPARISON ("Approximate", e1, e2);
      UValue* ret = new UValue();
      ret->dataType = DATA_NUM;

      ufloat d1 = 0;
      if (expression1->type == VARIABLE)
      {
	UVariable *v =
	  expression1->variablename->getVariable(command, connection);
	if (v)
	  d1 = v->delta;
      }
      ufloat d2 = 0;
      if (expression2->type == VARIABLE)
      {
	UVariable *v =
	  expression2->variablename->getVariable(command, connection);
	if (v)
	  d2 = v->delta;
      }

      ret->val = (ABSF(e1->val - e2->val) <= d1+d2 );

      delete e1;
      delete e2;
      return ret;
    }

    case TEST_PEQ:
    {
      UValue* e1 = expression1->eval(command, connection);
      UValue* e2 = expression2->eval(command, connection);
      ENSURE_COMPARISON ("Approximate", e1, e2);
      if (e2->val == 0 || e1->val == 0)
      {
	send_error(connection, command, this, "Division by zero");
	return 0;
      }

      UValue* ret = new UValue();
      ret->dataType = DATA_NUM;

      UVariable *v = ::urbiserver->getVariable(MAINDEVICE, "epsilonpercent");
      if (v)
	ret->val = (ABSF( 1 - (e1->val / e2->val)) <= v->value->val );
      else
	ret->val = 0;

      delete e1;
      delete e2;
      return ret;
    }

    /*----------------------.
    | Boolean expressions.  |
    `----------------------*/

#define EVAL_COMPARISON(Kind, Comparison)			\
    {								\
      UValue* lhs = expression1->eval(command, connection);	\
      UValue* rhs = expression2->eval(command, connection);	\
      ENSURE_COMPARISON(Kind, lhs, rhs);			\
      UValue* res = 0;						\
      if (lhs && rhs)						\
      {								\
	res = new UValue();					\
	res->dataType = DATA_NUM;				\
	res->val = Comparison;					\
      }								\
      delete lhs;						\
      delete rhs;						\
      return res;						\
    }

    case TEST_EQ:
      EVAL_COMPARISON ("", lhs->equal(rhs));

    case TEST_NE:
      EVAL_COMPARISON ("", !lhs->equal(rhs));

    case TEST_GT:
      EVAL_COMPARISON ("Numerical", lhs->val > rhs->val);

    case TEST_GE:
      EVAL_COMPARISON ("Numerical", lhs->val >= rhs->val);

    case TEST_LT:
      EVAL_COMPARISON ("Numerical", lhs->val < rhs->val);

    case TEST_LE:
      EVAL_COMPARISON ("Numerical", lhs->val <= rhs->val);

#undef EVAL_COMPARISON

    case TEST_BANG:
    {
      UEventCompound* ec1 = 0;
      UValue* e1 = expression1->eval(command, connection, ec1);

      if (e1==0)
      {
	delete e1;
	delete ec1;
	return 0;
      }

      UValue* ret = new UValue();
      ret->dataType = DATA_NUM;

      ret->val = e1->val == 0 ? 1 : 0;

      delete e1;
      if (ec1)
	ec = new UEventCompound (UEventCompound::EC_BANG, ec1);
      return ret;
    }

#define EVAL_BIN_BOOLEAN(Op, Command, OpAbort)				\
    {									\
      UValue* ret = new UValue();					\
      if (!ret)								\
	return 0;							\
      ret->dataType = DATA_NUM;						\
									\
      UEventCompound* ec1 = 0;						\
      UValue* e1 = expression1->eval(command, connection, ec1);		\
      if (!e1)								\
      {									\
	delete ec1;							\
	return 0;							\
      }									\
									\
      if ((int)e1->val OpAbort 0)					\
      {                                                                 \
        if (ec1)                                                        \
          ec = ec1;                                                     \
        ret->val =  (ufloat) (true OpAbort false);                      \
        delete e1;                                                      \
        return ret;                                                     \
      }                                                                 \
									\
      UEventCompound* ec2 = 0;						\
      UValue* e2 = expression2->eval(command, connection, ec2);		\
      if (!e2)								\
      {									\
	delete ret;							\
	delete e1;							\
	delete ec1;							\
	delete ec2;							\
	return 0;							\
      }									\
									\
      ret->val = (ufloat) ( ((int)e1->val) Op ((int)e2->val) );		\
									\
      if (ec1)								\
	if (ec2)							\
	  ec = new UEventCompound (Command, ec1, ec2);			\
	else								\
	  ec = new UEventCompound (Command, ec1, new UEventCompound (e2)); \
      else								\
	if (ec2)							\
	  ec = new UEventCompound (Command, new UEventCompound (e1), ec2); \
	else								\
	  ec = new UEventCompound (Command, new UEventCompound (e1),	\
				   new UEventCompound (e2));		\
									\
      delete e1;							\
      delete e2;							\
      return ret;							\
    }

    case TEST_AND:
      EVAL_BIN_BOOLEAN(&&, UEventCompound::EC_AND, ==);

    case TEST_OR:
      EVAL_BIN_BOOLEAN(||, UEventCompound::EC_OR, !=);

#undef EVAL_BIN_BOOLEAN
    default:
      return 0;
  }
}


UValue*
UExpression::eval_FUNCTION_EXEC_OR_LOAD (UCommand* command,
					 UConnection* connection)
{
#if 0
  PING();
  command->print (10);
#endif

  assert (STREQ(variablename->id->str(), "exec")
	  || STREQ(variablename->id->str(), "load"));

  bool in_load = STREQ(variablename->id->str(), "load");
  UValue* e1 = parameters->expression->eval(command, connection);
  ENSURE_TYPES_1 (DATA_STRING);

  // send string in the queue
  ::urbiserver->systemcommands = false;
  if (!connection->stack.empty())
    connection->functionTag = new UString("__Funct__");
  UParser& p = connection->parser();

  ECHO("Parsing " << variablename->id->str() << ':' << e1->str->str());

  if (in_load)
    p.process (::urbiserver->find_file (e1->str->str()));
  else
    p.process(reinterpret_cast<const ubyte*>(e1->str->str()),
	      e1->str->len());

#if 0
  ECHO("Parsed " << variablename->id->str() << ':' << e1->str->str());
  p.commandTree->print (3);
#endif

  if (connection->functionTag)
    {
      delete connection->functionTag;
      connection->functionTag = 0;
    }
  ::urbiserver->systemcommands = true;

  if (p.errorMessage[0])
    {
      // a parsing error occured
      if (p.commandTree)
	{
	  delete p.commandTree;
	  p.commandTree = 0;
	}
      connection->send(p.errorMessage, "error");
    }

  if (p.commandTree)
    {
      command->morph = p.commandTree;
      command->persistant = false;
      p.commandTree = 0;
    }
  else
    {
      send_error(connection, command, this,
		 (in_load
		  ? "Error loading file: %s"
		  : "Error parsing string: %s"),
		 e1->str->str());
      return 0;
    }

  delete e1;
  UValue* ret = new UValue();
  ret->dataType = DATA_VOID;
  return ret;
}

UValue*
UExpression::eval_FUNCTION (UCommand *command,
			    UConnection *connection,
			    UEventCompound*& ec)
{
  assert (type == FUNCTION);
  UString* funname = variablename->buildFullname(command, connection);

  // Event detection
  UEventHandler* eh;
  if (parameters)
    eh = kernel::findEventHandler(funname, parameters->size());
  else
    eh = kernel::findEventHandler(funname, 0);
  if (eh)
  {
    UValue* ret = new UValue(ufloat(1));
    if (eh->noPositive())
      ret->val = 0; // no active (positive) event in the handler

    ec = new UEventCompound (new UEventMatch (funname,
					      parameters,
					      command,
					      connection));
    return ret;
  }

  if (parameters == 0 &&
      (variablename->id->equal("freemem")
       || variablename->id->equal("power")
       || variablename->id->equal("cpuload")
       || variablename->id->equal("time")))
  {
    UValue* ret = new UValue();
    ret->dataType = DATA_NUM;

    if (STREQ(variablename->id->str(), "freemem"))
      ret->val = availableMemory - usedMemory;
    else if (STREQ(variablename->id->str(), "time"))
      ret->val = connection->server->getTime();
    else if (STREQ(variablename->id->str(), "cpuload"))
      ret->val = connection->server->cpuload;
    else if (STREQ(variablename->id->str(), "power"))
      ret->val = connection->server->getPower();

    return ret;
  }

  if (parameters && parameters->size() == 2)
  {
    if (STREQ(variablename->id->str(), "save"))
    {
      UValue* e1 = parameters->expression->eval(command, connection);
      UValue* e2 = parameters->next->expression->eval(command, connection);

      ENSURE_TYPES_2 (DATA_STRING, DATA_STRING);
      UValue* ret = new UValue();
      ret->dataType = DATA_VOID;

      // save to file

      if (connection->server->saveFile(e1->str->str(),
				       e2->str->str()) == UFAIL)
      {
	send_error(connection, command, this, "Cannot save to the file %s",
		   e1->str->str());
	delete ret;
	ret = 0;
      }

      delete e1;
      delete e2;
      return ret;
    }

    if (STREQ(variablename->id->str(), "getIndex"))
    {
      UValue* e1 = parameters->expression->eval(command, connection);
      UValue* e2 = parameters->next->expression->eval(command, connection);

      ENSURE_TYPES_2 (DATA_LIST, DATA_NUM);
      UValue* e3 = e1->liststart;
      int indx = 0;
      while (e3 && indx != (int)e2->val)
      {
	e3 = e3->next;
	++indx;
      }
      UValue* ret = 0;
      if (!e3)
      {
	send_error(connection, command, this, "Index out of range");
	ret = 0;
      }
      else
	ret = e3->copy();

      delete e1;
      delete e2;
      return ret;
    }

    if (STREQ(variablename->id->str(), "cat"))
    {
      UValue* e1 = parameters->expression->eval(command, connection);
      UValue* e2 = parameters->next->expression->eval(command, connection);

      ENSURE_TYPES_2 (DATA_LIST, DATA_LIST);

      UValue* ret = e1->copy();
      UValue* e3 = ret->liststart;
      while (e3 && e3->next)
	e3 = e3->next;
      UValue *e4 = e2->liststart;

      if (e4)
	if (!e3)
	{
	  ret->liststart = e4->copy();
	  e3 = ret->liststart;
	  e4 = e4->next;

	  while (e4)
	  {
	    e3->next = e4->copy();
	    e3 = e3->next;
	    e4 = e4->next;
	  }
	}
	else
	  while (e4)
	  {
	    e3->next = e4->copy();
	    e3 = e3->next;
	    e4 = e4->next;
	  }

      delete e1;
      delete e2;
      return ret;
    }

  }

  if (parameters && parameters->size() == 1)
  {
    if (STREQ(variablename->id->str(), "strlen"))
    {
      UValue* e1 = parameters->expression->eval(command, connection);
      ENSURE_TYPES_1 (DATA_STRING);
      UValue* ret = new UValue();
      ret->dataType = DATA_NUM;
      ret->val = e1->str->len();

      for (int i=0;i<e1->str->len()-1; ++i)
	if (e1->str->str()[i] == '\\' &&
	    e1->str->str()[i+1] == '"')
	  --ret->val;

      delete e1;
      return ret;
    }

    if (STREQ(variablename->id->str(), "head"))
    {
      UValue* e1 = parameters->expression->eval(command, connection);
      ENSURE_TYPES_1 (DATA_LIST);
      UValue* ret;
      if (e1->liststart)
	ret = e1->liststart->copy();
      else
	ret = new UValue();

      delete e1;
      return ret;
    }

    if (STREQ(variablename->id->str(), "tail"))
    {
      UValue* e1 = parameters->expression->eval(command, connection);
      ENSURE_TYPES_1 (DATA_LIST);
      UValue* ret = 0;
      if (e1->liststart)
      {
	ret = new UValue();
	ret->dataType = DATA_LIST;
	UValue* e2 = e1->liststart->next;
	UValue* e3 = ret;
	if (e2)
	{
	  e3->liststart = e2->copy();
	  e2 = e2->next;
	  e3 = e3->liststart;
	  while (e2)
	  {
	    e3->next = e2->copy();
	    e2 = e2->next;
	    e3 = e3->next;
	  }
	}
      }
      else
	ret = e1->copy();

      delete e1;
      return ret;
    }

    if (STREQ(variablename->id->str(), "size"))
    {
      UValue* e1 = parameters->expression->eval(command, connection);
      ENSURE_TYPES_1 (DATA_LIST);
      UValue* ret = new UValue(0.0);

      if (e1->liststart)
      {
	UValue* e2 = e1->liststart;
	while (e2)
	{
	  e2 = e2->next;
	  ret->val = ret->val + 1;
	}
      }

      delete e1;
      return ret;
    }


    if (STREQ(variablename->id->str(), "isdef"))
    {
      UValue* ret = new UValue();
      ret->dataType = DATA_NUM;
      ret->val = 0;

      if (parameters->expression->type == VARIABLE)
	if ((parameters->expression->variablename->
	     getVariable(command, connection)) ||
	    (parameters->expression->variablename->isFunction(command,
							      connection)))
	  ret->val = 1;

      return ret;
    }

    if (STREQ(variablename->id->str(), "isvoid"))
    {
      UValue* ret = new UValue();
      ret->dataType = DATA_NUM;
      ret->val = 0;

      if (parameters->expression->type == VARIABLE)
      {
	UVariable* v = parameters->expression->
	  variablename->getVariable(command, connection);
	if (v==0 || v->value==0)
	  return ret;
	if (v->value->dataType == DATA_VOID)
	  ret->val = 1;
      }

      return ret;
    }

    if (STREQ(variablename->id->str(), "loadwav"))
    {
      UValue* e1 = parameters->expression->eval(command, connection);
      ENSURE_TYPES_1 (DATA_STRING);
      UValue* ret = new UValue();
      ret->dataType = DATA_BINARY;
      UCommandQueue* loadQueue = new UCommandQueue (4096, 1048576, false);
      // load file
      if (connection->server->loadFile(e1->str->str(),
				       loadQueue) == UFAIL)
      {
	send_error(connection, command, this,
		   "Cannot load the file %s", e1->str->str());
	delete ret;
	delete loadQueue;
	ret = 0;
      }
      else
      {
	UBinary *b =
	  new UBinary(loadQueue->dataSize(),
		      new UNamedParameters
		      (new UExpression(loc(), VALUE,
				       new UString("wav")),
		       0));
	memcpy(b->buffer,
	       loadQueue->pop(loadQueue->dataSize()),
	       loadQueue->dataSize());

	ret->refBinary = new libport::RefPt<UBinary>(b);
	delete loadQueue;
      }

      return ret;
    }


    // Exec and load are exactly the same thing with one difference:
    // exec parse the string, and load, the file whose name is given.
    if (STREQ(variablename->id->str(), "exec")
	|| STREQ(variablename->id->str(), "load"))
      return eval_FUNCTION_EXEC_OR_LOAD (command, connection);
  }

  if (parameters &&
      parameters->size() == 3 &&
      STREQ(variablename->id->str(), "strsub"))
  {
    UValue* e1 = parameters->expression->eval(command, connection);
    UValue* e2 = parameters->next->expression->eval(command, connection);
    UValue* e3 = parameters->next->next->expression->eval(command, connection);

    ENSURE_TYPES_3 (DATA_STRING, DATA_NUM, DATA_NUM);
    UValue* ret = new UValue();
    ret->dataType = DATA_STRING;

    if (STREQ(variablename->id->str(), "strsub"))
      ret->str = new UString(e1->str->ext((int)e2->val, (int)e3->val));

    delete e1;
    delete e2;
    delete e3;
    return ret;
  }

  if (parameters
      && parameters->size() == 2
      && STREQ(variablename->id->str(), "atan2"))
  {
    UValue* e1 = parameters->expression->eval(command, connection);
    UValue* e2 = parameters->next->expression->eval(command, connection);
    ENSURE_TYPES_2 (DATA_NUM, DATA_NUM);
    UValue* ret = new UValue();
    ret->dataType = DATA_NUM;

    if (STREQ(variablename->id->str(), "atan2"))
      ret->val = atan2(e1->val, e2->val);

    delete e1;
    delete e2;
    return ret;
  }
  if (parameters
      && parameters->size() == 1
      && (false
	  || STREQ(variablename->id->str(), "sin")
	  || STREQ(variablename->id->str(), "asin")
	  || STREQ(variablename->id->str(), "cos")
	  || STREQ(variablename->id->str(), "acos")
	  || STREQ(variablename->id->str(), "tan")
	  || STREQ(variablename->id->str(), "atan")
	  || STREQ(variablename->id->str(), "sgn")
	  || STREQ(variablename->id->str(), "abs")
	  || STREQ(variablename->id->str(), "exp")
	  || STREQ(variablename->id->str(), "log")
	  || STREQ(variablename->id->str(), "round")
	  || STREQ(variablename->id->str(), "random")
	  || STREQ(variablename->id->str(), "trunc")
	  || STREQ(variablename->id->str(), "sqr")
	  || STREQ(variablename->id->str(), "sqrt")
	  || STREQ(variablename->id->str(), "string")))
  {
    UValue* e1 = parameters->expression->eval(command, connection);
    ENSURE_TYPES_1 (DATA_NUM);

    if (STREQ(variablename->id->str(), "string"))
    {
      UValue* ret = new UValue();
      ret->dataType = DATA_STRING;
      char errorString[256];
      sprintf(errorString, "%d", (int)e1->val);
      ret->str = new UString(errorString);

      delete e1;
      return ret;
    }

    UValue* ret = new UValue();
    ret->dataType = DATA_NUM;

    if (STREQ(variablename->id->str(), "sin"))
      ret->val = sin(e1->val);
    else if (STREQ(variablename->id->str(), "asin"))
      ret->val = asin(e1->val);
    else if (STREQ(variablename->id->str(), "cos"))
      ret->val = cos(e1->val);
    else if (STREQ(variablename->id->str(), "acos"))
      ret->val = acos(e1->val);
    else if (STREQ(variablename->id->str(), "tan"))
      ret->val = tan(e1->val);
    else if (STREQ(variablename->id->str(), "atan"))
      ret->val = atan(e1->val);
    else if (STREQ(variablename->id->str(), "sgn"))
    {
      // FIXME: No value set of 0.
      if (e1->val>0)
	ret->val =1;
      else if (e1->val<0)
	ret->val = -1;
    }
    else if (STREQ(variablename->id->str(), "abs"))
      ret->val = fabs(e1->val);
    else if (STREQ(variablename->id->str(), "random"))
    {
      int range =  (int)e1->val;
      if (range)
        ret->val = rand()%range;
      else
        ret->val = 0;
    }
    else if (STREQ(variablename->id->str(), "round"))
    {
      if (e1->val>=0)
	ret->val = (ufloat)(int)(e1->val+0.5);
      else
	ret->val = (ufloat)(int)(e1->val-0.5);
    }
    else if (STREQ(variablename->id->str(), "trunc"))
      ret->val = (ufloat)(int)(e1->val);
    else if (STREQ(variablename->id->str(), "exp"))
      ret->val = exp(e1->val);
    else if (STREQ(variablename->id->str(), "sqr"))
      ret->val = e1->val*e1->val;
    else if (STREQ(variablename->id->str(), "sqrt"))
    {
      if (e1->val<0)
      {
	send_error(connection, command, this, "Negative square root");
	return 0;
      }
      ret->val = sqrt(e1->val);
    }
    else if (STREQ(variablename->id->str(), "log"))
    {
      if (e1->val<0)
      {
	send_error(connection, command, this, "Negative logarithm");
	return 0;
      }
      ret->val = log(e1->val);
    }

    delete e1;
    return ret;
  }

  // default = unknown.
  funname = variablename->buildFullname(command, connection);
  if (!variablename->getFullname()) return 0;
  HMfunctiontab::iterator hmf =
    ::urbiserver->functiontab.find(funname->str());
  if (hmf != ::urbiserver->functiontab.end())
  {
    send_error(connection, command, this,
	       "Custom function call in expressions"
	       " not allowed in kernel 1");
    return 0;
  }

  send_error(connection, command, this,
	     "Error with function eval: %s [nb param=%d]",
	     variablename->getFullname()->str(),
	     parameters ? parameters->size() : 0);
  return 0;
}


UValue*
UExpression::eval_GROUP (UCommand *command, UConnection *connection)
{
  assert (type == GROUP);
  UValue* ret = new UValue();
  HMgrouptab::iterator retr = connection->server->grouptab.find(str->str());
  if (retr != connection->server->grouptab.end())
  {
    ret->dataType = DATA_LIST;
    UValue* e1 = 0;
    std::list<UString*>::iterator it = retr->second->members.begin();
    if (it !=  retr->second->members.end())
    {
      UExpression *e = new UExpression (loc(), GROUP, (*it)->copy());
      UValue* e2 = e->eval(command, connection);
      delete e;
      if (e2->dataType == DATA_VOID)
      {
	ret->liststart = new UValue((*it)->str());
	delete e2;
      }
      else
      {
	delete ret;
	ret = e2;
      }

      e1 = ret->liststart;
      while (e1->next)
	e1 = e1->next;
      ++it;
    }

    while (it != retr->second->members.end())
    {
      UExpression *e = new UExpression (loc(), GROUP, (*it)->copy());
      UValue* e2 = e->eval(command, connection);
      delete e;
      if (e2->dataType == DATA_VOID)
      {
	e1->next = new UValue((*it)->str());
	delete e2;
	e1 = e1->next;
      }
      else
      {
	UValue* e3 = e2;
	e2 = e2->liststart;
	while (e2)
	{
	  e1->next = e2->copy();
	  e1 = e1->next;
	  e2 = e2->next;
	}
	delete e3;
      }
      ++it;
    }
  }
  return ret;
}

UValue*
UExpression::eval_LIST (UCommand *command, UConnection *connection)
{
  assert (type == LIST);
  UValue* ret = new UValue();
  ret->dataType = DATA_LIST;
  UNamedParameters *pevent = parameters;
  UValue* e1 = ret;
  if (pevent)
  {
    e1->liststart = pevent->expression->eval(command, connection);

    if (!e1->liststart)
    {
      delete ret;
      return 0;
    }

    if (e1->liststart->dataType == DATA_OBJ)
    {
      send_error(connection, command, this,
		 "Objects not allowed in lists with Kernel 1. "
		 "Use lists of keys and object maps instead");
      delete ret;
      return 0;
    }

    e1 = e1->liststart;
    pevent = pevent->next;
  }

  while (pevent)
  {
    e1->next = pevent->expression->eval(command, connection);
    if (e1->next==0)
    {
      delete ret;
      return 0;
    }
    if (e1->next->dataType == DATA_OBJ)
    {
      send_error(connection, command, this,
		 "Objects not allowed in lists with Kernel 1. "
		 "Use lists of keys and object maps instead");
      delete ret;
      return 0;
    }

    pevent = pevent->next;
    e1 = e1->next;
  }
  return ret;
}

UValue*
UExpression::eval_VARIABLE (UCommand *command,
			    UConnection *connection,
			    UEventCompound*& ec)
{
  assert (type == VARIABLE);
  UVariable *variable = variablename->getVariable(command, connection);
  if (!variablename->getFullname())
    return 0;
  UString* devicename = variablename->getDevice();
  UValue* ret = 0;
  const char* varname;
  if (!variable)
  {
    varname = variablename->getFullname()->str();

    // Event detection
    UEventHandler* eh =
      kernel::findEventHandler(variablename->getFullname(), 0);
    if (eh)
    {
      ret = new UValue(ufloat(1));
      if (eh->noPositive())
	ret->val = 0; // no active (positive) event in the handler

      ec = new UEventCompound
	(new UEventMatch
	 (variablename->getFullname(),
	  0,
	  command,
	  connection));
      return ret;
    }

    // virtual variables
    const char* devname = variablename->getDevice()->str();
    bool ambiguous;
    UVariable *vari = 0;
    HMobjtab::iterator itobj;
    if ((itobj = ::urbiserver->objtab.find(devname)) !=
	::urbiserver->objtab.end())
    {
      vari = itobj->second->
	searchVariable(variablename->getMethod()->str(), ambiguous);
      if (ambiguous)
      {
	send_error(connection, command, this,
		   "Ambiguous multiple inheritance on variable %s",
		   variablename->getFullname()->str());
	return new UValue();
      }

      variable = vari;
      if (vari)
      {
	devicename->update(vari->method);
	variablename->device->update(vari->method);
	variablename->buildFullname(command, connection);
      }
    }
  }

  if (!variable)
  {
    char* pp = const_cast<char*>(strchr(varname, '.'));
    if (!pp)
      pp = const_cast<char*> (varname);
    char* p = const_cast<char*>(strstr(pp, "__"));
    if (p==varname) // the name starts by __, we skip
      p = const_cast<char*>(strstr(varname+2, "__"));

    if (p)
    {
      // could be a list index.... (dirty hack)
      p[0]=0;

      HMvariabletab::iterator hmv =::urbiserver->variabletab.find(varname);
      while (hmv == ::urbiserver->variabletab.end()
	     && p)
      {
	p[0]='_';
	p = const_cast<char*>(strstr(p+2, "__"));
	if (p)
	{
	  p[0] = 0;
	  hmv = ::urbiserver->variabletab.find(varname);
	}
      }
      if (hmv != ::urbiserver->variabletab.end() && p)
      {
	UVariable* tmpvar = hmv->second;
	tmpvar->get (); // to trigger the UNotifyAccess
	if (tmpvar->value->dataType == DATA_LIST)
	{
	  // welcome to C-string hacking grand master area
	  // Don't let unaccompagnied children see this.
	  UValue* xval = tmpvar->value->liststart;
	  int index;
	  int curr;
	  p[0] = '_';
	  p = p + 2; // beginning of the index
	  char* p2 = strchr(p, '_');
	  while (p)
	  {
	    if (p2)
	      p2[0] = 0;
	    index = atoi(p);
	    curr = 0;
	    while (curr != index && xval)
	    {
	      xval = xval->next;
	      ++curr;
	    }
	    if (!xval)
	    {
	      send_error(connection, command, this, "Index out of range");
	      return new UValue();
	    }
	    else
	    {
	      if (p2)
	      {
		if (xval->dataType != DATA_LIST)
		{
		  send_error(connection, command, this, "Invalid index usage");
		  return new UValue();
		}
		else
		  xval = xval->liststart;
	      }
	    }

	    if (p2)
	      p2[0] = '_';
	    if (p2)
	      p = p2 + 2;
	    else
	      p = 0;
	    if (p)
	      p2 = strchr(p, '_');
	  }

	  return xval->copy();
	}
      }
      if (p)
	p[0] = '_';
    }

    send_error(connection, command, this, "Unknown identifier: %s",
	       variablename->getFullname()->str());
    return 0;
  }

  if (!variablename->isstatic || firsteval)
  {
    ret = variable->get()->copy();
    // error evaluation for variables (target-val)
    if (variablename->varerror
	&& variable->value->dataType == DATA_NUM)
      ret->val = variable->previous - ret->val;

    if (variablename->varin
	&& variable->value->dataType == DATA_NUM)
    {
      ret->val = variable->target;
    }

    // normalized variables
    if (variablename->isnormalized
	&& variable->rangemax != variable->rangemin)
    {
      if (variable->rangemin == -UINFINITY ||
	  variable->rangemax ==  UINFINITY ||
	  variable->value->dataType != DATA_NUM)
      {
	send_error(connection, command, this,
		   "Impossible to normalize: "
		   "no range defined for variable %s",
		   variablename->getFullname()->str());
	return 0;
      }

      ret->val = (ret->val - variable->rangemin) /
	(variable->rangemax - variable->rangemin);
    }

    if (variablename->deriv != UVariableName::UNODERIV)
    {
      if (variable->autoUpdate)
      {
	if (variablename->deriv == UVariableName::UTRUEDERIV)
	  variablename->deriv = UVariableName::UDERIV;
	if (variablename->deriv == UVariableName::UTRUEDERIV2)
	  variablename->deriv = UVariableName::UDERIV2;
      }

      switch (variablename->deriv)
      {
	case UVariableName::UNODERIV:
	  // Impossible case, but GCC cannot infer it.
	  abort ();

	case UVariableName::UDERIV:
	  ret->val = 1000. * (variable->previous - variable->previous2)/
	    (::urbiserver->previousTime - ::urbiserver->previous2Time);
	  break;

	case UVariableName::UDERIV2:
	  ret->val = 1000000. * 2 *
	    ( variable->previous  * (::urbiserver->previous2Time-
				     ::urbiserver->previous3Time)
	      - variable->previous2 *(::urbiserver->previousTime-
				      ::urbiserver->previous3Time)
	      +
	      variable->previous3 *
	      (::urbiserver->previousTime  -
	       ::urbiserver->previous2Time)
	      ) / (	 (::urbiserver->previous2Time
			  - ::urbiserver->previous3Time) *
			 (::urbiserver->previousTime
			  - ::urbiserver->previous3Time) *
			 (::urbiserver->previousTime
			  - ::urbiserver->previous2Time) );

	  break;

	case UVariableName::UTRUEDERIV:
	  ret->val = 1000. *
	    (variable->get()->val - variable->valPrev)/
	    (::urbiserver->currentTime
	     - ::urbiserver->previousTime);
	  break;

	case UVariableName::UTRUEDERIV2:
	  ret->val = 1000000. * 2 *
	    ( variable->get()->val	*
	      (::urbiserver->previousTime -
	       ::urbiserver->previous2Time) -
	      variable->valPrev	*
	      (::urbiserver->currentTime  -
	       ::urbiserver->previous2Time) +
	      variable->valPrev2	*
	      (::urbiserver->currentTime-
	       ::urbiserver->previousTime)
	      ) / (	 (::urbiserver->previousTime
			  - ::urbiserver->previous2Time) *
			 (::urbiserver->currentTime
			  - ::urbiserver->previous2Time) *
			 (::urbiserver->currentTime
			  - ::urbiserver->previousTime) );
	  break;
      }
    }
  }

  // static variables
  if (variablename->isstatic)
    if (firsteval)
    {
      firsteval = false;
      staticcache = ret->copy();
      if (!staticcache)
	return 0;
    }
    else
      ret = staticcache->copy();

  return ret;
}


/** UExpression scan to notify variables and events of async dependencies */
UErrorValue
UExpression::asyncScan(UASyncCommand *cmd,
		       UConnection *c)
{
  UVariable *variable;
  UNamedParameters *pevent;
  HMfunctiontab::iterator hmf;
  std::list<UString*>::iterator it;
  UEventHandler* eh;
  UString* fullname;
  const char* varname;
  int nbargs;

  switch (type)
  {
    case LIST:
      pevent = parameters;
      while (pevent)
      {
	if (pevent->expression->asyncScan(cmd, c) == UFAIL)
	  return UFAIL;
	pevent = pevent->next;
      }
      return USUCCESS;

    case VARIABLE:

      variable = variablename->getVariable(cmd, c);
      fullname = variablename->getFullname();
      if (!fullname)
	return UFAIL;
      varname  = variablename->getFullname()->str();

      if (!variable)
      {
	// Is this a virtual variable?
	const char* devname = variablename->getDevice()->str();
	bool ambiguous;

	HMobjtab::iterator itobj;
	if ((itobj = ::urbiserver->objtab.find(devname)) !=
	    ::urbiserver->objtab.end())
	{
	  variable = itobj->second->
	    searchVariable(variablename->getMethod()->str(),
			   ambiguous);

	  if (ambiguous)
	    return UFAIL;

	  if (variable)
	  {
	    send_error(c, cmd, this,
		       "Pure virtual variables not allowed"
		       " in asynchronous tests.");
	    return UFAIL;
	  }
	}
      }

      if (!variable)
      {
	// Is this a list?
	char* p = const_cast<char*> (strstr (varname, "__"));
	char* pnext = p;
	while (pnext)
	{
	  p = pnext;
	  pnext = strstr(p + 2, "__");
	}
	if (p)
	{
	  // could be a list index
	  p[0] = 0;
	  HMvariabletab::iterator hmv =
	    ::urbiserver->variabletab.find(varname);
	  p[0] = '_';
	  if (hmv != ::urbiserver->variabletab.end())
	    variable = hmv->second;
	}
      }

      if (variable)
      {
	// It is a variable
	variable->registerCmd(cmd);
	return USUCCESS;
      }
      else
      {
	// It is not a variable but it could be an event
	eh = kernel::findEventHandler(fullname, 0);
	if (eh)
	{
	  eh->registerCmd(cmd);
	  return USUCCESS;
	}
	else
	  if (c->server->defcheck) //strict
	    return UFAIL;
	  else
	  {
	    variable = new UVariable (fullname->str (),
				      new UValue (ufloat (0)));
	    variable->registerCmd(cmd);
	    return USUCCESS;
	  }
      }

    case PROPERTY:

      variable = variablename->getVariable(cmd, c);
      fullname = variablename->getFullname();
      if (!fullname)
	return UFAIL;
      if (!variable)
	return UFAIL;
      variable->registerCmd(cmd);
      return USUCCESS;

    case FUNCTION:

      fullname = variablename->buildFullname (cmd, c);
      nbargs = 0;
      if (parameters)
	nbargs = parameters->size ();
      eh = kernel::findEventHandler(fullname, nbargs);

      if (eh)
      {
	// This is an event
	eh->registerCmd (cmd);
	return USUCCESS;
      }
      else
      {
	// is it a known kernel function?
	if ( kernel::isCoreFunction (variablename->id))
	{
	  UNamedParameters* param = parameters;
	  while (param)
	  {
	    if (param->expression)
	      if (param->expression->asyncScan(cmd, c) == UFAIL)
		return UFAIL;
	    param = param->next;
	  }
	  return USUCCESS;
	}
	else
	{
	  // it is not a known function
	  if (c->server->defcheck) //strict
	    return UFAIL;
	  else
	  {
	    eh = new UEventHandler (fullname, nbargs);
	    eh->registerCmd (cmd);
	    return USUCCESS;
	  }
	}
      }

    default:
      if (expression1)
	if (expression1->asyncScan(cmd, c) == UFAIL)
	  return UFAIL;

      if (expression2)
	if (expression2->asyncScan(cmd, c) == UFAIL)
	  return UFAIL;
      return USUCCESS;
  }
}
