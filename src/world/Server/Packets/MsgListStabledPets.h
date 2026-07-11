/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

struct PlayerStablePet
{
    uint32_t petNumber;
    uint32_t entry;
    uint32_t level;
    utf8_string name;
};

namespace AscEmu::Packets
{
    class MsgListStabledPets : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t slotCount;
        std::map<uint8_t, PlayerStablePet> const& stableList;

        MsgListStabledPets() : MsgListStabledPets(0, 0, {})
        {
        }

        MsgListStabledPets(uint64_t guid, uint8_t slotCount, std::map<uint8_t, PlayerStablePet> const& stableList) :
            ManagedPacket(MSG_LIST_STABLED_PETS, 8),
            guid(guid),
            slotCount(slotCount),
            stableList(stableList)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_protocol.expansion >= WoW::Expansion::_Cata
                ? 10 + (stableList.size() * 29)
                : 10 + (stableList.size() * 25);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid.getRawGuid() << uint8_t(stableList.size()) << slotCount;
            for (const auto& [slot, stablePet] : stableList)
            {
                if (m_protocol.expansion >= WoW::Expansion::_Cata)
                    packet << int32_t(slot);

                packet << stablePet.petNumber;
                packet << stablePet.entry;
                packet << stablePet.level;
                packet << stablePet.name;
                // Seems to be some kind of flag; 1 for active pet, 2 or 3 for stabled pet. Any other value breaks the stable window.
                // TODO: verify values for cata
                packet << uint8_t(slot < PET_SLOT_MAX_ACTIVE_SLOT ? 1 : 2);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.init(unpackedGuid);
            return true;
        }
    };
}
