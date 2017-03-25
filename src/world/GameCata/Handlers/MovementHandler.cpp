/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Warden/SpeedDetector.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Map/WorldCreatorDefines.hpp"
#include "GameCata/Movement/MovementStructures.h"

#define MOVEMENT_PACKET_TIME_DELAY 500

void WorldSession::HandleMovementOpcodes(WorldPacket& recv_data)
{
    uint32 opcode = recv_data.GetOpcode();
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
    movementInfo.Read(recv_data, opcode);
    //recv_data >> movementInfo;

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
//    if (recv_data.GetOpcode() == MSG_MOVE_FALL_LAND || movement_info.flags & MOVEFLAG_SWIMMING)
//        _player->jumping = false;
//    if (!_player->jumping && (recv_data.GetOpcode() == MSG_MOVE_JUMP || movement_info.flags & MOVEFLAG_FALLING))
//        _player->jumping = true;
//
//    /************************************************************************/
//    /* Update player movement state                                         */
//    /************************************************************************/
//    uint32_t opcode = recv_data.GetOpcode();
//
//    switch (opcode)
//    {
//        case MSG_MOVE_START_FORWARD:
//        case MSG_MOVE_START_BACKWARD:
//            _player->moving = true;
//            break;
//        case MSG_MOVE_START_STRAFE_LEFT:
//        case MSG_MOVE_START_STRAFE_RIGHT:
//            _player->strafing = true;
//            break;
//        case MSG_MOVE_JUMP:
//            _player->jumping = true;
//            break;
//        case MSG_MOVE_STOP:
//            _player->moving = false;
//            break;
//        case MSG_MOVE_STOP_STRAFE:
//            _player->strafing = false;
//            break;
//        case MSG_MOVE_FALL_LAND:
//            _player->jumping = false;
//            break;
//
//        default:
//            moved = false;
//            break;
//    }
//
//#if 0
//
//    LOG_DETAIL("Got %s", g_worldOpcodeNames[opcode].name);
//
//    LOG_DETAIL("Movement flags");
//    for (uint32 i = 0; i < nmovementflags; i++)
//        if ((movement_info.flags & MoveFlagsToNames[i].flag) != 0)
//            LOG_DETAIL("%s", MoveFlagsToNames[i].name);
//
//#endif
//
//    if (moved)
//    {
//        if (!_player->moving && !_player->strafing && !_player->jumping)
//        {
//            _player->m_isMoving = false;
//        }
//        else
//        {
//            _player->m_isMoving = true;
//        }
//    }
//
//    // Rotating your character with a hold down right click mouse button
//    if (_player->GetOrientation() != movement_info.position.o)
//        _player->isTurning = true;
//    else
//        _player->isTurning = false;
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
//        _player->SDetector->AddSample(movement_info.position.x, movement_info.position.y, getMSTime(), speed);
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
//
//    /************************************************************************/
//    /* Dump movement flags - Wheee!                                         */
//    /************************************************************************/
//#if 0
//    LOG_DEBUG("=========================================================");
//    LOG_DEBUG("Full movement flags: 0x%.8X", movement_info.flags);
//    uint32 z, b;
//    for (z = 1, b = 1; b < 32;)
//    {
//        if (movement_info.flags & z)
//            LOG_DEBUG("   Bit %u (0x%.8X or %u) is set!", b, z, z);
//
//        z <<= 1;
//        b += 1;
//    }
//    LOG_DEBUG("=========================================================");
//#endif
//
//    /************************************************************************/
//    /* Orientation dumping                                                  */
//    /************************************************************************/
//#if 0
//    LOG_DEBUG("Packet: 0x%03X (%s)", recv_data.GetOpcode(), getOpcodeName(recv_data.GetOpcode()).c_str());
//    LOG_DEBUG("Orientation: %.10f", movement_info.orientation);
//#endif

    ///************************************************************************/
    ///* Calculate the timestamp of the packet we have to send out            */
    ///************************************************************************/
    //size_t pos = (size_t)m_MoverWoWGuid.GetNewGuidLen() + 1;
    //uint32 mstime = getMSTime();
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
    mover->handleBreathing(movement_info, this);

    ///************************************************************************/
    ///* Remove Spells                                                        */
    ///************************************************************************/
    //uint32 flags = 0;
    //if ((movement_info.flags & MOVEFLAG_MOTION_MASK) != 0)
    //    flags |= AURA_INTERRUPT_ON_MOVEMENT;

    //if (!(movement_info.flags & MOVEFLAG_SWIMMING || movement_info.flags & MOVEFLAG_FALLING))
    //    flags |= AURA_INTERRUPT_ON_LEAVE_WATER;
    //if (movement_info.flags & MOVEFLAG_SWIMMING)
    //    flags |= AURA_INTERRUPT_ON_ENTER_WATER;
    //if ((movement_info.flags & MOVEFLAG_TURNING_MASK) || _player->isTurning)
    //    flags |= AURA_INTERRUPT_ON_TURNING;
    ///*if (movement_info.flags & MOVEFLAG_REDIRECTED)
    //    flags |= AURA_INTERRUPT_ON_JUMP;*/

    //_player->RemoveAurasByInterruptFlag(flags);

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
    mover->SetPosition(movementInfo.position.x, movementInfo.position.y, movementInfo.position.z, movementInfo.position.o);
    //mover->movement_info = movementInfo;


    /*if (opcode == MSG_MOVE_FALL_LAND && mover)
        mover->HandleFall(movementInfo);*/

    WorldPacket data(SMSG_PLAYER_MOVE, recv_data.size());
    //data << movementInfo;
    movementInfo.Write(data, opcode);
    mover->SendMessageToSet(&data, false);
}

void MovementInfo::Read(ByteBuffer& data, uint32 opcode)
{
    MovementStatusElements* sequence = GetMovementStatusElementsSequence(opcode);
    if (!sequence)
    {
        LogError("Unsupported MovementInfo::Read for 0x%X (%s)!", opcode);
        return;
    }

    bool hasMovementFlags = false;
    bool hasMovementFlags2 = false;
    bool hasTimestamp = false;
    bool hasOrientation = false;
    bool hasTransportData = false;
    bool hasTransportTime2 = false;
    bool hasTransportVehicleId = false;
    bool hasPitch = false;
    bool hasFallData = false;
    bool hasFallDirection = false;
    bool hasSplineElevation = false;

    ObjectGuid guid;
    ObjectGuid tguid;

    for (; *sequence != MSEEnd; ++sequence)
    {
        MovementStatusElements const& element = *sequence;

        switch (element)
        {
            case MSEHasGuidByte0:
            case MSEHasGuidByte1:
            case MSEHasGuidByte2:
            case MSEHasGuidByte3:
            case MSEHasGuidByte4:
            case MSEHasGuidByte5:
            case MSEHasGuidByte6:
            case MSEHasGuidByte7:
                guid[element - MSEHasGuidByte0] = data.readBit();
                break;
            case MSEHasTransportGuidByte0:
            case MSEHasTransportGuidByte1:
            case MSEHasTransportGuidByte2:
            case MSEHasTransportGuidByte3:
            case MSEHasTransportGuidByte4:
            case MSEHasTransportGuidByte5:
            case MSEHasTransportGuidByte6:
            case MSEHasTransportGuidByte7:
                if (hasTransportData)
                    tguid[element - MSEHasTransportGuidByte0] = data.readBit();
                break;
            case MSEGuidByte0:
            case MSEGuidByte1:
            case MSEGuidByte2:
            case MSEGuidByte3:
            case MSEGuidByte4:
            case MSEGuidByte5:
            case MSEGuidByte6:
            case MSEGuidByte7:
                data.ReadByteSeq(guid[element - MSEGuidByte0]);
                break;
            case MSETransportGuidByte0:
            case MSETransportGuidByte1:
            case MSETransportGuidByte2:
            case MSETransportGuidByte3:
            case MSETransportGuidByte4:
            case MSETransportGuidByte5:
            case MSETransportGuidByte6:
            case MSETransportGuidByte7:
                if (hasTransportData)
                    data.ReadByteSeq(tguid[element - MSETransportGuidByte0]);
                break;
            case MSEHasMovementFlags:
                hasMovementFlags = !data.readBit();
                break;
            case MSEHasMovementFlags2:
                hasMovementFlags2 = !data.readBit();
                break;
            case MSEHasTimestamp:
                hasTimestamp = !data.readBit();
                break;
            case MSEHasOrientation:
                hasOrientation = !data.readBit();
                break;
            case MSEHasTransportData:
                hasTransportData = data.readBit();
                break;
            case MSEHasTransportTime2:
                if (hasTransportData)
                    hasTransportTime2 = data.readBit();
                break;
            case MSEHasTransportTime3:
                if (hasTransportData)
                    hasTransportVehicleId = data.readBit();
                break;
            case MSEHasPitch:
                hasPitch = !data.readBit();
                break;
            case MSEHasFallData:
                hasFallData = data.readBit();
                break;
            case MSEHasFallDirection:
                if (hasFallData)
                    hasFallDirection = data.readBit();
                break;
            case MSEHasSplineElevation:
                hasSplineElevation = !data.readBit();
                break;
            case MSEHasSpline:
                data.readBit();
                break;
            case MSEMovementFlags:
                if (hasMovementFlags)
                    flags = data.readBits(30);
                break;
            case MSEMovementFlags2:
                if (hasMovementFlags2)
                    flags2 = data.readBits(12);
                break;
            case MSETimestamp:
                if (hasTimestamp)
                    data >> time;
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
            case MSEOrientation:
                if (hasOrientation)
                    data >> position.o;
                break;
            case MSETransportPositionX:
                if (hasTransportData)
                    data >> transporter_info.position.x;
                break;
            case MSETransportPositionY:
                if (hasTransportData)
                    data >> transporter_info.position.y;
                break;
            case MSETransportPositionZ:
                if (hasTransportData)
                    data >> transporter_info.position.z;
                break;
            case MSETransportOrientation:
                if (hasTransportData)
                    data >> transporter_info.position.o;
                break;
            case MSETransportSeat:
                if (hasTransportData)
                    data >> transporter_info.seat;
                break;
            case MSETransportTime:
                if (hasTransportData)
                    data >> transporter_info.time;
                break;
            case MSETransportTime2:
                if (hasTransportData && hasTransportTime2)
                    data >> transporter_info.time2;
                break;
            case MSETransportVehicleId:
                if (hasTransportData && hasTransportVehicleId)
                    data >> vehicle_id;
                break;
            case MSEPitch:
                if (hasPitch)
                    pitch = G3D::wrap(data.read<float>(), float(-M_PI), float(M_PI));
                break;
            case MSEFallTime:
                if (hasFallData)
                    data >> fall_time;
                break;
            case MSEFallVerticalSpeed:
                if (hasFallData)
                    data >> redirectVelocity;
                break;
            case MSEFallCosAngle:
                if (hasFallData && hasFallDirection)
                    data >> redirectCos;
                break;
            case MSEFallSinAngle:
                if (hasFallData && hasFallDirection)
                    data >> redirectSin;
                break;
            case MSEFallHorizontalSpeed:
                if (hasFallData && hasFallDirection)
                    data >> redirect2DSpeed;
                break;
            case MSESplineElevation:
                if (hasSplineElevation)
                    data >> spline_elevation;
                break;
            case MSECounter:
                data.read_skip<uint32>();   /// @TODO: Maybe compare it with m_movementCounter to verify that packets are sent & received in order?
                break;
            case MSEZeroBit:
            case MSEOneBit:
                data.readBit();
                break;
            case MSEByteParam:
                data >> byteParam;
                break;
            default:
                ARCEMU_ASSERT(false && "Wrong movement status element");
                break;
        }
    }

    guid = guid;
    transporter_info.guid = tguid;
}

void MovementInfo::Write(ByteBuffer& data, uint32 opcode, float extra) const
{
    bool hasMovementFlags = flags != 0;
    bool hasMovementFlags2 = flags2 != 0;
    bool hasTimestamp = true;
    bool hasOrientation = !G3D::fuzzyEq(position.o, 0.0f);
    bool hasTransportData = !t_guid.IsEmpty();
    bool hasSpline = false; // IsSplineEnabled();

    bool hasTransportTime2 = hasTransportData && transporter_info.time2 != 0;
    bool hasTransportVehicleId = hasTransportData && vehicle_id != 0;
    bool hasPitch = HasMovementFlag(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING) || HasMovementFlag(MOVEFLAG2_ALLOW_PITCHING);
    bool hasFallDirection = HasMovementFlag(MOVEFLAG_FALLING);
    bool hasFallData = hasFallDirection || fall_time != 0;
    bool hasSplineElevation = HasMovementFlag(MOVEFLAG_SPLINE_ELEVATION);

    MovementStatusElements* sequence = GetMovementStatusElementsSequence(opcode);
    if (!sequence)
    {
        LogError("Unsupported MovementInfo::Write for 0x%X!", opcode);
        return;
    }

    ObjectGuid l_guid = guid;
    ObjectGuid l_tguid = hasTransportData ? t_guid : ObjectGuid();

    for (; *sequence != MSEEnd; ++sequence)
    {
        MovementStatusElements const& element = *sequence;

        switch (element)
        {
            case MSEHasGuidByte0:
            case MSEHasGuidByte1:
            case MSEHasGuidByte2:
            case MSEHasGuidByte3:
            case MSEHasGuidByte4:
            case MSEHasGuidByte5:
            case MSEHasGuidByte6:
            case MSEHasGuidByte7:
                data.writeBit(l_guid[element - MSEHasGuidByte0]);
                break;
            case MSEHasTransportGuidByte0:
            case MSEHasTransportGuidByte1:
            case MSEHasTransportGuidByte2:
            case MSEHasTransportGuidByte3:
            case MSEHasTransportGuidByte4:
            case MSEHasTransportGuidByte5:
            case MSEHasTransportGuidByte6:
            case MSEHasTransportGuidByte7:
                if (hasTransportData)
                    data.writeBit(l_tguid[element - MSEHasTransportGuidByte0]);
                break;
            case MSEGuidByte0:
            case MSEGuidByte1:
            case MSEGuidByte2:
            case MSEGuidByte3:
            case MSEGuidByte4:
            case MSEGuidByte5:
            case MSEGuidByte6:
            case MSEGuidByte7:
                data.WriteByteSeq(l_guid[element - MSEGuidByte0]);
                break;
            case MSETransportGuidByte0:
            case MSETransportGuidByte1:
            case MSETransportGuidByte2:
            case MSETransportGuidByte3:
            case MSETransportGuidByte4:
            case MSETransportGuidByte5:
            case MSETransportGuidByte6:
            case MSETransportGuidByte7:
                if (hasTransportData)
                    data.WriteByteSeq(l_tguid[element - MSETransportGuidByte0]);
                break;
            case MSEHasMovementFlags:
                data.writeBit(!hasMovementFlags);
                break;
            case MSEHasMovementFlags2:
                data.writeBit(!hasMovementFlags2);
                break;
            case MSEHasTimestamp:
                data.writeBit(!hasTimestamp);
                break;
            case MSEHasOrientation:
                data.writeBit(!hasOrientation);
                break;
            case MSEHasTransportData:
                data.writeBit(hasTransportData);
                break;
            case MSEHasTransportTime2:
                if (hasTransportData)
                    data.writeBit(hasTransportTime2);
                break;
            case MSEHasTransportTime3:
                if (hasTransportData)
                    data.writeBit(hasTransportVehicleId);
                break;
            case MSEHasPitch:
                data.writeBit(!hasPitch);
                break;
            case MSEHasFallData:
                data.writeBit(hasFallData);
                break;
            case MSEHasFallDirection:
                if (hasFallData)
                    data.writeBit(hasFallDirection);
                break;
            case MSEHasSplineElevation:
                data.writeBit(!hasSplineElevation);
                break;
            case MSEHasSpline:
                data.writeBit(hasSpline);
                break;
            case MSEMovementFlags:
                if (hasMovementFlags)
                    data.writeBits(flags, 30);
                break;
            case MSEMovementFlags2:
                if (hasMovementFlags2)
                    data.writeBits(flags2, 12);
                break;
            case MSETimestamp:
                if (hasTimestamp)
                    data << getMSTime();
                break;
            case MSEPositionX:
                data << position.x;
                break;
            case MSEPositionY:
                data << position.y;
                break;
            case MSEPositionZ:
                data << position.z;
                break;
            case MSEOrientation:
                if (hasOrientation)
                    data << position.o;
                break;
            case MSETransportPositionX:
                if (hasTransportData)
                    data << transporter_info.position.x;
                break;
            case MSETransportPositionY:
                if (hasTransportData)
                    data << transporter_info.position.y;
                break;
            case MSETransportPositionZ:
                if (hasTransportData)
                    data << transporter_info.position.z;
                break;
            case MSETransportOrientation:
                if (hasTransportData)
                    data << transporter_info.position.o;
                break;
            case MSETransportSeat:
                if (hasTransportData)
                    data << transporter_info.seat;
                break;
            case MSETransportTime:
                if (hasTransportData)
                    data << transporter_info.time;
                break;
            case MSETransportTime2:
                if (hasTransportData && hasTransportTime2)
                    data << transporter_info.time2;
                break;
            case MSETransportVehicleId:
                if (hasTransportData && hasTransportVehicleId)
                    data << vehicle_id;
                break;
            case MSEPitch:
                if (hasPitch)
                    data << pitch;
                break;
            case MSEFallTime:
                if (hasFallData)
                    data << fall_time;
                break;
            case MSEFallVerticalSpeed:
                if (hasFallData)
                    data << redirectVelocity;
                break;
            case MSEFallCosAngle:
                if (hasFallData && hasFallDirection)
                    data << redirectCos;
                break;
            case MSEFallSinAngle:
                if (hasFallData && hasFallDirection)
                    data << redirectSin;
                break;
            case MSEFallHorizontalSpeed:
                if (hasFallData && hasFallDirection)
                    data << redirect2DSpeed;
                break;
            case MSESplineElevation:
                if (hasSplineElevation)
                    data << spline_elevation;
                break;
            case MSECounter:
                data << uint32(0);
                break;
            case MSEZeroBit:
                data.writeBit(0);
                break;
            case MSEOneBit:
                data.writeBit(1);
                break;
            case MSEExtraFloat:
                data << float(extra);
                break;
            default:
                LOG_ERROR("MSE %u is not handled!", uint32(element));
                //ARCEMU_ASSERT(false && "Wrong movement status element");
                break;
        }
    }
}

void WorldSession::HandleForceSpeedAckOpcodes(WorldPacket& recv_data)
{
    LOG_DEBUG("WORLD : Received FORCED SPEED ACK package!");
}
