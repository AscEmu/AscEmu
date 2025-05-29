/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <sstream>

#include "Management/ObjectMgr.hpp"
#include "Chat/Channel.hpp"
#include "Chat/ChannelMgr.hpp"
#include "Logging/Logger.hpp"
#include "Management/Tickets/TicketMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/Packets/CmsgGmTicketCreate.h"
#include "Server/Packets/SmsgGmTicketCreate.h"
#include "Server/Packets/CmsgGmTicketUpdateText.h"
#include "Server/Packets/SmsgGmTicketUpdateText.h"
#include "Server/Packets/SmsgGmTicketDeleteTicket.h"
#include "Server/Packets/SmsgGmTicketGetTicket.h"
#include "Server/Packets/SmsgGmTicketSystemstatus.h"
#include "Server/Packets/CmsgGmReportLag.h"
#include "Server/Packets/CmsgGmSurveySubmit.h"

#if VERSION_STRING > WotLK
#include <zlib.h>
#endif

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

    auto result = CharacterDatabase.Query("SELECT MAX(survey_id) FROM gm_survey");
    if (result == nullptr)
        return;

    uint32_t next_survey_id = result->Fetch()[0].asUint32() + 1;

    for (auto subSurvey : srlPacket.subSurvey)
        CharacterDatabase.Execute("INSERT INTO gm_survey_answers VALUES(%u , %u , %u)",
            next_survey_id, subSurvey.subSurveyId, subSurvey.answerId);

    CharacterDatabase.Execute("INSERT INTO gm_survey VALUES (%u, %u, %u, \'%s\', UNIX_TIMESTAMP(NOW()))",
        next_survey_id, _player->getGuidLow(), srlPacket.mainSurveyId, CharacterDatabase.EscapeString(srlPacket.mainComment).c_str());

    sLogger.debug("Player {} has submitted the gm suvey {} successfully.",
        _player->getName(), next_survey_id);
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

        sLogger.debug("Player {} has reported a lagreport with Type: {} on Map: {}", _player->getName(), srlPacket.lagType, srlPacket.mapId);
    }

#endif
}

void WorldSession::handleGMTicketUpdateOpcode(WorldPacket& recvPacket)
{
    CmsgGmTicketUpdateText srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    GM_Ticket* ticket = sTicketMgr.getGMTicketByPlayer(_player->getGuid());
    if (ticket == nullptr)
    {
        SendPacket(SmsgGmTicketUpdateText(GMTNoTicketFound).serialise().get());
    }
    else
    {
        ticket->message = srlPacket.message;
        ticket->timestamp = static_cast<uint32_t>(UNIXTIME);
        sTicketMgr.updateGMTicket(ticket);

        SendPacket(SmsgGmTicketUpdateText(GMTNoErrors).serialise().get());
    }

#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
    auto channel = sChannelMgr.getChannel(sWorld.getGmClientChannel(), _player);
    if (channel != nullptr)
    {
        std::stringstream ss;
        ss << "GmTicket:";
        ss << GM_TICKET_CHAT_OPCODE_UPDATED;
        ss << ":";
        ss << ticket->guid;
        channel->say(_player, ss.str().c_str(), nullptr, true);
    }
#endif
}

void WorldSession::handleGMTicketDeleteOpcode(WorldPacket& /*recvPacket*/)
{
    GM_Ticket* ticket = sTicketMgr.getGMTicketByPlayer(_player->getGuid());

    sTicketMgr.removeGMTicketByPlayer(_player->getGuid());

    SendPacket(SmsgGmTicketDeleteTicket(GMTTicketRemoved).serialise().get());

    auto channel = sChannelMgr.getChannel(worldConfig.getGmClientChannelName(), _player);
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
        channel->say(_player, ss.str(), nullptr, true);
    }
}

void WorldSession::handleGMTicketCreateOpcode(WorldPacket& recvPacket)
{
    CmsgGmTicketCreate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

#if VERSION_STRING > WotLK

    // chatLog
    std::string chatLog;
    if (srlPacket.ticketCount && srlPacket.decompressedSize && srlPacket.decompressedSize < 0xFFFF)
    {
        const auto pos = static_cast<uint32_t>(recvPacket.rpos());
        ByteBuffer dest;
        dest.resize(srlPacket.decompressedSize);

        uLongf realSize = srlPacket.decompressedSize;
        if (uncompress(dest.contents(), &realSize, recvPacket.contents() + pos, static_cast<uLong>(recvPacket.size() - pos)) == Z_OK)
        {
            dest >> chatLog;
        }
        else
        {
            sLogger.failure("CMSG_GMTICKET_CREATE failed to uncompress packet.");
            recvPacket.rfinish();
            return;
        }

        recvPacket.rfinish();
    }
#endif  

    // Remove pending tickets
    sTicketMgr.removeGMTicketByPlayer(_player->getGuid());

    auto* ticket = sTicketMgr.createGMTicket(_player, srlPacket);

    SendPacket(SmsgGmTicketCreate(GMTNoErrors).serialise().get());

    // send message indicating new ticket
    auto channel = sChannelMgr.getChannel(worldConfig.getGmClientChannelName(), _player);
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
        channel->say(_player, ss.str(), nullptr, true);
    }
}


void WorldSession::handleGMTicketGetTicketOpcode(WorldPacket& /*recvPacket*/)
{
    if (const auto ticket = sTicketMgr.getGMTicketByPlayer(_player->getGuid()))
    {
        if (!ticket->deleted)
        {
            SendPacket(SmsgGmTicketGetTicket(GMTCurrentTicketFound, ticket->message, 0, ticket->guid, ticket->timestamp, ticket->comment).serialise().get());
        }
#if VERSION_STRING > WotLK
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
#endif
    }
    else
    {
        SendPacket(SmsgGmTicketGetTicket(GMTNoCurrentTicket, "", 0, 0, 0, "").serialise().get());
    }
}

