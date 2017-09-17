/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "Objects/ObjectMgr.h"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"

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
    uint32_t map;
    float x, y, z;
    std::string message;
    std::string message2;

    recv_data >> map;
    recv_data >> x;
    recv_data >> y;
    recv_data >> z;
    recv_data >> message;
    recv_data >> message2;

    // Remove pending tickets
    objmgr.RemoveGMTicketByPlayer(GetPlayer()->GetGUID());

    GM_Ticket* ticket = new GM_Ticket;
    ticket->guid = uint64_t(objmgr.GenerateTicketID());
    ticket->playerGuid = GetPlayer()->GetGUID();
    ticket->map = map;
    ticket->posX = x;
    ticket->posY = y;
    ticket->posZ = z;
    ticket->message = message;
    ticket->timestamp = (uint32_t)UNIXTIME;
    ticket->name = GetPlayer()->GetName();
    ticket->level = GetPlayer()->getLevel();
    ticket->deleted = false;
    ticket->assignedToPlayer = 0;
    ticket->comment = "";

    objmgr.AddGMTicket(ticket, false);

    WorldPacket data(SMSG_GMTICKET_CREATE, 4);
    data << uint32_t(GMTNoErrors);
    SendPacket(&data);

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
    std::string message;
    recv_data >> message;

    WorldPacket data(SMSG_GMTICKET_UPDATETEXT, 4);

    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(GetPlayer()->GetGUID());
    if (ticket == nullptr)
    {
        data << uint32_t(GMTNoTicketFound);
    }
    else
    {
        ticket->message = message;
        ticket->timestamp = (uint32_t)UNIXTIME;
        objmgr.UpdateGMTicket(ticket);

        data << uint32_t(GMTNoErrors);
    }

    SendPacket(&data);

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

void WorldSession::HandleGMTicketDeleteOpcode(WorldPacket& recv_data)
{
    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(GetPlayer()->GetGUID());

    objmgr.RemoveGMTicketByPlayer(GetPlayer()->GetGUID());

    WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
    data << uint32_t(GMTTicketRemoved);
    SendPacket(&data);

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

    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(GetPlayer()->GetGUID());
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

    if (sWorld.getGmTicketStatus())
    {
        data << uint32_t(TicketSystemOK);
    }
    else
    {
        data << uint32_t(TicketSystemDisabled);
    }

    SendPacket(&data);
}
