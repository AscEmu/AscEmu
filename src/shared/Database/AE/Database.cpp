/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../Database.h"

#include "../MySQLDatabase.h"
#include "../../Utilities/CallBack.h"
#include "DatabaseRuntime.hpp"

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <optional>
#include <thread>

using AscEmu::Threading::AEThread;
using std::make_unique;
using std::unique_ptr;

namespace
{
    std::optional<std::string> formatSql(const char* format, va_list args)
    {
        if (format == nullptr)
            return std::nullopt;

        va_list argsCopy;
        va_copy(argsCopy, args);

        const int required = std::vsnprintf(nullptr, 0, format, argsCopy);
        va_end(argsCopy);

        if (required < 0)
            return std::nullopt;

        std::string sql(static_cast<std::size_t>(required), '\0');
        std::vsnprintf(sql.data(), sql.size() + 1, format, args);
        return sql;
    }
}

SQLCallbackBase::~SQLCallbackBase() = default;

Database::Database()
{
    _counter = 0;
    mConnectionCount = -1;
    mPort = 3306;
    qt = nullptr;
    m_dbConnection = nullptr;
    m_queryBufferConnection = nullptr;
#ifdef ASCEMU_USE_AE_DATABASE
    m_runtime = nullptr;
#endif
}

Database::~Database() = default;

void Database::_Initialize()
{
}

bool Database::runThread()
{
    return true;
}

void Database::createDbConnection()
{
    if (m_dbConnection == nullptr)
        m_dbConnection = GetFreeConnection();
}

void Database::destroyDbConnection()
{
    if (m_dbConnection != nullptr)
    {
        m_dbConnection->Busy.unlock();
        m_dbConnection = nullptr;
    }
}

void Database::createQueryBufferConnection()
{
    if (m_queryBufferConnection == nullptr)
        m_queryBufferConnection = GetFreeConnection();
}

void Database::destroyQueryBufferConnection()
{
    if (m_queryBufferConnection != nullptr)
    {
        m_queryBufferConnection->Busy.unlock();
        m_queryBufferConnection = nullptr;
    }
}

void Database::dbThreadRunner(AEThread&)
{
    dbRunAllQueries();
}

void Database::dbThreadShutdown()
{
    dbRunAllQueries();
    destroyDbConnection();
}

void Database::queryBufferThreadRunner(AEThread&)
{
    queryBufferRunAllQueries();
}

void Database::queryBufferThreadShutdown()
{
    queryBufferRunAllQueries();
    destroyQueryBufferConnection();
}

void Database::dbRunAllQueries()
{
    while (auto query = queries_queue.tryPop())
        WaitExecuteNA(query.value().c_str());
}

void Database::queryBufferRunAllQueries()
{
    while (auto buffer = query_buffer.tryPop())
        PerformQueryBuffer(buffer.value().get(), nullptr);
}

DatabaseConnection* Database::GetFreeConnection()
{
    for (;;)
    {
        for (auto& connection : Connections)
        {
            if (connection != nullptr && connection->Busy.try_lock())
                return connection.get();
        }

        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
}

std::unique_ptr<QueryResult> Database::Query(const char* queryString, ...)
{
    va_list args;
    va_start(args, queryString);
    const auto sql = formatSql(queryString, args);
    va_end(args);

    if (!sql)
        return nullptr;

#ifdef ASCEMU_USE_AE_DATABASE
    if (m_runtime)
    {
        auto future = m_runtime->enqueueQuery(*sql);
        auto outcome = future.get();
        return std::move(outcome.result);
    }
#endif

    return QueryNA(sql->c_str());
}

std::unique_ptr<QueryResult> Database::Query(bool* success, const char* queryString, ...)
{
    if (success == nullptr)
        return nullptr;

    va_list args;
    va_start(args, queryString);
    const auto sql = formatSql(queryString, args);
    va_end(args);

    if (!sql)
    {
        *success = false;
        return nullptr;
    }

#ifdef ASCEMU_USE_AE_DATABASE
    if (m_runtime)
    {
        auto future = m_runtime->enqueueQuery(*sql);
        auto outcome = future.get();
        *success = outcome.success;
        return std::move(outcome.result);
    }
#endif

    DatabaseConnection* con = GetFreeConnection();
    std::unique_lock<std::recursive_mutex> connectionLock(con->Busy, std::adopt_lock);

    std::unique_ptr<QueryResult> qResult;
    if (_SendQuery(con, sql->c_str(), false))
    {
        qResult = _StoreQueryResult(con);
        *success = true;
    }
    else
    {
        *success = false;
    }

    return qResult;
}

std::unique_ptr<QueryResult> Database::QueryNA(const char* queryString)
{
    if (queryString == nullptr)
        return nullptr;

#ifdef ASCEMU_USE_AE_DATABASE
    if (m_runtime)
    {
        auto future = m_runtime->enqueueQuery(std::string(queryString));
        auto outcome = future.get();
        return std::move(outcome.result);
    }
#endif

    bool success = false;
    return Query(&success, "%s", queryString);
}

std::unique_ptr<QueryResult> Database::FQuery(const char* queryString, DatabaseConnection* con)
{
    if (queryString == nullptr || con == nullptr)
        return nullptr;

    std::unique_ptr<QueryResult> result;
    if (_SendQuery(con, queryString, false))
        result = _StoreQueryResult(con);

    return result;
}

void Database::FWaitExecute(const char* queryString, DatabaseConnection* con)
{
    if (queryString != nullptr && con != nullptr)
        _SendQuery(con, queryString, false);
}

bool Database::Execute(const char* queryString, ...)
{
    va_list args;
    va_start(args, queryString);
    const auto sql = formatSql(queryString, args);
    va_end(args);

    if (!sql)
        return false;

#ifdef ASCEMU_USE_AE_DATABASE
    if (m_runtime)
    {
        [[maybe_unused]] auto future = m_runtime->enqueueExecute(*sql);
        return true;
    }
#endif

    return ExecuteNA(sql->c_str());
}

bool Database::ExecuteNA(const char* queryString)
{
    if (queryString == nullptr)
        return false;

#ifdef ASCEMU_USE_AE_DATABASE
    if (m_runtime)
    {
        [[maybe_unused]] auto future = m_runtime->enqueueExecute(std::string(queryString));
        return true;
    }
#endif

    return WaitExecuteNA(queryString);
}

bool Database::WaitExecute(const char* queryString, ...)
{
    va_list args;
    va_start(args, queryString);
    const auto sql = formatSql(queryString, args);
    va_end(args);

    if (!sql)
        return false;

#ifdef ASCEMU_USE_AE_DATABASE
    if (m_runtime)
    {
        auto future = m_runtime->enqueueExecute(*sql);
        return future.get();
    }
#endif

    return WaitExecuteNA(sql->c_str());
}

bool Database::WaitExecuteNA(const char* queryString)
{
    if (queryString == nullptr)
        return false;

#ifdef ASCEMU_USE_AE_DATABASE
    if (m_runtime)
    {
        auto future = m_runtime->enqueueExecute(std::string(queryString));
        return future.get();
    }
#endif

    DatabaseConnection* con = GetFreeConnection();
    std::unique_lock<std::recursive_mutex> connectionLock(con->Busy, std::adopt_lock);
    return _SendQuery(con, queryString, false);
}

void QueryBuffer::AddQuery(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const auto sql = formatSql(format, args);
    va_end(args);

    if (sql)
        queries.push_back(*sql);
}

void QueryBuffer::AddQueryNA(const char* str)
{
    if (str != nullptr)
        queries.emplace_back(str);
}

void QueryBuffer::AddQueryStr(const std::string& str)
{
    queries.push_back(str);
}

void Database::PerformQueryBuffer(QueryBuffer* buffer, DatabaseConnection* sharedConnection)
{
    if (buffer == nullptr || buffer->queries.empty())
        return;

    DatabaseConnection* connection = sharedConnection;
    std::unique_lock<std::recursive_mutex> connectionLock;

    if (connection == nullptr)
    {
        connection = GetFreeConnection();
        connectionLock = std::unique_lock<std::recursive_mutex>(connection->Busy, std::adopt_lock);
    }

    _BeginTransaction(connection);
    for (const auto& query : buffer->queries)
        _SendQuery(connection, query.c_str(), false);
    _EndTransaction(connection);
}

void AsyncQuery::AddQuery(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const auto sql = formatSql(format, args);
    va_end(args);

    if (sql)
        queries.emplace_back(AsyncQueryResult{ nullptr, *sql });
}

void AsyncQuery::Perform()
{
    if (db == nullptr)
        return;

    DatabaseConnection* connection = db->GetFreeConnection();
    std::unique_lock<std::recursive_mutex> connectionLock(connection->Busy, std::adopt_lock);

    for (auto& query : queries)
        query.result = db->FQuery(query.query.c_str(), connection);

    connectionLock.unlock();

    if (func)
        func->run(queries);
}

AsyncQuery::AsyncQuery(std::unique_ptr<SQLCallbackBase> f) : func(std::move(f)), db(nullptr)
{
}

AsyncQuery::~AsyncQuery() = default;

void Database::QueueAsyncQuery(std::unique_ptr<AsyncQuery> query)
{
    if (query == nullptr)
        return;

    query->db = this;

#ifdef ASCEMU_USE_AE_DATABASE
    if (m_runtime)
    {
        auto sharedQuery = std::shared_ptr<AsyncQuery>(query.release());

        m_runtime->enqueueTask(
            [this, sharedQuery](DatabaseConnection& connection)
            {
                for (auto& item : sharedQuery->queries)
                    item.result = FQuery(item.query.c_str(), &connection);

                if (sharedQuery->func)
                    sharedQuery->func->run(sharedQuery->queries);
            },
            "db.async_query");

        return;
    }
#endif

    query->Perform();
}

void Database::AddQueryBuffer(std::unique_ptr<QueryBuffer> buffer)
{
    if (buffer == nullptr)
        return;

#ifdef ASCEMU_USE_AE_DATABASE
    if (m_runtime)
    {
        [[maybe_unused]] auto future = m_runtime->enqueueBatch(std::move(buffer->queries));
        return;
    }
#endif

    PerformQueryBuffer(buffer.get(), nullptr);
}

void Database::EndThreads()
{
#ifdef ASCEMU_USE_AE_DATABASE
    if (m_runtime)
        m_runtime->stop();
#endif
}

std::unique_ptr<Database> Database::CreateDatabaseInterface()
{
    return make_unique<MySQLDatabase>();
}

void Database::CleanupLibs()
{
}

#ifdef ASCEMU_USE_AE_DATABASE
size_t Database::GetAeQueuedTaskCount() const
{
    return m_runtime ? m_runtime->queuedTaskCount() : 0;
}

size_t Database::GetAeWorkerCount() const
{
    return m_runtime ? m_runtime->workerCount() : 0;
}

uint64_t Database::GetAeCompletedTaskCount() const
{
    return m_runtime ? m_runtime->completedTaskCount() : 0;
}
#endif
