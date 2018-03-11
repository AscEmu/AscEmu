/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "Player.h"
#include "Server/Packets/Opcode.h"
#include "Chat/ChatDefines.hpp"
#include "Server/World.h"
#include "Spell/Spell.h"
#include "Spell/SpellMgr.h"
#include "Spell/SpellFailure.h"
#include "Map/MapMgr.h"
#include "Data/WoWPlayer.h"
#include "Management/Battleground/Battleground.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Data
uint64_t Player::getDuelArbiter() const { return playerData()->duel_arbiter; }
void Player::setDuelArbiter(uint64_t guid) { write(playerData()->duel_arbiter, guid); }

uint32_t Player::getPlayerFlags() const { return playerData()->player_flags; }
void Player::setPlayerFlags(uint32_t flags) { write(playerData()->player_flags, flags); }
void Player::addPlayerFlags(uint32_t flags) { setPlayerFlags(getPlayerFlags() | flags); }
void Player::removePlayerFlags(uint32_t flags) { setPlayerFlags(getPlayerFlags() & ~flags); }
bool Player::hasPlayerFlags(uint32_t flags) const { return (getPlayerFlags() & flags) != 0; }

//bytes begin
uint32_t Player::getPlayerBytes() const { return playerData()->player_bytes.raw; }
void Player::setPlayerBytes(uint32_t bytes) { write(playerData()->player_bytes.raw, bytes); }

uint8_t Player::getSkinColor() const { return playerData()->player_bytes.s.skin_color; }
void Player::setSkinColor(uint8_t color) { write(playerData()->player_bytes.s.skin_color, color); }

uint8_t Player::getFace() const { return playerData()->player_bytes.s.face; }
void Player::setFace(uint8_t face) { write(playerData()->player_bytes.s.face, face); }

uint8_t Player::getHairStyle() const { return playerData()->player_bytes.s.hair_style; }
void Player::setHairStyle(uint8_t style) { write(playerData()->player_bytes.s.hair_style, style); }

uint8_t Player::getHairColor() const { return playerData()->player_bytes.s.hair_color; }
void Player::setHairColor(uint8_t color) { write(playerData()->player_bytes.s.hair_color, color); }
//bytes end

//bytes2 begin
uint32_t Player::getPlayerBytes2() const { return playerData()->player_bytes_2.raw; }
void Player::setPlayerBytes2(uint32_t bytes2) { write(playerData()->player_bytes_2.raw, bytes2); }

uint8_t Player::getFacialFeatures() const { return playerData()->player_bytes_2.s.facial_hair; }
void Player::setFacialFeatures(uint8_t feature) { write(playerData()->player_bytes_2.s.facial_hair, feature); }

//unk1

uint8_t Player::getBankSlots() const { return playerData()->player_bytes_2.s.bank_slots; }
void Player::setBankSlots(uint8_t slots) { write(playerData()->player_bytes_2.s.bank_slots, slots); }

uint8_t Player::getRestState() const { return playerData()->player_bytes_2.s.rest_state; }
void Player::setRestState(uint8_t state) { write(playerData()->player_bytes_2.s.rest_state, state); }
//bytes2 end

//bytes3 begin
uint32_t Player::getPlayerBytes3() const { return playerData()->player_bytes_3.raw; }
void Player::setPlayerBytes3(uint32_t bytes3) { write(playerData()->player_bytes_3.raw, bytes3); }

uint8_t Player::getPlayerGender() const { return playerData()->player_bytes_3.s.gender; }
void Player::setPlayerGender(uint8_t gender) { write(playerData()->player_bytes_3.s.gender, gender); }

uint16_t Player::getDrunkValue() const { return playerData()->player_bytes_3.s.drunk_value; }
void Player::setDrunkValue(uint16_t value) { write(playerData()->player_bytes_3.s.drunk_value, value); }

uint8_t Player::getPvpRank() const { return playerData()->player_bytes_3.s.pvp_rank; }
void Player::setPvpRank(uint8_t rank) { write(playerData()->player_bytes_3.s.pvp_rank, rank); }
//bytes3 end

uint32_t Player::getDuelTeam() const { return playerData()->duel_team; }
void Player::setDuelTeam(uint32_t team) { write(playerData()->duel_team, team); }

uint32_t Player::getXp() const { return playerData()->xp; }
void Player::setXp(uint32_t xp) { write(playerData()->xp, xp); }

uint32_t Player::getNextLevelXp() { return playerData()->next_level_xp; }
void Player::setNextLevelXp(uint32_t xp) { write(playerData()->next_level_xp, xp); }

void Player::setAttackPowerMultiplier(float val) { write(playerData()->attack_power_multiplier, val); }

void Player::setRangedAttackPowerMultiplier(float val) { write(playerData()->ranged_attack_power_multiplier, val); }

void Player::setExploredZone(uint32_t idx, uint32_t data)
{
    ARCEMU_ASSERT(idx < WOWPLAYER_EXPLORED_ZONES_COUNT)

    write(playerData()->explored_zones[idx], data);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Movement

#if VERSION_STRING != Cata
void Player::sendForceMovePacket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data(50);

    switch (speed_type)
    {
        case TYPE_WALK:
            data.Initialize(SMSG_FORCE_WALK_SPEED_CHANGE);
            break;
        case TYPE_RUN:
            data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
            break;
        case TYPE_RUN_BACK:
            data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE);
            break;
        case TYPE_SWIM:
            data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE);
            break;
        case TYPE_SWIM_BACK:
            data.Initialize(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE);
            break;
        case TYPE_TURN_RATE:
            data.Initialize(SMSG_FORCE_TURN_RATE_CHANGE);
            break;
        case TYPE_FLY:
            data.Initialize(SMSG_FORCE_FLIGHT_SPEED_CHANGE);
            break;
        case TYPE_FLY_BACK:
            data.Initialize(SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE);
            break;
#if VERSION_STRING > TBC
        case TYPE_PITCH_RATE:
            data.Initialize(SMSG_FORCE_PITCH_RATE_CHANGE);
            break;
#endif
    }

    data << GetNewGUID();
    data << m_speedChangeCounter++;

    if (speed_type == TYPE_RUN)
        data << uint8_t(1);

    data << float(speed);

    SendMessageToSet(&data, true);
}

void Player::sendMoveSetSpeedPaket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data(45);

    switch (speed_type)
    {
#if VERSION_STRING >= WotLK
        case TYPE_WALK:
            data.Initialize(MSG_MOVE_SET_WALK_SPEED);
            break;
        case TYPE_RUN:
            data.Initialize(MSG_MOVE_SET_RUN_SPEED);
            break;
        case TYPE_RUN_BACK:
            data.Initialize(MSG_MOVE_SET_RUN_BACK_SPEED);
            break;
        case TYPE_SWIM:
            data.Initialize(MSG_MOVE_SET_SWIM_SPEED);
            break;
        case TYPE_SWIM_BACK:
            data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED);
            break;
        case TYPE_TURN_RATE:
            data.Initialize(MSG_MOVE_SET_TURN_RATE);
            break;
        case TYPE_FLY:
            data.Initialize(MSG_MOVE_SET_FLIGHT_SPEED);
            break;
        case TYPE_FLY_BACK:
            data.Initialize(MSG_MOVE_SET_FLIGHT_BACK_SPEED);
            break;
        case TYPE_PITCH_RATE:
            data.Initialize(MSG_MOVE_SET_PITCH_RATE);
            break;
#else
        case TYPE_RUN:
            data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
            break;
        case TYPE_RUN_BACK:
            data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE);
            break;
        case TYPE_SWIM:
            data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE);
            break;
        case TYPE_SWIM_BACK:
            data.Initialize(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE);
            break;
        case TYPE_FLY:
            data.Initialize(SMSG_FORCE_FLIGHT_SPEED_CHANGE);
            break;
        case TYPE_FLY_BACK:
        case TYPE_TURN_RATE:
        case TYPE_WALK:
        case TYPE_PITCH_RATE:
            break;
#endif
    }

    data << GetNewGUID();
#ifdef AE_TBC
    // TODO : Move to own file
    if (speed_type != TYPE_SWIM_BACK)
    {
        data << m_speedChangeCounter++;
        if (speed_type == TYPE_RUN)
            data << uint8_t(1);
    }
    else
    {
        data << uint32_t(0) << uint8_t(0) << uint32_t(Util::getMSTime())
            << GetPosition() << m_position.o << uint32_t(0);
    }
#endif

#ifndef AE_TBC
    BuildMovementPacket(&data);
#endif
    data << float(speed);

    SendMessageToSet(&data, true);
}

void Player::handleFall(MovementInfo const& /*movement_info*/)
{
}

bool Player::isPlayerJumping(MovementInfo const& /*movement_info*/, uint16_t /*opcode*/)
{
    return false;
}

void Player::handleBreathing(MovementInfo const& /*movement_info*/, WorldSession* /*session*/)
{
}

bool Player::isSpellFitByClassAndRace(uint32_t /*spell_id*/)
{
    return false;
}

void Player::sendAuctionCommandResult(Auction* /*auction*/, uint32_t /*action*/, uint32_t /*errorCode*/, uint32_t /*bidError*/)
{
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Spells
void Player::updateAutoRepeatSpell()
{
    // Get the autorepeat spell
    Spell* autoRepeatSpell = getCurrentSpell(CURRENT_AUTOREPEAT_SPELL);

    // Check is target valid
    // Note for self: remove this check when new Spell::canCast() is ready -Appled
    Unit* target = GetMapMgr()->GetUnit(autoRepeatSpell->m_targets.m_unitTarget);
    if (target == nullptr)
    {
        m_AutoShotAttackTimer = 0;
        interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
        return;
    }

    // If player is moving or casting a spell, interrupt wand casting and delay auto shot
    const bool isAutoShot = autoRepeatSpell->GetSpellInfo()->getId() == 75;
    if (m_isMoving || isCastingNonMeleeSpell(true, false, true, isAutoShot))
    {
        if (!isAutoShot)
        {
            interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
        }
        m_FirstCastAutoRepeat = true;
        return;
    }

    // Apply delay to wand shooting
    if (m_FirstCastAutoRepeat && m_AutoShotAttackTimer < 500 && !isAutoShot)
    {
        m_AutoShotAttackTimer = 500;
    }
    m_FirstCastAutoRepeat = false;

    if (m_AutoShotAttackTimer == 0)
    {
        // TODO: implement ::CanShootRangedWeapon() into new Spell::canCast()
        // also currently if target gets too far away, your autorepeat spell will get interrupted
        // it's related most likely to ::CanShootRangedWeapon()
        const int32_t canCastAutoRepeatSpell = CanShootRangedWeapon(autoRepeatSpell->GetSpellInfo()->getId(), target, isAutoShot);
        if (canCastAutoRepeatSpell != SPELL_CANCAST_OK)
        {
            if (!isAutoShot)
            {
                interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
            }
            return;
        }

        m_AutoShotAttackTimer = getUInt32Value(UNIT_FIELD_RANGEDATTACKTIME);

        // Cast the spell with triggered flag
        Spell* newAutoRepeatSpell = sSpellFactoryMgr.NewSpell(this, autoRepeatSpell->GetSpellInfo(), true, nullptr);
        newAutoRepeatSpell->prepare(&(autoRepeatSpell->m_targets));
    }
}

bool Player::isTransferPending() const
{
    return GetPlayerStatus() == TRANSFER_PENDING;
}

void Player::toggleAfk()
{
    if (hasPlayerFlags(PLAYER_FLAG_AFK))
    {
        removePlayerFlags(PLAYER_FLAG_AFK);
        if (worldConfig.getKickAFKPlayerTime())
            sEventMgr.RemoveEvents(this, EVENT_PLAYER_SOFT_DISCONNECT);
    }
    else
    {
        addPlayerFlags(PLAYER_FLAG_AFK);

        if (m_bg)
            m_bg->RemovePlayer(this, false);

        if (worldConfig.getKickAFKPlayerTime())
            sEventMgr.AddEvent(this, &Player::SoftDisconnect, EVENT_PLAYER_SOFT_DISCONNECT,
                               worldConfig.getKickAFKPlayerTime(), 1, 0);
    }
}

void Player::toggleDnd()
{
    if (hasPlayerFlags(PLAYER_FLAG_DND))
        removePlayerFlags(PLAYER_FLAG_DND);
    else
        addPlayerFlags(PLAYER_FLAG_DND);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Messages
void Player::sendReportToGmMessage(std::string playerName, std::string damageLog)
{
    std::string gm_ann(MSG_COLOR_GREEN);

    gm_ann += "|HPlayer:";
    gm_ann += playerName;
    gm_ann += "|h[";
    gm_ann += playerName;
    gm_ann += "]|h: ";
    gm_ann += MSG_COLOR_YELLOW;
    gm_ann += damageLog;

    sWorld.sendMessageToOnlineGms(gm_ann.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
bool Player::isGMFlagSet()
{
    return hasPlayerFlags(PLAYER_FLAG_GM);
}

void Player::sendMovie(uint32_t movieId)
{
#if VERSION_STRING > TBC
    WorldPacket data(SMSG_TRIGGER_MOVIE, 4);
    data << uint32_t(movieId);
    m_session->SendPacket(&data);
#endif
}

uint8 Player::GetPlayerStatus() const { return m_status; }

PlayerSpec& Player::getActiveSpec()
{
#ifdef FT_DUAL_SPEC
    return m_specs[m_talentActiveSpec];
#else
    return m_spec;
#endif
}
