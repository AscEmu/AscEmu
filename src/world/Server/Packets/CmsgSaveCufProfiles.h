/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/CUFProfileMgr.h"
#include <array>
#include <cstdint>
#include <memory>

namespace AscEmu::Packets
{
    class CmsgSaveCufProfiles : public ManagedPacket
    {
    public:
        uint8_t profileCount = 0;
        std::array<std::unique_ptr<CUFProfile>, MAX_CUF_PROFILES> profiles;

        CmsgSaveCufProfiles() : ManagedPacket(CMSG_SAVE_CUF_PROFILES, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
                return deserialiseMop(packet);
            else if (m_protocol.isCata())
                return deserialiseCata(packet);

            return false;
        }

        // Mop 5.4.8 bit layout, verified against ProjectSkyfire/SkyFire_548
        bool deserialiseMop(WorldPacket& packet)
        {
            profileCount = static_cast<uint8_t>(packet.readBits(19));
            if (profileCount > MAX_CUF_PROFILES)
                return false;

            std::array<std::bitset<CUF_BOOL_OPTIONS_COUNT>, MAX_CUF_PROFILES> boolOptions{};
            std::array<uint8_t, MAX_CUF_PROFILES> nameLength{};

            for (uint8_t i = 0; i < profileCount; ++i)
            {
                boolOptions[i].set(CUF_AUTO_ACTIVATE_SPEC_2, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_MAIN_TANK_AND_ASSIST, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_POWER_BAR, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_10_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_3_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_UNK_156, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_40_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_2_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_KEEP_GROUPS_TOGETHER, packet.readBit());
                boolOptions[i].set(CUF_USE_CLASS_COLORS, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_25_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_UNK_145, packet.readBit());
                nameLength[i] = static_cast<uint8_t>(packet.readBits(7));
                boolOptions[i].set(CUF_DISPLAY_PETS, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_PVP, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_ONLY_DISPELLABLE_DEBUFFS, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_NON_BOSS_DEBUFFS, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_15_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_UNK_157, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_BORDER, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_HORIZONTAL_GROUPS, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_SPEC_1, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_5_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_PVE, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_HEAL_PREDICTION, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_AGGRO_HIGHLIGHT, packet.readBit());
            }

            for (uint8_t i = 0; i < profileCount; ++i)
            {
                uint16_t frameHeight = 0, frameWidth = 0, topOffset = 0, bottomOffset = 0, leftOffset = 0;
                uint8_t topPoint = 0, bottomPoint = 0, leftPoint = 0, sortBy = 0, healthText = 0;

                packet >> frameHeight;
                packet >> topPoint;
                packet >> healthText;
                packet >> frameWidth;
                packet >> leftPoint;
                packet >> sortBy;
                packet >> topOffset;
                const std::string profileName = packet.readString(nameLength[i]);
                packet >> bottomPoint;
                packet >> bottomOffset;
                packet >> leftOffset;

                profiles[i] = std::make_unique<CUFProfile>(profileName, frameHeight, frameWidth, sortBy, healthText,
                    static_cast<uint32_t>(boolOptions[i].to_ulong()), topPoint, bottomPoint, leftPoint, topOffset, bottomOffset, leftOffset);
            }

            return true;
        }

        // Cataclysm 4.3.4 bit layout, verified against The-Cataclysm-Preservation-Project/TrinityCore
        bool deserialiseCata(WorldPacket& packet)
        {
            profileCount = static_cast<uint8_t>(packet.readBits(20));
            if (profileCount > MAX_CUF_PROFILES)
                return false;

            std::array<std::bitset<CUF_BOOL_OPTIONS_COUNT>, MAX_CUF_PROFILES> boolOptions{};
            std::array<uint8_t, MAX_CUF_PROFILES> nameLength{};

            for (uint8_t i = 0; i < profileCount; ++i)
            {
                boolOptions[i].set(CUF_AUTO_ACTIVATE_SPEC_2, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_10_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_UNK_157, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_HEAL_PREDICTION, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_SPEC_1, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_PVP, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_POWER_BAR, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_15_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_40_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_PETS, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_5_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_ONLY_DISPELLABLE_DEBUFFS, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_2_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_UNK_156, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_NON_BOSS_DEBUFFS, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_MAIN_TANK_AND_ASSIST, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_AGGRO_HIGHLIGHT, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_3_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_BORDER, packet.readBit());
                boolOptions[i].set(CUF_USE_CLASS_COLORS, packet.readBit());
                boolOptions[i].set(CUF_UNK_145, packet.readBit());
                nameLength[i] = static_cast<uint8_t>(packet.readBits(8));
                boolOptions[i].set(CUF_AUTO_ACTIVATE_PVE, packet.readBit());
                boolOptions[i].set(CUF_DISPLAY_HORIZONTAL_GROUPS, packet.readBit());
                boolOptions[i].set(CUF_AUTO_ACTIVATE_25_PLAYERS, packet.readBit());
                boolOptions[i].set(CUF_KEEP_GROUPS_TOGETHER, packet.readBit());
            }

            for (uint8_t i = 0; i < profileCount; ++i)
            {
                uint16_t frameHeight = 0, frameWidth = 0, topOffset = 0, bottomOffset = 0, leftOffset = 0;
                uint8_t topPoint = 0, bottomPoint = 0, leftPoint = 0, sortBy = 0, healthText = 0;

                packet >> topPoint;
                const std::string profileName = packet.readString(nameLength[i]);
                packet >> bottomOffset;
                packet >> frameHeight;
                packet >> frameWidth;
                packet >> topOffset;
                packet >> healthText;
                packet >> bottomPoint;
                packet >> sortBy;
                packet >> leftOffset;
                packet >> leftPoint;

                profiles[i] = std::make_unique<CUFProfile>(profileName, frameHeight, frameWidth, sortBy, healthText,
                    static_cast<uint32_t>(boolOptions[i].to_ulong()), topPoint, bottomPoint, leftPoint, topOffset, bottomOffset, leftOffset);
            }

            return true;
        }

        bool internalSerialise(WorldPacket& /*packet*/) override { return false; }
    };
}
