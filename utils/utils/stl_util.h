///////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
//
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef UTILS_STL_UTIL_INCLUDE_H_
#define UTILS_STL_UTIL_INCLUDE_H_

#include <algorithm>
#include <vector>

#include "utils/basictypes.h"
#include "utils/compiler.h"

const char kUtf8ByteOrderMark[] = "\xEF\xBB\xBF";
const wchar_t kWhitespaceWide[] = { WHITESPACE_UNICODE };
const char16 kWhitespaceUTF16[] = { WHITESPACE_UNICODE };
const char kWhitespaceASCII[] = {
  0x09,    // <control-0009> to <control-000D>
  0x0A,
  0x0B,
  0x0C,
  0x0D,
  0x20,    // Space
  0
};

enum TrimPositions {
    TRIM_NONE       = 0,
    TRIM_LEADING    = 1 << 0,
    TRIM_TRAILING   = 1 << 1,
    TRIM_ALL        = TRIM_LEADING | TRIM_TRAILING,
};

// ASCII-specific tolower.  The standard library's tolower is locale sensitive,
// so we don't want to use it here.
template <class Char> inline Char ToLowerASCII(Char c) {
    return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

// ASCII-specific toupper.  The standard library's toupper is locale sensitive,
// so we don't want to use it here.
template <class Char> inline Char ToUpperASCII(Char c) {
    return (c >= 'a' && c <= 'z') ? (c + ('A' - 'a')) : c;
}

// Function objects to aid in comparing/searching strings.

template<typename Char> struct CaseInsensitiveCompare {
public:
    bool operator()(Char x, Char y) const {
        // TODO(darin): Do we really want to do locale sensitive comparisons here?
        // See http://crbug.com/24917
        return tolower(x) == tolower(y);
    }
};

template<typename Char> struct CaseInsensitiveCompareASCII {
public:
    bool operator()(Char x, Char y) const {
        return ToLowerASCII(x) == ToLowerASCII(y);
    }
};

// Determines the type of ASCII character, independent of locale (the C
// library versions will change based on locale).
template <typename Char>
static bool IsAsciiWhitespace(Char c) {
    return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}
template <typename Char>
static bool IsAsciiAlpha(Char c) {
    return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'));
}
template <typename Char>
static bool IsAsciiDigit(Char c) {
    return c >= '0' && c <= '9';
}

template <typename Char>
static bool IsHexDigit(Char c) {
    return (c >= '0' && c <= '9') ||
        (c >= 'A' && c <= 'F') ||
        (c >= 'a' && c <= 'f');
}

template <typename Char>
static Char HexDigitToInt(Char c) {
    DCHECK(IsHexDigit(c));
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return 0;
}

// Returns true if it's a whitespace character.
static bool IsWhitespace(wchar_t c) {
    return wcschr(kWhitespaceWide, c) != NULL;
}

static bool IsWildcard(uint32 character) {
    return character == '*' || character == '?';
}

struct ReplacementOffset {
    ReplacementOffset(uintptr_t parameter, size_t offset) : parameter(parameter), offset(offset) {}

    // Index of the parameter.
    uintptr_t parameter = 0;

    // Starting position in the string.
    size_t offset = 0;
};

static bool CompareParameter(const ReplacementOffset& elem1, const ReplacementOffset& elem2) {
    return elem1.parameter < elem2.parameter;
}

namespace std {

template<typename Container, typename Elem>
void erase(Container cont, Elem elem) {
    cont.erase(std::remove(cont.begin(), cont.end(), elem), cont.end());
}

// Free the STL object's memory.
template<typename T>
void clear(T* obj) {
    T tmp;
    tmp.swap(*obj);
    obj->reserve(0);
}

template<typename ForwardIterator>
void DeletePointer(ForwardIterator begin, ForwardIterator end) {
    while (begin != end) {
        ForwardIterator tmp = begin;
        ++begin;
        delete *tmp;
    }
}
template <class ForwardIterator>
void DeletePairPointers(ForwardIterator begin, ForwardIterator end) {
    while (begin != end) {
        ForwardIterator temp = begin;
        ++begin;
        delete temp->first;
        delete temp->second;
    }
}

// To treat a possibly-empty vector as an array, use these functions.
// If you know the array will never be empty, you can use &*v.begin()
// directly, but that is undefined behaviour if |v| is empty.
template<typename T>
inline T* vector_as_array(std::vector<T>* v) {
    return v->empty() ? NULL : &*v->begin();
}

template<typename T>
inline const T* vector_as_array(const std::vector<T>* v) {
    return v->empty() ? NULL : &*v->begin();
}

// Return a mutable char* pointing to a string's internal buffer,
// which may not be null-terminated. Writing through this pointer will
// modify the string.
//
// string_as_array(&str)[i] is valid for 0 <= i < str.size() until the
// next call to a string method that invalidates iterators.
inline char* string_as_array(std::string* str) {
    // DO NOT USE const_cast<char*>(str->data())
    return str->empty() ? NULL : &*str->begin();
}

// Test to see if a set, map, hash_set or hash_map contains a particular key.
// Returns true if the key is in the collection.
template <typename Collection, typename Key>
bool ContainsKey(const Collection& collection, const Key& key) {
    return collection.find(key) != collection.end();
}

// Returns true if the container is sorted.
template <typename Container>
bool Sorted(const Container& cont) {
    return std::adjacent_find(cont.begin(), cont.end(),
        std::greater<typename Container::value_type>())
        == cont.end();
}

// Returns a new ResultType containing the difference of two sorted containers.
template <typename ResultType, typename Arg1, typename Arg2>
ResultType Difference(const Arg1& a1, const Arg2& a2) {
    ResultType difference;
    std::set_difference(a1.begin(), a1.end(), a2.begin(), a2.end(),
        std::inserter(difference, difference.end()));
    return difference;
}

template<typename Collection, typename Key>
typename Collection::iterator Next(Collection& collection, const Key& key) {
    if (collection.empty()) return collection.begin();

    auto it = collection.find(key);
    if (it == collection.end() || std::next(it) == collection.end()) {
        return collection.begin();
    }
    return ++it;
}

} // namespace std

namespace internal {

template <typename CHAR>
static size_t lcpyT(CHAR* dst, const CHAR* src, size_t dst_size) {
    for (size_t i = 0; i < dst_size; ++i) {
        // We hit and copied the terminating NULL.
        if ((dst[i] = src[i]) == 0) return i;
    }

    // We were left off at dst_size.  We over copied 1 byte.  Null terminate.
    if (dst_size != 0) dst[dst_size - 1] = 0;

    // Count the rest of the |src|, and return it's length in characters.
    while (src[dst_size]) ++dst_size;
    return dst_size;
}

template<typename STR>
bool ReplaceCharsT(const STR& input,
    const typename STR::value_type replace_chars[],
    const STR& replace_with,
    STR* output) {
    bool removed = false;
    size_t replace_length = replace_with.length();

    *output = input;

    size_t found = output->find_first_of(replace_chars);
    while (found != STR::npos) {
        removed = true;
        output->replace(found, 1, replace_with);
        found = output->find_first_of(replace_chars, found + replace_length);
    }

    return removed;
}

template<typename STR>
TrimPositions TrimStringT(const STR& input, const typename STR::value_type trim_chars[], TrimPositions positions, STR* output) {
    // Find the edges of leading/trailing whitespace as desired.
    const typename STR::size_type last_char = input.length() - 1;
    const typename STR::size_type first_good_char = (positions & TRIM_LEADING) ? input.find_first_not_of(trim_chars) : 0;
    const typename STR::size_type last_good_char = (positions & TRIM_TRAILING) ? input.find_last_not_of(trim_chars) : last_char;

    // When the string was all whitespace, report that we stripped off whitespace
    // from whichever position the caller was interested in.  For empty input, we
    // stripped no whitespace, but we still need to clear |output|.
    if (input.empty() || (first_good_char == STR::npos) || (last_good_char == STR::npos)) {
        bool input_was_empty = input.empty();  // in case output == &input
        output->clear();
        return input_was_empty ? TRIM_NONE : positions;
    }

    // Trim the whitespace.
    *output = input.substr(first_good_char, last_good_char - first_good_char + 1);

    // Return where we trimmed from.
    return static_cast<TrimPositions>(((first_good_char == 0) ? TRIM_NONE : TRIM_LEADING) | ((last_good_char == last_char) ? TRIM_NONE : TRIM_TRAILING));
}

template<typename STR>
STR CollapseWhitespaceT(const STR& text, bool trim_sequences_with_line_breaks) {
    STR result;
    result.resize(text.size());

    // Set flags to pretend we're already in a trimmed whitespace sequence, so we
    // will trim any leading whitespace.
    bool in_whitespace = true;
    bool already_trimmed = true;

    int chars_written = 0;
    for (typename STR::const_iterator i(text.begin()); i != text.end(); ++i) {
        if (IsWhitespace(*i)) {
            if (!in_whitespace) {
                // Reduce all whitespace sequences to a single space.
                in_whitespace = true;
                result[chars_written++] = L' ';
            }
            if (trim_sequences_with_line_breaks && !already_trimmed &&
                ((*i == '\n') || (*i == '\r'))) {
                // Whitespace sequences containing CR or LF are eliminated entirely.
                already_trimmed = true;
                --chars_written;
            }
        } else {
            // Non-whitespace chracters are copied straight across.
            in_whitespace = false;
            already_trimmed = false;
            result[chars_written++] = *i;
        }
    }

    if (in_whitespace && !already_trimmed) {
        // Any trailing whitespace is eliminated.
        --chars_written;
    }

    result.resize(chars_written);
    return result;
}

template<typename STR>
static bool ContainsOnlyCharsT(const STR& input, const STR& characters) {
    for (typename STR::const_iterator iter = input.begin(); iter != input.end(); ++iter) {
        if (characters.find(*iter) == STR::npos)
            return false;
    }
    return true;
}

template<typename Iter>
static inline bool DoLowerCaseEqualsASCII(Iter a_begin,
    Iter a_end,
    const char* b) {
    for (Iter it = a_begin; it != a_end; ++it, ++b) {
        if (!*b || ToLowerASCII(*it) != *b)
            return false;
    }
    return *b == 0;
}

template <typename STR>
bool StartsWithT(const STR& str, const STR& search, bool case_sensitive) {
    if (case_sensitive) {
        return str.compare(0, search.length(), search) == 0;
    } else {
        if (search.size() > str.size())
            return false;
        return std::equal(search.begin(), search.end(), str.begin(),
            CaseInsensitiveCompare<typename STR::value_type>());
    }
}

template <typename STR>
bool EndsWithT(const STR& str, const STR& search, bool case_sensitive) {
    typename STR::size_type str_length = str.length();
    typename STR::size_type search_length = search.length();
    if (search_length > str_length)
        return false;
    if (case_sensitive) {
        return str.compare(str_length - search_length, search_length, search) == 0;
    }
    else {
        return std::equal(search.begin(), search.end(),
            str.begin() + (str_length - search_length),
            CaseInsensitiveCompare<typename STR::value_type>());
    }
}

template<class StringType>
void DoReplaceSubstringsAfterOffset(StringType* str,
    typename StringType::size_type start_offset,
    const StringType& find_this,
    const StringType& replace_with,
    bool replace_all) {
    if ((start_offset == StringType::npos) || (start_offset >= str->length()))
        return;

    for (typename StringType::size_type offs(str->find(find_this, start_offset));
        offs != StringType::npos; offs = str->find(find_this, offs)) {
        str->replace(offs, find_this.length(), replace_with);
        offs += replace_with.length();

        if (!replace_all)
            break;
    }
}

template<typename STR>
static size_t TokenizeT(const STR& str, const STR& delimiters, std::vector<STR>* tokens) {
    tokens->clear();

    typename STR::size_type start = str.find_first_not_of(delimiters);
    while (start != STR::npos) {
        typename STR::size_type end = str.find_first_of(delimiters, start + 1);
        if (end == STR::npos) {
            tokens->push_back(str.substr(start));
            break;
        }
        else {
            tokens->push_back(str.substr(start, end - start));
            start = str.find_first_not_of(delimiters, end + 1);
        }
    }

    return tokens->size();
}

template<typename STR>
static STR JoinStringT(const std::vector<STR>& parts, const STR& sep) {
    if (parts.empty())
        return STR();

    STR result(parts[0]);
    typename std::vector<STR>::const_iterator iter = parts.begin();
    ++iter;

    for (; iter != parts.end(); ++iter) {
        result += sep;
        result += *iter;
    }

    return result;
}

template<class FormatStringType, class OutStringType>
OutStringType DoReplaceStringPlaceholders(const FormatStringType& format_string,
    const std::vector<OutStringType>& subst, std::vector<size_t>* offsets) {
    size_t substitutions = subst.size();

    size_t sub_length = 0;
    for (typename std::vector<OutStringType>::const_iterator iter = subst.begin();
        iter != subst.end(); ++iter) {
        sub_length += iter->length();
    }

    OutStringType formatted;
    formatted.reserve(format_string.length() + sub_length);

    std::vector<ReplacementOffset> r_offsets;
    for (typename FormatStringType::const_iterator i = format_string.begin();
        i != format_string.end(); ++i) {
        if ('$' == *i) {
            if (i + 1 != format_string.end()) {
                ++i;
                if ('$' == *i) {
                    while (i != format_string.end() && '$' == *i) {
                        formatted.push_back('$');
                        ++i;
                    }
                    --i;
                } else {
                    uintptr_t index = 0;
                    while (i != format_string.end() && '0' <= *i && *i <= '9') {
                        index *= 10;
                        index += *i - '0';
                        ++i;
                    }
                    --i;
                    index -= 1;
                    if (offsets) {
                        ReplacementOffset r_offset(index, static_cast<int>(formatted.size()));
                        r_offsets.insert(std::lower_bound(r_offsets.begin(), r_offsets.end(), r_offset, &CompareParameter), r_offset);
                    }
                    if (index < substitutions)
                        formatted.append(subst.at(index));
                }
            }
        }
        else {
            formatted.push_back(*i);
        }
    }
    if (offsets) {
        for (std::vector<ReplacementOffset>::const_iterator i = r_offsets.begin();
            i != r_offsets.end(); ++i) {
            offsets->push_back(i->offset);
        }
    }
    return formatted;
}

} // namespace internal

namespace x {

static inline char* strdup(const char* str) {
    return _strdup(str);
}

// C standard-library functions like "strncasecmp" and "snprintf" that aren't
// cross-platform are provided as "base::strncasecmp", and their prototypes
// are listed below.  These functions are then implemented as inline calls
// to the platform-specific equivalents in the platform-specific headers.

// Compares the two strings s1 and s2 without regard to case using
// the current locale; returns 0 if they are equal, 1 if s1 > s2, and -1 if
// s2 > s1 according to a lexicographic comparison.
static int strcasecmp(const char* s1, const char* s2) {
    return _stricmp(s1, s2);
}

// Compares up to count characters of s1 and s2 without regard to case using
// the current locale; returns 0 if they are equal, 1 if s1 > s2, and -1 if
// s2 > s1 according to a lexicographic comparison.
static int strncasecmp(const char* s1, const char* s2, size_t count) {
    return _strnicmp(s1, s2, count);
}

// Same as strncmp but for char16 strings.
static int strncmp16(const char16* s1, const char16* s2, size_t count) {
    return ::wcsncmp(s1, s2, count);
}

// Wrapper for vsnprintf that always null-terminates and always returns the
// number of characters that would be in an untruncated formatted
// string, even when truncation occurs.
static inline int vsnprintf(char* buffer, size_t size, const char* format, va_list arguments) PRINTF_FORMAT(3, 0) {
    int length = _vsprintf_p(buffer, size, format, arguments);
    if (length < 0) {
        if (size > 0) buffer[0] = 0;
        return _vscprintf_p(format, arguments);
    }
    return length;
}

// vswprintf always null-terminates, but when truncation occurs, it will either
// return -1 or the number of characters that would be in an untruncated
// formatted string.  The actual return value depends on the underlying
// C library's vswprintf implementation.
static int vswprintf(wchar_t* buffer, size_t size, const wchar_t* format, va_list arguments) WPRINTF_FORMAT(3, 0) {
    int length = _vswprintf_p(buffer, size, format, arguments);
    if (length < 0) {
        if (size > 0) buffer[0] = 0;
        return _vscwprintf_p(format, arguments);
    }
    return length;
}

// Some of these implementations need to be inlined.

// We separate the declaration from the implementation of this inline
// function just so the PRINTF_FORMAT works.
static int snprintf(char* buffer, size_t size, const char* format, ...) PRINTF_FORMAT(3, 4) {
    va_list arguments;
    va_start(arguments, format);
    int result = vsnprintf(buffer, size, format, arguments);
    va_end(arguments);
    return result;
}

// We separate the declaration from the implementation of this inline
// function just so the WPRINTF_FORMAT works.
static int swprintf(wchar_t* buffer, size_t size, const wchar_t* format, ...) WPRINTF_FORMAT(3, 4) {
    va_list arguments;
    va_start(arguments, format);
    int result = vswprintf(buffer, size, format, arguments);
    va_end(arguments);
    return result;
}


// BSD-style safe and consistent string copy functions.
// Copies |src| to |dst|, where |dst_size| is the total allocated size of |dst|.
// Copies at most |dst_size|-1 characters, and always NULL terminates |dst|, as
// long as |dst_size| is not 0.  Returns the length of |src| in characters.
// If the return value is >= dst_size, then the output was truncated.
// NOTE: All sizes are in number of characters, NOT in bytes.
static size_t strlcpy(char* dst, const char* src, size_t dst_size) {
    return ::internal::lcpyT<char>(dst, src, dst_size);
}
static size_t wcslcpy(wchar_t* dst, const wchar_t* src, size_t dst_size) {
    return ::internal::lcpyT<wchar_t>(dst, src, dst_size);
}

// Scan a wprintf format string to determine whether it's portable across a
// variety of systems.  This function only checks that the conversion
// specifiers used by the format string are supported and have the same meaning
// on a variety of systems.  It doesn't check for other errors that might occur
// within a format string.
//
// Nonportable conversion specifiers for wprintf are:
//  - 's' and 'c' without an 'l' length modifier.  %s and %c operate on char
//     data on all systems except Windows, which treat them as wchar_t data.
//     Use %ls and %lc for wchar_t data instead.
//  - 'S' and 'C', which operate on wchar_t data on all systems except Windows,
//     which treat them as char data.  Use %ls and %lc for wchar_t data
//     instead.
//  - 'F', which is not identified by Windows wprintf documentation.
//  - 'D', 'O', and 'U', which are deprecated and not available on all systems.
//     Use %ld, %lo, and %lu instead.
//
// Note that there is no portable conversion specifier for char data when
// working with wprintf.
//
// This function is intended to be called from base::vswprintf.
static bool IsWprintfFormatPortable(const wchar_t* format) {
    for (const wchar_t* position = format; *position != '\0'; ++position) {
        if (*position == '%') {
            bool in_specification = true;
            bool modifier_l = false;
            while (in_specification) {
                // Eat up characters until reaching a known specifier.
                if (*++position == '\0') {
                    // The format string ended in the middle of a specification.  Call
                    // it portable because no unportable specifications were found.  The
                    // string is equally broken on all platforms.
                    return true;
                }

                if (*position == 'l') {
                    // 'l' is the only thing that can save the 's' and 'c' specifiers.
                    modifier_l = true;
                } else if (((*position == 's' || *position == 'c') && !modifier_l) ||
                    *position == 'S' || *position == 'C' || *position == 'F' ||
                    *position == 'D' || *position == 'O' || *position == 'U') {
                    // Not portable.
                    return false;
                }

                if (wcschr(L"diouxXeEfgGaAcspn%", *position)) {
                    // Portable, keep scanning the rest of the format string.
                    in_specification = false;
                }
            }
        }
    }
    return true;
}


// Replaces characters in |replace_chars| from anywhere in |input| with
// |replace_with|.  Each character in |replace_chars| will be replaced with
// the |replace_with| string.  Returns true if any characters were replaced.
// |replace_chars| must be null-terminated.
// NOTE: Safe to use the same variable for both |input| and |output|.
static bool ReplaceChars(const std::wstring& input, const char16 replace_chars[], const std::wstring& replace_with, std::wstring* output) {
    return ::internal::ReplaceCharsT(input, replace_chars, replace_with, output);
}
static bool ReplaceChars(const std::string& input, const char replace_chars[], const std::string& replace_with, std::string* output) {
    return ::internal::ReplaceCharsT(input, replace_chars, replace_with, output);
}


// Removes characters in |remove_chars| from anywhere in |input|.  Returns true
// if any characters were removed.  |remove_chars| must be null-terminated.
// NOTE: Safe to use the same variable for both |input| and |output|.
static bool RemoveChars(const std::wstring& input, const char16 remove_chars[], std::wstring* output) {
    return ReplaceChars(input, remove_chars, std::wstring(), output);
}
static bool RemoveChars(const std::string& input, const char remove_chars[], std::string* output) {
    return ReplaceChars(input, remove_chars, std::string(), output);
}

// Removes characters in |trim_chars| from the beginning and end of |input|.
// |trim_chars| must be null-terminated.
// NOTE: Safe to use the same variable for both |input| and |output|.
static bool TrimString(const std::wstring& input, const char16 trim_chars[], std::wstring* output) {
    return ::internal::TrimStringT(input, trim_chars, TRIM_ALL, output) != TRIM_NONE;
}
static bool TrimString(const std::string& input, const char trim_chars[], std::string* output) {
    return ::internal::TrimStringT(input, trim_chars, TRIM_ALL, output) != TRIM_NONE;
}

// Trims any whitespace from either end of the input string.  Returns where
// whitespace was found.
// The non-wide version has two functions:
// * TrimWhitespaceASCII()
//   This function is for ASCII strings and only looks for ASCII whitespace;
// Please choose the best one according to your usage.
// NOTE: Safe to use the same variable for both input and output.
static TrimPositions TrimWhitespace(const std::wstring& input, TrimPositions positions, std::wstring* output) {
    return ::internal::TrimStringT(input, kWhitespaceUTF16, positions, output);
}
static TrimPositions TrimWhitespaceASCII(const std::string& input, TrimPositions positions, std::string* output) {
    return ::internal::TrimStringT(input, kWhitespaceASCII, positions, output);
}

// Deprecated. This function is only for backward compatibility and calls
// TrimWhitespaceASCII().
static TrimPositions TrimWhitespace(const std::string& input, TrimPositions positions, std::string* output) {
    return TrimWhitespaceASCII(input, positions, output);
}

// Searches  for CR or LF characters.  Removes all contiguous whitespace
// strings that contain them.  This is useful when trying to deal with text
// copied from terminals.
// Returns |text|, with the following three transformations:
// (1) Leading and trailing whitespace is trimmed.
// (2) If |trim_sequences_with_line_breaks| is true, any other whitespace
//     sequences containing a CR or LF are trimmed.
// (3) All other whitespace sequences are converted to single spaces.
static std::wstring CollapseWhitespace(const std::wstring& text, bool trim_sequences_with_line_breaks) {
    return ::internal::CollapseWhitespaceT(text, trim_sequences_with_line_breaks);
}
static std::string CollapseWhitespaceASCII(const std::string& text, bool trim_sequences_with_line_breaks) {
    return ::internal::CollapseWhitespaceT(text, trim_sequences_with_line_breaks);
}

// Returns true if the passed string is empty or contains only white-space
// characters.
static bool ContainsOnlyWhitespaceASCII(const std::string& str) {
    for (std::string::const_iterator i(str.begin()); i != str.end(); ++i) {
        if (!IsAsciiWhitespace(*i))
            return false;
    }
    return true;
}
static bool ContainsOnlyWhitespace(const std::wstring& str) {
    return str.find_first_not_of(kWhitespaceUTF16) == std::wstring::npos;
}

// Returns true if |input| is empty or contains only characters found in
// |characters|.
static bool ContainsOnlyChars(const std::wstring& input, const std::wstring& characters) {
    return ::internal::ContainsOnlyCharsT(input, characters);
}

static bool ContainsOnlyChars(const std::string& input, const std::string& characters) {
    return ::internal::ContainsOnlyCharsT(input, characters);
}

// Converts the given wide string to the corresponding Latin1. This will fail
// (return false) if any characters are more than 255.
static bool WideToLatin1(const std::wstring& wide, std::string* latin1) {
    std::string output;
    output.resize(wide.size());
    latin1->clear();
    for (size_t i = 0; i < wide.size(); i++) {
        if (wide[i] > 255)
            return false;
        output[i] = static_cast<char>(wide[i]);
    }
    latin1->swap(output);
    return true;
}

// Converts the elements of the given string.  This version uses a pointer to
// clearly differentiate it from the non-pointer variant.
template <class str> inline void StringToLowerASCII(str* s) {
    for (typename str::iterator i = s->begin(); i != s->end(); ++i)
        *i = ToLowerASCII(*i);
}

template <class str> inline str StringToLowerASCII(const str& s) {
    // for std::string and std::wstring
    str output(s);
    StringToLowerASCII(&output);
    return output;
}

// Converts the elements of the given string.  This version uses a pointer to
// clearly differentiate it from the non-pointer variant.
template <class str> inline void StringToUpperASCII(str* s) {
    for (typename str::iterator i = s->begin(); i != s->end(); ++i)
        *i = ToUpperASCII(*i);
}

template <class str> inline str StringToUpperASCII(const str& s) {
    // for std::string and std::wstring
    str output(s);
    StringToUpperASCII(&output);
    return output;
}

// Compare the lower-case form of the given string against the given ASCII
// string.  This is useful for doing checking if an input string matches some
// token, and it is optimized to avoid intermediate string copies.  This API is
// borrowed from the equivalent APIs in Mozilla.
static bool LowerCaseEqualsASCII(const std::string& a, const char* b) {
    return ::internal::DoLowerCaseEqualsASCII(a.begin(), a.end(), b);
}
static bool LowerCaseEqualsASCII(const std::wstring& a, const char* b) {
    return ::internal::DoLowerCaseEqualsASCII(a.begin(), a.end(), b);
}

// Same thing, but with string iterators instead.
static bool LowerCaseEqualsASCII(std::string::const_iterator a_begin, std::string::const_iterator a_end, const char* b) {
    return ::internal::DoLowerCaseEqualsASCII(a_begin, a_end, b);
}
static bool LowerCaseEqualsASCII(std::wstring::const_iterator a_begin, std::wstring::const_iterator a_end, const char* b) {
    return ::internal::DoLowerCaseEqualsASCII(a_begin, a_end, b);
}
static bool LowerCaseEqualsASCII(const char* a_begin, const char* a_end, const char* b) {
    return ::internal::DoLowerCaseEqualsASCII(a_begin, a_end, b);
}
static bool LowerCaseEqualsASCII(const char16* a_begin, const char16* a_end, const char* b) {
    return ::internal::DoLowerCaseEqualsASCII(a_begin, a_end, b);
}

// Performs a case-sensitive string compare. The behavior is undefined if both
// strings are not ASCII.
static bool EqualsASCII(const std::wstring& a, const std::string& b) {
    if (a.length() != b.length()) return false;
    return std::equal(b.begin(), b.end(), a.begin());
}

// Returns true if str starts with search, or false otherwise.
static bool StartsWithASCII(const std::string& str, const std::string& search, bool case_sensitive) {
    if (case_sensitive)
        return str.compare(0, search.length(), search) == 0;
    else
        return strncasecmp(str.c_str(), search.c_str(), search.length()) == 0;
}
static bool StartsWith(const std::wstring& str, const std::wstring& search, bool case_sensitive) {
    return ::internal::StartsWithT(str, search, case_sensitive);
}

// Returns true if str ends with search, or false otherwise.
static bool EndsWith(const std::string& str, const std::string& search, bool case_sensitive) {
    return ::internal::EndsWithT(str, search, case_sensitive);
}
static bool EndsWith(const std::wstring& str, const std::wstring& search, bool case_sensitive) {
    return ::internal::EndsWithT(str, search, case_sensitive);
}


// Starting at |start_offset| (usually 0), replace the first instance of
// |find_this| with |replace_with|.
static void ReplaceFirstSubstringAfterOffset(std::wstring* str, std::wstring::size_type start_offset, const std::wstring& find_this, const std::wstring& replace_with) {
    ::internal::DoReplaceSubstringsAfterOffset(str, start_offset, find_this, replace_with, false);  // replace first instance
}
static void ReplaceFirstSubstringAfterOffset(std::string* str, std::string::size_type start_offset, const std::string& find_this, const std::string& replace_with) {
    ::internal::DoReplaceSubstringsAfterOffset(str, start_offset, find_this, replace_with, false);  // replace first instance
}

// Starting at |start_offset| (usually 0), look through |str| and replace all
// instances of |find_this| with |replace_with|.
//
// This does entire substrings; use std::replace in <algorithm> for single
// characters, for example:
//   std::replace(str.begin(), str.end(), 'a', 'b');
static void ReplaceSubstringsAfterOffset(std::wstring* str, std::wstring::size_type start_offset, const std::wstring& find_this, const std::wstring& replace_with) {
    ::internal::DoReplaceSubstringsAfterOffset(str, start_offset, find_this, replace_with, true);  // replace all instances
}
static void ReplaceSubstringsAfterOffset(std::string* str, std::string::size_type start_offset, const std::string& find_this, const std::string& replace_with) {
    ::internal::DoReplaceSubstringsAfterOffset(str, start_offset, find_this, replace_with, true);  // replace all instances
}

// Reserves enough memory in |str| to accommodate |length_with_null| characters,
// sets the size of |str| to |length_with_null - 1| characters, and returns a
// pointer to the underlying contiguous array of characters.  This is typically
// used when calling a function that writes results into a character array, but
// the caller wants the data to be managed by a string-like object.  It is
// convenient in that is can be used inline in the call, and fast in that it
// avoids copying the results of the call from a char* into a string.
//
// |length_with_null| must be at least 2, since otherwise the underlying string
// would have size 0, and trying to access &((*str)[0]) in that case can result
// in a number of problems.
//
// Internally, this takes linear time because the resize() call 0-fills the
// underlying array for potentially all
// (|length_with_null - 1| * sizeof(string_type::value_type)) bytes.  Ideally we
// could avoid this aspect of the resize() call, as we expect the caller to
// immediately write over this memory, but there is no other way to set the size
// of the string, and not doing that will mean people who access |str| rather
// than str.c_str() will get back a string of whatever size |str| had on entry
// to this function (probably 0).
template <class string_type>
static typename string_type::value_type* WriteInto(string_type* str,
    size_t length_with_null) {
    str->reserve(length_with_null);
    str->resize(length_with_null - 1);
    return &((*str)[0]);
}

//-----------------------------------------------------------------------------

// Splits a string into its fields delimited by any of the characters in
// |delimiters|.  Each field is added to the |tokens| vector.  Returns the
// number of tokens found.
static size_t Tokenize(const std::wstring& str, const std::wstring& delimiters, std::vector<std::wstring>* tokens) {
    return ::internal::TokenizeT(str, delimiters, tokens);
}
static size_t Tokenize(const std::string& str, const std::string& delimiters, std::vector<std::string>* tokens) {
    return ::internal::TokenizeT(str, delimiters, tokens);
}

static size_t Tokenize(const std::wstring& str, const char16& delimiters, std::vector<std::wstring>* tokens) {
    return Tokenize(str, std::wstring(1, delimiters), tokens);
}
static size_t Tokenize(const std::string& str, const char& delimiters, std::vector<std::string>* tokens) {
    return Tokenize(str, std::string(1, delimiters), tokens);
}

// Join |parts| using |separator|.
static std::string JoinString(const std::vector<std::string>& parts, const std::string& separator) {
    return ::internal::JoinStringT(parts, separator);
}
static std::wstring JoinString(const std::vector<std::wstring>& parts, const std::wstring& separator) {
    return ::internal::JoinStringT(parts, separator);
}

// Does the opposite of SplitString().
static std::wstring JoinString(const std::vector<std::wstring>& parts, char16 s) {
    return JoinString(parts, std::wstring(1, s));
}
static std::string JoinString(const std::vector<std::string>& parts, char s) {
    return JoinString(parts, std::string(1, s));
}

// Replace $1-$2-$3..$9 in the format string with |a|-|b|-|c|..|i| respectively.
// Additionally, any number of consecutive '$' characters is replaced by that
// number less one. Eg $$->$, $$$->$$, etc. The offsets parameter here can be
// NULL. This only allows you to use up to nine replacements.
static std::wstring ReplaceStringPlaceholders(const std::wstring& format_string, const std::vector<std::wstring>& subst, std::vector<size_t>* offsets) {
    return ::internal::DoReplaceStringPlaceholders(format_string, subst, offsets);
}

static std::string ReplaceStringPlaceholders(const std::string& format_string, const std::vector<std::string>& subst, std::vector<size_t>* offsets) {
    return ::internal::DoReplaceStringPlaceholders(format_string, subst, offsets);
}

namespace internal {

template<class FormatStringType>
FormatStringType DoReplaceStringPlaceholders(const FormatStringType& format_string, const FormatStringType& a, size_t* offset) {
    std::vector<size_t> offsets;
    std::vector<FormatStringType> subst;
    subst.push_back(a);
    FormatStringType result = ReplaceStringPlaceholders(format_string, subst, &offsets);

    if (offset) {
        *offset = offsets[0];
    }
    return result;
    
}

}

// Single-string shortcut for ReplaceStringHolders. |offset| may be NULL.
static std::wstring ReplaceStringPlaceholders(const std::wstring& format_string, const std::wstring& a, size_t* offset) {
    return internal::DoReplaceStringPlaceholders(format_string, a, offset);
}

static std::string ReplaceStringPlaceholders(const std::string& format_string, const std::string& a, size_t* offset) {
    return internal::DoReplaceStringPlaceholders(format_string, a, offset);
}

} // namespace x

#endif // !UTILS_STL_UTIL_INCLUDE_H_
