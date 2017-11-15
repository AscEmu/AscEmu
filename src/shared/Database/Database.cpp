/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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

#include "DatabaseEnv.h"
#include "Util.hpp"
#include <string>
#include <vector>

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
        m_dbConnection->Busy.Release();
        m_dbConnection = nullptr;
    }
}

void Database::createQueryBufferConnection()
{
    if (m_queryBufferConnection == nullptr)
        m_queryBufferConnection = GetFreeConnection();
}

void Database::dbThreadRunner(AEThread& thread)
{
    dbRunAllQueries();
}

void Database::dbThreadShutdown()
{
    // Shut down thread
    m_dbThread->join();

    // Execute remaining queries
    dbRunAllQueries();
    destroyDbConnection();
}

Database::Database()
{
    _counter = 0;
    Connections = NULL;
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
    uint32 i = 0;
    for (;;)
    {
        DatabaseConnection* con = Connections[((i++) % mConnectionCount)];
        if (con->Busy.AttemptAcquire())
            return con;
    }
}

// Use this when we request data that can return a value (not async)
QueryResult* Database::Query(const char* QueryString, ...)
{
    char sql[16384];
    va_list vlist;
    va_start(vlist, QueryString);
    vsnprintf(sql, 16384, QueryString, vlist);
    va_end(vlist);

    // Send the query
    QueryResult* qResult = NULL;
    DatabaseConnection* con = GetFreeConnection();

    if (_SendQuery(con, sql, false))
        qResult = _StoreQueryResult(con);

    con->Busy.Release();
    return qResult;
}

QueryResult* Database::Query(bool *success, const char* QueryString, ...)
{
    char sql[16384];
    va_list vlist;
    va_start(vlist, QueryString);
    vsnprintf(sql, 16384, QueryString, vlist);
    va_end(vlist);

    // Send the query
    QueryResult* qResult = NULL;
    DatabaseConnection* con = GetFreeConnection();

    if (_SendQuery(con, sql, false))
    {
        qResult = _StoreQueryResult(con);
        *success = true;
    }
    else
    {
        *success = false;
    }

    con->Busy.Release();
    return qResult;
}

QueryResult* Database::QueryNA(const char* QueryString)
{
    // Send the query
    QueryResult* qResult = NULL;
    DatabaseConnection* con = GetFreeConnection();

    if (_SendQuery(con, QueryString, false))
        qResult = _StoreQueryResult(con);

    con->Busy.Release();
    return qResult;
}

QueryResult* Database::FQuery(const char* QueryString, DatabaseConnection* con)
{
    // Send the query
    QueryResult* qResult = NULL;
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
    char query[16384];
    va_list vlist;
    va_start(vlist, format);
    vsnprintf(query, 16384, format, vlist);
    va_end(vlist);

    size_t len = strlen(query);
    char* pBuffer = new char[len + 1];
    memcpy(pBuffer, query, len + 1);

    queries.push_back(pBuffer);
}

void QueryBuffer::AddQueryNA(const char* str)
{
    size_t len = strlen(str);
    char* pBuffer = new char[len + 1];
    memcpy(pBuffer, str, len + 1);

    queries.push_back(pBuffer);
}

void Database::destroyQueryBufferConnection()
{
    if (m_queryBufferConnection)
    {
        m_queryBufferConnection->Busy.Release();
        m_queryBufferConnection = nullptr;
    }
}

void Database::dbRunAllQueries()
{
    while (auto query = queries_queue.pop())
    {
        createDbConnection();
        _SendQuery(m_dbConnection, query, false);
        delete[] query;
    }
}

void Database::queryBufferThreadRunner(AEThread& thread)
{
    queryBufferRunAllQueries();
}

void Database::queryBufferThreadShutdown() {
    m_queryBufferThread->join();
    queryBufferRunAllQueries();
    destroyQueryBufferConnection();
}

void Database::queryBufferRunAllQueries()
{
    while (auto query = query_buffer.pop())
    {
        createQueryBufferConnection();
        PerformQueryBuffer(query, m_queryBufferConnection);
        delete query;
    }
}

void QueryBuffer::AddQueryStr(const std::string & str)
{
    size_t len = str.size();
    char* pBuffer = new char[len + 1];
    memcpy(pBuffer, str.c_str(), len + 1);

    queries.push_back(pBuffer);
}

void Database::PerformQueryBuffer(QueryBuffer* b, DatabaseConnection* ccon)
{
    if (!b->queries.size())
        return;

    DatabaseConnection* con = ccon;
    if (ccon == NULL)
        con = GetFreeConnection();

    _BeginTransaction(con);

    for (std::vector<char*>::iterator itr = b->queries.begin(); itr != b->queries.end(); ++itr)
    {
        _SendQuery(con, *itr, false);
        delete[](*itr);
    }

    _EndTransaction(con);

    if (ccon == NULL)
        con->Busy.Release();
}
// Use this when we do not have a result. ex: INSERT into SQL 1
bool Database::Execute(const char* QueryString, ...)
{
    char query[16384];

    va_list vlist;
    va_start(vlist, QueryString);
    vsnprintf(query, 16384, QueryString, vlist);
    va_end(vlist);

    if (m_dbThread->isKilled())
        return WaitExecuteNA(query);

    size_t len = strlen(query);
    char* pBuffer = new char[len + 1];
    memcpy(pBuffer, query, len + 1);

    queries_queue.push(pBuffer);
    return true;
}

bool Database::ExecuteNA(const char* QueryString)
{
    if (m_dbThread->isKilled())
        return WaitExecuteNA(QueryString);

    size_t len = strlen(QueryString);
    char* pBuffer = new char[len + 1];
    memcpy(pBuffer, QueryString, len + 1);

    queries_queue.push(pBuffer);
    return true;
}

// Wait till the other queries are done, then execute
bool Database::WaitExecute(const char* QueryString, ...)
{
    char sql[16384];
    va_list vlist;
    va_start(vlist, QueryString);
    vsnprintf(sql, 16384, QueryString, vlist);
    va_end(vlist);

    DatabaseConnection* con = GetFreeConnection();
    bool Result = _SendQuery(con, sql, false);
    con->Busy.Release();
    return Result;
}

bool Database::WaitExecuteNA(const char* QueryString)
{
    DatabaseConnection* con = GetFreeConnection();
    bool Result = _SendQuery(con, QueryString, false);
    con->Busy.Release();
    return Result;
}

void AsyncQuery::AddQuery(const char* format, ...)
{
    AsyncQueryResult res;
    va_list ap;
    char buffer[10000];
    size_t len;
    va_start(ap, format);
    vsnprintf(buffer, 10000, format, ap);
    va_end(ap);
    len = strlen(buffer);
    ASSERT(len);
    res.query = new char[len + 1];
    res.query[len] = 0;
    memcpy(res.query, buffer, len);
    res.result = NULL;
    queries.push_back(res);
}

void AsyncQuery::Perform()
{
    DatabaseConnection* conn = db->GetFreeConnection();
    for (std::vector<AsyncQueryResult>::iterator itr = queries.begin(); itr != queries.end(); ++itr)
        itr->result = db->FQuery(itr->query, conn);

    conn->Busy.Release();
    func->run(queries);

    delete this;
}

AsyncQuery::~AsyncQuery()
{
    delete func;
    for (std::vector<AsyncQueryResult>::iterator itr = queries.begin(); itr != queries.end(); ++itr)
    {
        if (itr->result)
            delete itr->result;

        delete[] itr->query;
    }
}

void Database::EndThreads()
{
    if (m_dbThread)
        m_dbThread->requestKill();
    if (m_queryBufferThread)
        m_queryBufferThread->requestKill();

    dbThreadShutdown();
    queryBufferThreadShutdown();
}


void Database::QueueAsyncQuery(AsyncQuery* query)
{
    query->db = this;
    query->Perform();
}

void Database::AddQueryBuffer(QueryBuffer* b)
{
    if (qt != NULL)
        query_buffer.push(b);
    else
    {
        PerformQueryBuffer(b, NULL);
        delete b;
    }
}

void Database::FreeQueryResult(QueryResult* p)
{
    delete p;
}
