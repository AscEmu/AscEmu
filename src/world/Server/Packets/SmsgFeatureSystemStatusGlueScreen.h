/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    // 8.x: the features of the character screen, sent after the authentication
    class SmsgFeatureSystemStatusGlueScreen : public ManagedPacket
    {
    public:
        uint8_t maximumExpansionLevel;

        SmsgFeatureSystemStatusGlueScreen() : SmsgFeatureSystemStatusGlueScreen(0)
        {
        }

        SmsgFeatureSystemStatusGlueScreen(uint8_t maximumExpansionLevel) :
            ManagedPacket(SMSG_FEATURE_SYSTEM_STATUS_GLUE_SCREEN, 40),
            maximumExpansionLevel(maximumExpansionLevel)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isBfA())
                return false;

            packet.writeBit(0);                     // shop enabled
            packet.writeBit(0);                     // shop available
            packet.writeBit(0);                     // shop disabled by parental controls
            packet.writeBit(0);                     // character restore
            packet.writeBit(0);                     // commerce system
            packet.writeBit(0);
            packet.writeBit(0);                     // will kick from world
            packet.writeBit(0);                     // expansion preorder in the shop
            packet.writeBit(0);                     // kiosk mode
            packet.writeBit(0);                     // competitive mode
            packet.writeBit(0);                     // trial boost
            packet.writeBit(0);                     // token balance
            packet.writeBit(0);                     // live region character list
            packet.writeBit(0);                     // live region character copy
            packet.writeBit(0);                     // live region account copy
            packet.flushBits();

            packet << uint32_t(300);                // token poll time in seconds
            packet << int64_t(0);                   // token balance
            packet << int32_t(10);                  // characters per realm
            packet << uint32_t(0);                  // shop delivery delay
            packet << int32_t(0);                   // character upgrade boost
            packet << int32_t(0);                   // class trial boost
            packet << int32_t(0);                   // minimum expansion level
            packet << int32_t(maximumExpansionLevel);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
