/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/MovementInfo.hpp"

#include <cstdint>
#include <utility>

namespace AscEmu::Packets
{
    // movement info under any movement opcode, written with the descriptor of the receiving client
    class MsgMovementInfo : public ManagedPacket
    {
    public:
        MovementInfo mi;
        bool withGuid = true;

        MsgMovementInfo(uint16_t opcode, MovementInfo mi, bool withGuid = true) :
            ManagedPacket(opcode, 0),
            mi(std::move(mi)),
            withGuid(withGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return sizeof(mi); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                const auto writeGuid = [&]()
                {
                    WoWGuid guid = mi.guid;
                    if (guid.getModernHigh() == 0)
                        guid = WoWGuid::createModernFromLegacy(guid.getRawGuid(), m_protocol.realmId);

                    const auto packedGuid = guid.packModern();
                    packet.append(packedGuid.data(), packedGuid.size());
                };

                switch (m_opcode)
                {
                    case SMSG_FORCE_WALK_SPEED_CHANGE:
                    case SMSG_FORCE_RUN_SPEED_CHANGE:
                    case SMSG_FORCE_RUN_BACK_SPEED_CHANGE:
                    case SMSG_FORCE_SWIM_SPEED_CHANGE:
                    case SMSG_FORCE_SWIM_BACK_SPEED_CHANGE:
                    case SMSG_FORCE_TURN_RATE_CHANGE:
                    case SMSG_FORCE_FLIGHT_SPEED_CHANGE:
                    case SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE:
                    case SMSG_FORCE_PITCH_RATE_CHANGE:
                        writeGuid();
                        packet << uint32_t(mi.counter);
                        packet << float(mi.newSpeed);
                        return true;

                    case SMSG_SPLINE_SET_WALK_SPEED:
                    case SMSG_SPLINE_SET_RUN_SPEED:
                    case SMSG_SPLINE_SET_RUN_BACK_SPEED:
                    case SMSG_SPLINE_SET_SWIM_SPEED:
                    case SMSG_SPLINE_SET_SWIM_BACK_SPEED:
                    case SMSG_SPLINE_SET_TURN_RATE:
                    case SMSG_SPLINE_SET_FLIGHT_SPEED:
                    case SMSG_SPLINE_SET_FLIGHT_BACK_SPEED:
                    case SMSG_SPLINE_SET_PITCH_RATE:
                        writeGuid();
                        packet << float(mi.newSpeed);
                        return true;

                    case SMSG_MOVE_SET_CAN_FLY:
                    case SMSG_MOVE_UNSET_CAN_FLY:
                        writeGuid();
                        packet << uint32_t(mi.counter);
                        return true;

                    case SMSG_SPLINE_MOVE_SET_FLYING:
                    case SMSG_SPLINE_MOVE_UNSET_FLYING:
                        writeGuid();
                        return true;

                    default:
                        return false;
                }
            }

            mi.write(packet, m_protocol, withGuid);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
