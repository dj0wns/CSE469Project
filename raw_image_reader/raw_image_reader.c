#include <stdio.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>

#define DIVIDER_LEN 60
#define DIVIDER_CHAR "="

#define DELAY_LENGTH 500
#define DATA_BUF_SIZE 2048

#define MD5_TAG "MD5"
#define SHA1_TAG "SHA1"

#define MBR_OFFSET 0x1BE
#define NUM_OF_PARTITIONS 4
#define DEFAULT_SECTOR_SIZE 512
#define OEM_NAME_LEN 8
#define SIZE_OF_BOOT_INSTRUCTIONS 3

#define FAT32Switch case(0x0B): case(0x0C): case(0x1B):
#define FAT16Switch case(0x04): case(0x06):
#define FAT16Offset 32

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

typedef struct __attribute__((__packed__)){
	uint8_t asmBootInstructions[SIZE_OF_BOOT_INSTRUCTIONS];
	char OEMName[OEM_NAME_LEN];
	uint16_t bytesPerSector;
	uint8_t sectorsPerCluster;
	uint16_t sizeOfReserved;
	uint8_t numFATs;
	uint16_t maxFilesInRoot;
	uint16_t numSectors16;
	uint8_t mediaType;
	uint16_t sizeOfFAT;
	uint16_t sectorsPerTrack;
	uint16_t numHeads;
	uint32_t numPriorSectors;
	uint32_t numSectors32;
	uint32_t sizeOfFAT32;
} VBR;

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
};


int calcMD5(int, int, char*);
int calcSHA1(int, int, char*);
int readMBR(int, char*);
int readVBR(int, MBR*, VBR*);
void printPartitions(MBR*);
void printDivider();
void printVBR(MBR*, VBR*);

int main(int argc, char** argv){

	if(argc != 2){
		printf("Invalid Arguments. Usage: raw_image_reader <path_to_file>\n");
		return -1;
	} 
	struct stat fstats;	
	int file = open(argv[1], O_RDONLY);
	MBR mbr;
	VBR vbr[NUM_OF_PARTITIONS];
	if(file<0){
		printf("Unable to open file");
		return -1;
	}
	printDivider();
	fstat(file, &fstats);
	calcMD5(file, fstats.st_size, argv[1]);
	calcSHA1(file, fstats.st_size, argv[1]);
	readMBR(file, (char*)&mbr);
	readVBR(file, &mbr, vbr);
	printDivider();
	printPartitions(&mbr);
	printDivider();
	printVBR(&mbr,vbr);
	close(file);
	return 1;
}

void printDivider(){
	printf("\n");
	for(int i = 0; i < DIVIDER_LEN; i++) printf(DIVIDER_CHAR); 
	printf("\n\n");
}

void printVBR(MBR *mbr, VBR *vbr){
	int fatSize, endingSec, firstSecCluster2;
	
	for(int i =0; i < NUM_OF_PARTITIONS; i++){	
		fatSize = vbr[i].sizeOfFAT;
		endingSec = 0;
		firstSecCluster2 = FAT16Offset;
		switch(mbr->entries[i].type){
			FAT32Switch
				fatSize = vbr[i].sizeOfFAT32;
				firstSecCluster2-= FAT16Offset;
			FAT16Switch
				endingSec = vbr[i].sizeOfReserved-1 + vbr[i].numFATs * fatSize;
				firstSecCluster2 += mbr->entries[i].sectorDistance + endingSec + 1;
				
				printf("Partition %i(%s):\n", i, partition_type[mbr->entries[i].type]);	
				printf("Reserved Area: Start Sector: %i Ending Sector: %i Size: %i sectors\n",
						0, vbr[i].sizeOfReserved-1, vbr[i].sizeOfReserved);
				printf("Sectors per cluster: %i sectors\n", 
						vbr[i].sectorsPerCluster);
				printf("FAT ares: Start Sector: %i Ending Sector: %i\n",
						vbr[i].sizeOfReserved, endingSec);
				printf("# of FATs: %i\n",
					vbr[i].numFATs);
				printf("The size of each FAT: %i sectors\n",
						fatSize);
				printf("The first sector of cluster %i: %i sectors\n",
					2, firstSecCluster2);
				printDivider();
				break;
			default:
				printf("Partition %i(%s) is not a FAT16/32 Partition\n",
						i, partition_type[mbr->entries[i].type]);
		}
	}
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

int calcSHA1(int file, int size, char* fname){
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
	char final[256] = SHA1_TAG;
	strcat(final, fname);
	strcat (final, ".txt");
	FILE *fp = fopen(final, "w");
	if(fp){
		for(i=0; i < SHA_DIGEST_LENGTH; i++) fprintf(fp, "%02x",c[i]);
		fprintf(fp, "\t%s", fname);
	}
	fclose(fp);
	

	return 0;
}

int calcMD5(int file, int size, char* fname){
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
	
	//write to file
	char final[256] = MD5_TAG;
	strcat(final, fname);
	strcat (final, ".txt");
	FILE *fp = fopen(final, "w");
	if(fp){
		for(i=0; i < MD5_DIGEST_LENGTH; i++) fprintf(fp, "%02x",c[i]);
		fprintf(fp, "\t%s", fname);
	}
	fclose(fp);
	
	return 0;
}

int readMBR(int file, char *mbr){
	lseek(file, MBR_OFFSET, SEEK_SET);
	int i = read(file, mbr, sizeof(MBR));
	if(i <0){
		printf("Error no MBR found");
	}
	return 0;
}

int readVBR(int file, MBR *mbr, VBR* vbr){
	for(int i = 0; i < NUM_OF_PARTITIONS; i++){
		lseek(file, mbr->entries[i].sectorDistance * DEFAULT_SECTOR_SIZE, SEEK_SET);
		int r = read(file, (char*)(&vbr[i]), sizeof(VBR));	
		if(r <0){
			printf("Error no VBR  found");
		}	
	}
	return 0;
}
