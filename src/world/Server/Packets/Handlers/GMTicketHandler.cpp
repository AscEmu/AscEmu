/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "Objects/ObjectMgr.h"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Server/Packets/CmsgGmTicketCreate.h"
#include "Server/Packets/SmsgGmTicketCreate.h"
#include "Server/Packets/CmsgGmTicketUpdateText.h"
#include "Server/Packets/SmsgGmTicketUpdateText.h"
#include "Server/Packets/SmsgGmTicketDeleteTicket.h"

using namespace AscEmu::Packets;

#if VERSION_STRING != Cata
enum GMTicketResults
{
    GMTNoTicketFound = 1,
    GMTNoErrors = 2,
    GMTCurrentTicketFound = 6,
    GMTTicketRemoved = 9,
    GMTNoCurrentTicket = 10
};

enum GMTicketSystem
{
    TicketSystemDisabled = 0,
    TicketSystemOK = 1
};

void WorldSession::HandleGMTicketCreateOpcode(WorldPacket& recv_data)
{
    CmsgGmTicketCreate srlPacket;
    if (!srlPacket.deserialise(recv_data))
        return;

    // Remove pending tickets
    objmgr.RemoveGMTicketByPlayer(GetPlayer()->getGuid());

    GM_Ticket* ticket = new GM_Ticket;
    ticket->guid = uint64_t(objmgr.GenerateTicketID());
    ticket->playerGuid = GetPlayer()->getGuid();
    ticket->map = srlPacket.map;
    ticket->posX = srlPacket.location.x;
    ticket->posY = srlPacket.location.y;
    ticket->posZ = srlPacket.location.z;
    ticket->message = srlPacket.message;
    ticket->timestamp = (uint32_t)UNIXTIME;
    ticket->name = GetPlayer()->getName().c_str();
    ticket->level = GetPlayer()->getLevel();
    ticket->deleted = false;
    ticket->assignedToPlayer = 0;
    ticket->comment = "";

    objmgr.AddGMTicket(ticket, false);

    SendPacket(SmsgGmTicketCreate(GMTNoErrors).serialise().get());

    // send message indicating new ticket
    Channel* channel = channelmgr.GetChannel(worldConfig.getGmClientChannelName().c_str(), GetPlayer());
    if (channel != nullptr)
    {
        std::stringstream ss;
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        ss << "GmTicket 5, ";
        ss << ticket->name;
#else
        ss << "GmTicket:";
        ss << GM_TICKET_CHAT_OPCODE_NEWTICKET;
        ss << ":" << ticket->guid;
        ss << ":" << ticket->level;
        ss << ":" << ticket->name;
#endif
        channel->Say(_player, ss.str().c_str(), nullptr, true);
    }
}

void WorldSession::HandleGMTicketUpdateOpcode(WorldPacket& recv_data)
{
    CmsgGmTicketUpdateText srlPacket;
    if (!srlPacket.deserialise(recv_data))
        return;

    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(GetPlayer()->getGuid());
    if (ticket == nullptr)
    {
        SendPacket(SmsgGmTicketUpdateText(GMTNoTicketFound).serialise().get());
    }
    else
    {
        ticket->message = srlPacket.message;
        ticket->timestamp = static_cast<uint32_t>(UNIXTIME);
        objmgr.UpdateGMTicket(ticket);

        SendPacket(SmsgGmTicketUpdateText(GMTNoErrors).serialise().get());
    }

#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
    Channel* channel = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), GetPlayer());
    if (channel != nullptr)
    {
        std::stringstream ss;
        ss << "GmTicket:";
        ss<< GM_TICKET_CHAT_OPCODE_UPDATED;
        ss << ":";
        ss<< ticket->guid;
        channel->Say(_player, ss.str().c_str(), nullptr, true);
    }
#endif
}

void WorldSession::HandleGMTicketDeleteOpcode(WorldPacket& /*recv_data*/)
{
    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(GetPlayer()->getGuid());

    objmgr.RemoveGMTicketByPlayer(GetPlayer()->getGuid());

    SendPacket(SmsgGmTicketDeleteTicket(GMTTicketRemoved).serialise().get());

    Channel* channel = channelmgr.GetChannel(worldConfig.getGmClientChannelName().c_str(), GetPlayer());
    if (channel != nullptr && ticket != nullptr)
    {
        std::stringstream ss;
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        ss << "GmTicket 1,";
        ss << ticket->name;
#else
        ss << "GmTicket:";
        ss << GM_TICKET_CHAT_OPCODE_REMOVED;
        ss << ":";
        ss << ticket->guid;
#endif
        channel->Say(_player, ss.str().c_str(), nullptr, true);
    }
}

void WorldSession::HandleGMTicketGetTicketOpcode(WorldPacket& /*recv_data*/)
{
    WorldPacket data(SMSG_GMTICKET_GETTICKET, 400);

    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(GetPlayer()->getGuid());
    if (ticket == nullptr)
    {
        data << uint32_t(GMTNoCurrentTicket);
    }
    else
    {
        data << uint32_t(GMTCurrentTicketFound);
        data << ticket->message.c_str();
        data << (uint8_t)ticket->map;
    }

    SendPacket(&data);
}

void WorldSession::HandleGMTicketSystemStatusOpcode(WorldPacket& /*recv_data*/)
{
    WorldPacket data(SMSG_GMTICKET_SYSTEMSTATUS, 4);
    data << uint32_t(sWorld.getGmTicketStatus() ? TicketSystemOK : TicketSystemDisabled);
    SendPacket(&data);
}

void WorldSession::HandleGMTicketToggleSystemStatusOpcode(WorldPacket& /*recv_data*/)
{
    if (HasGMPermissions())
    {
        sWorld.toggleGmTicketStatus();
    }
}

void WorldSession::HandleReportLag(WorldPacket& recv_data)
{
    uint32_t lagType;
    uint32_t mapId;
    float position_x;
    float position_y;
    float position_z;

    recv_data >> lagType;
    recv_data >> mapId;
    recv_data >> position_x;
    recv_data >> position_y;
    recv_data >> position_z;

    if (GetPlayer() != nullptr)
    {
        CharacterDatabase.Execute("INSERT INTO lag_reports (player, account, lag_type, map_id, position_x, position_y, position_z) VALUES(%u, %u, %u, %u, %f, %f, %f)", GetPlayer()->getGuidLow(), _accountId, lagType, mapId, position_x, position_y, position_z);
    }

    LogDebugFlag(LF_OPCODE, "Player %s has reported a lagreport with Type: %u on Map: %u", GetPlayer()->getName().c_str(), lagType, mapId);
}

void WorldSession::HandleGMSurveySubmitOpcode(WorldPacket& recv_data)
{
    QueryResult* result = CharacterDatabase.Query("SELECT MAX(survey_id) FROM gm_survey");
    if (result == nullptr)
        return;

    uint32_t next_survey_id = result->Fetch()[0].GetUInt32() + 1;

    uint32_t main_survey;
    recv_data >> main_survey;

    std::unordered_set<uint32> survey_ids;
    for (uint8_t i = 0; i < 10; ++i)
    {
        uint32_t sub_survey_id;
        recv_data >> sub_survey_id;
        if (sub_survey_id == 0)
            break;

        uint8_t answer_id;
        recv_data >> answer_id;

        std::string comment; // unused empty string
        recv_data >> comment;

        if (!survey_ids.insert(sub_survey_id).second)
            continue;

        CharacterDatabase.Execute("INSERT INTO gm_survey_answers VALUES(%u , %u , %u)", next_survey_id, sub_survey_id, answer_id);
    }

    std::string comment; // receive the player comment for this survey
    recv_data >> comment;

    CharacterDatabase.Execute("INSERT INTO gm_survey VALUES (%u, %u, %u, \'%s\', UNIX_TIMESTAMP(NOW()))", next_survey_id, GetPlayer()->getGuidLow(), main_survey, CharacterDatabase.EscapeString(comment).c_str());

    LogDebugFlag(LF_OPCODE, "Player %s has submitted the gm suvey %u successfully.", GetPlayer()->getName().c_str(), next_survey_id);
}
#endif
