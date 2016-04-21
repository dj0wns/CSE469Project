#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>

#define DEFAULT_ADDR_RETURN_TYPE -1
#define DEFAULT_ADDR_INPUT_TYPE -1
#define DEFAULT_OFFSET 0
#define DEFAULT_BYTES 512
#define DEFAULT_ADDRESS 0
#define DEFAULT_SECTORS_RESERVED 0
#define DEFAULT_SECTORS_PER_CLUSTER 0
#define DEFAULT_SECTORS_FAT_TABLE 0
#define DEFAULT_TABLES 0
#define DEFAULT_BYTE_OFFSET_FLAG 0

#define FIRST_CLUSTER 2

typedef enum {
	LOGICAL = 'l',
	PHYSICAL = 'p',
	CLUSTER = 'c' } TYPE;

typedef struct {
	TYPE addrReturnType;
	TYPE addrInputType;
	int offset;
	int bytes;
	int address;
	int sectorsReserved;
	int sectorsPerCluster;
	int sectorsFATTable;
	int tables;
	int byteOffsetFlag;
} ARGS;

typedef struct {
	int logical;
	int physical;
	int cluster;

} ADDR;

//TODO
//Use the parsed input to calculate the return values

void initArgs(ARGS*);
int parseArgs(int, char**, ARGS*);
void printArgs(int*);

void fromLogical(ARGS*, ADDR*);
void fromPhysical(ARGS*, ADDR*);
void fromCluster(ARGS*, ADDR*);

void toLogical(ADDR*);
void toPhysical(ADDR*);
void toCluster(ADDR*);

int main(int argc, char **argv){
	static ARGS args;
	static ADDR addr;
	void (*readFunc[])(ARGS*,ADDR*)  = {
		['l'] = fromLogical, 
		['p'] = fromPhysical, 
		['c'] = fromCluster
	};
	void (*printFunc[])(ADDR*) = {
		['l'] = toLogical,
		['p'] = toPhysical,
		['c'] = toCluster
	};
	initArgs(&args);
	parseArgs(argc, argv, &args);
	
	if(args.addrReturnType == DEFAULT_ADDR_RETURN_TYPE 
			|| args.addrInputType == DEFAULT_ADDR_INPUT_TYPE){
		printf("Invalid Arguments\n");
		return 0;
	}

	(*readFunc[args.addrInputType])(&args, &addr);
	(*printFunc[args.addrReturnType])(&addr);

	return 1;
}	

void fromLogical(ARGS *args, ADDR *addr){
	addr->logical = args->address;
	addr->physical = args->address+args->offset;
	addr->cluster = addr->logical - args->sectorsReserved - (args->tables * args->sectorsFATTable);
	addr->cluster = (addr->cluster/args->sectorsPerCluster) + FIRST_CLUSTER;

}
void fromPhysical(ARGS *args, ADDR *addr){
	addr->logical = args->address - args->offset;
	addr->physical = args->address;
	addr->cluster = addr->logical - args->sectorsReserved - (args->tables * args->sectorsFATTable);
	addr->cluster = (addr->cluster/args->sectorsPerCluster) + FIRST_CLUSTER;
}
void fromCluster(ARGS *args, ADDR *addr){
	addr->logical= (args->address-FIRST_CLUSTER)*args->sectorsPerCluster 
		+ (args->tables * args->sectorsFATTable) + args->sectorsReserved; 
	addr->physical = addr->logical + args->offset;
	addr->cluster = args->address;
}
void toLogical(ADDR *addr){
	printf("%i\n", addr->logical);
}
void toPhysical(ADDR *addr){
	printf("%i\n", addr->physical);

}
void toCluster(ADDR *addr){
	printf("%i\n", addr->cluster);
}

void initArgs(ARGS *args){
	args->addrReturnType = DEFAULT_ADDR_RETURN_TYPE;
	args->addrInputType = DEFAULT_ADDR_RETURN_TYPE;
	args->offset = DEFAULT_OFFSET;
	args->bytes = DEFAULT_BYTES;
	args->address = DEFAULT_ADDRESS;
	args->sectorsReserved = DEFAULT_SECTORS_RESERVED;
	args->sectorsPerCluster = DEFAULT_SECTORS_PER_CLUSTER;
	args->sectorsFATTable = DEFAULT_SECTORS_FAT_TABLE;
	args->tables = DEFAULT_TABLES;
	args->byteOffsetFlag = DEFAULT_BYTE_OFFSET_FLAG;	
}


int parseArgs(int argc, char **argv, ARGS *args){
	int c;
	while(1){
		static struct option long_options[]={
			//Flag options
			{"logical", no_argument, 0, 'L'},
			{"physical", no_argument, 0, 'P'},
			{"cluster", no_argument, 0, 'C'},
	
			//argument options
			{"partition-start", required_argument, 0, 'b'},
			{"byte-address", no_argument, 0, 'B'},
			{"sector-size", required_argument, 0, 's'},
			{"logical-known", required_argument, 0, 'l'},
			{"physical-known", required_argument, 0, 'p'},
			{"cluster-known", required_argument, 0, 'c'},
			{"cluster-size", required_argument, 0, 'k'},
			{"reserved", required_argument, 0, 'r'},
			{"fat-tables", required_argument, 0, 't'},
			{"fat-length", required_argument, 0, 'f'},
			{"print-args", no_argument, 0, 'D'}, //for debugging
			{0,0,0,0}

		};
		int option_index = 0;

		c = getopt_long(argc, argv, "DLPCb:B:s:l:p:c:k:r:t:f:", long_options, 
					&option_index);
		if(c==-1)
			break;

		switch(c){
			case 0:
				//should never happen
				break;
			case 'b':
				args->offset = atoi(optarg);
				break;
			case 'B':
				args->byteOffsetFlag = 1;		
				break;
			case 's':
				args->bytes = atoi(optarg);
				break;
				
			case 'l':
			case 'p':
			case 'c':
				if(args->addrInputType != -1){
					printf("Redundant paramater: %c\n", c);	
				} else {
					args->addrInputType = c;
				}
				args->address = atoi(optarg);
				break;
			
			case 'k':
				args->sectorsPerCluster = atoi(optarg);
				break;
			case 'r':
				args->sectorsReserved = atoi(optarg);
				break;
			case 't':
				args->tables = atoi(optarg);
				break;
			case 'f':
				args->sectorsFATTable = atoi(optarg);
				break;	
					
			case 'L':
			case 'P':
			case 'C':
				if(args->addrReturnType != -1){
					printf("Redundant parameter: %c\n", c);
				} else {
					args->addrReturnType = tolower(c);
				}
				break;
			case 'D':
				printArgs((int*)args);
			case '?':
				break;
			default:
				abort();


		}
	}
	return 1;
}

void printArgs(int *args){
	int i;
	for(i = 0; i < sizeof(ARGS)/sizeof(int); i++){
		printf("Args: [%d]: %d\n", i, args[i]);
	}
	
}


