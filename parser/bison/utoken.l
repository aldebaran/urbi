%{
/* \file utoken.l
 *******************************************************************************

 File: utoken.l\n
 Definition of scanner used by the UParser object.
 This scanned is defined with flex, using option c++.

 This file is part of 
 %URBI, version 1.0\n
 (c) Jean-Christophe Baillie, 2004.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../uparser.h"

#include <list>
using namespace std;

#define YY_USER_ACTION locp->columns(yyleng);

%}

%option noyywrap

NUMBER   (\.[0-9]+)|([0-9]+(\.[0-9]*)?)
ID       [a-zA-Z_][_A-Za-z0-9]*
STRING   \"([^"]|"\\\"")*\"
%%
%{
  locp->step();
%}

"//"[^\n]*           /* eat up one-line comments, C++ like   (//) */
"#"[^\n]*            /* eat up one-line comments, shell like (#)  */
\/\*(.|"\n")*\*\/    /* eat up several-lines comments, C++ like   */


"inf"         { valp->val = new UFloat(UINFINITY);
                return NUM;
              }

"pi"          { valp->val = new UFloat(3.14159265358979323846264338327950288);
                return NUM;
              }


"+report"     { valp->val = new UFloat(1); 
                return FLAG;}

"+error"      { valp->val = new UFloat(2);
                return FLAG;}

"+end"        { valp->val = new UFloat(3);
                return FLAG;}

"+begin"      { valp->val = new UFloat(4);
                return FLAG;}

"+timeout"    { valp->val = new UFloat(5);
                return FLAGTIME;}

"+bg"         { valp->val = new UFloat(10);
                return FLAG;}

"+stop"       { valp->val = new UFloat(6);
                return FLAGTEST;}

"+top"        { valp->val = new UFloat(7); 
                return FLAG;}

"+freeze"     { valp->val = new UFloat(8);
                return FLAGTEST;}

"+connection" { valp->val = new UFloat(9);
                return FLAGID;}

{NUMBER}"d"   { yytext[yyleng-1]=0;
                valp->val = new UFloat(atof(yytext) *60000*60*24);
                return TIMEVALUE; }

{NUMBER}"h"   { yytext[yyleng-1]=0;
                valp->val = new UFloat(atof(yytext) *60000*60);
                return TIMEVALUE; }

{NUMBER}"m"   { yytext[yyleng-1]=0;
                valp->val = new UFloat(atof(yytext) *60000);
                return TIMEVALUE; }

{NUMBER}"s"   { yytext[yyleng-1]=0;
                valp->val = new UFloat(atof(yytext) *1000);
                return TIMEVALUE; }

{NUMBER}"ms"  { yytext[yyleng-2]=0;
                valp->val = new UFloat(atof(yytext));
                return TIMEVALUE; }


{NUMBER}      { valp->val = new UFloat(atof(yytext));
                return NUM; }

{STRING}      { valp->str = new UString(yytext+1);
                if (valp->str->str() == 0) {
                  delete (valp->str);
                  valp->str = 0;
                }
                
                if (valp->str != 0) {
                  
                  char* position = const_cast<char*>(&(valp->str->str()[strlen(yytext)-2]));
                  *position=0;
                  /* 
                  while (position = strstr(valp->str->str(),"\\\""))
                    memmove((void*)position,
                            (void*)(position+1), 
                            strlen(valp->str->str())-(int)(position-valp->str->str())); 
                  */
                }
                
                return STRING; }


stop       |
unblock    |
block      |
unfreeze   |
freeze     |
killall    |
disconnect   { valp->str = new UString(yytext); 
               if (valp->str->str() == 0) {
                  delete (valp->str);
                  valp->str = 0;
               }
               return OPERATOR_ID;
             }

info  |
undef        { 
               valp->str = new UString(yytext); 
               if (valp->str->str() == 0) {
                  delete (valp->str);
                  valp->str = 0;
               }
               return OPERATOR_VAR;
             }

mix     |
add     |
normal  |
cancel  |
discard |
queue       { valp->str = new UString(yytext); 
              if (valp->str->str() == 0) {
                delete (valp->str);
                valp->str = 0;
              }
              return STRING;
            }

on     |
off    |
switch     { valp->str = new UString(yytext); 
              if (valp->str->str() == 0) {
                delete (valp->str);
                valp->str = 0;
              }
              return SWITCH;
            }

external |
internal   { valp->str = new UString(yytext); 
             if (valp->str->str() == 0) {
                delete (valp->str);
                valp->str = 0;
             }
             return BINDER;
            }

motoron     |
motoroff    |
strict      |
unstrict    |
ping        |
devices     |
connections |
vars        |
uservars    |
undefall    |
reset       |
commands    |
debugon     |
debugoff    |
quit        |
stopall     |
reboot      |
shutdown      { valp->str = new UString(yytext); 
                if (valp->str->str() == 0) {
                  delete (valp->str);
                  valp->str = 0;
                }
                return OPERATOR;
              }

	      

if            { return IF;}  
else          { return ELSE;} 
var           { return VAR;} 
object        { return OBJECT;}  
function      { return FUNCTION;} 
class         { return CLASS;} 
subclass      { return SUBCLASS;} 
new           { return NEW;} 
event         { return EVENT;} 
while         { return WHILE;}
timeout       { return TIMEOUT;}
stopif        { return STOPIF;}
freezeif      { return FREEZEIF;}
every         { return EVERY;}
at            { return AT;}
whenever      { return WHENEVER;}
onleave       { return ONLEAVE;}
for           { return FOR;}
foreach       { return FOREACH;}
in            { return IN;}
loop          { return LOOP;}
loopn         { return LOOPN;}
group         { return GROUP;}
alias         { return ALIAS;}
echo          { return ECHO;}
wait          { return WAIT;}
waituntil     { return WAITUNTIL;}
emit          { return EMIT;}
def           { return DEF;}
return        { return RETURN;}
"BIN"         { return BIN;}
bin           { return BIN;}
noop          { return NOOP;}
true          { return TRUECONST;}
false         { return FALSECONST;}
copy          { return COPY;} 
only          { return ONLY;}
list          { return GROUPLIST;}

"'n"          { return NORM;}
"'e"          { return VARERROR;}
"'d"          { return TRUEDERIV;}
"'dd"         { return TRUEDERIV2;}
"''"          { return DERIV2;}
"'"           { return DERIV;}
"&&"          { return ANDOPERATOR;}
"||"          { return OROPERATOR;}
";"           { return SEMICOLON;}
":"           { return COLON;}
","           { return COMMA;}
"&"           { return AND;}
"|"           { return PIPE;}
"!"           { return BANG;}
"--"          { return MINUSMINUS;}
"++"          { return PLUSPLUS;} 
"*"           { return MULT;}
"/"           { return DIV;}
"+"           { return PLUS;}
"^"           { return EXP;}
"-"           { return MINUS;}
"="           { return ASSIGN;}
"=="          { return EQ;}
"~="          { return REQ;}
"=~="         { return DEQ;}
"%="          { return PEQ;}
"!="          { return NE;}
">"           { return GT;}
"<"           { return LT;}
"<="          { return LE;}
">="          { return GE;}
"->"          { return DIR;}
"~"           { return TILDE;}
"%"           { return PERCENT;}
"@"           { return AROBASE;}
"("           { return LPAREN;}
")"           { return RPAREN;}
"["           { return LSBRACKET;}
"]"           { return RSBRACKET;}
"{"           { return LBRACKET;}
"}"           { return RBRACKET;}
"$"           { return DOLLAR;}



{ID}          { valp->str = new UString(yytext);
                if (valp->str->str() == 0) {
                  delete (valp->str);
                  valp->str = 0;
                }
                return IDENTIFIER; } 


{ID}"."{ID}   { char* tmp_position = strchr(yytext,'.');
                tmp_position[0] = 0;
                valp->structure.device = new UString(yytext); 
                if (valp->structure.device->str() == 0) {
                  delete (valp->structure.device);
                  valp->structure.device = 0;
                }
                valp->structure.id = new UString(tmp_position+1); 
                if (valp->structure.id->str() == 0) {
                  delete (valp->structure.id);
                  valp->structure.id = 0;
                }
                valp->structure.rooted = false; 
                  
                return STRUCT; } 

[ \t\r]+       locp->step();/* eat up whitespace */
\n+            locp->lines(yyleng);locp->step();
%%
