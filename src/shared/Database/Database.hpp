/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Field.hpp"
#include "CommonTypes.hpp"
#include <string>
#include <mutex>
#include <vector>

class QueryResult;
class Database;
class SQLCallbackBase;
class DatabaseRuntime;

struct DatabaseConnection
{
    virtual ~DatabaseConnection() = default;
    std::recursive_mutex Busy;
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
    friend class AsyncQuery;
	
	friend class DatabaseRuntime;

    public:

        Database();
        virtual ~Database();

        [[nodiscard]] size_t GetAeQueuedTaskCount() const;
        [[nodiscard]] size_t GetAeWorkerCount() const;
        [[nodiscard]] uint64_t GetAeCompletedTaskCount() const;

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

        const std::string & GetHostName() { return mHostname; }
        const std::string & GetDatabaseName() { return mDatabaseName; }

        virtual std::string EscapeString(std::string Escape) = 0;
        virtual void EscapeLongString(const char* str, uint32_t len, std::stringstream & out) = 0;
        virtual std::string EscapeString(const char* esc, DatabaseConnection* con) = 0;

        void QueueAsyncQuery(std::unique_ptr<AsyncQuery> query);
        void EndThreads();

        void AddQueryBuffer(std::unique_ptr<QueryBuffer> b);

        static std::unique_ptr<Database> CreateDatabaseInterface();
        static void CleanupLibs();

        virtual bool SupportsReplaceInto() = 0;
        virtual bool SupportsTableLocking() = 0;

    protected:
        std::unique_ptr<DatabaseRuntime> m_runtime;

        // spawn threads and shizzle
        void _Initialize();

        virtual void _BeginTransaction(DatabaseConnection* conn) = 0;
        virtual void _EndTransaction(DatabaseConnection* conn) = 0;

        // actual query function
        virtual bool _SendQuery(DatabaseConnection* con, const char* Sql, bool Self) = 0;
        virtual std::unique_ptr<QueryResult> _StoreQueryResult(DatabaseConnection* con) = 0;

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
