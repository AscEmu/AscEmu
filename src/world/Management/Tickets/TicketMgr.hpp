/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string>
#include <list>
#include <atomic>
#include "CommonTypes.hpp"

struct GM_Ticket
{
    uint64_t guid;
    uint64_t playerGuid;
    std::string name;
    uint32_t level;
    uint32_t map;
    float posX;
    float posY;
    float posZ;
    std::string message;
    uint32_t timestamp;
    bool deleted;
    uint64_t assignedToPlayer;
    std::string comment;
};

enum
{
    GM_TICKET_CHAT_OPCODE_NEWTICKET = 1,
    GM_TICKET_CHAT_OPCODE_LISTSTART = 2,
    GM_TICKET_CHAT_OPCODE_LISTENTRY = 3,
    GM_TICKET_CHAT_OPCODE_CONTENT = 4,
    GM_TICKET_CHAT_OPCODE_APPENDCONTENT = 5,
    GM_TICKET_CHAT_OPCODE_REMOVED = 6,
    GM_TICKET_CHAT_OPCODE_UPDATED = 7,
    GM_TICKET_CHAT_OPCODE_ASSIGNED = 8,
    GM_TICKET_CHAT_OPCODE_RELEASED = 9,
    GM_TICKET_CHAT_OPCODE_COMMENT = 10,
    GM_TICKET_CHAT_OPCODE_ONLINESTATE = 11
};

typedef std::list<GM_Ticket*> GmTicketList;

class QueryBuffer;

class SERVER_DECL TicketMgr
{
private:
    TicketMgr() = default;
    ~TicketMgr() = default;

public:
    //NIT
    static TicketMgr& getInstance();
    void initialize();
    void finalize();

    TicketMgr(TicketMgr&&) = delete;
    TicketMgr(TicketMgr const&) = delete;
    TicketMgr& operator=(TicketMgr&&) = delete;
    TicketMgr& operator=(TicketMgr const&) = delete;

    uint32_t generateNextTicketId();
    void addGMTicket(GM_Ticket* ticket, bool startup);
    void loadGMTickets();
    void saveGMTicket(GM_Ticket* ticket, QueryBuffer* buf);
    void updateGMTicket(GM_Ticket* ticket);

    void removeGMTicketByPlayer(uint64 playerGuid);
    void removeGMTicket(uint64 ticketGuid);
    void closeTicket(uint64 ticketGuid);
    void deleteGMTicketPermanently(uint64 ticketGuid);
    void deleteAllRemovedGMTickets();
    GM_Ticket* getGMTicket(uint64 ticketGuid);
    GM_Ticket* getGMTicketByPlayer(uint64 playerGuid);

    GmTicketList m_ticketList;

protected:
    uint32_t m_nextTicketId = 0;
};

#define sTicketMgr TicketMgr::getInstance()
