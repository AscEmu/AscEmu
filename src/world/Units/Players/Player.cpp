/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "Player.h"
#include "Server/Packets/Opcode.h"
#include "Chat/ChatDefines.hpp"
#include "Server/World.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/Definitions/Spec.h"
#include "Spell/Definitions/SpellIsFlags.h"
#include "Spell/Spell.h"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/SpellFailure.h"
#include "Spell/SpellMgr.h"
#include "Map/MapMgr.h"
#include "Data/WoWPlayer.h"
#include "Management/Battleground/Battleground.h"
#include "Objects/GameObject.h"
#include "Units/Creatures/Pet.h"
#include "Server/Packets/SmsgNewWorld.h"
#include "Objects/ObjectMgr.h"
#include "Management/GuildMgr.h"
#include "Management/ItemInterface.h"
#include "Server/Packets/MsgTalentWipeConfirm.h"
#include "Server/Packets/SmsgPetUnlearnConfirm.h"
#include "Server/Packets/MsgSetDungeonDifficulty.h"
#include "Server/Packets/MsgSetRaidDifficulty.h"
#include "Server/Packets/SmsgInstanceDifficulty.h"
#include "Server/Packets/SmsgCrossedInebriationThreshold.h"
#include "Server/Packets/SmsgSetProficiency.h"
#include "Server/Packets/SmsgPartyKillLog.h"
#include "Server/Packets/SmsgEquipmentSetUseResult.h"
#include "Server/Packets/SmsgTotemCreated.h"
#include "Server/Packets/SmsgGossipPoi.h"
#include "Server/Packets/SmsgStopMirrorTimer.h"
#include "Server/Packets/SmsgMeetingstoneSetQueue.h"
#include "Server/Packets/SmsgPlayObjectSound.h"
#include "Server/Packets/SmsgPlaySound.h"
#include "Server/Packets/SmsgExplorationExperience.h"
#include "Server/Packets/SmsgCooldownEvent.h"
#include "Server/Packets/SmsgSetFlatSpellModifier.h"
#include "Server/Packets/SmsgSetPctSpellModifier.h"
#include "Server/Packets/SmsgLoginVerifyWorld.h"
#include "Server/Packets/SmsgMountResult.h"
#include "Server/Packets/SmsgDismountResult.h"
#include "Server/Packets/SmsgLogXpGain.h"
#include "Server/Packets/SmsgCastFailed.h"
#include "Server/Packets/SmsgLevelupInfo.h"
#include "Server/Packets/SmsgItemPushResult.h"
#include "Server/Packets/SmsgClientControlUpdate.h"
#include "Server/Packets/SmsgGuildEvent.h"
#include "Server/Packets/SmsgDestoyObject.h"
#include "Storage/MySQLDataStore.hpp"
#include "Spell/Definitions/AuraInterruptFlags.h"

using namespace AscEmu::Packets;

void Player::resendSpeed()
{
    if (resend_speed)
    {
        setSpeedForType(TYPE_RUN, getSpeedForType(TYPE_RUN));
        setSpeedForType(TYPE_FLY, getSpeedForType(TYPE_FLY));
        resend_speed = false;
    }
}

void Player::ProcessPendingUpdates()
{
    m_updateMgr.processPendingUpdates();
}

UpdateManager & Player::getUpdateMgr()
{
    return m_updateMgr;
}

SplineManager & Player::getSplineMgr()
{
    return m_splineMgr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Data
uint64_t Player::getDuelArbiter() const { return playerData()->duel_arbiter; }
void Player::setDuelArbiter(uint64_t guid) { write(playerData()->duel_arbiter, guid); }

uint32_t Player::getPlayerFlags() const { return playerData()->player_flags; }
void Player::setPlayerFlags(uint32_t flags) { write(playerData()->player_flags, flags); }
void Player::addPlayerFlags(uint32_t flags) { setPlayerFlags(getPlayerFlags() | flags); }
void Player::removePlayerFlags(uint32_t flags) { setPlayerFlags(getPlayerFlags() & ~flags); }
bool Player::hasPlayerFlags(uint32_t flags) const { return (getPlayerFlags() & flags) != 0; }

uint32_t Player::getGuildId() const
{
#if VERSION_STRING < Cata
    return playerData()->guild_id;
#else
    return static_cast<uint32_t>(objectData()->data);
#endif
}
void Player::setGuildId(uint32_t guildId)
{
#if VERSION_STRING < Cata
    write(playerData()->guild_id, guildId);
#else
    write(objectData()->data, MAKE_NEW_GUID(guildId, 0, HIGHGUID_TYPE_GUILD));

    ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_GUILD_LVL_ENABLED, guildId != 0);
    setUInt16Value(OBJECT_FIELD_TYPE, 1, guildId != 0);
#endif
}

uint32_t Player::getGuildRank() const { return playerData()->guild_rank; }
void Player::setGuildRank(uint32_t guildRank) { write(playerData()->guild_rank, guildRank); }

#if VERSION_STRING >= Cata
uint32_t Player::getGuildLevel() const { return playerData()->guild_level; }
void Player::setGuildLevel(uint32_t guildLevel) { write(playerData()->guild_level, guildLevel); }
#endif

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

uint32_t Player::getGuildTimestamp() const { return playerData()->guild_timestamp; }
void Player::setGuildTimestamp(uint32_t timestamp) { write(playerData()->guild_timestamp, timestamp); }

uint64_t Player::getFarsightGuid() const { return playerData()->farsight_guid; }
void Player::setFarsightGuid(uint64_t farsightGuid) { write(playerData()->farsight_guid, farsightGuid); }

#if VERSION_STRING > Classic
uint32_t Player::getChosenTitle() const { return playerData()->chosen_title; }
void Player::setChosenTitle(uint32_t title) { write(playerData()->chosen_title, title); }
#endif

uint32_t Player::getXp() const { return playerData()->xp; }
void Player::setXp(uint32_t xp) { write(playerData()->xp, xp); }

uint32_t Player::getNextLevelXp() const { return playerData()->next_level_xp; }
void Player::setNextLevelXp(uint32_t xp) { write(playerData()->next_level_xp, xp); }

#if VERSION_STRING <= WotLK
uint32_t Player::getFreeTalentPoints() const { return playerData()->character_points_1; }
void Player::setFreeTalentPoints(uint32_t points) { write(playerData()->character_points_1, points); }

uint32_t Player::getFreePrimaryProfessionPoints() const { return playerData()->character_points_2; }
void Player::setFreePrimaryProfessionPoints(uint32_t points) { write(playerData()->character_points_2, points); }
#else
uint32_t Player::getFreeTalentPoints() const { return m_specs[m_talentActiveSpec].GetTP(); }

uint32_t Player::getFreePrimaryProfessionPoints() const { return playerData()->character_points_1; }
void Player::setFreePrimaryProfessionPoints(uint32_t points) { write(playerData()->character_points_1, points); }
#endif

void Player::setExploredZone(uint32_t idx, uint32_t data)
{
    ARCEMU_ASSERT(idx < WOWPLAYER_EXPLORED_ZONES_COUNT)

    write(playerData()->explored_zones[idx], data);
}

uint32_t Player::getSelfResurrectSpell() const { return playerData()->self_resurrection_spell; }
void Player::setSelfResurrectSpell(uint32_t spell) { write(playerData()->self_resurrection_spell, spell); }

uint32_t Player::getWatchedFaction() const { return playerData()->field_watched_faction_idx; }
void Player::setWatchedFaction(uint32_t factionId) { write(playerData()->field_watched_faction_idx, factionId); }

uint32_t Player::getMaxLevel() const
{
#if VERSION_STRING > Classic
    return playerData()->field_max_level;
#else
    return max_level;
#endif
}

void Player::setMaxLevel(uint32_t level)
{
#if VERSION_STRING > Classic
    write(playerData()->field_max_level, level);
#else
    max_level = level;
#endif 
}

#if VERSION_STRING < Cata
uint32_t Player::getCoinage() const { return playerData()->field_coinage; }
void Player::setCoinage(uint32_t coinage) { write(playerData()->field_coinage, coinage); }
bool Player::hasEnoughCoinage(uint32_t coinage) const { return getCoinage() >= coinage; }
void Player::modCoinage(int32_t coinage)
{
    setCoinage(getCoinage() + coinage);
}
#else
uint64_t Player::getCoinage() const { return playerData()->field_coinage; }
void Player::setCoinage(uint64_t coinage) { write(playerData()->field_coinage, coinage); }
bool Player::hasEnoughCoinage(uint64_t coinage) const { return getCoinage() >= coinage; }
void Player::modCoinage(int64_t coinage)
{
    setCoinage(getCoinage() + coinage);
}
#endif

uint32_t Player::getModDamageDonePositive(uint16_t school) const { return playerData()->field_mod_damage_done_positive[school]; }
void Player::modModDamageDonePositive(uint16_t school, uint32_t value)
{
    uint32_t damageDone = getModDamageDonePositive(school);
    damageDone += value;
    write(playerData()->field_mod_damage_done_positive[school], damageDone);
}

uint32_t Player::getModDamageDoneNegative(uint16_t school) const { return playerData()->field_mod_damage_done_negative[school]; }
void Player::modModDamageDoneNegative(uint16_t school, uint32_t value)
{
    uint32_t damageDone = getModDamageDoneNegative(school);
    damageDone += value;
    write(playerData()->field_mod_damage_done_negative[school], damageDone);
}

#if VERSION_STRING > Classic
uint32_t Player::getModHealingDone() const { return playerData()->field_mod_healing_done; }
void Player::modModHealingDone(uint32_t value)
{
    uint32_t healingDone = getModHealingDone();
    healingDone += value;
    write(playerData()->field_mod_healing_done, healingDone);
}
#endif

float Player::getModDamageDonePct(uint8_t shool) const { return playerData()->field_mod_damage_done_pct[shool]; }
void Player::setModDamageDonePct(float damagePct, uint8_t shool) { write(playerData()->field_mod_damage_done_pct[shool], damagePct); }

uint32_t Player::getPlayerFieldBytes() const { return playerData()->player_field_bytes.raw; }
void Player::setPlayerFieldBytes(uint32_t bytes) { write(playerData()->player_field_bytes.raw, bytes); }

uint8_t Player::getActionBarId() const { return playerData()->player_field_bytes.s.actionBarId; }
void Player::setActionBarId(uint8_t actionBarId) { write(playerData()->player_field_bytes.s.actionBarId, actionBarId); }

#if VERSION_STRING < Cata
uint32_t Player::getAmmoId() const { return playerData()->ammo_id; }
void Player::setAmmoId(uint32_t id) { write(playerData()->ammo_id, id); }
#endif

#if VERSION_STRING != Mop
uint32_t Player::getPlayerFieldBytes2() const { return playerData()->player_field_bytes_2.raw; }
void Player::setPlayerFieldBytes2(uint32_t bytes) { write(playerData()->player_field_bytes_2.raw, bytes); }
#endif

#if VERSION_STRING > TBC
uint32_t Player::getGlyph(uint16_t slot) const { return playerData()->field_glyphs[slot]; }
void Player::setGlyph(uint16_t slot, uint32_t glyph) { write(playerData()->field_glyphs[slot], glyph); }
#endif

#if VERSION_STRING > TBC
uint32_t Player::getGlyphsEnabled() const { return playerData()->glyphs_enabled; }
void Player::setGlyphsEnabled(uint32_t glyphs) { write(playerData()->glyphs_enabled, glyphs); }
#endif

#if VERSION_STRING > Classic
#if VERSION_STRING < Cata
uint32_t Player::getArenaCurrency() const { return playerData()->field_arena_currency; }
void Player::setArenaCurrency(uint32_t amount) { write(playerData()->field_arena_currency, amount); }
void Player::modArenaCurrency(int32_t value)
{
    setArenaCurrency(getArenaCurrency() + value);
}
#endif
#endif

#if VERSION_STRING >= WotLK
uint32_t Player::getNoReagentCost(uint8_t index) const { return playerData()->no_reagent_cost[index]; }
void Player::setNoReagentCost(uint8_t index, uint32_t value) { write(playerData()->no_reagent_cost[index], value); }
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Movement

#if VERSION_STRING < Cata
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
#else
void Player::sendForceMovePacket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data(60);
    switch (speed_type)
    {
        case TYPE_WALK:
        {
            data.Initialize(SMSG_FORCE_WALK_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_WALK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_RUN:
        {
            data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_RUN_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_RUN_BACK:
        {
            data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_RUN_BACK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_SWIM:
        {
            data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_SWIM_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_SWIM_BACK:
        {
            data.Initialize(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_SWIM_BACK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_TURN_RATE:
        {
            data.Initialize(SMSG_FORCE_TURN_RATE_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_TURN_RATE_CHANGE, speed);
            break;
        }
        case TYPE_FLY:
        {
            data.Initialize(SMSG_FORCE_FLIGHT_SPEED_CHANGE);
            movement_info.writeMovementInfo(data, SMSG_FORCE_FLIGHT_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_FLY_BACK:
        {
            data.Initialize(SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_PITCH_RATE:
        {
            data.Initialize(SMSG_FORCE_PITCH_RATE_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_PITCH_RATE_CHANGE, speed);
            break;
        }
    }

    SendMessageToSet(&data, true);
}

void Player::sendMoveSetSpeedPaket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data;
    ObjectGuid guid = getGuid();

    switch (speed_type)
    {
        case TYPE_WALK:
        {
            data.Initialize(MSG_MOVE_SET_WALK_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_WALK_SPEED, speed);
            break;
        }
        case TYPE_RUN:
        {
            data.Initialize(MSG_MOVE_SET_RUN_SPEED, 1 + 8 + 4 + 4);
#if VERSION_STRING == Mop
            data.writeBit(guid[1]);
            data.writeBit(guid[7]);
            data.writeBit(guid[4]);
            data.writeBit(guid[2]);
            data.writeBit(guid[5]);
            data.writeBit(guid[3]);
            data.writeBit(guid[6]);
            data.writeBit(guid[0]);

            data.flushBits();

            data.WriteByteSeq(guid[1]);

            data << uint32_t(0);

            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[0]);

            data << float(speed);

            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[5]);
#else
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_RUN_SPEED, speed);
#endif
            break;
        }
        case TYPE_RUN_BACK:
        {
            data.Initialize(MSG_MOVE_SET_RUN_BACK_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_RUN_BACK_SPEED, speed);
            break;
        }
        case TYPE_SWIM:
        {
            data.Initialize(MSG_MOVE_SET_SWIM_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_SWIM_SPEED, speed);
            break;
        }
        case TYPE_SWIM_BACK:
        {
            data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_SWIM_BACK_SPEED, speed);
            break;
        }
        case TYPE_TURN_RATE:
        {
            data.Initialize(MSG_MOVE_SET_TURN_RATE, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_TURN_RATE, speed);
            break;
        }
        case TYPE_FLY:
        {
            data.Initialize(MSG_MOVE_SET_FLIGHT_SPEED, 1 + 8 + 4 + 4);
#if VERSION_STRING == Mop
            data << float(speed);
            data << uint32_t(0);

            data.writeBit(guid[6]);
            data.writeBit(guid[5]);
            data.writeBit(guid[0]);
            data.writeBit(guid[4]);
            data.writeBit(guid[1]);
            data.writeBit(guid[7]);
            data.writeBit(guid[3]);
            data.writeBit(guid[2]);

            data.flushBits();

            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[1]);
#else
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_FLIGHT_SPEED, speed);
#endif
            break;
        }
        case TYPE_FLY_BACK:
        {
            data.Initialize(MSG_MOVE_SET_FLIGHT_BACK_SPEED, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_FLIGHT_BACK_SPEED, speed);
            break;
        }
        case TYPE_PITCH_RATE:
        {
            data.Initialize(MSG_MOVE_SET_PITCH_RATE, 1 + 8 + 4 + 4);
            movement_info.writeMovementInfo(data, MSG_MOVE_SET_PITCH_RATE, speed);
            break;
        }
    }

    SendMessageToSet(&data, true);
}


void Player::handleFall(MovementInfo const& movementInfo)
{
    if (!z_axisposition)
    {
        z_axisposition = movementInfo.getPosition()->z;
    }

    uint32 falldistance = float2int32(z_axisposition - movementInfo.getPosition()->z);
    if (z_axisposition <= movementInfo.getPosition()->z)
    {
        falldistance = 1;
    }

    if (static_cast<int>(falldistance) > m_safeFall)
    {
        falldistance -= m_safeFall;
    }
    else
    {
        falldistance = 1;
    }

    if (isAlive() && !bInvincible && (falldistance > 12) && !m_noFallDamage && ((!m_cheats.GodModeCheat && (UNIXTIME >= m_fallDisabledUntil))))
    {
        auto health_loss = static_cast<uint32_t>(getHealth() * (falldistance - 12) * 0.017f);

        if (health_loss >= getHealth())
        {
            health_loss = getHealth();
        }
        else if ((falldistance >= 65))
        {
            GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING, falldistance, GetDrunkenstateByValue(GetDrunkValue()), 0);
        }

        sendEnvironmentalDamageLogPacket(getGuid(), DAMAGE_FALL, health_loss);
        DealDamage(this, health_loss, 0, 0, 0);
    }

    z_axisposition = 0.0f;
}

bool Player::isPlayerJumping(MovementInfo const& movementInfo, uint16_t opcode)
{
    if (opcode == MSG_MOVE_FALL_LAND || movementInfo.hasMovementFlag(MOVEFLAG_SWIMMING))
    {
        jumping = false;
        return false;
    }

    if (!jumping && (opcode == MSG_MOVE_JUMP || movementInfo.hasMovementFlag(MOVEFLAG_FALLING)))
    {
        jumping = true;
        return true;
    }

    return false;
}

void Player::handleBreathing(MovementInfo const& movementInfo, WorldSession* session)
{
    if (!worldConfig.server.enableBreathing || m_cheats.FlyCheat || m_bUnlimitedBreath || !isAlive() || m_cheats.GodModeCheat)
    {
        if (m_UnderwaterState & UNDERWATERSTATE_SWIMMING)
            m_UnderwaterState &= ~UNDERWATERSTATE_SWIMMING;

        if (m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
        {
            m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;

            SendMirrorTimer(MIRROR_TYPE_BREATH, m_UnderwaterTime, m_UnderwaterMaxTime, -1);
        }

        if (session->m_bIsWLevelSet)
        {
            if ((movementInfo.getPosition()->z + m_noseLevel) > session->m_wLevel)
            {
                RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_LEAVE_WATER);

                session->m_bIsWLevelSet = false;
            }
        }

        return;
    }

    if (movementInfo.hasMovementFlag(MOVEFLAG_SWIMMING) && !(m_UnderwaterState & UNDERWATERSTATE_SWIMMING))
    {
        RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_ENTER_WATER);

        if (!session->m_bIsWLevelSet)
        {
            session->m_wLevel = movementInfo.getPosition()->z + m_noseLevel * 0.95f;
            session->m_bIsWLevelSet = true;
        }

        m_UnderwaterState |= UNDERWATERSTATE_SWIMMING;
    }

    if (!(movementInfo.hasMovementFlag(MOVEFLAG_SWIMMING)) && (movementInfo.hasMovementFlag(MOVEFLAG_NONE)) && (m_UnderwaterState & UNDERWATERSTATE_SWIMMING))
    {
        if ((movementInfo.getPosition()->z + m_noseLevel) > session->m_wLevel)
        {
            RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_LEAVE_WATER);

            session->m_bIsWLevelSet = false;

            m_UnderwaterState &= ~UNDERWATERSTATE_SWIMMING;
        }
    }

    if (m_UnderwaterState & UNDERWATERSTATE_SWIMMING && !(m_UnderwaterState & UNDERWATERSTATE_UNDERWATER))
    {
        if ((movementInfo.getPosition()->z + m_noseLevel) < session->m_wLevel)
        {
            m_UnderwaterState |= UNDERWATERSTATE_UNDERWATER;
            SendMirrorTimer(MIRROR_TYPE_BREATH, m_UnderwaterTime, m_UnderwaterMaxTime, -1);
        }
    }

    if (m_UnderwaterState & UNDERWATERSTATE_SWIMMING && m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        if ((movementInfo.getPosition()->z + m_noseLevel) > session->m_wLevel)
        {
            m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            SendMirrorTimer(MIRROR_TYPE_BREATH, m_UnderwaterTime, m_UnderwaterMaxTime, 10);
        }
    }

    if (!(m_UnderwaterState & UNDERWATERSTATE_SWIMMING) && m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        if ((movementInfo.getPosition()->z + m_noseLevel) > session->m_wLevel)
        {
            m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            SendMirrorTimer(MIRROR_TYPE_BREATH, m_UnderwaterTime, m_UnderwaterMaxTime, 10);
        }
    }
}

void Player::handleAuraInterruptForMovementFlags(MovementInfo const& movementInfo)
{
    uint32_t auraInterruptFlags = 0;
    if (movementInfo.hasMovementFlag(MOVEFLAG_MOTION_MASK))
    {
        auraInterruptFlags |= AURA_INTERRUPT_ON_MOVEMENT;
    }

    if (!(movementInfo.hasMovementFlag(MOVEFLAG_SWIMMING)) || movementInfo.hasMovementFlag(MOVEFLAG_FALLING))
    {
        auraInterruptFlags |= AURA_INTERRUPT_ON_LEAVE_WATER;
    }

    if (movementInfo.hasMovementFlag(MOVEFLAG_SWIMMING))
    {
        auraInterruptFlags |= AURA_INTERRUPT_ON_ENTER_WATER;
    }

    if ((movementInfo.hasMovementFlag(MOVEFLAG_TURNING_MASK)) || isTurning)
    {
        auraInterruptFlags |= AURA_INTERRUPT_ON_TURNING;
    }

    RemoveAurasByInterruptFlag(auraInterruptFlags);
}

static const uint32_t LanguageSkills[NUM_LANGUAGES] =
{
    0,              // UNIVERSAL          0x00
    109,            // ORCISH             0x01
    113,            // DARNASSIAN         0x02
    115,            // TAURAHE            0x03
    0,                // -                0x04
    0,                // -                0x05
    111,            // DWARVISH           0x06
    98,             // COMMON             0x07
    139,            // DEMON TONGUE       0x08
    140,            // TITAN              0x09
    137,            // THALSSIAN          0x0A
    138,            // DRACONIC           0x0B
    0,              // KALIMAG            0x0C
    313,            // GNOMISH            0x0D
    315,            // TROLL              0x0E
    0,                // -                0x0F
    0,                // -                0x10
    0,                // -                0x11
    0,                // -                0x12
    0,                // -                0x13
    0,                // -                0x14
    0,                // -                0x15
    0,                // -                0x16
    0,                // -                0x17
    0,                // -                0x18
    0,                // -                0x19
    0,                // -                0x1A
    0,                // -                0x1B
    0,                // -                0x1C
    0,                // -                0x1D
    0,                // -                0x1E
    0,                // -                0x1F
    0,                // -                0x20
    673,            // GUTTERSPEAK        0x21
    0,                // -                0x22
    759,            // DRAENEI            0x23
    0,              // ZOMBIE             0x24
    0,              // GNOMISH_BINAR      0x25
    0,              // GOBLIN_BINARY      0x26
    791,            // WORGEN             0x27
    792,            // GOBLIN             0x28
};

bool Player::hasSkilledSkill(uint32_t skill)
{
    return (m_skills.find(skill) != m_skills.end());
}

bool Player::hasLanguage(uint32_t language)
{
    return hasSkilledSkill(LanguageSkills[language]);
}

WorldPacket Player::buildChatMessagePacket(Player* targetPlayer, uint32_t type, uint32_t language, const char* message, uint64_t guid, uint8_t flag)
{
    uint32_t messageLength = (uint32_t)strlen(message) + 1;
    WorldPacket data(SMSG_MESSAGECHAT, messageLength + 60);
    data << uint8_t(type);

    if (targetPlayer->hasLanguage(language) || targetPlayer->isGMFlagSet())
    {
        data << LANG_UNIVERSAL;
    }
    else
    {
        data << language;
    }

    data << guid;
    data << uint32_t(0);

    data << targetPlayer->getGuid();

    data << messageLength;
    data << message;

    data << uint8_t(flag);
    return data;
}

void Player::sendChatPacket(uint32_t type, uint32_t language, const char* message, uint64_t guid, uint8_t flag)
{
    if (IsInWorld() == false)
        return;

    uint32_t senderPhase = GetPhase();

    for (const auto& itr : getInRangePlayersSet())
    {
        Player* p = static_cast<Player*>(itr);
        if (p && p->GetSession() && !p->Social_IsIgnoring(getGuidLow()) && (p->GetPhase() & senderPhase) != 0)
        {
            WorldPacket data = p->buildChatMessagePacket(p, type, language, message, guid, flag);
            p->SendPacket(&data);
        }
    }

    // send to self
    WorldPacket selfData = buildChatMessagePacket(this, type, language, message, guid, flag);
    this->SendPacket(&selfData);
}

void Player::sendAuctionCommandResult(Auction* auction, uint32_t action, uint32_t errorCode, uint32_t bidError)
{
    WorldPacket data(SMSG_AUCTION_COMMAND_RESULT);
    data << uint32_t(auction ? auction->Id : 0);
    data << uint32_t(action);
    data << uint32_t(errorCode);

    switch (errorCode)
    {
        case AUCTION_ERROR_NONE:
        {
            if (action == AUCTION_BID)
            {
                data << uint64_t(auction->HighestBid ? auction->GetAuctionOutBid() : 0);
            }
            break;
        }
        case AUCTION_ERROR_INVENTORY:
        {
            data << uint32_t(bidError);
            break;
        }
        case AUCTION_ERROR_HIGHER_BID:
        {
            data << uint64_t(auction->HighestBidder);
            data << uint64_t(auction->HighestBid);
            data << uint64_t(auction->HighestBid ? auction->GetAuctionOutBid() : 0);
            break;
        }
        default: break;
    }

    SendPacket(&data);
}

bool Player::isSpellFitByClassAndRace(uint32_t spell_id)
{
    auto racemask = getRaceMask();
    auto classmask = getClassMask();

    auto bounds = objmgr.GetSkillLineAbilityMapBounds(spell_id);
    if (bounds.first == bounds.second)
    {
        return true;
    }

    for (auto _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        // skip wrong race skills
        if (_spell_idx->second->race_mask && (_spell_idx->second->race_mask & racemask) == 0)
        {
            continue;
        }

        // skip wrong class skills
        if (_spell_idx->second->class_mask && (_spell_idx->second->class_mask & classmask) == 0)
        {
            continue;
        }

        return true;
    }

    return false;
}

void Player::cancelTrade(bool sendback)
{
    if (m_TradeData)
    {
        auto trade_target = m_TradeData->getTradeTarget();

        if (sendback || trade_target == nullptr)
        {
            GetSession()->sendTradeCancel();
            delete m_TradeData;
            m_TradeData = nullptr;
        }
        else
        {
            trade_target->GetSession()->sendTradeCancel();
            delete trade_target->m_TradeData;
            trade_target->m_TradeData = nullptr;
        }
    }
}

TradeData* TradeData::getTargetTradeData() const
{
    return m_tradeTarget->getTradeData();
}

Item* TradeData::getTradeItem(TradeSlots slot) const
{
    return m_items[slot] ? m_player->getItemInterface()->GetItemByGUID(m_items[slot]) : nullptr;
}

bool TradeData::hasTradeItem(uint64 item_guid) const
{
    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (m_items[i] == item_guid)
        {
            return true;
        }
    }

    return false;
}

Item* TradeData::getSpellCastItem() const
{
    return m_spellCastItem ? m_player->getItemInterface()->GetItemByGUID(m_spellCastItem) : nullptr;
}

void TradeData::setItem(TradeSlots slot, Item* item)
{
    ObjectGuid itemGuid;

    if (item)
    {
        itemGuid = item->getGuid();
    }
    else
    {
        itemGuid = ObjectGuid();
    }

    if (m_items[slot] == itemGuid)
    {
        return;
    }

    m_items[slot] = itemGuid;

    setAccepted(false);
    getTargetTradeData()->setAccepted(false);

    updateTrade();
}

void TradeData::setSpell(uint32_t spell_id, Item* cast_item /*= nullptr*/)
{
    ObjectGuid itemGuid;

    if (cast_item)
    {
        itemGuid = cast_item->getGuid();
    }
    else
    {
        itemGuid = ObjectGuid();
    }

    if (m_spell == spell_id && m_spellCastItem == itemGuid)
    {
        return;
    }

    m_spell = spell_id;
    m_spellCastItem = itemGuid;

    setAccepted(false);
    getTargetTradeData()->setAccepted(false);

    updateTrade(true); // spell info to owner
    updateTrade(false); // spell info to caster
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Basic

std::string Player::getName() const { return m_name; }
void Player::setName(std::string name) { m_name = name; }

void Player::setInitialDisplayIds(uint8_t gender, uint8_t race)
{
    if (const auto raceEntry = sChrRacesStore.LookupEntry(race))
    {
        switch (gender)
        {
            case GENDER_MALE:
                setDisplayId(raceEntry->model_male);
                setNativeDisplayId(raceEntry->model_male);
                break;
            case GENDER_FEMALE:
                setDisplayId(raceEntry->model_female);
                setNativeDisplayId(raceEntry->model_female);
                break;
            default:
                LOG_ERROR("Gender %u is not valid for Player charecters!", gender);
        }
    }
    else
    {
        LOG_ERROR("Race %u is not supported by this AEVersion (%u)", race, getAEVersion());
    }
}

bool Player::isTransferPending() const
{
    return GetPlayerStatus() == TRANSFER_PENDING;
}

bool Player::isClassMage() { return false; }
bool Player::isClassDeathKnight() { return false; }
bool Player::isClassPriest() { return false; }
bool Player::isClassRogue() { return false; }
bool Player::isClassShaman() { return false; }
bool Player::isClassHunter() { return false; }
bool Player::isClassWarlock() { return false; }
bool Player::isClassWarrior() { return false; }
bool Player::isClassPaladin() { return false; }
bool Player::isClassDruid() { return false; }

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

PlayerTeam Player::getTeam() const { return m_team == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE; }
PlayerTeam Player::getBgTeam() const { return m_bgTeam == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE; }
void Player::setTeam(uint32_t team) { m_team = team; m_bgTeam = team; }
void Player::setBgTeam(uint32_t team) { m_bgTeam = team; }

uint32_t Player::getInitialTeam() const { return myRace->team_id == 7 ? TEAM_ALLIANCE : TEAM_HORDE; }

void Player::resetTeam()
{
    m_team = myRace->team_id == 7 ? TEAM_ALLIANCE : TEAM_HORDE;
    m_bgTeam = m_team;
}

bool Player::isTeamHorde() const { return getTeam() == TEAM_HORDE; }
bool Player::isTeamAlliance() const { return getTeam() == TEAM_ALLIANCE; }

//////////////////////////////////////////////////////////////////////////////////////////
// Spells
void Player::updateAutoRepeatSpell()
{
    // Get the autorepeat spell
    const auto autoRepeatSpell = getCurrentSpell(CURRENT_AUTOREPEAT_SPELL);

    // If player is moving or casting a spell, interrupt wand casting and delay auto shot
    const auto isAutoShot = autoRepeatSpell->getSpellInfo()->getId() == 75;
    if (m_isMoving || isCastingSpell(false, true, isAutoShot))
    {
        if (!isAutoShot)
        {
            interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
        }
        m_FirstCastAutoRepeat = true;
        return;
    }

    // Apply delay to wand shooting
    if (m_FirstCastAutoRepeat && (getAttackTimer(RANGED) - Util::getMSTime() < 500) && !isAutoShot)
    {
        setAttackTimer(RANGED, 500);
    }
    m_FirstCastAutoRepeat = false;

    if (isAttackReady(RANGED))
    {
        // TODO: implement ::CanShootRangedWeapon() into new Spell::canCast()
        // also currently if target gets too far away, your autorepeat spell will get interrupted
        // it's related most likely to ::CanShootRangedWeapon()
        const auto target = GetMapMgr()->GetUnit(autoRepeatSpell->m_targets.m_unitTarget);
        const auto canCastAutoRepeatSpell = CanShootRangedWeapon(autoRepeatSpell->getSpellInfo()->getId(), target, isAutoShot);
        if (canCastAutoRepeatSpell != SPELL_CANCAST_OK)
        {
            if (!isAutoShot)
            {
                interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
            }
            else if (isPlayer())
                autoRepeatSpell->sendCastResult(static_cast<SpellCastResult>(canCastAutoRepeatSpell));
            return;
        }

        // Cast the spell with triggered flag
        const auto newAutoRepeatSpell = sSpellMgr.newSpell(this, autoRepeatSpell->getSpellInfo(), true, nullptr);
        newAutoRepeatSpell->prepare(&autoRepeatSpell->m_targets);

        setAttackTimer(RANGED, getBaseAttackTime(RANGED));
    }
}

bool Player::canUseFlyingMountHere()
{
#if VERSION_STRING == Classic
    return false;
#else
    auto areaEntry = GetArea();
    if (areaEntry == nullptr)
        // If area is null, try finding any area from the zone with zone id
        areaEntry = sAreaStore.LookupEntry(GetZoneId());
    if (areaEntry == nullptr)
        return false;

    // Not flyable areas (such as Dalaran in wotlk)
    if (areaEntry->flags & MapManagement::AreaManagement::AreaFlags::AREA_FLAG_NO_FLY_ZONE)
        return false;

    // Get continent map id
    auto mapId = GetMapId();
    if (mapId == 530 || mapId == 571)
    {
        const auto worldMapEntry = sWorldMapAreaStore.LookupEntry(GetZoneId());
        if (worldMapEntry != nullptr)
            mapId = worldMapEntry->continentMapId >= 0 ? worldMapEntry->continentMapId : worldMapEntry->mapId;
    }

    switch (mapId)
    {
        // Eastern Kingdoms
        case 0:
        // Kalimdor
        case 1:
            // Flight Master's License
            if (!HasSpell(90267))
                return false;
            break;
        // Outland
        case 530:
            return true;
        // Northrend
        case 571:
            // Cold Weather Flying
            if (!HasSpell(54197))
                return false;
            break;
        default:
            return false;
    }
    return true;
#endif
}

bool Player::canDualWield2H() const
{
    return m_canDualWield2H;
}

void Player::setDualWield2H(bool enable)
{
    m_canDualWield2H = enable;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Talents
void Player::learnTalent(uint32_t talentId, uint32_t talentRank)
{
    auto curTalentPoints = getActiveSpec().GetTP();
    if (curTalentPoints == 0)
        return;

    if (talentRank > 4)
        return;

    auto talentInfo = sTalentStore.LookupEntry(talentId);
    if (talentInfo == nullptr)
        return;

    if (objmgr.IsSpellDisabled(talentInfo->RankID[talentRank]))
    {
        if (IsInWorld())
            sendCastFailedPacket(talentInfo->RankID[talentRank], SPELL_FAILED_SPELL_UNAVAILABLE, 0, 0);
        return;
    }

    // Check if player already has the talent with same or higher rank
    for (auto i = talentRank; i <= 4; ++i)
    {
        if (talentInfo->RankID[i] != 0 && HasSpell(talentInfo->RankID[i]))
            return;
    }

    // Check if talent tree is for player's class
    auto talentTreeInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTree);
    if (talentTreeInfo == nullptr || !(getClassMask() & talentTreeInfo->ClassMask))
        return;

#if VERSION_STRING >= Cata
    // Check if enough talent points are spent in the primary talent tree before unlocking other trees
    if (talentInfo->TalentTree != m_FirstTalentTreeLock && m_FirstTalentTreeLock != 0)
    {
        auto pointsUsed = 0;
        for (const auto talent : getActiveSpec().talents)
        {
            pointsUsed += talent.second + 1;
        }

        // You need to spent 31 points in the primary tree before you're able to unlock other trees
        if (pointsUsed < 31)
            return;
    }
#endif

    // Check if talent requires another talent
    if (talentInfo->DependsOn > 0)
    {
        auto dependsOnTalent = sTalentStore.LookupEntry(talentInfo->DependsOn);
        if (dependsOnTalent != nullptr)
        {
            auto hasEnoughRank = false;
            for (auto i = 0; i <= 4; ++i)
            {
                if (dependsOnTalent->RankID[i] != 0)
                {
                    if (HasSpell(dependsOnTalent->RankID[i]))
                    {
                        hasEnoughRank = true;
                        break;
                    }
                }
            }
            if (!hasEnoughRank)
                return;
        }
    }

    auto spellId = talentInfo->RankID[talentRank];
    if (spellId == 0)
    {
        LOG_DETAIL("Player::learnTalent: Player tried to learn talent %u (rank %u) but talent's spell id is 0.", talentId, talentRank);
        return;
    }

    // Check can player yet access this talent
    uint32_t spentPoints = 0;
    if (talentInfo->Row > 0)
    {
        // Loop through player's talents
        for (const auto talent : getActiveSpec().talents)
        {
            auto tmpTalent = sTalentStore.LookupEntry(talent.first);
            if (tmpTalent == nullptr)
                continue;
            // Skip talents from other trees
            if (tmpTalent->TalentTree != talentInfo->TalentTree)
                continue;
            spentPoints += talent.second + 1;
        }
    }

    if (spentPoints < (talentInfo->Row * 5))
        return;

    // Get current talent rank
    uint8_t curTalentRank = 0;
    for (int8_t _talentRank = 4; _talentRank >= 0; --_talentRank)
    {
        if (talentInfo->RankID[_talentRank] != 0 && HasSpell(talentInfo->RankID[_talentRank]))
        {
            curTalentRank = _talentRank + 1;
            break;
        }
    }

    // Check does player have enough talent points
    auto requiredTalentPoints = (talentRank + 1) - curTalentRank;
    if (curTalentPoints < requiredTalentPoints)
        return;

    // Check if player already knows this or higher rank
    if (curTalentRank >= (talentRank + 1))
        return;

    // Check if player already has the talent spell
    if (HasSpell(spellId))
        return;

    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo == nullptr)
        return;

    if (talentRank > 0)
    {
        // Remove the current rank
        if (talentInfo->RankID[talentRank - 1] != 0)
            removeTalent(talentInfo->RankID[talentRank - 1]);
    }

    addTalent(spellInfo);

#if VERSION_STRING >= Cata
    // Set primary talent tree and lock others
    if (m_FirstTalentTreeLock == 0)
    {
        m_FirstTalentTreeLock = talentInfo->TalentTree;
        // TODO: learning Mastery and spec spells
        // also need to handle them in talent reset
    }
#endif

    // Add the new talent to player talent map
    getActiveSpec().AddTalent(talentId, static_cast<uint8_t>(talentRank));
    setTalentPoints(curTalentPoints - requiredTalentPoints, false);
}

void Player::addTalent(SpellInfo const* sp)
{
    // Add to player's spellmap
    addSpell(sp->getId());

    // Cast passive spells and spells with learn effect
    if (sp->hasEffect(SPELL_EFFECT_LEARN_SPELL))
        castSpell(getGuid(), sp, true);
    else if (sp->isPassive())
    {
        if (sp->getRequiredShapeShift() == 0 || (getShapeShiftMask() != 0 && (sp->getRequiredShapeShift() & getShapeShiftMask())) ||
            (getShapeShiftMask() == 0 && (sp->getAttributesExB() & ATTRIBUTESEXB_NOT_NEED_SHAPESHIFT)))
        {
            if (sp->getCasterAuraState() == 0 || hasAuraState(AuraState(sp->getCasterAuraState()), sp, this))
                // TODO: temporarily check for this custom flag, will be removed when spell system checks properly for pets!
                if (((sp->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET) == 0) || (sp->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET && GetSummon() != nullptr))
                    castSpell(getGuid(), sp, true);
        }
    }
}

void Player::removeTalent(uint32_t spellId, bool onSpecChange /*= false*/)
{
    SpellInfo const* spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo != nullptr)
    {
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            // If talent teaches another spell, remove it as well
            if (spellInfo->getEffect(i) == SPELL_EFFECT_LEARN_SPELL)
            {
                auto taughtSpellId = spellInfo->getEffectTriggerSpell(i);
                // There is one case in 3.3.5a and 4.3.4 where the learnt spell yet teaches another spell
                SpellInfo const* taughtSpell = sSpellMgr.getSpellInfo(taughtSpellId);
                if (taughtSpell != nullptr)
                {
                    for (uint8_t u = 0; u < MAX_SPELL_EFFECTS; ++u)
                    {
                        if (taughtSpell->getEffect(u) == SPELL_EFFECT_LEARN_SPELL)
                        {
                            auto taughtSpell2Id = taughtSpell->getEffectTriggerSpell(u);
                            removeSpell(taughtSpell2Id, false, false, 0);
                            RemoveAura(taughtSpell2Id);
                        }
                    }
                }
                removeSpell(taughtSpellId, false, false, 0);
                RemoveAura(taughtSpellId);
            }

            // If talent triggers another spell, remove it (but only self-applied auras)
            if (spellInfo->getEffect(i) == SPELL_EFFECT_TRIGGER_SPELL && spellInfo->getEffectTriggerSpell(i) > 0)
                RemoveAura(spellInfo->getEffectTriggerSpell(i), getGuid());
        }
    }
    removeSpell(spellId, onSpecChange, false, 0);
    RemoveAura(spellId);
}

void Player::resetTalents()
{
    // Loop through player's talents
    for (const auto talent : getActiveSpec().talents)
    {
        auto tmpTalent = sTalentStore.LookupEntry(talent.first);
        if (tmpTalent == nullptr)
            continue;
        removeTalent(tmpTalent->RankID[talent.second]);
        // TODO: Spells, which have multiple ranks and where the first rank is a talent, must be removed from spell book as well
        // (i.e. Mortal Strike and Pyroblast)
    }

    // Unsummon pet
    if (GetSummon() != nullptr)
        GetSummon()->Dismiss();

    // Check offhand
    unEquipOffHandIfRequired();

    // Clear talents
    getActiveSpec().talents.clear();
#if VERSION_STRING >= Cata
    m_FirstTalentTreeLock = 0;
#endif

    // Reset talent point amount
    setInitialTalentPoints(true);
}

void Player::setTalentPoints(uint32_t talentPoints, bool forBothSpecs /*= true*/)
{
    if (!forBothSpecs)
        getActiveSpec().SetTP(talentPoints);
    else
    {
#ifndef FT_DUAL_SPEC
        getActiveSpec().SetTP(talentPoints);
#else
        m_specs[SPEC_PRIMARY].SetTP(talentPoints);
        m_specs[SPEC_SECONDARY].SetTP(talentPoints);
#endif
    }

#if VERSION_STRING < Cata
    // Send talent points also to client
    setFreeTalentPoints(talentPoints);
#endif
}

void Player::addTalentPoints(uint32_t talentPoints, bool forBothSpecs /*= true*/)
{
    if (!forBothSpecs)
        setTalentPoints(getActiveSpec().GetTP() + talentPoints);
    else
    {
#ifndef FT_DUAL_SPEC
        setTalentPoints(getActiveSpec().GetTP() + talentPoints);
#else
        m_specs[SPEC_PRIMARY].SetTP(m_specs[SPEC_PRIMARY].GetTP() + talentPoints);
        m_specs[SPEC_SECONDARY].SetTP(m_specs[SPEC_SECONDARY].GetTP() + talentPoints);

#if VERSION_STRING < Cata
        setFreeTalentPoints(getFreeTalentPoints() + talentPoints);
#endif
#endif
    }
}

void Player::setInitialTalentPoints(bool talentsResetted /*= false*/)
{
    if (getLevel() < 10)
    {
        setTalentPoints(0);
        return;
    }

    // Calculate initial talent points based on level
    uint32_t talentPoints = 0;
#if VERSION_STRING >= Cata
    auto talentPointsAtLevel = sNumTalentsAtLevel.LookupEntry(getLevel());
    if (talentPointsAtLevel != nullptr)
        talentPoints = uint32_t(talentPointsAtLevel->talentPoints);
#else
    talentPoints = getLevel() - 9;
#endif

#ifdef FT_DEATH_KNIGHT
    if (getClass() == DEATHKNIGHT)
    {
        if (GetMapId() == 609)
        {
            // If Death Knight is in the instanced Ebon Hold (map 609), talent points are calculated differently,
            // because Death Knights receive their talent points from their starting quest chain.
            // However if Death Knight is not in the instanced Ebon Hold, it is safe to assume that
            // the player has completed the DK starting quest chain and normal calculation can be used.
            uint32_t dkTalentPoints = 0;
#if VERSION_STRING >= Cata
            auto dkBaseTalentPoints = sNumTalentsAtLevel.LookupEntry(55);
            if (dkBaseTalentPoints != nullptr)
                dkTalentPoints = getLevel() < 55 ? 0 : talentPoints - uint32_t(dkBaseTalentPoints->talentPoints);
#else
            dkTalentPoints = getLevel() < 55 ? 0 : getLevel() - 55;
#endif
            // Add talent points from quests
            dkTalentPoints += m_talentPointsFromQuests;

            if (dkTalentPoints < talentPoints)
                talentPoints = dkTalentPoints;
        }

        // Add extra talent points if any is set in config files
        talentPoints += worldConfig.player.deathKnightStartTalentPoints;
    }
#endif

    // If player's level is increased, player's already spent talent points must be subtracted from initial talent points
    uint32_t usedTalentPoints = 0;
    if (!talentsResetted)
    {
#ifdef FT_DUAL_SPEC
        if (m_talentSpecsCount == 2)
        {
            auto inactiveSpec = m_talentActiveSpec == SPEC_PRIMARY ? SPEC_SECONDARY : SPEC_PRIMARY;
            if (m_specs[inactiveSpec].talents.size() > 0)
            {
                uint32_t usedTalentPoints2 = 0;
                for (const auto talent : m_specs[inactiveSpec].talents)
                {
                    usedTalentPoints2 += talent.second + 1;
                }

                if (usedTalentPoints2 > talentPoints)
                    usedTalentPoints2 = talentPoints;

                m_specs[inactiveSpec].SetTP(talentPoints - usedTalentPoints2);
            }
        }
#endif
        if (getActiveSpec().talents.size() > 0)
        {
            for (const auto talent : getActiveSpec().talents)
            {
                usedTalentPoints += talent.second + 1;
            }

            if (usedTalentPoints > talentPoints)
                usedTalentPoints = talentPoints;
        }
    }

    setTalentPoints(talentPoints - usedTalentPoints, false);
    smsg_TalentsInfo(false);
}

uint32_t Player::getTalentPointsFromQuests() const
{
    return m_talentPointsFromQuests;
}

void Player::setTalentPointsFromQuests(uint32_t talentPoints)
{
    m_talentPointsFromQuests = talentPoints;
}

void Player::smsg_TalentsInfo(bool SendPetTalents)
{
    // TODO: classic and tbc
#if VERSION_STRING >= WotLK
    WorldPacket data(SMSG_TALENTS_INFO, 1000);
    data << uint8_t(SendPetTalents ? 1 : 0);
    if (SendPetTalents)
    {
        if (GetSummon() != nullptr)
            GetSummon()->SendTalentsToOwner();
        return;
    }
    else
    {
        data << uint32_t(getActiveSpec().GetTP()); // Free talent points
        data << uint8_t(m_talentSpecsCount); // How many specs player has
        data << uint8_t(m_talentActiveSpec); // Which spec is active right now

        if (m_talentSpecsCount > MAX_SPEC_COUNT)
            m_talentSpecsCount = MAX_SPEC_COUNT;

        // Loop through specs
        for (auto specId = 0; specId < m_talentSpecsCount; ++specId)
        {
            PlayerSpec spec = m_specs[specId];

#if VERSION_STRING >= Cata
            // Send primary talent tree
            data << uint32_t(m_FirstTalentTreeLock);
#endif

            // How many talents player has learnt
            data << uint8_t(spec.talents.size());
            for (const auto talent : spec.talents)
            {
                data << uint32_t(talent.first);
                data << uint8_t(talent.second);
            }

            // What kind of glyphs player has
            data << uint8_t(GLYPHS_COUNT);
            for (auto i = 0; i < GLYPHS_COUNT; ++i)
            {
                data << uint16_t(GetGlyph(specId, i));
            }
        }
    }
    GetSession()->SendPacket(&data);
#endif
}

void Player::activateTalentSpec(uint8_t specId)
{
#ifndef FT_DUAL_SPEC
    return;
#else
    if (specId >= MAX_SPEC_COUNT || m_talentActiveSpec >= MAX_SPEC_COUNT || m_talentActiveSpec == specId)
        return;

    const auto oldSpec = m_talentActiveSpec;
    m_talentActiveSpec = specId;

    // Dismiss pet
    if (GetSummon() != nullptr)
        GetSummon()->Dismiss();

    // Remove old glyphs
    for (auto i = 0; i < GLYPHS_COUNT; ++i)
    {
        auto glyphProperties = sGlyphPropertiesStore.LookupEntry(m_specs[oldSpec].glyphs[i]);
        if (glyphProperties != nullptr)
            RemoveAura(glyphProperties->SpellID);
    }

    // Remove old talents and move them to deleted spells
    for (const auto itr : m_specs[oldSpec].talents)
    {
        auto talentInfo = sTalentStore.LookupEntry(itr.first);
        if (talentInfo != nullptr)
            removeTalent(talentInfo->RankID[itr.second], true);
    }

    // Add new glyphs
    for (auto i = 0; i < GLYPHS_COUNT; ++i)
    {
        auto glyphProperties = sGlyphPropertiesStore.LookupEntry(m_specs[m_talentActiveSpec].glyphs[i]);
        if (glyphProperties != nullptr)
            castSpell(this, glyphProperties->SpellID, true);
    }

    // Add new talents
    for (const auto itr : m_specs[m_talentActiveSpec].talents)
    {
        auto talentInfo = sTalentStore.LookupEntry(itr.first);
        if (talentInfo == nullptr)
            continue;
        auto spellInfo = sSpellMgr.getSpellInfo(talentInfo->RankID[itr.second]);
        if (spellInfo == nullptr)
            continue;
        addTalent(spellInfo);
    }

    // Set action buttons from new spec
    WorldPacket data(SMSG_ACTION_BUTTONS, PLAYER_ACTION_BUTTON_SIZE + 1);
    // Clears action bars clientside
    data << uint8_t(1);
    // Load buttons
    for (auto i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
    {
        data << m_specs[m_talentActiveSpec].mActions[i].Action;
        data << m_specs[m_talentActiveSpec].mActions[i].Type;
        data << m_specs[m_talentActiveSpec].mActions[i].Misc;
    }
    GetSession()->SendPacket(&data);

    // Reset power
    setPower(getPowerType(), 0);
    SendPowerUpdate(false);

    // Check offhand
    unEquipOffHandIfRequired();

    // Send talent points
    setInitialTalentPoints();
#endif
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
// Items
void Player::unEquipOffHandIfRequired()
{
    auto offHandWeapon = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (offHandWeapon == nullptr)
        return;

    auto needToRemove = true;
    // Check if player has a two-handed weapon in offhand
    if (offHandWeapon->getItemProperties()->InventoryType == INVTYPE_2HWEAPON)
        needToRemove = !canDualWield2H();
    else
    {
        // Player has something in offhand, check if main hand is a two-handed weapon
        const auto mainHandWeapon = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        if (mainHandWeapon != nullptr && mainHandWeapon->getItemProperties()->InventoryType == INVTYPE_2HWEAPON)
            needToRemove = !canDualWield2H();
        else
        {
            // Main hand nor offhand is a two-handed weapon, check if player can dual wield one-handed weapons
            if (offHandWeapon->getItemProperties()->Class == ITEM_CLASS_WEAPON)
                needToRemove = !canDualWield();
            else
                // Offhand is not a weapon
                needToRemove = false;
        }
    }

    if (!needToRemove)
        return;

    // Unequip offhand and find a bag slot for it
    offHandWeapon = getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND, false);
    auto result = getItemInterface()->FindFreeInventorySlot(offHandWeapon->getItemProperties());
    if (!result.Result)
    {
        // Player has no free slots in inventory, send it by mail
        offHandWeapon->RemoveFromWorld();
        offHandWeapon->setOwner(nullptr);
        offHandWeapon->SaveToDB(INVENTORY_SLOT_NOT_SET, 0, true, nullptr);
        sMailSystem.SendAutomatedMessage(MAIL_TYPE_NORMAL, getGuid(), getGuid(), "There were troubles with your item.", "There were troubles storing your item into your inventory.", 0, 0, offHandWeapon->getGuidLow(), MAIL_STATIONERY_GM);
        offHandWeapon->DeleteMe();
        offHandWeapon = nullptr;
    }
    else if (!getItemInterface()->SafeAddItem(offHandWeapon, result.ContainerSlot, result.Slot) && !getItemInterface()->AddItemToFreeSlot(offHandWeapon))
    {
        // shouldn't happen
        offHandWeapon->DeleteMe();
        offHandWeapon = nullptr;
    }
}

bool Player::hasOffHandWeapon() const
{
    if (!canDualWield())
        return false;

    const auto offHandItem = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (offHandItem == nullptr)
        return false;

    return offHandItem->getItemProperties()->Class == ITEM_CLASS_WEAPON;
}

bool Player::hasItem(uint32_t itemId, uint32_t amount /*= 1*/, bool checkBankAlso /*= false*/) const
{
    return getItemInterface()->GetItemCount(itemId, checkBankAlso) >= amount;
}

ItemInterface* Player::getItemInterface() const
{
    return m_itemInterface;
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

void Player::cancelDuel()
{
    // arbiter
    WoWGuid wowGuid;
    wowGuid.Init(getDuelArbiter());
    const auto arbiter = GetMapMgr()->GetGameObject(wowGuid.getGuidLowPart());
    if (arbiter)
        arbiter->RemoveFromWorld(true);

    // duel setup (duelpartner and us)
    DuelingWith->setDuelArbiter(0);
    DuelingWith->m_duelState = DUEL_STATE_FINISHED;
    DuelingWith->DuelingWith = nullptr;
    DuelingWith->setDuelTeam(0);
    DuelingWith->m_duelCountdownTimer = 0;

    setDuelArbiter(0);
    m_duelState = DUEL_STATE_FINISHED;
    DuelingWith = nullptr;
    setDuelTeam(0);
    m_duelCountdownTimer = 0;

    // auras
    for (auto i = MAX_NEGATIVE_AURAS_EXTEDED_START; i < MAX_NEGATIVE_AURAS_EXTEDED_END; ++i)
    {
        if (m_auras[i])
            m_auras[i]->Remove();
    }

    // summons
    for (const auto& summonedPet: GetSummons())
    {
        if (summonedPet && summonedPet->isAlive())
            summonedPet->SetPetAction(PET_ACTION_STAY);
    }
}

void Player::logIntoBattleground()
{
    const auto mapMgr = sInstanceMgr.GetInstance(this);
    if (mapMgr && mapMgr->m_battleground)
    {
        const auto battleground = mapMgr->m_battleground;
        if (battleground->HasEnded() && battleground->HasFreeSlots(getInitialTeam(), battleground->GetType()))
        {
            if (!IS_INSTANCE(m_bgEntryPointMap))
            {
                m_position.ChangeCoords({ m_bgEntryPointX, m_bgEntryPointY, m_bgEntryPointZ, m_bgEntryPointO });
                m_mapId = m_bgEntryPointMap;
            }
            else
            {
                m_position.ChangeCoords({ GetBindPositionX(), GetBindPositionY(), GetBindPositionZ() });
                m_mapId = GetBindMapId();
            }
        }
    }
}

bool Player::logOntoTransport()
{
    bool success = true;
#if VERSION_STRING < Cata
    if (obj_movement_info.transport_data.transportGuid != 0)
#else
    if (!obj_movement_info.getTransportGuid().IsEmpty())
#endif
    {
#if VERSION_STRING < Cata
        const auto transporter = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(obj_movement_info.transport_data.transportGuid));
#else
        const auto transporter = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(static_cast<uint32>(obj_movement_info.getTransportGuid())));
#endif
        if (transporter)
        {
            if (isDead())
            {
                ResurrectPlayer();
                setHealth(getMaxHealth());
                setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));
            }

            const float c_tposx = transporter->GetPositionX() + GetTransPositionX();
            const float c_tposy = transporter->GetPositionY() + GetTransPositionY();
            const float c_tposz = transporter->GetPositionZ() + GetTransPositionZ();

            const LocationVector positionOnTransport = LocationVector(c_tposx, c_tposy, c_tposz, GetOrientation());

            if (GetMapId() != transporter->GetMapId())
            {
                SetMapId(transporter->GetMapId());
                SendPacket(AscEmu::Packets::SmsgNewWorld(transporter->GetMapId(), positionOnTransport).serialise().get());

                success = false;
            }

            SetPosition(positionOnTransport.x, positionOnTransport.y, positionOnTransport.z, positionOnTransport.o, false);
            transporter->AddPassenger(this);
        }
    }

    return success;
}

void Player::setLoginPosition()
{
    bool startOnGMIsland = false;
    if (m_session->HasGMPermissions() && m_FirstLogin && sWorld.settings.gm.isStartOnGmIslandEnabled)
        startOnGMIsland = true;

    uint32_t mapId = 1;
    float orientation = 0;
    float position_x = 16222.6f;
    float position_y = 16265.9f;
    float position_z = 14.2085f;

    if (startOnGMIsland)
    {
        m_position.ChangeCoords({ position_x, position_y, position_z, orientation });
        m_mapId = mapId;

        SetBindPoint(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId(), GetZoneId());
    }
    else
    {
        mapId = GetMapId();
        orientation = GetOrientation();
        position_x = GetPositionX();
        position_y = GetPositionY();
        position_z = GetPositionZ();
    }

    sendLoginVerifyWorldPacket(mapId, position_x, position_y, position_z, orientation);
}

void Player::setPlayerInfoIfNeeded()
{
    auto playerInfo = objmgr.GetPlayerInfo(getGuidLow());
    if (playerInfo == nullptr)
    {
        playerInfo = new PlayerInfo;
        playerInfo->cl = getClass();
        playerInfo->gender = getGender();
        playerInfo->guid = getGuidLow();
        playerInfo->name = strdup(getName().c_str());
        playerInfo->lastLevel = getLevel();
        playerInfo->lastOnline = UNIXTIME;
        playerInfo->lastZone = GetZoneId();
        playerInfo->race = getRace();
        playerInfo->team = getTeam();
        playerInfo->guildRank = GUILD_RANK_NONE;
        playerInfo->m_Group = nullptr;
        playerInfo->subGroup = 0;

        playerInfo->m_loggedInPlayer = this;

        objmgr.AddPlayerInfo(playerInfo);
    }

    m_playerInfo = playerInfo;
}

void Player::setGuildAndGroupInfo()
{
    if (getPlayerInfo()->m_guild)
    {
        if (const auto guild = sGuildMgr.getGuildById(getPlayerInfo()->m_guild))
        {
            setGuildId(getPlayerInfo()->m_guild);
            setGuildRank(getPlayerInfo()->guildRank);
            guild->sendLoginInfo(GetSession());
#if VERSION_STRING >= Cata
            setGuildLevel(guild->getLevel());
#endif
        }
    }

    if (getPlayerInfo()->m_Group)
        getPlayerInfo()->m_Group->Update();
}

void Player::sendCinematicOnFirstLogin()
{
    if (m_FirstLogin && !worldConfig.player.skipCinematics)
    {
#if VERSION_STRING > TBC
        if (const auto charEntry = sChrClassesStore.LookupEntry(getClass()))
        {
            if (charEntry->cinematic_id != 0)
                OutPacket(SMSG_TRIGGER_CINEMATIC, 4, &charEntry->cinematic_id);
            else if (const auto raceEntry = sChrRacesStore.LookupEntry(getRace()))
                OutPacket(SMSG_TRIGGER_CINEMATIC, 4, &raceEntry->cinematic_id);
        }
#else
        if (const auto raceEntry = sChrRacesStore.LookupEntry(getRace()))
            OutPacket(SMSG_TRIGGER_CINEMATIC, 4, &raceEntry->cinematic_id);
#endif
    }
}

int32_t Player::getMyCorpseInstanceId() const
{
    return myCorpseInstanceId;
}

void Player::sendTalentResetConfirmPacket()
{
    m_session->SendPacket(MsgTalentWipeConfirm(getGuid(), CalcTalentResetCost(GetTalentResetTimes())).serialise().get());
}

void Player::sendPetUnlearnConfirmPacket()
{
    if (GetSummon() == nullptr)
        return;

    m_session->SendPacket(SmsgPetUnlearnConfirm(GetSummon()->getGuid(), GetSummon()->GetUntrainCost()).serialise().get());
}

void Player::sendDungeonDifficultyPacket()
{
    m_session->SendPacket(MsgSetDungeonDifficulty(m_RaidDifficulty, 1, InGroup()).serialise().get());
}

void Player::sendRaidDifficultyPacket()
{
#if VERSION_STRING > TBC
    m_session->SendPacket(MsgSetRaidDifficulty(m_RaidDifficulty, 1, InGroup()).serialise().get());
#endif
}

void Player::sendInstanceDifficultyPacket(uint8_t difficulty)
{
    m_session->SendPacket(SmsgInstanceDifficulty(difficulty).serialise().get());
}

void Player::sendNewDrunkStatePacket(uint32_t state, uint32_t itemId)
{
    SendMessageToSet(SmsgCrossedInebriationThreshold(getGuid(), state, itemId).serialise().get(), true);
}

void Player::sendSetProficiencyPacket(uint8_t itemClass, uint32_t proficiency)
{
    m_session->SendPacket(SmsgSetProficiency(itemClass, proficiency).serialise().get());
}

void Player::sendPartyKillLogPacket(uint64_t killedGuid)
{
    SendMessageToSet(SmsgPartyKillLog(getGuid(), killedGuid).serialise().get(), true);
}

void Player::sendDestroyObjectPacket(uint64_t destroyedGuid)
{
    m_session->SendPacket(SmsgDestroyObject(destroyedGuid).serialise().get());
}

void Player::sendEquipmentSetUseResultPacket(uint8_t result)
{
#if VERSION_STRING > TBC
    m_session->SendPacket(SmsgEquipmentSetUseResult(result).serialise().get());
#endif
}

void Player::sendTotemCreatedPacket(uint8_t slot, uint64_t guid, uint32_t duration, uint32_t spellId)
{
    m_session->SendPacket(SmsgTotemCreated(slot, guid, duration, spellId).serialise().get());
}

void Player::sendGossipPoiPacket(float posX, float posY, uint32_t icon, uint32_t flags, uint32_t data, std::string name)
{
    m_session->SendPacket(SmsgGossipPoi(flags, posX, posY, icon, data, name).serialise().get());
}

void Player::sendPoiById(uint32_t id)
{
    if (const auto pPoi = sMySQLStore.getPointOfInterest(id))
        sendGossipPoiPacket(pPoi->x, pPoi->y, pPoi->icon, pPoi->flags, pPoi->data, pPoi->iconName);
}

void Player::sendStopMirrorTimerPacket(MirrorTimerTypes type)
{
    m_session->SendPacket(SmsgStopMirrorTimer(type).serialise().get());
}

void Player::sendMeetingStoneSetQueuePacket(uint32_t dungeonId, uint8_t status)
{
    m_session->SendPacket(SmsgMeetingstoneSetQueue(dungeonId, status).serialise().get());
}

void Player::sendPlayObjectSoundPacket(uint64_t objectGuid, uint32_t soundId)
{
    SendMessageToSet(SmsgPlayObjectSound(soundId, objectGuid).serialise().get(), true);
}

void Player::sendPlaySoundPacket(uint32_t soundId)
{
    m_session->SendPacket(SmsgPlaySound(soundId).serialise().get());
}

void Player::sendExploreExperiencePacket(uint32_t areaId, uint32_t experience)
{
    m_session->SendPacket(SmsgExplorationExperience(areaId, experience).serialise().get());
}

void Player::sendSpellCooldownEventPacket(uint32_t spellId)
{
    m_session->SendPacket(SmsgCooldownEvent(spellId, getGuid()).serialise().get());
}

void Player::sendSpellModifierPacket(uint8_t spellGroup, uint8_t spellType, int32_t modifier, bool isPct)
{
    if (isPct)
        m_session->SendPacket(SmsgSetPctSpellModifier(spellGroup, spellType, modifier).serialise().get());
    else
        m_session->SendPacket(SmsgSetFlatSpellModifier(spellGroup, spellType, modifier).serialise().get());
}

void Player::sendLoginVerifyWorldPacket(uint32_t mapId, float posX, float posY, float posZ, float orientation)
{
    m_session->SendPacket(SmsgLoginVerifyWorld(mapId, LocationVector(posX, posY, posZ, orientation)).serialise().get());
}

void Player::sendMountResultPacket(uint32_t result)
{
    m_session->SendPacket(SmsgMountResult(result).serialise().get());
}

void Player::sendDismountResultPacket(uint32_t result)
{
    m_session->SendPacket(SmsgDismountResult(result).serialise().get());
}

void Player::sendLogXpGainPacket(uint64_t guid, uint32_t normalXp, uint32_t restedXp, bool type)
{
    m_session->SendPacket(SmsgLogXpGain(guid, normalXp, restedXp, type).serialise().get());
}

void Player::sendCastFailedPacket(uint32_t spellId, uint8_t errorMessage, uint8_t multiCast, uint32_t extra1, uint32_t extra2 /*= 0*/)
{
    m_session->SendPacket(SmsgCastFailed(multiCast, spellId, errorMessage, extra1, extra2).serialise().get());
}

void Player::sendLevelupInfoPacket(uint32_t level, uint32_t hp, uint32_t mana, uint32_t stat0, uint32_t stat1, uint32_t stat2, uint32_t stat3, uint32_t stat4)
{
    m_session->SendPacket(SmsgLevelupInfo(level, hp, mana, stat0, stat1, stat2, stat3, stat4).serialise().get());
}

void Player::sendItemPushResultPacket(bool created, bool recieved, bool sendtoset, uint8_t destbagslot, uint32_t destslot, uint32_t count, uint32_t entry, uint32_t suffix, uint32_t randomprop, uint32_t stack)
{
    const std::unique_ptr<WorldPacket>::pointer data = SmsgItemPushResult(getGuid(), recieved, created, destbagslot, destslot,
                                                                    entry, suffix, randomprop, count,
                                                                    stack).serialise().get();

    if (sendtoset && InGroup())
        GetGroup()->SendPacketToAll(data);
    else
        m_session->SendPacket(data);
}

void Player::sendClientControlPacket(Unit* target, uint8_t allowMove)
{
    SendPacket(SmsgClientControlUpdate(target->GetNewGUID(), allowMove).serialise().get());

    if (target == this)
        SetMover(this);
}

void Player::sendGuildMotd()
{
    if (!GetGuild())
        return;

    SendPacket(SmsgGuildEvent(GE_MOTD, { GetGuild()->getMOTD() }, 0).serialise().get());
}

bool Player::isPvpFlagSet()
{
#if VERSION_STRING > TBC
    return getPvpFlags() & U_FIELD_BYTES_FLAG_PVP;
#else
    return getUnitFlags() & UNIT_FLAG_PVP;
#endif
}

void Player::setPvpFlag()
{
    StopPvPTimer();
#if VERSION_STRING > TBC
    setPvpFlags(getPvpFlags() | U_FIELD_BYTES_FLAG_PVP);
#else
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
#endif

    addPlayerFlags(PLAYER_FLAG_PVP_TIMER);

    summonhandler.SetPvPFlags();
    for (auto& summon : GetSummons())
        summon->setPvpFlag();

    if (CombatStatus.IsInCombat())
        addPlayerFlags(PLAYER_FLAG_PVP_GUARD_ATTACKABLE);
}

void Player::removePvpFlag()
{
    StopPvPTimer();
#if VERSION_STRING > TBC
    setPvpFlags(getPvpFlags() & ~U_FIELD_BYTES_FLAG_PVP);
#else
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
#endif

    removePlayerFlags(PLAYER_FLAG_PVP_TIMER);

    summonhandler.RemovePvPFlags();
    for (auto& summon : GetSummons())
        summon->removePvpFlag();
}

bool Player::isFfaPvpFlagSet()
{
    return getPvpFlags() & U_FIELD_BYTES_FLAG_FFA_PVP;
}

void Player::setFfaPvpFlag()
{
    StopPvPTimer();
    setPvpFlags(getPvpFlags() | U_FIELD_BYTES_FLAG_FFA_PVP);
    addPlayerFlags(PLAYER_FLAG_FREE_FOR_ALL_PVP);

    summonhandler.SetFFAPvPFlags();
    for (auto& summon : GetSummons())
        summon->setFfaPvpFlag();
}

void Player::removeFfaPvpFlag()
{
    StopPvPTimer();
    setPvpFlags(getPvpFlags() & ~U_FIELD_BYTES_FLAG_FFA_PVP);
    removePlayerFlags(PLAYER_FLAG_FREE_FOR_ALL_PVP);

    summonhandler.RemoveFFAPvPFlags();
    for (auto& summon : GetSummons())
        summon->removeFfaPvpFlag();
}

bool Player::isSanctuaryFlagSet()
{
    return getPvpFlags() & U_FIELD_BYTES_FLAG_SANCTUARY;
}

void Player::setSanctuaryFlag()
{
    setPvpFlags(getPvpFlags() | U_FIELD_BYTES_FLAG_SANCTUARY);

    summonhandler.SetSanctuaryFlags();
    for (auto& summon : GetSummons())
        summon->setSanctuaryFlag();
}

void Player::removeSanctuaryFlag()
{
    setPvpFlags(getPvpFlags() & ~U_FIELD_BYTES_FLAG_SANCTUARY);

    summonhandler.RemoveSanctuaryFlags();
    for (auto& summon : GetSummons())
        summon->removeSanctuaryFlag();
}
