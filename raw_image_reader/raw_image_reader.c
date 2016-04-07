#include <stdio.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>

#define DIVIDER_LEN 60
#define DIVIDER_CHAR "="

#define DELAY_LENGTH 500
#define DATA_BUF_SIZE 2048

#define MD5_TAG "MD5"
#define SHA1_TAG "SHA1"

#define MBR_OFFSET 0x1BE
#define NUM_OF_PARTITIONS 4

typedef struct{
	uint8_t state;
	uint8_t startHead;
	uint16_t startCylinderSector;
	uint8_t type;
	uint8_t endHead;
	uint16_t endCylinderSector;
	uint32_t sectorDistance;
	uint32_t numSectors;
	
} partition_entry;

typedef struct __attribute__((__packed__)){
	partition_entry entries[NUM_OF_PARTITIONS];
	uint16_t bootRecordSig;
} MBR;

const char *partition_type[256] = {
	[0x01]="DOS 12-bit FAT",
	[0x04]="DOS 16-bit FAT for partitions smaller than 32 MB",
	[0x05]="Extended Partition",
	[0x06]="DOS 16-bit FAT for partitions larger than 32 MB",
	[0x07]="NTFS",
	[0x08]="AIX Bootable Partition",
	[0x09]="AIX Data Partition",
	[0x0B]="DOS 32-bit FAT",
	[0x0C]="DOS 32-bit FAT for interrupt 13 support",
	[0x17]="Hidden NTFS Partition (XP and earlier)",
	[0x1B]="Hidden FAT32 Partition",
	[0x1E]="Hidden VFAT Partition",
	[0x3C]="Partition Magic Recovery Partition",
	[0x66]="Novell Partition",
	[0x67]="Novell Partition",
	[0x68]="Novell Partition",
	[0x69]="Novell Partition",
	[0x81]="Linux",
	[0x82]="Linux Swap Partition",
	[0x83]="Linux Native File Systems",
	[0x86]="FAT16 volume/stripe set",
	[0x87]="HPFS",
	[0xA5]="FreeBSD",
	[0xA6]="OpenBSD",
	[0xA9]="NetBSD",
	[0xC7]="Corrupted NTFS",
	[0xEB]="BeOS"
} ;


int calcMD5(int, int);
int calcSHA1(int, int);
int readMBR(int, char*);
void printPartitions(MBR *mbr);
void printDivider();
//TODO
//Print md5 and sha1 to a file
//locate and extract mbr
//read each FAT16/32 VBR

int main(int argc, char** argv){

	if(argc != 2){
		printf("Invalid Arguments. Usage: raw_image_reader <path_to_file>\n");
		return -1;
	} 
	struct stat fstats;	
	int file = open(argv[1], O_RDONLY);
	MBR mbr;
	if(file<0){
		printf("Unable to open file");
		return -1;
	}
	printDivider();
	fstat(file, &fstats);
	calcMD5(file, fstats.st_size);
	calcSHA1(file, fstats.st_size);
	readMBR(file, (char*)&mbr);
	printDivider();
	printPartitions(&mbr);
	printDivider();
	close(file);
	return 1;
}

void printDivider(){
	printf("\n");
	for(int i = 0; i < DIVIDER_LEN; i++) printf(DIVIDER_CHAR); 
	printf("\n\n");
}

void printPartitions(MBR *mbr){
	int i;
	for(i = 0; i< NUM_OF_PARTITIONS; i++){
		printf("(%02x) %s, %08u, %08u\n", mbr->entries[i].type, 
				partition_type[mbr->entries[i].type],
				mbr->entries[i].sectorDistance,
			   	mbr->entries[i].numSectors);
	}

}

void printProgress(const char* name, int written, int total){
	printf("Calculating %s: %i/%i bytes written - %2.0f%% <", 
			name, written, total, (100.*(float)written)/(float)total);
	int equals = (int)((100.*(float)written)/(float)total) / 5;
	int spaces = 19 - equals;
	
	int i;
	for(i=0; i < equals; i++) printf("=");
	for(i=0; i < spaces; i++) printf(" ");
	printf(">\r");
}

int calcSHA1(int file, int size){
	unsigned char c[SHA_DIGEST_LENGTH];
	SHA_CTX mdContext;
	int delay = 0, bytes, i, written=0;
	unsigned char data[DATA_BUF_SIZE];
	SHA1_Init(&mdContext);
	while((bytes = read(file, data, DATA_BUF_SIZE)) != 0){
		written += bytes;
		if(++delay > DELAY_LENGTH){
			printProgress(SHA1_TAG, written, size);
			delay = 0;
		}
		SHA1_Update(&mdContext, data, bytes);	
	}
	SHA1_Final(c, &mdContext);
	printf("\n%s: ", SHA1_TAG);
	for(i = 0; i < SHA_DIGEST_LENGTH; i++) printf("%02x",c[i]);
	printf("\n");
	lseek(file, 0, SEEK_SET); //return to beginning of file
	return 0;
}

int calcMD5(int file, int size){
	unsigned char c[MD5_DIGEST_LENGTH];
	MD5_CTX mdContext;
	int delay = 0, bytes, i, written = 0;
	unsigned char data[DATA_BUF_SIZE];
	MD5_Init(&mdContext);
	while((bytes = read(file, data, DATA_BUF_SIZE)) != 0){
		written+=bytes;
		if(++delay > DELAY_LENGTH) {
			printProgress(MD5_TAG, written, size);
			delay = 0;
		}
		MD5_Update(&mdContext, data, bytes);
	}
	MD5_Final(c, &mdContext);
	printf("\n%s: ", MD5_TAG);
	for(i=0; i < MD5_DIGEST_LENGTH; i++) printf("%02x",c[i]);
	printf("\n");
	lseek(file, 0, SEEK_SET); //return to beginning of file
	return 0;
}

int readMBR(int file, char *mbr){
	lseek(file, MBR_OFFSET, SEEK_SET);
	int i = read(file, (char*)mbr, sizeof(MBR));
	if(i <0){
		printf("Error no MBR found");
	}
	return 0;
}
