#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_LEN 256
#define YEAR_OFFSET 1980

typedef enum {
	time,
	date
} TYPE;

typedef struct{
	TYPE conversionModule;
	char filename[MAX_LEN];
	unsigned short hexValue;
} ARGS;

typedef struct{
	unsigned short day : 5;
	unsigned short month : 4;
	unsigned short year : 7;
} DATE_STRUCT;

typedef struct{
	unsigned short second : 5;
	unsigned short minute : 6;
	unsigned short hour : 5;
} TIME_STRUCT;

const char *months[] = {
	"",
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

int parseArgs(int, char**, ARGS*);
void initArgs(ARGS*);
void printArgs(ARGS);
void getHex(ARGS*);

int main(int argc, char **argv){
	static ARGS args;
	initArgs(&args);
	parseArgs(argc, argv, &args);
	if(args.conversionModule == -1) {
		printf("Select a conversion module\n");
		return 0;
	}
	if(args.hexValue == 0) getHex(&args);
	if((short)args.hexValue == -1 && args.filename[0] == '\0'){
		printf("Input a hex value or file containing a hex value");
	}

	//reverse bytes because endianess
	char temp = ((char*)&args.hexValue)[0];
	((char*)&args.hexValue)[0] = ((char*)&args.hexValue)[1];
	((char*)&args.hexValue)[1] = temp;
	DATE_STRUCT *dateS = &(args.hexValue);
	TIME_STRUCT *timeS = &(args.hexValue);
	if(args.conversionModule == date)
		printf("Date: %s, %i, %i\n", months[dateS->month], dateS->day, dateS->year + YEAR_OFFSET);
	if(args.conversionModule == time)
		printf("Time: %i:%i:%i %s\n", timeS->hour%12, timeS->minute, timeS->second*2, timeS->hour > 12 ? "PM" : "AM");
	return 1;
}	

void getHex(ARGS *args){
	FILE *fp = fopen(args->filename, "r");
	char temp[256];
	if(fp != NULL){
		fgets(temp, 256, fp);
		args->hexValue = strtol(temp,NULL,0);
	} else {
		printf("Invalid File");
	}
	
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
				args->hexValue = strtol(optarg,NULL,0);
				break;
			case 'p':
				printArgs(*args);
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
	args->hexValue = 0;
}

void printArgs(ARGS args){
	printf("Args: [0]: %i\n",args.conversionModule);
	printf("Args: [1]: %s\n",args.filename);
	printf("Args: [2]: %i\n",args.hexValue);
}


