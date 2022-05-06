%{
#include <assert.h>
#include <qiss_alloc.h>
#include <token.h>

struct Object;
struct Object* yylval;
%}

%option noinput nounput noyyalloc noyyrealloc noyyfree reentrant

digit    [0-9]
exp      e[-+]?[0-9]+
hex      [0-9A-Fa-f]
integer  ([0-9]+|0N|0W)

/*
    All of the following are legal floating point atom literals:
    3f -3f
    3. 3.5 3.f 3.5f 3.e5f
    .5 3.5 3.e5 .3e5f
    3e5 3e5f

    Floating point list literals are particularly complicated:
    1 2 3f
    1. 2 3
    1 .2 3
*/

floate   {integer}{exp}
floatl   {integer}\.[0-9]*{exp}?
floatr   \.[0-9]+{exp}?
float    {floate}|{integer}|{floatl}|{floatr}|0n|0w

date     [0-9][0-9][0-9][0-9](-|\.)[0-9][0-9](-|\.)[0-9][0-9]|0N|0W
time     [0-9][0-9]:[0-9][0-9]:[0-9][0-9]\.[0-9][0-9][0-9]|0N|0W
letter   [A-Za-z]
adverb   [\/\\']:?
op       [~!@#$%^&*\-_=+|:,<\.>?]
strchar  \\.|[^"]
symchar  [A-Za-zi0-9_+\-*%:/\\\.]
ws       [ ]+

%{
%}

%%

  /* Order matters!  Lex goes for maximal munch, but if there's a tie,
     the rule appearing first in this file wins.
  */

(0|1)(0|1)+b                      { yylval = parse_B(yytext); return BOOL_LIST; }
(0|1)b                            { yylval = parse_b(yytext); return BOOL; }

{integer}j?({ws}-?{integer}j?)+j? { yylval = parse_J(yytext); return LONG_LIST; }
{integer}j?                       { yylval = parse_j(yytext); return LONG; }

{integer}i?({ws}-?{integer}i?)+i? { yylval = parse_I(yytext); return INT_LIST; }
{integer}i                        { yylval = parse_i(yytext); return INT; }

{float}f?({ws}-?{float}f?)+f?     { yylval = parse_F(yytext); return FLOAT_LIST; }
{float}f?                         { yylval = parse_f(yytext); return FLOAT; }

0x{hex}{hex}({hex}{hex})+         { yylval = parse_X(yytext); return BYTE_LIST; }
0x{hex}{hex}?                     { yylval = parse_x(yytext); return BYTE; }

{date}d?({ws}{date}d?)+d?         { yylval = parse_D(yytext); return DATE_LIST; }
{date}d?                          { yylval = parse_d(yytext); return DATE; }

{time}t?({ws}{time}t?)+t?         { yylval = parse_T(yytext); return TIME_LIST; }
{time}t?                          { yylval = parse_t(yytext); return TIME; }

delete|export|if|import|insert|select|update  { yylval = parse_id(yytext); return KEYWORD; }

{letter}({letter}|{digit})*       { yylval = parse_id(yytext); return IDENTIFIER; }

`{symchar}*(`{symchar}*)+         { yylval = parse_S(yytext); return SYM_LIST; }
`{symchar}*                       { yylval = parse_s(yytext); return SYM; }
\"{strchar}\"                     { yylval = parse_c(yytext); return CHAR; }
\"\"                              { yylval = parse_C(yytext); return CHAR_LIST; }
\"{strchar}{strchar}+\"           { yylval = parse_C(yytext); return CHAR_LIST; }
{ws}\/.*                          {} /* { yylval = NULL; return COMMENT; } */
^\/.*                             {} /* { yylval = NULL; return COMMENT; } */
^\\\\$                            { yylval = NULL; return QUIT; }
{op}                              { yylval = parse_Op(yytext); return OPERATOR; }
{adverb}                          { yylval = NULL; return ADVERB; }
;                                 { yylval = NULL; return SEMICOLON; }

"("                               { yylval = NULL; return LPAREN; }
")"                               { yylval = NULL; return RPAREN; }
"["                               { yylval = NULL; return LBRACKET; }
"]"                               { yylval = NULL; return RBRACKET; }
"{"                               { yylval = NULL; return LBRACE; }
"}"                               { yylval = NULL; return RBRACE; }

\n                                { yylval = NULL; return NEWLINE; }
{ws}                              { yylval = NULL; return WHITESPACE; }
<<EOF>>                           { yylval = NULL; return 0; }
.                                 { yylval = NULL; return *yytext; }

%%

void* yyalloc  (size_t sz, void* yyscanner)          { return qiss_malloc(sz); }
void* yyrealloc(void* p, size_t sz, void* yyscanner) { return qiss_realloc(p, sz); }
void  yyfree   (void* p, void* yyscanner)            { qiss_free(p); }
int   yywrap   (yyscan_t unused) { return 1; }
