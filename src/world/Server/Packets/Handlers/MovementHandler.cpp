/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Warden/SpeedDetector.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Map/WorldCreatorDefines.hpp"
#include <Spell/Definitions/AuraInterruptFlags.h>
#include "Units/Creatures/Pet.h"

#define SWIMMING_TOLERANCE_LEVEL -0.08f
#define MOVEMENT_PACKET_TIME_DELAY 500

#ifdef WIN32

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#define DELTA_EPOCH_IN_USEC 11644473600000000ULL

uint32 TimeStamp()
{
    //return timeGetTime();

    FILETIME ft;
    uint64 t;
    GetSystemTimeAsFileTime(&ft);

    t = (uint64)ft.dwHighDateTime << 32;
    t |= ft.dwLowDateTime;
    t /= 10;
    t -= DELTA_EPOCH_IN_USEC;

    return uint32(((t / 1000000L) * 1000) + ((t % 1000000L) / 1000));
}

uint32 mTimeStamp()
{
    return timeGetTime();
}

#else

uint32 TimeStamp()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

uint32 mTimeStamp()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

#endif

//\TODO move it to version specific files.
#if VERSION_STRING != Cata
void WorldSession::HandleMoveWorldportAckOpcode(WorldPacket& /*recv_data*/)
{
    GetPlayer()->SetPlayerStatus(NONE);
    if (_player->IsInWorld())
    {
        // get outta here
        return;
    }
    LOG_DEBUG("WORLD: got MSG_MOVE_WORLDPORT_ACK.");

    if (_player->GetTransport() && _player->GetMapId() != _player->GetTransport()->GetMapId())
    {
        /* wow, our pc must really suck. */
        Transporter* pTrans = _player->GetTransport();

        float c_tposx = pTrans->GetPositionX() + _player->GetTransPositionX();
        float c_tposy = pTrans->GetPositionY() + _player->GetTransPositionY();
        float c_tposz = pTrans->GetPositionZ() + _player->GetTransPositionZ();


        _player->SetMapId(pTrans->GetMapId());
        _player->SetPosition(c_tposx, c_tposy, c_tposz, _player->GetOrientation());

        WorldPacket dataw(SMSG_NEW_WORLD, 20);

        dataw << pTrans->GetMapId();
        dataw << c_tposx;
        dataw << c_tposy;
        dataw << c_tposz;
        dataw << _player->GetOrientation();

        SendPacket(&dataw);
    }
    else
    {
        _player->m_TeleportState = 2;
        _player->AddToWorld();
    }
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleMoveTeleportAckOpcode(WorldPacket& recv_data)
{
    WoWGuid guid;
    recv_data >> guid;

    uint32 flags;
    uint32 time;
    recv_data >> flags;
    recv_data >> time;

    if (m_MoverWoWGuid.GetOldGuid() == _player->GetGUID())
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
#endif

#if VERSION_STRING != Cata
void _HandleBreathing(MovementInfo & movement_info, Player* _player, WorldSession* pSession)
{
    // no water breathing is required
    if (!worldConfig.server.enableBreathing || _player->FlyCheat || _player->m_bUnlimitedBreath || !_player->isAlive() || _player->GodModeCheat)
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
    if (!(movement_info.flags & MOVEFLAG_SWIMMING) && (movement_info.flags != MOVEFLAG_MOVE_STOP) && (_player->m_UnderwaterState & UNDERWATERSTATE_SWIMMING))
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
#endif

#if VERSION_STRING != Cata
struct MovementFlagName
{
    uint32 flag;
    const char* name;
};

static MovementFlagName MoveFlagsToNames[] =
{
    { MOVEFLAG_MOVE_STOP, "MOVEFLAG_MOVE_STOP" },
    { MOVEFLAG_MOVE_FORWARD, "MOVEFLAG_MOVE_FORWARD" },
    { MOVEFLAG_MOVE_BACKWARD, "MOVEFLAG_MOVE_BACKWARD" },
    { MOVEFLAG_STRAFE_LEFT, "MOVEFLAG_STRAFE_LEFT" },
    { MOVEFLAG_STRAFE_RIGHT, "MOVEFLAG_STRAFE_RIGHT" },
    { MOVEFLAG_TURN_LEFT, "MOVEFLAG_TURN_LEFT" },
    { MOVEFLAG_TURN_RIGHT, "MOVEFLAG_TURN_RIGHT" },
    { MOVEFLAG_PITCH_DOWN, "MOVEFLAG_PITCH_DOWN" },
    { MOVEFLAG_PITCH_UP, "MOVEFLAG_PITCH_UP" },
    { MOVEFLAG_WALK, "MOVEFLAG_WALK" },
    { MOVEFLAG_TRANSPORT, "MOVEFLAG_TRANSPORT" },
    { MOVEFLAG_DISABLEGRAVITY, "MOVEFLAG_DISABLEGRAVITY" },
    { MOVEFLAG_ROOTED, "MOVEFLAG_ROOTED" },
    { MOVEFLAG_REDIRECTED, "MOVEFLAG_REDIRECTED" },
    { MOVEFLAG_FALLING, "MOVEFLAG_FALLING" },
    { MOVEFLAG_FALLING_FAR, "MOVEFLAG_FALLING_FAR" },
    { MOVEFLAG_FREE_FALLING, "MOVEFLAG_FREE_FALLING" },
    { MOVEFLAG_TB_PENDING_STOP, "MOVEFLAG_TB_PENDING_STOP" },
    { MOVEFLAG_TB_PENDING_UNSTRAFE, "MOVEFLAG_TB_PENDING_UNSTRAFE" },
    { MOVEFLAG_TB_PENDING_FALL, "MOVEFLAG_TB_PENDING_FALL" },
    { MOVEFLAG_TB_PENDING_FORWARD, "MOVEFLAG_TB_PENDING_FORWARD" },
    { MOVEFLAG_TB_PENDING_BACKWARD, "MOVEFLAG_TB_PENDING_BACKWARD" },
    { MOVEFLAG_SWIMMING, "MOVEFLAG_SWIMMING" },
    { MOVEFLAG_ASCENDING, "MOVEFLAG_ASCENDING" },
    { MOVEFLAG_DESCENDING, "MOVEFLAG_DESCENDING" },
    { MOVEFLAG_CAN_FLY, "MOVEFLAG_CAN_FLY" },
    { MOVEFLAG_FLYING, "MOVEFLAG_FLYING" },
    { MOVEFLAG_SPLINE_MOVER, "MOVEFLAG_SPLINE_MOVER" },
    { MOVEFLAG_SPLINE_ENABLED, "MOVEFLAG_SPLINE_ENABLED" },
    { MOVEFLAG_WATER_WALK, "MOVEFLAG_WATER_WALK" },
    { MOVEFLAG_FEATHER_FALL, "MOVEFLAG_FEATHER_FALL" },
    { MOVEFLAG_HOVER, "MOVEFLAG_HOVER" }
    //{ MOVEFLAG_LOCAL, "MOVEFLAG_LOCAL" }
};

static const uint32 nmovementflags = sizeof(MoveFlagsToNames) / sizeof(MovementFlagName);
void WorldSession::HandleMovementOpcodes(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    bool moved = true;

    if (/*_player->GetCharmedByGUID() || */_player->GetPlayerStatus() == TRANSFER_PENDING || _player->GetTaxiState() || _player->getDeathState() == JUST_DIED)
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

                auto spell = _player->getCurrentSpell(CURRENT_CHANNELED_SPELL);
                if (spell != nullptr)
                {
                    spell->SendChannelUpdate(0);
                    spell->finish(false);
                }
            }
        }
    }

    Unit* mover = _player->mControledUnit;

    ASSERT(mover != nullptr)

    Player* plrMover = nullptr;
    if (mover->IsPlayer())
        plrMover = dynamic_cast<Player*>(mover);

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

    if (guid != mover->GetGUID())
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

    uint16 opcode = recv_data.GetOpcode();
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

    LOG_DETAIL("Got %s", g_worldOpcodeNames[ opcode ].name);

    LOG_DETAIL("Movement flags");
    for (uint32 i = 0; i < nmovementflags; i++)
        if ((movement_info.flags & MoveFlagsToNames[ i ].flag) != 0)
            LOG_DETAIL("%s", MoveFlagsToNames[ i ].name);

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
    {
        _player->isTurning = true;
    }
    else
    {
        _player->isTurning = false;
    }


    if (!(HasGMPermissions() && worldConfig.antiHack.isAntiHackCheckDisabledForGm) && !_player->GetCharmedUnitGUID())
    {
        /************************************************************************/
        /* Anti-Teleport                                                        */
        /************************************************************************/
        if (worldConfig.antiHack.isTeleportHackCheckEnabled && _player->m_position.Distance2DSq(movement_info.position.x, movement_info.position.y) > 3025.0f
            && _player->getSpeedForType(TYPE_RUN) < 50.0f && !_player->obj_movement_info.transporter_info.guid)
        {
            sCheatLog.writefromsession(this, "Disconnected for teleport hacking. Player speed: %f, Distance traveled: %f", _player->getSpeedForType(TYPE_RUN), sqrt(_player->m_position.Distance2DSq(movement_info.position.x, movement_info.position.y)));
            Disconnect();
            return;
        }
    }

    //update the detector
    if (worldConfig.antiHack.isSpeedHackCkeckEnabled && !_player->GetTaxiState() && _player->obj_movement_info.transporter_info.guid == 0 && !_player->GetSession()->GetPermissionCount())
    {
        // simplified: just take the fastest speed. less chance of fuckups too
        float speed = (_player->flying_aura) ? _player->getSpeedForType(TYPE_FLY) : (_player->getSpeedForType(TYPE_SWIM) > _player->getSpeedForType(TYPE_RUN)) ? _player->getSpeedForType(TYPE_SWIM) : _player->getSpeedForType(TYPE_RUN);

        _player->SDetector->AddSample(movement_info.position.x, movement_info.position.y,Util::getMSTime(), speed);

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
    uint32 mstime = mTimeStamp();
    int32 move_time;
    if (m_clientTimeDelay == 0)
        m_clientTimeDelay = mstime - movement_info.time;

    /************************************************************************/
    /* Copy into the output buffer.                                         */
    /************************************************************************/
    if (_player->getInRangePlayersCount())
    {
        move_time = (movement_info.time - (mstime - m_clientTimeDelay)) + MOVEMENT_PACKET_TIME_DELAY + mstime;
        memcpy(&movement_packet[0], recv_data.contents(), recv_data.size());
        movement_packet[pos + 6] = 0;

        /************************************************************************/
        /* Distribute to all inrange players.                                   */
        /************************************************************************/
        for (const auto& itr : mover->getInRangePlayersSet())
        {
            Player* p = static_cast<Player*>(itr);

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
#if VERSION_STRING > TBC
                else if ((falldistance >= 65) && (mover->GetGUID() == _player->GetGUID()))
                {
                    // Rather than Updating achievement progress every time fall damage is taken, all criteria currently have 65 yard requirement...
                    // Achievement 964: Fall 65 yards without dying.
                    // Achievement 1260: Fall 65 yards without dying while completely smashed during the Brewfest Holiday.
                    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING, falldistance, Player::GetDrunkenstateByValue(_player->GetDrunkValue()), 0);
                }
#endif

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
    if (movement_info.flags & MOVEFLAG_REDIRECTED)
        flags |= AURA_INTERRUPT_ON_JUMP;

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

                MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(_player->GetMapId());
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
#endif

void WorldSession::HandleMoveTimeSkippedOpcode(WorldPacket& /*recvData*/)
{}

void WorldSession::HandleMoveNotActiveMoverOpcode(WorldPacket& recvData)
{
#if VERSION_STRING != Cata
    CHECK_INWORLD_RETURN

    WoWGuid guid;
    recvData >> guid;

    if (guid == m_MoverWoWGuid)
        return;

    movement_info.init(recvData);

    if ((guid != uint64(0)) && (guid == _player->GetCharmedUnitGUID()))
        m_MoverWoWGuid = guid;
    else
        m_MoverWoWGuid.Init(_player->GetGUID());

    // set up to the movement packet
    movement_packet[0] = m_MoverWoWGuid.GetNewGuidMask();
    memcpy(&movement_packet[1], m_MoverWoWGuid.GetNewGuid(), m_MoverWoWGuid.GetNewGuidLen());
#else
    if (recvData.GetOpcode() == 0) { return; }
#endif
}

void WorldSession::HandleSetActiveMoverOpcode(WorldPacket& recvData)
{
    CHECK_INWORLD_RETURN

    // set current movement object
    uint64 guid;
    recvData >> guid;

    if (guid != m_MoverWoWGuid.GetOldGuid())
    {
        // make sure the guid is valid and we aren't cheating
        if (!(_player->m_CurrentCharm == guid) && !(_player->GetGUID() == guid))
        {
            if (_player->GetCurrentVehicle()->GetOwner()->GetGUID() != guid)
                return;
        }

        // generate wowguid
        if (guid != 0)
            m_MoverWoWGuid.Init(guid);
        else
            m_MoverWoWGuid.Init(_player->GetGUID());

        // set up to the movement packet
        movement_packet[0] = m_MoverWoWGuid.GetNewGuidMask();
        memcpy(&movement_packet[1], m_MoverWoWGuid.GetNewGuid(), m_MoverWoWGuid.GetNewGuidLen());
    }
}

void WorldSession::HandleMoveSplineCompleteOpcode(WorldPacket& /*recvPacket*/)
{}

void WorldSession::HandleMountSpecialAnimOpcode(WorldPacket& /*recvdata*/)
{
    CHECK_INWORLD_RETURN

    WorldPacket data(SMSG_MOUNTSPECIAL_ANIM, 8);
    data << _player->GetGUID();
    _player->SendMessageToSet(&data, true);
}

void WorldSession::HandleWorldportOpcode(WorldPacket& recvData)
{
    CHECK_INWORLD_RETURN

    uint32_t time;
    uint32_t mapid;
    float target_position_x;
    float target_position_y;
    float target_position_z;
    float target_position_o;

    recvData >> time;
    recvData >> mapid;
    recvData >> target_position_x;
    recvData >> target_position_y;
    recvData >> target_position_z;
    recvData >> target_position_o;

    if (!HasGMPermissions())
    {
        SendNotification("You do not have permission to use this function.");
        return;
    }

    LocationVector vec(target_position_x, target_position_y, target_position_z, target_position_o);
    _player->SafeTeleport(mapid, 0, vec);
}

void WorldSession::HandleTeleportToUnitOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint8 unk;
    Unit* target;
    recv_data >> unk;

    if (!HasGMPermissions())
    {
        SendNotification("You do not have permission to use this function.");
        return;
    }

    if ((target = _player->GetMapMgr()->GetUnit(_player->GetSelection())) == NULL)
        return;

    _player->SafeTeleport(_player->GetMapId(), _player->GetInstanceID(), target->GetPosition());
}

void WorldSession::HandleTeleportCheatOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    float target_position_x;
    float target_position_y;
    float target_position_z;
    float target_position_o;

    LocationVector vec;

    if (!HasGMPermissions())
    {
        SendNotification("You do not have permission to use this function.");
        return;
    }

    recv_data >> target_position_x;
    recv_data >> target_position_y;
    recv_data >> target_position_z;
    recv_data >> target_position_o;

    vec.ChangeCoords(target_position_x, target_position_y, target_position_z, target_position_o);
    _player->SafeTeleport(_player->GetMapId(), _player->GetInstanceID(), vec);
}

#if VERSION_STRING != Cata
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

    if (HasMovementFlag(MOVEFLAG_TRANSPORT))
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
    }

    if (HasMovementFlag((MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || HasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING))
    {
        data >> pitch;
    }

    data >> fall_time;

    if (HasMovementFlag(MOVEFLAG_REDIRECTED))
    {
        data >> redirectVelocity;
        data >> redirectSin;
        data >> redirectCos;
        data >> redirect2DSpeed;
    }
    if (HasMovementFlag(MOVEFLAG_SPLINE_MOVER))
    {
        data >> spline_elevation;
    }
}

void MovementInfo::write(WorldPacket& data)
{
    data << flags;
    data << flags2;
    data <<Util::getMSTime();

    data << position.x;
    data << position.y;
    data << position.z;
    data << position.o;

    if (HasMovementFlag(MOVEFLAG_TRANSPORT))
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
    }

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

    if (HasMovementFlag(MOVEFLAG_SPLINE_MOVER))
    {
        data << spline_elevation;
    }
}
#endif
