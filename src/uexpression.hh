/*! \file uexpression.hh
 *******************************************************************************

 File: uexpression.h\n
 Definition of the UExpression class.

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

#ifndef UEXPRESSION_HH
#define UEXPRESSION_HH

#include <list>

#include "fwd.hh"
#include "utypes.hh"
#include "ustring.hh"
#include "ucommandqueue.hh"
#include "memorymanager/memorymanager.hh"


// ****************************************************************************
//! Contain an expression tree as returned by the parser
/*! UExpression class:

    There is no subclass to UExpression to make it easier to have
    expressions with transforming type, like in the numerical reduction
    that happends in the parser: 4+5 which is a EXPR_PLUS expression will
    be transformed into a EXPR_VALUE with value 9. Sub classes would make
    this operation more difficult to do. Furthermore, the memory loss due
    to non subclassing is very low in the case of UExpression.
*/
class UExpression
{
public:
  MEMORY_MANAGED;

  //! The different types for a UExpression.
  enum Type
  {
    EXPR_VALUE,
    EXPR_VARIABLE,
    EXPR_LIST,
    EXPR_GROUP,
    EXPR_ADDR_VARIABLE,
    EXPR_FUNCTION,
    EXPR_PLUS,
    EXPR_MINUS,
    EXPR_MULT,
    EXPR_DIV,
    EXPR_MOD,
    EXPR_EXP,
    EXPR_NEG,
    EXPR_COPY,
    EXPR_PROPERTY,
    EXPR_EVENT,

    EXPR_TEST_EQ,
    EXPR_TEST_REQ,
    EXPR_TEST_PEQ,
    EXPR_TEST_DEQ,
    EXPR_TEST_NE,
    EXPR_TEST_GT,
    EXPR_TEST_GE,
    EXPR_TEST_LT,
    EXPR_TEST_LE,
    EXPR_TEST_BANG,
    EXPR_TEST_AND,
    EXPR_TEST_OR
  };

  UExpression(Type type, ufloat *val);
  UExpression(Type type, ufloat val);

  UExpression(Type type, UString *str);
  UExpression(Type type, UValue *v);
  UExpression(Type type, UValue v);

  UExpression(Type type,
	      UExpression* expression1,
	      UExpression* expression2);
  UExpression(Type type, UVariableName* variablename);
  UExpression(Type type,
	      UVariableName* variablename,
	      UExpression *expression1);
  UExpression(Type type,
	      UVariableName* variablename,
	      UNamedParameters *parameters);
  UExpression(Type type,
	      UNamedParameters *parameters);
  UExpression(Type type,
	      UString *oper,
	      UString *id);
  UExpression(Type type,
	      UString *oper,
	      UVariableName *variablename);
  UExpression (Type type,
	       UExpression* expression1,
	       UExpression* expression2,
	       UVariableName* variablename,
	       UNamedParameters* parameters,
	       UString* str,
	       UString* id,
	       UExpression* softtest_time,
	       UValue* staticcache,
	       UDataType dataType,
	       ufloat val,
	       bool issconst,
	       bool issofttest,
	       bool firsteval,
	       UValue* tmp_value);
  ~UExpression();

  void            print       ();
  void            initialize  ();

  // Backward compatible version of eval.
  UValue*         eval        (UCommand *command,
			       UConnection *connection);

  // New version of eval, capable of returning a UEventCompound
  UValue*         eval        (UCommand *command,
			       UConnection *connection,
			       UEventCompound*& ec);

  UErrorValue     asyncScan   (UASyncCommand* cmd,
			       UConnection* c);

  UExpression*    copy        ();

  /// Type of the expression.
  Type type;
  /// Type of the expression's data.
  UDataType       dataType;

  /// numerical value used for the EXPR_NUM.
  ufloat          val;
  /// string of the EXPR_STRING or EXPR_FUNCTOR type.
  UString         *str;

  ///  stores a tmp UValue resulting from a function evaluation (which
  /// temporarily is processed as an UExpression), Usually, the value
  /// of this is 0.
  UValue          *tmp_value;

  /// id of the EXPR_FUNCTOR
  UString         *id;
  /// true on first evaluation (used by static)
  bool            firsteval;
  /// true when the expr is const
  bool            isconst;
  /// true when the expr is a soft test
  bool            issofttest;
  /// used for static variables
  UValue          *staticcache;

  /// Left side of a compound expression.
  UExpression     *expression1;
  /// Right side of a compound expression.
  UExpression     *expression2;
  /// variable when the expression is a EXPR_VARIABLE or EXPR_FUNCTION
  UVariableName   *variablename;

  /// list of parameters of the EXPR_FUNCTION or EXPR_LIST
  UNamedParameters *parameters;

  /// Time constant for a soft test (0 means "hard test").
  UExpression      *softtest_time;

private:
  /// eval() specialized for type == EXPR_VARIABLE.
  UValue* eval_EXPR_VARIABLE (UCommand *command,
			      UConnection *connection,
			      UEventCompound*& ec);

  /// eval() specialized for type == EXPR_FUNCTION.
  UValue*
  eval_EXPR_FUNCTION (UCommand *command,
		      UConnection *connection,
		      UEventCompound*& ec);
};

#endif
