#ifndef RUNNER_H
#define RUNNER_H

#include "../common/env.h"

class Runner {
    static constexpr auto X64_DLL_FILE_NAME = L"Bin/Hook-x64.dll";
    static constexpr auto X86_DLL_FILE_NAME = L"Bin/Hook-x86.dll";

    static constexpr auto HELPER_EXE_NAME = L"Bin/Helper.exe";

    Runner () = default;

    static Path helper_exe_path ();
    static FARPROC get_remote_kernel32_LoadLibraryW_address ();

    static void inject_run (const String &exePath, const String &dllPath, bool is_x64, bool suspendThread, bool waitProcess);

public:
    static Path dll_path(bool is_x64);

    static bool determine_if_x64 (const Path &file_path);
    static void run (const String &game_name);
};

#endif //RUNNER_H
