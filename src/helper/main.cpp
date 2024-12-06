#include "../common/util.h"
#include <iostream>

void SendKernel32AddressToParent () {
    FARPROC pfLoadLibraryW = get_kernel32_LoadLibraryW_address();
    if (!pfLoadLibraryW) {
        std::cerr << "get_kernel32_LoadLibraryW_address failed" << std::endl;
    }

    // create file handle
    HANDLE hPipe = CreateFileW(PIPE_NAME,GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateFile failed: " << GetLastError() << std::endl;
        return;
    }

    // send address to pipe
    DWORD dwWritten;
    if (!WriteFile(hPipe, &pfLoadLibraryW, sizeof(pfLoadLibraryW), &dwWritten, nullptr)) {
        std::cerr << "WriteFile failed: " << GetLastError() << std::endl;
    }

    // close handle
    CloseHandle(hPipe);
}

int main () {
    SendKernel32AddressToParent();
    return 0;
}
