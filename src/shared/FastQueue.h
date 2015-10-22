/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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

#ifndef _FASTQUEUE_H
#define _FASTQUEUE_H


/// dummy lock to use a non-locked queue.
class DummyLock
{
    public:

        inline void Acquire() { }
        inline void Release() { }
};


/// linked-list style queue
template<class T, class LOCK>
class FastQueue
{
    struct node
    {
        T element;
        node* next;
    };

    node* last;
    node* first;
    LOCK m_lock;

    public:

        FastQueue()
        {
            last = nullptr;
            first = nullptr;
        }

        ~FastQueue()
        {
            Clear();
        }

        void Clear()
        {
            // clear any elements
            while(last != nullptr)
                Pop();
        }

        void Push(T elem)
        {
            m_lock.Acquire();
            node* n = new node;
            if (last)
                last->next = n;
            else
                first = n;

            last = n;
            n->next = nullptr;
            n->element = elem;
            m_lock.Release();
        }

        T Pop()
        {
            m_lock.Acquire();
            if (first == nullptr)
            {
                m_lock.Release();
                return nullptr;
            }

            T ret = first->element;
            node* td = first;
            first = td->next;
            if (!first)
                last = nullptr;

            delete td;
            m_lock.Release();
            return ret;
        }

        T front()
        {
            m_lock.Acquire();
            if (first == nullptr)
            {
                m_lock.Release();
                return nullptr;
            }

            T ret = first->element;
            m_lock.Release();
            return ret;
        }

        void pop_front()
        {
            m_lock.Acquire();
            if (first == nullptr)
            {
                m_lock.Release();
                return;
            }

            node* td = first;
            first = td->next;
            if (!first)
                last = nullptr;

            delete td;
            m_lock.Release();
        }

        bool HasItems()
        {
            bool ret;
            m_lock.Acquire();
            ret = (first != nullptr);
            m_lock.Release();
            return ret;
        }
};

#endif      //_FASTQUEUE_H
