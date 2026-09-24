/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BNetAuthTicketStore.hpp"

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>

namespace AscEmu::Battlenet
{
    namespace
    {
        struct TicketEntry
        {
            std::string login;
            std::chrono::steady_clock::time_point createdAt;
        };

        std::mutex ticketMutex;
        std::unordered_map<std::string, TicketEntry> tickets;

        void pruneExpiredTickets()
        {
            const auto cutoff = std::chrono::steady_clock::now() - std::chrono::minutes(2);

            for (auto itr = tickets.begin(); itr != tickets.end();)
            {
                if (itr->second.createdAt < cutoff)
                    itr = tickets.erase(itr);
                else
                    ++itr;
            }
        }
    }

    void storeWebAuthTicket(const std::string& ticket, const std::string& login)
    {
        if (ticket.empty() || login.empty())
            return;

        std::lock_guard<std::mutex> guard(ticketMutex);
        pruneExpiredTickets();

        tickets[ticket] = TicketEntry{
            login,
            std::chrono::steady_clock::now()
        };
    }

    bool consumeWebAuthTicket(const std::string& ticket, std::string& login)
    {
        login.clear();

        if (ticket.empty())
            return false;

        std::lock_guard<std::mutex> guard(ticketMutex);
        pruneExpiredTickets();

        const auto itr = tickets.find(ticket);
        if (itr == tickets.end())
            return false;

        login = itr->second.login;
        tickets.erase(itr);
        return true;
    }
}
