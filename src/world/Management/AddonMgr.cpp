/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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

initialiseSingleton(AddonMgr);

//#define DEBUG_PRINT_ADDON_PACKET            // Prints out Received addon packet when char logging in

// hacky key
unsigned char PublicKey[256] =
{
    0xC3, 0x5B, 0x50, 0x84, 0xB9, 0x3E, 0x32, 0x42, 0x8C, 0xD0, 0xC7, 0x48, 0xFA, 0x0E, 0x5D, 0x54,
    0x5A, 0xA3, 0x0E, 0x14, 0xBA, 0x9E, 0x0D, 0xB9, 0x5D, 0x8B, 0xEE, 0xB6, 0x84, 0x93, 0x45, 0x75,
    0xFF, 0x31, 0xFE, 0x2F, 0x64, 0x3F, 0x3D, 0x6D, 0x07, 0xD9, 0x44, 0x9B, 0x40, 0x85, 0x59, 0x34,
    0x4E, 0x10, 0xE1, 0xE7, 0x43, 0x69, 0xEF, 0x7C, 0x16, 0xFC, 0xB4, 0xED, 0x1B, 0x95, 0x28, 0xA8,
    0x23, 0x76, 0x51, 0x31, 0x57, 0x30, 0x2B, 0x79, 0x08, 0x50, 0x10, 0x1C, 0x4A, 0x1A, 0x2C, 0xC8,
    0x8B, 0x8F, 0x05, 0x2D, 0x22, 0x3D, 0xDB, 0x5A, 0x24, 0x7A, 0x0F, 0x13, 0x50, 0x37, 0x8F, 0x5A,
    0xCC, 0x9E, 0x04, 0x44, 0x0E, 0x87, 0x01, 0xD4, 0xA3, 0x15, 0x94, 0x16, 0x34, 0xC6, 0xC2, 0xC3,
    0xFB, 0x49, 0xFE, 0xE1, 0xF9, 0xDA, 0x8C, 0x50, 0x3C, 0xBE, 0x2C, 0xBB, 0x57, 0xED, 0x46, 0xB9,
    0xAD, 0x8B, 0xC6, 0xDF, 0x0E, 0xD6, 0x0F, 0xBE, 0x80, 0xB3, 0x8B, 0x1E, 0x77, 0xCF, 0xAD, 0x22,
    0xCF, 0xB7, 0x4B, 0xCF, 0xFB, 0xF0, 0x6B, 0x11, 0x45, 0x2D, 0x7A, 0x81, 0x18, 0xF2, 0x92, 0x7E,
    0x98, 0x56, 0x5D, 0x5E, 0x69, 0x72, 0x0A, 0x0D, 0x03, 0x0A, 0x85, 0xA2, 0x85, 0x9C, 0xCB, 0xFB,
    0x56, 0x6E, 0x8F, 0x44, 0xBB, 0x8F, 0x02, 0x22, 0x68, 0x63, 0x97, 0xBC, 0x85, 0xBA, 0xA8, 0xF7,
    0xB5, 0x40, 0x68, 0x3C, 0x77, 0x86, 0x6F, 0x4B, 0xD7, 0x88, 0xCA, 0x8A, 0xD7, 0xCE, 0x36, 0xF0,
    0x45, 0x6E, 0xD5, 0x64, 0x79, 0x0F, 0x17, 0xFC, 0x64, 0xDD, 0x10, 0x6F, 0xF3, 0xF5, 0xE0, 0xA6,
    0xC3, 0xFB, 0x1B, 0x8C, 0x29, 0xEF, 0x8E, 0xE5, 0x34, 0xCB, 0xD1, 0x2A, 0xCE, 0x79, 0xC3, 0x9A,
    0x0D, 0x36, 0xEA, 0x01, 0xE0, 0xAA, 0x91, 0x20, 0x54, 0xF0, 0x72, 0xD8, 0x1E, 0xC7, 0x89, 0xD2
};

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

    return false;
}

bool AddonMgr::ShouldShowInList(std::string name)
{
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

    return true;
}

void AddonMgr::SendAddonInfoPacket(WorldPacket* source, uint32 pos, WorldSession* m_session)
{
    WorldPacket returnpacket;
    returnpacket.Initialize(SMSG_ADDON_INFO);

    source->ResetRead();
    source->read_skip(52);

    uint32 addonSize;
    *source >> addonSize;

    uint32 realsize;
    uLongf rsize;

    *source >> realsize;

    rsize = realsize;
    size_t position = source->rpos();

    ByteBuffer unpacked;
    unpacked.resize(rsize);

    if (source->rpos() + 4 >= source->wpos())
    {
        // we shouldnt get here.. but just in case this will stop any crash here.
        sLog.outDebug("Warning: Incomplete auth session sent.");
        return;
    }

    int32 result;
    result = uncompress((uint8*)unpacked.contents(), &rsize, (uint8*)(*source).contents() + position, (uLong)((*source).size() - position));

    if (result != Z_OK)
    {
        sLog.outError("Decompression of addon section of CMSG_AUTH_SESSION failed.");
        return;
    }

    sLog.outDetail("Decompression of addon section of CMSG_AUTH_SESSION succeeded.");

    uint32 addoncount;
    unpacked >> addoncount;

    if (addoncount > 100)
        return; //have a hunch they are fucking with us

    uint8 Enable; // based on the parsed files from retool
    uint32 crc;
    uint32 unknown;
    bool have_new_addons = false;

    std::string name;
    for (uint32 i = 0; i<addoncount; ++i)
    {
        unpacked >> name;
        unpacked >> Enable;
        unpacked >> crc;
        unpacked >> unknown;

        // Hacky fix, Yea I know its a hacky fix I will make a proper handler one's I got the crc crap
        if (crc != 0x4C1C776D) // CRC of public key version 2.0.1
        {
            returnpacket.append(PublicKey, 264); // part of the hacky fix
            have_new_addons = true;
        }
        else
        {
            returnpacket << uint8(0x02);
            returnpacket << uint8(0x01);
            returnpacket << uint8(0x00);
            returnpacket << uint32(0);
            returnpacket << uint8(0);
        }

        if (unpacked.rpos() >= unpacked.wpos())
            break;
    }

    if (have_new_addons == false)
    {
        returnpacket << uint32(0);	//this is a counter !
    }

    m_session->SendPacket(&returnpacket);
}

bool AddonMgr::AppendPublicKey(WorldPacket & data, std::string & AddonName, uint32 CRC)
{
    if (CRC == 0x4C1C776D)
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
    return false;
}

void AddonMgr::LoadFromDB()
{
    const char* loadClientAddons = "SELECT id, name, crc, banned, showinlist FROM clientaddons";
    bool success = false;
    QueryResult* result = CharacterDatabase.Query(&success, loadClientAddons);
    if (!success)
    {
        Log.Error("AddonMgr", "Query failed: %s", loadClientAddons);
        return;
    }
    if (!result)
    {
        Log.Notice("AddonMgr", "No defined ClientAddons");
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
}

void AddonMgr::SaveToDB()
{
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
}
