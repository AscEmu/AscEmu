/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgAreatrigger.h"
#include "Server/WorldSession.h"
#include "Server/Packets/SmsgAreaTriggerMessage.h"

using namespace AscEmu::Packets;

namespace AreaTriggerResult
{
    enum
    {
        Success = 0,
        Unavailable = 1,
        NoBurningCrusade = 2,
        NoNeroic = 3,
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

uint32_t checkTriggerPrerequisites(MySQLStructure::AreaTrigger const* pAreaTrigger, WorldSession* pSession, Player* pPlayer, MySQLStructure::MapInfo const* pMapInfo)
{
    if (!pMapInfo || !pMapInfo->hasFlag(WMI_INSTANCE_ENABLED))
        return AreaTriggerResult::Unavailable;

    if (pMapInfo->hasFlag(WMI_INSTANCE_XPACK_01) && !pSession->HasFlag(ACCOUNT_FLAG_XPACK_01) && !pSession->HasFlag(ACCOUNT_FLAG_XPACK_02))
        return AreaTriggerResult::NoBurningCrusade;

    if (pMapInfo->hasFlag(WMI_INSTANCE_XPACK_02) && !pSession->HasFlag(ACCOUNT_FLAG_XPACK_02))
        return AreaTriggerResult::NoWotLK;

    // These can be overridden by cheats/GM
    if (pPlayer->TriggerpassCheat)
        return AreaTriggerResult::Success;

    if (pAreaTrigger->requiredLevel && pPlayer->getLevel() < pAreaTrigger->requiredLevel)
        return AreaTriggerResult::Level;

    if (pPlayer->iInstanceType >= MODE_HEROIC && pMapInfo->type != INSTANCE_MULTIMODE && pMapInfo->type != INSTANCE_NULL)
        return AreaTriggerResult::NoNeroic;

    if (pMapInfo->type == INSTANCE_RAID && (!pPlayer->GetGroup() || (pPlayer->GetGroup() && pPlayer->GetGroup()->getGroupType() != GROUP_TYPE_RAID)))
        return AreaTriggerResult::NoRaid;

    if ((pMapInfo->type == INSTANCE_MULTIMODE && pPlayer->iInstanceType >= MODE_HEROIC) && !pPlayer->GetGroup())
        return AreaTriggerResult::NoGroup;

    if (pMapInfo && pMapInfo->required_quest_A && (pPlayer->GetTeam() == TEAM_ALLIANCE) && !pPlayer->HasFinishedQuest(pMapInfo->required_quest_A))
        return AreaTriggerResult::NoAttuneQA;

    if (pMapInfo && pMapInfo->required_quest_H && (pPlayer->GetTeam() == TEAM_HORDE) && !pPlayer->HasFinishedQuest(pMapInfo->required_quest_H))
        return AreaTriggerResult::NoAttuneQH;

    if (pMapInfo && pMapInfo->required_item && !pPlayer->GetItemInterface()->GetItemCount(pMapInfo->required_item, true))
        return AreaTriggerResult::NoAttuneI;

    if (pPlayer->iInstanceType >= MODE_HEROIC &&
        pMapInfo->type == INSTANCE_MULTIMODE
        && ((pMapInfo->heroic_key_1 > 0 && !pPlayer->GetItemInterface()->GetItemCount(pMapInfo->heroic_key_1, false))
        && (pMapInfo->heroic_key_2 > 0 && !pPlayer->GetItemInterface()->GetItemCount(pMapInfo->heroic_key_2, false))
        )
        )
        return AreaTriggerResult::NoKey;

    if (pMapInfo->type != INSTANCE_NULL && pPlayer->iInstanceType >= MODE_HEROIC && pPlayer->getLevel() < pMapInfo->minlevel_heroic)
        return AreaTriggerResult::LevelHeroic;

    return AreaTriggerResult::Success;
}

void WorldSession::handleAreaTriggerOpcode(WorldPacket& recvPacket)
{
    CmsgAreatrigger recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_AREATRIGGER: %u (triggerId)", recv_packet.triggerId);

    if (!GetPlayer()->IsInWorld())
        return;

    sQuestMgr.OnPlayerExploreArea(GetPlayer(), recv_packet.triggerId);

    const auto areaTriggerEntry = sAreaTriggerStore.LookupEntry(recv_packet.triggerId);
    if (areaTriggerEntry == nullptr)
    {
        LOG_DEBUG("%u is not part of AreaTrigger.dbc", recv_packet.triggerId);
        return;
    }

    sHookInterface.OnAreaTrigger(GetPlayer(), recv_packet.triggerId);
    CALL_INSTANCE_SCRIPT_EVENT(GetPlayer()->GetMapMgr(), OnAreaTrigger)(GetPlayer(), recv_packet.triggerId);

    if (GetPlayer()->m_bg)
    {
        GetPlayer()->m_bg->HookOnAreaTrigger(GetPlayer(), recv_packet.triggerId);
        return;
    }

    const auto areaTrigger = sMySQLStore.getAreaTrigger(recv_packet.triggerId);
    if (areaTrigger == nullptr)
        return;

    switch (areaTrigger->type)
    {
        case ATTYPE_INSTANCE:
        {
            if (GetPlayer()->GetPlayerStatus() == TRANSFER_PENDING)
                break;

            if (worldConfig.instance.checkTriggerPrerequisitesOnEnter)
            {
                const auto mapInfo = sMySQLStore.getWorldMapInfo(areaTrigger->mapId);
                const uint32_t reason = checkTriggerPrerequisites(areaTrigger, this, GetPlayer(), mapInfo);
                if (reason != AreaTriggerResult::Success)
                {
                    char buffer[200];
                    const auto session = GetPlayer()->GetSession();

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
            GetPlayer()->SaveEntryPoint(areaTrigger->mapId);
            GetPlayer()->SafeTeleport(areaTrigger->mapId, 0, LocationVector(areaTrigger->x, areaTrigger->y, areaTrigger->z, areaTrigger->o));
        } break;
        case ATTYPE_QUESTTRIGGER:
        {

        } break;
        case ATTYPE_INN:
        {
            if (!GetPlayer()->m_isResting)
                GetPlayer()->ApplyPlayerRestState(true);
        } break;
        case ATTYPE_TELEPORT:
        {
            if (GetPlayer()->GetPlayerStatus() != TRANSFER_PENDING)
            {
                GetPlayer()->SaveEntryPoint(areaTrigger->mapId);
                GetPlayer()->SafeTeleport(areaTrigger->mapId, 0, LocationVector(areaTrigger->x, areaTrigger->y, areaTrigger->z, areaTrigger->o));
            }
        } break;
        default:
            break;
    }
}
