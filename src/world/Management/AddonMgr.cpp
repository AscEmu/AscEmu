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

#if VERSION_STRING >= Cata
void AddonMgr::SaveAddon(AddonEntry const& addon)
{
    CharacterDatabase.execute("REPLACE INTO clientaddons(name, crc) VALUES('%s', %u )", addon.name.c_str(), addon.crc);

    mKnownAddons.emplace_back(SavedAddon(addon.name, addon.crc));
}
#endif

#if VERSION_STRING < Cata
//\Todo: Zyres: This part is a real mess, we should try to handle it the same for all versions
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
#endif

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
