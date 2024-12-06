#include "path_util.h"

#include <iostream>
#include <ntstatus.h>
#include <filesystem>

using namespace std::literals;

#define SafeDeletePoint(pData) { if(pData){delete pData;pData=NULL;} }
#define SafeDeleteArraySize(pData) { if(pData){delete []pData;pData=NULL;} }


typedef struct _OBJECT_NAME_INFORMATION {
    WORD Length;
    WORD MaximumLength;
    LPWSTR Buffer;
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

typedef long (__stdcall*PNtQueryObject) (HANDLE ObjectHandle, ULONG ObjectInformationClass, PVOID ObjectInformation,
                                         ULONG ObjectInformationLength, PULONG ReturnLength);

NTSTATUS GetFileFullDeviceDosPathPath (HANDLE hFile, std::wstring &full_path) {
    PNtQueryObject NtQueryObject(
        reinterpret_cast<PNtQueryObject>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryObject")));
    OBJECT_NAME_INFORMATION name;

    ULONG len;
    NtQueryObject(hFile, 1, &name, sizeof name, &len);

#ifdef DEBUG
    std::wstring debug_info = L"@MilkFeng GetFileFullDosPath: NtQueryObject 得到的 len = "s + std::to_wstring(len).c_str() + L"\n";
    OutputDebugStringW(debug_info.c_str());
#endif

    OBJECT_NAME_INFORMATION *pname = reinterpret_cast<POBJECT_NAME_INFORMATION>(new char[len]);
    NtQueryObject(hFile, 1, pname, len, &len);

    full_path = std::wstring(pname->Buffer, pname->Length / sizeof(wchar_t));
    delete[] reinterpret_cast<char *>(pname);

#ifdef DEBUG
    debug_info = L"@MilkFeng GetFileFullDosPath: "s + full_path.c_str() + L"\n";
    OutputDebugStringW(debug_info.c_str());
#endif

    // 如果路径为空，返回失败
    if (full_path.empty()) return STATUS_UNSUCCESSFUL;

    return STATUS_SUCCESS;
}

NTSTATUS GetFileFullNtPath (HANDLE hFile, std::wstring &full_path) {
#ifdef DEBUG
    std::wstring debug_info = L"@MilkFeng GetFileFullNtPath\n";
    OutputDebugStringW(debug_info.c_str());
#endif
    std::wstring dos_path;
    if (!NT_SUCCESS(GetFileFullDeviceDosPathPath(hFile, dos_path))) {
#ifdef DEBUG
        debug_info = L"@MilkFeng GetFileFullNtPath: GetFileFullDeviceDosPathPath Failed\n";
        OutputDebugStringW(debug_info.c_str());
#endif
        return STATUS_UNSUCCESSFUL;
    }
#ifdef DEBUG
    debug_info = L"@MilkFeng GetFileFullNtPath: GetFileFullDeviceDosPathPath result is " + dos_path + L"\n";
    OutputDebugStringW(debug_info.c_str());
#endif


    std::wstring nt_path;
    if (!NT_SUCCESS(DosPathToNtPath(dos_path, nt_path))) {
#ifdef DEBUG
        debug_info = L"@MilkFeng GetFileFullNtPath: DosPathToNtPath Failed\n";
        OutputDebugStringW(debug_info.c_str());
#endif
        return STATUS_UNSUCCESSFUL;
    }

#ifdef DEBUG
    debug_info = L"@MilkFeng GetFileFullNtPath: DosPathToNtPath result is " + nt_path + L"\n";
    OutputDebugStringW(debug_info.c_str());
#endif

    full_path = nt_path;
    return STATUS_SUCCESS;
}


typedef UNICODE_STRING *PUNICODE_STRING;

typedef struct _RTL_BUFFER {
    PWCHAR Buffer;
    PWCHAR StaticBuffer;
    SIZE_T Size;
    SIZE_T StaticSize;
    SIZE_T ReservedForAllocatedSize; // for future doubling
    PVOID ReservedForIMalloc; // for future pluggable growth
} RTL_BUFFER, *PRTL_BUFFER;

typedef struct _RTL_UNICODE_STRING_BUFFER {
    UNICODE_STRING String;
    RTL_BUFFER ByteBuffer;
    WCHAR MinimumStaticBufferForTerminalNul[sizeof(WCHAR)];
} RTL_UNICODE_STRING_BUFFER, *PRTL_UNICODE_STRING_BUFFER;


// DOS 路径转换 NT 路径    C:\\WINDOWS\\system32\\drivers    -- \\??\\C:\\WINDOWS\\system32\\drivers
NTSTATUS DosPathToNtPath (const std::wstring &dosPath, std::wstring &ntPath) {
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    typedef BOOLEAN (__stdcall *fnRtlDosPathNameToNtPathName_U) (PCWSTR DosFileName, PUNICODE_STRING NtFileName,
                                                                 PWSTR *FilePart, PVOID Reserved);
    static fnRtlDosPathNameToNtPathName_U RtlDosPathNameToNtPathName_U = reinterpret_cast<
        fnRtlDosPathNameToNtPathName_U>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlDosPathNameToNtPathName_U"));

    // 参数效验
    if (RtlDosPathNameToNtPathName_U == nullptr) return Status;

    // 将 std::wstring 转换为 PUNICODE_STRING
    UNICODE_STRING NtFileName;
    NtFileName.Buffer = const_cast<wchar_t *>(dosPath.c_str());
    NtFileName.Length = static_cast<USHORT>(dosPath.length() * sizeof(wchar_t));
    NtFileName.MaximumLength = static_cast<USHORT>((dosPath.length() + 1) * sizeof(wchar_t));

    // 调用 RtlDosPathNameToNtPathName_U
    if (RtlDosPathNameToNtPathName_U(dosPath.c_str(), &NtFileName, nullptr, nullptr)) {
        // 转换成功，将结果从 PUNICODE_STRING 转换为 std::wstring
        ntPath = std::wstring(NtFileName.Buffer, NtFileName.Length / sizeof(wchar_t));
        Status = STATUS_SUCCESS;
    }

    return Status;
}


// NT 路径转换 DOS 路径    \\??\\C:\\WINDOWS\\system32\\drivers    -- C:\\WINDOWS\\system32\\drivers
NTSTATUS NtPathToDosPath (std::wstring &ntPath, std::wstring &dosPath) {
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    RTL_UNICODE_STRING_BUFFER DosPath = {0};
    wchar_t *ByteDosPathBuffer = nullptr;
    wchar_t *ByteNtPathBuffer = nullptr;

    typedef NTSTATUS (__stdcall *fnRtlNtPathNameToDosPathName) (ULONG Flags, PRTL_UNICODE_STRING_BUFFER Path,
                                                                PULONG Disposition, PWSTR *FilePart);
    static fnRtlNtPathNameToDosPathName RtlNtPathNameToDosPathName = reinterpret_cast<fnRtlNtPathNameToDosPathName>(
        GetProcAddress(GetModuleHandle(reinterpret_cast<LPCSTR>(L"ntdll.dll")), "RtlNtPathNameToDosPathName"));

    // 参数验证
    if (RtlNtPathNameToDosPathName == nullptr) return Status;

    // 将 std::wstring 转换为 PUNICODE_STRING
    UNICODE_STRING pNtPath;
    pNtPath.Buffer = const_cast<wchar_t *>(ntPath.c_str());
    pNtPath.Length = static_cast<USHORT>(ntPath.length() * sizeof(wchar_t));
    pNtPath.MaximumLength = static_cast<USHORT>((ntPath.length() + 1) * sizeof(wchar_t));

    ByteDosPathBuffer = reinterpret_cast<wchar_t *>(new char[pNtPath.Length + sizeof(wchar_t)]);
    ByteNtPathBuffer = reinterpret_cast<wchar_t *>(new char[pNtPath.Length + sizeof(wchar_t)]);
    if (ByteDosPathBuffer == nullptr || ByteNtPathBuffer == nullptr) return Status;

    RtlZeroMemory(ByteDosPathBuffer, pNtPath.Length + sizeof(wchar_t));
    RtlZeroMemory(ByteNtPathBuffer, pNtPath.Length + sizeof(wchar_t));
    RtlCopyMemory(ByteDosPathBuffer, pNtPath.Buffer, pNtPath.Length);
    RtlCopyMemory(ByteNtPathBuffer, pNtPath.Buffer, pNtPath.Length);

    DosPath.ByteBuffer.Buffer = ByteDosPathBuffer;
    DosPath.ByteBuffer.StaticBuffer = ByteNtPathBuffer;
    DosPath.String.Buffer = pNtPath.Buffer;
    DosPath.String.Length = pNtPath.Length;
    DosPath.String.MaximumLength = pNtPath.Length;
    DosPath.ByteBuffer.Size = pNtPath.Length;
    DosPath.ByteBuffer.StaticSize = pNtPath.Length;

    Status = RtlNtPathNameToDosPathName(0, &DosPath, NULL, NULL);
    if (NT_SUCCESS(Status)) {
        if (_wcsnicmp(pNtPath.Buffer, ByteDosPathBuffer, pNtPath.Length) == 0) {
            Status = STATUS_UNSUCCESSFUL;
        } else {
            // 转换为 std::wstring 输出
            dosPath = ByteDosPathBuffer;
        }
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

    SafeDeleteArraySize(ByteDosPathBuffer);
    SafeDeleteArraySize(ByteNtPathBuffer);
    return Status;
}

// \\Device\\HarddiskVolume1\x86.sys     \\??\\c:\x86.sys
NTSTATUS DeviceDosPathToNtPath (const std::wstring &dosPath, std::wstring &ntPath) {
    static WCHAR szDriveStr[MAX_PATH] = {0};
    static WCHAR szDevName[MAX_PATH] = {0};

#ifdef DEBUG
    std::wstring debug_str = L"@MilkFeng DeviceDosPathToNtPath: dosPath = " + dosPath + L"\n";
    OutputDebugStringW(debug_str.c_str());
#endif

    // 获取本地磁盘字符串
    ZeroMemory(szDriveStr, ARRAYSIZE(szDriveStr));
    ZeroMemory(szDevName, ARRAYSIZE(szDevName));
    if (GetLogicalDriveStringsW(sizeof(szDriveStr), szDriveStr)) {
        for (INT i = 0; szDriveStr[i]; i += 4) {
            // 跳过 A: 和 B: 盘符
            if (!lstrcmpiW(&szDriveStr[i], L"A:\\") || !lstrcmpiW(&szDriveStr[i], L"B:\\")) continue;

            WCHAR szDrive[3] = {szDriveStr[i], szDriveStr[i + 1], L'\0'};

#ifdef DEBUG
            debug_str = L"@MilkFeng DeviceDosPathToNtPath: szDrive = " + std::wstring(szDrive) + L"\n";
            OutputDebugStringW(debug_str.c_str());
#endif

            // 查询 Dos 设备名
            if (!QueryDosDeviceW(szDrive, szDevName, MAX_PATH)) {
#ifdef DEBUG
                OutputDebugStringW(L"@MilkFeng DeviceDosPathToNtPath: QueryDosDeviceW Failed\n");
#endif
                return STATUS_UNSUCCESSFUL;
            }

#ifdef DEBUG
            debug_str = L"@MilkFeng DeviceDosPathToNtPath: szDevName = " + std::wstring(szDevName) + L"\n";
            OutputDebugStringW(debug_str.c_str());
#endif

            INT cchDevName = lstrlenW(szDevName);
            if (_wcsnicmp(dosPath.c_str(), szDevName, cchDevName) == 0) {
                ntPath = szDrive; // 复制驱动器
                ntPath += dosPath.substr(cchDevName); // 复制路径

                // 加上 \??\ 前缀
                ntPath.insert(0, L"\\??\\");

                return STATUS_SUCCESS;
            }
        }
    } else {
#ifdef DEBUG
        OutputDebugStringW(L"@MilkFeng DeviceDosPathToNtPath: GetLogicalDriveStringsW Failed\n");
#endif
    }
    return STATUS_UNSUCCESSFUL;
}

Env &GlobalEnv () {
    static Env env;
    return env;
}


std::optional<std::wstring> ModifyPath (const std::wstring &src_) {
    // only handle \??\ and ignore other paths

#ifdef DEBUG
    std::wstring debug_info = L"@MilkFeng ModifyPath: invoke "s + src_ + L"\n"s;
    OutputDebugStringW(debug_info.c_str());
#endif

    // check if dos device path
    std::wstring src(src_);
    if (src.size() >= 8 && _wcsnicmp(src.c_str(), L"\\Device\\", 8) == 0) {
        std::wstring src_nt;
        if (!DeviceDosPathToNtPath(src, src_nt)) {
            return std::nullopt;
        }

        src = src_nt;

// #ifdef DEBUG
//         std::wstring debug_info = L"@MilkFeng ModifyPath: nt_path is "s + src + L"\n"s;
//         OutputDebugStringW(debug_info.c_str());
// #endif
    }

    if (src.size() < 4) {
        return std::nullopt;
    }

    if (_wcsnicmp(src.c_str(), L"\\??\\", 4) != 0 && _wcsnicmp(src.c_str(), L"\\DosDevices\\", 12) != 0) {
        return std::nullopt;
    }

    // remove \??\ and then apply rules
    std::wstring src_without_prefix = src.substr(4);

    std::filesystem::path dst_without_prefix;
    if (!GlobalEnv().apply_rules(src_without_prefix, dst_without_prefix)) {
        return std::nullopt;
    }

    // add \??\ back
    std::wstring dst = L"\\??\\"s + dst_without_prefix.wstring();

#ifdef DEBUG
    debug_info = L"@MilkFeng ModifyPath: success "s + src + L" -> "s + dst + L"\n"s;
    OutputDebugStringW(debug_info.c_str());
#endif

    return dst;
}

NTSTATUS ModifyObjectAttributes (
    OBJECT_ATTRIBUTES &objectAttributes,
    UNICODE_STRING &unicodeString,
    std::wstring &unicodeStringBuffer
) {
    if (!objectAttributes.ObjectName->Buffer) {
        return STATUS_ALREADY_COMPLETE;
    }

    try {
        std::wstring src(objectAttributes.ObjectName->Buffer,
                         objectAttributes.ObjectName->Length / sizeof(wchar_t));

        if (objectAttributes.RootDirectory != nullptr) {
#ifdef DEBUG
            std::wstring debug_info = L"@MilkFeng ModifyObjectAttributes: ObjectAttributes->RootDirectory != nullptr\n";
            OutputDebugStringW(debug_info.c_str());
#endif
            // if RootDirectory is not nullptr, it is a relative path
            // get the full path
            std::wstring rootPath;
            if (!NT_SUCCESS(GetFileFullNtPath(objectAttributes.RootDirectory, rootPath))) {
                return STATUS_UNSUCCESSFUL;
            }

            // concatenate the full path and the relative path
            src = rootPath + L"\\" + src;

#ifdef DEBUG
            debug_info = L"@MilkFeng ModifyObjectAttributes: transform to " + src + L"\n";
            OutputDebugStringW(debug_info.c_str());
#endif
        }

        const auto ret = ModifyPath(src);
        if (!ret) {
            return STATUS_ALREADY_COMPLETE;
        }

        unicodeStringBuffer = ret.value();
        unicodeString.Buffer = const_cast<PWSTR>(unicodeStringBuffer.c_str());
        unicodeString.Length = unicodeStringBuffer.size() * sizeof(wchar_t); // Length does not include null terminator
        unicodeString.MaximumLength = unicodeStringBuffer.size() * sizeof(wchar_t);
        objectAttributes.ObjectName = &unicodeString;

        return STATUS_SUCCESS;
    } catch (...) {
    }

    return STATUS_UNSUCCESSFUL;
}