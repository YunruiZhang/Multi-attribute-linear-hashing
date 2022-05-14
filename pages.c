
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bits.h"
#include "hash.h"

// Global constants

#define MAX_ATTRS 8
#define MAX_DEPTH 30
#define MAX_CVLEN (MAX_DEPTH+2)

// File parameters ... done as globals (tsk, tsk)

typedef struct { int attr; int bit; } CVitem;
typedef struct { int known; uint32 hash; } Qitem;

int    n;               // number of attributes;
int    d;               // file depth (use d or d+1 hash bits)
CVitem cv[MAX_CVLEN];   // array of choice vector items
int    ncv;             // number of items in choice vector
Qitem  q[MAX_ATTRS+1];  // parsed version of query

// Functions (forward referenced)

void usage(char *);
int  makeChoiceVector(char *); // sets "cv", "ncv"
int  parseQuery(char *);       // reads "n", sets "q"

// ===== Main =====

int main(int argc, char **argv)
{
	char line[200];  // input line buffer
	
	// process command-lines args and set things up

	if (argc < 4)
		usage("Not enough parameters");
	if (sscanf(argv[1], "%d", &n) != 1 || n > MAX_ATTRS)
		usage("Invalid number of attributes");
	if (sscanf(argv[2], "%d", &d) != 1 || d > MAX_DEPTH)
		usage("Invalid file depth");
	if (!makeChoiceVector(argv[3]))
		usage("Invalid choice vector");

	// read queries and display required pages

	printf("query> ");
	while (fgets(line,200,stdin) != NULL) {
		if (strcmp(line,"quit\n") == 0)
			break;
		if (!parseQuery(line)) {
			fprintf(stderr, "Invalid query\n");
		}
		else {
			// Hint: what your code should do ...
			// determine unknown bits from query and choice vector
			// set known bits from query hashes and choice vector
			// for each configuration of unknown bits
				// generate new page number


			// XXX your code goes here XXX
		}
		printf("query> ");
	}
	return 0;
}

// XXX your functions go here, if you need any XXX


// ===== Supplied functions =====

// makeChoiceVector()
// reads a string giving choice vector data
// parses string and sets values of "cv" and "ncv"

int makeChoiceVector(char *in)
{
	char *w;
	int attr;
	int attrbit[MAX_ATTRS] = {0,0,0,0,0,0,0,0};

	ncv = 0;
	for (w = strtok(in,","); w != NULL; w = strtok(NULL,",")) {
		if (sscanf(w,"%d",&attr) != 1)
			return 0;
		if (attr < 1 || attr > n)
				return 0;
		cv[ncv].attr = attr;
		cv[ncv].bit  = attrbit[attr]++;
		ncv++;
	}
	if (ncv < d) usage("Choice vector too short");
#if 1
	// debugging: comment out if you don't need it
	int i;
	printf("CV = < ");
	for (i = 0; i < ncv; i++)
		printf("(%d,%d) ", cv[i].attr, cv[i].bit);
	printf(">\n");
#endif
	return 1;
}

// parseQuery()
// reads a string giving query data
// parses string and set value of "q"
// checks that there are "n" items in string

int parseQuery(char *in)
{
	char *w;
	int attr;
	char buf[40]; // allows for 32 bits + spaces

	for (w = in; *w != '\n'; w++) /*skip*/;
	*w = '\0';
	attr = 1;
	for (w = strtok(in,","); w != NULL; w = strtok(NULL,",")) {
		if (w[0] == '?') {
			q[attr].known = 0;
			q[attr].hash  = 0;
		}
		else {
			q[attr].known = 1;
			q[attr].hash = hash_any((unsigned char *)w, strlen(w));
		}
		attr++;
		if (attr > n+1) // too many values
			return 0;
	}
	if (attr < n+1) // not enough values
		return 0;
#if 1
	// debugging: comment out if you don't need it
	int i;
	for (i = 1; i < attr; i++) {
		if (!q[i].known)
			printf("q[%d] = ??\n",i);
		else
			printf("q[%d] = %s\n",i,bitsString(q[i].hash,buf));
	}
#endif
	return 1;
}


void usage(char *message)
{
	if (message[0] != '\0')
		fprintf(stderr, "%s\n", message);
	fprintf(stderr, "Usage: ./pages  n  d  cv\n");
	exit(1);
}
