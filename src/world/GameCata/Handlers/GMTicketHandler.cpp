/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "Objects/ObjectMgr.h"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "zlib.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Units/Players/Player.h"

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
    data << uint32_t(sWorld.getGmTicketStatus() ? TicketSystemOK : TicketSystemDisabled);
    SendPacket(&data);
}

void WorldSession::HandleGMTicketCreateOpcode(WorldPacket& recvData)
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

    recvData >> map;
    recvData >> x;
    recvData >> y;
    recvData >> z;
    recvData >> message;
    recvData >> responseNeeded;
    recvData >> moreHelpNeeded;
    recvData >> ticketCount;

    for (uint32 i = 0; i < ticketCount; ++i)
    {
        uint32_t time;
        recvData >> time;
        timesList.push_back(time);
    }

    recvData >> decompressedSize;

    if (ticketCount && decompressedSize && decompressedSize < 0xFFFF)
    {
        uint32 pos = static_cast<uint32_t>(recvData.rpos());
        ByteBuffer dest;
        dest.resize(decompressedSize);

        uLongf realSize = decompressedSize;
        if (uncompress(dest.contents(), &realSize, recvData.contents() + pos, static_cast<uLong>(recvData.size() - pos)) == Z_OK)
        {
            dest >> chatLog;
        }
        else
        {
            LOG_ERROR("CMSG_GMTICKET_CREATE failed to uncompress packet.");
            recvData.rfinish();
            return;
        }

        recvData.rfinish();
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
    data << uint32_t(GMTNoErrors);
    SendPacket(&data);

    Channel* channel = channelmgr.GetChannel(worldConfig.getGmClientChannelName().c_str(), GetPlayer());
    if (channel != nullptr)
    {
        std::stringstream messageStream;
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        messageStream << "GmTicket 5, ";
        messageStream << ticket->name;
#else
        messageStream << "GmTicket:";
        messageStream << GM_TICKET_CHAT_OPCODE_NEWTICKET;
        messageStream << ":";
        messageStream << ticket->guid;
        messageStream << ":";
        messageStream << ticket->level;
        messageStream << ":";
        messageStream << ticket->name;
#endif
        channel->Say(_player, messageStream.str().c_str(), nullptr, true);
    }
}

void WorldSession::HandleGMTicketDeleteOpcode(WorldPacket& /*recv_data*/)
{
    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(GetPlayer()->GetGUID());

    objmgr.RemoveGMTicketByPlayer(GetPlayer()->GetGUID());

    WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
    data << uint32_t(GMTTicketRemoved);
    SendPacket(&data);

    Channel* channel = channelmgr.GetChannel(worldConfig.getGmClientChannelName().c_str(), GetPlayer());
    if (channel != nullptr && ticket != nullptr)
    {
        std::stringstream deleteStream;
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        deleteStream << "GmTicket 1,";
        deleteStream << ticket->name;
#else
        deleteStream << "GmTicket:";
        deleteStream << GM_TICKET_CHAT_OPCODE_REMOVED;
        deleteStream << ":";
        deleteStream << ticket->guid;
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
    if (channel)
    {
        std::stringstream channelStream;
        channelStream << "GmTicket:" << GM_TICKET_CHAT_OPCODE_UPDATED;
        channelStream << ":" << ticket->guid;
        channel->Say(_player, channelStream.str().c_str(), nullptr, true);
    }
#endif
}

void WorldSession::HandleGMTicketToggleSystemStatusOpcode(WorldPacket& /*recvData*/)
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
        CharacterDatabase.Execute("INSERT INTO lag_reports (player, account, lag_type, map_id, position_x, position_y, position_z) VALUES(%u, %u, %u, %u, %f, %f, %f)", GetPlayer()->GetLowGUID(), _accountId, lagType, mapId, position_x, position_y, position_z);
    }

    LogDebugFlag(LF_OPCODE, "Player %s has reported a lagreport with Type: %u on Map: %u", GetPlayer()->GetName(), lagType, mapId);
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

    CharacterDatabase.Execute("INSERT INTO gm_survey VALUES (%u, %u, %u, \'%s\', UNIX_TIMESTAMP(NOW()))", next_survey_id, GetPlayer()->GetLowGUID(), main_survey, CharacterDatabase.EscapeString(comment).c_str());

    LogDebugFlag(LF_OPCODE, "Player %s has submitted the gm suvey %u successfully.", GetPlayer()->GetName(), next_survey_id);
}
