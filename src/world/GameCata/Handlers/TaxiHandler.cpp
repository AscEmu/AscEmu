/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/TaxiMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/MapMgr.h"

#define TAXIMASK_SIZE 86

void WorldSession::HandleTaxiNodeStatusQueryOpcode(WorldPacket& recv_data)
{
    uint64_t guid;

    recv_data >> guid;

    uint32_t curloc = sTaxiMgr.GetNearestTaxiNode(GetPlayer()->GetPositionX(), GetPlayer()->GetPositionY(), GetPlayer()->GetPositionZ(), GetPlayer()->GetMapId());
    uint8_t field = (uint8_t)((curloc - 1) / 32);
    uint32_t submask = 1 << ((curloc - 1) % 32);

    WorldPacket data(SMSG_TAXINODE_STATUS, 9);
    data << guid;

    if ((GetPlayer()->GetTaximask(field) & submask) != submask)
        data << uint8_t(0);
    else
        data << uint8_t(1);

    SendPacket(&data);
}

void WorldSession::HandleTaxiQueryAvaibleNodesOpcode(WorldPacket& recv_data)
{
    uint64_t guid;

    recv_data >> guid;

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

    uint8_t field = (uint8_t)((curloc - 1) / 32);
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

void WorldSession::HandleActivateTaxiOpcode(WorldPacket& recv_data)
{
    uint64_t guid;
    uint32_t sourcenode;
    uint32_t destinationnode;

    recv_data >> guid;
    recv_data >> sourcenode;
    recv_data >> destinationnode;

    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER))
        return;

    TaxiPath* taxipath = sTaxiMgr.GetTaxiPath(sourcenode, destinationnode);
    TaxiNode* taxinode = sTaxiMgr.GetTaxiNode(sourcenode);

    if (taxinode == nullptr)
        return;

    uint32_t curloc = taxinode->id;
    uint8_t field = (uint8_t)((curloc - 1) / 32);
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
        CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(taxinode->horde_mount);

        if (creature_properties == nullptr)
            creature_properties = sMySQLStore.getCreatureProperties(taxinode->alliance_mount);

        if (creature_properties == nullptr)
            creature_properties = sMySQLStore.getCreatureProperties(541);

        if (creature_properties != nullptr)
            modelid = creature_properties->Male_DisplayID;
        else
            modelid = 6852;
    }
    else
    {
        CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(taxinode->alliance_mount);

        if (creature_properties == nullptr)
            creature_properties = sMySQLStore.getCreatureProperties(taxinode->horde_mount);

        if (creature_properties == nullptr)
            creature_properties = sMySQLStore.getCreatureProperties(541);

        if (creature_properties != nullptr)
            modelid = creature_properties->Male_DisplayID;
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
