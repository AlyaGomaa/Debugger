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
    HardwareBreakpoint* hardwareBreakpoints[4];

    HANDLE open_process(int PID);
    void load(char *path_to_exe);
    void attach(int PID);
    //void print_array(int arr[]);
    void run();
    void get_debug_event();
    DWORD HandleSoftwareBreakpoint();
    bool detach();
    int *enumerate_threads();
    HANDLE open_thread(DWORD TID);
    _CONTEXT get_thread_context(HANDLE h_thread);
    void get_regs();
    std::string read_process_memory(LPCVOID address, SIZE_T length);
    FARPROC resolve_function(LPCSTR dll, LPCSTR function);
    bool write_process_memory(LPVOID address, LPCVOID data);
    bool SetSoftwareBreakpoint(LPVOID address);
    bool SetHardwareBreakpoint(LPVOID address, unsigned char length, unsigned char condition );
    
    DWORD SingleStep();
    bool DeleteHardwareBreakpoint(int slot);
};

#endif // DEBUGGER_H
