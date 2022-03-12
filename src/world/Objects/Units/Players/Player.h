/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Objects/Units/Stats.h"
#include "Management/QuestDefines.hpp"
#include "Management/Battleground/BattlegroundMgr.h"
#include "Management/MailMgr.h"
#include "Management/ItemPrototype.h"
#include "Management/AchievementMgr.h"
#include "Objects/Units/Unit.h" 
#include "Storage/MySQLStructures.h"
#include "Macros/PlayerMacros.hpp"
#include "Objects/Units/Creatures/AIInterface.h" //?? what?
#include "WorldConf.h"
#include "Management/AuctionHouse.h"
#include "Management/Guild/Guild.hpp"
#include "Management/ObjectUpdates/UpdateManager.h"
#include "Data/WoWPlayer.hpp"
#include <mutex>

#include "TradeData.hpp"

class ArenaTeam;
struct CharCreate;
class QuestLogEntry;
struct BGScore;
class AchievementMgr;
class Channel;
class Creature;
class Battleground;
class TaxiPath;
class GameObject;
class Transporter;
class Corpse;
class Guild;
class Pet;
class Charter;
class LfgMatch;
struct LevelInfo;
class SpeedCheatDetector;
struct GuildMember;

class QueryBuffer;
struct QuestProperties;
struct SpellShapeshiftForm;
class CBattleground;
class Instance;
struct CharRaceEntry;
struct CharClassEntry;
struct Trainer;
class Aura;

struct OnHitSpell;
class CachedCharacterInfo;

//\todo: everything above this comment, does not belong in this file. Refactor this file to hold only the player class ;-)
// Everything below this line is bloated (seems we need some new concepts like RAII and a lot of refactoring to shrink it to a manageable class.
// Group all related members to a struct/class. Follow the "modern" way of C++ and leave the C way behind.
// 1. Initialize class members in the class
// 2. Use const wherever possible
// 3. move stuff out of this class
// 4. Check out the members (there are duplicats)
// 5. Get rid of legacy files (Player.Legacy.cpp)
struct WoWPlayer;
class SERVER_DECL Player : public Unit
{
public:

    friend class WorldSession;
    friend class Pet;

    Player(uint32_t guid);
    ~Player();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Essential functions

    void Update(unsigned long time_passed);             // hides function Unit::Update
    void AddToWorld();                                  // hides virtual function Object::AddToWorld
    void AddToWorld(MapMgr* pMapMgr);                   // hides virtual function Object::AddToWorld
    // void PushToWorld(MapMgr*);                       // not used
    // void RemoveFromWorld(bool free_guid);            // not used
    void OnPrePushToWorld() override;                   // overrides virtual function  Object::OnPrePushToWorld
    void OnPushToWorld() override;                      // overrides virtual function  Object::OnPushToWorld
    // void OnPreRemoveFromWorld();                     // not used
    // void OnRemoveFromWorld();                        // not used

private:
    const WoWPlayer* playerData() const { return reinterpret_cast<WoWPlayer*>(wow_data); }
public:
    void resendSpeed();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Data
    uint64_t getDuelArbiter() const;
    void setDuelArbiter(uint64_t guid);

    uint32_t getPlayerFlags() const;
    void setPlayerFlags(uint32_t flags);
    void addPlayerFlags(uint32_t flags);
    void removePlayerFlags(uint32_t flags);
    bool hasPlayerFlags(uint32_t flags) const;

    uint32_t getGuildId() const;
    void setGuildId(uint32_t guildId);

    uint32_t getGuildRank() const;
    void setGuildRank(uint32_t guildRank);

#if VERSION_STRING >= Cata
    uint32_t getGuildLevel() const;
    void setGuildLevel(uint32_t guildLevel);
#endif

    //bytes begin
    uint32_t getPlayerBytes() const;
    void setPlayerBytes(uint32_t bytes);

    uint8_t getSkinColor() const;
    void setSkinColor(uint8_t color);

    uint8_t getFace() const;
    void setFace(uint8_t face);

    uint8_t getHairStyle() const;
    void setHairStyle(uint8_t style);

    uint8_t getHairColor() const;
    void setHairColor(uint8_t color);
    //bytes end

    //bytes2 begin
    uint32_t getPlayerBytes2() const;
    void setPlayerBytes2(uint32_t bytes2);

    uint8_t getFacialFeatures() const;
    void setFacialFeatures(uint8_t feature);

    uint8_t getBytes2UnknownField() const;
    void setBytes2UnknownField(uint8_t value);

    uint8_t getBankSlots() const;
    void setBankSlots(uint8_t slots);

    uint8_t getRestState() const;
    void setRestState(uint8_t state);
    //bytes2 end

    //bytes3 begin
    uint32_t getPlayerBytes3() const;
    void setPlayerBytes3(uint32_t bytes3);

    //\note already available in unit data
    uint8_t getPlayerGender() const;
    void setPlayerGender(uint8_t gender);

    uint8_t getDrunkValue() const;
    void setDrunkValue(uint8_t value);

    uint8_t getPvpRank() const;
    void setPvpRank(uint8_t rank);

    uint8_t getArenaFaction() const;
    void setArenaFaction(uint8_t faction);
    //bytes3 end

    uint32_t getDuelTeam() const;
    void setDuelTeam(uint32_t team);

    uint32_t getGuildTimestamp() const;
    void setGuildTimestamp(uint32_t timestamp);

    //QuestLog start
    uint32_t getQuestLogEntryForSlot(uint8_t slot) const;
    void setQuestLogEntryBySlot(uint8_t slot, uint32_t questEntry);

    uint32_t getQuestLogStateForSlot(uint8_t slot) const;
    void setQuestLogStateBySlot(uint8_t slot, uint32_t state);

#if VERSION_STRING > TBC
    uint64_t getQuestLogRequiredMobOrGoForSlot(uint8_t slot) const;
    void setQuestLogRequiredMobOrGoBySlot(uint8_t slot, uint64_t mobOrGoCount);
#else
    uint32_t getQuestLogRequiredMobOrGoForSlot(uint8_t slot) const;
    void setQuestLogRequiredMobOrGoBySlot(uint8_t slot, uint32_t mobOrGoCount);
#endif

    uint32_t getQuestLogExpireTimeForSlot(uint8_t slot) const;
    void setQuestLogExpireTimeBySlot(uint8_t slot, uint32_t expireTime);
    //QuestLog end

//VisibleItem start
    uint32_t getVisibleItemEntry(uint32_t slot) const;
    void setVisibleItemEntry(uint32_t slot, uint32_t entry);
#if VERSION_STRING > TBC
    uint16_t getVisibleItemEnchantment(uint32_t slot, uint8_t pos) const;
    void setVisibleItemEnchantment(uint32_t slot, uint8_t pos, uint16_t enchantment);
#else
    uint32_t getVisibleItemEnchantment(uint32_t slot, uint8_t pos) const;
    void setVisibleItemEnchantment(uint32_t slot, uint8_t pos, uint32_t enchantment);
#endif
//VisibleItem end

    uint64_t getVendorBuybackSlot(uint8_t slot) const;
    void setVendorBuybackSlot(uint8_t slot, uint64_t guid);

    uint64_t getFarsightGuid() const;
    void setFarsightGuid(uint64_t farsightGuid);

#if VERSION_STRING > Classic
    uint64_t getKnownTitles(uint8_t index) const;
    void setKnownTitles(uint8_t index, uint64_t title);
#endif

#if VERSION_STRING > Classic
    uint32_t getChosenTitle() const;
    void setChosenTitle(uint32_t title);
#endif

#if VERSION_STRING == WotLK
    uint64_t getKnownCurrencies() const;
    void setKnownCurrencies(uint64_t currencies);
#endif

    uint32_t getXp() const;
    void setXp(uint32_t xp);

    uint32_t getNextLevelXp() const;
    void setNextLevelXp(uint32_t xp);

#if VERSION_STRING < Cata
    uint16_t getSkillInfoId(uint32_t index) const;
    uint16_t getSkillInfoStep(uint32_t index) const;
    uint16_t getSkillInfoCurrentValue(uint32_t index) const;
    uint16_t getSkillInfoMaxValue(uint32_t index) const;
    uint16_t getSkillInfoBonusTemporary(uint32_t index) const;
    uint16_t getSkillInfoBonusPermanent(uint32_t index) const;
    void setSkillInfoId(uint32_t index, uint16_t id);
    void setSkillInfoStep(uint32_t index, uint16_t step);
    void setSkillInfoCurrentValue(uint32_t index, uint16_t current);
    void setSkillInfoMaxValue(uint32_t index, uint16_t max);
    void setSkillInfoBonusTemporary(uint32_t index, uint16_t bonus);
    void setSkillInfoBonusPermanent(uint32_t index, uint16_t bonus);
#else
    uint16_t getSkillInfoId(uint32_t index, uint8_t offset) const;
    uint16_t getSkillInfoStep(uint32_t index, uint8_t offset) const;
    uint16_t getSkillInfoCurrentValue(uint32_t index, uint8_t offset) const;
    uint16_t getSkillInfoMaxValue(uint32_t index, uint8_t offset) const;
    uint16_t getSkillInfoBonusTemporary(uint32_t index, uint8_t offset) const;
    uint16_t getSkillInfoBonusPermanent(uint32_t index, uint8_t offset) const;
    uint32_t getProfessionSkillLine(uint32_t index) const;
    void setSkillInfoId(uint32_t index, uint8_t offset, uint16_t id);
    void setSkillInfoStep(uint32_t index, uint8_t offset, uint16_t step);
    void setSkillInfoCurrentValue(uint32_t index, uint8_t offset, uint16_t current);
    void setSkillInfoMaxValue(uint32_t index, uint8_t offset, uint16_t max);
    void setSkillInfoBonusTemporary(uint32_t index, uint8_t offset, uint16_t bonus);
    void setSkillInfoBonusPermanent(uint32_t index, uint8_t offset, uint16_t bonus);
    void setProfessionSkillLine(uint32_t index, uint32_t value);
#endif

    uint32_t getFreeTalentPoints() const;
#if VERSION_STRING < Cata
    void setFreeTalentPoints(uint32_t points);
#endif

    uint32_t getFreePrimaryProfessionPoints() const;
    void setFreePrimaryProfessionPoints(uint32_t points);
    void modFreePrimaryProfessionPoints(int32_t amount);

    uint32_t getTrackCreature() const;
    void setTrackCreature(uint32_t id);

    uint32_t getTrackResource() const;
    void setTrackResource(uint32_t id);

    float getBlockPercentage() const;
    void setBlockPercentage(float value);

    float getDodgePercentage() const;
    void setDodgePercentage(float value);

    float getParryPercentage() const;
    void setParryPercentage(float value);

#if VERSION_STRING >= TBC
    uint32_t getExpertise() const;
    void setExpertise(uint32_t value);
    void modExpertise(int32_t value);

    uint32_t getOffHandExpertise() const;
    void setOffHandExpertise(uint32_t value);
    void modOffHandExpertise(int32_t value);
#endif

    float getMeleeCritPercentage() const;
    void setMeleeCritPercentage(float value);

    float getRangedCritPercentage() const;
    void setRangedCritPercentage(float value);

#if VERSION_STRING >= TBC
    float getOffHandCritPercentage() const;
    void setOffHandCritPercentage(float value);

    float getSpellCritPercentage(uint8_t school) const;
    void setSpellCritPercentage(uint8_t school, float value);

    uint32_t getShieldBlock() const;
    void setShieldBlock(uint32_t value);
#endif

#if VERSION_STRING >= WotLK
    float getShieldBlockCritPercentage() const;
    void setShieldBlockCritPercentage(float value);
#endif

    uint32_t getExploredZone(uint32_t idx) const;
    void setExploredZone(uint32_t idx, uint32_t data);

    uint32_t getSelfResurrectSpell() const;
    void setSelfResurrectSpell(uint32_t spell);

    uint32_t getWatchedFaction() const;
    void setWatchedFaction(uint32_t factionId);

#if VERSION_STRING < WotLK
    float getManaRegeneration() const;
    void setManaRegeneration(float value);

    float getManaRegenerationWhileCasting() const;
    void setManaRegenerationWhileCasting(float value);
#endif

    uint32_t getMaxLevel() const;
    void setMaxLevel(uint32_t level);

#if VERSION_STRING >= WotLK
    float getRuneRegen(uint8_t rune) const;
    void setRuneRegen(uint8_t rune, float regen);
#endif

    uint32_t getRestStateXp() const;
    void setRestStateXp(uint32_t xp);

    //\brief: the playerfield coinage is an uint64_t since cata
#if VERSION_STRING < Cata
    uint32_t getCoinage() const;
    void setCoinage(uint32_t coinage);
    bool hasEnoughCoinage(uint32_t coinage) const;
    void modCoinage(int32_t coinage);
#else
    uint64_t getCoinage() const;
    void setCoinage(uint64_t coinage);
    bool hasEnoughCoinage(uint64_t coinage) const;
    void modCoinage(int64_t coinage);
#endif

#if VERSION_STRING == Classic
    uint32_t getResistanceBuffModPositive(uint8_t type) const;
    void setResistanceBuffModPositive(uint8_t type, uint32_t value);

    uint32_t getResistanceBuffModNegative(uint8_t type) const;
    void setResistanceBuffModNegative(uint8_t type, uint32_t value);
#endif

    uint32_t getModDamageDonePositive(uint16_t school) const;
    void setModDamageDonePositive(uint16_t school, uint32_t value);
    void modModDamageDonePositive(uint16_t school, int32_t value);

    uint32_t getModDamageDoneNegative(uint16_t school) const;
    void setModDamageDoneNegative(uint16_t school, uint32_t value);
    void modModDamageDoneNegative(uint16_t school, int32_t value);

    float getModDamageDonePct(uint8_t shool) const;
    void setModDamageDonePct(float damagePct, uint8_t shool);

#if VERSION_STRING >= TBC
    uint32_t getModHealingDone() const;
    void setModHealingDone(uint32_t value);
    void modModHealingDone(int32_t value);

    // Spell penetration?
    uint32_t getModTargetResistance() const;
    void setModTargetResistance(uint32_t value);
    void modModTargetResistance(int32_t value);

    // Armor penetration?
    uint32_t getModTargetPhysicalResistance() const;
    void setModTargetPhysicalResistance(uint32_t value);
    void modModTargetPhysicalResistance(int32_t value);
#endif

    // playerfieldbytes start
    uint32_t getPlayerFieldBytes() const;
    void setPlayerFieldBytes(uint32_t bytes);

    uint8_t getActionBarId() const;
    void setActionBarId(uint8_t actionBarId);
    // playerfieldbytes end

#if VERSION_STRING < Cata
    uint32_t getAmmoId() const;
    void setAmmoId(uint32_t id);
#endif

    uint32_t getBuybackPriceSlot(uint8_t slot) const;
    void setBuybackPriceSlot(uint8_t slot, uint32_t price);

    uint32_t getBuybackTimestampSlot(uint8_t slot) const;
    void setBuybackTimestampSlot(uint8_t slot, uint32_t timestamp);

#if VERSION_STRING > Classic
    uint32_t getFieldKills() const;
    void setFieldKills(uint32_t kills);
#endif

#if VERSION_STRING > Classic
#if VERSION_STRING < Cata
    uint32_t getContributionToday() const;
    void setContributionToday(uint32_t contribution);

    uint32_t getContributionYesterday() const;
    void setContributionYesterday(uint32_t contribution);
#endif
#endif

    uint32_t getLifetimeHonorableKills() const;
    void setLifetimeHonorableKills(uint32_t kills);

    // playerfieldbytes2 start
    uint32_t getPlayerFieldBytes2() const;
    void setPlayerFieldBytes2(uint32_t bytes);
    // playerfieldbytes2 end

    uint32_t getCombatRating(uint8_t combatRating) const;
    void setCombatRating(uint8_t combatRating, uint32_t value);
    void modCombatRating(uint8_t combatRating, int32_t value);

#if VERSION_STRING > Classic
    // field_arena_team_info start
    uint32_t getArenaTeamId(uint8_t teamSlot) const;
    void setArenaTeamId(uint8_t teamSlot, uint32_t teamId);

    uint32_t getArenaTeamMemberRank(uint8_t teamSlot) const;
    void setArenaTeamMemberRank(uint8_t teamSlot, uint32_t rank);
    // field_arena_team_info end
#endif

    uint64_t getInventorySlotItemGuid(uint8_t index) const;
    void setInventorySlotItemGuid(uint8_t index, uint64_t guid);

#if VERSION_STRING > Classic
#if VERSION_STRING < Cata
    uint32_t getHonorCurrency() const;
    void setHonorCurrency(uint32_t amount);
    void modHonorCurrency(int32_t value);

    uint32_t getArenaCurrency() const;
    void setArenaCurrency(uint32_t amount);
    void modArenaCurrency(int32_t value);
#endif
#endif

#if VERSION_STRING >= WotLK
    uint32_t getNoReagentCost(uint8_t index) const;
    void setNoReagentCost(uint8_t index, uint32_t value);

    uint32_t getGlyphSlot(uint16_t slot) const;
    void setGlyphSlot(uint16_t slot, uint32_t glyph);

    uint32_t getGlyph(uint16_t slot) const;
    void setGlyph(uint16_t slot, uint32_t glyph);

    uint32_t getGlyphsEnabled() const;
    void setGlyphsEnabled(uint32_t glyphs);
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Movement/Position
public:
    void sendForceMovePacket(UnitSpeedType speed_type, float speed);
    void sendMoveSetSpeedPaket(UnitSpeedType speed_type, float speed);

    bool isMoving() const;

    bool isMounted() const;
    uint32_t getMountSpellId() const;
    void setMountSpellId(uint32_t id);

    bool isOnVehicle() const;
    uint32_t getMountVehicleId() const;
    void setMountVehicleId(uint32_t id);

    void dismount();

    void handleAuraInterruptForMovementFlags(MovementInfo const& movement_info);

    uint32_t getAreaId() const;
    void setAreaId(uint32_t area);

    bool isInCity() const;

    void handleBreathing(MovementInfo const& movement_info, WorldSession* session);
    void initialiseNoseLevel();

    bool m_isWaterBreathingEnabled = false;
    uint32_t m_underwaterTime = 180000;
    uint32_t m_underwaterMaxTime = 180000;
    uint32_t m_underwaterState = 0;
    uint32_t m_underwaterLastDamage = Util::getMSTime();

    void handleKnockback(Object* caster, float horizontal, float vertical) override;

    bool teleport(const LocationVector& vec, MapMgr* map);
    void eventTeleport(uint32_t mapId, LocationVector position, uint32_t instanceId = 0);

    bool safeTeleport(uint32_t mapId, uint32_t instanceId, const LocationVector& vec);

    //\Todo: this function is not as "safe" as the one above, reduce it to one function.
    void safeTeleport(MapMgr* mgr, const LocationVector& vec);

    void setTransferStatus(uint8_t status);
    uint8_t getTransferStatus() const;
    bool isTransferPending() const;

    uint32_t getTeleportState() const;

    void sendTeleportPacket(LocationVector position);
    void sendTeleportAckPacket(LocationVector position);

    void onWorldPortAck();
    void eventPortToGm(Player* gmPlayer);

    void indoorCheckUpdate(uint32_t time);

    time_t getFallDisabledUntil() const;
    void setFallDisabledUntil(time_t time);

    void setMapEntryPoint(uint32_t mapId);

protected:
    bool m_isMoving = false;
    bool m_isMovingFB = false;
    bool m_isStrafing = false;
    bool m_isTurning = false;
    bool m_isJumping = false;

    uint32 m_mountSpellId = 0;
    uint32 m_mountVehicleId = 0;

    uint32_t m_areaId = 0;

    float m_noseLevel = .0f;

    LocationVector m_sentTeleportPosition;
    uint8_t m_transferStatus = TRANSFER_NONE;
    uint32_t m_teleportState = 1;

    uint32_t m_indoorCheckTimer = 0;

    time_t m_fallDisabledUntil = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Instance, Zone, Area, Phase
public:
    void setPhase(uint8_t command = PHASE_SET, uint32_t newPhase = 1) override;

    void zoneUpdate(uint32_t zoneId);
    void forceZoneUpdate();

    bool hasAreaExplored(DBC::Structures::AreaTableEntry const*);
    bool hasOverlayUncovered(uint32_t overlayId);
    void eventExploration();

    uint32_t m_explorationTimer = Util::getMSTime();

    void ejectFromInstance();
    bool exitInstance();
    uint32_t getPersistentInstanceId(uint32_t mapId, uint8_t difficulty);
    void setPersistentInstanceId(Instance* instance);
    void setPersistentInstanceId(uint32_t mapId, uint8_t difficulty, uint32_t instanceId);
private:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Basic
public:
    DBC::Structures::ChrRacesEntry const* getDbcRaceEntry();
    DBC::Structures::ChrClassesEntry const* getDbcClassEntry();

    std::string getName() const;
    void setName(std::string name);

    uint32_t getLoginFlag() const;
    void setLoginFlag(uint32_t flag);

    void setInitialDisplayIds(uint8_t gender, uint8_t race);

    void applyLevelInfo(uint32_t newLevel);

    virtual bool isClassMage();
    virtual bool isClassDeathKnight();
    virtual bool isClassPriest();
    virtual bool isClassRogue();
    virtual bool isClassShaman();
    virtual bool isClassHunter();
    virtual bool isClassWarlock();
    virtual bool isClassWarrior();
    virtual bool isClassPaladin();
    virtual bool isClassMonk();
    virtual bool isClassDruid();

    PlayerTeam getTeam() const;
    PlayerTeam getBgTeam() const;
    void setTeam(uint32_t team);
    void setBgTeam(uint32_t team);

    uint32_t getInitialTeam() const;

    void resetTeam();
    bool isTeamHorde() const;
    bool isTeamAlliance() const;

    Player* getPlayerOwner() override;

    void toggleAfk();
    void toggleDnd();

    uint32_t* getPlayedTime();

    CachedCharacterInfo* getPlayerInfo() const;

private:
    LevelInfo* m_levelInfo = nullptr;

    DBC::Structures::ChrRacesEntry const* m_dbcRace = nullptr;
    DBC::Structures::ChrClassesEntry const* m_dbcClass = nullptr;

    uint32_t m_loadHealth = 0;
    uint32_t m_loadMana = 0;

    uint32_t m_classicMaxLevel = 60;

    std::string m_name;

    uint32_t m_loginFlag = LOGIN_NO_FLAG;

    uint32_t m_team = 0;
    uint32_t m_bgTeam = 0;

    //\note: 0 = played on level, 1 = played total, 2 = played session
    uint32_t m_playedTime[3] = { 0, 0, static_cast<uint32_t>(UNIXTIME) };
    uint32_t m_onlineTime = static_cast<uint32_t>(UNIXTIME);
    uint32_t m_timeLogoff = 0;

    CachedCharacterInfo* m_playerInfo = nullptr;

protected:
    PlayerCreateInfo const* m_playerCreateInfo = nullptr;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Session & Packets
public:
    WorldSession* getSession() const;
    void setSession(WorldSession* session);
    void removePendingPlayer();
    void softDisconnect();

    void sendDelayedPacket(WorldPacket* data, bool bDeleteOnSend);

    void processPendingUpdates();
    bool compressAndSendUpdateBuffer(uint32_t size, const uint8_t* update_buffer);
    uint32_t buildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target) override;

    static void initVisibleUpdateBits();
    static UpdateMask m_visibleUpdateMask;

    void copyAndSendDelayedPacket(WorldPacket* data);

    UpdateManager& getUpdateMgr();

private:
    UpdateManager m_updateMgr;

protected:
    WorldSession* m_session = nullptr;

    void setCreateBits(UpdateMask* updateMask, Player* target) const;
    void setUpdateBits(UpdateMask* updateMask, Player* target) const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Stats
    // Initializes stats and unit/playerdata fields
public:
    void setInitialPlayerData();

    // Not same as Unit::regeneratePowers
    void regeneratePlayerPowers(uint16_t diff);
#if VERSION_STRING >= Cata
    void resetHolyPowerTimer();
#endif

    // PlayerStats.cpp
    void updateManaRegeneration();

private:
    // Regenerate timers
    // Rage and Runic Power
    uint16_t m_rageRunicPowerRegenerateTimer = 0;
#if VERSION_STRING >= Cata
    uint16_t m_holyPowerRegenerateTimer = 0;
#endif

    // This timer ticks even if the player is not eating or drinking
    uint16_t m_foodDrinkSpellVisualTimer = 5000;

#if VERSION_STRING == Classic
    // Classic doesn't have these in unit or playerdata
    float m_manaRegeneration = 0.0f;
    float m_manaRegenerationWhileCasting = 0.0f;
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Database stuff
public:
    bool loadSpells(QueryResult* result);
    bool loadSkills(QueryResult* result);
    bool loadReputations(QueryResult* result);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spells and skills
#if VERSION_STRING >= Cata
    void setInitialPlayerProfessions();
#endif

    void updateAutoRepeatSpell();
    bool canUseFlyingMountHere();

    bool canDualWield2H() const;
    void setDualWield2H(bool enable);

    bool isSpellFitByClassAndRace(uint32_t spell_id) const;

    // Cooldowns
    bool hasSpellOnCooldown(SpellInfo const* spellInfo);
    bool hasSpellGlobalCooldown(SpellInfo const* spellInfo);
    // Do NOT add cooldownTime if you don't know what you're doing (it's required for spells with dynamic cooldown)
    void addSpellCooldown(SpellInfo const* spellInfo, Item const* itemCaster, Spell* castingSpell = nullptr, int32_t cooldownTime = 0);
    void addGlobalCooldown(SpellInfo const* spellInfo, Spell* castingSpell, const bool sendPacket = false);
    void sendSpellCooldownPacket(SpellInfo const* spellInfo, const uint32_t duration, const bool isGcd);
    void clearCooldownForSpell(uint32_t spellId);
    void clearGlobalCooldown();
    void resetAllCooldowns();

    // Skills
    void advanceAllSkills(uint16_t amount = 1);
    void advanceSkillLine(uint16_t skillLine, uint16_t amount = 1);
    void addSkillLine(uint16_t skillLine, uint16_t currentValue, uint16_t maxValue, bool noSpellLearning = false, bool initializeProfession = false);
    bool hasSkillLine(uint16_t skillLine, bool strict = false) const;
    uint16_t getSkillLineCurrent(uint16_t skillLine, bool includeBonus = true) const;
    uint16_t getSkillLineMax(uint16_t skillLine) const;
    void learnInitialSkills();
    void learnSkillSpells(uint16_t skillLine, uint16_t currentValue);
    void modifySkillBonus(uint16_t skillLine, int16_t amount, bool permanentBonus);
    void modifySkillMaximum(uint16_t skillLine, uint16_t maxValue);
    void removeSkillLine(uint16_t skillLine);
    void removeSkillSpells(uint16_t skillLine);
    void removeAllSkills();
    void updateSkillMaximumValues();

    uint32_t getArmorProficiency() const;
    void addArmorProficiency(uint32_t proficiency);
    void removeArmorProficiency(uint32_t proficiency);
    uint32_t getWeaponProficiency() const;
    void addWeaponProficiency(uint32_t proficiency);
    void removeWeaponProficiency(uint32_t proficiency);
    void applyItemProficienciesFromSpell(SpellInfo const* spellInfo, bool apply);

#ifdef FT_GLYPHS
    // Glyphs
    // Initializes glyph slots or updates them on levelup
    void updateGlyphs();
#endif

    // Combo Points
    uint64_t getComboPointTarget() const;
    int8_t getComboPoints() const;
    void addComboPoints(uint64_t targetGuid, int8_t points);
    void updateComboPoints();
    void clearComboPoints();

    bool m_FirstCastAutoRepeat = false;

private:
    bool m_canDualWield2H = false;

    // Skills
    void _verifySkillValues(DBC::Structures::SkillLineEntry const* skillEntry, uint16_t* currentValue, uint16_t* maxValue, uint16_t* skillStep, bool* requireUpdate);
    void _verifySkillValues(DBC::Structures::SkillLineEntry const* skillEntry, uint16_t* currentValue, uint16_t* maxValue, uint16_t* skillStep);
    void _updateSkillFieldOnValueChange(const PlayerSkillFieldPosition fieldPosition, uint16_t skillStep, uint16_t currentValue, uint16_t maxValue);
    void _updateSkillBonusFields(const PlayerSkillFieldPosition fieldPosition, uint16_t tempBonus, uint16_t permBonus);
    SkillMap m_skills;

    uint32_t armorProficiency = 0;
    uint32_t weaponProficiency = 0;

    uint64_t m_comboTarget = 0;
    int8_t m_comboPoints = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Talents
public:
    void learnTalent(uint32_t talentId, uint32_t talentRank);
    void addTalent(SpellInfo const* sp);
    void removeTalent(uint32_t spellId, bool onSpecChange = false);
    // Resets only current spec's talents
    void resetTalents();
    // Resets talents for both specs
    void resetAllTalents();
    void setTalentPoints(uint32_t talentPoints, bool forBothSpecs = true);
    void addTalentPoints(uint32_t talentPoints, bool forBothSpecs = true);
    void setInitialTalentPoints(bool talentsResetted = false);

    uint32_t getTalentPointsFromQuests() const;
    void setTalentPointsFromQuests(uint32_t talentPoints);
    void smsg_TalentsInfo(bool SendPetTalents); // TODO: classic and tbc

    void activateTalentSpec(uint8_t specId);

    uint32_t getTalentResetsCount() const;
    void setTalentResetsCount(uint32_t value);

    uint32_t calcTalentResetCost(uint32_t resetnum) const;

private:
    uint32_t m_talentPointsFromQuests = 0;
    uint32_t m_talentResetsCount = 0;
    bool m_resetTalents = false;

    /////////////////////////////////////////////////////////////////////////////////////////
    // Tutorials
public:
    uint32_t getTutorialValueById(uint8_t id);
    void setTutorialValueForId(uint8_t id, uint32_t value);

    void loadTutorials();
    void saveTutorials();

protected:
    uint32_t m_tutorials[8] = {0};
    bool m_tutorialsDirty = true;

    /////////////////////////////////////////////////////////////////////////////////////////
    // Actionbar
public:
    void setActionButton(uint8_t button, uint32_t action, uint8_t type, uint8_t misc);
    void sendActionBars(bool clearBars);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Auction
    void sendAuctionCommandResult(Auction* auction, uint32_t Action, uint32_t ErrorCode, uint32_t bidError = 0);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Trade
public:
    Player* getTradeTarget() const;
    TradeData* getTradeData() const;
    void cancelTrade(bool sendToSelfAlso, bool silently = false);

private:
    TradeData* m_TradeData = nullptr;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Messages
public:
    void sendReportToGmMessage(std::string playerName, std::string damageLog);
    void broadcastMessage(const char* Format, ...);
    void sendAreaTriggerMessage(const char* message, ...);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Commands
public:
    void disableSummoning(bool disable);
    bool isSummoningDisabled() const;
    void disableAppearing(bool disable);
    bool isAppearingDisabled() const;

    bool isBanned() const;
    void setBanned(uint32_t timestamp = 4, std::string Reason = "");
    void unsetBanned();
    std::string getBanReason() const;

    GameObject* getSelectedGo() const;
    void setSelectedGo(uint64_t guid);

    void kickFromServer(uint32_t delay = 0);
    void eventKickFromServer();

    void sendSummonRequest(uint32_t requesterId, uint32_t zoneId, uint32_t mapId, uint32_t instanceId, const LocationVector& position);

    PlayerCheat m_cheats = {false};
    float m_goLastXRotation = 0.0f;
    float m_goLastYRotation = 0.0f;

    bool m_saveAllChangesCommand = false;

    AIInterface* m_aiInterfaceWaypoint = nullptr;

    bool m_isGmInvisible = false;

    Creature* m_formationMaster = nullptr;

private:
    bool m_disableAppearing = false;
    bool m_disableSummoning = false;

    uint64_t m_GMSelectedGO = 0;

    uint32_t m_banned = 0;
    std::string m_banreason;
    uint32_t m_kickDelay = 0;

    struct SummonData
    {
        uint32_t summonerId = 0;
        uint32_t mapId = 0;
        uint32_t instanceId = 0;
        LocationVector position = { 0, 0, 0, 0 };
    };
    SummonData m_summonData;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Items
public:
    void unEquipOffHandIfRequired();
    bool hasOffHandWeapon() const;

    bool hasItem(uint32_t itemId, uint32_t amount = 1, bool checkBankAlso = false) const;

#if VERSION_STRING >= WotLK
    // Soulbound Tradeable
    void updateSoulboundTradeItems();
    void addTradeableItem(Item* item);
    void removeTradeableItem(Item* item);
    ItemDurationList m_itemSoulboundTradeable;
#endif

#if VERSION_STRING > TBC
    void calculateHeirloomBonus(ItemProperties const* proto, int16_t slot, bool apply);
    DBC::Structures::ScalingStatDistributionEntry const* getScalingStatDistributionFor(ItemProperties const& itemProto) const;
    DBC::Structures::ScalingStatValuesEntry const* getScalingStatValuesFor(ItemProperties const& itemProto) const;
#endif

    // Player's item storage
    ItemInterface* getItemInterface() const;

    void removeTempItemEnchantsOnArena();

    void addGarbageItem(Item* item);

    void applyItemMods(Item* item, int16 slot, bool apply, bool justBrokedown = false, bool skipStatApply = false);

private:
    ItemInterface* m_itemInterface = nullptr;

    void removeGarbageItems();
    std::list<Item*> m_GarbageItems;

protected:
    std::list<ItemSet> m_itemSets;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Difficulty
public:
    void setDungeonDifficulty(uint8_t diff);
    uint8_t getDungeonDifficulty();

    void setRaidDifficulty(uint8_t diff);
    uint8_t getRaidDifficulty();

private:
    uint8_t m_dungeonDifficulty = 0;
    uint8_t m_raidDifficulty = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Die, Kill, Corpse & Repop
public:
    void die(Unit* unitAttacker, uint32_t damage, uint32_t spellId) override;
    void kill();

    void setCorpseData(LocationVector position, int32_t instanceId);
    LocationVector getCorpseLocation() const;
    int32_t getCorpseInstanceId() const;

    void setAllowedToCreateCorpse(bool allowed);
    bool isAllowedToCreateCorpse() const;

    void createCorpse();
    void spawnCorpseBody();
    void spawnCorpseBones();

    void repopRequest();
    void repopAtGraveyard(float ox, float oy, float oz, uint32_t mapId);
    void resurrect();
    void buildRepop();
    void calcDeathDurabilityLoss(double percent);

    void setResurrecterGuid(uint64_t guid);
    void setResurrectHealth(uint32_t health);
    void setResurrectMana(uint32_t mana);
    void setResurrectInstanceId(uint32_t id);
    void setResurrectMapId(uint32_t id);
    void setResurrectPosition(LocationVector position);

    uint64_t getAreaSpiritHealerGuid() const;
    void setAreaSpiritHealerGuid(uint64_t guid);

    void setFullHealthMana();
    void setResurrect();

private:
    struct CorpseData
    {
        LocationVector location = {0,0,0,0};
        int32_t instanceId = 0;
    };
    CorpseData m_corpseData;

    bool m_isCorpseCreationAllowed = true;

    uint64_t m_resurrecter = 0;
    uint32_t m_resurrectHealth = 0;
    uint32_t m_resurrectMana = 0;
    uint32_t m_resurrectInstanceID = 0;
    uint32_t m_resurrectMapId = 0;
    LocationVector m_resurrectPosition = { 0, 0, 0, 0 };

    uint64_t m_areaSpiritHealerGuid = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Bind
public:
    void setBindPoint(float x, float y, float z, float o, uint32_t mapId, uint32_t zoneId);

    LocationVector getBindPosition() const;
    uint32_t getBindMapId() const;
    uint32_t getBindZoneId() const;

    bool m_hasBindDialogOpen = false;

private:
    struct BindData
    {
        LocationVector location = { 0, 0, 0, 0 };
        uint32_t mapId = 0;
        uint32_t zoneId = 0;
    };
    BindData m_bindData;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Battleground Entry
public:
    void setBGEntryPoint(float x, float y, float z, float o, uint32_t mapId, int32_t instanceId);

    LocationVector getBGEntryPosition() const;
    uint32_t getBGEntryMapId() const;
    int32_t getBGEntryInstanceId() const;

private:
    struct BGEntryData
    {
        LocationVector location = { 0, 0, 0, 0 };
        uint32_t mapId = 0;
        int32_t instanceId = 0;
    };
    BGEntryData m_bgEntryData;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Charter
public:
    void unsetCharter(uint8_t charterType);
    Charter* getCharter(uint8_t charterType);

    bool canSignCharter(Charter* charter, Player* requester);
    void initialiseCharters();

private:
    Charter* m_charters[NUM_CHARTER_TYPES] = {nullptr};

    //////////////////////////////////////////////////////////////////////////////////////////
    // Guild
public:
    void setInvitedByGuildId(uint32_t GuildId);
    uint32_t getInvitedByGuildId() const;

    Guild* getGuild() const;
    bool isInGuild();

    uint32_t getGuildRankFromDB();

private:
    uint32_t m_invitedByGuildId = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Group
public:
    void setGroupInviterId(uint32_t inviterId);
    uint32_t getGroupInviterId() const;
    bool isAlreadyInvitedToGroup() const;

    bool isInGroup() const;

    Group* getGroup();
    bool isGroupLeader() const;

    int8_t getSubGroupSlot() const;

    uint32_t getGroupUpdateFlags() const;
    void setGroupUpdateFlags(uint32_t flags);
    void addGroupUpdateFlag(uint32_t flag);
    uint16_t getGroupStatus();

    void sendUpdateToOutOfRangeGroupMembers();

    void eventGroupFullUpdate();

    bool isSendOnlyRaidgroupSet() const;
    void setSendOnlyRaidgroup(bool set);

    LocationVector getLastGroupPosition() const;

private:
    uint32_t m_grouIdpInviterId = 0;

    bool m_sendOnlyRaidgroup = false;

    LocationVector m_lastGroupPosition;

    uint32 m_groupUpdateFlags;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Channels
public:
    void joinedChannel(Channel* channel);
    void leftChannel(Channel* channel);

    void updateChannels();
    void removeAllChannels();

private:
    std::set<Channel*> m_channels;
    mutable std::mutex m_mutexChannel;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Arena
public:
    void setArenaTeam(uint8_t type, ArenaTeam* arenaTeam);
    ArenaTeam* getArenaTeam(uint8_t type);

    bool isInArenaTeam(uint8_t type) const;
    void initialiseArenaTeam();

    void addArenaPoints(uint32_t arenaPoints, bool sendUpdate);
    uint32_t getArenaPoints() const;
    void removeArenaPoints(uint32_t arenaPoints, bool sendUpdate);
    void updateArenaPoints();

    void setInviteArenaTeamId(uint32_t id);
    uint32_t getInviteArenaTeamId() const;

private:
    ArenaTeam* m_arenaTeams[NUM_ARENA_TEAM_TYPES] = {nullptr};
    uint32_t m_arenaPoints = 0;
    uint32_t m_inviteArenaTeamId = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Honor
public:
    void addHonor(uint32_t honorPoints, bool sendUpdate);
    uint32_t getHonor() const;
    void removeHonor(uint32_t honorPoints, bool sendUpdate);

    void updateHonor();

    void rolloverHonor();
    uint32_t getHonorToday() const;
    uint32_t getHonorYesterday() const;
    uint32_t getHonorless() const;
    void incrementHonorless();
    void decrementHonorless();

    void incrementKills(uint32_t count = 0);
    uint32_t getKillsToday() const;
    uint32_t getKillsLifetime() const;
    uint32_t getKillsYesterday() const;

private:
    uint32_t m_honorPoints = 0;

    uint32_t m_honorRolloverTime = 0;
    uint32_t m_lastHonorResetTime = 0;

    uint32_t m_honorToday = 0;
    uint32_t m_honorYesterday = 0;
    uint32_t m_honorless = 0;

    uint32_t m_killsToday = 0;
    uint32_t m_killsLifetime = 0;
    uint32_t m_killsYesterday = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // PvP
public:
    void resetPvPTimer();
    void stopPvPTimer();

    void setupPvPOnLogin();
    void updatePvPArea();
    void togglePvP();

    void updatePvPCurrencies();

    bool hasPvPTitle(RankTitles title);
    void setKnownPvPTitle(RankTitles title, bool set);

private:
    uint32 m_pvpTimer = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Battleground
public:
    CBattleground* getBattleground() const;
    void setBattleground(CBattleground* bg);

    CBattleground* getPendingBattleground() const;
    void setPendingBattleground(CBattleground* bg);

    bool isQueuedForBg() const;
    void setIsQueuedForBg(bool set);

    bool hasQueuedBgInstanceId() const;
    uint32_t getQueuedBgInstanceId() const;
    void setQueuedBgInstanceId(uint32_t id);

    bool isQueuedForRbg() const;
    void setIsQueuedForRbg(bool value);

    void removeFromBgQueue();

    bool hasWonRbgToday() const;
    void setHasWonRbgToday(bool value);

    void setBgQueueType(uint32_t type);
    uint32_t getBgQueueType() const;

    bool hasBgFlag() const;
    void setHasBgFlag(bool set);

    void setRoles(uint8_t role);
    uint8_t retRoles() const;

    void fillRandomBattlegroundReward(bool wonBattleground, uint32_t& honorPoints, uint32_t& arenaPoints);
    void applyRandomBattlegroundReward(bool wonBattleground);

    BGScore m_bgScore;

private:
    CBattleground* m_bg = nullptr;
    CBattleground* m_pendingBattleground = nullptr;

    bool m_isQueuedForBg = false;
    uint32_t m_queuedBgInstanceId = 0;

    bool m_isQueuedForRbg = false;
    bool m_hasWonRbgToday = false;

    uint32_t m_bgQueueType = 0;

    bool m_bgHasFlag = false;

    uint8_t m_roles = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Quests
public:
    void acceptQuest(uint64_t guid, uint32_t quest_id);

    void setQuestLogInSlot(QuestLogEntry* entry, uint32_t slotId);

    bool hasAnyQuestInQuestSlot() const;
    bool hasQuestInQuestLog(uint32_t questId) const;
    uint8_t getFreeQuestSlot() const;

    QuestLogEntry* getQuestLogByQuestId(uint32_t questId) const;
    QuestLogEntry* getQuestLogBySlotId(uint32_t slotId) const;

    void addQuestIdToFinishedDailies(uint32_t questId);
    std::set<uint32_t> getFinishedDailies() const;
    bool hasQuestInFinishedDailies(uint32_t questId) const;
    void resetFinishedDailies();

    bool hasTimedQuestInQuestSlot() const;
    void eventTimedQuestExpire(uint32_t questId);

    uint32_t getQuestSharerByDbId() const;
    void setQuestSharerDbId(uint32_t id);

    void addQuestToRemove(uint32_t questId);

    void addQuestToFinished(uint32_t questId);
    bool hasQuestFinished(uint32_t questId);

    void areaExploredQuestEvent(uint32_t questId);

    void clearQuest(uint32_t questId);

    bool hasQuestForItem(uint32_t itemId);

    void addQuestSpell(uint32_t spellId);
    bool hasQuestSpell(uint32_t spellId);
    void removeQuestSpell(uint32_t spellId);

    void addQuestMob(uint32_t entry);
    bool hasQuestMob(uint32_t entry);
    void removeQuestMob(uint32_t entry);

    void addQuestKill(uint32_t questId, uint8_t reqId, uint32_t delay = 0);

    std::set<uint32_t> getFinishedQuests() const;

private:
    QuestLogEntry* m_questlog[MAX_QUEST_LOG_SIZE] = {nullptr};

    mutable std::mutex m_mutextDailies;
    std::set<uint32_t> m_finishedDailies = {};

    std::set<uint32_t> m_removequests = {};
    std::set<uint32_t> m_finishedQuests = {};
    uint32_t m_questSharer = 0;
    std::set<uint32_t> quest_spells = {};
    std::set<uint32_t> quest_mobs = {};

    //////////////////////////////////////////////////////////////////////////////////////////
    // Social
public:

    struct SocialFriends
    {
        uint32_t friendGuid = 0;
        mutable std::string note;
    };

    // Initialise Database values
    void loadFriendList();
    void loadFriendedByOthersList();
    void loadIgnoreList();

    void addToFriendList(std::string name, std::string note);
    void removeFromFriendList(uint32_t guid);
    void addNoteToFriend(uint32_t guid, std::string note);
    bool isFriended(uint32_t guid) const;

    void sendFriendStatus(bool comesOnline);
    void sendFriendLists(uint32_t flags);

    void addToIgnoreList(std::string name);
    void removeFromIgnoreList(uint32_t guid);
    bool isIgnored(uint32_t guid) const;

private:
    std::vector<SocialFriends> m_socialIFriends = {};
    mutable std::mutex m_mutexFriendList;

    std::vector<uint32_t> m_socialFriendedByGuids = {};
    mutable std::mutex m_mutexFriendedBy;

    std::vector<uint32_t> m_socialIgnoring = {};
    mutable std::mutex m_mutexIgnoreList;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Hack/Cheat Detection
public:
    void speedCheatDelay(uint32_t delay);
    void speedCheatReset();

private:
    SpeedCheatDetector* m_speedCheatDetector;

    //Speed
    //Fly
    //Teleport
    //NoClip
    //Waterwalk
    //Size
    //Wallclimb
    //Itemstacking (spell/attack power stacking)

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
public:

    bool isGMFlagSet();

    void sendMovie(uint32_t movieId);

    void logIntoBattleground();
    bool logOntoTransport();
    void setLoginPosition();
    void setPlayerInfoIfNeeded();
    void setGuildAndGroupInfo();
    void sendCinematicOnFirstLogin();

    void sendTalentResetConfirmPacket();
    void sendPetUnlearnConfirmPacket();
    void sendDungeonDifficultyPacket();
    void sendRaidDifficultyPacket();
    void sendInstanceDifficultyPacket(uint8_t difficulty);
    void sendNewDrunkStatePacket(uint32_t state, uint32_t itemId);
    void sendSetProficiencyPacket(uint8_t itemClass, uint32_t proficiency);
    void sendPartyKillLogPacket(uint64_t killedGuid);
    void sendDestroyObjectPacket(uint64_t destroyedGuid);
    void sendEquipmentSetUseResultPacket(uint8_t result);
    void sendTotemCreatedPacket(uint8_t slot, uint64_t guid, uint32_t duration, uint32_t spellId);

    void sendGossipPoiPacket(float posX, float posY, uint32_t icon, uint32_t flags, uint32_t data, std::string name);
    void sendPoiById(uint32_t id);

    void sendStopMirrorTimerPacket(MirrorTimerTypes type);
    void sendMeetingStoneSetQueuePacket(uint32_t dungeonId, uint8_t status);
    void sendPlayObjectSoundPacket(uint64_t objectGuid, uint32_t soundId);
    void sendPlaySoundPacket(uint32_t soundId);
    void sendExploreExperiencePacket(uint32_t areaId, uint32_t experience);
    void sendSpellCooldownEventPacket(uint32_t spellId);
#if VERSION_STRING < Cata
    void sendSpellModifierPacket(uint8_t spellGroup, uint8_t spellType, int32_t modifier, bool isPct);
#else
    void sendSpellModifierPacket(uint8_t spellType, std::vector<std::pair<uint8_t, float>> modValues, bool isPct);
#endif
    void sendLoginVerifyWorldPacket(uint32_t mapId, float posX, float posY, float posZ, float orientation);
    void sendMountResultPacket(uint32_t result);
    void sendDismountResultPacket(uint32_t result);
    
    void sendCastFailedPacket(uint32_t spellId, uint8_t errorMessage, uint8_t multiCast, uint32_t extra1, uint32_t extra2 = 0);
    void sendLevelupInfoPacket(uint32_t level, uint32_t hp, uint32_t mana, uint32_t stat0, uint32_t stat1, uint32_t stat2, uint32_t stat3, uint32_t stat4);
    void sendItemPushResultPacket(bool created, bool recieved, bool sendtoset, uint8_t destbagslot, uint32_t destslot, uint32_t count, uint32_t entry, uint32_t suffix, uint32_t randomprop, uint32_t stack);
    void sendClientControlPacket(Unit* target, uint8_t allowMove);
    void sendGuildMotd();

    bool isPvpFlagSet() override;
    void setPvpFlag() override;
    void removePvpFlag() override;

    bool isFfaPvpFlagSet() override;
    void setFfaPvpFlag() override;
    void removeFfaPvpFlag() override;

    bool isSanctuaryFlagSet() override;
    void setSanctuaryFlag() override;
    void removeSanctuaryFlag() override;

    void sendPvpCredit(uint32_t honor, uint64_t victimGuid, uint32_t victimRank);
    void sendRaidGroupOnly(uint32_t timeInMs, uint32_t type);

    void setVisibleItemFields(uint32_t slot, Item* item);
    void applyReforgeEnchantment(Item* item, bool apply);

    void setAFKReason(std::string reason);
    std::string getAFKReason() const;

    void addToGMTargetList(uint32_t guid);
    void removeFromGMTargetList(uint32_t guid);
    bool isOnGMTargetList(uint32_t guid) const;

    bool isAtGroupRewardDistance(Object* pRewardSource);

    void tagUnit(Object* object);

#if VERSION_STRING > TBC
    AchievementMgr& getAchievementMgr();
#endif

    void sendUpdateDataToSet(ByteBuffer* groupBuf, ByteBuffer* nonGroupBuf, bool sendToSelf);
    void sendWorldStateUpdate(uint32_t worldState, uint32_t value);

    bool canBuyAt(MySQLStructure::VendorRestrictions const* vendor);
    bool canTrainAt(Trainer* trainer);

    void sendCinematicCamera(uint32_t id);

    void setMover(Unit* target);

    void resetTimeSync();
    void sendTimeSync();

private:
    uint16_t m_spellAreaUpdateTimer = 1000;
    uint16_t m_pendingPacketTimer = 100;
    uint16_t m_partyUpdateTimer = 1000;
    uint16_t m_durationUpdateTimer = 1000;

    std::string afkReason;

    std::vector<uint32_t> m_gmPlayerTargetList;
    mutable std::mutex m_lockGMTargetList;

#if VERSION_STRING > TBC
    AchievementMgr m_achievementMgr = this;
#endif

    uint32 m_timeSyncCounter = 0;
    uint32 m_timeSyncTimer = 0;
    uint32 m_timeSyncClient = 0;
    uint32 m_timeSyncServer = 0;

#if VERSION_STRING > WotLK
    /////////////////////////////////////////////////////////////////////////////////////////
    // Void Storage
public:
    void loadVoidStorage();
    void saveVoidStorage();

    bool isVoidStorageUnlocked() const;
    void unlockVoidStorage();
    void lockVoidStorage();

    uint8_t getNextVoidStorageFreeSlot() const;
    uint8_t getNumOfVoidStorageFreeSlots() const;
    uint8_t addVoidStorageItem(const VoidStorageItem& item);
    void addVoidStorageItemAtSlot(uint8_t slot, const VoidStorageItem& item);
    void deleteVoidStorageItem(uint8_t slot);
    bool swapVoidStorageItem(uint8_t oldSlot, uint8_t newSlot);

    VoidStorageItem* getVoidStorageItem(uint8_t slot) const;
    VoidStorageItem* getVoidStorageItem(uint64_t id, uint8_t& slot) const;

private:
    VoidStorageItem* _voidStorageItems[VOID_STORAGE_MAX_SLOT];
#endif

    /////////////////////////////////////////////////////////////////////////////////////////
    // Taxi
public:   
    TaxiPath* getTaxiPath() const;
    void setTaxiPath(TaxiPath* path);

    void loadTaxiMask(const char* data);
    const uint32_t& getTaxiMask(uint32_t index) const;
    void setTaxiMask(uint32_t index, uint32_t value);

    void setTaxiPosition();
    void unsetTaxiPosition();

    bool isOnTaxi() const;
    void setOnTaxi(bool state);

    void startTaxiPath(TaxiPath* path, uint32_t modelid, uint32_t start_node);
    void skipTaxiPathNodesToEnd(TaxiPath* path);
    void dismountAfterTaxiPath(uint32_t money, float x, float y, float z);
    void interpolateTaxiPosition();

    void eventTeleportTaxi(uint32_t mapId, float x, float y, float z);

private:
    TaxiPath* m_currentTaxiPath = nullptr;

    uint32_t m_taxiMountDisplayId = 0;
    uint32_t m_lastTaxiNode = 0;
    uint32_t m_taxiMapChangeNode = 0;
    uint32_t m_taxiRideTime = 0;
    uint32_t m_taxiMask[DBC_TAXI_MASK_SIZE];

    LocationVector m_taxiPosition = { 0, 0, 0 };

    bool m_isOnTaxi = false;
    std::vector<TaxiPath*> m_taxiPaths;

    /////////////////////////////////////////////////////////////////////////////////////////
    // Loot
public:
    const uint64_t& getLootGuid() const;
    void setLootGuid(const uint64_t& guid);

    void sendLoot(uint64_t guid, uint8_t loot_type, uint32_t mapId);
    void sendLootUpdate(Object* object);

    void sendLooter(Creature* creature);
    Item* storeNewLootItem(uint8_t slot, Loot* loot);
    Item* storeItem(LootItem const* lootItem);

    bool isLootableOnCorpse() const;
    void setLootableOnCorpse(bool lootable);

private:
    uint64_t m_lootGuid = 0;
    uint64_t m_currentLoot = 0;
    bool m_lootableOnCorpse = false;

    /////////////////////////////////////////////////////////////////////////////////////////
    // Reputation/faction
public:
    void setFactionStanding(uint32_t faction, int32_t value);
    int32_t getFactionStanding(uint32_t faction);
    int32_t getBaseFactionStanding(uint32_t faction);
    void modFactionStanding(uint32_t faction, int32_t value);
    
    Standing getFactionStandingRank(uint32_t faction);
    static Standing getReputationRankFromStanding(int32_t value);

    void setFactionAtWar(uint32_t faction, bool set);

    bool isHostileBasedOnReputation(DBC::Structures::FactionEntry const* factionEntry);
    void updateInrangeSetsBasedOnReputation();

    void onKillUnitReputation(Unit* unit, bool innerLoop);
    void onTalkReputation(DBC::Structures::FactionEntry const* factionEntry);
    
    void setFactionInactive(uint32_t faction, bool set);
    bool addNewFaction(DBC::Structures::FactionEntry const* factionEntry, int32_t standing, bool base);
    void onModStanding(DBC::Structures::FactionEntry const* factionEntry, FactionReputation* reputation);
    uint32_t getExaltedCount();

    void sendSmsgInitialFactions();
    void initialiseReputation();
    uint32_t getInitialFactionId();

    int32_t getPctReputationMod() const;
    void setPctReputationMod(int32_t value);

    void setChampioningFaction(uint32_t factionId);

private:
    ReputationMap m_reputation;
    int32_t m_pctReputationMod = 0;
    FactionReputation* m_reputationByListId[128] = { nullptr };

    uint32_t m_championingFactionId = 0;

    /////////////////////////////////////////////////////////////////////////////////////////
    // Drunk system
public:
    uint16_t getServersideDrunkValue() const;
    void setServersideDrunkValue(uint16 newDrunkValue, uint32_t itemId = 0);
    static DrunkenState getDrunkStateByValue(uint16_t value);
    void handleSobering();

private:
    uint32_t m_drunkTimer = 0;
    uint16_t m_serversideDrunkValue = 0;

    /////////////////////////////////////////////////////////////////////////////////////////
    // Duel
public:
    Player* getDuelPlayer() const;
    void requestDuel(Player* target);
    void testDuelBoundary();
    void endDuel(uint8_t condition);
    void cancelDuel();

    void handleDuelCountdown();

    void setDuelStatus(uint8_t status);
    uint8_t getDuelStatus() const;

    void setDuelState(uint8_t state);
    uint8_t getDuelState() const;

private:
    Player* m_duelPlayer = nullptr;
    uint8_t m_duelStatus = 0;
    uint8_t m_duelState = DUEL_STATE_FINISHED;
    uint32_t m_duelCountdownTimer = 0;

    /////////////////////////////////////////////////////////////////////////////////////////
    // Resting/Experience XP
public:
    void GiveXP(uint32_t xp, const uint64_t& guid, bool allowBonus);
    void sendLogXpGainPacket(uint64_t guid, uint32_t normalXp, uint32_t restedXp, bool type);

    void toggleXpGain();
    bool canGainXp() const;

    uint32_t subtractRestXp(uint32_t amount);
    void addCalculatedRestXp(uint32_t seconds);

    void applyPlayerRestState(bool apply);
    void updateRestState();

private:
    bool m_isXpGainAllowed = true;

    uint8_t m_isResting = 0;
    uint8_t m_restState = 0;
    uint32_t m_restAmount = 0;

    /////////////////////////////////////////////////////////////////////////////////////////
    // Pets/Summons
public:
    std::list<Pet*> getSummons();
    void addPetToSummons(Pet* pet);
    void removePetFromSummons(Pet* pet);
    Pet* getFirstPetFromSummons() const;

    PlayerPet* getPlayerPet(uint32_t petId);
    void addPlayerPet(PlayerPet* pet, uint32_t index);
    void removePlayerPet(uint32_t petId);
    uint8_t getPetCount() const;

    uint32_t getFreePetNumber() const;

    void spawnPet(uint32_t petId);
    void spawnActivePet();
    void dismissActivePets();

    void setStableSlotCount(uint8_t count);
    uint8_t getStableSlotCount() const;

    uint32_t getUnstabledPetNumber() const;

    void eventSummonPet(Pet* summonPet);
    void eventDismissPet();

    Object* getSummonedObject() const;
    void setSummonedObject(Object* summonedObject);

private:
    std::list<Pet*> m_summons;
    std::map<uint32_t, PlayerPet*> m_pets;

    uint8_t m_stableSlotCount = 0;
    uint32_t m_maxPetNumber = 0;

    Object* m_summonedObject = nullptr;

public:
    //MIT End
    /////////////////////////////////////////////////////////////////////////////////////////
    // After this point we have to deal with legacy code -.-
    // Keep the code above this line clean and structured (encapsulating)
    /////////////////////////////////////////////////////////////////////////////////////////
    //AGPL Start

    protected:

        // COOLDOWNS
        uint32 m_lastPotionId = 0;
        PlayerCooldownMap m_cooldownMap[NUM_COOLDOWN_TYPES];
        uint32 m_globalCooldown = 0;

    public:
        void SetLastPotion(uint32 itemid) { m_lastPotionId = itemid; }
        void Cooldown_AddItem(ItemProperties const* pProto, uint32 x);
        bool Cooldown_CanCast(ItemProperties const* pProto, uint32 x);
        void UpdatePotionCooldown();
        bool HasSpellWithAuraNameAndBasePoints(uint32 auraname, uint32 basepoints);

    protected:

        void AddCategoryCooldown(uint32 category_id, uint32 time, uint32 SpellId, uint32 ItemId);
        void _Cooldown_Add(uint32 Type, uint32 Misc, uint32 Time, uint32 SpellId, uint32 ItemId);
        void _LoadPlayerCooldowns(QueryResult* result);
        void _SavePlayerCooldowns(QueryBuffer* buf);

        // END COOLDOWNS
    public:

        //! Okay to remove from world
        bool ok_to_remove = false;

        void SendEquipmentSetList();
        void SendEquipmentSetSaved(uint32 setID, uint32 setGUID);
        
        void SendEmptyPetSpellList();

        void SendInitialWorldstates();

        void OutPacket(uint16 opcode, uint16 len, const void* data);
        void SendPacket(WorldPacket* packet);
        void SendMessageToSet(WorldPacket* data, bool self, bool myteam_only = false);
        void OutPacketToSet(uint16 Opcode, uint16 Len, const void* Data, bool self);

        static void CharChange_Looks(uint64 GUID, uint8 gender, uint8 skin, uint8 face, uint8 hairStyle, uint8 hairColor, uint8 facialHair);
        static void CharChange_Language(uint64 GUID, uint8 race);

        void RemoveFromWorld();
        bool Create(CharCreate& charCreateContent);

        void BuildFlagUpdateForNonGroupSet(uint32 index, uint32 flag);
        void BuildPetSpellList(WorldPacket & data);

        void ModifyBonuses(uint32 type, int32 val, bool apply);
        void CalcExpertise();
        std::map<uint32, uint32> m_wratings;

        /////////////////////////////////////////////////////////////////////////////////////////
        // Spells
        /////////////////////////////////////////////////////////////////////////////////////////
        bool HasSpell(uint32 spell);
        bool HasDeletedSpell(uint32 spell);
        void smsg_InitialSpells();
        void addSpell(uint32 spell_idy, uint16_t fromSkill = 0);
        bool removeSpell(uint32 SpellID, bool MoveToDeleted, bool SupercededSpell, uint32 SupercededSpellID);
        bool removeDeletedSpell(uint32 SpellID);
        void SendPreventSchoolCast(uint32 SpellSchool, uint32 unTimeMs);

        // PLEASE DO NOT INLINE!
        void AddOnStrikeSpell(SpellInfo const* sp, uint32 delay)
        {
            m_onStrikeSpells.insert(std::map<SpellInfo const*, std::pair<uint32, uint32>>::value_type(sp, std::make_pair(delay, 0)));
        }
        void RemoveOnStrikeSpell(SpellInfo const* sp)
        {
            m_onStrikeSpells.erase(sp);
        }
        void AddOnStrikeSpellDamage(uint32 spellid, uint32 mindmg, uint32 maxdmg)
        {
            OnHitSpell sp;
            sp.spellid = spellid;
            sp.mindmg = mindmg;
            sp.maxdmg = maxdmg;
            m_onStrikeSpellDmg[spellid] = sp;
        }
        void RemoveOnStrikeSpellDamage(uint32 spellid)
        {
            m_onStrikeSpellDmg.erase(spellid);
        }

        //Spells variables
        StrikeSpellMap m_onStrikeSpells;
        StrikeSpellDmgMap m_onStrikeSpellDmg;
        SpellOverrideMap mSpellOverrideMap;
        SpellSet mSpells;
        SpellSet mDeletedSpells;
        SpellSet mShapeShiftSpells;

        void AddShapeShiftSpell(uint32 id);
        void RemoveShapeShiftSpell(uint32 id);

        bool IsInFeralForm()
        {
            //\todo shapeshiftform is never negative.
            int s = getShapeShiftForm();
            if (s <= 0)
                return false;

            // Fight forms that do not use player's weapon
            return (s == FORM_BEAR || s == FORM_DIREBEAR || s == FORM_CAT);     //Shady: actually ghostwolf form doesn't use weapon too.
        }

        void CalcDamage();
        float offhand_dmg_mod = 0.5f;

        int32 GetDamageDoneMod(uint16_t school)
        {
            if (school >= TOTAL_SPELL_SCHOOLS)
                return 0;

            return static_cast<int32>(getModDamageDonePositive(school)) - static_cast<int32>(getModDamageDoneNegative(school));
        }

        float GetDamageDonePctMod(uint16_t school)
        {
            if (school >= TOTAL_SPELL_SCHOOLS)
                return 0;

            return getModDamageDonePct(static_cast<uint8_t>(school));
        }

        uint32 GetMainMeleeDamage(uint32 AP_owerride);          // I need this for windfury
        
        void SendAvailSpells(DBC::Structures::SpellShapeshiftFormEntry const* shapeshift_form, bool active);

public:

        // Talents
        void SetTalentHearthOfWildPCT(int value) { hearth_of_wild_pct = value; }
        void EventTalentHearthOfWildChange(bool apply);

        std::list<LoginAura> loginauras;

        /////////////////////////////////////////////////////////////////////////////////////////
        // Player loading and savings Serialize character to db
        /////////////////////////////////////////////////////////////////////////////////////////
        void SaveToDB(bool bNewCharacter);
        void SaveAuras(std::stringstream &);
        bool LoadFromDB(uint32 guid);
        void LoadFromDBProc(QueryResultVector & results);

        bool SaveSpells(bool NewCharacter, QueryBuffer* buf);

        bool LoadDeletedSpells(QueryResult* result);
        bool SaveDeletedSpells(bool NewCharacter, QueryBuffer* buf);

        bool SaveReputations(bool NewCharacter, QueryBuffer *buf);
        bool SaveSkills(bool NewCharacter, QueryBuffer* buf);

        bool m_FirstLogin = false;

        /////////////////////////////////////////////////////////////////////////////////////////
        // Talent Specs
        /////////////////////////////////////////////////////////////////////////////////////////
        uint16 m_maxTalentPoints = 0;
        uint8 m_talentSpecsCount = 1;
        uint8 m_talentActiveSpec = 0;
#if VERSION_STRING >= Cata
        uint32 m_FirstTalentTreeLock = 0;
#endif

#ifdef FT_DUAL_SPEC
        PlayerSpec m_specs[MAX_SPEC_COUNT];
#else
        PlayerSpec m_spec;
#endif

        PlayerSpec& getActiveSpec();

#if VERSION_STRING > TBC
        uint32 GetGlyph(uint32 spec, uint32 slot) const { return m_specs[spec].glyphs[slot]; }
#endif

        /////////////////////////////////////////////////////////////////////////////////////////
        // Attack stuff
        /////////////////////////////////////////////////////////////////////////////////////////
public:
        void EventAttackStart();
        void EventAttackStop();
        void EventAttackUpdateSpeed() { }
        void EventDeath();


        /////////////////////////////////////////////////////////////////////////////////////////
        // Visible objects
        /////////////////////////////////////////////////////////////////////////////////////////
    public:
        bool IsVisible(uint64 pObj) { return !(m_visibleObjects.find(pObj) == m_visibleObjects.end()); }
        void addToInRangeObjects(Object* pObj);
        void onRemoveInRangeObject(Object* pObj);
        void clearInRangeSets();
        void AddVisibleObject(uint64 pObj) { m_visibleObjects.insert(pObj); }
        void RemoveVisibleObject(uint64 pObj) { m_visibleObjects.erase(pObj); }
        void RemoveVisibleObject(std::set< uint64 >::iterator itr) { m_visibleObjects.erase(itr); }
        std::set< uint64 >::iterator FindVisible(uint64 obj) { return m_visibleObjects.find(obj); }
        void RemoveIfVisible(uint64 obj);
protected:
        std::set<uint64> m_visibleObjects;
public:
        /////////////////////////////////////////////////////////////////////////////////////////
        //  PVP Stuff
        /////////////////////////////////////////////////////////////////////////////////////////
        

        //Note:ModSkillLine -> value+=amt;ModSkillMax -->value=amt; --weird
        float GetSkillUpChance(uint16_t id);

        float SpellHasteRatingBonus = 1.0f;
        void UpdateAttackSpeed();
        float GetDefenseChance(uint32 opLevel);
        float GetDodgeChance();
        float GetBlockChance();
        float GetParryChance();
        void UpdateChances();
        void UpdateStats();
        uint32 GetBlockDamageReduction();
        void ApplyFeralAttackPower(bool apply, Item* item = NULL);

        float GetSpellCritFromSpell() { return m_spellcritfromspell; }
        float GetHitFromSpell() { return m_hitfromspell; }
        void SetSpellCritFromSpell(float value) { m_spellcritfromspell = value; }
        void SetHitFromSpell(float value) { m_hitfromspell = value; }

        uint32 GetHealthFromSpell() { return m_healthfromspell; }
        uint32 GetManaFromSpell() { return m_manafromspell; }
        void SetHealthFromSpell(uint32 value) { m_healthfromspell = value;}
        void SetManaFromSpell(uint32 value) { m_manafromspell = value;}

        uint32 m_nextSave;

        int m_lifetapbonus = 0;         //warlock spell related
        bool m_requiresNoAmmo = false;      //warlock spell related

        // Misc
        void EventCannibalize(uint32 amount);

        uint32 m_modblockabsorbvalue = 0;
        uint32 m_modblockvaluefromspells = 0;
        void SendInitialLogonPackets();
        void Reset_Spells();

        void EventActivateGameObject(GameObject* obj);
        void EventDeActivateGameObject(GameObject* obj);
        void UpdateNearbyGameObjects();

        void CalcResistance(uint8_t type);
        float res_M_crit_get() { return m_resist_critical[0]; }
        void res_M_crit_set(float newvalue) { m_resist_critical[0] = newvalue; }
        float res_R_crit_get() { return m_resist_critical[1]; }
        void res_R_crit_set(float newvalue) { m_resist_critical[1] = newvalue; }
        uint32 FlatResistanceModifierPos[TOTAL_SPELL_SCHOOLS] = {0};
        uint32 FlatResistanceModifierNeg[TOTAL_SPELL_SCHOOLS] = {0};
        uint32 BaseResistanceModPctPos[TOTAL_SPELL_SCHOOLS] = {0};
        uint32 BaseResistanceModPctNeg[TOTAL_SPELL_SCHOOLS] = {0};
        uint32 ResistanceModPctPos[TOTAL_SPELL_SCHOOLS] = {0};
        uint32 ResistanceModPctNeg[TOTAL_SPELL_SCHOOLS] = {0};
        float m_resist_critical[2] = {0};             // when we are a victim we can have talents to decrease chance for critical hit. This is a negative value and it's added to critchances
        float m_resist_hit[2] = {0};                  // 0 = melee; 1= ranged;
        int32 m_resist_hit_spell[TOTAL_SPELL_SCHOOLS] = {0}; // spell resist per school
        uint32 m_modphyscritdmgPCT = 0;
        uint32 m_RootedCritChanceBonus = 0;         // Class Script Override: Shatter
        uint32 m_IncreaseDmgSnaredSlowed = 0;

        // SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
        uint32 m_ModInterrMRegenPCT = 0;
        // SPELL_AURA_MOD_POWER_REGEN
        int32 m_ModInterrMRegen = 0;
        // SPELL_AURA_REGEN_MANA_STAT_PCT
        int32_t m_modManaRegenFromStat[STAT_COUNT] = {0};
        float m_RegenManaOnSpellResist = 0.0f;
        uint32 m_casted_amount[TOTAL_SPELL_SCHOOLS] = {0};   // Last casted spells amounts. Need for some spells. Like Ignite etc. DOesn't count HoTs and DoTs. Only directs

        uint32 FlatStatModPos[5] = {0};
        uint32 FlatStatModNeg[5] = {0};
        uint32 StatModPctPos[5] = {0};
        uint32 StatModPctNeg[5] = {0};
        uint32 TotalStatModPctPos[5] = {0};
        uint32 TotalStatModPctNeg[5] = {0};
        int32 IncreaseDamageByType[12] = {0};         // mod dmg by creature type
        float IncreaseDamageByTypePCT[12] = {0};
        float IncreaseCricticalByTypePCT[12] = {0};
        int32 DetectedRange = 0;
        float PctIgnoreRegenModifier = 0.0f;
        uint32 m_retainedrage = 0;                  // Warrior spell related

        void CalcStat(uint8_t t);
        float CalcRating(PlayerCombatRating t);
        void RegenerateHealth(bool inCombat);

        uint64 misdirectionTarget = 0;              // Hunter spell related

        uint64 GetMisdirectionTarget() { return misdirectionTarget; }
        void SetMisdirectionTarget(uint64 PlayerGUID) { misdirectionTarget = PlayerGUID; }

        bool bReincarnation = false;                    // Shaman spell related

        std::map<uint32, WeaponModifier> damagedone;
        std::map<uint32, WeaponModifier> tocritchance;
        bool cannibalize = false;
        uint8 cannibalizeCount = 0;
        int32 rageFromDamageDealt = 0;
        int32 rageFromDamageTaken = 0;

        void AddItemsToWorld();
        void RemoveItemsFromWorld();
        void UpdateKnownCurrencies(uint32 itemId, bool apply);

        uint32 TrackingSpell = 0;
        void _EventCharmAttack();

        void ClearCooldownsOnLine(uint32 skill_line, uint32 called_from);

        //bool m_Autojoin = false;
        //bool m_AutoAddMem = false;
        void SendMirrorTimer(MirrorTimerTypes Type, uint32 max, uint32 current, int32 regen);

        void UpdateChanceFields();

        uint32 m_lastSeenWeather = 0;
        std::set<Object*> m_visibleFarsightObjects;

    // PVP/BG
        uint32 GetMaxPersonalRating();

    //movement/position
        void _Relocate(uint32 mapid, const LocationVector& v, bool sendpending, bool force_new_world, uint32 instance_id);

        std::map<uint32, std::set<uint32> > SummonSpells;
        std::map<uint32, std::map<SpellInfo const*, uint16>*> PetSpells;

        void AddSummonSpell(uint32 Entry, uint32 SpellID);
        void RemoveSummonSpell(uint32 Entry, uint32 SpellID);
        std::set<uint32>* GetSummonSpells(uint32 Entry);

        void HandleSpellLoot(uint32 itemid);

        void CompleteLoading();
        
        bool blinked = false;
        bool m_beingPushed = false;
        
        uint32 flying_aura = 0;

        bool m_changingMaps = true;
        bool resend_speed = false; // set to true if m_changingMaps is true.

        int32 m_rap_mod_pct = 0;

        bool m_deathVision = false;
    // paladin related
        SpellInfo const* last_heal_spell = nullptr;

        Mailbox m_mailBox;
        bool m_finishingmovesdodge = false;

        bool IsAttacking() { return m_attacking; }

    protected:

        void _SaveQuestLogEntry(QueryBuffer* buf);
        void _LoadQuestLogEntry(QueryResult* result);

        void _LoadPet(QueryResult* result);
        void _LoadPetSpells(QueryResult* result);
        void _SavePet(QueryBuffer* buf);
        void _SavePetSpells(QueryBuffer* buf);
        
        void _EventAttack(bool offhand);
        
        void CastSpellArea();

        /////////////////////////////////////////////////////////////////////////////////////////
        // Player Class systems, info and misc things
        /////////////////////////////////////////////////////////////////////////////////////////
        uint32 m_AttackMsgTimer = 0;        // "too far away" and "wrong facing" timer
        bool m_attacking = false;

        //combat mods
        float m_blockfromspellPCT = 0.0f;
        float m_critfromspell = 0.0f;
        float m_spellcritfromspell = 0.0f;
        float m_hitfromspell = 0.0f;
        //stats mods
        uint32 m_healthfromspell = 0;
        uint32 m_manafromspell = 0;
        uint32 m_healthfromitems = 0;
        uint32 m_manafromitems = 0;

        // Raid
        uint8 m_targetIcon = 0;

        uint32 _fields[getSizeOfStructure(WoWPlayer)];
        int hearth_of_wild_pct = 0;        // druid hearth of wild talent used on shapeshifting. We either know what is last talent level or memo on learn

    public:

        void addDeletedSpell(uint32 id) { mDeletedSpells.insert(id); }

        std::map<uint32, uint32> m_forcedReactions;

        bool m_passOnLoot = false;
        uint32 m_tradeSequence;

        uint32 m_outStealthDamageBonusPct = 0;
        uint32 m_outStealthDamageBonusPeriod = 0;
        uint32 m_outStealthDamageBonusTimer = 0;

        
        uint32 CheckDamageLimits(uint32 dmg, uint32 spellid);

        void LoadFieldsFromString(const char* string, uint16 firstField, uint32 fieldsNum);

        // Avenging Wrath
        void AvengingWrath() { mAvengingWrath = true; }
        bool mAvengingWrath = true;

        // AGPL End
};
