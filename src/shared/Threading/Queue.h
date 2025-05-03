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
 *
 */

#ifndef FQUEUE_H
#define FQUEUE_H

#include "Mutex.hpp"

template<class T>
class FQueue
{
    public:
        FQueue() { first = last = NULL; size = 0; }
        volatile unsigned int size;

        uint32_t get_size()
        {
            lock.acquire();
            ::uint32_t retval = size;
            lock.release();
            return retval;
        }

        void push(T & item)
        {
            h* p = new h;
            p->value = item;
            p->pNext = NULL;

            lock.acquire();
            if(last != NULL)//have some items
            {
                last->pNext = (h*)p;
                last = p;
                ++size;
            }
            else //first item
            {
                last = first = p;
                size = 1;
            }
            lock.release();
        }

        T pop_nowait() { return pop(); }

        T pop()
        {
            lock.acquire();
            if(size == 0)
            {
                lock.release();
                return NULL;
            }

            h* tmp = first;
            if(tmp == NULL)
            {
                lock.release();
                return NULL;
            }

            if(--size) //more than 1 item
                first = (h*)first->pNext;
            else //last item
            {
                first = last = NULL;
            }

            lock.release();

            T returnVal = tmp->value;
            delete tmp;
            return returnVal;
        }

    private:

        struct h
        {
            T value;
            void* pNext;
        };

        h* first;
        h* last;

        Mutex lock;
};

#endif  //FQUEUE_H
