/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once
#include "Common.hpp"

namespace Arcemu
{
    namespace Utility
    {
        template<typename T>
        class TLSObject
        {
                FastMutex createlock_;    //Lock held during creation and destruction.
#ifdef _WIN32
                DWORD key_;
#elif defined(__APPLE__) || defined(__unix__)
                pthread_key_t key_;
#else
#error Your platform does not support Thread Local Storage mechanism!
#endif
            public:
                TLSObject() { create(); }
                ~TLSObject() { destroy(); }

                //************************************
                // Purpose : Does any instantiation required by the tls mechanism. Called in constructor.
                // Return : void
                //************************************
                void create()
                {
                    createlock_.Acquire();
#if defined(__APPLE__) || defined(__unix__)
                    pthread_key_create(&key_, NULL);
#elif defined(_WIN32)
                    key_ = TlsAlloc();
#endif
                    createlock_.Release();
                }

                //************************************
                // Purpose : Sets the given value to a tls slot of the calling thread.
                // Parameter:    T - value to store in the tls slot
                // Return : void
                //************************************
                inline void set(T val)
                {
#if defined(__APPLE__) || defined(__unix__)
                    pthread_setspecific(key_, static_cast<const void*>(val));
#elif _WIN32
                    TlsSetValue(key_, static_cast<LPVOID>(val));
#endif
                }

                //************************************
                // Purpose : Returns a reference to the stored tls value.
                // Return : T
                //************************************
                inline T get() const
                {
#if defined(__APPLE__) || defined(__unix__)
                    return static_cast<T>(pthread_getspecific(key_));
#elif _WIN32
                    return static_cast<T>(TlsGetValue(key_));
#else
                    return static_cast<T>(NULL);
#endif
                }

                //************************************
                // Purpose : Does any necessary cleanup required by the tls mechanism. Called in destructor
                // Return : void
                //************************************
                void destroy()
                {
                    createlock_.Acquire();
#if defined(__APPLE__) || defined(__unix__)
                    pthread_key_delete(key_);
#elif _WIN32
                    TlsFree(key_);
#endif
                    createlock_.Release();
                }
                void operator=(T val) { set(val); }
                bool operator==(T val) const { return get() == val; }
                bool operator!=(T val) const { return operator==(val); }
        };
    }
}
