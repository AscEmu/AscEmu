/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "MovementInfo.hpp"
#include "MovementDefines.hpp"
#include "MovementDescriptors.hpp"

template <WoW::Expansion Version>
class MovementCodec
{
public:
    static void read(ByteBuffer& buffer, MovementInfo& movementInfo, uint16_t opcode)
    {
        unsetState(movementInfo);

        for (MovementStep const& step : getDescriptor(opcode, true))
        {
            if (step.op == MovementOp::End)
                break;

            if (!checkCondition(step.cond, movementInfo))
                continue;

            if (!checkReadCondition(step.cond, movementInfo))
                continue;

            executeReadStep(buffer, movementInfo, step.op, opcode);
        }
    }

    static bool hasDescriptor(uint16_t opcode, bool read)
    {
        return !isUnknownDescriptor(getDescriptor(opcode, read));
    }

    static void write(ByteBuffer& data, MovementInfo movementInfo, uint16_t opcode, bool withGuid = true)
    {
        setState(movementInfo);

        for (MovementStep const& step : getDescriptor(opcode, false))
        {
            if (step.op == MovementOp::End)
                break;

            if (!checkCondition(step.cond, movementInfo))
                continue;

            if (!checkWriteCondition(step.cond, movementInfo, withGuid))
                continue;

            executeWriteStep(data, movementInfo, step.op);
        }

        data.flushBits();
    }

private:
    //unset all states to false for read
    static void unsetState(MovementInfo& movementInfo)
    {
        movementInfo.hasTransportData = false;
        movementInfo.hasMovementFlags = false;
        movementInfo.hasMovementFlags2 = false;
        movementInfo.status_info = {};
    }

    //set states based on data for write
    static void setState(MovementInfo& movementInfo)
    {
        if constexpr (Version == WoW::Expansion::_Classic || Version == WoW::Expansion::_TBC || Version == WoW::Expansion::_WotLK)
        {
            movementInfo.hasTransportData = movementInfo.hasMovementFlag(MOVEFLAG_TRANSPORT);
        }
        else
        {
            movementInfo.hasTransportData = !movementInfo.transport_guid.isEmpty();
        }

        movementInfo.hasMovementFlags = movementInfo.flags != 0;
        movementInfo.hasMovementFlags2 = movementInfo.flags2 != 0;
        movementInfo.status_info.hasFallDirection = movementInfo.hasMovementFlag(MOVEFLAG_FALLING);
        movementInfo.status_info.hasFallData = movementInfo.status_info.hasFallDirection || movementInfo.fall_time != 0;
        movementInfo.status_info.hasSplineElevation = movementInfo.hasMovementFlag(MovementFlags(MOVEFLAG_SPLINE_ELEVATION));
        movementInfo.status_info.hasTransportTime2 = movementInfo.hasTransportData && movementInfo.transport_time2 != 0;
        movementInfo.status_info.hasTimeStamp = movementInfo.update_time;
        movementInfo.status_info.hasTransportTime3 = false;
        movementInfo.status_info.hasOrientation = movementInfo.position.o != 0;

        if constexpr (Version == WoW::Expansion::_Classic)
        {
            movementInfo.status_info.hasPitch = 
                movementInfo.hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING));
        }
        else
        {
            movementInfo.status_info.hasPitch = 
                movementInfo.hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) ||
                movementInfo.hasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING);
        }
    }

    static void flushForAlignedWrite(ByteBuffer& data)
    {
        data.flushBits();
    }

    static bool checkCondition(Cond cond, MovementInfo const& movementInfo)
    {
        switch (cond)
        {
            case Cond::Always:             return true;
            case Cond::HasTransport:       return movementInfo.hasTransportData;
            case Cond::HasFallData:        return movementInfo.status_info.hasFallData;

            case Cond::HasFallDirection:   return movementInfo.status_info.hasFallDirection;
            case Cond::HasOrientation:     return movementInfo.status_info.hasOrientation;
            case Cond::HasPitch:           return movementInfo.status_info.hasPitch;
            case Cond::HasSplineElevation: return movementInfo.status_info.hasSplineElevation;
            case Cond::HasTimestamp:       return movementInfo.status_info.hasTimeStamp;
            case Cond::HasMovementFlags:   return movementInfo.hasMovementFlags;
            case Cond::HasMovementFlags2:  return movementInfo.hasMovementFlags2;
            case Cond::HasTransportTime2:  return movementInfo.status_info.hasTransportTime2;
            case Cond::HasTransportTime3:  return movementInfo.status_info.hasTransportTime3;
            case Cond::HasCount:           return movementInfo.hasCount;

            case Cond::NoReadOptWrite:     return true;
            case Cond::ReadOptWrite:       return true;
            case Cond::IsTransportPresent: return true;
        }

        return false;
    }

    static bool checkReadCondition(Cond cond, [[maybe_unused]] MovementInfo const& movementInfo)
    {
        switch (cond)
        {
            case Cond::NoReadOptWrite:     return false;
            case Cond::ReadOptWrite:       return true;
        }

        return true;
    }

    static bool checkWriteCondition(Cond cond, [[maybe_unused]] MovementInfo const& movementInfo, bool withGuid)
    {
        switch (cond)
        {
            case Cond::NoReadOptWrite:     return withGuid;
            case Cond::ReadOptWrite:       return withGuid;
        }

        return true;
    }

    static bool isUnknownDescriptor(std::span<MovementStep const> descriptor)
    {
        return descriptor.data() == UnknownDescriptor.data() &&
            descriptor.size() == UnknownDescriptor.size();
    }

    static void executeReadStep(ByteBuffer& buffer, MovementInfo& movementInfo, MovementOp op, uint16_t opcode)
    {
        switch (op)
        {
            case MovementOp::End:
                break;

            case MovementOp::Guid: buffer >> movementInfo.guid; break;
            case MovementOp::GuidBit0: movementInfo.guid[0] = buffer.readBit(); break;
            case MovementOp::GuidBit1: movementInfo.guid[1] = buffer.readBit(); break;
            case MovementOp::GuidBit2: movementInfo.guid[2] = buffer.readBit(); break;
            case MovementOp::GuidBit3: movementInfo.guid[3] = buffer.readBit(); break;
            case MovementOp::GuidBit4: movementInfo.guid[4] = buffer.readBit(); break;
            case MovementOp::GuidBit5: movementInfo.guid[5] = buffer.readBit(); break;
            case MovementOp::GuidBit6: movementInfo.guid[6] = buffer.readBit(); break;
            case MovementOp::GuidBit7: movementInfo.guid[7] = buffer.readBit(); break;

            case MovementOp::GuidByte0: buffer.readByteSeq(movementInfo.guid[0]); break;
            case MovementOp::GuidByte1: buffer.readByteSeq(movementInfo.guid[1]); break;
            case MovementOp::GuidByte2: buffer.readByteSeq(movementInfo.guid[2]); break;
            case MovementOp::GuidByte3: buffer.readByteSeq(movementInfo.guid[3]); break;
            case MovementOp::GuidByte4: buffer.readByteSeq(movementInfo.guid[4]); break;
            case MovementOp::GuidByte5: buffer.readByteSeq(movementInfo.guid[5]); break;
            case MovementOp::GuidByte6: buffer.readByteSeq(movementInfo.guid[6]); break;
            case MovementOp::GuidByte7: buffer.readByteSeq(movementInfo.guid[7]); break;

            case MovementOp::Flags:
            {
                if constexpr (Version == WoW::Expansion::_Cata || Version == WoW::Expansion::_Mop)
                    movementInfo.flags = buffer.readBits(30);
                else
                    buffer >> movementInfo.flags;
            } break;

            case MovementOp::Flags2:
            {
                if constexpr (MovementVersionTraits<Version>::hasFlags2)
                {
                    if constexpr (MovementVersionTraits<Version>::flags2IsBitPacked)
                    {
                        movementInfo.flags2 = static_cast<uint16_t>(
                            buffer.readBits(MovementVersionTraits<Version>::flags2BitWidth));
                    }
                    else
                    {
                        if constexpr (MovementVersionTraits<Version>::flags2BitWidth == 8)
                        {
                            uint8_t tempFlags2 = 0;
                            buffer >> tempFlags2;
                            movementInfo.flags2 = static_cast<uint16_t>(tempFlags2);
                        }
                        else
                        {
                            buffer >> movementInfo.flags2;
                        }
                    }
                }
            } break;

            case MovementOp::PosX:            buffer >> movementInfo.position.x; break;
            case MovementOp::PosY:            buffer >> movementInfo.position.y; break;
            case MovementOp::PosZ:            buffer >> movementInfo.position.z; break;
            case MovementOp::Orientation:     buffer >> movementInfo.position.o; break;
            case MovementOp::Timestamp:       buffer >> movementInfo.update_time; break;
            case MovementOp::Pitch:           buffer >> movementInfo.pitch_rate; break;
            case MovementOp::FallTime:        buffer >> movementInfo.fall_time; break;
            case MovementOp::SplineElevation: buffer >> movementInfo.spline_elevation; break;

            case MovementOp::JumpVelocity: buffer >> movementInfo.jump_info.velocity; break;
            case MovementOp::JumpSin:      buffer >> movementInfo.jump_info.sinAngle; break;
            case MovementOp::JumpCos:      buffer >> movementInfo.jump_info.cosAngle; break;
            case MovementOp::JumpXYSpeed:  buffer >> movementInfo.jump_info.xyspeed; break;

            case MovementOp::TGuid: buffer >> movementInfo.transport_guid; break;
            case MovementOp::TGuidBit0: movementInfo.transport_guid[0] = buffer.readBit(); break;
            case MovementOp::TGuidBit1: movementInfo.transport_guid[1] = buffer.readBit(); break;
            case MovementOp::TGuidBit2: movementInfo.transport_guid[2] = buffer.readBit(); break;
            case MovementOp::TGuidBit3: movementInfo.transport_guid[3] = buffer.readBit(); break;
            case MovementOp::TGuidBit4: movementInfo.transport_guid[4] = buffer.readBit(); break;
            case MovementOp::TGuidBit5: movementInfo.transport_guid[5] = buffer.readBit(); break;
            case MovementOp::TGuidBit6: movementInfo.transport_guid[6] = buffer.readBit(); break;
            case MovementOp::TGuidBit7: movementInfo.transport_guid[7] = buffer.readBit(); break;

            case MovementOp::TGuidByte0: buffer.readByteSeq(movementInfo.transport_guid[0]); break;
            case MovementOp::TGuidByte1: buffer.readByteSeq(movementInfo.transport_guid[1]); break;
            case MovementOp::TGuidByte2: buffer.readByteSeq(movementInfo.transport_guid[2]); break;
            case MovementOp::TGuidByte3: buffer.readByteSeq(movementInfo.transport_guid[3]); break;
            case MovementOp::TGuidByte4: buffer.readByteSeq(movementInfo.transport_guid[4]); break;
            case MovementOp::TGuidByte5: buffer.readByteSeq(movementInfo.transport_guid[5]); break;
            case MovementOp::TGuidByte6: buffer.readByteSeq(movementInfo.transport_guid[6]); break;
            case MovementOp::TGuidByte7: buffer.readByteSeq(movementInfo.transport_guid[7]); break;

            case MovementOp::TPosX:        buffer >> movementInfo.transport_position.x; break;
            case MovementOp::TPosY:        buffer >> movementInfo.transport_position.y; break;
            case MovementOp::TPosZ:        buffer >> movementInfo.transport_position.z; break;
            case MovementOp::TOrientation: buffer >> movementInfo.transport_position.o; break;
            case MovementOp::TTime:        buffer >> movementInfo.transport_time; break;
            case MovementOp::TTime2:       buffer >> movementInfo.transport_time2; break;
            case MovementOp::TTime3:       buffer >> movementInfo.transport_time3; break;
            case MovementOp::TSeat:        buffer >> movementInfo.transport_seat; break;

            case MovementOp::HasTransport:       movementInfo.hasTransportData = buffer.readBit(); break;
            case MovementOp::HasFallData:        movementInfo.status_info.hasFallData = buffer.readBit(); break;
            case MovementOp::HasFallDirection:   movementInfo.status_info.hasFallDirection = buffer.readBit(); break;
            case MovementOp::HasOrientation:     movementInfo.status_info.hasOrientation = !buffer.readBit(); break;
            case MovementOp::HasPitch:           movementInfo.status_info.hasPitch = !buffer.readBit(); break;
            case MovementOp::HasSpline:          movementInfo.status_info.hasSpline = buffer.readBit(); break;
            case MovementOp::HasSplineElevation: movementInfo.status_info.hasSplineElevation = !buffer.readBit(); break;
            case MovementOp::HasTimestamp:       movementInfo.status_info.hasTimeStamp = !buffer.readBit(); break;
            case MovementOp::HasMovementFlags:   movementInfo.hasMovementFlags = !buffer.readBit(); break;
            case MovementOp::HasMovementFlags2:  movementInfo.hasMovementFlags2 = !buffer.readBit(); break;
            case MovementOp::HasTransportTime2:  movementInfo.status_info.hasTransportTime2 = buffer.readBit(); break;
            case MovementOp::HasTransportTime3:  movementInfo.status_info.hasTransportTime3 = buffer.readBit(); break;
            case MovementOp::HasCount:           movementInfo.hasCount = !buffer.readBit(); break;

            case MovementOp::ForcesCount:
            {
                movementInfo.forcesCount = buffer.readBits(22);
                sLogger.debugFlag(AscEmu::Logging::LF_MOVE, "{} : ForcesCount is {}",
                    sOpcodeTables.getNameForOpcode(opcode), movementInfo.forcesCount);
            } break;

            case MovementOp::Count:
            {
                uint32_t counter;
                buffer >> counter;
                //buffer.readSkip<uint32_t>();
                sLogger.debugFlag(AscEmu::Logging::LF_MOVE, "MovementCodec::executeReadStep: {} : Count is {}",
                    sOpcodeTables.getNameForOpcode(opcode), counter);
            } break;

            case MovementOp::NewSpeed: buffer >> movementInfo.newSpeed; break;

            case MovementOp::SkipForcesCountUInt32:
            {
                for (uint32_t i = 0; i < movementInfo.forcesCount; ++i)
                {
                    uint32_t force;
                    buffer >> force;
                    //buffer.readSkip<uint32_t>();
                    sLogger.debugFlag(AscEmu::Logging::LF_MOVE, "MovementCodec::executeReadStep: {} : SkipForcesCountUInt32 {} is {}",
                        sOpcodeTables.getNameForOpcode(opcode), i, force);
                }
            } break;

            case MovementOp::SkipBit:
            {
                uint8_t bit = buffer.readBit();
                sLogger.debugFlag(AscEmu::Logging::LF_MOVE, "MovementCodec::executeReadStep: {} : SkipBit is {}",
                    sOpcodeTables.getNameForOpcode(opcode), bit);
            } break;

            case MovementOp::SkipUInt32:
            {
                uint32_t uint;
                buffer >> uint;
                //buffer.readSkip<uint32_t>();
                sLogger.debugFlag(AscEmu::Logging::LF_MOVE, "MovementCodec::executeReadStep: {} : SkipUInt32 is {}",
                    sOpcodeTables.getNameForOpcode(opcode), uint);
            } break;
        }
    }

    static void executeWriteStep(ByteBuffer& data, MovementInfo const& movementInfo, MovementOp op)
    {
        switch (op)
        {
            case MovementOp::End:
                break;

            case MovementOp::Guid: data << movementInfo.guid; break;
            case MovementOp::GuidBit0: data.writeBit(movementInfo.guid[0]); break;
            case MovementOp::GuidBit1: data.writeBit(movementInfo.guid[1]); break;
            case MovementOp::GuidBit2: data.writeBit(movementInfo.guid[2]); break;
            case MovementOp::GuidBit3: data.writeBit(movementInfo.guid[3]); break;
            case MovementOp::GuidBit4: data.writeBit(movementInfo.guid[4]); break;
            case MovementOp::GuidBit5: data.writeBit(movementInfo.guid[5]); break;
            case MovementOp::GuidBit6: data.writeBit(movementInfo.guid[6]); break;
            case MovementOp::GuidBit7: data.writeBit(movementInfo.guid[7]); break;

            case MovementOp::GuidByte0: data.writeByteSeq(movementInfo.guid[0]); break;
            case MovementOp::GuidByte1: data.writeByteSeq(movementInfo.guid[1]); break;
            case MovementOp::GuidByte2: data.writeByteSeq(movementInfo.guid[2]); break;
            case MovementOp::GuidByte3: data.writeByteSeq(movementInfo.guid[3]); break;
            case MovementOp::GuidByte4: data.writeByteSeq(movementInfo.guid[4]); break;
            case MovementOp::GuidByte5: data.writeByteSeq(movementInfo.guid[5]); break;
            case MovementOp::GuidByte6: data.writeByteSeq(movementInfo.guid[6]); break;
            case MovementOp::GuidByte7: data.writeByteSeq(movementInfo.guid[7]); break;

            case MovementOp::Flags:
            {
                if constexpr (Version == WoW::Expansion::_Cata || Version == WoW::Expansion::_Mop)
                    data.writeBits(movementInfo.flags, 30);
                else
                    data << movementInfo.flags;
            } break;

            case MovementOp::Flags2:
            {
                if constexpr (MovementVersionTraits<Version>::hasFlags2)
                {
                    if constexpr (MovementVersionTraits<Version>::flags2IsBitPacked)
                    {
                        data.writeBits(movementInfo.flags2, MovementVersionTraits<Version>::flags2BitWidth);
                    }
                    else
                    {
                        if constexpr (MovementVersionTraits<Version>::flags2BitWidth == 8)
                            data << static_cast<uint8_t>(movementInfo.flags2);
                        else
                            data << movementInfo.flags2;
                    }
                }
            } break;

            case MovementOp::PosX:            data << movementInfo.position.x; break;
            case MovementOp::PosY:            data << movementInfo.position.y; break;
            case MovementOp::PosZ:            data << movementInfo.position.z; break;
            case MovementOp::Orientation:     data << LocationVector::normalizeOrientation(movementInfo.position.o); break;
            case MovementOp::Timestamp:       data << Util::getMSTime(); break;
            case MovementOp::Pitch:           data << movementInfo.pitch_rate; break;
            case MovementOp::FallTime:        data << movementInfo.fall_time; break;
            case MovementOp::SplineElevation: data << movementInfo.spline_elevation; break;

            case MovementOp::JumpVelocity: data << movementInfo.jump_info.velocity; break;
            case MovementOp::JumpSin:      data << movementInfo.jump_info.sinAngle; break;
            case MovementOp::JumpCos:      data << movementInfo.jump_info.cosAngle; break;
            case MovementOp::JumpXYSpeed:  data << movementInfo.jump_info.xyspeed; break;

            case MovementOp::TGuid: data << movementInfo.transport_guid; break;
            case MovementOp::TGuidBit0: data.writeBit(movementInfo.transport_guid[0]); break;
            case MovementOp::TGuidBit1: data.writeBit(movementInfo.transport_guid[1]); break;
            case MovementOp::TGuidBit2: data.writeBit(movementInfo.transport_guid[2]); break;
            case MovementOp::TGuidBit3: data.writeBit(movementInfo.transport_guid[3]); break;
            case MovementOp::TGuidBit4: data.writeBit(movementInfo.transport_guid[4]); break;
            case MovementOp::TGuidBit5: data.writeBit(movementInfo.transport_guid[5]); break;
            case MovementOp::TGuidBit6: data.writeBit(movementInfo.transport_guid[6]); break;
            case MovementOp::TGuidBit7: data.writeBit(movementInfo.transport_guid[7]); break;

            case MovementOp::TGuidByte0: data.writeByteSeq(movementInfo.transport_guid[0]); break;
            case MovementOp::TGuidByte1: data.writeByteSeq(movementInfo.transport_guid[1]); break;
            case MovementOp::TGuidByte2: data.writeByteSeq(movementInfo.transport_guid[2]); break;
            case MovementOp::TGuidByte3: data.writeByteSeq(movementInfo.transport_guid[3]); break;
            case MovementOp::TGuidByte4: data.writeByteSeq(movementInfo.transport_guid[4]); break;
            case MovementOp::TGuidByte5: data.writeByteSeq(movementInfo.transport_guid[5]); break;
            case MovementOp::TGuidByte6: data.writeByteSeq(movementInfo.transport_guid[6]); break;
            case MovementOp::TGuidByte7: data.writeByteSeq(movementInfo.transport_guid[7]); break;

            case MovementOp::TPosX:        data << movementInfo.transport_position.x; break;
            case MovementOp::TPosY:        data << movementInfo.transport_position.y; break;
            case MovementOp::TPosZ:        data << movementInfo.transport_position.z; break;
            case MovementOp::TOrientation: data << LocationVector::normalizeOrientation(movementInfo.transport_position.o); break;
            case MovementOp::TTime:        data << movementInfo.transport_time; break;
            case MovementOp::TTime2:       data << movementInfo.transport_time2; break;
            case MovementOp::TTime3:       data << movementInfo.transport_time3; break;
            case MovementOp::TSeat:        data << movementInfo.transport_seat; break;

            case MovementOp::HasTransport:       data.writeBit(movementInfo.hasTransportData); break;
            case MovementOp::HasFallData:        data.writeBit(movementInfo.status_info.hasFallData); break;
            case MovementOp::HasFallDirection:   data.writeBit(movementInfo.status_info.hasFallDirection); break;
            case MovementOp::HasOrientation:     data.writeBit(!movementInfo.status_info.hasOrientation); break;
            case MovementOp::HasPitch:           data.writeBit(!movementInfo.status_info.hasPitch); break;
            case MovementOp::HasSpline:          data.writeBit(movementInfo.status_info.hasSpline); break;
            case MovementOp::HasSplineElevation: data.writeBit(!movementInfo.status_info.hasSplineElevation); break;
            case MovementOp::HasTimestamp:       data.writeBit(!movementInfo.status_info.hasTimeStamp); break;
            case MovementOp::HasMovementFlags:   data.writeBit(!movementInfo.hasMovementFlags); break;
            case MovementOp::HasMovementFlags2:  data.writeBit(!movementInfo.hasMovementFlags2); break;
            case MovementOp::HasTransportTime2:  data.writeBit(movementInfo.status_info.hasTransportTime2); break;
            case MovementOp::HasTransportTime3:  data.writeBit(movementInfo.status_info.hasTransportTime3); break;
            case MovementOp::HasCount:           data.writeBit(!movementInfo.hasCount); break;

            case MovementOp::ForcesCount: data.writeBits(0, 22); break;

            case MovementOp::Count: data << static_cast<uint32_t>(0); break;
            case MovementOp::NewSpeed: data << movementInfo.newSpeed; break;

            case MovementOp::SkipForcesCountUInt32:
            {
                //for (uint32_t i = 0; i < movementInfo.forcesCount; ++i)
                //    data << uint32_t(0);
            } break;

            case MovementOp::WriteBit0: data.writeBit(0); break;
            case MovementOp::WriteBit1: data.writeBit(1); break;
            case MovementOp::WriteUInt32_0: data << static_cast<uint32_t>(0); break;
            case MovementOp::WriteUInt8_1:  data << static_cast<uint8_t>(1); break;
            case MovementOp::FlushBits: data.flushBits(); break;
        }
    }

    static std::span<MovementStep const> getDescriptor(uint16_t opcode, bool read)
    {
        if constexpr (Version == WoW::Expansion::_Classic)
            return getClassicMovementDescriptor(opcode, read);
        else if constexpr (Version == WoW::Expansion::_TBC)
            return getTbcMovementDescriptor(opcode, read);
        else if constexpr (Version == WoW::Expansion::_WotLK)
            return getWotlkMovementDescriptor(opcode, read);
        else if constexpr (Version == WoW::Expansion::_Cata)
            return getCataMovementDescriptor(opcode, read);
        else
            return getMopMovementDescriptor(opcode, read);
    }
};
