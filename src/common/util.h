#ifndef UTIL_H
#define UTIL_H

#include <Windows.h>

#define PIPE_NAME L"\\\\.\\pipe\\Kernel32LoadLibraryWPipe"

FARPROC get_kernel32_LoadLibraryW_address();



#endif //UTIL_H
