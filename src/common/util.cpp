#include "util.h"


FARPROC get_kernel32_LoadLibraryW_address() {
    HMODULE hmKernel32 = GetModuleHandleW(L"Kernel32.dll");
    if (!hmKernel32) {
        return nullptr;
    }

    return GetProcAddress(hmKernel32, "LoadLibraryW");
}