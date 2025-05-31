/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "TicketMgr.hpp"

#include "Logging/Logger.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/Packets/CmsgGmTicketCreate.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"

GM_Ticket::GM_Ticket(Player const* player, AscEmu::Packets::CmsgGmTicketCreate const& srlPacket) :
    deleted(false), assignedToPlayer(0), comment("")
{
    guid = static_cast<uint64_t>(sTicketMgr.generateNextTicketId());
    playerGuid = player->getGuid();
    name = player->getName();
    level = player->getLevel();
    map = srlPacket.map;
    posX = srlPacket.location.x;
    posY = srlPacket.location.y;
    posZ = srlPacket.location.z;
    message = srlPacket.message;
    timestamp = static_cast<uint32_t>(UNIXTIME);
}

GM_Ticket::GM_Ticket(Field const* fields)
{
    guid = fields[0].asUint64();
    playerGuid = fields[1].asUint64();
    name = fields[2].asCString();
    level = fields[3].asUint32();
    map = fields[4].asUint32();
    posX = fields[5].asFloat();
    posY = fields[6].asFloat();
    posZ = fields[7].asFloat();
    message = fields[8].asCString();
    timestamp = fields[9].asUint32();
    deleted = fields[10].asUint32() == 1;
    assignedToPlayer = fields[11].asUint64();
    comment = fields[12].asCString();
}

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
    }

    sLogger.info("TicketMgr : HighGuid(TICKET) = {}", m_nextTicketId);
}
void TicketMgr::finalize()
{
    sLogger.info("TicketMgr : Deleting GM Tickets...");
    m_ticketList.clear();
}

uint32_t TicketMgr::generateNextTicketId()
{
    return ++m_nextTicketId;
}

GM_Ticket* TicketMgr::createGMTicket(Player const* player, AscEmu::Packets::CmsgGmTicketCreate const& srlPacket)
{
    auto* ticket = m_ticketList.emplace_back(std::make_unique<GM_Ticket>(player, srlPacket)).get();
    saveGMTicket(ticket, nullptr);
    return ticket;
}

GM_Ticket* TicketMgr::createGMTicket(Field const* fields)
{
    return m_ticketList.emplace_back(std::make_unique<GM_Ticket>(fields)).get();
}

void TicketMgr::loadGMTickets()
{
    auto result = CharacterDatabase.Query("SELECT ticketid, playerGuid, name, level, map, posX, posY, posZ, message, timestamp, deleted, assignedto, comment FROM gm_tickets");
    if (result == nullptr)
    {
        sLogger.info("TicketMgr : 0 active GM Tickets loaded.");
        return;
    }

    do
    {
        createGMTicket(result->Fetch());

    } while (result->NextRow());

    sLogger.info("ObjectMgr : {} active GM Tickets loaded.", result->GetRowCount());
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
        ss << uint32_t(1);
    else
        ss << uint32_t(0);
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

void TicketMgr::deleteGMTicketPermanently(uint64_t ticketGuid)
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

void TicketMgr::removeGMTicketByPlayer(uint64_t playerGuid)
{
    for (GmTicketList::iterator i = m_ticketList.begin(); i != m_ticketList.end(); ++i)
    {
        if ((*i)->playerGuid == playerGuid && !(*i)->deleted)
        {
            (*i)->deleted = true;
            saveGMTicket((*i).get(), nullptr);
            break;
        }
    }
}

void TicketMgr::removeGMTicket(uint64_t ticketGuid)
{
    for (GmTicketList::iterator i = m_ticketList.begin(); i != m_ticketList.end(); ++i)
    {
        if ((*i)->guid == ticketGuid && !(*i)->deleted)
        {
            (*i)->deleted = true;
            saveGMTicket((*i).get(), nullptr);
            break;
        }
    }
}

void TicketMgr::closeTicket(uint64_t ticketGuid)
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

GM_Ticket* TicketMgr::getGMTicketByPlayer(uint64_t playerGuid)
{
    for (GmTicketList::iterator i = m_ticketList.begin(); i != m_ticketList.end(); ++i)
    {
        if ((*i)->playerGuid == playerGuid && !(*i)->deleted)
        {
            return (*i).get();
        }
    }
    return nullptr;
}

GM_Ticket* TicketMgr::getGMTicket(uint64_t ticketGuid)
{
    for (GmTicketList::iterator i = m_ticketList.begin(); i != m_ticketList.end(); ++i)
    {
        if ((*i)->guid == ticketGuid)
        {
            return (*i).get();
        }
    }
    return nullptr;
}
