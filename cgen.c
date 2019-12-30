
#include "cgen.h"


#define MAXTEMP 64
static int tempOffset[MAXTEMP];
static int label = 0;
static int insMem = 0;
static int exist_return = 0;

AddressQuad CodeAddrQ;

int IS_OS;

void code_gen(TreeNode *tree);

static int create_label()
{
	return label++;
}

void initialize_temp_register()
{
	int i;
	for(i=0; i < MAXTEMP; i++)
		tempOffset[i] = 0;
}

static int temp_register()
{
	int i;
	// for(i=(IS_OS*13 + 1); i < MAXTEMP; i++)
	// 	if(!tempOffset[i])
	// 		break;

	for(i=1; i < MAXTEMP; i++)
		if(!tempOffset[i])
			break;

	tempOffset[i] = 1;
	return i;
}

void temp_register_discart(int t)
{
	tempOffset[t] = 0;
}


AddressQuad create_quad()
{
		AddressQuad CodeAddrQ = (AddressQuad) malloc(sizeof(struct AddressQuad));
		if(CodeAddrQ != NULL)
			CodeAddrQ->first = CodeAddrQ->last = NULL;
		return CodeAddrQ;
}

static int is_function_void_or_int(TreeNode *t){
	  BucketList b = st_lookup(t->attr.name, "Global");

	  if(strcmp(b->typedata,"void") == 0)
	  	return 0;

	  return 1; //int
}

static Quadruple create_element_quad(InstructionKind instruction, Operand * op1, Operand * op2, Operand * op3)
{
		Quadruple new_element = (Quadruple) malloc(sizeof(struct Quad));
		new_element->instruction = instruction;
		new_element->instructionMem = ++insMem;
		new_element->op1 = op1;
		new_element->op2 = op2;
		new_element->op3 = op3;
		new_element->next = NULL;
		return new_element;
}

int insert_quad(AddressQuad CodeAddrQ, InstructionKind instruction, Operand * op1, Operand * op2, Operand * op3, insert_mode mode)
{
		Quadruple new_element = create_element_quad(instruction, op1, op2, op3);
		if (new_element == NULL || CodeAddrQ == NULL) return 1; //ERROR

		if(CodeAddrQ->first == NULL) //empty list
				CodeAddrQ->first = CodeAddrQ->last = new_element;
		else if(mode == LAST) //insert LAST
		{
				CodeAddrQ->last->next = new_element;
				CodeAddrQ->last = new_element;
		}
		else if(mode == FIRST) //insert first
		{
			new_element->next = CodeAddrQ->first;
			CodeAddrQ->first = new_element;
		}
		return 0;
}

static Operand * create_operand_element(TreeNode *tree, const char * name, int val, char * scope, OperandKind kind)
{
	Operand * new_element = (Operand*) malloc(sizeof(Operand));

	if (tree == NULL) return NULL;

	new_element->kind = kind;

	switch(kind)
	{
		case String:
			new_element->contents.variable.name = name;
			new_element->contents.variable.bucket = st_lookup(tree->attr.name, scope);
			break;
		case IntConst:
			new_element->contents.val = val;
			break;
		case Temp:
			new_element->contents.val = val;
			break;
		case Label:
			new_element->contents.val = val;
			break;
		default: break;
	}

	return new_element;
}

void insert_instruction_vector(TreeNode *tree)
{
	Operand *op1, *op2, *op3;
	op1 = tree->oper;
	op2 = tree->child[0]->oper;
	op3 = create_operand_element(tree, NULL, temp_register(), NULL, Temp);
	tree->oper = op3;
	insert_quad(CodeAddrQ, ASN, op1, op2, op3, LAST);
}

InstructionKind instruction_operator_kind(TreeNode *tree)
{
	InstructionKind op;
	switch(tree->attr.op)
	{
		case PLUS:
			op = ADD;
			break;

		case MINUS:
			op = SUB;
			break;

		case TIMES:
			op = MULT;
			break;

		case OVER:
			op = DIV;
			break;

		case LT:
			op = LTH;
			break;

		case LTEQ:
			op = LETH;
			break;

		case GT:
			op = GTH;
			break;

		case GTEQ:
			op = GETH;
			break;

		case EQUAL:
			op = EQ;
			break;

		case DIFF:
			op = NEQ;
			break;

	}
	return op;

}

int is_os_param(char * name)
{

		// if(!strcmp(name,"datatohd"))
		// 	return 1;
		// else if(!strcmp(name,"hdtodata"))
		// 	return 1;
		// else if(!strcmp(name,"nextprocess"))
		// 	return 1;
		if(!strcmp(name,"hdmi"))
			return 1;
		else if(!strcmp(name,"cupr"))
			return 1;

		return 0;
}


int is_context_param(char * name)
{

		if(!strcmp(name,"regtomem"))
			return 1;
		else if(!strcmp(name,"memtoreg"))
			return 1;
		else if(!strcmp(name,"changecontext"))
			return 1;

		return 0;
}

static void genStmtK(AddressQuad CodeAddrQ, TreeNode *tree)
{
			TreeNode *p1, *p2, *p3;
			Operand *op1, *op2, *op3, *op_goto;

			op1 = op2 = op3 = NULL;
			switch(tree->kind.stmt)
			{
					case AssignK:
						p1 = tree->child[0];
						p2 = tree->child[1];
						code_gen(p1);
						code_gen(p2);


						//if vector is true
						if(p1->is_vector && p2->is_vector)
						{
							op1 = p2->oper;
							op2 = p2->child[0]->oper;
							op3 = create_operand_element(tree, NULL, temp_register(), NULL, Temp);
							insert_quad(CodeAddrQ, ASN, op1, op2, op3, LAST);
							op1 = op3;
							op2 = p1->child[0]->oper;
							op3 = p1->oper;

						} else
						{
							op1 = p2->oper;
							op3 = p1->oper;

							if(p1->is_vector)
								op2 = p1->child[0]->oper;
							else if(p2->is_vector)
								op2 = p2->child[0]->oper;
						}

						insert_quad(CodeAddrQ, ASN, op1, op2, op3, LAST);

						if(op1->kind == Temp)
							temp_register_discart(op1->contents.val);

						break;

					case IfK:
						p1 = tree->child[0];
						p2 = tree->child[1];
						p3 = tree->child[2];

						//chama exp
						code_gen(p1);

						//insert if_false
						op1 = p1->oper;
						op3 = create_operand_element(tree, NULL, create_label(), NULL, Label);
						insert_quad(CodeAddrQ, JPF, op1, op2, op3, LAST);

						//chama stmten
						code_gen(p2);

						op1 = NULL;

						//caso exista else
						if(p3 != NULL)
						{
							//enter goto, because if "if" is true, then skip the "else"
							op_goto = create_operand_element(tree, NULL, create_label(), NULL, Label);
							insert_quad(CodeAddrQ, GOTO, op1, op2, op_goto, LAST);

							//enter the same lab as if false, because if "if" is false, then skip the "if"
							insert_quad(CodeAddrQ, LBL, op1, op2, op3, LAST);

							code_gen(p3);

							//enter the same lab as goto, because if "if" is true, then skip the "else"
							insert_quad(CodeAddrQ, LBL, op1, op2, op_goto, LAST);
						}
						else
							//enter the same lab as if false, because if "if" is false, then skip the "if"
							insert_quad(CodeAddrQ, LBL, op1, op2, op3, LAST);

						break;

					case WhileK:

						p1 = tree->child[0];
						p2 = tree->child[1];

						//insert lab para goto
						op_goto = create_operand_element(tree, NULL, create_label(), NULL, Label);
						insert_quad(CodeAddrQ, LBL, op1, op2, op_goto, LAST);

						//chama exp
						code_gen(p1);

						//insert if_false
						op1 = p1->oper;
						op3 = create_operand_element(tree, NULL, create_label(), NULL, Label);
						insert_quad(CodeAddrQ, JPF, op1, op2, op3, LAST);

						//chama stmtem
						code_gen(p2);

						//insert goto do while
						op1 = NULL;
						insert_quad(CodeAddrQ, GOTO, op1, op2, op_goto, LAST);

						//insert lab se if for false
						insert_quad(CodeAddrQ, LBL, op1, op2, op3, LAST);

						break;

					case ReturnK:

						p1 = tree->child[0];
						if (p1 != NULL)
						{
							code_gen(p1);
							op3 = p1->oper;

							if(p1->is_vector && p1->child[0] != NULL)
							{

									TreeNode *aux = p1->child[0];
									switch (aux->kind.stmt)
									{
										case IdK:
											op2 = create_operand_element(aux, aux->attr.name, 0, aux->scope, String);
											break;
										case ConstK:
											op2 = create_operand_element(aux, NULL, aux->attr.val, NULL, IntConst);
											break;
										case OpK:
											op2 = aux->oper;
											break;
										default: break;
									}
									op1 = op3;
									op3 = create_operand_element(p1, NULL, temp_register(), NULL, Temp);
									insert_quad(CodeAddrQ, ASN, op1, op2, op3, LAST);
									op1 = NULL;
							}
						}


						insert_quad(CodeAddrQ, RTN, op1, op2, op3, LAST);
						exist_return = 1;

						break;

					case CallK:
						p1 = tree->child[0];
						code_gen(p1);

						int count = 0;

						//Insert params, if not input
						if(strcmp(tree->attr.name,"input") || strcmp(tree->attr.name,"inputSensor"))
						{

							TreeNode *aux = p1;
							InstructionKind ins = PARAM;

							if(!strcmp(tree->attr.name,"output"))
								ins = OUTPARAM;
							else if(!strcmp(tree->attr.name,"outputSensor"))
								ins = OUTPARAM;
							else if(is_os_param(tree->attr.name))
								ins = OSPARAM;
							else if(is_context_param(tree->attr.name))
								ins = PARAMCONTEXT;

							while(aux != NULL)
							{
								op1 = aux->oper;
								op2 = NULL;

								if(aux->is_vector)
								{
									code_gen(aux->child[0]);
									op2 = aux->child[0]->oper;
								}

								insert_quad(CodeAddrQ, ins, op1, op2, op3, LAST);
								aux = aux->sibling;
								count++;
							}
						}

						//Name function
						op1 = create_operand_element(tree, tree->attr.name, 0, tree->scope, String);
						//quantidade parametros
						op2 = create_operand_element(p1, NULL, count, NULL, IntConst);
						//If function type int, create temp
						if(is_function_void_or_int(tree))
							op3 = create_operand_element(tree, NULL, temp_register(), NULL, Temp);

						tree->oper = op3;

						char * name = tree->attr.name;
						if(!strcmp(name,"input"))
							insert_quad(CodeAddrQ, IN, op1, NULL, op3, LAST);
						else if(!strcmp(name,"output"))
							insert_quad(CodeAddrQ, OUT, op1, op2, op3, LAST);
						else if(!strcmp(name,"datatohd"))
							insert_quad(CodeAddrQ, DATATOHD, op1, op2, op3, LAST);
						else if(!strcmp(name,"hdtodata"))
							insert_quad(CodeAddrQ, HDTODATA, op1, op2, op3, LAST);
						else if(!strcmp(name,"nextprocess"))
							insert_quad(CodeAddrQ, NEXTPROCESS, op1, op2, op3, LAST);
						else if(!strcmp(name,"regtomem"))
							insert_quad(CodeAddrQ, REGTOMEM, op1, op2, op3, LAST);
						else if(!strcmp(name,"hdmi"))
							insert_quad(CodeAddrQ, HDMI, op1, op2, op3, LAST);
						else if(!strcmp(name,"memtoreg"))
							insert_quad(CodeAddrQ, MEMTOREG, op1, op2, op3, LAST);
						else if(!strcmp(name,"cupr"))
							insert_quad(CodeAddrQ, CUPR, op1, op2, op3, LAST);
						else if(!strcmp(name,"changecontext"))
							insert_quad(CodeAddrQ, CHANGECONTEXT, op1, op2, op3, LAST);
						// else if(!strcmp(name,"inputSensor"))
						// 	insert_quad(CodeAddrQ, INSENSOR, op1, NULL, op3, LAST);
						// else if(!strcmp(name,"outputSensor"))
						// 	insert_quad(CodeAddrQ, OUTSENSOR, op1, op2, op3, LAST);
						else
							insert_quad(CodeAddrQ, CALL, op1, op2, op3, LAST);

						break;

					case FuncK:
						exist_return = 0;
						op1 = create_operand_element(tree, tree->attr.name, 0, "Global", String);
						p1 = tree->child[0];
						p2 = tree->child[1];
						insert_quad(CodeAddrQ, FUNC, op1, op2, op3, LAST);

						//insert call main in the first list element
						if(strcmp(tree->attr.name,"main")==0)
							insert_quad(CodeAddrQ, CALL, op1, op2, op3, FIRST);

						code_gen(p1);
						code_gen(p2);

						/*função não declarado return, então cria-se um return vazio*/
						if(!exist_return)
						{
							insert_quad(CodeAddrQ, RTN, NULL, NULL, NULL, LAST);
							exist_return = 0;
						}
						break;

					case ParamsK:
						p1 = tree->child[0];
						TreeNode *args, *child;
						args = p1;
						while(args != NULL)
						{

							child = args->child[0];
							code_gen(child);
							insert_quad(CodeAddrQ, ARGS, child->oper, NULL, NULL, LAST);

							args = args->sibling;
						}
						break;
					default: break;

			}

}

void genExp(AddressQuad CodeAddrQ, TreeNode *tree)
{
			TreeNode *p1, *p2, *p3;
			Operand *op1, *op2, *op3;
			op1 = op2 = op3 = NULL;
			switch(tree->kind.exp)
			{
					case OpK:
							p1 = tree->child[0];
							p2 = tree->child[1];
							code_gen(p1);
							code_gen(p2);

							//se vector
							if(p1->is_vector)
								insert_instruction_vector(p1);

							if(p2->is_vector)
								insert_instruction_vector(p2);


							op1 = p1->oper;
							op2 = p2->oper;
							op3 = create_operand_element(tree, NULL, temp_register(), NULL, Temp);
							tree->oper = op3;
							insert_quad(CodeAddrQ, instruction_operator_kind(tree), op1, op2, op3, LAST);
							break;

					case ConstK:
						op1 = create_operand_element(tree, NULL, tree->attr.val, NULL, IntConst);
						tree->oper = op1;
						break;

					case IdK:
						op1 = create_operand_element(tree, tree->attr.name, 0, tree->scope, String);
						tree->oper = op1;
						break;

					case TypeK:
						break;

					case VectorK:
						p1 = tree->child[0];
						code_gen(p1);

						op1 = create_operand_element(tree, tree->attr.name, 0, tree->scope, String);
						tree->oper = op1;
						//tree->is_vector = 1;

						break;

					default: break;
			}
}

void print_instruction(InstructionKind ins)
{
	switch (ins) {
		case ADD:
			fprintf(listing, "add");
			break;
		case SUB:
			fprintf(listing, "sub");
			break;
		case MULT:
			fprintf(listing, "mult");
			break;
		case DIV:
			fprintf(listing, "div");
			break;
		case VEC:
			fprintf(listing, "vector");
			break;
		case EQ:
			fprintf(listing, "equal");
			break;
		case NEQ:
			fprintf(listing, "not equal");
			break;
		case LTH:
			fprintf(listing, "less than");
			break;
		case LETH:
			fprintf(listing, "less equal than");
			break;
		case GTH:
			fprintf(listing, "greater than");
			break;
		case GETH:
			fprintf(listing, "greater equal than");
			break;
		case ASN:
			fprintf(listing, "assign");
			break;
		case FUNC:
			fprintf(listing, "function");
			break;
		case RTN:
			fprintf(listing, "return");
			break;
		case PARAM:
			fprintf(listing, "param");
			break;
		case OUTPARAM:
			fprintf(listing, "param");
			break;
		case OSPARAM:
			fprintf(listing, "param");
			break;
		case PARAMCONTEXT:
			fprintf(listing, "param");
			break;
		case CALL:
			fprintf(listing, "call");
			break;
		case ARGS:
			fprintf(listing, "arguments");
			break;
		case JPF:
			fprintf(listing, "if_false");
			break;
		case GOTO:
			fprintf(listing, "goto");
			break;
		case LBL:
			fprintf(listing, "label");
			break;
		case IN:
			fprintf(listing, "call");
			break;
		// case INSENSOR:
		// 	fprintf(listing, "call");
		// 	break;
		case OUT:
			fprintf(listing, "call");
			break;
		// case OUTSENSOR:
		// 	fprintf(listing, "call");
		// 	break;
		// case DATATOHD:
		// 	fprintf(listing, "call");
		// 	break;
		// case HDTODATA:
		// 	fprintf(listing, "call");
		// 	break;
		case NEXTPROCESS:
			fprintf(listing, "call");
			break;
		case REGTOMEM:
			fprintf(listing, "call");
			break;
		case MEMTOREG:
			fprintf(listing, "call");
			break;
		case HDMI:
			fprintf(listing, "call");
			break;
		case CUPR:
			fprintf(listing, "call");
			break;
		case CHANGECONTEXT:
			fprintf(listing, "call");
			break;
		case HALT:
			fprintf(listing, "halt");
			break;
		default: break;
	}
}

void print_operand(Operand * op)
{
	if (op == NULL)
	{
		fprintf(listing, ", _ ");
		return;
	}

	switch (op->kind) {
		case String:
			fprintf(listing, ", %s ", op->contents.variable.name );
			break;
		case Temp:
			fprintf(listing, ", t%d ", op->contents.val );
			break;
		case Label:
			fprintf(listing, ", L%d ", op->contents.val );
			break;
		case IntConst:
			fprintf(listing, ", %d ", op->contents.val );
			break;
		default: break;
	}
}

void print_code_gen(AddressQuad CodeAddrQ)
{
	int linecount = 0;
	Operand * op;

	if (CodeAddrQ == NULL) return;

	Quadruple quad = CodeAddrQ->first;

	while(quad != NULL)
	{
		linecount++;
		fprintf(listing, "%d -> ( ", linecount);
		print_instruction(quad->instruction);
		op = quad->op1;
		print_operand(op);
		op = quad->op2;
		print_operand(op);
		op = quad->op3;
		print_operand(op);
		fprintf(listing, ")\n");

		quad = quad->next;
	}
	return;
}

void code_gen(TreeNode *tree)
{

	if (tree == NULL) return;

	switch(tree->nodekind)
	{
			case StmtK:
					genStmtK(CodeAddrQ, tree);
					break;

			case ExpK:
					genExp(CodeAddrQ, tree);
					break;

			default:
					break;

	}
	code_gen(tree->sibling);

}

void codeGen(TreeNode *syntaxTree, FILE * code, int prog)
{
	// if(is_os > 0)
	// 	IS_OS = 1;
	// else
	// 	IS_OS = 0;

	CodeAddrQ = create_quad();
	code_gen(syntaxTree);
	insert_quad(CodeAddrQ, HALT, NULL, NULL, NULL, LAST);
	print_code_gen(CodeAddrQ);
}
