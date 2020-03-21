/**
 * @file args.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2020-03-21
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef args
#define args
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

int parseArgs(char* argstr, int (*callback)(int, char**));