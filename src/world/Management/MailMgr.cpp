/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
        msg.items.push_back(WoWGuid::getGuidLowPartFromUInt64(*itr));

    msg.stationery = stationery;
    msg.delivery_time = (uint32)UNIXTIME + deliverdelay;

    // 30 days expiration time for unread mail + possible delivery delay.
    if (!sMailSystem.MailOption(MAIL_FLAG_NO_EXPIRY))
        msg.expire_time = (uint32)UNIXTIME + deliverdelay + (TimeVars::Day * MAIL_DEFAULT_EXPIRATION_TIME);
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
