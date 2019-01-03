/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
 */

#ifndef _THREADING_LOCKEDQUEUE_H
#define _THREADING_LOCKEDQUEUE_H

#include "Mutex.h"
#include <deque>

template<class TYPE>
class LockedQueue
{
    public:
        ~LockedQueue()
        {

        }

        inline void add(const TYPE & element)
        {
            mutex.Acquire();
            queue.push_back(element);
            mutex.Release();
        }

        inline TYPE next()
        {
            mutex.Acquire();
            assert(queue.size() > 0);
            TYPE t = queue.front();
            queue.pop_front();
            mutex.Release();
            return t;
        }

        inline size_t size()
        {
            mutex.Acquire();
            size_t c = queue.size();
            mutex.Release();
            return c;
        }

        inline TYPE get_first_element()
        {
            mutex.Acquire();
            TYPE t;
            if(queue.size() == 0)
                t = reinterpret_cast<TYPE>(0);
            else
                t = queue.front();
            mutex.Release();
            return t;
        }

        inline void pop()
        {
            mutex.Acquire();
            ASSERT(queue.size() > 0);
            queue.pop_front();
            mutex.Release();
        }

        inline void clear()
        {
            mutex.Acquire();
            queue.resize(0);
            mutex.Release();
        }

    protected:

        std::deque<TYPE> queue;
        Mutex mutex;
};

#endif      //_THREADING_LOCKEDQUEUE_H
