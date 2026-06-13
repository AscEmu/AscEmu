/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Database.hpp"

#include "MySQLDatabase.hpp"
#include "../Utilities/CallBack.h"
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
    m_runtime = nullptr;
}

Database::~Database() = default;

void Database::_Initialize()
{
}

std::unique_ptr<QueryResult> Database::Query(const char* queryString, ...)
{
    va_list args;
    va_start(args, queryString);
    const auto sql = formatSql(queryString, args);
    va_end(args);

    if (!sql || !m_runtime)
        return nullptr;

    auto future = m_runtime->enqueueQuery(*sql);
    auto outcome = future.get();
    return std::move(outcome.result);
}

std::unique_ptr<QueryResult> Database::Query(bool* success, const char* queryString, ...)
{
    if (success == nullptr)
        return nullptr;

    va_list args;
    va_start(args, queryString);
    const auto sql = formatSql(queryString, args);
    va_end(args);

    if (!sql || !m_runtime)
    {
        *success = false;
        return nullptr;
    }

    auto future = m_runtime->enqueueQuery(*sql);
    auto outcome = future.get();
    *success = outcome.success;
    return std::move(outcome.result);
}

std::unique_ptr<QueryResult> Database::QueryNA(const char* queryString)
{
    if (queryString == nullptr || !m_runtime)
        return nullptr;

    auto future = m_runtime->enqueueQuery(std::string(queryString));
    auto outcome = future.get();
    return std::move(outcome.result);
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

    if (m_runtime)
    {
        [[maybe_unused]] auto future = m_runtime->enqueueExecute(*sql);
        return true;
    }

    return ExecuteNA(sql->c_str());
}

bool Database::ExecuteNA(const char* queryString)
{
    if (queryString == nullptr)
        return false;

    if (m_runtime)
    {
        [[maybe_unused]] auto future = m_runtime->enqueueExecute(std::string(queryString));
        return true;
    }

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

    if (m_runtime)
    {
        auto future = m_runtime->enqueueExecute(*sql);
        return future.get();
    }

    return WaitExecuteNA(sql->c_str());
}

bool Database::WaitExecuteNA(const char* queryString)
{
    if (queryString == nullptr || !m_runtime)
        return false;

    auto future = m_runtime->enqueueExecute(std::string(queryString));
    return future.get();
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

void AsyncQuery::AddQuery(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const auto sql = formatSql(format, args);
    va_end(args);

    if (sql)
        queries.emplace_back(AsyncQueryResult{ nullptr, *sql });
}

AsyncQuery::AsyncQuery(std::unique_ptr<SQLCallbackBase> f) : func(std::move(f)), db(nullptr)
{
}

AsyncQuery::~AsyncQuery() = default;

void Database::QueueAsyncQuery(std::unique_ptr<AsyncQuery> query)
{
    if (query == nullptr || !m_runtime)
        return;

    query->db = this;
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
}

void Database::AddQueryBuffer(std::unique_ptr<QueryBuffer> buffer)
{
    if (buffer == nullptr || !m_runtime)
        return;

    [[maybe_unused]] auto future = m_runtime->enqueueBatch(std::move(buffer->queries));
}

void Database::EndThreads()
{
    if (m_runtime)
        m_runtime->stop();
}

std::unique_ptr<Database> Database::CreateDatabaseInterface()
{
    return make_unique<MySQLDatabase>();
}

void Database::CleanupLibs()
{
}

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
