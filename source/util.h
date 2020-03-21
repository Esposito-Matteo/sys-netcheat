/**
 * @file util.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2020-03-21
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef util
#define util
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <switch.h>
#include "util.h"
#endif

u64 UniquePadIds[2];

void fatalLater(Result err);
int setupServerSocket();
void setupPattern(HidsysNotificationLedPattern *pattern); //thanks https://github.com/ELY3M/Lighting-up-LED-on-right-joycon-for-Nintendo-Switch
void lightUpLed(HidsysNotificationLedPattern *pattern, Result rc);