// the og injector was closed source so i made my own ;)
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>

DWORD GetProcId(const char* procName) {
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry)) {
            do {
                if (!_stricmp(procEntry.szExeFile, procName)) {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return procId;
}

void InjectDLL(DWORD processID, const std::string& dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    void* location = VirtualAllocEx(hProcess, 0, dllPath.size() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WriteProcessMemory(hProcess, location, dllPath.c_str(), dllPath.size() + 1, 0);
    HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("Kernel32.dll"), "LoadLibraryA"), location, 0, 0);
    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(hProcess, location, dllPath.size() + 1, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
}

int main() {
    std::cout << "Open source slinky crack injector" << std::endl;
    DWORD processID = GetProcId("javaw.exe");

    // c++ doesnt like direct names so we need to get the path of the exe ;(
    char exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    std::string basePath(exePath);
    basePath = basePath.substr(0, basePath.find_last_of("\\/"));

    const char* dllNames[] = {"slinky_library.dll", "slinkyhook.dll"};
    std::string dllPath = basePath + "\\" + dllNames[0];
    InjectDLL(processID, dllPath);
    for (int i = 1; i < sizeof(dllNames) / sizeof(dllNames[0]); i++) {
        dllPath = basePath + "\\" + dllNames[i];
        InjectDLL(processID, dllPath);
    }
    std::cout << "Injected!" << std::endl;
    Sleep(1000);
    return 0;
}