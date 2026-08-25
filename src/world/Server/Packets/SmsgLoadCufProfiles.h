/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/CUFProfileMgr.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLoadCufProfiles : public ManagedPacket
    {
    public:
        const CUFProfileMgr* cufProfiles = nullptr;

        SmsgLoadCufProfiles() : ManagedPacket(SMSG_LOAD_CUF_PROFILES, 1)
        {
        }

        SmsgLoadCufProfiles(const CUFProfileMgr* cufProfiles) :
            ManagedPacket(SMSG_LOAD_CUF_PROFILES, 1),
            cufProfiles(cufProfiles)
        {
        }

    protected:
        size_t expectedSize() const override { return 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
                return serialiseMop(packet);
            else if (m_protocol.isCata())
                return serialiseCata(packet);

            return false;
        }

        bool serialiseMop(WorldPacket& packet)
        {
            const uint8_t count = cufProfiles != nullptr ? cufProfiles->getCUFProfileCount() : 0;

            packet.writeBits(count, 20);

            if (cufProfiles != nullptr)
            {
                for (uint8_t i = 0; i < MAX_CUF_PROFILES; ++i)
                {
                    const CUFProfile* profile = cufProfiles->getCUFProfile(i);
                    if (profile == nullptr)
                        continue;

                    const auto& bits = profile->BoolOptions;

                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_SPEC_1]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_3_PLAYERS]);
                    packet.writeBit(bits[CUF_UNK_157]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_10_PLAYERS]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_40_PLAYERS]);
                    packet.writeBit(bits[CUF_DISPLAY_BORDER]);
                    packet.writeBit(bits[CUF_USE_CLASS_COLORS]);
                    packet.writeBit(bits[CUF_KEEP_GROUPS_TOGETHER]);
                    packet.writeBit(bits[CUF_DISPLAY_POWER_BAR]);
                    packet.writeBits(static_cast<uint32_t>(profile->ProfileName.size()), 8);
                    packet.writeBit(bits[CUF_DISPLAY_PETS]);
                    packet.writeBit(bits[CUF_DISPLAY_AGGRO_HIGHLIGHT]);
                    packet.writeBit(bits[CUF_UNK_145]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_PVP]);
                    packet.writeBit(bits[CUF_UNK_156]);
                    packet.writeBit(bits[CUF_DISPLAY_MAIN_TANK_AND_ASSIST]);
                    packet.writeBit(bits[CUF_DISPLAY_NON_BOSS_DEBUFFS]);
                    packet.writeBit(bits[CUF_DISPLAY_HORIZONTAL_GROUPS]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_SPEC_2]);
                    packet.writeBit(bits[CUF_DISPLAY_HEAL_PREDICTION]);
                    packet.writeBit(bits[CUF_DISPLAY_ONLY_DISPELLABLE_DEBUFFS]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_25_PLAYERS]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_PVE]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_5_PLAYERS]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_15_PLAYERS]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_2_PLAYERS]);
                }
            }

            packet.flushBits();

            if (cufProfiles != nullptr)
            {
                for (uint8_t i = 0; i < MAX_CUF_PROFILES; ++i)
                {
                    const CUFProfile* profile = cufProfiles->getCUFProfile(i);
                    if (profile == nullptr)
                        continue;

                    packet << uint16_t(profile->BottomOffset);
                    packet << uint16_t(profile->LeftOffset);
                    packet << uint8_t(profile->HealthText);
                    packet.writeString(profile->ProfileName);
                    packet << uint8_t(profile->BottomPoint);
                    packet << uint8_t(profile->TopPoint);
                    packet << uint16_t(profile->FrameHeight);
                    packet << uint8_t(profile->LeftPoint);
                    packet << uint8_t(profile->SortBy);
                    packet << uint16_t(profile->FrameWidth);
                    packet << uint16_t(profile->TopOffset);
                }
            }

            return true;
        }

        bool serialiseCata(WorldPacket& packet)
        {
            const uint8_t count = cufProfiles != nullptr ? cufProfiles->getCUFProfileCount() : 0;

            packet.writeBits(count, 20);

            if (cufProfiles != nullptr)
            {
                for (uint8_t i = 0; i < MAX_CUF_PROFILES; ++i)
                {
                    const CUFProfile* profile = cufProfiles->getCUFProfile(i);
                    if (profile == nullptr)
                        continue;

                    const auto& bits = profile->BoolOptions;

                    packet.writeBit(bits[CUF_UNK_157]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_10_PLAYERS]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_5_PLAYERS]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_25_PLAYERS]);
                    packet.writeBit(bits[CUF_DISPLAY_HEAL_PREDICTION]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_PVE]);
                    packet.writeBit(bits[CUF_DISPLAY_HORIZONTAL_GROUPS]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_40_PLAYERS]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_3_PLAYERS]);
                    packet.writeBit(bits[CUF_DISPLAY_AGGRO_HIGHLIGHT]);
                    packet.writeBit(bits[CUF_DISPLAY_BORDER]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_2_PLAYERS]);
                    packet.writeBit(bits[CUF_DISPLAY_NON_BOSS_DEBUFFS]);
                    packet.writeBit(bits[CUF_DISPLAY_MAIN_TANK_AND_ASSIST]);
                    packet.writeBit(bits[CUF_UNK_156]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_SPEC_2]);
                    packet.writeBit(bits[CUF_USE_CLASS_COLORS]);
                    packet.writeBit(bits[CUF_DISPLAY_POWER_BAR]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_SPEC_1]);
                    packet.writeBits(static_cast<uint32_t>(profile->ProfileName.size()), 8);
                    packet.writeBit(bits[CUF_DISPLAY_ONLY_DISPELLABLE_DEBUFFS]);
                    packet.writeBit(bits[CUF_KEEP_GROUPS_TOGETHER]);
                    packet.writeBit(bits[CUF_UNK_145]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_15_PLAYERS]);
                    packet.writeBit(bits[CUF_DISPLAY_PETS]);
                    packet.writeBit(bits[CUF_AUTO_ACTIVATE_PVP]);
                }
            }

            packet.flushBits();

            if (cufProfiles != nullptr)
            {
                for (uint8_t i = 0; i < MAX_CUF_PROFILES; ++i)
                {
                    const CUFProfile* profile = cufProfiles->getCUFProfile(i);
                    if (profile == nullptr)
                        continue;

                    packet << uint16_t(profile->LeftOffset);
                    packet << uint16_t(profile->FrameHeight);
                    packet << uint16_t(profile->BottomOffset);
                    packet << uint8_t(profile->BottomPoint);
                    packet << uint16_t(profile->TopOffset);
                    packet << uint8_t(profile->TopPoint);
                    packet << uint8_t(profile->HealthText);
                    packet << uint8_t(profile->SortBy);
                    packet << uint16_t(profile->FrameWidth);
                    packet << uint8_t(profile->LeftPoint);
                    packet.writeString(profile->ProfileName);
                }
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
