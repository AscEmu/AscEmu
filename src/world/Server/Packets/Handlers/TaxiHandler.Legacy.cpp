/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
 *
 */

#include "StdAfx.h"
#include "Management/TaxiMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/MapMgr.h"
#include "Server/Packets/CmsgTaxinodeStatusQuery.h"
#include "Server/Packets/SmsgTaxinodeStatus.h"
#include "Server/Packets/CmsgActivatetaxi.h"
#include "Server/Packets/SmsgActivatetaxireply.h"
#include "Server/Packets/CmsgActivatetaxiexpress.h"
#include "Server/Packets/CmsgTaxiQueryAvailableNodes.h"

using namespace AscEmu::Packets;

void WorldSession::HandleTaxiNodeStatusQueryOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CmsgTaxinodeStatusQuery recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    LOG_DEBUG("WORLD: Received CMSG_TAXINODE_STATUS_QUERY");

    uint32 curloc = sTaxiMgr.GetNearestTaxiNode(GetPlayer()->GetPositionX(), GetPlayer()->GetPositionY(),
                                                GetPlayer()->GetPositionZ(), GetPlayer()->GetMapId());

    uint8 field = (uint8)((curloc - 1) / 32);
    uint32 submask = 1 << ((curloc - 1) % 32);

    // Check for known nodes
    uint8_t status = 1;
    if ((GetPlayer()->GetTaximask(field) & submask) != submask)
        status = 0;

    SendPacket(SmsgTaxinodeStatus(recv_packet.guid, status).serialise().get());
}

void WorldSession::SendTaxiList(Creature* pCreature)
{
    uint32 TaxiMask[12];
    uint64 guid = pCreature->getGuid();

    uint32 nearestNode = sTaxiMgr.GetNearestTaxiNode(_player->GetPositionX(), _player->GetPositionY(),
                                                _player->GetPositionZ(), _player->GetMapId());
    if (nearestNode == 0)
        return;

    uint8 field = (uint8)((nearestNode - 1) / 32);
    uint32 submask = 1 << ((nearestNode - 1) % 32);

    // Check for known nodes
    if (!(GetPlayer()->GetTaximask(field) & submask) && !GetPlayer()->TaxiCheat)
    {
        GetPlayer()->SetTaximask(field, (submask | GetPlayer()->GetTaximask(field)));

        OutPacket(SMSG_NEW_TAXI_PATH);

        //Send packet
        SendPacket(SmsgTaxinodeStatus(guid, 1).serialise().get());
    }

    //Set Mask
    memset(TaxiMask, 0, sizeof(TaxiMask));
    sTaxiMgr.GetGlobalTaxiNodeMask(nearestNode, TaxiMask);
    TaxiMask[field] |= 1 << ((nearestNode - 1) % 32);

    //Remove nodes unknown to player
    if (!GetPlayer()->TaxiCheat)
    {
        for (uint8 i = 0; i < 12; i++)
        {
            TaxiMask[i] &= GetPlayer()->GetTaximask(i);
        }
    }

    WorldPacket data(64);
    data.Initialize(SMSG_SHOWTAXINODES);
    data << uint32(1) << guid;
    data << uint32(nearestNode);
    for (uint8 i = 0; i < 12; i++)
    {
        data << TaxiMask[i];
    }
    SendPacket(&data);

    LOG_DEBUG("WORLD: Sent SMSG_SHOWTAXINODES");
}

//MIT
uint32_t getMountForNode(Player* player, TaxiNode* taxiNode)
{
    if (player->IsTeamHorde())
    {
        CreatureProperties const* ci = sMySQLStore.getCreatureProperties(taxiNode->horde_mount);
        if (ci != nullptr)
            return ci->Male_DisplayID;
    }
    else
    {
        CreatureProperties const* ci = sMySQLStore.getCreatureProperties(taxiNode->alliance_mount);
        if (ci != nullptr)
            return ci->Male_DisplayID;
    }

    return 6852;  // riding gryphon
}

void WorldSession::HandleActivateTaxiOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CmsgActivatetaxi recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    LOG_DEBUG("WORLD: Received CMSG_ACTIVATETAXI");

    if (GetPlayer()->hasUnitFlags(UNIT_FLAG_LOCK_PLAYER))
        return;

    TaxiPath* taxipath = sTaxiMgr.GetTaxiPath(recv_packet.srcNode, recv_packet.destNode);
    TaxiNode* taxinode = sTaxiMgr.GetTaxiNode(recv_packet.srcNode);

    if (taxinode == nullptr || taxipath == nullptr)
        return;

    const uint32 currentNode = taxinode->id;
    uint8 field = (uint8)((currentNode - 1) / 32);
    uint32 submask = 1 << ((currentNode - 1) % 32);

    // Check for known nodes
    if ((GetPlayer()->GetTaximask(field) & submask) != submask)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::UnspecificError).serialise().get());
        return;
    }

    // Check for valid node
    if (!taxipath->GetNodeCount())
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::NoDirectPath).serialise().get());
        return;
    }

    // Check for gold
    int32 newmoney = (GetPlayer()->GetGold() - taxipath->GetPrice());
    if (newmoney < 0)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::NotEnoughMoney).serialise().get());
        return;
    }

    uint32 modelid = getMountForNode(_player, taxinode);

    //GetPlayer()->setDismountCost(newmoney);

    SendPacket(SmsgActivatetaxireply(TaxiNodeError::Ok).serialise().get());

    LogDebugFlag(LF_OPCODE, "HandleActivateTaxiOpcode : Sent SMSG_ACTIVATETAXIREPLY");

    // 0x001000 seems to make a mount visible
    // 0x002000 seems to make you sit on the mount, and the mount move with you
    // 0x000004 locks you so you can't move, no msg_move updates are sent to the server
    // 0x000008 seems to enable detailed collision checking

    //! Check if the player is casting, obviously they should not be able to cast on a taxi
    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
        _player->interruptSpellWithSpellType(CurrentSpellType(i));

    _player->taxi_model_id = modelid;
    _player->TaxiStart(taxipath, modelid, 0);

    //LogDefault("TAXI: Starting taxi trip. Next update in %d msec.", first_node_time);
}

void WorldSession::HandleMultipleActivateTaxiOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgActivatetaxiexpress recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "WORLD: Received CMSG_ACTIVATETAXIEXPRESS");

    if (GetPlayer()->hasUnitFlags(UNIT_FLAG_LOCK_PLAYER))
        return;

    // Check for valid node
    TaxiNode* taxinode = sTaxiMgr.GetTaxiNode(recv_packet.pathParts[0]);
    if (taxinode == nullptr)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::UnspecificError).serialise().get());
        return;
    }

    TaxiPath* taxipath = sTaxiMgr.GetTaxiPath(recv_packet.pathParts[0], recv_packet.pathParts[1]);
    if (taxipath == nullptr || !taxipath->GetNodeCount())
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::NoDirectPath).serialise().get());
        return;
    }

    const uint32 currentNode = taxinode->id;
    uint8 field = (uint8)((currentNode - 1) / 32);
    uint32 submask = 1 << ((currentNode - 1) % 32);

    // Check for known nodes
    if ((_player->GetTaximask(field) & submask) != submask)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::UnspecificError).serialise().get());
        return;
    }

    if (taxipath->GetID() == 766 || taxipath->GetID() == 767 || taxipath->GetID() == 771 || taxipath->GetID() == 772)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::NoDirectPath).serialise().get());
        return;
    }

    uint32 totalcost = taxipath->GetPrice();
    for (uint32 i = 2; i < recv_packet.nodeCount; ++i)
    {
        TaxiPath* np = sTaxiMgr.GetTaxiPath(recv_packet.pathParts[i - 1], recv_packet.pathParts[i]);
        if (np == nullptr)
            return;

        totalcost += np->GetPrice();
    }

    // Check for gold
    int32 newmoney = (GetPlayer()->GetGold() - totalcost);
    if (newmoney < 0)
    {
        SendPacket(SmsgActivatetaxireply(TaxiNodeError::NotEnoughMoney).serialise().get());
        return;
    }

    uint32 modelid = getMountForNode(_player, taxinode);

    //GetPlayer()->setDismountCost(newmoney);

    SendPacket(SmsgActivatetaxireply(TaxiNodeError::Ok).serialise().get());

    LogDebugFlag(LF_OPCODE, "HandleMultipleActivateTaxiOpcode : Sent SMSG_ACTIVATETAXIREPLY");

    // 0x001000 seems to make a mount visible
    // 0x002000 seems to make you sit on the mount, and the mount move with you
    // 0x000004 locks you so you can't move, no msg_move updates are sent to the server
    // 0x000008 seems to enable detailed collision checking

    _player->taxi_model_id = modelid;

    // build the rest of the path list
    for (uint32 i = 2; i < recv_packet.nodeCount; ++i)
    {
        TaxiPath* np = sTaxiMgr.GetTaxiPath(recv_packet.pathParts[i - 1], recv_packet.pathParts[i]);
        if (np == nullptr)
            return;

        // add to the list.. :)
        _player->m_taxiPaths.push_back(np);
    }

    // start the first trip :)
    _player->TaxiStart(taxipath, modelid, 0);
}
