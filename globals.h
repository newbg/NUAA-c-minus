

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "symtab.h"

/* Yacc/Bison generates internally its own values
 * for the tokens. Other files can access these values
 * by including the tab.h file generated using the
 * Yacc/Bison option -d ("generate header")
 *
 * The YYPARSER flag prevents inclusion of the tab.h
 * into the Yacc/Bison output itself
 */

#ifndef YYPARSER

/* the name of the following file may change */
#include "cminus.tab.h"

/* ENDFILE is implicitly defined by Yacc/Bison,
 * and not included in the tab.h file
 */
#define ENDFILE 0

#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 8

/* Yacc/Bison generates its own integer values
 * for tokens
 */
typedef int TokenType;

extern FILE* source; /* source code text file */
extern FILE* listing; /* listing output text file */
extern FILE* code; /* code text file for TM simulator */

extern int lineno; /* source line number for listing */

/**************************************************/
/***********   Itermadiario ************/
/**************************************************/

typedef enum {FIRST, LAST} insert_mode;


typedef enum {Empty, IntConst, String, Temp, Label} OperandKind;

typedef enum instrucao {ADD, SUB, MULT, DIV, VEC,
            EQ, NEQ, LTH, LETH, GTH, GETH, ASN,
            FUNC, RTN, PARAM, OUTPARAM, OSPARAM, CALL, ARGS,
            JPF, GOTO, LBL, HALT, IN, INSENSOR, OUT, OUTSENSOR, DATATOHD, PARAMCONTEXT, CHANGECONTEXT,
            HDTODATA, NEXTPROCESS, REGTOMEM, MEMTOREG, HDMI, CUPR} InstructionKind;


typedef struct {
          OperandKind kind;
          union {
                int val;
                struct {
                        const char * name;
                        BucketList bucket;
                      } variable;
          } contents;
} Operand;

/* Estrutura  Quadrupla  que  armazena  os  dados do  codigo  de tres
enderecos*/
typedef struct Quad {
      int instructionMem;
      InstructionKind instruction;
      Operand * op1, * op2, * op3;
      struct  Quad * next;
} * Quadruple;

/* Estrutura que armazena o primeiro e último elemento da lista Quad*/
typedef struct AddressQuad {
    struct Quad *first, *last;
} *AddressQuad;

/*  Operando  para  representar  vazio */
Operand emptyOp;


/**************************************************/
/***********        assembly ************/
/**************************************************/

typedef enum {regTemp, regAddress, immediate, address, regZero} OperatingType;

typedef enum {_ADD, _ADDI, _SUB, _SUBI, _MULT, _DIV, _LW, _LWR, _LI,
              _SW, _SWR, _NOT, _SLT, _SLE, _SGT, _SGE, _SEQ, _SNQ,
              _BEQ, _BNQ, _BGT, _BLT, _JUMP, _JR,
              _NOP, _HLT, _MOVE, _IN, _INSENSOR, _OUT, _OUTSENSOR, _DATATOHD, _HDTODATA, _CHANGECONTEXT,
              _NEXTPROCESS, _REGTOMEM, _MEMTOREG, _HDMI, _CUPR} AssemblyOperation;

typedef struct OperatingT
  { OperatingType type;
      int op;
  } *Operating;

typedef struct AssemblyList
  { AssemblyOperation ao;
    int memlocation;
    struct OperatingT *result, *op1, *op2;
    struct AssemblyList *next;
  } *AssemblyList;

typedef struct AssemblyCode
    {
        int memlocation;
        struct AssemblyList *first, *last;
    } *AssemblyCode;

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum {StmtK,ExpK} NodeKind;
typedef enum {IfK,WhileK,AssignK,ReturnK,FuncK,CallK,ParamsK} StmtKind;
typedef enum {OpK,ConstK,IdK,TypeK,VectorK} ExpKind;

/* ExpType is used for type checking */
typedef enum {Void,Integer,Boolean} ExpType;

#define MAXCHILDREN 6

typedef struct treeNode
   { struct treeNode * child[MAXCHILDREN];
     struct treeNode * sibling;
     int lineno;
     int already_seem;
     NodeKind nodekind;
     union { StmtKind stmt; ExpKind exp;} kind;
     union { TokenType op;
             int val;
             char * name; } attr;
     char * scope;
     Operand * oper;
     int is_vector;
     ExpType type; /* for type checking of exps */
   } TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* EchoSource = TRUE causes the source program to
 * be echoed to the listing file with line numbers
 * during parsing
 */
extern int EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern int TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern int Error;
#endif
