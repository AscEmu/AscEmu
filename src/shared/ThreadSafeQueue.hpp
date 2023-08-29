/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>

template<class T>
class ThreadSafeQueue
{
public:

    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() {}

    void push(T _element)
    {
        std::unique_lock lock(m_lock);

        m_queue.push(_element);
    }

    void pushWait(T _element)
    {
        std::unique_lock lock(m_lock);

        m_queue.push(_element);
        m_condition.notify_one();
    }

    T pop()
    {
        std::unique_lock lock(m_lock);

        if (!m_queue.empty())
        {
            T element = m_queue.front();
            m_queue.pop();
            return element;
        }

        return nullptr;
    }

    T popWait()
    {
        std::unique_lock lock(m_lock);

        m_condition.wait(lock, [this]() { return !m_queue.empty(); });

        T element = m_queue.front();
        m_queue.pop();

        return element;
    }

    bool hasItems()
    {
        return !m_queue.empty();
    }

private:
    std::queue<T> m_queue;
    std::mutex m_lock;
    std::condition_variable m_condition;
};
