/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <sstream>

#include "Chat/ChatHandler.hpp"
#include "Logging/Log.hpp"
#include "Management/MailMgr.h"
#include "Management/ObjectMgr.hpp"
#include "Management/Tickets/TicketMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Server/Packets/SmsgGmTicketDeleteTicket.h"
#include "Server/Packets/SmsgGmTicketStatusUpdate.h"
#include "Utilities/Util.hpp"

using namespace AscEmu::Packets;

bool ChatHandler::HandleTicketListCommand(const char* /*args*/, WorldSession* m_session)
{
    auto result = CharacterDatabase.Query("SELECT * FROM gm_tickets WHERE deleted=0");

    if (!result)
        return false;

    std::stringstream sstext;
    sstext << "List of active tickets: " << '\n';

    do
    {
        Field* fields = result->Fetch();
        sstext << "TicketID: " << fields[0].asUint16()
            << " | Player: " << fields[2].asCString()
            << " | Opened: " << Util::GetDateStringFromSeconds((uint32_t)UNIXTIME - fields[9].asUint32())
            << '\n';
    } while (result->NextRow());

    SendMultilineMessage(m_session, sstext.str().c_str());

    return true;
}

bool ChatHandler::HandleTicketListAllCommand(const char* /*args*/, WorldSession* m_session)
{
    auto result = CharacterDatabase.Query("SELECT * FROM gm_tickets");

    if (!result)
        return false;

    std::stringstream sstext;
    sstext << "List of active tickets: " << '\n';

    do
    {
        Field* fields = result->Fetch();
        sstext << "TicketID: " << fields[0].asUint16()
            << " | Player: " << fields[2].asCString()
            << " | Opened: " << Util::GetDateStringFromSeconds((uint32_t)UNIXTIME - fields[9].asUint32())
            << '\n';
    } while (result->NextRow());

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

    uint32_t ticketID = std::stoul(args);

    auto result = CharacterDatabase.Query("SELECT * FROM gm_tickets WHERE ticketid = %u", ticketID);

    if (!result)
        return false;

    std::stringstream sstext;
    Field* fields = result->Fetch();

    sstext << "Ticket ID: " << ticketID << " | Player: " << fields[2].asCString() << '\n'
            << "======= Content =======" << '\n'
            << fields[8].asCString() << '\n';

    SendMultilineMessage(m_session, sstext.str().c_str());

    return true;
}

void ParseTicketArgs(char* args, char** insertComment)
{
    char* Comment = strchr(args, ' ');
    if (Comment != nullptr)
    {
        if (isdigit(*(Comment + 1)))       // this is the duration of the ban
        {
            *Comment = 0;                  // nullptr-terminate the first string (character/account/ip)
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

    uint32_t ticketID = std::stoul(args);

    Player* player = m_session->GetPlayer();

    auto result = CharacterDatabase.Query("SELECT * FROM gm_tickets WHERE ticketid = %u AND deleted = 0", ticketID);
    if (!result)
    {
        RedSystemMessage(m_session, "Ticket %u is already closed!", ticketID);
        return false;
    }
    Field* fields = result->Fetch();
    uint32_t playerGuid = fields[1].asUint32();

    GM_Ticket* gm_ticket = sTicketMgr.getGMTicketByPlayer(playerGuid);
    if (gm_ticket == nullptr)
    {
        RedSystemMessage(m_session, "Ticket not found.");
        return true;
    }

    sTicketMgr.closeTicket(gm_ticket->guid);

    Player* ticketOwner = sObjectMgr.getPlayer(playerGuid);

    if (ticketOwner != nullptr)
    {
        ticketOwner->getSession()->SystemMessage("Your Ticket was closed by %s Comment: %s", player->getName().c_str(), comment);

        // Notify player about removing ticket
        ticketOwner->getSession()->SendPacket(SmsgGmTicketDeleteTicket(9).serialise().get());
        // Response - Send GM Survey
        ticketOwner->getSession()->SendPacket(SmsgGmTicketStatusUpdate(3).serialise().get());
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
    return true;
}

bool ChatHandler::HandleTicketDeleteCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        RedSystemMessage(m_session, "You need to specify a ticket ID");
        return false;
    }

    uint32_t ticketID = std::stoul(args);

    auto result = CharacterDatabase.Query("SELECT * FROM gm_tickets WHERE ticketid = %u AND deleted = 1", ticketID);
    if (!result)
    {
        RedSystemMessage(m_session, "Ticket %u is not available in gm_tickets table or not closed!", ticketID);
        return false;
    }

    CharacterDatabase.Execute("DELETE FROM gm_tickets WHERE ticketid = %u", ticketID);
    GreenSystemMessage(m_session, "Ticket %u is deleted", ticketID);
    sGMLog.writefromsession(m_session, "deleted ticket %u ", ticketID);

    return true;
}
