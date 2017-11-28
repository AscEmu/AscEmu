/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/Container.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"

void WorldSession::HandleItemRefundInfoOpcode(WorldPacket& recvPacket)
{
    uint64_t guid;
    recvPacket >> guid;

    SendRefundInfo(guid);
}

void WorldSession::SendRefundInfo(uint64_t guid)
{
    if (!_player || !_player->IsInWorld())
        return;

    Item* item = _player->GetItemInterface()->GetItemByGUID(guid);
    if (item == nullptr)
        return;

    if (item->IsEligibleForRefund())
    {
        std::pair<time_t, uint32_t> RefundEntry;

        RefundEntry = _player->GetItemInterface()->LookupRefundable(guid);

        if (RefundEntry.first == 0 || RefundEntry.second == 0)
            return;

        auto item_extended_cost = sItemExtendedCostStore.LookupEntry(RefundEntry.second);
        if (item_extended_cost == nullptr)
            return;

        ItemProperties const* item_properties = item->GetItemProperties();
        item->SetFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_REFUNDABLE);

        ObjectGuid objectGuid = item->GetGUID();
        WorldPacket data(SMSG_ITEMREFUNDINFO, 68);
        data.writeBit(objectGuid[3]);
        data.writeBit(objectGuid[5]);
        data.writeBit(objectGuid[7]);
        data.writeBit(objectGuid[6]);
        data.writeBit(objectGuid[2]);
        data.writeBit(objectGuid[4]);
        data.writeBit(objectGuid[0]);
        data.writeBit(objectGuid[1]);
        data.flushBits();
        data.WriteByteSeq(objectGuid[7]);

        uint32_t* played = _player->GetPlayedtime();

        if (played[1] >(RefundEntry.first + 60 * 60 * 2))
            data << uint32_t(0);
        else
            data << uint32_t(RefundEntry.first);

        for (uint8_t i = 0; i < 5; ++i)
        {
            data << uint32_t(item_extended_cost->item[i]);
            data << uint32_t(item_extended_cost->count[i]);
        }

        data.WriteByteSeq(objectGuid[6]);
        data.WriteByteSeq(objectGuid[4]);
        data.WriteByteSeq(objectGuid[3]);
        data.WriteByteSeq(objectGuid[2]);
        for (uint8_t i = 0; i < 5; ++i)
        {
            data << uint32_t(item_extended_cost->reqcurrcount[i]);
            data << uint32_t(item_extended_cost->reqcur[i]);
        }

        data.WriteByteSeq(objectGuid[1]);
        data.WriteByteSeq(objectGuid[5]);
        data << uint32_t(0);
        data.WriteByteSeq(objectGuid[0]);
        data << uint32_t(item_properties->BuyPrice);

        SendPacket(&data);
    }
}
