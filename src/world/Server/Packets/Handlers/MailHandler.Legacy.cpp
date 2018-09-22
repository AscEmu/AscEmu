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
#include "Server/Packets/CmsgMailReturnToSender.h"
#include "Server/Packets/CmsgMailCreateTextItem.h"
#include "Server/Packets/CmsgItemTextQuery.h"
#include "Server/Packets/CmsgSendMail.h"

using namespace  AscEmu::Packets;

// \todo refactoring
void WorldSession::handleSendMailOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CmsgSendMail srlPacket;
    if (!srlPacket.deserialise(recv_data))
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    Item* pItem;

    if (srlPacket.itemCount > MAIL_MAX_ITEM_SLOT)
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_TOO_MANY_ATTACHMENTS).serialise().get());
        return;
    }

    // Search for the recipient
    PlayerInfo* player = objmgr.GetPlayerInfoByName(srlPacket.receiverName.c_str());
    if (player == nullptr)
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_RECIPIENT_NOT_FOUND).serialise().get());
        return;
    }

    std::vector<Item*> items;
    for (uint8 i = 0; i < srlPacket.itemCount; ++i)
    {
        pItem = _player->GetItemInterface()->GetItemByGUID(srlPacket.itemGuid[i]);
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

    if (srlPacket.stationery == MAIL_STATIONERY_GM && !HasGMPermissions())
    {
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
        return;
    }

    // Set up the cost
    uint32_t cost = 0;

    MailMessage msg;

    // Check for attached money	
    if (srlPacket.money > 0)
        cost += srlPacket.money;

    if (!sMailSystem.MailOption(MAIL_FLAG_DISABLE_POSTAGE_COSTS) && !(GetPermissionCount() && sMailSystem.MailOption(MAIL_FLAG_NO_COST_FOR_GM)))
    {
        cost += srlPacket.itemCount ? 30 * srlPacket.itemCount : 30;;
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
        for (auto& item : items)
        {
            pItem = item;
            if (_player->GetItemInterface()->SafeRemoveAndRetreiveItemByGuid(pItem->getGuid(), false) != pItem)
                continue;        // should never be hit.

            pItem->RemoveFromWorld();
            pItem->setOwner(nullptr);
            pItem->SaveToDB(INVENTORY_SLOT_NOT_SET, 0, true, nullptr);
            msg.items.push_back(pItem->getGuidLow());

            if (GetPermissionCount() > 0)
            {
                /* log the message */
                sGMLog.writefromsession(this, "sent mail with item entry %u to %s, with gold %u.", pItem->getEntry(), player->name, srlPacket.money);
            }

            pItem->DeleteMe();
        }
    }

    // Instant delivery time by default.
    msg.delivery_time = static_cast<uint32>(UNIXTIME);

    if (srlPacket.money != 0 || srlPacket.cod != 0 || items.empty() && player->acct != _player->GetSession()->GetAccountId())
    {
        if (!sMailSystem.MailOption(MAIL_FLAG_DISABLE_HOUR_DELAY_FOR_ITEMS))
            msg.delivery_time += 3600;  // 1hr
    }

    // take the money
    _player->ModGold(-static_cast<int32_t>(cost));

    // Fill in the rest of the info
    msg.player_guid = player->guid;
    msg.sender_guid = _player->getGuid();
    msg.stationery = srlPacket.stationery;
    msg.money = static_cast<uint32_t>(srlPacket.money);
    msg.cod = static_cast<uint32_t>(srlPacket.cod);
    msg.subject = srlPacket.subject;
    msg.body = srlPacket.body;

    // 30 day expiry time for unread mail
    if (!sMailSystem.MailOption(MAIL_FLAG_NO_EXPIRY))
        msg.expire_time = static_cast<uint32>(UNIXTIME) + (TIME_DAY * MAIL_DEFAULT_EXPIRATION_TIME);
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
