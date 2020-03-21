/**
 * @file main.c
 * @author Matteo Esposito
 * @brief Main Function
 * @version 2.1
 * @date 2020-03-21
 * 
 * @copyright Copyright (c) 2020 GPL License
 * 
 */
#include "main.h"

int sock = -1;
Handle debughandle = 0;
char *valtypes[] = {"none", "u8", "u16", "u32", "u64", "s8", "s16", "s32", "s64"};
int numFreezes = 0;
HidsysNotificationLedPattern pattern;

bool initflag=0;
    size_t i;
    size_t total_entries;

// we aren't an applet
u32 __nx_applet_type = AppletType_None;

// setup a fake heap (we don't need the heap anyway)
char fake_heap[HEAP_SIZE];

// we override libnx internals to do a minimal init
void __libnx_initheap(void)
{
    extern char *fake_heap_start;
    extern char *fake_heap_end;

    // setup newlib fake heap
    fake_heap_start = fake_heap;
    fake_heap_end = fake_heap + HEAP_SIZE;
}
Result rc;
void __appInit(void)
{
    
    svcSleepThread(20000000000L);
    rc = smInitialize();
    if (R_FAILED(rc))
        fatalLater(rc);
    rc = fsInitialize();
    if (R_FAILED(rc))
        fatalLater(rc);
    rc = fsdevMountSdmc();
    if (R_FAILED(rc))
        fatalLater(rc);
    rc = timeInitialize();
    if (R_FAILED(rc))
        fatalLater(rc);
    rc = socketInitializeDefault();
    if (R_FAILED(rc))
        fatalLater(rc);
}

void __appExit(void)
{
    fsdevUnmountAll();
    fsExit();
    smExit();
    audoutExit();
    timeExit();
    socketExit();
}

// @Class List
void freezeList()
{
    for (int i = 0; i < numFreezes; i++)
    {
        printf("%d) %lx (%s) = %ld\r\n", i, freezeAddrs[i], valtypes[freezeTypes[i]], freezeVals[i]);
    }
}

void freezeAdd(u64 addr, int type, u64 value)
{
    if (numFreezes >= FREEZE_LIST_LEN)
    {
        printf("Can't add any more frozen values!\r\n"
               "Please remove some of the old ones!\r\n");
    }
    freezeAddrs[numFreezes] = addr;
    freezeTypes[numFreezes] = type;
    freezeVals[numFreezes] = value;
    numFreezes++;
}

void freezeDel(int index)
{
    if(numFreezes <= index) {
        printf("That number doesn't exit!");
    }
    numFreezes--;
    for (int i = index; i < numFreezes; i++)
    {
        freezeAddrs[i] = freezeAddrs[i + 1];
        freezeTypes[i] = freezeTypes[i + 1];
        freezeVals[i] = freezeVals[i + 1];
    }
}

void freezeLoop()
{
    while (1)
    {
        mutexLock(&actionLock);
        for (int i = 0; i < numFreezes; i++)
        {
            if (attach(&debughandle))
            {
                printf("The process apparently died. Cleaning the freezes up!\r\n");
                while (numFreezes > 0)
                {
                    freezeDel(0);
                }
                break;
            }
            if (freezeTypes[i] == VAL_U8)
            {
                u8 val = (u8)freezeVals[i];
                svcWriteDebugProcessMemory(debughandle, &val, freezeAddrs[i], sizeof(u8));
            }
            else if (freezeTypes[i] == VAL_U16)
            {
                u16 val = (u16)freezeVals[i];
                svcWriteDebugProcessMemory(debughandle, &val, freezeAddrs[i], sizeof(u16));
            }
            else if (freezeTypes[i] == VAL_U32)
            {
                u32 val = (u32)freezeVals[i];
                svcWriteDebugProcessMemory(debughandle, &val, freezeAddrs[i], sizeof(u32));
            }
            else if (freezeTypes[i] == VAL_U64)
            {
                u64 val = (u64)freezeVals[i];
                svcWriteDebugProcessMemory(debughandle, &val, freezeAddrs[i], sizeof(u64));
            }
            detach(&debughandle);
        }
        mutexUnlock(&actionLock);
        svcSleepThread(5e+8L);
    }
}


int argmain(int argc, char **argv)
{
    if (argc == 0)
    {
        return 0;
    }
    if (!strcmp(argv[0], "help") || !strcmp(argv[0], "h"))
    {
        printHelp();
        return 0;
    }

    if (!strcmp(argv[0], "ssearch") || !strcmp(argv[0], "s"))
    {
        int res = ssearch(argv[1],argv[2],argc);
        return res;
    }

    if (!strcmp(argv[0], "csearch") || !strcmp(argv[0], "c"))
    {
        if (argc != 2)
        {
            printHelp();
            return 0;
        }
        if (search == VAL_NONE)
        {
            printf("You need to start a search first!");
            return 0;
        }

        u8 u8NewVal = 0;
        u16 u16NewVal = 0;
        u32 u32NewVal = 0;
        u64 u64NewVal = 0;

        if (search == VAL_U8)
        {
            u8NewVal = strtoul(argv[1], NULL, 10);
        }
        else if (search == VAL_U16)
        {
            u16NewVal = strtoul(argv[1], NULL, 10);
        }
        else if (search == VAL_U32)
        {
            u32NewVal = strtoul(argv[1], NULL, 10);
        }
        else if (search == VAL_U64)
        {
            u64NewVal = strtoull(argv[1], NULL, 10);
        }

        u64 newSearchSize = 0;
        for (int i = 0; i < searchSize; i++)
        {
            if (search == VAL_U8)
            {
                u8 val;
                svcReadDebugProcessMemory(&val, debughandle, searchArr[i], sizeof(u8));
                if (val == u8NewVal)
                {
                    printf("Got a hit at %lx!\r\n", searchArr[i]);
                    searchArr[newSearchSize++] = searchArr[i];
                }
            }
            if (search == VAL_U16)
            {
                u16 val;
                svcReadDebugProcessMemory(&val, debughandle, searchArr[i], sizeof(u16));
                if (val == u16NewVal)
                {
                    printf("Got a hit at %lx!\r\n", searchArr[i]);
                    searchArr[newSearchSize++] = searchArr[i];
                }
            }
            if (search == VAL_U32)
            {
                u32 val;
                svcReadDebugProcessMemory(&val, debughandle, searchArr[i], sizeof(u32));
                if (val == u32NewVal)
                {
                    printf("Got a hit at %lx!\r\n", searchArr[i]);
                    searchArr[newSearchSize++] = searchArr[i];
                }
            }
            if (search == VAL_U64)
            {
                u64 val;
                svcReadDebugProcessMemory(&val, debughandle, searchArr[i], sizeof(u64));
                if (val == u64NewVal)
                {
                    printf("Got a hit at %lx!\r\n", searchArr[i]);
                    searchArr[newSearchSize++] = searchArr[i];
                }
            }
        }

        searchSize = newSearchSize;
        return 0;
    }

    if (!strcmp(argv[0], "poke") || !strcmp(argv[0], "p"))
    {
        if (argc != 4)
        {
            printHelp();
            return 0;
        }
        u64 addr;
        if (argv[1][0] == '$') {
            int index = strtol(argv[1] + 1, NULL, 10);
            if (index >= 0 && index < searchSize) {
                addr = searchArr[index];
            } else 
            {
                printHelp();
                return 0;
            }
        } else {
            addr = strtoull(argv[1], NULL, 16);
        }

        if (!strcmp(argv[2], "u8"))
        {
            u8 val = strtoul(argv[3], NULL, 10);
            svcWriteDebugProcessMemory(debughandle, &val, addr, sizeof(u8));
        }
        else if (!strcmp(argv[2], "u16"))
        {
            u16 val = strtoul(argv[3], NULL, 10);
            svcWriteDebugProcessMemory(debughandle, &val, addr, sizeof(u16));
        }
        else if (!strcmp(argv[2], "u32"))
        {
            u32 val = strtoul(argv[3], NULL, 10);
            svcWriteDebugProcessMemory(debughandle, &val, addr, sizeof(u32));
        }
        else if (!strcmp(argv[2], "u64"))
        {
            u64 val = strtoull(argv[3], NULL, 10);
            svcWriteDebugProcessMemory(debughandle, &val, addr, sizeof(u64));
        }
        else
        {
            printHelp();
            return 0;
        }
        return 0;
    }

    if (!strcmp(argv[0], "peek"))
    {
        if (argc != 3)
        {
            printHelp();
            return 0;
        }
            
        u64 addr = strtoull(argv[1], NULL, 16);

        if (!strcmp(argv[2], "u8"))
        {
            u8 val;
            svcReadDebugProcessMemory(&val, debughandle, addr, sizeof(u8));
            printf("Value is %hu\r\n", val);
        }
        else if (!strcmp(argv[2], "u16"))
        {
            u16 val;
            svcReadDebugProcessMemory(&val, debughandle, addr, sizeof(u16));
            printf("Value is %hu\r\n", val);
        }
        else if (!strcmp(argv[2], "u32"))
        {
            u32 val;
            svcReadDebugProcessMemory(&val, debughandle, addr, sizeof(u32));
            printf("Value is %u\r\n", val);
        }
        else if (!strcmp(argv[2], "u64"))
        {
            u64 val;
            svcReadDebugProcessMemory(&val, debughandle, addr, sizeof(u64));
            printf("Value is %lu\r\n", val);
        }
        else
            {
                printHelp();
                return 0;
            }
        return 0;
    }

    if (!strcmp(argv[0], "lfreeze") || !strcmp(argv[0], "lf"))
    {
        freezeList();
        return 0;
    }

    if (!strcmp(argv[0], "dfreeze") || !strcmp(argv[0], "df"))
    {
        if (argc != 2)
        {
            printHelp();
            return 0;
        }
        u32 index = strtoul(argv[1], NULL, 10);
        freezeDel(index);
        return 0;
    }

    if (!strcmp(argv[0], "afreeze") || !strcmp(argv[0], "af"))
    {
        if (argc != 4)
        {
            printHelp();
            return 0;
        }
        
        u64 addr;
        if (argv[1][0] == '$') {
            int index = strtol(argv[1] + 1, NULL, 10);
            if (index >= 0 && index < searchSize) {
                addr = searchArr[index];
            } else 
            {
                printHelp();
                return 0;
            }
        } else {
            addr = strtoull(argv[1], NULL, 16);
        }

        if (!strcmp(argv[2], "u8"))
        {
            u8 val = strtoul(argv[3], NULL, 10);
            freezeAdd(addr, VAL_U8, val);
        }
        else if (!strcmp(argv[2], "u16"))
        {
            u16 val = strtoul(argv[3], NULL, 10);
            freezeAdd(addr, VAL_U16, val);
        }
        else if (!strcmp(argv[2], "u32"))
        {
            u32 val = strtoul(argv[3], NULL, 10);
            freezeAdd(addr, VAL_U32, val);
        }
        else if (!strcmp(argv[2], "u64"))
        {
            u64 val = strtoull(argv[3], NULL, 10);
            freezeAdd(addr, VAL_U64, val);
        }
        else
        {
            printHelp();
            return 0;
        }
    }    
    
    return 0;
}


/**
 * @brief New search function
 * 
 * @param arg1 DataType (u/s int)
 * @param arg2 Data Value
 * @param argc Arguments Count
 * @return int 0 success 1 failure
 */
int ssearch(char *arg1,char *arg2, int argc)
{
  if (argc != 3)
    {
        printHelp();
        return 0;
    } 

    u8  u8query     = 0;
    u16 u16query    = 0;
    u32 u32query    = 0;
    u64 u64query    = 0;

    s8  s8query     = 0;
    s16 s16query    = 0;
    s32 s32query    = 0;
    s64 s64query    = 0;


    if (!strcmp(arg1, "u8"))
    {
        search = VAL_U8;
        u8query = strtoul(arg2, NULL, 10);
    }
    else if (!strcmp(arg1, "u16"))
    {
        search = VAL_U16;
        u16query = strtoul(arg2, NULL, 10);
    }
    else if (!strcmp(arg1, "u32"))
    {
        search = VAL_U32;
        u32query = strtoul(arg2, NULL, 10);
    }
    else if (!strcmp(arg1, "u64"))
    {
        search = VAL_U64;
        u64query = strtoull(arg2, NULL, 10);
    }
    else if (!strcmp(arg1, "s8"))
    {
        search = VAL_S8;
        u64query = strtoull(arg2, NULL, 10);
    }
    else if (!strcmp(arg1, "s16"))
    {
        search = VAL_S16;
        u64query = strtoull(arg2, NULL, 10);
    }
    else if (!strcmp(arg1, "s32"))
    {
        search = VAL_S32;
        u64query = strtoull(arg2, NULL, 10);
    } 
    else if (!strcmp(arg1, "s64"))
    {
        search = VAL_S64;
        u64query = strtoull(arg2, NULL, 10);
    }
    else
    {
        printHelp();
        return 0;
    }

    MemoryInfo meminfo;
    memset(&meminfo, 0, sizeof(MemoryInfo));

    searchSize = 0;
    u64 lastaddr = 0;
    void *outbuf = malloc(0x40000);
    do
    {
        lastaddr = meminfo.addr;
        u32 pageinfo;
        svcQueryDebugProcessMemory(&meminfo, &pageinfo, debughandle, meminfo.addr + meminfo.size);
        if ((meminfo.perm & Perm_Rw) == Perm_Rw)
        {
            u64 curaddr = meminfo.addr;
            u64 chunksize = 0x40000;

            while (curaddr < meminfo.addr + meminfo.size)
            {
                if (curaddr + chunksize > meminfo.addr + meminfo.size)
                {
                    chunksize = meminfo.addr + meminfo.size - curaddr;
                }

                svcReadDebugProcessMemory(outbuf, debughandle, curaddr, chunksize);
                u8 *u8buf = (u8 *)outbuf;
                u16 *u16buf = (u16 *)outbuf;
                u32 *u32buf = (u32 *)outbuf;
                u64 *u64buf = (u64 *)outbuf;

                s8 *s8buf = (s8 *)outbuf;
                s16 *s16buf = (s16*)outbuf;
                s32 *s32buf = (s32*)outbuf;
                s64 *s64buf = (s64*)outbuf;
                // Switch Case over the searched Value
                switch (search){
                    // Unsigned Cases
                    case VAL_U8:
                        
                        for (u64 i = 0; i < chunksize / sizeof(u8); i++)
                        {
                            if (u8buf[i] == u8query && searchSize < SEARCH_ARR_SIZE)
                            {
                                printf("Got a hit at %lx!\r\n", curaddr + i * sizeof(u8));
                                searchArr[searchSize++] = curaddr + i * sizeof(u8);
                            }
                        }
                        break;

                    case VAL_U16:
                        for (u64 i = 0; i < chunksize / sizeof(u16); i++)
                        {
                            if (u16buf[i] == u16query && searchSize < SEARCH_ARR_SIZE)
                            {
                                printf("Got a hit at %lx!\r\n", curaddr + i * sizeof(u16));
                                searchArr[searchSize++] = curaddr + i * sizeof(u16);
                            }
                        }
                        break;
                    
                    case VAL_U32:
                            
                        for (u64 i = 0; i < chunksize / sizeof(u32); i++)
                        {
                            if (u32buf[i] == u32query && searchSize < SEARCH_ARR_SIZE)
                            {
                                printf("Got a hit at %lx!\r\n", curaddr + i * sizeof(u32));
                                searchArr[searchSize++] = curaddr + i * sizeof(u32);
                            }
                        }
                        break;
                    
                    case VAL_U64:
                        
                        for (u64 i = 0; i < chunksize / sizeof(u64); i++)
                        {
                            if (u64buf[i] == u64query && searchSize < SEARCH_ARR_SIZE)
                            {
                                printf("Got a hit at %lx!\r\n", curaddr + i * sizeof(u64));
                                searchArr[searchSize++] = curaddr + i * sizeof(u64);
                            }
                        }
                        break;

                    case VAL_S8:
                        
                        for (s8 i = 0; i < chunksize / sizeof(s8); i++)
                        {
                            if (s8buf[i] == s8query && searchSize < SEARCH_ARR_SIZE)
                            {
                                printf("Got a hit at %lx!\r\n", curaddr + i * sizeof(s8));
                                searchArr[searchSize++] = curaddr + i * sizeof(s8);
                            }
                        }
                        break;

                    case VAL_S16:

                        for (s16 i = 0; i < chunksize / sizeof(s16); i++)
                        {
                            if (s16buf[i] == s16query && searchSize < SEARCH_ARR_SIZE)
                            {
                                printf("Got a hit at %lx!\r\n", curaddr + i * sizeof(s16));
                                searchArr[searchSize++] = curaddr + i * sizeof(s16);
                            }
                        }
                        break;

                        
                    case VAL_S32:

                        for (s32 i = 0; i < chunksize / sizeof(s32); i++)
                        {
                            if (s32buf[i] == s32query && searchSize < SEARCH_ARR_SIZE)
                            {
                                printf("Got a hit at %lx!\r\n", curaddr + i * sizeof(s32));
                                searchArr[searchSize++] = curaddr + i * sizeof(s32);
                            }
                        }
                        break;

                        
                    case VAL_S64:

                        for (s64 i = 0; i < chunksize / sizeof(s64); i++)
                        {
                            if (s64buf[i] == s64query && searchSize < SEARCH_ARR_SIZE)
                            {
                                printf("Got a hit at %lx!\r\n", curaddr + i * sizeof(s64));
                                searchArr[searchSize++] = curaddr + i * sizeof(s64);
                            }
                        }
                        break;


                }

                curaddr += chunksize;
            }
        }

    } 
    while (lastaddr < meminfo.addr + meminfo.size && searchSize < SEARCH_ARR_SIZE);
    
    if (searchSize >= SEARCH_ARR_SIZE)
    {
        printf("There might be more after this, try getting the variable to a number that's less 'common'\r\n");
    }

    free(outbuf);
    return 0;
}



/**
 * @brief print the usage on the Socket
 * 
 */
void printHelp()
{
    printf("Commands:\r\n"
           "    help                                 | Shows this text\r\n"
           "    ssearch u8/u16/u32/u64 value         | Starts a search with 'value' as the starting-value\r\n"
           "    csearch value                        | Searches the hits of the last search for the new value\r\n"
           "    poke address u8/u16/u32/u64 value    | Sets the memory at address to value\r\n"
           "    peek address u8/u16/u32/u64          | Gets the value at the address\r\n"
           "    afreeze address u8/u16/u32/u64 value | Freezes the memory at address to value\r\n"
           "    lfreeze                              | Lists all frozen values\r\n"
           "    dfreeze index                        | Unfreezes the memory at index\r\n");
}

/**
 * @brief Main Function
 * 
 * @return int 
 */
int main(){

    int listenfd = setupServerSocket();

    char *linebuf = malloc(sizeof(char) * MAX_LINE_LENGTH);

   
    int c = sizeof(struct sockaddr_in);
    struct sockaddr_in client;
    mutexInit(&actionLock);

    Thread freezeThread;
    Result rc = threadCreate(&freezeThread, freezeLoop, NULL, 0x4000, 49, 3);
    if (R_FAILED(rc))
        fatalLater(rc);
    rc = threadStart(&freezeThread);
    if (R_FAILED(rc))
        fatalLater(rc);

    while (appletMainLoop())
    {
        sock = accept(listenfd, (struct sockaddr *)&client, (socklen_t *)&c);
        if (sock <= 0)
        {
            // Accepting fails after sleep for some reason.
            svcSleepThread(1e+9L);
            close(listenfd);
            listenfd = setupServerSocket();
            continue;
        }

        fflush(stdout);
        dup2(sock, STDOUT_FILENO);
        fflush(stderr);
        dup2(sock, STDERR_FILENO);

        printf("Welcome to BetterSNC!\r\n"
               "This needs an atmos-base >= 0.8.2\r\n");

        // Setup and test Ligh Up Home Led Button
        setupPattern(pattern);
        Result _rc;
        lightUpLed(pattern,_rc);

        while (1)
        {
            write(sock, "> ", 2);

            int len = recv(sock, linebuf, MAX_LINE_LENGTH, 0);
            if (len < 1)
            {
                break;
            }

            linebuf[len - 1] = 0;

            mutexLock(&actionLock);
            if (attach(&debughandle)) {
                mutexUnlock(&actionLock);
                continue;
            }

            parseArgs(linebuf, &argmain);

            detach(&debughandle);
            mutexUnlock(&actionLock);

            svcSleepThread(1e+8L);
        }
        detach(&debughandle);
    }

    if (debughandle != 0)
        svcCloseHandle(debughandle);

    return 0;
}
