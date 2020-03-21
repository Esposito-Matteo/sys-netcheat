/**
 * @file main.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2020-03-21
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef main
#define main

#include <string.h>
#include <switch.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif


#include "args.h"
#include "util.h"
#include "processUtilities.h"

#define TITLE_ID 0x420000000000000F
#define HEAP_SIZE 0x000340000
#define MAX_LINE_LENGTH 300

#define SEARCH_ARR_SIZE 500000
#define FREEZE_LIST_LEN 100


enum
{
    VAL_NONE,
    VAL_U8,
    VAL_U16,
    VAL_U32,
    VAL_U64,
    VAL_S8,
    VAL_S16,
    VAL_S32,
    VAL_S64
};

int search = VAL_NONE;
u64 searchArr[SEARCH_ARR_SIZE];
int searchSize;



static Mutex actionLock;
u64 freezeAddrs[FREEZE_LIST_LEN];
int freezeTypes[FREEZE_LIST_LEN];
u64 freezeVals[FREEZE_LIST_LEN];

void printHelp();
int ssearch(char *arg1,char *arg2, int argc);