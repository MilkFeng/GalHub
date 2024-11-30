#ifndef INJECT_H
#define INJECT_H

#include <Windows.h>

#define PIPE_NAME L"\\\\.\\pipe\\Kernel32LoadLibraryWPipe"

FARPROC get_kernel32_LoadLibraryW_address();



#endif //INJECT_H
