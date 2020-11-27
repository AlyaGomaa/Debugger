#include <iostream>
#include "Debugger.h"

using namespace std;
#include <string>
int main(int argc, char *argv[])
{

    Debugger dbg;
    if (argc > 1){

        int pid = stoi(argv[1]);
        dbg.attach(pid);
        int x;
        LPVOID printf_address =(LPVOID)  dbg.resolve_function("msvcrt.dll", "printf");

        printf("[*] Address of printf: 0x%08x\n" , printf_address);

        //dbg.software_breakpoint((LPVOID)printf_address);
        if(!dbg.SetHardwareBreakpoint(printf_address , 1 , HW_EXECUTE)){
            printf("setting hardware breakpoint failed");
            return 0;
        }
        dbg.run();

    } 
    // else {
    //     dbg.load("C:\\Windows\\SysWOW64\\calc.exe");
    //     dbg.run();
    //     dbg.detach();
    // }
        return 0;
}
