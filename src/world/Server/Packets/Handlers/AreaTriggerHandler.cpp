/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Server/Packets/CmsgAreatrigger.h"
#include "Server/WorldSession.h"
#include "Map/Maps/InstanceDefines.hpp"
#include "Management/Group.h"
#include "Management/QuestMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Storage/WorldStrings.h"
#include "Management/Battleground/Battleground.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Server/Packets/SmsgTransferAborted.h"
#include "Server/Packets/SmsgRaidGroupOnly.h"
#include "Server/Packets/SmsgCorpseNotInInstance.h"
#include "Server/Script/HookInterface.hpp"
#include "Server/Script/InstanceScript.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleAreaTriggerOpcode(WorldPacket& recvPacket)
{
    CmsgAreatrigger srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_AREATRIGGER: {} (triggerId)", srlPacket.triggerId);

    if (!_player->IsInWorld())
        return;

    sQuestMgr.OnPlayerExploreArea(_player, srlPacket.triggerId);

    const auto areaTriggerEntry = sAreaTriggerStore.lookupEntry(srlPacket.triggerId);
    if (areaTriggerEntry == nullptr)
    {
        sLogger.debug("{} is not part of AreaTrigger.dbc", srlPacket.triggerId);
        return;
    }

    sHookInterface.OnAreaTrigger(_player, srlPacket.triggerId);

    if (_player->getWorldMap() && _player->getWorldMap()->getScript())
        _player->getWorldMap()->getScript()->OnAreaTrigger(_player, srlPacket.triggerId);

    if (_player->m_bg)
    {
        _player->m_bg->HookOnAreaTrigger(_player, srlPacket.triggerId);
        return;
    }

    const auto areaTrigger = sMySQLStore.getAreaTrigger(srlPacket.triggerId);
    if (areaTrigger == nullptr)
        return;

    if (_player->GetMapId() != areaTrigger->mapId)
    {
        const auto mapInfo = sMySQLStore.getWorldMapInfo(areaTrigger->mapId);
        EnterState denyReason = sMapMgr.canPlayerEnter(areaTrigger->mapId, areaTrigger->requiredLevel, _player, false);

        if (denyReason != CAN_ENTER)
        {
            if (const auto session = _player->getSession())
            {
                switch (denyReason)
                {
                    case CANNOT_ENTER_NOT_IN_RAID:
                    {
                        SmsgRaidGroupOnly managedPacket(0, 2);
                        session->sendManagedPacket(managedPacket);
                    } break;
                    case CANNOT_ENTER_CORPSE_IN_DIFFERENT_INSTANCE:
                    {
                        SmsgCorpseNotInInstance managedPacket;
                        session->sendManagedPacket(managedPacket);
                    } break;
                    case CANNOT_ENTER_INSTANCE_BIND_MISMATCH:
                    {
                        SmsgTransferAborted managedPacket(areaTrigger->mapId, INSTANCE_ABORT_ERROR);
                        session->sendManagedPacket(managedPacket);
                    } break;
                    case CANNOT_ENTER_TOO_MANY_INSTANCES:
                    {
                        SmsgTransferAborted managedPacket(areaTrigger->mapId, INSTANCE_ABORT_TOO_MANY);
                        session->sendManagedPacket(managedPacket);
                    } break;
                    case CANNOT_ENTER_MAX_PLAYERS:
                    {
                        SmsgTransferAborted managedPacket(areaTrigger->mapId, INSTANCE_ABORT_FULL);
                        session->sendManagedPacket(managedPacket);
                    } break;
                    case CANNOT_ENTER_ENCOUNTER:
                    {
                        SmsgTransferAborted managedPacket(areaTrigger->mapId, INSTANCE_ABORT_ENCOUNTER);
                        session->sendManagedPacket(managedPacket);
                    } break;
                    case CANNOT_ENTER_MIN_LEVEL:
                    {
                        const auto message = fmt::format(session->localizedWorldSrv(ServerString::SS_MUST_BE_LEVEL_X), areaTrigger->requiredLevel);
                        _player->sendAreaTriggerMessage(message);
                    } break;
                    case CANNOT_ENTER_ATTUNE_ITEM:
                    {
                        const auto itemProperties = sMySQLStore.getItemProperties(mapInfo->required_item);
                        const auto message = fmt::format(session->localizedWorldSrv(ServerString::SS_MUST_HAVE_ITEM), itemProperties ? itemProperties->Name : "UNKNOWN");
                        _player->sendAreaTriggerMessage(message);
                    } break;
                    case CANNOT_ENTER_ATTUNE_QA:
                    {
                        const auto questProperties = sMySQLStore.getQuestProperties(mapInfo->required_quest_A);
                        const auto message = fmt::format(session->localizedWorldSrv(ServerString::SS_MUST_HAVE_QUEST), questProperties ? questProperties->title : "UNKNOWN");
                        _player->sendAreaTriggerMessage(message);
                    } break;
                    case CANNOT_ENTER_ATTUNE_QH:
                    {
                        const auto questProperties = sMySQLStore.getQuestProperties(mapInfo->required_quest_H);

                        const auto message = fmt::format(session->localizedWorldSrv(ServerString::SS_MUST_HAVE_QUEST), questProperties ? questProperties->title : "UNKNOWN");
                        _player->sendAreaTriggerMessage(message);
                    } break;
                    case CANNOT_ENTER_KEY:
                    {
                        const auto itemProperties = sMySQLStore.getItemProperties(mapInfo->heroic_key_1);
                        const auto message = fmt::format(session->localizedWorldSrv(ServerString::SS_MUST_HAVE_ITEM), itemProperties ? itemProperties->Name : "UNKNOWN");
                        _player->sendAreaTriggerMessage(message);
                    } break;
                    case CANNOT_ENTER_MIN_LEVEL_HC:
                    {
                        const auto message = fmt::format(session->localizedWorldSrv(ServerString::SS_MUST_BE_LEVEL_X), mapInfo->minlevel_heroic);
                        _player->sendAreaTriggerMessage(message);
                    } break;
                    default:
                        break;
                }
            }
            return;
        }
    }

    switch (areaTrigger->type)
    {
        case ATTYPE_INSTANCE:
        {
            if (_player->isTransferPending())
                break;

            if (!_player->isOnTaxi())
            {
                _player->setMapEntryPoint(areaTrigger->mapId);
                _player->safeTeleport(areaTrigger->mapId, 0, LocationVector(areaTrigger->x, areaTrigger->y, areaTrigger->z, areaTrigger->o));
            }
        } break;
        case ATTYPE_QUESTTRIGGER:
        {

        } break;
        case ATTYPE_INN:
        {
            if (!_player->m_isResting)
                _player->applyPlayerRestState(true);
        } break;
        case ATTYPE_TELEPORT:
        {
            if (!_player->isTransferPending() && !_player->isOnTaxi())
            {
                _player->setMapEntryPoint(areaTrigger->mapId);
                _player->safeTeleport(areaTrigger->mapId, 0, LocationVector(areaTrigger->x, areaTrigger->y, areaTrigger->z, areaTrigger->o));
            }
        } break;
        default:
            break;
    }
}
