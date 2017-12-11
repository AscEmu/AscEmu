/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/Container.h"
#include "Management/ItemInterface.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Server/WorldSession.h"
#include "Server/World.h"
#include "Objects/ObjectMgr.h"

void WorldSession::sendTradeResult(TradeStatus result)
{
    WorldPacket data(SMSG_TRADE_STATUS, 4 + 8);
    data.writeBit(false);
    data.writeBits(result, 5);

    switch (result)
    {
        case TRADE_STATUS_BEGIN_TRADE:
        {
            ObjectGuid guid;

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
            data << uint32_t(0);
            break;
        }
        case TRADE_STATUS_CLOSE_WINDOW:
        {
            data.writeBit(false);
            data.flushBits();
            data << uint32_t(0);
            data << uint32_t(0);
            break;
        }
        case TRADE_STATUS_NOT_ON_TAPLIST:
        case TRADE_STATUS_ONLY_CONJURED:
        {
            data.flushBits();
            data << uint8_t(0);
            break;
        }
        case TRADE_STATUS_CURRENCY_NOT_TRADEABLE:
        case TRADE_STATUS_CURRENCY:
        {
            data.flushBits();
            data << uint32_t(0);
            data << uint32_t(0);
        }
        default:
            data.flushBits();
            break;
    }

    SendPacket(&data);
}

void WorldSession::sendTradeUpdate(bool tradeState /*= true*/)
{
    TradeData* tradeData = tradeState ? _player->getTradeData()->getTargetTradeData() : _player->getTradeData();

    WorldPacket data(SMSG_TRADE_STATUS_EXTENDED, 100);
    data << uint32_t(0);                  // unk
    data << uint32_t(0);                  // unk
    data << uint64_t(tradeData->getMoney());
    data << uint32_t(tradeData->getSpell());
    data << uint32_t(TRADE_SLOT_COUNT);
    data << uint32_t(0);                  // unk
    data << uint8_t(tradeState ? 1 : 0);
    data << uint32_t(TRADE_SLOT_COUNT);

    uint8_t count = 0;
    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (Item* item = tradeData->getTradeItem(TradeSlots(i)))
            ++count;
    }

    data.writeBits(count, 22);

    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (Item* item = tradeData->getTradeItem(TradeSlots(i)))
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

    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (Item* item = tradeData->getTradeItem(TradeSlots(i)))
        {
            ObjectGuid creatorGuid = item->GetCreatorGUID();
            ObjectGuid giftCreatorGuid = item->GetGiftCreatorGUID();

            if (!item->HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_WRAPPED))
            {
                data.WriteByteSeq(creatorGuid[1]);

                data << uint32_t(item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT));
                for (uint32_t enchant_slot = 2; enchant_slot < 5; ++enchant_slot)
                {
                    data << uint32_t(item->GetEnchantmentId(static_cast<uint16_t>(EnchantmentSlot(enchant_slot))));
                }

                data << uint32_t(item->getUInt32Value(ITEM_FIELD_MAXDURABILITY));

                data.WriteByteSeq(creatorGuid[6]);
                data.WriteByteSeq(creatorGuid[2]);
                data.WriteByteSeq(creatorGuid[7]);
                data.WriteByteSeq(creatorGuid[4]);

                data << uint32_t(item->GetEnchantmentId(REFORGE_ENCHANTMENT_SLOT));
                data << uint32_t(item->getUInt32Value(ITEM_FIELD_DURABILITY));
                data << uint32_t(item->GetItemRandomPropertyId());

                data.WriteByteSeq(creatorGuid[3]);

                data << uint32_t(0);                      // unk

                data.WriteByteSeq(creatorGuid[0]);

                data << uint32_t(item->GetCharges(0));
                data << uint32_t(item->GetItemRandomSuffixFactor());

                data.WriteByteSeq(creatorGuid[5]);
            }

            data.WriteByteSeq(giftCreatorGuid[6]);
            data.WriteByteSeq(giftCreatorGuid[1]);
            data.WriteByteSeq(giftCreatorGuid[7]);
            data.WriteByteSeq(giftCreatorGuid[4]);

            data << uint32_t(item->GetItemProperties()->ItemId);

            data.WriteByteSeq(giftCreatorGuid[0]);

            data << uint32_t(item->GetStackCount());

            data.WriteByteSeq(giftCreatorGuid[5]);

            data << uint8_t(i);                           // slot

            data.WriteByteSeq(giftCreatorGuid[2]);
            data.WriteByteSeq(giftCreatorGuid[3]);
        }
    }

    SendPacket(&data);
}

void WorldSession::HandleInitiateTradeOpcode(WorldPacket& recvData)
{
    ObjectGuid targetGuid;

    targetGuid[0] = recvData.readBit();
    targetGuid[3] = recvData.readBit();
    targetGuid[5] = recvData.readBit();
    targetGuid[1] = recvData.readBit();
    targetGuid[4] = recvData.readBit();
    targetGuid[6] = recvData.readBit();
    targetGuid[7] = recvData.readBit();
    targetGuid[2] = recvData.readBit();

    recvData.ReadByteSeq(targetGuid[7]);
    recvData.ReadByteSeq(targetGuid[4]);
    recvData.ReadByteSeq(targetGuid[3]);
    recvData.ReadByteSeq(targetGuid[5]);
    recvData.ReadByteSeq(targetGuid[1]);
    recvData.ReadByteSeq(targetGuid[2]);
    recvData.ReadByteSeq(targetGuid[6]);
    recvData.ReadByteSeq(targetGuid[0]);

    if (GetPlayer()->m_TradeData)
    {
        sendTradeResult(TRADE_STATUS_BUSY);
        return;
    }

    Player* player_target = _player->GetMapMgr()->GetPlayer(static_cast<uint32_t>(targetGuid));
    if (player_target == nullptr)
    {
        sendTradeResult(TRADE_STATUS_NO_TARGET);
        return;
    }

    if (player_target == GetPlayer() || player_target->m_TradeData)
    {
        sendTradeResult(TRADE_STATUS_BUSY);
        return;
    }

    if (player_target->CalcDistance(_player) > 10.0f)
    {
        sendTradeResult(TRADE_STATUS_TARGET_TO_FAR);
        return;
    }

    if (GetPlayer()->IsDead())
    {
        sendTradeResult(TRADE_STATUS_YOU_DEAD);
        return;
    }

    if (IsLoggingOut())
    {
        sendTradeResult(TRADE_STATUS_YOU_LOGOUT);
        return;
    }

    if (player_target->GetSession()->IsLoggingOut())
    {
        sendTradeResult(TRADE_STATUS_TARGET_LOGOUT);
        return;
    }

    if (player_target->IsDead())
    {
        sendTradeResult(TRADE_STATUS_TARGET_DEAD);
        return;
    }

    if (player_target->GetTeam() != _player->GetTeam() && GetPermissionCount() == 0 && !sWorld.settings.player.isInterfactionTradeEnabled)
    {
        sendTradeResult(TRADE_STATUS_WRONG_FACTION);
        return;
    }

    _player->m_TradeData = new TradeData(_player, player_target);
    player_target->m_TradeData = new TradeData(player_target, _player);

    WorldPacket data(SMSG_TRADE_STATUS, 12);
    data.writeBit(false);
    data.writeBits(TRADE_STATUS_BEGIN_TRADE, 5);

    ObjectGuid source_guid = _player->GetGUID();
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

    data << uint32_t(0);              // unk

    player_target->GetSession()->SendPacket(&data);
}

void WorldSession::HandleBeginTradeOpcode(WorldPacket& /*recvData*/)
{
    TradeData* trade_data = _player->m_TradeData;
    if (trade_data == nullptr)
        return;

    trade_data->getTradeTarget()->GetSession()->sendTradeResult(TRADE_STATUS_OPEN_WINDOW);
    sendTradeResult(TRADE_STATUS_OPEN_WINDOW);
}

void WorldSession::HandleSetTradeGold(WorldPacket& recvData)
{
    uint64_t gold;
    recvData >> gold;

    TradeData* trade_data = _player->getTradeData();
    if (trade_data == nullptr)
        return;

    trade_data->setMoney(gold);
}

void TradeData::setMoney(uint64_t money)
{
    if (m_money == money)
        return;

    if (money > m_player->GetGold())
    {
        m_player->GetSession()->sendTradeResult(TRADE_STATUS_CLOSE_WINDOW);
        return;
    }

    m_money = money;

    setAccepted(false);
    getTargetTradeData()->setAccepted(false);

    updateTrade();
}

void TradeData::updateTrade(bool forTrader /*= true*/)
{
    if (forTrader)
        m_tradeTarget->GetSession()->sendTradeUpdate(true);
    else
        m_player->GetSession()->sendTradeUpdate(false);
}

void TradeData::setAccepted(bool state, bool sendBoth /*= false*/)
{
    m_accepted = state;

    if (!state)
    {
        if (sendBoth)
            m_tradeTarget->GetSession()->sendTradeResult(TRADE_STATUS_BACK_TO_TRADE);
        else
            m_player->GetSession()->sendTradeResult(TRADE_STATUS_BACK_TO_TRADE);
    }
}

static void setAcceptTradeMode(TradeData* myTrade, TradeData* hisTrade, Item** myItems, Item** hisItems)
{
    myTrade->setInAcceptProcess(true);
    hisTrade->setInAcceptProcess(true);

    for (uint8_t i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        if (Item* item = myTrade->getTradeItem(TradeSlots(i)))
        {
            LOG_DEBUG("player trade Item %s", item->GetItemProperties()->Name.c_str());
            myItems[i] = item;
            myItems[i]->setIsInTrade();
        }

        if (Item* item = hisTrade->getTradeItem(TradeSlots(i)))
        {
            LOG_DEBUG("partner trade Item %s", item->GetItemProperties()->Name.c_str());
            hisItems[i] = item;
            hisItems[i]->setIsInTrade();
        }
    }
}

static void clearAcceptTradeMode(TradeData* myTrade, TradeData* hisTrade)
{
    myTrade->setInAcceptProcess(false);
    hisTrade->setInAcceptProcess(false);
}

static void clearAcceptTradeMode(Item** myItems, Item** hisItems)
{
    for (uint8_t i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        if (myItems[i] != nullptr)
            myItems[i]->setIsInTrade(false);

        if (hisItems[i] != nullptr)
            hisItems[i]->setIsInTrade(false);
    }
}

void WorldSession::HandleAcceptTrade(WorldPacket& recvData)
{
    recvData.read_skip<uint32_t>();

    TradeData* trade_data = _player->m_TradeData;
    if (trade_data == nullptr)
        return;

    Player* trade_target = trade_data->getTradeTarget();
    if (trade_target == nullptr)
        return;

    TradeData* target_trade_data = trade_target->m_TradeData;
    if (target_trade_data == nullptr)
        return;

    Item* trade_items[TRADE_SLOT_TRADED_COUNT];
    Item* target_trade_items[TRADE_SLOT_TRADED_COUNT];

    for (uint8_t i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        trade_items[i] = nullptr;
        target_trade_items[i] = nullptr;
    }

    // bool myCanCompleteTrade = true;
    // bool hisCanCompleteTrade = true;

    trade_data->setAccepted(true);

    if (trade_data->getMoney() > _player->GetGold())
    {
        trade_data->setAccepted(false, true);
        return;
    }

    if (target_trade_data->getMoney() > trade_target->GetGold())
    {
        target_trade_data->setAccepted(false, true);
        return;
    }

    for (int i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        if (Item* item = trade_data->getTradeItem(TradeSlots(i)))
        {
            if (item->IsContainer() && static_cast< Container* >(item)->HasItems() || (item->GetItemProperties()->Bonding == ITEM_BIND_ON_PICKUP))
            {
                sendTradeResult(TRADE_STATUS_TRADE_CANCELED);
                return;
            }
        }

        if (Item* item = target_trade_data->getTradeItem(TradeSlots(i)))
        {
            if (item->IsContainer() && static_cast<Container*>(item)->HasItems() || (item->GetItemProperties()->Bonding == ITEM_BIND_ON_PICKUP))
            {
                sendTradeResult(TRADE_STATUS_TRADE_CANCELED);
                return;
            }
        }
    }

    if (target_trade_data->isAccepted())
    {
        setAcceptTradeMode(trade_data, target_trade_data, trade_items, target_trade_items);

        trade_target->GetSession()->sendTradeResult(TRADE_STATUS_TRADE_ACCEPT);

        clearAcceptTradeMode(trade_items, target_trade_items);

        // Remove all items
        for (uint8_t i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
        {
            if (trade_items[i])
            {
                trade_items[i]->setUInt64Value(ITEM_FIELD_GIFTCREATOR, _player->GetGUID());
                _player->m_ItemInterface->SafeRemoveAndRetreiveItemByGuid(trade_items[i]->GetGUID(), true);
            }
            if (target_trade_items[i])
            {
                target_trade_items[i]->setUInt64Value(ITEM_FIELD_GIFTCREATOR, trade_target->GetGUID());
                trade_target->m_ItemInterface->SafeRemoveAndRetreiveItemByGuid(target_trade_items[i]->GetGUID(), true);
            }
        }

        // Add all items
        for (uint8_t i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
        {
            if (trade_items[i] != nullptr)
            {
                trade_items[i]->SetOwner(trade_target);
                if (!trade_target->m_ItemInterface->AddItemToFreeSlot(trade_items[i]))
                    trade_items[i]->DeleteMe();
            }
            if (target_trade_items[i] != nullptr)
            {
                target_trade_items[i]->SetOwner(_player);
                if (!_player->m_ItemInterface->AddItemToFreeSlot(target_trade_items[i]))
                    target_trade_items[i]->DeleteMe();
            }
        }

        // Trade Gold
        if (target_trade_data->getMoney())
        {
            if (sWorld.settings.player.isGoldCapEnabled && (_player->GetGold() + target_trade_data->getMoney()) > sWorld.settings.player.limitGoldAmount)
            {
                _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            }
            else
            {
                _player->ModGold(static_cast<int32_t>(target_trade_data->getMoney()));
                trade_target->ModGold(-(int32_t)target_trade_data->getMoney());
            }
        }

        if (trade_data->getMoney())
        {
            // Check they don't have more than the max gold
            if (sWorld.settings.player.isGoldCapEnabled && (trade_target->GetGold() + trade_data->getMoney()) > sWorld.settings.player.limitGoldAmount)
            {
                trade_target->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            }
            else
            {
                trade_target->ModGold(static_cast<int32_t>(trade_data->getMoney()));
                _player->ModGold(-static_cast<int32_t>(trade_data->getMoney()));
            }
        }

        clearAcceptTradeMode(trade_data, target_trade_data);

        delete _player->m_TradeData;
        _player->m_TradeData = nullptr;

        delete trade_target->m_TradeData;
        trade_target->m_TradeData = nullptr;

        trade_target->GetSession()->sendTradeResult(TRADE_STATUS_TRADE_COMPLETE);
        sendTradeResult(TRADE_STATUS_TRADE_COMPLETE);

        trade_target->SaveToDB(false);
        _player->SaveToDB(false);
    }
    else
    {
        trade_target->GetSession()->sendTradeResult(TRADE_STATUS_TRADE_ACCEPT);
    }
}

void WorldSession::HandleCancelTrade(WorldPacket& /*recvData*/)
{
    if (_player != nullptr)
        _player->cancelTrade(true);
}

void WorldSession::HandleSetTradeItem(WorldPacket& recvData)
{
    uint8_t tradeSlot;
    uint8_t sourceBag;
    uint8_t sourceSlot;

    recvData >> sourceSlot;
    recvData >> tradeSlot;
    recvData >> sourceBag;

    TradeData* tradeData = _player->m_TradeData;
    if (tradeData == nullptr)
        return;

    if (tradeSlot >= TRADE_SLOT_COUNT)
    {
        sendTradeResult(TRADE_STATUS_TRADE_CANCELED);
        return;
    }

    Item* item = _player->GetItemInterface()->GetInventoryItem(sourceBag, sourceSlot);
    if (item == nullptr || (tradeSlot != TRADE_SLOT_NONTRADED && (item->IsAccountbound() || item->IsSoulbound())))
    {
        sendTradeResult(TRADE_STATUS_TRADE_CANCELED);
        return;
    }

    if (tradeData->hasTradeItem(item->GetGUID()))
    {
        sendTradeResult(TRADE_STATUS_TRADE_CANCELED);
        return;
    }

    tradeData->setItem(TradeSlots(tradeSlot), item);
}

void WorldSession::sendTradeCancel()
{
    sendTradeResult(TRADE_STATUS_TRADE_CANCELED);
}

void WorldSession::HandleClearTradeItem(WorldPacket& recvData)
{
    uint8_t tradeSlot;
    recvData >> tradeSlot;

    TradeData* trade_data = _player->m_TradeData;
    if (trade_data == nullptr)
        return;

    if (tradeSlot >= TRADE_SLOT_COUNT)
        return;

    trade_data->setItem(TradeSlots(tradeSlot), nullptr);
}
