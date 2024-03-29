/* Request detailed parse error messages. */
%error-verbose
%locations
%define api.pure full

%{
    #include "../core/node.h"
    #include <cstdio>
    #include <cstdlib>
    #define ANSI_COLOR_RED     "\x1b[31m"
    #define ANSI_COLOR_YELLOW  "\x1b[33m"
    #define ANSI_COLOR_RESET   "\x1b[0m"

    NBlock *programBlock; /* the top level root node of our final AST */
    extern int yylex(union YYSTYPE*, struct YYLTYPE*);
    void yyerror (struct  YYLTYPE *llocp, const char *s);
%}

/* Represents the many different ways we can access our data */
%union {
    Node *node;
    NBlock *block;
    NExpression *expr;
    NStatement *stmt;
    NIdentifier *ident;
    NVariableDeclaration *var_decl;
    VariableList *varvec;
    ExpressionList *exprvec;
    NReference *ref;
    std::string *string;
    int token;
    VariableType vartype;
}

/* Define our terminal symbols (tokens). This should
   match our lexer.l lex file. We also define the node type
   they represent.
 */
%token <string> TIDENTIFIER TINTEGER TDOUBLE TSTRING
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT
%token <token> TPLUS TMINUS TMUL TDIV
%token <token> TRETURN TEXTERN
%token <token> TBREAK TCASE TCONST TCONTINUE TDEFAULT TDO TELSE TENUM
%token <token> TFOR TIF TSWITCH TVOID TWHILE TFOREACH TNOT TLOOP TIN
%token <token> TTRUE TFALSE
%token <token>  TINTEGERKEY TDOUBLEKEY TSTRINGKEY TOBJECTKEY

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <ident> ident
%type <ref> ref
%type <expr> numeric expr
%type <varvec> func_decl_args
%type <exprvec> call_args
%type <block> program stmts block
%type <stmt> stmt var_decl func_decl extern_decl
%type <token> comparison
%type <vartype> var_type

/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV

%start program

%%

program : stmts { programBlock = $1; }
        ;

stmts : stmt { $$ = new NBlock(); $$->statements.push_back($<stmt>1); }
    | stmts stmt { $1->statements.push_back($<stmt>2); }
    ;

stmt : var_decl | func_decl | extern_decl
   | expr { $$ = new NExpressionStatement(*$1); }
   | TRETURN expr { $$ = new NReturnStatement(*$2); }
   ;

block : TLBRACE stmts TRBRACE { $$ = $2; }
    | TLBRACE TRBRACE { $$ = new NBlock(); }
    ;

var_decl : var_type ident { $$ = new NVariableDeclaration($1, *$2); }
         | var_type ident TEQUAL expr { $$ = new NVariableDeclaration($1, *$2, $4); }
         ;

extern_decl : TEXTERN var_type ident TLPAREN func_decl_args TRPAREN
                { $$ = new NExternDeclaration($2, *$3, *$5); delete $5; }
            ;

var_type : TINTEGERKEY { $$ = VariableType::Integer; }
         | TDOUBLEKEY { $$ = VariableType::Double; }
         | TSTRINGKEY { $$ = VariableType::String; }
         | TOBJECTKEY { $$ = VariableType::Object; }
         | TVOID { $$ = VariableType::Void; }
         ;

func_decl : var_type ident TLPAREN func_decl_args TRPAREN block
      { $$ = new NFunctionDeclaration($1, *$2, *$4, *$6); delete $4; }
      ;

func_decl_args : /*blank*/  { $$ = new VariableList(); }
      | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
      | func_decl_args TCOMMA var_decl { $1->push_back($<var_decl>3); }
      ;

ref : ident { $$ = new NReference(); $$->refs.push_back($1); }
    | ref TDOT ident { $1->refs.push_back($3); }
    ;

ident : TIDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
      ;

expr : ref TEQUAL expr { $$ = new NAssignment(*$1, *$3); }
     | ref TLPAREN call_args TRPAREN { $$ = new NMethodCall(*$1, *$3); delete $3; }
     | ref { $<ref>$ = $1; }
     | numeric
     | TSTRING { $$ = new NString(*$1); }
     | expr TMUL expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | expr TDIV expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | expr TPLUS expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | expr TMINUS expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | expr comparison expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | TLPAREN expr TRPAREN { $$ = $2; }
     | block
     ;

numeric : TINTEGER { $$ = new NInteger(atol($1->c_str())); delete $1; }
        | TDOUBLE { $$ = new NDouble(atof($1->c_str())); delete $1; }
        ;

call_args : /*blank*/  { $$ = new ExpressionList(); }
      | expr { $$ = new ExpressionList(); $$->push_back($1); }
      | call_args TCOMMA expr  { $1->push_back($3); }
      ;

comparison : TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE;

%%

void yyerror(YYLTYPE *llocp, const char *s)
{
    std::printf("Satır: " ANSI_COLOR_YELLOW "%d" ANSI_COLOR_RESET " Sütun: " \
                ANSI_COLOR_YELLOW "%d" ANSI_COLOR_RESET ":" ANSI_COLOR_RED \
                " Sözdizimi hatası:" ANSI_COLOR_RESET " %s\n",
                llocp->first_line, llocp->first_column, s);
    std::exit(1);
}
