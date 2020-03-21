/**
 * @file processUtilities.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2020-03-21
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "processUtilities.h"
/**
 * @brief Scan the Procesess list and find a game-like process and try to get an handle for "debuggin process"
 * 
 * @return int 0 if ok, 1 else
 */
int attach(Handle *debughandle)
{
    if (debughandle != 0)
        svcCloseHandle(debughandle);
    u64 pids[300];
    u32 numProc;
    svcGetProcessList(&numProc, pids, 300);
    u64 pid = pids[numProc - 1];

    Result rc = svcDebugActiveProcess(&debughandle, pid);
    if (R_FAILED(rc))
    {
        printf("Couldn't open the process (Error: %x)\r\n"
               "Make sure that you actually started a game.\r\n",
               rc);
        return 1;
    }
    return 0;
}

/**
 * @brief Detach itself from the previously attached process
 * 
 */
void detach(Handle *debughandle )
{
    if (debughandle != 0)
        svcCloseHandle(debughandle);
    debughandle = 0;
}
