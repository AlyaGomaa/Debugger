#include <iostream>


#include "Debugger.h"

using namespace std;
#include <string>


static int pid;
static HANDLE h_thread;
static bool debugger_active;
static HANDLE h_process;
static _CONTEXT context;
static DWORD currentException;
static PVOID exception_address;
static std::map<int, int> breakpoints;

// i686-w64-mingw32-g++ main.cpp Debugger.cpp -m32 -o ../DBG.exe
Debugger::Debugger(){
    for(int i=0;i<4;i++){
        this->hardwareBreakpoints[i]=NULL;
    }
}


HANDLE  Debugger::open_process(int PID)
{
    return OpenProcess(PROCESS_ALL_ACCESS, false, PID);
}
void Debugger::load(char *path_to_exe)
{
    cout << "Now attaching to: " << path_to_exe << endl;

    STARTUPINFO startup_info;
    PROCESS_INFORMATION process_information;
    memset(&startup_info, 0, sizeof(startup_info));               //initialize a struct
    memset(&process_information, 0, sizeof(process_information)); // da el debugger bymlah hwa
    startup_info.dwFlags = 0x1;
    startup_info.wShowWindow = 0x0;
    startup_info.cb = sizeof(startup_info);
    if (CreateProcess(NULL,                  // No module name (use command line)
                      path_to_exe,           // Command line
                      NULL,                  // Process handle not inheritable
                      NULL,                  // Thread handle not inheritable
                      FALSE,                 // Set handle inheritance to FALSE
                      DEBUG_PROCESS,         // freezes the process you can use CREATE_NEW_CONSOLE, it doesnt show the gui of the proc yet bc its waiting for the debugger to continue execution
                      NULL,                  // Use parent's environment block
                      NULL,                  // Use parent's starting directory
                      &startup_info,         // Pointer to STARTUPINFO structure
                      &process_information)) // Pointer to PROCESS_INFORMATION structur
    {
        cout << "PID: " << process_information.dwProcessId << endl;
        pid = process_information.dwProcessId;
        debugger_active = true;
        h_process = open_process(process_information.dwProcessId);
        get_regs();
    }
    else
    {
        cout << "Can't Create the process.. Error: " << GetLastError() << endl;
    }
}


void Debugger::attach(int PID)
{


    h_process = open_process(PID);
    cout << "opened handle: " << h_process << endl;

    if (DebugActiveProcess(PID))
    {
        debugger_active = true;
        pid = PID;

        get_regs();
    }
    else
    {
        cout << "Error Attaching to the process " << PID << "\nError code: " << GetLastError() << endl;
    }
}

void Debugger::run()
{ 
    while (debugger_active)
    {
        get_debug_event();
    }
}

void Debugger::get_debug_event()
{
    /* If a debugging event occurs while a debugger is waiting for one, the system fills the DEBUG_EVENT structure specified by WaitForDebugEvent with information describing the event.*/
    DEBUG_EVENT debug_event; //gets populated whenever an event occurs
    memset(&debug_event, 0, sizeof(debug_event));
    DWORD continue_status=DBG_CONTINUE;

    bool event = WaitForDebugEvent(&debug_event, INFINITE);

    if (event)
    {
        
        h_thread = open_thread(debug_event.dwThreadId);
        context = get_thread_context(h_thread);
        printf("Event Code: %d Thread ID: %d\n", debug_event.dwDebugEventCode, debug_event.dwThreadId);

       // cout << " debug_event.dwDebugEventCode: " << debug_event.dwDebugEventCode << " EXCEPTION_DEBUG_EVENT: " << EXCEPTION_DEBUG_EVENT << endl;

    //EXCEPTION_DEBUG_EVENT is

        if (debug_event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) // a windows - driven breakpoint always executed to allow a debugger to inspect the process's state before resuming execution

        { // 0x1 ==  EXCEPTION_DEBUG_EVENT law el event ely 7sal kan lo 3laqa bel debugger

            //Obtain the exception code
            currentException = debug_event.u.Exception.ExceptionRecord.ExceptionCode; // access violation , bp , index outta range etc.
            exception_address = debug_event.u.Exception.ExceptionRecord.ExceptionAddress;
            
            if (currentException == EXCEPTION_ACCESS_VIOLATION)
            {
                
                cout << "Access Violation Detected." << endl;
                // If a breakpoint is detected, we call an internal handler.
                // han handle hena a bp if one occured
            }
            else if (currentException == EXCEPTION_BREAKPOINT) // software breakpoints
            {
                continue_status = HandleSoftwareBreakpoint();
            }
            else if (currentException == EXCEPTION_GUARD_PAGE) //  memory breakpoints
            { 
                cout << "Guard Page Access Detected." << endl;
            }
            else if (currentException == EXCEPTION_SINGLE_STEP) // hardware breakpoint
            {
                cout << "Single Stepping." << endl;
                // Since HW Breakpoints cause an INT1 (Single Step) exception when the given address is accessed
                continue_status = SingleStep();
            }
        }

        // When the system notifies the debugger of an event, it suspends all threads in the affected process.The threads do not resume execution until the debugger continues the debugging event
        /*
        The dwContinueStatus parameter signals the process to one of the following:
        1 - continue executing (DBG_CONTINUE)
        2 - to continue processing the exception(DBG_EXCEPTION_NOT_HANDLED). */

        ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, continue_status);
    }
}
DWORD Debugger::HandleSoftwareBreakpoint()
{ // and return the continue status
    cout << "[*] Inside the breakpoint handler." << endl;
    printf("Exception Address: 0x%x\n",exception_address);
    return DBG_CONTINUE;
}
bool Debugger::detach()
{
    if (DebugActiveProcessStop(pid))
    {
        cout << "Exiting..." << endl;
        return true;
    }
    else
    {
        cout << "Something Went Wrong, Can't detach!" << endl;
        return false;
    }
}
/* #Working with threads:
*/
/* To obtain CPU Regs We must obtain a list of all the threads that are executing inside the
#process, select the thread we want, and obtain a valid handle to it using
#OpenThread().*/

int *Debugger::enumerate_threads()
{

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid); //get all currently registered threads in the snapshot
    int *p_thread_list;
    int thread_list[30];
    if (snapshot)
    {

        memset(&thread_list, 0, sizeof(thread_list));
        THREADENTRY32 thread_entry;
        //This structure gets populated when Thread32First() call completes
        thread_entry.dwSize = sizeof(thread_entry); //you have to set the size of the struct or the call will fail
        bool success = Thread32First(snapshot, &(thread_entry));
        int i = 0;
        while (success)
        {

            if (thread_entry.th32OwnerProcessID == pid)
            {
                thread_list[i] = thread_entry.th32ThreadID;
                i += 1;
            }
            success = Thread32Next(snapshot, &(thread_entry));
        }
        CloseHandle(snapshot);

        p_thread_list = thread_list;
        return p_thread_list;
    }
    else
    {
        cout << "enumerate threads failed\n";
        return NULL;
    }
}

HANDLE Debugger::open_thread(DWORD TID)
{ //get_thread_handle
    h_thread = OpenThread(THREAD_ALL_ACCESS, false, TID);
    if (h_thread == NULL)
    {
        cout << "[*] Could not obtain a valid thread handle" << endl;
        
    }
    return h_thread;
}

//GetThreadContext to get the CPU regss of a specific thread
_CONTEXT Debugger::get_thread_context(HANDLE h_thread)
{
    _CONTEXT context;

    memset(&context, 0, sizeof(context));
    context.ContextFlags = 0x00010000b | CONTEXT_DEBUG_REGISTERS;

    if (!GetThreadContext(h_thread, &(context)))
    {
        cout << "get_thread_context failed." << endl;
    }
    return context;
}

void Debugger::get_regs()
{

    int *thread_list = enumerate_threads();
    for (int i = 0; i < sizeof(thread_list) / sizeof(thread_list[0]); i++)
    {
        DWORD tid = *(thread_list + i); //thread_list[i];
        HANDLE h_thread = open_thread(tid);
        // first we need tto get the thread context
        _CONTEXT context;
        memset(&context, 0, sizeof(context));
        context.ContextFlags = 0x00010000b | CONTEXT_DEBUG_REGISTERS;

        if (!GetThreadContext(h_thread, &(context)))
        {
            cout << "get_thread_context failed. Err no:" << GetLastError() << endl;
        }
        else
        {

            printf("[*] Dumping registers for thread ID: %d", tid);
            printf("\n[**] RIP: 0x%08x", context.Eip);
            printf("\n[**] RSP: 0x%08x", context.Esp);
            printf("\n[**] RBP: 0x%08x", context.Ebp);
            printf("\n[**] RAX: 0x%08x", context.Eax);
            printf("\n[**] RBX: 0x%08x", context.Ebx);
            printf("\n[**] RCX: 0x%08x", context.Ecx);
            printf("\n[**] RDX: 0x%08x", context.Edx);
            printf("\n[*] END DUMP");
            printf("\n--------------------------------------------------\n");
        }
    }
}
/*0x1 EXCEPTION_DEBUG_EVENT u.Exception
0x2 CREATE_THREAD_DEBUG_EVENT u.CreateThread
0x3 CREATE_PROCESS_DEBUG_EVENT u.CreateProcessInfo
0x4 EXIT_THREAD_DEBUG_EVENT u.ExitThread
0x5 EXIT_PROCESS_DEBUG_EVENT u.ExitProcess
0x6 LOAD_DLL_DEBUG_EVENT u.LoadDll
0x7 UNLOAD_DLL_DEBUG_EVENT u.UnloadDll
0x8 OUPUT_DEBUG_std::string_EVENT u.Debugstd::string
0x9 RIP_EVENT u.RipInfo*/
std::string Debugger::read_process_memory(LPCVOID address, SIZE_T length)
{
    char bytes_read[1024];
    //DWORD NumberOfBytesRead;
    if (ReadProcessMemory(h_process, address, &bytes_read, length, NULL))
    {
        
        std::string Bytes_read(bytes_read);
        return Bytes_read ;
        
    }
    else
    {
        return "";
    }
}

bool Debugger::write_process_memory(LPVOID address, LPCVOID data)
{
   // DWORD NumberOfBytesWritten;
    SIZE_T size = strlen((char*)data) +1;
    if(WriteProcessMemory(h_process, address, data, size, NULL)){
        return true;
    } else {
        return false;
    }
}

FARPROC Debugger::resolve_function(LPCSTR dll, LPCSTR function)
{

    HMODULE module_handle = GetModuleHandleA(dll);
    FARPROC address = GetProcAddress(module_handle, function);
    CloseHandle(module_handle);

    if ((module_handle && address))
    {
        return address;
    }
    else
    {
        cout << "Coudnt resolve function" << endl;
        return 0;
    }
}
bool Debugger::SetSoftwareBreakpoint(LPVOID address)
{
    std::map<int, int>::iterator it =  breakpoints.find((int) address);
    
    if ( it == breakpoints.end() ){ // breakpoint isn't registered
        printf("Breakpoint added @0x%x",address);
       try{
        std::string original_byte = read_process_memory(address, 1); // store the original byte
        write_process_memory(address, "\xcc");                  // write the INT3 opcode
        breakpoints[(int)address] = (int)original_byte[0];        //register the breakpoint in our internal list
       } catch (...){
           cout << "something went wrong while setting the bp, Error code: " << GetLastError()  << endl;
           return false;
       }
    }
    return true;


}


bool Debugger::SetHardwareBreakpoint(LPVOID address, unsigned char length, unsigned char condition)
{

    int available = -1;
    // Check length
    if (length != 1 && length != 2 && length != 4)
    {
        printf("[ERROR][SetHardwareBreakpoint] length cant be %d",length);
        return false;
    }
    else
    {
        length -= 1; //to fit in 2 bits
    }

    if (condition != HW_ACCESS && condition != HW_EXECUTE && condition != HW_WRITE)
    {
        printf("[ERROR][SetHardwareBreakpoint] condition not supported\n");
        return false;
    }
    //  Check for available slots
    for (int i = 0; i < 4; i++)
    {
        if (hardwareBreakpoints[i] == NULL)
        {
            available = i;
            break;
        }
    }

    if (available == -1)
    {
        printf("[ERROR][SetHardwareBreakpoint] no available hardware breakpoints\n");
        return false;
    }
    // Register a new hardware breakpoint
    HardwareBreakpoint *hwBreakpoint = new HardwareBreakpoint;
    hwBreakpoint->address = address;
    hwBreakpoint->length = length;
    hwBreakpoint->condition = condition;
    
    int *thread_list = enumerate_threads();
    for (int i = 0; i < sizeof(thread_list) / sizeof(thread_list[0]); i++)
    {
        DWORD tid = *(thread_list + i); //thread_list[i];
        HANDLE h_thread = open_thread(tid);

        // first we need to get the thread context
        _CONTEXT context;
        memset(&context, 0, sizeof(context));
        context.ContextFlags = 0x00010000b | CONTEXT_DEBUG_REGISTERS;

        if (!GetThreadContext(h_thread, &(context)))
        {
            cout << "Can't get thread context. Err no:" << GetLastError() << endl;
            delete hwBreakpoint;
            CloseHandle(h_thread);
            return false;
        }
        // Enable the appropriate flag in the DR7 register to set the breakpoint (0-7)
        context.Dr7 |= 1 << (available * 2);

        //# Save the address of the breakpoint in the free register that we found
        switch (available)
        {
        case 0:
            context.Dr0 = (DWORD32)address;
            break;
        case 1:
            context.Dr1 = (DWORD32)address;
            break;
        case 2:
            context.Dr2 = (DWORD32)address;
            break;
        case 3:
            context.Dr3 = (DWORD32)address;
            break;
        default:
            printf("[ERROR][SetHardwareBreakpoint] something went wrong\n");
        }
        // set the condition
        context.Dr7 |= condition << (available * 4 + 16);

        // Set the length
        context.Dr7 |= length << (available * 4 + 18);

        if (!SetThreadContext(h_thread, &context))
        {
            cout << "[ERROR][SetHardwareBreakpoint] can't set thread context. Err no:" << GetLastError() << endl;
            delete hwBreakpoint;
            CloseHandle(h_thread);
            return false;
        }    
    }
    hardwareBreakpoints[available] = hwBreakpoint;
    CloseHandle(h_thread);
    return true;
}

DWORD Debugger::SingleStep() //exception_handler_single_step
{
    //DR6  : status register.  determines the type of debugging event triggered by the breakpoint once it is hit
    // determine if this single step event occurred in reaction to a hardware breakpoint and grab the hit breakpoint.
    short slot;
    DWORD continue_status = DBG_CONTINUE;
    if ((context.Dr6 & 0x1) && hardwareBreakpoints[0] != NULL)
    {
        slot =0;
    }
    else if ((context.Dr6 & 0x2) && hardwareBreakpoints[1] != NULL)
    {
        slot =2;
    }
    else if ((context.Dr6 & 0x4) && hardwareBreakpoints[2] != NULL)
    {
        slot = 3;
    }
    else if ((context.Dr6 & 0x8) && hardwareBreakpoints[3] != NULL)
    {
        slot = 4;
    }
    else
    {
        //  This wasn't an INT1 generated by a hw breakpoint
        continue_status = DBG_EXCEPTION_NOT_HANDLED; // continue processing the exception
    }

    if (DeleteHardwareBreakpoint(slot)){ //  remove the breakpoint from the list if it was a hw bp
        continue_status = DBG_CONTINUE; // continue executing
    }
    cout << "[*] Hardware breakpoint removed." << endl;
    return continue_status;
}
bool Debugger::DeleteHardwareBreakpoint(int slot)
{
    int *thread_list = enumerate_threads();
    for (int i = 0; i < sizeof(thread_list) / sizeof(thread_list[0]); i++)
    {
        DWORD tid = *(thread_list + i); //thread_list[i];
       
        HANDLE h_thread = OpenThread(THREAD_ALL_ACCESS, false , tid );
        if (h_thread == NULL){
            printf("failed to open thread. Error code: %d", GetLastError());
            return false;
        }

        // first we need to get the thread context
        _CONTEXT context;
        memset(&context, 0, sizeof(context));
        context.ContextFlags = 0x00010000b | CONTEXT_DEBUG_REGISTERS;

        if (!GetThreadContext(h_thread, &(context)))
        {
            cout << "Can't get thread context. Err no:" << GetLastError() << endl;
            CloseHandle(h_thread);
            return false;
        }
        
        // Reset the flags to remove the breakpoint
        context.Dr7 &= ~(1 << (slot * 2));
        switch (slot){
            case 0:
                context.Dr0 = 0x00000000;
            case 1 :
                context.Dr1 = 0x00000000;
            case 2 : 
                context.Dr2 = 0x00000000;
            case 3:
                context.Dr3 = 0x00000000;
        }
        //Remove the condition flag
        context.Dr7 &= ~(3 << ((slot * 4) + 16));
        //Remove the length flag
        context.Dr7 &= ~(3 << ((slot * 4) + 18));
        SetThreadContext(h_thread,&context);
        CloseHandle(h_thread);
    }
    //remove the breakpoint from the internal list.
    delete hardwareBreakpoints[slot];
    return true;
}