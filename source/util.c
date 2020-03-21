/**
 * @file util.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2020-03-21
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "util.h"

int setupServerSocket()
{
    int lissock;
    struct sockaddr_in server;
    lissock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(5555);

    while (bind(lissock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        svcSleepThread(1e+9L);
    }
    listen(lissock, 3);
    return lissock;
}

void fatalLater(Result err)
{
    Handle srv;

    while (R_FAILED(smGetServiceOriginal(&srv, smEncodeName("fatal:u"))))
    {
        // wait one sec and retry
        svcSleepThread(1000000000L);
    }

    // fatal is here time, fatal like a boss
    IpcCommand c;
    ipcInitialize(&c);
    ipcSendPid(&c);
    struct
    {
        u64 magic;
        u64 cmd_id;
        u64 result;
        u64 unknown;
    } * raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 1;
    raw->result = err;
    raw->unknown = 0;

    ipcDispatch(srv);
    svcCloseHandle(srv);
}

void setupPattern(HidsysNotificationLedPattern pattern) //thanks https://github.com/ELY3M/Lighting-up-LED-on-right-joycon-for-Nintendo-Switch
{
        pattern.baseMiniCycleDuration = 0x1;                // 12.5 ms
        pattern.totalMiniCycles = 0x0;                   // 0+1 mini cycles. Last one 12.5ms.
        pattern.totalFullCycles = 0x1;                   // Repeat Only Once.
        pattern.startIntensity = 0xF;                    // 100%.


        pattern.miniCycles[0].ledIntensity = 0xF;        // 100%.
        pattern.miniCycles[0].transitionSteps = 0xF;     // 15 steps. Total 187.5ms.
        pattern.miniCycles[0].finalStepDuration = 0x0;   // Forced 12.5ms.

        pattern.miniCycles[1].ledIntensity = 0x0;        // 0%.
        pattern.miniCycles[1].transitionSteps = 0xF;     // 15 steps. Total 187.5ms.
        pattern.miniCycles[1].finalStepDuration = 0x0;   // Forced 12.5ms.

}

void lightUpLed(HidsysNotificationLedPattern pattern, Result rc){
        int total_entries = 0;
        memset(UniquePadIds, 0, sizeof(UniquePadIds));
        
        // Get the UniquePadIds for the specified controller, which will then be used with hidsysSetNotificationLedPattern.
        // If you want to get the UniquePadIds for all controllers, you can use hidsysGetUniquePadIds instead.
        rc = hidsysGetUniquePadsFromNpad(hidGetHandheldMode() ? CONTROLLER_HANDHELD : CONTROLLER_PLAYER_1, UniquePadIds, 2, &total_entries);
       if (R_SUCCEEDED(rc)) {
        for (int i = 0; i < total_entries; i++) { // System will skip sending the subcommand to controllers where this isn't available.
            rc = hidsysSetNotificationLedPattern(&pattern, UniquePadIds[i]);
        }
       }
}