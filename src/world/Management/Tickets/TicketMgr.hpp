/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <list>
#include <memory>
#include <string>

#include "CommonTypes.hpp"

class Field;
class Player;
class QueryBuffer;

namespace AscEmu::Packets
{
    class CmsgGmTicketCreate;
}

struct GM_Ticket
{
    GM_Ticket(Player const* player, AscEmu::Packets::CmsgGmTicketCreate const& srlPacket);
    GM_Ticket(Field const* fields);

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

typedef std::list<std::unique_ptr<GM_Ticket>> GmTicketList;

class SERVER_DECL TicketMgr
{
private:
    TicketMgr() = default;
    ~TicketMgr() = default;

public:
    static TicketMgr& getInstance();
    void initialize();
    void finalize();

    TicketMgr(TicketMgr&&) = delete;
    TicketMgr(TicketMgr const&) = delete;
    TicketMgr& operator=(TicketMgr&&) = delete;
    TicketMgr& operator=(TicketMgr const&) = delete;

    uint32_t generateNextTicketId();
    GM_Ticket* createGMTicket(Player const* player, AscEmu::Packets::CmsgGmTicketCreate const& srlPacket);
    GM_Ticket* createGMTicket(Field const* fields);
    void loadGMTickets();
    void saveGMTicket(GM_Ticket* ticket, QueryBuffer* buf);
    void updateGMTicket(GM_Ticket* ticket);

    void removeGMTicketByPlayer(uint64_t playerGuid);
    void removeGMTicket(uint64_t ticketGuid);
    void closeTicket(uint64_t ticketGuid);
    void deleteGMTicketPermanently(uint64_t ticketGuid);
    void deleteAllRemovedGMTickets();
    GM_Ticket* getGMTicket(uint64_t ticketGuid);
    GM_Ticket* getGMTicketByPlayer(uint64_t playerGuid);

    GmTicketList m_ticketList;

protected:
    uint32_t m_nextTicketId = 0;
};

#define sTicketMgr TicketMgr::getInstance()
