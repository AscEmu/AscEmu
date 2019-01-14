/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgLogXpGain : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t normalXp;
        uint32_t restedXp;
        bool isQuestXp;

        SmsgLogXpGain() : SmsgLogXpGain(0, 0, 0, false)
        {
        }

        SmsgLogXpGain(uint64_t guid, uint32_t normalXp, uint32_t restedXp, bool isQuestXp) :
            ManagedPacket(SMSG_LOG_XPGAIN, 1),
            guid(guid),
            normalXp(normalXp),
            restedXp(restedXp),
            isQuestXp(isQuestXp)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 4 + 4 + 4 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (isQuestXp == false)
                packet << guid << normalXp << uint8_t(0) << restedXp << float(1.0f);
            else
                packet << uint64_t(0) << normalXp << uint8_t(1) << uint8_t(0);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
