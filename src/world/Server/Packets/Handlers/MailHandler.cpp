/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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


void WorldSession::HandleSendMail(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        MailMessage msg;
    ObjectGuid mailbox;
    uint32 unk1, unk2;
    uint64 money, COD;
    uint32 bodyLength, subjectLength, receiverLength;
    std::string receiver, subject, body;

    std::vector< Item* > items;
    std::vector< Item* >::iterator itr;
    Item* pItem;

    recv_data >> unk1;
    recv_data >> unk2;

    recv_data >> COD;
    recv_data >> money;

    bodyLength = recv_data.readBits(12);
    subjectLength = recv_data.readBits(9);

    uint8 items_count = recv_data.readBits(5);              // attached items count

    if (items_count > MAIL_MAX_ITEM_SLOT)
    {
        SendMailError(MAIL_ERR_TOO_MANY_ATTACHMENTS);
        return;
    }

    mailbox[0] = recv_data.readBit();

    ObjectGuid itemGUIDs[MAIL_MAX_ITEM_SLOT];

    for (uint8 i = 0; i < items_count; ++i)
    {
        itemGUIDs[i][2] = recv_data.readBit();
        itemGUIDs[i][6] = recv_data.readBit();
        itemGUIDs[i][3] = recv_data.readBit();
        itemGUIDs[i][7] = recv_data.readBit();
        itemGUIDs[i][1] = recv_data.readBit();
        itemGUIDs[i][0] = recv_data.readBit();
        itemGUIDs[i][4] = recv_data.readBit();
        itemGUIDs[i][5] = recv_data.readBit();
    }

    mailbox[3] = recv_data.readBit();
    mailbox[4] = recv_data.readBit();
    receiverLength = recv_data.readBits(7);
    mailbox[2] = recv_data.readBit();
    mailbox[6] = recv_data.readBit();
    mailbox[1] = recv_data.readBit();
    mailbox[7] = recv_data.readBit();
    mailbox[5] = recv_data.readBit();

    recv_data.ReadByteSeq(mailbox[4]);

    for (uint8 i = 0; i < items_count; ++i)
    {
        recv_data.ReadByteSeq(itemGUIDs[i][6]);
        recv_data.ReadByteSeq(itemGUIDs[i][1]);
        recv_data.ReadByteSeq(itemGUIDs[i][7]);
        recv_data.ReadByteSeq(itemGUIDs[i][2]);
        recv_data.read_skip<uint8>();            // item slot in mail, not used
        recv_data.ReadByteSeq(itemGUIDs[i][3]);
        recv_data.ReadByteSeq(itemGUIDs[i][0]);
        recv_data.ReadByteSeq(itemGUIDs[i][4]);
        recv_data.ReadByteSeq(itemGUIDs[i][5]);
    }

    recv_data.ReadByteSeq(mailbox[7]);
    recv_data.ReadByteSeq(mailbox[3]);
    recv_data.ReadByteSeq(mailbox[6]);
    recv_data.ReadByteSeq(mailbox[5]);

    subject = recv_data.ReadString(subjectLength);
    receiver = recv_data.ReadString(receiverLength);

    recv_data.ReadByteSeq(mailbox[2]);
    recv_data.ReadByteSeq(mailbox[0]);

    body = recv_data.ReadString(bodyLength);

    recv_data.ReadByteSeq(mailbox[1]);

    // packet read complete, now do check

    // Search for the recipient
    PlayerInfo* player = ObjectMgr::getSingleton().GetPlayerInfoByName(receiver.c_str());
    if (player == NULL)
    {
        SendMailError(MAIL_ERR_RECIPIENT_NOT_FOUND);
        return;
    }

    for (uint8 i = 0; i < items_count; ++i)
    {
        pItem = _player->GetItemInterface()->GetItemByGUID(itemGUIDs[i]);
        if (pItem == NULL || pItem->IsSoulbound() || pItem->IsConjured())
        {
            SendMailError(MAIL_ERR_INTERNAL_ERROR);
            return;
        }
        if (pItem->IsAccountbound() && GetAccountId() != player->acct) // don't mail account-bound items to another account
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

    if (receiver.empty())
        return;

    bool interfaction = false;
    if (sMailSystem.MailOption(MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION) || (HasGMPermissions() && sMailSystem.MailOption(MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION_GM)))
    {
        interfaction = true;
    }

    // Check we're sending to the same faction (disable this for testing)
    if (player->team != _player->GetTeam() && !interfaction)
    {
        SendMailError(MAIL_ERR_NOT_YOUR_ALLIANCE);
        return;
    }

    // Check if we're sending mail to ourselves
    if (strcmp(player->name, _player->GetName()) == 0 && !GetPermissionCount())
    {
        SendMailError(MAIL_ERR_CANNOT_SEND_TO_SELF);
        return;
    }

    if (msg.stationery == MAIL_STATIONERY_GM && !HasGMPermissions())
    {
        SendMailError(MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    // Instant delivery time by default.
    msg.delivery_time = (uint32)UNIXTIME;

    // Set up the cost
    uint32 cost = items_count ? 30 * items_count : 30;  // price hardcoded in client

    uint64 reqmoney = cost + money;

    if (!sMailSystem.MailOption(MAIL_FLAG_DISABLE_POSTAGE_COSTS) && !(GetPermissionCount() && sMailSystem.MailOption(MAIL_FLAG_NO_COST_FOR_GM)))
    {
        cost += 30;
    }

    // check that we have enough in our backpack
    if (!_player->HasGold(cost))
    {
        SendMailError(MAIL_ERR_NOT_ENOUGH_MONEY);
        return;
    }

    // Check for the item, and required item.
    if (!items.empty())
    {
        for (itr = items.begin(); itr != items.end(); ++itr)
        {
            pItem = *itr;
            if (_player->GetItemInterface()->SafeRemoveAndRetreiveItemByGuid(pItem->GetGUID(), false) != pItem)
                continue;        // should never be hit.

            pItem->RemoveFromWorld();
            pItem->SetOwner(NULL);
            pItem->SaveToDB(INVENTORY_SLOT_NOT_SET, 0, true, NULL);
            msg.items.push_back(pItem->GetLowGUID());

            if (GetPermissionCount() > 0)
            {
                /* log the message */
                sGMLog.writefromsession(this, "sent mail with item entry %u to %s, with gold %u.", pItem->GetEntry(), player->name, money);
            }

            pItem->DeleteMe();
        }
    }

    if (money != 0 || COD != 0 || (!items.size() && player->acct != _player->GetSession()->GetAccountId()))
    {
        if (!sMailSystem.MailOption(MAIL_FLAG_DISABLE_HOUR_DELAY_FOR_ITEMS))
            msg.delivery_time += 3600;  // 1hr
    }

    // take the money
    _player->ModGold(-cost);

    // Fill in the rest of the info
    msg.player_guid = player->guid;
    msg.sender_guid = _player->GetGUID();
    msg.money = money;
    msg.cod = COD;
    msg.subject = subject;
    msg.body = body;

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
    // Success packet :)
    SendMailError(MAIL_OK);
}

void WorldSession::HandleMarkAsRead(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 mailbox;
    uint32 message_id;
    recv_data >> mailbox;
    recv_data >> message_id;

    MailMessage* message = _player->m_mailBox.GetMessage(message_id);
    if (message == 0)
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

    uint64 mailbox;
    uint32 message_id;
    recv_data >> mailbox;
    recv_data >> message_id;
    recv_data.read_skip<uint32>();

    WorldPacket data(SMSG_SEND_MAIL_RESULT, 12);
    data << message_id;
    data << uint32(MAIL_RES_DELETED);

    MailMessage* message = _player->m_mailBox.GetMessage(message_id);
    if (message == 0)
    {
        data << uint32(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    _player->m_mailBox.DeleteMessage(message_id, true);

    data << uint32(MAIL_OK);
    SendPacket(&data);
}

void WorldSession::HandleTakeItem(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 mailbox;
    uint32 message_id;
    uint32 lowguid;
    std::vector< uint32 >::iterator itr;

    recv_data >> mailbox;
    recv_data >> message_id;
    recv_data >> lowguid;

    WorldPacket data(SMSG_SEND_MAIL_RESULT, 12);
    data << message_id;
    data << uint32(MAIL_RES_ITEM_TAKEN);

    MailMessage* message = _player->m_mailBox.GetMessage(message_id);
    if (message == 0 || message->items.empty())
    {
        data << uint32(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    for (itr = message->items.begin(); itr != message->items.end(); ++itr)
    {
        if ((*itr) == lowguid)
            break;
    }

    if (itr == message->items.end())
    {
        data << uint32(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    if (message->cod > 0)   // check for cod credit
    {
        if (!_player->HasGold(message->cod))
        {
            data << uint32(MAIL_ERR_NOT_ENOUGH_MONEY);
            SendPacket(&data);
            return;
        }
    }

    // grab the item
    Item* item = objmgr.LoadItem(*itr);
    if (item == 0)  // doesn't exist
    {
        data << uint32(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);

        return;
    }

    //Find free slot
    SlotResult result = _player->GetItemInterface()->FindFreeInventorySlot(item->GetItemProperties());
    if (result.Result == 0) //End of slots
    {
        data << uint32(MAIL_ERR_BAG_FULL);
        SendPacket(&data);

        item->DeleteMe();
        return;
    }
    item->m_isDirty = true;

    if (!_player->GetItemInterface()->SafeAddItem(item, result.ContainerSlot, result.Slot))
    {
        if (!_player->GetItemInterface()->AddItemToFreeSlot(item))   //End of slots
        {
            data << uint32(MAIL_ERR_BAG_FULL);
            SendPacket(&data);
            item->DeleteMe();
            return;
        }
    }
    else
        item->SaveToDB(result.ContainerSlot, result.Slot, true, NULL);

    // send complete packet
    data << uint32(MAIL_OK);
    data << item->GetLowGUID();
    data << item->GetStackCount();

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

    uint64 mailbox;
    uint32 message_id;
    recv_data >> mailbox;
    recv_data >> message_id;

    WorldPacket data(SMSG_SEND_MAIL_RESULT, 12);
    data << message_id;
    data << uint32(MAIL_RES_MONEY_TAKEN);

    MailMessage* message = _player->m_mailBox.GetMessage(message_id);
    if (message == 0 || !message->money)
    {
        data << uint32(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    // Check they don't have more than the max gold
    if (sWorld.GoldCapEnabled)
    {
        if ((_player->GetGold() + message->money) > sWorld.GoldLimit)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_TOO_MUCH_GOLD);
            return;
        }
    }

    // add the money to the player
    _player->ModGold(message->money);

    // message no longer has any money
    message->money = 0;

    // update in sql!
    CharacterDatabase.WaitExecute("UPDATE mailbox SET money = 0 WHERE message_id = %u", message->message_id);

    // send result
    data << uint32(MAIL_OK);
    SendPacket(&data);
}

void WorldSession::HandleReturnToSender(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 mailbox;
    uint32 message_id;
    recv_data >> mailbox;
    recv_data >> message_id;

    WorldPacket data(SMSG_SEND_MAIL_RESULT, 12);
    data << message_id;
    data << uint32(MAIL_RES_RETURNED_TO_SENDER);

    MailMessage* msg = _player->m_mailBox.GetMessage(message_id);
    if (msg == 0)
    {
        data << uint32(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    // copy into a new struct
    MailMessage message = *msg;

    // remove the old message
    _player->m_mailBox.DeleteMessage(message_id, true);

    // re-assign the owner/sender
    message.player_guid = message.sender_guid;
    message.sender_guid = _player->GetGUID();

    message.deleted_flag = false;
    message.checked_flag = MAIL_CHECK_MASK_RETURNED;

    // null out the cod charges. (the sender doesn't want to have to pay for his own item
    // that he got nothing for.. :p)
    message.cod = 0;

    // assign new delivery time
    message.delivery_time = message.items.empty() ? (uint32)UNIXTIME : (uint32)UNIXTIME + 3600;

    // add to the senders mailbox
    sMailSystem.DeliverMessage(message.player_guid, &message);

    // finish the packet
    data << uint32(MAIL_OK);
    SendPacket(&data);
}

void WorldSession::HandleMailCreateTextItem(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 mailbox;
    uint32 message_id;
    recv_data >> mailbox;
    recv_data >> message_id;

    WorldPacket data(SMSG_SEND_MAIL_RESULT, 12);
    data << message_id;
    data << uint32(MAIL_RES_MADE_PERMANENT);

    ItemProperties const* proto = sMySQLStore.GetItemProperties(8383);
    MailMessage* message = _player->m_mailBox.GetMessage(message_id);
    if (message == 0 || !proto)
    {
        data << uint32(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    SlotResult result = _player->GetItemInterface()->FindFreeInventorySlot(proto);
    if (result.Result == 0)
    {
        data << uint32(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    Item* pItem = objmgr.CreateItem(8383, _player);
    if (pItem == NULL)
        return;

    pItem->SetFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_WRAP_GIFT); // the flag is probably misnamed
    pItem->SetText(message->body);

    if (_player->GetItemInterface()->AddItemToFreeSlot(pItem))
    {
        data << uint32(MAIL_OK);
        SendPacket(&data);
    }
    else
    {
        pItem->DeleteMe();
    }
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

void WorldSession::HandleMailTime(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    WorldPacket data(MSG_QUERY_NEXT_MAIL_TIME, 100);
    _player->m_mailBox.FillTimePacket(data);
    SendPacket(&data);
}

void WorldSession::HandleGetMail(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    WorldPacket* data = _player->m_mailBox.BuildMailboxListingPacket();
    SendPacket(data);
    delete data;
}

WorldPacket* Mailbox::BuildMailboxListingPacket()
{
    WorldPacket* data = new WorldPacket(SMSG_MAIL_LIST_RESULT, 200);
    MessageMap::iterator itr;
    uint8 i = 0;
    uint32 realCount = 0;
    uint32 mailsCount = 0;
    uint32 t = (uint32)UNIXTIME;
    *data << uint32(0);     // real mail's count
    *data << uint8(0);      // mail's count

    for (itr = Messages.begin(); itr != Messages.end(); ++itr)
    {
        if (itr->second.expire_time && t > itr->second.expire_time)
            continue;       // expired mail -> skip it

        if ((uint32)UNIXTIME < itr->second.delivery_time)
            continue;        // undelivered

        if (mailsCount >= 50) //VLack: We could calculate message sizes instead of this, but the original code did a break at 50, so I won't fix this up if no one felt the need to do so before ;-)
        {
            realCount += 1;
            continue;
        }

        uint8 item_count = itr->second.items.size();            // max count is MAX_MAIL_ITEMS (12)

        size_t next_mail_size = 2 + 4 + 1 + (itr->second.message_type == MAIL_TYPE_NORMAL ? 8 : 4) + 4 * 8 + (itr->second.subject.size() + 1) + (itr->second.body.size() + 1) + 1 + item_count*(1 + 4 + 4 + MAX_INSPECTED_ENCHANTMENT_SLOT * 3 * 4 + 4 + 4 + 4 + 4 + 4 + 4 + 1);


        *data << uint16(next_mail_size);                    // Message size
        *data << uint32(itr->second.message_id);            // Message ID
        *data << uint8(itr->second.message_type);           // Message Type

        switch (itr->second.message_type)
        {
            case MAIL_TYPE_NORMAL:
                *data << uint64((itr->second.sender_guid));
                break;
            case MAIL_TYPE_COD:
            case MAIL_TYPE_AUCTION:
            case MAIL_TYPE_ITEM:
                *data << uint32(Arcemu::Util::GUID_LOPART((itr->second.sender_guid)));
                break;
            case MAIL_TYPE_GAMEOBJECT:
            case MAIL_TYPE_CREATURE:
                *data << uint32(static_cast<uint32>((itr->second.sender_guid)));
                break;
        }

        *data << uint64(itr->second.cod);
        *data << uint32(0);                                                 // Package.dbc ID ?
        *data << uint32(itr->second.stationery);                            // stationery (Stationery.dbc)
        *data << uint64(itr->second.money);
        *data << uint32(itr->second.checked_flag);
        *data << float(float((itr->second.expire_time - uint32(UNIXTIME)) / DAY));
        *data << uint32(itr->second.message_id);                            // mail template (MailTemplate.dbc)
        *data << itr->second.subject;                                       // Subject string - once 00, when mail type = 3, max 256
        *data << itr->second.body;                                          // message? max 8000
        *data << uint8(item_count);                                         // client limit is 0x10

        Item* pItem;
        std::vector<uint32>::iterator itr2;
        for (uint8 i = 0; i < item_count; ++i)
        {
            pItem = objmgr.LoadItem(itr->second.items[i]);
            *data << uint8(i);                                              // item index (0-6)
            *data << uint32((pItem ? pItem->GetLowGUID() : 0));
            *data << uint32((pItem ? pItem->GetEntry() : 0));
            for (uint8 j = 0; j < MAX_INSPECTED_ENCHANTMENT_SLOT; ++j)
            {
                *data << uint32((pItem ? pItem->GetEnchantmentId((EnchantmentSlot)j) : 0));
                *data << uint32((pItem ? pItem->GetEnchantmentDuration((EnchantmentSlot)j) : 0));
                *data << uint32((pItem ? pItem->GetEnchantmentCharges((EnchantmentSlot)j) : 0));
            }

            *data << int32((pItem ? pItem->GetItemRandomPropertyId() : 0)); // can be negative
            *data << uint32((pItem ? pItem->GetItemRandomSuffixFactor() : 0));
            *data << uint32((pItem ? pItem->GetStackCount() : 0));
            *data << uint32((pItem ? pItem->GetChargesLeft() : 0));
            *data << uint32((pItem ? pItem->GetDurabilityMax() : 0));
            *data << uint32((pItem ? pItem->GetDurability() : 0));
            *data << uint8(0);                                              // unknown wotlk
        }
        ++realCount;
        ++mailsCount;
    }

    data->put<uint32>(0, realCount);        // this will display warning about undelivered mail to player if realCount > mailsCount
    data->put<uint8>(4, mailsCount);        // set real send mails to client

    // do cleanup on request mail
    CleanupExpiredMessages();
    return data;
}

void WorldSession::SendMailError(uint32 error)
{
    WorldPacket data(SMSG_SEND_MAIL_RESULT, 12);
    data << uint32(0);
    data << uint32(MAIL_RES_MAIL_SENT);
    data << error;
    SendPacket(&data);
}
