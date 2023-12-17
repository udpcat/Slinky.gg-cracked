#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>

DWORD GetProcId(const wchar_t* procName) {
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry)) {
            do {
                if (!_wcsicmp(procEntry.szExeFile, procName)) {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return procId;
}

void InjectDLL(DWORD processID, const std::wstring& dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    void* location = VirtualAllocEx(hProcess, 0, (dllPath.size() + 1) * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WriteProcessMemory(hProcess, location, dllPath.c_str(), (dllPath.size() + 1) * sizeof(wchar_t), 0);
    HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "LoadLibraryW"), location, 0, 0);
    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(hProcess, location, (dllPath.size() + 1) * sizeof(wchar_t), MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
}

int main() {
    std::wcout << L"Open source slinky crack injector" << std::endl;
    DWORD processID = GetProcId(L"javaw.exe");

    wchar_t exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    std::wstring basePath(exePath);
    basePath = basePath.substr(0, basePath.find_last_of(L"\\/"));

    const wchar_t* dllNames[] = { L"slinky_library.dll", L"slinkyhook.dll" };
    std::wstring dllPath = basePath + L"\\" + dllNames[0];
    InjectDLL(processID, dllPath);
    for (int i = 1; i < sizeof(dllNames) / sizeof(dllNames[0]); i++) {
        dllPath = basePath + L"\\" + dllNames[i];
        InjectDLL(processID, dllPath);
    }
    std::wcout << L"Injected!" << std::endl;
    Sleep(1000);
    return 0;
}