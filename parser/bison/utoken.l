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

//#include <math>
#include <cstdlib>
#include <cstring>

#include "../uparser.h"

// Shorter access to the tokens.
typedef yy::parser::token token;

#define YY_USER_ACTION locp->columns(yyleng);
// Otherwise Flex returns an int instead of an enum.
#define yyterminate() return token::UEOF
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


"inf"         { valp->val = new ufloat(UINFINITY);
                return token::NUM;
              }

"pi"          { valp->val = new ufloat(3.14159265358979323846264338327950288);
                return token::NUM;
              }

"off"          { valp->val = new ufloat(0);
                return token::NUM;
              }

"switch"      { valp->val = new ufloat(-1);
                return token::NUM;
              }


"on"          { valp->val = new ufloat(1);
                return token::NUM;
              }



"+report"     { valp->val = new ufloat(1); 
                return token::FLAG;}

"+error"      { valp->val = new ufloat(2);
                return token::FLAG;}

"+end"        { valp->val = new ufloat(3);
                return token::FLAG;}

"+begin"      { valp->val = new ufloat(4);
                return token::FLAG;}

"+timeout"    { valp->val = new ufloat(5);
                return token::FLAGTIME;}

"+bg"         { valp->val = new ufloat(10);
                return token::FLAG;}

"+stop"       { valp->val = new ufloat(6);
                return token::FLAGTEST;}

"+top"        { valp->val = new ufloat(7); 
                return token::FLAG;}

"+freeze"     { valp->val = new ufloat(8);
                return token::FLAGTEST;}

"+connection" { valp->val = new ufloat(9);
                return token::FLAGID;}

{NUMBER}"d"   { yytext[yyleng-1]=0;
                valp->val = new ufloat(atof(yytext) *60000*60*24);
                return token::TIMEVALUE; }

{NUMBER}"h"   { yytext[yyleng-1]=0;
                valp->val = new ufloat(atof(yytext) *60000*60);
                return token::TIMEVALUE; }

{NUMBER}"m"   { yytext[yyleng-1]=0;
                valp->val = new ufloat(atof(yytext) *60000);
                return token::TIMEVALUE; }

{NUMBER}"s"   { yytext[yyleng-1]=0;
                valp->val = new ufloat(atof(yytext) *1000);
                return token::TIMEVALUE; }

{NUMBER}"ms"  { yytext[yyleng-2]=0;
                valp->val = new ufloat(atof(yytext));
                return token::TIMEVALUE; }


{NUMBER}      { valp->val = new ufloat(atof(yytext));
                return token::NUM; }

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
                
                return token::STRING; }


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
               return token::OPERATOR_ID;
             }

info  |
undef        { 
               valp->str = new UString(yytext); 
               if (valp->str->str() == 0) {
                  delete (valp->str);
                  valp->str = 0;
               }
               return token::OPERATOR_VAR;
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
              return token::STRING;
            }

external |
internal   { valp->str = new UString(yytext); 
             if (valp->str->str() == 0) {
                delete (valp->str);
                valp->str = 0;
             }
             return token::BINDER;
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
                return token::OPERATOR;
              }

	      

if            { return token::IF;}  
else          { return token::ELSE;} 
var           { return token::VAR;} 
object        { return token::OBJECT;}  
function      { return token::FUNCTION;} 
class         { return token::CLASS;} 
subclass      { return token::SUBCLASS;} 
new           { return token::NEW;} 
event         { return token::EVENT;} 
while         { return token::WHILE;}
timeout       { return token::TIMEOUT;}
stopif        { return token::STOPIF;}
freezeif      { return token::FREEZEIF;}
every         { return token::EVERY;}
at            { return token::AT;}
whenever      { return token::WHENEVER;}
onleave       { return token::ONLEAVE;}
for           { return token::FOR;}
foreach       { return token::FOREACH;}
in            { return token::IN;}
loop          { return token::LOOP;}
loopn         { return token::LOOPN;}
group         { return token::GROUP;}
addgroup      { return token::ADDGROUP;}
delgroup      { return token::DELGROUP;}
alias         { return token::ALIAS;}
unalias       { return token::UNALIAS;}
echo          { return token::UECHO;}
wait          { return token::WAIT;}
waituntil     { return token::WAITUNTIL;}
emit          { return token::EMIT;}
def           { return token::DEF;}
return        { return token::RETURN;}
"BIN"         { return token::BIN;}
bin           { return token::BIN;}
noop          { return token::NOOP;}
true          { return token::TRUECONST;}
false         { return token::FALSECONST;}
copy          { return token::COPY;} 
only          { return token::ONLY;}
list          { return token::GROUPLIST;}

"'n"          { return token::NORM;}
"'e"          { return token::VARERROR;}
"'d"          { return token::TRUEDERIV;}
"'dd"         { return token::TRUEDERIV2;}
"''"          { return token::DERIV2;}
"'"           { return token::DERIV;}
"&&"          { return token::ANDOPERATOR;}
"||"          { return token::OROPERATOR;}
";"           { return token::SEMICOLON;}
":"           { return token::COLON;}
","           { return token::COMMA;}
"&"           { return token::AND;}
"|"           { return token::PIPE;}
"!"           { return token::BANG;}
"--"          { return token::MINUSMINUS;}
"++"          { return token::PLUSPLUS;} 
"+="          { return token::PLUSASSIGN;} 
"-="          { return token::MINUSASSIGN;} 
"*"           { return token::MULT;}
"/"           { return token::DIV;}
"+"           { return token::PLUS;}
"^"           { return token::EXP;}
"-"           { return token::MINUS;}
"="           { return token::ASSIGN;}
"=="          { return token::EQ;}
"~="          { return token::REQ;}
"=~="         { return token::DEQ;}
"%="          { return token::PEQ;}
"!="          { return token::NE;}
">"           { return token::GT;}
"<"           { return token::LT;}
"<="          { return token::LE;}
">="          { return token::GE;}
"->"          { return token::DIR;}
"~"           { return token::TILDE;}
"%"           { return token::PERCENT;}
"@"           { return token::AROBASE;}
"("           { return token::LPAREN;}
")"           { return token::RPAREN;}
"["           { return token::LSBRACKET;}
"]"           { return token::RSBRACKET;}
"{"           { return token::LBRACKET;}
"}"           { return token::RBRACKET;}
"$"           { return token::DOLLAR;}



{ID}          { valp->str = new UString(yytext);
                if (valp->str->str() == 0) {
                  delete (valp->str);
                  valp->str = 0;
                }
                return token::IDENTIFIER; } 


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
                  
                return token::STRUCT; } 


[ \t\r]+       locp->step();/* eat up whitespace */
\n+            locp->lines(yyleng);locp->step();
.              {
                 uparser.error(*locp, std::string ("invalid character: ")
		                      + yytext);
		 locp->step ();
	       }
%%
