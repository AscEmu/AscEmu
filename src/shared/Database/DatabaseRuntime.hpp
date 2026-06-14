/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Database.hpp"
#include "../Threading/ThreadPool.hpp"

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class DatabaseRuntime final
{
public:
    struct QueryExecutionResult
    {
        bool success = false;
        std::unique_ptr<QueryResult> result;
    };

    DatabaseRuntime(Database& owner, uint16_t workerCount) : m_owner(owner),
        m_pool("AE-Database", sanitizeWorkerCount(workerCount), sanitizeWorkerCount(workerCount), sanitizeWorkerCount(workerCount))
    {}

    ~DatabaseRuntime()
    {
        stop();
    }

    DatabaseRuntime(const DatabaseRuntime&) = delete;
    DatabaseRuntime& operator=(const DatabaseRuntime&) = delete;
    DatabaseRuntime(DatabaseRuntime&&) = delete;
    DatabaseRuntime& operator=(DatabaseRuntime&&) = delete;

    void start()
    {
        m_stopping.store(false);

        if (!m_pool.isStarted())
            m_pool.start();
    }

    void stop()
    {
        m_stopping.store(true);
        m_connectionCv.notify_all();

        if (m_pool.isStarted())
        {
            m_pool.shutdown();
            m_pool.join();
        }
    }

    void primeConnections(const std::vector<std::unique_ptr<DatabaseConnection>>& connections)
    {
        std::scoped_lock lock(m_connectionMutex);

        m_availableConnections.clear();
        for (const auto& connection : connections)
        {
            if (connection != nullptr)
                m_availableConnections.push_back(connection.get());
        }

        m_connectionCv.notify_all();
    }

    template <typename Fn>
    decltype(auto) withConnection(Fn&& fn)
    {
        auto lease = acquireConnection();
        if (!lease)
            throw std::runtime_error("DatabaseRuntime is stopping");

        return std::forward<Fn>(fn)(*lease.get());
    }

    [[nodiscard]] std::future<QueryExecutionResult> enqueueQuery(std::string sql)
    {
        if (m_stopping.load() || !m_pool.isStarted())
            return makeReadyFuture(QueryExecutionResult{});

        auto promise = std::make_shared<std::promise<QueryExecutionResult>>();
        auto future = promise->get_future();

        m_pool.addHighPriorityTask(
            [this, promise, sql = std::move(sql)]() mutable
            {
                runQuery(std::move(sql), *promise);
            },
            "db.query");

        return future;
    }

    [[nodiscard]] std::future<bool> enqueueExecute(std::string sql)
    {
        if (m_stopping.load() || !m_pool.isStarted())
            return makeReadyFuture(false);

        auto promise = std::make_shared<std::promise<bool>>();
        auto future = promise->get_future();

        m_pool.addTask(
            [this, promise, sql = std::move(sql)]() mutable
            {
                runExecute(std::move(sql), *promise);
            },
            "db.execute");

        return future;
    }

    [[nodiscard]] std::future<bool> enqueueBatch(std::vector<std::string> sqlBatch)
    {
        if (m_stopping.load() || !m_pool.isStarted())
            return makeReadyFuture(false);

        auto promise = std::make_shared<std::promise<bool>>();
        auto future = promise->get_future();

        m_pool.addTask(
            [this, promise, sqlBatch = std::move(sqlBatch)]() mutable
            {
                runBatch(std::move(sqlBatch), *promise);
            },
            "db.batch");

        return future;
    }

    void enqueueTask(std::function<void(DatabaseConnection&)> task, std::string taskName = "db.task")
    {
        if (m_stopping.load() || !m_pool.isStarted())
            return;

        m_pool.addTask(
            [this, task = std::move(task)]() mutable
            {
                auto lease = acquireConnection();
                if (!lease)
                    return;

                task(*lease.get());
            },
            std::move(taskName));
    }

    [[nodiscard]] size_t queuedTaskCount() const
    {
        return m_pool.queuedTaskCount();
    }

    [[nodiscard]] size_t workerCount() const
    {
        return m_pool.workerCount();
    }

    [[nodiscard]] uint64_t completedTaskCount() const
    {
        return m_pool.completedTaskCount();
    }

private:
    class ConnectionLease
    {
    public:
        ConnectionLease() = default;

        ConnectionLease(
            DatabaseRuntime* runtime,
            DatabaseConnection* connection,
            std::unique_lock<std::recursive_mutex>&& busyLock)
            : m_runtime(runtime),
            m_connection(connection),
            m_busyLock(std::move(busyLock))
        {}

        ConnectionLease(const ConnectionLease&) = delete;
        ConnectionLease& operator=(const ConnectionLease&) = delete;

        ConnectionLease(ConnectionLease&& other) noexcept = default;
        ConnectionLease& operator=(ConnectionLease&& other) noexcept = default;

        ~ConnectionLease()
        {
            reset();
        }

        explicit operator bool() const noexcept
        {
            return m_connection != nullptr;
        }

        DatabaseConnection* get() const noexcept
        {
            return m_connection;
        }

        void reset()
        {
            if (m_runtime != nullptr && m_connection != nullptr)
                m_runtime->releaseConnection(m_connection, std::move(m_busyLock));

            m_runtime = nullptr;
            m_connection = nullptr;
        }

    private:
        DatabaseRuntime* m_runtime = nullptr;
        DatabaseConnection* m_connection = nullptr;
        std::unique_lock<std::recursive_mutex> m_busyLock;
    };

    static uint16_t sanitizeWorkerCount(uint16_t workerCount)
    {
        return workerCount == 0 ? 1 : workerCount;
    }

    template <typename T>
    static std::future<T> makeReadyFuture(T value)
    {
        std::promise<T> promise;
        promise.set_value(std::move(value));
        return promise.get_future();
    }

    ConnectionLease acquireConnection()
    {
        std::unique_lock lock(m_connectionMutex);
        m_connectionCv.wait(lock, [this]
            {
                return m_stopping.load() || !m_availableConnections.empty();
            });

        if (m_stopping.load() || m_availableConnections.empty())
            return {};

        DatabaseConnection* connection = m_availableConnections.front();
        m_availableConnections.pop_front();
        lock.unlock();

        std::unique_lock<std::recursive_mutex> busyLock(connection->Busy);
        return ConnectionLease(this, connection, std::move(busyLock));
    }

    void releaseConnection(DatabaseConnection* connection, std::unique_lock<std::recursive_mutex> busyLock)
    {
        if (connection == nullptr)
            return;

        if (busyLock.owns_lock())
            busyLock.unlock();

        {
            std::scoped_lock lock(m_connectionMutex);
            m_availableConnections.push_back(connection);
        }

        m_connectionCv.notify_one();
    }

    void runQuery(std::string sql, std::promise<QueryExecutionResult>& promise)
    {
        auto lease = acquireConnection();
        if (!lease)
        {
            promise.set_value({});
            return;
        }

        try
        {
            QueryExecutionResult outcome;
            outcome.success = m_owner._sendQuery(lease.get(), sql.c_str(), false);

            if (outcome.success)
                outcome.result = m_owner._storeQueryResult(lease.get());

            promise.set_value(std::move(outcome));
        }
        catch (...)
        {
            promise.set_exception(std::current_exception());
        }
    }

    void runExecute(std::string sql, std::promise<bool>& promise)
    {
        auto lease = acquireConnection();
        if (!lease)
        {
            promise.set_value(false);
            return;
        }

        try
        {
            promise.set_value(m_owner._sendQuery(lease.get(), sql.c_str(), false));
        }
        catch (...)
        {
            promise.set_exception(std::current_exception());
        }
    }

    void runBatch(std::vector<std::string> sqlBatch, std::promise<bool>& promise)
    {
        auto lease = acquireConnection();
        if (!lease)
        {
            promise.set_value(false);
            return;
        }

        try
        {
            bool success = true;
            m_owner._beginTransaction(lease.get());

            for (const auto& sql : sqlBatch)
            {
                if (!m_owner._sendQuery(lease.get(), sql.c_str(), false))
                {
                    success = false;
                    break;
                }
            }

            if (success)
                m_owner._endTransaction(lease.get());
            else
                m_owner._sendQuery(lease.get(), "ROLLBACK", false);

            promise.set_value(success);
        }
        catch (...)
        {
            try
            {
                m_owner._sendQuery(lease.get(), "ROLLBACK", false);
            }
            catch (...)
            {
            }

            promise.set_exception(std::current_exception());
        }
    }

private:
    Database& m_owner;
    AscEmu::Threading::AEThreadPool m_pool;

    std::mutex m_connectionMutex;
    std::condition_variable m_connectionCv;
    std::deque<DatabaseConnection*> m_availableConnections;

    std::atomic_bool m_stopping{ false };
};
