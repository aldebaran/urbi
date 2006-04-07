/* \file ugrammar.y
 *******************************************************************************

 File: ugrammar.y\n
 Definition of the parser used by the UParser object.
 This parser is defined with bison, using the option %pure_parser to make it 
 reentrant. For more details about reentrancy issues, check the definition of 
 the UServer class.

 This file is part of 
 %URBI, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

%require "2.1b"
%error-verbose
%locations
%defines 
%skeleton "lalr1.cc"
%parse-param {UParser& uparser}
%lex-param {UParser& uparser}
%{

#include "utypes.h"
#include "ucommand.h"    

class UParser;

%}

/* Possible data type returned by the bison parsing mechanism */

%union {
  UCommand                *command;   
  UExpression             *expr;
  UBinary                 *binary;
  UNamedParameters        *namedparameters;  
  UVariableName           *variable;    
  UVariableList           *variablelist;   
  UProperty               *property;  

  UFloat                   *val;
  UString                  *str;
  struct {
    UString *device;
    UString *id; 
    bool rooted;
  }                        structure;
}

%{
// Is included in ugrammar.cc
//#include <cmath> 
#include <hash_map.h>

#include <string>

#define TRUE UFloat(1)
#define FALSE UFloat(0)

#include "../uparser.h"

#include "uconnection.h"
#include "udevice.h"
#include "uobj.h"
#include "ugroup.h"

extern UString** globalDelete;

/* Memory checking macros, used in the command tree building process */

#define MEMCHECK(p)       {if (p==0) { \
   uparser.connection->server->isolate(); \
   uparser.connection->server->memoryOverflow = true;}}

#define MEMCHECK1(p,p1)       {if (p==0) { \
   uparser.connection->server->isolate(); \
   uparser.connection->server->memoryOverflow = true;\
   if (p1!=0) { delete(p1);p1=0; };}}

#define MEMCHECK2(p,p1,p2)    {if (p==0) { \
   uparser.connection->server->isolate(); \
   uparser.connection->server->memoryOverflow = true;\
   if (p1!=0) { delete(p1);p1=0; }; \
   if (p2!=0) { delete(p2);p2=0; }; }}

#define MEMCHECK3(p,p1,p2,p3) {if (p==0) { \
   uparser.connection->server->isolate(); \
   uparser.connection->server->memoryOverflow = true;\
   if (p1!=0) { delete(p1);p1=0; }; \
   if (p2!=0) { delete(p2);p2=0; }; \
   if (p3!=0) { delete(p3);p3=0; }; }}

#define MEMCHECK4(p,p1,p2,p3,p4) {if (p==0) { \
   uparser.connection->server->isolate(); \
   uparser.connection->server->memoryOverflow = true;\
   if (p1!=0) { delete(p1);p1=0; }; \
   if (p2!=0) { delete(p2);p2=0; }; \
   if (p3!=0) { delete(p3);p3=0; }; \
   if (p4!=0) { delete(p4);p4=0; }; }}

//! Directs the call from 'bison' to the scanner in the right parser
inline yy::parser::token::yytokentype yylex(yy::parser::semantic_type* val,
                                            yy::location* loc, UParser& p)
{
  return p.scan(val, loc);
}



%}

/* List of all tokens and terminal symbols, with their type */

%start ROOT

%token SEMICOLON ";"
%token COLON  ":"
%token COMMA ","
%token AND  "&"
%token PIPE  "|"
%token BANG  "!"
%token MINUSMINUS "--"
%token PLUSPLUS  "++"
%token PLUSASSIGN  "+="
%token MINUSASSIGN  "-="
%token MULT "*"
%token DIV "/"
%token DEQ "=~="
%token PLUS "+"
%token EXP "^"
%token MINUS "-"
%token ASSIGN "="
%token EQ "=="
%token REQ "~="
%token PEQ "%="
%token NE "!="
%token GT ">"
%token GE ">="
%token LT "<"
%token LE "<="
%token TILDE "~"
%token RPAREN ")"
%token LPAREN "("
%token DIR "->" 
%token RSBRACKET  "]"
%token LSBRACKET "["
%token RBRACKET "}"
%token LBRACKET "{"
%token IF "if"
%token ELSE "else"
%token WHILE "while"
%token FOR "for"
%token NORM "normalized"
%token VARERROR "varerror"
%token LOOP "loop"
%token LOOPN  "loopn"
%token FOREACH "foreach"
%token IN "in"
%token STOP "stop"
%token BLOCK "block"
%token UNBLOCK "unblock"
%token NOOP "noop"
%token TRUECONST "true"
%token FALSECONST "false"
%token EMIT "emit"
%token CLASS "class"
%token VAR "var"
%token FUNCTION "function"
%token EVENT "event"
%token SUBCLASS "subclass"
%token NEW "new"
%token OBJECT "object"
%token GROUP "group"
%token INFO "info"
%token UNIT "unit"
%token WAIT "wait"
%token WAITUNTIL "waituntil"
%token UECHO "echo"    // Flex defines the ECHO macro
%token DOLLAR "$"
%token PERCENT "%"
%token AROBASE "@"
%token DEF "def"
%token RETURN "return"
%token BIN  "bin"
%token WHENEVER "whenever"
%token COPY "copy"
%token ALIAS "alias"
%token DERIV "derivation"
%token DERIV2 "second-derivation"
%token TRUEDERIV "command-derivation"
%token TRUEDERIV2 "second-command-derivation"
%token EVERY "every"
%token TIMEOUT "timeout"
%token STOPIF "stopif"
%token FREEZEIF "freezeif"
%token AT "at"
%token ONLEAVE "onleave"
%token ANDOPERATOR "&&"
%token OROPERATOR "||"
%token CMDBLOCK "command block"
%token EXPRBLOCK "expression block"
%token ONLY "only"
%token GROUPLIST "group list"

%token UEOF 0 "end of command"

%token <val>                 NUM        "number"
%token <val>                 TIMEVALUE  "time"
%token <val>                 FLAG       "flag"
%token <val>                 FLAGTEST   "flag test"  
%token <val>                 FLAGID     "flag identifier"
%token <val>                 FLAGTIME   "flag time"      
%token <str>                 IDENTIFIER  "identifier"  
%token <structure>           STRUCT      "structured identifier"
%token <structure>           REFSTRUCT   "structured ref-identifier"
%token <str>                 STRING      "string"
%token <str>                 SWITCH      "switch"
%token <str>                 BINDER      "binder"
%token <str>                 OPERATOR    "operator command" 
%token <str>                 OPERATOR_ID "operator"
%token <str>                 OPERATOR_ID_PARAM "param-operator"
%token <str>                 OPERATOR_VAR "var-operator"

%type  <expr>                expr            "expression"
%type  <val>                 timeexpr        "time expression"
%type  <command>             taggedcommands  "set of commands"
%type  <command>             taggedcommand   "tagged command"
%type  <command>             command         "command"
%type  <command>             instruction     "instruction" 
%type  <namedparameters>     parameters      "parameters"
%type  <namedparameters>     array           "array"
%type  <namedparameters>     parameterlist   "list of parameters"
%type  <namedparameters>     rawparameters   "list of attributes"
%type  <namedparameters>     namedparameters "list of named parameters"
%type  <namedparameters>     flags           "list of flags"
%type  <variablelist>        refvariables    "list of variables"
%type  <expr>                softtest        "soft test"
%type  <namedparameters>     identifiers     "list of identifiers"
%type  <expr>                class_declaration "class declaration"
%type  <namedparameters>     class_declaration_list "class declaration list"
%type  <binary>              binary          "binary"
%type  <property>            property        "property"
%type  <variable>            variable        "variable"
%type  <variable>            purevariable    "pure variable"
//%type  <namedparameters>     purevariables   "list of pure variables"
%type  <variable>            refvariable     "ref-variable"

/* Operators priority */

%left  OROPERATOR ANDOPERATOR BANG
%left  EQ REQ PEQ DEQ NE GE GT LT LE
%left  MINUS PLUS
%left  MULT DIV PERCENT
%left  NEG     /* Negation--unary minus */
%right EXP     /* Exponentiation*/       
%right NORM

%left  COMMA 
%left  SEMICOLON
%left  AND PIPE 
%left  CMDBLOCK EXPRBLOCK
%left  ELSE ONLEAVE
%nonassoc ASSIGN


/* URBI Grammar */

%initial-action { @$ = uparser.connection->lastloc; }

%%

ROOT: root {
	uparser.connection->lastloc = @$;
      }

root:   

    refvariable ASSIGN binary SEMICOLON { 

      URefPt<UBinary> *ref = new URefPt<UBinary>($3);
      MEMCHECK(ref);
      UCommand* tmpcmd = new UCommand_ASSIGN_BINARY($1,ref);
      if (tmpcmd) tmpcmd->tag->update("__node__");
      MEMCHECK2(tmpcmd,$1,ref);
      if (tmpcmd) uparser.binaryCommand = true;

      uparser.commandTree  = new UCommand_TREE(USEMICOLON,tmpcmd,0);
      if ( uparser.commandTree )
        uparser.commandTree->tag->update("__node__");
      MEMCHECK(uparser.commandTree);      
    } 

  | taggedcommands { 

      uparser.commandTree = 0;
      if ($1) {
        if ($1->type == CMD_TREE)
          uparser.commandTree = (UCommand_TREE*)$1;     
        else
          delete $1;
      }      
    }
;


/* TAGGEDCOMMANDS */

taggedcommands:

    taggedcommand

  | taggedcommands COMMA taggedcommands {
      
      $$ = new UCommand_TREE(UCOMMA,$1,$3);
      if ($$)       
        $$->tag->update("__node__");
      MEMCHECK2($$,$1,$3);
    }

  | taggedcommands SEMICOLON taggedcommands { 
      
      $$ = new UCommand_TREE(USEMICOLON,$1,$3);
      if ($$)         
        $$->tag->update("__node__");
      MEMCHECK2($$,$1,$3);
    }

  | taggedcommands PIPE taggedcommands {
      
      $$ = new UCommand_TREE(UPIPE,$1,$3);
      if ($$)        
        $$->tag->update("__node__");
      MEMCHECK2($$,$1,$3);
    }

  | taggedcommands AND taggedcommands {
      
      $$ = new UCommand_TREE(UAND,$1,$3);
      if ($$)        
        $$->tag->update("__node__");
      MEMCHECK2($$,$1,$3);
    }

;

/* TAGGEDCOMMAND */

taggedcommand:  

    command {
    
      if (($1) && (!$1->tag))        
        $1->tag = new UString(UNKNOWN_TAG);
      
      $$ = $1;     
    }

  | IDENTIFIER flags COLON command {
      
      MEMCHECK($1);
      if ($4) {
        
        if ($4->tag) delete $4->tag;
        $4->tag = $1;
        $4->flags = $2;
      }
      $$ = $4;
    }

  | IDENTIFIER COLON command {
      
      MEMCHECK($1);
      if ($3) {
        
        if ($3->tag) delete $3->tag;
        $3->tag = $1;        
      }
      $$ = $3;
    }

  | STRUCT COLON command {
            
      MEMCHECK($1.device);
      MEMCHECK($1.id);
      if ($3) {
        
        if ($3->tag) delete $3->tag;
        $3->tag = new UString($1.device,$1.id);
	delete $1.device;
	delete $1.id;
      }
      $$ = $3;
    }

  | STRUCT flags COLON command {
              
      MEMCHECK($1.device);
      MEMCHECK($1.id);

      if ($4) {
        
        if ($4->tag) delete $4->tag;	
       	$4->tag = new UString($1.device,$1.id);
	delete $1.device;
	delete $1.id;
      
        $4->flags = $2;
      }
      $$ = $4;
    }



//uparser.connection-

  | flags COLON command {
      
      MEMCHECK($1);
      if ($3) {
                
        $3->tag->update(UNKNOWN_TAG);        
        $3->flags = $1;
      }
      $$ = $3;
    }
;

/* FLAGS */

flags : 
     FLAG  {
            
      UExpression *flagval = new UExpression(EXPR_VALUE,$1); 
      MEMCHECK(flagval);

      $$ = new UNamedParameters(new UString("flag"),flagval,0); 
      MEMCHECK1($$,flagval);
    }
 
  |  FLAG flags  {
            
      UExpression *flagval = new UExpression(EXPR_VALUE,$1); 
      MEMCHECK(flagval);

      $$ = new UNamedParameters(new UString("flag"),flagval,$2); 
      MEMCHECK2($$,flagval,$2);
    }

  | FLAGTIME LPAREN expr RPAREN flags {
                     
      $$ = new UNamedParameters(new UString("flagtimeout"),$3,$5); 
      MEMCHECK2($$,$3,$5);
    }

  | FLAGTIME LPAREN expr RPAREN {
                     
      $$ = new UNamedParameters(new UString("flagtimeout"),$3,0); 
      MEMCHECK1($$,$3);
    }

  | FLAGID LPAREN expr RPAREN {
                
      $$ = new UNamedParameters(new UString("flagid"),$3,0); 
      MEMCHECK1($$,$3);
    }

  | FLAGID LPAREN expr RPAREN flags {
                
      $$ = new UNamedParameters(new UString("flagid"),$3,$5); 
      MEMCHECK2($$,$3,$5);
    }

  | FLAGTEST LPAREN softtest RPAREN flags {
                 
      if (*$1 == 6)
        $$ = new UNamedParameters(new UString("flagstop"),$3,$5); 
      else
        $$ = new UNamedParameters(new UString("flagfreeze"),$3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | FLAGTEST LPAREN softtest RPAREN {

      if (*$1 == 6)
        $$ = new UNamedParameters(new UString("flagstop"),$3,0); 
      else
        $$ = new UNamedParameters(new UString("flagfreeze"),$3,0); 
      MEMCHECK1($$,$3);
    }
;

/* COMMAND */

command:  

    instruction 

  | LBRACKET taggedcommands RBRACKET { 
    
      $$ = (UCommand*)
        new UCommand_TREE(UPIPE,
                          $2,
                          new UCommand_NOOP(true));
      $$->tag->update("__UGrouped_set_of_commands__");
      ((UCommand_TREE*)$$)->command2->tag->update("__system__");
    }
;

/* INSTRUCTION */

instruction: 
  /* empty */ { $$ = 0; }

  | NOOP { 
      $$ = new UCommand_NOOP();
      MEMCHECK($$);              
    }

  | refvariable ASSIGN expr namedparameters { 

    $$ = new UCommand_ASSIGN_VALUE($1,$3,$4, false);
      MEMCHECK3($$,$1,$3,$4);
    } 

  | refvariable PLUSASSIGN expr { 

      $$ = new UCommand_AUTOASSIGN($1,$3,0);
      MEMCHECK2($$,$1,$3);
    } 

  | refvariable MINUSASSIGN expr { 

      $$ = new UCommand_AUTOASSIGN($1,$3,1);
      MEMCHECK2($$,$1,$3);
    } 


  | VAR refvariable ASSIGN expr namedparameters { 

    $$ = new UCommand_ASSIGN_VALUE($2,$4,$5);
      MEMCHECK3($$,$2,$4,$5);
    } 

  | property ASSIGN expr { 

      $$ = new UCommand_ASSIGN_PROPERTY($1->variablename,$1->property,$3);
      MEMCHECK3($$,$1,$1,$3);
    } 

  | expr {
   
      $$ = new UCommand_EXPR($1); 
      MEMCHECK1($$,$1); 
    }         

  | IDENTIFIER SWITCH {

      $$ = new UCommand_DEVICE_CMD($1,$2);
      MEMCHECK2($$,$1,$2);
    }

  | RETURN {

      $$ = new UCommand_RETURN((UExpression*)0);
      MEMCHECK($$);
    }       

  | RETURN expr { 

      $$ = new UCommand_RETURN($2);
      MEMCHECK1($$,$2);
    }      

  | "echo" expr namedparameters {

      $$ = new UCommand_ECHO($2,$3,(UString*)0);
      MEMCHECK2($$,$2,$3);
    } 

  | refvariable ASSIGN NEW IDENTIFIER { 
        
      MEMCHECK($4);
      $$ = new UCommand_NEW($1->id,$4,(UNamedParameters*)0,true);
      MEMCHECK2($$,$1,$4);
    } 
    
  | refvariable ASSIGN NEW IDENTIFIER LPAREN parameterlist RPAREN { 
        
      MEMCHECK($4);
      $$ = new UCommand_NEW($1->id,$4,$6);
      MEMCHECK3($$,$1,$4,$6);
    } 


  | GROUP IDENTIFIER LBRACKET identifiers RBRACKET {

      $$ = new UCommand_GROUP($2,$4);
      MEMCHECK2($$,$4,$2);      
    } 
    
  | GROUP IDENTIFIER {

      $$ = new UCommand_GROUP($2,(UNamedParameters*)0);
      MEMCHECK1($$,$2);      
    } 

  | GROUP {
      
      $$ = new UCommand_GROUP((UString*)0,(UNamedParameters*)0);
      MEMCHECK($$);
    }


  | ALIAS purevariable purevariable {
      
      $$ = new UCommand_ALIAS($2,$3);
      MEMCHECK2($$,$2,$3);
    }

  | ALIAS purevariable {

      $$ = new UCommand_ALIAS($2,(UVariableName*)0);
      MEMCHECK1($$,$2);
    } 

  | ALIAS {
      
      $$ = new UCommand_ALIAS((UVariableName*)0,(UVariableName*)0);
      MEMCHECK($$);
    }

  | OPERATOR {
     
      MEMCHECK($1);
      $$ = new UCommand_OPERATOR($1);
      MEMCHECK1($$,$1);      
    } 

  | OPERATOR_ID IDENTIFIER {

      MEMCHECK($1);
      MEMCHECK($2);
      $$ = new UCommand_OPERATOR_ID($1,$2);
      MEMCHECK2($$,$1,$2);
    } 

  | OPERATOR_VAR variable {

      MEMCHECK($1);      
      $$ = new UCommand_OPERATOR_VAR($1,$2);
      MEMCHECK2($$,$1,$2);
    }  
    
  | BINDER OBJECT purevariable {

      MEMCHECK($1);
      $$ = new UCommand_BINDER($1,3,$3);
      MEMCHECK2($$,$1,$3);
    }


  | BINDER VAR purevariable {

      MEMCHECK($1);
      $$ = new UCommand_BINDER($1,1,$3);
      MEMCHECK2($$,$1,$3);
    }
    
  | BINDER FUNCTION LPAREN NUM RPAREN purevariable {

      MEMCHECK($1);
      $$ = new UCommand_BINDER($1,0,$6,(int)(*$4));
      MEMCHECK2($$,$1,$6);
    }

  | BINDER EVENT LPAREN NUM RPAREN purevariable {

      MEMCHECK($1);
      $$ = new UCommand_BINDER($1,2,$6,(int)(*$4));
      MEMCHECK2($$,$1,$6);
    }

  | WAIT expr { 

      $$ = new UCommand_WAIT($2);
      MEMCHECK1($$,$2);
    } 

  | EMIT purevariable { 

      $$ = new UCommand_EMIT($2,(UNamedParameters*)0);
      MEMCHECK1($$,$2);
    } 

  | EMIT purevariable LPAREN parameterlist RPAREN { 

      $$ = new UCommand_EMIT($2,$4);
      MEMCHECK2($$,$2,$4);
    } 

  | EMIT LPAREN expr RPAREN purevariable { 

      $$ = new UCommand_EMIT($5,(UNamedParameters*)0,$3);
      MEMCHECK2($$,$5,$3);
    } 

  | EMIT LPAREN expr RPAREN purevariable LPAREN parameterlist RPAREN { 

      $$ = new UCommand_EMIT($5,$7,$3);
      MEMCHECK3($$,$5,$7,$3);
    } 

  | EMIT LPAREN RPAREN purevariable { 

      $$ = new UCommand_EMIT($4,(UNamedParameters*)0, 
      	new UExpression(EXPR_VALUE,UINFINITY));
      MEMCHECK1($$,$4);
    } 

  | EMIT LPAREN RPAREN purevariable LPAREN parameterlist RPAREN { 

      $$ = new UCommand_EMIT($4,$6,
      	new UExpression(EXPR_VALUE,UINFINITY));
      MEMCHECK2($$,$4,$6);
    } 

  | WAITUNTIL softtest { 

      $$ = new UCommand_WAIT_TEST($2);
      MEMCHECK1($$,$2);
    } 

  | refvariable MINUSMINUS {

      $$ = new UCommand_INCDECREMENT(CMD_DECREMENT,$1);
      MEMCHECK1($$,$1);
    } 

  | refvariable PLUSPLUS { 

      $$ = new UCommand_INCDECREMENT(CMD_INCREMENT,$1);
      MEMCHECK1($$,$1);
    } 

  | DEF {

      $$ = new UCommand_DEF(UDEF_QUERY,
      	                    (UVariableName*)0,
                            (UNamedParameters*)0,
                            (UCommand*)0);
      MEMCHECK($$)
    }

  | VAR refvariable {
  
      $$ = new UCommand_DEF(UDEF_VAR,$2,
                            (UNamedParameters*)0,
                            (UCommand*)0);

      MEMCHECK1($$,$2)
    }

  | DEF refvariable {
  
      $$ = new UCommand_DEF(UDEF_VAR,$2,
                            (UNamedParameters*)0,
                            (UCommand*)0);

      MEMCHECK1($$,$2)
    }

  | VAR LBRACKET refvariables RBRACKET {
  
      $$ = new UCommand_DEF(UDEF_VARS,$3);
      MEMCHECK1($$,$3)
    }

  | CLASS IDENTIFIER LBRACKET class_declaration_list RBRACKET {
  
      $$ = new UCommand_CLASS($2,$4);
      MEMCHECK2($$,$2,$4)
    }
    
  | CLASS IDENTIFIER {
  
      $$ = new UCommand_CLASS($2,(UNamedParameters*)0);
      MEMCHECK1($$,$2)
    }

      
  | EVENT variable LPAREN identifiers RPAREN {
    
      $$ = new UCommand_DEF(UDEF_EVENT,$2,$4,(UCommand*)0);
      MEMCHECK2($$,$2,$4);      
    }
    
  | EVENT variable {
    
      $$ = new UCommand_DEF(UDEF_EVENT,$2,(UNamedParameters*)0,(UCommand*)0);
      MEMCHECK1($$,$2);      
    }

  | FUNCTION variable LPAREN identifiers RPAREN {

      if (uparser.connection->functionTag) {
        delete($2);
        delete($4);
        $2 = 0;
        delete uparser.connection->functionTag;
        uparser.connection->functionTag = 0;  
        error(@$,"Nested function def not allowed.");   
        YYERROR;
      }
      else {
	uparser.connection->functionTag = new UString("__Funct__");
	uparser.connection->functionClass = $2->device;
	globalDelete = &uparser.connection->functionTag;
      }
 
    } taggedcommand {
     
      $$ = new UCommand_DEF(UDEF_FUNCTION,$2,$4,$7);
      MEMCHECK2($$,$2,$4);
      if (uparser.connection->functionTag) {
        delete uparser.connection->functionTag;
        uparser.connection->functionTag = 0;  
	globalDelete = 0;
      }      
    }

  | IF LPAREN expr RPAREN taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_IF($3,$5,(UCommand*)0);
      MEMCHECK2($$,$3,$5);
    }

  | IF LPAREN expr RPAREN taggedcommand ELSE taggedcommand {

      $$ = new UCommand_IF($3,$5,$7);
      MEMCHECK3($$,$3,$5,$7);
    }

  | EVERY LPAREN expr RPAREN taggedcommand {

      $$ = new UCommand_EVERY($3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | TIMEOUT LPAREN expr RPAREN taggedcommand {

      $$ = new UCommand_TIMEOUT($3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | STOPIF LPAREN softtest RPAREN taggedcommand {

      $$ = new UCommand_STOPIF($3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | FREEZEIF LPAREN softtest RPAREN taggedcommand {

      $$ = new UCommand_FREEZEIF($3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | AT LPAREN softtest RPAREN taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_AT(CMD_AT,$3,$5,(UCommand*)0);
      MEMCHECK2($$,$3,$5);
    }

  | AT LPAREN softtest RPAREN taggedcommand ONLEAVE taggedcommand {

      $$ = new UCommand_AT(CMD_AT,$3,$5,$7);
      MEMCHECK3($$,$3,$5,$7);
    }

  | AT AND LPAREN softtest RPAREN taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_AT(CMD_AT_AND,$4,$6,(UCommand*)0);
      MEMCHECK2($$,$4,$6);
    }

  | AT AND LPAREN softtest RPAREN taggedcommand ONLEAVE taggedcommand {

      $$ = new UCommand_AT(CMD_AT_AND,$4,$6,$8);
      MEMCHECK3($$,$4,$6,$8);
    }

  | WHILE LPAREN expr RPAREN taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_WHILE(CMD_WHILE,$3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | WHILE PIPE LPAREN expr RPAREN taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_WHILE(CMD_WHILE_PIPE,$4,$6);
      MEMCHECK2($$,$4,$6);
    }

  | WHENEVER LPAREN softtest RPAREN taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_WHENEVER($3,$5,(UCommand*)0);
      MEMCHECK2($$,$3,$5);
    }

  | WHENEVER LPAREN softtest RPAREN taggedcommand ELSE taggedcommand {

      $$ = new UCommand_WHENEVER($3,$5,$7);
      MEMCHECK3($$,$3,$5,$7);
    }

  | LOOP taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_LOOP($2);
      MEMCHECK1($$,$2);
    }

  | FOREACH purevariable IN expr LBRACKET taggedcommands RBRACKET %prec CMDBLOCK {

      $$ = new UCommand_FOREACH(CMD_FOREACH,$2,$4,$6);
      MEMCHECK3($$,$2,$4,$6);
    }

  | FOREACH AND purevariable IN expr LBRACKET taggedcommands RBRACKET %prec CMDBLOCK {

      $$ = new UCommand_FOREACH(CMD_FOREACH_AND,$3,$5,$7);
      MEMCHECK3($$,$3,$5,$7);
    }

  | FOREACH PIPE purevariable IN expr LBRACKET taggedcommands RBRACKET %prec CMDBLOCK {

      $$ = new UCommand_FOREACH(CMD_FOREACH_PIPE,$3,$5,$7);
      MEMCHECK3($$,$3,$5,$7);
    }

  | LOOPN LPAREN expr RPAREN taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_LOOPN(CMD_LOOPN,$3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | LOOPN PIPE LPAREN expr RPAREN taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_LOOPN(CMD_LOOPN_PIPE,$4,$6);
      MEMCHECK2($$,$4,$6);
    }

  | LOOPN AND LPAREN expr RPAREN taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_LOOPN(CMD_LOOPN_AND,$4,$6);
      MEMCHECK2($$,$4,$6);
    }

  | FOR LPAREN instruction SEMICOLON
               expr SEMICOLON
               instruction RPAREN taggedcommand %prec CMDBLOCK { 

      $$ = new UCommand_FOR(CMD_FOR,$3,$5,$7,$9);
      MEMCHECK4($$,$3,$5,$7,$9);
    }

  | FOR PIPE LPAREN instruction SEMICOLON
                    expr SEMICOLON
                    instruction RPAREN taggedcommand %prec CMDBLOCK { 

      $$ = new UCommand_FOR(CMD_FOR_PIPE,$4,$6,$8,$10);
      MEMCHECK4($$,$4,$6,$8,$10);
    }

  | FOR AND LPAREN instruction SEMICOLON
                   expr SEMICOLON
                   instruction RPAREN taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_FOR(CMD_FOR_AND,$4,$6,$8,$10);
      MEMCHECK4($$,$4,$6,$8,$10);
    }
;


/* ARRAY */

array:
  
  /* empty */ { $$ = 0 }
  
  | LSBRACKET expr RSBRACKET array { 
        
      $$ = new UNamedParameters($2,$4);
      MEMCHECK2($$,$2,$4);     
    }
;


/* VARID, PUREVARIABLE, VARIABLE, REFVARIABLE */

purevariable: 

    DOLLAR LPAREN expr RPAREN {

      $$ = new UVariableName($3);
      MEMCHECK1($$,$3);
    }

  | IDENTIFIER array { 
  
      MEMCHECK($1); 
      if (uparser.connection->functionTag) {
	// We are inside a function

	std::string tmpname = std::string(uparser.connection->functionClass->str())
	  + "." + std::string($1->str());
	
	if ((::urbiserver->functiondeftab.find(tmpname.c_str()) != ::urbiserver->functiondeftab.end()) ||
	    (::urbiserver->eventdeftab.find(tmpname.c_str()) != ::urbiserver->eventdeftab.end()) ||
	    (::urbiserver->variabletab.find(tmpname.c_str()) != ::urbiserver->variabletab.end()))
	  $$ = new UVariableName(new UString("self"),$1,false,$2);
	else
	  $$ = new UVariableName(new UString(uparser.connection->functionTag),$1,false,$2);
      }
      else 
	$$ = new UVariableName(new UString(uparser.connection->connectionTag),
	    $1,false,$2);      
      MEMCHECK2($$,$1,$2);
      $$->nostruct = true;
    } 

  | STRUCT array {

      MEMCHECK($1.device);
      MEMCHECK($1.id);
      $$ = new UVariableName($1.device,$1.id,false,$2);
      MEMCHECK3($$,$1.device,$1.id,$2);      
    }
;

/* VARIDS */
/*
purevariables:  

    purevariable { 
            
      $$ = new UNamedParameters(
	  new UExpression(EXPR_VARIABLE,$1),
	  0); 
      MEMCHECK($$); 
    }

  | purevariable COMMA purevariables { 
    
      $$ = new UNamedParameters(
	  new UExpression(EXPR_VARIABLE,$1),
	  $3); 
      MEMCHECK1($$,$3);     
    }
;
*/

variable:

     purevariable {
     
       $$ = $1;
    }

  | AROBASE purevariable {

      $$ = $2;
      $$->isstatic = true;
    }

  | purevariable NORM {

      $$ = $1;
      $$->isnormalized = true;
    }

  | purevariable VARERROR {

      $$ = $1;
      $$->varerror = true;
    }

  | purevariable DERIV {

      $$ = $1;
      $$->deriv = UDERIV;
    }

  | purevariable DERIV2 {

      $$ = $1;
      $$->deriv = UDERIV2;
    }

  | purevariable TRUEDERIV {

      $$ = $1;
      $$->deriv = UTRUEDERIV;
    }

  | purevariable TRUEDERIV2 {

      $$ = $1;
      $$->deriv = UTRUEDERIV2;
    }
;

refvariable:  
    variable {
     
      $$ = $1;
    }

  | ONLY variable {

      $$ = $2;
      $$->rooted = true;
  }
;


/* PROPERTY */

property:

    purevariable DIR IDENTIFIER {

      $$ = new UProperty($1,$3);
      MEMCHECK2($$,$1,$3); 
    }
;


/* NAMEDPARAMETERS */

namedparameters:  
  /* empty */ { $$ = 0 }

  | IDENTIFIER COLON expr namedparameters { 

      MEMCHECK($1);
      $$ = new UNamedParameters($1,$3,$4);
      MEMCHECK3($$,$1,$4,$3);      
    }
;


/* BINARY */

binary: 
    BIN NUM {

      $$ = new UBinary((int)(*$2),0);
      MEMCHECK($$);
      if ($$ != 0)
        MEMCHECK1($$->buffer,$$);
    }

  | BIN NUM rawparameters {

      $$ = new UBinary((int)(*$2),$3);
      MEMCHECK1($$,$3);
      if ($$ != 0)
        MEMCHECK2($$->buffer,$$,$3);
    }
; 


/* TIMEEXPR */

timeexpr:
    TIMEVALUE {
      $$ = $1;
    }

  | timeexpr TIMEVALUE {
       $$ = new UFloat(*$1+*$2); //XXX shall we delete 1 and 2??
       delete $1;
       delete $2;
    }
;


/* EXPR */

expr:     
    NUM { 
     
      $$ = new UExpression(EXPR_VALUE,$1);         
      MEMCHECK($$);
    }

  | timeexpr {

      $$ = new UExpression(EXPR_VALUE,$1);         
      MEMCHECK($$);
    }

  | STRING { 
      
      MEMCHECK($1);
      $$ = new UExpression(EXPR_VALUE,$1);
      MEMCHECK1($$,$1);
    }

  | LSBRACKET parameterlist RSBRACKET {

      $$ = new UExpression(EXPR_LIST,$2);
      MEMCHECK1($$,$2);
    }

  | PERCENT variable { 
      
      $$ = new UExpression(EXPR_ADDR_VARIABLE,$2);
      MEMCHECK1($$,$2);
    }

  | property {
             
       $$ = new UExpression(EXPR_PROPERTY,$1->property,$1->variablename);
       MEMCHECK1($$,$1);
    }

  | refvariable LPAREN parameterlist RPAREN  { 
    
      if (($1) && ($1->device) && 
          ($1->device->equal(uparser.connection->functionTag)))
        $1->nameUpdate(uparser.connection->connectionTag->str(),
                       $1->id->str());

      $$ = new UExpression(EXPR_FUNCTION,$1,$3);
      MEMCHECK2($$,$1,$3);
    }

  | variable { 
    
      $$ = new UExpression(EXPR_VARIABLE,$1); 
      MEMCHECK1($$,$1);      
    }  

  /* num expr */

  | expr PLUS expr { 

      $$ = new UExpression(EXPR_PLUS,$1,$3);
      MEMCHECK2($$,$1,$3);
    }

  | expr MINUS expr { 

      $$ = new UExpression(EXPR_MINUS,$1,$3);
      MEMCHECK2($$,$1,$3);
    }

  | expr MULT expr { 

      $$ = new UExpression(EXPR_MULT,$1,$3);
      MEMCHECK2($$,$1,$3);
    }

  | expr DIV expr { 

      $$ = new UExpression(EXPR_DIV,$1,$3);
      MEMCHECK2($$,$1,$3);
    }

 | expr PERCENT expr { 

      $$ = new UExpression(EXPR_MOD,$1,$3);
      MEMCHECK2($$,$1,$3);
    }

  | COPY expr  %prec NEG { 

      $$ = new UExpression(EXPR_COPY,$2,(UExpression*)0);
      MEMCHECK1($$,$2);
    }

  | MINUS expr %prec NEG { 

      $$ = new UExpression(EXPR_NEG,$2,(UExpression*)0);
      MEMCHECK1($$,$2);
    }

  | expr EXP expr { 

      $$ = new UExpression(EXPR_EXP,$1,$3);
      MEMCHECK2($$,$1,$3);
    }

  | LPAREN expr RPAREN { 

      $$ = $2; 
    } 

  /* Tests */

  | TRUECONST {

      $$ = new UExpression(EXPR_VALUE,TRUE);  
    }

  | FALSECONST {

      $$ = new UExpression(EXPR_VALUE,FALSE);  
    }  

  | expr EQ expr { 

      $$ = new UExpression(EXPR_TEST_EQ,$1,$3); 
      MEMCHECK2($$,$1,$3);
    }

  | expr REQ expr { 

      $$ = new UExpression(EXPR_TEST_REQ,$1,$3); 
      MEMCHECK2($$,$1,$3);
    }

  | expr DEQ expr { 

      $$ = new UExpression(EXPR_TEST_DEQ,$1,$3); 
      MEMCHECK2($$,$1,$3);
    }

  | expr PEQ expr {

      $$ = new UExpression(EXPR_TEST_PEQ,$1,$3); 
      MEMCHECK2($$,$1,$3);
    }

  | expr NE expr { 

      $$ = new UExpression(EXPR_TEST_NE,$1,$3); 
      MEMCHECK2($$,$1,$3);
    }

  | expr GT expr {

      $$ = new UExpression(EXPR_TEST_GT,$1,$3); 
      MEMCHECK2($$,$1,$3);
    }

  | expr GE expr {

      $$ = new UExpression(EXPR_TEST_GE,$1,$3); 
      MEMCHECK2($$,$1,$3);
    }

  | expr LT expr {

      $$ = new UExpression(EXPR_TEST_LT,$1,$3); 
      MEMCHECK2($$,$1,$3);
    }

  | expr LE expr {

      $$ = new UExpression(EXPR_TEST_LE,$1,$3); 
      MEMCHECK2($$,$1,$3);
    }

  | BANG expr {

      $$ = new UExpression(EXPR_TEST_BANG,$2,(UExpression*)0); 
      MEMCHECK1($$,$2);
    }

  | expr ANDOPERATOR expr {

      $$ = new UExpression(EXPR_TEST_AND,$1,$3); 
      MEMCHECK2($$,$1,$3);
    }

  | expr OROPERATOR expr {

      $$ = new UExpression(EXPR_TEST_OR,$1,$3); 
      MEMCHECK2($$,$1,$3);
    }
    /*
    // Not needed anymore => will be handled nicely by aliases
  | 
  GROUPLIST purevariable {
    $$ = new UExpression(EXPR_GROUPLIST,$2); 
      MEMCHECK1($$,$2);
  }
  */
;


/* PARAMETERLIST, PARAMETERS, PARAMETERSERIES */

parameterlist: 
  /* empty */ { $$ = 0; }

  | parameters
;

parameters:   
    expr { 

      $$ = new UNamedParameters($1); 
      MEMCHECK1($$,$1);
    }

  | expr COMMA parameters {

      $$ = new UNamedParameters($1,$3); 
      MEMCHECK2($$,$1,$3);
    }
;

rawparameters:   
    NUM { 

      UExpression *expr = new UExpression(EXPR_VALUE,$1);       
      $$ = new UNamedParameters(expr); 
      MEMCHECK1($$,expr);
    }
 
  | IDENTIFIER { 

      UExpression *expr = new UExpression(EXPR_VALUE,$1);   
      $$ = new UNamedParameters(expr); 
      MEMCHECK1($$,expr);
    }

  |  NUM rawparameters {

      UExpression *expr = new UExpression(EXPR_VALUE,$1); 
      $$ = new UNamedParameters(expr,$2); 
      MEMCHECK2($$,$2,expr);
    }

  |  IDENTIFIER rawparameters {

      UExpression *expr = new UExpression(EXPR_VALUE,$1); 
      $$ = new UNamedParameters(expr,$2); 
      MEMCHECK2($$,$2,expr);
    }
;

/* SOFTTEST */

softtest: 
    expr                 
  | expr TILDE expr  { 

      $$ = $1;
      $$->issofttest = true;
      $$->softtest_time = $3; 
    }                            
  | LPAREN expr TILDE expr RPAREN { 

      $$ = $2;
      $$->issofttest = true;
      $$->softtest_time = $4; 
    }                
;

/* IDENTIFIERS */

identifiers:  
  /* empty */  { $$ = 0; }

  | IDENTIFIER { 
      
      MEMCHECK($1);
      $$ = new UNamedParameters($1,0); 
      MEMCHECK1($$,$1); 
    }

  | IDENTIFIER COMMA identifiers { 

      MEMCHECK($1);
      $$ = new UNamedParameters($1,0,$3); 
      MEMCHECK2($$,$3,$1);     
    }
;

/* CLASS_DELCARATION & CLASS_DECLARATION_LIST */

class_declaration:

    VAR IDENTIFIER {
    
      MEMCHECK($2);
      $$ = new UExpression(EXPR_VALUE,$2);
      MEMCHECK1($$,$2);
    }
    
  | FUNCTION variable LPAREN identifiers RPAREN {
      $$ = new UExpression(EXPR_FUNCTION,$2,$4);
      MEMCHECK2($$,$2,$4);
    }
    
  | FUNCTION variable {
      $$ = new UExpression(EXPR_FUNCTION,$2,(UNamedParameters*)0);
      MEMCHECK1($$,$2);      
    }
    
  | EVENT variable LPAREN identifiers RPAREN {
      $$ = new UExpression(EXPR_EVENT,$2,$4);
      MEMCHECK2($$,$2,$4);
    }
    
  | EVENT variable {
      $$ = new UExpression(EXPR_EVENT,$2,(UNamedParameters*)0);
      MEMCHECK1($$,$2);
    }
;


class_declaration_list:  
  /* empty */  { $$ = 0; }

  | class_declaration { 
            
      $$ = new UNamedParameters($1,0); 
      MEMCHECK1($$,$1); 
    }

  | class_declaration SEMICOLON class_declaration_list { 
      
      $$ = new UNamedParameters($1,$3); 
      MEMCHECK2($$,$3,$1);
    }
;

/* REFVARIABLES */

refvariables:  
  /* empty */  { $$ = 0; }

  | refvariable { 
      
      MEMCHECK($1);
      $$ = new UVariableList($1); 
      MEMCHECK1($$,$1); 
    }

  | refvariable SEMICOLON refvariables { 

      MEMCHECK($1);
      $$ = new UVariableList($1,$3); 
      MEMCHECK2($$,$3,$1);     
    }
;

/* End of grammar */

%%

// The error function that 'bison' calls
void yy::parser::error(const location_type& l, const std::string& what_error) { 

  uparser.error (l, what_error);
  if (globalDelete) {
     delete *globalDelete;
     (*globalDelete) = 0;
  }
}

