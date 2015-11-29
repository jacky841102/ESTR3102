#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "struct.h"
void printFileName(char* filename){
	int i;
	for(i = 0; i < 8 && filename[i] != ' '; i++) printf("%c",filename[i]);
	if(filename[8] == ' ') return;
	printf(".");
	for(i = 8; i < 11 && filename[i] != ' '; i++) printf("%c", filename[i]);
}
int main(int argc, char **argv){
	FILE* file;
	char buf[1000];
	struct BootEntry be;
	file = fopen(argv[1], "r");
	if(file == NULL){
		perror(argv[1]);
		exit(1);
	}
	fread(buf, sizeof(struct BootEntry), 1, file);
	memcpy(&be, buf, sizeof(struct BootEntry));
	struct DirEntry dirEntry;
	long pos = be.BPB_BytsPerSec*(be.BPB_RsvdSecCnt+
					be.BPB_NumFATs*be.BPB_FATSz32+
					(be.BPB_RootClus-2)*be.BPB_SecPerClus);
	fseek(file, pos, SEEK_SET);
	while(1){
		fread(buf, sizeof(struct DirEntry), 1, file);
		if(buf[0] == '\0') break;
		memcpy(&dirEntry, buf, sizeof(struct DirEntry));
		printFileName(dirEntry.DIR_Name);
		printf(" size: %ld, ", dirEntry.DIR_FileSize);
		printf("First Cluster address: %02x%02x\n", dirEntry.DIR_FstClusHI, dirEntry.DIR_FstClusLO);
		long clus = dirEntry.DIR_FstClusHI*256+dirEntry.DIR_FstClusLO;//strtol(hex, NULL, 0);
		long backPos = ftell(file);
		fseek(file, be.BPB_BytsPerSec*(be.BPB_RsvdSecCnt+
					be.BPB_NumFATs*be.BPB_FATSz32+
					(clus-2)*be.BPB_SecPerClus), SEEK_SET);
		char tmp;
		while(1){
			fread(&tmp, 1, 1, file);
			if(tmp == '\0') break;
			printf("%c",tmp);
		}
		printf("\n");
		fseek(file, backPos, SEEK_SET);
	}
	return 0;
}
