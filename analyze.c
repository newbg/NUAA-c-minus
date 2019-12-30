

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include <string.h>

/* counter for variable memory locations */
int SHIFT;
static int location = 2;
static char* scope = " ";
int main_already_seem = 0;
static void declarationError(TreeNode * t, char * message);

/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

static int number_of_parameters(TreeNode * t)
{
    int count = 0;
    TreeNode * aux = t->child[0];

    while(aux != NULL)
    {
      count++;
      aux = aux->sibling;
    }
    return count;
}

static void check_vector_operation(TreeNode * t, char * message)
{
     TreeNode * aux = t;
     BucketList l;

     if(t == NULL) return;

     if(aux->kind.exp != IdK) return;

     l = st_lookup(aux->attr.name, scope);

     if(l == NULL) return;

     if(l->is_vector)
     {
          if(aux->child[0] == NULL)
            declarationError(aux, message);
     }

}

/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 */
static void insertNode( TreeNode * t)
{
      BucketList l;

      switch (t->nodekind)
      {
          case StmtK:
                  switch (t->kind.stmt)
                  {
                        case AssignK:
                          check_vector_operation(t->child[0], "assignment in vector not allowed");
                          break;

                        case IfK: break;
                        case WhileK: break;
                        case ReturnK: break;
                        case ParamsK: break;

                        case CallK:
                              l = st_lookup(t->attr.name, scope);

                              if ( l == NULL)
                                  declarationError(t, "function called was not declared");

                              else if(!strcmp(t->attr.name, "output"))
                              {
                                    int count = number_of_parameters(t);
                                    if(count == 0)
                                      declarationError(t, "function called has no parameters");
                                    else if(count > 1)
                                      declarationError(t, "function called has many parameters");
                              }
                              else if(!strcmp(t->attr.name, "input"))
                              {
                                    if(number_of_parameters(t) > 0)
                                      declarationError(t, "function called has many parameters");

                              }
                              // else if(!strcmp(t->attr.name, "datatohd"))
                              // {
                              //       int count = number_of_parameters(t);
                              //       if(count != 3)
                              //         declarationError(t, "call function needs three parameters");
                              //
                              // }
                               else if(!strcmp(t->attr.name, "hdtodata"))
                              {
                                    int count = number_of_parameters(t);
                                    if(count != 3)
                                      declarationError(t, "call function needs three parameters");

                              }
                               else if(!strcmp(t->attr.name, "nextprocess"))
                              {
                                   int count = number_of_parameters(t);
                                   if(count != 2)
                                      declarationError(t, "call function needs two parameters");

                              }
                              else if(!strcmp(t->attr.name, "regtomem"))
                              {
                                    int count = number_of_parameters(t);
                                    if(count != 2)
                                      declarationError(t, "call function needs two parameters");

                              }
                              else if(!strcmp(t->attr.name, "memtoreg"))
                              {
                                    int count = number_of_parameters(t);
                                    if(count != 2)
                                      declarationError(t, "call function needs two parameters");

                              }
                              else if(!strcmp(t->attr.name, "hdmi"))
                              {
                                   int count = number_of_parameters(t);
                                   if(count != 3)
                                      declarationError(t, "call function needs three parameters");

                              }

                              else if(!strcmp(t->attr.name, "cupr"))
                              {
                                   int count = number_of_parameters(t);
                                   if(count != 1)
                                      declarationError(t, "call function needs one parameters");

                              }

                              else if(!strcmp(t->attr.name, "changecontext"))
                              {
                                   int count = number_of_parameters(t);
                                   if(count != 1)
                                      declarationError(t, "call function needs one parameters");

                              }


                              t->scope = scope;

                              break;

                        case FuncK:
                              scope = t->attr.name;
                              if(strcmp(t->attr.name, "main") == 0)
                                  main_already_seem = 1;
                              l = st_lookup(t->attr.name, " ");
                              if( l == NULL)
                              {
                                  if(t->type==Integer)
                                      st_insert(t->attr.name, t->lineno, location++, 0, " ", "function", "int");
                                  else
                                      st_insert(t->attr.name, t->lineno, location++, 0, " ", "function", "void");
                              }
                              else
                              {
                                declarationError(t, "name already used by another function");
                              }
                        default: break;
                  }
              break;

          case ExpK:
              switch (t->kind.exp)
              {
                    case VectorK:
                        t->scope = scope;
                        break;

                    case OpK:
                      check_vector_operation(t->child[0], "vector operation not allowed");
                      check_vector_operation(t->child[1], "vector operation not allowed");
                      break;
                    case ConstK: break;

                    case IdK:
                          l = st_lookup(t->attr.name, scope);
                          if (l == NULL)
                              declarationError(t, "variable not declared in this scope");
                          else
                          {
                                if(t->already_seem == 0)
                                {
                                    st_insert(t->attr.name, t->lineno, 0, 0, scope, " ", " ");
                                }
                                t->scope = scope;
                          }
                          break;

                    case TypeK:
                          if(t->type == Integer)
                          {
                                  l = st_lookup(t->child[0]->attr.name, scope);
                                  if(l == NULL)
                                  {
                                      if(t->child[0]->is_vector)
                                      {
                                        if(t->child[0]->child[0] != NULL)
                                        {
                                            st_insert(t->child[0]->attr.name, t->child[0]->lineno, location++, 1, scope, "vector", "int");
                                            location += t->child[0]->child[0]->attr.val;
                                        } else
                                            st_insert(t->child[0]->attr.name, t->child[0]->lineno, location++, 2, scope, "vector", "int");

                                      }
                                      else
                                        st_insert(t->child[0]->attr.name, t->child[0]->lineno, location++, 0, scope, "var", "int");

                                      t->child[0]->already_seem = 1;
                                      t->scope = scope;
                                  } else
                                  {
                                      declarationError(t->child[0], "name already used by another variable previosly declared in this scope or by another function");
                                  }
                          }
                          else
                          {
                              if(t->type == Void)
                                  declarationError(t, "variable must not be declared as void");
                          }
                          break;
                    default: break;
              }
          break;

      default: break;
      }
}

static void mainError()
{ if (main_already_seem == 0) {fprintf(listing,"Error: main must be declared\n");
   Error = TRUE;}
}

/* Function buildSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree, int prog)
{

  //shift memoria de dados
  // SHIFT = prog*256;
  // location += SHIFT;

  st_insert("input", 0, 0, 0, " ", "function", "int");
  st_insert("input", 0, 0, 0, " ", "function", "int");
  st_insert("output", 0, 1, 0, " ", "function", "void");
  // st_insert("datatohd", 0, 1, 0, " ", "function", "void");
  // st_insert("hdtodata", 0, 1, 0, " ", "function", "void");
  st_insert("nextprocess", 0, 1, 0, " ", "function", "void");
  st_insert("regtomem", 0, 1, 0, " ", "function", "void");
  st_insert("memtoreg", 0, 1, 0, " ", "function", "void");
  st_insert("hdmi", 0, 1, 0, " ", "function", "void");
  st_insert("cupr", 0, 1, 0, " ", "function", "void");
  st_insert("changecontext", 0, 1, 0, " ", "function", "void");
  // st_insert("inputSensor", 0, 0, 0, " ", "function", "int");
  // st_insert("outputSensor", 0, 0, 0, " ", "function", "int");
  traverse(syntaxTree,insertNode,nullProc);
  if (TraceAnalyze)
  {
    mainError();
    fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

static void declarationError(TreeNode * t, char * message)
{ fprintf(listing,"Declaration error at line %d: (%s) %s\n", t->lineno, t->attr.name, message);
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case OpK:
          if ((t->child[0]->type != Integer) ||
              (t->child[1]->type != Integer))
            typeError(t,"Op applied to non-integer");

          if ((t->attr.op == EQUAL) || (t->attr.op == LT) || (t->attr.op == GT) || (t->attr.op == DIFF) || (t->attr.op == LTEQ)
            || (t->attr.op == GTEQ) )
            t->type = Boolean;
          else
            t->type = Integer;
          break;

        case ConstK:
            t->type = Integer;
            break;

        case IdK:
          t->type = Integer;
          break;

        case VectorK:
            t->type = Integer; break;

        case TypeK: break;

        default:
          break;
      }
      break;

    case StmtK:
      switch (t->kind.stmt)
      { case IfK:
          if (t->child[0]->type == Integer)
            typeError(t->child[0],"if test is not Boolean");
          break;

        case AssignK:
          if (t->child[0]->type != t->child[1]->type)
            typeError(t->child[0],"assignment error: different types");
          break;

        case WhileK:
          if (t->child[0]->type == Integer)
            typeError(t->child[1],"while test is not Boolean");
          break;

        case CallK:
           if (checkFunctionType(t->attr.name)==1)
              t->type = Integer;
          else
              t->type = Void;
          break;

        case FuncK: break;

        default:
          break;
      }
      break;

    default:
      break;

  }
}

/* Procedure typeCheck performs type checking
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}
