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
    mover->movement_info = movementInfo;


    /*if (opcode == MSG_MOVE_FALL_LAND && mover)
        mover->HandleFall(movementInfo);*/

    WorldPacket data(SMSG_PLAYER_MOVE, recv_data.size());
    data << movementInfo;
    mover->SendMessageToSet(&data, false);
}

void MovementInfo::Read(ByteBuffer& data, uint32 opcode)
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

    for (uint32 i = 0; i < MSE_COUNT; ++i)
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
                t_guid[element - MSETransportGuidBit0] = data.readBit();
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
            if (hasTransportData && t_guid[element - MSETransportGuidByte0])
                t_guid[element - MSETransportGuidByte0] ^= data.readUInt8();
            continue;
        }

        switch (element)
        {
            case MSEFlags:
                if (hasMovementFlags)
                    flags = data.readBits(30);
                break;
            case MSEFlags2:
                if (hasMovementFlags2)
                    flags2 = data.readBits(12);
                break;
            case MSEHasUnknownBit:
                data.readBit();
                break;
            case MSETimestamp:
                if (si.hasTimeStamp)
                    data >> time;
                break;
            case MSEHasTimestamp:
                si.hasTimeStamp = !data.readBit();
                break;
            case MSEHasOrientation:
                si.hasOrientation = !data.readBit();
                break;
            case MSEHasMovementFlags:
                hasMovementFlags = !data.readBit();
                break;
            case MSEHasMovementFlags2:
                hasMovementFlags2 = !data.readBit();
                break;
            case MSEHasPitch:
                si.hasPitch = !data.readBit();
                break;
            case MSEHasFallData:
                si.hasFallData = data.readBit();
                break;
            case MSEHasFallDirection:
                if (si.hasFallData)
                    si.hasFallDirection = data.readBit();
                break;
            case MSEHasTransportData:
                hasTransportData = data.readBit();
                break;
            case MSEHasTransportTime2:
                if (hasTransportData)
                    si.hasTransportTime2 = data.readBit();
                break;
            case MSEHasTransportTime3:
                if (hasTransportData)
                    si.hasTransportTime3 = data.readBit();
                break;
            case MSEHasSpline:
                si.hasSpline = data.readBit();
                break;
            case MSEHasSplineElevation:
                si.hasSplineElevation = !data.readBit();
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
                if (si.hasOrientation)
                    data >> position.o;
                break;
            case MSEPitch:
                if (si.hasPitch)
                    data >> pitch;
                break;;
            case MSEFallTime:
                if (si.hasFallData)
                    data >> fall_time;
                break;
            case MSESplineElevation:
                if (si.hasSplineElevation)
                    data >> spline_elevation;
                break;
            case MSEFallHorizontalSpeed:
                if (si.hasFallData && si.hasFallDirection)
                    data >> redirect2DSpeed;
                break;
            case MSEFallVerticalSpeed:
                if (si.hasFallData)
                    data >> redirectVelocity;
                break;
            case MSEFallCosAngle:
                if (si.hasFallData && si.hasFallDirection)
                    data >> redirectCos;
                break;
            case MSEFallSinAngle:
                if (si.hasFallData && si.hasFallDirection)
                    data >> redirectSin;
                break;
            case MSETransportSeat:
                if (hasTransportData)
                    data >> transporter_info.seat;
                break;
            case MSETransportPositionO:
                if (hasTransportData)
                    data >> transporter_info.position.o;
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
            case MSETransportTime:
                if (hasTransportData)
                    data >> transporter_info.time;
                break;
            case MSETransportTime2:
                if (hasTransportData && si.hasTransportTime2)
                    data >> transporter_info.time2;
                break;
            case MSETransportTime3:
                if (hasTransportData && si.hasTransportTime3)
                    data >> fall_time2;
                break;
            case MSEMovementCounter:
                data.read_skip<uint32>();
                break;
            case MSEByteParam:
                data >> byteParam;
                break;
            default:
                ARCEMU_ASSERT(false && "Wrong movement status element");
                break;
        }
    }
}

void MovementInfo::Write(ByteBuffer& data, uint32 opcode) const
{
    bool hasTransportData = !t_guid.IsEmpty();

    MovementStatusElements* sequence = GetMovementStatusElementsSequence(opcode);
    if (!sequence)
    {
        LogError("Unsupported MovementInfo::Write for 0x%X!", opcode);
        return;
    }

    for (uint32 i = 0; i < MSE_COUNT; ++i)
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
                data.writeBit(t_guid[element - MSETransportGuidBit0]);
            continue;
        }

        if (element >= MSEGuidByte0 && element <= MSEGuidByte7)
        {
            if (guid[element - MSEGuidByte0])
                data << uint8((guid[element - MSEGuidByte0] ^ 1));
            continue;
        }

        if (element >= MSETransportGuidByte0 && element <= MSETransportGuidByte7)
        {
            if (hasTransportData && t_guid[element - MSETransportGuidByte0])
                data << uint8((t_guid[element - MSETransportGuidByte0] ^ 1));
            continue;
        }

        switch (element)
        {
            case MSEHasMovementFlags:
                data.writeBit(!flags);
                break;
            case MSEHasMovementFlags2:
                data.writeBit(!flags2);
                break;
            case MSEFlags:
                if (flags)
                    data.writeBits(flags, 30);
                break;
            case MSEFlags2:
                if (flags2)
                    data.writeBits(flags2, 12);
                break;
            case MSETimestamp:
                if (si.hasTimeStamp)
                    data << uint32(time);
                break;
            case MSEHasPitch:
                data.writeBit(!si.hasPitch);
                break;
            case MSEHasTimestamp:
                data.writeBit(!si.hasTimeStamp);
                break;
            case MSEHasUnknownBit:
                data.writeBit(false);
                break;
            case MSEHasFallData:
                data.writeBit(si.hasFallData);
                break;
            case MSEHasFallDirection:
                if (si.hasFallData)
                    data.writeBit(si.hasFallDirection);
                break;
            case MSEHasTransportData:
                data.writeBit(hasTransportData);
                break;
            case MSEHasTransportTime2:
                if (hasTransportData)
                    data.writeBit(si.hasTransportTime2);
                break;
            case MSEHasTransportTime3:
                if (hasTransportData)
                    data.writeBit(si.hasTransportTime3);
                break;
            case MSEHasSpline:
                data.writeBit(si.hasSpline);
                break;
            case MSEHasSplineElevation:
                data.writeBit(!si.hasSplineElevation);
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
                if (si.hasOrientation)
                    data << float(NormalizeOrientation(position.o));
                break;
            case MSEPitch:
                if (si.hasPitch)
                    data << float(pitch);
                break;
            case MSEHasOrientation:
                data.writeBit(!si.hasOrientation);
                break;
            case MSEFallTime:
                if (si.hasFallData)
                    data << uint32(fall_time);
                break;
            case MSESplineElevation:
                if (si.hasSplineElevation)
                    data << float(spline_elevation);
                break;
            case MSEFallHorizontalSpeed:
                if (si.hasFallData && si.hasFallDirection)
                    data << float(redirect2DSpeed);
                break;
            case MSEFallVerticalSpeed:
                if (si.hasFallData)
                    data << float(redirectVelocity);
                break;
            case MSEFallCosAngle:
                if (si.hasFallData && si.hasFallDirection)
                    data << float(redirectCos);
                break;
            case MSEFallSinAngle:
                if (si.hasFallData && si.hasFallDirection)
                    data << float(redirectSin);
                break;
            case MSETransportSeat:
                if (hasTransportData)
                    data << int8(transporter_info.seat);
                break;
            case MSETransportPositionO:
                if (hasTransportData)
                    data << float(NormalizeOrientation(transporter_info.position.o));
                break;
            case MSETransportPositionX:
                if (hasTransportData)
                    data << float(transporter_info.position.x);
                break;
            case MSETransportPositionY:
                if (hasTransportData)
                    data << float(transporter_info.position.y);
                break;
            case MSETransportPositionZ:
                if (hasTransportData)
                    data << float(transporter_info.position.z);
                break;
            case MSETransportTime:
                if (hasTransportData)
                    data << uint32(time);
                break;
            case MSETransportTime2:
                if (hasTransportData && si.hasTransportTime2)
                    data << uint32(time2);
                break;
            case MSETransportTime3:
                if (hasTransportData && si.hasTransportTime3)
                    data << uint32(fall_time);
                break;
            case MSEMovementCounter:
                data << uint32(0);
                break;
            default:
                ARCEMU_ASSERT(false && "Wrong movement status element");
                break;
        }
    }
}
