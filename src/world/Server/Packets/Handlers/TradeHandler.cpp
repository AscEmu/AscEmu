/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

void WorldSession::SendTradeResult(TradeStatus result)
{
    ObjectGuid guid;

    WorldPacket data(SMSG_TRADE_STATUS, 4 + 8);
    data.writeBit(false);
    data.writeBits(result, 5);

    switch (result)
    {
        case TRADE_STATUS_BEGIN_TRADE:
        {
            data.writeBit(guid[2]);
            data.writeBit(guid[4]);
            data.writeBit(guid[6]);
            data.writeBit(guid[0]);
            data.writeBit(guid[1]);
            data.writeBit(guid[3]);
            data.writeBit(guid[7]);
            data.writeBit(guid[5]);

            data.flushBits();

            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[5]);
            break;
        }
        case TRADE_STATUS_OPEN_WINDOW:
        {
            data.flushBits();
            data << uint32(0);
            break;
        }
        case TRADE_STATUS_CLOSE_WINDOW:
        {
            data.writeBit(false);
            data.flushBits();
            data << uint32(0);
            data << uint32(0);
            break;
        }
        case TRADE_STATUS_NOT_ON_TAPLIST:
        case TRADE_STATUS_ONLY_CONJURED:
        {
            data.flushBits();
            data << uint8(0);
            break;
        }
        case TRADE_STATUS_CURRENCY_NOT_TRADEABLE:
        case TRADE_STATUS_CURRENCY:
        {
            data.flushBits();
            data << uint32(0);
            data << uint32(0);
        }
        default:
            data.flushBits();
            break;
    }

    SendPacket(&data);
}

void WorldSession::SendTradeUpdate(bool trade_state /*= true*/)
{
    TradeData* trade_data = trade_state ? _player->GetTradeData()->GetTargetTradeData() : _player->GetTradeData();

    WorldPacket data(SMSG_TRADE_STATUS_EXTENDED, 100);
    data << uint32(0);                  // unk
    data << uint32(0);                  // unk
    data << uint64(trade_data->GetMoney());
    data << uint32(trade_data->GetSpell());
    data << uint32(TRADE_SLOT_COUNT);
    data << uint32(0);                  // unk
    data << uint8(trade_state ? 1 : 0);
    data << uint32(TRADE_SLOT_COUNT);

    uint8 count = 0;
    for (uint8 i = 0; i < TRADE_SLOT_COUNT; ++i)
        if (Item* item = trade_data->GetTradeItem(TradeSlots(i)))
            ++count;

    data.writeBits(count, 22);

    for (uint8 i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (Item* item = trade_data->GetTradeItem(TradeSlots(i)))
        {
            ObjectGuid creatorGuid = item->GetCreatorGUID();
            ObjectGuid giftCreatorGuid = item->GetGiftCreatorGUID();

            data.writeBit(giftCreatorGuid[7]);
            data.writeBit(giftCreatorGuid[1]);
            bool notWrapped = data.writeBit(!item->HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_WRAPPED));     //wrapped
            data.writeBit(giftCreatorGuid[3]);

            if (notWrapped)
            {
                data.writeBit(creatorGuid[7]);
                data.writeBit(creatorGuid[1]);
                data.writeBit(creatorGuid[4]);
                data.writeBit(creatorGuid[6]);
                data.writeBit(creatorGuid[2]);
                data.writeBit(creatorGuid[3]);
                data.writeBit(creatorGuid[5]);
                data.writeBit(item->GetItemProperties()->LockId != 0);
                data.writeBit(creatorGuid[0]);
            }
            data.writeBit(giftCreatorGuid[6]);
            data.writeBit(giftCreatorGuid[4]);
            data.writeBit(giftCreatorGuid[2]);
            data.writeBit(giftCreatorGuid[0]);
            data.writeBit(giftCreatorGuid[5]);
        }
    }

    data.flushBits();

    for (uint8 i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (Item* item = trade_data->GetTradeItem(TradeSlots(i)))
        {
            ObjectGuid creatorGuid = item->GetCreatorGUID();
            ObjectGuid giftCreatorGuid = item->GetGiftCreatorGUID();

            if (!item->HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_WRAPPED))
            {
                data.WriteByteSeq(creatorGuid[1]);

                data << uint32(item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT));
                for (uint32 enchant_slot = 2; enchant_slot < 5; ++enchant_slot)
                    data << uint32(item->GetEnchantmentId(EnchantmentSlot(enchant_slot)));
                data << uint32(item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY));

                data.WriteByteSeq(creatorGuid[6]);
                data.WriteByteSeq(creatorGuid[2]);
                data.WriteByteSeq(creatorGuid[7]);
                data.WriteByteSeq(creatorGuid[4]);

                data << uint32(item->GetEnchantmentId(REFORGE_ENCHANTMENT_SLOT));
                data << uint32(item->GetUInt32Value(ITEM_FIELD_DURABILITY));
                data << uint32(item->GetItemRandomPropertyId());

                data.WriteByteSeq(creatorGuid[3]);

                data << uint32(0);                      // unk

                data.WriteByteSeq(creatorGuid[0]);

                data << uint32(item->GetCharges(0));
                data << uint32(item->GetItemRandomSuffixFactor());

                data.WriteByteSeq(creatorGuid[5]);
            }

            data.WriteByteSeq(giftCreatorGuid[6]);
            data.WriteByteSeq(giftCreatorGuid[1]);
            data.WriteByteSeq(giftCreatorGuid[7]);
            data.WriteByteSeq(giftCreatorGuid[4]);

            data << uint32(item->GetItemProperties()->ItemId);

            data.WriteByteSeq(giftCreatorGuid[0]);

            data << uint32(item->GetStackCount());

            data.WriteByteSeq(giftCreatorGuid[5]);

            data << uint8(i);                           // slot

            data.WriteByteSeq(giftCreatorGuid[2]);
            data.WriteByteSeq(giftCreatorGuid[3]);
        }
    }

    SendPacket(&data);
}

void WorldSession::HandleInitiateTradeOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    ObjectGuid target_guid;

    target_guid[0] = recv_data.readBit();
    target_guid[3] = recv_data.readBit();
    target_guid[5] = recv_data.readBit();
    target_guid[1] = recv_data.readBit();
    target_guid[4] = recv_data.readBit();
    target_guid[6] = recv_data.readBit();
    target_guid[7] = recv_data.readBit();
    target_guid[2] = recv_data.readBit();

    recv_data.ReadByteSeq(target_guid[7]);
    recv_data.ReadByteSeq(target_guid[4]);
    recv_data.ReadByteSeq(target_guid[3]);
    recv_data.ReadByteSeq(target_guid[5]);
    recv_data.ReadByteSeq(target_guid[1]);
    recv_data.ReadByteSeq(target_guid[2]);
    recv_data.ReadByteSeq(target_guid[6]);
    recv_data.ReadByteSeq(target_guid[0]);

    if (GetPlayer()->m_TradeData)
        return;

    Player* player_target = _player->GetMapMgr()->GetPlayer((uint32)target_guid);
    if (player_target == nullptr)
    {
        SendTradeResult(TRADE_STATUS_NO_TARGET);
        return;
    }

    if (player_target == GetPlayer() || player_target->m_TradeData)
    {
        SendTradeResult(TRADE_STATUS_BUSY);
        return;
    }

    if (player_target->CalcDistance(_player) > 10.0f)
    {
        SendTradeResult(TRADE_STATUS_TARGET_TO_FAR);
        return;
    }

    if (GetPlayer()->IsDead())
    {
        SendTradeResult(TRADE_STATUS_YOU_DEAD);
        return;
    }

    if (IsLoggingOut())
    {
        SendTradeResult(TRADE_STATUS_YOU_LOGOUT);
        return;
    }

    if (player_target->GetSession()->IsLoggingOut())
    {
        SendTradeResult(TRADE_STATUS_TARGET_LOGOUT);
        return;
    }

    if (player_target->IsDead())
    {
        SendTradeResult(TRADE_STATUS_TARGET_DEAD);
        return;
    }

    if (player_target->GetTeam() != _player->GetTeam() && GetPermissionCount() == 0 && !sWorld.interfaction_trade)
    {
        SendTradeResult(TRADE_STATUS_WRONG_FACTION);
        return;
    }

    _player->m_TradeData = new TradeData(_player, player_target);
    player_target->m_TradeData = new TradeData(player_target, _player);

    ObjectGuid source_guid = _player->GetGUID();

    WorldPacket data(SMSG_TRADE_STATUS, 12);
    data.writeBit(false);
    data.writeBits(TRADE_STATUS_BEGIN_TRADE, 5);

    data.WriteByteMask(source_guid[2]);
    data.WriteByteMask(source_guid[4]);
    data.WriteByteMask(source_guid[6]);
    data.WriteByteMask(source_guid[0]);
    data.WriteByteMask(source_guid[1]);
    data.WriteByteMask(source_guid[3]);
    data.WriteByteMask(source_guid[7]);
    data.WriteByteMask(source_guid[5]);

    data.WriteByteSeq(source_guid[4]);
    data.WriteByteSeq(source_guid[1]);
    data.WriteByteSeq(source_guid[2]);
    data.WriteByteSeq(source_guid[3]);
    data.WriteByteSeq(source_guid[0]);
    data.WriteByteSeq(source_guid[7]);
    data.WriteByteSeq(source_guid[6]);
    data.WriteByteSeq(source_guid[5]);

    data << uint32(0);              // unk

    player_target->GetSession()->SendPacket(&data);
}

void WorldSession::HandleBeginTradeOpcode(WorldPacket& recv_data)
{
    TradeData* trade_data = _player->m_TradeData;
    if (!trade_data)
        return;

    trade_data->GetTradeTarget()->GetSession()->SendTradeResult(TRADE_STATUS_OPEN_WINDOW);
    SendTradeResult(TRADE_STATUS_OPEN_WINDOW);
}

void WorldSession::HandleSetTradeGoldOpcode(WorldPacket& recv_data)
{
    uint64 gold;
    recv_data >> gold;

    TradeData* trade_data = _player->GetTradeData();
    if (!trade_data)
        return;

    trade_data->SetMoney(gold);
}

void TradeData::SetMoney(uint64 money)
{
    if (mMoney == money)
        return;

    if (money > mPlayer->GetGold())
    {
        mPlayer->GetSession()->SendTradeResult(TRADE_STATUS_CLOSE_WINDOW);
        return;
    }

    mMoney = money;

    SetAccepted(false);
    GetTargetTradeData()->SetAccepted(false);

    UpdateTrade();
}

void TradeData::UpdateTrade(bool for_trader /*= true*/)
{
    if (for_trader)
        mTradeTarget->GetSession()->SendTradeUpdate(true);
    else
        mPlayer->GetSession()->SendTradeUpdate(false);
}

void TradeData::SetAccepted(bool state, bool send_both /*= false*/)
{
    mAccepted = state;

    if (!state)
    {
        if (send_both)
            mTradeTarget->GetSession()->SendTradeResult(TRADE_STATUS_BACK_TO_TRADE);
        else
            mPlayer->GetSession()->SendTradeResult(TRADE_STATUS_BACK_TO_TRADE);
    }
}

static void setAcceptTradeMode(TradeData* myTrade, TradeData* hisTrade, Item** myItems, Item** hisItems)
{
    myTrade->SetInAcceptProcess(true);
    hisTrade->SetInAcceptProcess(true);

    for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        if (Item* item = myTrade->GetTradeItem(TradeSlots(i)))
        {
            LOG_DEBUG("player trade Item %s", item->GetItemProperties()->Name.c_str());
            myItems[i] = item;
            myItems[i]->SetInTrade();
        }

        if (Item* item = hisTrade->GetTradeItem(TradeSlots(i)))
        {
            LOG_DEBUG("partner trade Item %s", item->GetItemProperties()->Name.c_str());
            hisItems[i] = item;
            hisItems[i]->SetInTrade();
        }
    }
}

static void clearAcceptTradeMode(TradeData* myTrade, TradeData* hisTrade)
{
    myTrade->SetInAcceptProcess(false);
    hisTrade->SetInAcceptProcess(false);
}

static void clearAcceptTradeMode(Item** myItems, Item** hisItems)
{
    for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        if (myItems[i])
            myItems[i]->SetInTrade(false);

        if (hisItems[i])
            hisItems[i]->SetInTrade(false);
    }
}

void WorldSession::HandleAcceptTradeOpcode(WorldPacket& recv_data)
{
    recv_data.read_skip<uint32>();

    TradeData* trade_data = _player->m_TradeData;
    if (!trade_data)
        return;

    Player* trade_target = trade_data->GetTradeTarget();

    TradeData* target_trade_data = trade_target->m_TradeData;
    if (!target_trade_data)
        return;

    Item* trade_items[TRADE_SLOT_TRADED_COUNT] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    Item* target_trade_items[TRADE_SLOT_TRADED_COUNT] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    bool myCanCompleteTrade = true, hisCanCompleteTrade = true;

    trade_data->SetAccepted(true);

    if (trade_data->GetMoney() > _player->GetGold())
    {
        trade_data->SetAccepted(false, true);
        return;
    }

    if (target_trade_data->GetMoney() > trade_target->GetGold())
    {
        target_trade_data->SetAccepted(false, true);
        return;
    }

    for (int i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        if (Item* item = trade_data->GetTradeItem(TradeSlots(i)))
        {
            if (item->IsContainer() && static_cast< Container* >(item)->HasItems() || (item->GetItemProperties()->Bonding == ITEM_BIND_ON_PICKUP))
            {
                SendTradeResult(TRADE_STATUS_TRADE_CANCELED);
                return;
            }
        }

        if (Item* item = target_trade_data->GetTradeItem(TradeSlots(i)))
        {
            if (item->IsContainer() && static_cast<Container*>(item)->HasItems() || (item->GetItemProperties()->Bonding == ITEM_BIND_ON_PICKUP))
            {
                SendTradeResult(TRADE_STATUS_TRADE_CANCELED);
                return;
            }
        }
    }

    if (target_trade_data->IsAccepted())
    {
        setAcceptTradeMode(trade_data, target_trade_data, trade_items, target_trade_items);

        trade_target->GetSession()->SendTradeResult(TRADE_STATUS_TRADE_ACCEPT);

        clearAcceptTradeMode(trade_items, target_trade_items);

        // Remove all items from the players inventory
        for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
        {
            if (trade_items[i])
            {
                trade_items[i]->SetUInt64Value(ITEM_FIELD_GIFTCREATOR, _player->GetGUID());
                _player->m_ItemInterface->SafeRemoveAndRetreiveItemByGuid(trade_items[i]->GetGUID(), true);
            }
            if (target_trade_items[i])
            {
                target_trade_items[i]->SetUInt64Value(ITEM_FIELD_GIFTCREATOR, trade_target->GetGUID());
                trade_target->m_ItemInterface->SafeRemoveAndRetreiveItemByGuid(target_trade_items[i]->GetGUID(), true);
            }
        }

        // Add all items to the targeted player
        //\todo check if target can take the items, otherwise send it back.
        for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
        {
            if (trade_items[i])
            {
                trade_items[i]->SetOwner(trade_target);
                if (!trade_target->m_ItemInterface->AddItemToFreeSlot(trade_items[i]))
                    trade_items[i]->DeleteMe();
            }
            if (target_trade_items[i])
            {
                target_trade_items[i]->SetOwner(_player);
                if (!_player->m_ItemInterface->AddItemToFreeSlot(target_trade_items[i]))
                    target_trade_items[i]->DeleteMe();
            }
        }

        // Trade Gold
        if (target_trade_data->GetMoney())
        {
            // Check they don't have more than the max gold
            if (sWorld.GoldCapEnabled && (_player->GetGold() + target_trade_data->GetMoney()) > sWorld.GoldLimit)
            {
                _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_TOO_MUCH_GOLD);
            }
            else
            {
                _player->ModGold(target_trade_data->GetMoney());
                trade_target->ModGold(-(int32)target_trade_data->GetMoney());
            }
        }

        if (trade_data->GetMoney())
        {
            // Check they don't have more than the max gold
            if (sWorld.GoldCapEnabled && (trade_target->GetGold() + trade_data->GetMoney()) > sWorld.GoldLimit)
            {
                trade_target->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_TOO_MUCH_GOLD);
            }
            else
            {
                trade_target->ModGold(trade_data->GetMoney());
                _player->ModGold(-(int32)trade_data->GetMoney());
            }
        }

        clearAcceptTradeMode(trade_data, target_trade_data);
        delete _player->m_TradeData;
        _player->m_TradeData = nullptr;
        delete trade_target->m_TradeData;
        trade_target->m_TradeData = nullptr;

        trade_target->GetSession()->SendTradeResult(TRADE_STATUS_TRADE_COMPLETE);
        SendTradeResult(TRADE_STATUS_TRADE_COMPLETE);

        // Save for each other
        trade_target->SaveToDB(false);
        _player->SaveToDB(false);
    }
    else
    {
        trade_target->GetSession()->SendTradeResult(TRADE_STATUS_TRADE_ACCEPT);
    }
}

void WorldSession::HandleCancelTradeOpcode(WorldPacket& recv_data)
{
    if (_player)
        _player->TradeCancel(true);
}

void WorldSession::HandleSetTradeItemOpcode(WorldPacket& recv_data)
{
    uint8 TradeSlot;
    uint8 SourceBag;
    uint8 SourceSlot;

    recv_data >> SourceSlot;
    recv_data >> TradeSlot;
    recv_data >> SourceBag;

    TradeData* trade_data = _player->m_TradeData;
    if (!trade_data)
        return;

    if (TradeSlot >= TRADE_SLOT_COUNT)
    {
        SendTradeResult(TRADE_STATUS_TRADE_CANCELED);
        return;
    }

    Item* item = _player->GetItemInterface()->GetInventoryItem(SourceBag, SourceSlot);
    if (!item || (TradeSlot != TRADE_SLOT_NONTRADED && (item->IsAccountbound() || item->IsSoulbound())))
    {
        SendTradeResult(TRADE_STATUS_TRADE_CANCELED);
        return;
    }

    if (trade_data->HasTradeItem(item->GetGUID()))
    {
        SendTradeResult(TRADE_STATUS_TRADE_CANCELED);
        return;
    }

    trade_data->SetItem(TradeSlots(TradeSlot), item);
}

void WorldSession::SendCancelTrade()
{
    SendTradeResult(TRADE_STATUS_TRADE_CANCELED);
}

void WorldSession::HandleClearTradeItemOpcode(WorldPacket& recv_data)
{
    uint8 TradeSlot;
    recv_data >> TradeSlot;

    TradeData* trade_data = _player->m_TradeData;
    if (!trade_data)
        return;

    if (TradeSlot >= TRADE_SLOT_COUNT)
        return;

    trade_data->SetItem(TradeSlots(TradeSlot), nullptr);
}

/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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


//void WorldSession::HandleBusyTrade(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN
//
//    uint32 TradeStatus = TRADE_STATUS_PLAYER_BUSY;
//
//    Player* plr = _player->GetTradeTarget();
//    if (_player->mTradeTarget == 0 || plr == 0)
//    {
//        TradeStatus = TRADE_STATUS_PLAYER_NOT_FOUND;
//
//        OutPacket(TRADE_STATUS_PLAYER_NOT_FOUND, 4, &TradeStatus);
//        return;
//    }
//
//
//    OutPacket(SMSG_TRADE_STATUS, 4, &TradeStatus);
//    plr->m_session->OutPacket(SMSG_TRADE_STATUS, 4, &TradeStatus);
//
//    plr->mTradeStatus = TradeStatus;
//    _player->mTradeStatus = TradeStatus;
//
//    plr->mTradeTarget = 0;
//    _player->mTradeTarget = 0;
//}

//void WorldSession::HandleIgnoreTrade(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN
//
//    uint32 TradeStatus = TRADE_STATUS_PLAYER_IGNORED;
//
//    Player* plr = _player->GetTradeTarget();
//    if (_player->mTradeTarget == 0 || plr == 0)
//    {
//        TradeStatus = TRADE_STATUS_PLAYER_NOT_FOUND;
//
//        OutPacket(TRADE_STATUS_PLAYER_NOT_FOUND, 4, &TradeStatus);
//        return;
//    }
//
//    OutPacket(SMSG_TRADE_STATUS, 4, &TradeStatus);
//    plr->m_session->OutPacket(SMSG_TRADE_STATUS, 4, &TradeStatus);
//
//    plr->mTradeStatus = TradeStatus;
//    _player->mTradeStatus = TradeStatus;
//
//    plr->mTradeTarget = 0;
//    _player->mTradeTarget = 0;
//}

//void WorldSession::HandleUnacceptTrade(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN
//
//    Player* plr = _player->GetTradeTarget();
//    //_player->ResetTradeVariables();
//
//    if (_player->mTradeTarget == 0 || plr == 0)
//        return;
//
//    uint32 TradeStatus = TRADE_STATUS_UNACCEPTED;
//
//    OutPacket(SMSG_TRADE_STATUS, 4, &TradeStatus);
//    plr->m_session->OutPacket(SMSG_TRADE_STATUS, 4, &TradeStatus);
//
//    TradeStatus = TRADE_STATUS_STATE_CHANGED;
//
//    OutPacket(SMSG_TRADE_STATUS, 4, &TradeStatus);
//    plr->m_session->OutPacket(SMSG_TRADE_STATUS, 4, &TradeStatus);
//
//    plr->mTradeStatus = TradeStatus;
//    _player->mTradeStatus = TradeStatus;
//}

