#ifndef UTIL_H
#define UTIL_H

#include <filesystem>
#include <windows.h>

#define PIPE_NAME L"\\\\.\\pipe\\Kernel32LoadLibraryWPipe"

FARPROC get_kernel32_LoadLibraryW_address();

const std::filesystem::path &working_dir ();

std::wstring read_wstring_from_file (const std::filesystem::path &path);
void write_wstring_to_file (const std::filesystem::path &path, const std::wstring &content);

#endif //UTIL_H
