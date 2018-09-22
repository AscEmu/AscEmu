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
#include "Server/MainServerDefines.h"
#include "Objects/ObjectMgr.h"

initialiseSingleton(MailSystem);

/// \todo refactoring
void MailSystem::StartMailSystem()
{}

MailError MailSystem::DeliverMessage(uint64 recipent, MailMessage* message)
{
    // assign a new id
    message->message_id = objmgr.GenerateMailID();

    Player* plr = objmgr.GetPlayer((uint32)recipent);
    if (plr != NULL)
    {
        plr->m_mailBox.AddMessage(message);
        if ((uint32)UNIXTIME >= message->delivery_time)
        {
            uint32 v = 0;
            plr->GetSession()->OutPacket(SMSG_RECEIVED_MAIL, 4, &v);
        }
    }

    SaveMessageToSQL(message);
    return MAIL_OK;
}

void Mailbox::AddMessage(MailMessage* Message)
{
    Messages[Message->message_id] = *Message;
}

void Mailbox::DeleteMessage(uint32 MessageId, bool sql)
{
    Messages.erase(MessageId);
    if (sql)
        CharacterDatabase.WaitExecute("DELETE FROM mailbox WHERE message_id = %u", MessageId);
}


void Mailbox::CleanupExpiredMessages()
{
    MessageMap::iterator itr, it2;
    uint32 curtime = (uint32)UNIXTIME;

    for (itr = Messages.begin(); itr != Messages.end();)
    {
        it2 = itr++;
        if (it2->second.expire_time && it2->second.expire_time < curtime)
        {
            Messages.erase(it2);
        }
    }
}

void MailSystem::SaveMessageToSQL(MailMessage* message)
{
    std::stringstream ss;

    ss << "DELETE FROM mailbox WHERE message_id = ";
    ss << message->message_id;
    ss << ";";

    CharacterDatabase.ExecuteNA(ss.str().c_str());

    ss.rdbuf()->str("");

    std::vector< uint32 >::iterator itr;
    ss << "INSERT INTO mailbox VALUES("
        << message->message_id << ","
        << message->message_type << ","
        << message->player_guid << ","
        << message->sender_guid << ",\'"
        << CharacterDatabase.EscapeString(message->subject) << "\',\'"
        << CharacterDatabase.EscapeString(message->body) << "\',"
        << message->money << ",'";

    for (itr = message->items.begin(); itr != message->items.end(); ++itr)
        ss << (*itr) << ",";

    ss << "',"
        << message->cod << ","
        << message->stationery << ","
        << message->expire_time << ","
        << message->delivery_time << ","
        << message->checked_flag << ","
        << message->deleted_flag << ");";

    CharacterDatabase.ExecuteNA(ss.str().c_str());
}

void MailSystem::RemoveMessageIfDeleted(uint32 message_id, Player* plr)
{
    MailMessage* msg = plr->m_mailBox.GetMessage(message_id);
    if (msg == 0) return;

    if (msg->deleted_flag)   // we've deleted from inbox
        plr->m_mailBox.DeleteMessage(message_id, true);   // wipe the message
}

void MailSystem::SendAutomatedMessage(uint32 type, uint64 sender, uint64 receiver, std::string subject, std::string body,
                                      uint32 money, uint32 cod, std::vector<uint64> &item_guids, uint32 stationery, MailCheckMask checked, uint32 deliverdelay)
{
    // This is for sending automated messages, for example from an auction house.
    MailMessage msg;
    msg.message_type = type;
    msg.sender_guid = sender;
    msg.player_guid = receiver;
    msg.subject = subject;
    msg.body = body;
    msg.money = money;
    msg.cod = cod;
    for (std::vector<uint64>::iterator itr = item_guids.begin(); itr != item_guids.end(); ++itr)
        msg.items.push_back(Arcemu::Util::GUID_LOPART(*itr));

    msg.stationery = stationery;
    msg.delivery_time = (uint32)UNIXTIME + deliverdelay;

    // 30 days expiration time for unread mail + possible delivery delay.
    if (!sMailSystem.MailOption(MAIL_FLAG_NO_EXPIRY))
        msg.expire_time = (uint32)UNIXTIME + deliverdelay + (TIME_DAY * MAIL_DEFAULT_EXPIRATION_TIME);
    else
        msg.expire_time = 0;

    msg.deleted_flag = false;
    msg.checked_flag = checked;

    // Send the message.
    DeliverMessage(receiver, &msg);
}

//overload to keep backward compatibility (passing just 1 item guid instead of a vector)
void MailSystem::SendAutomatedMessage(uint32 type, uint64 sender, uint64 receiver, std::string subject, std::string body, uint32 money,
                                      uint32 cod, uint64 item_guid, uint32 stationery, MailCheckMask checked, uint32 deliverdelay)
{
    std::vector<uint64> item_guids;
    if (item_guid != 0)
        item_guids.push_back(item_guid);
    SendAutomatedMessage(type, sender, receiver, subject, body, money, cod, item_guids, stationery, checked, deliverdelay);
}

void MailSystem::SendCreatureGameobjectMail(uint32 type, uint32 sender, uint64 receiver, std::string subject, std::string body, uint32 money,
                                      uint32 cod, uint64 item_guid, uint32 stationery, MailCheckMask checked, uint32 deliverdelay)
{
    std::vector<uint64> item_guids;
    if (item_guid != 0)
        item_guids.push_back(item_guid);
    SendAutomatedMessage(type, sender, receiver, subject, body, money, cod, item_guids, stationery, checked, deliverdelay);
}

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

#if VERSION_STRING != Cata
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
#else
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

        size_t next_mail_size = 2 + 4 + 1 + (itr->second.message_type == MAIL_TYPE_NORMAL ? 8 : 4) + 4 * 8 + (itr->second.subject.size() + 1) + (itr->second.body.size() + 1) + 1 + item_count * (1 + 4 + 4 + MAX_INSPECTED_ENCHANTMENT_SLOT * 3 * 4 + 4 + 4 + 4 + 4 + 4 + 4 + 1);


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
            *data << uint32_t((pItem ? pItem->getGuidLow() : 0));
            *data << uint32_t((pItem ? pItem->getEntry() : 0));
            for (uint8_t j = 0; j < MAX_INSPECTED_ENCHANTMENT_SLOT; ++j)
            {
                *data << uint32_t((pItem ? pItem->getEnchantmentId((EnchantmentSlot)j) : 0));
                *data << uint32_t((pItem ? pItem->getEnchantmentDuration((EnchantmentSlot)j) : 0));
                *data << uint32_t((pItem ? pItem->getEnchantmentCharges((EnchantmentSlot)j) : 0));
            }

            *data << int32_t((pItem ? pItem->getRandomPropertiesId() : 0)); // can be negative
            *data << uint32_t((pItem ? pItem->getPropertySeed() : 0));
            *data << uint32_t((pItem ? pItem->getStackCount() : 0));
            *data << uint32_t((pItem ? pItem->GetChargesLeft() : 0));
            *data << uint32_t((pItem ? pItem->getMaxDurability() : 0));
            *data << uint32_t((pItem ? pItem->getDurability() : 0));
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
#endif

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

void Mailbox::Load(QueryResult* result)
{
    if (!result)
        return;

    Field* fields;
    MailMessage msg;
    uint32 i;
    char* str;
    char* p;
    uint32 itemguid;
    uint32 now = (uint32)UNIXTIME;

    do
    {
        fields = result->Fetch();
        uint32 expiry_time = fields[10].GetUInt32();

        // Do not load expired mails!
        if (expiry_time < now)
            continue;

        // Create message struct
        i = 0;
        msg.items.clear();
        msg.message_id = fields[i++].GetUInt32();
        msg.message_type = fields[i++].GetUInt32();
        msg.player_guid = fields[i++].GetUInt32();
        msg.sender_guid = fields[i++].GetUInt32();
        msg.subject = fields[i++].GetString();
        msg.body = fields[i++].GetString();
        msg.money = fields[i++].GetUInt32();
        str = (char*)fields[i++].GetString();
        p = strchr(str, ',');
        if (p == NULL)
        {
            itemguid = atoi(str);
            if (itemguid != 0)
                msg.items.push_back(itemguid);
        }
        else
        {
            while (p)
            {
                *p = 0;
                p++;

                itemguid = atoi(str);
                if (itemguid != 0)
                    msg.items.push_back(itemguid);

                str = p;
                p = strchr(str, ',');
            }
        }

        msg.cod = fields[i++].GetUInt32();
        msg.stationery = fields[i++].GetUInt32();
        msg.expire_time = fields[i++].GetUInt32();
        msg.delivery_time = fields[i++].GetUInt32();
        msg.checked_flag = fields[i++].GetUInt32();
        msg.deleted_flag = fields[i++].GetBool();

        // Add to the mailbox
        AddMessage(&msg);

    }
    while (result->NextRow());
}
