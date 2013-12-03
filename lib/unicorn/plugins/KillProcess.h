/***************************************************************************
 *   Copyright (C) 2005 - 2007 by                                          *
 *      Last.fm Ltd <client@last.fm>                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#ifdef WIN32

#ifndef KILLPROCESS_H
#define KILLPROCESS_H

// This file contains legacy code that doesn't work with Unicode.
#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <tlhelp32.h>
//
// Some definitions from NTDDK and other sources
//

typedef LONG    NTSTATUS;
typedef LONG    KPRIORITY;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)

#define SystemProcessesAndThreadsInformation    5

typedef struct _CLIENT_ID {
    DWORD        UniqueProcess;
    DWORD        UniqueThread;
} CLIENT_ID;

typedef struct _UNICODE_STRING {
    USHORT        Length;
    USHORT        MaximumLength;
    PWSTR        Buffer;
} UNICODE_STRING;

typedef struct _VM_COUNTERS {
    SIZE_T        PeakVirtualSize;
    SIZE_T        VirtualSize;
    ULONG        PageFaultCount;
    SIZE_T        PeakWorkingSetSize;
    SIZE_T        WorkingSetSize;
    SIZE_T        QuotaPeakPagedPoolUsage;
    SIZE_T        QuotaPagedPoolUsage;
    SIZE_T        QuotaPeakNonPagedPoolUsage;
    SIZE_T        QuotaNonPagedPoolUsage;
    SIZE_T        PagefileUsage;
    SIZE_T        PeakPagefileUsage;
} VM_COUNTERS;

typedef struct _SYSTEM_THREADS {
    LARGE_INTEGER   KernelTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   CreateTime;
    ULONG            WaitTime;
    PVOID            StartAddress;
    CLIENT_ID        ClientId;
    KPRIORITY        Priority;
    KPRIORITY        BasePriority;
    ULONG            ContextSwitchCount;
    LONG            State;
    LONG            WaitReason;
} SYSTEM_THREADS, * PSYSTEM_THREADS;

// Note that the size of the SYSTEM_PROCESSES structure is 
// different on NT 4 and Win2K, but we don't care about it, 
// since we don't access neither IoCounters member nor 
//Threads array

typedef struct _SYSTEM_PROCESSES {
    ULONG            NextEntryDelta;
    ULONG            ThreadCount;
    ULONG            Reserved1[6];
    LARGE_INTEGER   CreateTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   KernelTime;
    UNICODE_STRING  ProcessName;
    KPRIORITY        BasePriority;
    ULONG            ProcessId;
    ULONG            InheritedFromProcessId;
    ULONG            HandleCount;
    ULONG            Reserved2[2];
    VM_COUNTERS        VmCounters;
#if _WIN32_WINNT >= 0x500
    IO_COUNTERS        IoCounters;
#endif
    SYSTEM_THREADS  Threads[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;


class CKillProcessHelper
{
private:
    //Functions loaded from Kernel32
    typedef HANDLE (WINAPI *PFCreateToolhelp32Snapshot)(
        DWORD dwFlags,       
        DWORD th32ProcessID  
        );

    typedef BOOL (WINAPI *PFProcess32First)(
        HANDLE hSnapshot,      
        LPPROCESSENTRY32 lppe  
        );

    typedef BOOL (WINAPI *PFProcess32Next)(
        HANDLE hSnapshot,      
        LPPROCESSENTRY32 lppe  
        );

    // Native NT API Definitions
    typedef NTSTATUS (WINAPI * PFZwQuerySystemInformation)
        (UINT, PVOID, ULONG, PULONG);
    typedef HANDLE (WINAPI* PFGetProcessHeap)(VOID);
    typedef LPVOID (WINAPI* PFHeapAlloc)
        (HANDLE,DWORD,SIZE_T);
    typedef BOOL (WINAPI* PFHeapFree)(HANDLE,DWORD,LPVOID);

public:
    CKillProcessHelper() : FCreateToolhelp32Snapshot(NULL), 
        FProcess32First(NULL), FProcess32Next(NULL), 
        m_hKernelLib(NULL),
        m_hNTLib(NULL)
    {
        m_hKernelLib = ::LoadLibraryA("Kernel32");
        if (m_hKernelLib)  
        {
            // Find ToolHelp functions
            FCreateToolhelp32Snapshot = 
                (PFCreateToolhelp32Snapshot)
                ::GetProcAddress(m_hKernelLib,
                ("CreateToolhelp32Snapshot"));
            FProcess32First = (PFProcess32First)
                ::GetProcAddress(m_hKernelLib, 
                ("Process32First"));
            FProcess32Next = (PFProcess32Next)
                ::GetProcAddress(m_hKernelLib, 
                ("Process32Next"));
        }
        if(!FCreateToolhelp32Snapshot || 
            !FProcess32First || !FProcess32Next)
        { // i.e. we couldn't find the ToolHelp functions, 
            //so we must be on NT4. Let's load the
            // undocumented one instead.
            if(!m_hKernelLib)
                return; // can't do anything at all without 
            //the kernel.

            m_hNTLib = ::LoadLibraryA("ntdll.dll");
            if(m_hNTLib)
            {
                FQuerySysInfo = 
                    (PFZwQuerySystemInformation)
                    ::GetProcAddress(m_hNTLib, 
                    ("ZwQuerySystemInformation"));
                // load some support funcs from the kernel
                FGetProcessHeap = (PFGetProcessHeap)
                    ::GetProcAddress(m_hKernelLib, 
                    ("GetProcessHeap"));
                FHeapAlloc = (PFHeapAlloc)
                    ::GetProcAddress(m_hKernelLib, 
                    ("HeapAlloc"));
                FHeapFree = (PFHeapFree)
                    ::GetProcAddress(m_hKernelLib, 
                    ("HeapFree"));
            }
        }
    }
    ~CKillProcessHelper()
    {
        if(m_hKernelLib)
            FreeLibrary(m_hKernelLib);
        if(m_hNTLib)
            FreeLibrary(m_hNTLib);
    }
    bool KillProcess(IN const char* pstrProcessName)
    {
        DWORD dwId;
        HANDLE hProcess = FindProcess(pstrProcessName, 
            dwId);
        BOOL bResult;
        if(!hProcess)
            return false;

        // TerminateAppEnum() posts WM_CLOSE to all windows whose PID
        // matches your process's.
        ::EnumWindows((WNDENUMPROC)
            CKillProcessHelper::TerminateAppEnum, 
            (LPARAM) dwId);
        // Wait on the handle. If it signals, great. 
        //If it times out, then you kill it.
        if(WaitForSingleObject(hProcess, 5000)
            !=WAIT_OBJECT_0)
            bResult = TerminateProcess(hProcess,0);
        else
            bResult = TRUE; 
        CloseHandle(hProcess);
        return static_cast<bool>(bResult != FALSE);
    }
    HANDLE FindProcess(IN const char* pstrProcessName,
    OUT DWORD& dwId)
    {
        if(!m_hKernelLib)
            return NULL;

        if(FCreateToolhelp32Snapshot && FProcess32First && 
            FProcess32Next) // use toolhelpapi
            return THFindProcess(pstrProcessName, dwId);
        if(FQuerySysInfo && FHeapAlloc && 
            FGetProcessHeap && FHeapFree) // use NT api
            return NTFindProcess(pstrProcessName, dwId);
        // neither one got loaded. Strange.
        return NULL;
    }

private:
    HANDLE THFindProcess(IN const char* pstrProcessName, 
        OUT DWORD& dwId)
    {
        HANDLE            hSnapShot=NULL;
        HANDLE            hResult = NULL;
        PROCESSENTRY32    processInfo;
        char*            pstrExeName;

        bool bFirst = true;
        ::ZeroMemory(&processInfo, sizeof(PROCESSENTRY32));
        processInfo.dwSize = sizeof(PROCESSENTRY32);
        hSnapShot = FCreateToolhelp32Snapshot(
            TH32CS_SNAPPROCESS, 0);
        if(hSnapShot == INVALID_HANDLE_VALUE)
            return NULL; 

        // ok now let's iterate with Process32Next until we 
        // match up the name of our process
        while((bFirst ? FProcess32First(hSnapShot, 
            &processInfo) : FProcess32Next(hSnapShot, 
            &processInfo)))
        {
            bFirst = false;
            // we need to check for path... and extract 
            // just the exe name
            pstrExeName = strrchr(processInfo.szExeFile, 
                '\\');
            if(!pstrExeName)
                pstrExeName = processInfo.szExeFile;
            else
                pstrExeName++; // skip the backslash
            // ok now compare against our process name
            if(_stricmp(pstrExeName, pstrProcessName) == 0)
                // wee weee we found it
            {
                // let's get a HANDLE on it
                hResult=OpenProcess(
                    SYNCHRONIZE|PROCESS_TERMINATE, TRUE, 
                    processInfo.th32ProcessID);
                dwId = processInfo.th32ProcessID;
                break;
            }
        } // while(Process32Next(hSnapShot, &processInfo){
        if(hSnapShot)
            CloseHandle(hSnapShot);
        return hResult;
    }
    HANDLE NTFindProcess(IN const char* pstrProcessName, 
        OUT DWORD& dwId)
    {
        HANDLE hHeap = FGetProcessHeap();
        NTSTATUS Status;
        ULONG cbBuffer = 0x8000;
        PVOID pBuffer = NULL;
        HANDLE hResult = NULL;
        // it is difficult to say a priory which size of 
        // the buffer will be enough to retrieve all 
        // information, so we startwith 32K buffer and 
        // increase its size until we get the
        // information successfully
        do
        {
            pBuffer = HeapAlloc(hHeap, 0, cbBuffer);
            if (pBuffer == NULL)
                return SetLastError(
                ERROR_NOT_ENOUGH_MEMORY), NULL;

            Status = FQuerySysInfo(
                SystemProcessesAndThreadsInformation,
                pBuffer, cbBuffer, NULL);

            if (Status == STATUS_INFO_LENGTH_MISMATCH)
            {
                HeapFree(hHeap, 0, pBuffer);
                cbBuffer *= 2;
            }
            else if (!NT_SUCCESS(Status))
            {
                HeapFree(hHeap, 0, pBuffer);
                return SetLastError(Status), NULL;
            }
        }
        while (Status == STATUS_INFO_LENGTH_MISMATCH);

        PSYSTEM_PROCESSES pProcesses = 
            (PSYSTEM_PROCESSES)pBuffer;

        for (;;)
        {
            PCWSTR pszProcessName = 
                pProcesses->ProcessName.Buffer;
            if (pszProcessName == NULL)
                pszProcessName = L"Idle";

            CHAR szProcessName[MAX_PATH];
            WideCharToMultiByte(CP_ACP, 0, pszProcessName, 
                -1,szProcessName, MAX_PATH, NULL, NULL);

            if(_stricmp(szProcessName, pstrProcessName)
                == 0) // found it
            {
                hResult=OpenProcess(
                    SYNCHRONIZE|PROCESS_TERMINATE, TRUE, 
                    pProcesses->ProcessId);
                dwId = pProcesses->ProcessId;
                break;
            }

            if (pProcesses->NextEntryDelta == 0)
                break;

            // find the address of the next process 
            // structure
            pProcesses = (PSYSTEM_PROCESSES)(
                ((LPBYTE)pProcesses)
                + pProcesses->NextEntryDelta);
        }

        HeapFree(hHeap, 0, pBuffer);
        return hResult;
    }
    // callback function for window enumeration
    static BOOL CALLBACK TerminateAppEnum( HWND hwnd, 
        LPARAM lParam )
    {
        DWORD dwID ;

        GetWindowThreadProcessId(hwnd, &dwID) ;

        if(dwID == (DWORD)lParam)
        {
            PostMessage(hwnd, WM_CLOSE, 0, 0) ;
        }

        return TRUE ;
    }
    HMODULE            m_hNTLib;
    HMODULE            m_hKernelLib;
    // ToolHelp related functions
    PFCreateToolhelp32Snapshot    FCreateToolhelp32Snapshot;
    PFProcess32First            FProcess32First;
    PFProcess32Next                FProcess32Next;
    // native NT api functions
    PFZwQuerySystemInformation    FQuerySysInfo;
    PFGetProcessHeap            FGetProcessHeap;
    PFHeapAlloc                    FHeapAlloc;
    PFHeapFree                    FHeapFree;
};

#endif // KILLPROCESS_H

#endif // WIN32
