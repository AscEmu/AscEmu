/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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

#include <deque>
#include <mutex>

template<class TYPE>
class LockedQueue
{
    public:
        ~LockedQueue() = default;

        inline void add(const TYPE & element)
        {
            std::lock_guard lock{mutex};
            queue.push_back(element);
        }

        inline TYPE next()
        {
            std::lock_guard lock{mutex};

            assert(queue.size() > 0);

            TYPE t = queue.front();
            queue.pop_front();

            return t;
        }

        inline size_t size()
        {
            std::lock_guard lock{mutex};

            return queue.size();
        }

        inline TYPE get_first_element()
        {
            std::lock_guard lock{mutex};

            if(queue.empty())
                return TYPE{};

            TYPE t = queue.front();

            return t;
        }

        inline void pop()
        {
            std::lock_guard lock{mutex};

            ASSERT(queue.size() > 0);
            queue.pop_front();
        }

        inline void clear()
        {
            std::lock_guard lock{mutex};

            queue.resize(0);
        }

    protected:

        std::deque<TYPE> queue;
        mutable std::mutex mutex;
};

#endif      //_THREADING_LOCKEDQUEUE_H
