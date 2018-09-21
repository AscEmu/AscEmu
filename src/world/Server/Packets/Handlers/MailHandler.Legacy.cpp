/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
 */

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Server/Packets/CmsgMailMarkAsRead.h"
#include "Server/Packets/CmsgMailDelete.h"
#include "Server/Packets/SmsgSendMailResult.h"
#include "Server/Packets/CmsgMailTakeItem.h"
#include "Server/Packets/CmsgMailTakeMoney.h"

using namespace  AscEmu::Packets;

#if VERSION_STRING != Cata
/// \todo refactoring
bool MailMessage::AddMessageDataToPacket(WorldPacket& data)
{
    uint8 i = 0;
    std::vector<uint32>::iterator itr;
    Item* pItem;

    // add stuff
    if (deleted_flag)
        return false;

    uint8 guidsize;
    if (message_type == 0)
        guidsize = 8;
    else
        guidsize = 4;

    size_t msize = 2 + 4 + 1 + guidsize + 4 * 8 + (subject.size() + 1) + (body.size() + 1) + 1 + (items.size() * (1 + 2 * 4 + 7 * (3 * 4) + 6 * 4 + 1));

    data << uint16(msize);     // message size
    data << uint32(message_id);
    data << uint8(message_type);

    switch (message_type)
    {
        case MAIL_TYPE_NORMAL:
            data << uint64(sender_guid);
            break;
        case MAIL_TYPE_COD:
        case MAIL_TYPE_AUCTION:
        case MAIL_TYPE_ITEM:
            data << uint32(Arcemu::Util::GUID_LOPART(sender_guid));
            break;
        case MAIL_TYPE_GAMEOBJECT:
        case MAIL_TYPE_CREATURE:
            data << uint32(static_cast<uint32>(sender_guid));
            break;
    }

    data << uint32(cod);            // cod
    data << uint32(0);
    data << uint32(stationery);
    data << uint32(money);        // money
    data << uint32(checked_flag);           // "checked" flag
    data << float((expire_time - uint32(UNIXTIME)) / 86400.0f);
    data << uint32(0);    // mail template
    data << subject;
    data << body;

    data << uint8(items.size());        // item count

    if (!items.empty())
    {
        for (itr = items.begin(); itr != items.end(); ++itr)
        {
            pItem = objmgr.LoadItem(*itr);
            if (pItem == nullptr)
                continue;

            data << uint8(i++);
            data << uint32(pItem->getGuidLow());
            data << uint32(pItem->getEntry());

            for (uint8_t j = 0; j < MAX_ENCHANTMENT_SLOT; ++j)
            {
                data << uint32(pItem->getEnchantmentId(j));
                data << uint32(pItem->getEnchantmentDuration(j));
                data << uint32(0);
            }

            data << uint32(pItem->getRandomPropertiesId());
            data << uint32(pItem->getPropertySeed());
            data << uint32(pItem->getStackCount());
            data << uint32(pItem->GetChargesLeft());
            data << uint32(pItem->getMaxDurability());
            data << uint32(pItem->getDurability());
            data << uint8(0);   // unknown

            delete pItem;
        }

    }

    return true;
}

void WorldSession::HandleSendMail(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    MailMessage msg;
    uint64 gameobject;
    uint32 unk2;
    uint8 itemcount;
    uint8 itemslot;
    uint8 i;
    uint64 itemguid;
    std::vector< Item* > items;
    std::vector< Item* >::iterator itr;
    std::string recepient;
    Item* pItem;
    //uint32 err = MAIL_OK;

    recv_data >> gameobject;
    recv_data >> recepient;
    recv_data >> msg.subject;
    recv_data >> msg.body;
    recv_data >> msg.stationery;
    recv_data >> unk2;
    recv_data >> itemcount;

    if (itemcount > MAIL_MAX_ITEM_SLOT || msg.body.find("%") != std::string::npos || msg.subject.find("%") != std::string::npos)
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    // Search for the recipient
    PlayerInfo* player = ObjectMgr::getSingleton().GetPlayerInfoByName(recepient.c_str());
    if (player == nullptr)
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_RECIPIENT_NOT_FOUND).serialise().get());
        return;
    }

    for (i = 0; i < itemcount; ++i)
    {
        recv_data >> itemslot;
        recv_data >> itemguid;

        pItem = _player->GetItemInterface()->GetItemByGUID(itemguid);
        if (pItem == nullptr || pItem->isSoulbound() || pItem->hasFlags(ITEM_FLAG_CONJURED))
        {
            SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
            return;
        }
        if (pItem->isAccountbound() && GetAccountId() != player->acct) // don't mail account-bound items to another account
        {
            WorldPacket data(SMSG_SEND_MAIL_RESULT, 16);
            data << uint32(0);
            data << uint32(0);
            data << uint32(MAIL_ERR_BAG_FULL);
            data << uint32(INV_ERR_ARTEFACTS_ONLY_FOR_OWN_CHARACTERS);
            SendPacket(&data);
            return;
        }

        items.push_back(pItem);
    }

    recv_data >> msg.money;
    recv_data >> msg.cod;
    ///\todo left over: (TODO- FIX ME BURLEX!)
    // uint32
    // uint32
    // uint8

    bool interfaction = false;
    if (sMailSystem.MailOption(MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION) || (HasGMPermissions() && sMailSystem.MailOption(MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION_GM)))
    {
        interfaction = true;
    }

    // Check we're sending to the same faction (disable this for testing)
    if (player->team != _player->GetTeam() && !interfaction)
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_NOT_YOUR_ALLIANCE).serialise().get());
        return;
    }

    // Check if we're sending mail to ourselves
    if (strcmp(player->name, _player->getName().c_str()) == 0 && !GetPermissionCount())
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_CANNOT_SEND_TO_SELF).serialise().get());
        return;
    }

    if (msg.stationery == MAIL_STATIONERY_GM && !HasGMPermissions())
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    // Instant delivery time by default.
    msg.delivery_time = (uint32)UNIXTIME;

    // Set up the cost
    int32 cost = 0;

    // Check for attached money
    if (msg.money > 0)
        cost += msg.money;


    if (!sMailSystem.MailOption(MAIL_FLAG_DISABLE_POSTAGE_COSTS) && !(GetPermissionCount() && sMailSystem.MailOption(MAIL_FLAG_NO_COST_FOR_GM)))
    {
        cost += 30;
    }

    // check that we have enough in our backpack
    if (!_player->HasGold(cost))
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_NOT_ENOUGH_MONEY).serialise().get());
        return;
    }

    // Check for the item, and required item.
    if (!items.empty())
    {
        for (itr = items.begin(); itr != items.end(); ++itr)
        {
            pItem = *itr;
            if (_player->GetItemInterface()->SafeRemoveAndRetreiveItemByGuid(pItem->getGuid(), false) != pItem)
                continue;        // should never be hit.

            pItem->RemoveFromWorld();
            pItem->setOwner(nullptr);
            pItem->SaveToDB(INVENTORY_SLOT_NOT_SET, 0, true, nullptr);
            msg.items.push_back(pItem->getGuidLow());

            if (GetPermissionCount() > 0)
            {
                /* log the message */
                sGMLog.writefromsession(this, "sent mail with item entry %u to %s, with gold %u.", pItem->getEntry(), player->name, msg.money);
            }

            pItem->DeleteMe();
        }
    }

    if (msg.money != 0 || msg.cod != 0 || (!msg.items.size() && player->acct != _player->GetSession()->GetAccountId()))
    {
        if (!sMailSystem.MailOption(MAIL_FLAG_DISABLE_HOUR_DELAY_FOR_ITEMS))
            msg.delivery_time += 3600;  // 1hr
    }

    // take the money
    _player->ModGold(-cost);

    // Fill in the rest of the info
    msg.player_guid = player->guid;
    msg.sender_guid = _player->getGuid();

    // 30 day expiry time for unread mail
    if (!sMailSystem.MailOption(MAIL_FLAG_NO_EXPIRY))
        msg.expire_time = (uint32)UNIXTIME + (TIME_DAY * MAIL_DEFAULT_EXPIRATION_TIME);
    else
        msg.expire_time = 0;

    msg.deleted_flag = false;
    msg.message_type = 0;
    msg.checked_flag = msg.body.empty() ? MAIL_CHECK_MASK_COPIED : MAIL_CHECK_MASK_HAS_BODY;

    // Great, all our info is filled in. Now we can add it to the other players mailbox.
    sMailSystem.DeliverMessage(player->guid, &msg);
    // Save/Update character's gold if they've received gold that is. This prevents a rollback.
    CharacterDatabase.Execute("UPDATE characters SET gold = %u WHERE guid = %u", _player->GetGold(), _player->m_playerInfo->guid);

    SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_OK).serialise().get());
}

void WorldSession::HandleMarkAsRead(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CmsgMailMarkAsRead recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    MailMessage* message = _player->m_mailBox.GetMessage(recv_packet.messageId);
    if (message == nullptr)
        return;

    // mark the message as read
    message->checked_flag |= MAIL_CHECK_MASK_READ;

    // mail now has a 30 day expiry time
    if (!sMailSystem.MailOption(MAIL_FLAG_NO_EXPIRY))
        message->expire_time = (uint32)UNIXTIME + (TIME_DAY * 30);

    // update it in sql
    CharacterDatabase.WaitExecute("UPDATE mailbox SET checked_flag = %u, expiry_time = %u WHERE message_id = %u",
                                  message->checked_flag, message->expire_time, message->message_id);
}

void WorldSession::HandleMailDelete(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CmsgMailDelete recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    MailMessage* message = _player->m_mailBox.GetMessage(recv_packet.messageId);
    if (message == nullptr)
    {
        SendPacket(SmsgSendMailResult(recv_packet.messageId, MAIL_RES_DELETED, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    _player->m_mailBox.DeleteMessage(recv_packet.messageId, true);

    SendPacket(SmsgSendMailResult(recv_packet.messageId, MAIL_RES_DELETED, MAIL_OK).serialise().get());
}

void WorldSession::HandleTakeItem(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CmsgMailTakeItem recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    std::vector< uint32 >::iterator itr;

    MailMessage* message = _player->m_mailBox.GetMessage(recv_packet.messageId);
    if (message == nullptr || message->items.empty())
    {
        SendPacket(SmsgSendMailResult(recv_packet.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    for (itr = message->items.begin(); itr != message->items.end(); ++itr)
    {
        if ((*itr) == recv_packet.lowGuid)
            break;
    }

    if (itr == message->items.end())
    {
        SendPacket(SmsgSendMailResult(recv_packet.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    if (message->cod > 0)   // check for cod credit
    {
        if (!_player->HasGold(message->cod))
        {
            SendPacket(SmsgSendMailResult(recv_packet.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_NOT_ENOUGH_MONEY).serialise().get());
            return;
        }
    }

    // grab the item
    Item* item = objmgr.LoadItem(*itr);
    if (item == nullptr)  // doesn't exist
    {
        SendPacket(SmsgSendMailResult(recv_packet.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    //Find free slot
    SlotResult result = _player->GetItemInterface()->FindFreeInventorySlot(item->getItemProperties());
    if (result.Result == 0) //End of slots
    {
        SendPacket(SmsgSendMailResult(recv_packet.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_BAG_FULL).serialise().get());

        item->DeleteMe();
        return;
    }
    item->m_isDirty = true;

    if (!_player->GetItemInterface()->SafeAddItem(item, result.ContainerSlot, result.Slot))
    {
        if (!_player->GetItemInterface()->AddItemToFreeSlot(item))   //End of slots
        {
            SendPacket(SmsgSendMailResult(recv_packet.messageId, MAIL_RES_ITEM_TAKEN, MAIL_ERR_BAG_FULL).serialise().get());
            item->DeleteMe();
            return;
        }
    }
    else
        item->SaveToDB(result.ContainerSlot, result.Slot, true, nullptr);

    // send complete packet
    WorldPacket data(SMSG_SEND_MAIL_RESULT, 12);
    data << recv_packet.messageId;
    data << uint32(MAIL_RES_ITEM_TAKEN);
    data << uint32(MAIL_OK);
    data << item->getGuidLow();
    data << item->getStackCount();

    message->items.erase(itr);

    // re-save (update the items field)
    sMailSystem.SaveMessageToSQL(message);
    SendPacket(&data);

    // Send message back if cod was set.
    if (message->cod > 0)
    {
        _player->ModGold(-(int32)message->cod);
        std::string subject = "COD Payment: ";
        subject += message->subject;
        uint64 answer_sender = message->player_guid;
        uint64 answer_receiver = message->sender_guid;
        uint32 answer_cod_money = message->cod;

        sMailSystem.SendAutomatedMessage(MAIL_TYPE_NORMAL, answer_sender, answer_receiver, subject, "", answer_cod_money, 0, 0, MAIL_STATIONERY_TEST1, MAIL_CHECK_MASK_COD_PAYMENT);

        message->cod = 0;
        CharacterDatabase.Execute("UPDATE mailbox SET cod = 0 WHERE message_id = %u", message->message_id);
    }

    // probably need to send an item push here
}

void WorldSession::HandleTakeMoney(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CmsgMailTakeMoney recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    MailMessage* message = _player->m_mailBox.GetMessage(recv_packet.messageId);
    if (message == nullptr || !message->money)
    {
        SendPacket(SmsgSendMailResult(recv_packet.messageId, MAIL_RES_MONEY_TAKEN, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    // Check they don't have more than the max gold
    if (worldConfig.player.isGoldCapEnabled)
    {
        if ((_player->GetGold() + message->money) > worldConfig.player.limitGoldAmount)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_TOO_MUCH_GOLD);
            return;
        }
    }

    // add the money to the player
    _player->ModGold(message->money);

    // message no longer has any money
    message->money = 0;

    // update in sql!
    CharacterDatabase.WaitExecute("UPDATE mailbox SET money = 0 WHERE message_id = %u", message->message_id);

    SendPacket(SmsgSendMailResult(recv_packet.messageId, MAIL_RES_MONEY_TAKEN, MAIL_OK).serialise().get());
}

void WorldSession::HandleReturnToSender(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 mailbox;
    uint32 message_id;
    recv_data >> mailbox;
    recv_data >> message_id;

    MailMessage* msg = _player->m_mailBox.GetMessage(message_id);
    if (msg == nullptr)
    {
        SendPacket(SmsgSendMailResult(message_id, MAIL_RES_RETURNED_TO_SENDER, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    // copy into a new struct
    MailMessage message = *msg;

    // remove the old message
    _player->m_mailBox.DeleteMessage(message_id, true);

    // re-assign the owner/sender
    message.player_guid = message.sender_guid;
    message.sender_guid = _player->getGuid();

    message.deleted_flag = false;
    message.checked_flag = MAIL_CHECK_MASK_RETURNED;

    // null out the cod charges. (the sender doesn't want to have to pay for his own item
    // that he got nothing for.. :p)
    message.cod = 0;

    // assign new delivery time
    message.delivery_time = message.items.empty() ? (uint32)UNIXTIME : (uint32)UNIXTIME + 3600;

    // add to the senders mailbox
    sMailSystem.DeliverMessage(message.player_guid, &message);

    SendPacket(SmsgSendMailResult(message_id, MAIL_RES_RETURNED_TO_SENDER, MAIL_OK).serialise().get());
}

void WorldSession::HandleMailCreateTextItem(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 mailbox;
    uint32 message_id;
    recv_data >> mailbox;
    recv_data >> message_id;

    ItemProperties const* proto = sMySQLStore.getItemProperties(8383);
    MailMessage* message = _player->m_mailBox.GetMessage(message_id);
    if (message == nullptr || !proto)
    {
        SendPacket(SmsgSendMailResult(message_id, MAIL_RES_MADE_PERMANENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    SlotResult result = _player->GetItemInterface()->FindFreeInventorySlot(proto);
    if (result.Result == 0)
    {
        SendPacket(SmsgSendMailResult(message_id, MAIL_RES_MADE_PERMANENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    Item* pItem = objmgr.CreateItem(8383, _player);
    if (pItem == nullptr)
        return;

    pItem->setFlags(ITEM_FLAG_WRAP_GIFT); // the flag is probably misnamed
    pItem->SetText(message->body);

    if (_player->GetItemInterface()->AddItemToFreeSlot(pItem))
        SendPacket(SmsgSendMailResult(message_id, MAIL_RES_MADE_PERMANENT, MAIL_OK).serialise().get());
    else
        pItem->DeleteMe();
}

void WorldSession::HandleItemTextQuery(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 itemGuid;
    recv_data >> itemGuid;

    Item* pItem = _player->GetItemInterface()->GetItemByGUID(itemGuid);
    WorldPacket data(SMSG_ITEM_TEXT_QUERY_RESPONSE, pItem->GetText().size() + 9);
    if (!pItem)
        data << uint8(1);
    else
    {
        data << uint8(0);
        data << uint64(itemGuid);
        data << pItem->GetText();
    }

    SendPacket(&data);
}

void Mailbox::FillTimePacket(WorldPacket& data)
{
    uint32 c = 0;
    MessageMap::iterator iter = Messages.begin();
    data << uint32(0);
    data << uint32(0);

    for (; iter != Messages.end(); ++iter)
    {
        if (iter->second.checked_flag & MAIL_CHECK_MASK_READ)
            continue;

        if (iter->second.deleted_flag == 0 && (uint32)UNIXTIME >= iter->second.delivery_time)
        {
            // unread message, w00t.
            ++c;
            data << uint64(iter->second.sender_guid);
            data << uint32(0);
            data << uint32(0);// money or something?
            data << uint32(iter->second.stationery);
            //data << float(UNIXTIME-iter->second.delivery_time);
            data << float(-9.0f);    // maybe the above?
        }
    }

    if (c == 0)
        *(uint32*)(&data.contents()[0]) = 0xc7a8c000;
    else
        *(uint32*)(&data.contents()[4]) = c;

}

void WorldSession::HandleMailTime(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

    WorldPacket data(MSG_QUERY_NEXT_MAIL_TIME, 100);
    _player->m_mailBox.FillTimePacket(data);
    SendPacket(&data);
}

void WorldSession::HandleGetMail(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

    WorldPacket* data = _player->m_mailBox.BuildMailboxListingPacket();
    SendPacket(data);
    delete data;
}

WorldPacket* Mailbox::BuildMailboxListingPacket()
{
    WorldPacket* data = new WorldPacket(SMSG_MAIL_LIST_RESULT, 500);
    MessageMap::iterator itr;
    uint32 realcount = 0;
    uint32 count = 0;
    uint32 t = (uint32)UNIXTIME;
    *data << uint32(0);     // realcount - this can be used to tell the client we have more mail than that fits into this packet
    *data << uint8(0);     // size placeholder

    for (itr = Messages.begin(); itr != Messages.end(); ++itr)
    {
        if (itr->second.expire_time && t > itr->second.expire_time)
            continue;       // expired mail -> skip it

        if ((uint32)UNIXTIME < itr->second.delivery_time)
            continue;        // undelivered

        if (count >= 50) //VLack: We could calculate message sizes instead of this, but the original code did a break at 50, so I won't fix this up if no one felt the need to do so before ;-)
        {
            ++realcount;
            continue;
        }

        if (itr->second.AddMessageDataToPacket(*data))
        {
            ++count;
            ++realcount;
        }
    }

    data->put<uint32>(0, realcount);
    data->put<uint8>(4, static_cast<uint8>(count));

    // do cleanup on request mail
    CleanupExpiredMessages();
    return data;
}

#endif
