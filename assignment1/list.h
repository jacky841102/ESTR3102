/*************************************************************************
	> File Name: list.h
	> Author: jacky
	> Mail: kuoyichun1102@gmail.com
	> Created Time: 二 10/13 14:32:12 2015
 ************************************************************************/

#ifndef _LIST_H
#define _LIST_H

#include<unistd.h>
#include<stdlib.h>

typedef struct jobs{
    char cmd[255];
    int pidNum;
    pid_t *pid;
    struct jobs* next;
}Jobs;

static void INIT_LIST_HEAD(Jobs* head){
    if(head == NULL){
        head = (Jobs*)malloc(sizeof(Jobs));
        head->next = NULL;
    }
}

static void list_add(Jobs* head, Jobs* newJob){
    Jobs* it = head; 
    while(it->next != NULL){
        it = it->next;
    }
    it->next = newJob;
    newJob->next = NULL;
    head->pidNum++;
}

static void list_del(Jobs* head, Jobs* delJobs){
    Jobs* it = head;
    while(it->next != delJobs){
        it = it->next;
    }
    it->next = it->next->next;
    head->pidNum--;
    free(delJobs);
}


static Jobs* list_entry(Jobs* head, int index){
    Jobs* it = head;
    while(index != 0){
        it = it->next;
        index--;
    }
    return it;
}


#endif


