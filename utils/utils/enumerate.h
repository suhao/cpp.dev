/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef UTILS_ENUMERATE_INCLUDE_H_ 
#define UTILS_ENUMERATE_INCLUDE_H_ 

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "utils/basictypes.h"
#include "utils/stl_util.h"


namespace utils {

template <typename Type>
std::map<Type, std::string> parse(std::string input) {
#ifdef _DEBUG
    std::cout << "ready to parse enum: " << input << std::endl;
#endif // _DEBUG

    std::vector<std::string> tokens;
    auto removed = utils::RemoveChars(input, " ()\r\t\n", &input);
    auto size =
        utils::Tokenize(input, ',', &tokens);  // Token: [Name | Name=EnumValue]
#ifdef _DEBUG
    std::cout << "parse enum: " << input << "->size:" << size;
#endif // _DEBUG

    Type index = 0;
    std::map<Type, std::string> result;
    for (auto iter = tokens.begin(); iter != tokens.end(); ++iter) {
        std::string name = *iter;
        if (iter->find('=') != std::string::npos) {
            std::vector<std::string> pair;
            auto size = utils::Tokenize(*iter, '=', &pair);
#ifdef _DEBUG
            if (size != 2) __debugbreak();
#endif // _DEBUG
            name = pair[0];                         // key = static_cast<Type>(pair[1]);
            if (std::is_unsigned<Type>::value) index = static_cast<Type>(std::stoull(pair[1], 0, 0));
            else index = static_cast<Type>(std::stoll(pair[1], 0, 0));
        }
        result[index++] = name;
    }
    return result;
}

} // namespace utils

#if _MSC_VER > 1910
// We should check the MSVC Version, otherwise maybe assist std::underlying_type.
// https://cplusplus.github.io/LWG/issue2396

template<typename Enumeration>
auto enumerate_cast(Enumeration const value)-> typename std::underlying_type<Enumeration>::type {
    return static_cast<std::underlying_type<Enumeration>::type>(value);
}

template<typename Enumeration, typename Type>
auto enumerate_cast(Type const value) -> typename Enumeration {
    return static_cast<Enumeration>(value);
}

#else

template<typename Enumeration>
auto enumerate_cast(Enumeration const value)-> int {
    return static_cast<int>(value);
}

template<typename Enumeration>
auto enumerate_cast(int const value) -> typename Enumeration {
    return static_cast<Enumeration>(value);
}

#endif // _MSC_VER >= 1900


// https://stackoverflow.com/questions/28828957/enum-to-string-in-modern-c11-c14-c17-and-future-c20
// 
#define DECLARE_ENUM_WITH_TYPE(Name, Type, ...)                                             \
    enum class Name : Type {                                                                \
        __VA_ARGS__                                                                         \
    };                                                                                      \
    static std::map<Type, std::string> Name##NamedMap(                                      \
      utils::parse<Type>(#__VA_ARGS__));                                                    \
    size_t enumeratesize(Name key) { (void)key; return TestEnumClassNamedMap.size(); }      \
    std::string operator*(Name key) { return Name##NamedMap[static_cast<Type>(key)]; }      \
    std::string operator+(std::string &&str, Name key) { return str + *key; }               \
    std::string operator+(Name key, std::string &&str) { return *key + str; }               \
    std::string &operator+=(std::string &str, Name key) { str += *key; return str; }        \
    std::ostream &operator<<(std::ostream &os, Name key) { os << *key; return os; }         \
    Name operator++(Name& key) {                                                            \
        auto it = std::Next(Name##NamedMap, static_cast<Type>(key));                        \
        if (it == Name##NamedMap.end()) return key;                                         \
        key = enumerate_cast<Name>(it->first);                                              \
        return key;                                                                         \
    }                                                                                       \
    template<typename T = Name> bool ContainsKey(Type key) { return std::ContainsKey(Name##NamedMap, key); }

#define DECLARE_ENUM(Name, ...) DECLARE_ENUM_WITH_TYPE(Name, int32_t, __VA_ARGS__)


#endif  // !#define (UTILS_ENUMERATE_INCLUDE_H_ )