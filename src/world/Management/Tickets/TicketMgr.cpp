/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "TicketMgr.hpp"
#include "Log.hpp"
#include "Storage/MySQLDataStore.hpp"

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
        m_nextTicketId = result->Fetch()[0].GetUInt32();
        delete result;
    }

    LogNotice("TicketMgr : HighGuid(TICKET) = %u", m_nextTicketId);
}
void TicketMgr::finalize()
{
    LogNotice("TicketMgr : Deleting GM Tickets...");
    for (GmTicketList::iterator itr = m_ticketList.begin(); itr != m_ticketList.end(); ++itr)
        delete(*itr);
}

uint32_t TicketMgr::generateNextTicketId()
{
    return ++m_nextTicketId;
}

void TicketMgr::addGMTicket(GM_Ticket* ticket, bool startup)
{
    ARCEMU_ASSERT(ticket != NULL);
    m_ticketList.push_back(ticket);

    if (!startup)
        saveGMTicket(ticket, nullptr);
}

void TicketMgr::loadGMTickets()
{
    QueryResult* result = CharacterDatabase.Query("SELECT ticketid, playerGuid, name, level, map, posX, posY, posZ, message, timestamp, deleted, assignedto, comment FROM gm_tickets");
    if (result == nullptr)
    {
        LogDetail("TicketMgr : 0 active GM Tickets loaded.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        GM_Ticket* ticket = new GM_Ticket;
        ticket->guid = fields[0].GetUInt64();
        ticket->playerGuid = fields[1].GetUInt64();
        ticket->name = fields[2].GetString();
        ticket->level = fields[3].GetUInt32();
        ticket->map = fields[4].GetUInt32();
        ticket->posX = fields[5].GetFloat();
        ticket->posY = fields[6].GetFloat();
        ticket->posZ = fields[7].GetFloat();
        ticket->message = fields[8].GetString();
        ticket->timestamp = fields[9].GetUInt32();
        ticket->deleted = fields[10].GetUInt32() == 1;
        ticket->assignedToPlayer = fields[11].GetUInt64();
        ticket->comment = fields[12].GetString();

        addGMTicket(ticket, true);
    } while (result->NextRow());

    LogDetail("ObjectMgr : %u active GM Tickets loaded.", result->GetRowCount());
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
