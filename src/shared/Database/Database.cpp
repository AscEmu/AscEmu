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
    m_runtime = nullptr;
}

Database::~Database() = default;

void Database::_initialize()
{
}

std::unique_ptr<QueryResult> Database::query(const char* queryString, ...)
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

std::unique_ptr<QueryResult> Database::query(bool* success, const char* queryString, ...)
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

std::unique_ptr<QueryResult> Database::queryNA(const char* queryString)
{
    if (queryString == nullptr || !m_runtime)
        return nullptr;

    auto future = m_runtime->enqueueQuery(std::string(queryString));
    auto outcome = future.get();
    return std::move(outcome.result);
}

std::unique_ptr<QueryResult> Database::fQuery(const char* queryString, DatabaseConnection* con)
{
    if (queryString == nullptr || con == nullptr)
        return nullptr;

    std::unique_ptr<QueryResult> result;
    if (_sendQuery(con, queryString, false))
        result = _storeQueryResult(con);

    return result;
}

void Database::fWaitExecute(const char* queryString, DatabaseConnection* con)
{
    if (queryString != nullptr && con != nullptr)
        _sendQuery(con, queryString, false);
}

bool Database::execute(const char* queryString, ...)
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

    return executeNA(sql->c_str());
}

bool Database::executeNA(const char* queryString)
{
    if (queryString == nullptr)
        return false;

    if (m_runtime)
    {
        [[maybe_unused]] auto future = m_runtime->enqueueExecute(std::string(queryString));
        return true;
    }

    return waitExecuteNA(queryString);
}

bool Database::waitExecute(const char* queryString, ...)
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

    return waitExecuteNA(sql->c_str());
}

bool Database::waitExecuteNA(const char* queryString)
{
    if (queryString == nullptr || !m_runtime)
        return false;

    auto future = m_runtime->enqueueExecute(std::string(queryString));
    return future.get();
}

void QueryBuffer::addQuery(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const auto sql = formatSql(format, args);
    va_end(args);

    if (sql)
        queries.push_back(*sql);
}

void QueryBuffer::addQueryNA(const char* str)
{
    if (str != nullptr)
        queries.emplace_back(str);
}

void QueryBuffer::addQueryStr(const std::string& str)
{
    queries.push_back(str);
}

void AsyncQuery::addQuery(const char* format, ...)
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

void Database::queueAsyncQuery(std::unique_ptr<AsyncQuery> query)
{
    if (query == nullptr || !m_runtime)
        return;

    query->db = this;
    auto sharedQuery = std::shared_ptr<AsyncQuery>(query.release());

    m_runtime->enqueueTask(
        [this, sharedQuery](DatabaseConnection& connection)
        {
            for (auto& item : sharedQuery->queries)
                item.result = fQuery(item.query.c_str(), &connection);

            if (sharedQuery->func)
                sharedQuery->func->run(sharedQuery->queries);
        },
        "db.async_query");
}

void Database::addQueryBuffer(std::unique_ptr<QueryBuffer> buffer)
{
    if (buffer == nullptr || !m_runtime)
        return;

    [[maybe_unused]] auto future = m_runtime->enqueueBatch(std::move(buffer->queries));
}

void Database::endThreads()
{
    if (m_runtime)
        m_runtime->stop();
}

std::unique_ptr<Database> Database::createDatabaseInterface()
{
    return make_unique<MySQLDatabase>();
}

void Database::cleanupLibs()
{
}

size_t Database::getQueuedTaskCount() const
{
    return m_runtime ? m_runtime->queuedTaskCount() : 0;
}

size_t Database::getWorkerCount() const
{
    return m_runtime ? m_runtime->workerCount() : 0;
}

uint64_t Database::getCompletedTaskCount() const
{
    return m_runtime ? m_runtime->completedTaskCount() : 0;
}
