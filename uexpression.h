/*! \file uexpression.h
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

#ifndef UEXPRESSION_H_DEFINED
#define UEXPRESSION_H_DEFINED

#include "utypes.h"
#include "ustring.h"
#include "ucommandqueue.h"
#include "memorymanager/memorymanager.h"
#include <list>
using namespace std;

class UCommand;
class UExpression;
class UConnection;
class UNamedParameters;
class UVariableName;
class UVariableList;
class UVariable;
class UValue;
class UDevice;
class UServer;
class UCommand_TREE;
class UContext;
		
// *****************************************************************************
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
  UExpression(UExpressionType type, UFloat *val);
  UExpression(UExpressionType type, UFloat val);

  UExpression(UExpressionType type, UString *str);
  UExpression(UExpressionType type, UValue *v);
  UExpression(UExpressionType type, UValue v);

  UExpression(UExpressionType type, 
              UExpression* expression1, 
              UExpression* expression2);
  UExpression(UExpressionType type, UVariableName* variablename);
  UExpression(UExpressionType type,
              UVariableName* variablename, 
              UExpression *expression1);
  UExpression(UExpressionType type,
              UVariableName* variablename, 
              UNamedParameters *parameters);
  UExpression(UExpressionType type,
              UNamedParameters *parameters);
  UExpression(UExpressionType type, 
              UString *oper,
              UString *id);
  UExpression(UExpressionType type, 
              UString *oper,
              UVariableName *variablename);
  ~UExpression();

  void            print       (); 
  void            initialize  ();
  UValue*         eval        (UCommand *command, UConnection *connection, bool silent = false);
  UExpression*    copy        (); 
  
  UExpressionType type;         ///< Type of the expression.
  UDataType       dataType;     ///< Type of the expression's data.

  UFloat          val;          ///< numerical value used for the EXPR_NUM                               
  UString         *str;         ///< string of the EXPR_STRING or EXPR_FUNCTOR 
                                ///< type.
  UString         *id;          ///< id of the EXPR_FUNCTOR
  bool            firsteval;    ///< true on first evaluation (used by static)
  bool            isconst;      ///< true when the expr is const
  bool            issofttest;   ///< true when the expr is a soft test
  UValue          *staticcache; ///< used for static variables

  UExpression     *expression1; ///< Left side of a compound expression.
  UExpression     *expression2; ///< Right side of a compound expression.
  UVariableName   *variablename;///< variable when the expression is a 
                                ///< EXPR_VARIABLE or  EXPR_FUNCTION 
  UNamedParameters *parameters; ///< list of parameters of the EXPR_FUNCTION or EXPR_LIST

  //  UFloat          softtest_time;///< Time constant for a soft test (0 means "hard test")
  UExpression      *softtest_time; ///< Time constant for a soft test (0 means "hard test")  
  //  int             softtest_rep; ///< Nb of repetition for a soft test
};

#endif
