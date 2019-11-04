////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http://ant.sh). All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
////////////////////////////////////////////////////////////////////////////////

#if defined(COMPILER_MSVC)
// We usually use the _CrtDumpMemoryLeaks() with the DEBUGER and CRT library to
// check a memory leak.
#if defined(_DEBUG) && _MSC_VER > 1000 // VC++ DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define  DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#if defined(DEBUG_NEW)
#define new DEBUG_NEW
#endif // DEBUG_NEW
#endif // VC++ DEBUG
#endif // defined(COMPILER_MSVC)



#include <iostream>
#include <string>

#include "driver/windows/dolby.h"

#include "function/function.h"

#include "bug/virtual_inheritance.h"

#include "language/stl.h"

int main() {
    std::cout << "Hello test!\n";

    // TODO: The next test suite.

    enum class State {
      kUnresolved = 0,
      kResolved,
      kRejected,
      kCanceled,
      kLastValue = kCanceled
    };
    uintptr_t value = static_cast<uintptr_t>(State::kLastValue);
    // Keep setting 1's to the right of the first one until there are only 1's.
    // In each iteration we double the number of 1's that we set. At last add 1
    // and we have the next power of 2.
    auto i = [&]() {
      for (size_t i = 1; i < sizeof(uintptr_t) * 8; i <<= 1) {
        value |= value >> i;
      }
      return value + 1;
    }();



    test_virtual_inheritance();

    function_test();

    auto result = dolby_version();
    std::cout << "Dolby version:" << result << "\n";


    // The end!
    std::cout << "End test! \n";
    // getchar();
    // std::string input;
    // std::cin >> input;
    system("PAUSE");
    return 0;
}