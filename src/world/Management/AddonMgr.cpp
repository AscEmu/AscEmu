/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AddonMgr.h"
#include "Cryptography/MD5.hpp"
#include "Database/Field.hpp"
#include "Database/Database.hpp"
#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Storage/WDB/WDBStores.hpp"

#include <cstdint>
#include <filesystem>
#include <list>
#include <string>

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

        auto const dbcMaxBannedAddon = static_cast<uint32_t>(sBannedAddOnsStore.getNumRows());

        do
        {
            Field* fields = addonsResult->fetch();
            bool banned = fields[3].asUint32() > 0;

            // All Known addons
            std::string name = fields[1].asCString();
            uint32_t crc = fields[2].asUint32();

            // Banned addons
            if (banned)
            {
                BannedAddon addon;
                addon.id = fields[0].asUint32() + dbcMaxBannedAddon;
                addon.timestamp = static_cast<uint32_t>(fields[4].asUint64());

                std::string version = fields[6].asCString();

                MD5(reinterpret_cast<uint8_t const*>(name.c_str()), name.length(), addon.nameMD5);
                MD5(reinterpret_cast<uint8_t const*>(version.c_str()), version.length(), addon.versionMD5);

                mBannedAddons.push_back(addon);

                ++bannedAddonsCount;
            }

            mKnownAddons.emplace_back(std::move(name), crc);
            ++knownAddonsCount;
        } while (addonsResult->nextRow());

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

    mKnownAddons.emplace_back(std::move(addon.name), addon.crc);
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
