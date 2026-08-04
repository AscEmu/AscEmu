/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

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
#include "Server/Packets/SmsgMailListResult.h"
#include "Server/WorldSession.h"
#include "Management/MailMgr.h"
#include "Server/World.h"
#include "Objects/Units/Players/Player.hpp"
#include "Management/ItemInterface.h"
#include "Management/ObjectMgr.hpp"
#include "Objects/Item.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Definitions.h"
#include "Server/WorldSessionLog.hpp"
#include "Utilities/CommonTime.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleMarkAsReadOpcode(WorldPacket& recvPacket)
{
    CmsgMailMarkAsRead srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    auto mailMessage = _player->m_mailBox->GetMessageById(srlPacket.messageId);
    if (mailMessage == nullptr)
        return;

    mailMessage->checked_flag |= MAIL_CHECK_MASK_READ;

    if (!sMailSystem.MailOption(MAIL_FLAG_NO_EXPIRY))
        mailMessage->expire_time = static_cast<uint32_t>(UNIXTIME) + (TimeVars::Day * 30);

    CharacterDatabase.waitExecute("UPDATE mailbox SET checked_flag = %u, expiry_time = %u WHERE message_id = %u",
        mailMessage->checked_flag, mailMessage->expire_time, mailMessage->message_id);
}

void WorldSession::handleMailDeleteOpcode(WorldPacket& recvPacket)
{
    CmsgMailDelete srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const auto mailMessage = _player->m_mailBox->GetMessageById(srlPacket.messageId);
    if (mailMessage == nullptr)
    {
        SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_DELETED, MAIL_ERR_INTERNAL_ERROR);
        sendManagedPacket(managedPacket);
        return;
    }

    _player->m_mailBox->DeleteMessage(srlPacket.messageId, true);

    SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_DELETED, MAIL_OK);
    sendManagedPacket(managedPacket);
}

void WorldSession::handleTakeMoneyOpcode(WorldPacket& recvPacket)
{
    CmsgMailTakeMoney srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const auto mailMessage = _player->m_mailBox->GetMessageById(srlPacket.messageId);
    if (mailMessage == nullptr || !mailMessage->money)
    {
        SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_MONEY_TAKEN, MAIL_ERR_INTERNAL_ERROR);
        sendManagedPacket(managedPacket);
        return;
    }

    if (worldConfig.player.isGoldCapEnabled)
    {
        if (_player->getCoinage() + mailMessage->money > worldConfig.player.limitGoldAmount)
        {
            _player->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            return;
        }
    }

    _player->modCoinage(mailMessage->money);
    mailMessage->money = 0;

    CharacterDatabase.waitExecute("UPDATE mailbox SET money = 0 WHERE message_id = %u", mailMessage->message_id);

    SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_MONEY_TAKEN, MAIL_OK);
    sendManagedPacket(managedPacket);
}

void WorldSession::handleReturnToSenderOpcode(WorldPacket& recvPacket)
{
    CmsgMailReturnToSender srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const auto mailMessage = _player->m_mailBox->GetMessageById(srlPacket.messageId);
    if (mailMessage == nullptr)
    {
        SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_RETURNED_TO_SENDER, MAIL_ERR_INTERNAL_ERROR);
        sendManagedPacket(managedPacket);
        return;
    }

    auto message = *mailMessage;

    _player->m_mailBox->DeleteMessage(srlPacket.messageId, true);

    message.player_guid = message.sender_guid;
    message.sender_guid = _player->getGuid();

    message.deleted_flag = false;
    message.checked_flag = MAIL_CHECK_MASK_RETURNED;

    message.cod = 0;

    message.delivery_time = message.items.empty() ? static_cast<uint32_t>(UNIXTIME) : static_cast<uint32_t>(UNIXTIME) + HOUR;

    sMailSystem.DeliverMessage(message.player_guid, &message);

    SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_RETURNED_TO_SENDER, MAIL_OK);
    sendManagedPacket(managedPacket);
}

void WorldSession::handleMailCreateTextItemOpcode(WorldPacket& recvPacket)
{
    CmsgMailCreateTextItem srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const auto itemProperties = sMySQLStore.getItemProperties(8383);
    auto message = _player->m_mailBox->GetMessageById(srlPacket.messageId);
    if (message == nullptr || !itemProperties)
    {
        SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_MADE_PERMANENT, MAIL_ERR_INTERNAL_ERROR);
        sendManagedPacket(managedPacket);
        return;
    }

    const auto slotResult = _player->getItemInterface()->FindFreeInventorySlot(itemProperties);
    if (slotResult.Result == 0)
    {
        SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_MADE_PERMANENT, MAIL_ERR_INTERNAL_ERROR);
        sendManagedPacket(managedPacket);
        return;
    }

    auto item = sObjectMgr.createItem(8383, _player);
    if (item == nullptr)
        return;

    item->setFlags(ITEM_FLAG_WRAP_GIFT);
    item->setText(message->body);

    // TODO: if add fails, should item be sent in mail? now it's destroyed
    const auto [addResult, _] = _player->getItemInterface()->AddItemToFreeSlot(std::move(item));
    if (addResult)
    {
        SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_MADE_PERMANENT, MAIL_OK);
        sendManagedPacket(managedPacket);
    }
}

void WorldSession::handleItemTextQueryOpcode(WorldPacket& recvPacket)
{
    CmsgItemTextQuery srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

#if VERSION_STRING > TBC
    if (const auto item = _player->getItemInterface()->GetItemByGUID(srlPacket.itemGuid))
    {
        SmsgItemTextQueryResponse managedPacket(0, srlPacket.itemGuid, item->getText());
        sendManagedPacket(managedPacket);
    }
    else
    {
        SmsgItemTextQueryResponse managedPacket(1, 0, "");
        sendManagedPacket(managedPacket);
    }
#else
    if (auto itemPage = sMySQLStore.getItemPage(srlPacket.itemTextId))
    {
        SmsgItemTextQueryResponse managedPacket(0, itemPage->id, itemPage->text);
        sendManagedPacket(managedPacket);
    }
    else
    {
        SmsgItemTextQueryResponse managedPacket(1, 0, "");
        sendManagedPacket(managedPacket);
    }
#endif
}

void WorldSession::handleMailTimeOpcode(WorldPacket& /*recvPacket*/)
{
    WorldPacket data(MSG_QUERY_NEXT_MAIL_TIME, 32);
    {
        uint32_t unreadMessageCount = 0;
        data << uint32_t(0);
        data << uint32_t(0);

        for (auto& message : _player->m_mailBox->Messages)
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
    SmsgMailListResult managedPacket(_player->m_mailBox->Messages);
    sendManagedPacket(managedPacket);

    // do cleanup on request mail
    _player->m_mailBox->CleanupExpiredMessages();
}

void WorldSession::handleTakeItemOpcode(WorldPacket& recvPacket)
{
    CmsgMailTakeItem srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    auto mailMessage = _player->m_mailBox->GetMessageById(srlPacket.messageId);
    if (mailMessage == nullptr || mailMessage->items.empty())
    {
        SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_INTERNAL_ERROR);
        sendManagedPacket(managedPacket);
        return;
    }

    const auto itr = std::find(mailMessage->items.begin(), mailMessage->items.end(), srlPacket.lowGuid);
    if (itr == mailMessage->items.end())
    {
        SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_INTERNAL_ERROR);
        sendManagedPacket(managedPacket);
        return;
    }

    if (mailMessage->cod > 0)
    {
        if (!_player->hasEnoughCoinage(mailMessage->cod))
        {
            SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_NOT_ENOUGH_MONEY);
            sendManagedPacket(managedPacket);
            return;
        }
    }

    auto itemHolder = sObjectMgr.loadItem(srlPacket.lowGuid);
    if (itemHolder == nullptr)
    {
        SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_INTERNAL_ERROR);
        sendManagedPacket(managedPacket);
        return;
    }

    const auto slotResult = _player->getItemInterface()->FindFreeInventorySlot(itemHolder->getItemProperties());
    if (slotResult.Result == 0)
    {
        SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_BAG_FULL, INV_ERR_INVENTORY_FULL);
        sendManagedPacket(managedPacket);
        return;
    }
    itemHolder->m_isDirty = true;

    auto* item = itemHolder.get();

    auto [addResult, returnedItem] = _player->getItemInterface()->SafeAddItem(std::move(itemHolder), slotResult.ContainerSlot, slotResult.Slot);
    if (!addResult)
    {
        const auto [addResult2, _] = _player->getItemInterface()->AddItemToFreeSlot(std::move(returnedItem));
        if (!addResult2)
        {
            SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_BAG_FULL, INV_ERR_INVENTORY_FULL);
            sendManagedPacket(managedPacket);
            return;
        }
    }
    else
    {
        item->saveToDB(slotResult.ContainerSlot, slotResult.Slot, true, nullptr);
    }

    // Remove taken items and update message.
    mailMessage->items.erase(itr);
    sMailSystem.SaveMessageToSQL(mailMessage);

    SmsgSendMailResult managedPacket(srlPacket.messageId, MAIL_RES_ITEM_TAKEN, MAIL_OK, item->getGuidLow(), item->getStackCount());
    sendManagedPacket(managedPacket);

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
        CharacterDatabase.execute("UPDATE mailbox SET cod = 0 WHERE message_id = %u", mailMessage->message_id);
    }
}

void WorldSession::handleSendMailOpcode(WorldPacket& recvPacket)
{
    CmsgSendMail srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
    {
        SmsgSendMailResult managedPacket(0, MAIL_RES_MAIL_SENT, MAIL_ERR_INTERNAL_ERROR);
        sendManagedPacket(managedPacket);
        return;
    }

    if (srlPacket.itemCount > MAIL_MAX_ITEM_SLOT)
    {
        SmsgSendMailResult managedPacket(0, MAIL_RES_MAIL_SENT, MAIL_ERR_TOO_MANY_ATTACHMENTS);
        sendManagedPacket(managedPacket);
        return;
    }

    const auto playerReceiverInfo = sObjectMgr.getCachedCharacterInfoByName(srlPacket.receiverName);
    if (playerReceiverInfo == nullptr)
    {
        SmsgSendMailResult managedPacket(0, MAIL_RES_MAIL_SENT, MAIL_ERR_RECIPIENT_NOT_FOUND);
        sendManagedPacket(managedPacket);
        return;
    }

    std::vector<Item*> attachedItems;
    for (uint8_t i = 0; i < srlPacket.itemCount; ++i)
    {
        Item* pItem = _player->getItemInterface()->GetItemByGUID(srlPacket.itemGuid[i]);
        if (pItem == nullptr || pItem->isSoulbound() || pItem->hasFlags(ITEM_FLAG_CONJURED))
        {
            SmsgSendMailResult managedPacket(0, MAIL_RES_MAIL_SENT, MAIL_ERR_INTERNAL_ERROR);
            sendManagedPacket(managedPacket);
            return;
        }

        if (pItem->isAccountbound() && GetAccountId() != playerReceiverInfo->acct)
        {
            SmsgSendMailResult managedPacket(0, MAIL_RES_MAIL_SENT, MAIL_ERR_BAG_FULL, INV_ERR_ARTEFACTS_ONLY_FOR_OWN_CHARACTERS);
            sendManagedPacket(managedPacket);
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
        SmsgSendMailResult managedPacket(0, MAIL_RES_MAIL_SENT, MAIL_ERR_NOT_YOUR_ALLIANCE);
        sendManagedPacket(managedPacket);
        return;
    }

    if (playerReceiverInfo->name == _player->getName() && !hasPermissions())
    {
        SmsgSendMailResult managedPacket(0, MAIL_RES_MAIL_SENT, MAIL_ERR_CANNOT_SEND_TO_SELF);
        sendManagedPacket(managedPacket);
        return;
    }

    if (srlPacket.stationery == MAIL_STATIONERY_GM && !HasGMPermissions())
    {
        SmsgSendMailResult managedPacket(0, MAIL_RES_MAIL_SENT, MAIL_ERR_INTERNAL_ERROR);
        sendManagedPacket(managedPacket);
        return;
    }

    // calculate cost
    uint32_t cost = 0;
    if (srlPacket.money > 0)
        cost += static_cast<uint32_t>(srlPacket.money); // \todo Change gold functions to uint64_t

    if (!sMailSystem.MailOption(MAIL_FLAG_DISABLE_POSTAGE_COSTS) && !(hasPermissions() && sMailSystem.MailOption(MAIL_FLAG_NO_COST_FOR_GM)))
        cost += srlPacket.itemCount ? 30 * srlPacket.itemCount : 30;

    if (!_player->hasEnoughCoinage(cost))
    {
        SmsgSendMailResult managedPacket(0, MAIL_RES_MAIL_SENT, MAIL_ERR_NOT_ENOUGH_MONEY);
        sendManagedPacket(managedPacket);
        return;
    }

    // build mail content
    MailMessage msg;

    if (!attachedItems.empty())
    {
        for (auto& item : attachedItems)
        {
            auto pItem = _player->getItemInterface()->SafeRemoveAndRetreiveItemByGuid(item->getGuid(), false);
            if (pItem == nullptr || pItem.get() != item)
                continue;

            pItem->removeFromWorld();
            pItem->setOwner(nullptr);
            pItem->saveToDB(INVENTORY_SLOT_NOT_SET, 0, true, nullptr);
            msg.items.push_back(pItem->getGuidLow());

            if (hasPermissions())
                sGMLog.writefromsession(this, "Sent mail with item entry {} to {}.", pItem->getEntry(), playerReceiverInfo->name);
        }
    }

    msg.delivery_time = static_cast<uint32_t>(UNIXTIME);
    if (srlPacket.money != 0 || srlPacket.cod != 0 || (attachedItems.empty() && playerReceiverInfo->acct != _player->getSession()->GetAccountId()))
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

    CharacterDatabase.execute("UPDATE characters SET gold = %u WHERE guid = %u", _player->getCoinage(), _player->m_playerInfo->guid);

    SmsgSendMailResult managedPacket(0, MAIL_RES_MAIL_SENT, MAIL_OK);
    sendManagedPacket(managedPacket);
}
