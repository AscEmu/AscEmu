/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/Units/Unit.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgMirrorimageCreatureData : public ManagedPacket
    {
    public:
        WoWGuid guid;
        Unit* caster {nullptr};

        SmsgMirrorimageCreatureData() : SmsgMirrorimageCreatureData(0, nullptr)
        {
        }

        SmsgMirrorimageCreatureData(uint64_t guid, Unit* caster) :
            ManagedPacket(SMSG_MIRRORIMAGE_CREATURE_DATA, 8 + 4),
            guid(guid),
            caster(caster)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (caster == nullptr)
                return false;

            packet.writeBit(guid[0]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[2]);
            packet.flushBits();

            packet.writeByteSeq(guid[0]);
            packet.writeByteSeq(guid[3]);
            packet.writeByteSeq(guid[6]);
            packet.writeByteSeq(guid[5]);
            packet.writeByteSeq(guid[7]);
            packet << uint32_t(caster->getDisplayId());
            packet.writeByteSeq(guid[4]);
            packet.writeByteSeq(guid[2]);
            packet.writeByteSeq(guid[1]);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
