#ifndef PATH_UTIL_H
#define PATH_UTIL_H

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <optional>
#include <winternl.h>

#include <string>

#include "../common/env.h"

NTSTATUS GetFileFullDeviceDosPathPath (HANDLE hFile, std::wstring &full_path);

NTSTATUS GetFileFullNtPath (HANDLE hFile, std::wstring &full_path);

NTSTATUS DosPathToNtPath (const std::wstring &dosPath, std::wstring &ntPath);

NTSTATUS NtPathToDosPath (std::wstring &ntPath, std::wstring &dosPath);

NTSTATUS DeviceDosPathToNtPath (const std::wstring &dosPath, std::wstring &ntPath);

Env &GlobalEnv ();

std::optional<std::wstring> ModifyPath (const std::wstring &src);

NTSTATUS ModifyObjectAttributes (
    OBJECT_ATTRIBUTES &objectAttributes,
    UNICODE_STRING &unicodeString,
    std::wstring &unicodeStringBuffer
);


#endif //PATH_UTIL_H
