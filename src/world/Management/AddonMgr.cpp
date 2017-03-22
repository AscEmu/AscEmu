/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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

#include "StdAfx.h"
#include "AddonMgr.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/MainServerDefines.h"

initialiseSingleton(AddonMgr);

//#define DEBUG_PRINT_ADDON_PACKET            // Prints out Received addon packet when char logging in

AddonMgr::AddonMgr()
{
    mKnownAddons.clear();
}

AddonMgr::~AddonMgr()
{
    KnownAddonsItr itr;
    for (itr = mKnownAddons.begin(); itr != mKnownAddons.end(); ++itr)
    {
        delete itr->second;
    }
    mKnownAddons.clear();
}

bool AddonMgr::IsAddonBanned(uint64 crc, std::string name)
{
    return false;    // bleh needs work
}

bool AddonMgr::IsAddonBanned(std::string name, uint64 crc)
{
#if VERSION_STRING != Cata
    KnownAddonsItr itr = mKnownAddons.find(name);
    if (itr != mKnownAddons.end())
    {
        if (itr->second->banned)
        {
            LOG_DEBUG("Addon %s is banned.", name.c_str());
            return true;
        }
    }
    else
    {
        // New addon. It'll be saved to db at server shutdown.
        AddonEntry* ent = new AddonEntry;
        ent->name = name;
        ent->crc = crc;
        ent->banned = false;    // by default.. we can change this I guess..
        ent->isNew = true;
        ent->showinlist = true;

        LOG_DEBUG("Discovered new addon %s sent by client.", name.c_str());

        mKnownAddons[ent->name] = ent;
    }
#endif
    return false;
}

bool AddonMgr::ShouldShowInList(std::string name)
{
#if VERSION_STRING != Cata
    KnownAddonsItr itr = mKnownAddons.find(name);

    if (itr != mKnownAddons.end())
    {
        if (itr->second->showinlist)
            return true;
        else
            return false;
    }
    else
    {
        // New addon. It'll be saved to db at server shutdown.
        AddonEntry* ent = new AddonEntry;
        ent->name = name;
        ent->crc = 0;
        ent->banned = false;    // by default.. we can change this I guess..
        ent->isNew = true;
        ent->showinlist = true;

        LOG_DEBUG("Discovered new addon %s sent by client.", name.c_str());

        mKnownAddons[ent->name] = ent;
    }
#endif
    return true;
}

void AddonMgr::SendAddonInfoPacket(WorldPacket* source, uint32 pos, WorldSession* m_session)
{
#if VERSION_STRING != Cata
    WorldPacket returnpacket;
    returnpacket.Initialize(SMSG_ADDON_INFO);    // SMSG_ADDON_INFO

    uint32 realsize;
    uLongf rsize;

    try
    {
        *source >> realsize;
    }
    catch(ByteBuffer::error &)
    {
        LOG_DEBUG("Warning: Incomplete auth session sent.");
        return;
    }

    rsize = realsize;
    size_t position = source->rpos();

    ByteBuffer unpacked;
    unpacked.resize(realsize);

    if ((source->size() - position) < 4 || realsize == 0)
    {
        // we shouldn't get here.. but just in case this will stop any crash here.
        LOG_DEBUG("Warning: Incomplete auth session sent.");
        return;
    }

    int32 result = uncompress((uint8*)unpacked.contents(), &rsize, (uint8*)(*source).contents() + position, (uLong)((*source).size() - position));

    if (result != Z_OK)
    {
        LOG_ERROR("Decompression of addon section of CMSG_AUTH_SESSION failed.");
        return;
    }

    LOG_DETAIL("Decompression of addon section of CMSG_AUTH_SESSION succeeded.");

    uint8 Enable; // based on the parsed files from retool
    uint32 crc;
    uint32 unknown;

    std::string name;

    uint32 addoncount;
    unpacked >> addoncount;

    uint8 unk;
    uint8 unk1;
    uint8 unk2;
    for (uint32 i = 0; i < addoncount; ++i)
    {
        if (unpacked.rpos() >= unpacked.size())
            break;

        unpacked >> name;
        unpacked >> Enable;
        unpacked >> crc;
        unpacked >> unknown;

        unk = (Enable ? 2 : 1);
        returnpacket << unk;
        unk1 = (Enable ? 1 : 0);
        returnpacket << unk1;
        if (unk1)
        {
            if (crc != 0x4C1C776D)
            {
                returnpacket << uint8(1);
                returnpacket.append(PublicKey, 264);
            }
            else
                returnpacket << uint8(0);

            returnpacket << uint32(0);
        }

        unk2 = (Enable ? 0 : 1);
        returnpacket << unk2;
        if (unk2)
            returnpacket << uint8(0);
    }

    //unknown 4 bytes at the end of the packet. Stays 0 for me. Tried custom addons, deleting, faulty etc. It stays 0.
    returnpacket << uint32(0);  //Some additional count for additional records, but we won't send them.

    m_session->SendPacket(&returnpacket);
#endif
}

bool AddonMgr::AppendPublicKey(WorldPacket & data, std::string & AddonName, uint32 CRC)
{
#if VERSION_STRING != Cata
    if (CRC == STANDARD_ADDON_CRC)
    {
        // Open public key file with that addon
        AddonDataItr itr = mAddonData.find(AddonName);

        if (itr != mAddonData.end())
            data.append(itr->second);
        else
        {
            // open the file
            char path[500];
            snprintf(path, 500, "addons\\%s.pub", AddonName.c_str());
            FILE* f = fopen(path, "rb");
            if (f != 0)
            {
                // read the file into a bytebuffer
                ByteBuffer buf;
                fseek(f, 0, SEEK_END);
                uint32 length = 264/*ftell(f)*/;
                fseek(f, 0, SEEK_SET);
                buf.resize(length);
                if (fread((void*)buf.contents(), length, 1, f) != 1)
                {
                    fclose(f);
                    return false;
                }

                fclose(f);

                mAddonData[AddonName] = buf;
                data.append(buf);
            }
            else
            {
                ByteBuffer buf;
                buf.append(PublicKey, 264);
                mAddonData[AddonName] = buf;
                data.append(buf);
            }
        }
        return true;
    }
#endif
    return false;
}

void AddonMgr::LoadFromDB()
{
#if VERSION_STRING != Cata
    const char* loadClientAddons = "SELECT id, name, crc, banned, showinlist FROM clientaddons";
    bool success = false;
    QueryResult* result = CharacterDatabase.Query(&success, loadClientAddons);
    if (!success)
    {
        LOG_ERROR("Query failed: %s", loadClientAddons);
        return;
    }
    if (!result)
    {
        LogNotice("AddonMgr : No defined ClientAddons");
        return;
    }

    Field* field;
    AddonEntry* ent;

    do
    {
        field = result->Fetch();
        ent = new AddonEntry;

        ent->name = field[1].GetString();
        ent->crc = field[2].GetUInt64();
        ent->banned = (field[3].GetUInt32() > 0 ? true : false);
        ent->isNew = false;

        // To avoid crashes for stilly nubs who don't update table :P
        if (result->GetFieldCount() == 5)
            ent->showinlist = (field[4].GetUInt32() > 0 ? true : false);

        mKnownAddons[ent->name] = ent;

    }
    while(result->NextRow());

    delete result;
#endif
}

void AddonMgr::SaveToDB()
{
#if VERSION_STRING != Cata
    LOG_DETAIL("AddonMgr: Saving any new addons discovered in this session to database.");

    KnownAddonsItr itr;

    for (itr = mKnownAddons.begin(); itr != mKnownAddons.end(); ++itr)
    {
        if (itr->second->isNew)
        {
            LOG_DETAIL("Saving new addon %s", itr->second->name.c_str());
            std::stringstream ss;
            ss << "INSERT INTO clientaddons (name, crc, banned, showinlist) VALUES(\""
               << CharacterDatabase.EscapeString(itr->second->name) << "\",\""
               << itr->second->crc << "\",\""
               << itr->second->banned << "\",\""
               << itr->second->showinlist << "\");";

            CharacterDatabase.Execute(ss.str().c_str());
        }
    }
#endif
}
