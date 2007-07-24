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
// #define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include <cmath>
#include "libport/cstdio"

#include <sstream>

#include "libport/assert.hh"
#include "libport/cstring"
#include "libport/ref-pt.hh"

#include "kernel/userver.hh"
#include "kernel/uconnection.hh"
#include "kernel/uvalue.hh"
#include "kernel/uvariable.hh"

#include "ast/ast.hh"

#include "parser/uparser.hh"
#include "ucommandqueue.hh"
#include "ubinary.hh"
#include "ucommand.hh"
#include "uasynccommand.hh"
#include "ucopy.hh"
#include "ueventhandler.hh"
#include "ueventcompound.hh"
#include "ueventinstance.hh"
#include "ueventmatch.hh"
#include "uexpression.hh"
#include "ugroup.hh"
#include "unamedparameters.hh"
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
    return c->sendf (cmd->getTag(), o.str().c_str(), args);
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
UExpression::UExpression(const location& l, UExpression::Type t, ufloat v)
 : UAst(l)
{
  passert (t, t == VALUE);
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
UExpression::UExpression(const location& l, UExpression::Type t, UString *s)
 : UAst(l)
{
  passert (t, t == VALUE || t == GROUP);
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
  passert (t, t == VALUE);
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
UExpression::UExpression(const location& l, UExpression::Type t,
			 UString *o,
			 UVariableName *v)
 : UAst(l)
{
  passert (t, t == PROPERTY);
  initialize();
  str = o;
  variablename = v;
  if (variablename)
    isconst = variablename->isstatic;
  type = t;
  dataType = DATA_UNKNOWN;
}

//! UExpression constructor for variable
/*! The parameter 'type' is required here only for the sake of uniformity
 between all the different constructors.
 */
UExpression::UExpression(const location& l,
			 UExpression::Type t, UVariableName* v)
 : UAst(l)
{
  // FIXME: The comment used to accept GROUPLIST, which AD translated
  // into GROUP.  What should it be?
  passert (t, t == ADDR_VARIABLE || t == GROUP || t == VARIABLE);
  initialize();
  type = t;
  if (t == ADDR_VARIABLE)
    dataType = DATA_STRING;
  variablename = v;
}

//! UExpression constructor for function
/*! The parameter 'type' is required here only for the sake of uniformity
 between all the different constructors.
 */
UExpression::UExpression(const location& l, UExpression::Type t,
			 UVariableName* v,
			 UNamedParameters *p)
 : UAst(l)
{
  passert (t, t == FUNCTION);
  initialize();
  type = t;
  dataType = DATA_UNKNOWN;
  variablename = v;
  parameters = p;
}

//! UExpression constructor for function
/*! The parameter 'type' is required here only for the sake of uniformity
 between all the different constructors.
 */
UExpression::UExpression(const location& l, UExpression::Type t,
			 UNamedParameters *p)
  : UAst(l)
{
  passert (t, t == LIST);
  initialize();
  type = t;
  dataType = DATA_LIST;
  parameters = p;
}

//! UExpression constructor for composed operation
UExpression::UExpression(const location& l, UExpression::Type t,
			 UExpression* e1, UExpression* e2)
 : UAst(l)
{
  initialize();
  // Compile time calculus reduction
  if (e1 && e2
      && e1->type == VALUE && e1->dataType == DATA_NUM
      && e2->type == VALUE && e2->dataType == DATA_NUM
      && (t == PLUS || t == MINUS || t == MULT || t == DIV || t == EXP)
      && ! (t == DIV && e2->val == 0))
  {
    switch (t)
    {
      case PLUS:
	val = e1->val + e2->val;
	break;
      case MINUS:
	val = e1->val - e2->val;
	break;
      case MULT:
	val = e1->val * e2->val;
	break;
      case DIV:
	val = e1->val / e2->val;
	break;
      case EXP:
	val = pow (e1->val , e2->val);
	break;
      default:
	// This case is not possible, but GCC does not seem to be
	// able to infer it.
	pabort (t);
    }
    type = VALUE;
    isconst = true;
    dataType = DATA_NUM;
    delete e1;
    delete e2;
  }
  else if (t == NEG && e1 && e1->type == VALUE && e1->dataType == DATA_NUM)
  {
    val = -e1->val;
    type = VALUE;
    isconst = true;
    dataType = DATA_NUM;
    delete e1;
  }
  else
  {
    type = t;
    expression1 = e1;
    expression2 = e2;
  }
}

//! UExpression constructor.
UExpression::UExpression(const location& l,
			 UExpression::Type _type, UExpression* e1,
			 UExpression* e2,
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
    expression1 (e1), expression2 (e2),
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

namespace
{
  const char*
  to_string (UExpression::Type s)
  {
    switch (s)
    {
#define CASE(K) case UExpression::K: return #K;
      CASE (VALUE);
      CASE (VARIABLE);
      CASE (LIST);
      CASE (GROUP);
      CASE (ADDR_VARIABLE);
      CASE (FUNCTION);
      CASE (PLUS);
      CASE (MINUS);
      CASE (MULT);
      CASE (DIV);
      CASE (MOD);
      CASE (EXP);
      CASE (NEG);
      CASE (COPY);
      CASE (PROPERTY);
      CASE (EVENT);
      CASE (TEST_EQ);
      CASE (TEST_REQ);
      CASE (TEST_PEQ);
      CASE (TEST_DEQ);
      CASE (TEST_NE);
      CASE (TEST_GT);
      CASE (TEST_GE);
      CASE (TEST_LT);
      CASE (TEST_LE);
      CASE (TEST_BANG);
      CASE (TEST_AND);
      CASE (TEST_OR);
#undef CASE
    }
    // Pacify warnings.
    pabort ("unexpected case:" << s);
  }
}

//! Print the expression
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UExpression::print(unsigned t)
{
  debug(t, "[Type: %s ", to_string (type));
  if (isconst)
    debug("(const) ");
  if (type == VALUE && dataType == DATA_NUM)
  {
    std::ostringstream o;
    o << "val=" << val << " ";
    debug("%s", o.str().c_str());
  }
  if (str)
    debug("str='%s' ", str->c_str());
  if (id)
    debug("id='%s' ", id->c_str());
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
      send_error(connection, command, this, "Invalid type"); \
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
      send_error(connection, command, this, "Invalid type"); \
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
      send_error(connection, command, this, "Invalid type"); \
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
    softtest_time->val = v ? v->val : 0;
    delete v;
  }

  switch (type)
  {
    case LIST:
      return eval_LIST (command, connection);

    case GROUP:
      return eval_GROUP (command, connection);

    case VALUE:
      if (tmp_value)
	// hack to be able to handle complex
	return tmp_value->copy();
      else if (dataType == DATA_NUM)
	return new UValue (val);
      else
	return new UValue (str->c_str());

    case ADDR_VARIABLE:
      // Hack here to be able to use objects pointeurs.
      return new UValue(variablename->buildFullname(command, connection)->c_str());

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
		   variablename->getFullname()->c_str());
	return 0;
      }

      if (*str == "rangemin")
	return new UValue (variable->rangemin);

      if (*str == "rangemax")
	return new UValue (variable->rangemax);

      if (*str == "speedmin")
	return new UValue (variable->speedmin);

      if (*str == "speedmax")
	return new UValue (variable->speedmax);

      if (*str == "delta")
	return new UValue (variable->delta);

      if (*str == "unit")
	return new UValue (variable->getUnit().c_str());

      if (*str == "blend")
	return new UValue (name(variable->blendType));

      send_error(connection, command, this,
		 "Unknown property: %s", str->c_str());
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
	if (e1 && e1->dataType == DATA_VOID
	    || e2 && e2->dataType == DATA_VOID)
	  send_error(connection, command, this, "Invalid type");
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

#define EVAL_ARITHMETICS(Value)					\
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
	if (ret->refBinary->ref()->parameters)
	  b->parameters = ret->refBinary->ref()->parameters->copy();
	else
	  b->parameters = 0;
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
#ifdef ENABLE_DEBUG_TRACES
  PING();
  if (command)
    command->print (10);
  PING();
#endif

  passert (variablename->id->c_str(),
	   *variablename->id == "exec" || *variablename->id == "load");
  bool in_load = *variablename->id == "load";

  UValue* e1 = parameters->expression->eval(command, connection);
  ENSURE_TYPES_1 (DATA_STRING);

  PING();
  // send string in the queue
  ::urbiserver->systemcommands = false;
  if (!connection->stack.empty())
    connection->functionTag = new UString("__Funct__");
  UParser& p = connection->parser();

  ECHO("Parsing " << variablename->id->c_str() << ':' << e1->str->c_str());

  if (in_load)
    p.process (::urbiserver->find_file (e1->str->c_str()));
  else
    p.process(reinterpret_cast<const ubyte*>(e1->str->c_str()),
	      e1->str->size());

#ifdef ENABLE_DEBUG_TRACES
  ECHO("Parsed " << variablename->id->str() << ':' << e1->str->str());
  if (p.commandTree)
    p.commandTree->print (3);
#endif

  if (connection->functionTag)
    {
      delete connection->functionTag;
      connection->functionTag = 0;
    }
  ::urbiserver->systemcommands = true;

  PING();

  // FIXME: Errors and warning are already reported in UConnection.
  //        Are these two blocks dead-code?
  if (p.hasError())
    {
      // a parsing error occured
      if (p.commandTree)
	{
	  // FIXME: 2007-07-20: Currently we can't free the commandTree,
	  // we might kill function bodies.
	  //delete p.commandTree;
	  p.commandTree = 0;
	}
      connection->send(p.error_get().c_str(), "error");
    }

  PING();

  if (p.hasWarning())
    // a warning was emitted
    connection->send(p.warning_get().c_str(), "warn ");

  if (p.commandTree)
    {
#if 0
      command->morph = p.commandTree;
      command->persistant = false;
#endif
      p.commandTree = 0;
    }
  else
    {
      send_error(connection, command, this,
		 (in_load
		  ? "Error loading file: %s"
		  : "Error parsing string: %s"),
		 e1->str->c_str());
      return 0;
    }
  PING();

  delete e1;
  return new UValue();
}

UValue*
UExpression::eval_FUNCTION_0 (UConnection *connection)
{
  passert (type, type == FUNCTION);
  passert (parameters, parameters == 0);

  if (*variablename->id == "freemem"
      || *variablename->id == "power"
      || *variablename->id == "cpuload"
      || *variablename->id == "time")
  {
    UValue* ret = new UValue();
    ret->dataType = DATA_NUM;

    if (*variablename->id == "freemem")
      ret->val = availableMemory - usedMemory;
    else if (*variablename->id == "time")
      ret->val = connection->server->getTime();
    else if (*variablename->id == "cpuload")
      ret->val = connection->server->cpuload;
    else if (*variablename->id == "power")
      ret->val = connection->server->getPower();
    return ret;
  }
  return 0;
}

UValue*
UExpression::eval_FUNCTION_1 (UCommand *command, UConnection *connection)
{
  passert (type, type == FUNCTION);
  passert (parameters, parameters->size() == 1);

  if (*variablename->id == "strlen")
  {
    UValue* e1 = parameters->expression->eval(command, connection);
    ENSURE_TYPES_1 (DATA_STRING);
    UValue* ret = new UValue();
    ret->dataType = DATA_NUM;
    ret->val = e1->str->size();

    for (size_t i=0; i < e1->str->size()-1; ++i)
      if (e1->str->c_str()[i] == '\\' &&
	  e1->str->c_str()[i+1] == '"')
	--ret->val;

    delete e1;
    return ret;
  }

  if (*variablename->id == "head")
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

  if (*variablename->id == "tail")
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

  if (*variablename->id == "size")
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


  if (*variablename->id == "isdef")
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

  if (*variablename->id == "isvoid")
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

  if (*variablename->id == "loadwav")
  {
    UValue* e1 = parameters->expression->eval(command, connection);
    ENSURE_TYPES_1 (DATA_STRING);
    UValue* ret = new UValue();
    ret->dataType = DATA_BINARY;
    UCommandQueue* loadQueue = new UCommandQueue (4096, 1048576, false);
    // load file
    if (connection->server->loadFile(e1->str->c_str(),
				     loadQueue) == UFAIL)
    {
      send_error(connection, command, this,
		 "Cannot load the file %s", e1->str->c_str());
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
  if (*variablename->id == "exec"
      || *variablename->id == "load")
    return eval_FUNCTION_EXEC_OR_LOAD (command, connection);

  if (false
      || *variablename->id == "sin"
      || *variablename->id == "asin"
      || *variablename->id == "cos"
      || *variablename->id == "acos"
      || *variablename->id == "tan"
      || *variablename->id == "atan"
      || *variablename->id == "sgn"
      || *variablename->id == "abs"
      || *variablename->id == "exp"
      || *variablename->id == "log"
      || *variablename->id == "round"
      || *variablename->id == "random"
      || *variablename->id == "trunc"
      || *variablename->id == "sqr"
      || *variablename->id == "sqrt"
      || *variablename->id == "string")
  {
    UValue* e1 = parameters->expression->eval(command, connection);
    ENSURE_TYPES_1 (DATA_NUM);

    if (*variablename->id == "string")
    {
      UValue* ret = new UValue();
      ret->dataType = DATA_STRING;
      std::ostringstream o;
      o << (int)e1->val;
      ret->str = new UString(o.str());
      delete e1;
      return ret;
    }

    UValue* ret = new UValue();
    ret->dataType = DATA_NUM;

    if (*variablename->id == "sin")
      ret->val = sin(e1->val);
    else if (*variablename->id == "asin")
      ret->val = asin(e1->val);
    else if (*variablename->id == "cos")
      ret->val = cos(e1->val);
    else if (*variablename->id == "acos")
      ret->val = acos(e1->val);
    else if (*variablename->id == "tan")
      ret->val = tan(e1->val);
    else if (*variablename->id == "atan")
      ret->val = atan(e1->val);
    else if (*variablename->id == "sgn")
    {
      // FIXME: No value set of 0.
      if (e1->val>0)
	ret->val =1;
      else if (e1->val<0)
	ret->val = -1;
    }
    else if (*variablename->id == "abs")
      ret->val = fabs(e1->val);
    else if (*variablename->id == "random")
    {
      int range =  (int)e1->val;
      if (range)
	ret->val = rand()%range;
      else
	ret->val = 0;
    }
    else if (*variablename->id == "round")
    {
      if (e1->val>=0)
	ret->val = (ufloat)(int)(e1->val+0.5);
      else
	ret->val = (ufloat)(int)(e1->val-0.5);
    }
    else if (*variablename->id == "trunc")
      ret->val = (ufloat)(int)(e1->val);
    else if (*variablename->id == "exp")
      ret->val = exp(e1->val);
    else if (*variablename->id == "sqr")
      ret->val = e1->val*e1->val;
    else if (*variablename->id == "sqrt")
    {
      if (e1->val<0)
      {
	send_error(connection, command, this, "Negative square root");
	return 0;
      }
      ret->val = sqrt(e1->val);
    }
    else if (*variablename->id == "log")
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

  return 0;
}

UValue*
UExpression::eval_FUNCTION_2 (UCommand *command,
			      UConnection *connection)
{
  passert (type, type == FUNCTION);
  passert (parameters, parameters->size() == 2);

  if (*variablename->id == "save")
  {
    UValue* e1 = parameters->expression->eval(command, connection);
    UValue* e2 = parameters->next->expression->eval(command, connection);

    ENSURE_TYPES_2 (DATA_STRING, DATA_STRING);
    UValue* ret = new UValue();
    ret->dataType = DATA_VOID;

    // save to file

    if (connection->server->saveFile(e1->str->c_str(),
				     e2->str->c_str()) == UFAIL)
    {
      send_error(connection, command, this, "Cannot save to the file %s",
		 e1->str->c_str());
      delete ret;
      ret = 0;
    }

    delete e1;
    delete e2;
    return ret;
  }

  if (*variablename->id == "getIndex")
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

  if (*variablename->id == "cat")
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
    {
      if (!e3)
      {
	ret->liststart = e4->copy();
	e3 = ret->liststart;
	e4 = e4->next;
      }
      for (; e4; e4 = e4->next)
      {
	e3->next = e4->copy();
	e3 = e3->next;
      }
    }

    delete e1;
    delete e2;
    return ret;
  }

  if (*variablename->id == "atan2")
  {
    UValue* e1 = parameters->expression->eval(command, connection);
    UValue* e2 = parameters->next->expression->eval(command, connection);
    ENSURE_TYPES_2 (DATA_NUM, DATA_NUM);
    UValue* ret = new UValue();
    ret->dataType = DATA_NUM;

    if (*variablename->id == "atan2")
      ret->val = atan2(e1->val, e2->val);

    delete e1;
    delete e2;
    return ret;
  }

  return 0;
}

UValue*
UExpression::eval_FUNCTION (UCommand *command,
			    UConnection *connection,
			    UEventCompound*& ec)
{
  PING();
  passert (type, type == FUNCTION);
  UString* funname = variablename->buildFullname(command, connection);

  // Event detection
  if (UEventHandler* eh =
      kernel::findEventHandler(funname, parameters ? parameters->size() : 0))
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

  if (parameters == 0)
    if (UValue* res = eval_FUNCTION_0 (connection))
      return res;
  PING();

  if (parameters && parameters->size() == 1)
    if (UValue* res = eval_FUNCTION_1 (command, connection))
      return res;
  PING();

  if (parameters && parameters->size() == 2)
    if (UValue* res = eval_FUNCTION_2 (command, connection))
      return res;
  PING();

  if (parameters &&
      parameters->size() == 3 &&
      *variablename->id == "strsub")
  {
    UValue* e1 = parameters->expression->eval(command, connection);
    UValue* e2 = parameters->next->expression->eval(command, connection);
    UValue* e3 = parameters->next->next->expression->eval(command, connection);

    ENSURE_TYPES_3 (DATA_STRING, DATA_NUM, DATA_NUM);
    UValue* ret = new UValue();
    ret->dataType = DATA_STRING;

    if (*variablename->id == "strsub")
      ret->str = new UString(e1->str->str().substr((int)e2->val, (int)e3->val));

    delete e1;
    delete e2;
    delete e3;
    return ret;
  }


  // default = unknown.
  funname = variablename->buildFullname(command, connection);
  if (!variablename->getFullname()) return 0;
  HMfunctiontab::iterator hmf =
    ::urbiserver->functiontab.find(funname->c_str());
  if (hmf != ::urbiserver->functiontab.end())
  {
    send_error(connection, command, this,
	       "Custom function call in expressions"
	       " not allowed in kernel 1");
    return 0;
  }

  send_error(connection, command, this,
	     "Error with function eval: %s [nb param=%d]",
	     variablename->getFullname()->c_str(),
	     parameters ? parameters->size() : 0);
  return 0;
}


UValue*
UExpression::eval_GROUP (UCommand *command, UConnection *connection)
{
  passert (type, type == GROUP);
  UValue* ret = new UValue();
  HMgrouptab::iterator retr = connection->server->grouptab.find(str->c_str());
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
	ret->liststart = new UValue((*it)->c_str());
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
	e1->next = new UValue((*it)->c_str());
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
  passert (type, type == LIST);
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
  passert (type, type == VARIABLE);
  UVariable *variable = variablename->getVariable(command, connection);
  if (!variablename->getFullname())
    return 0;
  UString* devicename = variablename->getDevice();

  const char* varname;
  if (!variable)
  {
    varname = variablename->getFullname()->c_str();

    // Event detection
    UEventHandler* eh =
      kernel::findEventHandler(variablename->getFullname(), 0);
    if (eh)
    {
      UValue* res = new UValue(ufloat(1));
      if (eh->noPositive())
	res->val = 0; // no active (positive) event in the handler

      ec = new UEventCompound
	(new UEventMatch
	 (variablename->getFullname(),
	  0,
	  command,
	  connection));
      return res;
    }

    // virtual variables
    const char* devname = variablename->getDevice()->c_str();
    bool ambiguous;
    UVariable *vari = 0;
    HMobjtab::iterator itobj;
    if ((itobj = ::urbiserver->objtab.find(devname)) !=
	::urbiserver->objtab.end())
    {
      vari = itobj->second->
	searchVariable(variablename->getMethod()->c_str(), ambiguous);
      if (ambiguous)
      {
	send_error(connection, command, this,
		   "Ambiguous multiple inheritance on variable %s",
		   variablename->getFullname()->c_str());
	return new UValue();
      }

      variable = vari;
      if (vari)
      {
	*devicename = vari->getMethod();
	*variablename->device = vari->getMethod();
	variablename->buildFullname(command, connection);
      }
    }
  }

  UValue* ret = 0;
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
	  p[0] = '_';
	  p = p + 2; // beginning of the index
	  char* p2 = strchr(p, '_');
	  while (p)
	  {
	    if (p2)
	      p2[0] = 0;
	    int index = atoi(p);
	    int curr = 0;
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
	       variablename->getFullname()->c_str());
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
		   variablename->getFullname()->c_str());
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
	  pabort ("not reachable");

	case UVariableName::UDERIV:
	  ret->val = 1000. * (variable->previous - variable->previous2)/
	    (::urbiserver->previousTime - ::urbiserver->previous2Time);
	  break;

	case UVariableName::UDERIV2:
	{
	  ufloat
	    t12 = ::urbiserver->previousTime - ::urbiserver->previous2Time,
	    t13 = ::urbiserver->previousTime - ::urbiserver->previous3Time,
	    t23 = ::urbiserver->previous2Time - ::urbiserver->previous3Time;
	  ret->val = 1000000. * 2 *
	    (variable->previous * t23
	     - variable->previous2 * t13
	     + variable->previous3 * t12
	      ) / (t23 * t13 * t12);

	}
	break;

	case UVariableName::UTRUEDERIV:
	  ret->val = 1000. *
	    (variable->get()->val - variable->valPrev)/
	    (::urbiserver->currentTime
	     - ::urbiserver->previousTime);
	  break;

	case UVariableName::UTRUEDERIV2:
	{
	  ufloat
	    t01 = ::urbiserver->currentTime - ::urbiserver->previousTime,
	    t02 = ::urbiserver->currentTime - ::urbiserver->previous2Time,
	    t12 = ::urbiserver->previousTime - ::urbiserver->previous2Time;
	  ret->val = 1000000. * 2 *
	    (variable->get()->val * t12
	     - variable->valPrev * t02
	     + variable->valPrev2 * t01
	      ) / (t12 * t02 * t01);
	}
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
  switch (type)
  {
    case LIST:
    {
      for (UNamedParameters *p = parameters; p; p = p->next)
	if (p->expression->asyncScan(cmd, c) == UFAIL)
	  return UFAIL;
      return USUCCESS;
    }

    case VARIABLE:
    {
      UVariable* variable = variablename->getVariable(cmd, c);
      UString* fullname = variablename->getFullname();
      if (!fullname)
	return UFAIL;
      const char* varname  = variablename->getFullname()->c_str();

      if (!variable)
      {
	// Is this a virtual variable?
	const char* devname = variablename->getDevice()->c_str();
	bool ambiguous;

	HMobjtab::iterator itobj;
	if ((itobj = ::urbiserver->objtab.find(devname)) !=
	    ::urbiserver->objtab.end())
	{
	  variable = itobj->second->
	    searchVariable(variablename->getMethod()->c_str(),
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
	  HMvariabletab::iterator hmv = ::urbiserver->variabletab.find(varname);
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
	if (UEventHandler* eh = kernel::findEventHandler(fullname, 0))
	{
	  eh->registerCmd(cmd);
	  return USUCCESS;
	}
	else if (c->server->defcheck) //strict
	  return UFAIL;
	else
	{
	  variable = new UVariable (fullname->c_str(),
				    new UValue (ufloat (0)));
	  variable->registerCmd(cmd);
	  return USUCCESS;
	}
      }
    }

    case PROPERTY:
    {
      UVariable* variable = variablename->getVariable(cmd, c);
      if (!variable)
	return UFAIL;
      UString* fullname = variablename->getFullname();
      if (!fullname)
	return UFAIL;
      variable->registerCmd(cmd);
      return USUCCESS;
    }

    case FUNCTION:
    {
      UString* fullname = variablename->buildFullname (cmd, c);
      int nbargs = 0;
      if (parameters)
	nbargs = parameters->size ();
      if (UEventHandler* eh = kernel::findEventHandler(fullname, nbargs))
      {
	// This is an event
	eh->registerCmd (cmd);
	return USUCCESS;
      }
      else
      {
	// is it a known kernel function?
	if (kernel::isCoreFunction (variablename->id))
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
	    UEventHandler* eh = new UEventHandler (fullname, nbargs);
	    eh->registerCmd (cmd);
	    return USUCCESS;
	  }
	}
      }
    }

    default:
    {
      if (expression1)
	if (expression1->asyncScan(cmd, c) == UFAIL)
	  return UFAIL;

      if (expression2)
	if (expression2->asyncScan(cmd, c) == UFAIL)
	  return UFAIL;
      return USUCCESS;
    }
  }
}
