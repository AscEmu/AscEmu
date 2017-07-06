/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "Objects/ObjectMgr.h"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"


void WorldSession::HandleGMTicketGetTicketOpcode(WorldPacket& /*recv_data*/)
{
    if (GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(GetPlayer()->GetGUID()))
    {
        if (ticket->deleted == false)
        {
            WorldPacket data(SMSG_GMTICKET_GETTICKET, 4 + 4 + 1 + 4 + 4 + 4 + 1 + 1);

            data << uint32_t(6);
            data << uint32_t(ticket->guid);
            data << ticket->message;
            data << uint8_t(0);         // unk
            data << float(ticket->timestamp);
            data << float(0);           // unk
            data << float(0);           // unk

            data << uint8_t(2);         // escalate?
            data << uint8_t(ticket->comment.empty() ? 0 : 1); // already comemnted

            std::string unkstring;
            data << unkstring;
            data << uint32_t(0);        // wait time

            SendPacket(&data);
        }
        else
        {
            WorldPacket data(SMSG_GMRESPONSE_RECEIVED);
            data << uint32_t(1);        // unk
            data << uint32_t(ticket->guid);
            data << ticket->message.c_str();

            size_t commentLength = ticket->comment.size();
            char const* commentChars = ticket->comment.c_str();

            for (int i = 0; i < 4; ++i)
            {
                if (commentLength)
                {
                    size_t writeLen = std::min<size_t>(commentLength, 3999);
                    data.append(commentChars, writeLen);

                    commentLength -= writeLen;
                    commentChars += writeLen;
                }

                data << uint8_t(0);
            }

            SendPacket(&data);
        }
    }
    else
    {
        WorldPacket data(SMSG_GMTICKET_GETTICKET, 4);
        data << uint32_t(10);     // always 10
        SendPacket(&data);
    }
}

void WorldSession::HandleGMTicketSystemStatusOpcode(WorldPacket& /*recv_data*/)
{
    WorldPacket data(SMSG_GMTICKET_SYSTEMSTATUS, 4);
    data << uint32_t(sWorld.getGmTicketStatus() ? 1 : 0);

    SendPacket(&data);
}

void WorldSession::HandleGMTicketCreateOpcode(WorldPacket& recv_data)
{
    uint32_t map;
    float x;
    float y;
    float z;
    std::string message;
    uint32_t responseNeeded;
    bool moreHelpNeeded;
    uint32_t ticketCount;
    std::list<uint32_t> timesList;
    uint32_t decompressedSize;
    std::string chatLog;

    recv_data >> map;
    recv_data >> x;
    recv_data >> y;
    recv_data >> z;
    recv_data >> message;
    recv_data >> responseNeeded;
    recv_data >> moreHelpNeeded;
    recv_data >> ticketCount;

    for (uint32 i = 0; i < ticketCount; ++i)
    {
        uint32_t time;
        recv_data >> time;
        timesList.push_back(time);
    }

    recv_data >> decompressedSize;

    if (ticketCount && decompressedSize && decompressedSize < 0xFFFF)
    {
        uint32 pos = recv_data.rpos();
        ByteBuffer dest;
        dest.resize(decompressedSize);

        uLongf realSize = decompressedSize;
        if (uncompress(dest.contents(), &realSize, recv_data.contents() + pos, recv_data.size() - pos) == Z_OK)
        {
            dest >> chatLog;
        }
        else
        {
            LOG_ERROR("CMSG_GMTICKET_CREATE failed to uncompress packet.");
            recv_data.rfinish();
            return;
        }

        recv_data.rfinish();
    }

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
    data << uint32_t(2);        // 2 = successful created
    SendPacket(&data);

    Channel* channel = channelmgr.GetChannel(worldConfig.getGmClientChannelName().c_str(), GetPlayer());
    if (channel)
    {
        std::stringstream messageStream;
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        messageStream << "GmTicket 5, " << ticket->name;
#else
        messageStream << "GmTicket:" << GM_TICKET_CHAT_OPCODE_NEWTICKET;
        messageStream << ":" << ticket->guid;
        messageStream << ":" << ticket->level;
        messageStream << ":" << ticket->name;
#endif
        channel->Say(_player, messageStream.str().c_str(), nullptr, true);
    }
}

void WorldSession::HandleGMTicketDeleteOpcode(WorldPacket& /*recv_data*/)
{
    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(GetPlayer()->GetGUID());

    objmgr.RemoveGMTicketByPlayer(GetPlayer()->GetGUID());

    WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
    data << uint32_t(9);      // 9 = successful deleted
    SendPacket(&data);

    Channel* channel = channelmgr.GetChannel(worldConfig.getGmClientChannelName().c_str(), GetPlayer());
    if (channel && ticket != nullptr)
    {
        std::stringstream deleteStream;
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        deleteStream << "GmTicket 1," << ticket->name;
#else
        deleteStream << "GmTicket:" << GM_TICKET_CHAT_OPCODE_REMOVED;
        deleteStream << ":" << ticket->guid;
#endif
        channel->Say(_player, deleteStream.str().c_str(), nullptr, true);
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
        data << uint32_t(1);      // 1 = error
        SendPacket(&data);

        return;
    }

    ticket->message = message;
    ticket->timestamp = (uint32_t)UNIXTIME;

    objmgr.UpdateGMTicket(ticket);

    data << uint32_t(2);       // successful

    SendPacket(&data);

#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
    Channel* channel = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), GetPlayer());
    if (channel)
    {
        std::stringstream channelStream;
        channelStream << "GmTicket:" << GM_TICKET_CHAT_OPCODE_UPDATED;
        channelStream << ":" << ticket->guid;
        channel->Say(_player, channelStream.str().c_str(), nullptr, true);
    }
#endif
}
