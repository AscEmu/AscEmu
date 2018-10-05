/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgSwapItem.h"
#include "Server/WorldSession.h"
#include "Units/Players/Player.h"
#include "Management/ItemInterface.h"
#include "Server/Packets/CmsgItemrefundinfo.h"
#include "Server/Packets/CmsgItemrefundrequest.h"

using namespace AscEmu::Packets;

void WorldSession::handleSwapItemOpcode(WorldPacket& recvPacket)
{
    CmsgSwapItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_SWAP_ITEM: destInventorySlot %i destSlot %i srcInventorySlot %i srcInventorySlot %i",
        srlPacket.destInventorySlot, srlPacket.destSlot, srlPacket.srcInventorySlot, srlPacket.srcSlot);

    _player->GetItemInterface()->SwapItems(srlPacket.destInventorySlot, srlPacket.destSlot, srlPacket.srcInventorySlot, srlPacket.srcSlot);
}

#if VERSION_STRING == WotLK
void WorldSession::sendRefundInfo(uint64_t GUID)
{
    if (!_player || !_player->IsInWorld())
        return;

    auto item = _player->GetItemInterface()->GetItemByGUID(GUID);
    if (item == nullptr)
        return;

    if (item->IsEligibleForRefund())
    {
        std::pair<time_t, uint32_t> RefundEntry = _player->GetItemInterface()->LookupRefundable(GUID);

        if (RefundEntry.first == 0 || RefundEntry.second == 0)
            return;

        auto item_extended_cost = sItemExtendedCostStore.LookupEntry(RefundEntry.second);
        if (item_extended_cost == nullptr)
            return;

        ItemProperties const* proto = item->getItemProperties();

        item->setFlags(ITEM_FLAG_REFUNDABLE);
        
        WorldPacket packet(SMSG_ITEMREFUNDINFO, 68);
        packet << uint64_t(GUID);
        packet << uint32_t(proto->BuyPrice);
        packet << uint32_t(item_extended_cost->honor_points);
        packet << uint32_t(item_extended_cost->arena_points);

        for (uint8 i = 0; i < 5; ++i)
        {
            packet << uint32_t(item_extended_cost->item[i]);
            packet << uint32_t(item_extended_cost->count[i]);
        }

        packet << uint32_t(0);

        uint32_t* played = _player->GetPlayedtime();

        if (played[1] > (RefundEntry.first + 60 * 60 * 2))
            packet << uint32_t(0);
        else
            packet << uint32_t(RefundEntry.first);

        this->SendPacket(&packet);
    }
}
#elif VERSION_STRING == Cata
void WorldSession::sendRefundInfo(uint64_t guid)
{
    if (!_player || !_player->IsInWorld())
        return;

    Item* item = _player->GetItemInterface()->GetItemByGUID(guid);
    if (item == nullptr)
        return;

    if (item->IsEligibleForRefund())
    {
        std::pair<time_t, uint32_t> refundEntryPair = _player->GetItemInterface()->LookupRefundable(guid);

        if (refundEntryPair.first == 0 || refundEntryPair.second == 0)
            return;

        auto itemExtendedCostEntry = sItemExtendedCostStore.LookupEntry(refundEntryPair.second);
        if (itemExtendedCostEntry == nullptr)
            return;

        ItemProperties const* item_properties = item->getItemProperties();
        item->addFlags(ITEM_FLAG_REFUNDABLE);

        ObjectGuid objectGuid = item->getGuid();
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

        if (played[1] > (refundEntryPair.first + 60 * 60 * 2))
            data << uint32_t(0);
        else
            data << uint32_t(refundEntryPair.first);

        for (uint8_t i = 0; i < 5; ++i)
        {
            data << uint32_t(itemExtendedCostEntry->item[i]);
            data << uint32_t(itemExtendedCostEntry->count[i]);
        }

        data.WriteByteSeq(objectGuid[6]);
        data.WriteByteSeq(objectGuid[4]);
        data.WriteByteSeq(objectGuid[3]);
        data.WriteByteSeq(objectGuid[2]);
        for (uint8_t i = 0; i < 5; ++i)
        {
            data << uint32_t(itemExtendedCostEntry->reqcurrcount[i]);
            data << uint32_t(itemExtendedCostEntry->reqcur[i]);
        }

        data.WriteByteSeq(objectGuid[1]);
        data.WriteByteSeq(objectGuid[5]);
        data << uint32_t(0);
        data.WriteByteSeq(objectGuid[0]);
        data << uint32_t(item_properties->BuyPrice);

        SendPacket(&data);
    }
}
#endif

#if VERSION_STRING >= WotLK
void WorldSession::handleItemRefundInfoOpcode(WorldPacket& recvPacket)
{
    CmsgItemrefundinfo srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_ITEMREFUNDINFO.");

    this->sendRefundInfo(srlPacket.itemGuid);
}

void WorldSession::handleItemRefundRequestOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN
    CmsgItemrefundrequest srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LOG_DEBUG("Recieved CMSG_ITEMREFUNDREQUEST.");

    uint32_t error = 1;

    std::pair<time_t, uint32_t> RefundEntry;
#if VERSION_STRING != Cata
    DBC::Structures::ItemExtendedCostEntry const* itemExtendedCostEntry = nullptr;
#else
    DB2::Structures::ItemExtendedCostEntry const* itemExtendedCostEntry = nullptr;
#endif
    ItemProperties const* itemProperties = nullptr;

    auto item = _player->GetItemInterface()->GetItemByGUID(srlPacket.itemGuid);

    if (item != nullptr)
    {
        if (item->IsEligibleForRefund())
        {
            RefundEntry.first = 0;
            RefundEntry.second = 0;

            RefundEntry = _player->GetItemInterface()->LookupRefundable(srlPacket.itemGuid);

            if (RefundEntry.first != 0 && RefundEntry.second != 0)
            {
                uint32_t* played = _player->GetPlayedtime();
                if (played[1] < (RefundEntry.first + 60 * 60 * 2))
                    itemExtendedCostEntry = sItemExtendedCostStore.LookupEntry(RefundEntry.second);
            }

            if (itemExtendedCostEntry != nullptr)
            {
                itemProperties = item->getItemProperties();

                for (uint8_t i = 0; i < 5; ++i)
                    _player->GetItemInterface()->AddItemById(itemExtendedCostEntry->item[i], itemExtendedCostEntry->count[i], 0);

                _player->GetItemInterface()->AddItemById(43308, itemExtendedCostEntry->honor_points, 0);
                _player->GetItemInterface()->AddItemById(43307, itemExtendedCostEntry->arena_points, 0);
                _player->ModGold(itemProperties->BuyPrice);

                _player->GetItemInterface()->RemoveItemAmtByGuid(srlPacket.itemGuid, 1);

                _player->GetItemInterface()->RemoveRefundable(srlPacket.itemGuid);

                error = 0;
            }
        }
    }

    WorldPacket packet(SMSG_ITEMREFUNDREQUEST, 60);
    packet << uint64_t(srlPacket.itemGuid);
    packet << uint32_t(error);

    if (error == 0)
    {
        packet << uint32_t(itemProperties->BuyPrice);
        packet << uint32_t(itemExtendedCostEntry->honor_points);
        packet << uint32_t(itemExtendedCostEntry->arena_points);

        for (uint8_t i = 0; i < 5; ++i)
        {
            packet << uint32_t(itemExtendedCostEntry->item[i]);
            packet << uint32_t(itemExtendedCostEntry->count[i]);
        }
    }

    SendPacket(&packet);

    LOG_DEBUG("Sent SMSG_ITEMREFUNDREQUEST.");
}
#endif
