/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/TaxiMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/MapMgr.h"

#define TAXIMASK_SIZE 86

void WorldSession::HandleTaxiNodeStatusQueryOpcode(WorldPacket& recvData)
{
    uint64_t guid;

    recvData >> guid;

    uint32_t curloc = sTaxiMgr.GetNearestTaxiNode(GetPlayer()->GetPositionX(), GetPlayer()->GetPositionY(), GetPlayer()->GetPositionZ(), GetPlayer()->GetMapId());
    uint8_t field = static_cast<uint8_t>((curloc - 1) / 32);
    uint32_t submask = 1 << ((curloc - 1) % 32);

    WorldPacket data(SMSG_TAXINODE_STATUS, 9);
    data << guid;

    if ((GetPlayer()->GetTaximask(field) & submask) != submask)
        data << uint8_t(0);
    else
        data << uint8_t(1);

    SendPacket(&data);
}

void WorldSession::HandleTaxiQueryAvaibleNodesOpcode(WorldPacket& recvData)
{
    uint64_t guid;

    recvData >> guid;

    Creature* creature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (creature == nullptr)
        return;

    SendTaxiList(creature);
}

void WorldSession::SendTaxiList(Creature* pCreature)
{
    uint32_t TaxiMask[TAXIMASK_SIZE];
    uint64_t guid = pCreature->GetGUID();

    uint32_t curloc = sTaxiMgr.GetNearestTaxiNode(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId());
    if (curloc == 0)
        return;

    uint8_t field = static_cast<uint8_t>((curloc - 1) / 32);
    uint32_t submask = 1 << ((curloc - 1) % 32);

    if (!(GetPlayer()->GetTaximask(field) & submask) && !GetPlayer()->TaxiCheat)
    {
        GetPlayer()->SetTaximask(field, (submask | GetPlayer()->GetTaximask(field)));

        OutPacket(SMSG_NEW_TAXI_PATH);

        WorldPacket update(SMSG_TAXINODE_STATUS, 9);
        update << guid;
        update << uint8_t(1);
        SendPacket(&update);
    }

    memset(TaxiMask, 0, sizeof(uint32_t) * TAXIMASK_SIZE);
    sTaxiMgr.GetGlobalTaxiNodeMask(curloc, TaxiMask);
    if (field < TAXIMASK_SIZE)
        TaxiMask[field] |= submask;

    if (!GetPlayer()->TaxiCheat)
    {
        for (uint8_t i = 0; i < TAXIMASK_SIZE; ++i)
            TaxiMask[i] &= GetPlayer()->GetTaximask(i);
    }

    WorldPacket data(SMSG_SHOWTAXINODES, 1 + 8 + TAXIMASK_SIZE * 4);
    data << uint32_t(1);
    data << guid;
    data << uint32_t(curloc);
    data << uint32_t(TAXIMASK_SIZE * 4);
    for (uint8_t i = 0; i < TAXIMASK_SIZE; ++i)
        data << TaxiMask[i];

    SendPacket(&data);
}

void WorldSession::HandleActivateTaxiOpcode(WorldPacket& recvData)
{
    uint64_t guid;
    uint32_t sourcenode;
    uint32_t destinationnode;

    recvData >> guid;
    recvData >> sourcenode;
    recvData >> destinationnode;

    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER))
        return;

    TaxiPath* taxipath = sTaxiMgr.GetTaxiPath(sourcenode, destinationnode);
    TaxiNode* taxinode = sTaxiMgr.GetTaxiNode(sourcenode);

    if (taxinode == nullptr)
        return;

    uint32_t curloc = taxinode->id;
    uint8_t field = static_cast<uint8_t>((curloc - 1) / 32);
    uint32_t submask = 1 << ((curloc - 1) % 32);

    WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);
    if ((GetPlayer()->GetTaximask(field) & submask) != submask)
    {
        data << uint32_t(1);
        SendPacket(&data);
        return;
    }

    if (taxipath == nullptr || !taxipath->GetNodeCount())
    {
        data << uint32_t(1);
        SendPacket(&data);
        return;
    }

    int32_t newmoney = (GetPlayer()->GetGold() - taxipath->GetPrice());
    if (newmoney < 0)
    {
        data << uint32_t(3);
        SendPacket(&data);
        return;
    }

    uint32_t modelid = 0;
    if (_player->IsTeamHorde())
    {
        CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(taxinode->horde_mount);

        if (creatureProperties == nullptr)
            creatureProperties = sMySQLStore.getCreatureProperties(taxinode->alliance_mount);

        if (creatureProperties == nullptr)
            creatureProperties = sMySQLStore.getCreatureProperties(541);

        if (creatureProperties != nullptr)
            modelid = creatureProperties->Male_DisplayID;
        else
            modelid = 6852;
    }
    else
    {
        CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(taxinode->alliance_mount);

        if (creatureProperties == nullptr)
            creatureProperties = sMySQLStore.getCreatureProperties(taxinode->horde_mount);

        if (creatureProperties == nullptr)
            creatureProperties = sMySQLStore.getCreatureProperties(541);

        if (creatureProperties != nullptr)
            modelid = creatureProperties->Male_DisplayID;
        else
            modelid = 6852;
    }

    data << uint32_t(0);
    SendPacket(&data);

    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
    {
        _player->interruptSpellWithSpellType(CurrentSpellType(i), false);
    }

    _player->taxi_model_id = modelid;
    _player->TaxiStart(taxipath, modelid, 0);
}

void WorldSession::HandleMultipleActivateTaxiOpcode(WorldPacket& recvPacket)
{
    LogDebugFlag(LF_OPCODE, "HandleMultipleActivateTaxiOpcode : Received CMSG_ACTIVATETAXI");

    uint64 guid;
    uint32 nodecount;
    std::vector<uint32> pathes;
    WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);

    recvPacket >> guid >> nodecount;
    if (nodecount < 2)
        return;

    if (nodecount > 10)
    {
        Disconnect();
        return;
    }

    for (uint32 i = 0; i < nodecount; ++i)
        pathes.push_back(recvPacket.read<uint32>());

    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER))
        return;

    // get first trip
    TaxiPath* taxipath = sTaxiMgr.GetTaxiPath(pathes[0], pathes[1]);
    TaxiNode* taxinode = sTaxiMgr.GetTaxiNode(pathes[0]);

    // Check for valid node
    if (taxinode == nullptr)
    {
        data << uint32(1);
        SendPacket(&data);
        return;
    }

    if (taxipath == nullptr || !taxipath->GetNodeCount())
    {
        data << uint32(2);
        SendPacket(&data);
        return;
    }

    uint32 curloc = taxinode->id;
    uint8 field = (uint8)((curloc - 1) / 32);
    uint32 submask = 1 << ((curloc - 1) % 32);

    // Check for known nodes
    if ((_player->GetTaximask(field) & submask) != submask)
    {
        data << uint32(1);
        SendPacket(&data);
        return;
    }

    if (taxipath->GetID() == 766 || taxipath->GetID() == 767 || taxipath->GetID() == 771 || taxipath->GetID() == 772)
    {
        data << uint32(2);
        SendPacket(&data);
        return;
    }

    uint32 totalcost = taxipath->GetPrice();
    for (uint32 i = 2; i < nodecount; ++i)
    {
        TaxiPath* np = sTaxiMgr.GetTaxiPath(pathes[i - 1], pathes[i]);
        if (!np) return;
        totalcost += np->GetPrice();
    }

    // Check for gold
    int32 newmoney = (GetPlayer()->GetGold() - totalcost);
    if (newmoney < 0)
    {
        data << uint32(3);
        SendPacket(&data);
        return;
    }

    // MOUNTDISPLAYID
    // bat: 1566
    // gryph: 1147
    // wyvern: 295
    // hippogryph: 479

    uint32 modelid = 0;
    if (_player->IsTeamHorde())
    {
        CreatureProperties const* ci = sMySQLStore.getCreatureProperties(taxinode->horde_mount);
        if (!ci)
            return;
        modelid = ci->Male_DisplayID;
        if (!modelid)
            return;
    }
    else
    {
        CreatureProperties const* ci = sMySQLStore.getCreatureProperties(taxinode->alliance_mount);
        if (!ci)
            return;
        modelid = ci->Male_DisplayID;
        if (!modelid)
            return;
    }

    //GetPlayer()->setDismountCost(newmoney);

    data << uint32(0);
    // 0 Ok
    // 1 Unspecified Server Taxi Error
    // 2.There is no direct path to that direction
    // 3 Not enough Money
    SendPacket(&data);
    LogDebugFlag(LF_OPCODE, "HandleMultipleActivateTaxiOpcode : Sent SMSG_ACTIVATETAXIREPLY");

    // 0x001000 seems to make a mount visible
    // 0x002000 seems to make you sit on the mount, and the mount move with you
    // 0x000004 locks you so you can't move, no msg_move updates are sent to the server
    // 0x000008 seems to enable detailed collision checking

    _player->taxi_model_id = modelid;

    // build the rest of the path list
    for (uint32 i = 2; i < nodecount; ++i)
    {
        TaxiPath* np = sTaxiMgr.GetTaxiPath(pathes[i - 1], pathes[i]);
        if (!np) return;

        // add to the list.. :)
        _player->m_taxiPaths.push_back(np);
    }

    // start the first trip :)
    _player->TaxiStart(taxipath, modelid, 0);
}
