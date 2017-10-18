#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SATISFIABLE 1
#define UNSATISFIABLE -1
#define UNCERTAIN 0

int DEBUG = 0; // set to 1 for debugging prints
int clauseNumber, variableNumber;
int * valuation; // global valuation array for ease of access during recursion

struct Literal {
  struct Literal * next; // points to the next literal in the clause
  int index;
};

struct Clause {
  struct Literal * head; // points to the first literal in the clause
  struct Clause * next; // points to the next clause in the set
};

// creates, initializes and returns an empty Clause
struct Clause * createClause(){
  struct Clause * instance = malloc(sizeof(struct Clause));
  instance->head = NULL;
  instance->next = NULL;
  return instance;
}

// creates, initializes and returns an empty Literal
struct Literal * createLiteral(){
  struct Literal * instance = malloc(sizeof(struct Literal));
  instance->next = NULL;
  instance->index = 0;
  return instance;
}

// prints the current state of the valuation array
void printValuation(){
  int i;
  for (i = 1; i < variableNumber + 1; i++) {
    printf("%d ", valuation[i]);
  }
  printf("\n");
}

void printClauseSet(struct Clause * root){
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
}

// finds a unit clause and return its literal index for unit-propagation step
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
  // no unit clause found, return 0
  return 0;
}

// signal function
int sign(int num){
  return (num > 0) - (num < 0);
}

// finds a pure literal by iterating through all clauses
int findPureLiteral(struct Clause * root){
  // create a lookup table to keep track of literal pureness
  int * literalLookup = (int*) calloc(variableNumber + 1, sizeof(int));
  struct Clause * itr = root;
  while (itr != NULL){
    struct Literal * l = itr->head;
    while (l != NULL){
      int seen = literalLookup[abs(l->index)];
      if (seen == 0) literalLookup[abs(l->index)] = sign(l->index);
      else if (seen == -1 && sign(l->index) == 1) literalLookup[abs(l->index)] = 2;
      else if (seen == 1 && sign(l->index) == -1) literalLookup[abs(l->index)] = 2;
      l = l->next;
    }
    itr = itr->next;
  }

  // iterate over the lookup table to send the first pure literal found
  int i;
  for (i = 1; i < variableNumber + 1; i++) {
    if (literalLookup[i] == -1 || literalLookup[i] == 1) return i * literalLookup[i];
  }
  // no pure literal found, return 0
  return 0;
}

// implements unit propagation algorithm
// returns 0 if it's unable to perform the algorithm in case there are no unit literals
int unitPropagation(struct Clause * root){
  int unitLiteralIndex = findUnitClause(root);
  if (DEBUG) printf("unit clause found with literal: %d\n", unitLiteralIndex);
  if (unitLiteralIndex == 0) return 0;

  // set the valuation for that literal
  if (DEBUG) printf("Setting value of literal %d as %d\n", abs(unitLiteralIndex), unitLiteralIndex > 0 ? 1 : 0);
  valuation[abs(unitLiteralIndex)] = unitLiteralIndex > 0 ? 1 : 0;

  // iterate over the clause set to
  // 1 - remove clauses containing the unit literal
  // 2 - remove the negated version of the unit literal when it's present in a clause
  struct Clause * itr = root;
  struct Clause * prev;
  while (itr != NULL){
    struct Literal * currentL = itr->head;
    struct Literal * previousL = createLiteral();
    while (currentL != NULL){
      if (currentL->index == unitLiteralIndex) {
        // unit literal found, remove whole clause and re-adjust pointers
        if (DEBUG) printf("Removing the clause that starts with %d\n", itr->head->index);
        if (itr == root){
          // the root has to change if we are removing the first clause
          *root = *(root->next);
          itr = NULL;
        } else {
          prev->next = itr->next;
          itr = prev;
        }
        break;
      } else if (currentL->index == -unitLiteralIndex) {
        // negated unit literal found, remove it from the clause. Other literals should stay
        if (DEBUG) printf("Removing the literal %d from the clause that starts with %d\n", currentL->index, itr->head->index);
        // if it's the first literal in the clause, the head pointer has to change
        if (currentL == itr->head) itr->head = currentL->next;
        else {
          previousL->next = currentL->next;
        }
        currentL = previousL;
        continue;
      }
      // update loop variables for easier access in following iterations
      previousL = currentL;
      currentL = currentL->next;
    }
    // update loop variables for easier access in following iterations
    // if the root clause removed, than we should still be in the (new) root
    prev = itr;
    itr = itr == NULL ? root : itr->next;
  }
  return 1;
}

// implements pure literal elimination algorithm
// returns 0 if it's unable to perform the algorithm in case there are no pure literals
int pureLiteralElimination(struct Clause * root){
  int pureLiteralIndex = findPureLiteral(root);
  if (DEBUG) printf("pure literal found: %d\n", pureLiteralIndex);
  if (pureLiteralIndex == 0) return 0;

  // set the valuation for that literal
  if (DEBUG) printf("Setting value of literal %d as %d\n", abs(pureLiteralIndex), pureLiteralIndex > 0 ? 1 : 0);
  valuation[abs(pureLiteralIndex)] = pureLiteralIndex > 0 ? 1 : 0;

  // iterate over the clause set to
  // remove clauses containing the pure literal
  struct Clause * itr = root;
  struct Clause * prev;
  while (itr != NULL){
    struct Literal * l = itr->head;
    while (l != NULL){
      if (l->index == pureLiteralIndex) {
        // unit literal found, remove whole clause and re-adjust pointers
        if (DEBUG) printf("Removing the clause that starts with %d\n", itr->head->index);
        if (itr == root){
          // the root has to change if we are removing the first clause
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
    // update loop variables for easier access in following iterations
    // if the root clause removed, than we should still be in the (new) root
    prev = itr;
    itr = itr == NULL ? root : itr->next;
  }
  return 1;
}

// reads the clause set from the given file and constructs a linked list of linked lists
struct Clause * readClauseSet(char * filename){
  FILE * fp;
  char line[256];
  size_t len = 0;

  fp = fopen(filename, "r");
  if (fp == NULL) exit(1);

  // define loop variables
  char * token;
  struct Clause * root = NULL, * currentClause = NULL, * previousClause = NULL;
  struct Literal * currentLiteral = NULL, * previousLiteral = NULL;

  while(fgets(line, sizeof(line), fp)){
    // ignore comment lines
    if (line[0] == 'c') continue;
    // this line is metadata information
    if (line[0] == 'p') {
      sscanf(line, "p cnf %d %d", &variableNumber, &clauseNumber);
      if (DEBUG) printf("Number of variables: %d\n", variableNumber);
      if (DEBUG) printf("Number of clauses: %d\n", clauseNumber);
      valuation = (int*) calloc(variableNumber + 1, sizeof(int));
      int i;
      for (i = 0; i < variableNumber + 1; i++) valuation[i] = -1;
    } else {
      // create a clause for each line
      currentClause = createClause();
      if (root == NULL) {
        if (DEBUG) printf("setting root\n");
        root = currentClause;
      }
      if (previousClause != NULL) {
        if (DEBUG) printf("setting current as the next of previous clause\n");
        previousClause->next = currentClause;
      }

      // split the line by the space characted and parse integers as literals
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

// checks if all the remaining clauses contain non-conflicting literals
// i.e. for each literal remaining in the clause set, either positive or
// negative index should be present. If that's the case, satisfiability is solved.
int areAllClausesUnit(struct Clause * root){
  int * literalLookup = (int*) calloc(variableNumber + 1, sizeof(int));

  struct Clause* itr = root;
  while (itr != NULL){
    struct Literal * l = itr->head;
    while (l != NULL){
      int seen = literalLookup[abs(l->index)];
      if (seen == 0) literalLookup[abs(l->index)] = sign(l->index);
      // if we previously have seen this literal with the opposite sign, return false
      else if (seen == -1 && sign(l->index) == 1) return 0;
      else if (seen == 1 && sign(l->index) == -1) return 0;
      l = l->next;
    }
    itr = itr->next;
  }

  // if we reached here, that means the clause set contains no conflicting literals
  // iterate over the clause set one last time to decide their valuation
  itr = root;
  while (itr != NULL){
    struct Literal * l = itr->head;
    while (l != NULL){
      valuation[abs(l->index)] = l->index > 0 ? 1 : 0;
      l = l->next;
    }
    itr = itr->next;
  }

  // return true to terminate dpll
  return 1;
}

// returns if the clause set contains and empty clause with no literal within
int containsEmptyClause(struct Clause * root){
  struct Clause* itr = root;
  while (itr != NULL){
    // if the head pointer is null, no literals
    if(itr->head == NULL) return 1;
    itr = itr->next;
  }
  return 0;
}

// checks if the current state of the clause set represents a solution
int checkSolution(struct Clause * root){
  if (containsEmptyClause(root)) return UNSATISFIABLE;
  if (areAllClausesUnit(root)) return SATISFIABLE;
  return UNCERTAIN;
}

// returns a random literal index to perform branching
int chooseLiteral(struct Clause * root){
  // just return the first literal, it doesn't change the outcome
  // but it maybe better to use a smarter approach for speed
  // (e.g. choose the literal with most frequency)
  return root->head->index;
}

// deep clones a clause constructing a new clause and literal structs
struct Clause * cloneClause(struct Clause * origin){
  struct Clause * cloneClause = createClause();
  struct Literal * iteratorLiteral = origin->head;
  struct Literal * previousLiteral = NULL;

  // iterate over the clause to clone literals as well
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

// deep clones a clause set and injects a new unit clause with the given literal index
// this is how branching is performed
struct Clause * branch(struct Clause * root, int literalIndex){
  if (DEBUG) printf("Branching with literal %d\n", literalIndex);
  if (DEBUG) printf("Setting value of literal %d as %d\n", abs(literalIndex), literalIndex > 0 ? 1 : 0);

  // set the valuation of the literal
  // we may backtrack and this valuation may become obsolete, but it doesn't matter
  // since the backtracked branch will overwrite this with the new valuation
  valuation[abs(literalIndex)] = literalIndex > 0 ? 1 : 0;

  struct Clause * newClone = NULL,
                * currentClause = NULL,
                * previousClause = NULL,
                * iterator = root;
  // deep clone each clause one by one
  while (iterator != NULL){
    struct Clause * clone = cloneClause(iterator);
    if (newClone == NULL) {
      newClone = clone;
    }
    if (previousClause != NULL) {
      previousClause->next = clone;
    }
    previousClause = clone;
    iterator = iterator->next;
  }
  // create a new unit clause with the given literalIndex
  // add it to the first place as the new root, because we want to make sure
  // that the same literalIndex will be chosen in the following immediate unit-propagation
  struct Clause * addedClause = createClause();
  struct Literal * addedLiteral = createLiteral();
  addedLiteral->index = literalIndex;
  addedClause->head = addedLiteral;

  addedClause->next = newClone;
  return addedClause;
}

void removeLiteral(struct Literal * literal){
  while (literal != NULL) {
    struct Literal * next = literal->next;
    free(literal);
    literal = next;
  }
}

void removeClause(struct Clause * root){
  while (root != NULL) {
    struct Clause * next = root->next;
    if (root->head != NULL) removeLiteral(root->head);
    free(root);
    root = next;
  }
}

// DPLL algorithm with recursive backtracking
int dpll(struct Clause * root){
  // first check if we are already in a solved state
  int solution = checkSolution(root);
  if (solution != UNCERTAIN){
    removeClause(root);
    return solution;
  }

  // do unit-propagation as long as the clause set allows
  while(1){
    solution = checkSolution(root);
    if (solution != UNCERTAIN){
      removeClause(root);
      return solution;
    }
    if (!unitPropagation(root)) break;
  }

  // then do pure-literal-elimination as long as the clause set allows
  while(1){
    int solution = checkSolution(root);
    if (solution != UNCERTAIN) {
      removeClause(root);
      return solution;
    }
    if (!pureLiteralElimination(root)) break;
  }

  // if we are stuck, then choose a random literal and branch on it
  int literalIndex = chooseLiteral(root);
  if (DEBUG) printf("Branching on literal %d\n", literalIndex);

  //   - insert a new unit clause with this chosen literal, and recurse
  if (dpll(branch(root, literalIndex)) == SATISFIABLE) return SATISFIABLE;

  //   - if it doesn't yield a solution, try the same with the negated literal
  return dpll(branch(root, -literalIndex));
}

// writes the solution to the given file
void writeSolution(struct Clause * root, char * filename){
  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    printf("Error opening file!\n");
    exit(1);
  }

  // iterate over valuation array to print the values of each literal
  int i;
  for (i = 1; i < variableNumber + 1; i++) {
    fprintf(f, "%d %d\n", i, valuation[i]);
  }

  fclose(f);
}

int main(int argc, char *argv[]){
  if (argc < 3) {
    printf("usage: ./dpll [problemX.cnf] [solutionX.sol]\n");
    return 1;
  }

  struct Clause * root = readClauseSet(argv[1]);

  if (dpll(root) == SATISFIABLE) {
    printf("SATISFIABLE\n");
    writeSolution(root, argv[2]);
  } else {
    printf("UNSATISFIABLE\n");
  }
  removeClause(root);
  return 0;
}
