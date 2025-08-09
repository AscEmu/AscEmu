/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Server/WorldSession.h"
#include "Management/TaxiMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Packets/CmsgTaxiQueryAvailableNodes.h"
#include "Server/Packets/CmsgEnabletaxi.h"
#include "Server/Packets/SmsgTaxinodeStatus.h"
#include "Server/Packets/CmsgTaxinodeStatusQuery.h"
#include "Server/Packets/SmsgShowTaxiNodes.h"
#include "Server/Packets/SmsgActivateTaxiReply.h"
#include "Server/Packets/CmsgActivateTaxiExpress.h"
#include "Server/Packets/CmsgActivateTaxi.h"
#include "Movement/MovementManager.h"
#include "Server/Packets/SmsgNewTaxiPath.h"
#include "Movement/MovementGenerators/FlightPathMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Storage/WDB/WDBStructures.hpp"

using namespace AscEmu::Packets;

void WorldSession::sendTaxiStatus(WoWGuid guid)
{
    Player* const player = GetPlayer();
    Creature* unit = player->getWorldMapCreature(guid.getRawGuid());
    if (!unit || unit->isHostileTo(player) || !unit->isTaxi())
    {
        sLogger.failure("WorldSession::sendTaxiStatus Creature with guid - {} not found.", std::to_string(unit->getGuid()));
        return;
    }

    // find taxi node
    const auto nearest = sTaxiMgr.getNearestTaxiNode(unit->GetPosition(), unit->GetMapId(), player->GetTeam());
    if (nearest == 0)
        return;

    SendPacket(SmsgTaxinodeStatus(guid.getRawGuid(), player->m_taxi->isTaximaskNodeKnown(nearest)).serialise().get());
}

void WorldSession::sendTaxiMenu(Creature* unit)
{
    // find current node
    const auto nearestNode = sTaxiMgr.getNearestTaxiNode(unit->GetPosition(), unit->GetMapId(), GetPlayer()->GetTeam());
    if (nearestNode == 0)
        return;

    bool lastTaxiCheaterState = GetPlayer()->m_cheats.hasTaxiCheat;
    if (unit->getEntry() == 29480)
        GetPlayer()->m_cheats.hasTaxiCheat = true; // Grimwing in Ebon Hold, special case. NOTE: Not perfect, Zul'Aman should not be included according to WoWhead, and I think taxicheat includes it.

    const auto& taxiMask = GetPlayer()->getTaxiData()->getTaxiMask(GetPlayer()->m_cheats.hasTaxiCheat);
    SendPacket(SmsgShowTaxiNodes(unit->getGuid(), nearestNode, taxiMask).serialise().get());

    GetPlayer()->m_cheats.hasTaxiCheat = lastTaxiCheaterState;
}

void WorldSession::sendDoFlight(uint32_t mountDisplayId, uint32_t path, uint32_t pathNode)
{
    TaxiPathNodeList const& nodes = sTaxiPathNodesByPath[path];
    if (nodes.size() < pathNode)
    {
        sLogger.failure("Taxi cannot be Started PathId {} contains {} nodes but Startnode was set to {}", path, nodes.size(), pathNode);
        return;
    }

    // remove fake death
    if (GetPlayer()->hasUnitStateFlag(UNIT_STATE_DIED))
        GetPlayer()->removeAllAurasByAuraEffect(SPELL_AURA_FEIGN_DEATH);

    if (mountDisplayId)
        GetPlayer()->mount(mountDisplayId);

    GetPlayer()->m_taxi->setNodeAfterTeleport(0);
    GetPlayer()->getMovementManager()->moveTaxiFlight(path, pathNode);
}

bool WorldSession::sendLearnNewTaxiNode(Creature* unit)
{
    // find current node
    const auto curloc = sTaxiMgr.getNearestTaxiNode(unit->GetPosition(), unit->GetMapId(), GetPlayer()->GetTeam());
    if (curloc == 0)
        return true;

    if (GetPlayer()->m_taxi->setTaximaskNode(curloc))
    {
        SendPacket(SmsgNewTaxiPath().serialise().get());
        SendPacket(SmsgTaxinodeStatus(unit->getGuid(), true).serialise().get());
        return true;
    }

    return false;
}

void WorldSession::sendDiscoverNewTaxiNode(uint32_t nodeid)
{
    if (GetPlayer()->m_taxi->setTaximaskNode(nodeid))
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
    Creature* unit = GetPlayer()->getCreatureWhenICanInteract(WoWGuid(srlPacket.creatureGuid), UNIT_NPC_FLAG_TAXI);
    if (!unit)
    {
        SendPacket(SmsgActivateTaxiReply(TaxiNodeError::ERR_TaxiTooFarAway).serialise().get());
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
    Creature* unit = GetPlayer()->getCreatureWhenICanInteract(srlPacket.creatureGuid, UNIT_NPC_FLAG_TAXI);
    if (!unit)
    {
        SendPacket(SmsgActivateTaxiReply(TaxiNodeError::ERR_TaxiTooFarAway).serialise().get());
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
    CmsgActivateTaxi srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_ACTIVATE_TAXI");

    Creature* npc = GetPlayer()->getCreatureWhenICanInteract(srlPacket.guid, UNIT_NPC_FLAG_TAXI);
    if (!npc)
    {
        SendPacket(SmsgActivateTaxiReply(TaxiNodeError::ERR_TaxiTooFarAway).serialise().get());
        return;
    }

    if (!GetPlayer()->m_cheats.hasTaxiCheat)
    {
        if (!GetPlayer()->m_taxi->isTaximaskNodeKnown(srlPacket.nodes[0]) || !GetPlayer()->m_taxi->isTaximaskNodeKnown(srlPacket.nodes[1]))
        {
            SendPacket(SmsgActivateTaxiReply(TaxiNodeError::ERR_TaxiNotVisited).serialise().get());
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
    CmsgActivateTaxiExpress srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_ACTIVATE_TAXI_EXPRESS");

    Creature* npc = GetPlayer()->getCreatureWhenICanInteract(srlPacket.guid, UNIT_NPC_FLAG_TAXI);
    if (!npc)
    {
        SendPacket(SmsgActivateTaxiReply(TaxiNodeError::ERR_TaxiTooFarAway).serialise().get());
        return;
    }
    std::vector<uint32_t> nodes;

    for (uint32_t i = 0; i < srlPacket.nodeCount; ++i)
    {
        if (!GetPlayer()->m_taxi->isTaximaskNodeKnown(srlPacket.pathParts[i]) && !GetPlayer()->m_cheats.hasTaxiCheat)
        {
            SendPacket(SmsgActivateTaxiReply(TaxiNodeError::ERR_TaxiNotVisited).serialise().get());
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

    uint32_t curDest = GetPlayer()->m_taxi->getTaxiDestination();
    if (curDest)
    {
        WDB::Structures::TaxiNodesEntry const* curDestNode = sTaxiNodesStore.lookupEntry(curDest);

        // far teleport case
        if (curDestNode && curDestNode->mapid != GetPlayer()->GetMapId() && GetPlayer()->getMovementManager()->getCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE)
        {
            if (FlightPathMovementGenerator* flight = dynamic_cast<FlightPathMovementGenerator*>(GetPlayer()->getMovementManager()->getCurrentMovementGenerator()))
            {
                // short preparations to continue flight
                flight->setCurrentNodeAfterTeleport();
                WDB::Structures::TaxiPathNodeEntry const* node = flight->getPath()[flight->getCurrentNode()];
                flight->skipCurrentNode();

                GetPlayer()->m_taxi->setNodeAfterTeleport(node->NodeIndex);
                GetPlayer()->safeTeleport(curDestNode->mapid, 0, LocationVector(node->x, node->y, node->z, GetPlayer()->GetOrientation()));
            }
        }
    }

    // at this point only 1 node is expected (final destination)
    if (GetPlayer()->m_taxi->getPath().size() != 1)
        return;

    GetPlayer()->cleanupAfterTaxiFlight();
}
