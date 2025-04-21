/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Packets/CmsgNameQuery.h"
#include "Server/Packets/CmsgGameobjectQuery.h"
#include "Server/Packets/SmsgQueryPlayernameResponse.h"
#include "Server/Packets/SmsgGameobjectQueryResponse.h"
#include "Server/Packets/SmsgQueryTimeResponse.h"
#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestMgr.h"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
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

    const auto info = sObjectMgr.getCachedCharacterInfo(srlPacket.guid.getGuidLow());
    if (!info)
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_NAME_QUERY for: {}", info->name);
    SendPacket(SmsgQueryPlayernameResponse(srlPacket.guid, info->name, info->race, info->gender, info->cl).serialise().get());
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


    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_GAMEOBJECT_QUERY for entry: {}", srlPacket.entry);
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

    sLogger.debug("Received SMSG_CREATURE_QUERY_RESPONSE for entry: {}", srlPacket.entry);
    SendPacket(SmsgCreatureQueryResponse(creature_info, srlPacket.entry, name, subName).serialise().get());
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

    auto player = sObjectMgr.getPlayer(srlPacket.guid.getGuidLow());
    if (player == nullptr)
        return;

#if VERSION_STRING >= Cata
    player->getAchievementMgr()->sendRespondInspectAchievements(_player);
#else
    player->getAchievementMgr()->sendAllAchievementData(_player);
#endif

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

        if (const auto creature = dynamic_cast<Creature*>(inrangeObject))
        {
            if (creature->isQuestGiver())
            {
                temp.rawGuid = creature->getGuid();
#if VERSION_STRING < Cata
                temp.status = static_cast<uint8_t>(sQuestMgr.CalcStatus(creature, _player));
#else
                temp.status = sQuestMgr.CalcStatus(creature, _player);
#endif
                questgiverSet.push_back(temp);
            }
        }
    }

    SendPacket(SmsgQuestgiverStatusMultiple(uint32_t(questgiverSet.size()), questgiverSet).serialise().get());
}

void WorldSession::handlePageTextQueryOpcode(WorldPacket& recvPacket)
{
    CmsgPageTextQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_PAGE_TEXT_QUERY: {} (pageId)", srlPacket.pageId);

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

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_ITEM_NAME_QUERY: {} (itemEntry)", srlPacket.itemEntry);

    const auto itemProperties = sMySQLStore.getItemProperties(srlPacket.itemEntry);
    if (itemProperties == nullptr)
        return;

    const auto localizedItem = language > 0 ? sMySQLStore.getLocalizedItem(srlPacket.itemEntry, language) : nullptr;
    const auto name = localizedItem ? localizedItem->name : itemProperties->Name.c_str();

    SendPacket(SmsgItemNameQueryResponse(srlPacket.itemEntry, name, itemProperties->InventoryType).serialise().get());
}

void WorldSession::handleCorpseQueryOpcode(WorldPacket& /*recvPacket*/)
{
    const auto corpse = sObjectMgr.getCorpseByOwner(_player->getGuidLow());
    if (corpse == nullptr)
        return;

    const auto mapInfo = sMySQLStore.getWorldMapInfo(corpse->GetMapId());
    if (mapInfo == nullptr || mapInfo->isNonInstanceMap() || mapInfo->isBattleground())
    {
        SendPacket(MsgCorspeQuery(uint8_t(1), corpse->GetMapId(), corpse->GetPosition(), corpse->GetMapId(), uint32_t(0)).serialise().get());
    }
    else
    {
        // type INSTANCE_RAID, INSTANCE_DUNGEON, INSTANCE_MULTIMODE
        SendPacket(MsgCorspeQuery(uint8_t(1), mapInfo->repopmapid, 
            LocationVector(mapInfo->repopx, mapInfo->repopy, mapInfo->repopz), corpse->GetMapId(), uint32_t(0)).serialise().get());
    }
}