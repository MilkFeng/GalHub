#include "util.h"

#include <codecvt>
#include <fstream>
#include <locale>

using namespace std::literals;

FARPROC get_kernel32_LoadLibraryW_address () {
    HMODULE hmKernel32 = GetModuleHandleW(L"Kernel32.dll");
    if (!hmKernel32) {
        return nullptr;
    }

    return GetProcAddress(hmKernel32, "LoadLibraryW");
}

std::filesystem::path get_working_dir () {
    wchar_t path[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, path, MAX_PATH)) {
        throw std::runtime_error("Failed to get module file name");
    }

    return std::filesystem::absolute(path).parent_path();
}

const std::filesystem::path &working_dir () {
    static std::filesystem::path working_dir = get_working_dir();
    return working_dir;
}

static std::string unicode_to_utf8 (const std::wstring &wstr) {
    // TODO: this function is marked as deprecated in C++17, find a better way to convert wstring to utf8
    std::string ret;
    try {
        std::wstring_convert<std::codecvt_utf8<wchar_t> > wcv;
        ret = wcv.to_bytes(wstr);
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to convert wstring to utf8: "s + e.what());
    }
    return ret;
}

static std::wstring utf8_to_unicode (const std::string &str) {
    // TODO: this function is marked as deprecated in C++17, find a better way to convert utf8 to wstring
    std::wstring ret;
    try {
        std::wstring_convert<std::codecvt_utf8<wchar_t> > wcv;
        ret = wcv.from_bytes(str);
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to convert utf8 to wstring: "s + e.what());
    }
    return ret;
}

std::wstring read_wstring_from_file (const std::filesystem::path &path) {
    // read file content
    std::string content;
    std::ifstream file(path, std::ios::in);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }
    file.seekg(0, std::ios::end);

    content.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());

    file.close();

    // convert to wstring
    return utf8_to_unicode(content);
}

void write_wstring_to_file (const std::filesystem::path &path, const std::wstring &content) {
    // convert to utf8
    std::string utf8_content = unicode_to_utf8(content);

    // write to file
    std::ofstream file(path, std::ios::out);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }
    file << utf8_content;
    file.close();
}
