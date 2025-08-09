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
 */

#ifndef _DATABASE_H
#define _DATABASE_H

#include "Field.hpp"
#include "Threading/ThreadSafeQueue.hpp"
#include "Threading/Mutex.hpp"
#include "Threading/AEThread.h"
#include "CommonTypes.hpp"
#include <string>

class QueryResult;
class QueryThread;
class Database;
class SQLCallbackBase;

struct DatabaseConnection
{
    virtual ~DatabaseConnection() = default;
    Mutex Busy;
};

struct SERVER_DECL AsyncQueryResult
{
    std::unique_ptr<QueryResult> result;
    std::string query;
};

class SERVER_DECL AsyncQuery
{
    friend class Database;

        std::unique_ptr<SQLCallbackBase> func;
        std::vector<AsyncQueryResult> queries;
        Database* db;

    public:

        AsyncQuery(std::unique_ptr<SQLCallbackBase> f);
        ~AsyncQuery();
        void AddQuery(const char* format, ...);
        void Perform();
        inline void SetDB(Database* dbb) { db = dbb; }
};

class SERVER_DECL QueryBuffer
{
        std::vector<std::string> queries;
    public:

        friend class Database;
        void AddQuery(const char* format, ...);
        void AddQueryNA(const char* str);
        void AddQueryStr(const std::string & str);
};

class SERVER_DECL Database
{
    friend class QueryThread;
    friend class AsyncQuery;

    DatabaseConnection* m_dbConnection;
    void createDbConnection();
    void destroyDbConnection();

    DatabaseConnection* m_queryBufferConnection;
    void createQueryBufferConnection();
    void destroyQueryBufferConnection();

    std::unique_ptr<AscEmu::Threading::AEThread> m_dbThread;
    void dbThreadRunner(AscEmu::Threading::AEThread& thread);
    void dbThreadShutdown();
    void dbRunAllQueries();

    std::unique_ptr<AscEmu::Threading::AEThread> m_queryBufferThread;
    void queryBufferThreadRunner(AscEmu::Threading::AEThread& thread);
    void queryBufferThreadShutdown();
    void queryBufferRunAllQueries();

    public:

        Database();
        virtual ~Database();

        //////////////////////////////////////////////////////////////////////////////////////////
        // Thread Stuff
        //////////////////////////////////////////////////////////////////////////////////////////
        bool runThread();

        //////////////////////////////////////////////////////////////////////////////////////////
        // Virtual Functions
        //////////////////////////////////////////////////////////////////////////////////////////
        virtual bool Initialize(const char* Hostname, unsigned int port,
                                const char* Username, const char* Password, const char* DatabaseName,
                                uint32_t ConnectionCount, uint32_t BufferSize, bool useLegacyAuth = false) = 0;

        virtual void Shutdown() = 0;

        virtual std::unique_ptr<QueryResult> Query(const char* QueryString, ...);
        virtual std::unique_ptr<QueryResult> Query(bool *success, const char* QueryString, ...);
        virtual std::unique_ptr<QueryResult> QueryNA(const char* QueryString);
        virtual std::unique_ptr<QueryResult> FQuery(const char* QueryString, DatabaseConnection* con);
        virtual void FWaitExecute(const char* QueryString, DatabaseConnection* con);
        virtual bool WaitExecute(const char* QueryString, ...);//Wait For Request Completion
        virtual bool WaitExecuteNA(const char* QueryString);//Wait For Request Completion
        virtual bool Execute(const char* QueryString, ...);
        virtual bool ExecuteNA(const char* QueryString);

        // Initialized on load: Database::Database() : CThread()
        //bool ThreadRunning;

        const std::string & GetHostName() { return mHostname; }
        const std::string & GetDatabaseName() { return mDatabaseName; }
        size_t GetQueueSize() { return queries_queue.getSize(); }

        virtual std::string EscapeString(std::string Escape) = 0;
        virtual void EscapeLongString(const char* str, uint32_t len, std::stringstream & out) = 0;
        virtual std::string EscapeString(const char* esc, DatabaseConnection* con) = 0;

        void QueueAsyncQuery(std::unique_ptr<AsyncQuery> query);
        void EndThreads();

        DatabaseConnection* GetFreeConnection();

        void PerformQueryBuffer(QueryBuffer* b, DatabaseConnection* ccon);
        void AddQueryBuffer(std::unique_ptr<QueryBuffer> b);

        static std::unique_ptr<Database> CreateDatabaseInterface();
        static void CleanupLibs();

        virtual bool SupportsReplaceInto() = 0;
        virtual bool SupportsTableLocking() = 0;

    protected:

        // spawn threads and shizzle
        void _Initialize();

        virtual void _BeginTransaction(DatabaseConnection* conn) = 0;
        virtual void _EndTransaction(DatabaseConnection* conn) = 0;

        // actual query function
        virtual bool _SendQuery(DatabaseConnection* con, const char* Sql, bool Self) = 0;
        virtual std::unique_ptr<QueryResult> _StoreQueryResult(DatabaseConnection* con) = 0;

        //////////////////////////////////////////////////////////////////////////////////////////
        ThreadSafeQueue<std::unique_ptr<QueryBuffer>> query_buffer;

        //////////////////////////////////////////////////////////////////////////////////////////
        ThreadSafeQueue<std::string> queries_queue;
        std::vector<std::unique_ptr<DatabaseConnection>> Connections;

        uint32_t _counter;
        //////////////////////////////////////////////////////////////////////////////////////////

        int32_t mConnectionCount;

        // For reconnecting a broken connection
        std::string mHostname;
        std::string mUsername;
        std::string mPassword;
        std::string mDatabaseName;
        uint32_t mPort;

        QueryThread* qt;
};

class SERVER_DECL QueryResult
{
    public:

        QueryResult(uint32_t fields, uint32_t rows) : mFieldCount(fields), mRowCount(rows), mCurrentRow(NULL) {}
        virtual ~QueryResult() {}

        virtual bool NextRow() = 0;

        inline Field* Fetch() { return mCurrentRow.get(); }
        inline uint32_t GetFieldCount() const { return mFieldCount; }
        inline uint32_t GetRowCount() const { return mRowCount; }

    protected:

        uint32_t mFieldCount;
        uint32_t mRowCount;
        std::unique_ptr<Field[]> mCurrentRow;
};

#endif      //_DATABASE_H
