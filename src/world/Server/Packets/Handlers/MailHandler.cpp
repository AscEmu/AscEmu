/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
        mailMessage->expire_time = static_cast<uint32>(UNIXTIME) + (TIME_DAY * 30);

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
        if (_player->GetGold() + mailMessage->money > worldConfig.player.limitGoldAmount)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            return;
        }
    }

    _player->ModGold(mailMessage->money);
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

    MailMessage message = *mailMessage;

    _player->m_mailBox.DeleteMessage(srlPacket.messageId, true);

    message.player_guid = message.sender_guid;
    message.sender_guid = _player->getGuid();

    message.deleted_flag = false;
    message.checked_flag = MAIL_CHECK_MASK_RETURNED;

    message.cod = 0;

    message.delivery_time = message.items.empty() ? static_cast<uint32>(UNIXTIME) : static_cast<uint32>(UNIXTIME) + 3600;

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

    const SlotResult result = _player->GetItemInterface()->FindFreeInventorySlot(itemProperties);
    if (result.Result == 0)
    {
        SendPacket(SmsgSendMailResult(srlPacket.messageId, MAIL_RES_MADE_PERMANENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    auto item = objmgr.CreateItem(8383, _player);
    if (item == nullptr)
        return;

    item->setFlags(ITEM_FLAG_WRAP_GIFT);
    item->SetText(message->body);

    if (_player->GetItemInterface()->AddItemToFreeSlot(item))
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
  
    if (const auto item = _player->GetItemInterface()->GetItemByGUID(srlPacket.itemGuid))
        SendPacket(SmsgItemTextQueryResponse(0, srlPacket.itemGuid, item->GetText()).serialise().get());
    else
        SendPacket(SmsgItemTextQueryResponse(1, 0, "").serialise().get());
}

void WorldSession::handleMailTimeOpcode(WorldPacket& /*recvPacket*/)
{
    CHECK_INWORLD_RETURN

    WorldPacket data(MSG_QUERY_NEXT_MAIL_TIME, 100);
    _player->m_mailBox.FillTimePacket(data);
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

#if VERSION_STRING != Cata
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
                data << uint32_t(Arcemu::Util::GUID_LOPART(message.second.sender_guid));
                break;
            case MAIL_TYPE_GAMEOBJECT:
            case MAIL_TYPE_CREATURE:
                data << uint32_t(static_cast<uint32_t>(message.second.sender_guid));
                break;
        }

#if VERSION_STRING != Cata
        data << uint32_t(message.second.cod);
#else
        data << uint64_t(message.second.cod);
#endif
        data << uint32_t(0);
        data << uint32_t(message.second.stationery);
#if VERSION_STRING != Cata
        data << uint32_t(message.second.money);
#else
        data << uint64_t(message.second.money);
#endif
        data << uint32_t(message.second.checked_flag);
        data << float(float((message.second.expire_time - uint32(UNIXTIME)) / DAY));
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
