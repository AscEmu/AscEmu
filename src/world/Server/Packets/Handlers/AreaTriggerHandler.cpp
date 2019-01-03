/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgAreatrigger.h"
#include "Server/WorldSession.h"
#include "Server/Packets/SmsgAreaTriggerMessage.h"
#include "Server/World.Legacy.h"
#include "Map/WorldCreatorDefines.hpp"
#include "Management/Group.h"
#include "Management/ItemInterface.h"
#include "Management/QuestMgr.h"
#include "Map/MapMgrDefines.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Map/MapMgr.h"
#include "Storage/WorldStrings.h"

using namespace AscEmu::Packets;

namespace AreaTriggerResult
{
    enum
    {
        Success = 0,
        Unavailable = 1,
        NoBurningCrusade = 2,
        NoHeroic = 3,
        NoRaid = 4,
        NoAttuneQA = 5,
        NoAttuneI = 6,
        Level = 7,
        NoGroup = 8,
        NoKey = 9,
        NoCheck = 10,
        NoWotLK = 11,
        LevelHeroic = 12,
        NoAttuneQH = 13
    };
}

uint32_t checkTriggerPrerequisites(MySQLStructure::AreaTrigger const* areaTrigger, WorldSession* session, Player* player, MySQLStructure::MapInfo const* mapInfo)
{
    if (!mapInfo || !mapInfo->hasFlag(WMI_INSTANCE_ENABLED))
        return AreaTriggerResult::Unavailable;

    if (mapInfo->hasFlag(WMI_INSTANCE_XPACK_01) && !session->HasFlag(ACCOUNT_FLAG_XPACK_01) && !session->HasFlag(ACCOUNT_FLAG_XPACK_02))
        return AreaTriggerResult::NoBurningCrusade;

    if (mapInfo->hasFlag(WMI_INSTANCE_XPACK_02) && !session->HasFlag(ACCOUNT_FLAG_XPACK_02))
        return AreaTriggerResult::NoWotLK;

    // These can be overridden by cheats/GM
    if (player->m_cheats.TriggerpassCheat)
        return AreaTriggerResult::Success;

    if (areaTrigger->requiredLevel && player->getLevel() < areaTrigger->requiredLevel)
        return AreaTriggerResult::Level;

    if (player->iInstanceType >= MODE_HEROIC && mapInfo->type != INSTANCE_MULTIMODE && mapInfo->type != INSTANCE_NULL)
        return AreaTriggerResult::NoHeroic;

    if (mapInfo->type == INSTANCE_RAID && (!player->GetGroup() || (player->GetGroup() && player->GetGroup()->getGroupType() != GROUP_TYPE_RAID)))
        return AreaTriggerResult::NoRaid;

    if ((mapInfo->type == INSTANCE_MULTIMODE && player->iInstanceType >= MODE_HEROIC) && !player->GetGroup())
        return AreaTriggerResult::NoGroup;

    if (mapInfo && mapInfo->required_quest_A && (player->getTeam() == TEAM_ALLIANCE) && !player->HasFinishedQuest(mapInfo->required_quest_A))
        return AreaTriggerResult::NoAttuneQA;

    if (mapInfo && mapInfo->required_quest_H && (player->getTeam() == TEAM_HORDE) && !player->HasFinishedQuest(mapInfo->required_quest_H))
        return AreaTriggerResult::NoAttuneQH;

    if (mapInfo && mapInfo->required_item && !player->getItemInterface()->GetItemCount(mapInfo->required_item, true))
        return AreaTriggerResult::NoAttuneI;

    if (player->iInstanceType >= MODE_HEROIC &&
        mapInfo->type == INSTANCE_MULTIMODE
        && ((mapInfo->heroic_key_1 > 0 && !player->getItemInterface()->GetItemCount(mapInfo->heroic_key_1, false))
        && (mapInfo->heroic_key_2 > 0 && !player->getItemInterface()->GetItemCount(mapInfo->heroic_key_2, false))
        )
        )
        return AreaTriggerResult::NoKey;

    if (mapInfo->type != INSTANCE_NULL && player->iInstanceType >= MODE_HEROIC && player->getLevel() < mapInfo->minlevel_heroic)
        return AreaTriggerResult::LevelHeroic;

    return AreaTriggerResult::Success;
}

void WorldSession::handleAreaTriggerOpcode(WorldPacket& recvPacket)
{
    CmsgAreatrigger srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AREATRIGGER: %u (triggerId)", srlPacket.triggerId);

    if (!_player->IsInWorld())
        return;

    sQuestMgr.OnPlayerExploreArea(_player, srlPacket.triggerId);

    const auto areaTriggerEntry = sAreaTriggerStore.LookupEntry(srlPacket.triggerId);
    if (areaTriggerEntry == nullptr)
    {
        LogDebugFlag(LF_OPCODE, "%u is not part of AreaTrigger.dbc", srlPacket.triggerId);
        return;
    }

    sHookInterface.OnAreaTrigger(_player, srlPacket.triggerId);
    CALL_INSTANCE_SCRIPT_EVENT(_player->GetMapMgr(), OnAreaTrigger)(_player, srlPacket.triggerId);

    if (_player->m_bg)
    {
        _player->m_bg->HookOnAreaTrigger(_player, srlPacket.triggerId);
        return;
    }

    const auto areaTrigger = sMySQLStore.getAreaTrigger(srlPacket.triggerId);
    if (areaTrigger == nullptr)
        return;

    switch (areaTrigger->type)
    {
        case ATTYPE_INSTANCE:
        {
            if (_player->GetPlayerStatus() == TRANSFER_PENDING)
                break;

            if (worldConfig.instance.checkTriggerPrerequisitesOnEnter)
            {
                const auto mapInfo = sMySQLStore.getWorldMapInfo(areaTrigger->mapId);
                const uint32_t reason = checkTriggerPrerequisites(areaTrigger, this, _player, mapInfo);
                if (reason != AreaTriggerResult::Success)
                {
                    char buffer[200];
                    const auto session = _player->GetSession();

                    switch (reason)
                    {
                        case AreaTriggerResult::Level:
                        {
                            snprintf(buffer, 200, session->LocalizedWorldSrv(SS_MUST_BE_LEVEL_X), areaTrigger->requiredLevel);
                        } break;
                        case AreaTriggerResult::NoAttuneI:
                        {
                            const auto itemProperties = sMySQLStore.getItemProperties(mapInfo->required_item);
                            snprintf(buffer, 200, session->LocalizedWorldSrv(SS_MUST_HAVE_ITEM), itemProperties ? itemProperties->Name.c_str() : "UNKNOWN");
                        } break;
                        case AreaTriggerResult::NoAttuneQA:
                        {
                            const auto questProperties = sMySQLStore.getQuestProperties(mapInfo->required_quest_A);
                            snprintf(buffer, 200, session->LocalizedWorldSrv(SS_MUST_HAVE_QUEST), questProperties ? questProperties->title.c_str() : "UNKNOWN");
                        } break;
                        case AreaTriggerResult::NoAttuneQH:
                        {
                            const auto questProperties = sMySQLStore.getQuestProperties(mapInfo->required_quest_H);
                            snprintf(buffer, 200, session->LocalizedWorldSrv(SS_MUST_HAVE_QUEST), questProperties ? questProperties->title.c_str() : "UNKNOWN");
                        } break;
                        case AreaTriggerResult::NoKey:
                        {
                            const auto itemProperties = sMySQLStore.getItemProperties(mapInfo->heroic_key_1);
                            snprintf(buffer, 200, session->LocalizedWorldSrv(SS_MUST_HAVE_ITEM), itemProperties ? itemProperties->Name.c_str() : "UNKNOWN");
                        } break;
                        case AreaTriggerResult::LevelHeroic:
                        {
                            snprintf(buffer, 200, session->LocalizedWorldSrv(SS_MUST_BE_LEVEL_X), mapInfo->minlevel_heroic);
                        } break;
                        default:
                            break;
                    }

                    SendPacket(SmsgAreaTriggerMessage(sizeof(buffer), buffer, 0).serialise().get());
                    return;
                }
            }
            _player->SaveEntryPoint(areaTrigger->mapId);
            _player->SafeTeleport(areaTrigger->mapId, 0, LocationVector(areaTrigger->x, areaTrigger->y, areaTrigger->z, areaTrigger->o));
        } break;
        case ATTYPE_QUESTTRIGGER:
        {

        } break;
        case ATTYPE_INN:
        {
            if (!_player->m_isResting)
                _player->ApplyPlayerRestState(true);
        } break;
        case ATTYPE_TELEPORT:
        {
            if (_player->GetPlayerStatus() != TRANSFER_PENDING)
            {
                _player->SaveEntryPoint(areaTrigger->mapId);
                _player->SafeTeleport(areaTrigger->mapId, 0, LocationVector(areaTrigger->x, areaTrigger->y, areaTrigger->z, areaTrigger->o));
            }
        } break;
        default:
            break;
    }
}
