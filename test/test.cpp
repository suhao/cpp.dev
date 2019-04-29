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

int main() {
    std::cout << "Hello test!\n";

    // TODO: The next test suite.

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