#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "struct.h"
#include <sys/stat.h>
#include <errno.h>
#define ENTRY_PER_DIR ((bootSector.BPB_BytsPerSec*bootSector.BPB_SecPerClus)/32)
#define CLUSTOR_END(x) ({x >= 0xffffff8;})
//#define DEBUG

FILE* file;
unsigned char bbuf[1024];
struct BootEntry bootSector;
struct DirEntry dirEntry;
unsigned long dataAreaPosi;
void usage(unsigned char* arg){
	printf("Usage: %s -d [device filename] [other arguments]\n", arg);
	printf("-l target            List the target directory\n");
	printf("-r target -o dest    Recover the target pathname\n");
}



void printFileName(unsigned char* filename){
	int i = 0;
	if(filename[0] == (unsigned char)0xe5){ 
		printf("?");
		i =1; 
	}
	for(i; i < 8 && filename[i] != ' '; i++) printf("%c",filename[i]);
	if(filename[8] == ' ') return;
	printf(".");
	for(i = 8; i < 11 && filename[i] != ' '; i++) printf("%c", filename[i]);
}

/* Set file stream indicator to the beginning of nth clus*/ 
void setToNthClus(int n){
	fseek(file, dataAreaPosi+bootSector.BPB_BytsPerSec*bootSector.BPB_SecPerClus*(n-2), SEEK_SET);
}

/*Find next clustor from current clustor in FAT*/
unsigned long nextClus(unsigned long curClus){
	unsigned long backupPos = ftell(file);
	unsigned long pos = bootSector.BPB_BytsPerSec*(bootSector.BPB_RsvdSecCnt)+4*curClus;
	fseek(file, pos, SEEK_SET);
	unsigned int next; 
	fread(&next, 4, 1, file);
	#ifdef DEBUG
	printf("now: %lu next: %u \n", curClus, next);
	#endif
	fseek(file, backupPos, SEEK_SET);
	return next;	 
}

unsigned long findFirstClus(struct DirEntry curDirEntry){
	return curDirEntry.DIR_FstClusHI*256*256+curDirEntry.DIR_FstClusLO; 
}

unsigned long findDirClus(unsigned long curDirClus, unsigned char* target){
	unsigned long backupPos = ftell(file);
	unsigned char buf[ENTRY_PER_DIR][sizeof(struct DirEntry)];
/* A directory may contains more than one clustor*/
	unsigned long targetDirClus;
	while(!CLUSTOR_END(curDirClus)){
		setToNthClus(curDirClus);
		fread(buf, sizeof(struct DirEntry), ENTRY_PER_DIR, file);
		int i;
		for(i = 0; i < ENTRY_PER_DIR; i++){
			struct DirEntry *tmp = (struct DirEntry*)buf[i];
			int len = 0; 
			for(; tmp->DIR_Name[len] != ' ' && len < 8; len++);
			if(len == strlen(target) && memcmp(target, tmp->DIR_Name, len) == 0){
	//			printf("len = %d strlen(target) = %d\n", len, strlen(target));		
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

void listDirEntry(unsigned char* targetDir){
	unsigned long backupPos = ftell(file);
/*Directoy and subdirectory*/
	unsigned char *tmp = strtok(targetDir, "/");     
	unsigned long curDirClus = bootSector.BPB_RootClus;
	while(tmp != NULL){
		curDirClus = findDirClus(curDirClus, tmp);
		tmp = strtok(NULL, "/");
	}

	unsigned char buf[ENTRY_PER_DIR][sizeof(struct DirEntry)];
/* A directory may contains more than one clustor*/
	int i = 1;
	while(!CLUSTOR_END(curDirClus)){
		setToNthClus(curDirClus);
		fread(buf, sizeof(struct DirEntry), ENTRY_PER_DIR, file);
		int j;
		for(j = 0; j < ENTRY_PER_DIR && buf[j][0] != '\0'; j++){
			struct DirEntry *tmp = (struct DirEntry*)buf[j];
			if(tmp->DIR_Attr == 0x0f) continue; //long filename 
			printf("%d, ", i);
			printFileName(tmp->DIR_Name);
			if(tmp->DIR_Attr & 0x10) printf("/"); //directory
			printf(", %lu, %lu\n", tmp->DIR_FileSize, findFirstClus(*tmp));
			i++;
		}	
		curDirClus = nextClus(curDirClus);
	}
	fseek(file, backupPos, SEEK_SET);		
}

void to8Dot3Filename(unsigned char newFilename[11], unsigned char* filename){
	if(strstr(filename, ".") != NULL){
		int i;
		for(i = 0; filename[i]!= '.'; i++) newFilename[i] = filename[i];
		int j = i+1;
		for(; i < 8; i++) newFilename[i] = ' ';
		for(; j < 11 && j < strlen(filename); i++, j++) newFilename[i] = filename[j];
		for(; i < 11; i++) newFilename[i] = ' '; 
	}else{
		int i;
		for(i = 0; i < 11 && i < strlen(filename); i++) newFilename[i] = filename[i];
		for(; i < 11; i++) newFilename[i] = ' ';
	}
}

void recover(struct DirEntry entry, unsigned char* dest){
	unsigned long backupPos = ftell(file);
	setToNthClus(findFirstClus(entry));
	FILE* outputFile = fopen(dest, "w");	
	if(errno == EACCES){
		printf("%s: failed to open\n", dest);
		exit(0);
	}
	unsigned char* charBuf = malloc(entry.DIR_FileSize);
	#ifdef DEBUG
	printf("size: %lu\n", entry.DIR_FileSize);
	#endif
	fread(charBuf, entry.DIR_FileSize, 1, file);
	fwrite(charBuf, entry.DIR_FileSize, 1, outputFile);
	free(charBuf);
	fclose(outputFile);
	fseek(file, backupPos, SEEK_SET);
}

int recoverTargetFile(unsigned char* target, unsigned char* dest){
	unsigned char* tokens[100];
	int tokenNum = 0;	
	unsigned char* tmp = strtok(target, "/");
	while(tmp != NULL){
		tokens[tokenNum++] = tmp;
		tmp = strtok(NULL, "/");
	}
	int i = 0; 
	unsigned long curDirClus = bootSector.BPB_RootClus;
	for(i = 0; i < tokenNum-1; i++) curDirClus = findDirClus(curDirClus, tokens[i]);
	unsigned char target8Dot3Name[11];
	to8Dot3Filename(target8Dot3Name, tokens[tokenNum-1]);
	target8Dot3Name[0] = (unsigned char)0xe5;	
	unsigned char buf[ENTRY_PER_DIR][sizeof(struct DirEntry)];
/* A directory may contains more than one clustor*/
	while(!CLUSTOR_END(curDirClus)){
		setToNthClus(curDirClus);
		fread(buf, sizeof(struct DirEntry), ENTRY_PER_DIR, file);
		int j;
		for(j = 0; j < ENTRY_PER_DIR && buf[j][0] != '\0'; j++, i++){
			struct DirEntry *tmp = (struct DirEntry*)buf[j];		
			//printf("%s %d, %d\n", tmp->DIR_Name, nextClus(findFirstClus(*tmp)), EOF); //Clustor occupied
			if(tmp->DIR_Attr & 0x10 ||  // directory
			   tmp->DIR_Attr == 0x0f || //long file name
			   tmp->DIR_Name[0] != 0xe5) // normal file 
				continue;
			else{
				if(memcmp(target8Dot3Name, tmp->DIR_Name, 11) == 0){
					if(findFirstClus(*tmp) == 0){
						recover(*tmp, dest);
						return 0;
					}
					if(nextClus(findFirstClus(*tmp)) != 0) return -2; //Clustor occupied or data not written to disk 
					recover(*tmp, dest);
					return 0;  //Success
				}
			}
		}	
		curDirClus = nextClus(curDirClus);
	}
	return -1;   //No such file
}

int main(int argc, char** argv){
	int ch;
	unsigned char *device = NULL, *targetDir = NULL, *targetFile = NULL, *dest = NULL; 
	if(argc == 1){
		usage(argv[0]);	
		exit(0);
	}
	if(strcmp(argv[1], "-d") != 0){
		usage(argv[0]);
		exit(0);
	}
	/* Handle option argument*/
	while((ch = getopt(argc, argv, "d:l:r:o:")) != -1){
		switch(ch){
			case 'd':
				if(device != NULL){
					usage(argv[0]);	
					exit(0);
				}
				device = malloc(strlen(optarg)+1);
				strcpy(device, optarg);
				break;
			case 'l':
				if(targetDir != NULL){
					usage(argv[0]);	
					exit(0);
				}
				targetDir = malloc(strlen(optarg)+1);
				strcpy(targetDir, optarg);
				break;
			case 'r':
				if(targetFile != NULL){
					usage(argv[0]);	
					exit(0);
				}
				targetFile = malloc(strlen(optarg)+1);
				strcpy(targetFile, optarg);
				break;
			case 'o':
				if(dest != NULL){
					usage(argv[0]);	
					exit(0);
				}
				dest = malloc(strlen(optarg)+1);
				strcpy(dest, optarg);
				break;
			case '?':
			default:
				usage(argv[0]);
				exit(0);
		}
	}
	if(targetDir == NULL && targetFile == NULL && dest == NULL){
		usage(argv[0]);
		exit(0);
	}
	if(device == NULL){
		usage(argv[0]);
		exit(0);
	}
	file = fopen(device, "r");
	if(file == NULL){
		perror(device);
		exit(0);
	}
	fread(bbuf, sizeof(struct BootEntry), 1, file);
	memcpy(&bootSector, bbuf, sizeof(struct BootEntry));
	dataAreaPosi = bootSector.BPB_BytsPerSec*(bootSector.BPB_RsvdSecCnt+bootSector.BPB_NumFATs*bootSector.BPB_FATSz32);
	#ifdef DEBUG
	printf("rootClustor: %lu\n", bootSector.BPB_RootClus);
	printf("targetDir = %s\n", targetDir);	
	#endif
	if(targetDir != NULL){
		listDirEntry(targetDir);
	}
	if(targetFile != NULL){
		unsigned char *target = malloc(strlen(targetFile));
		strcpy(target, targetFile);
		switch(recoverTargetFile(target, dest)){
			case 0:
				printf("%s: recovered\n", targetFile);
				break;
			case -1:
				printf("%s: error - file not found\n", targetFile);
				break;
			case -2:
				printf("%s: error - fail to recover\n", targetFile);
				break;
		}
		free(target);
	}
	fclose(file);
	return 0;	 
}


