/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Packets/ManagedPacket.h"
#include "Server/Packets/CmsgSetActiveMover.h"
#include "Server/Packets/MovementPacket.h"
#include "Server/Packets/SmsgMoveUnsetCanFly.h"
#include "Spell/Definitions/AuraInterruptFlags.h"
#include "Server/WorldSession.h"
#include "Units/Players/Player.h"
#include "Units/Creatures/Vehicle.h"
#include "Map/CellHandlerDefines.hpp"
#include "Objects/GameObject.h"
#include "Server/MainServerDefines.h"
#include "Server/Warden/SpeedDetector.h"
#include "Objects/ObjectMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Packets/CmsgWorldTeleport.h"
#include "Server/Packets/SmsgMountspecialAnim.h"
#include "Server/Packets/MsgMoveTeleportAck.h"
#include "Server/Packets/SmsgNewWorld.h"
#include "Units/Creatures/Pet.h"

using namespace AscEmu::Packets;

#if VERSION_STRING < Cata
void WorldSession::handleSetActiveMoverOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgSetActiveMover srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.guid == m_MoverWoWGuid.GetOldGuid())
        return;

    if (_player->m_CurrentCharm != srlPacket.guid.GetOldGuid() || _player->getGuid() != srlPacket.guid.GetOldGuid())
    {
        auto bad_packet = true;
#if VERSION_STRING >= TBC
        if (const auto vehicle = _player->getCurrentVehicle())
            if (const auto owner = vehicle->GetOwner())
                if (owner->getGuid() == srlPacket.guid.GetOldGuid())
                    bad_packet = false;
#endif
        if (bad_packet)
            return;
    }

    if (srlPacket.guid.GetOldGuid() == 0)
        m_MoverWoWGuid.Init(_player->getGuid());
    else
        m_MoverWoWGuid = srlPacket.guid;

    // set up to the movement packet
    movement_packet[0] = m_MoverWoWGuid.GetNewGuidMask();
    memcpy(&movement_packet[1], m_MoverWoWGuid.GetNewGuid(), m_MoverWoWGuid.GetNewGuidLen());
}
#endif

#if VERSION_STRING <= WotLK
void _HandleBreathing(MovementInfo & movement_info, Player* _player, WorldSession* pSession)
{
    // no water breathing is required
    if (!worldConfig.server.enableBreathing || _player->m_cheats.FlyCheat || _player->m_bUnlimitedBreath || !_player->isAlive() || _player->m_cheats.GodModeCheat)
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

#if VERSION_STRING <= TBC
void WorldSession::handleMovementOpcodes(WorldPacket& recvData)
{
    CHECK_INWORLD_RETURN
    ASSERT(_player->mControledUnit)

    if (_player->isTransferPending() || _player->isOnTaxi() || _player->justDied())
        return;

    auto mover = _player->mControledUnit;

    MovementPacket move_packet(recvData.GetOpcode(), 0);
    move_packet.deserialise(recvData);
    /*if (!move_packet.deserialise(recvData))
        return;*/

    movement_info = move_packet.info;

    auto out_of_bounds = false;
    out_of_bounds = out_of_bounds || movement_info.position.y < _minY;
    out_of_bounds = out_of_bounds || movement_info.position.y > _maxY;
    out_of_bounds = out_of_bounds || movement_info.position.x > _maxX;
    out_of_bounds = out_of_bounds || movement_info.position.x > _maxX;

    if (out_of_bounds)
    {
        Disconnect();
        return;
    }

    if (auto summoned_object = _player->m_SummonedObject)
    {
         if (summoned_object->isGameObject())
         {
             const auto go = static_cast<GameObject*>(summoned_object);
             if (go->isFishingNode())
             {
                 auto fishing_node = static_cast<GameObject_FishingNode*>(go);
                 fishing_node->EndFishing(true);

                 // This is done separately as not all channeled spells are canceled by all movement opcodes
                 if (auto spell = _player->getCurrentSpell(CURRENT_CHANNELED_SPELL))
                 {
                     spell->SendChannelUpdate(0U);
                     spell->finish(false);
                 }
             }
         }
    }

    if (recvData.GetOpcode() == MSG_MOVE_START_FORWARD)
        _player->setStandState(STANDSTATE_STAND);

    auto moved = true;
    switch (recvData.GetOpcode())
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

    if (moved)
        _player->m_isMoving = _player->moving || _player->strafing || _player->jumping;

    _player->isTurning = _player->GetOrientation() != movement_info.position.o;

    // Antihack Checks
    if (!(HasGMPermissions() && worldConfig.antiHack.isAntiHackCheckDisabledForGm))
    {
        // Prevent multi-jump cheat
        // TODO Account for falltime and jump flags
        // TODO Check this in the correct place
        /*if (recvData.GetOpcode() == MSG_MOVE_JUMP && _player->jumping)
        {
            sCheatLog.writefromsession(this, "Detected jump hacking");
            Disconnect();
            return;
        }*/

        if (worldConfig.antiHack.isTeleportHackCheckEnabled)
        {
            // TODO Fix for charmed units
            // TODO Fix for transports
            if (_player->getCharmGuid() == 0)
            {
                if (_player->m_position.Distance2DSq(movement_info.position) > 3025.f)
                {
                    if (_player->getSpeedForType(TYPE_RUN) < 50.f && !_player->obj_movement_info.isOnTransport())
                    {
                        sCheatLog.writefromsession(this, "Disconnected for teleport hacking. Player speed: %f, Distance traveled: %f", _player->getSpeedForType(TYPE_RUN), sqrt(_player->m_position.Distance2DSq({ movement_info.position.x, movement_info.position.y })));
                        Disconnect();
                        return;
                    }
                }
            }
        }

        if (worldConfig.antiHack.isSpeedHackCkeckEnabled)
        {
            if (!(_player->isOnTaxi() || _player->movement_info.isOnTransport()))
            {
                _player->SDetector->addSample(movement_info.position, Util::getMSTime(), _player->getFastestSpeed());

                if (_player->SDetector->IsCheatDetected())
                    _player->SDetector->ReportCheater(_player);
            }
        }
    }

    if (_player->getEmoteState())
        _player->setEmoteState(EMOTE_ONESHOT_NONE);

    // TODO Verify that timestamp can be replaced with AscEmu funcs
    const auto ms_time = Util::getMSTime();
    if (m_clientTimeDelay == 0)
        m_clientTimeDelay = ms_time - movement_info.time;

    MovementPacket packet(recvData.GetOpcode(), 0);
    packet.guid = mover->getGuid();
    packet.info = movement_info;

    if (_player->getInRangePlayersCount())
    {
        const auto move_time = packet.info.time - (ms_time - m_clientTimeDelay) + 500 + ms_time;
        for (const auto& obj : mover->getInRangePlayersSet())
        {
            ARCEMU_ASSERT(obj->isPlayer());
            packet.info.time = move_time + obj->asPlayer()->GetSession()->m_moveDelayTime;
            obj->asPlayer()->SendPacket(packet.serialise().get());
        }
    }

    // Remove flying aura if needed
    // TODO: This seems like a daft check, verify it
    if (!movement_info.isOnTransport() && recvData.GetOpcode() != MSG_MOVE_JUMP && !_player->m_cheats.FlyCheat && !_player->flying_aura)
        if (!movement_info.isSwimming() && !movement_info.isFalling())
            if (movement_info.position.z > _player->GetPositionZ() && movement_info.position.x == _player->GetPositionX() && movement_info.position.y == _player->GetPositionY())
                SendPacket(SmsgMoveUnsetCanFly(_player->getGuid()).serialise().get());

    if (movement_info.isFlying() && !movement_info.isSwimming() && !(_player->flying_aura || _player->m_cheats.FlyCheat))
        SendPacket(SmsgMoveUnsetCanFly(_player->getGuid()).serialise().get());

    if (_player->blinked)
    {
        _player->blinked = false;
        _player->m_fallDisabledUntil = UNIXTIME + 5;
        _player->SpeedCheatDelay(2000);
    }
    else
    {
        if (recvData.GetOpcode() != MSG_MOVE_FALL_LAND)
        {
            // Update Z while player is falling to calculate fall damage
            if (!movement_info.isFalling())
            {
                mover->z_axisposition = movement_info.position.z;
            }
            else
            {
                // Can't fall upwards - no fall dmg exploit
                // TODO Verify infrastructure for this check
                /*if (movement_info.position.z > mover->z_axisposition)
                {
                    Disconnect();
                    return;
                }*/

            }
        }
        else
        {
            if (!mover->z_axisposition)
                mover->z_axisposition = movement_info.position.z;

            auto fall_distance = float2int32(mover->z_axisposition - movement_info.position.z);
            // If player ended up going up, negate
            // TODO Anticheat checks
            if (mover->z_axisposition <= movement_info.position.z)
                fall_distance = 1;

            if (fall_distance > mover->m_safeFall)
                fall_distance-=  mover->m_safeFall;
            else
                fall_distance = 1;

            if (mover->isAlive() && !mover->bInvincible && fall_distance > 12 && !mover->m_noFallDamage && (mover->
                getGuid() != _player->getGuid() || !_player->m_cheats.GodModeCheat && UNIXTIME >= _player->m_fallDisabledUntil))
            {
                auto health_lost = static_cast<uint32_t>(mover->getHealth() * (fall_distance - 12) * 0.017f);
                if (health_lost >= mover->getHealth())
                {
                    health_lost = mover->getHealth();
                }
#ifdef FT_ACHIEVEMENTS
                else if (fall_distance >= 65 && mover->getGuid() == _player->getGuid())
                {
                    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING, fall_distance, Player::GetDrunkenstateByValue(_player->GetDrunkValue()), 0);
                }
#endif
                mover->sendEnvironmentalDamageLogPacket(mover->getGuid(), DAMAGE_FALL, health_lost);
                mover->DealDamage(mover, health_lost, 0, 0, 0);
            }

            mover->z_axisposition = 0.f;
        }
    }

    // End

    if (mover->obj_movement_info.transport_data.transportGuid != 0 && movement_info.transport_data.transportGuid == 0)
    {
        // Leaving transport we were on
        if (auto transporter = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(mover->obj_movement_info.transport_data.transportGuid)))
            transporter->RemovePassenger(static_cast<Player*>(mover));

        mover->obj_movement_info.transport_data.transportGuid = 0;
        _player->SpeedCheatReset();
    }
    else
    {
        if (movement_info.transport_data.transportGuid != 0)
        {

            if (mover->obj_movement_info.transport_data.transportGuid == 0)
            {
                Transporter *transporter = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(movement_info.transport_data.transportGuid));
                if (transporter != NULL)
                    transporter->AddPassenger(static_cast<Player*>(mover));

                /* set variables */
                mover->obj_movement_info.transport_data.transportGuid = movement_info.transport_data.transportGuid;
                mover->obj_movement_info.transport_time = movement_info.transport_time;
                mover->obj_movement_info.transport_data.relativePosition.x = movement_info.transport_data.relativePosition.x;
                mover->obj_movement_info.transport_data.relativePosition.y = movement_info.transport_data.relativePosition.y;
                mover->obj_movement_info.transport_data.relativePosition.z = movement_info.transport_data.relativePosition.z;
                mover->obj_movement_info.transport_data.relativePosition.o = movement_info.transport_data.relativePosition.o;

                mover->m_transportData.transportGuid = movement_info.transport_data.transportGuid;
                mover->m_transportData.relativePosition.x = movement_info.transport_data.relativePosition.x;
                mover->m_transportData.relativePosition.y = movement_info.transport_data.relativePosition.y;
                mover->m_transportData.relativePosition.z = movement_info.transport_data.relativePosition.z;
                mover->m_transportData.relativePosition.o = movement_info.transport_data.relativePosition.o;
            }
            else
            {
                /* no changes */
                mover->obj_movement_info.transport_time = movement_info.transport_time;
                mover->obj_movement_info.transport_data.relativePosition.x = movement_info.transport_data.relativePosition.x;
                mover->obj_movement_info.transport_data.relativePosition.y = movement_info.transport_data.relativePosition.y;
                mover->obj_movement_info.transport_data.relativePosition.z = movement_info.transport_data.relativePosition.z;
                mover->obj_movement_info.transport_data.relativePosition.o = movement_info.transport_data.relativePosition.o;

                mover->m_transportData.relativePosition.x = movement_info.transport_data.relativePosition.x;
                mover->m_transportData.relativePosition.y = movement_info.transport_data.relativePosition.y;
                mover->m_transportData.relativePosition.z = movement_info.transport_data.relativePosition.z;
                mover->m_transportData.relativePosition.o = movement_info.transport_data.relativePosition.o;
            }
        }
    }

    /************************************************************************/
    /* Breathing System                                                     */
    /************************************************************************/
    // TODO Restructure and implement
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
    if (m_MoverWoWGuid.GetOldGuid() == _player->getGuid())
    {
        if (!_player->GetTransport())
        {
            if (!_player->SetPosition(movement_info.position.x, movement_info.position.y, movement_info.position.z, movement_info.position.o))
            {
                //extra check to set HP to 0 only if the player is dead (KillPlayer() has already this check)
                if (_player->isAlive())
                {
                    _player->setHealth(0);
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

#if VERSION_STRING == WotLK

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

void WorldSession::handleMovementOpcodes(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

        bool moved = true;

    if (/*_player->getCharmedByGuid() || */_player->GetPlayerStatus() == TRANSFER_PENDING || _player->isOnTaxi() || _player->getDeathState() == JUST_DIED)
        return;

    // spell cancel on movement, for now only fishing is added
    Object* t_go = _player->m_SummonedObject;
    if (t_go != nullptr)
    {
        if (t_go->isGameObject())
        {
            GameObject* go = static_cast<GameObject*>(t_go);
            if (go->getGoType() == GAMEOBJECT_TYPE_FISHINGNODE)
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
    if (mover->isPlayer())
        plrMover = dynamic_cast<Player*>(mover);

    /************************************************************************/
    /* Clear standing state to stand.				                        */
    /************************************************************************/
    if (recvPacket.GetOpcode() == MSG_MOVE_START_FORWARD)
        _player->setStandState(STANDSTATE_STAND);

    /************************************************************************/
    /* Make sure the packet is the correct size range.                      */
    /************************************************************************/
    //if (recvPacket.size() > 80) { Disconnect(); return; }

    /************************************************************************/
    /* Read Movement Data Packet                                            */
    /************************************************************************/
    /*AscEmu::Packets::MovementPacket packet(recvPacket.GetOpcode(), 0);
    packet.guid = mover->getGuid();
    packet.info = movement_info;*/

    AscEmu::Packets::MovementPacket packet;
    if (!packet.deserialise(recvPacket))
        return;

    movement_info = packet.info;

    if (packet.guid != mover->getGuid())
        return;

    /*WoWGuid guid;
    recvPacket >> guid;
    movement_info.init(recvPacket);

    if (guid != mover->getGuid())
        return;*/

        /* Anti Multi-Jump Check */
    if (recvPacket.GetOpcode() == MSG_MOVE_JUMP && _player->jumping == true && !GetPermissionCount())
    {
        sCheatLog.writefromsession(this, "Detected jump hacking");
        Disconnect();
        return;
    }
    if (recvPacket.GetOpcode() == MSG_MOVE_FALL_LAND || movement_info.flags & MOVEFLAG_SWIMMING)
        _player->jumping = false;
    if (!_player->jumping && (recvPacket.GetOpcode() == MSG_MOVE_JUMP || movement_info.flags & MOVEFLAG_FALLING))
        _player->jumping = true;

    /************************************************************************/
    /* Update player movement state                                         */
    /************************************************************************/

    uint16 opcode = recvPacket.GetOpcode();
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


    if (!(HasGMPermissions() && worldConfig.antiHack.isAntiHackCheckDisabledForGm) && !_player->getCharmGuid())
    {
        /************************************************************************/
        /* Anti-Teleport                                                        */
        /************************************************************************/
        if (worldConfig.antiHack.isTeleportHackCheckEnabled && _player->m_position.Distance2DSq({ movement_info.position.x, movement_info.position.y }) > 3025.0f
            && _player->getSpeedForType(TYPE_RUN) < 50.0f && !_player->obj_movement_info.transport_data.transportGuid)
        {
            sCheatLog.writefromsession(this, "Disconnected for teleport hacking. Player speed: %f, Distance traveled: %f", _player->getSpeedForType(TYPE_RUN), sqrt(_player->m_position.Distance2DSq({ movement_info.position.x, movement_info.position.y })));
            Disconnect();
            return;
        }
    }

    //update the detector
    if (worldConfig.antiHack.isSpeedHackCkeckEnabled && !_player->isOnTaxi() && _player->obj_movement_info.transport_data.transportGuid == 0 && !_player->GetSession()->GetPermissionCount())
    {
        // simplified: just take the fastest speed. less chance of fuckups too
        float speed = (_player->flying_aura) ? _player->getSpeedForType(TYPE_FLY) : (_player->getSpeedForType(TYPE_SWIM) > _player->getSpeedForType(TYPE_RUN)) ? _player->getSpeedForType(TYPE_SWIM) : _player->getSpeedForType(TYPE_RUN);

        _player->SDetector->AddSample(movement_info.position.x, movement_info.position.y, Util::getMSTime(), speed);

        if (_player->SDetector->IsCheatDetected())
            _player->SDetector->ReportCheater(_player);
    }

    /************************************************************************/
    /* Remove Emote State                                                   */
    /************************************************************************/
    if (_player->getEmoteState())
        _player->setEmoteState(EMOTE_ONESHOT_NONE);

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
            LOG_DEBUG("Bit %u (0x%.8X or %u) is set!", b, z, z);

        z <<= 1;
        b += 1;
    }
    LOG_DEBUG("=========================================================");
#endif

    /************************************************************************/
    /* Orientation dumping                                                  */
    /************************************************************************/
#if 0
    LOG_DEBUG("Packet: 0x%03X (%s)", recvPacket.GetOpcode(), getOpcodeName(recvPacket.GetOpcode()).c_str());
    LOG_DEBUG("Orientation: %.10f", movement_info.position.o);
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
        memcpy(&movement_packet[0], recvPacket.contents(), recvPacket.size());
        movement_packet[pos + 6] = 0;

        /************************************************************************/
        /* Distribute to all inrange players.                                   */
        /************************************************************************/
        for (const auto& itr : mover->getInRangePlayersSet())
        {
            Player* p = static_cast<Player*>(itr);

            *(uint32*)&movement_packet[pos + 6] = uint32(move_time + p->GetSession()->m_moveDelayTime);

            p->GetSession()->OutPacket(recvPacket.GetOpcode(), uint16(recvPacket.size() + pos), movement_packet);
        }
    }

    /************************************************************************/
    /* Hack Detection by Classic	                                        */
    /************************************************************************/
    if (!movement_info.transport_data.transportGuid && recvPacket.GetOpcode() != MSG_MOVE_JUMP && !_player->m_cheats.FlyCheat && !_player->flying_aura && !(movement_info.flags & MOVEFLAG_SWIMMING || movement_info.flags & MOVEFLAG_FALLING) && movement_info.position.z > _player->GetPositionZ() && movement_info.position.x == _player->GetPositionX() && movement_info.position.y == _player->GetPositionY())
    {
        WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 13);
        data << _player->GetNewGUID();
        data << uint32(5);      // unknown 0
        SendPacket(&data);
    }

    if ((movement_info.flags & MOVEFLAG_FLYING) && !(movement_info.flags & MOVEFLAG_SWIMMING) && !(_player->flying_aura || _player->m_cheats.FlyCheat))
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
        if (recvPacket.GetOpcode() == MSG_MOVE_FALL_LAND)
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
                ((mover->getGuid() != _player->getGuid()) || (!_player->m_cheats.GodModeCheat && (UNIXTIME >= _player->m_fallDisabledUntil))))
            {
                // 1.7% damage for each unit fallen on Z axis over 13
                uint32 health_loss = static_cast<uint32>(mover->getHealth() * (falldistance - 12) * 0.017f);

                if (health_loss >= mover->getHealth())
                    health_loss = mover->getHealth();
#if VERSION_STRING > TBC
                else if ((falldistance >= 65) && (mover->getGuid() == _player->getGuid()))
                {
                    // Rather than Updating achievement progress every time fall damage is taken, all criteria currently have 65 yard requirement...
                    // Achievement 964: Fall 65 yards without dying.
                    // Achievement 1260: Fall 65 yards without dying while completely smashed during the Brewfest Holiday.
                    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING, falldistance, Player::GetDrunkenstateByValue(_player->GetDrunkValue()), 0);
                }
#endif

                mover->sendEnvironmentalDamageLogPacket(mover->getGuid(), DAMAGE_FALL, health_loss);
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
    if ((mover->obj_movement_info.transport_data.transportGuid != 0) && (movement_info.transport_data.transportGuid == 0))
    {
        /* we left the transporter we were on */

        Transporter *transporter = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(mover->obj_movement_info.transport_data.transportGuid));
        if (transporter != NULL)
            transporter->RemovePassenger(static_cast<Player*>(mover));

        mover->obj_movement_info.transport_data.transportGuid = 0;
        _player->SpeedCheatReset();

    }
    else
    {
        if (movement_info.transport_data.transportGuid != 0)
        {

            if (mover->obj_movement_info.transport_data.transportGuid == 0)
            {
                Transporter *transporter = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(movement_info.transport_data.transportGuid));
                if (transporter != NULL)
                    transporter->AddPassenger(static_cast<Player*>(mover));

                /* set variables */
                mover->obj_movement_info.transport_data.transportGuid = movement_info.transport_data.transportGuid;
                mover->obj_movement_info.transport_time = movement_info.transport_time;
                mover->obj_movement_info.transport_data.relativePosition.x = movement_info.transport_data.relativePosition.x;
                mover->obj_movement_info.transport_data.relativePosition.y = movement_info.transport_data.relativePosition.y;
                mover->obj_movement_info.transport_data.relativePosition.z = movement_info.transport_data.relativePosition.z;
                mover->obj_movement_info.transport_data.relativePosition.o = movement_info.transport_data.relativePosition.o;

                mover->m_transportData.transportGuid = movement_info.transport_data.transportGuid;
                mover->m_transportData.relativePosition.x = movement_info.transport_data.relativePosition.x;
                mover->m_transportData.relativePosition.y = movement_info.transport_data.relativePosition.y;
                mover->m_transportData.relativePosition.z = movement_info.transport_data.relativePosition.z;
                mover->m_transportData.relativePosition.o = movement_info.transport_data.relativePosition.o;
            }
            else
            {
                /* no changes */
                mover->obj_movement_info.transport_time = movement_info.transport_time;
                mover->obj_movement_info.transport_data.relativePosition.x = movement_info.transport_data.relativePosition.x;
                mover->obj_movement_info.transport_data.relativePosition.y = movement_info.transport_data.relativePosition.y;
                mover->obj_movement_info.transport_data.relativePosition.z = movement_info.transport_data.relativePosition.z;
                mover->obj_movement_info.transport_data.relativePosition.o = movement_info.transport_data.relativePosition.o;

                mover->m_transportData.relativePosition.x = movement_info.transport_data.relativePosition.x;
                mover->m_transportData.relativePosition.y = movement_info.transport_data.relativePosition.y;
                mover->m_transportData.relativePosition.z = movement_info.transport_data.relativePosition.z;
                mover->m_transportData.relativePosition.o = movement_info.transport_data.relativePosition.o;
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
    if (m_MoverWoWGuid.GetOldGuid() == _player->getGuid())
    {

        if (!_player->GetTransport())
        {
            if (!_player->SetPosition(movement_info.position.x, movement_info.position.y, movement_info.position.z, movement_info.position.o))
            {
                //extra check to set HP to 0 only if the player is dead (KillPlayer() has already this check)
                if (_player->isAlive())
                {
                    _player->setHealth(0);
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

#if VERSION_STRING >= Cata
void WorldSession::handleMovementOpcodes(WorldPacket& recvPacket)
{
    uint16_t opcode = recvPacket.GetOpcode();
    Player* mover = _player;

    if (m_MoverGuid != mover->getGuid())
        return;

    if (mover->getCharmedByGuid() || !mover->IsInWorld() || mover->GetPlayerStatus() == TRANSFER_PENDING || mover->isOnTaxi())
    {
        return;
    }

    /************************************************************************/
    /* Clear standing state to stand.				                        */
    /************************************************************************/
    if (opcode == MSG_MOVE_START_FORWARD)
        mover->setStandState(STANDSTATE_STAND);

    //extract packet
    MovementInfo movementInfo;
    recvPacket >> movementInfo;

    //    /************************************************************************/
    //    /* Make sure the packet is the correct size range.                      */
    //    /************************************************************************/
    //    //if (recvPacket.size() > 80) { Disconnect(); return; }
    //
    //    /************************************************************************/
    //    /* Read Movement Data Packet                                            */
    //    /************************************************************************/
    //    WoWGuid guid;
    //    recvPacket >> guid;
    //    movement_info.init(recvPacket);
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
    //    if (recvPacket.GetOpcode() == MSG_MOVE_JUMP && _player->jumping == true && !GetPermissionCount())
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
    if (_player->getEmoteState())
        _player->setEmoteState(EMOTE_ONESHOT_NONE);

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
        //    memcpy(&movement_packet[0], recvPacket.contents(), recvPacket.size());
        //    movement_packet[pos + 6] = 0;

        //    /************************************************************************/
        //    /* Distribute to all inrange players.                                   */
        //    /************************************************************************/
        //    for (std::set<Object*>::iterator itr = _player->m_inRangePlayers.begin(); itr != _player->m_inRangePlayers.end(); ++itr)
        //    {

        //        Player* p = static_cast< Player* >((*itr));

        //        *(uint32*)&movement_packet[pos + 6] = uint32(move_time + p->GetSession()->m_moveDelayTime);

        //        p->GetSession()->OutPacket(recvPacket.GetOpcode(), uint16(recvPacket.size() + pos), movement_packet);

        //    }
        //}

        ///************************************************************************/
        ///* Hack Detection by Classic	                                        */
        ///************************************************************************/
        //if (!movement_info.transporter_info.guid && recvPacket.GetOpcode() != MSG_MOVE_JUMP && !_player->FlyCheat && !_player->flying_aura && !(movement_info.flags & MOVEFLAG_SWIMMING || movement_info.flags & MOVEFLAG_FALLING) && movement_info.position.z > _player->GetPositionZ() && movement_info.position.x == _player->GetPositionX() && movement_info.position.y == _player->GetPositionY())
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
        //    if (recvPacket.GetOpcode() == MSG_MOVE_FALL_LAND)
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

        //            mover->sendEnvironmentalDamageLogPacket(mover->GetGUID(), DAMAGE_FALL, health_loss);
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

        //    Transporter *transporter = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(mover->obj_movement_info.transporter_info.guid));
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
        //            Transporter *transporter = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(movement_info.transporter_info.transGuid));
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

    WorldPacket data(SMSG_PLAYER_MOVE, recvPacket.size());
    data << movementInfo;
    mover->SendMessageToSet(&data, false);
}
#endif

void WorldSession::handleAcknowledgementOpcodes(WorldPacket& recvPacket)
{
    LogDebugFlag(LF_OPCODE, "Opcode %s (%u) received. This opcode is not known/implemented right now!",
        getOpcodeName(recvPacket.GetOpcode()).c_str(), recvPacket.GetOpcode());

    recvPacket.rfinish();
}

void WorldSession::handleWorldTeleportOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgWorldTeleport srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (!HasGMPermissions())
    {
        SendNotification("You do not have permission to use this function.");
        return;
    }

    _player->SafeTeleport(srlPacket.mapId, 0, srlPacket.location);
}

void WorldSession::handleMountSpecialAnimOpcode(WorldPacket& /*recvPacket*/)
{
    CHECK_INWORLD_RETURN

    _player->SendMessageToSet(SmsgMountspecialAnim(_player->getGuid()).serialise().get(), true);
}

void WorldSession::handleMoveWorldportAckOpcode(WorldPacket& /*recvPacket*/)
{
    _player->SetPlayerStatus(NONE);
    if (_player->IsInWorld())
        return;

    LogDebugFlag(LF_OPCODE, "Received MSG_MOVE_WORLDPORT_ACK");

    if (_player->GetTransport() && _player->GetMapId() != _player->GetTransport()->GetMapId())
    {
        const auto transporter = _player->GetTransport();

        const float transportPositionX = transporter->GetPositionX() + _player->GetTransPositionX();
        const float transportPositionY = transporter->GetPositionY() + _player->GetTransPositionY();
        const float transportPositionZ = transporter->GetPositionZ() + _player->GetTransPositionZ();

        const auto positionOnTransport = LocationVector(transportPositionX, transportPositionY, transportPositionZ, _player->GetOrientation());

        _player->SetMapId(transporter->GetMapId());
        _player->SetPosition(transportPositionX, transportPositionY, transportPositionZ, _player->GetOrientation());

        SendPacket(SmsgNewWorld(transporter->GetMapId(), positionOnTransport).serialise().get());
    }
    else
    {
        _player->m_TeleportState = 2;
        _player->AddToWorld();
    }
}

void WorldSession::handleMoveTeleportAckOpcode(WorldPacket& recvPacket)
{
    MsgMoveTeleportAck srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received MSG_MOVE_TELEPORT_ACK.");

    if (srlPacket.guid.GetOldGuid() == _player->getGuid())
    {
        if (worldConfig.antiHack.isTeleportHackCheckEnabled && !(HasGMPermissions() && worldConfig.antiHack.isAntiHackCheckDisabledForGm))
        {
            if (_player->GetPlayerStatus() != TRANSFER_PENDING)
            {
                sCheatLog.writefromsession(this, "Used Teleporthack 1, disconnecting.");
                Disconnect();
                return;
            }

            if (_player->m_position.Distance2DSq(_player->m_sentTeleportPosition) > 625.0f)
            {
                sCheatLog.writefromsession(this, "Used Teleporthack 2, disconnecting.");
                Disconnect();
                return;
            }
        }

        _player->SetPlayerStatus(NONE);
        _player->SpeedCheatReset();

        for (auto summon : _player->GetSummons())
            summon->SetPosition(_player->GetPositionX() + 2, _player->GetPositionY() + 2, _player->GetPositionZ(), M_PI_FLOAT);

        if (_player->m_sentTeleportPosition.x != 999999.0f)
        {
            _player->m_position = _player->m_sentTeleportPosition;
            _player->m_sentTeleportPosition.ChangeCoords({ 999999.0f, 999999.0f, 999999.0f });
        }
    }
}

void WorldSession::handleMoveNotActiveMoverOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    WoWGuid guid;
    recvPacket >> guid;

    if (guid == m_MoverWoWGuid)
        return;

    if (guid != uint64_t(0) && guid == _player->getCharmGuid())
        m_MoverWoWGuid = guid;
    else
        m_MoverWoWGuid.Init(_player->getGuid());

    movement_packet[0] = m_MoverWoWGuid.GetNewGuidMask();
    memcpy(&movement_packet[1], m_MoverWoWGuid.GetNewGuid(), m_MoverWoWGuid.GetNewGuidLen());
}
