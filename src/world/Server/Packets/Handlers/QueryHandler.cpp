/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
    //\todo check utf8 and cyrillic chars

    CmsgNameQuery query;
    if (!query.deserialise(recvData))
    {
        Disconnect();
        return;
    }

    const auto info = objmgr.GetPlayerInfo(query.guid.getGuidLow());
    if (!info)
        return;

    LOG_DEBUG("Received CMSG_NAME_QUERY for: %s", info->name);
    SendPacket(SmsgNameQueryResponse(query.guid, info->name, info->race, info->gender, info->cl).serialise().get());
}

void WorldSession::handleGameObjectQueryOpcode(WorldPacket& recvData)
{
    CmsgGameobjectQuery query;
    if (!query.deserialise(recvData))
    {
        Disconnect();
        return;
    }

    const auto gameobject_info = sMySQLStore.getGameObjectProperties(query.entry);
    if (!gameobject_info)
        return;

    const auto loc = (language > 0) ? sMySQLStore.getLocalizedGameobject(query.entry, language) : nullptr;
    const auto name = loc ? loc->name : gameobject_info->name.c_str();


    LOG_DEBUG("Received CMSG_GAMEOBJECT_QUERY for entry: %u", query.entry);
    SendPacket(SmsgGameobjectQueryResponse(*gameobject_info, name).serialise().get());
}

void WorldSession::handleCreatureQueryOpcode(WorldPacket& recvData)
{
    CmsgCreatureQuery query;
    if (!query.deserialise(recvData))
    {
        Disconnect();
        return;
    }

    const auto creature_info = sMySQLStore.getCreatureProperties(query.entry);
    if (!creature_info)
        return;

    const auto loc = (language > 0) ? sMySQLStore.getLocalizedCreature(query.entry, language) : nullptr;
    const auto name = loc ? loc->name : creature_info->Name.c_str();
    const auto subName = loc ? loc->subName : creature_info->SubName.c_str();

    LOG_DEBUG("Received SMSG_CREATURE_QUERY_RESPONSE for entry: %u", query.entry);
    SendPacket(SmsgCreatureQueryResponse(*creature_info, query.entry, name, subName).serialise().get());
}

void WorldSession::handleQueryTimeOpcode(WorldPacket&)
{
    SendPacket(SmsgQueryTimeResponse(UNIXTIME).serialise().get());
}

void WorldSession::handleAchievmentQueryOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    CmsgInspectAchievements recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    auto player = objmgr.GetPlayer(recv_packet.guid.getGuidLow());
    if (player == nullptr)
        return;

    player->GetAchievementMgr().SendAllAchievementData(GetPlayer());
#endif
}

void WorldSession::handleInrangeQuestgiverQuery(WorldPacket& /*recvPacket*/)
{
    std::vector<QuestgiverInrangeStatus> questgiverSet;
    QuestgiverInrangeStatus temp;

    for (const auto& inrangeObject : GetPlayer()->getInRangeObjectsSet())
    {
        if (inrangeObject == nullptr || !inrangeObject->isCreature())
            continue;

        const auto creature = dynamic_cast<Creature*>(inrangeObject);
        if (creature->isQuestGiver())
        {
            temp.rawGuid = creature->getGuid();
            temp.status = uint8_t(sQuestMgr.CalcStatus(creature, GetPlayer()));
            questgiverSet.push_back(temp);
        }
    }

    SendPacket(SmsgQuestgiverStatusMultiple(uint32_t(questgiverSet.size()), questgiverSet).serialise().get());
}

void WorldSession::handlePageTextQueryOpcode(WorldPacket& recvPacket)
{
    CmsgPageTextQuery recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_PAGE_TEXT_QUERY: %u (pageId)", recv_packet.pageId);

    uint32_t pageId = recv_packet.pageId;
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
    CmsgItemNameQuery recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_ITEM_NAME_QUERY: %u (itemEntry)", recv_packet.itemEntry);

    const auto itemProperties = sMySQLStore.getItemProperties(recv_packet.itemEntry);
    if (itemProperties == nullptr)
        return;

    const auto localizedItem = language > 0 ? sMySQLStore.getLocalizedItem(recv_packet.itemEntry, language) : nullptr;
    const auto name = localizedItem ? localizedItem->name : itemProperties->Name.c_str();

    SendPacket(SmsgItemNameQueryResponse(recv_packet.itemEntry, name, itemProperties->InventoryType).serialise().get());
}

void WorldSession::handleCorpseQueryOpcode(WorldPacket& /*recvPacket*/)
{
    const auto corpse = objmgr.GetCorpseByOwner(GetPlayer()->getGuidLow());
    if (corpse == nullptr)
        return;

    const auto mapInfo = sMySQLStore.getWorldMapInfo(corpse->GetMapId());
    if (mapInfo == nullptr || mapInfo->type == INSTANCE_NULL || mapInfo->type == INSTANCE_BATTLEGROUND)
    {
        SendPacket(MsgCorspeQuery(uint8_t(1), corpse->GetMapId(), corpse->GetPosition(), corpse->GetMapId(), uint32_t(0)).serialise().get());
    }
    else
    {
        // type INSTANCE_RAID, INSTANCE_NONRAID or INSTANCE_MULTIMODE
        SendPacket(MsgCorspeQuery(uint8_t(1), mapInfo->repopmapid, 
            LocationVector(mapInfo->repopx, mapInfo->repopy, mapInfo->repopz), corpse->GetMapId(), uint32_t(0)).serialise().get());
    }
}