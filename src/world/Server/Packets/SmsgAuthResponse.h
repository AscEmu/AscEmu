/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "world/Objects/Units/Players/PlayerDefines.hpp"
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

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

        uint32_t foreverVirtualRealmAddress = 0;
        uint32_t foreverVirtualRealmCount = 1;
        uint32_t foreverTimeRested = 0;
        uint8_t foreverActiveExpansionLevel = 0;
        uint8_t foreverAccountExpansionLevel = 0;
        uint32_t foreverTimeSecondsUntilPCKick = 0;
        uint32_t foreverCurrencyId = 40;
        uint32_t foreverBillingType = 2;
        uint32_t foreverMinutesRemaining = 0;
        uint32_t foreverRealBillingType = 2;
        bool foreverIsInIGR = false;
        bool foreverIsPaidForByIGR = false;
        bool foreverIsCAISEnabled = false;
        uint32_t foreverServerTime = 0;
        uint32_t foreverRealmAddress = 0;
        uint32_t foreverRealmUnknown = 0x46010002;
        bool foreverRealmIsLocal = true;
        bool foreverRealmIsInternal = false;
        std::string foreverRealmName = "Classic Beta PvE";
        std::string foreverRealmNameNormalized = "ClassicBetaPvE";

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
            if (m_protocol.isForever())
            {
                packet << uint32_t(0); // BattlenetRpcErrorCode::OK
                packet.writeBit(1);    // SuccessInfo
                packet.writeBit(0);    // WaitInfo
                packet.flushBits();

                packet << foreverVirtualRealmAddress;
                packet << foreverVirtualRealmCount;
                packet << foreverTimeRested;
                packet << foreverActiveExpansionLevel;
                packet << foreverAccountExpansionLevel;
                packet << foreverTimeSecondsUntilPCKick;
                packet << uint32_t(ForeverRaceClassAvailability.size());
                packet << uint32_t(0); // CharacterTemplates
                packet << foreverCurrencyId;

                packet << foreverBillingType;
                packet << foreverMinutesRemaining;
                packet << foreverRealBillingType;
                packet.writeBit(foreverIsInIGR);
                packet.writeBit(foreverIsPaidForByIGR);
                packet.writeBit(foreverIsCAISEnabled);
                packet.flushBits();
                packet << foreverServerTime;

                packet << foreverRealmAddress;
                packet << foreverRealmUnknown;
                packet.writeBit(foreverRealmIsLocal);
                packet.writeBit(foreverRealmIsInternal);
                packet.writeBits(foreverRealmName.size(), 8);
                packet.writeBits(foreverRealmNameNormalized.size(), 8);
                packet.flushBits();
                packet.writeString(foreverRealmName);
                packet.writeString(foreverRealmNameNormalized);

                for (const RaceClassAvailability& race : ForeverRaceClassAvailability)
                {
                    packet << uint8_t(race.race);
                    packet << uint32_t(race.classCount);
                    for (uint8_t i = 0; i < race.classCount; ++i)
                        packet << uint32_t(race.classes[i]);
                }

                packet.writeBit(0); // IsExpansionTrial
                packet.writeBit(0); // ForceCharacterTemplate
                packet.writeBit(0); // NumPlayersHorde
                packet.writeBit(0); // NumPlayersAlliance
                packet.writeBit(0); // ExpansionTrialExpiration
                packet.writeBit(0); // CurrentBuild
                packet.flushBits();
                return true;
            }

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
    };
}
