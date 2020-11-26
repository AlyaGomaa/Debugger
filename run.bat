echo off
g++ .\main.cpp .\Debugger.cpp -m32 -w -o debugger.exe
set arg1=%1
debugger.exe %1