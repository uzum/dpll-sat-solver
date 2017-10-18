//  Byron Boots (based on Stephen Majercik's code)

//  Compile with:  gcc verify.c -o verify

// Includes
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

//  Defines
#define LPC  100     		// Max literals per clause. 
#define VARS 10000	        // Max variables (minus 1). 
#define EC   500000	        // Max Total clauses. 
#define MAX_LINE_CHARS 500      //

// Command line arguments 
char *SATfilename;     
char *SOLUTIONfilename;

// Other global variables 
int vars = 0;		       // number of variables. 
int clauses = 0;             // number of clauses. 
int form[EC][LPC];	       // the formula... which vars in which clauses. 
int sgn[EC][LPC];	       // sign of variable (0 = negated, 1 = not negated)
int lic[EC];		       // number of literals in clause
int assgn[VARS];	       // holds current truth assignment

// functions
void read_formula(FILE *SATfile);
void read_solution(FILE *SOLUTIONfile);
bool check_solution();



// main function
//
int main(int argc, char *argv[]) {

  // make sure all arguments are present
  if (argc != 3) {
    fprintf(stderr, "verify SATfile SOLUTIONfile\n");
    exit(-1);
  }

  // process command-line arguments
  SATfilename = argv[1];
  FILE *SATfile;
  if ((SATfile = fopen(SATfilename, "r")) == NULL) {
    fprintf(stderr, "error opening \'%s\'\n", SATfilename);
    exit(-1);
  }
  
  SOLUTIONfilename = argv[2];
  FILE *SOLUTIONfile;
  if ((SOLUTIONfile = fopen(SOLUTIONfilename, "r")) == NULL) {
    fprintf(stderr, "error opening \'%s\'\n", SOLUTIONfilename);
    exit(-1);
  }
  
  // read the formula and solution files
  read_formula(SATfile);
  read_solution(SOLUTIONfile);

  // check answer
  printf("\n");
  if (check_solution())
    printf("Assignment is satisfying!\n");

  printf("\n");
return 0;
}




void read_formula(FILE *infile) {

  // read a line at a time
  char line_string[MAX_LINE_CHARS+1];
  char *pos_line_string;

  fgets(line_string, MAX_LINE_CHARS, infile);
  while (line_string[0] == 'c') 
    fgets(line_string, MAX_LINE_CHARS, infile);

  pos_line_string = line_string;
  // go past the 'p'
  pos_line_string = strchr(pos_line_string, ' ') + 1;
  // go past the 'cnf'
  pos_line_string = strchr(pos_line_string, ' ') + 1;
  // get the number of variables and clauses
  sscanf(pos_line_string, "%d %d", &vars, &clauses);

  // error if too many variables in SAT formula
  if (vars >= VARS) {
    fprintf(stderr, "Variable %d does not fit within ", vars);
    fprintf(stderr, "the limit of %d vars.\n", VARS-1);
    exit(-1);
  }

  // error if too many clauses in SAT formula
  if (clauses >= EC) {
    fprintf(stderr, "Too many clauses, should be at most %d\n", EC);
    exit(-1);
  }

  int lit;    	     // current literal read
  int litnum;	     // number of literals in clause 
  int c;
  for (c = 0 ; c < clauses ; c++) {

    fgets(line_string, MAX_LINE_CHARS, infile);
    pos_line_string = line_string;
    sscanf(pos_line_string, "%d", &lit);

    // start reading literals in clause
    litnum = 0;
    do {

      if (lit < 0) {   // literal is negated
	form[c][litnum] = -lit;
	sgn[c][litnum] = 0;
      }
      else {           // literal is not negated
	form[c][litnum] = lit;
	sgn[c][litnum] = 1;
      }
      
      // keep track of number of literals in clause
      litnum++;

      // error if too many literals in clause
      if (litnum >= LPC) {
	fprintf(stderr, "Two many literals in clause %d, ", c);
	fprintf(stderr, "should be at most %d\n", LPC);
	exit(-1);
      }

      pos_line_string = strchr(pos_line_string, ' ') + 1;
      sscanf(pos_line_string, "%d", &lit);

    } while (lit != 0);

    // store number of literals in clause
    lic[c] = litnum;

  }
  
}




void read_solution(FILE *infile) {

  char line_string[MAX_LINE_CHARS+1];

  fgets(line_string, MAX_LINE_CHARS, infile);
  while (line_string[0] == 'c') 
    fgets(line_string, MAX_LINE_CHARS, infile);
  int v;
  for (v = 1 ; v <= vars ; v++) {
    int var, value;
    sscanf(line_string, "%d  %d", &var, &value);
    assgn[var] = value;

    fgets(line_string, MAX_LINE_CHARS, infile);
  }

}

bool check_solution() {

  bool retval = true;
  int c;
  for (c = 0; c < clauses; c++) {
    int satlits = 0;
	int l;
    for (l = 0; l < lic[c]; l++) {
      if (assgn[form[c][l]] == sgn[c][l]) {
	++satlits;
	break;
      }
    }
    
    if (satlits == 0) {
      printf("Clause %d  (", c+1);
      if (sgn[c][0] == 0)
	printf("%d", -form[c][0]);
      else
	printf("%d", form[c][0]);
	 int l;
      for (l = 1; l < lic[c]; l++) {
	if (sgn[c][l] == 0)
	  printf(" %d", -form[c][l]);
	else
	  printf(" %d", form[c][l]);
      }
      printf(")  is not satisfied!\n");
      retval = false;
    }
  }

  return retval;
}

