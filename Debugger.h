#ifndef DEBUGGER_H
#define DEBUGGER_H
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
    DWORD handle_breakpoint();
    bool detach();
    int *enumerate_threads();

    HANDLE open_thread(DWORD TID);
    _CONTEXT get_thread_context(HANDLE h_thread);

    void get_regs();

    std::string read_process_memory(LPCVOID address, SIZE_T length);


    bool write_process_memory(LPVOID address, LPCVOID data);
    bool bp_set(LPVOID address);
    bool SetHardwareBreakpoint(LPVOID ,unsigned char , unsigned char );
    FARPROC resolve_function(LPCSTR dll, LPCSTR function);

};

#endif // DEBUGGER_H
