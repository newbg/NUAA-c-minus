

#include "globals.h"
#include "symtab.h"

void codeGen(TreeNode *syntaxTree, FILE * code, int prog);

void codeGenerationBinary(AssemblyCode CodeAssembly, FILE * codefile, int prog);


/*typedef enum {Empty, IntConst, String, Temp, Label} OperandKind;

typedef enum instrucao {ADD, SUB, MULT, DIV, VEC,
            EQ, NEQ, LTH, LETH, GTH, GETH, ASN,
            FUNC, RTN, PARAM, CALL , ARGS,
            JPF, GOTO, LBL, HALT} InstructionKind;


typedef struct {
          OperandKind kind;
          union {
                int val;
                struct {
                        const char * name;
                        //struct ScopeRec * scope;
                      } variable;
          } contents;
} Operand;



/*  Estrutura  Quadrupla  que  armazena  os  dados do  codigo  de tres
enderecos*/
/*typedef struct Quad {
      InstructionKind instruction;
      Operand op1, op2, op3;
      struct  Quad * next;
} * Quadruple;

/* Estrutura que armazena o primeiro e último elemento da lista Quad*/
/*typedef struct AddressQuad {
    struct Quad *first, *last;
} *AddressQuad;*/
