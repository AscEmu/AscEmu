/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgFeatureSystemStatus : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint8_t unknown1;
        uint8_t unknown2;

        SmsgFeatureSystemStatus() : SmsgFeatureSystemStatus(0, 0)
        {
        }

        SmsgFeatureSystemStatus(uint8_t unknown1, uint8_t unknown2) :
            ManagedPacket(SMSG_FEATURE_SYSTEM_STATUS, 35),
            unknown1(unknown1),
            unknown2(unknown2)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Cata
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
#elif VERSION_STRING == Mop
            bool isFeedbackSysEnabled = true;
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
#else
            packet << unknown1 << unknown2;
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
#endif
    };
}}
