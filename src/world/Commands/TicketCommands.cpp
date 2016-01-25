/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "StdAfx.h"

bool ChatHandler::HandleTicketListCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* player = m_session->GetPlayer();

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
            << " | Opened: " << ConvertTimeStampToString((uint32)UNIXTIME - fields[9].GetUInt32())
            << '\n';
    } while (result->NextRow());

    delete result;

    SendMultilineMessage(m_session, sstext.str().c_str());

    return true;
}

bool ChatHandler::HandleTicketListAllCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* player = m_session->GetPlayer();

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
            << " | Opened: " << ConvertTimeStampToString((uint32)UNIXTIME - fields[9].GetUInt32())
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

    Player* player = m_session->GetPlayer();

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
        ticketOwner->GetSession()->SystemMessage("Your Ticket was closed by %s Comment: %s", player->GetName(), comment);

        // Notify player about removing ticket
        WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
        data << uint32(9);
        ticketOwner->GetSession()->SendPacket(&data);
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
        sMailSystem.SendAutomatedMessage(MAIL_TYPE_NORMAL, player->GetGUID(), playerGuid, subject, comment, 0, 0, 0, MAIL_STATIONERY_GM, MAIL_CHECK_MASK_NONE);
    }

    CharacterDatabase.Execute("UPDATE gm_tickets SET deleted = 1, comment = 'GM: %s %s', assignedto = %u WHERE ticketid = %u", player->GetName(), comment, player->GetGUID(), ticketID);
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

    Player* player = m_session->GetPlayer();

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
