#include <stdio.h>
#include <Windows.h>
#include <processthreadsapi.h>
#include <map>

int main(){
    while(true){
        printf("Hello from process %d\n",GetCurrentProcessId());
        printf("Printf address 0x%x\n",GetProcAddress(GetModuleHandleA("msvcrt.dll"),"printf"));
        Sleep(3000);
    }
}