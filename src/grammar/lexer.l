%{
#include <string>
#include "../core/node.h"
#include "parser.hpp"

#define SAVE_TOKEN  yylval->string = new std::string(yytext, yyleng)
#define SAVE_STRING yylval->string = new std::string(yytext + 1, yyleng - 2)
#define TOKEN(t)    (yylval->token = t)

#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno; \
    yylloc->first_column = yycolumn; \
    yycolumn += yyleng;

int yycolumn = 1;

%}

%option bison-locations
%option yylineno
%option noyywrap

%%

[ \t]                           ;
"çık"                           return TOKEN(TBREAK);
"durum"                         return TOKEN(TCASE);
"sabit"                         return TOKEN(TCONST);
"atla"                          return TOKEN(TCONTINUE);
"kabul"                         return TOKEN(TDEFAULT);
"yap"                           return TOKEN(TDO);
"yoksa"                         return TOKEN(TELSE);
"seçimli"                       return TOKEN(TENUM);
"çevrim"                        return TOKEN(TFOR);
"eğer"                          return TOKEN(TIF);
"döndür"                        return TOKEN(TRETURN);
"şartlıgir"                     return TOKEN(TSWITCH);
"sürece"                        return TOKEN(TWHILE);
"herbir"                        return TOKEN(TFOREACH);
"değil"                         return TOKEN(TNOT);
"döngü"                         return TOKEN(TLOOP);
"içindeki"                      return TOKEN(TIN);
"dış"                           return TOKEN(TEXTERN);
"doğru"                         return TOKEN(TTRUE);
"yanlış"                        return TOKEN(TFALSE);
"yok"                           return TOKEN(TVOID);
"sayı"                          return TOKEN(TINTEGERKEY);
"ondalıklı"                     return TOKEN(TDOUBLEKEY);
"yazı"                          return TOKEN(TSTRINGKEY);
"nesne"                         return TOKEN(TOBJECTKEY);
[a-zA-Z_][a-zA-Z0-9_]*          SAVE_TOKEN; return TIDENTIFIER;
[0-9]+\.[0-9]*                  SAVE_TOKEN; return TDOUBLE;
[0-9]+                          SAVE_TOKEN; return TINTEGER;
\"[^"]*\"                       SAVE_STRING; return TSTRING;

"="                             return TOKEN(TEQUAL);
"=="                            return TOKEN(TCEQ);
"!="                            return TOKEN(TCNE);
"<"                             return TOKEN(TCLT);
"<="                            return TOKEN(TCLE);
">"                             return TOKEN(TCGT);
">="                            return TOKEN(TCGE);

"("                             return TOKEN(TLPAREN);
")"                             return TOKEN(TRPAREN);
"{"                             return TOKEN(TLBRACE);
"}"                             return TOKEN(TRBRACE);

"."                             return TOKEN(TDOT);
","                             return TOKEN(TCOMMA);

"+"                             return TOKEN(TPLUS);
"-"                             return TOKEN(TMINUS);
"*"                             return TOKEN(TMUL);
"/"                             return TOKEN(TDIV);

"//".*                          { /* Do nothing, single line comment */ }
"\n"                             yycolumn = 1;

[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/] { /* Do nothing, multi line comment */ }
[/][*]                              printf("Sonlandırılmamış Yorum!\n"); yyterminate();

.                                   printf("Bilinmeyen Simge!\n"); yyterminate();

%%
