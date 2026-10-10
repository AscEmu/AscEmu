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
            if (m_protocol.isDragonflight())
            {
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
                packet.writeBit(0);                     // boost
                packet.writeBit(0);                     // trial boost
                packet.writeBit(0);                     // token balance
                packet.writeBit(0);                     // live region character list
                packet.writeBit(0);                     // live region character copy
                packet.writeBit(0);                     // live region account copy
                packet.writeBit(0);                     // live region key bindings copy
                packet.writeBit(0);                     // checkout related
                packet.writeBit(0);
                packet.writeBit(0);                     // has the ticket system status
                packet.writeBit(0);                     // name reservation
                packet.writeBit(0);                     // has a launch eta
                packet.writeBit(0);                     // timerunning
                packet.writeBit(0);                     // addons disabled
                packet.writeBit(0);
                packet.writeBit(0);                     // account save data export
                packet.writeBit(0);                     // account locked by export
                packet.writeBits(1, 11);                // length of the hidden realm alert with its terminator
                packet.flushBits();

                packet << uint32_t(300);                // token poll time in seconds
                packet << uint32_t(0);                  // kiosk session minutes
                packet << int64_t(0);                   // token balance
                packet << int32_t(10);                  // characters per realm
                packet << uint32_t(0);                  // live region copy source regions
                packet << uint32_t(0);                  // shop delivery delay
                packet << int32_t(0);                   // character upgrade boost
                packet << int32_t(0);                   // class trial boost
                packet << int32_t(0);                   // minimum expansion level
                packet << int32_t(maximumExpansionLevel);
                packet << int32_t(0);                   // active season
                packet << uint32_t(0);                  // game rules
                packet << int32_t(0);                   // active timerunning season
                packet << int32_t(0);                   // remaining timerunning season seconds
                packet << int16_t(50);                  // player name queries per packet
                packet << int16_t(600);                 // player name query telemetry interval
                packet << uint32_t(10);                 // player name query interval
                packet << uint32_t(0);                  // debug time events
                packet << int32_t(0);
                packet << uint32_t(0);                  // event realm queues
                return true;
            }

            if (m_protocol.isShadowlands())
            {
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
                packet.writeBit(0);                     // live region key bindings copy
                packet.writeBit(0);                     // checkout related
                packet.writeBit(0);                     // has the ticket system status
                packet.writeBit(0);                     // has a launch eta
                packet.flushBits();

                packet << uint32_t(300);                // token poll time in seconds
                packet << uint32_t(0);                  // kiosk session minutes
                packet << int64_t(0);                   // token balance
                packet << int32_t(10);                  // characters per realm
                packet << uint32_t(0);                  // live region copy source regions
                packet << uint32_t(0);                  // shop delivery delay
                packet << int32_t(0);                   // character upgrade boost
                packet << int32_t(0);                   // class trial boost
                packet << int32_t(0);                   // minimum expansion level
                packet << int32_t(maximumExpansionLevel);
                packet << int32_t(0);                   // active season
                packet << uint32_t(0);                  // game rules
                packet << int16_t(50);                  // player name queries per packet
                packet << int16_t(600);                 // player name query telemetry interval
                return true;
            }

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
