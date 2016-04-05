#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_LEN 256

typedef enum {
	time,
	date
} TYPE;

typedef struct{
	TYPE conversionModule;
	char filename[MAX_LEN];
	int hexValue;
} ARGS;


int parseArgs(int, char**, ARGS*);
void initArgs(ARGS*);
void printArgs(ARGS);

int main(int argc, char **argv){
	static ARGS args;
	initArgs(&args);
	parseArgs(argc, argv, &args);
	return 1;
}	

int parseArgs(int argc, char **argv, ARGS *args){
	int c;
	opterr = 0;
	while((c=getopt(argc,argv,"pTDf:h:"))!=-1){
		switch(c){
			case 'T':
				if(args->conversionModule == -1){
					args->conversionModule = time;
				} else {
					printf("Redundant parameter %c\n",c);
				}
				break;
			case 'D':
				if(args->conversionModule == -1){
					args->conversionModule = date;
				} else {
					printf("Redundant parameter %c\n",c);
				}
				break;
			case 'f':
				strcpy(args->filename,optarg);
				break;
			case 'h':
				args->hexValue = (int)strtol(optarg,NULL,0);
				break;
			case 'p':
				printArgs(args);
			case '?':
				//invalid
			default:
				abort;
		}
	}
	return 1;
}

void initArgs(ARGS *args){
	args->conversionModule = -1;
	strcpy(args->filename,"");
	args->hexValue = -1;
}

void printArgs(ARGS args){
	printf("Args: [0]: %i\n",args.conversionModule);
	printf("Args: [1]: %s\n",args.filename);
	printf("Args: [2]: %i\n",args.hexValue);
}


