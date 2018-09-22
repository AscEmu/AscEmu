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

    CmsgItemTextQuery recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;
  
    if (const auto item = _player->GetItemInterface()->GetItemByGUID(recv_packet.itemGuid))
        SendPacket(SmsgItemTextQueryResponse(0, recv_packet.itemGuid, item->GetText()).serialise().get());
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

    WorldPacket* data = _player->m_mailBox.BuildMailboxListingPacket();
    SendPacket(data);
    delete data;
}
