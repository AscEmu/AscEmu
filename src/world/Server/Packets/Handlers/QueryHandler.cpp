/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgNameQuery.h"
#include "Server/Packets/CmsgGameobjectQuery.h"
#include "Server/Packets/SmsgNameQueryResponse.h"
#include "Server/Packets/SmsgGameobjectQueryResponse.h"
#include "Server/Packets/SmsgQueryTimeResponse.h"
#include "Log.hpp"
#include "Objects/ObjectMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Packets/CmsgCreatureQuery.h"
#include "Server/Packets/SmsgCreatureQueryResponse.h"
#include "Server/Packets/CmsgInspectAchievements.h"
#include "Server/Packets/SmsgQuestgiverStatusMultiple.h"
#include "Server/Packets/CmsgPageTextQuery.h"
#include "Server/Packets/SmsgPageTextQueryResponse.h"
#include "Server/Packets/CmsgItemNameQuery.h"
#include "Server/Packets/SmsgItemNameQueryResponse.h"
#include "Server/Packets/MsgCorpseQuery.h"

using namespace AscEmu::Packets;

void WorldSession::handleNameQueryOpcode(WorldPacket& recvData)
{
    CmsgNameQuery srlPacket;
    if (!srlPacket.deserialise(recvData))
    {
        Disconnect();
        return;
    }

    const auto info = objmgr.GetPlayerInfo(srlPacket.guid.getGuidLow());
    if (!info)
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_NAME_QUERY for: %s", info->name);
    SendPacket(SmsgNameQueryResponse(srlPacket.guid, info->name, info->race, info->gender, info->cl).serialise().get());
}

void WorldSession::handleGameObjectQueryOpcode(WorldPacket& recvData)
{
    CmsgGameobjectQuery srlPacket;
    if (!srlPacket.deserialise(recvData))
    {
        Disconnect();
        return;
    }

    const auto gameobject_info = sMySQLStore.getGameObjectProperties(srlPacket.entry);
    if (!gameobject_info)
        return;

    const auto loc = (language > 0) ? sMySQLStore.getLocalizedGameobject(srlPacket.entry, language) : nullptr;
    const auto name = loc ? loc->name : gameobject_info->name.c_str();


    LogDebugFlag(LF_OPCODE, "Received CMSG_GAMEOBJECT_QUERY for entry: %u", srlPacket.entry);
    SendPacket(SmsgGameobjectQueryResponse(*gameobject_info, name).serialise().get());
}

void WorldSession::handleCreatureQueryOpcode(WorldPacket& recvData)
{
    CmsgCreatureQuery srlPacket;
    if (!srlPacket.deserialise(recvData))
    {
        Disconnect();
        return;
    }

    const auto creature_info = sMySQLStore.getCreatureProperties(srlPacket.entry);
    if (!creature_info)
        return;

    const auto loc = (language > 0) ? sMySQLStore.getLocalizedCreature(srlPacket.entry, language) : nullptr;
    const auto name = loc ? loc->name : creature_info->Name.c_str();
    const auto subName = loc ? loc->subName : creature_info->SubName.c_str();

    LogDebugFlag(LF_OPCODE, "Received SMSG_CREATURE_QUERY_RESPONSE for entry: %u", srlPacket.entry);
    SendPacket(SmsgCreatureQueryResponse(*creature_info, srlPacket.entry, name, subName).serialise().get());
}

void WorldSession::handleQueryTimeOpcode(WorldPacket& /*recvPacket*/)
{
    SendPacket(SmsgQueryTimeResponse(UNIXTIME).serialise().get());
}

void WorldSession::handleAchievmentQueryOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    CmsgInspectAchievements srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    auto player = objmgr.GetPlayer(srlPacket.guid.getGuidLow());
    if (player == nullptr)
        return;

    player->GetAchievementMgr().SendAllAchievementData(_player);
#endif
}

void WorldSession::handleInrangeQuestgiverQuery(WorldPacket& /*recvPacket*/)
{
    std::vector<QuestgiverInrangeStatus> questgiverSet;
    QuestgiverInrangeStatus temp;

    for (const auto& inrangeObject : _player->getInRangeObjectsSet())
    {
        if (inrangeObject == nullptr || !inrangeObject->isCreature())
            continue;

        const auto creature = dynamic_cast<Creature*>(inrangeObject);
        if (creature->isQuestGiver())
        {
            temp.rawGuid = creature->getGuid();
            temp.status = uint8_t(sQuestMgr.CalcStatus(creature, _player));
            questgiverSet.push_back(temp);
        }
    }

    SendPacket(SmsgQuestgiverStatusMultiple(uint32_t(questgiverSet.size()), questgiverSet).serialise().get());
}

void WorldSession::handlePageTextQueryOpcode(WorldPacket& recvPacket)
{
    CmsgPageTextQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_PAGE_TEXT_QUERY: %u (pageId)", srlPacket.pageId);

    uint32_t pageId = srlPacket.pageId;
    while (pageId)
    {
        const auto itemPage = sMySQLStore.getItemPage(pageId);
        if (itemPage == nullptr)
            return;

        const auto localizedPage = language > 0 ? sMySQLStore.getLocalizedItemPages(pageId, language) : nullptr;
        const auto pageText = localizedPage ? localizedPage->text : itemPage->text.c_str();

        SendPacket(SmsgPageTextQueryResponse(pageId, pageText, itemPage->nextPage).serialise().get());

        pageId = itemPage->nextPage;
    }
}

void WorldSession::handleItemNameQueryOpcode(WorldPacket& recvPacket)
{
    CmsgItemNameQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_ITEM_NAME_QUERY: %u (itemEntry)", srlPacket.itemEntry);

    const auto itemProperties = sMySQLStore.getItemProperties(srlPacket.itemEntry);
    if (itemProperties == nullptr)
        return;

    const auto localizedItem = language > 0 ? sMySQLStore.getLocalizedItem(srlPacket.itemEntry, language) : nullptr;
    const auto name = localizedItem ? localizedItem->name : itemProperties->Name.c_str();

    SendPacket(SmsgItemNameQueryResponse(srlPacket.itemEntry, name, itemProperties->InventoryType).serialise().get());
}

void WorldSession::handleCorpseQueryOpcode(WorldPacket& /*recvPacket*/)
{
    const auto corpse = objmgr.GetCorpseByOwner(_player->getGuidLow());
    if (corpse == nullptr)
        return;

    const auto mapInfo = sMySQLStore.getWorldMapInfo(corpse->GetMapId());
    if (mapInfo == nullptr || mapInfo->type == INSTANCE_NULL || mapInfo->type == INSTANCE_BATTLEGROUND)
    {
        SendPacket(MsgCorspeQuery(uint8_t(1), corpse->GetMapId(), corpse->GetPosition(), corpse->GetMapId(), uint32_t(0)).serialise().get());
    }
    else
    {
        // type INSTANCE_RAID, INSTANCE_NONRAID, INSTANCE_MULTIMODE
        SendPacket(MsgCorspeQuery(uint8_t(1), mapInfo->repopmapid, 
            LocationVector(mapInfo->repopx, mapInfo->repopy, mapInfo->repopz), corpse->GetMapId(), uint32_t(0)).serialise().get());
    }
}