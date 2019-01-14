/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/MainServerDefines.h"
#include "Chat/ChatHandler.hpp"
#include "Server/WorldSession.h"
#include "Objects/ObjectMgr.h"
#include "Server/Packets/SmsgGmTicketDeleteTicket.h"

using namespace AscEmu::Packets;

bool ChatHandler::HandleTicketListCommand(const char* /*args*/, WorldSession* m_session)
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM gm_tickets WHERE deleted=0");

    if (!result)
        return false;

    std::stringstream sstext;
    sstext << "List of active tickets: " << '\n';

    do
    {
        Field* fields = result->Fetch();
        sstext << "TicketID: " << fields[0].GetUInt16()
            << " | Player: " << fields[2].GetString()
            << " | Opened: " << Util::GetDateStringFromSeconds((uint32)UNIXTIME - fields[9].GetUInt32())
            << '\n';
    } while (result->NextRow());

    delete result;

    SendMultilineMessage(m_session, sstext.str().c_str());

    return true;
}

bool ChatHandler::HandleTicketListAllCommand(const char* /*args*/, WorldSession* m_session)
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM gm_tickets");

    if (!result)
        return false;

    std::stringstream sstext;
    sstext << "List of active tickets: " << '\n';

    do
    {
        Field* fields = result->Fetch();
        sstext << "TicketID: " << fields[0].GetUInt16()
            << " | Player: " << fields[2].GetString()
            << " | Opened: " << Util::GetDateStringFromSeconds((uint32)UNIXTIME - fields[9].GetUInt32())
            << '\n';
    } while (result->NextRow());

    delete result;

    SendMultilineMessage(m_session, sstext.str().c_str());

    return true;
}

bool ChatHandler::HandleTicketGetCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        RedSystemMessage(m_session, "You need to specify a ticket ID!");
        return false;
    }

    uint32 ticketID = atol(args);

    QueryResult* result = CharacterDatabase.Query("SELECT * FROM gm_tickets WHERE ticketid = %u", ticketID);

    if (!result)
        return false;

    std::stringstream sstext;
    Field* fields = result->Fetch();

    sstext << "Ticket ID: " << ticketID << " | Player: " << fields[2].GetString() << '\n'
            << "======= Content =======" << '\n'
            << fields[8].GetString() << '\n';

    delete result;

    SendMultilineMessage(m_session, sstext.str().c_str());

    return true;
}

void ParseTicketArgs(char* args, char** insertComment)
{
    char* Comment = strchr(args, ' ');
    if (Comment != NULL)
    {
        if (isdigit(*(Comment + 1)))       // this is the duration of the ban
        {
            *Comment = 0;                  // NULL-terminate the first string (character/account/ip)
            ++Comment;                     // point to next arg
        }
    }
    *insertComment = Comment;
}

bool ChatHandler::HandleTicketCloseCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        RedSystemMessage(m_session, "You need to specify a ticket ID and a comment! Use .ticket close <ID> <comment text>");
        return false;
    }

    char* ticket = (char*)args;
    char* comment;
    ParseTicketArgs(ticket, &comment);

    if (comment == nullptr)
    {
        RedSystemMessage(m_session, "You need to add a comment for closing an ticket!");
        return false;
    }

    uint32 ticketID = atol(args);

    Player* player = m_session->GetPlayer();

    QueryResult* result = CharacterDatabase.Query("SELECT * FROM gm_tickets WHERE ticketid = %u AND deleted = 0", ticketID);
    if (!result)
    {
        RedSystemMessage(m_session, "Ticket %u is already closed!", ticketID);
        return false;
    }
    Field* fields = result->Fetch();
    uint32 playerGuid = fields[1].GetUInt32();

    GM_Ticket* gm_ticket = objmgr.GetGMTicketByPlayer(playerGuid);
    if (gm_ticket == nullptr)
    {
        RedSystemMessage(m_session, "Ticket not found.");
        return true;
    }

    objmgr.CloseTicket(gm_ticket->guid);

    Player* ticketOwner = objmgr.GetPlayer(playerGuid);

    if (ticketOwner != nullptr)
    {
        ticketOwner->GetSession()->SystemMessage("Your Ticket was closed by %s Comment: %s", player->getName().c_str(), comment);

        // Notify player about removing ticket
        ticketOwner->GetSession()->SendPacket(SmsgGmTicketDeleteTicket(9).serialise().get());
        // Response - Send GM Survey
        WorldPacket datab(SMSG_GM_TICKET_STATUS_UPDATE, 1);
        datab << uint32(3);
        ticketOwner->GetSession()->SendPacket(&datab);
    }
    else
    {
        std::string ticketIdString = std::to_string(ticketID);
        std::string subject = "Your Ticket: ";
        subject += ticketIdString;
        sMailSystem.SendAutomatedMessage(MAIL_TYPE_NORMAL, player->getGuid(), playerGuid, subject, comment, 0, 0, 0, MAIL_STATIONERY_GM, MAIL_CHECK_MASK_NONE);
    }

    CharacterDatabase.Execute("UPDATE gm_tickets SET deleted = 1, comment = 'GM: %s %s', assignedto = %u WHERE ticketid = %u", player->getName().c_str(), comment, player->getGuid(), ticketID);
    GreenSystemMessage(m_session, "Ticket %u is now closed and assigned to you.", ticketID);
    sGMLog.writefromsession(m_session, "closed ticket %u ", ticketID);
    delete result;
    return true;
}

bool ChatHandler::HandleTicketDeleteCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        RedSystemMessage(m_session, "You need to specify a ticket ID");
        return false;
    }

    uint32 ticketID = atol(args);

    QueryResult* result = CharacterDatabase.Query("SELECT * FROM gm_tickets WHERE ticketid = %u AND deleted = 1", ticketID);
    if (!result)
    {
        RedSystemMessage(m_session, "Ticket %u is not available in gm_tickets table or not closed!", ticketID);
        return false;
    }

    delete result;

    CharacterDatabase.Execute("DELETE FROM gm_tickets WHERE ticketid = %u", ticketID);
    GreenSystemMessage(m_session, "Ticket %u is deleted", ticketID);
    sGMLog.writefromsession(m_session, "deleted ticket %u ", ticketID);

    return true;
}
