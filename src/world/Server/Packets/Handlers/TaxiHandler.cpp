/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Management/TaxiMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Packets/CmsgTaxiQueryAvailableNodes.h"
#include "Server/Packets/CmsgEnabletaxi.h"
#include "Server/Packets/SmsgTaxinodeStatus.h"
#include "Server/Packets/CmsgTaxinodeStatusQuery.h"
#include "Server/Packets/SmsgShowTaxiNodes.h"
#include "Server/Packets/SmsgActivatetaxireply.h"
#include "Server/Packets/CmsgActivatetaxiexpress.h"
#include "Server/Packets/CmsgActivatetaxi.h"
#include "Map/Management/MapMgr.hpp"
#include "Server/Packets/SmsgNewTaxiPath.h"
#include "Movement/MovementGenerators/FlightPathMovementGenerator.h"

using namespace AscEmu::Packets;

void WorldSession::sendTaxiStatus(WoWGuid guid)
{
    Player* const player = GetPlayer();
    Creature* unit = player->getWorldMapCreature(guid.getRawGuid());
    if (!unit || isHostile(unit, player) || !unit->isTaxi())
    {
        sLogger.failure("WorldSession::sendTaxiStatus Creature with guid - " I64FMT " not found.", player->getGuid());
        return;
    }

    // find taxi node
    uint32 nearest = sTaxiMgr.getNearestTaxiNode(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), unit->GetMapId(), player->GetTeam());
    if (!nearest)
        return;

    uint8_t status = player->m_taxi.isTaximaskNodeKnown(nearest) ? 1 : 2;
    SendPacket(SmsgTaxinodeStatus(guid.getRawGuid(), status).serialise().get());
}

void WorldSession::sendTaxiMenu(Creature* unit)
{
    // find current node
    uint32_t nearestNode = sTaxiMgr.getNearestTaxiNode(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), unit->GetMapId(), GetPlayer()->GetTeam());
    if (!nearestNode)
        return;

    bool lastTaxiCheaterState = GetPlayer()->m_cheats.hasTaxiCheat;
    if (unit->getEntry() == 29480)
        GetPlayer()->m_cheats.hasTaxiCheat = true; // Grimwing in Ebon Hold, special case. NOTE: Not perfect, Zul'Aman should not be included according to WoWhead, and I think taxicheat includes it.

    WorldPacket data(SMSG_SHOWTAXINODES, (4 + 8 + 4 + 8 * 4));
    data << uint32_t(1);
    data << uint64_t(unit->getGuid());
    data << uint32_t(nearestNode);
    GetPlayer()->m_taxi.appendTaximaskTo(data, GetPlayer()->m_cheats.hasTaxiCheat);
    SendPacket(&data);

    GetPlayer()->m_cheats.hasTaxiCheat = lastTaxiCheaterState;
}

void WorldSession::sendDoFlight(uint32_t mountDisplayId, uint32_t path, uint32_t pathNode)
{
    TaxiPathNodeList const& nodes = sTaxiPathNodesByPath[path];
    if (nodes.size() < pathNode)
    {
        sLogger.failure("Taxi cannot be Started PathId %u contains %u nodes but Startnode was set to %u", path, nodes.size(), pathNode);
        return;
    }

    // remove fake death
    if (GetPlayer()->hasUnitStateFlag(UNIT_STATE_DIED))
        GetPlayer()->removeAllAurasByAuraEffect(SPELL_AURA_FEIGN_DEATH);

    if (mountDisplayId)
        GetPlayer()->mount(mountDisplayId);

    GetPlayer()->m_taxi.setNodeAfterTeleport(0);
    GetPlayer()->getMovementManager()->moveTaxiFlight(path, pathNode);
}

bool WorldSession::sendLearnNewTaxiNode(Creature* unit)
{
    // find current node
    uint32 curloc = sTaxiMgr.getNearestTaxiNode(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), unit->GetMapId(), GetPlayer()->GetTeam());

    if (curloc == 0)
        return true;

    if (GetPlayer()->m_taxi.setTaximaskNode(curloc))
    {
        SendPacket(SmsgNewTaxiPath().serialise().get());
        SendPacket(SmsgTaxinodeStatus(unit->getGuid(), 1).serialise().get());

        return true;
    }
    else
        return false;
}

void WorldSession::sendDiscoverNewTaxiNode(uint32_t nodeid)
{
    if (GetPlayer()->m_taxi.setTaximaskNode(nodeid))
    {
        GetPlayer()->sendPacket(SmsgNewTaxiPath().serialise().get());
    }
}

void WorldSession::handleTaxiNodeStatusQueryOpcode(WorldPacket& recvPacket)
{
    CmsgTaxinodeStatusQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debug("WORLD: Received CMSG_TAXINODE_STATUS_QUERY");

    sendTaxiStatus(WoWGuid(srlPacket.guid));
}

void WorldSession::handleTaxiQueryAvaibleNodesOpcode(WorldPacket& recvPacket)
{
    CmsgTaxiQueryAvailableNodes srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debug("WORLD: Received CMSG_TAXIQUERYAVAILABLENODES");

    // cheating checks
    Creature* unit = GetPlayer()->getCreatureWhenICanInteract(WoWGuid(srlPacket.creatureGuid), UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!unit)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::ERR_TaxiTooFarAway).serialise().get());
        return;
    }

    // remove fake death
    if (GetPlayer()->hasUnitStateFlag(UNIT_STATE_DIED))
        GetPlayer()->removeAllAurasByAuraEffect(SPELL_AURA_FEIGN_DEATH);

    // unknown taxi node case
    if (sendLearnNewTaxiNode(unit))
        return;

    // known taxi node case
    sendTaxiMenu(unit);
}

void WorldSession::handleEnabletaxiOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    CmsgEnabletaxi srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debug("WORLD: Received CMSG_ENABLETAXI");

    // cheating checks
    Creature* unit = GetPlayer()->getCreatureWhenICanInteract(srlPacket.creatureGuid, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!unit)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::ERR_TaxiTooFarAway).serialise().get());
        return;
    }

    // unknown taxi node case
    if (sendLearnNewTaxiNode(unit))
        return;

    // known taxi node case
    sendTaxiMenu(unit);
#endif
}

void WorldSession::handleActivateTaxiOpcode(WorldPacket& recvPacket)
{
    CmsgActivatetaxi srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_ACTIVATETAXI");

    Creature* npc = GetPlayer()->getCreatureWhenICanInteract(srlPacket.guid, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!npc)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::ERR_TaxiTooFarAway).serialise().get());
        return;
    }

    if (!GetPlayer()->m_cheats.hasTaxiCheat)
    {
        if (!GetPlayer()->m_taxi.isTaximaskNodeKnown(srlPacket.nodes[0]) || !GetPlayer()->m_taxi.isTaximaskNodeKnown(srlPacket.nodes[1]))
        {
            SendPacket(SmsgActivatetaxireply(TaxiNodeError::ERR_TaxiNotVisited).serialise().get());
            return;
        }
    }

    // interrupt all casts
    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
        _player->interruptSpellWithSpellType(CurrentSpellType(i));

    GetPlayer()->activateTaxiPathTo(srlPacket.nodes, npc);
}

void WorldSession::handleMultipleActivateTaxiOpcode(WorldPacket& recvPacket)
{
    CmsgActivatetaxiexpress srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_ACTIVATETAXIEXPRESS");

    Creature* npc = GetPlayer()->getCreatureWhenICanInteract(srlPacket.guid, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!npc)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::ERR_TaxiTooFarAway).serialise().get());
        return;
    }
    std::vector<uint32_t> nodes;

    for (uint32_t i = 0; i < srlPacket.nodeCount; ++i)
    {
        if (!GetPlayer()->m_taxi.isTaximaskNodeKnown(srlPacket.pathParts[i]) && !GetPlayer()->m_cheats.hasTaxiCheat)
        {
            SendPacket(SmsgActivatetaxireply(TaxiNodeError::ERR_TaxiNotVisited).serialise().get());
            return;
        }

        nodes.push_back(srlPacket.pathParts[i]);
    }

    if (nodes.empty())
        return;

    GetPlayer()->activateTaxiPathTo(nodes, npc);
}

void WorldSession::handleMoveSplineDoneOpcode(WorldPacket& recvData)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "WORLD: Received CMSG_MOVE_SPLINE_DONE");

    WoWGuid guid;
    recvData >> guid;

    MovementInfo movementInfo;  // used only for proper packet read
    movementInfo.readMovementInfo(recvData, recvData.GetOpcode());

    recvData.read_skip<uint32_t>();   // spline id

    uint32_t curDest = GetPlayer()->m_taxi.getTaxiDestination();
    if (curDest)
    {
        DBC::Structures::TaxiNodesEntry const* curDestNode = sTaxiNodesStore.LookupEntry(curDest);

        // far teleport case
        if (curDestNode && curDestNode->mapid != GetPlayer()->GetMapId() && GetPlayer()->getMovementManager()->getCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE)
        {
            if (FlightPathMovementGenerator* flight = dynamic_cast<FlightPathMovementGenerator*>(GetPlayer()->getMovementManager()->getCurrentMovementGenerator()))
            {
                // short preparations to continue flight
                flight->setCurrentNodeAfterTeleport();
                DBC::Structures::TaxiPathNodeEntry const* node = flight->getPath()[flight->getCurrentNode()];
                flight->skipCurrentNode();

                GetPlayer()->m_taxi.setNodeAfterTeleport(node->NodeIndex);
                GetPlayer()->safeTeleport(curDestNode->mapid, 0, LocationVector(node->x, node->y, node->z, GetPlayer()->GetOrientation()));
            }
        }
    }

    // at this point only 1 node is expected (final destination)
    if (GetPlayer()->m_taxi.getPath().size() != 1)
        return;

    GetPlayer()->cleanupAfterTaxiFlight();
}
