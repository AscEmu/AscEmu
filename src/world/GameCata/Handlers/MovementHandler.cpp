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

#define MOVEMENT_PACKET_TIME_DELAY 500

void _HandleBreathing(MovementInfo & movement_info, Player* _player, WorldSession* pSession)
{
    // no water breathing is required
    if (!sWorld.BreathingEnabled || _player->FlyCheat || _player->m_bUnlimitedBreath || !_player->isAlive() || _player->GodModeCheat)
    {
        // player is flagged as in water
        if (_player->m_UnderwaterState & UNDERWATERSTATE_SWIMMING)
            _player->m_UnderwaterState &= ~UNDERWATERSTATE_SWIMMING;

        // player is flagged as under water
        if (_player->m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
        {
            _player->m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            _player->SendMirrorTimer(MIRROR_TYPE_BREATH, _player->m_UnderwaterTime, _player->m_UnderwaterMaxTime, -1);
        }

        // player is above water level
        if (pSession->m_bIsWLevelSet)
        {
            if ((movement_info.position.z + _player->m_noseLevel) > pSession->m_wLevel)
            {
                _player->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_LEAVE_WATER);

                // unset swim session water level
                pSession->m_bIsWLevelSet = false;
            }
        }

        return;
    }

    //player is swimming and not flagged as in the water
    if (movement_info.flags & MOVEFLAG_SWIMMING && !(_player->m_UnderwaterState & UNDERWATERSTATE_SWIMMING))
    {
        _player->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_ENTER_WATER);

        // get water level only if it was not set before
        if (!pSession->m_bIsWLevelSet)
        {
            // water level is somewhere below the nose of the character when entering water
            pSession->m_wLevel = movement_info.position.z + _player->m_noseLevel * 0.95f;
            pSession->m_bIsWLevelSet = true;
        }

        _player->m_UnderwaterState |= UNDERWATERSTATE_SWIMMING;
    }

    // player is not swimming and is not stationary and is flagged as in the water
    if (!(movement_info.flags & MOVEFLAG_SWIMMING) && (movement_info.flags != MOVEFLAG_NONE) && (_player->m_UnderwaterState & UNDERWATERSTATE_SWIMMING))
    {
        // player is above water level
        if ((movement_info.position.z + _player->m_noseLevel) > pSession->m_wLevel)
        {
            _player->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_LEAVE_WATER);

            // unset swim session water level
            pSession->m_bIsWLevelSet = false;

            _player->m_UnderwaterState &= ~UNDERWATERSTATE_SWIMMING;
        }
    }

    // player is flagged as in the water and is not flagged as under the water
    if (_player->m_UnderwaterState & UNDERWATERSTATE_SWIMMING && !(_player->m_UnderwaterState & UNDERWATERSTATE_UNDERWATER))
    {
        //the player is in the water and has gone under water, requires breath bar.
        if ((movement_info.position.z + _player->m_noseLevel) < pSession->m_wLevel)
        {
            _player->m_UnderwaterState |= UNDERWATERSTATE_UNDERWATER;
            _player->SendMirrorTimer(MIRROR_TYPE_BREATH, _player->m_UnderwaterTime, _player->m_UnderwaterMaxTime, -1);
        }
    }

    // player is flagged as in the water and is flagged as under the water
    if (_player->m_UnderwaterState & UNDERWATERSTATE_SWIMMING && _player->m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        //the player is in the water but their face is above water, no breath bar needed.
        if ((movement_info.position.z + _player->m_noseLevel) > pSession->m_wLevel)
        {
            _player->m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            _player->SendMirrorTimer(MIRROR_TYPE_BREATH, _player->m_UnderwaterTime, _player->m_UnderwaterMaxTime, 10);
        }
    }

    // player is flagged as not in the water and is flagged as under the water
    if (!(_player->m_UnderwaterState & UNDERWATERSTATE_SWIMMING) && _player->m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        //the player is out of the water, no breath bar needed.
        if ((movement_info.position.z + _player->m_noseLevel) > pSession->m_wLevel)
        {
            _player->m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            _player->SendMirrorTimer(MIRROR_TYPE_BREATH, _player->m_UnderwaterTime, _player->m_UnderwaterMaxTime, 10);
        }
    }
}

void WorldSession::HandleMovementOpcodes(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        bool moved = true;

    if (_player->GetCharmedByGUID() || _player->GetPlayerStatus() == TRANSFER_PENDING || _player->GetTaxiState() || _player->getDeathState() == JUST_DIED)
        return;

    // spell cancel on movement, for now only fishing is added
    Object* t_go = _player->m_SummonedObject;
    if (t_go != nullptr)
    {
        if (t_go->IsGameObject())
        {
            GameObject* go = static_cast<GameObject*>(t_go);
            if (go->GetType() == GAMEOBJECT_TYPE_FISHINGNODE)
            {
                GameObject_FishingNode* go_fishing_node = static_cast<GameObject_FishingNode*>(go);
                go_fishing_node->EndFishing(true);

                auto spell = _player->GetCurrentSpell();
                if (spell != nullptr)
                {
                    spell->SendChannelUpdate(0);
                    spell->finish(false);
                }
            }
        }
    }

    /************************************************************************/
    /* Clear standing state to stand.				                        */
    /************************************************************************/
    if (recv_data.GetOpcode() == MSG_MOVE_START_FORWARD)
        _player->SetStandState(STANDSTATE_STAND);

    /************************************************************************/
    /* Make sure the packet is the correct size range.                      */
    /************************************************************************/
    //if (recv_data.size() > 80) { Disconnect(); return; }

    /************************************************************************/
    /* Read Movement Data Packet                                            */
    /************************************************************************/
    WoWGuid guid;
    recv_data >> guid;
    movement_info.init(recv_data);

    if (guid != m_MoverWoWGuid.GetOldGuid())
    {
        return;
    }

    // Player is in control of some entity, so we move that instead of the player
    Unit* mover = _player->GetMapMgr()->GetUnit(m_MoverWoWGuid.GetOldGuid());
    if (mover == NULL)
        return;

    /* Anti Multi-Jump Check */
    if (recv_data.GetOpcode() == MSG_MOVE_JUMP && _player->jumping == true && !GetPermissionCount())
    {
        sCheatLog.writefromsession(this, "Detected jump hacking");
        Disconnect();
        return;
    }
    if (recv_data.GetOpcode() == MSG_MOVE_FALL_LAND || movement_info.flags & MOVEFLAG_SWIMMING)
        _player->jumping = false;
    if (!_player->jumping && (recv_data.GetOpcode() == MSG_MOVE_JUMP || movement_info.flags & MOVEFLAG_FALLING))
        _player->jumping = true;

    /************************************************************************/
    /* Update player movement state                                         */
    /************************************************************************/
    uint32_t opcode = recv_data.GetOpcode();

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

        default:
            moved = false;
            break;
    }

#if 0

    LOG_DETAIL("Got %s", g_worldOpcodeNames[opcode].name);

    LOG_DETAIL("Movement flags");
    for (uint32 i = 0; i < nmovementflags; i++)
        if ((movement_info.flags & MoveFlagsToNames[i].flag) != 0)
            LOG_DETAIL("%s", MoveFlagsToNames[i].name);

#endif

    if (moved)
    {
        if (!_player->moving && !_player->strafing && !_player->jumping)
        {
            _player->m_isMoving = false;
        }
        else
        {
            _player->m_isMoving = true;
        }
    }

    // Rotating your character with a hold down right click mouse button
    if (_player->GetOrientation() != movement_info.position.o)
        _player->isTurning = true;
    else
        _player->isTurning = false;


    if (!(HasGMPermissions() && sWorld.no_antihack_on_gm) && !_player->GetCharmedUnitGUID())
    {
        /************************************************************************/
        /* Anti-Teleport                                                        */
        /************************************************************************/
        if (sWorld.antihack_teleport && _player->m_position.Distance2DSq(movement_info.position.x, movement_info.position.y) > 3025.0f
            && _player->getSpeedForType(TYPE_RUN) < 50.0f && !_player->obj_movement_info.transporter_info.guid)
        {
            sCheatLog.writefromsession(this, "Disconnected for teleport hacking. Player speed: %f, Distance traveled: %f", _player->getSpeedForType(TYPE_RUN), sqrt(_player->m_position.Distance2DSq(movement_info.position.x, movement_info.position.y)));
            Disconnect();
            return;
        }
    }

    //update the detector
    if (sWorld.antihack_speed && !_player->GetTaxiState() && _player->obj_movement_info.transporter_info.guid == 0 && !_player->GetSession()->GetPermissionCount())
    {
        // simplified: just take the fastest speed. less chance of fuckups too
        float speed = (_player->flying_aura) ? _player->getSpeedForType(TYPE_FLY) : (_player->getSpeedForType(TYPE_SWIM) > _player->getSpeedForType(TYPE_RUN)) ? _player->getSpeedForType(TYPE_SWIM) : _player->getSpeedForType(TYPE_RUN);

        _player->SDetector->AddSample(movement_info.position.x, movement_info.position.y, getMSTime(), speed);

        if (_player->SDetector->IsCheatDetected())
            _player->SDetector->ReportCheater(_player);
    }

    /************************************************************************/
    /* Remove Emote State                                                   */
    /************************************************************************/
    if (_player->GetEmoteState())
        _player->SetEmoteState(EMOTE_ONESHOT_NONE);

    /************************************************************************/
    /* Make sure the co-ordinates are valid.                                */
    /************************************************************************/
    if (!((movement_info.position.y >= _minY) && (movement_info.position.y <= _maxY)) || !((movement_info.position.x >= _minX) && (movement_info.position.x <= _maxX)))
    {
        Disconnect();
        return;
    }

    /************************************************************************/
    /* Dump movement flags - Wheee!                                         */
    /************************************************************************/
#if 0
    LOG_DEBUG("=========================================================");
    LOG_DEBUG("Full movement flags: 0x%.8X", movement_info.flags);
    uint32 z, b;
    for (z = 1, b = 1; b < 32;)
    {
        if (movement_info.flags & z)
            LOG_DEBUG("   Bit %u (0x%.8X or %u) is set!", b, z, z);

        z <<= 1;
        b += 1;
    }
    LOG_DEBUG("=========================================================");
#endif

    /************************************************************************/
    /* Orientation dumping                                                  */
    /************************************************************************/
#if 0
    LOG_DEBUG("Packet: 0x%03X (%s)", recv_data.GetOpcode(), getOpcodeName(recv_data.GetOpcode()).c_str());
    LOG_DEBUG("Orientation: %.10f", movement_info.orientation);
#endif

    /************************************************************************/
    /* Calculate the timestamp of the packet we have to send out            */
    /************************************************************************/
    size_t pos = (size_t)m_MoverWoWGuid.GetNewGuidLen() + 1;
    uint32 mstime = getMSTime();
    int32 move_time;
    if (m_clientTimeDelay == 0)
        m_clientTimeDelay = mstime - movement_info.time;

    /************************************************************************/
    /* Copy into the output buffer.                                         */
    /************************************************************************/
    if (_player->m_inRangePlayers.size())
    {
        move_time = (movement_info.time - (mstime - m_clientTimeDelay)) + MOVEMENT_PACKET_TIME_DELAY + mstime;
        memcpy(&movement_packet[0], recv_data.contents(), recv_data.size());
        movement_packet[pos + 6] = 0;

        /************************************************************************/
        /* Distribute to all inrange players.                                   */
        /************************************************************************/
        for (std::set<Object*>::iterator itr = _player->m_inRangePlayers.begin(); itr != _player->m_inRangePlayers.end(); ++itr)
        {

            Player* p = static_cast< Player* >((*itr));

            *(uint32*)&movement_packet[pos + 6] = uint32(move_time + p->GetSession()->m_moveDelayTime);

            p->GetSession()->OutPacket(recv_data.GetOpcode(), uint16(recv_data.size() + pos), movement_packet);

        }
    }

    /************************************************************************/
    /* Hack Detection by Classic	                                        */
    /************************************************************************/
    if (!movement_info.transporter_info.guid && recv_data.GetOpcode() != MSG_MOVE_JUMP && !_player->FlyCheat && !_player->flying_aura && !(movement_info.flags & MOVEFLAG_SWIMMING || movement_info.flags & MOVEFLAG_FALLING) && movement_info.position.z > _player->GetPositionZ() && movement_info.position.x == _player->GetPositionX() && movement_info.position.y == _player->GetPositionY())
    {
        WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 13);
        data << _player->GetNewGUID();
        data << uint32(5);      // unknown 0
        SendPacket(&data);
    }

    if ((movement_info.flags & MOVEFLAG_FLYING) && !(movement_info.flags & MOVEFLAG_SWIMMING) && !(_player->flying_aura || _player->FlyCheat))
    {
        WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 13);
        data << _player->GetNewGUID();
        data << uint32(5);      // unknown 0
        SendPacket(&data);
    }

    /************************************************************************/
    /* Falling damage checks                                                */
    /************************************************************************/

    if (_player->blinked)
    {
        _player->blinked = false;
        _player->m_fallDisabledUntil = UNIXTIME + 5;
        _player->SpeedCheatDelay(2000);   //some say they managed to trigger system with knockback. Maybe they moved in air ?
    }
    else
    {
        if (recv_data.GetOpcode() == MSG_MOVE_FALL_LAND)
        {
            // player has finished falling
            //if z_axisposition contains no data then set to current position
            if (!mover->z_axisposition)
                mover->z_axisposition = movement_info.position.z;

            // calculate distance fallen
            uint32 falldistance = float2int32(mover->z_axisposition - movement_info.position.z);
            if (mover->z_axisposition <= movement_info.position.z)
                falldistance = 1;
            /*Safe Fall*/
            if ((int)falldistance > mover->m_safeFall)
                falldistance -= mover->m_safeFall;
            else
                falldistance = 1;

            //checks that player has fallen more than 12 units, otherwise no damage will be dealt
            //falltime check is also needed here, otherwise sudden changes in Z axis position, such as using !recall, may result in death
            if (mover->isAlive() && !mover->bInvincible && (falldistance > 12) && !mover->m_noFallDamage &&
                ((mover->GetGUID() != _player->GetGUID()) || (!_player->GodModeCheat && (UNIXTIME >= _player->m_fallDisabledUntil))))
            {
                // 1.7% damage for each unit fallen on Z axis over 13
                uint32 health_loss = static_cast<uint32>(mover->GetHealth() * (falldistance - 12) * 0.017f);

                if (health_loss >= mover->GetHealth())
                    health_loss = mover->GetHealth();

                else if ((falldistance >= 65) && (mover->GetGUID() == _player->GetGUID()))
                {
                    // Rather than Updating achievement progress every time fall damage is taken, all criteria currently have 65 yard requirement...
                    // Achievement 964: Fall 65 yards without dying.
                    // Achievement 1260: Fall 65 yards without dying while completely smashed during the Brewfest Holiday.
                    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING, falldistance, Player::GetDrunkenstateByValue(_player->GetDrunkValue()), 0);
                }

                mover->SendEnvironmentalDamageLog(mover->GetGUID(), DAMAGE_FALL, health_loss);
                mover->DealDamage(mover, health_loss, 0, 0, 0);

                //_player->RemoveStealth(); // cebernic : why again? lost stealth by AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN already.
            }
            mover->z_axisposition = 0.0f;
        }
        else
            //whilst player is not falling, continuously update Z axis position.
            //once player lands this will be used to determine how far he fell.
            if (!(movement_info.flags & MOVEFLAG_FALLING))
                mover->z_axisposition = movement_info.position.z;
    }

    /************************************************************************/
    /* Transporter Setup                                                    */
    /************************************************************************/
    if ((mover->obj_movement_info.transporter_info.guid != 0) && (movement_info.transporter_info.transGuid.GetOldGuid() == 0))
    {
        /* we left the transporter we were on */

        Transporter *transporter = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(mover->obj_movement_info.transporter_info.guid));
        if (transporter != NULL)
            transporter->RemovePassenger(static_cast<Player*>(mover));

        mover->obj_movement_info.transporter_info.guid = 0;
        _player->SpeedCheatReset();

    }
    else
    {
        if (movement_info.transporter_info.transGuid.GetOldGuid() != 0)
        {

            if (mover->obj_movement_info.transporter_info.guid == 0)
            {
                Transporter *transporter = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(movement_info.transporter_info.transGuid));
                if (transporter != NULL)
                    transporter->AddPassenger(static_cast<Player*>(mover));

                /* set variables */
                mover->obj_movement_info.transporter_info.guid = movement_info.transporter_info.transGuid;
                mover->obj_movement_info.transporter_info.time = movement_info.transporter_info.time;
                mover->obj_movement_info.transporter_info.position.x = movement_info.transporter_info.position.x;
                mover->obj_movement_info.transporter_info.position.y = movement_info.transporter_info.position.y;
                mover->obj_movement_info.transporter_info.position.z = movement_info.transporter_info.position.z;
                mover->obj_movement_info.transporter_info.position.o = movement_info.transporter_info.position.o;

                mover->m_transportData.transportGuid = movement_info.transporter_info.transGuid;
                mover->m_transportData.relativePosition.x = movement_info.transporter_info.position.x;
                mover->m_transportData.relativePosition.y = movement_info.transporter_info.position.y;
                mover->m_transportData.relativePosition.z = movement_info.transporter_info.position.z;
                mover->m_transportData.relativePosition.o = movement_info.transporter_info.position.o;
            }
            else
            {
                /* no changes */
                mover->obj_movement_info.transporter_info.time = movement_info.transporter_info.time;
                mover->obj_movement_info.transporter_info.position.x = movement_info.transporter_info.position.x;
                mover->obj_movement_info.transporter_info.position.y = movement_info.transporter_info.position.y;
                mover->obj_movement_info.transporter_info.position.z = movement_info.transporter_info.position.z;
                mover->obj_movement_info.transporter_info.position.o = movement_info.transporter_info.position.o;

                mover->m_transportData.relativePosition.x = movement_info.transporter_info.position.x;
                mover->m_transportData.relativePosition.y = movement_info.transporter_info.position.y;
                mover->m_transportData.relativePosition.z = movement_info.transporter_info.position.z;
                mover->m_transportData.relativePosition.o = movement_info.transporter_info.position.o;
            }
        }
    }

    /************************************************************************/
    /* Breathing System                                                     */
    /************************************************************************/
    _HandleBreathing(movement_info, _player, this);

    /************************************************************************/
    /* Remove Spells                                                        */
    /************************************************************************/
    uint32 flags = 0;
    if ((movement_info.flags & MOVEFLAG_MOTION_MASK) != 0)
        flags |= AURA_INTERRUPT_ON_MOVEMENT;

    if (!(movement_info.flags & MOVEFLAG_SWIMMING || movement_info.flags & MOVEFLAG_FALLING))
        flags |= AURA_INTERRUPT_ON_LEAVE_WATER;
    if (movement_info.flags & MOVEFLAG_SWIMMING)
        flags |= AURA_INTERRUPT_ON_ENTER_WATER;
    if ((movement_info.flags & MOVEFLAG_TURNING_MASK) || _player->isTurning)
        flags |= AURA_INTERRUPT_ON_TURNING;
    /*if (movement_info.flags & MOVEFLAG_REDIRECTED)
        flags |= AURA_INTERRUPT_ON_JUMP;*/

    _player->RemoveAurasByInterruptFlag(flags);

    /************************************************************************/
    /* Update our position in the server.                                   */
    /************************************************************************/

    // Player is the active mover
    if (m_MoverWoWGuid.GetOldGuid() == _player->GetGUID())
    {

        if (!_player->GetTransport())
        {
            if (!_player->SetPosition(movement_info.position.x, movement_info.position.y, movement_info.position.z, movement_info.position.o))
            {
                //extra check to set HP to 0 only if the player is dead (KillPlayer() has already this check)
                if (_player->isAlive())
                {
                    _player->SetHealth(0);
                    _player->KillPlayer();
                }

                MapInfo const* pMapinfo = sMySQLStore.GetWorldMapInfo(_player->GetMapId());
                if (pMapinfo != nullptr)
                {
                    if (pMapinfo->type == INSTANCE_NULL || pMapinfo->type == INSTANCE_BATTLEGROUND)
                    {
                        _player->RepopAtGraveyard(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId());
                    }
                    else
                    {
                        _player->RepopAtGraveyard(pMapinfo->repopx, pMapinfo->repopy, pMapinfo->repopz, pMapinfo->repopmapid);
                    }
                }
                else
                {
                    _player->RepopAtGraveyard(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId());
                }//Teleport player to graveyard. Stops players from QQing..
            }
        }
    }
    else
    {
        if (!mover->isRooted())
            mover->SetPosition(movement_info.position.x, movement_info.position.y, movement_info.position.z, movement_info.position.o);
    }
}

void MovementInfo::init(WorldPacket& data)
{
    transporter_info.transGuid = 0;
    data >> flags;
    data >> flags2;
    data >> time;

    data >> position.x;
    data >> position.y;
    data >> position.z;
    data >> position.o;

    /*if (HasMovementFlag(MOVEFLAG_TRANSPORT))
    {
        data >> transporter_info.transGuid;
        data >> transporter_info.position.x;
        data >> transporter_info.position.y;
        data >> transporter_info.position.z;
        data >> transporter_info.position.o;
        data >> transporter_info.time;
        data >> transporter_info.seat;

        if (HasMovementFlag2(MOVEFLAG2_INTERPOLATED_MOVE))
        {
            data >> transporter_info.time2;
        }
    }*/

    if (HasMovementFlag((MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || HasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING))
    {
        data >> pitch;
    }

    data >> fall_time;

    /*if (HasMovementFlag(MOVEFLAG_REDIRECTED))
    {
        data >> redirectVelocity;
        data >> redirectSin;
        data >> redirectCos;
        data >> redirect2DSpeed;
    }
    if (HasMovementFlag(MOVEFLAG_SPLINE_MOVER))
    {
        data >> spline_elevation;
    }*/
}

void MovementInfo::write(WorldPacket& data)
{
    data << flags;
    data << flags2;
    data << getMSTime();

    data << position.x;
    data << position.y;
    data << position.z;
    data << position.o;

    /*if (HasMovementFlag(MOVEFLAG_TRANSPORT))
    {
        data << transporter_info.transGuid;
        data << transporter_info.position.x;
        data << transporter_info.position.y;
        data << transporter_info.position.z;
        data << transporter_info.position.o;
        data << transporter_info.time;
        data << transporter_info.seat;

        if (HasMovementFlag2(MOVEFLAG2_INTERPOLATED_MOVE))
        {
            data << transporter_info.time2;
        }
    }*/

    if (HasMovementFlag((MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || HasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING))
    {
        data << pitch;
    }

    data << fall_time;

    if (HasMovementFlag(MOVEFLAG_FALLING))
    {
        data << redirectVelocity;
        data << redirectSin;
        data << redirectCos;
        data << redirect2DSpeed;
    }

    /*if (HasMovementFlag(MOVEFLAG_SPLINE_MOVER))
    {
        data << spline_elevation;
    }*/
}
