#include <stdio.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define DELAY_LENGTH 500
#define DATA_BUF_SIZE 2048

#define MD5_TAG "MD5"
#define SHA1_TAG "SHA1"

int calcMD5(int, int);
int calcSHA1(int, int);

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
	if(file<0){
		printf("Unable to open file");
		return -1;
	}
	fstat(file, &fstats);
	calcMD5(file, fstats.st_size);
	calcSHA1(file, fstats.st_size);
	close(file);
	return 1;
}

void printProgress(const char* name, int written, int total){
	printf("Calculating %s: %i/%i bytes written - %2.0f%% <", name, written, total, (100.*(float)written)/(float)total);
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
	printf("\n%s Complete\n", SHA1_TAG);
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
	printf("\n%s Complete\n", MD5_TAG);
	for(i=0; i < MD5_DIGEST_LENGTH; i++) printf("%02x",c[i]);
	printf("\n");
	lseek(file, 0, SEEK_SET); //return to beginning of file
	return 0;

}
