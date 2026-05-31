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

template<class T>
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
            std::lock_guard lock{m_lock};
            m_queue.push(std::move(element));
        }

        m_condition.notify_one();
    }

    void pushWait(T element)
    {
        push(std::move(element));
    }

    [[nodiscard]] std::optional<T> pop()
    {
        std::lock_guard lock{m_lock};

        if (m_queue.empty())
            return std::nullopt;

        T element = std::move(m_queue.front());
        m_queue.pop();

        return element;
    }

    T popWait()
    {
        std::unique_lock lock{m_lock};

        m_condition.wait(lock, [this]
        {
            return !m_queue.empty();
        });

        T element = std::move(m_queue.front());
        m_queue.pop();

        return element;
    }

    [[nodiscard]] std::optional<T> front() const
    {
        std::lock_guard lock{m_lock};

        if (m_queue.empty())
            return std::nullopt;

        return m_queue.front();
    }

    void clear()
    {
        std::lock_guard lock{m_lock};

        std::queue<T> empty;
        m_queue.swap(empty);
    }

    [[nodiscard]] bool hasItems() const
    {
        std::lock_guard lock{m_lock};
        return !m_queue.empty();
    }

    [[nodiscard]] std::size_t getSize() const
    {
        std::lock_guard lock{m_lock};
        return m_queue.size();
    }

    // Compatibility helpers for old LockedQueue API.
    void add(const T& element)
    {
        push(element);
    }

    void add(T&& element)
    {
        push(std::move(element));
    }

    [[nodiscard]] T next()
    {
        auto element = pop();

        if (element)
            return std::move(*element);

        return T{};
    }

    [[nodiscard]] std::size_t size() const
    {
        return getSize();
    }

    [[nodiscard]] T get_first_element() const
    {
        auto element = front();

        if (element)
            return *element;

        return T{};
    }

    void pop_front()
    {
        std::lock_guard lock{m_lock};

        if (!m_queue.empty())
            m_queue.pop();
    }

private:
    mutable std::mutex m_lock;
    std::condition_variable m_condition;
    std::queue<T> m_queue;
};
