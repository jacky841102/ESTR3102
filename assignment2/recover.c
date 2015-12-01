#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "struct.h"
#include <sys/stat.h>
#include <errno.h>
#define ENTRY_PER_DIR ((bootSector.BPB_BytsPerSec*bootSector.BPB_SecPerClus)/32)
#define CLUSTOR_END(x) ({x >= 0xffffff8;})
FILE* file;
char bbuf[1024];
struct BootEntry bootSector;
struct DirEntry dirEntry;
unsigned long dataAreaPosi;
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

/*Find next clustor from current clustor in FAT*/
unsigned long nextClus(unsigned long curClus){
	unsigned long backupPos = ftell(file);
	unsigned long pos = bootSector.BPB_BytsPerSec*(bootSector.BPB_RsvdSecCnt)+4*(curClus-2);
	fseek(file, pos, SEEK_SET);
	unsigned long next; 
	fread(&next, 4, 1, file);
	printf("now: %u next: %u \n", curClus, next);
	fseek(file, backupPos, SEEK_SET);
	return next;	 
}

unsigned long findFirstClus(struct DirEntry curDirEntry){
	return curDirEntry.DIR_FstClusHI*256+curDirEntry.DIR_FstClusLO; 
}

unsigned long findDirClus(unsigned long curDirClus, char* target){
	unsigned long backupPos = ftell(file);
	char buf[ENTRY_PER_DIR][sizeof(struct DirEntry)];
/* A directory may contains more than one clustor*/
	unsigned long targetDirClus;
	while(!CLUSTOR_END(curDirClus)){
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
	unsigned long backupPos = ftell(file);
	char *tmp = strtok(targetDir, "/");
	unsigned long curDirClus = bootSector.BPB_RootClus;
	while(tmp != NULL){
		curDirClus = findDirClus(curDirClus, tmp);
		tmp = strtok(NULL, "/");
	}

	char buf[ENTRY_PER_DIR][sizeof(struct DirEntry)];
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
			printf(", %ld, %d\n", tmp->DIR_FileSize, findFirstClus(*tmp));
			i++;
		}	
		curDirClus = nextClus(curDirClus);
	}
	fseek(file, backupPos, SEEK_SET);		
}

void to8Dot3Filename(char newFilename[11], char* filename){
	if(strstr(filename, ".") != NULL){
		int i;
		for(i = 0; filename[i]!= '.'; i++) newFilename[i] = filename[i];
		int j = i+1;
		for(; i < 8; i++) newFilename[i] = ' ';
		for(; j < strlen(filename); i++, j++) newFilename[i] = filename[j];
		for(; i < 11; i++) newFilename[i] = ' '; 
	}else{
		int i;
		for(i = 0; i < strlen(filename); i++) newFilename[i] = filename[i];
		for(; i < 11; i++) newFilename[i] = ' ';
	}
}

void recover(struct DirEntry entry, char* dest){
	unsigned long backupPos = ftell(file);
	setToNthClus(findFirstClus(entry));
	struct stat tmp;
	FILE* outputFile = fopen(dest, "w");	
	if(errno == EACCES){
		printf("%s: failed to open\n", dest);
		exit(0);
	}
	char* charBuf = malloc(entry.DIR_FileSize);
	printf("size: %u\n", entry.DIR_FileSize);
	fread(charBuf, entry.DIR_FileSize, 1, file);
	int i = 0;
	while(i < entry.DIR_FileSize) fprintf(outputFile, "%c", charBuf[i++]);
	free(charBuf);
	fclose(outputFile);
	fseek(file, backupPos, SEEK_SET);
}


int recoverTargetFile(char* target, char* dest){
	char* tokens[100];
	int tokenNum = 0;	
	char* tmp = strtok(target, "/");
	while(tmp != NULL){
		tokens[tokenNum++] = tmp;
		tmp = strtok(NULL, "/");
	}
	int i = 0; 
	unsigned long curDirClus = bootSector.BPB_RootClus;
	for(; i < tokenNum-1; i++) curDirClus = findDirClus(curDirClus, tokens[i]);
	char target8Dot3Name[11];
	to8Dot3Filename(target8Dot3Name, tokens[tokenNum-1]);
	target8Dot3Name[0] = 0xe5;	
	char buf[ENTRY_PER_DIR][sizeof(struct DirEntry)];
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
					if(nextClus(findFirstClus(*tmp)) != 0 ||
					   findFirstClus(*tmp) == 0)return -2; //Clustor occupied or data not written to disk 
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
	int ch, fd;
	char *device = NULL, *targetDir = NULL, *targetFile = NULL, *dest = NULL; 
	if(argc == 1){
		usage(argv[0]);	
		exit(0);
	}

	/* Handle option argument*/
	while((ch = getopt(argc, argv, "d:l:r:o:")) != -1){
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
				break;
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
	printf("rootClustor: %u\n", bootSector.BPB_RootClus);
	if(targetDir != NULL){
		listDirEntry(targetDir);
	}

	if(targetFile != NULL){
		switch(recoverTargetFile(targetFile, dest)){
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
	}

	fclose(file);

	return 0;	 
}


