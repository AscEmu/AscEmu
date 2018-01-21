/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Warden/SpeedDetector.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Map/WorldCreatorDefines.hpp"
#include "GameCata/Movement/MovementStructures.h"
#include "Units/Creatures/Pet.h"

#define MOVEMENT_PACKET_TIME_DELAY 500

void WorldSession::HandleMovementOpcodes(WorldPacket& recv_data)
{
    uint16_t opcode = recv_data.GetOpcode();
    Player* mover = _player;

    if (m_MoverGuid != mover->GetGUID())
        return;

    if (mover->GetCharmedByGUID() || !mover->IsInWorld() || mover->GetPlayerStatus() == TRANSFER_PENDING || mover->GetTaxiState())
    {
        return;
    }

    /************************************************************************/
    /* Clear standing state to stand.				                        */
    /************************************************************************/
    if (opcode == MSG_MOVE_START_FORWARD)
        mover->SetStandState(STANDSTATE_STAND);

    //extract packet
    MovementInfo movementInfo;
    recv_data >> movementInfo;

//    /************************************************************************/
//    /* Make sure the packet is the correct size range.                      */
//    /************************************************************************/
//    //if (recv_data.size() > 80) { Disconnect(); return; }
//
//    /************************************************************************/
//    /* Read Movement Data Packet                                            */
//    /************************************************************************/
//    WoWGuid guid;
//    recv_data >> guid;
//    movement_info.init(recv_data);
//
//    if (guid != m_MoverWoWGuid.GetOldGuid())
//    {
//        return;
//    }
//
//    // Player is in control of some entity, so we move that instead of the player
//    Unit* mover = _player->GetMapMgr()->GetUnit(m_MoverWoWGuid.GetOldGuid());
//    if (mover == NULL)
//        return;
//
//    /* Anti Multi-Jump Check */
//    if (recv_data.GetOpcode() == MSG_MOVE_JUMP && _player->jumping == true && !GetPermissionCount())
//    {
//        sCheatLog.writefromsession(this, "Detected jump hacking");
//        Disconnect();
//        return;
//    }
//

    /************************************************************************/
    /* Update player movement state                                         */
    /************************************************************************/
    _player->isPlayerJumping(movementInfo, opcode);
    if (_player->GetOrientation() == movementInfo.getPosition()->o)
    {
        _player->isTurning = false;
    }

    switch (opcode)
    {
        case MSG_MOVE_START_FORWARD:
        case MSG_MOVE_START_BACKWARD:
            _player->moving = true;
            break;
        case MSG_MOVE_START_STRAFE_LEFT:
        case MSG_MOVE_START_STRAFE_RIGHT:
            _player->strafing = true;
            break;
        case MSG_MOVE_JUMP:
            _player->jumping = true;
            break;
        case MSG_MOVE_STOP:
            _player->moving = false;
            break;
        case MSG_MOVE_STOP_STRAFE:
            _player->strafing = false;
            break;
        case MSG_MOVE_FALL_LAND:
            _player->jumping = false;
            break;
        case MSG_MOVE_SET_FACING:
            _player->isTurning = true;
            break;
    }

    if (_player->moving == false && _player->strafing == false && _player->jumping == false)
    {
        _player->m_isMoving = false;
    }
    else
    {
        _player->m_isMoving = true;
    }

//
//
//    if (!(HasGMPermissions() && sWorld.no_antihack_on_gm) && !_player->GetCharmedUnitGUID())
//    {
//        /************************************************************************/
//        /* Anti-Teleport                                                        */
//        /************************************************************************/
//        if (sWorld.antihack_teleport && _player->m_position.Distance2DSq(movement_info.position.x, movement_info.position.y) > 3025.0f
//            && _player->getSpeedForType(TYPE_RUN) < 50.0f && !_player->obj_movement_info.transporter_info.guid)
//        {
//            sCheatLog.writefromsession(this, "Disconnected for teleport hacking. Player speed: %f, Distance traveled: %f", _player->getSpeedForType(TYPE_RUN), sqrt(_player->m_position.Distance2DSq(movement_info.position.x, movement_info.position.y)));
//            Disconnect();
//            return;
//        }
//    }
//
//    //update the detector
//    if (sWorld.antihack_speed && !_player->GetTaxiState() && _player->obj_movement_info.transporter_info.guid == 0 && !_player->GetSession()->GetPermissionCount())
//    {
//        // simplified: just take the fastest speed. less chance of fuckups too
//        float speed = (_player->flying_aura) ? _player->getSpeedForType(TYPE_FLY) : (_player->getSpeedForType(TYPE_SWIM) > _player->getSpeedForType(TYPE_RUN)) ? _player->getSpeedForType(TYPE_SWIM) : _player->getSpeedForType(TYPE_RUN);
//
//        _player->SDetector->AddSample(movement_info.position.x, movement_info.position.y, Util::getMSTime(), speed);
//
//        if (_player->SDetector->IsCheatDetected())
//            _player->SDetector->ReportCheater(_player);
//    }

    /************************************************************************/
    /* Remove Emote State                                                   */
    /************************************************************************/
    if (_player->GetEmoteState())
        _player->SetEmoteState(EMOTE_ONESHOT_NONE);

//    /************************************************************************/
//    /* Make sure the co-ordinates are valid.                                */
//    /************************************************************************/
//    if (!((movement_info.position.y >= _minY) && (movement_info.position.y <= _maxY)) || !((movement_info.position.x >= _minX) && (movement_info.position.x <= _maxX)))
//    {
//        Disconnect();
//        return;
//    }


    ///************************************************************************/
    ///* Calculate the timestamp of the packet we have to send out            */
    ///************************************************************************/
    //size_t pos = (size_t)m_MoverWoWGuid.GetNewGuidLen() + 1;
    //uint32 mstime = Util::getMSTime();
    //int32 move_time;
    //if (m_clientTimeDelay == 0)
    //    m_clientTimeDelay = mstime - movement_info.time;

    ///************************************************************************/
    ///* Copy into the output buffer.                                         */
    ///************************************************************************/
    //if (_player->m_inRangePlayers.size())
    //{
    //    move_time = (movement_info.time - (mstime - m_clientTimeDelay)) + MOVEMENT_PACKET_TIME_DELAY + mstime;
    //    memcpy(&movement_packet[0], recv_data.contents(), recv_data.size());
    //    movement_packet[pos + 6] = 0;

    //    /************************************************************************/
    //    /* Distribute to all inrange players.                                   */
    //    /************************************************************************/
    //    for (std::set<Object*>::iterator itr = _player->m_inRangePlayers.begin(); itr != _player->m_inRangePlayers.end(); ++itr)
    //    {

    //        Player* p = static_cast< Player* >((*itr));

    //        *(uint32*)&movement_packet[pos + 6] = uint32(move_time + p->GetSession()->m_moveDelayTime);

    //        p->GetSession()->OutPacket(recv_data.GetOpcode(), uint16(recv_data.size() + pos), movement_packet);

    //    }
    //}

    ///************************************************************************/
    ///* Hack Detection by Classic	                                        */
    ///************************************************************************/
    //if (!movement_info.transporter_info.guid && recv_data.GetOpcode() != MSG_MOVE_JUMP && !_player->FlyCheat && !_player->flying_aura && !(movement_info.flags & MOVEFLAG_SWIMMING || movement_info.flags & MOVEFLAG_FALLING) && movement_info.position.z > _player->GetPositionZ() && movement_info.position.x == _player->GetPositionX() && movement_info.position.y == _player->GetPositionY())
    //{
    //    WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 13);
    //    data << _player->GetNewGUID();
    //    data << uint32(5);      // unknown 0
    //    SendPacket(&data);
    //}

    //if ((movement_info.flags & MOVEFLAG_FLYING) && !(movement_info.flags & MOVEFLAG_SWIMMING) && !(_player->flying_aura || _player->FlyCheat))
    //{
    //    WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 13);
    //    data << _player->GetNewGUID();
    //    data << uint32(5);      // unknown 0
    //    SendPacket(&data);
    //}

    ///************************************************************************/
    ///* Falling damage checks                                                */
    ///************************************************************************/

    //if (_player->blinked)
    //{
    //    _player->blinked = false;
    //    _player->m_fallDisabledUntil = UNIXTIME + 5;
    //    _player->SpeedCheatDelay(2000);   //some say they managed to trigger system with knockback. Maybe they moved in air ?
    //}
    //else
    //{
    //    if (recv_data.GetOpcode() == MSG_MOVE_FALL_LAND)
    //    {
    //        // player has finished falling
    //        //if z_axisposition contains no data then set to current position
    //        if (!mover->z_axisposition)
    //            mover->z_axisposition = movement_info.position.z;

    //        // calculate distance fallen
    //        uint32 falldistance = float2int32(mover->z_axisposition - movement_info.position.z);
    //        if (mover->z_axisposition <= movement_info.position.z)
    //            falldistance = 1;
    //        /*Safe Fall*/
    //        if ((int)falldistance > mover->m_safeFall)
    //            falldistance -= mover->m_safeFall;
    //        else
    //            falldistance = 1;

    //        //checks that player has fallen more than 12 units, otherwise no damage will be dealt
    //        //falltime check is also needed here, otherwise sudden changes in Z axis position, such as using !recall, may result in death
    //        if (mover->isAlive() && !mover->bInvincible && (falldistance > 12) && !mover->m_noFallDamage &&
    //            ((mover->GetGUID() != _player->GetGUID()) || (!_player->GodModeCheat && (UNIXTIME >= _player->m_fallDisabledUntil))))
    //        {
    //            // 1.7% damage for each unit fallen on Z axis over 13
    //            uint32 health_loss = static_cast<uint32>(mover->GetHealth() * (falldistance - 12) * 0.017f);

    //            if (health_loss >= mover->GetHealth())
    //                health_loss = mover->GetHealth();

    //            else if ((falldistance >= 65) && (mover->GetGUID() == _player->GetGUID()))
    //            {
    //                // Rather than Updating achievement progress every time fall damage is taken, all criteria currently have 65 yard requirement...
    //                // Achievement 964: Fall 65 yards without dying.
    //                // Achievement 1260: Fall 65 yards without dying while completely smashed during the Brewfest Holiday.
    //                _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING, falldistance, Player::GetDrunkenstateByValue(_player->GetDrunkValue()), 0);
    //            }

    //            mover->SendEnvironmentalDamageLog(mover->GetGUID(), DAMAGE_FALL, health_loss);
    //            mover->DealDamage(mover, health_loss, 0, 0, 0);

    //            //_player->RemoveStealth(); // cebernic : why again? lost stealth by AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN already.
    //        }
    //        mover->z_axisposition = 0.0f;
    //    }
    //    else
    //        //whilst player is not falling, continuously update Z axis position.
    //        //once player lands this will be used to determine how far he fell.
    //        if (!(movement_info.flags & MOVEFLAG_FALLING))
    //            mover->z_axisposition = movement_info.position.z;
    //}

    ///************************************************************************/
    ///* Transporter Setup                                                    */
    ///************************************************************************/
    //if ((mover->obj_movement_info.transporter_info.guid != 0) && (movement_info.transporter_info.transGuid.GetOldGuid() == 0))
    //{
    //    /* we left the transporter we were on */

    //    Transporter *transporter = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(mover->obj_movement_info.transporter_info.guid));
    //    if (transporter != NULL)
    //        transporter->RemovePassenger(static_cast<Player*>(mover));

    //    mover->obj_movement_info.transporter_info.guid = 0;
    //    _player->SpeedCheatReset();

    //}
    //else
    //{
    //    if (movement_info.transporter_info.transGuid.GetOldGuid() != 0)
    //    {

    //        if (mover->obj_movement_info.transporter_info.guid == 0)
    //        {
    //            Transporter *transporter = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(movement_info.transporter_info.transGuid));
    //            if (transporter != NULL)
    //                transporter->AddPassenger(static_cast<Player*>(mover));

    //            /* set variables */
    //            mover->obj_movement_info.transporter_info.guid = movement_info.transporter_info.transGuid;
    //            mover->obj_movement_info.transporter_info.time = movement_info.transporter_info.time;
    //            mover->obj_movement_info.transporter_info.position.x = movement_info.transporter_info.position.x;
    //            mover->obj_movement_info.transporter_info.position.y = movement_info.transporter_info.position.y;
    //            mover->obj_movement_info.transporter_info.position.z = movement_info.transporter_info.position.z;
    //            mover->obj_movement_info.transporter_info.position.o = movement_info.transporter_info.position.o;

    //            mover->m_transportData.transportGuid = movement_info.transporter_info.transGuid;
    //            mover->m_transportData.relativePosition.x = movement_info.transporter_info.position.x;
    //            mover->m_transportData.relativePosition.y = movement_info.transporter_info.position.y;
    //            mover->m_transportData.relativePosition.z = movement_info.transporter_info.position.z;
    //            mover->m_transportData.relativePosition.o = movement_info.transporter_info.position.o;
    //        }
    //        else
    //        {
    //            /* no changes */
    //            mover->obj_movement_info.transporter_info.time = movement_info.transporter_info.time;
    //            mover->obj_movement_info.transporter_info.position.x = movement_info.transporter_info.position.x;
    //            mover->obj_movement_info.transporter_info.position.y = movement_info.transporter_info.position.y;
    //            mover->obj_movement_info.transporter_info.position.z = movement_info.transporter_info.position.z;
    //            mover->obj_movement_info.transporter_info.position.o = movement_info.transporter_info.position.o;

    //            mover->m_transportData.relativePosition.x = movement_info.transporter_info.position.x;
    //            mover->m_transportData.relativePosition.y = movement_info.transporter_info.position.y;
    //            mover->m_transportData.relativePosition.z = movement_info.transporter_info.position.z;
    //            mover->m_transportData.relativePosition.o = movement_info.transporter_info.position.o;
    //        }
    //    }
    //}

    /************************************************************************/
    /* Breathing System                                                     */
    /************************************************************************/
    mover->handleBreathing(movementInfo, this);

    /************************************************************************/
    /* Remove Auras                                                         */
    /************************************************************************/
    mover->handleAuraInterruptForMovementFlags(movementInfo);

    ///************************************************************************/
    ///* Update our position in the server.                                   */
    ///************************************************************************/

    //// Player is the active mover
    //if (m_MoverWoWGuid.GetOldGuid() == _player->GetGUID())
    //{

    //    if (!_player->GetTransport())
    //    {
    //        if (!_player->SetPosition(movement_info.position.x, movement_info.position.y, movement_info.position.z, movement_info.position.o))
    //        {
    //            //extra check to set HP to 0 only if the player is dead (KillPlayer() has already this check)
    //            if (_player->isAlive())
    //            {
    //                _player->SetHealth(0);
    //                _player->KillPlayer();
    //            }

    //            MapInfo const* pMapinfo = sMySQLStore.GetWorldMapInfo(_player->GetMapId());
    //            if (pMapinfo != nullptr)
    //            {
    //                if (pMapinfo->type == INSTANCE_NULL || pMapinfo->type == INSTANCE_BATTLEGROUND)
    //                {
    //                    _player->RepopAtGraveyard(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId());
    //                }
    //                else
    //                {
    //                    _player->RepopAtGraveyard(pMapinfo->repopx, pMapinfo->repopy, pMapinfo->repopz, pMapinfo->repopmapid);
    //                }
    //            }
    //            else
    //            {
    //                _player->RepopAtGraveyard(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId());
    //            }//Teleport player to graveyard. Stops players from QQing..
    //        }
    //    }
    //}
    //else
    //{
    //    if (!mover->isRooted())
    //        mover->SetPosition(movement_info.position.x, movement_info.position.y, movement_info.position.z, movement_info.position.o);
    //}

    /************************************************************************/
    /* Update our Position                                                  */
    /************************************************************************/
    mover->SetPosition(movementInfo.getPosition()->x, movementInfo.getPosition()->y, movementInfo.getPosition()->z, movementInfo.getPosition()->o);
    mover->movement_info = movementInfo;

    if (opcode == MSG_MOVE_FALL_LAND && mover /*&& !mover->IsTaxiFlying()*/)
        mover->handleFall(movementInfo);

    WorldPacket data(SMSG_PLAYER_MOVE, recv_data.size());
    data << movementInfo;
    mover->SendMessageToSet(&data, false);
}

void MovementInfo::readMovementInfo(ByteBuffer& data, uint16_t opcode)
{
    bool hasTransportData = false,
        hasMovementFlags = false,
        hasMovementFlags2 = false;

    MovementStatusElements* sequence = GetMovementStatusElementsSequence(opcode);
    if (!sequence)
    {
        LogError("Unsupported MovementInfo::Read for 0x%X (%s)!", opcode);
        return;
    }

    for (uint32_t i = 0; i < MSE_COUNT; ++i)
    {
        MovementStatusElements element = sequence[i];
        if (element == MSEEnd)
            break;

        if (element >= MSEGuidBit0 && element <= MSEGuidBit7)
        {
            guid[element - MSEGuidBit0] = data.readBit();
            continue;
        }

        if (element >= MSEGuid2Bit0 && element <= MSEGuid2Bit7)
        {
            guid2[element - MSEGuid2Bit0] = data.readBit();
            continue;
        }

        if (element >= MSETransportGuidBit0 && element <= MSETransportGuidBit7)
        {
            if (hasTransportData)
                transport_guid[element - MSETransportGuidBit0] = data.readBit();
            continue;
        }

        if (element >= MSEGuidByte0 && element <= MSEGuidByte7)
        {
            if (guid[element - MSEGuidByte0])
                guid[element - MSEGuidByte0] ^= data.readUInt8();
            continue;
        }

        if (element >= MSEGuid2Byte0 && element <= MSEGuid2Byte7)
        {
            if (guid2[element - MSEGuid2Byte0])
                guid2[element - MSEGuid2Byte0] ^= data.readUInt8();
            continue;
        }

        if (element >= MSETransportGuidByte0 && element <= MSETransportGuidByte7)
        {
            if (hasTransportData && transport_guid[element - MSETransportGuidByte0])
                transport_guid[element - MSETransportGuidByte0] ^= data.readUInt8();
            continue;
        }

        switch (element)
        {
            case MSEFlags:
                if (hasMovementFlags)
                    move_flags = data.readBits(30);
                break;
            case MSEFlags2:
                if (hasMovementFlags2)
                    move_flags2 = static_cast<uint16_t>(data.readBits(12));
                break;
            case MSEHasUnknownBit:
                data.readBit();
                break;
            case MSETimestamp:
                if (status_info.hasTimeStamp)
                    data >> update_time;
                break;
            case MSEHasTimestamp:
                status_info.hasTimeStamp = !data.readBit();
                break;
            case MSEHasOrientation:
                status_info.hasOrientation = !data.readBit();
                break;
            case MSEHasMovementFlags:
                hasMovementFlags = !data.readBit();
                break;
            case MSEHasMovementFlags2:
                hasMovementFlags2 = !data.readBit();
                break;
            case MSEHasPitch:
                status_info.hasPitch = !data.readBit();
                break;
            case MSEHasFallData:
                status_info.hasFallData = data.readBit();
                break;
            case MSEHasFallDirection:
                if (status_info.hasFallData)
                    status_info.hasFallDirection = data.readBit();
                break;
            case MSEHasTransportData:
                hasTransportData = data.readBit();
                break;
            case MSEHasTransportTime2:
                if (hasTransportData)
                    status_info.hasTransportTime2 = data.readBit();
                break;
            case MSEHasTransportTime3:
                if (hasTransportData)
                    status_info.hasTransportTime3 = data.readBit();
                break;
            case MSEHasSpline:
                status_info.hasSpline = data.readBit();
                break;
            case MSEHasSplineElevation:
                status_info.hasSplineElevation = !data.readBit();
                break;
            case MSEPositionX:
                data >> position.x;
                break;
            case MSEPositionY:
                data >> position.y;
                break;
            case MSEPositionZ:
                data >> position.z;
                break;
            case MSEPositionO:
                if (status_info.hasOrientation)
                    data >> position.o;
                break;
            case MSEPitch:
                if (status_info.hasPitch)
                    data >> pitch_rate;
                break;
            case MSEFallTime:
                if (status_info.hasFallData)
                    data >> fall_time;
                break;
            case MSESplineElevation:
                if (status_info.hasSplineElevation)
                    data >> spline_elevation;
                break;
            case MSEFallHorizontalSpeed:
                if (status_info.hasFallData && status_info.hasFallDirection)
                    data >> jump_info.xyspeed;
                break;
            case MSEFallVerticalSpeed:
                if (status_info.hasFallData)
                    data >> jump_info.velocity;
                break;
            case MSEFallCosAngle:
                if (status_info.hasFallData && status_info.hasFallDirection)
                    data >> jump_info.cosAngle;
                break;
            case MSEFallSinAngle:
                if (status_info.hasFallData && status_info.hasFallDirection)
                    data >> jump_info.sinAngle;
                break;
            case MSETransportSeat:
                if (hasTransportData)
                    data >> transport_seat;
                break;
            case MSETransportPositionO:
                if (hasTransportData)
                    data >> transport_position.o;
                break;
            case MSETransportPositionX:
                if (hasTransportData)
                    data >> transport_position.x;
                break;
            case MSETransportPositionY:
                if (hasTransportData)
                    data >> transport_position.y;
                break;
            case MSETransportPositionZ:
                if (hasTransportData)
                    data >> transport_position.z;
                break;
            case MSETransportTime:
                if (hasTransportData)
                    data >> transport_time;
                break;
            case MSETransportTime2:
                if (hasTransportData && status_info.hasTransportTime2)
                    data >> transport_time2;
                break;
            case MSETransportTime3:
                if (hasTransportData && status_info.hasTransportTime3)
                    data >> fall_time;
                break;
            case MSEMovementCounter:
                data.read_skip<uint32_t>();
                break;
            case MSEByteParam:
                data >> byte_parameter;
                break;
            default:
                ARCEMU_ASSERT(false && "Wrong movement status element");
                break;
        }
    }
}

void MovementInfo::writeMovementInfo(ByteBuffer& data, uint16_t opcode, float custom_speed) const
{
    bool hasTransportData = !transport_guid.IsEmpty();

    MovementStatusElements* sequence = GetMovementStatusElementsSequence(opcode);
    if (!sequence)
    {
        LogError("Unsupported MovementInfo::Write for 0x%X!", opcode);
        return;
    }

    for (uint32_t i = 0; i < MSE_COUNT; ++i)
    {
        MovementStatusElements element = sequence[i];

        if (element == MSEEnd)
            break;

        if (element >= MSEGuidBit0 && element <= MSEGuidBit7)
        {
            data.writeBit(guid[element - MSEGuidBit0]);
            continue;
        }

        if (element >= MSETransportGuidBit0 && element <= MSETransportGuidBit7)
        {
            if (hasTransportData)
                data.writeBit(transport_guid[element - MSETransportGuidBit0]);
            continue;
        }

        if (element >= MSEGuidByte0 && element <= MSEGuidByte7)
        {
            if (guid[element - MSEGuidByte0])
                data << uint8_t((guid[element - MSEGuidByte0] ^ 1));
            continue;
        }

        if (element >= MSETransportGuidByte0 && element <= MSETransportGuidByte7)
        {
            if (hasTransportData && transport_guid[element - MSETransportGuidByte0])
                data << uint8_t((transport_guid[element - MSETransportGuidByte0] ^ 1));
            continue;
        }

        switch (element)
        {
            case MSEHasMovementFlags:
                data.writeBit(!move_flags);
                break;
            case MSEHasMovementFlags2:
                data.writeBit(!move_flags2);
                break;
            case MSEFlags:
                if (move_flags)
                    data.writeBits(move_flags, 30);
                break;
            case MSEFlags2:
                if (move_flags2)
                    data.writeBits(move_flags2, 12);
                break;
            case MSETimestamp:
                if (status_info.hasTimeStamp)
                    data << Util::getMSTime();
                break;
            case MSEHasPitch:
                data.writeBit(!status_info.hasPitch);
                break;
            case MSEHasTimestamp:
                data.writeBit(!status_info.hasTimeStamp);
                break;
            case MSEHasUnknownBit:
                data.writeBit(false);
                break;
            case MSEHasFallData:
                data.writeBit(status_info.hasFallData);
                break;
            case MSEHasFallDirection:
                if (status_info.hasFallData)
                    data.writeBit(status_info.hasFallDirection);
                break;
            case MSEHasTransportData:
                data.writeBit(hasTransportData);
                break;
            case MSEHasTransportTime2:
                if (hasTransportData)
                    data.writeBit(status_info.hasTransportTime2);
                break;
            case MSEHasTransportTime3:
                if (hasTransportData)
                    data.writeBit(status_info.hasTransportTime3);
                break;
            case MSEHasSpline:
                data.writeBit(status_info.hasSpline);
                break;
            case MSEHasSplineElevation:
                data.writeBit(!status_info.hasSplineElevation);
                break;
            case MSEPositionX:
                data << float(position.x);
                break;
            case MSEPositionY:
                data << float(position.y);
                break;
            case MSEPositionZ:
                data << float(position.z);
                break;
            case MSEPositionO:
                if (status_info.hasOrientation)
                    data << float(normalizeOrientation(position.o));
                break;
            case MSEPitch:
                if (status_info.hasPitch)
                    data << float(pitch_rate);
                break;
            case MSEHasOrientation:
                data.writeBit(!status_info.hasOrientation);
                break;
            case MSEFallTime:
                if (status_info.hasFallData)
                    data << uint32_t(fall_time);
                break;
            case MSESplineElevation:
                if (status_info.hasSplineElevation)
                    data << float(spline_elevation);
                break;
            case MSEFallHorizontalSpeed:
                if (status_info.hasFallData && status_info.hasFallDirection)
                    data << float(jump_info.xyspeed);
                break;
            case MSEFallVerticalSpeed:
                if (status_info.hasFallData)
                    data << float(jump_info.velocity);
                break;
            case MSEFallCosAngle:
                if (status_info.hasFallData && status_info.hasFallDirection)
                    data << float(jump_info.cosAngle);
                break;
            case MSEFallSinAngle:
                if (status_info.hasFallData && status_info.hasFallDirection)
                    data << float(jump_info.sinAngle);
                break;
            case MSETransportSeat:
                if (hasTransportData)
                    data << int8_t(transport_seat);
                break;
            case MSETransportPositionO:
                if (hasTransportData)
                    data << float(normalizeOrientation(transport_position.o));
                break;
            case MSETransportPositionX:
                if (hasTransportData)
                    data << float(transport_position.x);
                break;
            case MSETransportPositionY:
                if (hasTransportData)
                    data << float(transport_position.y);
                break;
            case MSETransportPositionZ:
                if (hasTransportData)
                    data << float(transport_position.z);
                break;
            case MSETransportTime:
                if (hasTransportData)
                    data << uint32_t(transport_time);
                break;
            case MSETransportTime2:
                if (hasTransportData && status_info.hasTransportTime2)
                    data << uint32_t(transport_time2);
                break;
            case MSETransportTime3:
                if (hasTransportData && status_info.hasTransportTime3)
                    data << uint32_t(fall_time);
                break;
            case MSEMovementCounter:
                data << uint32_t(0);
                break;
            case MSECustomSpeed:
                data << float(custom_speed);
                break;
            default:
                ARCEMU_ASSERT(false && "Wrong movement status element");
                break;
            }
    }
}

void WorldSession::HandleForceSpeedAckOpcodes(WorldPacket& /*recvData*/)
{
    LOG_DEBUG("WORLD : Received FORCED SPEED ACK package!");
}

void WorldSession::HandleMoveWorldportAckOpcode(WorldPacket& /*recvData*/)
{
    GetPlayer()->SetPlayerStatus(NONE);
    if (_player->IsInWorld())
    {
        return;
    }

    if (_player->GetTransport() && _player->GetMapId() != _player->GetTransport()->GetMapId())
    {
        Transporter* pTrans = _player->GetTransport();

        float c_tposx = pTrans->GetPositionX() + _player->GetTransPositionX();
        float c_tposy = pTrans->GetPositionY() + _player->GetTransPositionY();
        float c_tposz = pTrans->GetPositionZ() + _player->GetTransPositionZ();


        _player->SetMapId(pTrans->GetMapId());
        _player->SetPosition(c_tposx, c_tposy, c_tposz, _player->GetOrientation());

        WorldPacket data(SMSG_NEW_WORLD, 20);
        data << c_tposx;
        data << _player->GetOrientation();
        data << c_tposz;
        data << pTrans->GetMapId();
        data << c_tposy;
        SendPacket(&data);
    }
    else
    {
        _player->m_TeleportState = 2;
        _player->AddToWorld();
    }
}

void WorldSession::HandleMoveTeleportAckOpcode(WorldPacket& recvData)
{
    uint32_t flags;
    uint32_t time;
    recvData >> flags;
    recvData >> time;

    ObjectGuid guid;
    guid[5] = recvData.readBit();
    guid[0] = recvData.readBit();
    guid[1] = recvData.readBit();
    guid[6] = recvData.readBit();
    guid[3] = recvData.readBit();
    guid[7] = recvData.readBit();
    guid[2] = recvData.readBit();
    guid[4] = recvData.readBit();

    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid[0]);

    if (guid == _player->GetGUID())
    {
        if (worldConfig.antiHack.isTeleportHackCheckEnabled && !(HasGMPermissions() && worldConfig.antiHack.isAntiHackCheckDisabledForGm) && _player->GetPlayerStatus() != TRANSFER_PENDING)
        {
            /* we're obviously cheating */
            sCheatLog.writefromsession(this, "Used teleport hack, disconnecting.");
            Disconnect();
            return;
        }

        if (worldConfig.antiHack.isTeleportHackCheckEnabled && !(HasGMPermissions() && worldConfig.antiHack.isAntiHackCheckDisabledForGm) && _player->m_position.Distance2DSq(_player->m_sentTeleportPosition) > 625.0f)	/* 25.0f*25.0f */
        {
            /* cheating.... :(*/
            sCheatLog.writefromsession(this, "Used teleport hack {2}, disconnecting.");
            Disconnect();
            return;
        }

        LOG_DEBUG("WORLD: got MSG_MOVE_TELEPORT_ACK.");
        GetPlayer()->SetPlayerStatus(NONE);
        _player->SpeedCheatReset();

        std::list<Pet*> summons = _player->GetSummons();
        for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
        {
            // move pet too
            (*itr)->SetPosition((GetPlayer()->GetPositionX() + 2), (GetPlayer()->GetPositionY() + 2), GetPlayer()->GetPositionZ(), M_PI_FLOAT);
        }
        if (_player->m_sentTeleportPosition.x != 999999.0f)
        {
            _player->m_position = _player->m_sentTeleportPosition;
            _player->m_sentTeleportPosition.ChangeCoords(999999.0f, 999999.0f, 999999.0f);
        }
    }
}

void WorldSession::HandleAcknowledgementOpcodes(WorldPacket& recv_data)
{
    LOG_DEBUG("Received ACK package!");

    recv_data.rfinish();
}
