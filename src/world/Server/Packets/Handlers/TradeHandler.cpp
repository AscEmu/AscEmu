/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgInitiateTrade.h"
#include "Server/Packets/SmsgTradeStatus.h"
#include "Server/Packets/CmsgSetTradeItem.h"
#include "Server/Packets/CmsgSetTradeGold.h"
#include "Server/WorldSession.h"
#include "Units/Players/PlayerDefines.hpp"
#include "Server/World.h"
#include "Units/Players/Player.h"
#include "Map/MapMgr.h"
#include "Objects/ObjectMgr.h"
#include "Server/MainServerDefines.h"
#include "Management/Container.h"
#include "Management/ItemInterface.h"
#include "Server/Packets/CmsgClearTradeItem.h"


using namespace AscEmu::Packets;

#if VERSION_STRING < Cata
void WorldSession::handleInitiateTradeOpcode(WorldPacket& recvPacket)
{
    CmsgInitiateTrade srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto playerTarget = _player->GetMapMgr()->GetPlayer(srlPacket.guid.getGuidLow());
    if (playerTarget == nullptr)
    {
        SendPacket(SmsgTradeStatus(TRADE_STATUS_PLAYER_NOT_FOUND, 0).serialise().get());
        return;
    }

    if (playerTarget->CalcDistance(_player) > 10.0f)
    {
        playerTarget->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_TOO_FAR_AWAY, 0).serialise().get());
        return;
    }

    if (playerTarget->isDead())
    {
        playerTarget->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_DEAD, 0).serialise().get());
        return;
    }

    if (playerTarget->mTradeTarget != 0)
    {
        playerTarget->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_ALREADY_TRADING, 0).serialise().get());
        return;
    }

    if (playerTarget->getTeam() != _player->getTeam() && GetPermissionCount() == 0 && !worldConfig.player.isInterfactionTradeEnabled)
    {
        playerTarget->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_WRONG_FACTION, 0).serialise().get());
        return;
    }

    _player->ResetTradeVariables();
    playerTarget->ResetTradeVariables();

    playerTarget->mTradeTarget = _player->getGuidLow();
    playerTarget->mTradeStatus = TRADE_STATUS_PROPOSED;

    _player->mTradeTarget = playerTarget->getGuidLow();
    _player->mTradeStatus = TRADE_STATUS_PROPOSED;

    playerTarget->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_PROPOSED, _player->getGuid()).serialise().get());
}

void WorldSession::handleBeginTradeOpcode(WorldPacket& /*recvPacket*/)
{
    const auto playerTarget = _player->GetTradeTarget();
    if (_player->mTradeTarget == 0 || playerTarget == nullptr)
    {
        SendPacket(SmsgTradeStatus(TRADE_STATUS_PLAYER_NOT_FOUND, 0).serialise().get());
        return;
    }

    if (_player->CalcDistance(objmgr.GetPlayer(_player->mTradeTarget)) > 10.0f)
    {
        SendPacket(SmsgTradeStatus(TRADE_STATUS_TOO_FAR_AWAY, 0).serialise().get());
        return;
    }

    SendPacket(SmsgTradeStatus(TRADE_STATUS_INITIATED, 0).serialise().get());
    playerTarget->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_INITIATED, 0).serialise().get());

    playerTarget->mTradeStatus = TRADE_STATUS_INITIATED;
    _player->mTradeStatus = TRADE_STATUS_INITIATED;
}

void WorldSession::handleBusyTrade(WorldPacket& /*recvPacket*/)
{
    const auto playerTarget = _player->GetTradeTarget();
    if (_player->mTradeTarget == 0 || playerTarget == nullptr)
    {
        SendPacket(SmsgTradeStatus(TRADE_STATUS_PLAYER_NOT_FOUND, 0).serialise().get());
        return;
    }

    SendPacket(SmsgTradeStatus(TRADE_STATUS_PLAYER_BUSY, 0).serialise().get());
    playerTarget->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_PLAYER_BUSY, 0).serialise().get());

    playerTarget->mTradeStatus = TRADE_STATUS_PLAYER_BUSY;
    _player->mTradeStatus = TRADE_STATUS_PLAYER_BUSY;

    playerTarget->mTradeTarget = 0;
    _player->mTradeTarget = 0;
}

void WorldSession::handleIgnoreTrade(WorldPacket& /*recvPacket*/)
{
    const auto playerTarget = _player->GetTradeTarget();
    if (_player->mTradeTarget == 0 || playerTarget == nullptr)
    {
        SendPacket(SmsgTradeStatus(TRADE_STATUS_PLAYER_NOT_FOUND, 0).serialise().get());
        return;
    }

    SendPacket(SmsgTradeStatus(TRADE_STATUS_PLAYER_IGNORED, 0).serialise().get());
    playerTarget->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_PLAYER_IGNORED, 0).serialise().get());

    playerTarget->mTradeStatus = TRADE_STATUS_PLAYER_IGNORED;
    _player->mTradeStatus = TRADE_STATUS_PLAYER_IGNORED;

    playerTarget->mTradeTarget = 0;
    _player->mTradeTarget = 0;
}

void WorldSession::handleCancelTrade(WorldPacket& /*recvPacket*/)
{
    if (_player->mTradeTarget == 0 || _player->mTradeStatus == TRADE_STATUS_COMPLETE)
        return;

    SendPacket(SmsgTradeStatus(TRADE_STATUS_CANCELLED, 0).serialise().get());

    const auto playerTarget = _player->GetTradeTarget();
    if (playerTarget != nullptr)
    {
        if (playerTarget->m_session && playerTarget->m_session->GetSocket())
            playerTarget->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_CANCELLED, 0).serialise().get());

        playerTarget->ResetTradeVariables();
    }

    _player->ResetTradeVariables();
}

void WorldSession::handleUnacceptTrade(WorldPacket& /*recvPacket*/)
{
    const auto playerTarget = _player->GetTradeTarget();
    if (_player->mTradeTarget == 0 || playerTarget == nullptr)
        return;

    SendPacket(SmsgTradeStatus(TRADE_STATUS_UNACCEPTED, 0).serialise().get());
    playerTarget->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_UNACCEPTED, 0).serialise().get());

    SendPacket(SmsgTradeStatus(TRADE_STATUS_STATE_CHANGED, 0).serialise().get());
    playerTarget->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_STATE_CHANGED, 0).serialise().get());

    playerTarget->mTradeStatus = TRADE_STATUS_STATE_CHANGED;
    _player->mTradeStatus = TRADE_STATUS_STATE_CHANGED;
}

void WorldSession::handleSetTradeItem(WorldPacket& recvPacket)
{
    if (_player->mTradeTarget == 0)
        return;

    CmsgSetTradeItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.tradeSlot > 6)
        return;

    const auto playerTarget = _player->GetTradeTarget();
    if (playerTarget == nullptr)
        return;

    const auto tradeItem = _player->getItemInterface()->GetInventoryItem(srlPacket.sourceBag, srlPacket.sourceSlot);
    if (tradeItem == nullptr)
        return;

    if (srlPacket.tradeSlot < 6)
    {
        if (tradeItem->isAccountbound())
            return;

        if (tradeItem->isSoulbound())
        {
            sCheatLog.writefromsession(this, "tried to cheat trade a soulbound item");
            Disconnect();
            return;
        }
    }

    SendPacket(SmsgTradeStatus(TRADE_STATUS_STATE_CHANGED, 0).serialise().get());
    playerTarget->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_STATE_CHANGED, 0).serialise().get());

    playerTarget->mTradeStatus = TRADE_STATUS_STATE_CHANGED;
    _player->mTradeStatus = TRADE_STATUS_STATE_CHANGED;

    if (tradeItem->isContainer())
    {
        if (dynamic_cast<Container*>(tradeItem)->HasItems())
        {
            _player->getItemInterface()->BuildInventoryChangeError(tradeItem, nullptr, INV_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS);

            SendPacket(SmsgTradeStatus(TRADE_STATUS_CANCELLED, 0).serialise().get());
            _player->ResetTradeVariables();

            playerTarget->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_CANCELLED, 0).serialise().get());
            playerTarget->ResetTradeVariables();

            return;
        }
    }

    for (uint8_t i = 0; i < 8; ++i)
    {
        if (_player->mTradeItems[i] == tradeItem || playerTarget->mTradeItems[i] == tradeItem)
        {
            sCheatLog.writefromsession(this, "tried to dupe an item through trade");
            Disconnect();
            return;
        }
    }

    if (srlPacket.sourceSlot >= INVENTORY_SLOT_BAG_START && srlPacket.sourceSlot < INVENTORY_SLOT_BAG_END)
    {
        const auto item = _player->getItemInterface()->GetInventoryItem(srlPacket.sourceBag);
        if (item == nullptr || srlPacket.sourceSlot >= item->getItemProperties()->ContainerSlots)
        {
            sCheatLog.writefromsession(this, "tried to cheat trade a soulbound item");
            Disconnect();
        }
    }

    _player->mTradeItems[srlPacket.tradeSlot] = tradeItem;
    _player->SendTradeUpdate();
}

void WorldSession::handleSetTradeGold(WorldPacket& recvPacket)
{
    const auto targetPlayer = _player->GetTradeTarget();
    if (targetPlayer == nullptr)
        return;

    CmsgSetTradeGold srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    SendPacket(SmsgTradeStatus(TRADE_STATUS_STATE_CHANGED, 0).serialise().get());
    targetPlayer->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_STATE_CHANGED, 0).serialise().get());

    targetPlayer->mTradeStatus = TRADE_STATUS_STATE_CHANGED;
    _player->mTradeStatus = TRADE_STATUS_STATE_CHANGED;

    if (_player->mTradeGold != srlPacket.tradeGoldAmount)
    {
        _player->mTradeGold = srlPacket.tradeGoldAmount > _player->getCoinage() ? _player->getCoinage() : srlPacket.tradeGoldAmount;
        _player->SendTradeUpdate();
    }
}

void WorldSession::handleClearTradeItem(WorldPacket& recvPacket)
{
    const auto targetPlayer = _player->GetTradeTarget();
    if (targetPlayer == nullptr)
        return;

    CmsgClearTradeItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.tradeSlot > 6)
        return;

    SendPacket(SmsgTradeStatus(TRADE_STATUS_STATE_CHANGED, 0).serialise().get());
    targetPlayer->m_session->SendPacket(SmsgTradeStatus(TRADE_STATUS_STATE_CHANGED, 0).serialise().get());

    targetPlayer->mTradeStatus = TRADE_STATUS_STATE_CHANGED;
    _player->mTradeStatus = TRADE_STATUS_STATE_CHANGED;

    _player->mTradeItems[srlPacket.tradeSlot] = nullptr;
    _player->SendTradeUpdate();
}

void WorldSession::handleAcceptTrade(WorldPacket& /*recvPacket*/)
{
    const auto targetPlayer = _player->GetTradeTarget();
    if (targetPlayer == nullptr)
        return;

    uint32_t tradeStatus = TRADE_STATUS_ACCEPTED;

    targetPlayer->m_session->SendPacket(SmsgTradeStatus(tradeStatus, 0).serialise().get());
    _player->mTradeStatus = tradeStatus;

    if (targetPlayer->mTradeStatus == TRADE_STATUS_ACCEPTED)
    {
        uint32_t itemCount = 0;
        uint32_t targetItemCount = 0;

        for (uint32_t index = 0; index < 6; ++index)
        {
            auto tradeItem = _player->mTradeItems[index];
            if (tradeItem)
            {
                if ((tradeItem->isContainer() && dynamic_cast< Container* >(tradeItem)->HasItems()) || tradeItem->getItemProperties()->Bonding == ITEM_BIND_ON_PICKUP)
                {
                    itemCount = 0;
                    targetItemCount = 0;
                    break;
                }

                ++itemCount;
            }

            tradeItem = targetPlayer->mTradeItems[index];
            if (tradeItem)
            {
                if ((tradeItem->isContainer() && dynamic_cast< Container* >(tradeItem)->HasItems()) || tradeItem->getItemProperties()->Bonding == ITEM_BIND_ON_PICKUP)
                {
                    itemCount = 0;
                    targetItemCount = 0;
                    break;
                }

                ++targetItemCount;
            }

        }

        if (_player->getItemInterface()->CalculateFreeSlots(nullptr) + itemCount < targetItemCount ||
            targetPlayer->getItemInterface()->CalculateFreeSlots(nullptr) + targetItemCount < itemCount ||
            itemCount == 0 && targetItemCount == 0 && !targetPlayer->mTradeGold && !_player->mTradeGold)
        {
            tradeStatus = TRADE_STATUS_CANCELLED;
        }
        else
        {
            for (uint32_t index = 0; index < 6; ++index)
            {
                uint64_t tradeItemGuid = _player->mTradeItems[index] ? _player->mTradeItems[index]->getGuid() : 0;
                if (tradeItemGuid != 0)
                {
                    if (_player->mTradeItems[index]->getItemProperties()->Bonding == ITEM_BIND_ON_PICKUP ||
                        _player->mTradeItems[index]->getItemProperties()->Bonding >= ITEM_BIND_QUEST)
                    {
                        _player->mTradeItems[index] = nullptr;
                    }
                    else
                    {
                        if (GetPermissionCount() > 0)
                        {
                            sGMLog.writefromsession(this, "traded item %s to %s", _player->mTradeItems[index]->getItemProperties()->Name.c_str(), targetPlayer->getName().c_str());
                        }
                    }
                }

                tradeItemGuid = targetPlayer->mTradeItems[index] ? targetPlayer->mTradeItems[index]->getGuid() : 0;
                if (tradeItemGuid != 0)
                {
                    if (targetPlayer->mTradeItems[index]->getItemProperties()->Bonding == ITEM_BIND_ON_PICKUP ||
                        targetPlayer->mTradeItems[index]->getItemProperties()->Bonding >= ITEM_BIND_QUEST)
                    {
                        targetPlayer->mTradeItems[index] = nullptr;
                    }
                    else
                    {
                        targetPlayer->getItemInterface()->SafeRemoveAndRetreiveItemByGuid(tradeItemGuid, true);
                    }
                }
            }

            for (uint32_t index = 0; index < 6; ++index)
            {
                auto tradeItem = _player->mTradeItems[index];
                if (tradeItem != nullptr)
                {
                    tradeItem->setOwner(targetPlayer);
                    if (!targetPlayer->getItemInterface()->AddItemToFreeSlot(tradeItem))
                        tradeItem->DeleteMe();
                }

                tradeItem = targetPlayer->mTradeItems[index];
                if (tradeItem != nullptr)
                {
                    tradeItem->setOwner(_player);
                    if (!_player->getItemInterface()->AddItemToFreeSlot(tradeItem))
                        tradeItem->DeleteMe();
                }
            }

            if (targetPlayer->mTradeGold)
            {
                if (worldConfig.player.isGoldCapEnabled && (_player->getCoinage() + targetPlayer->mTradeGold) > worldConfig.player.limitGoldAmount)
                {
                    _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
                }
                else
                {
                    _player->modCoinage(targetPlayer->mTradeGold);
                    targetPlayer->modCoinage(-static_cast<int32_t>(targetPlayer->mTradeGold));
                }
            }

            if (_player->mTradeGold)
            {
                if (worldConfig.player.isGoldCapEnabled && (targetPlayer->getCoinage() + _player->mTradeGold) > worldConfig.player.limitGoldAmount)
                {
                    targetPlayer->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
                }
                else
                {
                    targetPlayer->modCoinage(_player->mTradeGold);
                    _player->modCoinage(-static_cast<int32_t>(_player->mTradeGold));
                }
            }

            tradeStatus = TRADE_STATUS_COMPLETE;

        }

        SendPacket(SmsgTradeStatus(tradeStatus, 0).serialise().get());
        targetPlayer->m_session->SendPacket(SmsgTradeStatus(tradeStatus, 0).serialise().get());

        _player->mTradeStatus = tradeStatus;
        targetPlayer->mTradeStatus = tradeStatus;

        _player->ResetTradeVariables();
        targetPlayer->ResetTradeVariables();

        _player->SaveToDB(false);
        targetPlayer->SaveToDB(false);
    }
}
#else
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
            ObjectGuid creatorGuid = item->getCreatorGuid();
            ObjectGuid giftCreatorGuid = item->getGiftCreatorGuid();

            data.writeBit(giftCreatorGuid[7]);
            data.writeBit(giftCreatorGuid[1]);
            bool notWrapped = data.writeBit(!item->hasFlags(ITEM_FLAG_WRAPPED));     //wrapped
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
                data.writeBit(item->getItemProperties()->LockId != 0);
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
            ObjectGuid creatorGuid = item->getCreatorGuid();
            ObjectGuid giftCreatorGuid = item->getGiftCreatorGuid();

            if (!item->hasFlags(ITEM_FLAG_WRAPPED))
            {
                data.WriteByteSeq(creatorGuid[1]);

                data << uint32_t(item->getEnchantmentId(PERM_ENCHANTMENT_SLOT));
                for (uint32_t enchant_slot = 2; enchant_slot < 5; ++enchant_slot)
                {
                    data << uint32_t(item->getEnchantmentId(static_cast<uint8_t>(EnchantmentSlot(enchant_slot))));
                }

                data << uint32_t(item->getMaxDurability());

                data.WriteByteSeq(creatorGuid[6]);
                data.WriteByteSeq(creatorGuid[2]);
                data.WriteByteSeq(creatorGuid[7]);
                data.WriteByteSeq(creatorGuid[4]);

                data << uint32_t(item->getEnchantmentId(REFORGE_ENCHANTMENT_SLOT));
                data << uint32_t(item->getDurability());
                data << uint32_t(item->getRandomPropertiesId());

                data.WriteByteSeq(creatorGuid[3]);

                data << uint32_t(0);                      // unk

                data.WriteByteSeq(creatorGuid[0]);

                data << uint32_t(item->getSpellCharges(0));
                data << uint32_t(item->getPropertySeed());

                data.WriteByteSeq(creatorGuid[5]);
            }

            data.WriteByteSeq(giftCreatorGuid[6]);
            data.WriteByteSeq(giftCreatorGuid[1]);
            data.WriteByteSeq(giftCreatorGuid[7]);
            data.WriteByteSeq(giftCreatorGuid[4]);

            data << uint32_t(item->getItemProperties()->ItemId);

            data.WriteByteSeq(giftCreatorGuid[0]);

            data << uint32_t(item->getStackCount());

            data.WriteByteSeq(giftCreatorGuid[5]);

            data << uint8_t(i);                           // slot

            data.WriteByteSeq(giftCreatorGuid[2]);
            data.WriteByteSeq(giftCreatorGuid[3]);
        }
    }

    SendPacket(&data);
}

void WorldSession::handleInitiateTradeOpcode(WorldPacket& recvData)
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

    if (_player->m_TradeData)
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

    if (player_target == _player || player_target->m_TradeData)
    {
        sendTradeResult(TRADE_STATUS_BUSY);
        return;
    }

    if (player_target->CalcDistance(_player) > 10.0f)
    {
        sendTradeResult(TRADE_STATUS_TARGET_TO_FAR);
        return;
    }

    if (_player->isDead())
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

    if (player_target->isDead())
    {
        sendTradeResult(TRADE_STATUS_TARGET_DEAD);
        return;
    }

    if (player_target->getTeam() != _player->getTeam() && GetPermissionCount() == 0 && !sWorld.settings.player.isInterfactionTradeEnabled)
    {
        sendTradeResult(TRADE_STATUS_WRONG_FACTION);
        return;
    }

    _player->m_TradeData = new TradeData(_player, player_target);
    player_target->m_TradeData = new TradeData(player_target, _player);

    WorldPacket data(SMSG_TRADE_STATUS, 12);
    data.writeBit(false);
    data.writeBits(TRADE_STATUS_BEGIN_TRADE, 5);

    ObjectGuid source_guid = _player->getGuid();
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

void WorldSession::handleBeginTradeOpcode(WorldPacket& /*recvData*/)
{
    TradeData* trade_data = _player->m_TradeData;
    if (trade_data == nullptr)
        return;

    trade_data->getTradeTarget()->GetSession()->sendTradeResult(TRADE_STATUS_OPEN_WINDOW);
    sendTradeResult(TRADE_STATUS_OPEN_WINDOW);
}

void WorldSession::handleSetTradeGold(WorldPacket& recvData)
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

    if (money > m_player->getCoinage())
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
            LOG_DEBUG("player trade Item %s", item->getItemProperties()->Name.c_str());
            myItems[i] = item;
            myItems[i]->setIsInTrade();
        }

        if (Item* item = hisTrade->getTradeItem(TradeSlots(i)))
        {
            LOG_DEBUG("partner trade Item %s", item->getItemProperties()->Name.c_str());
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

void WorldSession::handleAcceptTrade(WorldPacket& recvData)
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

    if (trade_data->getMoney() > _player->getCoinage())
    {
        trade_data->setAccepted(false, true);
        return;
    }

    if (target_trade_data->getMoney() > trade_target->getCoinage())
    {
        target_trade_data->setAccepted(false, true);
        return;
    }

    for (int i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        if (Item* item = trade_data->getTradeItem(TradeSlots(i)))
        {
            if (item->isContainer() && static_cast< Container* >(item)->HasItems() || (item->getItemProperties()->Bonding == ITEM_BIND_ON_PICKUP))
            {
                sendTradeResult(TRADE_STATUS_TRADE_CANCELED);
                return;
            }
        }

        if (Item* item = target_trade_data->getTradeItem(TradeSlots(i)))
        {
            if (item->isContainer() && static_cast<Container*>(item)->HasItems() || (item->getItemProperties()->Bonding == ITEM_BIND_ON_PICKUP))
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
                trade_items[i]->setCreatorGuid(_player->getGuid());
                _player->getItemInterface()->SafeRemoveAndRetreiveItemByGuid(trade_items[i]->getGuid(), true);
            }
            if (target_trade_items[i])
            {
                target_trade_items[i]->setCreatorGuid(trade_target->getGuid());
                trade_target->getItemInterface()->SafeRemoveAndRetreiveItemByGuid(target_trade_items[i]->getGuid(), true);
            }
        }

        // Add all items
        for (uint8_t i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
        {
            if (trade_items[i] != nullptr)
            {
                trade_items[i]->setOwner(trade_target);
                if (!trade_target->getItemInterface()->AddItemToFreeSlot(trade_items[i]))
                    trade_items[i]->DeleteMe();
            }
            if (target_trade_items[i] != nullptr)
            {
                target_trade_items[i]->setOwner(_player);
                if (!_player->getItemInterface()->AddItemToFreeSlot(target_trade_items[i]))
                    target_trade_items[i]->DeleteMe();
            }
        }

        // Trade Gold
        if (target_trade_data->getMoney())
        {
            if (sWorld.settings.player.isGoldCapEnabled && (_player->getCoinage() + target_trade_data->getMoney()) > sWorld.settings.player.limitGoldAmount)
            {
                _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            }
            else
            {
                _player->modCoinage(static_cast<int32_t>(target_trade_data->getMoney()));
                trade_target->modCoinage(-(int32_t)target_trade_data->getMoney());
            }
        }

        if (trade_data->getMoney())
        {
            // Check they don't have more than the max gold
            if (sWorld.settings.player.isGoldCapEnabled && (trade_target->getCoinage() + trade_data->getMoney()) > sWorld.settings.player.limitGoldAmount)
            {
                trade_target->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            }
            else
            {
                trade_target->modCoinage(static_cast<int32_t>(trade_data->getMoney()));
                _player->modCoinage(-static_cast<int32_t>(trade_data->getMoney()));
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

void WorldSession::handleCancelTrade(WorldPacket& /*recvData*/)
{
    if (_player != nullptr)
        _player->cancelTrade(true);
}

void WorldSession::handleSetTradeItem(WorldPacket& recvData)
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

    Item* item = _player->getItemInterface()->GetInventoryItem(sourceBag, sourceSlot);
    if (item == nullptr || (tradeSlot != TRADE_SLOT_NONTRADED && (item->isAccountbound() || item->isSoulbound())))
    {
        sendTradeResult(TRADE_STATUS_TRADE_CANCELED);
        return;
    }

    if (tradeData->hasTradeItem(item->getGuid()))
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

void WorldSession::handleClearTradeItem(WorldPacket& recvData)
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
#endif
