#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
void usage(char* arg){
	printf("Usage: %s -d [device filename] [other arguments]\n", arg);
	printf("-l target            List the target directory\n");
	printf("-r target -o dest    Recoverthe target pathname\n");
}
int main(int argc, char** argv){
	int dFlag = 0, ch, fd;
	char *device, *target, *dest; 
	if(argc == 1){
		usage(argv[0]);	
		exit(0);
	}
	while((ch = getopt(argc, argv, "d:l:r:o")) != -1){
		switch(ch){
			case 'd':
				dFlag = 1;
				device = malloc(strlen(optarg));
				strcpy(device, optarg);
				break;
			case 'l':
				target = malloc(strlen(optarg));
				strcpy(target, optarg);
				break;
			case 'r':
				dest = malloc(strlen(optarg));
				strcpy(dest, optarg);
				break;
			case '?':
			default:
				usage(argv[0]);
				exit(0);
		}
	}

	if(!dFlag) usage(argv[0]);
	return 0;	 
}


