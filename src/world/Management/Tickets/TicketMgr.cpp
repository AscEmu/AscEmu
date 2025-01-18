/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "TicketMgr.hpp"

#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"

TicketMgr& TicketMgr::getInstance()
{
    static TicketMgr mInstance;
    return mInstance;
}

void TicketMgr::initialize()
{
    m_nextTicketId = 0;
    auto result = CharacterDatabase.Query("SELECT MAX(ticketid) FROM gm_tickets");
    if (result)
    {
        m_nextTicketId = result->Fetch()[0].asUint32();
        delete result;
    }

    sLogger.info("TicketMgr : HighGuid(TICKET) = {}", m_nextTicketId);
}
void TicketMgr::finalize()
{
    sLogger.info("TicketMgr : Deleting GM Tickets...");
    for (GmTicketList::iterator itr = m_ticketList.begin(); itr != m_ticketList.end(); ++itr)
        delete(*itr);
}

uint32_t TicketMgr::generateNextTicketId()
{
    return ++m_nextTicketId;
}

void TicketMgr::addGMTicket(GM_Ticket* ticket, bool startup)
{
    if (ticket)
    {
        m_ticketList.push_back(ticket);

        if (!startup)
            saveGMTicket(ticket, nullptr);
    }
}

void TicketMgr::loadGMTickets()
{
    QueryResult* result = CharacterDatabase.Query("SELECT ticketid, playerGuid, name, level, map, posX, posY, posZ, message, timestamp, deleted, assignedto, comment FROM gm_tickets");
    if (result == nullptr)
    {
        sLogger.info("TicketMgr : 0 active GM Tickets loaded.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        GM_Ticket* ticket = new GM_Ticket;
        ticket->guid = fields[0].asUint64();
        ticket->playerGuid = fields[1].asUint64();
        ticket->name = fields[2].asCString();
        ticket->level = fields[3].asUint32();
        ticket->map = fields[4].asUint32();
        ticket->posX = fields[5].asFloat();
        ticket->posY = fields[6].asFloat();
        ticket->posZ = fields[7].asFloat();
        ticket->message = fields[8].asCString();
        ticket->timestamp = fields[9].asUint32();
        ticket->deleted = fields[10].asUint32() == 1;
        ticket->assignedToPlayer = fields[11].asUint64();
        ticket->comment = fields[12].asCString();

        addGMTicket(ticket, true);
    } while (result->NextRow());

    sLogger.info("ObjectMgr : {} active GM Tickets loaded.", result->GetRowCount());
    delete result;
}

void TicketMgr::saveGMTicket(GM_Ticket* ticket, QueryBuffer* buf)
{
    std::stringstream ss;

    ss << "DELETE FROM gm_tickets WHERE ticketid = ";
    ss << ticket->guid;
    ss << ";";

    if (buf == nullptr)
        CharacterDatabase.ExecuteNA(ss.str().c_str());
    else
        buf->AddQueryStr(ss.str());

    ss.rdbuf()->str("");

    ss << "INSERT INTO gm_tickets (ticketid, playerguid, name, level, map, posX, posY, posZ, message, timestamp, deleted, assignedto, comment) VALUES(";
    ss << ticket->guid << ", ";
    ss << ticket->playerGuid << ", '";
    ss << CharacterDatabase.EscapeString(ticket->name) << "', ";
    ss << ticket->level << ", ";
    ss << ticket->map << ", ";
    ss << ticket->posX << ", ";
    ss << ticket->posY << ", ";
    ss << ticket->posZ << ", '";
    ss << CharacterDatabase.EscapeString(ticket->message) << "', ";
    ss << ticket->timestamp << ", ";

    if (ticket->deleted)
        ss << uint32(1);
    else
        ss << uint32(0);
    ss << ",";

    ss << ticket->assignedToPlayer << ", '";
    ss << CharacterDatabase.EscapeString(ticket->comment) << "');";

    if (buf == nullptr)
        CharacterDatabase.ExecuteNA(ss.str().c_str());
    else
        buf->AddQueryStr(ss.str());
}

void TicketMgr::updateGMTicket(GM_Ticket* ticket)
{
    saveGMTicket(ticket, nullptr);
}

void TicketMgr::deleteGMTicketPermanently(uint64 ticketGuid)
{
    for (GmTicketList::iterator i = m_ticketList.begin(); i != m_ticketList.end();)
    {
        if ((*i)->guid == ticketGuid)
        {
            i = m_ticketList.erase(i);
        }
        else
        {
            ++i;
        }
    }

    CharacterDatabase.Execute("DELETE FROM gm_tickets WHERE guid=%u", ticketGuid);      // kill from db
}

void TicketMgr::deleteAllRemovedGMTickets()
{
    for (GmTicketList::iterator i = m_ticketList.begin(); i != m_ticketList.end(); ++i)
    {
        if ((*i)->deleted)
        {
            i = m_ticketList.erase(i);
        }
        else
        {
            ++i;
        }
    }

    CharacterDatabase.Execute("DELETE FROM gm_tickets WHERE deleted=1");
}

void TicketMgr::removeGMTicketByPlayer(uint64 playerGuid)
{
    for (GmTicketList::iterator i = m_ticketList.begin(); i != m_ticketList.end(); ++i)
    {
        if ((*i)->playerGuid == playerGuid && !(*i)->deleted)
        {
            (*i)->deleted = true;
            saveGMTicket((*i), nullptr);
            break;
        }
    }
}

void TicketMgr::removeGMTicket(uint64 ticketGuid)
{
    for (GmTicketList::iterator i = m_ticketList.begin(); i != m_ticketList.end(); ++i)
    {
        if ((*i)->guid == ticketGuid && !(*i)->deleted)
        {
            (*i)->deleted = true;
            saveGMTicket((*i), nullptr);
            break;
        }
    }
}

void TicketMgr::closeTicket(uint64 ticketGuid)
{
    for (GmTicketList::iterator i = m_ticketList.begin(); i != m_ticketList.end(); ++i)
    {
        if ((*i)->guid == ticketGuid && !(*i)->deleted)
        {
            (*i)->deleted = true;
            break;
        }
    }
}

GM_Ticket* TicketMgr::getGMTicketByPlayer(uint64 playerGuid)
{
    for (GmTicketList::iterator i = m_ticketList.begin(); i != m_ticketList.end(); ++i)
    {
        if ((*i)->playerGuid == playerGuid && !(*i)->deleted)
        {
            return (*i);
        }
    }
    return nullptr;
}

GM_Ticket* TicketMgr::getGMTicket(uint64 ticketGuid)
{
    for (GmTicketList::iterator i = m_ticketList.begin(); i != m_ticketList.end(); ++i)
    {
        if ((*i)->guid == ticketGuid)
        {
            return (*i);
        }
    }
    return nullptr;
}
