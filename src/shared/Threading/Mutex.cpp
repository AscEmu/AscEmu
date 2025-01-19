/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Mutex.hpp"

Mutex::Mutex() 
{
    // No need for explicit initialization as std::recursive_mutex handles it
}

Mutex::~Mutex() 
{
    // Destructor is automatic, no need for explicit destruction as std::recursive_mutex manages it
}

bool Mutex::attemptAcquire()
{
    // Use try_lock for non-blocking attempt to acquire the mutex
    return mtx.try_lock();
}

void Mutex::acquire()
{
    // Use lock to block and acquire the mutex
    mtx.lock();
}

void Mutex::release()
{
    // Use unlock to release the mutex
    mtx.unlock();
}
