/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/WorldCreatorDefines.hpp"


void WorldSession::HandleCorpseQueryOpcode(WorldPacket& /*recvData*/)
{
    Corpse* pCorpse = objmgr.GetCorpseByOwner(GetPlayer()->GetLowGUID());
    if (pCorpse == nullptr)
    {
        WorldPacket data(MSG_CORPSE_QUERY, 1);
        data << uint8_t(0);                         // no coprse for player
        SendPacket(&data);
    }
    else
    {
        WorldPacket data(MSG_CORPSE_QUERY, 25);
        data << uint8_t(1);                         // corpse found

        uint32_t corpsemap;
        uint32_t repopmap;
        float x, y, z;

        MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(pCorpse->GetMapId());
        if (pMapinfo)
        {
            if (pMapinfo->type == INSTANCE_NULL || pMapinfo->type == INSTANCE_BATTLEGROUND)
            {
                repopmap = pCorpse->GetMapId();     // mapid (that tombstones shown on)
                x = pCorpse->GetPositionX();
                y = pCorpse->GetPositionY();
                z = pCorpse->GetPositionZ();
                corpsemap = pCorpse->GetMapId();    // instance mapid (needs to be same as mapid to be able to recover corpse)
            }
            else
            {
                repopmap = pMapinfo->repopmapid;    // mapid (that tombstones shown on)
                x = pMapinfo->repopx;
                y = pMapinfo->repopy;
                z = pMapinfo->repopz;
                corpsemap = pCorpse->GetMapId();    // instance mapid (needs to be same as mapid to be able to recover corpse)
            }
        }
        else
        {
            repopmap = pCorpse->GetMapId();         // mapid (that tombstones shown on)
            x = pCorpse->GetPositionX();
            y = pCorpse->GetPositionY();
            z = pCorpse->GetPositionZ();
            corpsemap = pCorpse->GetMapId();        // instance mapid (needs to be same as mapid to be able to recover corpse)
        }

        data << uint32_t(repopmap);
        data << float(x);
        data << float(y);
        data << float(z);
        data << uint32_t(corpsemap);
        data << uint32_t(0);                        // unk
        SendPacket(&data);
    }
}

void WorldSession::HandleInrangeQuestgiverQuery(WorldPacket& /*recvData*/)
{
    uint32_t count = 0;

    WorldPacket data(SMSG_QUESTGIVER_STATUS_MULTIPLE, 4 + 8 + 4);
    data << uint32_t(count);

    Object::InRangeSet::iterator itr;
    for (itr = _player->m_objectsInRange.begin(); itr != _player->m_objectsInRange.end(); ++itr)
    {
        if (!(*itr)->IsCreature())
            continue;

        Creature* pCreature = static_cast<Creature*>(*itr);

        if (pCreature->isQuestGiver())
        {
            data << uint64_t(pCreature->GetGUID());
            data << uint32_t(sQuestMgr.CalcStatus(pCreature, _player));
            ++count;
        }
    }

    data.put<uint32_t>(0, count);
    SendPacket(&data);
}

void WorldSession::HandleCreatureQueryOpcode(WorldPacket& recvData)
{
    uint32_t entry;
    uint64_t guid;

    recvData >> entry;
    recvData >> guid;

    WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 250);
    CreatureProperties const* ci = sMySQLStore.getCreatureProperties(entry);
    if (ci != nullptr)
    {
        MySQLStructure::LocalesCreature const* lcn = (language > 0) ? sMySQLStore.getLocalizedCreature(entry, language) : nullptr;
        data << uint32_t(entry);
        data << (lcn ? lcn->name : ci->Name);

        for (int i = 0; i < 7; ++i)
        {
            data << uint8_t(0);       // unk
        }

        data << (lcn ? lcn->subName : ci->SubName);
        data << ci->info_str;
        data << uint32_t(ci->typeFlags);
        data << uint32_t(0);                  // unk set 4 times with 1
        data << uint32_t(ci->Type);
        data << uint32_t(ci->Family);
        data << uint32_t(ci->Rank);
        data << uint32_t(ci->killcredit[0]);
        data << uint32_t(ci->killcredit[1]);
        data << uint32_t(ci->Male_DisplayID);
        data << uint32_t(ci->Female_DisplayID);
        data << uint32_t(ci->Male_DisplayID2);
        data << uint32_t(ci->Female_DisplayID2);
        data << float(ci->baseAttackMod);
        data << float(ci->rangeAttackMod);
        data << uint8_t(ci->Leader);

        for (uint8_t i = 0; i < 6; ++i)
        {
            data << uint32_t(ci->QuestItems[i]);
        }

        data << uint32_t(ci->waypointid);
        data << uint32_t(0);                  // unk
    }
    else
    {
        WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 4);
        data << uint32_t(entry | 0x80000000);
    }

    SendPacket(&data);
}
