/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <sstream>

#include "Chat/Channel.hpp"
#include "Chat/ChannelMgr.hpp"
#include "Chat/ChatCommandHandler.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/Tickets/TicketMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/Packets/SmsgGmTicketDeleteTicket.h"
#include "Server/Packets/SmsgGmTicketStatusUpdate.h"

using namespace AscEmu::Packets;

#ifdef GM_TICKET_MY_MASTER_COMPATIBLE

bool ChatCommandHandler::HandleGMTicketListCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* cplr = m_session->GetPlayer();

    auto* chn = sChannelMgr.getChannel(worldConfig.getGmClientChannelName(), cplr);
    if (!chn)
        return false;

    chn->say(cplr, "GmTicket 2", cplr, true);

    for (GmTicketList::iterator itr = sTicketMgr.m_ticketList.begin(); itr != sTicketMgr.m_ticketList.end(); ++itr)
    {
        if ((*itr)->deleted)
            continue;

        Player* plr = sObjectMgr.getPlayer((uint32_t)(*itr)->playerGuid);

        if (plr == nullptr)
            continue;

        //        Player* aplr = ((*itr)->assignedToPlayer == 0 ? nullptr : sObjectMgr.GetPlayer((uint32_t)(*itr)->assignedToPlayer));

        std::stringstream ss;

        uint32_t zone = 0;
        if (plr->IsInWorld())
        {
            zone = plr->getZoneId();
        }
        ss << "GmTicket 0," << (*itr)->name << "," << (*itr)->level << ",0," << zone;
        chn->say(cplr, ss.str(), cplr, true);
    }

    return true;
}

bool ChatCommandHandler::HandleGMTicketGetByIdCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    Player* cplr = m_session->GetPlayer();
    auto* chn = sChannelMgr.getChannel(worldConfig.getGmClientChannelName(), cplr);
    if (!chn)
        return false;

    Player* plr = sObjectMgr.getPlayer(args, false);
    if (plr == nullptr)
    {
        redSystemMessage(m_session, "Player not found.");
        return true;
    }
    GM_Ticket* ticket = sTicketMgr.getGMTicketByPlayer(plr->getGuid());
    if (ticket == nullptr || ticket->deleted)
    {
        redSystemMessage(m_session, "Ticket not found.");
        return true;
    }

    auto msg = std::make_unique<char[]>(ticket->message.size() + 1);
    strcpy(msg.get(), ticket->message.c_str());
    char* start = msg.get(), *end;
    bool firstLine = true;
    for (;;)
    {
        end = strchr(start, '\n');
        if (!end)
            break;

        *end = '\0';

        std::stringstream ss;
        ss << "GmTicket " << (firstLine ? "3" : "4") << "," << ticket->name << "," << start;
        chn->say(cplr, ss.str(), cplr, true);

        firstLine = false;

        start = end + 1;
    }
    if (*start != '\0')
    {
        std::stringstream ss;
        ss << "GmTicket " << (firstLine ? "3" : "4") << "," << ticket->name << "," << start;
        chn->say(cplr, ss.str(), cplr, true);
    }

    return true;
}

bool ChatCommandHandler::HandleGMTicketRemoveByIdCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    Player* cplr = m_session->GetPlayer();
    auto* chn = sChannelMgr.getChannel(worldConfig.getGmClientChannelName(), cplr);
    if (!chn)
        return false;

    Player* plr = sObjectMgr.getPlayer(args, true);
    if (plr == nullptr)
    {
        redSystemMessage(m_session, "Player not found.");
        return true;
    }
    GM_Ticket* ticket = sTicketMgr.getGMTicketByPlayer(plr->getGuid());
    if (ticket == nullptr || ticket->deleted)
    {
        redSystemMessage(m_session, "Ticket not found.");
        return true;
    }

    std::stringstream ss;
    ss << "GmTicket 1," << ticket->name;
    chn->say(cplr, ss.str(), nullptr, true);

    sTicketMgr.removeGMTicket(ticket->guid);

    if (!plr->IsInWorld())
        return true;

    // Notify player about removing ticket
    plr->getSession()->SendPacket(SmsgGmTicketDeleteTicket(9).serialise().get());

    // Response - Send GM Survey
    plr->getSession()->SendPacket(SmsgGmTicketStatusUpdate(3).serialise().get());

    return true;
}

#else

void ChatCommandHandler::SendGMSurvey()
{}

bool ChatCommandHandler::HandleGMTicketListCommand(const char* args, WorldSession* m_session)
{
    Player* cplr = m_session->GetPlayer();

    Channel* chn = sChannelMgr.getChannel(sWorld.getGmClientChannel(), cplr);
    if (!chn)
        return false;

    std::stringstream ss0;
    ss0 << "GmTicket:" << GM_TICKET_CHAT_OPCODE_LISTSTART;
    chn->say(cplr, ss0.str().c_str(), cplr, true);

    for (GmTicketList::iterator itr = sTicketMgr.m_ticketList.begin(); itr != sTicketMgr.m_ticketList.end(); itr++)
    {
        if ((*itr)->deleted)
            continue;

        Player* plr = sObjectMgr.GetPlayer((uint32_t)(*itr)->playerGuid);

        Player* aplr = nullptr;
        CachedCharacterInfo const* aplri = nullptr;
        if ((*itr)->assignedToPlayer != 0)
        {
            aplr = sObjectMgr.GetPlayer((uint32_t)(*itr)->assignedToPlayer);
            if (aplr == nullptr)
                aplri = sObjectMgr.getCachedCharacterInfo((uint32_t)(*itr)->assignedToPlayer)
        }

        std::stringstream ss;
        ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_LISTENTRY;
        ss << ":" << (*itr)->guid;
        ss << ":" << (plr == nullptr ? (*itr)->level : plr->getLevel());
        ss << ":" << (plr == nullptr ? 0 : plr->IsInWorld());
        ss << ":" << (aplr == nullptr ? (aplri == nullptr ? "" : aplri->name) : aplr->GetName());
        ss << ":" << (plr == nullptr ? (*itr)->name : plr->GetName());
        ss << ":" << (*itr)->comment;
        chn->say(cplr, ss.str().c_str(), cplr, true);
    }

    return true;
}

bool ChatCommandHandler::HandleGMTicketGetByIdCommand(const char* args, WorldSession* m_session)
{
    uint64_t ticketGuid = (args ? atoi(args) : 0);
    if (!ticketGuid)
    {
        redSystemMessage(m_session, "You must specify a ticket id.");
        return true;
    }

    Player* cplr = m_session->GetPlayer();
    Channel* chn = sChannelMgr.getChannel(sWorld.getGmClientChannel(), cplr);
    if (!chn)
        return false;

    GM_Ticket* ticket = sTicketMgr.getGMTicket(ticketGuid);
    if (ticket == nullptr || ticket->deleted)
    {
        chn->say(cplr, "GmTicket:0:Ticket not found.", cplr, true);
        return true;
    }

    auto msg = std::make_unique<char[]>(ticket->message.size() + 1);
    strcpy(msg.get(), ticket->message.c_str());
    char* start = msg.get(), *end;
    bool firstLine = true;
    for (;;)
    {
        end = strchr(start, '\n');
        if (!end)
            break;

        *end = '\0';

        std::stringstream ss;
        ss << "GmTicket:" << (firstLine ? GM_TICKET_CHAT_OPCODE_CONTENT : GM_TICKET_CHAT_OPCODE_APPENDCONTENT);
        ss << ":" << ticket->guid;
        ss << ":" << start;
        chn->say(cplr, ss.str().c_str(), cplr, true);

        firstLine = false;

        start = end + 1;
    }
    if (*start != '\0')
    {
        std::stringstream ss;
        ss << "GmTicket:" << (firstLine ? GM_TICKET_CHAT_OPCODE_CONTENT : GM_TICKET_CHAT_OPCODE_APPENDCONTENT);
        ss << ":" << ticket->guid;
        ss << ":" << start;
        chn->say(cplr, ss.str().c_str(), cplr, true);
    }

    return true;
}

bool ChatCommandHandler::HandleGMTicketRemoveByIdCommand(const char* args, WorldSession* m_session)
{
    uint64_t ticketGuid = (args ? atoi(args) : 0);
    if (!ticketGuid)
    {
        redSystemMessage(m_session, "You must specify a ticket id.");
        return true;
    }

    Player* cplr = m_session->GetPlayer();
    Channel* chn = sChannelMgr.getChannel(sWorld.getGmClientChannel(), cplr);
    if (!chn)
        return false;

    GM_Ticket* ticket = sTicketMgr.getGMTicket(ticketGuid);
    if (ticket == nullptr || ticket->deleted)
    {
        chn->say(cplr, "GmTicket:0:Ticket not found.", cplr, true);
        return true;
    }

    if (ticket->assignedToPlayer != 0 && ticket->assignedToPlayer != cplr->getGuid() && !cplr->getSession()->CanUseCommand('z'))
    {
        chn->say(cplr, "GmTicket:0:Ticket is assigned to another GM.", cplr, true);
        return true;
    }

    Player* plr = sObjectMgr.GetPlayer((uint32_t)ticket->playerGuid);

    std::stringstream ss;
    ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_REMOVED;
    ss << ":" << ticket->guid;
    chn->say(cplr, ss.str().c_str(), nullptr, true);

    sTicketMgr.removeGMTicket(ticket->guid);

    if (!plr)
        return true;
    if (!plr->IsInWorld())
        return true;

    // Notify player about removing ticket
    plr->getSession()->sendPacket(SmsgGmTicketDeleteTicket(9).serialise().get());

    // Response - Send GM Survey
    plr->getSession()->sendPacket(SmsgGmTicketStatusUpdate(3).serialise().get());

    systemMessage(plr->getSession(), "You have been selected to fill out a GM Performance Survey. Please respond truthfully to the questions that you are asked and include the Game Masters name to your comment.");
    return true;
}


bool ChatCommandHandler::HandleGMTicketAssignToCommand(const char* args, WorldSession* m_session)
{
    uint64_t ticketGuid;
    char guidString[100], name[100];
    int argc = sscanf(args, "%s %s", guidString, name);
    if (argc < 1 || argc > 2)
        return false;

    ticketGuid = atoi(guidString);
    if (!ticketGuid)
    {
        redSystemMessage(m_session, "You must specify a ticket id.");
        return true;
    }

    Player* cplr = m_session->GetPlayer();
    Channel* chn = sChannelMgr.getChannel(sWorld.getGmClientChannel(), cplr);
    if (!chn)
        return false;

    GM_Ticket* ticket = sTicketMgr.getGMTicket(ticketGuid);

    if (ticket == nullptr || ticket->deleted)
    {
        chn->say(cplr, "GmTicket:0:Ticket not found.", cplr, true);
        return true;
    }

    Player* mplr = sObjectMgr.GetPlayer((uint32_t)ticket->playerGuid);
    Player* plr = (argc == 1 ? cplr : sObjectMgr.GetPlayer(name, false));
    if (plr == nullptr)
    {
        chn->say(cplr, "GmTicket:0:Player not found.", cplr, true);
        return true;
    }

    if (!plr->IsInWorld())
    {
        chn->say(cplr, "GmTicket:0:Player isn't online.", cplr, true);
        return true;
    }

    if (!plr->getSession()->HasGMPermissions())
    {
        chn->say(cplr, "GmTicket:0:Player is not a GM.", cplr, true);
        return true;
    }

    if (ticket->assignedToPlayer == plr->getGuid())
    {
        chn->say(cplr, "GmTicket:0:Ticket already assigned to this GM.", cplr, true);
        return true;
    }

    if (ticket->assignedToPlayer != 0 && ticket->assignedToPlayer != cplr->getGuid())
    {
        Player* aplr = sObjectMgr.GetPlayer((uint32_t)ticket->assignedToPlayer);
        if (aplr != nullptr && aplr->IsInWorld() && !cplr->getSession()->CanUseCommand('z'))
        {
            chn->say(cplr, "GmTicket:0:Ticket already assigned to another GM.", cplr, true);
            return true;
        }
    }

    ticket->assignedToPlayer = plr->getGuid();
    sTicketMgr.updateGMTicket(ticket);

    std::stringstream ss;
    ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_ASSIGNED;
    ss << ":" << ticket->guid;
    ss << ":" << plr->GetName();
    chn->say(cplr, ss.str().c_str(), nullptr, true);
    //Send Response Packet to update Ticket
    //WorldPacket data(SMSG_GMTICKET_GETTICKET, 400);
    //data << uint32_t(6); // Packet Status
    //data << uint8_t(0x7);//static Category
    //data << ticket->message.c_str();//ticketDescription
    //data << float(0.0);//ticketAge - days //update time =  last time ticket was modified?
    //data << float(0.0);//oldestTicketTime - days
    //data << float(0.0);//updateTime - days | How recent is the data for oldest ticket time, measured in days.  If this number 1 hour, we have bad data.
    //data << unit64(2);//assignedToGM |0 - ticket is not currently assigned to a gm | 1 - ticket is assigned to a normal gm |    2 - ticket is in the escalation queue
    //data << uint64_t(1);//openedByGM | 0 - ticket has never been opened by a gm | 1 - ticket has been opened by a gm
    //mplr->getSession()->sendPacket(&data);
    systemMessage(mplr->getSession(), "SYSTEM: Your ticket has been escalated. A Senior Game Master will be with you shortly!");
    return true;
}

bool ChatCommandHandler::HandleGMTicketReleaseCommand(const char* args, WorldSession* m_session)
{
    uint64_t ticketGuid = (args ? atoi(args) : 0);
    if (!ticketGuid)
    {
        redSystemMessage(m_session, "You must specify a ticket id.");
        return true;
    }

    Player* cplr = m_session->GetPlayer();
    Channel* chn = sChannelMgr.getChannel(sWorld.getGmClientChannel(), cplr);
    if (!chn)
        return false;

    GM_Ticket* ticket = sTicketMgr.getGMTicket(ticketGuid);
    if (ticket == nullptr || ticket->deleted)
    {
        chn->say(cplr, "GmTicket:0:Ticket not found.", cplr, true);
        return true;
    }

    if (ticket->assignedToPlayer == 0)
    {
        chn->say(cplr, "GmTicket:0:Ticket not assigned to a GM.", cplr, true);
        return true;
    }

    Player* plr = sObjectMgr.GetPlayer((uint32_t)ticket->assignedToPlayer);
    if (!cplr->getSession()->CanUseCommand('z') && plr != nullptr && plr->IsInWorld() && plr->getSession()->CanUseCommand('z'))
    {
        chn->say(cplr, "GmTicket:0:You can not release tickets from Senior Game Masters.", cplr, true);
        return true;
    }

    ticket->assignedToPlayer = 0;
    sTicketMgr.updateGMTicket(ticket);

    std::stringstream ss;
    ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_RELEASED;
    ss << ":" << ticket->guid;
    chn->say(cplr, ss.str().c_str(), nullptr, true);

    return true;
}

bool ChatCommandHandler::HandleGMTicketCommentCommand(const char* args, WorldSession* m_session)
{
    uint64_t ticketGuid;
    int argc = 1;
    char* comment = nullptr;
    char* guidString = (char*)args;

    // Parse arguments
    char* space = (char*)strchr(args, ' ');
    if (space)
    {
        *space = '\0';
        comment = space + 1;
        argc = 2;
    }

    ticketGuid = atoi(guidString);
    if (!ticketGuid)
    {
        redSystemMessage(m_session, "You must specify a ticket id.");
        return true;
    }

    Player* cplr = m_session->GetPlayer();
    Channel* chn = sChannelMgr.getChannel(sWorld.getGmClientChannel(), cplr);
    if (!chn)
        return false;

    GM_Ticket* ticket = sTicketMgr.getGMTicket(ticketGuid);
    if (ticket == nullptr || ticket->deleted)
    {
        chn->say(cplr, "GmTicket:0:Ticket not found.", cplr, true);
        return true;
    }

    if (ticket->assignedToPlayer != 0 && ticket->assignedToPlayer != cplr->getGuid() && !cplr->getSession()->CanUseCommand('z'))
    {
        chn->say(cplr, "GmTicket:0:Ticket is assigned to another GM.", cplr, true);
        return true;
    }

    ticket->comment = (argc == 1 ? "" : comment);
    sTicketMgr.updateGMTicket(ticket);

    std::stringstream ss;
    ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_COMMENT;
    ss << ":" << ticket->guid;
    ss << ":" << cplr->GetName();
    ss << ":" << ticket->comment;
    chn->say(cplr, ss.str().c_str(), nullptr, true);

    return true;
}

bool ChatCommandHandler::HandleGMTicketDeletePermanentCommand(const char* args, WorldSession* m_session)
{
    uint64_t ticketGuid = (args ? atoi(args) : 0);
    if (!ticketGuid)
    {
        redSystemMessage(m_session, "You must specify a ticket id.");
        return true;
    }

    Player* cplr = m_session->GetPlayer();
    Channel* chn = sChannelMgr.getChannel(sWorld.getGmClientChannel(), cplr);
    if (!chn)
        return false;

    GM_Ticket* ticket = sTicketMgr.getGMTicket(ticketGuid);
    if (ticket == nullptr)
    {
        chn->say(cplr, "GmTicket:0:Ticket not found.", cplr, true);
        return true;
    }

    Player* plr = nullptr;

    if (!ticket->deleted)
    {
        plr = sObjectMgr.GetPlayer((uint32_t)ticket->playerGuid);

        std::stringstream ss;
        ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_REMOVED;
        ss << ":" << ticket->guid;
        chn->say(cplr, ss.str().c_str(), nullptr, true);

        sTicketMgr.removeGMTicket(ticket->guid);
    }

    sTicketMgr.deleteGMTicketPermanently(ticket->guid);
    ticket = nullptr;
    if (plr != nullptr && plr->IsInWorld())
    {
        // Notify player about removing ticket
        plr->getSession()->sendPacket(SmsgGmTicketDeleteTicket(9).serialise().get());

        // Response - Send GM Survey
        plr->getSession()->sendPacket(SmsgGmTicketStatusUpdate(3).serialise().get());

        systemMessage(plr->getSession(), "You have been selected to fill out a GM Performance Survey. Please respond truthfully to the questions that you are asked and include the Game Masters name to your comment.");
    }

    return true;
}

#endif

bool ChatCommandHandler::HandleGMTicketToggleTicketSystemStatusCommand(const char* /*args*/, WorldSession* m_session)
{
    sWorld.toggleGmTicketStatus();

    if (sWorld.getGmTicketStatus())
        greenSystemMessage(m_session, "TicketSystem enabled.");
    else
        greenSystemMessage(m_session, "TicketSystem disabled.");

    return true;
}
