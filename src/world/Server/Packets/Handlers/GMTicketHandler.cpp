/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include <zlib.h>

using namespace AscEmu::Packets;

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

void WorldSession::handleGMTicketSystemStatusOpcode(WorldPacket& /*recvPacket*/)
{
    SendPacket(SmsgGmTicketSystemstatus(sWorld.getGmTicketStatus() ? TicketSystemOK : TicketSystemDisabled).serialise().get());
}

void WorldSession::handleGMTicketToggleSystemStatusOpcode(WorldPacket& /*recvPacket*/)
{
    if (HasGMPermissions())
        sWorld.toggleGmTicketStatus();
}

void WorldSession::handleGMSurveySubmitOpcode(WorldPacket& recvPacket)
{
    CmsgGmSurveySubmit srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    QueryResult* result = CharacterDatabase.Query("SELECT MAX(survey_id) FROM gm_survey");
    if (result == nullptr)
        return;

    uint32_t next_survey_id = result->Fetch()[0].GetUInt32() + 1;

    for (auto subSurvey : srlPacket.subSurvey)
        CharacterDatabase.Execute("INSERT INTO gm_survey_answers VALUES(%u , %u , %u)",
            next_survey_id, subSurvey.subSurveyId, subSurvey.answerId);

    CharacterDatabase.Execute("INSERT INTO gm_survey VALUES (%u, %u, %u, \'%s\', UNIX_TIMESTAMP(NOW()))",
        next_survey_id, _player->getGuidLow(), srlPacket.mainSurveyId, CharacterDatabase.EscapeString(srlPacket.mainComment).c_str());

    LogDebugFlag(LF_OPCODE, "Player %s has submitted the gm suvey %u successfully.",
        _player->getName().c_str(), next_survey_id);
}

void WorldSession::handleReportLag(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    CmsgGmReportLag srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (_player != nullptr)
    {
        CharacterDatabase.Execute("INSERT INTO lag_reports (player, account, lag_type, map_id, position_x, position_y, position_z) VALUES(%u, %u, %u, %u, %f, %f, %f)",
            _player->getGuidLow(), _accountId, srlPacket.lagType, srlPacket.mapId, srlPacket.location.x, srlPacket.location.y, srlPacket.location.z);
    }

    LogDebugFlag(LF_OPCODE, "Player %s has reported a lagreport with Type: %u on Map: %u", _player->getName().c_str(), srlPacket.lagType, srlPacket.mapId);
#endif
}

void WorldSession::handleGMTicketUpdateOpcode(WorldPacket& recvPacket)
{
    CmsgGmTicketUpdateText srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(_player->getGuid());
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
    Channel* channel = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), _player);
    if (channel != nullptr)
    {
        std::stringstream ss;
        ss << "GmTicket:";
        ss << GM_TICKET_CHAT_OPCODE_UPDATED;
        ss << ":";
        ss << ticket->guid;
        channel->Say(_player, ss.str().c_str(), nullptr, true);
    }
#endif
}

void WorldSession::handleGMTicketDeleteOpcode(WorldPacket& /*recvPacket*/)
{
    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(_player->getGuid());

    objmgr.RemoveGMTicketByPlayer(_player->getGuid());

    SendPacket(SmsgGmTicketDeleteTicket(GMTTicketRemoved).serialise().get());

    Channel* channel = channelmgr.GetChannel(worldConfig.getGmClientChannelName().c_str(), _player);
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

#if VERSION_STRING < Cata
void WorldSession::handleGMTicketCreateOpcode(WorldPacket& recvPacket)
{
    CmsgGmTicketCreate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    // Remove pending tickets
    objmgr.RemoveGMTicketByPlayer(_player->getGuid());

    GM_Ticket* ticket = new GM_Ticket;
    ticket->guid = uint64_t(objmgr.GenerateTicketID());
    ticket->playerGuid = _player->getGuid();
    ticket->map = srlPacket.map;
    ticket->posX = srlPacket.location.x;
    ticket->posY = srlPacket.location.y;
    ticket->posZ = srlPacket.location.z;
    ticket->message = srlPacket.message;
    ticket->timestamp = (uint32_t)UNIXTIME;
    ticket->name = _player->getName().c_str();
    ticket->level = _player->getLevel();
    ticket->deleted = false;
    ticket->assignedToPlayer = 0;
    ticket->comment = "";

    objmgr.AddGMTicket(ticket, false);

    SendPacket(SmsgGmTicketCreate(GMTNoErrors).serialise().get());

    // send message indicating new ticket
    Channel* channel = channelmgr.GetChannel(worldConfig.getGmClientChannelName().c_str(), _player);
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
#else
void WorldSession::handleGMTicketCreateOpcode(WorldPacket& recvPacket)
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

    recvPacket >> map;
    recvPacket >> x;
    recvPacket >> y;
    recvPacket >> z;
    recvPacket >> message;
    recvPacket >> responseNeeded;
    recvPacket >> moreHelpNeeded;
    recvPacket >> ticketCount;

    for (uint32 i = 0; i < ticketCount; ++i)
    {
        uint32_t time;
        recvPacket >> time;
        timesList.push_back(time);
    }

    recvPacket >> decompressedSize;

    if (ticketCount && decompressedSize && decompressedSize < 0xFFFF)
    {
        uint32 pos = static_cast<uint32_t>(recvPacket.rpos());
        ByteBuffer dest;
        dest.resize(decompressedSize);

        uLongf realSize = decompressedSize;
        if (uncompress(dest.contents(), &realSize, recvPacket.contents() + pos, static_cast<uLong>(recvPacket.size() - pos)) == Z_OK)
        {
            dest >> chatLog;
        }
        else
        {
            LOG_ERROR("CMSG_GMTICKET_CREATE failed to uncompress packet.");
            recvPacket.rfinish();
            return;
        }

        recvPacket.rfinish();
    }

    objmgr.RemoveGMTicketByPlayer(_player->getGuid());

    GM_Ticket* ticket = new GM_Ticket;
    ticket->guid = uint64_t(objmgr.GenerateTicketID());
    ticket->playerGuid = _player->getGuid();
    ticket->map = map;
    ticket->posX = x;
    ticket->posY = y;
    ticket->posZ = z;
    ticket->message = message;
    ticket->timestamp = (uint32_t)UNIXTIME;
    ticket->name = _player->getName().c_str();
    ticket->level = _player->getLevel();
    ticket->deleted = false;
    ticket->assignedToPlayer = 0;
    ticket->comment = "";

    objmgr.AddGMTicket(ticket, false);

    WorldPacket data(SMSG_GMTICKET_CREATE, 4);
    data << uint32_t(GMTNoErrors);
    SendPacket(&data);

    Channel* channel = channelmgr.GetChannel(worldConfig.getGmClientChannelName().c_str(), _player);
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
#endif

#if VERSION_STRING < Cata
void WorldSession::handleGMTicketGetTicketOpcode(WorldPacket& /*recvPacket*/)
{
    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(_player->getGuid());
    if (ticket == nullptr)
        SendPacket(SmsgGmTicketGetTicket(GMTNoCurrentTicket, "", 0).serialise().get());
    else
        SendPacket(SmsgGmTicketGetTicket(GMTCurrentTicketFound, ticket->message, static_cast<uint8_t>(ticket->map)).serialise().get());
}
#else
void WorldSession::handleGMTicketGetTicketOpcode(WorldPacket& /*recvPacket*/)
{
    if (GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(_player->getGuid()))
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
#endif
