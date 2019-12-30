

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

typedef struct BucketListRec
   { char * name;
     LineList lines;
     int memloc ; /* memory location for variable */
     int is_vector;
     struct BucketListRec * next;
     char * scope;
     char * typeID;
     char * typedata;
   } * BucketList;

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, int is_vector, char * scope, char * typeID, char * typedata );

/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 */
BucketList st_lookup ( char * name , char * scope );

int checkFunctionType (char * name);

/* Procedure printSymTab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymTab(FILE * listing);

#endif
