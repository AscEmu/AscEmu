/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "Management/Guild.h"

namespace AscEmu { namespace Packets
{
    class CmsgGuildSetRank : public ManagedPacket
    {
    public:
        uint32_t newRankId;
        uint32_t newRights;
        std::string rankName;
        uint32_t moneyPerDay;

        // cata specific
        uint32_t oldRankId = 0;
        uint32_t oldRights = 0;

        GuildBankRightsAndSlotsVec _rightsAndSlots;

        CmsgGuildSetRank() : CmsgGuildSetRank(0, 0, "", 0)
        {
        }

        CmsgGuildSetRank(uint32_t newRankId, uint32_t newRights, std::string rankName, uint32_t moneyPerDay) :
            ManagedPacket(CMSG_GUILD_SET_RANK, 4 + 4 + 1 + 4),
            newRankId(newRankId),
            newRights(newRights),
            rankName(rankName),
            moneyPerDay(moneyPerDay)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet >> newRankId >> newRights >> rankName >> moneyPerDay;
#else
            packet >> oldRankId >> oldRights >> newRights;
#endif

            GuildBankRightsAndSlotsVec rightsAndSlots(MAX_GUILD_BANK_TABS);
            for (uint8_t tabId = 0; tabId < MAX_GUILD_BANK_TABS; ++tabId)
            {
                uint8_t bankRights;
                uint32_t slots;

                packet >> bankRights;
                packet >> slots;
                rightsAndSlots[tabId] = GuildBankRightsAndSlots(tabId, bankRights, slots);

                _rightsAndSlots.push_back(rightsAndSlots[tabId]);
            }
#if VERSION_STRING >= Cata
            packet >> moneyPerDay >> newRankId;

            const uint32_t nameLength = packet.readBits(7);
            rankName = packet.ReadString(nameLength);
#endif
            return true;
        }
    };
}}
