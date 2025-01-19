/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
#include "Server/Script/HookInterface.hpp"
#include "Server/Script/InstanceScript.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleAreaTriggerOpcode(WorldPacket& recvPacket)
{
    CmsgAreatrigger srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_AREATRIGGER: {} (triggerId)", srlPacket.triggerId);

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
            char buffer[200];

            const auto session = _player->getSession();

            switch (denyReason)
            {
                case CANNOT_ENTER_NOT_IN_RAID:
                {
                    _player->sendPacket(SmsgRaidGroupOnly(0, 2).serialise().get());
                } break;
                case CANNOT_ENTER_CORPSE_IN_DIFFERENT_INSTANCE:
                {
                    WorldPacket data(SMSG_CORPSE_NOT_IN_INSTANCE, 0);
                    _player->sendPacket(&data);
                } break;
                case CANNOT_ENTER_INSTANCE_BIND_MISMATCH:
                {
                    _player->sendPacket(SmsgTransferAborted(areaTrigger->mapId, INSTANCE_ABORT_ERROR).serialise().get());
                } break;
                case CANNOT_ENTER_TOO_MANY_INSTANCES:
                {
                    _player->sendPacket(SmsgTransferAborted(areaTrigger->mapId, INSTANCE_ABORT_TOO_MANY).serialise().get());
                } break;
                case CANNOT_ENTER_MAX_PLAYERS:
                {
                    _player->sendPacket(SmsgTransferAborted(areaTrigger->mapId, INSTANCE_ABORT_FULL).serialise().get());
                } break;
                case CANNOT_ENTER_ENCOUNTER:
                {
                    _player->sendPacket(SmsgTransferAborted(areaTrigger->mapId, INSTANCE_ABORT_ENCOUNTER).serialise().get());
                } break;
                case CANNOT_ENTER_MIN_LEVEL:
                {
                    snprintf(buffer, 200, session->LocalizedWorldSrv(ServerString::SS_MUST_BE_LEVEL_X), areaTrigger->requiredLevel);
                    _player->sendAreaTriggerMessage(buffer);
                } break;
                case CANNOT_ENTER_ATTUNE_ITEM:
                {
                    const auto itemProperties = sMySQLStore.getItemProperties(mapInfo->required_item);
                    snprintf(buffer, 200, session->LocalizedWorldSrv(ServerString::SS_MUST_HAVE_ITEM), itemProperties ? itemProperties->Name.c_str() : "UNKNOWN");
                    _player->sendAreaTriggerMessage(buffer);
                } break;
                case CANNOT_ENTER_ATTUNE_QA:
                {
                    const auto questProperties = sMySQLStore.getQuestProperties(mapInfo->required_quest_A);
                    snprintf(buffer, 200, session->LocalizedWorldSrv(ServerString::SS_MUST_HAVE_QUEST), questProperties ? questProperties->title.c_str() : "UNKNOWN");
                    _player->sendAreaTriggerMessage(buffer);
                } break;
                case CANNOT_ENTER_ATTUNE_QH:
                {
                    const auto questProperties = sMySQLStore.getQuestProperties(mapInfo->required_quest_H);
                    snprintf(buffer, 200, session->LocalizedWorldSrv(ServerString::SS_MUST_HAVE_QUEST), questProperties ? questProperties->title.c_str() : "UNKNOWN");
                    _player->sendAreaTriggerMessage(buffer);
                } break;
                case CANNOT_ENTER_KEY:
                {
                    const auto itemProperties = sMySQLStore.getItemProperties(mapInfo->heroic_key_1);
                    snprintf(buffer, 200, session->LocalizedWorldSrv(ServerString::SS_MUST_HAVE_ITEM), itemProperties ? itemProperties->Name.c_str() : "UNKNOWN");
                    _player->sendAreaTriggerMessage(buffer);
                } break;
                case CANNOT_ENTER_MIN_LEVEL_HC:
                {
                    snprintf(buffer, 200, session->LocalizedWorldSrv(ServerString::SS_MUST_BE_LEVEL_X), mapInfo->minlevel_heroic);
                    _player->sendAreaTriggerMessage(buffer);
                } break;
                default:
                    break;
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
