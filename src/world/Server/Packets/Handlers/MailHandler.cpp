/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/SmsgSendMailResult.h"
#include "Server/Packets/CmsgMailMarkAsRead.h"
#include "Server/Packets/CmsgMailDelete.h"
#include "Server/Packets/CmsgMailTakeMoney.h"
#include "Server/Packets/CmsgMailCreateTextItem.h"
#include "Server/Packets/CmsgMailReturnToSender.h"
#include "Server/Packets/CmsgItemTextQuery.h"
#include "Server/Packets/SmsgItemTextQueryResponse.h"
#include "Server/Packets/CmsgSendMail.h"
#include "Server/Packets/CmsgMailTakeItem.h"
#include "Server/WorldSession.h"
#include "Management/MailMgr.h"
#include "Server/World.h"
#include "Units/Players/Player.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleMarkAsReadOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgMailMarkAsRead srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    auto mailMessage = _player->m_mailBox.GetMessage(srlPacket.messageId);
    if (mailMessage == nullptr)
        return;

    mailMessage->checked_flag |= MAIL_CHECK_MASK_READ;

    if (!sMailSystem.MailOption(MAIL_FLAG_NO_EXPIRY))
        mailMessage->expire_time = static_cast<uint32_t>(UNIXTIME) + (TimeVars::Day * 30);

    CharacterDatabase.WaitExecute("UPDATE mailbox SET checked_flag = %u, expiry_time = %u WHERE message_id = %u",
        mailMessage->checked_flag, mailMessage->expire_time, mailMessage->message_id);
}

void WorldSession::handleMailDeleteOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgMailDelete srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto mailMessage = _player->m_mailBox.GetMessage(srlPacket.messageId);
    if (mailMessage == nullptr)
    {
        SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_DELETED, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    _player->m_mailBox.DeleteMessage(srlPacket.messageId, true);

    SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_DELETED, MAIL_OK).serialise().get());
}

void WorldSession::handleTakeMoneyOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgMailTakeMoney srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto mailMessage = _player->m_mailBox.GetMessage(srlPacket.messageId);
    if (mailMessage == nullptr || !mailMessage->money)
    {
        SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_MONEY_TAKEN, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    if (worldConfig.player.isGoldCapEnabled)
    {
        if (_player->getCoinage() + mailMessage->money > worldConfig.player.limitGoldAmount)
        {
            _player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            return;
        }
    }

    _player->modCoinage(mailMessage->money);
    mailMessage->money = 0;

    CharacterDatabase.WaitExecute("UPDATE mailbox SET money = 0 WHERE message_id = %u", mailMessage->message_id);

    SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_MONEY_TAKEN, MAIL_OK).serialise().get());
}

void WorldSession::handleReturnToSenderOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgMailReturnToSender srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto mailMessage = _player->m_mailBox.GetMessage(srlPacket.messageId);
    if (mailMessage == nullptr)
    {
        SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_RETURNED_TO_SENDER, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    auto message = *mailMessage;

    _player->m_mailBox.DeleteMessage(srlPacket.messageId, true);

    message.player_guid = message.sender_guid;
    message.sender_guid = _player->getGuid();

    message.deleted_flag = false;
    message.checked_flag = MAIL_CHECK_MASK_RETURNED;

    message.cod = 0;

    message.delivery_time = message.items.empty() ? static_cast<uint32_t>(UNIXTIME) : static_cast<uint32_t>(UNIXTIME) + HOUR;

    sMailSystem.DeliverMessage(message.player_guid, &message);

    SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_RETURNED_TO_SENDER, MAIL_OK).serialise().get());
}

void WorldSession::handleMailCreateTextItemOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgMailCreateTextItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto itemProperties = sMySQLStore.getItemProperties(8383);
    auto message = _player->m_mailBox.GetMessage(srlPacket.messageId);
    if (message == nullptr || !itemProperties)
    {
        SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_MADE_PERMANENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    const auto slotResult = _player->getItemInterface()->FindFreeInventorySlot(itemProperties);
    if (slotResult.Result == 0)
    {
        SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_MADE_PERMANENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    auto item = objmgr.CreateItem(8383, _player);
    if (item == nullptr)
        return;

    item->setFlags(ITEM_FLAG_WRAP_GIFT);
    item->SetText(message->body);

    if (_player->getItemInterface()->AddItemToFreeSlot(item))
        SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_MADE_PERMANENT, MAIL_OK).serialise().get());
    else
        item->DeleteMe();
}

void WorldSession::handleItemTextQueryOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgItemTextQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;
  
    if (const auto item = _player->getItemInterface()->GetItemByGUID(srlPacket.itemGuid))
        SendPacket(SmsgItemTextQueryResponse(0, srlPacket.itemGuid, item->GetText()).serialise().get());
    else
        SendPacket(SmsgItemTextQueryResponse(1, 0, "").serialise().get());
}

void WorldSession::handleMailTimeOpcode(WorldPacket& /*recvPacket*/)
{
    CHECK_INWORLD_RETURN

    WorldPacket data(MSG_QUERY_NEXT_MAIL_TIME, 32);
    {
        uint32_t unreadMessageCount = 0;
        data << uint32_t(0);
        data << uint32_t(0);

        for (auto& message : _player->m_mailBox.Messages)
        {
            if (message.second.checked_flag & MAIL_CHECK_MASK_READ)
                continue;

            if (message.second.deleted_flag == 0 && static_cast<uint32_t>(UNIXTIME) >= message.second.delivery_time)
            {
                ++unreadMessageCount;
                data << uint64_t(message.second.sender_guid);
                data << uint32_t(message.second.message_type != MAIL_TYPE_NORMAL ? message.second.sender_guid : 0);
                data << uint32_t(message.second.message_type);
                data << uint32_t(message.second.stationery);
                data << float(message.second.delivery_time - static_cast<uint32_t>(UNIXTIME));
            }
        }

        if (unreadMessageCount == 0)
            data.put<uint32_t>(0, 0xc7a8c000);
        else
            data.put<uint32_t>(4, unreadMessageCount);
    }

    SendPacket(&data);
}

void WorldSession::handleGetMailOpcode(WorldPacket& /*recvPacket*/)
{
    CHECK_INWORLD_RETURN

    WorldPacket data(SMSG_MAIL_LIST_RESULT, 200);
    uint32_t realCount = 0;
    uint8_t count = 0;

    data << uint32_t(0);
    data << uint8_t(0);

    for (auto& message : _player->m_mailBox.Messages)
    {
        if (message.second.expire_time && static_cast<uint32_t>(UNIXTIME) > message.second.expire_time)
            continue;

        if (static_cast<uint32_t>(UNIXTIME) < message.second.delivery_time)
            continue;

        if (count >= 50)
        {
            ++realCount;
            continue;
        }

        uint8_t guidSize;
        if (message.second.message_type == 0)
            guidSize = 8;
        else
            guidSize = 4;

#if VERSION_STRING < Cata
        const size_t messageSize = 2 + 4 + 1 + guidSize + 4 * 8 + (message.second.subject.size() + 1) + (message.second.body.size() + 1) + 1 + (
            message.second.items.size() * (1 + 4 + 4 + MAX_INSPECTED_ENCHANTMENT_SLOT * 3 * 4 + 4 + 4 + 4 + 4 + 4 + 4 + 1));
#else
        const size_t messageSize = 2 + 4 + 1 + guidSize + 4 * 8 + (message.second.subject.size() + 1) + (message.second.body.size() + 1) + 1 + (
            message.second.items.size() * (1 + 4 + 4 + MAX_INSPECTED_ENCHANTMENT_SLOT * 3 * 4 + 4 + 4 + 4 + 4 + 4 + 4 + 1));
#endif

        data << uint16_t(messageSize);
        data << uint32_t(message.second.message_id);
        data << uint8_t(message.second.message_type);

        switch (message.second.message_type)
        {
            case MAIL_TYPE_NORMAL:
                data << uint64_t(message.second.sender_guid);
                break;
            case MAIL_TYPE_COD:
            case MAIL_TYPE_AUCTION:
            case MAIL_TYPE_ITEM:
                data << uint32_t(WoWGuid::getGuidLowPartFromUInt64(message.second.sender_guid));
                break;
            case MAIL_TYPE_GAMEOBJECT:
            case MAIL_TYPE_CREATURE:
                data << uint32_t(static_cast<uint32_t>(message.second.sender_guid));
                break;
        }

#if VERSION_STRING < Cata
        data << uint32_t(message.second.cod);
#else
        data << uint64_t(message.second.cod);
#endif
        data << uint32_t(0);
        data << uint32_t(message.second.stationery);
#if VERSION_STRING < Cata
        data << uint32_t(message.second.money);
#else
        data << uint64_t(message.second.money);
#endif
        data << uint32_t(message.second.checked_flag);
        data << float(float((message.second.expire_time - uint32_t(UNIXTIME)) / DAY));
        data << uint32_t(0);
        data << message.second.subject;
        data << message.second.body;

        data << uint8_t(message.second.items.size());

        uint8_t i = 0;
        if (!message.second.items.empty())
        {
            for (auto itemEntry : message.second.items)
            {
                const auto item = objmgr.LoadItem(itemEntry);
                if (item == nullptr)
                    continue;

                data << uint8_t(i++);
                data << uint32_t(item->getGuidLow());
                data << uint32_t(item->getEntry());

                for (uint8_t j = 0; j < MAX_INSPECTED_ENCHANTMENT_SLOT; ++j)
                {
                    data << uint32_t(item->getEnchantmentId(j));
                    data << uint32_t(item->getEnchantmentDuration(j));
                    data << uint32_t(item->getEnchantmentCharges(j));
                }

                data << uint32_t(item->getRandomPropertiesId());
                data << uint32_t(item->getPropertySeed());
                data << uint32_t(item->getStackCount());
                data << uint32_t(item->GetChargesLeft());
                data << uint32_t(item->getMaxDurability());
                data << uint32_t(item->getDurability());
                data << uint8_t(item->locked ? 1 : 0);

                delete item;
            }
        }
        ++count;
        ++realCount;
    }

    data.put<uint32_t>(0, realCount);
    data.put<uint8_t>(4, count);

    SendPacket(&data);

    // do cleanup on request mail
    _player->m_mailBox.CleanupExpiredMessages();
}

void WorldSession::handleTakeItemOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgMailTakeItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    auto mailMessage = _player->m_mailBox.GetMessage(srlPacket.messageId);
    if (mailMessage == nullptr || mailMessage->items.empty())
    {
        SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    const auto itr = std::find(mailMessage->items.begin(), mailMessage->items.end(), srlPacket.lowGuid);
    if (itr == mailMessage->items.end())
    {
        SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    if (mailMessage->cod > 0)
    {
        if (!_player->hasEnoughCoinage(mailMessage->cod))
        {
            SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_NOT_ENOUGH_MONEY).serialise().get());
            return;
        }
    }

    auto item = objmgr.LoadItem(srlPacket.lowGuid);
    if (item == nullptr)
    {
        SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    const auto slotResult = _player->getItemInterface()->FindFreeInventorySlot(item->getItemProperties());
    if (slotResult.Result == 0)
    {
        SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_BAG_FULL, INV_ERR_INVENTORY_FULL).serialise().get());

        item->DeleteMe();
        return;
    }
    item->m_isDirty = true;

    if (!_player->getItemInterface()->SafeAddItem(item, slotResult.ContainerSlot, slotResult.Slot))
    {
        if (!_player->getItemInterface()->AddItemToFreeSlot(item))
        {
            SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_BAG_FULL, INV_ERR_INVENTORY_FULL).serialise().get());
            item->DeleteMe();
            return;
        }
    }
    else
    {
        item->SaveToDB(slotResult.ContainerSlot, slotResult.Slot, true, nullptr);
    }

    // Remove taken items and update message.
    mailMessage->items.erase(itr);
    sMailSystem.SaveMessageToSQL(mailMessage);

    SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_OK, item->getGuidLow(), item->getStackCount()).serialise().get());

    if (mailMessage->cod > 0)
    {
        _player->modCoinage(-static_cast<int32_t>(mailMessage->cod));
        std::string subject = "COD Payment: ";
        subject += mailMessage->subject;

        const uint64_t answerSender = mailMessage->player_guid;
        const uint64_t answerReceiver = mailMessage->sender_guid;
        const uint32_t answerCodMoney = mailMessage->cod;

        sMailSystem.SendAutomatedMessage(MAIL_TYPE_NORMAL, answerSender, answerReceiver, subject, "", answerCodMoney, 0, 0, MAIL_STATIONERY_TEST1, MAIL_CHECK_MASK_COD_PAYMENT);

        mailMessage->cod = 0;
        CharacterDatabase.Execute("UPDATE mailbox SET cod = 0 WHERE message_id = %u", mailMessage->message_id);
    }
}

void WorldSession::handleSendMailOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgSendMail srlPacket;
    if (!srlPacket.deserialise(recvPacket))
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    if (srlPacket.itemCount > MAIL_MAX_ITEM_SLOT)
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_TOO_MANY_ATTACHMENTS).serialise().get());
        return;
    }

    const auto playerReceiverInfo = objmgr.GetPlayerInfoByName(srlPacket.receiverName.c_str());
    if (playerReceiverInfo == nullptr)
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_RECIPIENT_NOT_FOUND).serialise().get());
        return;
    }

    std::vector<Item*> attachedItems;
    for (uint8_t i = 0; i < srlPacket.itemCount; ++i)
    {
        Item* pItem = _player->getItemInterface()->GetItemByGUID(srlPacket.itemGuid[i]);
        if (pItem == nullptr || pItem->isSoulbound() || pItem->hasFlags(ITEM_FLAG_CONJURED))
        {
            SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
            return;
        }

        if (pItem->isAccountbound() && GetAccountId() != playerReceiverInfo->acct)
        {
            SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_BAG_FULL, INV_ERR_ARTEFACTS_ONLY_FOR_OWN_CHARACTERS).serialise().get());
            return;
        }
        attachedItems.push_back(pItem);
    }

    bool isInterfactionMailAllowed = false;
    if (sMailSystem.MailOption(MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION) || (HasGMPermissions() && sMailSystem.MailOption(MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION_GM)))
    {
        isInterfactionMailAllowed = true;
    }

    if (playerReceiverInfo->team != _player->getTeam() && !isInterfactionMailAllowed)
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_NOT_YOUR_ALLIANCE).serialise().get());
        return;
    }

    if (strcmp(playerReceiverInfo->name, _player->getName().c_str()) == 0 && !GetPermissionCount())
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_CANNOT_SEND_TO_SELF).serialise().get());
        return;
    }

    if (srlPacket.stationery == MAIL_STATIONERY_GM && !HasGMPermissions())
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    // calculate cost
    uint32_t cost = 0;
    if (srlPacket.money > 0)
        cost += static_cast<uint32_t>(srlPacket.money); // \todo Change gold functions to uint64

    if (!sMailSystem.MailOption(MAIL_FLAG_DISABLE_POSTAGE_COSTS) && !(GetPermissionCount() && sMailSystem.MailOption(MAIL_FLAG_NO_COST_FOR_GM)))
        cost += srlPacket.itemCount ? 30 * srlPacket.itemCount : 30;;

    if (!_player->hasEnoughCoinage(cost))
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_NOT_ENOUGH_MONEY).serialise().get());
        return;
    }

    // build mail content
    MailMessage msg;

    if (!attachedItems.empty())
    {
        for (auto& item : attachedItems)
        {
            Item* pItem = item;
            if (_player->getItemInterface()->SafeRemoveAndRetreiveItemByGuid(item->getGuid(), false) != pItem)
                continue;

            pItem->RemoveFromWorld();
            pItem->setOwner(nullptr);
            pItem->SaveToDB(INVENTORY_SLOT_NOT_SET, 0, true, nullptr);
            msg.items.push_back(pItem->getGuidLow());

            if (GetPermissionCount() > 0)
                sGMLog.writefromsession(this, "sent mail with item entry %u to %s", pItem->getEntry(), playerReceiverInfo->name);

            pItem->DeleteMe();
        }
    }

    msg.delivery_time = static_cast<uint32_t>(UNIXTIME);
    if (srlPacket.money != 0 || srlPacket.cod != 0 || attachedItems.empty() && playerReceiverInfo->acct != _player->GetSession()->GetAccountId())
    {
        if (!sMailSystem.MailOption(MAIL_FLAG_DISABLE_HOUR_DELAY_FOR_ITEMS))
            msg.delivery_time += HOUR;
    }

    msg.player_guid = playerReceiverInfo->guid;
    msg.sender_guid = _player->getGuid();
    msg.stationery = srlPacket.stationery;
    msg.money = static_cast<uint32_t>(srlPacket.money);
    msg.cod = static_cast<uint32_t>(srlPacket.cod);
    msg.subject = srlPacket.subject;
    msg.body = srlPacket.body;

    if (!sMailSystem.MailOption(MAIL_FLAG_NO_EXPIRY))
        msg.expire_time = static_cast<uint32_t>(UNIXTIME) + (TimeVars::Day * MAIL_DEFAULT_EXPIRATION_TIME);
    else
        msg.expire_time = 0;

    msg.deleted_flag = false;
    msg.message_type = 0;
    msg.checked_flag = msg.body.empty() ? MAIL_CHECK_MASK_COPIED : MAIL_CHECK_MASK_HAS_BODY;

    sMailSystem.DeliverMessage(playerReceiverInfo->guid, &msg);

    // charge and save gold
    _player->modCoinage(-static_cast<int32_t>(cost));

    CharacterDatabase.Execute("UPDATE characters SET gold = %u WHERE guid = %u", _player->getCoinage(), _player->m_playerInfo->guid);

    SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_OK).serialise().get());
}
