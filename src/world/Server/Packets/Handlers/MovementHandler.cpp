/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Server/Packets/ManagedPacket.h"
#include "Server/Packets/CmsgSetActiveMover.h"
#include "Server/Packets/MovementPacket.h"
#include "Server/WorldSession.h"
#include "Objects/Units/Creatures/Summons/SummonHandler.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Creatures/Vehicle.hpp"
#include "Map/Visibility/VisibilityTypes.hpp"
#include "Objects/GameObject.h"
#include "Server/Warden/SpeedDetector.h"
#include "Management/ObjectMgr.hpp"
#include "Management/TaxiMgr.hpp"
#include "Server/Packets/CmsgWorldTeleport.h"
#include "Server/Packets/SmsgMountspecialAnim.h"
#include "Server/Packets/MsgMoveTeleportAck.h"
#include "Server/Packets/SmsgNewWorld.h"
#include "Server/Packets/SmsgPlayerMove.h"
#include "Objects/Units/Creatures/Pet.h"
#include "Server/OpcodeTable.hpp"
#include "Spell/Definitions/AuraInterruptFlags.hpp"
#include "Objects/Transporter.hpp"
#include "Server/World.h"
#include "Server/WorldSessionLog.hpp"
#include "Spell/Spell.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/MathConstants.hpp"
#include "Server/PacketBroadcast.hpp"

using namespace AscEmu::Packets;

namespace
{
    void addPetToTransport(Player* player, Transporter* transport, const LocationVector& transportOffset)
    {
        if (!player || !transport)
            return;

        Pet* pet = player->getPet();
        if (!pet || !pet->IsInWorld() || pet->getWorldMap() != player->getWorldMap())
            return;

        if (pet->GetTransport() == transport)
            return;

        if (pet->GetTransport())
            pet->GetTransport()->RemovePassenger(pet);

        // Put the pet onto the same local transport position as its owner.
        // The transport will keep the pet in world-space from this point on.
        transport->AddPassenger(pet, transportOffset);
    }

    void removePetFromTransport(Player* player, Transporter* transport)
    {
        if (!player || !transport)
            return;

        if (Pet* pet = player->getPet())
        {
            if (pet->GetTransport() == transport)
                transport->RemovePassenger(pet);
        }
    }
}

void WorldSession::handleSetActiveMoverOpcode(WorldPacket& recvPacket)
{
    CmsgSetActiveMover srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (srlPacket.guid == m_MoverWoWGuid.getRawGuid())
        return;

#if VERSION_STRING < Cata
    #if VERSION_STRING > TBC
    if (_player->getCharmGuid() != srlPacket.guid.getRawGuid() && _player->getGuid() != srlPacket.guid.getRawGuid())
    {
        auto bad_packet = true;
        if (const auto vehicle = _player->getVehicle())
            if (const auto owner = vehicle->getBase())
                if (owner->getGuid() == srlPacket.guid.getRawGuid())
                    bad_packet = false;

        if (bad_packet)
            return;
    }
    #endif

    if (srlPacket.guid.getRawGuid() == 0)
        m_MoverWoWGuid.init(_player->getGuid());
    else
        m_MoverWoWGuid = srlPacket.guid;
#endif
}

// Client tells us it skipped `timeSkipped` ms of its own movement clock (e.g. after a lag spike),
// so we shift our tracked timestamp for the mover forward by the same amount and echo the skip to
// everyone else who can see the mover, so their view of its movement clock stays in sync too.
void WorldSession::handleMoveTimeSkippedOpcode(WorldPacket& recvPacket)
{
    uint64_t guid;
    uint32_t timeSkipped;
    recvPacket >> guid;
    recvPacket >> timeSkipped;

    Unit* mover = _player->m_controledUnit;
    if (mover == nullptr || guid != mover->getGuid())
        return;

    mover->obj_movement_info.update_time += timeSkipped;

    WorldPacket data(MSG_MOVE_TIME_SKIPPED, 16);
    data << WoWGuid(mover->getGuid());
    data << timeSkipped;
    mover->sendMessageToSet(&data, false);
}

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
        sCheatLog.writefromsession(this, "Detected jump hacking.");
        return true;
    }

    // Teleport
    // implement worldConfig.antiHack.isTeleportHackCheckEnabled
    if (_player->m_position.distance2DSq({ sessionMovementInfo.position.x, sessionMovementInfo.position.y }) > 3025.0f &&
        _player->getSpeedRate(TYPE_RUN, true) < 50.0f && !_player->obj_movement_info.transport_guid)
    {
        sLogger.debug("isHackDetectedInMovementData() TELEPORT CHECK TRIGGERED distance={:.2f} speed={} storedTransportGuid={} incomingTransportGuid={}",
            std::sqrt(_player->m_position.distance2DSq({ sessionMovementInfo.position.x, sessionMovementInfo.position.y })),
            _player->getSpeedRate(TYPE_RUN, true), _player->obj_movement_info.transport_guid.getRawGuid(), sessionMovementInfo.transport_guid.getRawGuid());

        sCheatLog.writefromsession(this, "Teleport exploit detected. Speed: {}. Distance traveled: {:.2f}.", _player->getSpeedRate(TYPE_RUN, true),
                                          std::sqrt(_player->m_position.distance2DSq({ sessionMovementInfo.position.x, sessionMovementInfo.position.y })));

        return true;
    }

    // Speed
    // implement worldConfig.antiHack.isSpeedHackCkeckEnabled
    if (_player->isOnTaxi() && _player->obj_movement_info.transport_guid.isEmpty() && !_player->getSession()->hasPermissions())
    {
        // simplified: just take the fastest speed. less chance of fuckups too
        // get the "normal speeds" not the changed ones!
        float speed = (_player->m_flyingAura) ? _player->getSpeedRate(TYPE_FLY, false) : (_player->getSpeedRate(TYPE_SWIM, false) > _player->getSpeedRate(TYPE_RUN, false)) ? _player->getSpeedRate(TYPE_SWIM, false) : _player->getSpeedRate(TYPE_RUN, false);

        _player->m_speedCheatDetector->AddSample(sessionMovementInfo.position.x, sessionMovementInfo.position.y, Util::getMSTime(), speed);

        if (_player->m_speedCheatDetector->IsCheatDetected())
        {
            _player->m_speedCheatDetector->ReportCheater(_player);
            return true;
        }
    }

    return false;
}

void WorldSession::handleMovementOpcodes(WorldPacket& recvData)
{
    if (_player->isTransferPending() || _player->isOnTaxi() || _player->justDied())
        return;

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Set up some vars to simplify code. We use the internal opcode id for Multiversion support
    // Zyres: save the opcode here for better handling
    const auto opcode = sOpcodeTables.getInternalIdForHex(recvData.getOpcode());

    // Zyres: We (the player) controles the movement of us or another player/unit.
    // this is always initialise with the player, can be changed to any other unit.
    Unit* mover = _player->m_controledUnit;

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
#if VERSION_STRING > TBC
    // wotlk check
    if (sessionMovementInfo.guid != mover->getGuid())
        return;
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    /// out of bounds check
    {
        bool out_of_bounds = false;
        out_of_bounds = out_of_bounds || sessionMovementInfo.position.y < visibility::Terrain::MinY;
        out_of_bounds = out_of_bounds || sessionMovementInfo.position.y > visibility::Terrain::MaxY;
        out_of_bounds = out_of_bounds || sessionMovementInfo.position.x > visibility::Terrain::MaxX;
        out_of_bounds = out_of_bounds || sessionMovementInfo.position.x > visibility::Terrain::MaxX;

        if (out_of_bounds)
        {
            Disconnect();
            return;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// stop using go on movement
    if (auto* const summoned_object = _player->m_summonedObject)
    {
        if (summoned_object->isGameObject())
        {
            auto* const go = dynamic_cast<GameObject*>(summoned_object);
            if (go->isFishingNode())
            {
                // This is done separately as not all channeled spells are canceled by all movement opcodes
                if (auto* spell = _player->getCurrentSpell(CURRENT_CHANNELED_SPELL))
                {
                    spell->cancel();
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// hack detected?
    // Player movement anti-cheat currently relies on player-specific state/position.
    // A possessed creature is a different authoritative mover, so feeding its
    // coordinates into the player checks can falsely reject perfectly valid movement
    // (most notably the teleport-distance check against the player's stationary body).
    if (mover == _player && isHackDetectedInMovementData(static_cast<uint16_t>(opcode)))
    {
        return;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Lets update our internal save vars
    // These flags belong to Player itself. Do not derive them from a possessed unit.
    if (mover == _player)
        updatePlayerMovementVars(static_cast<uint16_t>(opcode));

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Remove emote state if available
    if (mover->getEmoteState())
        mover->setEmoteState(EMOTE_ONESHOT_NONE);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Falling damage

    // Zyres: Spell realted "blinking"
    if (_player->m_blinked)
    {
        _player->m_blinked = false;
        _player->m_fallDisabledUntil = UNIXTIME + 5;
        _player->speedCheatDelay(2000);
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
                mover->m_zAxisPosition = sessionMovementInfo.position.z;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Transport position
#if VERSION_STRING <= WotLK
    if (movementInfo.hasMovementFlag(MOVEFLAG_TRANSPORT))
    {
        // if we boarded a transport, add us to it
        if (mover->isPlayer())
        {
            if (!mover->GetTransport())
            {
                if (Transporter* transport = sTransportHandler.getTransporter(movementInfo.transport_guid))
                {
                    transport->AddPassenger(mover, sessionMovementInfo.transport_position);
                    addPetToTransport(mover->ToPlayer(), transport, sessionMovementInfo.transport_position);

                    /* set variables */
                    mover->obj_movement_info.transport_time = sessionMovementInfo.transport_time;
                    mover->obj_movement_info.transport_position.x = sessionMovementInfo.transport_position.x;
                    mover->obj_movement_info.transport_position.y = sessionMovementInfo.transport_position.y;
                    mover->obj_movement_info.transport_position.z = sessionMovementInfo.transport_position.z;
                    mover->obj_movement_info.transport_position.o = sessionMovementInfo.transport_position.o;
                }
            }
            else if (mover->GetTransport() != sTransportHandler.getTransporter(movementInfo.transport_guid))
            {
                Transporter* oldTransport = mover->GetTransport();
                removePetFromTransport(mover->ToPlayer(), oldTransport);
                oldTransport->RemovePassenger(mover);
                if (Transporter* transport = sTransportHandler.getTransporter(movementInfo.transport_guid))
                {
                    transport->AddPassenger(mover, sessionMovementInfo.transport_position);
                    addPetToTransport(mover->ToPlayer(), transport, sessionMovementInfo.transport_position);

                    /* set variables */
                    mover->obj_movement_info.transport_time = sessionMovementInfo.transport_time;
                    mover->obj_movement_info.transport_position.x = sessionMovementInfo.transport_position.x;
                    mover->obj_movement_info.transport_position.y = sessionMovementInfo.transport_position.y;
                    mover->obj_movement_info.transport_position.z = sessionMovementInfo.transport_position.z;
                    mover->obj_movement_info.transport_position.o = sessionMovementInfo.transport_position.o;
                }
                else
                {
                    movementInfo.clearTransportData();
                    mover->obj_movement_info.clearTransportData();
                }
            }
            else
            {
                /* set variables */
                mover->obj_movement_info.transport_time = sessionMovementInfo.transport_time;
#if VERSION_STRING > TBC
                mover->obj_movement_info.transport_seat = movementInfo.transport_seat;
#endif
                mover->obj_movement_info.transport_position.x = sessionMovementInfo.transport_position.x;
                mover->obj_movement_info.transport_position.y = sessionMovementInfo.transport_position.y;
                mover->obj_movement_info.transport_position.z = sessionMovementInfo.transport_position.z;
                mover->obj_movement_info.transport_position.o = sessionMovementInfo.transport_position.o;
            }
        }
#ifdef FT_VEHICLES
        // Transports like Elevators
        if (!mover->GetTransport() && !mover->getVehicle())
        {
            GameObject* go = mover->getWorldMapGameObject(movementInfo.transport_guid);
            if (!go || go->getOType() != GAMEOBJECT_TYPE_TRANSPORT)
                movementInfo.removeMovementFlag(MOVEFLAG_TRANSPORT);
        }
#else
        // Transports like Elevators
        if (!mover->GetTransport())
        {
            GameObject* go = mover->getWorldMapGameObject(movementInfo.transport_guid);
            if (!go || go->getOType() != GAMEOBJECT_TYPE_TRANSPORT)
                movementInfo.removeMovementFlag(MOVEFLAG_TRANSPORT);
        }
#endif
    }
    else if (mover->ToPlayer() && mover->GetTransport()) // if we were on a transport, leave
    {
        Transporter* transport = mover->GetTransport();
        removePetFromTransport(mover->ToPlayer(), transport);
        transport->RemovePassenger(mover);
        movementInfo.clearTransportData();
    }
#else
    if (mover->isPlayer())
    {
        // if we boarded a transport, add us to it
        if (movementInfo.transport_guid)
        {
            sLogger.debug("MovementHandler transport_guid={} currentTransport={} lookedUpTransport={}",
                movementInfo.transport_guid.getRawGuid(), mover->GetTransport() ? mover->GetTransport()->getGuid() : 0,
                sTransportHandler.getTransporter(movementInfo.transport_guid.getLowGuid()) ? "found" : "NOT FOUND");

            if (!mover->GetTransport())
            {
                if (Transporter* transport = sTransportHandler.getTransporter(movementInfo.transport_guid))
                {
                    transport->AddPassenger(mover, sessionMovementInfo.transport_position);
                    addPetToTransport(mover->ToPlayer(), transport, sessionMovementInfo.transport_position);

                    /* set variables */
                    mover->obj_movement_info.transport_time = sessionMovementInfo.transport_time;
                    mover->obj_movement_info.transport_position.x = sessionMovementInfo.transport_position.x;
                    mover->obj_movement_info.transport_position.y = sessionMovementInfo.transport_position.y;
                    mover->obj_movement_info.transport_position.z = sessionMovementInfo.transport_position.z;
                    mover->obj_movement_info.transport_position.o = sessionMovementInfo.transport_position.o;
                }
            }
            else if (mover->GetTransport()->getGuid() != movementInfo.transport_guid)
            {
                Transporter* oldTransport = mover->GetTransport();
                removePetFromTransport(mover->ToPlayer(), oldTransport);
                oldTransport->RemovePassenger(mover);
                if (Transporter* transport = sTransportHandler.getTransporter(movementInfo.transport_guid))
                {
                    transport->AddPassenger(mover, sessionMovementInfo.transport_position);
                    addPetToTransport(mover->ToPlayer(), transport, sessionMovementInfo.transport_position);

                    /* set variables */
                    mover->obj_movement_info.transport_time = sessionMovementInfo.transport_time;
                    mover->obj_movement_info.transport_position.x = sessionMovementInfo.transport_position.x;
                    mover->obj_movement_info.transport_position.y = sessionMovementInfo.transport_position.y;
                    mover->obj_movement_info.transport_position.z = sessionMovementInfo.transport_position.z;
                    mover->obj_movement_info.transport_position.o = sessionMovementInfo.transport_position.o;
                }
                else
                {
                    movementInfo.clearTransportData();
                    mover->obj_movement_info.clearTransportData();
                }
            }
            else
            {
                /* set variables */
                mover->obj_movement_info.transport_time = sessionMovementInfo.transport_time;
                mover->obj_movement_info.transport_seat = movementInfo.transport_seat;
                mover->obj_movement_info.transport_position.x = sessionMovementInfo.transport_position.x;
                mover->obj_movement_info.transport_position.y = sessionMovementInfo.transport_position.y;
                mover->obj_movement_info.transport_position.z = sessionMovementInfo.transport_position.z;
                mover->obj_movement_info.transport_position.o = sessionMovementInfo.transport_position.o;
            }

            // Transports like Elevators
            if (!mover->GetTransport() && !mover->getVehicle())
            {
                GameObject* go = mover->getWorldMapGameObject(movementInfo.transport_guid);
                if (!go || go->getGoType() != GAMEOBJECT_TYPE_TRANSPORT)
                    movementInfo.transport_guid = 0;
            }
        }
        else if (mover && mover->GetTransport())
        {
            // if we were on a transport, leave
            Transporter* transport = mover->GetTransport();
            removePetFromTransport(mover->ToPlayer(), transport);
            transport->RemovePassenger(mover);
            movementInfo.clearTransportData();
            mover->obj_movement_info.clearTransportData();
        }
    }
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Update the authoritative server position before visibility routing/broadcast.
    // For possessed units this also moves the SpatialIndex entry, transfers grid/cell
    // ownership and refreshes the remote viewer before recipients are collected below.
    mover->SetPosition(sessionMovementInfo.position.x, sessionMovementInfo.position.y,
        sessionMovementInfo.position.z, sessionMovementInfo.position.o);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Breathing & Underwaterstate
    // These are player-body systems. Movement of a possessed creature must not modify
    // the stationary controller's breathing/aura state.
    if (mover == _player)
    {
        _player->handleBreathing(sessionMovementInfo, this);

        //////////////////////////////////////////////////////////////////////////////////////
        /// Aura Interruption
        _player->handleAuraInterruptForMovementFlags(sessionMovementInfo);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// send our move to all inrange players

#if VERSION_STRING >= Cata

    SmsgPlayerMove managedPacket(sessionMovementInfo);
    PacketBroadcast::sendToSet(*mover, managedPacket, false);

#elif VERSION_STRING == WotLK

    WorldPacket data(static_cast<uint16_t>(opcode), recvData.size());
    data << sessionMovementInfo;
    mover->sendMessageToSet(&data, _player);

#else // TBC and Classic

    // Zyres NOTE: versions older than WotLK do not send us the guid within the movement packet (needed for the packet send to other players)
    // but we should already received the active mover
    sessionMovementInfo.guid = m_MoverWoWGuid;

    WorldPacket data(static_cast<uint16_t>(opcode), recvData.size());
    data << sessionMovementInfo;
    mover->sendMessageToSet(&data, false);

#endif

#ifdef FT_VEHICLES
    //////////////////////////////////////////////////////////////////////////////////////////
    /// Some vehicles allow the passenger to turn by himself
    if (Vehicle* vehicle = mover->getVehicle())
    {
        if (auto const* seat = vehicle->getSeatForPassenger(mover))
        {
            if (seat->flags & WDB::Structures::VehicleSeatFlags::VEHICLE_SEAT_FLAG_ALLOW_TURNING)
            {
                if (movementInfo.position.getOrientation() != mover->GetOrientation())
                {
                    mover->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_TURNING);
                    mover->SetOrientation(movementInfo.position.getOrientation());
                }
            }
        }
        return;
    }
#endif
}

void WorldSession::handleAcknowledgementOpcodes(WorldPacket& recvPacket)
{
    const auto opcode = sOpcodeTables.getInternalIdForHex(recvPacket.getOpcode());
    switch (opcode)
    {
        case CMSG_MOVE_SET_CAN_FLY_ACK:
        {
            MovementInfo movementInfo;
            recvPacket >> movementInfo;

            _player->obj_movement_info.flags = movementInfo.getMovementFlags();
        } break;
        default:
        {
            sLogger.debug("WorldSession::handleAcknowledgementOpcodes : Opcode {} ({}) received. This opcode is not known/implemented right now!",
                sOpcodeTables.getNameForInternalId(recvPacket.getOpcode()), recvPacket.getOpcode());

            recvPacket.rfinish();
        }
    }
}

void WorldSession::handleForceSpeedChangeAck(WorldPacket& recvPacket)
{
    Unit* mover = _player->m_controledUnit;

    MovementInfo movementInfo;
    recvPacket >> movementInfo;

    if (movementInfo.getGuid() != mover->getGuid())
        return;

    sLogger.debugOpcode("WorldSession::handleForceSpeedChangeAck: Counter {}, speed {} received.", movementInfo.counter, movementInfo.newSpeed);

    // client ACK send one packet for mounted/run case and need skip all except last from its
    // in other cases anti-cheat check can be fail in false case
    UnitSpeedType move_type;
    UnitSpeedType force_move_type;

    const auto opcode = sOpcodeTables.getInternalIdForHex(recvPacket.getOpcode());
    switch (opcode)
    {
        case CMSG_FORCE_WALK_SPEED_CHANGE_ACK:          move_type = TYPE_WALK;          force_move_type = TYPE_WALK;        break;
        case CMSG_FORCE_RUN_SPEED_CHANGE_ACK:           move_type = TYPE_RUN;           force_move_type = TYPE_RUN;         break;
        case CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK:      move_type = TYPE_RUN_BACK;      force_move_type = TYPE_RUN_BACK;    break;
        case CMSG_FORCE_SWIM_SPEED_CHANGE_ACK:          move_type = TYPE_SWIM;          force_move_type = TYPE_SWIM;        break;
        case CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK:     move_type = TYPE_SWIM_BACK;     force_move_type = TYPE_SWIM_BACK;   break;
        case CMSG_FORCE_TURN_RATE_CHANGE_ACK:           move_type = TYPE_TURN_RATE;     force_move_type = TYPE_TURN_RATE;   break;
        case CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK:        move_type = TYPE_FLY;           force_move_type = TYPE_FLY;         break;
        case CMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK:   move_type = TYPE_FLY_BACK;      force_move_type = TYPE_FLY_BACK;    break;
        case CMSG_FORCE_PITCH_RATE_CHANGE_ACK:          move_type = TYPE_PITCH_RATE;    force_move_type = TYPE_PITCH_RATE;  break;
        default:
            sLogger.failure("WorldSession::handleForceSpeedChangeAck: Unknown move type opcode: {}", recvPacket.getOpcode());
            return;
    }

    // skip all forced speed changes except last and unexpected
    // in run/mounted case used one ACK and it must be skipped.m_forced_speed_changes[MOVE_RUN} store both.
    if (_player->m_forced_speed_changes[force_move_type] > 0)
    {
        --_player->m_forced_speed_changes[force_move_type];
        if (_player->m_forced_speed_changes[force_move_type] > 0)
            return;
    }

    if (!_player->GetTransport() && std::fabs(_player->getSpeedRate(move_type, false) - movementInfo.newSpeed) > 0.01f)
    {
        if (_player->getSpeedRate(move_type, false) > movementInfo.newSpeed) // must be greater - just correct
        {
            _player->setSpeedRate(move_type, _player->getSpeedRate(move_type, false), false);
        }
        else // must be lesser - cheating
        {
            // handle something here
        }
    }
}

void WorldSession::handleWorldTeleportOpcode(WorldPacket& recvPacket)
{
    CmsgWorldTeleport srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (!HasGMPermissions())
    {
        sendNotification("You do not have permission to use this function.");
        return;
    }

    _player->safeTeleport(srlPacket.mapId, 0, srlPacket.location);
}

void WorldSession::handleMountSpecialAnimOpcode(WorldPacket& /*recvPacket*/)
{
    SmsgMountspecialAnim managedPacket(_player->getGuid());
    PacketBroadcast::sendToSet(*_player, managedPacket, true);
}

bool WorldSession::recoverFailedWorldport(const char* reason)
{
    if (_player == nullptr)
        return false;

    const uint32_t failedMapId = _player->GetMapId();
    const uint32_t failedInstanceId = _player->GetInstanceID();
    const LocationVector failedPosition = _player->GetPosition();

    if (_player->hasValidBGEntryPoint())
    {
        const LocationVector entryPosition = _player->getBGEntryPosition();
        const uint32_t entryMapId = _player->getBGEntryMapId();
        const uint32_t entryInstanceId = static_cast<uint32_t>(_player->getBGEntryInstanceId());

        // Do not bounce forever if the recovery destination itself is
        // the world that just failed to attach. MapId 0 is valid; only
        // an all-zero location is considered an empty entry point.
        const float entryDx = failedPosition.x - entryPosition.x;
        const float entryDy = failedPosition.y - entryPosition.y;
        const float entryDz = failedPosition.z - entryPosition.z;
        const bool failedAtEntryPoint =
            failedMapId == entryMapId &&
            failedInstanceId == entryInstanceId &&
            (entryDx * entryDx + entryDy * entryDy + entryDz * entryDz) < 0.01f;

        if (!failedAtEntryPoint && _player->safeTeleport(entryMapId, entryInstanceId, entryPosition))
        {
            sLogger.failure("{} for player GUID {} MapId {} InstanceId {}. Returning player to BG/instance entry point MapId {} InstanceId {}.",
                reason, std::to_string(_player->getGuid()), failedMapId, failedInstanceId, entryMapId, entryInstanceId);
            return true;
        }
    }

    const uint32_t bindMapId = _player->getBindMapId();
    const LocationVector bindPosition = _player->getBindPosition();

    if ((failedMapId != bindMapId || failedInstanceId != 0) &&
        _player->safeTeleport(bindMapId, 0, bindPosition))
    {
        sLogger.failure("{} for player GUID {} MapId {} InstanceId {}. Returning player to bind map {}.",
            reason, std::to_string(_player->getGuid()), failedMapId, failedInstanceId, bindMapId);
        return true;
    }

    sLogger.failure("{} for player GUID {} MapId {} InstanceId {} and recovery also failed. Disconnecting.",
        reason, std::to_string(_player->getGuid()), failedMapId, failedInstanceId);
    Disconnect();
    return false;
}

void WorldSession::handleMoveWorldportAckOpcode(WorldPacket& /*recvPacket*/)
{
    _player->setTransferStatus(TRANSFER_NONE);
    if (_player->IsInWorld())
        return;

    sLogger.debugOpcode("Received CMSG_MOVE_WORLDPORT_ACK.");

    // A transport can cross another map boundary before a slow client finishes
    // the previous worldport. Follow the transport to its current map while
    // preserving the transport-local passenger offset.
    if (_player->hasTeleportTransport())
    {
        if (Transporter* transporter = sTransportHandler.getTransporter(_player->getTeleportTransportGuid()))
        {
            if (transporter->IsInWorld() && _player->GetMapId() != transporter->GetMapId())
            {
                LocationVector positionOnTransport = _player->getTeleportTransportOffset();
                transporter->calculatePassengerPosition(
                    positionOnTransport.x,
                    positionOnTransport.y,
                    positionOnTransport.z,
                    &positionOnTransport.o);

                _player->SetMapId(transporter->GetMapId());
                _player->SetInstanceID(transporter->GetInstanceID());
                _player->SetPosition(positionOnTransport);

                SmsgNewWorld managedPacket(transporter->GetMapId(), positionOnTransport);
                sendManagedPacket(managedPacket);

                _player->resetTimeSync();
                _player->sendTimeSync();
                return;
            }
        }
    }

    _player->m_teleportState = 2;

    const auto mapInfo = sMySQLStore.getWorldMapInfo(_player->GetMapId());
    if (mapInfo == nullptr || _player->GetMapId() >= MAX_NUM_MAPS)
    {
        recoverFailedWorldport("Invalid worldport destination");
        return;
    }

    WorldMap* world = sMapMgr.findWorldMap(_player->GetMapId(), _player->GetInstanceID());
    if (world == nullptr)
    {
        recoverFailedWorldport("Worldport destination map not found");
        return;
    }

    // Preserve the transport relationship across world transfers. Resolve the
    // transport by the GUID saved with the teleport destination, restore the
    // passenger relation and derive the player's world position from the
    // transport's current transform before attaching the player to the target map.
    if (_player->hasTeleportTransport())
    {
        Transporter* transporter = sTransportHandler.getTransporter(_player->getTeleportTransportGuid());
        if (transporter && transporter->IsInWorld() && transporter->getWorldMap() == world)
        {
            const LocationVector transportOffset = _player->getTeleportTransportOffset();
            transporter->RestorePassengerAfterTeleport(_player, transportOffset);

            LocationVector worldPosition = transportOffset;
            transporter->calculatePassengerPosition(
                worldPosition.x,
                worldPosition.y,
                worldPosition.z,
                &worldPosition.o);

            _player->SetPosition(worldPosition);
        }
        else
        {
            sLogger.warning("Worldport transport rebind failed for player {}: transportGuid={} playerMap={} instance={}.", _player->getName(), _player->getTeleportTransportGuid().getRawGuid(), _player->GetMapId(), _player->GetInstanceID());
        }
    }

    if (!world->onPlayerEnter(_player))
    {
        recoverFailedWorldport("Worldport attach failed");
        return;
    }

    _player->clearTeleportTransport();
    _player->resetTimeSync();
    _player->sendTimeSync();
}

void WorldSession::handleMoveTeleportAckOpcode(WorldPacket& recvPacket)
{
    MsgMoveTeleportAck srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_MOVE_TELEPORT_ACK.");

    if (srlPacket.guid.getRawGuid() == _player->getGuid())
    {
        if (worldConfig.antiHack.isTeleportHackCheckEnabled && !(HasGMPermissions() && worldConfig.antiHack.isAntiHackCheckDisabledForGm))
        {
            if (!_player->isTransferPending())
            {
                sCheatLog.writefromsession(this, "Teleport exploit detected: missing transfer state. Player disconnected.");

                Disconnect();
                return;
            }

            if (_player->m_position.distance2DSq(_player->m_sentTeleportPosition) > 625.0f)
            {
                sCheatLog.writefromsession(this, "Teleport exploit detected: position mismatch. DistanceSq: {:.2f}. Player disconnected.", _player->m_position.distance2DSq(_player->m_sentTeleportPosition));

                Disconnect();
                return;
            }
        }

        _player->setTransferStatus(TRANSFER_NONE);
        _player->speedCheatReset();

        for (const auto& summon : _player->getSummonInterface()->getSummons())
        {
            if (!summon->isTotem())
                summon->SetPosition(_player->GetPositionX() + 2, _player->GetPositionY() + 2, _player->GetPositionZ(), AscEmu::Math::PiF);
        }

        if (_player->m_sentTeleportPosition.x != 999999.0f)
        {
            _player->m_position = _player->m_sentTeleportPosition;
            _player->m_sentTeleportPosition.changeCoords({ 999999.0f, 999999.0f, 999999.0f });
        }
    }
}

void WorldSession::handleMoveNotActiveMoverOpcode(WorldPacket& recvPacket)
{
    WoWGuid guid;
    recvPacket >> guid;

    if (guid == m_MoverWoWGuid)
        return;

    if (guid != uint64_t(0) && guid == _player->getCharmGuid())
        m_MoverWoWGuid = guid;
    else
        m_MoverWoWGuid.init(_player->getGuid());
}
