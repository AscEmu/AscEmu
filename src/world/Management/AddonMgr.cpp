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

#if VERSION_STRING < Cata
void AddonMgr::finalize()
{
    mKnownAddons.clear();
}

bool AddonMgr::IsAddonBanned(uint64_t /*crc*/, const std::string& /*name*/)
{
    return false; // bleh needs work
}

bool AddonMgr::IsAddonBanned(const std::string& name, uint64_t crc)
{
    KnownAddonsItr itr = mKnownAddons.find(name);
    if (itr != mKnownAddons.end())
    {
        if (itr->second->banned)
        {
            sLogger.debug("Addon {} is banned.", name);
            return true;
        }
    }
    else
    {
        // New addon. It'll be saved to db at server shutdown.
        auto ent = std::make_unique<AddonEntry>();
        ent->name = name;
        ent->crc = crc;
        ent->banned = false; // by default.. we can change this I guess..
        ent->isNew = true;
        ent->showinlist = true;

        sLogger.debug("Discovered new addon {} sent by client.", name);

        mKnownAddons.try_emplace(ent->name, std::move(ent));
    }

    return false;
}

bool AddonMgr::ShouldShowInList(const std::string& name)
{
    KnownAddonsItr itr = mKnownAddons.find(name);

    if (itr != mKnownAddons.end())
        return itr->second->showinlist;

    auto ent = std::make_unique<AddonEntry>();
    ent->name = name;
    ent->crc = 0;
    ent->banned = false;
    ent->isNew = true;
    ent->showinlist = true;

    sLogger.debug("Discovered new addon {} sent by client.", name);
    mKnownAddons.try_emplace(ent->name, std::move(ent));

    return true;
}

void AddonMgr::SendAddonInfoPacket(WorldPacket* source, uint32_t /*pos*/, WorldSession* m_session)
{
    WorldPacket returnpacket;
    returnpacket.Initialize(SMSG_ADDON_INFO); // SMSG_ADDON_INFO

    uint32_t realsize;
    uLongf rsize;

    try
    {
        *source >> realsize;
    }
    catch (ByteBuffer::error&)
    {
        sLogger.debug("Warning: Incomplete auth session sent.");
        return;
    }

    rsize = realsize;
    size_t position = source->rpos();

    ByteBuffer unpacked;
    unpacked.resize(realsize);

    if ((source->size() - position) < 4 || realsize == 0)
    {
        // we shouldn't get here.. but just in case this will stop any crash here.
        sLogger.debug("Warning: Incomplete auth session sent.");
        return;
    }

    int32_t result = uncompress(unpacked.contents(), &rsize, source->contents() + position, static_cast<uLong>(source->size() - position));

    if (result != Z_OK)
    {
        sLogger.failure("Decompression of addon section of CMSG_AUTH_SESSION failed.");
        return;
    }

    sLogger.info("Decompression of addon section of CMSG_AUTH_SESSION succeeded.");

    uint8_t enabled; // based on the parsed files from retool
    uint32_t crc;
    uint32_t unknown;

    std::string name;

    uint32_t addoncount;
    unpacked >> addoncount;

    for (uint32_t i = 0; i < addoncount; ++i)
    {
        if (unpacked.rpos() >= unpacked.size())
            break;

        unpacked >> name;
        unpacked >> enabled;
        unpacked >> crc;
        unpacked >> unknown;

#if VERSION_STRING == WotLK
        uint8_t unk;
        uint8_t unk1;
        uint8_t unk2;

        unk = (enabled ? 2 : 1);
        returnpacket << unk;

        unk1 = (enabled ? 1 : 0);
        returnpacket << unk1;

        if (unk1)
        {
            if (crc != STANDARD_ADDON_CRC)
            {
                returnpacket << uint8_t(1);
                returnpacket.append(PublicKey, 264);
            }
            else
            {
                returnpacket << uint8_t(0);
            }

            returnpacket << uint32_t(0);
        }

        unk2 = (enabled ? 0 : 1);
        returnpacket << unk2;

        if (unk2)
            returnpacket << uint8_t(0);
#else
        if (crc != STANDARD_ADDON_CRC)
        {
            returnpacket.append(PublicKey, 264);
        }
        else
        {
            returnpacket << uint8_t(2) << uint8_t(1) << uint8_t(0) << uint32_t(0) << uint8_t(0);
        }
#endif
    }

    // unknown 4 bytes at the end of the packet. Stays 0 for me. Tried custom addons, deleting, faulty etc. It stays 0.
#ifndef AE_TBC
    returnpacket << uint32_t(0); // some additional count for additional records, but we won't send them.
#endif

    m_session->SendPacket(&returnpacket);
}

bool AddonMgr::AppendPublicKey(WorldPacket& data, const std::string& addonName, uint32_t crc)
{
    static constexpr uint32_t addonPublicKeySize = 264;

    if (crc != STANDARD_ADDON_CRC)
        return false;

    const auto itr = mAddonData.find(addonName);
    if (itr != mAddonData.end())
    {
        data.append(itr->second);
        return true;
    }

    const std::filesystem::path path = std::filesystem::path("addons") / fmt::format("{}.pub", addonName);
    std::ifstream file(path, std::ios::binary);

    ByteBuffer buffer;

    if (file)
    {
        buffer.resize(addonPublicKeySize);
        file.read(reinterpret_cast<char*>(buffer.contents()), addonPublicKeySize);

        if (!file)
            return false;
    }
    else
    {
        buffer.append(PublicKey, addonPublicKeySize);
    }

    mAddonData[addonName] = buffer;
    data.append(buffer);

    return true;
}

void AddonMgr::LoadFromDB()
{
    const char* loadClientAddons = "SELECT id, name, crc, banned, showinlist FROM clientaddons";
    bool success = false;
    auto result = CharacterDatabase.Query(&success, loadClientAddons);
    if (!success)
    {
        sLogger.failure("Query failed: {}", loadClientAddons);
        return;
    }
    if (!result)
    {
        sLogger.info("AddonMgr : No defined ClientAddons");
        return;
    }

    Field* field;

    do
    {
        field = result->Fetch();
        auto ent = std::make_unique<AddonEntry>();

        ent->name = field[1].asCString();
        ent->crc = field[2].asUint64();
        ent->banned = field[3].asUint32() > 0;
        ent->isNew = false;

        // To avoid crashes for stilly nubs who don't update table :P
        if (result->GetFieldCount() == 5)
            ent->showinlist = field[4].asUint32() > 0;

        mKnownAddons.try_emplace(ent->name, std::move(ent));

    }
    while(result->NextRow());
}

void AddonMgr::SaveToDB()
{
    sLogger.info("AddonMgr: Saving any new addons discovered in this session to database.");

    for (const auto& [name, addon] : mKnownAddons)
    {
        if (!addon->isNew)
            continue;

        sLogger.info("Saving new addon {}", addon->name);

        CharacterDatabase.Execute("REPLACE INTO clientaddons "
                                  "(name, crc, banned, showinlist) "
                                  "VALUES('%s', %u, %u, %u)", 
                                  CharacterDatabase.EscapeString(addon->name).c_str(),
                                  addon->crc,
                                  addon->banned,
                                  addon->showinlist);
    }
}
#endif

#if VERSION_STRING >= Cata

void AddonMgr::LoadFromDB()
{
    auto startTime = Util::TimeNow();

    auto clientAddonsResult = CharacterDatabase.Query("SELECT name, crc FROM clientaddons");
    if (clientAddonsResult)
    {
        uint32_t knownAddonsCount = 0;

        do
        {
            Field* fields = clientAddonsResult->Fetch();

            std::string name = fields[0].asCString();
            uint32_t crc = fields[1].asUint32();

            mKnownAddons.emplace_back(SavedAddon(name, crc));

            ++knownAddonsCount;
        } while (clientAddonsResult->NextRow());

        sLogger.debug("Loaded {} known addons from table `clientaddons` in {} ms", knownAddonsCount, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)) );
    }
    else
    {
        sLogger.debug("Loaded 0 known addons, table `clientaddons` is empty");
    }

    startTime = Util::TimeNow();
    clientAddonsResult = CharacterDatabase.Query("SELECT id, name, banned, UNIX_TIMESTAMP(timestamp), version FROM clientaddons WHERE banned = 1");
    if (clientAddonsResult)
    {
        uint32_t bannedAddonsCount = 0;
        uint32_t dbcMaxBannedAddon = sBannedAddOnsStore.getNumRows();

        do
        {
            Field* fields = clientAddonsResult->Fetch();

            BannedAddon addon;
            addon.id = fields[0].asUint32() + dbcMaxBannedAddon;
            addon.timestamp = uint32_t(fields[2].asUint64());

            std::string name = fields[1].asCString();
            std::string version = fields[3].asCString();

            MD5(reinterpret_cast<uint8_t const*>(name.c_str()), name.length(), addon.nameMD5);//
            MD5(reinterpret_cast<uint8_t const*>(version.c_str()), version.length(), addon.versionMD5);//

            mBannedAddons.push_back(addon);

            ++bannedAddonsCount;
        } while (clientAddonsResult->NextRow());

        sLogger.debug("Loaded {} banned addons from table `clientaddons` in {} ms", bannedAddonsCount, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    }
}

void AddonMgr::SaveAddon(AddonEntry const& addon)
{
    CharacterDatabase.Execute("REPLACE INTO clientaddons(name, crc) VALUES('%s', %u )", addon.name.c_str(), addon.crc);

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

#endif
