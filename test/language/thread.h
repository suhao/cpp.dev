#pragma once

#include <Windows.h>

volatile int a = 0;

#include <thread>
void ThreadFunction() {
    while (a < 2000)
    {
        a++;
        Sleep(10);
    }

    Sleep(1000 * 5);
}

void thread_test() {
    auto thread = std::thread(&ThreadFunction);
    while (a < 200)
    {
        Sleep(10);
    }
    thread.join();
    a = 3;
}