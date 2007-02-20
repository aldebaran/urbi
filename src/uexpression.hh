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
# define UEXPRESSION_HH

# include <list>

# include "fwd.hh"
# include "uast.hh"
# include "utypes.hh"
# include "memorymanager/memorymanager.hh"

// ****************************************************************************
//! An expression tree as returned by the parser
/*! UExpression class:

    There is no subclass to UExpression to make it easier to have
    expressions with transforming type, like in the numerical reduction
    that happends in the parser: 4+5 which is a PLUS expression will
    be transformed into a VALUE with value 9. Sub classes would make
    this operation more difficult to do. Furthermore, the memory loss due
    to non subclassing is very low in the case of UExpression.
*/
class UExpression : public UAst
{
public:
  MEMORY_MANAGED;

  //! The different types for a UExpression.
  enum Type
    {
      VALUE,
      VARIABLE,
      LIST,
      GROUP,
      ADDR_VARIABLE,
      FUNCTION,
      PLUS,
      MINUS,
      MULT,
      DIV,
      MOD,
      EXP,
      NEG,
      COPY,
      PROPERTY,
      EVENT,

      TEST_EQ,
      TEST_REQ,
      TEST_PEQ,
      TEST_DEQ,
      TEST_NE,
      TEST_GT,
      TEST_GE,
      TEST_LT,
      TEST_LE,
      TEST_BANG,
      TEST_AND,
      TEST_OR
    };

  UExpression(const location& l, Type type, ufloat val);

  /// The ownership is taken.
  UExpression(const location& l, Type type, UString* str);

  UExpression(const location& l, Type type, UValue* v);

  UExpression(const location& l, Type type, UExpression* e1, UExpression* e2);
  UExpression(const location& l, Type type, UVariableName* v);
  UExpression(const location& l, Type type, UVariableName* v, UExpression* e);
  UExpression(const location& l, Type type, UVariableName* v, UNamedParameters* p);
  UExpression(const location& l, Type type, UNamedParameters* p);
  UExpression(const location& l, Type type, UString* op, UString* id);
  UExpression(const location& l, Type type, UString* op, UVariableName* v);

  /// The complete ctor, used for copies.
  UExpression (const location& l,
	       Type type,
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

  /// \a t is the indentation level.
  void print (unsigned t);


  // Backward compatible version of eval.
  UValue* eval(UCommand* command, UConnection* connection);

  // New version of eval, capable of returning a UEventCompound
  UValue* eval(UCommand* command, UConnection* connection,
	       UEventCompound*& ec);

  UErrorValue asyncScan (UASyncCommand* cmd, UConnection* c);

  UExpression* copy() const;

  /// Type of the expression.
  Type type;
  /// Type of the expression's data.
  UDataType dataType;

  /// numerical value used for the NUM.
  ufloat val;
  /// string of the STRING or FUNCTOR type.
  UString* str;

  ///  stores a tmp UValue resulting from a function evaluation (which
  /// temporarily is processed as an UExpression), Usually, the value
  /// of this is 0.
  UValue* tmp_value;

  /// id of the FUNCTOR
  UString* id;
  /// true on first evaluation (used by static)
  bool firsteval;
  /// true when the expr is const
  bool isconst;
  /// true when the expr is a soft test
  bool issofttest;
  /// used for static variables
  UValue* staticcache;

  /// Left side of a compound expression.
  UExpression* expression1;
  /// Right side of a compound expression.
  UExpression* expression2;
  /// variable when the expression is a VARIABLE or FUNCTION
  UVariableName* variablename;

  /// list of parameters of the FUNCTION or LIST
  UNamedParameters* parameters;

  /// Time constant for a soft test (0 means "hard test").
  UExpression* softtest_time;

private:
  /// Used to factor ctors.
  void initialize ();

  /// eval() specialized for type == GROUP.
  UValue*
  eval_GROUP (UCommand* command, UConnection* connection);

  /// Functions with no arguments.
  UValue*
  eval_FUNCTION_0 (UConnection *connection);

  /// Unary functions.
  UValue*
  eval_FUNCTION_1 (UCommand *command, UConnection *connection);

  /// Binary functions.
  UValue*
  eval_FUNCTION_2 (UCommand *command, UConnection *connection);

  /// eval() specialized for type == FUNCTION.
  UValue*
  eval_FUNCTION (UCommand* command, UConnection* connection,
		 UEventCompound*& ec);

  /// eval_FUNCTION() specialized for variablename = "exec" or "load".
  UValue*
  eval_FUNCTION_EXEC_OR_LOAD (UCommand* command, UConnection* connection);

  /// eval() specialized for type == LIST.
  UValue* eval_LIST (UCommand* command, UConnection* connection);

  /// eval() specialized for type == VARIABLE.
  UValue* eval_VARIABLE (UCommand* command, UConnection* connection,
			 UEventCompound*& ec);
};

#endif
