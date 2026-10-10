/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgFeatureSystemStatus : public ManagedPacket
    {
    public:
        uint8_t unknown1;
        uint8_t unknown2;

        SmsgFeatureSystemStatus() : SmsgFeatureSystemStatus(0, 0)
        {
        }

        SmsgFeatureSystemStatus(uint8_t unknown1, uint8_t unknown2) :
            ManagedPacket(SMSG_FEATURE_SYSTEM_STATUS, 0),
            unknown1(unknown1),
            unknown2(unknown2)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion > WoW::Expansion::_Cata)
            {
                return 4 + 4 + 4 + 1 + 4 + 2 + 4 + 4 + 4 + 4 + 4 + 4 + 4;
            }
            else if (m_protocol.expansion > WoW::Expansion::_WotLK)
            {
                return 1 + 4 + 4 + 4 + 4 + 2 + 4 + 4 + 4 + 4 + 4 + 4 + 4;
            }
            else
            {
                return 1 + 1;
            }
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            if (m_protocol.isWoD())
            {
                packet << uint8_t(2);                   // complaint status
                packet << uint32_t(1);                  // scroll of resurrection: requests remaining
                packet << uint32_t(1);                  // scroll of resurrection: requests per day
                packet << uint32_t(m_protocol.realmId); // realm id
                packet << int32_t(0);                   // realm record id
                packet << uint32_t(60);                 // twitter post throttle limit
                packet << uint32_t(20);                 // twitter post throttle cooldown
                packet << uint32_t(300);                // token poll time in seconds
                packet << uint32_t(0);                  // token redeem index

                packet.writeBit(0);                     // voice chat
                packet.writeBit(0);                     // has the ticket system status
                packet.writeBit(0);                     // scroll of resurrection
                packet.writeBit(0);                     // shop enabled
                packet.writeBit(0);                     // shop available
                packet.writeBit(0);                     // shop disabled by parental controls
                packet.writeBit(1);                     // item restoration button
                packet.writeBit(0);                     // browser
                packet.writeBit(0);                     // has a session alert
                packet.writeBit(0);                     // recruit a friend
                packet.writeBit(0);                     // character restore
                packet.writeBit(0);                     // restricted account
                packet.writeBit(1);                     // tutorials
                packet.writeBit(1);                     // new player tutorials
                packet.writeBit(0);                     // twitter
                packet.writeBit(0);                     // commerce system
                packet.writeBit(1);
                packet.writeBit(0);                     // will kick from world
                packet.writeBit(0);
                packet.flushBits();
            }
            else if (m_protocol.isShadowlands())
            {
                packet << uint8_t(2);                   // complaint status
                packet << uint32_t(1);                  // scroll of resurrection: requests remaining
                packet << uint32_t(1);                  // scroll of resurrection: requests per day
                packet << uint32_t(m_protocol.realmId); // realm id
                packet << int32_t(0);                   // realm record id
                packet << uint32_t(0);                  // recruit a friend: max recruits
                packet << uint32_t(0);                  // recruit a friend: max recruit months
                packet << uint32_t(0);                  // recruit a friend: max recruitment uses
                packet << uint32_t(0);                  // recruit a friend: days in cycle
                packet << uint32_t(60);                 // twitter post throttle limit
                packet << uint32_t(20);                 // twitter post throttle cooldown
                packet << uint32_t(300);                // token poll time in seconds
                packet << uint32_t(0);                  // kiosk session minutes
                packet << int64_t(0);                   // token balance
                packet << uint32_t(0);                  // shop delivery delay
                packet << uint32_t(0);                  // clubs presence update timer
                packet << uint32_t(0);                  // hidden ui clubs presence update timer
                packet << int32_t(0);                   // active season
                packet << uint32_t(0);                  // game rules
                packet << int16_t(50);                  // player name queries per packet
                packet << int16_t(600);                 // player name query telemetry interval

                packet.writeBit(0);                     // voice chat
                packet.writeBit(0);                     // has the ticket system status
                packet.writeBit(0);                     // scroll of resurrection
                packet.writeBit(0);                     // shop enabled
                packet.writeBit(0);                     // shop available
                packet.writeBit(0);                     // shop disabled by parental controls
                packet.writeBit(1);                     // item restoration button
                packet.writeBit(0);                     // browser
                packet.writeBit(0);                     // has a session alert
                packet.writeBit(0);                     // recruit a friend
                packet.writeBit(0);                     // recruit a friend: recruiting
                packet.writeBit(0);                     // character restore
                packet.writeBit(0);                     // restricted account
                packet.writeBit(0);                     // commerce system
                packet.writeBit(1);                     // tutorials
                packet.writeBit(0);                     // twitter
                packet.writeBit(1);
                packet.writeBit(0);                     // will kick from world
                packet.writeBit(0);                     // kiosk mode
                packet.writeBit(0);                     // competitive mode
                packet.writeBit(0);                     // token balance
                packet.writeBit(0);                     // war mode
                packet.writeBit(0);                     // clubs
                packet.writeBit(0);                     // clubs: battle.net club type
                packet.writeBit(0);                     // clubs: character club type
                packet.writeBit(0);                     // clubs: presence update
                packet.writeBit(0);                     // voice chat disabled by parental controls
                packet.writeBit(0);                     // voice chat muted by parental controls
                packet.writeBit(0);                     // quest session
                packet.writeBit(0);                     // is muted
                packet.writeBit(0);                     // club finder
                packet.writeBit(0);                     // checkout related
                packet.writeBit(0);                     // text to speech
                packet.writeBit(0);                     // chat disabled by default
                packet.writeBit(0);                     // chat disabled by the player
                packet.writeBit(0);                     // custom lfg list requires an authenticator
                packet.flushBits();

                // quick join: toasts and the throttle values of the social queue
                packet.writeBit(0);                     // toasts disabled
                packet.flushBits();
                packet << float(7) << float(10) << float(1);    // toast duration, delay, queue multiplier
                for (uint8_t i = 0; i < 19; ++i)
                    packet << float(0);

                // voice chat: squelched, battle.net account, guild
                packet.writeBit(0);
                packet.flushBits();
                packet << WoWGuid128();
                packet << WoWGuid128();
            }
            else if (m_protocol.isBfA())
            {
                packet << uint8_t(2);                   // complaint status
                packet << uint32_t(1);                  // scroll of resurrection: requests remaining
                packet << uint32_t(1);                  // scroll of resurrection: requests per day
                packet << uint32_t(m_protocol.realmId); // realm id
                packet << int32_t(0);                   // realm record id
                packet << uint32_t(0);                  // recruit a friend: max recruits
                packet << uint32_t(0);                  // recruit a friend: max recruit months
                packet << uint32_t(0);                  // recruit a friend: max recruitment uses
                packet << uint32_t(0);                  // recruit a friend: days in cycle
                packet << uint32_t(60);                 // twitter post throttle limit
                packet << uint32_t(20);                 // twitter post throttle cooldown
                packet << uint32_t(300);                // token poll time in seconds
                packet << int64_t(0);                   // token balance
                packet << uint32_t(0);                  // shop delivery delay
                packet << uint32_t(0);                  // clubs presence update timer
                packet << uint32_t(0);                  // hidden ui clubs presence update timer

                packet.writeBit(0);                     // voice chat
                packet.writeBit(0);                     // has the ticket system status
                packet.writeBit(0);                     // scroll of resurrection
                packet.writeBit(0);                     // shop enabled
                packet.writeBit(0);                     // shop available
                packet.writeBit(0);                     // shop disabled by parental controls
                packet.writeBit(1);                     // item restoration button
                packet.writeBit(0);                     // browser
                packet.writeBit(0);                     // has a session alert
                packet.writeBit(0);                     // recruit a friend
                packet.writeBit(0);                     // recruit a friend: recruiting
                packet.writeBit(0);                     // character restore
                packet.writeBit(0);                     // restricted account
                packet.writeBit(0);                     // commerce system
                packet.writeBit(1);                     // tutorials
                packet.writeBit(1);                     // new player tutorials
                packet.writeBit(0);                     // twitter
                packet.writeBit(1);
                packet.writeBit(0);                     // will kick from world
                packet.writeBit(0);                     // kiosk mode
                packet.writeBit(0);                     // competitive mode
                packet.writeBit(0);                     // token balance
                packet.writeBit(0);                     // war mode
                packet.writeBit(0);                     // clubs
                packet.writeBit(0);                     // clubs: battle.net club type
                packet.writeBit(0);                     // clubs: character club type
                packet.writeBit(0);                     // clubs: presence update
                packet.writeBit(0);                     // voice chat disabled by parental controls
                packet.writeBit(0);                     // voice chat muted by parental controls
                packet.writeBit(0);                     // quest session
                packet.writeBit(0);                     // is muted
                packet.writeBit(0);                     // club finder
                packet.flushBits();

                // quick join: toasts and the throttle values of the social queue
                packet.writeBit(0);                     // toasts disabled
                packet.flushBits();
                packet << float(7) << float(10) << float(1);    // toast duration, delay, queue multiplier
                for (uint8_t i = 0; i < 19; ++i)
                    packet << float(0);

                // voice chat: squelched, battle.net account, guild
                packet.writeBit(0);
                packet.flushBits();
                packet << WoWGuid128();
                packet << WoWGuid128();
            }
            else if (m_protocol.isLegion())
            {
                packet << uint8_t(2);                   // complaint status
                packet << uint32_t(1);                  // scroll of resurrection: requests remaining
                packet << uint32_t(1);                  // scroll of resurrection: requests per day
                packet << uint32_t(m_protocol.realmId); // realm id
                packet << int32_t(0);                   // realm record id
                packet << uint32_t(60);                 // twitter post throttle limit
                packet << uint32_t(20);                 // twitter post throttle cooldown
                packet << uint32_t(300);                // token poll time in seconds
                packet << uint32_t(0);                  // token redeem index
                packet << int64_t(0);                   // token balance
                packet << uint32_t(0);                  // shop delivery delay

                packet.writeBit(0);                     // voice chat
                packet.writeBit(0);                     // has the ticket system status
                packet.writeBit(0);                     // scroll of resurrection
                packet.writeBit(0);                     // shop enabled
                packet.writeBit(0);                     // shop available
                packet.writeBit(0);                     // shop disabled by parental controls
                packet.writeBit(1);                     // item restoration button
                packet.writeBit(0);                     // browser
                packet.writeBit(0);                     // has a session alert
                packet.writeBit(0);                     // recruit a friend
                packet.writeBit(0);                     // character restore
                packet.writeBit(0);                     // restricted account
                packet.writeBit(1);                     // tutorials
                packet.writeBit(1);                     // new player tutorials
                packet.writeBit(0);                     // twitter
                packet.writeBit(0);                     // commerce system
                packet.writeBit(1);
                packet.writeBit(0);                     // will kick from world
                packet.writeBit(0);                     // kiosk mode
                packet.writeBit(0);                     // competitive mode
                packet.writeBit(0);                     // has race and class expansion levels
                packet.writeBit(0);                     // token balance
                packet.flushBits();

                // quick join: toasts and the throttle values of the social queue
                packet.writeBit(0);                     // toasts disabled
                packet.flushBits();
                packet << float(7) << float(10) << float(1);    // toast duration, delay, queue multiplier
                for (uint8_t i = 0; i < 19; ++i)
                    packet << float(0);
            }
            else if (m_protocol.expansion == WoW::Expansion::_Cata)
            {
                bool featureBitFour = true;

                packet << unknown1 << uint32_t(1) << uint32_t(1) << uint32_t(2) << uint32_t(0);
                packet.writeBit(1);
                packet.writeBit(1);
                packet.writeBit(0);
                packet.writeBit(featureBitFour);
                packet.writeBit(0);
                packet.writeBit(0);
                packet.flushBits();

                if (featureBitFour)
                    packet << uint32_t(1) << uint32_t(0) << uint32_t(10) << uint32_t(60);
            }
            else if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                bool isFeedbackSysEnabled = false;
                bool isExcessiveWarningEnabled = false;

                packet << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint8_t(2) << uint32_t(0);
                packet.writeBit(0);
                packet.writeBit(1);
                packet.writeBit(0);
                packet.writeBit(0);
                packet.writeBit(0);
                packet.writeBit(1);
                packet.writeBit(0);
                packet.writeBit(isExcessiveWarningEnabled);
                packet.writeBit(0);
                packet.writeBit(isFeedbackSysEnabled);
                packet.flushBits();

                if (isExcessiveWarningEnabled)
                    packet << uint32_t(14400) << uint32_t(0) << uint32_t(0);

                if (isFeedbackSysEnabled)
                    packet << uint32_t(0) << uint32_t(1) << uint32_t(10) << uint32_t(60000);
            }
            else
            {
                packet << unknown1 << unknown2;
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
