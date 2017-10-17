#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SATISFIABLE 1
#define UNSATISFIABLE -1
#define UNCERTAIN 0

int DEBUG = 1;
int clauseNumber, variableNumber;
int * valuation;

struct Literal {
  struct Literal * next;
  int index;
};

struct Clause {
  struct Literal * head;
  struct Clause * next;
};

struct Clause * createClause(){
  struct Clause * instance = malloc(sizeof(struct Clause));
  instance->head = NULL;
  instance->next = NULL;
  return instance;
}

struct Literal * createLiteral(){
  struct Literal * instance = malloc(sizeof(struct Literal));
  instance->next = NULL;
  instance->index = 0;
  return instance;
}

void printValuation(){
  int i;
  for (i = 1; i < variableNumber + 1; i++) {
    printf("%d ", valuation[i]);
  }
  printf("\n");
}

void printClauseSet(struct Clause * root){
  printf("xxxxxxxxxxxxx CLAUSE SET xxxxxxxxxxxx\n\n");
  struct Clause* itr = root;
  while (itr != NULL){
    struct Literal * l = itr->head;
    while (l != NULL){
      printf("%d ", l->index);
      l = l->next;
    }
    printf("\n");
    itr = itr->next;
  }
  printf("xxxxxxxxxxxxx CLAUSE SET xxxxxxxxxxxx\n\n");
}

int findUnitClause(struct Clause * root){
  struct Clause * itr = root;
  while (itr != NULL){
    if (itr->head == NULL) {
      if (DEBUG) printf("Empty clause\n");
      continue;
    }
    if(itr->head->next == NULL){
      return itr->head->index;
    }
    itr = itr->next;
  }
  return 0;
}

int sign(int num){
  return (num > 0) - (num < 0);
}

int findPureLiteral(struct Clause * root){
  int * literalLookup = (int*) calloc(variableNumber + 1, sizeof(int));
  struct Clause * itr = root;
  while (itr != NULL){
    struct Literal * l = itr->head;
    while (l != NULL){
      if (DEBUG) printf("inspecting literal %d\n", l->index);
      int seen = literalLookup[abs(l->index)];
      if (seen == 0) literalLookup[abs(l->index)] = sign(l->index);
      else if (seen == -1 && sign(l->index) == 1) literalLookup[abs(l->index)] = 2;
      else if (seen == 1 && sign(l->index) == -1) literalLookup[abs(l->index)] = 2;
      l = l->next;
    }
    itr = itr->next;
  }
  int i;
  for (i = 1; i < variableNumber + 1; i++) {
    printf("literal lookup @%d: %d\n", i, literalLookup[i]);
    if (literalLookup[i] == -1 || literalLookup[i] == 1) return i * literalLookup[i];
  }
  return 0;
}

int unitPropagation(struct Clause * root){
  int unitLiteralIndex = findUnitClause(root);
  if (DEBUG) printf("unit clause found with literal: %d\n", unitLiteralIndex);
  if (unitLiteralIndex == 0) return 0;

  printf("Setting value of literal %d as %d\n", abs(unitLiteralIndex), sign(unitLiteralIndex));

  valuation[abs(unitLiteralIndex)] = unitLiteralIndex > 0 ? 1 : 0;

  struct Clause * itr = root;
  struct Clause * prev;
  while (itr != NULL){
    struct Literal * currentL = itr->head;
    struct Literal * previousL = createLiteral();
    while (currentL != NULL){
      if (currentL->index == unitLiteralIndex) {
        // remove this clause
        if (DEBUG) printf("Removing the clause that starts with %d\n", itr->head->index);
        if (itr == root){
          *root = *(root->next);
          itr = NULL;
        } else {
          prev->next = itr->next;
          itr = prev;
        }
        break;
      } else if (currentL->index == -unitLiteralIndex) {
        // remove this literal
        if (DEBUG) printf("Removing the literal %d from the clause that starts with %d\n", currentL->index, itr->head->index);
        if (currentL == itr->head) itr->head = currentL->next;
        else {
          previousL->next = currentL->next;
        }
        currentL = previousL;
        continue;
      }
      previousL = currentL;
      currentL = currentL->next;
    }
    prev = itr;
    itr = itr == NULL ? root : itr->next;
  }
}

int pureLiteralElimination(struct Clause * root){
  int pureLiteralIndex = findPureLiteral(root);
  if (DEBUG) printf("pure literal found: %d\n", pureLiteralIndex);
  if (pureLiteralIndex == 0) return 0;

  printf("Setting value of literal %d as %d\n", abs(pureLiteralIndex), sign(pureLiteralIndex));

  valuation[abs(pureLiteralIndex)] = pureLiteralIndex > 0 ? 1 : 0;

  struct Clause * itr = root;
  struct Clause * prev;
  while (itr != NULL){
    struct Literal * l = itr->head;
    while (l != NULL){
      if (l->index == pureLiteralIndex) {
        // remove this clause
        if (DEBUG) printf("Removing the clause that starts with %d\n", itr->head->index);
        if (itr == root){
          *root = *(root->next);
          itr = NULL;
        } else {
          prev->next = itr->next;
          itr = prev;
        }
        break;
      }
      l = l->next;
    }
    prev = itr;
    itr = itr == NULL ? root : itr->next;
  }
}

struct Clause * readClauseSet(char * filename){
  FILE * fp;
  char line[256];
  size_t len = 0;

  fp = fopen(filename, "r");
  if (fp == NULL) exit(1);

  char * token;
  struct Clause * root = NULL, * currentClause = NULL, * previousClause = NULL;
  struct Literal * currentLiteral = NULL, * previousLiteral = NULL;
  
  while(fgets(line, sizeof(line), fp)){
    if (line[0] == 'c') continue;
    if (line[0] == 'p') {
      sscanf(line, "p cnf %d %d", &variableNumber, &clauseNumber);
      printf("Number of variables: %d\n", variableNumber);
      printf("Number of clauses: %d\n", clauseNumber);
      valuation = (int*) calloc(variableNumber + 1, sizeof(int));
      int i;
      for (i = 0; i < variableNumber + 1; i++) valuation[i] = -1;
    } else {
      currentClause = createClause();
      if (root == NULL) {
        if (DEBUG) printf("setting root\n");
        root = currentClause;
      }
      if (previousClause != NULL) {
        if (DEBUG) printf("setting current as the next of previous clause\n");
        previousClause->next = currentClause;  
      }
      
      token = strtok(line, " ");
      while(token != NULL){
        int literalIndex = atoi(token);
        currentLiteral = createLiteral();
        currentLiteral->index = literalIndex;
        if (literalIndex != 0){
          if (currentClause->head == NULL){
            if (DEBUG) printf("setting literal %d as head of current clause\n", currentLiteral->index);
            currentClause->head = currentLiteral;
          }
          
          if (previousLiteral != NULL){
            if (DEBUG) printf("setting literal %d as the next of previous literal\n", currentLiteral->index);
            previousLiteral->next = currentLiteral;
          }
        }

        if (DEBUG) printf("current literal is now previous literal\n");
        previousLiteral = currentLiteral; 
         
        token = strtok(NULL, " ");
      }
      if (DEBUG) printf("current clause is now previous clause\n");
      previousClause = currentClause;
    }
  }
  fclose(fp);
  
  return root;
}

int areAllClausesUnit(struct Clause * root){
  int * literalLookup = (int*) calloc(variableNumber + 1, sizeof(int));

  struct Clause* itr = root;
  while (itr != NULL){
    struct Literal * l = itr->head;
    while (l != NULL){
      int seen = literalLookup[abs(l->index)];
      if (seen == 0) literalLookup[abs(l->index)] = sign(l->index);
      else if (seen == -1 && sign(l->index) == 1) return 0;
      else if (seen == 1 && sign(l->index) == -1) return 0;      
      l = l->next;
    }
    itr = itr->next;
  }

  itr = root;
  while (itr != NULL){
    struct Literal * l = itr->head;
    while (l != NULL){
      if (valuation[abs(l->index)] == -1) {
        valuation[abs(l->index)] = l->index > 0 ? 1 : 0;
      }
      l = l->next;
    }
    itr = itr->next;
  }

  return 1;
}

int containsEmptyClause(struct Clause * root){
  struct Clause* itr = root;
  while (itr != NULL){
    if(itr->head == NULL) return 1;
    itr = itr->next;
  }
  return 0;
}

int checkSolution(struct Clause * root){
  if (containsEmptyClause(root)) return UNSATISFIABLE;
  if (areAllClausesUnit(root)) return SATISFIABLE;
  return UNCERTAIN;
}

int chooseLiteral(struct Clause * root){
  // todo: choose a literal that does not have a valuation yet
  return root->head->index;
}

struct Clause * cloneClause(struct Clause * origin){
  struct Clause * cloneClause = createClause();
  struct Literal * iteratorLiteral = origin->head;
  struct Literal * previousLiteral = NULL;
  while (iteratorLiteral != NULL){
    struct Literal * literalClone = createLiteral();
    literalClone->index = iteratorLiteral->index;
    if (cloneClause->head == NULL) {
      cloneClause->head = literalClone;
    }
    if (previousLiteral != NULL) {
      previousLiteral->next = literalClone;
    }
    previousLiteral = literalClone;
    iteratorLiteral = iteratorLiteral->next;
  }
  return cloneClause;
}

struct Clause * branch(struct Clause * root, int literalIndex){
  if (DEBUG) printf("Branching with literal %d\n", literalIndex);

  printf("Setting value of literal %d as %d\n", abs(literalIndex), sign(literalIndex));
  valuation[abs(literalIndex)] = literalIndex > 0 ? 1 : 0;

  struct Clause * newRoot = NULL,
                * currentClause = NULL,
                * previousClause = NULL,
                * iterator = root;
  while (iterator != NULL){
    struct Clause * clone = cloneClause(iterator);
    if (newRoot == NULL) {
      newRoot = clone;
    }
    if (previousClause != NULL) {
      previousClause->next = clone;
    }
    previousClause = clone;
    iterator = iterator->next;
  }
  struct Clause * addedClause = createClause();
  struct Literal * addedLiteral = createLiteral();
  addedLiteral->index = literalIndex;
  addedClause->head = addedLiteral;
  previousClause->next = addedClause;
  return newRoot;
}

int dpll(struct Clause * root){
  printClauseSet(root);
  if (checkSolution(root) != UNCERTAIN) return checkSolution(root);

  while(1){
    printClauseSet(root);
    if (checkSolution(root) != UNCERTAIN) return checkSolution(root);
    if (!unitPropagation(root)) break;
  }

  while(1){
    printClauseSet(root);
    if (checkSolution(root) != UNCERTAIN) return checkSolution(root);
    if (!pureLiteralElimination(root)) break;
  }

  // branch here
  int literalIndex = chooseLiteral(root);
  int try1 = dpll(branch(root, literalIndex));
  if (try1 == SATISFIABLE) return try1;
  else return dpll(branch(root, -literalIndex));
}

void writeSolution(struct Clause * root, char * filename){
  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    printf("Error opening file!\n");
    exit(1);
  }

  int i;
  for (i = 1; i < variableNumber + 1; i++) {
    fprintf(f, "%d %d\n", i, valuation[i]);
  }

  fclose(f);
}

int main(int argc, char *argv[]){
  if (argc < 2) {
    printf("Filename should be provided\n");
    return 1;
  }

  struct Clause * root = readClauseSet(argv[1]);
  int dpllResult = dpll(root);
  printf("DPLL finished\n");
  printValuation();
  if (dpllResult == SATISFIABLE) {
    writeSolution(root, argv[2]);
  } else {
    printf("UNSATISFIABLE\n");
  }
  return 0;
}
