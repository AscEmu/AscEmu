/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
 */

#include "StdAfx.h"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/MapMgrDefines.hpp"
#include "Map/MapMgr.h"
#include "Map/WorldCreatorDefines.hpp"
#include "Storage/WorldStrings.h"

#if VERSION_STRING != Cata
void WorldSession::HandleAreaTriggerOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recv_data, 4);
    uint32 id;
    recv_data >> id;
    _HandleAreaTriggerOpcode(id);
}

enum AreaTriggerFailures
{
    AREA_TRIGGER_FAILURE_OK             = 0,
    AREA_TRIGGER_FAILURE_UNAVAILABLE    = 1,
    AREA_TRIGGER_FAILURE_NO_BC          = 2,
    AREA_TRIGGER_FAILURE_NO_HEROIC      = 3,
    AREA_TRIGGER_FAILURE_NO_RAID        = 4,
    AREA_TRIGGER_FAILURE_NO_ATTUNE_QA   = 5,
    AREA_TRIGGER_FAILURE_NO_ATTUNE_I    = 6,
    AREA_TRIGGER_FAILURE_LEVEL          = 7,
    AREA_TRIGGER_FAILURE_NO_GROUP       = 8,
    AREA_TRIGGER_FAILURE_NO_KEY         = 9,
    AREA_TRIGGER_FAILURE_NO_CHECK       = 10,
    AREA_TRIGGER_FAILURE_NO_WOTLK       = 11,
    AREA_TRIGGER_FAILURE_LEVEL_HEROIC   = 12,
    AREA_TRIGGER_FAILURE_NO_ATTUNE_QH   = 13
};

uint32 AreaTriggerFailureMessages[] =
{
    34,
    26,
    27,
    28,
    29,
    30,
    30,
    31,
    32,
    30,
    33,
    81,
    31, // 33="You must be level 70 to enter Heroic mode." 31="You must be at least level %u to pass through here."
};

uint32 CheckTriggerPrerequisites(MySQLStructure::AreaTrigger const* pAreaTrigger, WorldSession* pSession, Player* pPlayer, MySQLStructure::MapInfo const* pMapInfo)
{
    if (!pMapInfo || !pMapInfo->hasFlag(WMI_INSTANCE_ENABLED))
        return AREA_TRIGGER_FAILURE_UNAVAILABLE;

    if (pMapInfo->hasFlag(WMI_INSTANCE_XPACK_01) && !pSession->HasFlag(ACCOUNT_FLAG_XPACK_01) && !pSession->HasFlag(ACCOUNT_FLAG_XPACK_02))
        return AREA_TRIGGER_FAILURE_NO_BC;

    if (pMapInfo->hasFlag(WMI_INSTANCE_XPACK_02) && !pSession->HasFlag(ACCOUNT_FLAG_XPACK_02))
        return AREA_TRIGGER_FAILURE_NO_WOTLK;

    // These can be overridden by cheats/GM
    if (pPlayer->TriggerpassCheat)
        return AREA_TRIGGER_FAILURE_OK;

    if (pAreaTrigger->requiredLevel && pPlayer->getLevel() < pAreaTrigger->requiredLevel)
        return AREA_TRIGGER_FAILURE_LEVEL;

    if (pPlayer->iInstanceType >= MODE_HEROIC && pMapInfo->type != INSTANCE_MULTIMODE && pMapInfo->type != INSTANCE_NULL)
        return AREA_TRIGGER_FAILURE_NO_HEROIC;

    if (pMapInfo->type == INSTANCE_RAID && (!pPlayer->GetGroup() || (pPlayer->GetGroup() && pPlayer->GetGroup()->GetGroupType() != GROUP_TYPE_RAID)))
        return AREA_TRIGGER_FAILURE_NO_RAID;

    if ((pMapInfo->type == INSTANCE_MULTIMODE && pPlayer->iInstanceType >= MODE_HEROIC) && !pPlayer->GetGroup())
        return AREA_TRIGGER_FAILURE_NO_GROUP;

    if (pMapInfo && pMapInfo->required_quest_A && (pPlayer->GetTeam() == TEAM_ALLIANCE) && !pPlayer->HasFinishedQuest(pMapInfo->required_quest_A))
        return AREA_TRIGGER_FAILURE_NO_ATTUNE_QA;

    if (pMapInfo && pMapInfo->required_quest_H && (pPlayer->GetTeam() == TEAM_HORDE) && !pPlayer->HasFinishedQuest(pMapInfo->required_quest_H))
        return AREA_TRIGGER_FAILURE_NO_ATTUNE_QH;

    if (pMapInfo && pMapInfo->required_item && !pPlayer->GetItemInterface()->GetItemCount(pMapInfo->required_item, true))
        return AREA_TRIGGER_FAILURE_NO_ATTUNE_I;

    if (pPlayer->iInstanceType >= MODE_HEROIC &&
        pMapInfo->type == INSTANCE_MULTIMODE
        && ((pMapInfo->heroic_key_1 > 0 && !pPlayer->GetItemInterface()->GetItemCount(pMapInfo->heroic_key_1, false))
        && (pMapInfo->heroic_key_2 > 0 && !pPlayer->GetItemInterface()->GetItemCount(pMapInfo->heroic_key_2, false))
       )
       )
        return AREA_TRIGGER_FAILURE_NO_KEY;

    if (pMapInfo->type != INSTANCE_NULL && pPlayer->iInstanceType >= MODE_HEROIC && pPlayer->getLevel() < pMapInfo->minlevel_heroic)
        return AREA_TRIGGER_FAILURE_LEVEL_HEROIC;

    return AREA_TRIGGER_FAILURE_OK;
}

void WorldSession::_HandleAreaTriggerOpcode(uint32 id)
{
    LOG_DEBUG("AreaTrigger: %u", id);

    if (!_player->IsInWorld())
        return;

    // Search quest log, find any exploration quests
    sQuestMgr.OnPlayerExploreArea(GetPlayer(), id);

    auto area_trigger_entry = sAreaTriggerStore.LookupEntry(id);
    if (area_trigger_entry == nullptr)
    {
        LOG_DEBUG("Missing AreaTrigger: %u", id);
        return;
    }

    Player* pPlayer = GetPlayer();
    sHookInterface.OnAreaTrigger(pPlayer, id);
    CALL_INSTANCE_SCRIPT_EVENT(pPlayer->GetMapMgr(), OnAreaTrigger)(GetPlayer(), id);

#ifdef GM_Z_DEBUG_DIRECTLY
    if (_player->GetSession() && _player->GetSession()->CanUseCommand('z'))
        sChatHandler.BlueSystemMessage(this, "[%sSystem%s] |rEntered areatrigger: %s%u. (%s)", MSG_COLOR_WHITE, MSG_COLOR_LIGHTBLUE, MSG_COLOR_SUBWHITE, id, pAreaTrigger ? pAreaTrigger->Name : "Unknown name");
#endif

    // if in BG handle is triggers
    if (_player->m_bg)
    {
        _player->m_bg->HookOnAreaTrigger(_player, id);
        return;
    }

    MySQLStructure::AreaTrigger const* pAreaTrigger = sMySQLStore.getAreaTrigger(id);
    if (pAreaTrigger == nullptr)
        return;

    switch (pAreaTrigger->type)
    {
        case ATTYPE_INSTANCE:
        {
            //only ports if player is out of pendings
            if (GetPlayer()->GetPlayerStatus() == TRANSFER_PENDING)
                break;
            if (worldConfig.instance.checkTriggerPrerequisitesOnEnter)
            {
                uint32 reason = CheckTriggerPrerequisites(pAreaTrigger, this, _player, sMySQLStore.getWorldMapInfo(pAreaTrigger->mapId));
                if (reason != AREA_TRIGGER_FAILURE_OK)
                {
                    const char* pReason = GetPlayer()->GetSession()->LocalizedWorldSrv(AreaTriggerFailureMessages[reason]);
                    char msg[200];
                    WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 50);
                    data << uint32(0);

                    switch (reason)
                    {
                        case AREA_TRIGGER_FAILURE_LEVEL:
                            snprintf(msg, 200, pReason, pAreaTrigger->requiredLevel);
                            data << msg;
                            break;
                        case AREA_TRIGGER_FAILURE_NO_ATTUNE_I:
                        {
                            MySQLStructure::MapInfo const* pMi = sMySQLStore.getWorldMapInfo(pAreaTrigger->mapId);
                            ItemProperties const* pItem = sMySQLStore.getItemProperties(pMi->required_item);
                            if (pItem)
                                snprintf(msg, 200, GetPlayer()->GetSession()->LocalizedWorldSrv(ServerString::SS_MUST_HAVE_ITEM), pItem->Name.c_str());
                            else
                                snprintf(msg, 200, "%s", GetPlayer()->GetSession()->LocalizedWorldSrv(36));

                            data << msg;
                        }
                        break;
                        case AREA_TRIGGER_FAILURE_NO_ATTUNE_QA:
                        {
                            MySQLStructure::MapInfo const* pMi = sMySQLStore.getWorldMapInfo(pAreaTrigger->mapId);
                            QuestProperties const* pQuest = sMySQLStore.getQuestProperties(pMi->required_quest_A);
                            if (pQuest)
                                snprintf(msg, 200, GetPlayer()->GetSession()->LocalizedWorldSrv(ServerString::SS_MUST_HAVE_QUEST), pQuest->title.c_str());
                            else
                                snprintf(msg, 200, "%s", GetPlayer()->GetSession()->LocalizedWorldSrv(36));

                            data << msg;
                        }
                        break;
                        case AREA_TRIGGER_FAILURE_NO_ATTUNE_QH:
                        {
                            MySQLStructure::MapInfo const* pMi = sMySQLStore.getWorldMapInfo(pAreaTrigger->mapId);
                            QuestProperties const* pQuest = sMySQLStore.getQuestProperties(pMi->required_quest_H);
                            if (pQuest)
                                snprintf(msg, 200, GetPlayer()->GetSession()->LocalizedWorldSrv(ServerString::SS_MUST_HAVE_QUEST), pQuest->title.c_str());
                            else
                                snprintf(msg, 200, "%s", GetPlayer()->GetSession()->LocalizedWorldSrv(36));

                            data << msg;
                        }
                        break;
                        case AREA_TRIGGER_FAILURE_NO_KEY:
                        {
                            MySQLStructure::MapInfo const* pMi = sMySQLStore.getWorldMapInfo(pAreaTrigger->mapId);
                            ItemProperties const* pItem = sMySQLStore.getItemProperties(pMi->heroic_key_1);
                            if (pItem)
                                snprintf(msg, 200, GetPlayer()->GetSession()->LocalizedWorldSrv(ServerString::SS_MUST_HAVE_ITEM), pItem->Name.c_str());
                            else
                                snprintf(msg, 200, "%s", GetPlayer()->GetSession()->LocalizedWorldSrv(36));

                            data << msg;
                        }
                        break;
                        case AREA_TRIGGER_FAILURE_LEVEL_HEROIC:
                        {
                            MySQLStructure::MapInfo const* pMi = sMySQLStore.getWorldMapInfo(pAreaTrigger->mapId);
                            snprintf(msg, 200, pReason, pMi->minlevel_heroic);
                            data << msg;
                        }
                        break;
                        default:
                            data << pReason;
                            break;
                    }

                    data << uint8(0);
                    SendPacket(&data);
                    return;
                }
            }
            GetPlayer()->SaveEntryPoint(pAreaTrigger->mapId);
            GetPlayer()->SafeTeleport(pAreaTrigger->mapId, 0, LocationVector(pAreaTrigger->x, pAreaTrigger->y, pAreaTrigger->z, pAreaTrigger->o));
        }
        break;
        case ATTYPE_QUESTTRIGGER:
        {

        } break;
        case ATTYPE_INN:
        {
            // Inn
            if (!GetPlayer()->m_isResting) GetPlayer()->ApplyPlayerRestState(true);
        }
        break;
        case ATTYPE_TELEPORT:
        {
            if (GetPlayer()->GetPlayerStatus() != TRANSFER_PENDING) //only ports if player is out of pendings
            {
                GetPlayer()->SaveEntryPoint(pAreaTrigger->mapId);
                GetPlayer()->SafeTeleport(pAreaTrigger->mapId, 0, LocationVector(pAreaTrigger->x, pAreaTrigger->y, pAreaTrigger->z, pAreaTrigger->o));
            }
        }
        break;
        default:
            break;
    }
}
#endif
