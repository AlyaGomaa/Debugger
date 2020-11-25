#include <iostream>
#include "Debugger.h"

using namespace std;
#include <string>
int main(int argc, char *argv[])
{

    Debugger dbg;
    if (argc > 1){

        int pid = stoi(argv[1]);
        dbg.attach(pid); // calss get regs

        FARPROC printf_address = dbg.resolve_function("msvcrt.dll", "printf");
        printf("[*] Address of printf: 0x%08x" , printf_address);

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
