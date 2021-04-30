/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "Player.h"

#include "Chat/ChatDefines.hpp"
#include "Data/WoWPlayer.hpp"
#include "Management/Battleground/Battleground.h"
#include "Management/Guild/GuildMgr.hpp"
#include "Management/ItemInterface.h"
#include "Map/Area/AreaManagementGlobals.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Map/MapMgr.h"
#include "Objects/GameObject.h"
#include "Objects/ObjectMgr.h"
#include "Server/Opcodes.hpp"
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
#include "Server/Packets/SmsgNewWorld.h"
#include "Server/Packets/SmsgPvpCredit.h"
#include "Server/Packets/SmsgRaidGroupOnly.h"
#include "Server/Packets/SmsgAuctionCommandResult.h"
#include "Server/Packets/SmsgClearCooldown.h"
#include "Server/World.h"
#include "Server/Packets/SmsgContactList.h"
#include "Spell/Definitions/AuraInterruptFlags.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/Definitions/Spec.h"
#include "Spell/Definitions/SpellDamageType.h"
#include "Spell/Definitions/SpellFailure.h"
#include "Spell/Definitions/SpellIsFlags.h"
#include "Spell/Spell.h"
#include "Spell/SpellAuras.h"
#include "Spell/SpellDefines.hpp"
#include "Spell/SpellMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Creatures/Pet.h"
#include "Units/UnitDefines.hpp"
#include "Server/Packets/SmsgTriggerMovie.h"
#include "Server/Packets/SmsgTriggerCinematic.h"
#include "Server/Packets/SmsgSpellCooldown.h"

using namespace AscEmu::Packets;

TradeData::TradeData(Player* player, Player* trader)
{
    m_player = player;
    m_tradeTarget = trader;

    m_accepted = false;

    m_money = 0;
    m_spell = 0;
    m_spellCastItem = 0;
    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
        m_items[i] = 0;
}

Player* TradeData::getTradeTarget() const
{
    return m_tradeTarget;
}

TradeData* TradeData::getTargetTradeData() const
{
    return m_tradeTarget->getTradeData();
}

Item* TradeData::getTradeItem(TradeSlots slot) const
{
    return m_items[slot] != 0 ? m_player->getItemInterface()->GetItemByGUID(m_items[slot]) : nullptr;
}

bool TradeData::hasTradeItem(uint64_t itemGuid) const
{
    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (m_items[i] == itemGuid)
            return true;
    }

    return false;
}

bool TradeData::hasPlayerOrTraderItemInTrade(uint64_t itemGuid) const
{
    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (m_items[i] == itemGuid)
            return true;
        if (getTargetTradeData()->m_items[i] == itemGuid)
            return true;
    }

    return false;
}

uint32_t TradeData::getSpell() const
{
    return m_spell;
}

Item* TradeData::getSpellCastItem() const
{
    return hasSpellCastItem() ? m_player->getItemInterface()->GetItemByGUID(m_spellCastItem) : nullptr;
}

bool TradeData::hasSpellCastItem() const
{
    return m_spellCastItem != 0;
}

uint64_t TradeData::getTradeMoney() const
{
    return m_money;
}

void TradeData::setTradeMoney(uint64_t money)
{
    if (m_money == money)
        return;

    if (money > m_player->getCoinage())
        return;

    m_money = money;

    setTradeAccepted(false);
    getTargetTradeData()->setTradeAccepted(false);

    // Send update packet to trader
    m_tradeTarget->GetSession()->sendTradeUpdate(true);
}

void TradeData::setTradeAccepted(bool state, bool sendBoth/* = false*/)
{
    m_accepted = state;

    if (!state)
    {
        if (sendBoth)
            m_tradeTarget->GetSession()->sendTradeResult(TRADE_STATUS_STATE_CHANGED);
        else
            m_player->GetSession()->sendTradeResult(TRADE_STATUS_STATE_CHANGED);
    }
}

bool TradeData::isTradeAccepted() const
{
    return m_accepted;
}

void TradeData::setTradeItem(TradeSlots slot, Item* item)
{
    const auto itemGuid = item != nullptr ? item->getGuid() : 0;
    if (m_items[slot] == itemGuid)
        return;

    m_items[slot] = itemGuid;

    setTradeAccepted(false);
    getTargetTradeData()->setTradeAccepted(false);

    // Send update packet to trader
    m_tradeTarget->GetSession()->sendTradeUpdate(true);
}

void TradeData::setTradeSpell(uint32_t spell_id, Item* castItem /*= nullptr*/)
{
    const auto itemGuid = castItem != nullptr ? castItem->getGuid() : 0;
    if (m_spell == spell_id && m_spellCastItem == itemGuid)
        return;

    m_spell = spell_id;
    m_spellCastItem = itemGuid;

    setTradeAccepted(false);
    getTargetTradeData()->setTradeAccepted(false);

    // Send update packet to both parties
    m_player->GetSession()->sendTradeUpdate(false);
    m_tradeTarget->GetSession()->sendTradeUpdate(true);
}

void Player::resendSpeed()
{
    if (resend_speed)
    {
        setSpeedRate(TYPE_RUN, getSpeedRate(TYPE_RUN, true), true);
        setSpeedRate(TYPE_FLY, getSpeedRate(TYPE_FLY, true), true);
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
void Player::setPlayerFlags(uint32_t flags)
{
    write(playerData()->player_flags, flags);

#if VERSION_STRING == TBC
    // TODO Fix this later
    return;
#endif

    // Update player flags also to group
    if (!IsInWorld() || getGroup() == nullptr)
        return;

    AddGroupUpdateFlag(GROUP_UPDATE_FLAG_STATUS);
}
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
    write(objectData()->data, WoWGuid(guildId, 0, HIGHGUID_TYPE_GUILD).getRawGuid());

    if (guildId)
        addPlayerFlags(PLAYER_FLAGS_GUILD_LVL_ENABLED);
    else
        removePlayerFlags(PLAYER_FLAGS_GUILD_LVL_ENABLED);

    write(objectData()->parts.guild_id, static_cast<uint16_t>(guildId != 0 ? 1 : 0));
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

uint8_t Player::getBytes2UnknownField() const { return playerData()->player_bytes_2.s.unk1; }
void Player::setBytes2UnknownField(uint8_t value) { write(playerData()->player_bytes_2.s.unk1, value); }

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

uint8_t Player::getDrunkValue() const { return playerData()->player_bytes_3.s.drunk_value; }
void Player::setDrunkValue(uint8_t value) { write(playerData()->player_bytes_3.s.drunk_value, value); }

uint8_t Player::getPvpRank() const { return playerData()->player_bytes_3.s.pvp_rank; }
void Player::setPvpRank(uint8_t rank) { write(playerData()->player_bytes_3.s.pvp_rank, rank); }

uint8_t Player::getArenaFaction() const { return playerData()->player_bytes_3.s.arena_faction; }
void Player::setArenaFaction(uint8_t faction) { write(playerData()->player_bytes_3.s.arena_faction, faction); }
//bytes3 end

uint32_t Player::getDuelTeam() const { return playerData()->duel_team; }
void Player::setDuelTeam(uint32_t team) { write(playerData()->duel_team, team); }

uint32_t Player::getGuildTimestamp() const { return playerData()->guild_timestamp; }
void Player::setGuildTimestamp(uint32_t timestamp) { write(playerData()->guild_timestamp, timestamp); }

//QuestLog start
uint32_t Player::getQuestLogEntryForSlot(uint8_t slot) const { return playerData()->quests[slot].quest_id; }
void Player::setQuestLogEntryBySlot(uint8_t slot, uint32_t questEntry) { write(playerData()->quests[slot].quest_id, questEntry); }

#if VERSION_STRING > Classic
uint32_t Player::getQuestLogStateForSlot(uint8_t slot) const { return playerData()->quests[slot].state; }
void Player::setQuestLogStateBySlot(uint8_t slot, uint32_t state) { write(playerData()->quests[slot].state, state); }
#else
uint32_t Player::getQuestLogStateForSlot(uint8_t slot) const
{
    //\todo: get last 1*8 bits as state
    return playerData()->quests[slot].required_count_state;
}

void Player::setQuestLogStateBySlot(uint8_t slot, uint32_t state)
{
    //\todo: write last 1*8 bits as state
    write(playerData()->quests[slot].required_count_state, state);
}
#endif

#if VERSION_STRING > TBC
uint64_t Player::getQuestLogRequiredMobOrGoForSlot(uint8_t slot) const { return playerData()->quests[slot].required_mob_or_go; }
void Player::setQuestLogRequiredMobOrGoBySlot(uint8_t slot, uint64_t mobOrGoCount) { write(playerData()->quests[slot].required_mob_or_go, mobOrGoCount); }
#elif VERSION_STRING == TBC
uint32_t Player::getQuestLogRequiredMobOrGoForSlot(uint8_t slot) const { return playerData()->quests[slot].required_mob_or_go; }
void Player::setQuestLogRequiredMobOrGoBySlot(uint8_t slot, uint32_t mobOrGoCount) { write(playerData()->quests[slot].required_mob_or_go, mobOrGoCount); }
#else
uint32_t Player::getQuestLogRequiredMobOrGoForSlot(uint8_t slot) const
{
    //\todo: get first 4*6 bits as required count
    return playerData()->quests[slot].required_count_state;
}
void Player::setQuestLogRequiredMobOrGoBySlot(uint8_t slot, uint32_t mobOrGoCount)
{
    //\todo: write first 4*6 bits as required count
    write(playerData()->quests[slot].required_count_state, mobOrGoCount);
}
#endif

uint32_t Player::getQuestLogExpireTimeForSlot(uint8_t slot) const { return playerData()->quests[slot].expire_time; }
void Player::setQuestLogExpireTimeBySlot(uint8_t slot, uint32_t expireTime) { write(playerData()->quests[slot].expire_time, expireTime); }
//QuestLog end

//VisibleItem start
uint32_t Player::getVisibleItemEntry(uint32_t slot) const { return playerData()->visible_items[slot].entry; }
void Player::setVisibleItemEntry(uint32_t slot, uint32_t entry) { write(playerData()->visible_items[slot].entry, entry); }

#if VERSION_STRING > TBC
uint32_t Player::getVisibleItemEnchantment(uint32_t slot) const { return playerData()->visible_items[slot].enchantment; }
void Player::setVisibleItemEnchantment(uint32_t slot, uint32_t enchantment) { write(playerData()->visible_items[slot].enchantment, enchantment); }
#else
uint32_t Player::getVisibleItemEnchantment(uint32_t slot, uint32_t pos) const { return playerData()->visible_items[slot].unk0[pos]; }
void Player::setVisibleItemEnchantment(uint32_t slot, uint32_t pos, uint32_t enchantment)  { write(playerData()->visible_items[slot].unk0[pos], enchantment); }
#endif
//VisibleItem end

uint64_t Player::getVendorBuybackSlot(uint8_t slot) const { return playerData()->vendor_buy_back_slot[slot]; }
void Player::setVendorBuybackSlot(uint8_t slot, uint64_t guid) { write(playerData()->vendor_buy_back_slot[slot], guid); }

uint64_t Player::getFarsightGuid() const { return playerData()->farsight_guid; }
void Player::setFarsightGuid(uint64_t farsightGuid) { write(playerData()->farsight_guid, farsightGuid); }

#if VERSION_STRING > Classic
uint64_t Player::getKnownTitles(uint8_t index) const { return playerData()->field_known_titles[index]; }
void Player::setKnownTitles(uint8_t index, uint64_t title) { write(playerData()->field_known_titles[index], title); }
#endif

#if VERSION_STRING > Classic
uint32_t Player::getChosenTitle() const { return playerData()->chosen_title; }
void Player::setChosenTitle(uint32_t title) { write(playerData()->chosen_title, title); }
#endif

#if VERSION_STRING == WotLK
uint64_t Player::getKnownCurrencies() const { return playerData()->field_known_currencies; }
void Player::setKnownCurrencies(uint64_t currencies) { write(playerData()->field_known_currencies, currencies); }
#endif

uint32_t Player::getXp() const { return playerData()->xp; }
void Player::setXp(uint32_t xp) { write(playerData()->xp, xp); }

uint32_t Player::getNextLevelXp() const { return playerData()->next_level_xp; }
void Player::setNextLevelXp(uint32_t xp) { write(playerData()->next_level_xp, xp); }

uint32_t Player::getValueFromSkillInfoIndex(uint32_t index) const { return playerData()->skill_info[index]; }
void Player::setValueBySkillInfoIndex(uint32_t index, uint32_t value) { write(playerData()->skill_info[index], value); }

uint32_t Player::getFreeTalentPoints() const
{
#if VERSION_STRING < Cata
    return playerData()->character_points_1;
#else
    return m_specs[m_talentActiveSpec].GetTP();
#endif
}

#if VERSION_STRING < Cata
void Player::setFreeTalentPoints(uint32_t points) { write(playerData()->character_points_1, points); }
#endif

uint32_t Player::getFreePrimaryProfessionPoints() const
{
#if VERSION_STRING < Cata
    return playerData()->character_points_2;
#else
    return playerData()->character_points_1;
#endif
}

void Player::setFreePrimaryProfessionPoints(uint32_t points)
{
#if VERSION_STRING < Cata
    write(playerData()->character_points_2, points);
#else
    write(playerData()->character_points_1, points);
#endif
}

uint32_t Player::getTrackCreature() const { return playerData()->track_creatures; }
void Player::setTrackCreature(uint32_t id) { write(playerData()->track_creatures, id); }

uint32_t Player::getTrackResource() const { return playerData()->track_resources; }
void Player::setTrackResource(uint32_t id) { write(playerData()->track_resources, id); }

float Player::getBlockPercentage() const { return playerData()->block_pct; }
void Player::setBlockPercentage(float value) { write(playerData()->block_pct, value); }

float Player::getDodgePercentage() const { return playerData()->dodge_pct; }
void Player::setDodgePercentage(float value) { write(playerData()->dodge_pct, value); }

float Player::getParryPercentage() const { return playerData()->parry_pct; }
void Player::setParryPercentage(float value) { write(playerData()->parry_pct, value); }

#if VERSION_STRING >= TBC
uint32_t Player::getExpertise() const { return playerData()->expertise; }
void Player::setExpertise(uint32_t value) { write(playerData()->expertise, value); }
void Player::modExpertise(int32_t value) { setExpertise(getExpertise() + value); }

uint32_t Player::getOffHandExpertise() const { return playerData()->offhand_expertise; }
void Player::setOffHandExpertise(uint32_t value) { write(playerData()->offhand_expertise, value); }
void Player::modOffHandExpertise(int32_t value) { setOffHandExpertise(getOffHandExpertise() + value); }
#endif

float Player::getMeleeCritPercentage() const { return playerData()->crit_pct; }
void Player::setMeleeCritPercentage(float value) { write(playerData()->crit_pct, value); }

float Player::getRangedCritPercentage() const { return playerData()->ranged_crit_pct; }
void Player::setRangedCritPercentage(float value) { write(playerData()->ranged_crit_pct, value); }

#if VERSION_STRING >= TBC
float Player::getOffHandCritPercentage() const { return playerData()->offhand_crit_pct; }
void Player::setOffHandCritPercentage(float value) { write(playerData()->offhand_crit_pct, value); }

float Player::getSpellCritPercentage(uint8_t school) const { return playerData()->spell_crit_pct[school]; }
void Player::setSpellCritPercentage(uint8_t school, float value) { write(playerData()->spell_crit_pct[school], value); }

uint32_t Player::getShieldBlock() const { return playerData()->shield_block; }
void Player::setShieldBlock(uint32_t value) { write(playerData()->shield_block, value); }
#endif

#if VERSION_STRING >= WotLK
float Player::getShieldBlockCritPercentage() const { return playerData()->shield_block_crit_pct; }
void Player::setShieldBlockCritPercentage(float value) { write(playerData()->shield_block_crit_pct, value); }
#endif

uint32_t Player::getExploredZone(uint32_t idx) const
{
    ARCEMU_ASSERT(idx < WOWPLAYER_EXPLORED_ZONES_COUNT)

    return playerData()->explored_zones[idx];
}

void Player::setExploredZone(uint32_t idx, uint32_t data)
{
    ARCEMU_ASSERT(idx < WOWPLAYER_EXPLORED_ZONES_COUNT)

    write(playerData()->explored_zones[idx], data);
}

uint32_t Player::getSelfResurrectSpell() const { return playerData()->self_resurrection_spell; }
void Player::setSelfResurrectSpell(uint32_t spell) { write(playerData()->self_resurrection_spell, spell); }

uint32_t Player::getWatchedFaction() const { return playerData()->field_watched_faction_idx; }
void Player::setWatchedFaction(uint32_t factionId) { write(playerData()->field_watched_faction_idx, factionId); }

#if VERSION_STRING == Classic
float Player::getManaRegeneration() const { return m_manaRegeneration; }
void Player::setManaRegeneration(float value) { m_manaRegeneration = value; }

float Player::getManaRegenerationWhileCasting() const { return m_manaRegenerationWhileCasting; }
void Player::setManaRegenerationWhileCasting(float value) { m_manaRegenerationWhileCasting = value; }
#elif VERSION_STRING == TBC
float Player::getManaRegeneration() const { return playerData()->field_mod_mana_regen; }
void Player::setManaRegeneration(float value) { write(playerData()->field_mod_mana_regen, value); }

float Player::getManaRegenerationWhileCasting() const { return playerData()->field_mod_mana_regen_interrupt; }
void Player::setManaRegenerationWhileCasting(float value) { write(playerData()->field_mod_mana_regen_interrupt, value); }
#endif

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

#if VERSION_STRING >= WotLK
float Player::getRuneRegen(uint8_t rune) const { return playerData()->rune_regen[rune]; }
void Player::setRuneRegen(uint8_t rune, float regen) { write(playerData()->rune_regen[rune], regen); }
#endif

uint32_t Player::getRestStateXp() const { return playerData()->rest_state_xp; }
void Player::setRestStateXp(uint32_t xp)  { write(playerData()->rest_state_xp, xp); }

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

#if VERSION_STRING == Classic
uint32_t Player::getResistanceBuffModPositive(uint8_t type) const { return playerData()->resistance_buff_mod_positive[type]; }
void Player::setResistanceBuffModPositive(uint8_t type, uint32_t value) { write(playerData()->resistance_buff_mod_positive[type], value); }

uint32_t Player::getResistanceBuffModNegative(uint8_t type) const { return playerData()->resistance_buff_mod_negative[type]; }
void Player::setResistanceBuffModNegative(uint8_t type, uint32_t value) { write(playerData()->resistance_buff_mod_negative[type], value); }
#endif

uint32_t Player::getModDamageDonePositive(uint16_t school) const { return playerData()->field_mod_damage_done_positive[school]; }
void Player::setModDamageDonePositive(uint16_t school, uint32_t value) { write(playerData()->field_mod_damage_done_positive[school], value); }
void Player::modModDamageDonePositive(uint16_t school, int32_t value) { setModDamageDonePositive(school, getModDamageDonePositive(school) + value); }

uint32_t Player::getModDamageDoneNegative(uint16_t school) const { return playerData()->field_mod_damage_done_negative[school]; }
void Player::setModDamageDoneNegative(uint16_t school, uint32_t value) { write(playerData()->field_mod_damage_done_negative[school], value); }
void Player::modModDamageDoneNegative(uint16_t school, int32_t value) { setModDamageDoneNegative(school, getModDamageDoneNegative(school) + value); }

float Player::getModDamageDonePct(uint8_t shool) const { return playerData()->field_mod_damage_done_pct[shool]; }
void Player::setModDamageDonePct(float damagePct, uint8_t shool) { write(playerData()->field_mod_damage_done_pct[shool], damagePct); }

#if VERSION_STRING >= TBC
uint32_t Player::getModHealingDone() const { return playerData()->field_mod_healing_done; }
void Player::setModHealingDone(uint32_t value) { write(playerData()->field_mod_healing_done, value); }
void Player::modModHealingDone(int32_t value) { setModHealingDone(getModHealingDone() + value); }

uint32_t Player::getModTargetResistance() const { return playerData()->field_mod_target_resistance; }
void Player::setModTargetResistance(uint32_t value) { write(playerData()->field_mod_target_resistance, value); }
void Player::modModTargetResistance(int32_t value) { setModTargetResistance(getModTargetResistance() + value); }

uint32_t Player::getModTargetPhysicalResistance() const { return playerData()->field_mod_target_physical_resistance; }
void Player::setModTargetPhysicalResistance(uint32_t value) { write(playerData()->field_mod_target_physical_resistance, value); }
void Player::modModTargetPhysicalResistance(int32_t value) { setModTargetPhysicalResistance(getModTargetPhysicalResistance() + value); }
#endif

uint32_t Player::getPlayerFieldBytes() const { return playerData()->player_field_bytes.raw; }
void Player::setPlayerFieldBytes(uint32_t bytes) { write(playerData()->player_field_bytes.raw, bytes); }

uint8_t Player::getActionBarId() const { return playerData()->player_field_bytes.s.actionBarId; }
void Player::setActionBarId(uint8_t actionBarId) { write(playerData()->player_field_bytes.s.actionBarId, actionBarId); }

#if VERSION_STRING < Cata
uint32_t Player::getAmmoId() const { return playerData()->ammo_id; }
void Player::setAmmoId(uint32_t id) { write(playerData()->ammo_id, id); }
#endif

uint32_t Player::getBuybackPriceSlot(uint8_t slot) const { return playerData()->field_buy_back_price[slot]; }
void Player::setBuybackPriceSlot(uint8_t slot, uint32_t price) { write(playerData()->field_buy_back_price[slot], price); }

uint32_t Player::getBuybackTimestampSlot(uint8_t slot) const { return playerData()->field_buy_back_timestamp[slot]; }
void Player::setBuybackTimestampSlot(uint8_t slot, uint32_t timestamp) { write(playerData()->field_buy_back_timestamp[slot], timestamp); }

#if VERSION_STRING > Classic
uint32_t Player::getFieldKills() const { return playerData()->field_kills; }
void Player::setFieldKills(uint32_t kills) { write(playerData()->field_kills, kills); }
#endif

#if VERSION_STRING > Classic
#if VERSION_STRING < Cata
    uint32_t Player::getContributionToday() const { return playerData()->field_contribution_today; }
    void Player::setContributionToday(uint32_t contribution) { write(playerData()->field_contribution_today, contribution); }

    uint32_t Player::getContributionYesterday() const { return playerData()->field_contribution_yesterday; }
    void Player::setContributionYesterday(uint32_t contribution) { write(playerData()->field_contribution_yesterday, contribution); }
#endif
#endif

uint32_t Player::getLifetimeHonorableKills() const { return playerData()->field_lifetime_honorable_kills; }
void Player::setLifetimeHonorableKills(uint32_t kills) { write(playerData()->field_lifetime_honorable_kills, kills); }

#if VERSION_STRING != Mop
uint32_t Player::getPlayerFieldBytes2() const { return playerData()->player_field_bytes_2.raw; }
void Player::setPlayerFieldBytes2(uint32_t bytes) { write(playerData()->player_field_bytes_2.raw, bytes); }
#endif

uint32_t Player::getCombatRating(uint8_t combatRating) const { return playerData()->field_combat_rating[combatRating]; }
void Player::setCombatRating(uint8_t combatRating, uint32_t value) { write(playerData()->field_combat_rating[combatRating], value); }
void Player::modCombatRating(uint8_t combatRating, int32_t value) { setCombatRating(combatRating, getCombatRating(combatRating) + value); }

#if VERSION_STRING > Classic
    // field_arena_team_info start
uint32_t Player::getArenaTeamId(uint8_t teamSlot) const { return playerData()->field_arena_team_info[teamSlot].team_id; }
void Player::setArenaTeamId(uint8_t teamSlot, uint32_t teamId) { write(playerData()->field_arena_team_info[teamSlot].team_id, teamId); }

uint32_t Player::getArenaTeamMemberRank(uint8_t teamSlot) const { return playerData()->field_arena_team_info[teamSlot].member_rank; }
void Player::setArenaTeamMemberRank(uint8_t teamSlot, uint32_t rank) { write(playerData()->field_arena_team_info[teamSlot].member_rank, rank); }
    // field_arena_team_info end
#endif

uint64_t Player::getInventorySlotItemGuid(uint8_t index) const { return playerData()->inventory_slot[index]; }
void Player::setInventorySlotItemGuid(uint8_t index, uint64_t guid) { write(playerData()->inventory_slot[index], guid); }

#if VERSION_STRING > Classic
#if VERSION_STRING < Cata
uint32_t Player::getHonorCurrency() const { return playerData()->field_honor_currency; }
void Player::setHonorCurrency(uint32_t amount) { write(playerData()->field_honor_currency, amount); }
void Player::modHonorCurrency(int32_t value) { setArenaCurrency(getArenaCurrency() + value); }

uint32_t Player::getArenaCurrency() const { return playerData()->field_arena_currency; }
void Player::setArenaCurrency(uint32_t amount) { write(playerData()->field_arena_currency, amount); }
void Player::modArenaCurrency(int32_t value) { setArenaCurrency(getArenaCurrency() + value); }
#endif
#endif

#if VERSION_STRING >= WotLK
uint32_t Player::getNoReagentCost(uint8_t index) const { return playerData()->no_reagent_cost[index]; }
void Player::setNoReagentCost(uint8_t index, uint32_t value) { write(playerData()->no_reagent_cost[index], value); }

uint32_t Player::getGlyphSlot(uint16_t slot) const { return playerData()->field_glyph_slots[slot]; }
void Player::setGlyphSlot(uint16_t slot, uint32_t glyph) { write(playerData()->field_glyph_slots[slot], glyph); }

uint32_t Player::getGlyph(uint16_t slot) const { return playerData()->field_glyphs[slot]; }
void Player::setGlyph(uint16_t slot, uint32_t glyph) { write(playerData()->field_glyphs[slot], glyph); }

uint32_t Player::getGlyphsEnabled() const { return playerData()->glyphs_enabled; }
void Player::setGlyphsEnabled(uint32_t glyphs) { write(playerData()->glyphs_enabled, glyphs); }
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
#if VERSION_STRING > Classic
        case TYPE_FLY:
            data.Initialize(SMSG_FORCE_FLIGHT_SPEED_CHANGE);
            break;
        case TYPE_FLY_BACK:
            data.Initialize(SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE);
            break;
#endif
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
#if VERSION_STRING > Classic
        case TYPE_FLY:
            data.Initialize(SMSG_FORCE_FLIGHT_SPEED_CHANGE);
            break;
#endif
        case TYPE_FLY_BACK:
        case TYPE_TURN_RATE:
        case TYPE_WALK:
        case TYPE_PITCH_RATE:
            break;
#endif
    }

    data << GetNewGUID();
#if VERSION_STRING == TBC
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

#if VERSION_STRING != TBC
    BuildMovementPacket(&data);
#endif
    data << float(speed);

    SendMessageToSet(&data, true);
}

bool Player::isSpellFitByClassAndRace(uint32_t /*spell_id*/)
{
    return false;
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
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_WALK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_RUN:
        {
            data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_RUN_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_RUN_BACK:
        {
            data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE);
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_RUN_BACK_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_SWIM:
        {
            data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE);
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_SWIM_SPEED_CHANGE, speed);
            break;
        }
        case TYPE_SWIM_BACK:
        {
            data.Initialize(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE);
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_SWIM_BACK_SPEED_CHANGE, speed);
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
            obj_movement_info.writeMovementInfo(data, SMSG_FORCE_FLIGHT_SPEED_CHANGE, speed);
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
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_WALK_SPEED, speed);
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
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_RUN_SPEED, speed);
#endif
            break;
        }
        case TYPE_RUN_BACK:
        {
            data.Initialize(MSG_MOVE_SET_RUN_BACK_SPEED, 1 + 8 + 4 + 4);
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_RUN_BACK_SPEED, speed);
            break;
        }
        case TYPE_SWIM:
        {
            data.Initialize(MSG_MOVE_SET_SWIM_SPEED, 1 + 8 + 4 + 4);
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_SWIM_SPEED, speed);
            break;
        }
        case TYPE_SWIM_BACK:
        {
            data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED, 1 + 8 + 4 + 4);
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_SWIM_BACK_SPEED, speed);
            break;
        }
        case TYPE_TURN_RATE:
        {
            data.Initialize(MSG_MOVE_SET_TURN_RATE, 1 + 8 + 4 + 4);
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_TURN_RATE, speed);
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

            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[1]);
#else
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_FLIGHT_SPEED, speed);
#endif
            break;
        }
        case TYPE_FLY_BACK:
        {
            data.Initialize(MSG_MOVE_SET_FLIGHT_BACK_SPEED, 1 + 8 + 4 + 4);
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_FLIGHT_BACK_SPEED, speed);
            break;
        }
        case TYPE_PITCH_RATE:
        {
            data.Initialize(MSG_MOVE_SET_PITCH_RATE, 1 + 8 + 4 + 4);
            obj_movement_info.writeMovementInfo(data, MSG_MOVE_SET_PITCH_RATE, speed);
            break;
        }
    }

    SendMessageToSet(&data, true);
}

bool Player::isSpellFitByClassAndRace(uint32_t spell_id)
{
    auto racemask = getRaceMask();
    auto classmask = getClassMask();

    auto bounds = sObjectMgr.GetSkillLineAbilityMapBounds(spell_id);
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

#endif

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

    if ((movementInfo.hasMovementFlag(MOVEFLAG_TURNING_MASK)) || m_isTurning)
    {
        auraInterruptFlags |= AURA_INTERRUPT_ON_TURNING;
    }

    RemoveAurasByInterruptFlag(auraInterruptFlags);
}

void Player::handleBreathing(MovementInfo const& movementInfo, WorldSession* session)
{
    if (!worldConfig.server.enableBreathing || m_cheats.hasFlyCheat || m_isWaterBreathingEnabled || !isAlive() || m_cheats.hasGodModeCheat)
    {
        if (m_underwaterState & UNDERWATERSTATE_SWIMMING)
            m_underwaterState &= ~UNDERWATERSTATE_SWIMMING;

        if (m_underwaterState & UNDERWATERSTATE_UNDERWATER)
        {
            m_underwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            SendMirrorTimer(MIRROR_TYPE_BREATH, m_underwaterTime, m_underwaterMaxTime, -1);
        }

        if (session->m_bIsWLevelSet)
        {
            if (movementInfo.getPosition()->z + m_noseLevel > session->m_wLevel)
            {
                RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_LEAVE_WATER);
                session->m_bIsWLevelSet = false;
            }
        }

        return;
    }

    if (movementInfo.hasMovementFlag(MOVEFLAG_SWIMMING) && !(m_underwaterState & UNDERWATERSTATE_SWIMMING))
    {
        RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_ENTER_WATER);

        if (!session->m_bIsWLevelSet)
        {
            session->m_wLevel = movementInfo.getPosition()->z + m_noseLevel * 0.95f;
            session->m_bIsWLevelSet = true;
        }

        m_underwaterState |= UNDERWATERSTATE_SWIMMING;
    }

#if VERSION_STRING <= WotLK
    if (!movementInfo.hasMovementFlag(MOVEFLAG_SWIMMING) && movementInfo.flags != MOVEFLAG_MOVE_STOP && m_underwaterState & UNDERWATERSTATE_SWIMMING)
#else
    if (!movementInfo.hasMovementFlag(MOVEFLAG_SWIMMING) && movementInfo.flags != MOVEFLAG_NONE && m_underwaterState & UNDERWATERSTATE_SWIMMING)
#endif
    {
        if (movementInfo.getPosition()->z + m_noseLevel > session->m_wLevel)
        {
            RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_LEAVE_WATER);
            session->m_bIsWLevelSet = false;

            m_underwaterState &= ~UNDERWATERSTATE_SWIMMING;
        }
    }

    if (m_underwaterState & UNDERWATERSTATE_SWIMMING && !(m_underwaterState & UNDERWATERSTATE_UNDERWATER))
    {
        if (movementInfo.getPosition()->z + m_noseLevel < session->m_wLevel)
        {
            m_underwaterState |= UNDERWATERSTATE_UNDERWATER;
            SendMirrorTimer(MIRROR_TYPE_BREATH, m_underwaterTime, m_underwaterMaxTime, -1);
        }
    }

    if (m_underwaterState & UNDERWATERSTATE_SWIMMING && m_underwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        if (movementInfo.getPosition()->z + m_noseLevel > session->m_wLevel)
        {
            m_underwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            SendMirrorTimer(MIRROR_TYPE_BREATH, m_underwaterTime, m_underwaterMaxTime, 10);
        }
    }

    if (!(m_underwaterState & UNDERWATERSTATE_SWIMMING) && m_underwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        if (movementInfo.getPosition()->z + m_noseLevel > session->m_wLevel)
        {
            m_underwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            SendMirrorTimer(MIRROR_TYPE_BREATH, m_underwaterTime, m_underwaterMaxTime, 10);
        }
    }
}

bool Player::isInCity() const
{
    const auto at = GetMapMgr()->GetArea(GetPositionX(), GetPositionY(), GetPositionZ());
    if (at != nullptr)
    {
        ::DBC::Structures::AreaTableEntry const* zt = nullptr;
        if (at->zone)
            zt = MapManagement::AreaManagement::AreaStorage::GetAreaById(at->zone);

        bool areaIsCity = at->flags & MapManagement::AreaManagement::AREA_CITY_AREA || at->flags & MapManagement::AreaManagement::AREA_CITY;
        bool zoneIsCity = zt && (zt->flags & MapManagement::AreaManagement::AREA_CITY_AREA || zt->flags & MapManagement::AreaManagement::AREA_CITY);

        return (areaIsCity || zoneIsCity);
    }

    return false;
}

//\todo: find another solution for this
void Player::initialiseNoseLevel()
{
    // Set the height of the player
    switch (getRace())
    {
    case RACE_HUMAN:
        // female
        if (getGender())
            m_noseLevel = 1.72f;
        // male
        else
            m_noseLevel = 1.78f;
        break;
    case RACE_ORC:
        if (getGender())
            m_noseLevel = 1.82f;
        else
            m_noseLevel = 1.98f;
        break;
    case RACE_DWARF:
        if (getGender())
            m_noseLevel = 1.27f;
        else
            m_noseLevel = 1.4f;
        break;
    case RACE_NIGHTELF:
        if (getGender())
            m_noseLevel = 1.84f;
        else
            m_noseLevel = 2.13f;
        break;
    case RACE_UNDEAD:
        if (getGender())
            m_noseLevel = 1.61f;
        else
            m_noseLevel = 1.8f;
        break;
    case RACE_TAUREN:
        if (getGender())
            m_noseLevel = 2.48f;
        else
            m_noseLevel = 2.01f;
        break;
    case RACE_GNOME:
        if (getGender())
            m_noseLevel = 1.06f;
        else
            m_noseLevel = 1.04f;
        break;
#if VERSION_STRING >= Cata
    case RACE_GOBLIN:
        if (getGender())
            m_noseLevel = 1.06f;
        else
            m_noseLevel = 1.04f;
        break;
#endif
    case RACE_TROLL:
        if (getGender())
            m_noseLevel = 2.02f;
        else
            m_noseLevel = 1.93f;
        break;
#if VERSION_STRING > Classic
    case RACE_BLOODELF:
        if (getGender())
            m_noseLevel = 1.83f;
        else
            m_noseLevel = 1.93f;
        break;
    case RACE_DRAENEI:
        if (getGender())
            m_noseLevel = 2.09f;
        else
            m_noseLevel = 2.36f;
        break;
#endif
#if VERSION_STRING >= Cata
    case RACE_WORGEN:
        if (getGender())
            m_noseLevel = 1.72f;
        else
            m_noseLevel = 1.78f;
        break;
#endif
    }
}

void Player::setTransferStatus(uint8_t status) { m_transferStatus = status; }
uint8_t Player::getTransferStatus() const { return m_transferStatus; }
bool Player::isTransferPending() const { return getTransferStatus() == TRANSFER_PENDING; }

//////////////////////////////////////////////////////////////////////////////////////////
// Commands
void Player::disableSummoning(bool disable) { m_disableSummoning = disable; }
bool Player::isSummoningDisabled() const { return m_disableSummoning; }
void Player::disableAppearing(bool disable) { m_disableAppearing = disable; }
bool Player::isAppearingDisabled() const { return m_disableAppearing; }

bool Player::isBanned() const
{
    if (m_banned)
    {
        if (m_banned < 100 || static_cast<uint32_t>(UNIXTIME) < m_banned)
            return true;
    }
    return false;
}

void Player::setBanned(uint32_t timestamp /*= 4*/, std::string Reason /*= ""*/) { m_banned = timestamp; m_banreason = Reason; }
void Player::unsetBanned() { m_banned = 0; }
std::string Player::getBanReason() const { return m_banreason; }

GameObject* Player::getSelectedGo() const
{
    if (m_GMSelectedGO)
        return GetMapMgr()->GetGameObject(static_cast<uint32_t>(m_GMSelectedGO));

    return nullptr;
}

void Player::setSelectedGo(uint64_t guid) { m_GMSelectedGO = guid; }

void Player::kickFromServer(uint32_t delay)
{
    if (delay)
    {
        m_kickDelay = delay;
        sEventMgr.AddEvent(this, &Player::eventKickFromServer, EVENT_PLAYER_KICK, 1000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
    else
    {
        m_kickDelay = 0;
        eventKickFromServer();
    }
}

void Player::eventKickFromServer()
{
    if (m_kickDelay)
    {
        if (m_kickDelay < 1500)
            m_kickDelay = 0;
        else
            m_kickDelay -= 1000;

        sChatHandler.BlueSystemMessage(GetSession(), "You will be removed from the server in %u seconds.", static_cast<uint32>(m_kickDelay / 1000));
    }
    else
    {
        sEventMgr.RemoveEvents(this, EVENT_PLAYER_KICK);
        GetSession()->LogoutPlayer(true);
    }
}

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

void Player::applyLevelInfo(uint32_t newLevel)
{
    // Save current level
    const auto previousLevel = getLevel();

    if (!m_FirstLogin)
    {
        const auto previousLevelInfo = lvlinfo;

        // Get new level info
        lvlinfo = sObjectMgr.GetLevelInfo(getRace(), getClass(), newLevel);
        if (lvlinfo == nullptr)
            return;

        // Small chance that you die at the same time you level up, and you may enter in a weird state
        if (isDead())
            ResurrectPlayer();

        setLevel(newLevel);

        // Set new base health and mana
        //\ TODO: LevelInfo base health and mana stats already have stamina and intellect calculated into them
        const auto levelone = sObjectMgr.GetLevelInfo(getRace(), getClass(), 1);
        if (levelone != nullptr)
        {
            setBaseHealth(lvlinfo->HP - ((lvlinfo->Stat[STAT_STAMINA] - levelone->Stat[STAT_STAMINA]) * 10));
            setBaseMana(lvlinfo->Mana - ((lvlinfo->Stat[STAT_INTELLECT] - levelone->Stat[STAT_INTELLECT]) * 15));
        }
        else
        {
            setBaseHealth(lvlinfo->HP);
            setBaseMana(lvlinfo->Mana);
        }

        // Set new base stats
        for (uint8_t i = 0; i < STAT_COUNT; ++i)
        {
            BaseStats[i] = lvlinfo->Stat[i];
            CalcStat(i);
        }
        UpdateStats();

        // Set current health
        setHealth(getMaxHealth());
        // Restore powers to full
        setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));
        setPower(POWER_TYPE_FOCUS, getMaxPower(POWER_TYPE_FOCUS));
        setPower(POWER_TYPE_ENERGY, getMaxPower(POWER_TYPE_ENERGY));
#if VERSION_STRING >= WotLK
        setPower(POWER_TYPE_RUNES, getMaxPower(POWER_TYPE_RUNES));
#endif

        // Send levelup info packet
        sendLevelupInfoPacket(
            newLevel,
            lvlinfo->HP - previousLevelInfo->HP,
            lvlinfo->Mana - previousLevelInfo->Mana,
            lvlinfo->Stat[STAT_STRENGTH] - previousLevelInfo->Stat[STAT_STRENGTH],
            lvlinfo->Stat[STAT_AGILITY] - previousLevelInfo->Stat[STAT_AGILITY],
            lvlinfo->Stat[STAT_STAMINA] - previousLevelInfo->Stat[STAT_STAMINA],
            lvlinfo->Stat[STAT_INTELLECT] - previousLevelInfo->Stat[STAT_INTELLECT],
            lvlinfo->Stat[STAT_SPIRIT] - previousLevelInfo->Stat[STAT_SPIRIT]);
    }

    // Update max skill level
    _UpdateMaxSkillCounts();

    if (newLevel > previousLevel || m_FirstLogin)
    {
        setInitialTalentPoints();
    }
    else if (newLevel != previousLevel)
    {
        // Reset talents if new level is lower than the previous level
        resetAllTalents();
    }

    m_playerInfo->lastLevel = previousLevel;

#if VERSION_STRING >= WotLK
    updateGlyphs();
    GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL);
#endif

    // Send script hooks
    if (m_FirstLogin)
        sHookInterface.OnFirstEnterWorld(this);
    else
        sHookInterface.OnPostLevelUp(this);

    // If player is warlock and has a summoned pet, its level should match owner's
    if (getClass() == WARLOCK)
    {
        const auto pet = GetSummon();
        if (pet != nullptr && pet->IsInWorld() && pet->isAlive())
        {
            pet->setLevel(newLevel);
            pet->ApplyStatsForLevel();
            pet->UpdateSpellList();
        }
    }

    // Send talent info to client
    smsg_TalentsInfo(false);

    // Reset current played time
    m_playedtime[0] = 0;
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
bool Player::isClassMonk() { return false; }
bool Player::isClassDruid() { return false; }

Player* Player::getPlayerOwner()
{
    if (getCharmedByGuid() != 0)
    {
        const auto charmerUnit = GetMapMgrUnit(getCharmedByGuid());
        if (charmerUnit != nullptr && charmerUnit->isPlayer())
            return dynamic_cast<Player*>(charmerUnit);
    }

    return this;
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
// Stats
void Player::setInitialPlayerData()
{
    ARCEMU_ASSERT(lvlinfo != nullptr);

    //\ TODO: LevelInfo base health and mana stats already have stamina and intellect calculated into them
    const auto levelone = sObjectMgr.GetLevelInfo(getRace(), getClass(), 1);
    if (levelone != nullptr)
    {
        setBaseHealth(lvlinfo->HP - ((lvlinfo->Stat[STAT_STAMINA] - levelone->Stat[STAT_STAMINA]) * 10));
        setBaseMana(lvlinfo->Mana - ((lvlinfo->Stat[STAT_INTELLECT] - levelone->Stat[STAT_INTELLECT]) * 15));
    }
    else
    {
        setBaseHealth(lvlinfo->HP);
        setBaseMana(lvlinfo->Mana);
    }

    // Set max health and powers
    setMaxHealth(lvlinfo->HP);
    // First initialize all power fields to 0
    for (uint8_t power = POWER_TYPE_MANA; power < TOTAL_PLAYER_POWER_TYPES; ++power)
        setMaxPower(static_cast<PowerType>(power), 0);

    // Next set correct power for each class
    switch (getClass())
    {
        case WARRIOR:
        {
            setMaxPower(POWER_TYPE_RAGE, info->rage);
        } break;
#if VERSION_STRING >= Cata
        case HUNTER:
        {
            setMaxPower(POWER_TYPE_FOCUS, info->focus);
        } break;
#endif
        case ROGUE:
        {
            setMaxPower(POWER_TYPE_ENERGY, info->energy);
        } break;
#if VERSION_STRING >= WotLK
        case DEATHKNIGHT:
        {
            setMaxPower(POWER_TYPE_RUNES, 8);
            setMaxPower(POWER_TYPE_RUNIC_POWER, 1000);
        } break;
#endif
        default:
        {
#if VERSION_STRING >= Cata
            // Another switch case to set secondary powers
            switch (getClass())
            {
                case PALADIN:
                    setMaxPower(POWER_TYPE_HOLY_POWER, 3);
                    break;
                case WARLOCK:
                    setMaxPower(POWER_TYPE_SOUL_SHARDS, 3);
                    break;
                case DRUID:
                    setMaxPower(POWER_TYPE_ECLIPSE, 100);
                    break;
                default:
                    break;
            }
#endif
            setMaxPower(POWER_TYPE_MANA, getBaseMana());
        } break;
    }

    setMinDamage(0.0f);
    setMaxDamage(0.0f);
    setMinOffhandDamage(0.0f);
    setMaxOffhandDamage(0.0f);
    setMinRangedDamage(0.0f);
    setMaxRangedDamage(0.0f);

    setBaseAttackTime(MELEE, 2000);
    setBaseAttackTime(OFFHAND, 2000);
    setBaseAttackTime(RANGED, 2000);

    setAttackPower(0);
    setAttackPowerMods(0);
    setAttackPowerMultiplier(0.0f);
    setRangedAttackPower(0);
    setRangedAttackPowerMods(0);
    setRangedAttackPowerMultiplier(0.0f);

    setBlockPercentage(0.0f);
    setDodgePercentage(0.0f);
    setParryPercentage(0.0f);
    setMeleeCritPercentage(0.0f);
    setRangedCritPercentage(0.0f);
#if VERSION_STRING >= TBC
    setExpertise(0);
    setOffHandExpertise(0);
    setOffHandCritPercentage(0.0f);
    setShieldBlock(0);
#endif
#if VERSION_STRING >= WotLK
    setShieldBlockCritPercentage(0.0f);
#endif

#if VERSION_STRING >= TBC
    setModHealingDone(0);

    setModTargetResistance(0);
    setModTargetPhysicalResistance(0);
#endif

    setModCastSpeed(1.0f);

    for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; ++i)
    {
        setModDamageDonePositive(i, 0);
        setModDamageDoneNegative(i, 0);
        setModDamageDonePct(1.0f, i);

#if VERSION_STRING >= TBC
        setSpellCritPercentage(i, 0.0f);
#endif
        setResistance(i, 0);

        setPowerCostModifier(i, 0);
        setPowerCostMultiplier(i, 0.0f);
    }

#if VERSION_STRING >= WotLK
    for (uint8_t i = 0; i < WOWPLAYER_NO_REAGENT_COST_COUNT; ++i)
    {
        setNoReagentCost(i, 0);
    }
#endif

    for (uint8_t i = 0; i < MAX_PCR; ++i)
        setCombatRating(i, 0);

    for (uint8_t i = 0; i < STAT_COUNT; ++i)
    {
        BaseStats[i] = lvlinfo->Stat[i];
        CalcStat(i);
    }

    UpdateStats();

    setMaxLevel(worldConfig.player.playerLevelCap);

    setPvpFlags(getPvpFlags() | U_FIELD_BYTES_FLAG_PVP);
    addUnitFlags(UNIT_FLAG_PVP_ATTACKABLE);
#if VERSION_STRING >= TBC
    addUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
#endif

    setFreePrimaryProfessionPoints(worldConfig.player.maxProfessions);

    // Set current health and power after stats are loaded
    setHealth(getMaxHealth());
    setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));
    setPower(POWER_TYPE_RAGE, 0);
    setPower(POWER_TYPE_FOCUS, getMaxPower(POWER_TYPE_FOCUS));
    setPower(POWER_TYPE_ENERGY, getMaxPower(POWER_TYPE_ENERGY));
#if VERSION_STRING >= WotLK
    setPower(POWER_TYPE_RUNES, getMaxPower(POWER_TYPE_RUNES));
    setPower(POWER_TYPE_RUNIC_POWER, 0);
#endif
}

void Player::regeneratePlayerPowers(uint16_t diff)
{
    // Rage and Runic Power (neither decays while in combat)
    if ((isClassDeathKnight() || isClassDruid() || isClassWarrior()) && !CombatStatus.IsInCombat())
    {
        m_rageRunicPowerRegenerateTimer += diff;
        if (m_rageRunicPowerRegenerateTimer >= REGENERATION_INTERVAL_RAGE_RUNIC_POWER)
        {
            if (isClassDruid() || isClassWarrior())
                regeneratePower(POWER_TYPE_RAGE);
#if VERSION_STRING >= WotLK
            if (isClassDeathKnight())
                regeneratePower(POWER_TYPE_RUNIC_POWER);
#endif
            m_rageRunicPowerRegenerateTimer = 0;
        }
    }

#if VERSION_STRING >= Cata
    // Holy Power (does not decay while in combat)
    if (isClassPaladin() && !CombatStatus.IsInCombat())
    {
        m_holyPowerRegenerateTimer += diff;
        if (m_holyPowerRegenerateTimer >= REGENERATION_INTERVAL_HOLY_POWER)
        {
            regeneratePower(POWER_TYPE_HOLY_POWER);
            m_holyPowerRegenerateTimer = 0;
        }
    }
#endif

    // Food/Drink visual effect
    // Confirmed from sniffs that the timer keeps going on even when there is no food/drink aura
    if (diff >= m_foodDrinkSpellVisualTimer)
    {
        // Find food/drink aura
        auto foundFood = false, foundDrink = false;
        for (const auto& aur : m_auras)
        {
            if (aur == nullptr)
                continue;

            if (aur->hasAuraEffect(SPELL_AURA_MOD_REGEN) && aur->getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
            {
                // Food takes priority over drink
                foundFood = true;
                break;
            }

            if (aur->hasAuraEffect(SPELL_AURA_MOD_POWER_REGEN) && aur->getSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
            {
                // Don't break here, try find a food aura
                foundDrink = true;
            }
        }

        if (foundFood)
            playSpellVisual(SPELL_VISUAL_FOOD, 0);
        else if (foundDrink)
            playSpellVisual(SPELL_VISUAL_DRINK, 0);

        m_foodDrinkSpellVisualTimer = 5000;
    }
    else
    {
        m_foodDrinkSpellVisualTimer -= diff;
    }
}

#if VERSION_STRING >= Cata
void Player::resetHolyPowerTimer()
{
    m_holyPowerRegenerateTimer = 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Database stuff
bool Player::loadSpells(QueryResult* result)
{
    // Add initial spells on first login
    if (m_FirstLogin)
    {
        for (const auto& spellId : info->spell_list)
            mSpells.insert(spellId);

        return true;
    }

    if (result == nullptr)
        return false;

    do
    {
        const auto fields = result->Fetch();
        const auto spellId = fields[0].GetUInt32();

        const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
        if (spellInfo == nullptr)
            continue;

        mSpells.insert(spellId);
    } while (result->NextRow());

    return true;
}

bool Player::loadReputations(QueryResult* result)
{
    // Add initial reputations on first login
    if (m_FirstLogin)
    {
        _InitialReputation();
        return true;
    }

    if (result == nullptr)
        return false;

    do
    {
        const auto field = result->Fetch();

        const auto id = field[0].GetUInt32();
        const auto flag = field[1].GetUInt8();
        const auto basestanding = field[2].GetInt32();
        const auto standing = field[3].GetInt32();

        const auto faction = sFactionStore.LookupEntry(id);
        if (faction == nullptr || faction->RepListId < 0)
            continue;

        auto itr = m_reputation.find(id);
        if (itr != m_reputation.end())
            delete itr->second;

        FactionReputation* reputation = new FactionReputation;
        reputation->baseStanding = basestanding;
        reputation->standing = standing;
        reputation->flag = flag;
        m_reputation[id] = reputation;
        reputationByListId[faction->RepListId] = reputation;
    } while (result->NextRow());

    return true;
}

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
        const auto canCastAutoRepeatSpell = autoRepeatSpell->canCast(true, 0, 0);
        if (canCastAutoRepeatSpell != SPELL_CAST_SUCCESS)
        {
            if (!isAutoShot)
                interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
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

bool Player::hasSpellOnCooldown(SpellInfo const* spellInfo)
{
    const auto curTime = Util::getMSTime();

    // Check category cooldown
    if (spellInfo->getCategory() > 0)
    {
        const auto itr = m_cooldownMap[COOLDOWN_TYPE_CATEGORY].find(spellInfo->getCategory());
        if (itr != m_cooldownMap[COOLDOWN_TYPE_CATEGORY].end())
        {
            if (curTime < itr->second.ExpireTime)
                return true;

            // Cooldown has expired
            m_cooldownMap[COOLDOWN_TYPE_CATEGORY].erase(itr);
        }
    }

    // Check spell cooldown
    const auto itr = m_cooldownMap[COOLDOWN_TYPE_SPELL].find(spellInfo->getId());
    if (itr != m_cooldownMap[COOLDOWN_TYPE_SPELL].end())
    {
        if (curTime < itr->second.ExpireTime)
            return true;

        // Cooldown has expired
        m_cooldownMap[COOLDOWN_TYPE_SPELL].erase(itr);
    }

    return false;
}

bool Player::hasSpellGlobalCooldown(SpellInfo const* spellInfo)
{
    const auto curTime = Util::getMSTime();

    // Check for cooldown cheat as well
    if (spellInfo->getStartRecoveryTime() > 0 && m_globalCooldown > 0 && !m_cheats.hasCooldownCheat)
    {
        if (curTime < m_globalCooldown)
            return true;

        // Global cooldown has expired
        m_globalCooldown = 0;
    }

    return false;
}

void Player::addSpellCooldown(SpellInfo const* spellInfo, Item const* itemCaster, Spell* castingSpell/* = nullptr*/, int32_t cooldownTime/* = 0*/)
{
    const auto curTime = Util::getMSTime();
    const auto spellId = spellInfo->getId();

    // Set category cooldown
    int32_t spellCategoryCooldown = static_cast<int32_t>(spellInfo->getCategoryRecoveryTime());
    if (spellCategoryCooldown > 0 && spellInfo->getCategory() > 0)
    {
        // Add cooldown modifiers
        if (castingSpell != nullptr)
            applySpellModifiers(SPELLMOD_COOLDOWN_DECREASE, &spellCategoryCooldown, spellInfo, castingSpell);

        AddCategoryCooldown(spellInfo->getCategory(), spellCategoryCooldown + curTime, spellId, itemCaster != nullptr ? itemCaster->getEntry() : 0);
    }

    // Set spell cooldown
    int32_t spellCooldown = cooldownTime == 0 ? static_cast<int32_t>(spellInfo->getRecoveryTime()) : cooldownTime;
    if (spellCooldown > 0)
    {
        // Add cooldown modifers
        if (castingSpell != nullptr)
            applySpellModifiers(SPELLMOD_COOLDOWN_DECREASE, &spellCooldown, spellInfo, castingSpell);

        _Cooldown_Add(COOLDOWN_TYPE_SPELL, spellId, spellCooldown + curTime, spellId, itemCaster != nullptr ? itemCaster->getEntry() : 0);
    }

    // Send cooldown packet
    sendSpellCooldownPacket(spellInfo, spellCooldown > spellCategoryCooldown ? spellCooldown : spellCategoryCooldown, false);
}

void Player::addGlobalCooldown(SpellInfo const* spellInfo, Spell* castingSpell, const bool sendPacket/* = false*/)
{
    if (spellInfo->getStartRecoveryTime() == 0 && spellInfo->getStartRecoveryCategory() == 0)
        return;

    const auto curTime = Util::getMSTime();
    auto gcdDuration = static_cast<int32_t>(spellInfo->getStartRecoveryTime());

    // Apply global cooldown modifiers
    applySpellModifiers(SPELLMOD_GLOBAL_COOLDOWN, &gcdDuration, spellInfo, castingSpell);

    // Apply haste modifier only to magic spells
    if (spellInfo->getStartRecoveryCategory() == 133 && spellInfo->getDmgClass() == SPELL_DMG_TYPE_MAGIC &&
        !(spellInfo->getAttributes() & ATTRIBUTES_RANGED) && !(spellInfo->getAttributes() & ATTRIBUTES_ABILITY))
    {
        gcdDuration = static_cast<int32_t>(gcdDuration * getModCastSpeed());

        // Global cooldown cannot be shorter than 1 second or longer than 1.5 seconds
        gcdDuration = std::max(gcdDuration, 1000);
        gcdDuration = std::min(gcdDuration, 1500);
    }

    if (gcdDuration <= 0)
        return;

    m_globalCooldown = curTime + gcdDuration;

    if (sendPacket)
        sendSpellCooldownPacket(spellInfo, 0, true);
}

void Player::sendSpellCooldownPacket(SpellInfo const* spellInfo, const uint32_t duration, const bool isGcd)
{
    std::vector<SmsgSpellCooldownMap> spellMap;

    SmsgSpellCooldownMap mapMembers;
    mapMembers.spellId = spellInfo->getId();
    mapMembers.duration = duration;

    spellMap.push_back(mapMembers);

    SendMessageToSet(SmsgSpellCooldown(GetNewGUID(), isGcd, spellMap).serialise().get(), true);
}

void Player::clearCooldownForSpell(uint32_t spellId)
{
    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
    if (spellInfo == nullptr)
        return;

    // Send cooldown clear packet
    GetSession()->SendPacket(SmsgClearCooldown(spellId, getGuid()).serialise().get());

    for (uint8_t i = 0; i < NUM_COOLDOWN_TYPES; ++i)
    {
        for (auto itr = m_cooldownMap[i].begin(); itr != m_cooldownMap[i].end();)
        {
            auto cooldown = (*itr);
            if ((i == COOLDOWN_TYPE_CATEGORY && cooldown.first == spellInfo->getCategory()) ||
                (i == COOLDOWN_TYPE_SPELL && cooldown.first == spellInfo->getId()))
            {
                itr = m_cooldownMap[i].erase(itr);
            }
            else
            {
                ++itr;
            }
        }
    }
}

void Player::clearGlobalCooldown()
{
    m_globalCooldown = Util::getMSTime();
}

void Player::resetAllCooldowns()
{
    // Clear spell cooldowns
    for (const auto& spell : mSpells)
        clearCooldownForSpell(spell);

    // Clear global cooldown
    clearGlobalCooldown();

    // Clear other cooldowns, like items
    for (uint8_t i = 0; i < NUM_COOLDOWN_TYPES; ++i)
    {
        for (auto itr = m_cooldownMap[i].begin(); itr != m_cooldownMap[i].end();)
        {
            auto spellId = (*itr).second.SpellId;
            GetSession()->SendPacket(SmsgClearCooldown(spellId, getGuid()).serialise().get());
            itr = m_cooldownMap[i].erase(itr);
        }
    }

    // Clear proc cooldowns
    clearProcCooldowns();
}

#ifdef FT_GLYPHS
void Player::updateGlyphs()
{
#if VERSION_STRING == WotLK
    for (uint32_t i = 0; i < sGlyphSlotStore.GetNumRows(); ++i)
    {
        const auto glyphSlot = sGlyphSlotStore.LookupEntry(i);
        if (glyphSlot == nullptr)
            continue;

        if (glyphSlot->Slot > 0)
            setGlyphSlot(static_cast<uint16_t>(glyphSlot->Slot - 1), glyphSlot->Id);
    }
#else
    uint16_t slot = 0;
    for (uint32_t i = 0; i < sGlyphSlotStore.GetNumRows(); ++i)
    {
        const auto glyphSlot = sGlyphSlotStore.LookupEntry(i);
        if (glyphSlot != nullptr)
            setGlyphSlot(slot++, glyphSlot->Id);
    }
#endif

    const auto level = getLevel();
    uint32_t slotMask = 0;

#if VERSION_STRING == WotLK
    if (level >= 15)
        slotMask |= (GS_MASK_1 | GS_MASK_2);
    if (level >= 30)
        slotMask |= GS_MASK_3;
    if (level >= 50)
        slotMask |= GS_MASK_4;
    if (level >= 70)
        slotMask |= GS_MASK_5;
    if (level >= 80)
        slotMask |= GS_MASK_6;
#elif VERSION_STRING == Cata
    if (level >= 25)
        slotMask |= GS_MASK_LEVEL_25;
    if (level >= 50)
        slotMask |= GS_MASK_LEVEL_50;
    if (level >= 75)
        slotMask |= GS_MASK_LEVEL_75;
#elif VERSION_STRING == Mop
    // TODO
#endif

    setGlyphsEnabled(slotMask);
}
#endif

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

    if (sObjectMgr.IsSpellDisabled(talentInfo->RankID[talentRank]))
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
            if (sp->getCasterAuraState() == 0 || hasAuraState(static_cast<AuraState>(sp->getCasterAuraState()), sp, this))
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

void Player::resetAllTalents()
{
    if (m_talentSpecsCount == 1)
    {
        resetTalents();
        return;
    }

    const auto activeSpec = m_talentActiveSpec;
    resetTalents();

    if (activeSpec == SPEC_PRIMARY)
        activateTalentSpec(SPEC_SECONDARY);
    else
        activateTalentSpec(SPEC_PRIMARY);

    resetTalents();
    // Change back to the original spec
    activateTalentSpec(activeSpec);
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
        talentPoints = static_cast<uint32_t>(talentPointsAtLevel->talentPoints);
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
                dkTalentPoints = getLevel() < 55 ? 0 : talentPoints - static_cast<uint32_t>(dkBaseTalentPoints->talentPoints);
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

void Player::smsg_TalentsInfo([[maybe_unused]]bool SendPetTalents)
{
    // TODO: classic and tbc
#if VERSION_STRING < Mop
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
            for (uint8_t i = 0; i < GLYPHS_COUNT; ++i)
            {
                data << uint16_t(GetGlyph(specId, i));
            }
        }
    }
    GetSession()->SendPacket(&data);
#endif
#else
    WorldPacket data(SMSG_TALENTS_INFO, 1000);
    data << uint8_t(m_talentActiveSpec); // Which spec is active right now
    data.writeBits(m_talentSpecsCount, 19);

    size_t* wpos = new size_t[m_talentSpecsCount];
    for (int i = 0; i < m_talentSpecsCount; ++i)
    {
        wpos[i] = data.bitwpos();
        data.writeBits(0, 23);
    }

    data.flushBits();

    for (auto specId = 0; specId < m_talentSpecsCount; ++specId)
    {
        PlayerSpec spec = m_specs[specId];

        for (uint8_t i = 0; i < 6; ++i)
            data << uint16_t(GetGlyph(specId, i));

        int32 talentCount = 0;
        for (const auto talent : spec.talents)
        {
            data << uint16_t(talent.first);
            talentCount++;
        }
        data.PutBits(wpos[specId], talentCount, 23);
        data << uint32(spec.GetTP());
    }

    GetSession()->SendPacket(&data);
#endif
}

void Player::activateTalentSpec([[maybe_unused]]uint8_t specId)
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
    for (uint8_t i = 0; i < GLYPHS_COUNT; ++i)
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
    for (uint8_t i = 0; i < GLYPHS_COUNT; ++i)
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
    sendActionBars(true);

    // Reset power
    setPower(getPowerType(), 0);
    sendPowerUpdate(false);

    // Check offhand
    unEquipOffHandIfRequired();

    // Send talent points
    setInitialTalentPoints();
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
// Tutorials
uint32_t Player::getTutorialValueById(uint8_t id)
{
    ARCEMU_ASSERT(id < 8)
    return m_Tutorials[id];
}

void Player::setTutorialValueForId(uint8_t id, uint32_t value)
{
    ARCEMU_ASSERT(id < 8)
    m_Tutorials[id] = value;
    tutorialsDirty = true;
}

void Player::loadTutorials()
{
    if (auto result = CharacterDatabase.Query("SELECT * FROM tutorials WHERE playerId = %u", getGuidLow()))
    {
        auto* const fields = result->Fetch();
        for (uint8_t id = 0; id < 8; ++id)
            m_Tutorials[id] = fields[id + 1].GetUInt32();
    }
    tutorialsDirty = false;
}

void Player::saveTutorials()
{
    if (tutorialsDirty)
    {
        CharacterDatabase.Execute("DELETE FROM tutorials WHERE playerid = %u;", getGuidLow());
        CharacterDatabase.Execute("INSERT INTO tutorials VALUES('%u','%u','%u','%u','%u','%u','%u','%u','%u');", getGuidLow(), m_Tutorials[0], m_Tutorials[1], m_Tutorials[2], m_Tutorials[3], m_Tutorials[4], m_Tutorials[5], m_Tutorials[6], m_Tutorials[7]);

        tutorialsDirty = false;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Actionbar
void Player::setActionButton(uint8_t button, uint32_t action, uint8_t type, uint8_t misc)
{
    if (button >= PLAYER_ACTION_BUTTON_COUNT)
        return;

    getActiveSpec().mActions[button].Action = action;
    getActiveSpec().mActions[button].Misc = misc;
    getActiveSpec().mActions[button].Type = type;
}

void Player::sendActionBars([[maybe_unused]]bool clearBars)
{
#if VERSION_STRING < Mop
    WorldPacket data(SMSG_ACTION_BUTTONS, PLAYER_ACTION_BUTTON_SIZE + 1);

#if VERSION_STRING == WotLK
    // 0 does nothing, 1 clears bars from clientside
    data << uint8_t(clearBars ? 1 : 0);
#endif

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
    {
        // TODO: this needs investigation
        // action, as in spell id, can be and will be over uint16_t max (65535) on wotlk and cata
        // but if I send action in uint32_t, client ignores the button completely and leaves an empty button slot, or corrupts other slots as well
        // however casting the action to uint16_t seems to somehow work. I tested it with a spell id over 65535.
        // but this is not a solution and can cause undefined behaviour... (previously ActionButton::Action was stored in uint16_t)
        // I believe client accepts at most 4 bytes per button -Appled
        data << uint16_t(getActiveSpec().mActions[i].Action);
#if VERSION_STRING < WotLK
        data << getActiveSpec().mActions[i].Type;
        data << getActiveSpec().mActions[i].Misc;
#else
        // Since Wotlk misc needs to be sent before type
        data << getActiveSpec().mActions[i].Misc;
        data << getActiveSpec().mActions[i].Type;
#endif
    }
#else
    WorldPacket data(SMSG_ACTION_BUTTONS, (PLAYER_ACTION_BUTTON_COUNT * 8) + 1);

    uint8_t buttons[PLAYER_ACTION_BUTTON_COUNT][8];

    // Bits
    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][4]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][5]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][3]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][1]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][6]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][7]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][0]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.writeBit(buttons[i][2]);

    // Data
    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][0]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][1]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][4]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][6]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][7]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][2]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][5]);

    for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; ++i)
        data.WriteByteSeq(buttons[i][3]);
#endif

#if VERSION_STRING >= Cata
    // 0 does nothing, 1 clears bars from clientside
    data << uint8_t(clearBars ? 1 : 0);
#endif

    GetSession()->SendPacket(&data);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Auction
void Player::sendAuctionCommandResult(Auction* auction, uint32_t action, uint32_t errorCode, uint32_t bidError)
{
    const auto auctionId = auction ? auction->Id : 0;

#if VERSION_STRING >= Cata
    uint64_t outBid = 0;
    uint64_t highestBid = 0;
#else
    uint32_t outBid = 0;
    uint32_t highestBid = 0;
#endif
    uint64_t highestBidderGuid = 0;

    if (auction)
    {
        outBid = auction->highestBid ? auction->getAuctionOutBid() : 0;
        highestBid = auction->highestBid;
        highestBidderGuid = auction->highestBidderGuid;
    }

    SendPacket(SmsgAuctionCommandResult(auctionId, action, errorCode, outBid, highestBid, bidError, highestBidderGuid).serialise().get());
}

//////////////////////////////////////////////////////////////////////////////////////////
// Trade
Player* Player::getTradeTarget() const
{
    return m_TradeData != nullptr ? m_TradeData->getTradeTarget() : nullptr;
}

TradeData* Player::getTradeData() const
{
    return m_TradeData;
}

void Player::cancelTrade(bool sendToSelfAlso, bool silently /*= false*/)
{
    if (m_TradeData != nullptr)
    {
        if (sendToSelfAlso)
            GetSession()->sendTradeResult(TRADE_STATUS_CANCELLED);

        auto tradeTarget = m_TradeData->getTradeTarget();
        if (tradeTarget != nullptr)
        {
            if (!silently)
                tradeTarget->GetSession()->sendTradeResult(TRADE_STATUS_CANCELLED);

            delete tradeTarget->m_TradeData;
            tradeTarget->m_TradeData = nullptr;
        }

        delete m_TradeData;
        m_TradeData = nullptr;
    }
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
    {
        needToRemove = !canDualWield2H();
    }
    else
    {
        // Player has something in offhand, check if main hand is a two-handed weapon
        const auto mainHandWeapon = getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        if (mainHandWeapon != nullptr && mainHandWeapon->getItemProperties()->InventoryType == INVTYPE_2HWEAPON)
            needToRemove = !canDualWield2H();
        else
        {
            // Main hand nor offhand is a two-handed weapon, check if player can dual wield one-handed weapons
            if (offHandWeapon->isWeapon())
                needToRemove = !canDualWield();
            else
                needToRemove = false; // Offhand is not a weapon
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

    return offHandItem->isWeapon();
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
// Difficulty
void Player::setDungeonDifficulty(uint8_t diff)
{
    m_dungeonDifficulty = diff;
}

uint8_t Player::getDungeonDifficulty()
{
    return m_dungeonDifficulty;
}

void Player::setRaidDifficulty(uint8_t diff)
{
    m_raidDifficulty = diff;
}

uint8_t Player::getRaidDifficulty()
{
    return m_raidDifficulty;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Corpse
void Player::setCorpseData(LocationVector position, int32_t instanceId)
{
    m_corpseData.location = position;
    m_corpseData.instanceId = instanceId;
}

LocationVector Player::getCorpseLocation() const
{
    return m_corpseData.location;
}

int32_t Player::getCorpseInstanceId() const
{
    return m_corpseData.instanceId;
}

void Player::setAllowedToCreateCorpse(bool allowed)
{
    isCorpseCreationAllowed = allowed;
}

bool Player::isAllowedToCreateCorpse() const
{
    return isCorpseCreationAllowed;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Bind
void Player::setBindPoint(float x, float y, float z, uint32_t mapId, uint32_t zoneId)
{
    m_bindData.location = { x, y, z };
    m_bindData.mapId = mapId;
    m_bindData.zoneId = zoneId;
}

LocationVector Player::getBindPosition() const { return m_bindData.location; }
uint32_t Player::getBindMapId() const { return m_bindData.mapId; }
uint32_t Player::getBindZoneId() const { return m_bindData.zoneId; }

//////////////////////////////////////////////////////////////////////////////////////////
// Battleground Entry
void Player::setBGEntryPoint(float x, float y, float z, float o, uint32_t mapId, int32_t instanceId)
{
    m_bgEntryData.location = { x, y, z, o };
    m_bgEntryData.mapId = mapId;
    m_bgEntryData.instanceId = instanceId;
}

LocationVector Player::getBGEntryPosition() const { return m_bindData.location; }
uint32_t Player::getBGEntryMapId() const { return m_bindData.mapId; }
int32_t Player::getBGEntryInstanceId() const { return m_bindData.zoneId; }

//////////////////////////////////////////////////////////////////////////////////////////
// Guild
void Player::setInvitedByGuildId(uint32_t GuildId) { m_invitedByGuildId = GuildId; }
uint32_t Player::getInvitedByGuildId() const { return m_invitedByGuildId; }
Guild* Player::getGuild() { return getGuildId() ? sGuildMgr.getGuildById(getGuildId()) : nullptr; }
bool Player::isInGuild() { return getGuild() != nullptr; }

uint32_t Player::getGuildRankFromDB()
{
    if (auto result = CharacterDatabase.Query("SELECT playerid, guildRank FROM guild_members WHERE playerid = %u", WoWGuid::getGuidLowPartFromUInt64(getGuid())))
    {
        Field* fields = result->Fetch();
        return fields[1].GetUInt32();
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Group
void Player::setGroupInviterId(uint32_t inviterId) { m_GroupInviter = inviterId; }
uint32_t Player::getGroupInviterId() const { return m_GroupInviter; }
bool Player::isAlreadyInvitedToGroup() const { return m_GroupInviter != 0; }

bool Player::isInGroup() const { return m_playerInfo && m_playerInfo->m_Group; }

Group* Player::getGroup() { return m_playerInfo ? m_playerInfo->m_Group : nullptr; }
bool Player::isGroupLeader() const
{
    if (m_playerInfo->m_Group != nullptr)
    {
        if (m_playerInfo->m_Group->GetLeader() == m_playerInfo)
            return true;
    }
    return false;
}

int8_t Player::getSubGroupSlot() const { return m_playerInfo->subGroup; }

//////////////////////////////////////////////////////////////////////////////////////////
// Quests
void Player::setQuestLogInSlot(QuestLogEntry* entry, uint32_t slotId)
{
    if (slotId < MAX_QUEST_SLOT)
        m_questlog[slotId] = entry;
}

bool Player::hasAnyQuestInQuestSlot() const
{
    for (auto& questlogSlot : m_questlog)
        if (questlogSlot != nullptr)
            return true;

    return false;
}

bool Player::hasTimedQuestInQuestSlot() const
{
    for (auto& questlogSlot : m_questlog)
        if (questlogSlot != nullptr && questlogSlot->getQuestProperties()->time != 0)
            return true;

    return false;
}

bool Player::hasQuestInQuestLog(uint32_t questId) const
{
    if (getQuestLogByQuestId(questId))
        return true;

    return false;
}

uint8_t Player::getFreeQuestSlot() const
{
    for (uint8_t slotId = 0; slotId < MAX_QUEST_SLOT; ++slotId)
        if (m_questlog[slotId] == nullptr)
            return slotId;

    return MAX_QUEST_SLOT + 1;
}

QuestLogEntry* Player::getQuestLogByQuestId(uint32_t questId) const
{
    for (auto& questlogSlot : m_questlog)
        if (questlogSlot != nullptr)
            if (questlogSlot->getQuestProperties()->id == questId)
                return questlogSlot;

    return nullptr;
}

QuestLogEntry* Player::getQuestLogBySlotId(uint32_t slotId) const
{
    if (slotId < MAX_QUEST_SLOT)
        return m_questlog[slotId];

    return nullptr;
}

void Player::addQuestIdToFinishedDailies(uint32_t questId)
{
    std::lock_guard<std::mutex> lock(m_mutextDailies);
    m_finishedDailies.insert(questId);
}
std::set<uint32_t> Player::getFinishedDailies() const
{
    std::lock_guard<std::mutex> lock(m_mutextDailies);
    return m_finishedDailies;
}
bool Player::hasQuestInFinishedDailies(uint32_t questId) const
{
    std::lock_guard<std::mutex> lock(m_mutextDailies);
    return m_finishedDailies.find(questId) != m_finishedDailies.end();
}
void Player::resetFinishedDailies()
{
    std::lock_guard<std::mutex> lock(m_mutextDailies);
    m_finishedDailies.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Social
void Player::loadFriendList()
{
    if (auto* result = CharacterDatabase.Query("SELECT * FROM social_friends WHERE character_guid = %u", getGuidLow()))
    {
        do
        {
            SocialFriends socialFriend;

            auto* const socialField = result->Fetch();
            socialFriend.friendGuid = socialField[1].GetUInt32();
            socialFriend.note = socialField[2].GetString();

            m_socialIFriends.push_back(socialFriend);

        } while (result->NextRow());

        delete result;
    }
}

void Player::loadFriendedByOthersList()
{
    if (auto* result = CharacterDatabase.Query("SELECT character_guid FROM social_friends WHERE friend_guid = %u", getGuidLow()))
    {
        do
        {
            auto* const socialField = result->Fetch();
            uint32_t friendedByGuid= socialField[0].GetUInt32();

            m_socialFriendedByGuids.push_back(friendedByGuid);

        } while (result->NextRow());

        delete result;
    }
}

void Player::loadIgnoreList()
{
    if (auto* result = CharacterDatabase.Query("SELECT * FROM social_ignores WHERE character_guid = %u", getGuidLow()))
    {
        do
        {
            auto* const ignoreField = result->Fetch();
            uint32_t ignoreGuid = ignoreField[1].GetUInt32();

            m_socialIgnoring.push_back(ignoreGuid);

        } while (result->NextRow());

        delete result;
    }
}

void Player::addToFriendList(std::string name, std::string note)
{
    if (auto* targetPlayer = sObjectMgr.GetPlayer(name.c_str()))
    {
        // we can not add us ;)
        if (targetPlayer->getGuidLow() == getGuidLow())
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_SELF, getGuid()).serialise().get());
            return;
        }

        if (targetPlayer->isGMFlagSet())
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_NOT_FOUND).serialise().get());
            return;
        }

        if (isFriended(targetPlayer->getGuidLow()))
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_ALREADY, targetPlayer->getGuidLow()).serialise().get());
            return;
        }

        if (targetPlayer->getPlayerInfo()->team != getInitialTeam() && m_session->permissioncount == 0 && !worldConfig.player.isInterfactionFriendsEnabled)
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_ENEMY, targetPlayer->getGuidLow()).serialise().get());
            return;
        }

        if (targetPlayer->GetSession())
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_ADDED_ONLINE, targetPlayer->getGuidLow(), note, 1,
                targetPlayer->GetZoneId(), targetPlayer->getLevel(), targetPlayer->getClass()).serialise().get());
        }
        else
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_ADDED_OFFLINE, targetPlayer->getGuidLow()).serialise().get());
        }

        CharacterDatabase.Execute("INSERT INTO social_friends VALUES(%u, %u, \'%s\')",
            getGuidLow(), targetPlayer->getGuidLow(), !note.empty() ? CharacterDatabase.EscapeString(std::string(note)).c_str() : "");

        SocialFriends socialFriend;
        socialFriend.friendGuid = targetPlayer->getGuidLow();
        socialFriend.note = note;

        std::lock_guard<std::mutex> guard(m_mutexFriendList);
        m_socialIFriends.push_back(socialFriend);
    }
    else
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_NOT_FOUND).serialise().get());
    }
}

void Player::removeFromFriendList(uint32_t guid)
{
    if (isFriended(guid))
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_REMOVED, guid).serialise().get());

        CharacterDatabase.Execute("DELETE FROM social_friends WHERE character_guid = %u AND friend_guid = %u", getGuidLow(), guid);

        std::lock_guard<std::mutex> guard(m_mutexFriendList);
        m_socialIFriends.erase(std::remove_if(m_socialIFriends.begin(), m_socialIFriends.end(), [&](SocialFriends const& friends) {
                return friends.friendGuid == guid;
            }),
            m_socialIFriends.end());
    }
    else
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_NOT_FOUND).serialise().get());
    }
}

void Player::addNoteToFriend(uint32_t guid, std::string note)
{
    std::lock_guard<std::mutex> guard(m_mutexFriendList);
    for (const auto friends : m_socialIFriends)
    {
        if (friends.friendGuid == guid)
        {
            friends.note = note;
            CharacterDatabase.Execute("UPDATE social_friends SET note = \'%s\' WHERE character_guid = %u AND friend_guid = %u",
                !note.empty() ? CharacterDatabase.EscapeString(note).c_str() : "", getGuidLow(), guid);
        }
    }
}

bool Player::isFriended(uint32_t guid) const
{
    std::lock_guard<std::mutex> guard(m_mutexFriendList);
    for (const auto friends : m_socialIFriends)
    {
        if (friends.friendGuid == guid)
            return true;
    }
    return false;
}

void Player::sendFriendStatus(bool comesOnline)
{
    std::lock_guard<std::mutex> guard(m_mutexFriendedBy);
    if (!m_socialFriendedByGuids.empty())
    {
        for (auto friendedGuids : m_socialFriendedByGuids)
        {
            if (auto* targetPlayer = sObjectMgr.GetPlayer(friendedGuids))
            {
                if (targetPlayer->GetSession())
                {
                    if (comesOnline)
                        targetPlayer->SendPacket(SmsgFriendStatus(FRIEND_ONLINE, getGuid(), "", 1, getAreaId(), getLevel(), getClass()).serialise().get());
                    else
                        targetPlayer->SendPacket(SmsgFriendStatus(FRIEND_OFFLINE, getGuid()).serialise().get());
                }
            }
        }
    }
}

void Player::sendFriendLists(uint32_t flags)
{
    std::vector<SmsgContactListMember> contactMemberList;

    if (flags & 0x01)    // friend
    {
        uint32_t maxCount = 0;

        std::lock_guard<std::mutex> guard(m_mutexFriendList);
        for (auto friends : m_socialIFriends)
        {
            SmsgContactListMember friendListMember;
            friendListMember.guid = friends.friendGuid;
            friendListMember.flag = 0x01;
            friendListMember.note = friends.note;

            if (auto* plr = sObjectMgr.GetPlayer(friends.friendGuid))
            {
                friendListMember.isOnline = 1;
                friendListMember.zoneId = plr->GetZoneId();
                friendListMember.level = plr->getLevel();
                friendListMember.playerClass = plr->getClass();
            }
            else
            {
                friendListMember.isOnline = 0;
            }

            contactMemberList.push_back(friendListMember);
            ++maxCount;

            if (maxCount >= 50)
                break;
        }

    }

    if (flags & 0x02)    // ignore
    {
        uint32_t maxCount = 0;

        std::lock_guard<std::mutex> guard(m_mutexIgnoreList);
        for (auto ignoredGuid : m_socialIgnoring)
        {
            SmsgContactListMember ignoreListMember;
            ignoreListMember.guid = ignoredGuid;
            ignoreListMember.flag = 0x02;
            ignoreListMember.note = "";

            contactMemberList.push_back(ignoreListMember);
            ++maxCount;

            if (maxCount >= 50)
                break;
        }
    }

    SendPacket(SmsgContactList(flags, contactMemberList).serialise().get());
}

void Player::addToIgnoreList(std::string name)
{
    if (auto* targetPlayer = sObjectMgr.GetPlayer(name.c_str()))
    {
        // we can not add us ;)
        if (targetPlayer->getGuidLow() == getGuidLow())
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_SELF, getGuid()).serialise().get());
            return;
        }

        if (targetPlayer->isGMFlagSet())
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_NOT_FOUND).serialise().get());
            return;
        }

        if (isIgnored(targetPlayer->getGuidLow()))
        {
            m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_ALREADY, targetPlayer->getGuidLow()).serialise().get());
            return;
        }

        CharacterDatabase.Execute("INSERT INTO social_ignores VALUES(%u, %u)", getGuidLow(), targetPlayer->getGuidLow());

        std::lock_guard<std::mutex> guard(m_mutexIgnoreList);
        m_socialIgnoring.push_back(targetPlayer->getGuidLow());
    }
    else
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_NOT_FOUND).serialise().get());
    }
}

void Player::removeFromIgnoreList(uint32_t guid)
{
    if (isIgnored(guid))
    {
        CharacterDatabase.Execute("DELETE FROM social_ignores WHERE character_guid = %u AND ignore_guid = %u", getGuidLow(), guid);

        std::lock_guard<std::mutex> guard(m_mutexIgnoreList);
        std::remove(m_socialIgnoring.begin(), m_socialIgnoring.end(), guid);
    }
    else
    {
        m_session->SendPacket(SmsgFriendStatus(FRIEND_IGNORE_NOT_FOUND).serialise().get());
    }
}

bool Player::isIgnored(uint32_t guid) const
{
    std::lock_guard<std::mutex> guard(m_mutexIgnoreList);
    if (std::find(m_socialIgnoring.begin(), m_socialIgnoring.end(), guid) != m_socialIgnoring.end())
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
bool Player::isGMFlagSet()
{
    return hasPlayerFlags(PLAYER_FLAG_GM);
}

void Player::sendMovie([[maybe_unused]]uint32_t movieId)
{
#if VERSION_STRING > TBC
    m_session->SendPacket(SmsgTriggerMovie(movieId).serialise().get());
#endif
}

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
            m_auras[i]->removeAura();
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
            if (!IS_INSTANCE(getBGEntryMapId()))
            {
                m_position.ChangeCoords(getBGEntryPosition());
                m_mapId = getBGEntryMapId();
            }
            else
            {
                m_position.ChangeCoords(getBindPosition());
                m_mapId = getBindMapId();
            }
        }
    }
}

bool Player::logOntoTransport()
{
    bool success = true;
    if (obj_movement_info.transport_guid != 0)
    {
        const auto transporter = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(obj_movement_info.transport_guid));
        if (transporter)
        {
            if (isDead())
            {
                ResurrectPlayer();
                setHealth(getMaxHealth());
                setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));
            }

            const float c_tposx = transporter->GetPositionX() + GetTransOffsetX();
            const float c_tposy = transporter->GetPositionY() + GetTransOffsetY();
            const float c_tposz = transporter->GetPositionZ() + GetTransOffsetZ();

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

        setBindPoint(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId(), GetZoneId());
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
    auto playerInfo = sObjectMgr.GetPlayerInfo(getGuidLow());
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

        sObjectMgr.AddPlayerInfo(playerInfo);
    }

    playerInfo->m_loggedInPlayer = this;
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
                SendPacket(SmsgTriggerCinematic(charEntry->cinematic_id).serialise().get());
            else if (const auto raceEntry = sChrRacesStore.LookupEntry(getRace()))
                SendPacket(SmsgTriggerCinematic(raceEntry->cinematic_id).serialise().get());
        }
#else
        if (const auto raceEntry = sChrRacesStore.LookupEntry(getRace()))
            SendPacket(SmsgTriggerCinematic(raceEntry->cinematic_id).serialise().get());
#endif
    }
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
    m_session->SendPacket(MsgSetDungeonDifficulty(m_raidDifficulty, 1, isInGroup()).serialise().get());
}

void Player::sendRaidDifficultyPacket()
{
#if VERSION_STRING > TBC
    m_session->SendPacket(MsgSetRaidDifficulty(m_raidDifficulty, 1, isInGroup()).serialise().get());
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
    if (sendtoset && isInGroup())
        getGroup()->SendPacketToAll(SmsgItemPushResult(getGuid(), recieved, created, destbagslot, destslot, entry, suffix, randomprop, count, stack).serialise().get());
    else
        m_session->SendPacket(SmsgItemPushResult(getGuid(), recieved, created, destbagslot, destslot, entry, suffix, randomprop, count, stack).serialise().get());
}

void Player::sendClientControlPacket(Unit* target, uint8_t allowMove)
{
    SendPacket(SmsgClientControlUpdate(target->GetNewGUID(), allowMove).serialise().get());

    if (target == this)
        SetMover(this);
}

void Player::sendGuildMotd()
{
    if (!getGuild())
        return;

    SendPacket(SmsgGuildEvent(GE_MOTD, { getGuild()->getMOTD() }, 0).serialise().get());
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
    addUnitFlags(UNIT_FLAG_PVP);
#endif

    addPlayerFlags(PLAYER_FLAG_PVP_TIMER);

    getSummonInterface()->setPvPFlags(true);
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
    removeUnitFlags(UNIT_FLAG_PVP);
#endif

    removePlayerFlags(PLAYER_FLAG_PVP_TIMER);

    getSummonInterface()->setPvPFlags(false);
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

    getSummonInterface()->setFFAPvPFlags(true);
    for (auto& summon : GetSummons())
        summon->setFfaPvpFlag();
}

void Player::removeFfaPvpFlag()
{
    StopPvPTimer();
    setPvpFlags(getPvpFlags() & ~U_FIELD_BYTES_FLAG_FFA_PVP);
    removePlayerFlags(PLAYER_FLAG_FREE_FOR_ALL_PVP);

    getSummonInterface()->setFFAPvPFlags(false);
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
    addPlayerFlags(PLAYER_FLAG_SANCTUARY);

    getSummonInterface()->setSanctuaryFlags(true);
    for (auto& summon : GetSummons())
        summon->setSanctuaryFlag();
}

void Player::removeSanctuaryFlag()
{
    setPvpFlags(getPvpFlags() & ~U_FIELD_BYTES_FLAG_SANCTUARY);
    removePlayerFlags(PLAYER_FLAG_SANCTUARY);

    getSummonInterface()->setSanctuaryFlags(false);
    for (auto& summon : GetSummons())
        summon->removeSanctuaryFlag();
}

void Player::sendPvpCredit(uint32_t honor, uint64_t victimGuid, uint32_t victimRank)
{
    this->SendPacket(SmsgPvpCredit(honor, victimGuid, victimRank).serialise().get());
}

void Player::sendRaidGroupOnly(uint32_t timeInMs, uint32_t type)
{
    this->SendPacket(SmsgRaidGroupOnly(timeInMs, type).serialise().get());
}

void Player::setVisibleItemFields(uint32_t slot, Item* item)
{
    if (item)
    {
        setVisibleItemEntry(slot, item->getEntry());
#if VERSION_STRING > TBC
        setVisibleItemEnchantment(slot, item->getEnchantmentId(0));
#else
        setVisibleItemEnchantment(slot, 0, item->getEnchantmentId(0));
        setVisibleItemEnchantment(slot, 1, item->getEnchantmentId(3));
        setVisibleItemEnchantment(slot, 2, item->getEnchantmentId(6));
        setVisibleItemEnchantment(slot, 3, item->getEnchantmentId(9));
        setVisibleItemEnchantment(slot, 4, item->getEnchantmentId(12));
        setVisibleItemEnchantment(slot, 5, item->getEnchantmentId(15));
        setVisibleItemEnchantment(slot, 6, item->getEnchantmentId(18));
        setVisibleItemEnchantment(slot, 7, item->getRandomPropertiesId());
#endif
    }
    else
    {
        setVisibleItemEntry(slot, 0);
#if VERSION_STRING > TBC
        setVisibleItemEnchantment(slot, 0);
#else
        for (uint8_t i = 0; i < WOWPLAYER_VISIBLE_ITEM_UNK0_COUNT; ++i)
            setVisibleItemEnchantment(slot, i, 0);
#endif
    }
}

void Player::addToGMTargetList(uint32_t guid)
{
    std::lock_guard<std::mutex> guard(m_lockGMTargetList);
    m_gmPlayerTargetList.push_back(guid);
}

void Player::removeFromGMTargetList(uint32_t guid)
{
    std::lock_guard<std::mutex> guard(m_lockGMTargetList);
    std::remove(m_gmPlayerTargetList.begin(), m_gmPlayerTargetList.end(), guid);
}

bool Player::isOnGMTargetList(uint32_t guid) const
{
    std::lock_guard<std::mutex> guard(m_lockGMTargetList);
    if (std::find(m_gmPlayerTargetList.begin(), m_gmPlayerTargetList.end(), guid) != m_gmPlayerTargetList.end())
        return true;

    return false;
}
