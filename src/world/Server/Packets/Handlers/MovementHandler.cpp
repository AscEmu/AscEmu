/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
using namespace AscEmu::Packets;

#if VERSION_STRING != Cata
void WorldSession::handleSetActiveMoverOpcode(WorldPacket& recvData)
{
    CHECK_INWORLD_RETURN

    CmsgSetActiveMover cmsg;
    if (!cmsg.deserialise(recvData))
        return;

    if (cmsg.guid == m_MoverWoWGuid.GetOldGuid())
        return;

    if (_player->m_CurrentCharm != cmsg.guid.GetOldGuid() || _player->getGuid() != cmsg.guid.GetOldGuid())
    {
        auto bad_packet = true;
#if VERSION_STRING >= TBC
        if (const auto vehicle = _player->GetCurrentVehicle())
            if (const auto owner = vehicle->GetOwner())
                if (owner->getGuid() == cmsg.guid.GetOldGuid())
                    bad_packet = false;
#endif
        if (bad_packet)
            return;
    }

    if (cmsg.guid.GetOldGuid() == 0)
        m_MoverWoWGuid.Init(_player->getGuid());
    else
        m_MoverWoWGuid = cmsg.guid;

    // set up to the movement packet
    movement_packet[0] = m_MoverWoWGuid.GetNewGuidMask();
    memcpy(&movement_packet[1], m_MoverWoWGuid.GetNewGuid(), m_MoverWoWGuid.GetNewGuidLen());
}
#endif

#ifdef AE_TBC
void WorldSession::HandleMovementOpcodes(WorldPacket& recvData)
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
         if (summoned_object->IsGameObject())
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
                        sCheatLog.writefromsession(this, "Disconnected for teleport hacking. Player speed: %f, Distance traveled: %f", _player->getSpeedForType(TYPE_RUN), sqrt(_player->m_position.Distance2DSq(movement_info.position.x, movement_info.position.y)));
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
            ARCEMU_ASSERT(obj->IsPlayer());
            packet.info.time = move_time + obj->asPlayer()->GetSession()->m_moveDelayTime;
            obj->asPlayer()->SendPacket(packet.serialise().get());
        }
    }

    // Remove flying aura if needed
    // TODO: This seems like a daft check, verify it
    if (!movement_info.isOnTransport() && recvData.GetOpcode() != MSG_MOVE_JUMP && !_player->FlyCheat && !_player->flying_aura)
        if (!movement_info.isSwimming() && !movement_info.isFalling())
            if (movement_info.position.z > _player->GetPositionZ() && movement_info.position.x == _player->GetPositionX() && movement_info.position.y == _player->GetPositionY())
                SendPacket(SmsgMoveUnsetCanFly(_player->getGuid()).serialise().get());

    if (movement_info.isFlying() && !movement_info.isSwimming() && !(_player->flying_aura || _player->FlyCheat))
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
                getGuid() != _player->getGuid() || !_player->GodModeCheat && UNIXTIME >= _player->m_fallDisabledUntil))
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
                mover->SendEnvironmentalDamageLog(mover->getGuid(), DAMAGE_FALL, health_lost);
                mover->DealDamage(mover, health_lost, 0, 0, 0);
            }

            mover->z_axisposition = 0.f;
        }
    }

    // End

    if (mover->obj_movement_info.transport_data.transportGuid != 0 && movement_info.transport_data.transportGuid == 0)
    {
        // Leaving transport we were on
        if (auto transporter = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(mover->obj_movement_info.transport_data.transportGuid)))
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
                Transporter *transporter = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(movement_info.transport_data.transportGuid));
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
    //_HandleBreathing(movement_info, _player, this);

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
