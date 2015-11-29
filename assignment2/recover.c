#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "struct.h"
#define ENTRY_PER_DIR ((bootSector.BPB_BytsPerSec*bootSector.BPB_SecPerClus)/32)
FILE* file;
char bbuf[1024];
struct BootEntry bootSector;
struct DirEntry dirEntry;
long dataAreaPosi;

void usage(char* arg){
	printf("Usage: %s -d [device filename] [other arguments]\n", arg);
	printf("-l target            List the target directory\n");
	printf("-r target -o dest    Recoverthe target pathname\n");
}


void printFileName(char* filename){
	int i;
	for(i = 0; i < 8 && filename[i] != ' '; i++) printf("%c",filename[i]);
	if(filename[8] == ' ') return;
	printf(".");
	for(i = 8; i < 11 && filename[i] != ' '; i++) printf("%c", filename[i]);
}

/* Set file stream indicator to the beginning of nth clus*/ 
void setToNthClus(int n){
	fseek(file, dataAreaPosi+bootSector.BPB_BytsPerSec*bootSector.BPB_SecPerClus*(n-2), SEEK_SET);
}

int nextClus(int curClus){
	long backupPos = ftell(file);
	long pos = bootSector.BPB_BytsPerSec*(bootSector.BPB_RsvdSecCnt)+sizeof(int)*curClus;
	fseek(file, pos, SEEK_SET);
	int next; 
	fread(&next, sizeof(int), 1, file);
	fseek(file, backupPos, SEEK_SET);
	return next;	 
}

int findFirstClus(struct DirEntry curDirEntry){
	return curDirEntry.DIR_FstClusHI*256+curDirEntry.DIR_FstClusLO; 
}

long findDirClus(long curDirClus, char* target){
	long backupPos = ftell(file);
	char buf[ENTRY_PER_DIR][sizeof(struct DirEntry)];
/* A directory may contains more than one clustor*/
	long targetDirClus;
	while(curDirClus != 0){
		setToNthClus(curDirClus);
		fread(buf, sizeof(struct DirEntry), ENTRY_PER_DIR, file);
		int i;
		for(i = 0; i < ENTRY_PER_DIR; i++){
			struct DirEntry *tmp = (struct DirEntry*)buf[i];
			if(memcmp(target, tmp->DIR_Name, strlen(target)) == 0){
				targetDirClus = findFirstClus(*tmp);
				goto restore;
			}
		}	
		curDirClus = nextClus(curDirClus);
	}
restore:
	fseek(file, backupPos, SEEK_SET);		
	return targetDirClus; 
}

void listDirEntry(char* targetDir){
	long backupPos = ftell(file);
	char *tmp = strtok(targetDir, "/");
	long curDirClus = bootSector.BPB_RootClus;
	while(tmp != NULL){
		curDirClus = findDirClus(curDirClus, tmp);
		tmp = strtok(NULL, "/");
	}

	char buf[ENTRY_PER_DIR][sizeof(struct DirEntry)];
/* A directory may contains more than one clustor*/
	int i = 1;
	while(curDirClus != 0){
		setToNthClus(curDirClus);
		fread(buf, sizeof(struct DirEntry), ENTRY_PER_DIR, file);
		int j;
		for(j = 0; j < ENTRY_PER_DIR && buf[j][0] != '\0'; j++, i++){
			struct DirEntry *tmp = (struct DirEntry*)buf[j];
			printf("%d, ", i);
			printFileName(tmp->DIR_Name);
			if(tmp->DIR_Attr & 16) printf("/");
			printf(", %ld, %d\n", tmp->DIR_FileSize, findFirstClus(*tmp));
		}	
		curDirClus = nextClus(curDirClus);
	}
	fseek(file, backupPos, SEEK_SET);		
}

int main(int argc, char** argv){
	int ch, fd;
	char *device = NULL, *targetDir = NULL, *targetFile = NULL, *dest = NULL; 
	if(argc == 1){
		usage(argv[0]);	
		exit(0);
	}

	/* Handle option argument*/
	while((ch = getopt(argc, argv, "d:l:r:o")) != -1){
		switch(ch){
			case 'd':
				device = malloc(strlen(optarg));
				strcpy(device, optarg);
				break;
			case 'l':
				targetDir = malloc(strlen(optarg));
				strcpy(targetDir, optarg);
				break;
			case 'r':
				targetFile = malloc(strlen(optarg));
				strcpy(targetFile, optarg);
			case 'o':
				dest = malloc(strlen(optarg));
				strcpy(dest, optarg);
				break;
			case '?':
			default:
				usage(argv[0]);
				exit(0);
		}
	}
	if(device == NULL) usage(argv[0]);
	
	file = fopen(device, "r");
	if(file == NULL){
		perror(device);
		exit(1);
	}
	
	fread(bbuf, sizeof(struct BootEntry), 1, file);
	memcpy(&bootSector, bbuf, sizeof(struct BootEntry));
	dataAreaPosi = bootSector.BPB_BytsPerSec*(bootSector.BPB_RsvdSecCnt+bootSector.BPB_NumFATs*bootSector.BPB_FATSz32);
	if(targetDir != NULL){
		listDirEntry(targetDir);
	}
	return 0;	 
}


