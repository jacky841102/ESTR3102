/*************************************************************************
	> File Name: cd.c
	> Author: jacky
	> Mail: kuoyichun1102@gmail.com
	> Created Time: 二  9/29 21:56:21 2015
 ************************************************************************/

#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include"cd.h"

void cd(char* dest){
    if(chdir(dest) != -1){
        return;
    }else{
        printf("[%s]: cannot change directory\n", dest);
        printf("Error: [%s]\n",strerror(errno));
        return;
    }
    
}
