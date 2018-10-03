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
#include "Server/Packets/SmsgGmTicketGetTicket.h"
#include "Server/Packets/SmsgGmTicketSystemstatus.h"
#include "Server/Packets/CmsgGmReportLag.h"
#include "Server/Packets/CmsgGmSurveySubmit.h"

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
    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(GetPlayer()->getGuid());
    if (ticket == nullptr)
        SendPacket(SmsgGmTicketGetTicket(GMTNoCurrentTicket, "", 0).serialise().get());
    else
        SendPacket(SmsgGmTicketGetTicket(GMTCurrentTicketFound, ticket->message, ticket->map).serialise().get());
}

void WorldSession::HandleGMTicketSystemStatusOpcode(WorldPacket& /*recv_data*/)
{
    SendPacket(SmsgGmTicketSystemstatus(sWorld.getGmTicketStatus() ? TicketSystemOK : TicketSystemDisabled).serialise().get());
}

void WorldSession::HandleGMTicketToggleSystemStatusOpcode(WorldPacket& /*recv_data*/)
{
    if (HasGMPermissions())
        sWorld.toggleGmTicketStatus();
}

void WorldSession::HandleReportLag(WorldPacket& recv_data)
{
    CmsgGmReportLag srlPacket;
    if (!srlPacket.deserialise(recv_data))
        return;

    if (GetPlayer() != nullptr)
    {
        CharacterDatabase.Execute("INSERT INTO lag_reports (player, account, lag_type, map_id, position_x, position_y, position_z) VALUES(%u, %u, %u, %u, %f, %f, %f)", 
            GetPlayer()->getGuidLow(), _accountId, srlPacket.lagType, srlPacket.mapId, srlPacket.location.x, srlPacket.location.y, srlPacket.location.z);
    }

    LogDebugFlag(LF_OPCODE, "Player %s has reported a lagreport with Type: %u on Map: %u", GetPlayer()->getName().c_str(), srlPacket.lagType, srlPacket.mapId);
}

void WorldSession::HandleGMSurveySubmitOpcode(WorldPacket& recv_data)
{
    CmsgGmSurveySubmit srlPacket;
    if (!srlPacket.deserialise(recv_data))
        return;

    QueryResult* result = CharacterDatabase.Query("SELECT MAX(survey_id) FROM gm_survey");
    if (result == nullptr)
        return;

    uint32_t next_survey_id = result->Fetch()[0].GetUInt32() + 1;

    for (auto subSurvey : srlPacket.subSurvey)
        CharacterDatabase.Execute("INSERT INTO gm_survey_answers VALUES(%u , %u , %u)",
            next_survey_id, subSurvey.subSurveyId, subSurvey.answerId);

    CharacterDatabase.Execute("INSERT INTO gm_survey VALUES (%u, %u, %u, \'%s\', UNIX_TIMESTAMP(NOW()))",
        next_survey_id, GetPlayer()->getGuidLow(), srlPacket.mainSurveyId, CharacterDatabase.EscapeString(srlPacket.mainComment).c_str());

    LogDebugFlag(LF_OPCODE, "Player %s has submitted the gm suvey %u successfully.",
        GetPlayer()->getName().c_str(), next_survey_id);
}
#endif
