/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Server/Packets/SmsgSendMailResult.h"

using namespace  AscEmu::Packets;

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
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_TOO_MANY_ATTACHMENTS).serialise().get());
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
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_RECIPIENT_NOT_FOUND).serialise().get());
        return;
    }

    for (uint8_t i = 0; i < items_count; ++i)
    {
        pItem = _player->GetItemInterface()->GetItemByGUID(itemGUIDs[i]);
        if (pItem == nullptr || pItem->isSoulbound() || pItem->hasFlags(ITEM_FLAG_CONJURED))
        {
            SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_INTERNAL_ERROR).serialise().get());
            return;
        }
        if (pItem->isAccountbound() && GetAccountId() != player_info->acct) // don't mail account-bound items to another account
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
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_NOT_YOUR_ALLIANCE).serialise().get());
        return;
    }

    // Check if we're sending mail to ourselves
    if (strcmp(player_info->name, _player->getName().c_str()) == 0 && !GetPermissionCount())
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
        SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_ERR_NOT_ENOUGH_MONEY).serialise().get());
        return;
    }

    // Check for the item, and required item.
    if (!items.empty())
    {
        for (std::vector< Item* >::iterator itr = items.begin(); itr != items.end(); ++itr)
        {
            pItem = *itr;
            if (_player->GetItemInterface()->SafeRemoveAndRetreiveItemByGuid(pItem->getGuid(), false) != pItem)
                continue;        // should never be hit.

            pItem->RemoveFromWorld();
            pItem->setOwner(NULL);
            pItem->SaveToDB(INVENTORY_SLOT_NOT_SET, 0, true, NULL);
            msg.items.push_back(pItem->getGuidLow());

            if (GetPermissionCount() > 0)
            {
                /* log the message */
                sGMLog.writefromsession(this, "sent mail with item entry %u to %s, with gold %u.", pItem->getEntry(), player_info->name, money);
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
    msg.sender_guid = _player->getGuid();
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

    SendPacket(SmsgSendMailResult(0, MAIL_RES_MAIL_SENT, MAIL_OK).serialise().get());
}

