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

 //////////////////////////////////////////////
 // Notes: .Execute is ASYNC! -
 // We should probably be using QueryBuffer for ASYNC and NONE-ASYNC queries to not lag the emu.
 // See: Player::_SavePetSpells for example of usage
 // updated: Tuesday, June 16th 2009 - Hasbro
 //////////////////////////////////////////////

#include "Utilities/Util.hpp"
#include "Utilities/CallBack.h"

#include <string>
#include <vector>
#include <cstdarg>

#include "Debugging/Errors.h"

using AscEmu::Threading::AEThread;
using std::unique_ptr;
using std::make_unique;

SQLCallbackBase::~SQLCallbackBase()
{

}

void Database::createDbConnection()
{
    if (m_dbConnection == nullptr)
        m_dbConnection = GetFreeConnection();
}

void Database::destroyDbConnection()
{
    if (m_dbConnection)
    {
        m_dbConnection->Busy.release();
        m_dbConnection = nullptr;
    }
}

void Database::createQueryBufferConnection()
{
    if (m_queryBufferConnection == nullptr)
        m_queryBufferConnection = GetFreeConnection();
}

void Database::dbThreadRunner(AEThread& /*thread*/)
{
    dbRunAllQueries();
}

void Database::dbThreadShutdown()
{
    // Shut down thread
    m_dbThread->killAndJoin();

    // Execute remaining queries
    dbRunAllQueries();
    destroyDbConnection();
}

Database::Database()
{
    _counter = 0;
    mConnectionCount = -1;   // Not connected.
    //ThreadRunning = true;
    mPort = 3306;
    qt = nullptr;

    m_dbConnection = nullptr;
    m_queryBufferConnection = nullptr;
}

Database::~Database()
{

}

void Database::_Initialize()
{
    if (m_dbThread == nullptr)
        m_dbThread = std::make_unique<AEThread>("DatabaseThread", [this](AEThread& thread) { this->dbThreadRunner(thread); }, std::chrono::milliseconds(10));
    else
        m_dbThread->reboot();

    if (m_queryBufferThread == nullptr)
        m_queryBufferThread = std::make_unique<AEThread>("QueryBufferThread", [this](AEThread& thread) { this->queryBufferThreadRunner(thread); }, std::chrono::milliseconds(10));
    else
        m_queryBufferThread->reboot();
}

DatabaseConnection* Database::GetFreeConnection()
{
    uint32_t i = 0;
    for (;;)
    {
        DatabaseConnection* con = Connections[((i++) % mConnectionCount)].get();
        if (con->Busy.attemptAcquire())
            return con;
    }
}

// Use this when we request data that can return a value (not async)
std::unique_ptr<QueryResult> Database::Query(const char* QueryString, ...)
{
    va_list vlist;
    va_start(vlist, QueryString);

    // Get buffer size
    va_list vlist_copy;
    va_copy(vlist_copy, vlist);
    const auto size = vsnprintf(nullptr, 0, QueryString, vlist_copy);
    va_end(vlist_copy);

    if (size < 0)
    {
        va_end(vlist);
        return nullptr;
    }

    std::string sql(size, '\0');
    vsnprintf(&sql[0], static_cast<size_t>(size) + 1, QueryString, vlist);
    va_end(vlist);

    // Send the query
    std::unique_ptr<QueryResult> qResult;
    DatabaseConnection* con = GetFreeConnection();

    if (_SendQuery(con, sql.data(), false))
        qResult = _StoreQueryResult(con);

    con->Busy.release();
    return qResult;
}

std::unique_ptr<QueryResult> Database::Query(bool *success, const char* QueryString, ...)
{
    va_list vlist;
    va_start(vlist, QueryString);

    // Get buffer size
    va_list vlist_copy;
    va_copy(vlist_copy, vlist);
    const auto size = vsnprintf(nullptr, 0, QueryString, vlist_copy);
    va_end(vlist_copy);

    if (size < 0)
    {
        va_end(vlist);
        *success = false;
        return nullptr;
    }

    std::string sql(size, '\0');
    vsnprintf(&sql[0], static_cast<size_t>(size) + 1, QueryString, vlist);
    va_end(vlist);

    // Send the query
    std::unique_ptr<QueryResult> qResult;
    DatabaseConnection* con = GetFreeConnection();

    if (_SendQuery(con, sql.data(), false))
    {
        qResult = _StoreQueryResult(con);
        *success = true;
    }
    else
    {
        *success = false;
    }

    con->Busy.release();
    return qResult;
}

std::unique_ptr<QueryResult> Database::QueryNA(const char* QueryString)
{
    // Send the query
    std::unique_ptr<QueryResult> qResult;
    DatabaseConnection* con = GetFreeConnection();

    if (_SendQuery(con, QueryString, false))
        qResult = _StoreQueryResult(con);

    con->Busy.release();
    return qResult;
}

std::unique_ptr<QueryResult> Database::FQuery(const char* QueryString, DatabaseConnection* con)
{
    // Send the query
    std::unique_ptr<QueryResult> qResult;
    if (_SendQuery(con, QueryString, false))
        qResult = _StoreQueryResult(con);

    return qResult;
}

void Database::FWaitExecute(const char* QueryString, DatabaseConnection* con)
{
    // Send the query
    _SendQuery(con, QueryString, false);
}

void QueryBuffer::AddQuery(const char* format, ...)
{
    va_list vlist;
    va_start(vlist, format);

    // Get buffer size
    va_list vlist_copy;
    va_copy(vlist_copy, vlist);
    const auto size = vsnprintf(nullptr, 0, format, vlist_copy);
    va_end(vlist_copy);

    if (size < 0)
    {
        va_end(vlist);
        return;
    }

    std::string queryBuf(size, '\0');
    vsnprintf(&queryBuf[0], static_cast<size_t>(size) + 1, format, vlist);
    va_end(vlist);

    queries.push_back(std::move(queryBuf));
}

void QueryBuffer::AddQueryNA(const char* str)
{
    queries.emplace_back(str);
}

void Database::destroyQueryBufferConnection()
{
    if (m_queryBufferConnection)
    {
        m_queryBufferConnection->Busy.release();
        m_queryBufferConnection = nullptr;
    }
}

void Database::dbRunAllQueries()
{
    while (auto query = queries_queue.pop())
    {
        createDbConnection();
        _SendQuery(m_dbConnection, query.value().data(), false);
    }
    // No more queries => free the connection in the thread and reconnect when there are new queries
    destroyDbConnection();
}

void Database::queryBufferThreadRunner(AEThread& /*thread*/)
{
    queryBufferRunAllQueries();
}

void Database::queryBufferThreadShutdown() {
    m_queryBufferThread->killAndJoin();
    queryBufferRunAllQueries();
    destroyQueryBufferConnection();
}

void Database::queryBufferRunAllQueries()
{
    while (auto query = query_buffer.pop())
    {
        createQueryBufferConnection();
        PerformQueryBuffer(query.value().get(), m_queryBufferConnection);
    }
    // No more queries => free the connection in the thread and reconnect when there are new queries
    destroyQueryBufferConnection();
}

void QueryBuffer::AddQueryStr(const std::string & str)
{
    queries.emplace_back(str);
}

void Database::PerformQueryBuffer(QueryBuffer* b, DatabaseConnection* ccon)
{
    if (!b->queries.size())
        return;

    DatabaseConnection* con = ccon;
    if (ccon == NULL)
        con = GetFreeConnection();

    _BeginTransaction(con);

    for (auto itr = b->queries.begin(); itr != b->queries.end(); ++itr)
    {
        _SendQuery(con, (*itr).data(), false);
    }

    _EndTransaction(con);

    if (ccon == NULL)
        con->Busy.release();
}
// Use this when we do not have a result. ex: INSERT into SQL 1
bool Database::Execute(const char* QueryString, ...)
{
    va_list vlist;
    va_start(vlist, QueryString);

    // Get buffer size
    va_list vlist_copy;
    va_copy(vlist_copy, vlist);
    const auto size = vsnprintf(nullptr, 0, QueryString, vlist_copy);
    va_end(vlist_copy);

    if (size < 0)
    {
        va_end(vlist);
        return false;
    }

    std::string queryBuf(size, '\0');
    vsnprintf(&queryBuf[0], static_cast<size_t>(size) + 1, QueryString, vlist);
    va_end(vlist);

    if (m_dbThread->isKilled())
        return WaitExecuteNA(queryBuf.data());

    queries_queue.push(std::move(queryBuf));
    return true;
}

bool Database::ExecuteNA(const char* QueryString)
{
    if (m_dbThread->isKilled())
        return WaitExecuteNA(QueryString);

    queries_queue.push(std::string(QueryString));
    return true;
}

// Wait till the other queries are done, then execute
bool Database::WaitExecute(const char* QueryString, ...)
{
    va_list vlist;
    va_start(vlist, QueryString);

    // Get buffer size
    va_list vlist_copy;
    va_copy(vlist_copy, vlist);
    const auto size = vsnprintf(nullptr, 0, QueryString, vlist);
    va_end(vlist_copy);

    if (size < 0)
    {
        va_end(vlist);
        return false;
    }

    std::string sql(size, '\0');
    vsnprintf(&sql[0], static_cast<size_t>(size) + 1, QueryString, vlist);
    va_end(vlist);

    DatabaseConnection* con = GetFreeConnection();
    bool Result = _SendQuery(con, sql.data(), false);
    con->Busy.release();
    return Result;
}

bool Database::WaitExecuteNA(const char* QueryString)
{
    DatabaseConnection* con = GetFreeConnection();
    bool Result = _SendQuery(con, QueryString, false);
    con->Busy.release();
    return Result;
}

void AsyncQuery::AddQuery(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);

    // Get buffer size
    va_list ap_copy;
    va_copy(ap_copy, ap);
    const auto size = vsnprintf(nullptr, 0, format, ap_copy);
    va_end(ap_copy);

    if (size < 0)
    {
        va_end(ap);
        return;
    }

    std::string queryBuf(size, '\0');
    vsnprintf(&queryBuf[0], static_cast<size_t>(size) + 1, format, ap);
    va_end(ap);

    queries.emplace_back(nullptr, std::move(queryBuf));
}

void AsyncQuery::Perform()
{
    DatabaseConnection* conn = db->GetFreeConnection();
    for (std::vector<AsyncQueryResult>::iterator itr = queries.begin(); itr != queries.end(); ++itr)
        itr->result = db->FQuery(itr->query.data(), conn);

    conn->Busy.release();
    func->run(queries);
}

AsyncQuery::AsyncQuery(std::unique_ptr<SQLCallbackBase> f) : func(std::move(f)), db(nullptr)
{}

AsyncQuery::~AsyncQuery() = default;

void Database::EndThreads()
{
    if (m_dbThread)
        m_dbThread->requestKill();
    if (m_queryBufferThread)
        m_queryBufferThread->requestKill();

    dbThreadShutdown();
    queryBufferThreadShutdown();
}


void Database::QueueAsyncQuery(std::unique_ptr<AsyncQuery> query)
{
    query->db = this;
    query->Perform();
}

void Database::AddQueryBuffer(std::unique_ptr<QueryBuffer> b)
{
    // TODO: qt is always nullptr
    if (qt != NULL)
        query_buffer.push(std::move(b));
    else
        PerformQueryBuffer(b.get(), NULL);
}
