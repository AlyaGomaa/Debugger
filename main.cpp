#include <iostream>
#include "Debugger.h"

using namespace std;
#include <string>
typedef int(__stdcall *printf_ex)(const char*, ...);
int main(int argc, char *argv[])
{

    Debugger dbg;
    if (argc > 1){

        int pid = stoi(argv[1]);
        dbg.attach(pid); // calss get regs
        int x;
        printf_ex printf_address =(printf_ex) dbg.resolve_function("msvcrt.dll", "printf");
        printf_address("HelloWorld from here\n");
        printf("[*] Address of printf: 0x%08x\n" , printf_address);
        dbg.bp_set((LPVOID)printf_address);
        dbg.run();

    } else {
        dbg.load("C:\\Windows\\SysWOW64\\calc.exe"); //// calss get regs
        dbg.run();
        dbg.detach();
    }

//cout << "Enter the PID of the process to attach to: ";

//cin >> pid;



    //dbg.load("C:\\Users\\alya\\Desktop\\a.exe");
  
        int x;
        cin >> x;
        return 0;
}
