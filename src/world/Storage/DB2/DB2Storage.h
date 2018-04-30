/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "DB2Loader.h"
#include "../DBC/DBCGlobals.hpp"

template<class T>
class DB2Storage
{
        typedef std::list<char*> StringPoolList;

    public:

        explicit DB2Storage(const char *f) : nCount(0), fieldCount(0), fmt(f), indexTable(NULL), m_dataTable(NULL) { }
        ~DB2Storage() { Clear(); }

        T const* LookupEntry(uint32 id) const { return (id >= nCount) ? NULL : indexTable[id]; }
        uint32  GetNumRows() const { return nCount; }
        char const* GetFormat() const { return fmt; }
        uint32 GetFieldCount() const { return fieldCount; }

        bool Load(char const* fn, DBC::LocaleConstant loc)
        {
            DB2::DB2FileLoader db2;
            if (!db2.Load(fn, fmt))
                return false;

            fieldCount = db2.GetCols();

            m_dataTable = (T*)db2.AutoProduceData(fmt, nCount, (char**&)indexTable);

            m_stringPoolList.push_back(db2.AutoProduceStringsArrayHolders(fmt, (char*)m_dataTable));

            m_stringPoolList.push_back(db2.AutoProduceStrings(fmt, (char*)m_dataTable, loc));

            return indexTable != NULL;
        }

        bool LoadStringsFrom(char const* fn, DBC::LocaleConstant loc)
        {
            // DBC must be already loaded using Load
            if (!indexTable)
                return false;

            DB2::DB2FileLoader db2;
            // Check if load was successful, only then continue
            if (!db2.Load(fn, fmt))
                return false;

            // load strings from another locale dbc data
            m_stringPoolList.push_back(db2.AutoProduceStrings(fmt, (char*)m_dataTable, loc));

            return true;
        }

        void Clear()
        {
            if (!indexTable)
                return;

            delete[]((char*)indexTable);
            indexTable = NULL;
            delete[]((char*)m_dataTable);
            m_dataTable = NULL;

            while (!m_stringPoolList.empty())
            {
                delete[] m_stringPoolList.front();
                m_stringPoolList.pop_front();
            }
            nCount = 0;
        }

        void EraseEntry(uint32_t id) { indexTable[id] = NULL; }

    private:

        uint32 nCount;
        uint32 fieldCount;
        char const* fmt;
        T** indexTable;
        T* m_dataTable;
        StringPoolList m_stringPoolList;
};
