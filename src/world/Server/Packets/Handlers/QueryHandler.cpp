/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <cstdint>
#include <vector>
#include <AEVersion.hpp>
#include "Server/Packets/CmsgNameQuery.h"
#include "Server/Packets/CmsgGameobjectQuery.h"
#include "Server/Packets/CmsgRealmNameQuery.h"
#include "Server/Packets/SmsgQueryPlayernameResponse.h"
#include "Server/Packets/SmsgRealmNameQueryResponse.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/Packets/SmsgGameobjectQueryResponse.h"
#include "Server/Packets/SmsgQueryTimeResponse.h"
#include "Utilities/LocationVector.hpp"
#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include <Logging/Severity.hpp>
#include "Management/ObjectMgr.hpp"
#include "Management/QuestMgr.h"
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
#include "Network/WorldPacket.hpp"
#include "Objects/Units/Creatures/Corpse.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleNameQueryOpcode(WorldPacket& recvData)
{
    CmsgNameQuery srlPacket;
    if (!parsePacket(recvData, srlPacket))
    {
        sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Failed to deserialize CMSG_NAME_QUERY.");
        Disconnect();
        return;
    }

    SmsgQueryPlayerNameResponse response;
    response.guid = srlPacket.guid;

    if (const auto info = sObjectMgr.getCachedCharacterInfo(srlPacket.guid.getGuidLow()))
    {
        sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_NAME_QUERY for name: {}, race: {}, gender: {}, class: {}, level: {}",
            info->name, info->race, info->gender, info->cl, info->lastLevel);

        response.hasData = true;
        response.player_name = info->name;
        response.race = info->race;
        response.gender = info->gender;
        response.class_ = info->cl;
        response.level = static_cast<uint8_t>(info->lastLevel);

        response.realmId = sLogonCommHandler.getRealmId();
        response.accountId = GetAccountId();
    }
    else
    {
        sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "CMSG_NAME_QUERY for unknown GUID: {}", srlPacket.guid.getGuidLow());
        response.hasData = false;
    }

    sendManagedPacket(response);
}

void WorldSession::handleRealmNameQueryOpcode(WorldPacket& recvData)
{
    CmsgRealmNameQuery srlPacket;
    if (!parsePacket(recvData, srlPacket))
    {
        sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Failed to deserialize CMSG_REALM_NAME_QUERY.");
        Disconnect();
        return;
    }

    SmsgRealmNameQueryResponse response;
    response.realmId = srlPacket.realmId;

    const auto realmName = sLogonCommHandler.getRealmName(srlPacket.realmId);
    if (!realmName.empty())
    {
        response.found = true;
        response.realmName = realmName;
        response.isLocalRealm = srlPacket.realmId == sLogonCommHandler.getRealmId();
    }

    sendManagedPacket(response);
}

void WorldSession::handleGameObjectQueryOpcode(WorldPacket& recvData)
{
    CmsgGameobjectQuery srlPacket;
    if (!parsePacket(recvData, srlPacket))
    {
        Disconnect();
        return;
    }

    const auto gameobject_info = sMySQLStore.getGameObjectProperties(srlPacket.entry);
    if (!gameobject_info)
        return;

    const auto loc = (language > 0) ? sMySQLStore.getLocalizedGameobject(srlPacket.entry, language) : nullptr;
    const auto name = loc ? loc->name : gameobject_info->name.c_str();

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_GAMEOBJECT_QUERY for entry: {} name : {}", srlPacket.entry, name);

    SmsgGameobjectQueryResponse responsePacket(*gameobject_info, name);
    sendManagedPacket(responsePacket);
}

void WorldSession::handleCreatureQueryOpcode(WorldPacket& recvData)
{
    CmsgCreatureQuery srlPacket;
    if (!parsePacket(recvData, srlPacket))
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

    sLogger.debug("Received CMSG_CREATURE_QUERY for entry: {} ({})", srlPacket.entry, name);

    SmsgCreatureQueryResponse responsePacket(creature_info, srlPacket.entry, name, subName);
    sendManagedPacket(responsePacket);
}

void WorldSession::handleQueryTimeOpcode(WorldPacket& /*recvPacket*/)
{
    SmsgQueryTimeResponse responsePacket(UNIXTIME);
    sendManagedPacket(responsePacket);
}

void WorldSession::handleAchievmentQueryOpcode([[maybe_unused]] WorldPacket& recvPacket)
{
    CmsgInspectAchievements srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    auto player = sObjectMgr.getPlayer(srlPacket.guid.getGuidLow());
    if (player == nullptr)
        return;

#if VERSION_STRING >= Cata
    player->getAchievementMgr()->sendRespondInspectAchievements(_player);
#elif VERSION_STRING == WotLK
    player->getAchievementMgr()->sendAllAchievementData(_player);
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
                temp.status = sQuestMgr.CalcStatus(creature, _player);
                questgiverSet.push_back(temp);
            }
        }
    }

    SmsgQuestgiverStatusMultiple responsePacket(uint32_t(questgiverSet.size()), questgiverSet);
    sendManagedPacket(responsePacket);
}

void WorldSession::handlePageTextQueryOpcode(WorldPacket& recvPacket)
{
    CmsgPageTextQuery srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
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

        SmsgPageTextQueryResponse responsePacket(pageId, pageText, itemPage->nextPage);
        sendManagedPacket(responsePacket);

        pageId = itemPage->nextPage;
    }
}

void WorldSession::handleItemNameQueryOpcode(WorldPacket& recvPacket)
{
    CmsgItemNameQuery srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_ITEM_NAME_QUERY: {} (itemEntry)", srlPacket.itemEntry);

    const auto itemProperties = sMySQLStore.getItemProperties(srlPacket.itemEntry);
    if (itemProperties == nullptr)
        return;

    const auto localizedItem = language > 0 ? sMySQLStore.getLocalizedItem(srlPacket.itemEntry, language) : nullptr;
    const auto name = localizedItem ? localizedItem->name : itemProperties->Name.c_str();

    SmsgItemNameQueryResponse responsePacket(srlPacket.itemEntry, name, itemProperties->InventoryType);
    sendManagedPacket(responsePacket);
}

void WorldSession::handleCorpseQueryOpcode(WorldPacket& /*recvPacket*/)
{
    const auto corpse = sObjectMgr.getCorpseByOwner(_player->getGuidLow());
    if (corpse == nullptr)
        return;

    const auto mapInfo = sMySQLStore.getWorldMapInfo(corpse->GetMapId());
    if (mapInfo == nullptr || mapInfo->isWorldMap() || mapInfo->isBattlegroundOrArena())
    {
        MsgCorspeQuery responsePacket(uint8_t(1), corpse->GetMapId(), corpse->GetPosition(), corpse->GetMapId(), uint32_t(0));
        sendManagedPacket(responsePacket);
    }
    else
    {
        // type INSTANCE_RAID, INSTANCE_DUNGEON, INSTANCE_MULTIMODE
        MsgCorspeQuery responsePacket(uint8_t(1), mapInfo->repopmapid, 
            LocationVector(mapInfo->repopx, mapInfo->repopy, mapInfo->repopz), corpse->GetMapId(), uint32_t(0));
        sendManagedPacket(responsePacket);
    }
}