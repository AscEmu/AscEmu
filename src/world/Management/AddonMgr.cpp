/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AddonMgr.h"

#include <zlib.h>
#include <fstream>
#include <filesystem>
#include <fmt/format.h>

#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Cryptography/MD5.hpp"
#include "Database/Field.hpp"
#include "Database/Database.hpp"
#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/Opcodes.hpp"
#include "Server/WorldSession.h"
#include "Storage/WDB/WDBStores.hpp"

AddonMgr& AddonMgr::getInstance()
{
    static AddonMgr mInstance;
    return mInstance;
}

void AddonMgr::initialize()
{
    mKnownAddons.clear();
}

void AddonMgr::finalize()
{
    mKnownAddons.clear();
}

void AddonMgr::LoadFromDB()
{
    auto startTime = Util::TimeNow();
    //                                                  0    1     2      3              4                    5          6
    auto addonsResult = CharacterDatabase.query("SELECT id, name, crc, banned, UNIX_TIMESTAMP(timestamp), showinlist, version FROM clientaddons");
    if (addonsResult)
    {
        uint32_t knownAddonsCount = 0;
        uint32_t bannedAddonsCount = 0;
        uint32_t dbcMaxBannedAddon = 0;

        do
        {
            Field* fields = addonsResult->fetch();
            bool banned = fields[3].asUint32() > 0;

            // All Known addons
            {
                std::string name = fields[1].asCString();
                uint32_t crc = fields[2].asUint32();

                mKnownAddons.emplace_back(SavedAddon(name, crc));

                ++knownAddonsCount;
            }

            // Banned addons
            if (banned)
            {
#if VERSION_STRING >= Cata
                dbcMaxBannedAddon = sBannedAddOnsStore.getNumRows();
#endif
                BannedAddon addon;
                addon.id = fields[0].asUint32() + dbcMaxBannedAddon;
                addon.timestamp = uint32_t(fields[5].asUint64());

                std::string name = fields[1].asCString();
                std::string version = fields[6].asCString();

                MD5(reinterpret_cast<uint8_t const*>(name.c_str()), name.length(), addon.nameMD5);
                MD5(reinterpret_cast<uint8_t const*>(version.c_str()), version.length(), addon.versionMD5);

                mBannedAddons.push_back(addon);

                ++bannedAddonsCount;
            }
        } while(addonsResult->nextRow());

        sLogger.debug("Loaded {} addons ({} banned) from table `clientaddons` in {} ms", knownAddonsCount, bannedAddonsCount, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    }
    else
    {
        sLogger.debug("Loaded 0 known addons, table `clientaddons` is empty");
    }
}

void AddonMgr::SaveAddon(AddonEntry const& addon)
{
    CharacterDatabase.execute("REPLACE INTO clientaddons(name, crc) VALUES('%s', %u )", addon.name.c_str(), addon.crc);

    mKnownAddons.emplace_back(SavedAddon(addon.name, addon.crc));
}

SavedAddon const* AddonMgr::getAddonInfoForAddonName(const std::string& name)
{
    for (SavedAddonsList::const_iterator it = mKnownAddons.begin(); it != mKnownAddons.end(); ++it)
    {
        SavedAddon const& addon = (*it);
        if (addon.name == name)
            return &addon;
    }

    return nullptr;
}

BannedAddonList const* AddonMgr::getBannedAddonsList()
{
    return &mBannedAddons;
}
