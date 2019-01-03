/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

#include "CThreads.h"
#include "Field.h"
#include "../Threading/Queue.h"
#include "../CallBack.h"
#include <string>
#include "Threading/AEThread.h"

class QueryResult;
class QueryThread;
class Database;
class SQLCallbackBase;

struct DatabaseConnection
{
    Mutex Busy;
};

struct SERVER_DECL AsyncQueryResult
{
    QueryResult* result;
    char* query;
};

class SERVER_DECL AsyncQuery
{
    friend class Database;

        SQLCallbackBase* func;
        std::vector<AsyncQueryResult> queries;
        Database* db;

    public:

        AsyncQuery(SQLCallbackBase* f) : func(f), db(nullptr) {}
        ~AsyncQuery();
        void AddQuery(const char* format, ...);
        void Perform();
        inline void SetDB(Database* dbb) { db = dbb; }
};

class SERVER_DECL QueryBuffer
{
        std::vector<char*> queries;
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
                                uint32 ConnectionCount, uint32 BufferSize) = 0;

        virtual void Shutdown() = 0;

        virtual QueryResult* Query(const char* QueryString, ...);
        virtual QueryResult* Query(bool *success, const char* QueryString, ...);
        virtual QueryResult* QueryNA(const char* QueryString);
        virtual QueryResult* FQuery(const char* QueryString, DatabaseConnection* con);
        virtual void FWaitExecute(const char* QueryString, DatabaseConnection* con);
        virtual bool WaitExecute(const char* QueryString, ...);//Wait For Request Completion
        virtual bool WaitExecuteNA(const char* QueryString);//Wait For Request Completion
        virtual bool Execute(const char* QueryString, ...);
        virtual bool ExecuteNA(const char* QueryString);

        // Initialized on load: Database::Database() : CThread()
        //bool ThreadRunning;

        const std::string & GetHostName() { return mHostname; }
        const std::string & GetDatabaseName() { return mDatabaseName; }
        uint32 GetQueueSize() { return queries_queue.get_size(); }

        virtual std::string EscapeString(std::string Escape) = 0;
        virtual void EscapeLongString(const char* str, uint32 len, std::stringstream & out) = 0;
        virtual std::string EscapeString(const char* esc, DatabaseConnection* con) = 0;

        void QueueAsyncQuery(AsyncQuery* query);
        void EndThreads();

        void FreeQueryResult(QueryResult* p);

        DatabaseConnection* GetFreeConnection();

        void PerformQueryBuffer(QueryBuffer* b, DatabaseConnection* ccon);
        void AddQueryBuffer(QueryBuffer* b);

        static Database* CreateDatabaseInterface();
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
        virtual QueryResult* _StoreQueryResult(DatabaseConnection* con) = 0;

        //////////////////////////////////////////////////////////////////////////////////////////
        FQueue<QueryBuffer*> query_buffer;

        //////////////////////////////////////////////////////////////////////////////////////////
        FQueue<char*> queries_queue;
        DatabaseConnection** Connections;

        uint32 _counter;
        //////////////////////////////////////////////////////////////////////////////////////////

        int32 mConnectionCount;

        // For reconnecting a broken connection
        std::string mHostname;
        std::string mUsername;
        std::string mPassword;
        std::string mDatabaseName;
        uint32 mPort;

        QueryThread* qt;
};

class SERVER_DECL QueryResult
{
    public:

        QueryResult(uint32 fields, uint32 rows) : mFieldCount(fields), mRowCount(rows), mCurrentRow(NULL) {}
        virtual ~QueryResult() {}

        virtual bool NextRow() = 0;
        void Delete() { delete this; }

        inline Field* Fetch() { return mCurrentRow; }
        inline uint32 GetFieldCount() const { return mFieldCount; }
        inline uint32 GetRowCount() const { return mRowCount; }

    protected:

        uint32 mFieldCount;
        uint32 mRowCount;
        Field* mCurrentRow;
};

#endif      //_DATABASE_H
