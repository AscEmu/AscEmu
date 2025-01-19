/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "Mutex.hpp"
#include <deque>

template<class TYPE>
class LockedQueue
{
    public:
        ~LockedQueue() = default;

        inline void add(const TYPE & element)
        {
            mutex.acquire();
            queue.push_back(element);
            mutex.release();
        }

        inline TYPE next()
        {
            mutex.acquire();
            assert(queue.size() > 0);
            TYPE t = queue.front();
            queue.pop_front();
            mutex.release();
            return t;
        }

        inline size_t size()
        {
            mutex.acquire();
            size_t c = queue.size();
            mutex.release();
            return c;
        }

        inline TYPE get_first_element()
        {
            mutex.acquire();
            TYPE t;
            if(queue.size() == 0)
                t = reinterpret_cast<TYPE>(0);
            else
                t = queue.front();
            mutex.release();
            return t;
        }

        inline void pop()
        {
            mutex.acquire();
            ASSERT(queue.size() > 0);
            queue.pop_front();
            mutex.release();
        }

        inline void clear()
        {
            mutex.acquire();
            queue.resize(0);
            mutex.release();
        }

    protected:

        std::deque<TYPE> queue;
        Mutex mutex;
};

#endif      //_THREADING_LOCKEDQUEUE_H
