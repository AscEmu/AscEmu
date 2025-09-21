/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Units/Unit.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Objects/Units/Stats.h"
#include "Management/QuestDefines.hpp"
#include "Management/ObjectUpdates/UpdateManager.hpp"
#include "Data/WoWPlayer.hpp"
#include "AEVersion.hpp"
#include "Logging/Log.hpp"
#include "Server/UpdateFieldInclude.h"
#include "Objects/ItemDefines.hpp"

// todo include header for now struct InstancePlayerBind definition
#include "Map/Maps/InstanceMgr.hpp"

#include <Utilities/utf8.hpp>

#include <mutex>

#include "Utilities/CallBack.h"

class QueryResult;

namespace WDB::Structures
{
    struct SpellShapeshiftFormEntry;
    struct ChrClassesEntry;
    struct ChrRacesEntry;
#if VERSION_STRING > TBC
    struct ScalingStatValuesEntry;
    struct ScalingStatDistributionEntry;
#endif
}

//struct InstancePlayerBind;
struct ItemSet;
class AchievementMgr;
class Mailbox;

namespace MySQLStructure
{
    struct VendorRestrictions;
}

struct VoidStorageItem;
class TradeData;
class ItemInterface;
struct ItemProperties;
struct Auction;
enum AchievementCriteriaTypes : uint8_t;
class ArenaTeam;
struct CharCreate;
class QuestLogEntry;
struct BGScore;
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
class Instance;
class InstanceSaved;
struct CharRaceEntry;
struct CharClassEntry;
struct Trainer;
class Aura;

struct OnHitSpell;
class CachedCharacterInfo;

typedef std::unordered_map<uint32_t, time_t> InstanceTimeMap;

//\todo: everything above this comment, does not belong in this file. Refactor this file to hold only the player class ;-)
// Everything below this line is bloated (seems we need some new concepts like RAII and a lot of refactoring to shrink it to a manageable class.
// Group all related members to a struct/class. Follow the "modern" way of C++ and leave the C way behind.
// 1. Initialize class members in the class
// 2. Use const wherever possible
// 3. move stuff out of this class
// 4. Check out the members (there are duplicats)
// 5. Get rid of legacy files (Player.Legacy.cpp) - done!?
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
    void AddToWorld(WorldMap* pMapMgr);                 // hides virtual function Object::AddToWorld
    // void PushToWorld(WorldMap*);                     // not used
    // void RemoveFromWorld(bool free_guid);            // not used
    void OnPrePushToWorld() override;                   // overrides virtual function  Object::OnPrePushToWorld
    void OnPushToWorld() override;                      // overrides virtual function  Object::OnPushToWorld
    // void OnPreRemoveFromWorld();                     // not used
    // void OnRemoveFromWorld();                        // not used

    void removeFromWorld();
    bool m_isReadyToBeRemoved = false;

private:
    const WoWPlayer* playerData() const { return reinterpret_cast<WoWPlayer*>(wow_data); }
public:
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

    //////////////////////////////////////////////////////////////////////////////////////////
    // bytes begin
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
    // bytes end
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // bytes2 begin
    uint32_t getPlayerBytes2() const;
    void setPlayerBytes2(uint32_t bytes2);

    uint8_t getFacialFeatures() const;
    void setFacialFeatures(uint8_t feature);

    uint8_t getBankSlots() const;
    void setBankSlots(uint8_t slots);

    uint8_t getRestState() const;
    void setRestState(uint8_t state);
    // bytes2 end
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // bytes3 begin
    uint32_t getPlayerBytes3() const;
    void setPlayerBytes3(uint32_t bytes3);

    //\note already available in unit data
    uint8_t getPlayerGender() const;
    void setPlayerGender(uint8_t gender);

    uint8_t getDrunkValue() const;
    void setDrunkValue(uint8_t value);

    uint8_t getPvpRank() const;
    void setPvpRank(uint8_t rank);

#if VERSION_STRING >= TBC
    uint8_t getArenaFaction() const;
    void setArenaFaction(uint8_t faction);
#endif
    // bytes3 end
    //////////////////////////////////////////////////////////////////////////////////////////

    uint32_t getDuelTeam() const;
    void setDuelTeam(uint32_t team);

    uint32_t getGuildTimestamp() const;
    void setGuildTimestamp(uint32_t timestamp);

    //////////////////////////////////////////////////////////////////////////////////////////
    // QuestLog start
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
    // QuestLog end
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // VisibleItem start
    uint32_t getVisibleItemEntry(uint32_t slot) const;
    void setVisibleItemEntry(uint32_t slot, uint32_t entry);
#if VERSION_STRING > TBC
    uint16_t getVisibleItemEnchantment(uint32_t slot, uint8_t pos) const;
    void setVisibleItemEnchantment(uint32_t slot, uint8_t pos, uint16_t enchantment);
#else
    uint32_t getVisibleItemEnchantment(uint32_t slot, uint8_t pos) const;
    void setVisibleItemEnchantment(uint32_t slot, uint8_t pos, uint32_t enchantment);
#endif
    // VisibleItem end
    //////////////////////////////////////////////////////////////////////////////////////////

    uint64_t getInventorySlotItemGuid(uint8_t slot) const;
    void setInventorySlotItemGuid(uint8_t slot, uint64_t guid);

    uint64_t getPackSlotItemGuid(uint8_t slot) const;
    void setPackSlotItemGuid(uint8_t slot, uint64_t guid);

    uint64_t getBankSlotItemGuid(uint8_t slot) const;
    void setBankSlotItemGuid(uint8_t slot, uint64_t guid);

    uint64_t getBankBagSlotItemGuid(uint8_t slot) const;
    void setBankBagSlotItemGuid(uint8_t slot, uint64_t guid);

    uint64_t getVendorBuybackSlot(uint8_t slot) const;
    void setVendorBuybackSlot(uint8_t slot, uint64_t guid);

#if VERSION_STRING < Cata
    uint64_t getKeyRingSlotItemGuid(uint8_t slot) const;
    void setKeyRingSlotItemGuid(uint8_t slot, uint64_t guid);
#endif

#if VERSION_STRING == TBC
    uint64_t getVanityPetSlotItemGuid(uint8_t slot) const;
    void setVanityPetSlotItemGuid(uint8_t slot, uint64_t guid);
#endif

#if VERSION_STRING == WotLK
    uint64_t getCurrencyTokenSlotItemGuid(uint8_t slot) const;
    void setCurrencyTokenSlotItemGuid(uint8_t slot, uint64_t guid);
#endif

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
    void addXP(uint32_t xp);

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

#if VERSION_STRING == TBC
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

    uint8_t getPlayerFieldBytesMiscFlag() const;
    void setPlayerFieldBytesMiscFlag(uint8_t miscFlag);
    void addPlayerFieldBytesMiscFlag(uint8_t miscFlag);
    void removePlayerFieldBytesMiscFlag(uint8_t miscFlag);

    uint8_t getEnabledActionBars() const;
    void setEnabledActionBars(uint8_t actionBarId);
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

    uint8_t getAuraVision() const;
    void setAuraVision(uint8_t auraVision);
    void addAuraVision(uint8_t auraVision);
    void removeAuraVision(uint8_t auraVision);
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
#if VERSION_STRING >= Cata
    void sendForceMovePacket(UnitSpeedType speed_type, float speed);
    void sendMoveSetSpeedPaket(UnitSpeedType speed_type, float speed);
#endif
    void resendSpeed();

    bool isMoving() const;

    uint32_t getMountSpellId() const;
    void setMountSpellId(uint32_t id);

    bool isOnVehicle() const;
    uint32_t getMountVehicleId() const;
    void setMountVehicleId(uint32_t id);

    void handleAuraInterruptForMovementFlags(MovementInfo const& movement_info);

    bool isInCity() const;

    void handleBreathing(MovementInfo const& movement_info, WorldSession* session);
    void initialiseNoseLevel();

    bool m_isWaterBreathingEnabled = false;
    uint32_t m_underwaterTime = 180000;
    uint32_t m_underwaterMaxTime = 180000;
    uint32_t m_underwaterState = 0;
    uint32_t m_underwaterLastDamage;

    void handleKnockback(Object* caster, float horizontal, float vertical) override;

    bool teleport(const LocationVector& vec, WorldMap* map);
    void eventTeleport(uint32_t mapId, LocationVector position, uint32_t instanceId = 0);

    bool safeTeleport(uint32_t mapId, uint32_t instanceId, const LocationVector& vec);

    //\Todo: this function is not as "safe" as the one above, reduce it to one function.
    void safeTeleport(WorldMap* mgr, const LocationVector& vec);

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

    uint32_t m_mountSpellId = 0;
    uint32_t m_mountVehicleId = 0;

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

    bool hasAreaExplored(WDB::Structures::AreaTableEntry const*);
    bool hasOverlayUncovered(uint32_t overlayId);
    void eventExploration();

    uint32_t m_explorationTimer;

    void ejectFromInstance();
    bool exitInstance();
private:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Basic
public:
    bool create(CharCreate& charCreateContent);

    WDB::Structures::ChrRacesEntry const* getDbcRaceEntry();
    WDB::Structures::ChrClassesEntry const* getDbcClassEntry();

    utf8_string getName() const;
    void setName(std::string name);

    uint32_t getLoginFlag() const;
    void setLoginFlag(uint32_t flag);

    void setInitialDisplayIds(uint8_t gender, uint8_t race);

    void applyLevelInfo(uint32_t newLevel);

    virtual bool isClassMage() const;
    virtual bool isClassDeathKnight() const;
    virtual bool isClassPriest() const;
    virtual bool isClassRogue() const;
    virtual bool isClassShaman() const;
    virtual bool isClassHunter() const;
    virtual bool isClassWarlock() const;
    virtual bool isClassWarrior() const;
    virtual bool isClassPaladin() const;
    virtual bool isClassMonk() const;
    virtual bool isClassDruid() const;

    PlayerTeam getTeam() const;
    PlayerTeam getBgTeam() const;
    void setTeam(uint32_t team);
    void setBgTeam(uint32_t team);

    uint32_t getInitialTeam() const;

    void resetTeam();
    bool isTeamHorde() const;
    bool isTeamAlliance() const;

    // Returns unit charmer
    Unit* getUnitOwner() override;
    // Returns unit charmer
    Unit const* getUnitOwner() const override;
    // Returns unit charmer or self
    Unit* getUnitOwnerOrSelf() override;
    // Returns unit charmer or self
    Unit const* getUnitOwnerOrSelf() const override;
    // Returns player charmer
    Player* getPlayerOwner() override;
    // Returns player charmer
    Player const* getPlayerOwner() const override;
    // Returns player charmer or self
    Player* getPlayerOwnerOrSelf() override;
    // Returns player charmer or self
    Player const* getPlayerOwnerOrSelf() const override;

    void toggleAfk();
    void toggleDnd();

    uint32_t* getPlayedTime();

    CachedCharacterInfo* getPlayerInfo() const;

    static void changeLooks(uint64_t guid, uint8_t gender, uint8_t skin, uint8_t face, uint8_t hairStyle, uint8_t hairColor, uint8_t facialHair);
    static void changeLanguage(uint64_t guid, uint8_t race);

    void sendInitialLogonPackets();

private:
    LevelInfo const* m_levelInfo = nullptr;

    WDB::Structures::ChrRacesEntry const* m_dbcRace = nullptr;
    WDB::Structures::ChrClassesEntry const* m_dbcClass = nullptr;

    uint32_t m_loadHealth = 0;
    uint32_t m_loadMana = 0;

    uint32_t m_classicMaxLevel = 60;

    utf8_string m_name;

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

    void outPacket(uint16_t opcode, uint16_t length, const void* data) override;
    void sendPacket(WorldPacket* packet) override;
    void outPacketToSet(uint16_t opcode, uint16_t length, const void* data, bool sendToSelf) override;
    void sendMessageToSet(WorldPacket* data, bool sendToSelf, bool sendToOwnTeam = false) override;

    void sendDelayedPacket(WorldPacket* data, bool deleteDataOnSend);

    void processPendingUpdates();
    bool compressAndSendUpdateBuffer(uint32_t size, const uint8_t* update_buffer);
    uint32_t buildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target) override;

    static void initVisibleUpdateBits();
    static UpdateMask m_visibleUpdateMask;

    void copyAndSendDelayedPacket(WorldPacket* data);

    void setEnteringToWorld();

    Creature* getCreatureWhenICanInteract(WoWGuid const& guid, uint32_t npcflagmask);

    UpdateManager& getUpdateMgr();

private:
    UpdateManager m_updateMgr;

    bool m_enteringWorld = false;

protected:
    WorldSession* m_session = nullptr;

    void setCreateBits(UpdateMask* updateMask, Player* target) const;
    void setUpdateBits(UpdateMask* updateMask, Player* target) const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Visiblility
public:
    void addVisibleObject(uint64_t guid);
    void removeVisibleObject(uint64_t guid);
    bool isVisibleObject(uint64_t guid);

    void removeIfVisiblePushOutOfRange(uint64_t guid);

protected:
    std::set<uint64_t> m_visibleObjects;

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
    void updateManaRegeneration(bool initialUpdate = false);
    void updateRageRegeneration(bool initialUpdate = false);
#if VERSION_STRING >= WotLK
    void updateRunicPowerRegeneration(bool initialUpdate = false);
#endif
    // Returns health regen value per 2 sec
    float_t calculateHealthRegenerationValue(bool inCombat) const;

private:
    // Regenerate timers
#if VERSION_STRING >= Cata
    uint16_t m_holyPowerRegenerateTimer = 0;
#endif

    // This timer ticks even if the player is not eating or drinking
    uint16_t m_foodDrinkSpellVisualTimer = 5000;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Database stuff
public:
    bool loadSpells(QueryResult* result);
    bool loadSkills(QueryResult* result);
    bool loadReputations(QueryResult* result);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spells and skills
    bool hasSpell(uint32_t spellId) const;
    bool hasDeletedSpell(uint32_t spellId) const;
    void addSpell(uint32_t spellId, uint16_t fromSkill = 0);
    void addDeletedSpell(uint32_t spellId);
    bool removeSpell(uint32_t spellId, bool moveToDeleted);
    bool removeDeletedSpell(uint32_t spellId);
    SpellSet const& getSpellSet() const;
    SpellSet const& getDeletedSpellSet() const;

    void sendSmsgInitialSpells();
    void sendPreventSchoolCast(uint32_t spellSchool, uint32_t timeMs);

    void resetSpells();

    void addShapeShiftSpell(uint32_t spellId);
    void removeShapeShiftSpell(uint32_t spellId);
    SpellSet const& getShapeshiftSpells() const;

    void sendAvailSpells(WDB::Structures::SpellShapeshiftFormEntry const* shapeshiftFormEntry, bool active);

    bool isInFeralForm();
    bool isInDisallowedMountForm() const;

    // Spells variables
    SpellOverrideMap m_spellOverrideMap;

    void updateAutoRepeatSpell();
    bool canUseFlyingMountHere();

    bool canDualWield2H() const;
    void setDualWield2H(bool enable);

    bool isSpellFitByClassAndRace(uint32_t spell_id) const;

    uint32_t getHealthFromSpell() { return m_healthFromSpell; }
    void setHealthFromSpell(uint32_t value) { m_healthFromSpell = value; }

    uint32_t getManaFromSpell() { return m_manaFromSpell; }
    void setManaFromSpell(uint32_t value) { m_manaFromSpell = value; }

    void calcResistance(uint8_t type);
    float getResistMCrit() { return m_resistCritical[0]; }
    void setResistMCrit(float newvalue) { m_resistCritical[0] = newvalue; }

    float getResistRCrit() { return m_resistCritical[1]; }
    void setResistRCrit(float newvalue) { m_resistCritical[1] = newvalue; }

    float m_resistCritical[2] = { 0 }; // when we are a victim we can have talents to decrease chance for critical hit. This is a negative value and it's added to critchances

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

    void setLastPotion(uint32_t itemId) { m_lastPotionId = itemId; }
    void cooldownAddItem(ItemProperties const* itemProp, uint32_t spellIndex);
    bool cooldownCanCast(ItemProperties const* itemProp, uint32_t spellIndex);
    void updatePotionCooldown();
    bool hasSpellWithAuraNameAndBasePoints(uint32_t auraName, uint32_t basePoints);

protected:
    void _addCategoryCooldown(uint32_t categoryId, uint32_t time, uint32_t SpellId, uint32_t ItemId);
    void _addCooldown(uint32_t type, uint32_t mis, uint32_t time, uint32_t SpellId, uint32_t ItemId);
    void _loadPlayerCooldowns(QueryResult* result);
    void _savePlayerCooldowns(QueryBuffer* buf);

    uint32_t m_lastPotionId = 0;
    PlayerCooldownMap m_cooldownMap[NUM_COOLDOWN_TYPES];
    uint32_t m_globalCooldown = 0;

public:
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
    float getSkillUpChance(uint16_t id);
#if VERSION_STRING >= Cata
    void setInitialPlayerProfessions();
#endif

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
    // learningPreviousRanks and ignorePreviousRanks are for internal use only
    void _addSpell(uint32_t spellId, uint16_t fromSkill = 0, bool learningPreviousRanks = false, bool ignorePreviousRanks = false);
    // removingPreviousRanks and forceRemoveHigherRanks are for internal use only
    bool _removeSpell(uint32_t spellId, bool moveToDeleted, bool silently = false, bool removingPreviousRank = false, bool forceRemoveHigherRanks = false);
    SpellSet m_spellSet;
    SpellSet m_deletedSpellSet;
    SpellSet m_shapeshiftSpells;

    bool m_canDualWield2H = false;

    // Skills
    void _verifySkillValues(WDB::Structures::SkillLineEntry const* skillEntry, uint16_t* currentValue, uint16_t* maxValue, uint16_t* skillStep, bool* requireUpdate);
    void _verifySkillValues(WDB::Structures::SkillLineEntry const* skillEntry, uint16_t* currentValue, uint16_t* maxValue, uint16_t* skillStep);
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
    // Action param
    // 0: Normal action, sends bars
    // 1: Sent after changing talent spec, sends bars and client will check if spell is known
    // 2: Sent before changing talent spec, clears bars clientside
    void sendActionBars(uint8_t action);

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
    std::unique_ptr<TradeData> m_TradeData;

    std::mutex m_tradeMutex;

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

    void setAFKReason(std::string reason);
    std::string getAFKReason() const;

    void addToGMTargetList(uint32_t guid);
    void removeFromGMTargetList(uint32_t guid);
    bool isOnGMTargetList(uint32_t guid) const;

    PlayerCheat m_cheats = {false};
    float m_goLastXRotation = 0.0f;
    float m_goLastYRotation = 0.0f;

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

    std::string afkReason;

    std::vector<uint32_t> m_gmPlayerTargetList;
    mutable std::mutex m_lockGMTargetList;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Items
public:
    void unEquipOffHandIfRequired();
    bool hasOffHandWeapon() const;

    bool hasItem(uint32_t itemId, uint32_t amount = 1, bool checkBankAlso = false) const;

#if VERSION_STRING > TBC
    void calculateHeirloomBonus(ItemProperties const* proto, int16_t slot, bool apply);
    WDB::Structures::ScalingStatDistributionEntry const* getScalingStatDistributionFor(ItemProperties const& itemProto) const;
    WDB::Structures::ScalingStatValuesEntry const* getScalingStatValuesFor(ItemProperties const& itemProto) const;
#endif

    // Player's item storage
    ItemInterface* getItemInterface() const;

    void removeTempItemEnchantsOnArena();

    void addGarbageItem(std::unique_ptr<Item> item);

    void applyItemMods(Item* item, int16_t slot, bool apply, bool justBrokedown = false, bool skipStatApply = false);

private:
    std::unique_ptr<ItemInterface> m_itemInterface;

    void removeGarbageItems();
    std::list<std::unique_ptr<Item>> m_GarbageItems;

protected:
    std::list<ItemSet> m_itemSets;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Difficulty
public:
    InstanceDifficulty::Difficulties getDifficulty(bool isRaid) const { return isRaid ? InstanceDifficulty::Difficulties(m_raidDifficulty) : InstanceDifficulty::Difficulties(m_dungeonDifficulty); }

    void setDungeonDifficulty(uint8_t diff);
    uint8_t getDungeonDifficulty();

    void setRaidDifficulty(uint8_t diff);
    uint8_t getRaidDifficulty();

private:
    uint8_t m_dungeonDifficulty = 0;
    uint8_t m_raidDifficulty = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Die, Corpse & Repop
public:
    void die(Unit* unitAttacker, uint32_t damage, uint32_t spellId) override;

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
    Charter const* getCharter(uint8_t charterType);

    bool canSignCharter(Charter const* charter, Player* requester);
    void initialiseCharters();

private:
    std::array<Charter*, NUM_CHARTER_TYPES> m_charters = { nullptr };

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

    uint32_t m_groupUpdateFlags;

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
    std::array<ArenaTeam*, NUM_ARENA_TEAM_TYPES> m_arenaTeams = { nullptr };
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
    uint32_t m_pvpTimer = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Battleground
public:
    Battleground* getBattleground() const;
    void setBattleground(Battleground* bg);

    Battleground* getPendingBattleground() const;
    void setPendingBattleground(Battleground* bg);

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

    uint32_t getLevelGrouping();

    BGScore m_bgScore;

private:
    Battleground* m_bg = nullptr;
    Battleground* m_pendingBattleground = nullptr;

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

    QuestLogEntry* createQuestLogInSlot(QuestProperties const* questProperties, uint8_t slotId);

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
    bool hasQuestFinished(uint32_t questId) const;

    void areaExploredQuestEvent(uint32_t questId);

    void clearQuest(uint32_t questId);

    bool hasQuestForItem(uint32_t itemId) const;

    void addQuestSpell(uint32_t spellId);
    bool hasQuestSpell(uint32_t spellId);
    void removeQuestSpell(uint32_t spellId);

    void addQuestMob(uint32_t entry);
    bool hasQuestMob(uint32_t entry);
    void removeQuestMob(uint32_t entry);

    void addQuestKill(uint32_t questId, uint8_t reqId, uint32_t delay = 0);

    void updateNearbyQuestGameObjects();

    std::set<uint32_t> getFinishedQuests() const;

private:
    std::array<std::unique_ptr<QuestLogEntry>, MAX_QUEST_LOG_SIZE> m_questlog;

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
    std::unique_ptr<SpeedCheatDetector> m_speedCheatDetector;

    //Speed
    //Fly
    //Teleport
    //NoClip
    //Waterwalk
    //Size
    //Wallclimb
    //Itemstacking (spell/attack power stacking)

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
    std::array<std::unique_ptr<VoidStorageItem>, VOID_STORAGE_MAX_SLOT> _voidStorageItems;
#endif

    /////////////////////////////////////////////////////////////////////////////////////////
    // Taxi
public:
    bool activateTaxiPathTo(std::vector<uint32_t> const& nodes, Creature* npc = nullptr, uint32_t spellid = 0);
    bool activateTaxiPathTo(uint32_t taxi_path_id, uint32_t spellid = 0);
    bool activateTaxiPathTo(uint32_t taxi_path_id, Creature* npc);
    void cleanupAfterTaxiFlight();
    void continueTaxiFlight() const;
    void sendTaxiNodeStatusMultiple();

    bool isInFlight() const;
    bool isOnTaxi() const;

    void initTaxiNodesForLevel();

    TaxiPath* getTaxiData() const { return m_taxi.get(); }

private:
    std::unique_ptr<TaxiPath> m_taxi;

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

    void applyForcedReaction(uint32_t faction_id, Standing rank, bool apply);
    Standing const* getForcedReputationRank(WDB::Structures::FactionTemplateEntry const* factionTemplateEntry) const;

    void setFactionAtWar(uint32_t faction, bool set);
    bool isFactionAtWar(WDB::Structures::FactionEntry const* factionEntry) const;

    bool isHostileBasedOnReputation(WDB::Structures::FactionEntry const* factionEntry);
    void updateInrangeSetsBasedOnReputation();

    void onKillUnitReputation(Unit* unit, bool innerLoop);
    void onTalkReputation(WDB::Structures::FactionEntry const* factionEntry);
    
    void setFactionInactive(uint32_t faction, bool set);
    bool addNewFaction(WDB::Structures::FactionEntry const* factionEntry, int32_t standing, bool base);
    void onModStanding(WDB::Structures::FactionEntry const* factionEntry, FactionReputation* reputation);
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
    void setServersideDrunkValue(uint16_t newDrunkValue, uint32_t itemId = 0);
    static PlayerBytes3_DrunkValue getDrunkStateByValue(uint16_t value);
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
    void giveXp(uint32_t xp, const uint64_t& guid, bool allowBonus);
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
    PetCache const* getPetCache(uint8_t petId) const;
    PetCache* getModifiablePetCache(uint8_t petId) const;
    PetCacheMap const& getPetCacheMap() const;
    // <Slot, pet id>
    std::map<uint8_t, uint8_t> const& getPetCachedSlotMap() const;
    void addPetCache(std::unique_ptr<PetCache> petCache, uint8_t index);
    void removePetCache(uint8_t petId);
    uint8_t getPetCount() const;

    uint8_t getFreePetNumber();
    std::optional<uint8_t> getPetIdFromSlot(uint8_t slot) const;
    bool hasPetInSlot(uint8_t slot) const;
    std::optional<uint8_t> findFreeActivePetSlot() const;
    std::optional<uint8_t> findFreeStablePetSlot() const;

    bool tryPutPetToSlot(uint8_t petId, uint8_t newSlot, bool sendErrors = true);

    // Summons existing pet from PetCache map
    // Pet must be in active slot
    void spawnPet(uint8_t petId);
    // Summons temporarily unsummoned pet if one exists
    void summonTemporarilyUnsummonedPet();
    // Unsummons current pet and saves it id for quick re-summon
    // Used i.e. when entering vehicle, mounting or using taxi
    void unSummonPetTemporarily();
    bool isPetRequiringTemporaryUnsummon() const;
    void setTemporarilyUnsummonedPetsOffline();

    void setLastBattlegroundPetId(uint8_t petId);
    uint8_t getLastBattlegroundPetId() const;
    void setLastBattlegroundPetSpell(uint32_t petSpell);
    uint32_t getLastBattlegroundPetSpell() const;

    void setStableSlotCount(uint8_t count);
    uint8_t getStableSlotCount() const;

    void eventSummonPet(Pet* summonPet);
    void eventDismissPet();

    Object* getSummonedObject() const;
    void setSummonedObject(Object* summonedObject);

private:
    void _spawnPet(PetCache const* petCache);

    PetCacheMap m_cachedPets;
    // <Slot, pet id>
    std::map<uint8_t, uint8_t> m_cachedPetSlots;

    uint8_t m_battlegroundLastPetId = 0;
    uint32_t m_battlegroundLastPetSpell = 0;

    uint8_t m_stableSlotCount = 0;
    uint8_t m_maxPetNumber = 0;

    Object* m_summonedObject = nullptr;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
public:
    bool isGMFlagSet() const;

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
    void sendResetFailedNotify(uint32_t mapid);
    void sendInstanceDifficultyPacket(uint8_t difficulty);
    void sendNewDrunkStatePacket(uint32_t state, uint32_t itemId);
    void sendSetProficiencyPacket(uint8_t itemClass, uint32_t proficiency);
    void sendPartyKillLogPacket(uint64_t killedGuid);
    void sendDestroyObjectPacket(uint64_t destroyedGuid);
    void sendEquipmentSetUseResultPacket(uint8_t result);
    void sendTotemCreatedPacket(uint8_t slot, uint64_t guid, uint32_t duration, uint32_t spellId);
    void sendPetTameFailure(uint8_t result) const;

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

    void sendEquipmentSetList();
    void sendEquipmentSetSaved(uint32_t setId, uint32_t setGuid);
    void sendEmptyPetSpellList();
    void sendInitialWorldstates();

    bool isPvpFlagSet() const override;
    void setPvpFlag() override;
    void removePvpFlag() override;

    bool isFfaPvpFlagSet() const override;
    void setFfaPvpFlag() override;
    void removeFfaPvpFlag() override;

    bool isSanctuaryFlagSet() const override;
    void setSanctuaryFlag() override;
    void removeSanctuaryFlag() override;

    void sendPvpCredit(uint32_t honor, uint64_t victimGuid, uint32_t victimRank);
    void sendRaidGroupOnly(uint32_t timeInMs, uint32_t type);

    void setVisibleItemFields(uint32_t slot, Item* item);
#if VERSION_STRING >= Cata
    void applyReforgeEnchantment(Item* item, bool apply);
#endif

    bool isAtGroupRewardDistance(Object* pRewardSource);

    void tagUnit(Object* object);

#if VERSION_STRING > TBC
    void updateAchievementCriteria(AchievementCriteriaTypes type, int32_t miscValue1 = 0, int32_t miscValue2 = 0, uint32_t miscValue3 = 0, Unit* unit = nullptr);
    AchievementMgr* getAchievementMgr();
#endif

    void sendUpdateDataToSet(ByteBuffer* groupBuf, ByteBuffer* nonGroupBuf, bool sendToSelf);
    void sendWorldStateUpdate(uint32_t worldState, uint32_t value);

    bool canBuyAt(MySQLStructure::VendorRestrictions const* vendor);
    bool canTrainAt(Trainer const* trainer);

    void sendCinematicCamera(uint32_t id);

    void setMover(Unit* target);

    void resetTimeSync();
    void sendTimeSync();

private:
    uint16_t m_spellAreaUpdateTimer = 1000;
    uint16_t m_pendingPacketTimer = 100;
    uint16_t m_partyUpdateTimer = 1000;
    uint32_t m_itemUpdateTimer = 0;

#if VERSION_STRING > TBC
    std::unique_ptr<AchievementMgr> m_achievementMgr;
#endif

    uint32_t m_timeSyncCounter = 0;
    uint32_t m_timeSyncTimer = 0;
    uint32_t m_timeSyncClient = 0;
    uint32_t m_timeSyncServer = 0;

public:
    void buildFlagUpdateForNonGroupSet(uint32_t index, uint32_t flag);

    void modifyBonuses(uint32_t type, int32_t val, bool apply);
    void calcExpertise();
    std::map<uint32_t, uint32_t> m_wratings;

    /////////////////////////////////////////////////////////////////////////////////////////
    // Spells
    void calculateDamage() override;
    float m_offhandDmgMod = 0.5f;

    int32_t GetDamageDoneMod(uint16_t school) override
    {
        if (school >= TOTAL_SPELL_SCHOOLS)
            return 0;

        return static_cast<int32_t>(getModDamageDonePositive(school)) - static_cast<int32_t>(getModDamageDoneNegative(school));
    }

    float GetDamageDonePctMod(uint16_t school) override
    {
        if (school >= TOTAL_SPELL_SCHOOLS)
            return 0;

        return getModDamageDonePct(static_cast<uint8_t>(school));
    }

    uint32_t getMainMeleeDamage(uint32_t attackPowerOverride); // I need this for windfury

    // Talents
    void setTalentHearthOfWildPCT(int value) { m_hearthOfWildPct = value; }
    void eventTalentHearthOfWildChange(bool apply);

protected:
    int m_hearthOfWildPct = 0;        // druid hearth of wild talent used on shapeshifting. We either know what is last talent level or memo on learn

    std::list<LoginAura> m_loginAuras;

    /////////////////////////////////////////////////////////////////////////////////////////
    // Player loading and savings Serialize character to db
public:
    void saveToDB(bool newCharacter);
    void saveAuras(std::stringstream&);
    bool loadFromDB(uint32_t guid);
    void loadFromDBProc(QueryResultVector& results);

    bool saveSpells(bool newCharacter, QueryBuffer* buf);

    bool loadDeletedSpells(QueryResult* result);
    bool saveDeletedSpells(bool newCharacter, QueryBuffer* buf);

    bool saveReputations(bool newCharacter, QueryBuffer* buf);
    bool saveSkills(bool newCharacter, QueryBuffer* buf);

    bool m_firstLogin = false;

    /////////////////////////////////////////////////////////////////////////////////////////
    // Talent Specs
    uint16_t m_maxTalentPoints = 0;
    uint8_t m_talentSpecsCount = 1;
    uint8_t m_talentActiveSpec = 0;
#if VERSION_STRING >= Cata
    uint32_t m_FirstTalentTreeLock = 0;
#endif

#ifdef FT_DUAL_SPEC
    PlayerSpec m_specs[MAX_SPEC_COUNT];
#else
    PlayerSpec m_spec;
#endif

    PlayerSpec& getActiveSpec();

#ifdef FT_GLYPHS
    uint16_t getGlyph(uint8_t spec, uint16_t slot) const { return m_specs[spec].getGlyph(slot); }
#endif

    /////////////////////////////////////////////////////////////////////////////////////////
    // Attack stuff
public:
    void eventAttackStart();
    void eventAttackStop();

    void eventDeath();

    /////////////////////////////////////////////////////////////////////////////////////////
    // Inrange
    void addToInRangeObjects(Object* object) override;
    void onRemoveInRangeObject(Object* object) override;
    void clearInRangeSets() override;

    /////////////////////////////////////////////////////////////////////////////////////////
    // PVP Stuff
    float m_spellHasteRatingBonus = 1.0f;
    void updateAttackSpeed();

#if VERSION_STRING >= TBC // support classic
    float getDefenseChance(uint32_t opLevel);
    float getDodgeChance();
    float getBlockChance();
    float getParryChance();
    void updateChances();
#endif
    void updateStats();
    uint32_t getBlockDamageReduction();
    void applyFeralAttackPower(bool apply, Item* item = NULL);

    float getSpellCritFromSpell() { return m_spellCritFromSpell; }
    float getHitFromSpell() { return m_hitFromSpell; }
    void setSpellCritFromSpell(float value) { m_spellCritFromSpell = value; }
    void setHitFromSpell(float value) { m_hitFromSpell = value; }

    uint32_t m_nextSave;

    int m_lifeTapBonus = 0;             // warlock spell related
    bool m_requiresNoAmmo = false;      // warlock spell related

    // Misc
    void eventCannibalize(uint32_t amount);

    uint32_t m_modBlockAbsorbValue = 0;
    uint32_t m_modBlockValueFromSpells = 0;

    uint32_t m_flatResistanceModifierPos[TOTAL_SPELL_SCHOOLS] = { 0 };
    uint32_t m_flatResistanceModifierNeg[TOTAL_SPELL_SCHOOLS] = { 0 };
    uint32_t m_baseResistanceModPctPos[TOTAL_SPELL_SCHOOLS] = { 0 };
    uint32_t m_baseResistanceModPctNeg[TOTAL_SPELL_SCHOOLS] = { 0 };
    uint32_t m_resistanceModPctPos[TOTAL_SPELL_SCHOOLS] = { 0 };
    uint32_t m_resistanceModPctNeg[TOTAL_SPELL_SCHOOLS] = { 0 };
    float m_resistHit[2] = { 0 };                           // 0 = melee; 1= ranged;
    int32_t m_resistHitSpell[TOTAL_SPELL_SCHOOLS] = { 0 };  // spell resist per school
    uint32_t m_modPhysCritDmgPct = 0;
    uint32_t m_rootedCritChanceBonus = 0;                   // Class Script Override: Shatter
    uint32_t m_increaseDmgSnaredSlowed = 0;

    // SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
    uint32_t m_modInterrManaRegenPct = 0;
    // SPELL_AURA_REGEN_MANA_STAT_PCT
    int32_t m_modManaRegenFromStat[STAT_COUNT] = { 0 };
    float m_RegenManaOnSpellResist = 0.0f;
    uint32_t m_castedAmount[TOTAL_SPELL_SCHOOLS] = { 0 };   // Last casted spells amounts. Need for some spells. Like Ignite etc. DOesn't count HoTs and DoTs. Only directs

    uint32_t m_flatStatModPos[5] = { 0 };
    uint32_t m_flatStatModNeg[5] = { 0 };
    uint32_t m_statModPctPos[5] = { 0 };
    uint32_t m_statModPctNeg[5] = { 0 };
    uint32_t m_totalStatModPctPos[5] = { 0 };
    uint32_t m_totalStatModPctNeg[5] = { 0 };
    int32_t m_increaseDamageByType[12] = { 0 };             // mod dmg by creature type
    float m_increaseDamageByTypePct[12] = { 0 };
    float m_increaseCricticalByTypePct[12] = { 0 };
    int32_t m_detectedRange = 0;
    uint32_t m_retaineDrage = 0;                            // Warrior spell related

    void calcStat(uint8_t t);
    float calcRating(PlayerCombatRating t);
    void regenerateHealth(bool inCombat);

    uint64_t m_misdirectionTarget = 0;                      // Hunter spell related

    uint64_t getMisdirectionTarget() { return m_misdirectionTarget; }
    void setMisdirectionTarget(uint64_t PlayerGUID) { m_misdirectionTarget = PlayerGUID; }

    bool m_reincarnation = false;                           // Shaman spell related

    std::map<uint32_t, WeaponModifier> m_damageDone;
    std::map<uint32_t, WeaponModifier> m_toCritChance;
    bool m_cannibalize = false;
    uint8_t m_cannibalizeCount = 0;
    int32_t m_rageFromDamageDealt = 0;
    int32_t m_rageFromDamageTaken = 0;

    void addItemsToWorld();
    void removeItemsFromWorld();
    void updateKnownCurrencies(uint32_t itemId, bool apply);

    uint32_t m_trackingSpell = 0;
    void eventCharmAttack();

    void clearCooldownsOnLine(uint32_t skillLine, uint32_t calledFrom);

    //bool m_Autojoin = false;
    //bool m_AutoAddMem = false;
    void sendMirrorTimer(MirrorTimerTypes mirrorType, uint32_t max, uint32_t current, int32_t regen);

    void updateChanceFields();

    uint32_t m_lastSeenWeather = 0;
    std::set<Object*> m_visibleFarsightObjects;

    // PVP/BG
    uint32_t getMaxPersonalRating();

    // Instance IDs
    typedef std::unordered_map<uint32_t /*mapId*/, InstancePlayerBind> BoundInstancesMap;
    void loadBoundInstances();

    // permanent binds and solo binds by difficulty
    BoundInstancesMap m_boundInstances[InstanceDifficulty::MAX_DIFFICULTY];
    InstancePlayerBind* getBoundInstance(uint32_t mapId, InstanceDifficulty::Difficulties difficulty, bool withExpired = false);
    BoundInstancesMap& getBoundInstances(InstanceDifficulty::Difficulties difficulty) { return m_boundInstances[difficulty]; }
    InstanceSaved* getInstanceSave(uint32_t mapId, bool isRaid);

    void unbindInstance(uint32_t mapid, InstanceDifficulty::Difficulties difficulty, bool unload = false);
    void unbindInstance(BoundInstancesMap::iterator& itr, InstanceDifficulty::Difficulties difficulty, bool unload = false);

    InstancePlayerBind* bindToInstance(InstanceSaved* save, bool permanent, BindExtensionState extendState = EXTEND_STATE_NORMAL, bool load = false);
    void bindToInstance();

    void setPendingBind(uint32_t instanceId, uint32_t bindTimer);
    bool hasPendingBind() const { return m_pendingBindId > 0; }

    void sendRaidInfo();
    void sendSavedInstances();

    void resetInstances(uint8_t method, bool isRaid);

    void sendResetInstanceFailed(uint32_t reason, uint32_t MapId);

    void sendInstanceResetWarning(uint32_t mapid, InstanceDifficulty::Difficulties difficulty, uint32_t time, bool welcome);

    void loadInstanceTimeRestrictions();
    bool checkInstanceCount(uint32_t instanceId) const;
    void addInstanceEnterTime(uint32_t instanceId, time_t enterTime);
    void saveInstanceTimeRestrictions();
    InstanceTimeMap m_instanceResetTimes;

private:
    uint32_t m_pendingBindId = 0;
    uint32_t m_pendingBindTimer = 0;

public:
    //movement/position
    void _Relocate(uint32_t mapid, const LocationVector& v, bool sendpending, bool force_new_world, uint32_t instance_id);

    std::map<uint32_t, std::set<uint32_t> > m_summonSpells;
    std::map<uint32_t, std::map<SpellInfo const*, uint16_t>*> m_petSpells;

    void addSummonSpell(uint32_t entry, uint32_t spellId);
    void removeSummonSpell(uint32_t entry, uint32_t spellId);
    std::set<uint32_t>* getSummonSpells(uint32_t spellId);

    void handleSpellLoot(uint32_t itemId);

    void completeLoading();

    bool m_blinked = false;
    bool m_beingPushed = false;

    uint32_t m_flyingAura = 0;

    bool m_changingMaps = true;
    bool m_resendSpeed = false; // set to true if m_changingMaps is true.

    int32_t m_rapModPct = 0;

    bool m_deathVision = false;
    // paladin related
    SpellInfo const* m_lastHealSpell = nullptr;

    std::unique_ptr<Mailbox> m_mailBox;
    bool m_finishingMovesDodge = false;

    bool isAttacking() { return m_attacking; }

protected:
    void _saveQuestLogEntry(QueryBuffer* buf);
    void _loadQuestLogEntry(QueryResult* result);

    void _loadPet(QueryResult* result);
    void _loadPetSpells(QueryResult* result);
    void _savePet(QueryBuffer* buf, bool updateCurrentPetCache = false, Pet* currentPet = nullptr);
    void _savePetSpells(QueryBuffer* buf);

    void _eventAttack(bool offhand);

    void _castSpellArea();

    /////////////////////////////////////////////////////////////////////////////////////////
    // Player Class systems, info and misc things
    uint32_t m_AttackMsgTimer = 0; // "too far away" and "wrong facing" timer
    bool m_attacking = false;

    // combat mods
    float m_blockFromSpellPct = 0.0f;
    float m_critFromSpell = 0.0f;
    float m_spellCritFromSpell = 0.0f;
    float m_hitFromSpell = 0.0f;
    // stats mods
    uint32_t m_healthFromSpell = 0;
    uint32_t m_manaFromSpell = 0;
    uint32_t m_healthFromItems = 0;
    uint32_t m_manaFromItems = 0;

    // Raid
    uint8_t m_targetIcon = 0;

    uint32_t _fields[getSizeOfStructure(WoWPlayer)];

public:
    std::map<uint32_t, Standing> m_forcedReactions;

    bool m_passOnLoot = false;
    uint32_t m_tradeSequence;

    uint32_t m_outStealthDamageBonusPct = 0;
    uint32_t m_outStealthDamageBonusPeriod = 0;
    uint32_t m_outStealthDamageBonusTimer = 0;

    // helper for InstanceCommands
    void displayDataStateList();
    void displayTimerList();
    void displayCreatureSetForEntry(uint32_t _creatureEntry);

    uint32_t checkDamageLimits(uint32_t damage, uint32_t spellId);

    void loadFieldsFromString(const char* string, uint16_t firstField, uint32_t fieldsNum);

    // Avenging Wrath
    void avengingWrath() { m_avengingWrath = true; }
    bool m_avengingWrath = true;
};
