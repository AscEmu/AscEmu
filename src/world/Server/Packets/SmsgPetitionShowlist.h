/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include <utility>

#include "ManagedPacket.h"
#include "Management/Guild/GuildDefinitions.h"

namespace AscEmu { namespace Packets
{
    class SmsgPetitionShowlist : public ManagedPacket
    {
    public:
        WoWGuid guid;
        bool showArenaCharters;

        SmsgPetitionShowlist() : SmsgPetitionShowlist(0, false)
        {
        }

        SmsgPetitionShowlist(uint64_t guid, bool showArenaCharters) :
            ManagedPacket(SMSG_PETITION_SHOWLIST, showArenaCharters ? 33 : 81),
            guid(guid),
            showArenaCharters(showArenaCharters)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid.GetOldGuid();


            if (showArenaCharters)
            {
                packet << uint8_t(3);   // charter count

                packet << uint32_t(1) << uint32_t(CharterEntry::TwoOnTwo) << uint32_t(CHARTER_DISPLAY_ID) << uint32_t(CharterCost::TwoOnTwo)
                    << uint32_t(CharterType::Arena) << uint32_t(CharterRequiredSigns::TwoOnTwo);

                packet << uint32_t(2) << uint32_t(CharterEntry::ThreeOnThree) << uint32_t(CHARTER_DISPLAY_ID) << uint32_t(CharterCost::ThreeOnThree)
                    << uint32_t(CharterType::Arena) << uint32_t(CharterRequiredSigns::ThreeOnThree);

                packet << uint32_t(3) << uint32_t(CharterEntry::FiveOnFive) << uint32_t(CHARTER_DISPLAY_ID) << uint32_t(CharterCost::FiveOnFive)
                    << uint32_t(CharterType::Arena) << uint32_t(CharterRequiredSigns::FiveOnFive);
            }
            else
            {
                packet << uint8_t(1);   // charter count

                packet << uint32_t(1) << uint32_t(CharterEntry::Guild) << uint32_t(CHARTER_DISPLAY_ID) << uint32_t(CharterCost::Guild)
                    << uint32_t(CharterType::Guild) << uint32_t(CharterRequiredSigns::Guild);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override { return false; }
    };
}}
