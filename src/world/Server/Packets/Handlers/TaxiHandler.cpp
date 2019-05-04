/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
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
#include "Map/MapMgr.h"

using namespace AscEmu::Packets;

void WorldSession::sendTaxiList(Creature* creature)
{
    uint32_t tmpTaxiNodeMask[12];

    uint32_t nearestNode = sTaxiMgr.getNearestNodeForPlayer(_player);
    if (nearestNode == 0)
        return;

    const uint8_t field = static_cast<uint8_t>((nearestNode - 1) / 32);
    const uint32_t subMask = 1 << ((nearestNode - 1) % 32);

    if (!(_player->GetTaximask(field) & subMask) && !_player->m_cheats.TaxiCheat)
    {
        _player->SetTaximask(field, (subMask | _player->GetTaximask(field)));

        OutPacket(SMSG_NEW_TAXI_PATH);

        SendPacket(SmsgTaxinodeStatus(creature->getGuid(), 1).serialise().get());
    }

    memset(tmpTaxiNodeMask, 0, sizeof(tmpTaxiNodeMask));
    sTaxiMgr.GetGlobalTaxiNodeMask(nearestNode, tmpTaxiNodeMask);
    tmpTaxiNodeMask[field] |= 1 << ((nearestNode - 1) % 32);

    if (!_player->m_cheats.TaxiCheat)
    {
        for (uint8_t i = 0; i < 12; ++i)
            tmpTaxiNodeMask[i] &= _player->GetTaximask(i);
    }

    std::array<uint32_t, 12> taxiMask{};
    std::copy(std::begin(tmpTaxiNodeMask), std::end(tmpTaxiNodeMask), std::begin(taxiMask));

    SendPacket(SmsgShowTaxiNodes(creature->getGuid(), nearestNode, taxiMask).serialise().get());
}

uint8_t isTaximaskKnown(Player* player, uint32_t nearestNode)
{
    const auto field = static_cast<uint8_t>((nearestNode - 1) / 32);
    const uint32_t submask = 1 << ((nearestNode - 1) % 32);

    if ((player->GetTaximask(field) & submask) != submask)
        return 0;

    return 1;
}

void WorldSession::handleTaxiNodeStatusQueryOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgTaxinodeStatusQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "WORLD: Received CMSG_TAXINODE_STATUS_QUERY");

    const auto nearestNode = sTaxiMgr.getNearestNodeForPlayer(_player);
    if (!nearestNode)
        return;

    uint8_t status = isTaximaskKnown(_player, nearestNode);
    SendPacket(SmsgTaxinodeStatus(srlPacket.guid, status).serialise().get());
}

void WorldSession::handleTaxiQueryAvaibleNodesOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgTaxiQueryAvailableNodes srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "WORLD: Received CMSG_TAXIQUERYAVAILABLENODES");

    if (const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.creatureGuid.getGuidLowPart()))
        sendTaxiList(creature);
}

void WorldSession::handleEnabletaxiOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    CHECK_INWORLD_RETURN

    CmsgEnabletaxi srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "WORLD: Received CMSG_ENABLETAXI");

    if (const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.creatureGuid.getGuidLowPart()))
        sendTaxiList(creature);
#endif
}

uint32_t getMountForNode(Player* player, TaxiNode* taxiNode)
{
    if (player->isTeamHorde())
    {
        const auto creatureProperties = sMySQLStore.getCreatureProperties(taxiNode->horde_mount);
        if (creatureProperties != nullptr)
            return creatureProperties->Male_DisplayID;
    }
    else
    {
        const auto creatureProperties = sMySQLStore.getCreatureProperties(taxiNode->alliance_mount);
        if (creatureProperties != nullptr)
            return creatureProperties->Male_DisplayID;
    }

    return 6852;
}

void WorldSession::handleActivateTaxiOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgActivatetaxi srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_ACTIVATETAXI");

    if (_player->hasUnitFlags(UNIT_FLAG_LOCK_PLAYER))
        return;

    const auto taxiPath = sTaxiMgr.GetTaxiPath(srlPacket.srcNode, srlPacket.destNode);
    const auto taxiNode = sTaxiMgr.GetTaxiNode(srlPacket.srcNode);

    if (taxiNode == nullptr || taxiPath == nullptr)
        return;

    const uint32_t currentNode = taxiNode->id;
    const uint8_t field = static_cast<uint8_t>((currentNode - 1) / 32);
    const uint32_t subMask = 1 << ((currentNode - 1) % 32);

    if ((_player->GetTaximask(field) & subMask) != subMask)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::UnspecificError).serialise().get());
        return;
    }

    if (!taxiPath->GetNodeCount())
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::NoDirectPath).serialise().get());
        return;
    }

    if (!_player->hasEnoughCoinage(taxiPath->GetPrice()))
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::NotEnoughMoney).serialise().get());
        return;
    }

    SendPacket(SmsgActivatetaxireply(TaxiNodeError::Ok).serialise().get());

    // interrupt all casts
    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
        _player->interruptSpellWithSpellType(CurrentSpellType(i));

    const uint32_t modelId = getMountForNode(_player, taxiNode);
    _player->taxi_model_id = modelId;

    _player->TaxiStart(taxiPath, modelId, 0);
}

void WorldSession::handleMultipleActivateTaxiOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgActivatetaxiexpress srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_ACTIVATETAXIEXPRESS");

    if (_player->hasUnitFlags(UNIT_FLAG_LOCK_PLAYER))
        return;

    const auto taxiNode = sTaxiMgr.GetTaxiNode(srlPacket.pathParts[0]);
    if (taxiNode == nullptr)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::UnspecificError).serialise().get());
        return;
    }

    const auto taxiPath = sTaxiMgr.GetTaxiPath(srlPacket.pathParts[0], srlPacket.pathParts[1]);
    if (taxiPath == nullptr || !taxiPath->GetNodeCount())
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::NoDirectPath).serialise().get());
        return;
    }

    const uint32_t currentNode = taxiNode->id;
    const auto field = static_cast<uint8_t>((currentNode - 1) / 32);
    const uint32_t subMask = 1 << ((currentNode - 1) % 32);

    if ((_player->GetTaximask(field) & subMask) != subMask)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::UnspecificError).serialise().get());
        return;
    }

    if (taxiPath->GetID() == 766 || taxiPath->GetID() == 767 || taxiPath->GetID() == 771 || taxiPath->GetID() == 772)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::NoDirectPath).serialise().get());
        return;
    }

    uint32_t totalCost = taxiPath->GetPrice();
    for (uint32_t i = 2; i < srlPacket.nodeCount; ++i)
    {
        const auto additionalTaxiPath = sTaxiMgr.GetTaxiPath(srlPacket.pathParts[i - 1], srlPacket.pathParts[i]);
        if (additionalTaxiPath == nullptr)
            return;

        totalCost += additionalTaxiPath->GetPrice();
    }

    if (!_player->hasEnoughCoinage(totalCost))
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::NotEnoughMoney).serialise().get());
        return;
    }

    const uint32_t modelId = getMountForNode(_player, taxiNode);
    _player->taxi_model_id = modelId;

    SendPacket(SmsgActivatetaxireply(TaxiNodeError::Ok).serialise().get());

    // build path list
    for (uint32_t i = 2; i < srlPacket.nodeCount; ++i)
    {
        const auto additionalTaxiPath = sTaxiMgr.GetTaxiPath(srlPacket.pathParts[i - 1], srlPacket.pathParts[i]);
        if (additionalTaxiPath == nullptr)
            return;

        _player->m_taxiPaths.push_back(additionalTaxiPath);
    }

    _player->TaxiStart(taxiPath, modelId, 0);
}
