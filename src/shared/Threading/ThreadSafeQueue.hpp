/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

template <typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() = default;

    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    ThreadSafeQueue(ThreadSafeQueue&&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

    void push(T element)
    {
        {
            std::lock_guard lock{ m_lock };
            m_queue.push(std::move(element));
        }

        m_condition.notify_one();
    }

    [[nodiscard]] std::optional<T> tryPop()
    {
        std::lock_guard lock{ m_lock };

        if (m_queue.empty())
            return std::nullopt;

        T element = std::move(m_queue.front());
        m_queue.pop();

        return element;
    }

    T waitPop()
    {
        std::unique_lock lock{ m_lock };

        m_condition.wait(lock, [this]
            {
                return !m_queue.empty();
            });

        T element = std::move(m_queue.front());
        m_queue.pop();

        return element;
    }

    [[nodiscard]] std::optional<T> tryFront() const
    {
        std::lock_guard lock{ m_lock };

        if (m_queue.empty())
            return std::nullopt;

        return m_queue.front();
    }

    bool discardFront()
    {
        std::lock_guard lock{ m_lock };

        if (m_queue.empty())
            return false;

        m_queue.pop();
        return true;
    }

    void clear()
    {
        std::lock_guard lock{ m_lock };

        std::queue<T> empty;
        m_queue.swap(empty);
    }

    [[nodiscard]] bool empty() const
    {
        std::lock_guard lock{ m_lock };
        return m_queue.empty();
    }

    [[nodiscard]] bool hasItems() const
    {
        return !empty();
    }

    [[nodiscard]] std::size_t size() const
    {
        std::lock_guard lock{ m_lock };
        return m_queue.size();
    }

    [[nodiscard]] std::size_t getSize() const
    {
        return size();
    }

private:
    mutable std::mutex m_lock;
    std::condition_variable m_condition;
    std::queue<T> m_queue;
};
