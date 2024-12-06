#ifndef NT_SET_INFORMATION_FILE_HPP
#define NT_SET_INFORMATION_FILE_HPP

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>

#include <string>
#include <memory>

#include "path_util.h"

using TNtSetInformationFile = NTSTATUS(WINAPI*) (HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, FILE_INFORMATION_CLASS);


typedef enum _M_FILE_INFORMATION_CLASS {
    // FileDirectoryInformation = 1,
    FileFullDirectoryInformation = 2,
    FileBothDirectoryInformation = 3,
    FileBasicInformation = 4,
    FileStandardInformation = 5,
    FileInternalInformation = 6,
    FileEaInformation = 7,
    FileAccessInformation = 8,
    FileNameInformation = 9,
    FileRenameInformation = 10,
    FileLinkInformation = 11,
    FileNamesInformation = 12,
    FileDispositionInformation = 13,
    FilePositionInformation = 14,
    FileFullEaInformation = 15,
    FileModeInformation = 16,
    FileAlignmentInformation = 17,
    FileAllInformation = 18,
    FileAllocationInformation = 19,
    FileEndOfFileInformation = 20,
    FileAlternateNameInformation = 21,
    FileStreamInformation = 22,
    FilePipeInformation = 23,
    FilePipeLocalInformation = 24,
    FilePipeRemoteInformation = 25,
    FileMailslotQueryInformation = 26,
    FileMailslotSetInformation = 27,
    FileCompressionInformation = 28,
    FileObjectIdInformation = 29,
    FileCompletionInformation = 30,
    FileMoveClusterInformation = 31,
    FileQuotaInformation = 32,
    FileReparsePointInformation = 33,
    FileNetworkOpenInformation = 34,
    FileAttributeTagInformation = 35,
    FileTrackingInformation = 36,
    FileIdBothDirectoryInformation = 37,
    FileIdFullDirectoryInformation = 38,
    FileValidDataLengthInformation = 39,
    FileShortNameInformation = 40,
    FileIoCompletionNotificationInformation = 41,
    FileIoStatusBlockRangeInformation = 42,
    FileIoPriorityHintInformation = 43,
    FileSfioReserveInformation = 44,
    FileSfioVolumeInformation = 45,
    FileHardLinkInformation = 46,
    FileProcessIdsUsingFileInformation = 47,
    FileNormalizedNameInformation = 48,
    FileNetworkPhysicalNameInformation = 49,
    FileIdGlobalTxDirectoryInformation = 50,
    FileIsRemoteDeviceInformation = 51,
    FileUnusedInformation = 52,
    FileNumaNodeInformation = 53,
    FileStandardLinkInformation = 54,
    FileRemoteProtocolInformation = 55,
    FileRenameInformationBypassAccessCheck = 56,
    FileLinkInformationBypassAccessCheck = 57,
    FileVolumeNameInformation = 58,
    FileIdInformation = 59,
    FileIdExtdDirectoryInformation = 60,
    FileReplaceCompletionInformation = 61,
    FileHardLinkFullIdInformation = 62,
    FileIdExtdBothDirectoryInformation = 63,
    FileDispositionInformationEx = 64,
    FileRenameInformationEx = 65,
    FileRenameInformationExBypassAccessCheck = 66,
    FileDesiredStorageClassInformation = 67,
    FileStatInformation = 68,
    FileMemoryPartitionInformation = 69,
    FileStatLxInformation = 70,
    FileCaseSensitiveInformation = 71,
    FileLinkInformationEx = 72,
    FileLinkInformationExBypassAccessCheck = 73,
    FileStorageReserveIdInformation = 74,
    FileCaseSensitiveInformationForceAccessCheck = 75,
    FileKnownFolderInformation = 76,
    FileStatBasicInformation = 77,
    FileId64ExtdDirectoryInformation = 78,
    FileId64ExtdBothDirectoryInformation = 79,
    FileIdAllExtdDirectoryInformation = 80,
    FileIdAllExtdBothDirectoryInformation = 81,
    FileStreamReservationInformation,
    FileMupProviderInfo,
    FileMaximumInformation
} M_FILE_INFORMATION_CLASS, *PM_FILE_INFORMATION_CLASS;


inline std::optional<std::pair<std::unique_ptr<char[]>, ULONG> > ModifyRenameFileInformation (
    PVOID FileInformation, ULONG Length) {
    struct FILE_RENAME_INFORMATION {
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10_RS1)
        union {
            BOOLEAN ReplaceIfExists; // FileRenameInformation
            ULONG Flags; // FileRenameInformationEx
        } DUMMYUNIONNAME;
#else
        BOOLEAN ReplaceIfExists;
#endif
        HANDLE RootDirectory;
        ULONG FileNameLength;
        WCHAR FileName[1];
    };

    ULONG SIZE = sizeof(FILE_RENAME_INFORMATION);


    if (!FileInformation) return std::nullopt;
    if (Length < sizeof(FILE_RENAME_INFORMATION)) return std::nullopt;

    auto &fileRenameInformation = *static_cast<FILE_RENAME_INFORMATION *>(FileInformation);
    ULONG NEED_SIZE = SIZE - sizeof(FILE_RENAME_INFORMATION::FileName) + fileRenameInformation.FileNameLength;

    if (fileRenameInformation.FileNameLength % sizeof(wchar_t) != 0) return std::nullopt;
    if (Length < NEED_SIZE) return std::nullopt;

    std::wstring src(&fileRenameInformation.FileName[0], fileRenameInformation.FileNameLength / sizeof(wchar_t));

    if (fileRenameInformation.RootDirectory != nullptr) {
#ifdef DEBUG
        std::wstring debug_info = L"@MilkFeng ModifyRenameFileInformation: fileRenameInformation->RootDirectory != nullptr\n";
        OutputDebugStringW(debug_info.c_str());
#endif
        // if RootDirectory is not nullptr, it is a relative path
        // get the full path
        std::wstring rootPath;
        if (!NT_SUCCESS(GetFileFullNtPath(fileRenameInformation.RootDirectory, rootPath))) {
            return std::nullopt;
        }

        // concatenate the full path and the relative path
        src = rootPath + L"\\" + src;
#ifdef DEBUG
        debug_info = L"@MilkFeng ModifyRenameFileInformation: transform to " + src + L"\n";
        OutputDebugStringW(debug_info.c_str());
#endif
    }

    const auto ret = ModifyPath(src);
    if (!ret) return std::nullopt;

    const ULONG newLength = sizeof(FILE_RENAME_INFORMATION) -
                            sizeof(FILE_RENAME_INFORMATION::FileName) +
                            (ret.value().size() + 1) * sizeof(wchar_t);
    auto newFileRenameInformationBuffer = std::make_unique<char[]>(newLength);
    auto ptrNewFileRenameInformationBuffer = reinterpret_cast<FILE_RENAME_INFORMATION *>(newFileRenameInformationBuffer.
        get());

    *ptrNewFileRenameInformationBuffer = fileRenameInformation;
    ptrNewFileRenameInformationBuffer->FileNameLength = ret.value().size() * sizeof(wchar_t);
    std::wmemcpy(&ptrNewFileRenameInformationBuffer->FileName[0], ret.value().c_str(),
                 (ret.value().size() + 1) * sizeof(wchar_t));

    return std::make_pair(std::move(newFileRenameInformationBuffer), newLength);
}


template<TNtSetInformationFile * OrgNtZwSetInformationFile>
NTSTATUS WINAPI HookedNtSetInformationFile (
    HANDLE FileHandle,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass
) {
    std::unique_ptr<char[]> newFileInformation;
    ULONG newLength = -1;
#ifdef DEBUG
    std::wstring debug_info = L"@MilkFeng HookedNtZwSetInformationFile: " + std::to_wstring(FileInformationClass) + L"\n";
    OutputDebugStringW(debug_info.c_str());
#endif


    switch (FileInformationClass) {
        case FileRenameInformation:
        case FileRenameInformationEx:
            if (auto result = ModifyRenameFileInformation(FileInformation, Length)) {
                newFileInformation = std::move(result.value().first);
                newLength = result.value().second;
            }
            break;
        default:
            break;
    }

    if (newLength == -1) {
        return (*OrgNtZwSetInformationFile)(FileHandle, IoStatusBlock, FileInformation, Length, FileInformationClass);
    } else {
        return (*OrgNtZwSetInformationFile)(FileHandle, IoStatusBlock, newFileInformation.get(), newLength,
                                            FileInformationClass);
    }
}

#endif //NT_SET_INFORMATION_FILE_HPP
