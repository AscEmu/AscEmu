/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <array>
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgForeverEmptyCharacterList final : public ManagedPacket
    {
    public:
        SmsgForeverEmptyCharacterList() : ManagedPacket(SMSG_ENUM_CHARACTERS_RESULT, 430) {}

    protected:
        size_t expectedSize() const override { return 430; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            // [FOREVER-VERIFIED] Empty character-list header from the working 69893 capture.
            packet.writeBit(1);
            packet.writeBit(0);
            packet.writeBit(0);
            packet.writeBit(0);
            packet.writeBit(1);
            packet.writeBit(0);
            packet.writeBit(0);
            packet.writeBit(1);
            packet.writeBit(1);
            packet.writeBit(1);
            packet.flushBits();

            packet << uint32_t(0); // RegionwideCharacters
            packet << uint32_t(0); // Characters
            packet << int32_t(0);  // MaxCharacterLevel
            packet << uint32_t(RaceClassAvailability.size());
            packet << uint32_t(0); // UnlockedConditionalAppearances
            packet << uint32_t(0); // RaceLimitDisables
            packet << uint32_t(0); // WarbandGroups
            packet << uint32_t(0x7FFFFA20U); // ClassDisableMask

            for (const auto& availability : RaceClassAvailability)
            {
                packet << int8_t(availability.raceId);
                packet << uint32_t(availability.classCount);

                for (uint8_t i = 0; i < availability.classCount; ++i)
                {
                    packet << int8_t(availability.classes[i]);
                    packet << uint32_t(0); // AchievementID
                    packet.writeBit(1); // HasExpansion
                    packet.writeBit(1); // HasUnlockedAchievement
                    packet.writeBit(1); // HasEntitlement
                    packet.flushBits();
                }

                packet.writeBit(1); // HasUnlockedLicense
                packet.writeBit(0); // HasUnlockedAchievement
                packet.writeBit(0); // HasHeritageArmorUnlockAchievement
                packet.writeBit(1); // HasEntitlement
                packet.writeBit(0); // HideRaceOnClient
                packet.writeBit(0); // FactionBalanceDisabled
                packet.writeBit(0); // DoesNotHaveAvailableClasses
                packet.flushBits();
            }

            return packet.size() == expectedSize();
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }

    private:
        struct RaceAvailability
        {
            uint8_t raceId;
            uint8_t classCount;
            std::array<uint8_t, 7> classes;
        };

        static constexpr std::array<RaceAvailability, 10> RaceClassAvailability =
        {{
            { 95, 5, { 3, 4, 1, 8, 11, 0, 0 } },
            { 96, 5, { 3, 4, 7, 1, 11, 0, 0 } },
            {  1, 7, { 1, 2, 4, 5, 8, 9, 3 } },
            {  2, 6, { 1, 3, 4, 7, 9, 8, 0 } },
            {  3, 6, { 1, 2, 3, 5, 4, 7, 0 } },
            {  4, 5, { 1, 3, 4, 5, 11, 0, 0 } },
            {  5, 6, { 1, 4, 5, 8, 9, 2, 0 } },
            {  6, 4, { 1, 3, 7, 11, 0, 0, 0 } },
            {  7, 5, { 1, 4, 8, 9, 5, 0, 0 } },
            {  8, 7, { 1, 4, 3, 5, 7, 8, 9 } }
        }};
    };
}
