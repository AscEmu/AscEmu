/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

//\NOTE: This packet file is ugly as hell ;)

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

        SmsgAuthAccount accountInfo = {
            0, 0, 0,
#if VERSION_STRING == Classic
            0
#endif
#if VERSION_STRING == TBC
            1
#endif
#if VERSION_STRING == WotLK
            2
#endif
#if VERSION_STRING == Cata
            3
#endif
#if VERSION_STRING == Mop
            4
#endif
        };

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            switch (sendType)
            {
                case ARST_ONLY_ERROR:
                {
 #if VERSION_STRING <= WotLK
                    packet << error;
#else
                    packet.writeBit(0);     // has account info
                    packet.writeBit(0);     // has queue info
                    packet << error;
#endif
                } break;
                case ARST_QUEUE:
                {
                    if (queuePosition == 0)
                    {
                        error = AuthOkay;
#if VERSION_STRING <= WotLK
                        packet << error;
#endif 
#if VERSION_STRING == Cata
                        packet.writeBit(0); // has account info
                        packet.writeBit(0); // has queue info
                        packet << error;
                        packet.flushBits();
#endif 
#if VERSION_STRING == Mop
                        packet.writeBit(0); // has account info
                        packet.writeBit(0); // has queue info
                        packet << error;
                        packet.flushBits();
#endif    
                    }
                    else
                    {
                        error = AuthWaitQueue;
#if VERSION_STRING <= WotLK
                        packet << error;
                        packet << queuePosition;
    #if VERSION_STRING == WotLK
                        packet << uint8_t(0);
    #endif
#endif 
#if VERSION_STRING == Cata
                        packet.writeBit(1); // has queue info
                        packet.writeBit(0); // unk queue bool
                        packet.writeBit(0); // has account info
                        packet.flushBits();
                        packet << error;
                        packet << queuePosition;
#endif 
#if VERSION_STRING == Mop
                        packet.writeBit(0); // has account info
                        packet.writeBit(1); // has queue info
                        packet.writeBit(0); // unk queue bool
                        packet << error;
                        packet.flushBits();
                        packet << queuePosition;
#endif 
                    }
                } break;
                case ARST_ACCOUNT_DATA:
                {
#if VERSION_STRING <= WotLK
                    packet << error << accountInfo.billingTimeRemaining << accountInfo.billingPlanFlags << accountInfo.billingTimeRested << accountInfo.expansion;
                    
#endif
#if VERSION_STRING == Cata
                    packet.writeBit(0);       // has queue info
                    packet.writeBit(1);       // has account Info
                    packet << accountInfo.billingTimeRemaining << accountInfo.expansion << uint32_t(0)
                        << accountInfo.expansion << accountInfo.billingTimeRested << accountInfo.billingPlanFlags << error;
            
#endif
#if VERSION_STRING == Mop
                    packet.writeBit(1);     // has account info

                    uint8_t realmsCount = 0;
                    packet.writeBits(realmsCount, 21);      // send realms

                    packet.writeBits(11, 23);         // classes
                    packet.writeBits(0, 21);
                    packet.writeBit(0);
                    packet.writeBit(0);
                    packet.writeBit(0);
                    packet.writeBit(0);
                    packet.writeBits(15, 23);           // races
                    packet.writeBit(0);

                    packet.writeBit(0);                         // is queued

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
#endif
                }
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override { return false; }
    };
}
