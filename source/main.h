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

#include "args.h"
#include "util.h"
#include "processUtilities.h"

#define TITLE_ID 0x420000000000000F
#define HEAP_SIZE 0x000340000
#define MAX_LINE_LENGTH 300

#endif
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
#define SEARCH_ARR_SIZE 500000
u64 searchArr[SEARCH_ARR_SIZE];
int searchSize;

int sock = -1;

Handle debughandle = 0;

char *valtypes[] = {"none", "u8", "u16", "u32", "u64", "s8", "s16", "s32", "s64"};


static Mutex actionLock;


#define FREEZE_LIST_LEN 100
u64 freezeAddrs[FREEZE_LIST_LEN];
int freezeTypes[FREEZE_LIST_LEN];
u64 freezeVals[FREEZE_LIST_LEN];
int numFreezes = 0;

void printHelp();
int ssearch(char *arg1,char *arg2, int argc);