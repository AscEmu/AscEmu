/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <mutex>
#include <stdexcept>

class Mutex
{
public:
    Mutex();
    ~Mutex();

    bool attemptAcquire();
    void acquire();
    void release();

private:
    std::recursive_mutex mtx; // Using std::recursive_mutex for recursive locking behavior
};
