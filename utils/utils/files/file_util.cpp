////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http://ant.sh). All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
////////////////////////////////////////////////////////////////////////////////
#include "utils/files/file_util.h"

#include <string>


#include "utils/dynamic_library.h"

#if defined(COMPILER_MSVC)
// We usually use the _CrtDumpMemoryLeaks() with the DEBUGER and CRT library to
// check a memory leak.
#if defined(_DEBUG) && _MSC_VER > 1000  // VC++ DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#if defined(DEBUG_NEW)
#define new DEBUG_NEW
#endif  // DEBUG_NEW
#endif  // VC++ DEBUG
#endif  // defined(COMPILER_MSVC)


namespace internal {

typedef HMODULE(WINAPI* LoadLibraryFunction)(const wchar_t* file_name);

// LoadLibrary() opens the file off disk.
HMODULE LoadNativeLibraryHelper(const std::wstring& library_path,
                                LoadLibraryFunction load_library_api) {
  // Switch the current directory to the library directory as the library
  // may have dependencies on DLLs in this directory.
  bool restore_directory = false;
  std::wstring current_directory;
  if (utils::GetCurrentDirectory(&current_directory)) {
    auto plugin_path = utils::GetParent(library_path);
    if (!plugin_path.empty()) {
      utils::SetCurrentDirectory(plugin_path);
      restore_directory = true;
    }
  }
  HMODULE module = (*load_library_api)(library_path.c_str());
  if (restore_directory) utils::SetCurrentDirectory(current_directory);

  return module;
}

}  // namespace internal

UTILS_API bool utils::IsSeparator(wchar_t character) {
  for (size_t i = 0; i < kSeparatorsLength - 1; ++i) {
    if (character == kSeparators[i]) {
      return true;
    }
  }
  return false;
}

UTILS_API std::wstring::size_type utils::FindDriveLetter(
    const std::wstring& path) {
  // This is dependent on an ASCII-based character set, but that's a
  // reasonable assumption.  iswalpha can be too inclusive here.
  if (path.length() >= 2 && path[1] == L':' &&
      ((path[0] >= L'A' && path[0] <= L'Z') ||
       (path[0] >= L'a' && path[0] <= L'z'))) {
    return 1;
  }
  return std::wstring::npos;
}

UTILS_API std::wstring utils::StripTrailingSeparators(std::wstring path) {
  auto start = FindDriveLetter(path) + 2;
  std::wstring::size_type last_stripped = std::wstring::npos;
  for (std::wstring::size_type pos = path.length();
       pos > start && IsSeparator(path[pos - 1]); --pos) {
    // If the string only has two separators and they're at the beginning,
    // don't strip them, unless the string began with more than two separators.
    if (pos != start + 1 || last_stripped == start + 2 ||
        !IsSeparator(path[start - 1])) {
      path.resize(pos - 1);
      last_stripped = pos;
    }
  }
  return path;
}

UTILS_API bool utils::IsPathAbsolute(const std::wstring path) {
  std::wstring::size_type letter = FindDriveLetter(path);
  if (letter != std::wstring::npos) {
    // Look for a separator right after the drive specification.
    return path.length() > letter + 1 && IsSeparator(path[letter + 1]);
  }
  // Look for a pair of leading separators.
  return path.length() > 1 && IsSeparator(path[0]) && IsSeparator(path[1]);
}

UTILS_API std::wstring utils::GetParent(std::wstring path) {
  path = StripTrailingSeparators(path);

  // The drive letter, if any, always needs to remain in the output.  If there
  // is no drive letter, as will always be the case on platforms which do not
  // support drive letters, letter will be npos, or -1, so the comparisons and
  // resizes below using letter will still be valid.
  auto letter = FindDriveLetter(path);

  auto last_separator =
      path.find_last_of(kSeparators, std::wstring::npos, kSeparatorsLength - 1);
  if (last_separator == std::wstring::npos) {
    // path_ is in the current directory.
    path.resize(letter + 1);
  } else if (last_separator == letter + 1) {
    // path_ is in the root directory.
    path.resize(letter + 2);
  } else if (last_separator == letter + 2 && IsSeparator(path[letter + 1])) {
    // path_ is in "//" (possibly with a drive letter); leave the double
    // separator intact indicating alternate root.
    path.resize(letter + 3);
  } else if (last_separator != 0) {
    // path_ is somewhere else, trim the basename.
    path.resize(last_separator);
  }

  path = StripTrailingSeparators(path);
  if (!path.length()) path = kCurrentDirectory;

  return path;
}

UTILS_API std::wstring utils::Append(std::wstring path,
                                     const std::wstring& component) {
  const std::wstring* appended = &component;
  std::wstring without_nuls;
  auto nul_pos = component.find(kStringTerminator);
  if (nul_pos != std::wstring::npos) {
    without_nuls = component.substr(0, nul_pos);
    appended = &without_nuls;
  }

  if (path.length() <= 0) return *appended;
  if (appended->length() <= 0) return path;

  if (path.compare(kCurrentDirectory) == 0) {
    return *appended;
  }
  if (IsPathAbsolute(*appended)) return L"";

  path = StripTrailingSeparators(path);
  if (!IsSeparator(path[path.length() - 1])) {
    // Don't append a separator if the path is just a drive letter.
    if (FindDriveLetter(path) + 1 != path.length()) {
      path.append(1, kSeparators[0]);
    }
  }
  return path.append(*appended);
}

UTILS_API std::wstring utils::GetFileName(std::wstring path) {
  path = StripTrailingSeparators(path);
  auto letter = FindDriveLetter(path);
  if (letter != std::wstring::npos) {
    path.erase(0, letter + 1);
  }
  auto last_separator =
      path.find_last_of(kSeparators, std::wstring::npos, kSeparatorsLength - 1);
  if (last_separator != std::wstring::npos &&
      last_separator < path.length() - 1) {
    path.erase(0, last_separator + 1);
  }
  return path;
}

UTILS_API bool utils::DirectoryExists(const std::wstring& path) {
  DWORD fileattr = GetFileAttributes(path.c_str());
  if (fileattr != INVALID_FILE_ATTRIBUTES)
    return (fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0;
  return false;
}

UTILS_API bool utils::CreatePathTree(const std::wstring& path) {
  auto result = CreateDirectoryEx(nullptr, path.c_str(), nullptr);
  if (result == ERROR_SUCCESS) return true;
  DWORD fileattr = ::GetFileAttributes(path.c_str());
  if (fileattr != INVALID_FILE_ATTRIBUTES) {
    if ((fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
      return true;
    }
    return false;
  }

  auto parent_path = GetParent(path);
  if (path == parent_path) return false;
  if (!CreatePathTree(parent_path)) return false;

  if (!::CreateDirectory(path.c_str(), NULL)) {
    DWORD error_code = ::GetLastError();
    if (error_code == ERROR_ALREADY_EXISTS && DirectoryExists(path)) {
      return true;
    }
    return false;
  }
  return true;
}

UTILS_API bool utils::GetFileInfo(const std::wstring& path,
                                  PlatformFileInfo* results) {
  WIN32_FILE_ATTRIBUTE_DATA attr;
  if (!GetFileAttributesEx(path.c_str(), GetFileExInfoStandard, &attr)) {
    return false;
  }

  ULARGE_INTEGER size;
  size.HighPart = attr.nFileSizeHigh;
  size.LowPart = attr.nFileSizeLow;
  results->size = size.QuadPart;

  results->attributes = attr.dwFileAttributes;
  results->directory = (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  results->last_modified = attr.ftLastWriteTime;
  results->last_accessed = attr.ftLastAccessTime;
  results->creation_time = attr.ftCreationTime;
  return true;
}

UTILS_API int64 utils::GetFileSize(const std::wstring& path) {
  PlatformFileInfo info;
  if (!GetFileInfo(path, &info)) return -1;
  return info.size;
}

UTILS_API int64_t
utils::GetFileSize(const WIN32_FILE_ATTRIBUTE_DATA& find_data) {
  ULARGE_INTEGER size;
  size.HighPart = find_data.nFileSizeHigh;
  size.LowPart = find_data.nFileSizeLow;
  if (size.QuadPart >
      static_cast<ULONGLONG>((std::numeric_limits<int64_t>::max)())) {
    return (std::numeric_limits<int64_t>::max)();
  }
  return static_cast<int64_t>(size.QuadPart);
}

UTILS_API int64_t utils::GetFileSize(const WIN32_FIND_DATA& find_data) {
  ULARGE_INTEGER size;
  size.HighPart = find_data.nFileSizeHigh;
  size.LowPart = find_data.nFileSizeLow;
  if (size.QuadPart >
      static_cast<ULONGLONG>((std::numeric_limits<int64_t>::max)())) {
    return (std::numeric_limits<int64_t>::max)();
  }
  return static_cast<int64_t>(size.QuadPart);
}

UTILS_API bool utils::IsSymbolicLink(const std::wstring& path) {
  WIN32_FIND_DATA find_data;
  ScopedSearchHANDLE handle(::FindFirstFileEx(path.c_str(), FindExInfoBasic,
                                              &find_data, FindExSearchNameMatch,
                                              nullptr, 0));
  if (!handle.is_valid()) return false;
  return (find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0 &&
         find_data.dwReserved0 == IO_REPARSE_TAG_SYMLINK;
}

UTILS_API bool utils::IsRegularFile(const std::wstring& path) {
  auto fileattr = ::GetFileAttributes(path.c_str());
  if (fileattr == INVALID_FILE_ATTRIBUTES) return false;
  if ((fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0 ||
      (fileattr & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
    return false;
  }
  return true;
}

UTILS_API bool utils::IsDirectory(const DWORD& attributes,
                                  bool allow_symlinks) {
  if (attributes == INVALID_FILE_ATTRIBUTES) return false;
  if (!allow_symlinks && (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
    return false;
  return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

UTILS_API bool utils::IsDirectory(const std::wstring& path,
                                  bool allow_symlinks) {
  return IsDirectory(::GetFileAttributes(path.c_str()), allow_symlinks);
}

UTILS_API bool utils::IsDirectoryEmpty(const std::wstring& path) {
  FileEnumerator files(path, false,
                       FileEnumerator::FILES | FileEnumerator::DIRECTORIES);
  if (files.Next().empty()) return true;
  return false;
}

UTILS_API ScopedComObject<IStream> utils::Open(const std::wstring& path,
                                               bool read) {
  ScopedComObject<IStream> file_stream;
  auto mode = read ? STGM_READ : (STGM_CREATE | STGM_WRITE);
  auto result = ::SHCreateStreamOnFileEx(
      path.c_str(), mode, FILE_ATTRIBUTE_NORMAL, read ? FALSE : TRUE, nullptr,
      file_stream.Receive());
  return file_stream;
}

UTILS_API bool utils::GetCurrentDirectory(std::wstring* dir) {
  wchar_t system_buffer[MAX_PATH] = {0};
  auto len = ::GetCurrentDirectory(MAX_PATH, system_buffer);
  if (len == 0 || len > MAX_PATH) return false;
  std::wstring dir_str(system_buffer);
  *dir = StripTrailingSeparators(dir_str);
  return true;
}

UTILS_API bool utils::SetCurrentDirectory(const std::wstring& directory) {
  BOOL ret = ::SetCurrentDirectory(directory.c_str());
  return ret != 0;
}

UTILS_API HMODULE utils::LoadLibrary(const std::wstring& path,
                                     std::string* error) {
  return ::internal::LoadNativeLibraryHelper(path, ::LoadLibraryW);
}

UTILS_API HMODULE utils::LoadLibraryDynamically(const std::wstring& path) {
  typedef HMODULE(WINAPI * LoadLibraryFunction)(const wchar_t* file_name);

  LoadLibraryFunction load_library = reinterpret_cast<LoadLibraryFunction>(
      ::GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW"));

  return ::internal::LoadNativeLibraryHelper(path, load_library);
}

UTILS_API void utils::UnloadNativeLibrary(HMODULE library) {
  if (library == nullptr) return;
  ::FreeLibrary(library);
}

UTILS_API void* utils::GetFunctionPointerFromNativeLibrary(HMODULE library,
                                                           const char* name) {
  if (name == nullptr) return nullptr;
  return ::GetProcAddress(library, name);
}

UTILS_API void* utils::GetFunctionPointerFromNativeLibrary(
    const std::wstring& library_name, const char* name) {
  if (name == nullptr) return nullptr;
  HMODULE wellknown_handler = ::GetModuleHandle(library_name.c_str());
  if (nullptr == wellknown_handler) return nullptr;
  return GetFunctionPointerFromNativeLibrary(wellknown_handler, name);
}

// Returns the result whether |library_name| had been loaded.
// It will be true if |library_name| is empty.
UTILS_API bool utils::WellKnownLibrary(const std::wstring& library_name) {
  if (library_name.empty()) return false;
  HMODULE wellknown_handler = ::GetModuleHandle(library_name.c_str());
  return nullptr != wellknown_handler;
}

UTILS_API DWORD utils::GetDllVersion(LPCTSTR library_name) {
  DWORD version = 0;
  std::wstring library(library_name);
  utils::DynamicLibrary module(library);
  if (!module.is_valid()) return version;

  auto DllGetVersion = reinterpret_cast<DLLGETVERSIONPROC>(
      module.GetFunctionPointer("DllGetVersion"));
  if (!DllGetVersion) return version;

  DLLVERSIONINFO dvi;
  ZeroMemory(&dvi, sizeof(dvi));
  dvi.cbSize = sizeof(dvi);
  HRESULT hr = (*DllGetVersion)(&dvi);
  if (FAILED(hr)) return version;

  version = MAKELONG(dvi.dwMinorVersion, dvi.dwMajorVersion);
  return version;
}

UTILS_API int utils::DeleteFile(const std::wstring& path) {
    const int size = path.length() + 2;
    WCHAR *buf = new WCHAR[size];
    memset(buf, 0, sizeof(WCHAR)*size);
    wmemcpy_s(buf, size, path.data(), path.length());

    SHFILEOPSTRUCT shfo = {
    NULL,
    FO_DELETE,
    buf,
    NULL,
    FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION,
    FALSE,
    NULL,
    NULL };

    return SHFileOperation(&shfo);
}

UTILS_API bool utils::IsFolder(const std::wstring& path) {
    DWORD attr = ::GetFileAttributes(path.c_str());
    if ((attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }

    return false;
}
