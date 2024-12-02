#include "runner.h"
#include "../common/util.h"

#include <fstream>

#include "env_manager.h"

#include <iostream>
#include <string>

using namespace std::literals;

#define MKPTR(p1,p2) ((DWORD_PTR)(p1) + (DWORD_PTR)(p2))

Path Runner::helper_exe_path () {
    return working_dir() / HELPER_EXE_NAME;
}

FARPROC Runner::get_remote_kernel32_LoadLibraryW_address () {
    // create x86 helper process
    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    LPCWSTR exe_path_str = helper_exe_path().c_str();

    if (!CreateProcessW(exe_path_str, nullptr, nullptr,
                        nullptr, FALSE, CREATE_NO_WINDOW,
                        nullptr, nullptr, &si, &pi)) {
        std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
        throw std::runtime_error("CreateProcess failed");
    }

    // create named pipe
    HANDLE hPipe = CreateNamedPipeW(
        PIPE_NAME, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
        512, 512, NMPWAIT_WAIT_FOREVER, nullptr);

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateNamedPipe failed: " << GetLastError() << std::endl;
        throw std::runtime_error("CreateNamedPipe failed");
    }

    // connect to named pipe
    if (!ConnectNamedPipe(hPipe, nullptr)) {
        std::cerr << "ConnectNamedPipe failed: " << GetLastError() << std::endl;
        throw std::runtime_error("ConnectNamedPipe failed");
    }

    // wait for child process to send kernel32.dll LoadLibraryW address
    DWORD dwRead;
    FARPROC pfLoadLibraryW;
    if (!ReadFile(hPipe, &pfLoadLibraryW, sizeof(pfLoadLibraryW), &dwRead, nullptr)) {
        std::cerr << "ReadFile failed: " << GetLastError() << std::endl;
        throw std::runtime_error("ReadFile failed");
    }

    // disconnect from named pipe
    DisconnectNamedPipe(hPipe);

    // close handle
    CloseHandle(hPipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return pfLoadLibraryW;
}


/// from https://github.com/SegaraRai/InjectExec
void Runner::inject_run (const String &exePath, const String &dllPath, const bool is_x64,
                         const bool suspendThread = true, const bool waitProcess = true) {
    FARPROC pfLoadLibraryW;
    if (is_x64) {
        pfLoadLibraryW = get_kernel32_LoadLibraryW_address();
        if (!pfLoadLibraryW) {
            std::wcerr << L"get_kernel32_LoadLibraryW_address failed"sv << std::endl;
            throw std::runtime_error("get_kernel32_LoadLibraryW_address failed");
        }
    } else {
        pfLoadLibraryW = get_remote_kernel32_LoadLibraryW_address();
        if (!pfLoadLibraryW) {
            std::wcerr << L"get_remote_kernel32_LoadLibraryW_address failed"sv << std::endl;
            throw std::runtime_error("get_remote_kernel32_LoadLibraryW_address failed");
        }
    }

    const DWORD creationFlags = suspendThread ? CREATE_SUSPENDED : 0;
    STARTUPINFOW startupInfo{sizeof(startupInfo)};
    PROCESS_INFORMATION processInformation{};

    Path workingDirPath = std::filesystem::absolute(exePath).parent_path();

    LPWSTR exePathStr = const_cast<LPWSTR>(exePath.c_str());
    LPWSTR workingDirPathStr = const_cast<LPWSTR>(workingDirPath.c_str());

    if (!CreateProcessW(nullptr, exePathStr, nullptr, nullptr, FALSE, creationFlags, nullptr, workingDirPathStr,
                        &startupInfo, &processInformation)) {
        std::wcerr << L"CreateProcessW failed (GetLastError = "sv << GetLastError() << L")"sv << std::endl;
        throw std::runtime_error("CreateProcessW failed");
    }

    const size_t dllPathDataSize = (dllPath.size() + 1) * sizeof(wchar_t);

    void *remotePtrDllPath = VirtualAllocEx(processInformation.hProcess, nullptr, dllPathDataSize,
                                            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remotePtrDllPath) {
        std::wcerr << L"VirtualAllocEx failed (GetLastError = "sv << GetLastError() << L")"sv << std::endl;
        CloseHandle(processInformation.hThread);
        CloseHandle(processInformation.hProcess);
        throw std::runtime_error("VirtualAllocEx failed");
    }

    if (!WriteProcessMemory(processInformation.hProcess, remotePtrDllPath, dllPath.c_str(), dllPathDataSize, nullptr)) {
        std::wcerr << L"WriteProcessMemory failed (GetLastError = "sv << GetLastError() << L")"sv << std::endl;
        VirtualFreeEx(processInformation.hProcess, remotePtrDllPath, 0, MEM_RELEASE);
        CloseHandle(processInformation.hThread);
        CloseHandle(processInformation.hProcess);
        throw std::runtime_error("WriteProcessMemory failed");
    }

    HANDLE hRemoteThread = CreateRemoteThread(processInformation.hProcess, nullptr, 0,
                                              reinterpret_cast<LPTHREAD_START_ROUTINE>(pfLoadLibraryW),
                                              remotePtrDllPath, 0, nullptr);
    if (!hRemoteThread) {
        std::wcerr << L"CreateRemoteThread failed (GetLastError = "sv << GetLastError() << L")"sv << std::endl;
        VirtualFreeEx(processInformation.hProcess, remotePtrDllPath, 0, MEM_RELEASE);
        CloseHandle(processInformation.hThread);
        CloseHandle(processInformation.hProcess);
        throw std::runtime_error("CreateRemoteThread failed");
    }
    WaitForSingleObject(hRemoteThread, INFINITE);
    CloseHandle(hRemoteThread);
    hRemoteThread = nullptr;

    VirtualFreeEx(processInformation.hProcess, remotePtrDllPath, 0, MEM_RELEASE);

    if (suspendThread) {
        if (ResumeThread(processInformation.hThread) == static_cast<DWORD>(-1)) {
            std::wcerr << L"ResumeThread failed (GetLastError = "sv << GetLastError() << L")"sv << std::endl;
            CloseHandle(processInformation.hThread);
            CloseHandle(processInformation.hProcess);
            throw std::runtime_error("ResumeThread failed");
        }
    }

    if (waitProcess) {
        WaitForSingleObject(processInformation.hProcess, INFINITE);

        DWORD exitCode;
        if (!GetExitCodeProcess(processInformation.hProcess, &exitCode)) {
            std::wcout << L"process exited with code "sv << exitCode << std::endl;
        } else {
            std::wcout << L"process exited"sv;
        }
    }

    CloseHandle(processInformation.hThread);
    CloseHandle(processInformation.hProcess);
}


Path Runner::dll_path (bool is_x64) {
    Path working_dir_ = working_dir();
    if (is_x64) {
        return working_dir_ / X64_DLL_FILE_NAME;
    } else {
        return working_dir_ / X86_DLL_FILE_NAME;
    }
}

bool Runner::determine_if_x64 (const Path &file_path) {
    // Open the file
    std::ifstream file(file_path, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    std::cout << "file opened" << std::endl;

    // Read the DOS header
    IMAGE_DOS_HEADER dos_header = {0};
    file.read(reinterpret_cast<char *>(&dos_header), sizeof(dos_header));
    if (dos_header.e_magic != IMAGE_DOS_SIGNATURE) {
        throw std::runtime_error("Invalid DOS header");
    }

    std::cout << "DOS header read" << std::endl;

    // Seek to the PE header
    file.seekg(dos_header.e_lfanew, std::ios::beg);

    // Read the PE header
    IMAGE_NT_HEADERS ntHeaders;
    file.read(reinterpret_cast<char *>(&ntHeaders), sizeof(ntHeaders));

    if (ntHeaders.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64) {
        return true; // 64 bit file
    } else if (ntHeaders.FileHeader.Machine == IMAGE_FILE_MACHINE_I386) {
        return false; // 32 bit file
    } else {
        throw std::runtime_error("Invalid machine type");
    }
}

void Runner::run (const String &game_name) {
    EnvManager *env_manager = &EnvManager::instance();
    env_manager->gen_env_for_game(game_name);

    auto config = env_manager->config();
    auto game_config = config.game_configs[game_name];

    Path exe_path = game_config.original_game_full_path();
    bool is_x64 = determine_if_x64(exe_path);

    inject_run(exe_path, dll_path(is_x64), is_x64);
}
