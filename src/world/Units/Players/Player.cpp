/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

using namespace AscEmu::Packets;

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
#if VERSION_STRING != Cata
    return playerData()->guild_id;
#else
    return static_cast<uint32_t>(objectData()->data);
#endif
}
void Player::setGuildId(uint32_t guildId)
{
#if VERSION_STRING != Cata
    write(playerData()->guild_id, guildId);
#else
    write(objectData()->data, MAKE_NEW_GUID(guildId, 0, HIGHGUID_TYPE_GUILD));

    ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_GUILD_LVL_ENABLED, guildId != 0);
    setUInt16Value(OBJECT_FIELD_TYPE, 1, guildId != 0);
#endif
}

uint32_t Player::getGuildRank() const { return playerData()->guild_rank; }
void Player::setGuildRank(uint32_t guildRank) { write(playerData()->guild_rank, guildRank); }

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

float Player::getModDamageDonePct(uint8_t shool) const { return playerData()->field_mod_damage_done_pct[shool]; }
void Player::setModDamageDonePct(float damagePct, uint8_t shool) { write(playerData()->field_mod_damage_done_pct[shool], damagePct); }

uint32_t Player::getPlayerFieldBytes() const { return playerData()->player_field_bytes.raw; }
void Player::setPlayerFieldBytes(uint32_t bytes) { write(playerData()->player_field_bytes.raw, bytes); }

uint8_t Player::getActionBarId() const { return playerData()->player_field_bytes.s.actionBarId; }
void Player::setActionBarId(uint8_t actionBarId) { write(playerData()->player_field_bytes.s.actionBarId, actionBarId); }

uint32_t Player::getPlayerFieldBytes2() const { return playerData()->player_field_bytes_2.raw; }
void Player::setPlayerFieldBytes2(uint32_t bytes) { write(playerData()->player_field_bytes_2.raw, bytes); }
#if VERSION_STRING > TBC
uint32_t Player::getGlyphsEnabled() const { return playerData()->glyphs_enabled; }
void Player::setGlyphsEnabled(uint32_t glyphs) { write(playerData()->glyphs_enabled, glyphs); }
#endif

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
// Spells
void Player::updateAutoRepeatSpell()
{
    // Get the autorepeat spell
    const auto autoRepeatSpell = getCurrentSpell(CURRENT_AUTOREPEAT_SPELL);

    // If player is moving or casting a spell, interrupt wand casting and delay auto shot
    const auto isAutoShot = autoRepeatSpell->GetSpellInfo()->getId() == 75;
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
        const auto canCastAutoRepeatSpell = CanShootRangedWeapon(autoRepeatSpell->GetSpellInfo()->getId(), target, isAutoShot);
        if (canCastAutoRepeatSpell != SPELL_CANCAST_OK)
        {
            if (!isAutoShot)
            {
                interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
            }
            else if (isPlayer())
                autoRepeatSpell->SendCastResult(canCastAutoRepeatSpell);
            return;
        }

        // Cast the spell with triggered flag
        const auto newAutoRepeatSpell = sSpellFactoryMgr.NewSpell(this, autoRepeatSpell->GetSpellInfo(), true, nullptr);
        newAutoRepeatSpell->prepare(&autoRepeatSpell->m_targets);

        setAttackTimer(RANGED, getBaseAttackTime(RANGED));
    }
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

#if VERSION_STRING == Cata
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

    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);
    if (spellInfo == nullptr)
        return;

    if (talentRank > 0)
    {
        // Remove the current rank
        if (talentInfo->RankID[talentRank - 1] != 0)
            removeTalent(talentInfo->RankID[talentRank - 1]);
    }

    addTalent(spellInfo);

#if VERSION_STRING == Cata
    // Set primary talent tree and lock others
    if (m_FirstTalentTreeLock == 0)
    {
        m_FirstTalentTreeLock = talentInfo->TalentTree;
        // TODO: learning Mastery and spec spells
        // also need to handle them in talent reset
    }
#endif

    // Add the new talent to player talent map
    getActiveSpec().AddTalent(talentId, talentRank);
    setTalentPoints(curTalentPoints - requiredTalentPoints, false);
}

void Player::addTalent(SpellInfo* sp)
{
    // Add to player's spellmap
    addSpell(sp->getId());

    // Cast passive spells and spells with learn effect
    if (sp->hasEffect(SPELL_EFFECT_LEARN_SPELL))
        CastSpell(getGuid(), sp, true);
    else if (sp->isPassive())
    {
        if (sp->getRequiredShapeShift() == 0 || (getShapeShiftMask() != 0 && (sp->getRequiredShapeShift() & getShapeShiftMask())) ||
            (getShapeShiftMask() == 0 && (sp->getAttributesExB() & ATTRIBUTESEXB_NOT_NEED_SHAPESHIFT)))
        {
            if (sp->getCasterAuraState() == 0 || hasAuraState(AuraState(sp->getCasterAuraState()), sp, this))
                // TODO: temporarily check for this custom flag, will be removed when spell system checks properly for pets!
                if (((sp->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET) == 0) || (sp->custom_c_is_flags & SPELL_FLAG_IS_EXPIREING_WITH_PET && GetSummon() != nullptr))
                    CastSpell(getGuid(), sp, true);
        }
    }
}

void Player::removeTalent(uint32_t spellId, bool onSpecChange /*= false*/)
{
    SpellInfo const* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);
    if (spellInfo != nullptr)
    {
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            // If talent teaches another spell, remove it as well
            if (spellInfo->getEffect(i) == SPELL_EFFECT_LEARN_SPELL)
            {
                auto taughtSpellId = spellInfo->getEffectTriggerSpell(i);
                // There is one case in 3.3.5a and 4.3.4 where the learnt spell yet teaches another spell
                SpellInfo const* taughtSpell = sSpellCustomizations.GetSpellInfo(taughtSpellId);
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
#if VERSION_STRING == Cata
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

#if VERSION_STRING != Cata
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

#if VERSION_STRING != Cata
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
#if VERSION_STRING == Cata
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
#if VERSION_STRING == Cata
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

#if VERSION_STRING == Cata
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
            CastSpell(this, glyphProperties->SpellID, true);
    }

    // Add new talents
    for (const auto itr : m_specs[m_talentActiveSpec].talents)
    {
        auto talentInfo = sTalentStore.LookupEntry(itr.first);
        if (talentInfo == nullptr)
            continue;
        auto spellInfo = sSpellCustomizations.GetSpellInfo(talentInfo->RankID[itr.second]);
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
    SetPower(getPowerType(), 0);
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
        if (battleground->HasEnded() && battleground->HasFreeSlots(GetTeamInitial(), battleground->GetType()))
        {
            if (!IS_INSTANCE(m_bgEntryPointMap))
            {
                m_position.ChangeCoords(m_bgEntryPointX, m_bgEntryPointY, m_bgEntryPointZ, m_bgEntryPointO);
                m_mapId = m_bgEntryPointMap;
            }
            else
            {
                m_position.ChangeCoords(GetBindPositionX(), GetBindPositionY(), GetBindPositionZ(), 0.0f);
                m_mapId = GetBindMapId();
            }
        }
    }
}

bool Player::logOntoTransport()
{
    bool success = true;
#if VERSION_STRING != Cata
    if (obj_movement_info.transport_data.transportGuid != 0)
#else
    if (!obj_movement_info.getTransportGuid().IsEmpty())
#endif
    {
#if VERSION_STRING != Cata
        const auto transporter = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(obj_movement_info.transport_data.transportGuid));
#else
        const auto transporter = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(static_cast<uint32>(obj_movement_info.getTransportGuid())));
#endif
        if (transporter)
        {
            if (IsDead())
            {
                ResurrectPlayer();
                setHealth(getMaxHealth());
                SetPower(POWER_TYPE_MANA, GetMaxPower(POWER_TYPE_MANA));
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
        m_position.ChangeCoords(position_x, position_y, position_z, orientation);
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
        playerInfo->team = GetTeam();
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
#if VERSION_STRING == Cata
            SetGuildLevel(guild->getLevel());
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

void Player::unEquipOffHandIfRequired()
{
    auto offHandWeapon = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (offHandWeapon == nullptr)
        return;

    auto needToRemove = true;
    // Check if player has a two-handed weapon in offhand
    if (offHandWeapon->getItemProperties()->InventoryType == INVTYPE_2HWEAPON)
        needToRemove = !canDualWield2H();
    else
    {
        // Player has something in offhand, check if main hand is a two-handed weapon
        const auto mainHandWeapon = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
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
    offHandWeapon = GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, EQUIPMENT_SLOT_OFFHAND, false);
    auto result = GetItemInterface()->FindFreeInventorySlot(offHandWeapon->getItemProperties());
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
    else if (!GetItemInterface()->SafeAddItem(offHandWeapon, result.ContainerSlot, result.Slot) && !GetItemInterface()->AddItemToFreeSlot(offHandWeapon))
    {
        // shouldn't happen
        offHandWeapon->DeleteMe();
        offHandWeapon = nullptr;
    }
}

bool Player::hasOffHandWeapon()
{
    if (!canDualWield())
        return false;

    const auto offHandItem = GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (offHandItem == nullptr)
        return false;

    return offHandItem->getItemProperties()->Class == ITEM_CLASS_WEAPON;
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
