/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Packets/ManagedPacket.h"
#include "Server/Packets/CmsgSetActiveMover.h"
#include "Server/Packets/MovementPacket.h"
using namespace AscEmu::Packets;

void WorldSession::handleSetActiveMoverOpcode(WorldPacket& recvData)
{
    CHECK_INWORLD_RETURN

    CmsgSetActiveMover cmsg;
    if (!cmsg.deserialise(recvData))
        return;

    if (cmsg.guid == m_MoverWoWGuid.GetOldGuid())
        return;

    if (_player->m_CurrentCharm != cmsg.guid.GetOldGuid() || _player->GetGUID() != cmsg.guid.GetOldGuid())
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

    m_MoverWoWGuid.Init(cmsg.guid.GetOldGuid() == 0 ? _player->GetGUID() : cmsg.guid);

    // set up to the movement packet
    movement_packet[0] = m_MoverWoWGuid.GetNewGuidMask();
    memcpy(&movement_packet[1], m_MoverWoWGuid.GetNewGuid(), m_MoverWoWGuid.GetNewGuidLen());
}

#ifdef AE_TBC
void WorldSession::HandleMovementOpcodes(WorldPacket& recvData)
{
    CHECK_INWORLD_RETURN
    ASSERT(_player->mControledUnit)

    if (_player->isTransferPending() || _player->isOnTaxi() || _player->justDied())
        return;

    auto mover = _player->mControledUnit;

    MovementPacket move_packet(recvData.GetOpcode(), 0);
    if (!move_packet.deserialise(recvData))
        return;

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
        _player->SetStandState(STANDSTATE_STAND);

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
        if (recvData.GetOpcode() == MSG_MOVE_JUMP && _player->jumping)
        {
            sCheatLog.writefromsession(this, "Detected jump hacking");
            Disconnect();
            return;
        }

        if (worldConfig.antiHack.isTeleportHackCheckEnabled)
        {
            // TODO Fix for charmed units
            // TODO Fix for transports
            if (_player->GetCharmedUnitGUID() == 0)
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

    if (_player->GetEmoteState())
        _player->SetEmoteState(EMOTE_ONESHOT_NONE);

    // TODO Verify that timestamp can be replaced with AscEmu funcs
    const auto ms_time = Util::getMSTime();
    if (m_clientTimeDelay == 0)
        m_clientTimeDelay = ms_time - movement_info.time;

    MovementPacket packet(recvData.GetOpcode(), 0);
    packet.guid = mover->GetGUID();
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

    // TODO Implement below
    // End

//    /************************************************************************/
//    /* Hack Detection by Classic	                                        */
//    /************************************************************************/
//    if (!movement_info.transporter_info.guid && recvData.GetOpcode() != MSG_MOVE_JUMP && !_player->FlyCheat && !_player->flying_aura && !(movement_info.flags & MOVEFLAG_SWIMMING || movement_info.flags & MOVEFLAG_FALLING) && movement_info.position.z > _player->GetPositionZ() && movement_info.position.x == _player->GetPositionX() && movement_info.position.y == _player->GetPositionY())
//    {
//        WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 13);
//        data << _player->GetNewGUID();
//        data << uint32(5);      // unknown 0
//        SendPacket(&data);
//    }
//
//    if ((movement_info.flags & MOVEFLAG_FLYING) && !(movement_info.flags & MOVEFLAG_SWIMMING) && !(_player->flying_aura || _player->FlyCheat))
//    {
//        WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 13);
//        data << _player->GetNewGUID();
//        data << uint32(5);      // unknown 0
//        SendPacket(&data);
//    }
//
//    /************************************************************************/
//    /* Falling damage checks                                                */
//    /************************************************************************/
//
//    if (_player->blinked)
//    {
//        _player->blinked = false;
//        _player->m_fallDisabledUntil = UNIXTIME + 5;
//        _player->SpeedCheatDelay(2000);   //some say they managed to trigger system with knockback. Maybe they moved in air ?
//    }
//    else
//    {
//        if (recvData.GetOpcode() == MSG_MOVE_FALL_LAND)
//        {
//            // player has finished falling
//            //if z_axisposition contains no data then set to current position
//            if (!mover->z_axisposition)
//                mover->z_axisposition = movement_info.position.z;
//
//            // calculate distance fallen
//            uint32 falldistance = float2int32(mover->z_axisposition - movement_info.position.z);
//            if (mover->z_axisposition <= movement_info.position.z)
//                falldistance = 1;
//            /*Safe Fall*/
//            if ((int)falldistance > mover->m_safeFall)
//                falldistance -= mover->m_safeFall;
//            else
//                falldistance = 1;
//
//            //checks that player has fallen more than 12 units, otherwise no damage will be dealt
//            //falltime check is also needed here, otherwise sudden changes in Z axis position, such as using !recall, may result in death
//            if (mover->isAlive() && !mover->bInvincible && (falldistance > 12) && !mover->m_noFallDamage &&
//                ((mover->GetGUID() != _player->GetGUID()) || (!_player->GodModeCheat && (UNIXTIME >= _player->m_fallDisabledUntil))))
//            {
//                // 1.7% damage for each unit fallen on Z axis over 13
//                uint32 health_loss = static_cast<uint32>(mover->GetHealth() * (falldistance - 12) * 0.017f);
//
//                if (health_loss >= mover->GetHealth())
//                    health_loss = mover->GetHealth();
//#if VERSION_STRING > TBC
//                else if ((falldistance >= 65) && (mover->GetGUID() == _player->GetGUID()))
//                {
//                    // Rather than Updating achievement progress every time fall damage is taken, all criteria currently have 65 yard requirement...
//                    // Achievement 964: Fall 65 yards without dying.
//                    // Achievement 1260: Fall 65 yards without dying while completely smashed during the Brewfest Holiday.
//                    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING, falldistance, Player::GetDrunkenstateByValue(_player->GetDrunkValue()), 0);
//                }
//#endif
//
//                mover->SendEnvironmentalDamageLog(mover->GetGUID(), DAMAGE_FALL, health_loss);
//                mover->DealDamage(mover, health_loss, 0, 0, 0);
//
//                //_player->RemoveStealth(); // cebernic : why again? lost stealth by AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN already.
//            }
//            mover->z_axisposition = 0.0f;
//        }
//        else
//            //whilst player is not falling, continuously update Z axis position.
//            //once player lands this will be used to determine how far he fell.
//            if (!(movement_info.flags & MOVEFLAG_FALLING))
//                mover->z_axisposition = movement_info.position.z;
//    }
//
//    /************************************************************************/
//    /* Transporter Setup                                                    */
//    /************************************************************************/
//    if ((mover->obj_movement_info.transporter_info.guid != 0) && (movement_info.transporter_info.transGuid.GetOldGuid() == 0))
//    {
//        /* we left the transporter we were on */
//
//        Transporter *transporter = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(mover->obj_movement_info.transporter_info.guid));
//        if (transporter != NULL)
//            transporter->RemovePassenger(static_cast<Player*>(mover));
//
//        mover->obj_movement_info.transporter_info.guid = 0;
//        _player->SpeedCheatReset();
//
//    }
//    else
//    {
//        if (movement_info.transporter_info.transGuid.GetOldGuid() != 0)
//        {
//
//            if (mover->obj_movement_info.transporter_info.guid == 0)
//            {
//                Transporter *transporter = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(movement_info.transporter_info.transGuid));
//                if (transporter != NULL)
//                    transporter->AddPassenger(static_cast<Player*>(mover));
//
//                /* set variables */
//                mover->obj_movement_info.transporter_info.guid = movement_info.transporter_info.transGuid;
//                mover->obj_movement_info.transporter_info.time = movement_info.transporter_info.time;
//                mover->obj_movement_info.transporter_info.position.x = movement_info.transporter_info.position.x;
//                mover->obj_movement_info.transporter_info.position.y = movement_info.transporter_info.position.y;
//                mover->obj_movement_info.transporter_info.position.z = movement_info.transporter_info.position.z;
//                mover->obj_movement_info.transporter_info.position.o = movement_info.transporter_info.position.o;
//
//                mover->m_transportData.transportGuid = movement_info.transporter_info.transGuid;
//                mover->m_transportData.relativePosition.x = movement_info.transporter_info.position.x;
//                mover->m_transportData.relativePosition.y = movement_info.transporter_info.position.y;
//                mover->m_transportData.relativePosition.z = movement_info.transporter_info.position.z;
//                mover->m_transportData.relativePosition.o = movement_info.transporter_info.position.o;
//            }
//            else
//            {
//                /* no changes */
//                mover->obj_movement_info.transporter_info.time = movement_info.transporter_info.time;
//                mover->obj_movement_info.transporter_info.position.x = movement_info.transporter_info.position.x;
//                mover->obj_movement_info.transporter_info.position.y = movement_info.transporter_info.position.y;
//                mover->obj_movement_info.transporter_info.position.z = movement_info.transporter_info.position.z;
//                mover->obj_movement_info.transporter_info.position.o = movement_info.transporter_info.position.o;
//
//                mover->m_transportData.relativePosition.x = movement_info.transporter_info.position.x;
//                mover->m_transportData.relativePosition.y = movement_info.transporter_info.position.y;
//                mover->m_transportData.relativePosition.z = movement_info.transporter_info.position.z;
//                mover->m_transportData.relativePosition.o = movement_info.transporter_info.position.o;
//            }
//        }
//    }
//
//    /************************************************************************/
//    /* Breathing System                                                     */
//    /************************************************************************/
//    _HandleBreathing(movement_info, _player, this);
//
//    /************************************************************************/
//    /* Remove Spells                                                        */
//    /************************************************************************/
//    uint32 flags = 0;
//    if ((movement_info.flags & MOVEFLAG_MOTION_MASK) != 0)
//        flags |= AURA_INTERRUPT_ON_MOVEMENT;
//
//    if (!(movement_info.flags & MOVEFLAG_SWIMMING || movement_info.flags & MOVEFLAG_FALLING))
//        flags |= AURA_INTERRUPT_ON_LEAVE_WATER;
//    if (movement_info.flags & MOVEFLAG_SWIMMING)
//        flags |= AURA_INTERRUPT_ON_ENTER_WATER;
//    if ((movement_info.flags & MOVEFLAG_TURNING_MASK) || _player->isTurning)
//        flags |= AURA_INTERRUPT_ON_TURNING;
//    if (movement_info.flags & MOVEFLAG_REDIRECTED)
//        flags |= AURA_INTERRUPT_ON_JUMP;
//
//    _player->RemoveAurasByInterruptFlag(flags);
//
//    /************************************************************************/
//    /* Update our position in the server.                                   */
//    /************************************************************************/
//
//    // Player is the active mover
//    if (m_MoverWoWGuid.GetOldGuid() == _player->GetGUID())
//    {
//
//        if (!_player->GetTransport())
//        {
//            if (!_player->SetPosition(movement_info.position.x, movement_info.position.y, movement_info.position.z, movement_info.position.o))
//            {
//                //extra check to set HP to 0 only if the player is dead (KillPlayer() has already this check)
//                if (_player->isAlive())
//                {
//                    _player->setHealth(0);
//                    _player->KillPlayer();
//                }
//
//                MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(_player->GetMapId());
//                if (pMapinfo != nullptr)
//                {
//                    if (pMapinfo->type == INSTANCE_NULL || pMapinfo->type == INSTANCE_BATTLEGROUND)
//                    {
//                        _player->RepopAtGraveyard(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId());
//                    }
//                    else
//                    {
//                        _player->RepopAtGraveyard(pMapinfo->repopx, pMapinfo->repopy, pMapinfo->repopz, pMapinfo->repopmapid);
//                    }
//                }
//                else
//                {
//                    _player->RepopAtGraveyard(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId());
//                }//Teleport player to graveyard. Stops players from QQing..
//            }
//        }
//    }
//    else
//    {
//        if (!mover->isRooted())
//            mover->SetPosition(movement_info.position.x, movement_info.position.y, movement_info.position.z, movement_info.position.o);
//    }
}
#endif
