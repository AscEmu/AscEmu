/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgResurrectRequest : public ManagedPacket
    {
    public:
        uint64_t casterGuid;
        uint32_t stringSize;
        std::string casterName;
        uint8_t isSicknessAffected;
        uint8_t overrideTimer;
        //4.3.4
        uint32_t spellId;

        SmsgResurrectRequest() : SmsgResurrectRequest(0, 0, 0, 0)
        {
        }

        SmsgResurrectRequest(uint64_t casterGuid, std::string casterName, uint8_t isSicknessAffected, uint8_t overrideTimer = 0, uint32_t spellId = 0) :
            ManagedPacket(SMSG_RESURRECT_REQUEST, 8 + 4 + casterName.size() + 1 + 1 + 1),
            casterGuid(casterGuid),
            stringSize(casterName.size() + 1),
            casterName(casterName),
            isSicknessAffected(isSicknessAffected),
            overrideTimer(overrideTimer),
            spellId(spellId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << casterGuid << stringSize << spellId << casterName;
#if VERSION_STRING == Cata
            packet << uint8_t(0);
#endif
            packet << isSicknessAffected;

#if VERSION_STRING < Cata
            packet << overrideTimer;
#else
            packet << spellId;
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
