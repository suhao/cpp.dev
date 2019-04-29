///////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
//
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef UTILS_BASIC_UTIL_INCLUDE_H_
#define UTILS_BASIC_UTIL_INCLUDE_H_

#include <memory>
#include <stack>
#include <Windows.h>
#include <Shlwapi.h>

#include "utils.h"
#include "utils/basictypes.h"
#include "utils/scoped_object.h"

namespace utils {

const wchar_t kSeparators[] = L"\\/";
const size_t kSeparatorsLength = arraysize(kSeparators);
const wchar_t kCurrentDirectory[] = L".";
const wchar_t kParentDirectory[] = L"..";
const wchar_t kExtensionSeparator[] = L".";
const wchar_t kStringTerminator = L'\0';
const wchar_t kSearchAll = L'*';

struct UTILS_API PlatformFileInfo {
    PlatformFileInfo() {}
    virtual ~PlatformFileInfo() {}

    ULONGLONG size = 0;
    DWORD attributes = 0;
    bool directory = false;
    FILETIME creation_time;
    FILETIME last_modified;
    FILETIME last_accessed;
    std::wstring filename;
    std::wstring path;
};

UTILS_API bool IsSeparator(wchar_t character);

UTILS_API std::wstring::size_type FindDriveLetter(const std::wstring& path);

UTILS_API std::wstring StripTrailingSeparators(std::wstring path);

UTILS_API bool IsPathAbsolute(const std::wstring path);

UTILS_API std::wstring GetParent(std::wstring path);

UTILS_API std::wstring GetFileName(std::wstring path);

UTILS_API std::wstring Append(std::wstring path,
                              const std::wstring& component) WARN_UNUSED_RESULT;

UTILS_API bool DirectoryExists(const std::wstring& path);

UTILS_API bool CreatePathTree(const std::wstring& path);

UTILS_API bool GetFileInfo(const std::wstring& path, PlatformFileInfo* results);

UTILS_API int64 GetFileSize(const std::wstring& path);

UTILS_API int64_t GetFileSize(const WIN32_FILE_ATTRIBUTE_DATA& find_data);

UTILS_API int64_t GetFileSize(const WIN32_FIND_DATA& find_data);

namespace internal {

struct ScopedHANDLECloseTraits {
    static HANDLE InvalidValue() { return nullptr; }
    static void Free(HANDLE handle) { ::CloseHandle(handle); }
};

struct ScopedSearchHANDLECloseTraits {
    static HANDLE InvalidValue() { return INVALID_HANDLE_VALUE; }
    static void Free(HANDLE handle) { ::FindClose(handle); }
};

} // namespace internal

// TODO: This should be terminate for |FILE| when |CloseHandle| failed.
using ScopedHANDLE = ScopedGeneric<HANDLE, internal::ScopedHANDLECloseTraits>;
using ScopedSearchHANDLE = ScopedGeneric<HANDLE, internal::ScopedSearchHANDLECloseTraits>;

UTILS_API bool IsSymbolicLink(const std::wstring& path);

UTILS_API bool IsRegularFile(const std::wstring& path);

UTILS_API bool IsDirectory(const DWORD& attributes, bool allow_symlinks);

UTILS_API bool IsDirectory(const std::wstring& path, bool allow_symlinks);

// A class for enumerating the files in a provided path. The order of the
// results is not guaranteed. This is blocking. Do not use on critical threads.
// Example:
//   base::FileEnumerator enum(my_dir, false, base::FileEnumerator::FILES, L"*.txt");
//   for (auto name = enum.Next(); !name.empty(); name = enum.Next())
//     ...
class UTILS_API FileEnumerator {
 public:
    // Note: copy & assign supported.
    class UTILS_API FileInfo {
    public:
        explicit FileInfo() { }
        virtual ~FileInfo() {}
        std::wstring GetName() const { return find_data_.cFileName; } // The name of the file. This will not include any path information.
        int64_t GetSize() const { return GetFileSize(find_data_); }
        auto GetLastModifiedTime() const { return find_data_.ftLastWriteTime; }
        bool IsDirectory() const {
          return utils::IsDirectory(find_data_.dwFileAttributes, true);
        }
        const WIN32_FIND_DATA& find_data() const { return find_data_; }
    private:
        friend class FileEnumerator;
        WIN32_FIND_DATA find_data_ = { 0 };
    };

    enum FileType {
        FILES = 1 << 0,
        DIRECTORIES = 1 << 1,
        INCLUDE_DOT_DOT = 1 << 2,
    };

    explicit FileEnumerator(const std::wstring& root_path, bool recursive, int file_type)
        : FileEnumerator(root_path, recursive, file_type, L"") {}

    explicit FileEnumerator(const std::wstring& root_path, bool recursive, int file_type, const std::wstring& pattern)
        : recursive_(recursive), file_type_(file_type), pattern_(pattern.empty() ? std::wstring(1, kSearchAll) : pattern) {
        assert(!(recursive && (INCLUDE_DOT_DOT & file_type_)));
        pending_paths_.push(root_path);
    }
    virtual ~FileEnumerator() {}

    std::wstring Next() {
        while (has_find_data_ || !pending_paths_.empty()) {
            if (!has_find_data_) {
                // The last find FindFirstFile operation is done, prepare a new one.
                root_path_ = pending_paths_.top();
                pending_paths_.pop();

                // Start a new find operation.
                auto path = Append(root_path_, pattern_);
                find_handle_.reset(FindFirstFileEx(path.c_str(), FindExInfoBasic, &find_data_, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH));
                has_find_data_ = true;
            } else if (find_handle_.is_valid() && !FindNextFile(find_handle_.get(), &find_data_)) { // Search for the next file/directory.
                find_handle_.reset();
            }

            if (!find_handle_.is_valid()) {
                has_find_data_ = false;
                pattern_ = kSearchAll;
                continue;
            }

            if (ShouldSkip(find_data_.cFileName))
                continue;

            auto cur_file = Append(root_path_, find_data_.cFileName);
            if (IsDirectory(find_data_.dwFileAttributes, true)) {
                if (recursive_ && utils::IsDirectory(cur_file.c_str(), true)) {
                    pending_paths_.push(cur_file);
                }
                if (file_type_ & FileEnumerator::DIRECTORIES) return cur_file;
            } else if (file_type_ & FileEnumerator::FILES) {
                return cur_file;
            }
        }
        return L"";
    }

    FileInfo GetInfo() const {
        if (!has_find_data_) return FileInfo();
        FileInfo ret;
        std::memcpy(&ret.find_data_, &find_data_, sizeof(find_data_));
        return ret;
    }

    PlatformFileInfo GetPlatformFileInfo() const {
        PlatformFileInfo results;
        if (!has_find_data_) return results;
        auto data = GetInfo();
        results.size = data.GetSize();
        results.attributes = data.find_data().dwFileAttributes;
        results.directory = data.IsDirectory();
        results.last_modified = data.find_data().ftLastWriteTime;
        results.last_accessed = data.find_data().ftLastAccessTime;
        results.creation_time = data.find_data().ftCreationTime;
        return results;
    }

private:
    // Returns true if the given path should be skipped in enumeration.
    bool ShouldSkip(const std::wstring& path) {
        auto basename = GetFileName(path);
        return basename == L"." || (basename == L".." && !(INCLUDE_DOT_DOT & file_type_));
    }

    // True when find_data_ is valid.
    bool has_find_data_ = false;
    bool recursive_ = false;
    int file_type_ = 0;

    ScopedSearchHANDLE find_handle_;
    WIN32_FIND_DATA find_data_ = { 0 };

    std::wstring root_path_; 
    std::wstring pattern_;  // Empty when we want to find everything.
    std::stack<std::wstring> pending_paths_;
    DISALLOW_COPY_AND_ASSIGN(FileEnumerator);
};

UTILS_API bool IsDirectoryEmpty(const std::wstring& path);

UTILS_API ScopedComObject<IStream> Open(const std::wstring& path, bool read);

UTILS_API bool GetCurrentDirectory(std::wstring* dir);

UTILS_API bool SetCurrentDirectory(const std::wstring& directory);

UTILS_API HMODULE LoadLibrary(const std::wstring& path, std::string* error);

UTILS_API HMODULE LoadLibraryDynamically(const std::wstring& path);

UTILS_API void UnloadNativeLibrary(HMODULE library);

UTILS_API void* GetFunctionPointerFromNativeLibrary(HMODULE library,
                                                    const char* name);

UTILS_API void* GetFunctionPointerFromNativeLibrary(
    const std::wstring& library_name, const char* name);

// Returns the result whether |library_name| had been loaded.
// It will be true if |library_name| is empty.
UTILS_API bool WellKnownLibrary(const std::wstring& library_name);

UTILS_API DWORD GetDllVersion(LPCTSTR library_name);

// delete file and folder
UTILS_API int DeleteFile(const std::wstring& path);

// whether path is a folder
UTILS_API bool IsFolder(const std::wstring& path);

} // namespace utils


#endif // !UTILS_BASIC_UTIL_INCLUDE_H_