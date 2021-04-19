/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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
#include "Server/OpcodeTable.hpp"

using namespace AscEmu::Packets;

#if VERSION_STRING < Cata
void WorldSession::handleSetActiveMoverOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgSetActiveMover srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.guid == m_MoverWoWGuid.getRawGuid())
        return;

#if VERSION_STRING > TBC
    if (_player->getCharmGuid() != srlPacket.guid.getRawGuid() || _player->getGuid() != srlPacket.guid.getRawGuid())
    {
        auto bad_packet = true;
        if (const auto vehicle = _player->getCurrentVehicle())
            if (const auto owner = vehicle->GetOwner())
                if (owner->getGuid() == srlPacket.guid.getRawGuid())
                    bad_packet = false;

        if (bad_packet)
            return;
    }
#endif

    if (srlPacket.guid.getRawGuid() == 0)
        m_MoverWoWGuid.Init(_player->getGuid());
    else
        m_MoverWoWGuid = srlPacket.guid;
}
#endif

void WorldSession::updatePlayerMovementVars(uint16_t opcode)
{
    if (opcode == MSG_MOVE_FALL_LAND || sessionMovementInfo.flags & MOVEFLAG_SWIMMING)
        _player->m_isJumping = false;

    if (!_player->m_isJumping && (opcode == MSG_MOVE_JUMP || sessionMovementInfo.flags & MOVEFLAG_FALLING))
        _player->m_isJumping = true;

    auto moved = true;
    switch (opcode)
    {
        case MSG_MOVE_START_FORWARD:
        case MSG_MOVE_START_BACKWARD:
            _player->m_isMovingFB = true;
            break;
        case MSG_MOVE_START_STRAFE_LEFT:
        case MSG_MOVE_START_STRAFE_RIGHT:
            _player->m_isStrafing = true;
            break;
        case MSG_MOVE_JUMP:
            _player->m_isJumping = true;
            break;
        case MSG_MOVE_STOP:
            _player->m_isMovingFB = false;
            break;
        case MSG_MOVE_STOP_STRAFE:
            _player->m_isStrafing = false;
            break;
        case MSG_MOVE_FALL_LAND:
            _player->m_isJumping = false;
            break;

        default:
            moved = false;
            break;
    }

    if (moved)
        _player->m_isMoving = _player->m_isMovingFB || _player->m_isStrafing || _player->m_isJumping;

    _player->m_isTurning = _player->GetOrientation() != sessionMovementInfo.position.o;
}

bool WorldSession::isHackDetectedInMovementData(uint16_t opcode)
{
    // NOTE: All this stuff is probably broken. However, if you want to implement hack detection, here is the place for it ;)
    // We have activated gm mode - we can hack as much as we want ;)
    // Zyres: it is not helpfull to check for permission count right now
    if (_player->hasPlayerFlags(PLAYER_FLAG_GM) && worldConfig.antiHack.isAntiHackCheckDisabledForGm)
        return false;

    // Double Jump
    if (opcode == MSG_MOVE_JUMP && _player->m_isJumping)
    {
        sCheatLog.writefromsession(this, "Detected jump hacking");
        return true;
    }

    // Teleport
    // implement worldConfig.antiHack.isTeleportHackCheckEnabled
    if (_player->m_position.Distance2DSq({ sessionMovementInfo.position.x, sessionMovementInfo.position.y }) > 3025.0f &&
        _player->getSpeedRate(TYPE_RUN, true) < 50.0f && !_player->obj_movement_info.transport_guid)
    {
        sCheatLog.writefromsession(this, "Disconnected for teleport hacking. Player speed: %f, Distance traveled: %f",
            _player->getSpeedRate(TYPE_RUN, true),
            std::sqrt(_player->m_position.Distance2DSq({ sessionMovementInfo.position.x, sessionMovementInfo.position.y })));

        return true;
    }

    // Speed
    // implement worldConfig.antiHack.isSpeedHackCkeckEnabled
    if (!_player->isOnTaxi() && _player->obj_movement_info.transport_guid == 0 && !_player->GetSession()->GetPermissionCount())
    {
        // simplified: just take the fastest speed. less chance of fuckups too
        // get the "normal speeds" not the changed ones!
        float speed = (_player->flying_aura) ? _player->getSpeedRate(TYPE_FLY, false) : (_player->getSpeedRate(TYPE_SWIM, false) > _player->getSpeedRate(TYPE_RUN, false)) ? _player->getSpeedRate(TYPE_SWIM, false) : _player->getSpeedRate(TYPE_RUN, false);

        _player->SDetector->AddSample(sessionMovementInfo.position.x, sessionMovementInfo.position.y, Util::getMSTime(), speed);

        if (_player->SDetector->IsCheatDetected())
        {
            _player->SDetector->ReportCheater(_player);
            return true;
        }
    }

    return false;
}

void WorldSession::handleMovementOpcodes(WorldPacket& recvData)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    /// Check before reading movementinfo packet
    CHECK_INWORLD_RETURN

    if (_player->isTransferPending() || _player->isOnTaxi() || _player->justDied())
        return;

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Set up some vars to simplify code
    // Zyres: save the opcode here for better handling
    const uint16_t opcode = recvData.GetOpcode();

    // Zyres: We (the player) controles the movement of us or another player/unit.
    // this is always initialise with the player, can be changed to any other unit.
    Unit* mover = _player->mControledUnit;

    // Zyres: Clear standing state to stand... investigate further if this is really needed
    if (mover->getStandState() != STANDSTATE_STAND && opcode == MSG_MOVE_START_FORWARD)
        mover->setStandState(STANDSTATE_STAND);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// read movement info from packet
    MovementInfo movementInfo;
    recvData >> movementInfo;

    // store the read movementInfo here. We will need it for other functions related to this handler (e.g. updatePlayerMovementVars)
    sessionMovementInfo = movementInfo;

    // Zyres: now we have the data from the movement packet. Check out if we are the mover, otherwise stop processing
    //\todo why do we check different on other versions?
#if VERSION_STRING > TBC
    // wotlk check
    if (sessionMovementInfo.guid != mover->getGuid())
        return;

    // cata check
    if (m_MoverGuid != mover->getGuid())
        return;
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    /// out of bounds check
    {
        bool out_of_bounds = false;
        out_of_bounds = out_of_bounds || sessionMovementInfo.position.y < _minY;
        out_of_bounds = out_of_bounds || sessionMovementInfo.position.y > _maxY;
        out_of_bounds = out_of_bounds || sessionMovementInfo.position.x > _maxX;
        out_of_bounds = out_of_bounds || sessionMovementInfo.position.x > _maxX;

        if (out_of_bounds)
        {
            Disconnect();
            return;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// stop using go on movement
    if (auto* const summoned_object = _player->m_SummonedObject)
    {
        if (summoned_object->isGameObject())
        {
            auto* const go = dynamic_cast<GameObject*>(summoned_object);
            if (go->isFishingNode())
            {
                auto* fishing_node = dynamic_cast<GameObject_FishingNode*>(go);
                fishing_node->EndFishing(true);

                // This is done separately as not all channeled spells are canceled by all movement opcodes
                if (auto* spell = _player->getCurrentSpell(CURRENT_CHANNELED_SPELL))
                {
                    spell->sendChannelUpdate(0U);
                    spell->finish(false);
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// hack detected?
    if (isHackDetectedInMovementData(opcode))
    {
        return;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Lets update our internal save vars
    updatePlayerMovementVars(opcode);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Remove emote state if available
    if (mover->getEmoteState())
        mover->setEmoteState(EMOTE_ONESHOT_NONE);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Falling damage

    // Zyres: Spell realted "blinking"
    if (_player->blinked)
    {
        _player->blinked = false;
        _player->m_fallDisabledUntil = UNIXTIME + 5;
        _player->SpeedCheatDelay(2000);
    }
    else
    {
        if (opcode == MSG_MOVE_FALL_LAND)
        {
            mover->handleFall(sessionMovementInfo);
        }
        else
        {
            // whilst player is not falling, continuously update Z axis position.
            // once player lands, this will be used to determine how far he fell.
            if (!(sessionMovementInfo.flags & MOVEFLAG_FALLING))
                mover->z_axisposition = sessionMovementInfo.position.z;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Transport position
    if (mover->obj_movement_info.transport_guid != 0 && sessionMovementInfo.transport_guid == 0)
    {
        /* we left the transporter we were on */
        LOG_DEBUG("Left Transport guid %u", WoWGuid::getGuidLowPartFromUInt64(mover->obj_movement_info.transport_guid));

        Transporter* transporter = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(mover->obj_movement_info.transport_guid));
        if (transporter != NULL)
            transporter->RemovePassenger(static_cast<Player*>(mover));

        mover->obj_movement_info.transport_guid = 0;
        _player->SpeedCheatReset();

    }
    else
    {
        if (sessionMovementInfo.transport_guid != 0)
        {

            if (mover->obj_movement_info.transport_guid == 0)
            {
                LOG_DEBUG("Entered Transport guid %u", WoWGuid::getGuidLowPartFromUInt64(sessionMovementInfo.transport_guid));

                Transporter* transporter = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(sessionMovementInfo.transport_guid));
                if (transporter != NULL)
                    transporter->AddPassenger(static_cast<Player*>(mover));

                /* set variables */
                mover->obj_movement_info.transport_guid = sessionMovementInfo.transport_guid;
                mover->obj_movement_info.transport_time = sessionMovementInfo.transport_time;
                mover->obj_movement_info.transport_position.x = sessionMovementInfo.transport_position.x;
                mover->obj_movement_info.transport_position.y = sessionMovementInfo.transport_position.y;
                mover->obj_movement_info.transport_position.z = sessionMovementInfo.transport_position.z;
                mover->obj_movement_info.transport_position.o = sessionMovementInfo.transport_position.o;

                mover->m_transportData.transportGuid = sessionMovementInfo.transport_guid;
                mover->m_transportData.relativePosition.x = sessionMovementInfo.transport_position.x;
                mover->m_transportData.relativePosition.y = sessionMovementInfo.transport_position.y;
                mover->m_transportData.relativePosition.z = sessionMovementInfo.transport_position.z;
                mover->m_transportData.relativePosition.o = sessionMovementInfo.transport_position.o;
            }
            else
            {
                /* no changes */
                mover->obj_movement_info.transport_time = sessionMovementInfo.transport_time;
                mover->obj_movement_info.transport_position.x = sessionMovementInfo.transport_position.x;
                mover->obj_movement_info.transport_position.y = sessionMovementInfo.transport_position.y;
                mover->obj_movement_info.transport_position.z = sessionMovementInfo.transport_position.z;
                mover->obj_movement_info.transport_position.o = sessionMovementInfo.transport_position.o;

                mover->m_transportData.relativePosition.x = sessionMovementInfo.transport_position.x;
                mover->m_transportData.relativePosition.y = sessionMovementInfo.transport_position.y;
                mover->m_transportData.relativePosition.z = sessionMovementInfo.transport_position.z;
                mover->m_transportData.relativePosition.o = sessionMovementInfo.transport_position.o;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Breathing & Underwaterstate
    _player->handleBreathing(sessionMovementInfo, this);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Aura Interruption
    _player->handleAuraInterruptForMovementFlags(sessionMovementInfo);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Update our Server position
#if VERSION_STRING <= WotLK    
    if (m_MoverWoWGuid.getRawGuid() == mover->getGuid())
    {
        if (!mover->GetTransport())
        {
            if (!mover->SetPosition(sessionMovementInfo.position.x, sessionMovementInfo.position.y, sessionMovementInfo.position.z, sessionMovementInfo.position.o))
            {
                //extra check to set HP to 0 only if the player is dead (KillPlayer() has already this check)
                if (mover->isAlive())
                {
                    mover->setHealth(0);
                    _player->KillPlayer();
                }

                MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(mover->GetMapId());
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
                }
            }
        }
    }
    else
    {
        if (!mover->isRooted())
            mover->SetPosition(sessionMovementInfo.position.x, sessionMovementInfo.position.y, sessionMovementInfo.position.z, sessionMovementInfo.position.o);
    }
#else
    mover->SetPosition(sessionMovementInfo.getPosition()->x, sessionMovementInfo.getPosition()->y, sessionMovementInfo.getPosition()->z, sessionMovementInfo.getPosition()->o);
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    /// send our move to all inrange players
    
#if VERSION_STRING >= Cata

    WorldPacket data(SMSG_PLAYER_MOVE, recvData.size());
    data << sessionMovementInfo;
    mover->SendMessageToSet(&data, false);

#elif VERSION_STRING == WotLK

    WorldPacket data(opcode, recvData.size());
    data << sessionMovementInfo;
    mover->SendMessageToSet(&data, false);

#else

    // Zyres NOTE: versions older than WotLK do not send us the guid within the movement packet (needed for the packet send to other players)
    // but we should already received the active mover
    sessionMovementInfo.guid = m_MoverWoWGuid;

    WorldPacket data(opcode, recvData.size());
    data << sessionMovementInfo;
    mover->SendMessageToSet(&data, false);

#endif
}

void WorldSession::handleAcknowledgementOpcodes(WorldPacket& recvPacket)
{
    LogDebugFlag(LF_OPCODE, "Opcode %s (%u) received. This opcode is not known/implemented right now!",
        sOpcodeTables.getNameForInternalId(recvPacket.GetOpcode()).c_str(), recvPacket.GetOpcode());

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
    _player->setTransferStatus(TRANSFER_NONE);
    if (_player->IsInWorld())
        return;

    LogDebugFlag(LF_OPCODE, "Received MSG_MOVE_WORLDPORT_ACK");

    if (_player->GetTransport() && _player->GetMapId() != _player->GetTransport()->GetMapId())
    {
        const auto transporter = _player->GetTransport();

        const float transportPositionX = transporter->GetPositionX() + _player->GetTransOffsetX();
        const float transportPositionY = transporter->GetPositionY() + _player->GetTransOffsetY();
        const float transportPositionZ = transporter->GetPositionZ() + _player->GetTransOffsetZ();

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

    _player->ResetTimeSync();
    _player->SendTimeSync();
}

void WorldSession::handleMoveTeleportAckOpcode(WorldPacket& recvPacket)
{
    MsgMoveTeleportAck srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received MSG_MOVE_TELEPORT_ACK.");

    if (srlPacket.guid.getRawGuid() == _player->getGuid())
    {
        if (worldConfig.antiHack.isTeleportHackCheckEnabled && !(HasGMPermissions() && worldConfig.antiHack.isAntiHackCheckDisabledForGm))
        {
            if (!_player->isTransferPending())
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

        _player->setTransferStatus(TRANSFER_NONE);
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
}
