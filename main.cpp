#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
/*
amarenahook injector
by kAzan
support: http://amarena.space/discord
*/
using namespace std;


DWORD GetProcessIdByName(CHAR* Name);
BOOL FileExists(CHAR* Path);
BOOL InjectDLL(DWORD PID, CHAR* Path);

INT main()
{
    
    CHAR ProcessName[MAX_PATH], LibraryName[MAX_PATH], LibraryPath[MAX_PATH] = {0};
    DWORD PID = 0;

    while(1)
    {
        cout << "Welcome to the amarenahook injector!" <<endl;
        cout << "Process: " <<endl;
        cin >> ProcessName;
        cout << "DLL: " <<endl;
        cin >> LibraryName;

        if((PID = GetProcessIdByName(ProcessName)) != 0)
        {
            GetFullPathName(LibraryName, MAX_PATH, LibraryPath, NULL);

            if(FileExists(LibraryPath))
            {
                cout << (InjectDLL(PID, LibraryPath) ? "Injected successfully" : "Error! Try again!") << endl;
            }
            else
            {
                cout << ".dll not found." << endl;
            }
        }
        else
        {
            cout << "Process not found. Make sure it's running." << endl;
        }
    }

    return EXIT_SUCCESS;
}

DWORD GetProcessIdByName(CHAR* Name)
{
    PROCESSENTRY32 pe;
    HANDLE Snapshot;
    DWORD ret = 0;

    Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pe.dwSize = sizeof(PROCESSENTRY32);

    if(Snapshot != INVALID_HANDLE_VALUE)
    {
        if(Process32First(Snapshot, &pe))
        {
            do
            {
                if(!lstrcmpi(Name, pe.szExeFile))
                {
                    ret = pe.th32ProcessID;
                    break;
                }
            }
            while(Process32Next(Snapshot, &pe));
        }

        CloseHandle(Snapshot);
    }

    return ret;
}

BOOL FileExists(CHAR* Path)
{
    DWORD Attributes = GetFileAttributes(Path);

    return (Attributes != INVALID_FILE_ATTRIBUTES && !(Attributes & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL InjectDLL(DWORD PID, CHAR* Path)
{
    HANDLE Process, Thread;
    LPVOID RemoteMemoryAddress;
    LPTHREAD_START_ROUTINE LoadLibraryAddress;
    BOOL ret = FALSE;

    if((Process = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, FALSE, PID)) != NULL)
    {
        DWORD MemorySize = strlen(Path) + 1;

        if((RemoteMemoryAddress = VirtualAllocEx(Process, NULL, MemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READ)) != NULL)
        {
            if(WriteProcessMemory(Process, RemoteMemoryAddress, Path, MemorySize, NULL))
            {
                LoadLibraryAddress = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

                if((Thread = CreateRemoteThread(Process, NULL, 0, LoadLibraryAddress, RemoteMemoryAddress, 0, NULL)) != NULL)
                {
                    ret = TRUE;
                    CloseHandle(Thread);
                }
            }

        }

        CloseHandle(Process);
    }

    return ret;
}
