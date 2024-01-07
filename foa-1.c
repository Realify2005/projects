/* Program to perform multi-digit integer arithmetic.

   Skeleton program written by Alistair Moffat, ammoffat@unimelb.edu.au,
   August 2023, with the intention that it be modified by students
   to add functionality, as required by the assignment specification.
   All included code is (c) Copyright University of Melbourne, 2023

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>

/* All necessary #defines provided as part of the initial skeleton */

#define INTSIZE	500	/* max number of digits per integer value */
#define LINELEN	999	/* maximum length of any input line */
#define NVARS	26	/* number of different variables */

#define CH_A     'a'    /* character 'a', first variable name */

#define ERROR	(-1)	/* error return value from some functions */
#define PROMPT	"> "	/* the prompt string for interactive input */

#define PRINT	'?'	/* the print operator */
#define ASSIGN	'='	/* the assignment operator */
#define PLUS	'+'	/* the addition operator */
#define MULT	'*'	/* the multiplication operator */
#define POWR	'^'	/* the power-of operator */
#define DIVS	'/'	/* the division operator */
#define ALLOPS  "?=+*^/"

#define CH_ZERO  '0'    /* character zero */
#define CH_ONE   '1'    /* character one */
#define CH_NINE  '9'    /* character nine */

#define CH_COM   ','    /* character ',' */
#define PUT_COMMAS 3    /* interval between commas in output values */

#define INT_ZERO 0	/* integer 0 */
#define INT_ONE  1	/* integer 1 */
#define INT_TEN  10	/* integer 10 */

#define OVERFLOW 1	/*Number code for overflow*/
#define NON_OVERFLOW 0	/*Number code for non-overflow*/

typedef int longint_t[INTSIZE + 1];


/****************************************************************/

/* A "magic" additional function needing explicit declaration */
int fileno(FILE *);

/* Skeleton program function prototypes */

void print_prompt(void);
void print_tadaa();
void print_error(char *message);
int  read_line(char *line, int maxlen);
void process_line(longint_t vars[], char *line);
int  get_second_value(longint_t vars[], char *rhsarg,
	longint_t *second_value);
int  to_varnum(char ident);
void do_print(int varnum, longint_t *var);
void do_assign(longint_t *var1, longint_t *var2);
void do_plus(longint_t *var1, longint_t *var2);
void do_mult(longint_t *var1, longint_t *var2);
void do_expo(longint_t *var1, longint_t *var2);
void zero_vars(longint_t vars[]);
void parse_num(char *rhs, longint_t array);

/* Non skeleton program function prototypes */

void arr_to_int(longint_t *var1, int *num);
int int_overflow(int len);



/****************************************************************/

/* Main program controls all the action
*/
int
main(int argc, char *argv[]) {
	char line[LINELEN+1] = {0};
	longint_t vars[NVARS];

	zero_vars(vars);
	print_prompt();
	while (read_line(line, LINELEN)) {
		if (strlen(line) > 0) {
			/* non empty line, so process it */
			process_line(vars, line);
		}
		print_prompt();
	}

	print_tadaa();
	return 0;
}

/****************************************************************/

/* Prints the prompt indicating ready for input, but only if it
   can be confirmed that the input is coming from a terminal.
   Plus, output might be going to a file, that's why the prompt,
   if required, is written to stderr and not stdout
*/
void
print_prompt(void) {
	if (isatty(fileno(stdin))) {
		fprintf(stderr, "> ");
		fflush(stderr);
	}
}

void
print_tadaa() {
	/* all done, so pack up bat and ball and head home,
	   getting the exact final lines right is a bit tedious,
	   because input might be coming from a file and output
	   might be going to a file */
	if (isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		printf("\n");
	}
	printf("ta daa!!!\n");
	if (isatty(fileno(stdin)) && !isatty(fileno(stdout))) {
		fprintf(stderr, "\n");
	}
}

void
print_error(char *message) {
	/* need to write an error message to the right place(s)
	*/
	if (isatty(fileno(stdin)) || isatty(fileno(stdout))) {
		fprintf(stderr, "%s\n", message);
		fflush(stderr);
	}
	if (!isatty(fileno(stdout))) {
		printf("%s\n", message);
	}
}

/****************************************************************/

/* Reads a line of input into the array passed as argument,
   returns false if there is no input available.
   All whitespace characters are removed on the way through.
*/
int
read_line(char *line, int maxlen) {
	int i=0, c;
	while (((c=getchar())!=EOF) && (c!='\n')) {
		if (i<maxlen && !isspace(c)) {
			line[i++] = c;
		}
	}
	line[i] = '\0';
	/* then, if the input is coming from a file or the output
	   is going to a file, it is helpful to echo the input line
	   and record what the command was */
	if (!isatty(fileno(stdin)) || !isatty(fileno(stdout))) {
		printf("%s%s\n", PROMPT, line);
	}
	return ((i>0) || (c!=EOF));
}

/****************************************************************/

/* Process a command by parsing the input line into parts
*/
void
process_line(longint_t vars[], char *line) {
	int varnum, optype, status;
	longint_t second_value;

	/* determine the LHS variable, it
	   must be first character in compacted line
	*/
	varnum = to_varnum(line[0]);
	if (varnum==ERROR) {
		print_error("invalid LHS variable");
		return;
	}

	/* more testing for validity 
	*/
	if (strlen(line)<2) {
		print_error("no operator supplied");
		return;
	}

	/* determine the operation to be performed, it
	   must be second character of compacted line
	*/
	optype = line[1];
	if (strchr(ALLOPS, optype) == NULL) {
		print_error("unknown operator\n");
		return;
	}

	/* determine the RHS argument (if one is required),
	   it must start in the third character of compacted line
	*/
	if (optype != PRINT) {
		if (strlen(line)<3) {
			print_error("no RHS supplied");
			return;
		}
		status = get_second_value(vars, line+2, &second_value);
		if (status==ERROR) {
			print_error("RHS argument is invalid");
			return;
		}
	}

	/* finally, do the actual operation
	*/
	if (optype == PRINT) {
		do_print(varnum, vars+varnum);
	} else if (optype == ASSIGN) {
		do_assign(vars+varnum, &second_value);
	} else if (optype == PLUS) {
		do_plus(vars+varnum, &second_value);
	} else if (optype == MULT) {
		do_mult(vars+varnum, &second_value);
	} else if (optype == POWR) {
		do_expo(vars+varnum, &second_value);
	} else {
		print_error("operation not available yet");
		return;
	}
	return;
}

/****************************************************************/

/* Convert a character variable identifier to a variable number
*/
int
to_varnum(char ident) {
	int varnum;
	varnum = ident - CH_A;
	if (0<=varnum && varnum<NVARS) {
		return varnum;
	} else {
		return ERROR;
	}
}

/****************************************************************/

/* Process the input line to extract the RHS argument, which
   should start at the pointer that is passed
*/
int
get_second_value(longint_t vars[], char *rhsarg,
			longint_t *second_value) {
	char *p;
	int varnum2;
	if (isdigit(*rhsarg)) {
		/* first character is a digit, so RHS is a number
		   now check the rest of RHS for validity */
		for (p=rhsarg+1; *p; p++) {
			if (!isdigit(*p)) {
				/* nope, found an illegal character */
				return ERROR;
			}
		}
		/* nothing wrong, ok to convert */
		parse_num(rhsarg, *second_value);
		return !ERROR;
	} else {
		/* argument is not a number, so should be a variable */
		varnum2 = to_varnum(*rhsarg);
		if (varnum2==ERROR || strlen(rhsarg)!=1) {
			/* nope, not a variable either */
			return ERROR;
		}
		/* and finally, get that variable's value */
		do_assign(second_value, vars+varnum2);
		return !ERROR;
	}
	return ERROR;
}

/* Set the vars array to all zero values
*/
void
zero_vars(longint_t vars[]) {
	int i;
	longint_t zero = {0};
	for (i=0; i<NVARS; i++) {
		do_assign(vars+i, &zero);
		/* Set the length of all arrays to 1, since it contains 0 */
		(*(vars+i))[0] = INT_ONE;
	}
	return;
}

/*****************************************************************
******************************************************************

Your answer to the assignment should start here, using your new
typedef defined at the top of the program. The next few functions
will require modifications because of the change of structure
used for a long_int, and then you'll need to start adding whole
new functions after you get these first ones working properly.
Try and make the new functions fit the style and naming pattern
of the existing ones, ok?

******************************************************************
*****************************************************************/

/* Taking an integer as array of digits as input, and modifying it 
into an array consisting of the length of the integer at the first
index, continued by the "reversed" version of the input integer
*/
void
parse_num(char *rhs, longint_t array) {
	int len = strlen(rhs);
	int leading_zero = INT_ZERO;

	/* Initialise all arrays to 0, to avoid possible array corruption
	   by rogue array index */
	for (int i = 0; i <= INTSIZE + 1; i++) {
		array[i] = INT_ZERO;
	}

	/* Check for integer overflow */
	if (int_overflow(len) == OVERFLOW) {
	    exit(EXIT_FAILURE);
	}

	/* Check and eliminate leading zeros for integers having more than 1 digit*/
	if (strlen(rhs) > 1) {
        for (int i = 0; i < len; i++) {
			if (rhs[i] == CH_ZERO) {
				leading_zero++;
			}
			else {
				break;
			}
		}
		if (leading_zero == len) {
			/* Array of all 0 digits, so "leave out" one zero */
			leading_zero--;
		}
	}

	len = len - leading_zero;
	array[0] = len;
	for (int i = 0; i < len; i++) {
		/* Convert str to int */
		array[len - i] = rhs[i + leading_zero] - CH_ZERO;
	}
}

/****************************************************************/

/* Print out a longint value
*/
void
do_print(int varnum, longint_t *var) {
	printf("register %c: ", varnum+CH_A);
	int len = (*var)[0];
	/* Check for integer overflow */
	if (int_overflow(len) == OVERFLOW) {
	    exit(EXIT_FAILURE);
	}
	for (int i = len; i > 0; i--) {
		if ((i != len) && ((i % PUT_COMMAS) == 0)) {
			printf("%c", CH_COM);
		}
		printf("%d", (*var)[i]);
	}
	printf("\n");
}

/****************************************************************/

/* Assign a longint value, could do this with just an assignment
   statement, because structs can be assigned, but this is more
   elegant, and only copies over the array elements (digits) that
   are currently in use: var1 = var2

*/
void
do_assign(longint_t *var1, longint_t *var2) {
	int len = (*var2)[0];
	/* Check for integer overflow */
	if (int_overflow(len) == OVERFLOW) {
		    exit(EXIT_FAILURE);
	}
	for (int i = 0; i < INTSIZE + 1; i++) {
		(*var1)[i] = (*var2)[i];
	}
}

/****************************************************************/

/* Update the indicated variable var1 by doing an addition
   using var2 to compute var1 = var1 + var2
*/
void
do_plus(longint_t *var1, longint_t *var2) {
	int len;
	int len_var1 = (*var1)[0];
	int len_var2 = (*var2)[0];

	/* Predict the length of output integer */
	if (len_var1 > len_var2) {
		len = len_var1;
	}
	else if (len_var1 < len_var2) {
		len = len_var2;
	}
	else {
		len = len_var1;
	}

	/* Check for integer overflow */
	if (int_overflow(len) == OVERFLOW) {
		exit(EXIT_FAILURE);
	}


	for (int i = 1; i < len + 1; i++) {
	    (*var1)[i] = (*var1)[i] + (*var2)[i];
		/* Ensure that no digits are greater than 9
		If a digit is greater than 9, then it needs to
		be properly "carried over" to the next digit */
		if ((*var1)[i] >= INT_TEN) {
			if ((i == len) && ((*var1)[i] >= 10)) {
				len++;
			}
			(*var1)[i] -= INT_TEN;
			(*var1)[i + 1]++;
		}
	}
	
	(*var1)[0] = len;
}

/*****************************************************************
******************************************************************

Put your new functions below this line. Make sure you add suitable
prototypes at the top of the program.

******************************************************************
*****************************************************************/

/* Update the indicated variable var1 by doing a multiplication
   using var2 to compute var1 = var1 * var2
*/

void do_mult(longint_t *var1, longint_t *var2) {
	longint_t ans = {INT_ZERO};
	int len_var1 = (*var1)[0];
	int len_var2 = (*var2)[0];
	int len;

	/* Stage 1 of predicting the resulting integer's final length */
	if ((*var1)[len_var1] * (*var2)[len_var2] >= 10) {
		len = len_var1 + len_var2;
	}
	else if ((((*var1)[0] == 1) && ((*var1)[1] == 0)) 
	|| (((*var2)[0] == 1) && ((*var2)[1] == 0))) {
        /* Either one of the integer is 0, so product must be 0*/
		len = 1;
	}
	else {
		len = len_var1 + len_var2 - 1;
	}
    
	ans[0] = len;

	/* Check for integer overflow */
	if (int_overflow(len) == OVERFLOW) {
		exit(EXIT_FAILURE);
	}

	/* Long multiplication starts here */
	int curr, first_digit, second_digit;
	for (int i = len_var1; i > 0; i--) {
		for (int j = len_var2; j > 0; j--) {
	        curr = (*var1)[i] * (*var2)[j];
			/* Single digit multiplication results in 2 digits, so
			separate and put them into their proper array index slots */
			if (curr >= 10) {
				second_digit = curr % INT_TEN;
				first_digit = curr / INT_TEN;
			}
			else {		
			    second_digit = curr;
				first_digit = 0;
			}
			ans[i + j] += first_digit;
			ans[i + j - 1] += second_digit;
		}
	}

	/* Make sure that ALL integer in a single array which are greater
	than 10 are carried over to the next array index slot */
	for (int i = 1; i <= len; i++) {
		while (ans[i] >= INT_TEN) {
			ans[i] -= INT_TEN;
			ans[i + 1]++;
			/* First digit is greater or equal to 10, so fix integer's
			final length */
			if (i == len) {
				ans[0]++;
			}
		}
	}

	/* When done, make sure all is copied to original array */
	do_assign(var1, &ans);
}

/****************************************************************/

/* Update the indicated variable var1 by doing exponential
   using var2 to compute var1 = var1 ^ var2
*/

void do_expo(longint_t *var1, longint_t *var2) {
	longint_t LHS;
	int RHS	= 0;

	do_assign(&LHS, var1);
	arr_to_int(var2, &RHS);

	/* Multiply var1 by (RHS - 1) times */
	for (int i = 0; i < RHS - 1; i++) {
		do_mult(var1, &LHS);
	}
}

/****************************************************************/

/* Function to convert an array of digits into integer*/

void arr_to_int(longint_t *var1, int *num) {
	int ten_multiplier = 1;
	int len_var1 = (*var1)[0];
	for (int i = 1; i < len_var1 + 1; i++) {
		if (i > 1) {
			ten_multiplier *= INT_TEN;
		}
	    *num += (*var1)[i] * ten_multiplier;
	}
}

/****************************************************************/

/* Function to check for integer overflow, which occurs if length of array
(number of digits) exceed 500
*/

int int_overflow(int len) {
	if (len > INTSIZE) {
		print_error("integer overflow, program terminated");
		return OVERFLOW;
	}
	else {
		return NON_OVERFLOW;
	}
}

// algorithms are fun
