/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <utility>

namespace AscEmu::Packets
{
    // 11.x moved the status bits: the 10.x bits in their 11.x positions
    inline uint64_t questgiverStatusForTheWarWithin(uint64_t status)
    {
        static constexpr std::pair<uint64_t, uint64_t> movedBits[] =
        {
            { 0x0002, 0x000000002 },            // future
            { 0x0004, 0x000000040 },            // trivial
            { 0x0008, 0x000000020 },            // trivial repeatable turn in
            { 0x0010, 0x000000080 },            // trivial daily quest
            { 0x0020, 0x000002000 },            // reward
            { 0x0040, 0x000010000 },            // journey reward
            { 0x0080, 0x000020000 },            // covenant calling reward
            { 0x0100, 0x000100000 },            // repeatable turn in
            { 0x0200, 0x000800000 },            // daily quest
            { 0x0400, 0x000400000 },            // quest
            { 0x0800, 0x200000000 },            // reward complete without poi
            { 0x1000, 0x400000000 },            // reward complete with poi
        };

        uint64_t result = 0;
        for (const auto& [oldBit, newBit] : movedBits)
        {
            if (status & oldBit)
                result |= newBit;
        }

        return result;
    }

    class SmsgQuestgiverStatus : public ManagedPacket
    {
    public:
        uint64_t questgiverGuid;
        uint32_t status;

        SmsgQuestgiverStatus() : SmsgQuestgiverStatus(0, 0)
        {
        }

        SmsgQuestgiverStatus(uint64_t questgiverGuid, uint32_t status) :
            ManagedPacket(SMSG_QUESTGIVER_STATUS, 0),
            questgiverGuid(questgiverGuid),
            status(status)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_protocol.expansion < WoW::Expansion::_Cata ? 9 : 12;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isTheWarWithin())
            {
                // 11.x: as 10.x with the moved status bits
                packet << WoWGuid(questgiverGuid).toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << questgiverStatusForTheWarWithin(status);
                return true;
            }

            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight())
            {
                packet << WoWGuid(questgiverGuid).toGuid128(m_protocol.realmId, m_receiverMapId);
                if (m_protocol.isDragonflight())
                    packet << uint64_t(status);
                else
                    packet << status;
                return true;
            }

            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << questgiverGuid << static_cast<uint8_t>(status);
            }
            else if (m_protocol.isCata())
            {
                packet << questgiverGuid << status;
            }
            else
            {
                WoWGuid guid = questgiverGuid;

                packet.writeBit(guid[1]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[0]);
                packet.flushBits();

                packet.writeByteSeq(guid[7]);
                packet << status;
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[3]);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
