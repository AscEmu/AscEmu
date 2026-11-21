/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "CharacterRaceClasses.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // 1.60: the game rules of the character screen (rule, value, fraction)
    struct GameRule
    {
        int32_t id;
        int32_t value;
        float fraction;
    };

    inline constexpr GameRule foreverGameRules[] = {
        { 216, 1, 0.0f }, { 174, 1, 0.0f }, { 206, 1, 0.0f }, { 109, 1, 0.0f }, { 114, 1, 0.0f }, { 211, 1, 0.0f },
        { 112, 1, 0.0f }, { 173, 1, 0.0f }, { 154, 0, 0.0f }, { 180, 0, 0.0f }, { 181, 1, 0.0f }, { 182, 1, 0.0f },
        { 195, 1, 0.0f }, { 200, 0, 0.5f }, { 23, 1, 0.0f }, { 98, 0, 0.0f }, { 93, 2, 0.0f }, { 102, 1, 0.0f },
        { 42, 1, 0.0f }, { 40, 0, 0.0f }, { 157, 1, 0.0f }, { 190, 0, 0.0f }, { 162, 1, 0.0f }, { 209, 1, 0.0f },
        { 89, 1, 0.0f }, { 177, 1, 0.0f }, { 197, 1, 0.0f }, { 199, -5, 0.0f }, { 63, 1, 0.0f }, { 90, 1, 0.0f },
        { 3, 10, 0.0f }, { 33, 20, 0.0f }, { 129, 0, 0.0f }, { 5, 1787684400, 0.0f }, { 228, 1, 0.0f }, { 101, 1, 0.0f },
        { 237, 0, 0.0f }
    };

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
            if (m_protocol.isMidnight())
            {
                // 12.x: the bits without the parental controls and the v2 service values, the paid transfers behind the
                // commerce system, the bleep and navigation bar bits
                const bool forever = m_protocol.isForever();
                if (forever)
                {
                    // 1.60: 48 feature bits in another order, none of them set
                    for (uint8_t i = 0; i < 6; ++i)
                        packet << uint8_t(0);
                }
                else
                {
                    packet.writeBit(0);                     // shop available
                    packet.writeBit(0);                     // character restore
                    packet.writeBit(0);                     // commerce system
                    packet.writeBit(0);                     // paid character transfers between battle.net accounts
                    packet.writeBit(0);                     // veteran token redeem will kick
                    packet.writeBit(0);                     // world token redeem will kick
                    packet.writeBit(0);                     // expansion preorder in the shop
                    packet.writeBit(0);                     // kiosk mode
                    packet.writeBit(0);                     // competitive mode
                    packet.writeBit(0);                     // boost
                    packet.writeBit(0);                     // trial boost
                    packet.writeBit(0);                     // redeem for balance
                    packet.writeBit(0);                     // live region character list
                    packet.writeBit(0);                     // live region character copy
                    packet.writeBit(0);                     // live region account copy
                    packet.writeBit(0);                     // live region key bindings copy
                    packet.writeBit(0);                     // browser crash reporter
                    packet.writeBit(0);                     // employee account
                    packet.writeBit(0);                     // use bleep
                    packet.writeBit(0);                     // has the ticket system status
                    packet.writeBit(0);                     // name reservation only
                    packet.writeBit(0);                     // has a launch eta
                    packet.writeBit(0);                     // timerunning
                    packet.writeBit(0);                     // scripts disallowed for beta
                    packet.writeBit(0);                     // player identity options
                    packet.writeBit(0);                     // account export
                    packet.writeBit(0);                     // account locked after export
                    packet.writeBits(1, 11);                // length of the hidden realm alert with its terminator
                    packet.writeBit(0);                     // realmless character select list
                    packet.writeBit(0);                     // token limited mode
                    packet.writeBit(0);                     // navigation bar
                    packet.writeBit(0);                     // global user generated content mute
                    packet.writeBit(0);                     // account user generated content is risky
                    packet.flushBits();
                }
                packet << uint32_t(300);                // commerce price poll time in seconds
                if (!forever)
                    packet << uint32_t(0);              // kiosk session minutes
                packet << int64_t(0);                   // redeem for balance amount
                packet << int32_t(10);                  // characters per realm
                packet << uint32_t(0);                  // live region copy source regions
                packet << int32_t(0);                   // character upgrade boost
                packet << int32_t(0);                   // class trial boost
                packet << int32_t(0);                   // minimum expansion level
                packet << int32_t(forever ? 0 : maximumExpansionLevel);
                packet << int32_t(forever ? foreverContentSetId : 0);   // content set
                packet << uint32_t(0);                  // disabled game modes
                packet << uint32_t(forever ? sizeof(foreverGameRules) / sizeof(foreverGameRules[0]) : 0);   // game rules
                packet << uint32_t(0);                  // available game modes
                packet << int32_t(0);                   // active timerunning season
                packet << int32_t(0);                   // remaining timerunning season seconds
                packet << int32_t(forever ? 86400 : 0); // timerunning conversion: minimum character age in seconds
                packet << int32_t(forever ? -1 : 0);    // timerunning conversion: maximum season
                packet << int16_t(50);                  // player guid lookups per request
                packet << int16_t(600);                 // name lookup telemetry interval
                packet << uint32_t(10);                 // not found cache time in seconds
                packet << uint32_t(0);                  // debug time events
                packet << int32_t(0);                   // most recent time event
                packet << uint32_t(0);                  // event realm queues
                if (forever)
                {
                    for (const auto& rule : foreverGameRules)
                        packet << rule.id << rule.value << rule.fraction;
                }
                return true;
            }
            if (m_protocol.isTheWarWithinOrLater())
            {
                packet.writeBit(0);                     // shop available
                packet.writeBit(0);                     // shop disabled by parental controls
                packet.writeBit(0);                     // character restore
                packet.writeBit(0);                     // commerce system
                packet.writeBit(0);                     // veteran token redeem will kick
                packet.writeBit(0);                     // world token redeem will kick
                packet.writeBit(0);                     // expansion preorder in the shop
                packet.writeBit(0);                     // kiosk mode
                packet.writeBit(0);                     // competitive mode
                packet.writeBit(0);                     // boost
                packet.writeBit(0);                     // trial boost
                packet.writeBit(0);                     // redeem for balance
                packet.writeBit(0);                     // paid character transfers between battle.net accounts
                packet.writeBit(0);                     // live region character list
                packet.writeBit(0);                     // live region character copy
                packet.writeBit(0);                     // live region account copy
                packet.writeBit(0);                     // live region key bindings copy
                packet.writeBit(0);                     // browser crash reporter
                packet.writeBit(0);                     // employee account
                packet.writeBit(0);                     // has the ticket system status
                packet.writeBit(0);                     // name reservation only
                packet.writeBit(0);                     // has a launch eta
                packet.writeBit(0);                     // timerunning
                packet.writeBit(0);                     // scripts disallowed for beta
                packet.writeBit(0);                     // player identity options
                packet.writeBit(0);                     // account export
                packet.writeBit(0);                     // account locked after export
                packet.writeBits(1, 11);                // length of the hidden realm alert with its terminator
                packet.writeBit(0);                     // battle.net whisper: v2 services
                packet.writeBit(0);                     // battle.net game data: v2 services
                packet.writeBit(0);                     // realmless character select list
                packet.writeBit(0);                     // token limited mode
                packet.writeBit(0);
                packet.writeBit(0);
                packet.writeBit(0);                     // pandaren level boost allowed
                packet.flushBits();

                packet << uint32_t(300);                // commerce price poll time in seconds
                packet << uint32_t(0);                  // kiosk session minutes
                packet << int64_t(0);                   // redeem for balance amount
                packet << int32_t(10);                  // characters per realm
                packet << uint32_t(0);                  // live region copy source regions
                packet << int32_t(0);                   // character upgrade boost
                packet << int32_t(0);                   // class trial boost
                packet << int32_t(0);                   // minimum expansion level
                packet << int32_t(maximumExpansionLevel);
                packet << int32_t(0);                   // content set
                packet << uint32_t(0);                  // disabled game modes
                packet << uint32_t(0);                  // game rules
                packet << uint32_t(0);                  // available game modes
                packet << int32_t(0);                   // active timerunning season
                packet << int32_t(0);                   // remaining timerunning season seconds
                packet << int32_t(0);                   // timerunning conversion: minimum character age in seconds
                packet << int32_t(0);                   // timerunning conversion: maximum season
                packet << int16_t(50);                  // player guid lookups per request
                packet << int16_t(600);                 // name lookup telemetry interval
                packet << uint32_t(10);                 // not found cache time in seconds
                packet << uint32_t(0);                  // debug time events
                packet << int32_t(0);                   // most recent time event
                packet << uint32_t(0);                  // event realm queues
                return true;
            }

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
