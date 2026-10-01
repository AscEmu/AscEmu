/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <ctime>
#include <string>

namespace AscEmu::Packets
{
    enum AuthResponseSendType
    {
        ARST_ONLY_ERROR = 0,
        ARST_QUEUE = 1,
        ARST_ACCOUNT_DATA = 2
    };

    enum AuthRespondError
    {
        AuthOkay = 0x0C,
        AuthFailed = 0x0D,
        AuthRejected = 0x0E,
        AuthUnknownAccount = 0x15,
        AuthWaitQueue = 0x1B,
    };

    struct SmsgAuthAccount
    {
        uint32_t billingTimeRemaining;
        uint8_t billingPlanFlags;
        uint32_t billingTimeRested;
        uint8_t expansion;
    };

    class SmsgAuthResponse : public ManagedPacket
    {
    public:
        uint8_t error;
        uint8_t sendType;
        uint32_t queuePosition;

        // realm the client joined, only sent to 6.2.4 and 7.3.5 clients
        std::string realmName;

        SmsgAuthResponse() : SmsgAuthResponse(0, 0, 0)
        {
        }

        SmsgAuthResponse(uint8_t error, uint8_t sendType, uint32_t queuePosition = 0) :
            ManagedPacket(SMSG_AUTH_RESPONSE, 1),
            error(error),
            sendType(sendType),
            queuePosition(queuePosition)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD())
                return serialiseWoD(packet);

            if (m_protocol.isLegion())
                return serialiseLegion(packet);

            SmsgAuthAccount accountInfo = { 0, 0, 0, static_cast<uint8_t>(m_protocol.expansion) };

            sLogger.debug("SmsgAuthResponse::internalSerialise: for expansion {}", accountInfo.expansion);

            switch (sendType)
            {
                case ARST_ONLY_ERROR:
                {
                    if (m_protocol.expansion <= WoW::Expansion::_WotLK)
                        packet << error;
                    else
                    {
                        packet.writeBit(0);                 // has account info
                        packet.writeBit(0);                 // has queue info
                        packet << error;
                    }

                } break;
                case ARST_QUEUE:
                {
                    if (queuePosition == 0)
                    {
                        error = AuthOkay;
                        if (m_protocol.expansion <= WoW::Expansion::_WotLK)
                            packet << error;

                        if (m_protocol.expansion == WoW::Expansion::_Cata)
                        {
                            packet.writeBit(0);             // has account info
                            packet.writeBit(0);             // has queue info
                            packet << error;
                            packet.flushBits();
                        }
                        if (m_protocol.expansion == WoW::Expansion::_Mop)
                        {
                            packet.writeBit(0);             // has account info
                            packet.writeBit(0);             // has queue info
                            packet << error;
                            packet.flushBits();
                        }
                    }
                    else
                    {
                        error = AuthWaitQueue;
                        if (m_protocol.expansion <= WoW::Expansion::_WotLK)
                        {
                            packet << error;
                            packet << queuePosition;

                            if (m_protocol.expansion == WoW::Expansion::_WotLK)
                                packet << uint8_t(0);
                        }
                        if (m_protocol.expansion == WoW::Expansion::_Cata)
                        {
                            packet.writeBit(1);             // has queue info
                            packet.writeBit(0);             // unk queue bool
                            packet.writeBit(0);             // has account info
                            packet.flushBits();
                            packet << error;
                            packet << queuePosition;
                        }
                        if (m_protocol.expansion == WoW::Expansion::_Mop)
                        {
                            packet.writeBit(0);             // has account info
                            packet.writeBit(1);             // has queue info
                            packet.writeBit(0);             // unk queue bool
                            packet << error;
                            packet.flushBits();
                            packet << queuePosition;
                        }
                    }
                } break;
                case ARST_ACCOUNT_DATA:
                {
                    if (m_protocol.expansion <= WoW::Expansion::_WotLK)
                    {
                        packet << error << accountInfo.billingTimeRemaining << accountInfo.billingPlanFlags;
                        packet << accountInfo.billingTimeRested << accountInfo.expansion;
                    }
                    if (m_protocol.expansion == WoW::Expansion::_Cata)
                    {
                        packet.writeBit(0);                 // has queue info
                        packet.writeBit(1);                 // has account Info
                        packet << accountInfo.billingTimeRemaining << accountInfo.expansion << uint32_t(0);
                        packet << accountInfo.expansion << accountInfo.billingTimeRested << accountInfo.billingPlanFlags;
                        packet << error;

                    }
                    if (m_protocol.expansion == WoW::Expansion::_Mop)
                    {
                        packet.writeBit(1);                 // has account info

                        uint8_t realmsCount = 0;
                        packet.writeBits(realmsCount, 21);  // send realms

                        packet.writeBits(11, 23);           // classes
                        packet.writeBits(0, 21);
                        packet.writeBit(0);
                        packet.writeBit(0);
                        packet.writeBit(0);
                        packet.writeBit(0);
                        packet.writeBits(15, 23);           // races
                        packet.writeBit(0);

                        packet.writeBit(0);                 // is queued

                        packet.flushBits();

                        // add expansion-race combination
                        packet << uint8_t(0) << uint8_t(1);
                        packet << uint8_t(0) << uint8_t(2);
                        packet << uint8_t(0) << uint8_t(3);
                        packet << uint8_t(0) << uint8_t(4);
                        packet << uint8_t(0) << uint8_t(5);
                        packet << uint8_t(0) << uint8_t(6);
                        packet << uint8_t(0) << uint8_t(7);
                        packet << uint8_t(0) << uint8_t(8);
                        packet << uint8_t(3) << uint8_t(9);
                        packet << uint8_t(1) << uint8_t(10);
                        packet << uint8_t(1) << uint8_t(11);
                        packet << uint8_t(3) << uint8_t(22);
                        packet << uint8_t(4) << uint8_t(24);
                        packet << uint8_t(4) << uint8_t(25);
                        packet << uint8_t(4) << uint8_t(26);

                        // add expansion-class combination
                        packet << uint8_t(0) << uint8_t(1);
                        packet << uint8_t(0) << uint8_t(2);
                        packet << uint8_t(0) << uint8_t(3);
                        packet << uint8_t(0) << uint8_t(4);
                        packet << uint8_t(0) << uint8_t(5);
                        packet << uint8_t(2) << uint8_t(6);
                        packet << uint8_t(0) << uint8_t(7);
                        packet << uint8_t(0) << uint8_t(8);
                        packet << uint8_t(0) << uint8_t(9);
                        packet << uint8_t(4) << uint8_t(10);
                        packet << uint8_t(0) << uint8_t(11);

                        packet << accountInfo.billingTimeRemaining;
                        packet << accountInfo.expansion;
                        packet << uint32_t(4);
                        packet << uint32_t(0);
                        packet << accountInfo.expansion;
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint32_t(0);

                        packet << error;
                    }
                }
            }

            return true;
        }

        // 6.2.4: battle.net result code, success and wait info behind two bits
        bool serialiseWoD(WorldPacket& packet)
        {
            constexpr uint32_t resultOk = 0;
            constexpr uint32_t resultDenied = 3;

            const bool queued = sendType == ARST_QUEUE && queuePosition != 0;
            const bool success = sendType == ARST_ACCOUNT_DATA && error == AuthOkay;
            const bool accepted = error == AuthOkay || sendType == ARST_QUEUE;

            packet << (accepted ? resultOk : resultDenied);
            packet.writeBit(success);
            packet.writeBit(queued);
            packet.flushBits();

            if (success)
            {
                // race and class ids with the expansion they need
                static constexpr uint8_t races[][2] = {
                    { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 }, { 6, 0 }, { 7, 0 }, { 8, 0 },
                    { 9, 3 }, { 10, 1 }, { 11, 1 }, { 22, 3 }, { 24, 4 }, { 25, 4 }, { 26, 4 }
                };
                static constexpr uint8_t classes[][2] = {
                    { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 }, { 6, 2 }, { 7, 0 }, { 8, 0 },
                    { 9, 0 }, { 10, 4 }, { 11, 0 }
                };

                const uint8_t expansion = static_cast<uint8_t>(m_protocol.expansion);
                const uint32_t realmAddress = m_protocol.getVirtualRealmAddress();

                std::string normalizedName = realmName;
                normalizedName.erase(std::remove_if(normalizedName.begin(), normalizedName.end(), [](unsigned char c) { return std::isspace(c) != 0; }), normalizedName.end());

                packet << realmAddress;
                packet << uint32_t(1);                  // virtual realms
                packet << uint32_t(0);                  // time rested
                packet << expansion;                    // active expansion
                packet << expansion;                    // account expansion
                packet << uint32_t(0);                  // seconds until pc kick
                packet << uint32_t(sizeof(races) / sizeof(races[0]));
                packet << uint32_t(sizeof(classes) / sizeof(classes[0]));
                packet << uint32_t(0);                  // character templates
                packet << uint32_t(0);                  // currency id

                packet << uint32_t(0);                  // billing plan
                packet << uint32_t(0);                  // billing time remaining
                packet.writeBit(0);                     // in game room, the client reads it three times
                packet.writeBit(0);
                packet.writeBit(0);
                packet.flushBits();

                packet << realmAddress;
                packet.writeBit(1);                     // is local
                packet.writeBit(0);                     // is internal
                packet.writeBits(static_cast<uint32_t>(realmName.size()), 8);
                packet.writeBits(static_cast<uint32_t>(normalizedName.size()), 8);
                packet.flushBits();
                packet.append(realmName.data(), realmName.size());
                packet.append(normalizedName.data(), normalizedName.size());

                for (const auto& race : races)
                    packet << race[0] << race[1];

                for (const auto& playerClass : classes)
                    packet << playerClass[0] << playerClass[1];

                packet.writeBit(0);                     // is expansion trial
                packet.writeBit(0);                     // force character template
                packet.writeBit(0);                     // has horde player count
                packet.writeBit(0);                     // has alliance player count
                packet.flushBits();
            }

            if (queued)
            {
                packet << queuePosition;                // wait count
                packet << uint32_t(0);                  // wait time
                packet.writeBit(0);                     // has fcm
                packet.flushBits();
            }

            return true;
        }

        // 7.3.5: classes only, game time, billing after the bits, realms after the billing
        bool serialiseLegion(WorldPacket& packet)
        {
            constexpr uint32_t resultOk = 0;
            constexpr uint32_t resultDenied = 3;

            const bool queued = sendType == ARST_QUEUE && queuePosition != 0;
            const bool success = sendType == ARST_ACCOUNT_DATA && error == AuthOkay;
            const bool accepted = error == AuthOkay || sendType == ARST_QUEUE;

            packet << (accepted ? resultOk : resultDenied);
            packet.writeBit(success);
            packet.writeBit(queued);
            packet.flushBits();

            if (success)
            {
                // class ids with the expansion they need
                static constexpr uint8_t classes[][2] = {
                    { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 }, { 6, 2 }, { 7, 0 }, { 8, 0 },
                    { 9, 0 }, { 10, 4 }, { 11, 0 }, { 12, 6 }
                };

                const uint8_t expansion = static_cast<uint8_t>(m_protocol.expansion);
                const uint32_t realmAddress = m_protocol.getVirtualRealmAddress();

                std::string normalizedName = realmName;
                normalizedName.erase(std::remove_if(normalizedName.begin(), normalizedName.end(), [](unsigned char c) { return std::isspace(c) != 0; }), normalizedName.end());

                packet << realmAddress;
                packet << uint32_t(1);                  // virtual realms
                packet << uint32_t(0);                  // time rested
                packet << expansion;                    // active expansion
                packet << expansion;                    // account expansion
                packet << uint32_t(0);                  // seconds until pc kick
                packet << uint32_t(sizeof(classes) / sizeof(classes[0]));
                packet << uint32_t(0);                  // character templates
                packet << uint32_t(0);                  // currency id
                packet << static_cast<int32_t>(std::time(nullptr));

                for (const auto& playerClass : classes)
                    packet << playerClass[0] << playerClass[1];

                packet.writeBit(0);                     // is expansion trial
                packet.writeBit(0);                     // force character template
                packet.writeBit(0);                     // has horde player count
                packet.writeBit(0);                     // has alliance player count
                packet.flushBits();

                packet << uint32_t(0);                  // billing plan
                packet << uint32_t(0);                  // billing time remaining
                packet << uint32_t(0);                  // billing unknown
                packet.writeBit(0);                     // in game room, the client reads it three times
                packet.writeBit(0);
                packet.writeBit(0);
                packet.flushBits();

                packet << realmAddress;
                packet.writeBit(1);                     // is local
                packet.writeBit(0);                     // is internal
                packet.writeBits(static_cast<uint32_t>(realmName.size()), 8);
                packet.writeBits(static_cast<uint32_t>(normalizedName.size()), 8);
                packet.flushBits();
                packet.append(realmName.data(), realmName.size());
                packet.append(normalizedName.data(), normalizedName.size());
            }

            if (queued)
            {
                packet << queuePosition;                // wait count
                packet << uint32_t(0);                  // wait time
                packet.writeBit(0);                     // has fcm
                packet.flushBits();
            }

            return true;
        }
    };
}
