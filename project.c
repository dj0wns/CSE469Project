#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>

typedef enum {
	LOGICAL = 'L',
	PHYSICAL = 'P',
	CLUSTER = 'C' } TYPE;

//TODO
//Add cases for all arguments
//Define magic numbers as constants
//Set up macros for flags
//Move argument parsing out of main


int main(int argc, char **argv){

	static TYPE addrType = -1;
	int flags = 0;
	int c;
	
	int offset = 0, bytes = 512, address = 0, sectors = 0, tables = 0;
	//parse args
	while(1){
		static struct option long_options[]={
			//Flag options
			{"logical", no_argument, 0, 'L'},
			{"physical", no_argument, 0, 'P'},
			{"cluster", no_argument, 0, 'C'},
	
			//argument options
			{"offset", required_argument, 0, 'o'},
			{"byte-address", no_argument, 0, 'B'},
			{"sector-size", required_argument, 0, 's'},
			{"logical-known", required_argument, 0, 'l'},
			{"physical-known", required_argument, 0, 'p'},
			{"cluster-known", required_argument, 0, 'c'},
			{"cluster-size", required_argument, 0, 'k'},
			{"reserved", required_argument, 0, 'r'},
			{"fat-tables", required_argument, 0, 't'},
			{"fat-length", required_argument, 0, 'f'},
			{0,0,0,0}

		};
		int option_index = 0;

		c = getopt_long(argc, argv, "LPCo:B:s:l:p:c:k:r:t:f:", long_options, &option_index);
		if(c==-1)
			break;

		switch(c){
			/*this occurs when a flag is set*/
			case 0:
				if(long_options[option_index].flag != 0)
					break;
				printf("option %s", long_options[option_index].name);
				if(optarg)
					printf(" with arg %s", optarg);
				printf("\n");
				break;

			case 'o':
				printf("option -o with value %s\n", optarg);
				break;
			case 'L':
			case 'P':
			case 'C':
				printf("option %c\n", c);
				if(addrType != -1){
					//TODO error for conflicting arguments
				} else {
					addrType = c;
				}
			case '?': 
				break;
			default:
				abort();


		}
	}
}	
