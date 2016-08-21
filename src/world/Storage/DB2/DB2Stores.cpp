/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "StdAfx.h"

#include "DB2Stores.hpp"
#include "Log.h"

#include "DB2Structures.hpp"
#include "../DBC/DBCGlobals.hpp"

#include <map>

SERVER_DECL DB2Storage <DB2::Structures::ItemEntry>                    sItemStore(DB2::Structures::item_entry_format);
SERVER_DECL DB2Storage <DB2::Structures::ItemCurrencyCostEntry>        sItemCurrencyCostStore(DB2::Structures::item_currency_cost_format);
SERVER_DECL DB2Storage <DB2::Structures::ItemExtendedCostEntry>        sItemExtendedCostStore(DB2::Structures::item_extended_cost_format);

typedef std::list<std::string> StoreProblemList1;
uint32 DB2_Count = 0;

static bool LoadDB2_assert_print(uint32 fsize, uint32 rsize, const std::string& filename)
{
    Log.Error("LoadDB2_assert_print", "Size of '%s' setted by format string (%u) not equal size of C++ structure (%u).", filename.c_str(), fsize, rsize);

    // ASSERT must fail after function call
    return false;
}

struct LocalDB2Data
{
    LocalDB2Data(DBC::LocaleConstant loc)
        : defaultLocale(loc), availableDb2Locales(0xFFFFFFFF) {}

    DBC::LocaleConstant defaultLocale;

    // bitmasks for index of fullLocaleNameList
    uint32 availableDb2Locales;
};

template<class T>
inline void LoadDB2(LocalDB2Data& localeData, StoreProblemList1& errors, DB2Storage<T>& storage, std::string const& db2Path, std::string const& filename)
{
    // compatibility format and C++ structure sizes
    ARCEMU_ASSERT(DB2::DB2FileLoader::GetFormatRecordSize(storage.GetFormat()) == sizeof(T) || LoadDB2_assert_print(DB2::DB2FileLoader::GetFormatRecordSize(storage.GetFormat()), sizeof(T), filename));

    ++DB2_Count;
    std::string db2Filename = db2Path + filename;
    if (storage.Load(db2Filename.c_str(), localeData.defaultLocale))
    {
        for (uint8 i = 0; DBC::fullLocaleNameList[i].name; ++i)
        {
            if (!(localeData.availableDb2Locales & (1 << i)))
                continue;

            DBC::LocaleNameStr const* localStr = &DBC::fullLocaleNameList[i];

            std::string db2_dir_loc = db2Path + localStr->name + "/";

            std::string localizedName = db2Path + localStr->name + "/" + filename;
            if (!storage.LoadStringsFrom(localizedName.c_str(), localStr->locale))
                localeData.availableDb2Locales &= ~(1 << i);  // mark as not available for speedup next checks
        }
    }
    else
    {
        // sort problematic db2 to (1) non compatible and (2) nonexistent
        if (FILE* f = fopen(db2Filename.c_str(), "rb"))
        {
            char buf[100];
            snprintf(buf, 100, " (exist, but have %d fields instead %s) Wrong client version DB2 file?", storage.GetFieldCount(), strlen(storage.GetFormat()));
            errors.push_back(db2Filename + buf);
            fclose(f);
        }
        else
            errors.push_back(db2Filename);
    }
}

void LoadDB2Stores()
{
    std::string db2Path = "./DBC/";

    DBC::LocaleNameStr const* defaultLocaleNameStr = NULL;
    StoreProblemList1 bad_db2_files;

    LocalDB2Data availableDb2Locales(DBC::LocaleConstant(0));//defaultLocaleNameStr->locale));

    LoadDB2(availableDb2Locales, bad_db2_files, sItemStore, db2Path, "Item.db2");
    LoadDB2(availableDb2Locales, bad_db2_files, sItemCurrencyCostStore, db2Path, "ItemCurrencyCost.db2");
    LoadDB2(availableDb2Locales, bad_db2_files, sItemExtendedCostStore, db2Path, "ItemExtendedCost.db2");

    // error checks
    if (bad_db2_files.size() >= DB2_Count)
    {
        Log.Error("LoadDB2Stores", "Incorrect DataDir value in worldserver.conf or ALL required *.db2 files (%d) not found", DB2_Count);
        exit(1);
    }
    else if (!bad_db2_files.empty())
    {
        std::string str;
        for (StoreProblemList1::iterator i = bad_db2_files.begin(); i != bad_db2_files.end(); ++i)
            str += *i + "\n";

        Log.Error("LoadDB2Stores", "Some required *.db2 files (%u from %d) not found or not compatible:%s", (uint32)bad_db2_files.size(), DB2_Count, str.c_str());
        exit(1);
    }

    // Check loaded DBC files proper version
    if (!sItemStore.LookupEntry(83086) || !sItemExtendedCostStore.LookupEntry(3872))
    {
        Log.Error("LoadDB2Stores", "Please extract correct db2 files from build 15595");
        exit(1);
    }

    Log.Notice("LoadDB2Stores", "Initialized %u db2 stores", DB2_Count);
}
