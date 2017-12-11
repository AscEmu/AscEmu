/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"

void WorldSession::HandleSendMail(WorldPacket& recvData)
{
    MailMessage msg;
    ObjectGuid mailbox;
    uint32_t unk1, unk2;
    uint64_t money, COD;

    std::vector< Item* > items;
    Item* pItem;

    recvData >> unk1;
    recvData >> unk2;

    recvData >> COD;
    recvData >> money;

    uint32_t bodyLength = recvData.readBits(12);
    uint32_t subjectLength = recvData.readBits(9);

    uint8_t items_count = static_cast<uint8_t>(recvData.readBits(5));              // attached items count

    if (items_count > MAIL_MAX_ITEM_SLOT)
    {
        SendMailError(MAIL_ERR_TOO_MANY_ATTACHMENTS);
        return;
    }

    mailbox[0] = recvData.readBit();

    ObjectGuid itemGUIDs[MAIL_MAX_ITEM_SLOT];

    for (uint8_t i = 0; i < items_count; ++i)
    {
        itemGUIDs[i][2] = recvData.readBit();
        itemGUIDs[i][6] = recvData.readBit();
        itemGUIDs[i][3] = recvData.readBit();
        itemGUIDs[i][7] = recvData.readBit();
        itemGUIDs[i][1] = recvData.readBit();
        itemGUIDs[i][0] = recvData.readBit();
        itemGUIDs[i][4] = recvData.readBit();
        itemGUIDs[i][5] = recvData.readBit();
    }

    mailbox[3] = recvData.readBit();
    mailbox[4] = recvData.readBit();
    uint32_t receiverLength = recvData.readBits(7);
    mailbox[2] = recvData.readBit();
    mailbox[6] = recvData.readBit();
    mailbox[1] = recvData.readBit();
    mailbox[7] = recvData.readBit();
    mailbox[5] = recvData.readBit();

    recvData.ReadByteSeq(mailbox[4]);

    for (uint8_t i = 0; i < items_count; ++i)
    {
        recvData.ReadByteSeq(itemGUIDs[i][6]);
        recvData.ReadByteSeq(itemGUIDs[i][1]);
        recvData.ReadByteSeq(itemGUIDs[i][7]);
        recvData.ReadByteSeq(itemGUIDs[i][2]);
        recvData.read_skip<uint8_t>();            // item slot in mail, not used
        recvData.ReadByteSeq(itemGUIDs[i][3]);
        recvData.ReadByteSeq(itemGUIDs[i][0]);
        recvData.ReadByteSeq(itemGUIDs[i][4]);
        recvData.ReadByteSeq(itemGUIDs[i][5]);
    }

    recvData.ReadByteSeq(mailbox[7]);
    recvData.ReadByteSeq(mailbox[3]);
    recvData.ReadByteSeq(mailbox[6]);
    recvData.ReadByteSeq(mailbox[5]);

    std::string subject = recvData.ReadString(subjectLength);
    std::string receiver = recvData.ReadString(receiverLength);

    recvData.ReadByteSeq(mailbox[2]);
    recvData.ReadByteSeq(mailbox[0]);

    std::string body = recvData.ReadString(bodyLength);

    recvData.ReadByteSeq(mailbox[1]);

    // Search for the recipient
    PlayerInfo* player_info = ObjectMgr::getSingleton().GetPlayerInfoByName(receiver.c_str());
    if (!player_info)
    {
        SendMailError(MAIL_ERR_RECIPIENT_NOT_FOUND);
        return;
    }

    for (uint8_t i = 0; i < items_count; ++i)
    {
        pItem = _player->GetItemInterface()->GetItemByGUID(itemGUIDs[i]);
        if (pItem == nullptr || pItem->IsSoulbound() || pItem->IsConjured())
        {
            SendMailError(MAIL_ERR_INTERNAL_ERROR);
            return;
        }
        if (pItem->IsAccountbound() && GetAccountId() != player_info->acct) // don't mail account-bound items to another account
        {
            WorldPacket data(SMSG_SEND_MAIL_RESULT, 16);
            data << uint32_t(0);
            data << uint32_t(0);
            data << uint32_t(MAIL_ERR_BAG_FULL);
            data << uint32_t(INV_ERR_ARTEFACTS_ONLY_FOR_OWN_CHARACTERS);
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
    if (player_info->team != _player->GetTeam() && !interfaction)
    {
        SendMailError(MAIL_ERR_NOT_YOUR_ALLIANCE);
        return;
    }

    // Check if we're sending mail to ourselves
    if (strcmp(player_info->name, _player->GetName()) == 0 && !GetPermissionCount())
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
    msg.delivery_time = (uint32_t)UNIXTIME;

    // Set up the cost
    uint32_t cost = items_count ? 30 * items_count : 30;  // price hardcoded in client

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
        for (std::vector< Item* >::iterator itr = items.begin(); itr != items.end(); ++itr)
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
                sGMLog.writefromsession(this, "sent mail with item entry %u to %s, with gold %u.", pItem->GetEntry(), player_info->name, money);
            }

            pItem->DeleteMe();
        }
    }

    if (money != 0 || COD != 0 || (!items.size() && player_info->acct != _player->GetSession()->GetAccountId()))
    {
        if (!sMailSystem.MailOption(MAIL_FLAG_DISABLE_HOUR_DELAY_FOR_ITEMS))
            msg.delivery_time += 3600;  // 1hr
    }

    // take the money
    _player->ModGold(-static_cast<int32_t>(cost));

    // Fill in the rest of the info
    msg.player_guid = player_info->guid;
    msg.sender_guid = _player->GetGUID();
    msg.money = static_cast<uint32_t>(money);
    msg.cod = static_cast<uint32_t>(COD);
    msg.subject = subject;
    msg.body = body;

    // 30 day expiry time for unread mail
    if (!sMailSystem.MailOption(MAIL_FLAG_NO_EXPIRY))
        msg.expire_time = (uint32_t)UNIXTIME + (TIME_DAY * MAIL_DEFAULT_EXPIRATION_TIME);
    else
        msg.expire_time = 0;

    msg.deleted_flag = false;
    msg.message_type = 0;
    msg.checked_flag = msg.body.empty() ? MAIL_CHECK_MASK_COPIED : MAIL_CHECK_MASK_HAS_BODY;

    // Great, all our info is filled in. Now we can add it to the other players mailbox.
    sMailSystem.DeliverMessage(player_info->guid, &msg);
    // Save/Update character's gold if they've received gold that is. This prevents a rollback.
    CharacterDatabase.Execute("UPDATE characters SET gold = %u WHERE guid = %u", _player->GetGold(), _player->m_playerInfo->guid);
    // Success packet :)
    SendMailError(MAIL_OK);
}

void WorldSession::HandleMarkAsRead(WorldPacket& recvData)
{
    uint64_t mailbox;
    uint32_t message_id;
    recvData >> mailbox;
    recvData >> message_id;

    MailMessage* message = _player->m_mailBox.GetMessage(message_id);
    if (message == 0)
        return;

    // mark the message as read
    message->checked_flag |= MAIL_CHECK_MASK_READ;

    // mail now has a 30 day expiry time
    if (!sMailSystem.MailOption(MAIL_FLAG_NO_EXPIRY))
        message->expire_time = (uint32_t)UNIXTIME + (TIME_DAY * 30);

    // update it in sql
    CharacterDatabase.WaitExecute("UPDATE mailbox SET checked_flag = %u, expiry_time = %u WHERE message_id = %u",
                                  message->checked_flag, message->expire_time, message->message_id);
}

void WorldSession::HandleMailDelete(WorldPacket& recvData)
{
    uint64_t mailbox;
    uint32_t messageId;

    recvData >> mailbox;
    recvData >> messageId;
    recvData.read_skip<uint32_t>();

    WorldPacket data(SMSG_SEND_MAIL_RESULT, 12);
    data << messageId;
    data << uint32_t(MAIL_RES_DELETED);

    MailMessage* message = _player->m_mailBox.GetMessage(messageId);
    if (!message)
    {
        data << uint32_t(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    _player->m_mailBox.DeleteMessage(messageId, true);

    data << uint32_t(MAIL_OK);
    SendPacket(&data);
}

void WorldSession::HandleTakeItem(WorldPacket& recvData)
{
    uint64_t mailbox;
    uint32_t message_id;
    uint32_t lowguid;
    std::vector< uint32_t >::iterator itr;

    recvData >> mailbox;
    recvData >> message_id;
    recvData >> lowguid;

    WorldPacket data(SMSG_SEND_MAIL_RESULT, 12);
    data << message_id;
    data << uint32_t(MAIL_RES_ITEM_TAKEN);

    MailMessage* message = _player->m_mailBox.GetMessage(message_id);
    if (!message || message->items.empty())
    {
        data << uint32_t(MAIL_ERR_INTERNAL_ERROR);
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
        data << uint32_t(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    if (message->cod > 0)   // check for cod credit
    {
        if (!_player->HasGold(message->cod))
        {
            data << uint32_t(MAIL_ERR_NOT_ENOUGH_MONEY);
            SendPacket(&data);
            return;
        }
    }

    Item* item = objmgr.LoadItem(*itr);
    if (!item)
    {
        data << uint32_t(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    //Find free slot
    SlotResult result = _player->GetItemInterface()->FindFreeInventorySlot(item->GetItemProperties());
    if (result.Result == false) //End of slots
    {
        data << uint32_t(MAIL_ERR_BAG_FULL);
        SendPacket(&data);

        item->DeleteMe();
        return;
    }
    item->m_isDirty = true;

    if (!_player->GetItemInterface()->SafeAddItem(item, result.ContainerSlot, result.Slot))
    {
        if (!_player->GetItemInterface()->AddItemToFreeSlot(item))   //End of slots
        {
            data << uint32_t(MAIL_ERR_BAG_FULL);
            SendPacket(&data);
            item->DeleteMe();
            return;
        }
    }
    else
    {
        item->SaveToDB(result.ContainerSlot, result.Slot, true, nullptr);
    }

    data << uint32_t(MAIL_OK);
    data << item->GetLowGUID();
    data << item->GetStackCount();

    message->items.erase(itr);

    // re-save (update the items field)
    sMailSystem.SaveMessageToSQL(message);
    SendPacket(&data);

    // Send message back if cod was set.
    if (message->cod > 0)
    {
        _player->ModGold(-(int32_t)message->cod);
        std::string subject = "COD Payment: ";
        subject += message->subject;
        uint64_t answer_sender = message->player_guid;
        uint64_t answer_receiver = message->sender_guid;
        uint32_t answer_cod_money = message->cod;

        sMailSystem.SendAutomatedMessage(MAIL_TYPE_NORMAL, answer_sender, answer_receiver, subject, "", answer_cod_money, 0, 0, MAIL_STATIONERY_TEST1, MAIL_CHECK_MASK_COD_PAYMENT);

        message->cod = 0;
        CharacterDatabase.Execute("UPDATE mailbox SET cod = 0 WHERE message_id = %u", message->message_id);
    }

    // probably need to send an item push here
}

void WorldSession::HandleTakeMoney(WorldPacket& recvData)
{
    uint64_t mailbox;
    uint32_t messageId;

    recvData >> mailbox;
    recvData >> messageId;

    WorldPacket data(SMSG_SEND_MAIL_RESULT, 12);
    data << messageId;
    data << uint32_t(MAIL_RES_MONEY_TAKEN);

    MailMessage* message = _player->m_mailBox.GetMessage(messageId);
    if (!message || !message->money)
    {
        data << uint32_t(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    // Check they don't have more than the max gold
    if (sWorld.settings.player.isGoldCapEnabled)
    {
        if ((_player->GetGold() + message->money) > sWorld.settings.player.limitGoldAmount)
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
    data << uint32_t(MAIL_OK);
    SendPacket(&data);
}

void WorldSession::HandleReturnToSender(WorldPacket& recvData)
{
    uint64_t mailbox;
    uint32_t messageId;

    recvData >> mailbox;
    recvData >> messageId;

    WorldPacket data(SMSG_SEND_MAIL_RESULT, 12);
    data << messageId;
    data << uint32_t(MAIL_RES_RETURNED_TO_SENDER);

    MailMessage* msg = _player->m_mailBox.GetMessage(messageId);
    if (!msg)
    {
        data << uint32_t(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    // copy into a new struct
    MailMessage message = *msg;

    // remove the old message
    _player->m_mailBox.DeleteMessage(messageId, true);

    // re-assign the owner/sender
    message.player_guid = message.sender_guid;
    message.sender_guid = _player->GetGUID();

    message.deleted_flag = false;
    message.checked_flag = MAIL_CHECK_MASK_RETURNED;

    // null out the cod charges. (the sender doesn't want to have to pay for his own item
    // that he got nothing for.. :p)
    message.cod = 0;

    // assign new delivery time
    message.delivery_time = message.items.empty() ? (uint32_t)UNIXTIME : (uint32_t)UNIXTIME + 3600;

    // add to the senders mailbox
    sMailSystem.DeliverMessage(message.player_guid, &message);

    // finish the packet
    data << uint32_t(MAIL_OK);
    SendPacket(&data);
}

void WorldSession::HandleMailCreateTextItem(WorldPacket& recvData)
{
    uint64_t mailbox;
    uint32_t messageId;

    recvData >> mailbox;
    recvData >> messageId;

    WorldPacket data(SMSG_SEND_MAIL_RESULT, 12);
    data << messageId;
    data << uint32_t(MAIL_RES_MADE_PERMANENT);

    ItemProperties const* item_properties = sMySQLStore.getItemProperties(8383);
    MailMessage* message = _player->m_mailBox.GetMessage(messageId);
    if (!message || !item_properties)
    {
        data << uint32_t(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    SlotResult result = _player->GetItemInterface()->FindFreeInventorySlot(item_properties);
    if (result.Result == false)
    {
        data << uint32_t(MAIL_ERR_INTERNAL_ERROR);
        SendPacket(&data);
        return;
    }

    Item* pItem = objmgr.CreateItem(8383, _player);
    if (!pItem)
        return;

    pItem->SetFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_WRAP_GIFT); // the flag is probably misnamed
    pItem->SetText(message->body);

    if (_player->GetItemInterface()->AddItemToFreeSlot(pItem))
    {
        data << uint32_t(MAIL_OK);
        SendPacket(&data);
    }
    else
    {
        pItem->DeleteMe();
    }
}

void WorldSession::HandleItemTextQuery(WorldPacket& recvData)
{
    uint64_t itemGuid;

    recvData >> itemGuid;

    Item* pItem = _player->GetItemInterface()->GetItemByGUID(itemGuid);
    WorldPacket data(SMSG_ITEM_TEXT_QUERY_RESPONSE, pItem->GetText().size() + 9);
    if (!pItem)
    {
        data << uint8_t(1);
    }
    else
    {
        data << uint8_t(0);
        data << uint64_t(itemGuid);
        data << pItem->GetText();
    }

    SendPacket(&data);
}

void Mailbox::FillTimePacket(WorldPacket& data)
{
    uint32_t c = 0;
    MessageMap::iterator iter = Messages.begin();
    data << uint32_t(0);
    data << uint32_t(0);

    for (; iter != Messages.end(); ++iter)
    {
        if (iter->second.checked_flag & MAIL_CHECK_MASK_READ)
            continue;

        if (iter->second.deleted_flag == 0 && (uint32_t)UNIXTIME >= iter->second.delivery_time)
        {
            ++c;
            data << uint64_t(iter->second.sender_guid);
            data << uint32_t(0);
            data << uint32_t(0);
            data << uint32_t(iter->second.stationery);
            data << float(-9.0f);
        }
    }

    if (c == 0)
        *(uint32_t*)(&data.contents()[0]) = 0xc7a8c000;
    else
        *(uint32_t*)(&data.contents()[4]) = c;

}

void WorldSession::HandleMailTime(WorldPacket& /*recvData*/)
{
    WorldPacket data(MSG_QUERY_NEXT_MAIL_TIME, 100);
    _player->m_mailBox.FillTimePacket(data);
    SendPacket(&data);
}

void WorldSession::HandleGetMail(WorldPacket& /*recvData*/)
{
    WorldPacket* data = _player->m_mailBox.BuildMailboxListingPacket();
    SendPacket(data);
    delete data;
}

WorldPacket* Mailbox::BuildMailboxListingPacket()
{
    WorldPacket* data = new WorldPacket(SMSG_MAIL_LIST_RESULT, 200);
    uint32_t realCount = 0;
    uint8_t mailsCount = 0;
    uint32_t t = (uint32_t)UNIXTIME;
    *data << uint32_t(0);     // real mail's count
    *data << uint8_t(0);      // mail's count

    for (MessageMap::iterator itr = Messages.begin(); itr != Messages.end(); ++itr)
    {
        if (itr->second.expire_time && t > itr->second.expire_time)
            continue;       // expired mail -> skip it

        if ((uint32_t)UNIXTIME < itr->second.delivery_time)
            continue;        // undelivered

        if (mailsCount >= 50)
        {
            realCount += 1;
            continue;
        }

        uint8_t item_count = static_cast<uint8_t>(itr->second.items.size());

        size_t next_mail_size = 2 + 4 + 1 + (itr->second.message_type == MAIL_TYPE_NORMAL ? 8 : 4) + 4 * 8 + (itr->second.subject.size() + 1) + (itr->second.body.size() + 1) + 1 + item_count*(1 + 4 + 4 + MAX_INSPECTED_ENCHANTMENT_SLOT * 3 * 4 + 4 + 4 + 4 + 4 + 4 + 4 + 1);


        *data << uint16_t(next_mail_size);                    // Message size
        *data << uint32_t(itr->second.message_id);            // Message ID
        *data << uint8_t(itr->second.message_type);           // Message Type

        switch (itr->second.message_type)
        {
            case MAIL_TYPE_NORMAL:
                *data << uint64_t((itr->second.sender_guid));
                break;
            case MAIL_TYPE_COD:
            case MAIL_TYPE_AUCTION:
            case MAIL_TYPE_ITEM:
                *data << uint32_t(Arcemu::Util::GUID_LOPART((itr->second.sender_guid)));
                break;
            case MAIL_TYPE_GAMEOBJECT:
            case MAIL_TYPE_CREATURE:
                *data << uint32_t(static_cast<uint32_t>((itr->second.sender_guid)));
                break;
        }

        *data << uint64_t(itr->second.cod);
        *data << uint32_t(0);                                                 // Package.dbc ID ?
        *data << uint32_t(itr->second.stationery);                            // stationery (Stationery.dbc)
        *data << uint64_t(itr->second.money);
        *data << uint32_t(itr->second.checked_flag);
        *data << float(float((itr->second.expire_time - uint32_t(UNIXTIME)) / DAY));
        *data << uint32_t(itr->second.message_id);                            // mail template (MailTemplate.dbc)
        *data << itr->second.subject;                                       // Subject string - once 00, when mail type = 3, max 256
        *data << itr->second.body;                                          // message? max 8000
        *data << uint8_t(item_count);                                         // client limit is 0x10

        std::vector<uint32_t>::iterator itr2;
        for (uint8_t i = 0; i < item_count; ++i)
        {
            Item * pItem = objmgr.LoadItem(itr->second.items[i]);
            *data << uint8_t(i);                                              // item index (0-6)
            *data << uint32_t((pItem ? pItem->GetLowGUID() : 0));
            *data << uint32_t((pItem ? pItem->GetEntry() : 0));
            for (uint8_t j = 0; j < MAX_INSPECTED_ENCHANTMENT_SLOT; ++j)
            {
                *data << uint32_t((pItem ? pItem->GetEnchantmentId((EnchantmentSlot)j) : 0));
                *data << uint32_t((pItem ? pItem->GetEnchantmentDuration((EnchantmentSlot)j) : 0));
                *data << uint32_t((pItem ? pItem->GetEnchantmentCharges((EnchantmentSlot)j) : 0));
            }

            *data << int32_t((pItem ? pItem->GetItemRandomPropertyId() : 0)); // can be negative
            *data << uint32_t((pItem ? pItem->GetItemRandomSuffixFactor() : 0));
            *data << uint32_t((pItem ? pItem->GetStackCount() : 0));
            *data << uint32_t((pItem ? pItem->GetChargesLeft() : 0));
            *data << uint32_t((pItem ? pItem->GetDurabilityMax() : 0));
            *data << uint32_t((pItem ? pItem->GetDurability() : 0));
            *data << uint8_t(0);
        }
        ++realCount;
        ++mailsCount;
    }

    data->put<uint32_t>(0, realCount);        // this will display warning about undelivered mail to player if realCount > mailsCount
    data->put<uint8_t>(4, mailsCount);        // set real send mails to client

                                            // do cleanup on request mail
    CleanupExpiredMessages();
    return data;
}

void WorldSession::SendMailError(uint32_t error)
{
    WorldPacket data(SMSG_SEND_MAIL_RESULT, 12);
    data << uint32_t(0);
    data << uint32_t(MAIL_RES_MAIL_SENT);
    data << uint32_t(error);
    SendPacket(&data);
}
