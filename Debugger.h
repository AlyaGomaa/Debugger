#ifndef DEBUGGER_H
#define DEBUGGER_H
#define HW_ACCESS 0x00000003
#define HW_EXECUTE 0x00000000
#define HW_WRITE 0x00000001
#include <iostream>

#include <windows.h>

#include <iostream>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include "tlhelp32.h"
#include <map>
#include <vector>

class Debugger
{
public:

    Debugger();
    struct HardwareBreakpoint
    {
        LPVOID address;
        unsigned char length;
        unsigned char condition;
    };
    struct MemoryBreakpoint{
        LPVOID address;
        DWORD size;
        _MEMORY_BASIC_INFORMATION memoryBasicInfo;
    };

    //Attributes
    HardwareBreakpoint* hardwareBreakpoints[4];
    std::vector<MemoryBreakpoint> memoryBreakpoints;
    std::map<int, int> breakpoints;
    int pid;
    HANDLE h_thread;
    bool debugger_active;
    HANDLE h_process;
    _CONTEXT context;
    DWORD currentException;
    PVOID exception_address;
    DWORD pageSize;

    //Methods
    HANDLE open_process(int PID);
    int *enumerate_threads();
    HANDLE open_thread(DWORD TID);
    _CONTEXT get_thread_context(HANDLE h_thread);
    std::string read_process_memory(LPCVOID address, SIZE_T length);
    FARPROC resolve_function(LPCSTR dll, LPCSTR function);
    bool write_process_memory(LPVOID address, LPCVOID data);

    //Debugger methods
    void load(char *path_to_exe);
    void attach(int PID);
    void run();
    void get_debug_event();
    bool detach();
    void get_regs();
    bool SetSoftwareBreakpoint(LPVOID address);
    bool SetHardwareBreakpoint(LPVOID address, unsigned char length, unsigned char condition );
    bool SetMemoryBreakpoint(LPVOID,DWORD);
    bool SetMemoryBreakpoint2(LPVOID,DWORD);
    DWORD HandleSoftwareBreakpoint();
    DWORD SingleStep();
    bool DeleteHardwareBreakpoint(int slot);
};

#endif // DEBUGGER_H
