/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
#include "Map/Cells/CellHandlerDefines.hpp"
#include "Objects/GameObject.h"
#include "Server/Warden/SpeedDetector.h"
#include "Management/ObjectMgr.hpp"
#include "Management/TaxiMgr.hpp"
#include "Server/Packets/CmsgWorldTeleport.h"
#include "Server/Packets/SmsgMountspecialAnim.h"
#include "Server/Packets/MsgMoveTeleportAck.h"
#include "Server/Packets/SmsgNewWorld.h"
#include "Objects/Units/Creatures/Pet.h"
#include "Server/OpcodeTable.hpp"
#include "Spell/Definitions/AuraInterruptFlags.hpp"
#include "Objects/Transporter.hpp"
#include "Server/World.h"
#include "Server/WorldSessionLog.hpp"
#include "Spell/Spell.hpp"
#include "Storage/WDB/WDBStructures.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleSetActiveMoverOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING < Cata
    CmsgSetActiveMover srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.guid == m_MoverWoWGuid.getRawGuid())
        return;

#if VERSION_STRING > TBC
    if (_player->getCharmGuid() != srlPacket.guid.getRawGuid() || _player->getGuid() != srlPacket.guid.getRawGuid())
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
        m_MoverWoWGuid.Init(_player->getGuid());
    else
        m_MoverWoWGuid = srlPacket.guid;
#endif
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
    if (_player->isOnTaxi() && _player->obj_movement_info.transport_guid == 0 && !_player->getSession()->hasPermissions())
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
    /// Set up some vars to simplify code
    // Zyres: save the opcode here for better handling
    const auto opcode = sOpcodeTables.getInternalIdForHex(recvData.GetOpcode());

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
        out_of_bounds = out_of_bounds || sessionMovementInfo.position.y < Map::Terrain::_minY;
        out_of_bounds = out_of_bounds || sessionMovementInfo.position.y > Map::Terrain::_maxY;
        out_of_bounds = out_of_bounds || sessionMovementInfo.position.x > Map::Terrain::_maxX;
        out_of_bounds = out_of_bounds || sessionMovementInfo.position.x > Map::Terrain::_maxX;

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
                if (Transporter* transport = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(movementInfo.transport_guid)))
                {
                    transport->AddPassenger(mover->ToPlayer());

                    /* set variables */
                    mover->obj_movement_info.transport_time = sessionMovementInfo.transport_time;
                    mover->obj_movement_info.transport_position.x = sessionMovementInfo.transport_position.x;
                    mover->obj_movement_info.transport_position.y = sessionMovementInfo.transport_position.y;
                    mover->obj_movement_info.transport_position.z = sessionMovementInfo.transport_position.z;
                    mover->obj_movement_info.transport_position.o = sessionMovementInfo.transport_position.o;
                }
            }
            else if (mover->GetTransport() != sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(movementInfo.transport_guid)))
            {
                mover->GetTransport()->RemovePassenger(mover);
                if (Transporter* transport = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(movementInfo.transport_guid)))
                {
                    transport->AddPassenger(mover->ToPlayer());

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
        mover->GetTransport()->RemovePassenger(mover);
        movementInfo.clearTransportData();
    }
#else
    if (mover->isPlayer())
    {
        // if we boarded a transport, add us to it
        if (movementInfo.transport_guid)
        {
            if (!mover->GetTransport())
            {
                if (Transporter* transport = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(movementInfo.transport_guid)))
                {
                    transport->AddPassenger(mover->ToPlayer());

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
                mover->GetTransport()->RemovePassenger(mover);
                if (Transporter* transport = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(movementInfo.transport_guid)))
                {
                    transport->AddPassenger(mover->ToPlayer());

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
            mover->GetTransport()->RemovePassenger(mover);
            movementInfo.clearTransportData();
            mover->obj_movement_info.clearTransportData();
        }
    }
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Breathing & Underwaterstate
    _player->handleBreathing(sessionMovementInfo, this);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Aura Interruption
    _player->handleAuraInterruptForMovementFlags(sessionMovementInfo);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// send our move to all inrange players

#if VERSION_STRING >= Cata

    WorldPacket data(SMSG_PLAYER_MOVE, recvData.size());
    data << sessionMovementInfo;
    mover->sendMessageToSet(&data, false);

#elif VERSION_STRING == WotLK

    WorldPacket data(opcode, recvData.size());
    data << sessionMovementInfo;
    mover->sendMessageToSet(&data, _player);

#else

    // Zyres NOTE: versions older than WotLK do not send us the guid within the movement packet (needed for the packet send to other players)
    // but we should already received the active mover
    sessionMovementInfo.guid = m_MoverWoWGuid;

    WorldPacket data(opcode, recvData.size());
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

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Update our Server position
    mover->SetPosition(sessionMovementInfo.position.x, sessionMovementInfo.position.y, sessionMovementInfo.position.z, sessionMovementInfo.position.o);
}

void WorldSession::handleAcknowledgementOpcodes(WorldPacket& recvPacket)
{
    sLogger.debug("Opcode {} ({}) received. This opcode is not known/implemented right now!",
        sOpcodeTables.getNameForInternalId(recvPacket.GetOpcode()), recvPacket.GetOpcode());

    recvPacket.rfinish();
}

void WorldSession::handleForceSpeedChangeAck(WorldPacket& recvPacket)
{
#if VERSION_STRING < Cata
    /* extract packet */
    uint32_t unk1;
    float  newspeed;
    Unit* mover = _player->m_controledUnit;

    // continue parse packet

    recvPacket >> unk1;                          // counter or moveEvent

    MovementInfo movementInfo;
    recvPacket >> movementInfo;

    // now can skip not our packet
    // TODO: following statement is always true -Appled
    if (movementInfo.getGuid() != mover->getGuid())
    {
        recvPacket.rfinish();                   // prevent warnings spam
        return;
    }

    recvPacket >> newspeed;
    /*----------------*/

    // client ACK send one packet for mounted/run case and need skip all except last from its
    // in other cases anti-cheat check can be fail in false case
    UnitSpeedType move_type;
    UnitSpeedType force_move_type;

    static char const* move_type_name[MAX_SPEED_TYPE] = { "Walk", "Run", "RunBack", "Swim", "SwimBack", "TurnRate", "Flight", "FlightBack", "PitchRate" };

    const auto opcode = sOpcodeTables.getInternalIdForHex(recvPacket.GetOpcode());
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
            sLogger.failure("WorldSession::handleForceSpeedChangeAck: Unknown move type opcode: {}", recvPacket.GetOpcode());
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

    if (!_player->GetTransport() && std::fabs(_player->getSpeedRate(move_type, false) - newspeed) > 0.01f)
    {
        if (_player->getSpeedRate(move_type, false) > newspeed)         // must be greater - just correct
        {
            _player->setSpeedRate(move_type, _player->getSpeedRate(move_type, false), false);
        }
        else                                                            // must be lesser - cheating
        {
            // handle something here
        }
    }
#else // todo fix for cata / mop
    sLogger.debug("Opcode {} ({}) received. This opcode is not known/implemented right now!",
        sOpcodeTables.getNameForInternalId(recvPacket.GetOpcode()), recvPacket.GetOpcode());

    recvPacket.rfinish();
#endif
}

void WorldSession::handleWorldTeleportOpcode(WorldPacket& recvPacket)
{
    CmsgWorldTeleport srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (!HasGMPermissions())
    {
        SendNotification("You do not have permission to use this function.");
        return;
    }

    _player->safeTeleport(srlPacket.mapId, 0, srlPacket.location);
}

void WorldSession::handleMountSpecialAnimOpcode(WorldPacket& /*recvPacket*/)
{
    _player->sendMessageToSet(SmsgMountspecialAnim(_player->getGuid()).serialise().get(), true);
}

void WorldSession::handleMoveWorldportAckOpcode(WorldPacket& /*recvPacket*/)
{
    _player->setTransferStatus(TRANSFER_NONE);
    if (_player->IsInWorld())
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_MOVE_WORLDPORT_ACK");

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
        _player->m_teleportState = 2;
        _player->AddToWorld();
    }

    _player->resetTimeSync();
    _player->sendTimeSync();
}

void WorldSession::handleMoveTeleportAckOpcode(WorldPacket& recvPacket)
{
    MsgMoveTeleportAck srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_MOVE_TELEPORT_ACK.");

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
        _player->speedCheatReset();

        for (const auto& summon : _player->getSummonInterface()->getSummons())
        {
            if (!summon->isTotem())
                summon->SetPosition(_player->GetPositionX() + 2, _player->GetPositionY() + 2, _player->GetPositionZ(), M_PI_FLOAT);
        }

        if (_player->m_sentTeleportPosition.x != 999999.0f)
        {
            _player->m_position = _player->m_sentTeleportPosition;
            _player->m_sentTeleportPosition.ChangeCoords({ 999999.0f, 999999.0f, 999999.0f });
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
        m_MoverWoWGuid.Init(_player->getGuid());
}
