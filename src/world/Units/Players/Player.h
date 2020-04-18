/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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

#include "Units/Players/PlayerDefines.hpp"
#include "Units/Players/PlayerCache.h"
#include "Units/Stats.h"
#include "Server/Definitions.h"
#include "Management/QuestDefines.hpp"
#include "Management/Battleground/BattlegroundMgr.h"
#include "Management/MailMgr.h"
#include "Management/ItemPrototype.h"
#include "Management/AchievementMgr.h"
#include "Units/Unit.h"
#include "Storage/DBC/DBCStructures.hpp"
#include "Storage/MySQLStructures.h"
#include "Units/Creatures/AIInterface.h" //?? what?
#include "WorldConf.h"
#include "Management/AuctionHouse.h"
#include "Management/Guild.h"
#include "Management/ObjectUpdates/SplineManager.h"
#include "Management/ObjectUpdates/UpdateManager.h"

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
//struct VendorRestrictionEntry;
struct Trainer;
class Aura;

struct OnHitSpell;

struct CreateInfo_ItemStruct
{
    uint32_t protoid;
    uint8_t slot;
    uint32_t amount;
};

struct CreateInfo_SkillStruct
{
    uint32_t skillid;
    uint32_t currentval;
    uint32_t maxval;
};

// APGL End
// MIT Start
#pragma pack(push,1)
struct ActionButton
{
    uint32_t Action;
    uint8_t Type;
    uint8_t Misc;
};
#pragma pack(pop)

struct CreateInfo_ActionBarStruct
{
    uint8_t button;
    uint32_t action;
    uint8_t type;
    uint8_t misc;
};
// MIT End
// APGL Start

struct PlayerCreateInfo
{
    uint8_t index;
    uint8_t race;
    uint8_t class_;
    uint32_t mapId;
    uint32_t zoneId;
    float positionX;
    float positionY;
    float positionZ;
    float orientation;
    uint8_t strength;
    uint8_t ability;
    uint8_t stamina;
    uint8_t intellect;
    uint8_t spirit;
    uint32_t health;
    uint32_t mana;
    uint32_t rage;
    uint32_t focus;
    uint32_t energy;
    uint32_t attackpower;
    float mindmg;
    float maxdmg;
    uint32_t taximask[DBC_TAXI_MASK_SIZE];
    std::list<CreateInfo_ItemStruct> items;
    std::list<CreateInfo_SkillStruct> skills;
    std::list<CreateInfo_ActionBarStruct> actionbars;
    std::set<uint32_t> spell_list;
    //uint32_t item[10];
    //uint8_t item_slot[10];
    //uint16_t spell[10];
};

struct DamageSplit
{
    Player* caster;
    Aura* aura;
    uint32_t miscVal;
    union
    {
        uint32_t damage;
        float damagePCT;
    };
};

struct LoginAura
{
    uint32_t id;
    uint32_t dur;
    bool positive;
    uint32_t charges;
};

struct FactionReputation
{
    int32_t standing;
    uint8_t flag;
    int32_t baseStanding;
    int32_t CalcStanding() { return standing - baseStanding; }
    bool Positive() { return standing >= 0; }
};

typedef std::unordered_map<uint32_t, uint32_t> PlayerInstanceMap;
class SERVER_DECL PlayerInfo
{
    public:

        ~PlayerInfo();

        uint32_t guid;
        uint32_t acct;
        char* name;
        uint8_t race;
        uint8_t gender;
        uint8_t cl;
        uint32_t team;
        uint8_t role;

        time_t lastOnline;
        uint32_t lastZone;
        uint32_t lastLevel;
        Group* m_Group;
        int8_t subGroup;
        Mutex savedInstanceIdsLock;
        PlayerInstanceMap savedInstanceIds[NUM_INSTANCE_MODES];

        Player* m_loggedInPlayer;
        uint32_t m_guild;
        uint32_t guildRank;
};

struct PlayerPet
{
    std::string name;
    uint32_t entry;
    uint32_t xp;
    bool active;
    bool alive;
    char stablestate;
    uint32_t number;
    uint32_t level;
    uint32_t happinessupdate;
    std::string actionbar;
    time_t reset_time;
    uint32_t reset_cost;
    uint32_t spellid;
    uint32_t petstate;
    uint32_t talentpoints;
    uint32_t current_power;
    uint32_t current_hp;
    uint32_t current_happiness;
    uint32_t renamable;
    uint32_t type;
};

enum MeetingStoneQueueStatus
{
    MEETINGSTONE_STATUS_NONE                                = 0,
    MEETINGSTONE_STATUS_JOINED_MEETINGSTONE_QUEUE_FOR       = 1,
    MEETINGSTONE_STATUS_PARTY_MEMBER_LEFT_LFG               = 2,
    MEETINGSTONE_STATUS_PARTY_MEMBER_REMOVED_PARTY_REMOVED  = 3,
    MEETINGSTONE_STATUS_LOOKING_FOR_NEW_PARTY_IN_QUEUE      = 4,
    MEETINGSTONE_STATUS_NONE_UNK                            = 5
};

enum ItemPushResultTypes
{
    ITEM_PUSH_TYPE_LOOT     = 0x00000000,
    ITEM_PUSH_TYPE_RECEIVE  = 0x00000001,
    ITEM_PUSH_TYPE_CREATE   = 0x00000002
};

struct WeaponModifier
{
    uint32_t wclass;
    uint32_t subclass;
    float value;
};

struct PetActionBar
{
    uint32_t spell[10];
};

struct classScriptOverride
{
    uint32_t id;
    uint32_t effect;
    uint32_t aura;
    uint32_t damage;
    bool percent;
};

class AchievementMgr;
class Spell;
class Item;
class Container;
class WorldSession;
class ItemInterface;
class SpeedCheatDetector;
struct TaxiPathNode;

struct PlayerSkill
{
    DBC::Structures::SkillLineEntry const* Skill;
    uint32_t CurrentValue;
    uint32_t MaximumValue;
    uint32_t BonusValue;
    float GetSkillUpChance();
    void Reset(uint32_t Id);
};

enum SPELL_INDEX2
{
    SPELL_TYPE2_PALADIN_AURA        = 1,
    SPELL_TYPE3_DEATH_KNIGHT_AURA   = 1
};

class ArenaTeam;

struct PlayerCooldown
{
    uint32_t ExpireTime;
    uint32_t ItemId;
    uint32_t SpellId;
};

class PlayerSpec
{
    public:

        PlayerSpec()
        {
            tp = 0;
            for (uint8_t i = 0; i < PLAYER_ACTION_BUTTON_COUNT; i++)
            {
                mActions[i].Action = 0;
                mActions[i].Type = 0;
                mActions[i].Misc = 0;
            }
        }

        void SetTP(uint32_t points){ tp = points; }

        uint32_t GetTP() const{ return tp; }

        void Reset()
        {
            tp += static_cast<uint32_t>(talents.size());
            talents.clear();
        }

        void AddTalent(uint32_t talentid, uint8_t rankid);
        bool HasTalent(uint32_t talentid, uint8_t rankid)
        {
            std::map<uint32_t, uint8_t>::iterator itr = talents.find(talentid);
            if (itr != talents.end())
                return itr->second == rankid;

            return false;
        }

        std::map<uint32_t, uint8_t> talents;
#ifdef FT_GLYPHS
        uint16_t glyphs[GLYPHS_COUNT];
#endif
        ActionButton mActions[PLAYER_ACTION_BUTTON_COUNT];
    private:

        uint32_t tp;
};


typedef std::set<uint32_t>                                  SpellSet;
typedef std::list<classScriptOverride*>                     ScriptOverrideList;
typedef std::set<uint32_t>                                  SaveSet;
typedef std::map<uint32_t, ScriptOverrideList* >            SpellOverrideMap;
typedef std::map<uint32_t, uint32_t>                        SpellOverrideExtraAuraMap;
typedef std::map<uint32_t, FactionReputation*>              ReputationMap;
typedef std::map<uint32_t, uint64_t>                        SoloSpells;
typedef std::map<SpellInfo const*, std::pair<uint32_t,      uint32_t> >StrikeSpellMap;
typedef std::map<uint32_t, OnHitSpell >                     StrikeSpellDmgMap;
typedef std::map<uint32_t, PlayerSkill>                     SkillMap;
typedef std::set<Player**>                                  ReferenceSet;
typedef std::map<uint32_t, PlayerCooldown>                  PlayerCooldownMap;

// AGPL End

// MIT Start
class TradeData
{
private:
    Player* m_player;
    Player* m_tradeTarget;
    bool m_accepted;
    uint64_t m_money;
    uint32_t m_spell;
    uint64_t m_spellCastItem;
    uint64_t m_items[TRADE_SLOT_COUNT];

 public:
    TradeData(Player* player, Player* trader);

    Player* getTradeTarget() const;
    TradeData* getTargetTradeData() const;

    Item* getTradeItem(TradeSlots slot) const;
    bool hasTradeItem(uint64_t itemGuid) const;
    bool hasPlayerOrTraderItemInTrade(uint64_t itemGuid) const;

    uint32_t getSpell() const;
    Item* getSpellCastItem() const;
    bool hasSpellCastItem() const;

    uint64_t getTradeMoney() const;
    void setTradeMoney(uint64_t money);

    void setTradeAccepted(bool state, bool sendBoth = false);
    bool isTradeAccepted() const;

    void setTradeItem(TradeSlots slot, Item* item);
    void setTradeSpell(uint32_t spell_id, Item* cast_item = nullptr);
};

struct PlayerCheat
{
    bool TaxiCheat;
    bool CooldownCheat;
    bool CastTimeCheat;
    bool GodModeCheat;
    bool PowerCheat;
    bool FlyCheat;
    bool AuraStackCheat;
    bool ItemStackCheat;
    bool TriggerpassCheat;
};

struct WoWPlayer;
class SERVER_DECL Player : public Unit
{
    const WoWPlayer* playerData() const { return reinterpret_cast<WoWPlayer*>(wow_data); }
public:
    void resendSpeed();

private:
    UpdateManager m_updateMgr;
public:
    UpdateManager& getUpdateMgr();
private:
    SplineManager m_splineMgr;
public:
    SplineManager& getSplineMgr();

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

    //unk1

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

    uint16_t getDrunkValue() const;
    void setDrunkValue(uint16_t value);

    uint8_t getPvpRank() const;
    void setPvpRank(uint8_t rank);
    //bytes3 end

    uint32_t getDuelTeam() const;
    void setDuelTeam(uint32_t team);

    uint32_t getGuildTimestamp() const;
    void setGuildTimestamp(uint32_t timestamp);

    uint64_t getFarsightGuid() const;
    void setFarsightGuid(uint64_t farsightGuid);

#if VERSION_STRING > Classic
    uint32_t getChosenTitle() const;
    void setChosenTitle(uint32_t title);
#endif

    uint32_t getXp() const;
    void setXp(uint32_t xp);

    uint32_t getNextLevelXp() const;
    void setNextLevelXp(uint32_t xp);

    uint32_t getFreeTalentPoints() const;
#if VERSION_STRING < Cata
    void setFreeTalentPoints(uint32_t points);
#endif

    uint32_t getFreePrimaryProfessionPoints() const;
    void setFreePrimaryProfessionPoints(uint32_t points);

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

    // playerfieldbytes2 start
    uint32_t getPlayerFieldBytes2() const;
    void setPlayerFieldBytes2(uint32_t bytes);
    // playerfieldbytes2 end

    uint32_t getCombatRating(uint8_t combatRating) const;
    void setCombatRating(uint8_t combatRating, uint32_t value);
    void modCombatRating(uint8_t combatRating, int32_t value);

#if VERSION_STRING > Classic
#if VERSION_STRING < Cata
    uint32_t getArenaCurrency() const;
    void setArenaCurrency(uint32_t amount);
    void modArenaCurrency(int32_t value);
#endif
#endif

#if VERSION_STRING >= WotLK
    uint32_t getNoReagentCost(uint8_t index) const;
    void setNoReagentCost(uint8_t index, uint32_t value);

    uint32_t getGlyph(uint16_t slot) const;
    void setGlyph(uint16_t slot, uint32_t glyph);

    uint32_t getGlyphsEnabled() const;
    void setGlyphsEnabled(uint32_t glyphs);
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Movement
    void sendForceMovePacket(UnitSpeedType speed_type, float speed);
    void sendMoveSetSpeedPaket(UnitSpeedType speed_type, float speed);

    void handleFall(MovementInfo const& movement_info);

    bool isPlayerJumping(MovementInfo const& movement_info, uint16_t opcode);

    void handleBreathing(MovementInfo const& movement_info, WorldSession* session);
    void handleAuraInterruptForMovementFlags(MovementInfo const& movement_info);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Basic

private:

    //used for classic
    uint32_t max_level;

    std::string m_name;

    uint32_t m_team;
    uint32_t m_bgTeam;

public:

    std::string getName() const;
    void setName(std::string name);

    void setInitialDisplayIds(uint8_t gender, uint8_t race);

    void applyLevelInfo(uint32_t newLevel);

    bool isTransferPending() const;

    virtual bool isClassMage();
    virtual bool isClassDeathKnight();
    virtual bool isClassPriest();
    virtual bool isClassRogue();
    virtual bool isClassShaman();
    virtual bool isClassHunter();
    virtual bool isClassWarlock();
    virtual bool isClassWarrior();
    virtual bool isClassPaladin();
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

    //////////////////////////////////////////////////////////////////////////////////////////
    // Stats
    // Initializes stats and unit/playerdata fields
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

#if VERSION_STRING == Classic
    // Classic doesn't have these in unit or playerdata
    float m_manaRegeneration = 0.0f;
    float m_manaRegenerationWhileCasting = 0.0f;
#endif

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Database stuff
    bool loadSpells(QueryResult* result);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spells
    bool isSpellFitByClassAndRace(uint32_t spell_id);
    void updateAutoRepeatSpell();
    bool canUseFlyingMountHere();

    bool canDualWield2H() const;
    void setDualWield2H(bool enable);

    // Cooldowns
    bool hasSpellOnCooldown(SpellInfo const* spellInfo);
    bool hasSpellGlobalCooldown(SpellInfo const* spellInfo);
    // Do NOT add cooldownTime if you don't know what you're doing (it's required for spells with dynamic cooldown)
    void addSpellCooldown(SpellInfo const* spellInfo, Item const* itemCaster, int32_t cooldownTime = 0);
    void addGlobalCooldown(SpellInfo const* spellInfo, const bool sendPacket = false);
    void sendSpellCooldownPacket(SpellInfo const* spellInfo, const uint32_t duration, const bool isGcd);

    bool m_FirstCastAutoRepeat;

private:
    bool m_canDualWield2H;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Talents
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

private:
    uint32_t m_talentPointsFromQuests;

public:
    /////////////////////////////////////////////////////////////////////////////////////////
    // Actionbar
    void setActionButton(uint8_t button, uint32_t action, uint8_t type, uint8_t misc);
    void sendActionBars(bool clearBars);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Auction
    void sendAuctionCommandResult(Auction* auction, uint32_t Action, uint32_t ErrorCode, uint32_t bidError = 0);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Trade
private:
    TradeData* m_TradeData;

public:
    Player* getTradeTarget() const;
    TradeData* getTradeData() const;
    void cancelTrade(bool sendToSelfAlso, bool silently = false);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Messages
public:

    void sendReportToGmMessage(std::string playerName, std::string damageLog);

#if VERSION_STRING >= Cata
private:

    //////////////////////////////////////////////////////////////////////////////////////////
    // Chat
public:
    void sendChatPacket(uint32_t type, uint32_t language, const char* message, uint64_t guid, uint8_t flag);
    WorldPacket buildChatMessagePacket(Player* targetPlayer, uint32_t type, uint32_t language, const char* message, uint64_t guid, uint8_t flag);
    bool hasLanguage(uint32_t language);
    bool hasSkilledSkill(uint32_t skill);
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Cheats
    PlayerCheat m_cheats;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Items
    void unEquipOffHandIfRequired();
    bool hasOffHandWeapon() const;

    bool hasItem(uint32_t itemId, uint32_t amount = 1, bool checkBankAlso = false) const;

    // Player's item storage
    ItemInterface* getItemInterface() const;
private:
    ItemInterface* m_itemInterface;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    bool isGMFlagSet();

    void sendMovie(uint32_t movieId);

    void logIntoBattleground();
    bool logOntoTransport();
    void setLoginPosition();
    void setPlayerInfoIfNeeded();
    void setGuildAndGroupInfo();
    void sendCinematicOnFirstLogin();

    int32_t getMyCorpseInstanceId() const;

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
    void sendSpellModifierPacket(uint8_t spellGroup, uint8_t spellType, int32_t modifier, bool isPct);
    void sendLoginVerifyWorldPacket(uint32_t mapId, float posX, float posY, float posZ, float orientation);
    void sendMountResultPacket(uint32_t result);
    void sendDismountResultPacket(uint32_t result);
    void sendLogXpGainPacket(uint64_t guid, uint32_t normalXp, uint32_t restedXp, bool type);
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

public:
    //MIT End
    //AGPL Start

    friend class WorldSession;
    friend class Pet;

    public:

        bool Teleport(const LocationVector& vec, MapMgr* map) override;

        Player(uint32_t guid);
        ~Player();
    PlayerCache* m_cache;

        void EventGroupFullUpdate();

        // Skill System
        void _AdvanceSkillLine(uint32_t SkillLine, uint32_t Count = 1);
        void _AddSkillLine(uint32_t SkillLine, uint32_t Current, uint32_t Max);
        uint32_t _GetSkillLineMax(uint32_t SkillLine);
        uint32_t _GetSkillLineCurrent(uint32_t SkillLine, bool IncludeBonus = true);
        void _RemoveSkillLine(uint32_t SkillLine);
        void _UpdateMaxSkillCounts();
        void _ModifySkillBonus(uint32_t SkillLine, int32_t Delta);
        void _ModifySkillBonusByType(uint32_t SkillType, int32_t Delta);
        bool _HasSkillLine(uint32_t SkillLine);
        void RemoveSpellsFromLine(uint32_t skill_line);
        void _RemoveAllSkills();
        void _RemoveLanguages();
        void _AddLanguages(bool All);
        void _AdvanceAllSkills(uint32_t count);
        void _ModifySkillMaximum(uint32_t SkillLine, uint32_t NewMax);
        void _LearnSkillSpells(uint32_t SkillLine, uint32_t Current);

        void UpdatePvPCurrencies();
        void FillRandomBattlegroundReward(bool wonBattleground, uint32_t &honorPoints, uint32_t &arenaPoints);
        void ApplyRandomBattlegroundReward(bool wonBattleground);

        LfgMatch* m_lfgMatch;
        uint32_t m_lfgInviterGuid;

        // Summon and Appear Blocking
        void DisableSummon(bool disable) { disableSummon = disable; }
        bool IsSummonDisabled() { return disableSummon; }
        void DisableAppear(bool disable) { disableAppear = disable; }
        bool IsAppearDisabled() { return disableAppear; }

        // Scripting
        void SendChatMessage(uint8_t type, uint32_t lang, const char* msg, uint32_t delay = 0) override;
        void SendChatMessageToPlayer(uint8_t type, uint32_t lang, const char* msg, Player* plr) override;
    protected:

        void _UpdateSkillFields();

        SkillMap m_skills;

        // Summon and Appear Blocking
        bool disableAppear;
        bool disableSummon;

        // COOLDOWNS
        uint32_t m_lastPotionId;
        PlayerCooldownMap m_cooldownMap[NUM_COOLDOWN_TYPES];
        uint32_t m_globalCooldown;

        /***********************************************************************************
            AFTER THIS POINT, public and private ARE PASSED AROUND LIKE A CHEAP WH*RE :P
            Let's keeps thing clean (use encapsulation) above this line. Thanks.
        ***********************************************************************************/

    public:
        void SetLastPotion(uint32_t itemid) { m_lastPotionId = itemid; }
        void Cooldown_AddItem(ItemProperties const* pProto, uint32_t x);
        bool Cooldown_CanCast(ItemProperties const* pProto, uint32_t x);
        void UpdatePotionCooldown();
        bool HasSpellWithAuraNameAndBasePoints(uint32_t auraname, uint32_t basepoints);

    protected:

        void AddCategoryCooldown(uint32_t category_id, uint32_t time, uint32_t SpellId, uint32_t ItemId);
        void _Cooldown_Add(uint32_t Type, uint32_t Misc, uint32_t Time, uint32_t SpellId, uint32_t ItemId);
        void _LoadPlayerCooldowns(QueryResult* result);
        void _SavePlayerCooldowns(QueryBuffer* buf);

        // END COOLDOWNS
    public:

        void RemoveItemByGuid(uint64_t GUID);

        //! Okay to remove from world
        bool ok_to_remove;
        void OnLogin();//custom stuff on player login.

        void SendEquipmentSetList();
        void SendEquipmentSetSaved(uint32_t setID, uint32_t setGUID);
        
        void SendEmptyPetSpellList();

        void SendInitialWorldstates();

        void OutPacket(uint16_t opcode, uint16_t len, const void* data);
        void SendPacket(WorldPacket* packet);
        void SendMessageToSet(WorldPacket* data, bool self, bool myteam_only = false);
        void OutPacketToSet(uint16_t Opcode, uint16_t Len, const void* Data, bool self);


        /////////////////////////////////////////////////////////////////////////////////////////
        // static void CharChange_Looks(uint64_t GUID, uint8_t gender, uint8_t skin, uint8_t face, uint8_t hairStyle, uint8_t hairColor, uint8_t facialHair)
        // Updates database with characters new looks, gender, and name after character customization is called at login.
        //
        // \param uint64_t GUID          - GUID of the character to customized
        // \param uint8_t gender         - New gender of the character customized
        // \param uint8_t skin           - New skin colour of the character customized
        // \param uint8_t face           - New face selection of the character customized
        // \param uint8_t hairStyle      - New hair style selected for the character customized
        // \param uint8_t hairColor      - New hair color selected for the character customized
        // \param uint8_t facialHair     - New facial hair selected for the character customized
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        static void CharChange_Looks(uint64_t GUID, uint8_t gender, uint8_t skin, uint8_t face, uint8_t hairStyle, uint8_t hairColor, uint8_t facialHair);


        /////////////////////////////////////////////////////////////////////////////////////////
        // static void CharChange_Language(uint64_t GUID, uint8_t race)
        // Updates the characters racial languages
        //
        // \param uint64_t GUID         -  GUID of the character to customized
        // \param uint8_t race          -  New race to be usedd for racial language change
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        static void CharChange_Language(uint64_t GUID, uint8_t race);


        void AddToWorld();
        void AddToWorld(MapMgr* pMapMgr);
        void RemoveFromWorld();
        bool Create(CharCreate& charCreateContent);

        void Update(unsigned long time_passed);
        void BuildFlagUpdateForNonGroupSet(uint32_t index, uint32_t flag);
        void BuildPetSpellList(WorldPacket & data);
        void SetAFKReason(std::string reason) { m_cache->SetStringValue(CACHE_AFK_DND_REASON, reason); };

        void GiveXP(uint32_t xp, const uint64_t & guid, bool allowbonus);       /// to stop rest xp being given
        void ModifyBonuses(uint32_t type, int32_t val, bool apply);
        void CalcExpertise();
        std::map<uint32_t, uint32_t> m_wratings;

        ArenaTeam* m_arenaTeams[NUM_ARENA_TEAM_TYPES];

        /////////////////////////////////////////////////////////////////////////////////////////
        // Taxi
        /////////////////////////////////////////////////////////////////////////////////////////
        TaxiPath* GetTaxiPath() { return m_CurrentTaxiPath; }
        bool isOnTaxi() const { return m_onTaxi; }
        const uint32_t & GetTaximask(uint32_t index) const { return m_taximask[index]; }
        void LoadTaxiMask(const char* data);
        void TaxiStart(TaxiPath* path, uint32_t modelid, uint32_t start_node);
        void JumpToEndTaxiNode(TaxiPath* path);
        void EventDismount(uint32_t money, float x, float y, float z);
        void EventTaxiInterpolate();

        void SetTaxiState(bool state) { m_onTaxi = state; }
        void SetTaximask(uint32_t index, uint32_t value) { m_taximask[index] = value; }
        void SetTaxiPath(TaxiPath* path) { m_CurrentTaxiPath = path; }
        void SetTaxiPos() { m_taxi_pos_x = m_position.x; m_taxi_pos_y = m_position.y; m_taxi_pos_z = m_position.z; }
        void UnSetTaxiPos() { m_taxi_pos_x = 0; m_taxi_pos_y = 0; m_taxi_pos_z = 0; }

        // Taxi related variables
        std::vector<TaxiPath*> m_taxiPaths;
        TaxiPath* m_CurrentTaxiPath;
        uint32_t taxi_model_id;
        uint32_t lastNode;
        uint32_t m_taxi_ride_time;
        uint32_t m_taximask[DBC_TAXI_MASK_SIZE];
        float m_taxi_pos_x;
        float m_taxi_pos_y;
        float m_taxi_pos_z;
        bool m_onTaxi;
        uint32_t m_taxiMapChangeNode;

        /////////////////////////////////////////////////////////////////////////////////////////
        // Quests
        /////////////////////////////////////////////////////////////////////////////////////////
        bool HasQuests()
        {
            for (uint8_t i = 0; i < MAX_QUEST_SLOT; ++i)
                if (m_questlog[i] != nullptr)
                    return true;

            return false;
        }

        uint16_t GetOpenQuestSlot();
        QuestLogEntry* GetQuestLogForEntry(uint32_t quest);
        QuestLogEntry* GetQuestLogInSlot(uint32_t slot) { return m_questlog[slot]; }
        uint32_t GetQuestSharer() { return m_questSharer; }

        void SetQuestSharer(uint32_t guid) { m_questSharer = guid; }
        void SetQuestLogSlot(QuestLogEntry* entry, uint32_t slot);

        void PushToRemovedQuests(uint32_t questid) { m_removequests.insert(questid);}
        void PushToFinishedDailies(uint32_t questid) { DailyMutex.Acquire(); m_finishedDailies.insert(questid); DailyMutex.Release();}
        bool HasFinishedDaily(uint32_t questid) { return (m_finishedDailies.find(questid) == m_finishedDailies.end() ? false : true); }
        void AddToFinishedQuests(uint32_t quest_id);
        void AreaExploredOrEventHappens(uint32_t questId);   // scriptdev2

        bool HasFinishedQuest(uint32_t quest_id);

        void EventTimedQuestExpire(uint32_t questid);


        //////////////////////////////////////////////////////////////////////////////////////////
        // bool HasTimedQuest()
        // Tells if the Player has a timed quest already
        //
        // \param none
        //
        // \return true if the Player already has a timed quest, false otherwise
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        bool HasTimedQuest();


        //////////////////////////////////////////////////////////////////////////////////////////
        // void ClearQuest(uint32_t id)
        // Clears the finished status of a quest
        //
        // \param uint32_t id  -  Identifier of the quest
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void ClearQuest(uint32_t id);

        bool GetQuestRewardStatus(uint32_t quest_id);
        bool HasQuestForItem(uint32_t itemid);
        bool HasQuestSpell(uint32_t spellid);
        void RemoveQuestSpell(uint32_t spellid);
        bool HasQuestMob(uint32_t entry);
        bool HasQuest(uint32_t entry);
        void RemoveQuestMob(uint32_t entry);
        void AddQuestKill(uint32_t questid, uint8_t reqid, uint32_t delay = 0);


        /////////////////////////////////////////////////////////////////////////////////////////
        // void AcceptQuest(uint64_t guid, uint32_t quest_id)
        // Checks if the quest is acceptable from that questgiver and accepts it.
        //
        // \param uin64 guid      -  guid of the questgiver
        // \param uint32_t quest_id -  id of the quest
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void AcceptQuest(uint64_t guid, uint32_t quest_id);

        //Quest related variables
        QuestLogEntry* m_questlog[MAX_QUEST_LOG_SIZE];
        std::set<uint32_t> m_removequests;
        std::set<uint32_t> m_finishedQuests;
        Mutex DailyMutex;
        std::set<uint32_t> m_finishedDailies;
        uint32_t m_questSharer;
        std::set<uint32_t> quest_spells;
        std::set<uint32_t> quest_mobs;

        void EventPortToGM(Player* p);

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

        int32_t GetDamageDoneMod(uint16_t school)
        {
            if (school >= TOTAL_SPELL_SCHOOLS)
                return 0;

            return static_cast<int32_t>(getModDamageDonePositive(school)) - static_cast<int32_t>(getModDamageDoneNegative(school));
        }

        float GetDamageDonePctMod(uint16_t school)
        {
            if (school >= TOTAL_SPELL_SCHOOLS)
                return 0;

            return getModDamageDonePct(static_cast<uint8_t>(school));
        }

        uint32_t GetMainMeleeDamage(uint32_t AP_owerride);          // I need this for windfury

        const uint64_t & GetSelection() const { return m_curSelection; }
        const uint64_t & GetTarget() const { return m_curTarget; }
        void SetSelection(const uint64_t & guid) { m_curSelection = guid; }
        void SetTarget(const uint64_t & guid) { m_curTarget = guid; }

        /////////////////////////////////////////////////////////////////////////////////////////
        // Spells
        /////////////////////////////////////////////////////////////////////////////////////////
        bool HasSpell(uint32_t spell);
        bool HasDeletedSpell(uint32_t spell);
        void smsg_InitialSpells();
        void addSpell(uint32_t spell_idy);
        bool removeSpell(uint32_t SpellID, bool MoveToDeleted, bool SupercededSpell, uint32_t SupercededSpellID);
        bool removeDeletedSpell(uint32_t SpellID);
        void SendPreventSchoolCast(uint32_t SpellSchool, uint32_t unTimeMs);

        // PLEASE DO NOT INLINE!
        void AddOnStrikeSpell(SpellInfo const* sp, uint32_t delay)
        {
            m_onStrikeSpells.insert(std::map<SpellInfo const*, std::pair<uint32_t, uint32_t>>::value_type(sp, std::make_pair(delay, 0)));
        }
        void RemoveOnStrikeSpell(SpellInfo const* sp)
        {
            m_onStrikeSpells.erase(sp);
        }
        void AddOnStrikeSpellDamage(uint32_t spellid, uint32_t mindmg, uint32_t maxdmg)
        {
            OnHitSpell sp;
            sp.spellid = spellid;
            sp.mindmg = mindmg;
            sp.maxdmg = maxdmg;
            m_onStrikeSpellDmg[spellid] = sp;
        }
        void RemoveOnStrikeSpellDamage(uint32_t spellid)
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

        void AddShapeShiftSpell(uint32_t id);
        void RemoveShapeShiftSpell(uint32_t id);
        /////////////////////////////////////////////////////////////////////////////////////////
        // Reputation
        /////////////////////////////////////////////////////////////////////////////////////////
        void ModStanding(uint32_t Faction, int32_t Value);
        int32_t GetStanding(uint32_t Faction);
        int32_t GetBaseStanding(uint32_t Faction);
        void SetStanding(uint32_t Faction, int32_t Value);
        void SetAtWar(uint32_t Faction, bool Set);

        Standing GetStandingRank(uint32_t Faction);
        bool IsHostileBasedOnReputation(DBC::Structures::FactionEntry const* dbc);
        void UpdateInrangeSetsBasedOnReputation();
        void Reputation_OnKilledUnit(Unit* pUnit, bool InnerLoop);
        void Reputation_OnTalk(DBC::Structures::FactionEntry const* dbc);
        static Standing GetReputationRankFromStanding(int32_t Standing_);
        void SetFactionInactive(uint32_t faction, bool set);
        bool AddNewFaction(DBC::Structures::FactionEntry const* dbc, int32_t standing, bool base);
        void OnModStanding(DBC::Structures::FactionEntry const* dbc, FactionReputation* rep);
        uint32_t GetExaltedCount();

        // Factions
        void smsg_InitialFactions();
        uint32_t GetInitialFactionId();
        // factions variables
        int32_t pctReputationMod;

        /////////////////////////////////////////////////////////////////////////////////////////
        // PVP
        /////////////////////////////////////////////////////////////////////////////////////////
        uint32_t GetMaxPersonalRating();

        bool HasTitle(RankTitles title)
        {
            return (getUInt64Value(PLAYER_FIELD_KNOWN_TITLES + ((title >> 6) << 1)) & (uint64_t(1) << (title % 64))) != 0;
        }
        void SetKnownTitle(RankTitles title, bool set);
        void SendAvailSpells(DBC::Structures::SpellShapeshiftFormEntry const* shapeshift_form, bool active);

        /////////////////////////////////////////////////////////////////////////////////////////
        // Groups
        /////////////////////////////////////////////////////////////////////////////////////////
        void SetInviter(uint32_t pInviter) { m_GroupInviter = pInviter; }
        uint32_t GetInviter() { return m_GroupInviter; }
        bool InGroup() { return m_playerInfo && (m_playerInfo->m_Group != NULL && !m_GroupInviter); }

        bool IsGroupLeader();
        int HasBeenInvited() { return m_GroupInviter != 0; }
        Group* GetGroup() { return m_playerInfo ? m_playerInfo->m_Group : NULL; }
        int8_t GetSubGroup() { return m_playerInfo->subGroup; }
        bool IsGroupMember(Player* plyr);

        bool IsBanned();
        void SetBanned() { m_banned = 4;}
        void SetBanned(std::string Reason) { m_banned = 4; m_banreason = Reason;}
        void SetBanned(uint32_t timestamp, std::string & Reason) { m_banned = timestamp; m_banreason = Reason; }
        void UnSetBanned() { m_banned = 0; }
        std::string GetBanReason() {return m_banreason;}

        /////////////////////////////////////////////////////////////////////////////////////////
        // Guilds
        /////////////////////////////////////////////////////////////////////////////////////////

        uint32_t m_GuildIdInvited;

        void SetGuildIdInvited(uint32_t GuildId) { m_GuildIdInvited = GuildId; }
        uint32_t GetGuildIdInvited() { return m_GuildIdInvited; }


        static uint32_t GetGuildIdFromDB(uint64_t guid);
        Guild* GetGuild();
        bool IsInGuild() { return GetGuild() != nullptr; }


        static int8_t GetRankFromDB(uint64_t guid);
        uint32_t GetGuildRank() { return (uint32_t)GetRankFromDB(getGuid()); }

        std::string GetGuildName();

        /////////////////////////////////////////////////////////////////////////////////////////
        // Duel
        /////////////////////////////////////////////////////////////////////////////////////////
        void RequestDuel(Player* pTarget);
        void DuelBoundaryTest();
        void EndDuel(uint8_t WinCondition);
        void DuelCountdown();
        void cancelDuel();
        void SetDuelStatus(uint8_t status) { m_duelStatus = status; }
        uint8_t GetDuelStatus() { return m_duelStatus; }
        void SetDuelState(uint8_t state) { m_duelState = state; }
        uint8_t GetDuelState() { return m_duelState; }
        // duel variables
        Player* DuelingWith;

        /////////////////////////////////////////////////////////////////////////////////////////
        // Pets
        /////////////////////////////////////////////////////////////////////////////////////////
        void AddSummon(Pet* pet) { m_Summons.push_front(pet); }
        Pet* GetSummon()     // returns 1st summon
        {
            if (!m_Summons.empty())
                return m_Summons.front();

            return nullptr;
        }
        std::list<Pet*> GetSummons() { return m_Summons; }

        void RemoveSummon(Pet* pet);
        uint32_t GeneratePetNumber();
        void RemovePlayerPet(uint32_t pet_number);
        void AddPlayerPet(PlayerPet* pet, uint32_t index) { m_Pets[index] = pet; }
        PlayerPet* GetPlayerPet(uint32_t idx)
        {
            std::map<uint32_t, PlayerPet*>::iterator itr = m_Pets.find(idx);
            if (itr != m_Pets.end())
                return itr->second;

            return nullptr;
        }
        void SpawnPet(uint32_t pet_number);
        void SpawnActivePet();
        void DismissActivePets();
        uint8_t GetPetCount() { return (uint8_t)m_Pets.size(); }
        void SetStableSlotCount(uint8_t count) { m_StableSlotCount = count; }
        uint8_t GetStableSlotCount() { return m_StableSlotCount; }

        uint32_t GetUnstabledPetNumber();
        void EventSummonPet(Pet* new_pet);   // if we charmed or simply summoned a pet, this function should get called
        void EventDismissPet();              // if pet/charm died or whatever happened we should call this function

        /////////////////////////////////////////////////////////////////////////////////////////
        // Item Interface
        /////////////////////////////////////////////////////////////////////////////////////////
        void ApplyItemMods(Item* item, int16_t slot, bool apply, bool justdrokedown = false) { _ApplyItemMods(item, slot, apply, justdrokedown); }
        /// item interface variables
        int32_t GetVisibleBase(int16_t slot)
        {
#if VERSION_STRING < WotLK
            return (PLAYER_VISIBLE_ITEM_1_0 + (slot * 16));
#else
            return (PLAYER_VISIBLE_ITEM_1_ENTRYID + (slot * 2));
#endif
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // Loot
        /////////////////////////////////////////////////////////////////////////////////////////
        const uint64_t & GetLootGUID() const { return m_lootGuid; }
        void SetLootGUID(const uint64_t & guid) { m_lootGuid = guid; }
        void SendLoot(uint64_t guid, uint8_t loot_type, uint32_t mapid);
        void SendLootUpdate(Object* o);
        void TagUnit(Object* o);
        
        // loot variables
        uint64_t m_lootGuid;
        uint64_t m_currentLoot;
        bool bShouldHaveLootableOnCorpse;

        /////////////////////////////////////////////////////////////////////////////////////////
        // World Session
        /////////////////////////////////////////////////////////////////////////////////////////
        WorldSession* GetSession() const { return m_session; }
        void SetSession(WorldSession* s) { m_session = s; }
        void SetBindPoint(float x, float y, float z, uint32_t m, uint32_t v) { m_bind_pos_x = x; m_bind_pos_y = y; m_bind_pos_z = z; m_bind_mapid = m; m_bind_zoneid = v;}

        void SendDelayedPacket(WorldPacket* data, bool bDeleteOnSend);
        float offhand_dmg_mod;

        // Talents
        // These functions build a specific type of A9 packet
        uint32_t buildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target);
        void SetTalentHearthOfWildPCT(int value) { hearth_of_wild_pct = value; }
        void EventTalentHearthOfWildChange(bool apply);

        std::list<LoginAura> loginauras;

        /////////////////////////////////////////////////////////////////////////////////////////
        // Player loading and savings Serialize character to db
        /////////////////////////////////////////////////////////////////////////////////////////
        void SaveToDB(bool bNewCharacter);
        void SaveAuras(std::stringstream &);
        bool LoadFromDB(uint32_t guid);
        void LoadFromDBProc(QueryResultVector & results);

        bool SaveSpells(bool NewCharacter, QueryBuffer* buf);

        bool LoadDeletedSpells(QueryResult* result);
        bool SaveDeletedSpells(bool NewCharacter, QueryBuffer* buf);

        bool LoadReputations(QueryResult* result);
        bool SaveReputations(bool NewCharacter, QueryBuffer *buf);

        bool LoadSkills(QueryResult* result);
        bool SaveSkills(bool NewCharacter, QueryBuffer* buf);

        bool m_FirstLogin;

        /////////////////////////////////////////////////////////////////////////////////////////
        // Drunk system
        /////////////////////////////////////////////////////////////////////////////////////////
        void SetDrunkValue(uint16_t newDrunkValue, uint32_t itemid = 0);
        uint16_t GetDrunkValue() const { return m_drunk; }
        static DrunkenState GetDrunkenstateByValue(uint16_t value);
        void HandleSobering();

        uint32_t m_drunkTimer;
        uint16_t m_drunk;

        /////////////////////////////////////////////////////////////////////////////////////////
        // Death system
        /////////////////////////////////////////////////////////////////////////////////////////
        void SpawnCorpseBody();
        void SpawnCorpseBones();
        void CreateCorpse();
        void KillPlayer();
        void ResurrectPlayer();
        void BuildPlayerRepop();
        void RepopRequestedPlayer();
        void DeathDurabilityLoss(double percent);
        void RepopAtGraveyard(float ox, float oy, float oz, uint32_t mapid);

        uint64_t m_resurrecter;

        /////////////////////////////////////////////////////////////////////////////////////////
        // Movement system
        /////////////////////////////////////////////////////////////////////////////////////////

        bool m_isMoving;            // moving + strafing + jumping
        bool moving;
        bool strafing;
        bool isTurning;
        bool jumping;
        //Invisibility stuff
        bool m_isGmInvisible;

        /////////////////////////////////////////////////////////////////////////////////////////
        // Channel stuff
        /////////////////////////////////////////////////////////////////////////////////////////
        void JoinedChannel(Channel* c);
        void LeftChannel(Channel* c);
        void CleanupChannels();

        /////////////////////////////////////////////////////////////////////////////////////////
        // Attack stuff
        /////////////////////////////////////////////////////////////////////////////////////////
        void EventAttackStart();
        void EventAttackStop();
        void EventAttackUpdateSpeed() { }
        void EventDeath();

        //Note:ModSkillLine -> value+=amt;ModSkillMax -->value=amt; --weird
        float GetSkillUpChance(uint32_t id);

        float SpellHasteRatingBonus;
        void UpdateAttackSpeed();
        float GetDefenseChance(uint32_t opLevel);
        float GetDodgeChance();
        float GetBlockChance();
        float GetParryChance();
        void UpdateChances();
        void UpdateStats();
        uint32_t GetBlockDamageReduction();
        void ApplyFeralAttackPower(bool apply, Item* item = NULL);

        float GetSpellCritFromSpell() { return m_spellcritfromspell; }
        float GetHitFromSpell() { return m_hitfromspell; }
        void SetSpellCritFromSpell(float value) { m_spellcritfromspell = value; }
        void SetHitFromSpell(float value) { m_hitfromspell = value; }

        uint32_t GetHealthFromSpell() { return m_healthfromspell; }
        uint32_t GetManaFromSpell() { return m_manafromspell; }
        void SetHealthFromSpell(uint32_t value) { m_healthfromspell = value;}
        void SetManaFromSpell(uint32_t value) { m_manafromspell = value;}

        uint32_t CalcTalentResetCost(uint32_t resetnum);
        
        uint32_t GetTalentResetTimes() { return m_talentresettimes; }
        void SetTalentResetTimes(uint32_t value) { m_talentresettimes = value; }

        void SetPlayerStatus(uint8_t pStatus) { m_status = pStatus; }
    uint8_t GetPlayerStatus() const;

        const float & GetBindPositionX() const { return m_bind_pos_x; }
        const float & GetBindPositionY() const { return m_bind_pos_y; }
        const float & GetBindPositionZ() const { return m_bind_pos_z; }

        const uint32_t & GetBindMapId() const { return m_bind_mapid; }
        const uint32_t & GetBindZoneId() const { return m_bind_zoneid; }

        void SetShapeShift(uint8_t ss);

        uint32_t m_furorChance;

        // Showing Units WayPoints
        AIInterface* waypointunit;

        uint32_t m_nextSave;
        // Tutorials
        uint32_t GetTutorialInt(uint32_t intId);
        void SetTutorialInt(uint32_t intId, uint32_t value);

        // Rest
        uint32_t SubtractRestXP(uint32_t amount);
        void AddCalculatedRestXP(uint32_t seconds);
        void ApplyPlayerRestState(bool apply);
        void UpdateRestState();

        int m_lifetapbonus;
        bool m_requiresNoAmmo;

        bool m_bUnlimitedBreath;
        uint32_t m_UnderwaterTime;
        uint32_t m_UnderwaterState;
        // Visible objects
        bool IsVisible(uint64_t pObj) { return !(m_visibleObjects.find(pObj) == m_visibleObjects.end()); }
        void addToInRangeObjects(Object* pObj);
        void onRemoveInRangeObject(Object* pObj);
        void clearInRangeSets();
        void AddVisibleObject(uint64_t pObj) { m_visibleObjects.insert(pObj); }
        void RemoveVisibleObject(uint64_t pObj) { m_visibleObjects.erase(pObj); }
        void RemoveVisibleObject(std::set< uint64_t >::iterator itr) { m_visibleObjects.erase(itr); }
        std::set< uint64_t >::iterator FindVisible(uint64_t obj) { return m_visibleObjects.find(obj); }
        void RemoveIfVisible(uint64_t obj);

        // Misc
        void EventCannibalize(uint32_t amount);
        bool m_AllowAreaTriggerPort;
        void EventAllowTiggerPort(bool enable);
        uint32_t m_modblockabsorbvalue;
        uint32_t m_modblockvaluefromspells;
        void SendInitialLogonPackets();
        void Reset_Spells();
        // Battlegrounds xD
        CBattleground* m_bg;
        CBattleground* m_pendingBattleground;
        uint32_t m_bgEntryPointMap;
        float m_bgEntryPointX;
        float m_bgEntryPointY;
        float m_bgEntryPointZ;
        float m_bgEntryPointO;
        int32_t m_bgEntryPointInstance;
        bool m_bgHasFlag;
        bool m_bgIsQueued;
        uint32_t m_bgQueueType;
        uint32_t m_bgQueueInstanceId;

    protected:

        // True if player queued for Random Battleground
        bool m_bgIsRbg;

        // True if player has won a Random Battleground today
        bool m_bgIsRbgWon;

    public:
        bool QueuedForRbg();
        void SetQueuedForRbg(bool value);
        bool HasWonRbgToday();
        void SetHasWonRbgToday(bool value);

        void _InitialReputation();
        void EventActivateGameObject(GameObject* obj);
        void EventDeActivateGameObject(GameObject* obj);
        void UpdateNearbyGameObjects();

        void CalcResistance(uint8_t type);
        float res_M_crit_get() { return m_resist_critical[0]; }
        void res_M_crit_set(float newvalue) { m_resist_critical[0] = newvalue; }
        float res_R_crit_get() { return m_resist_critical[1]; }
        void res_R_crit_set(float newvalue) { m_resist_critical[1] = newvalue; }
        uint32_t FlatResistanceModifierPos[TOTAL_SPELL_SCHOOLS];
        uint32_t FlatResistanceModifierNeg[TOTAL_SPELL_SCHOOLS];
        uint32_t BaseResistanceModPctPos[TOTAL_SPELL_SCHOOLS];
        uint32_t BaseResistanceModPctNeg[TOTAL_SPELL_SCHOOLS];
        uint32_t ResistanceModPctPos[TOTAL_SPELL_SCHOOLS];
        uint32_t ResistanceModPctNeg[TOTAL_SPELL_SCHOOLS];
        float m_resist_critical[2]; // when we are a victim we can have talents to decrease chance for critical hit. This is a negative value and it's added to critchances
        float m_resist_hit[2]; // 0 = melee; 1= ranged;
        int32_t m_resist_hit_spell[TOTAL_SPELL_SCHOOLS]; // spell resist per school
        float SpellHealDoneByAttribute[5][TOTAL_SPELL_SCHOOLS];
        uint32_t m_modphyscritdmgPCT;
        uint32_t m_RootedCritChanceBonus; // Class Script Override: Shatter
        uint32_t m_IncreaseDmgSnaredSlowed;

        // SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
        uint32_t m_ModInterrMRegenPCT;
        // SPELL_AURA_MOD_POWER_REGEN
        int32_t m_ModInterrMRegen;
        // SPELL_AURA_REGEN_MANA_STAT_PCT
        int32_t m_modManaRegenFromStat[STAT_COUNT];
        float m_RegenManaOnSpellResist;
        uint32_t m_casted_amount[TOTAL_SPELL_SCHOOLS]; // Last casted spells amounts. Need for some spells. Like Ignite etc. DOesn't count HoTs and DoTs. Only directs

        uint32_t FlatStatModPos[5];
        uint32_t FlatStatModNeg[5];
        uint32_t StatModPctPos[5];
        uint32_t StatModPctNeg[5];
        uint32_t TotalStatModPctPos[5];
        uint32_t TotalStatModPctNeg[5];
        int32_t IncreaseDamageByType[12]; // mod dmg by creature type
        float IncreaseDamageByTypePCT[12];
        float IncreaseCricticalByTypePCT[12];
        int32_t DetectedRange;
        float PctIgnoreRegenModifier;
        uint32_t m_retainedrage;

        uint32_t* GetPlayedtime() { return m_playedtime; };
        void CalcStat(uint8_t t);
        float CalcRating(PlayerCombatRating t);
        void RegenerateHealth(bool inCombat);

        uint32_t SoulStone;
        uint32_t SoulStoneReceiver;
        void removeSoulStone();

        uint32_t GetSoulStoneReceiver() { return SoulStoneReceiver; }
        void SetSoulStoneReceiver(uint32_t StoneGUID) { SoulStoneReceiver = StoneGUID; }
        uint32_t GetSoulStone() { return SoulStone; }
        void SetSoulStone(uint32_t StoneID) { SoulStone = StoneID; }

        uint64_t misdirectionTarget;

        uint64_t GetMisdirectionTarget() { return misdirectionTarget; }
        void SetMisdirectionTarget(uint64_t PlayerGUID) { misdirectionTarget = PlayerGUID; }

        bool bReincarnation;

        std::map<uint32_t, WeaponModifier> damagedone;
        std::map<uint32_t, WeaponModifier> tocritchance;
        bool cannibalize;
        uint8_t cannibalizeCount;
        int32_t rageFromDamageDealt;
        int32_t rageFromDamageTaken;
        // GameObject commands
        GameObject * GetSelectedGo();

        uint64_t m_GM_SelectedGO;

        void _Relocate(uint32_t mapid, const LocationVector & v, bool sendpending, bool force_new_world, uint32_t instance_id);

        void AddItemsToWorld();
        void RemoveItemsFromWorld();
        void UpdateKnownCurrencies(uint32_t itemId, bool apply);

        uint32_t m_ShapeShifted;
        uint32_t m_MountSpellId;
        uint32_t mountvehicleid;

        bool IsMounted();

        void Dismount()
        {
            if (m_MountSpellId != 0)
            {
                RemoveAura(m_MountSpellId);
                m_MountSpellId = 0;
            }
        }

        bool isVehicle() const override
        {
            if (mountvehicleid != 0)
                return true;

            return false;
        }


        void addVehicleComponent(uint32_t creature_entry, uint32_t vehicleid);

        void removeVehicleComponent();

        bool bHasBindDialogOpen;
        uint32_t TrackingSpell;
        void _EventCharmAttack();
        void _Kick();
        void Kick(uint32_t delay = 0);
        void SoftDisconnect();
        uint32_t m_KickDelay;
        uint64_t m_CurrentCharm;

        Object* GetSummonedObject() { return m_SummonedObject; };
        void SetSummonedObject(Object* t_SummonedObject) { m_SummonedObject = t_SummonedObject; };

        void ClearCooldownsOnLine(uint32_t skill_line, uint32_t called_from);
        void ResetAllCooldowns();
        void ClearCooldownForSpell(uint32_t spell_id);

        void Phase(uint8_t command = PHASE_SET, uint32_t newphase = 1);

        void ProcessPendingUpdates();
        bool CompressAndSendUpdateBuffer(uint32_t size, const uint8_t* update_buffer);

        uint32_t GetArmorProficiency() { return armor_proficiency; }
        uint32_t GetWeaponProficiency() { return weapon_proficiency; }

        bool ExitInstance();
        void SaveEntryPoint(uint32_t mapId);

        // Cheat section
        void SpeedCheatDelay(uint32_t ms_delay);
        void SpeedCheatReset();

        bool SaveAllChangesCommand;

        void ZoneUpdate(uint32_t ZoneId);
        void UpdateChannels(uint16_t AreaID);
        uint32_t GetAreaID() { return m_AreaID; }
        void SetAreaID(uint32_t area) { m_AreaID = area; }
        bool IsInCity();

        // Instance IDs
        uint32_t GetPersistentInstanceId(uint32_t mapId, uint8_t difficulty)
        {
            if (mapId >= MAX_NUM_MAPS || difficulty >= NUM_INSTANCE_MODES || m_playerInfo == NULL)
                return 0;

            m_playerInfo->savedInstanceIdsLock.Acquire();
            PlayerInstanceMap::iterator itr = m_playerInfo->savedInstanceIds[difficulty].find(mapId);
            if (itr == m_playerInfo->savedInstanceIds[difficulty].end())
            {
                m_playerInfo->savedInstanceIdsLock.Release();
                return 0;
            }

            m_playerInfo->savedInstanceIdsLock.Release();
            return (*itr).second;
        }

        void SetPersistentInstanceId(Instance* pInstance);
        //Use this method carefully..
        void SetPersistentInstanceId(uint32_t mapId, uint8_t difficulty, uint32_t instanceId);

    public:

        bool m_Autojoin;
        bool m_AutoAddMem;
        void SendMirrorTimer(MirrorTimerTypes Type, uint32_t max, uint32_t current, int32_t regen);
        
        BGScore m_bgScore;
        
        void UpdateChanceFields();
        //Honor Variables
        time_t m_fallDisabledUntil;
        uint32_t m_honorToday;
        uint32_t m_honorYesterday;

        void RolloverHonor();
        uint32_t m_honorPoints;
        uint32_t m_honorRolloverTime;
        uint32_t m_killsToday;
        uint32_t m_killsYesterday;
        uint32_t m_killsLifetime;
        uint32_t m_arenaPoints;
        uint32_t m_honorless;
        uint32_t m_lastSeenWeather;
        std::set<Object*> m_visibleFarsightObjects;
        void EventTeleport(uint32_t mapid, float x, float y, float z);
        void EventTeleportTaxi(uint32_t mapid, float x, float y, float z);
        void BroadcastMessage(const char* Format, ...);
        std::map<uint32_t, std::set<uint32_t> > SummonSpells;
        std::map<uint32_t, std::map<SpellInfo const*, uint16_t>*> PetSpells;
        void AddSummonSpell(uint32_t Entry, uint32_t SpellID);
        void RemoveSummonSpell(uint32_t Entry, uint32_t SpellID);
        std::set<uint32_t>* GetSummonSpells(uint32_t Entry);
        uint32_t m_UnderwaterMaxTime;
        uint32_t m_UnderwaterLastDmg;
        LocationVector getMyCorpseLocation() const { return myCorpseLocation; }
        bool bCorpseCreateable;
        uint32_t m_resurrectHealth, m_resurrectMana;
        uint32_t m_resurrectInstanceID, m_resurrectMapId;
        LocationVector m_resurrectPosition;
        bool blinked;
        uint32_t m_explorationTimer;
        // DBC stuff
        DBC::Structures::ChrRacesEntry const* myRace;
        DBC::Structures::ChrClassesEntry const* myClass;
        Creature* linkTarget;

        bool SafeTeleport(uint32_t MapID, uint32_t InstanceID, float X, float Y, float Z, float O);
        bool SafeTeleport(uint32_t MapID, uint32_t InstanceID, const LocationVector & vec);
        void SafeTeleport(MapMgr* mgr, const LocationVector & vec);
        void EjectFromInstance();
        bool raidgrouponlysent;

        void SetDungeonDifficulty(uint8_t diff);
        uint8_t GetDungeonDifficulty();

        void SetRaidDifficulty(uint8_t diff);
        uint8_t GetRaidDifficulty();

        void EventSafeTeleport(uint32_t MapID, uint32_t InstanceID, LocationVector vec)
        {
            SafeTeleport(MapID, InstanceID, vec);
        }

        // Hack fix here!
        void ForceZoneUpdate();

        bool HasAreaExplored(::DBC::Structures::AreaTableEntry const*);
        bool HasOverlayUncovered(uint32_t overlayID);

        /////////////////////////////////////////////////////////////////////////////////////////
        //  PVP Stuff
        /////////////////////////////////////////////////////////////////////////////////////////
        uint32_t m_pvpTimer;


        /////////////////////////////////////////////////////////////////////////////////////////
        // EASY FUNCTIONS - MISC
        /////////////////////////////////////////////////////////////////////////////////////////

        void SetInventorySlot(uint16_t slot, uint64_t guid) { setUInt64Value(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2), guid); }

        //\todo fix this
        void ModPrimaryProfessionPoints(int32_t amt)
        {
#if VERSION_STRING < Cata
            modUInt32Value(PLAYER_CHARACTER_POINTS2, amt);
#else
            if (amt == 0) { return; }
#endif
        }

        void SetHonorCurrency(uint32_t value)
        {
#if VERSION_STRING >= Cata
            if (value == 0) { return; }
#elif VERSION_STRING == Classic
#else
            setUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, value);
#endif
        }
        void ModHonorCurrency(uint32_t value)
        {
#if VERSION_STRING >= Cata
            if (value == 0) { return; }
#elif VERSION_STRING == Classic
#else
            modUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, value);
#endif
        }
        uint32_t GetHonorCurrency()
        {
#if VERSION_STRING >= Cata
            return 0;
#elif VERSION_STRING == Classic
            return 0;
#else
            return getUInt32Value(PLAYER_FIELD_HONOR_CURRENCY);
#endif
        }

        void AddHonor(uint32_t honorPoints, bool sendUpdate);
        void UpdateHonor();

        void AddArenaPoints(uint32_t arenaPoints, bool sendUpdate);
        void UpdateArenaPoints();

#if VERSION_STRING > TBC
        uint32_t GetGlyph(uint32_t spec, uint32_t slot) const { return m_specs[spec].glyphs[slot]; }
#endif

        // Do this on /pvp off
        void ResetPvPTimer();
        // Stop the timer for pvp off
        void StopPvPTimer() { m_pvpTimer = 0; }

        // Called at login to add the honorless buff, etc.
        void LoginPvPSetup();
        // Update our pvp area (called when zone changes)
        void UpdatePvPArea();
        // PvP Toggle (called on /pvp)
        void PvPToggle();

        //////////////////////////////////////////////////////////////////////////////////////////
        // void HandleSpellLoot(uint32_t itemid)
        // Generates loot for the spell loot item (clams for example) , then adds the generated loot to the Player
        //
        // \param uint32_t itemid   -  unique numerical identifier of the item the Player is looting
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void HandleSpellLoot(uint32_t itemid);

        void DealDamage(Unit* pVictim, uint32_t damage, uint32_t targetEvent, uint32_t unitEvent, uint32_t spellId, bool no_remove_auras = false);
        void TakeDamage(Unit* pAttacker, uint32_t damage, uint32_t spellid, bool no_remove_auras = false);
        void Die(Unit* pAttacker, uint32_t damage, uint32_t spellid);
        void HandleKnockback(Object* caster, float horizontal, float vertical);

        uint32_t LastHonorResetTime() const { return m_lastHonorResetTime; }
        void LastHonorResetTime(uint32_t val) { m_lastHonorResetTime = val; }
        uint32_t OnlineTime;
        bool tutorialsDirty;
        LevelInfo* lvlinfo;
        uint32_t load_health;
        uint32_t load_mana;
        void CompleteLoading();
        void OnPushToWorld();
        void OnPrePushToWorld();
        void OnWorldPortAck();
        uint32_t m_TeleportState;
        bool m_beingPushed;
        bool CanSignCharter(Charter* charter, Player* requester);
        Charter* m_charters[NUM_CHARTER_TYPES];
        uint32_t flying_aura;
        bool resend_speed;
        uint32_t login_flags;
        uint8_t iInstanceType;

        FactionReputation* reputationByListId[128];

        uint64_t m_comboTarget;
        int8_t m_comboPoints;
        bool m_retainComboPoints;
        int8_t m_spellcomboPoints;        // rogue talent Ruthlessness will change combopoints while consuming them. solutions 1) add post cast prochandling, 2) delay adding the CP
        void UpdateComboPoints();

        void AddComboPoints(uint64_t target, int8_t count);

        void NullComboPoints() { if (!m_retainComboPoints) { m_comboTarget = 0; m_comboPoints = 0; m_spellcomboPoints = 0; } UpdateComboPoints(); }
        uint32_t m_speedChangeCounter;

        void SendAreaTriggerMessage(const char* message, ...);

        // Water level related stuff (they are public because they need to be accessed fast)
        // Nose level of the character (needed for proper breathing)
        float m_noseLevel;

        void RemoteRevive();

        LocationVector m_last_group_position;
        int32_t m_rap_mod_pct;
        void SummonRequest(uint32_t Requestor, uint32_t ZoneID, uint32_t MapID, uint32_t InstanceID, const LocationVector & Position);

        bool m_deathVision;
        SpellInfo const* last_heal_spell;
        LocationVector m_sentTeleportPosition;

        bool InBattleground() const { return m_bgQueueInstanceId != 0; }
        bool InBattlegroundQueue() const { return m_bgIsQueued != 0; }

        void RemoveFromBattlegroundQueue();
        void FullHPMP();
        void RemoveTempEnchantsOnArena();
        uint32_t m_arenateaminviteguid;

        void ResetTimeSync();
        void SendTimeSync();

        /////////////////////////////////////////////////////////////////////////////////////////
        // Spell Packet wrapper Please keep this separated
        /////////////////////////////////////////////////////////////////////////////////////////
        void SendWorldStateUpdate(uint32_t WorldState, uint32_t Value);

        /////////////////////////////////////////////////////////////////////////////////////////
        // End of SpellPacket wrapper
        /////////////////////////////////////////////////////////////////////////////////////////

        Mailbox m_mailBox;
        uint64_t m_areaSpiritHealer_guid;
        bool m_finishingmovesdodge;

        bool IsAttacking() { return m_attacking; }

        static void InitVisibleUpdateBits();
        static UpdateMask m_visibleUpdateMask;

        void CopyAndSendDelayedPacket(WorldPacket* data);
        void PartLFGChannel();
        SpeedCheatDetector* SDetector;

    protected:

        LocationVector m_summonPos;
        uint32_t m_summonInstanceId;
        uint32_t m_summonMapId;
        uint32_t m_summoner;

        void _SetCreateBits(UpdateMask* updateMask, Player* target) const;
        void _SetUpdateBits(UpdateMask* updateMask, Player* target) const;

        void _LoadTutorials(QueryResult* result);
        void _SaveTutorials(QueryBuffer* buf);
        void _SaveQuestLogEntry(QueryBuffer* buf);
        void _LoadQuestLogEntry(QueryResult* result);

        void _LoadPet(QueryResult* result);
        void _LoadPetSpells(QueryResult* result);
        void _SavePet(QueryBuffer* buf);
        void _SavePetSpells(QueryBuffer* buf);
        void _ApplyItemMods(Item* item, int16_t slot, bool apply, bool justdrokedown = false, bool skip_stat_apply = false);
        void _EventAttack(bool offhand);
        void _EventExploration();
        void CastSpellArea();

        // Water level related stuff
        void SetNoseLevel();

        /////////////////////////////////////////////////////////////////////////////////////////
        // Player Class systems, info and misc things
        /////////////////////////////////////////////////////////////////////////////////////////
        PlayerCreateInfo const* info;
        uint32_t m_AttackMsgTimer; // "too far away" and "wrong facing" timer
        bool m_attacking;
        
        uint32_t m_Tutorials[8];

        // Character Ban
        uint32_t m_banned;
        std::string m_banreason;
        uint32_t m_AreaID;
        std::list<Pet*>  m_Summons;
        uint32_t m_PetNumberMax;
        std::map<uint32_t, PlayerPet*> m_Pets;

        uint32_t m_invitersGuid; // It is guild inviters guid ,0 when its not used

        // bind
        float m_bind_pos_x;
        float m_bind_pos_y;
        float m_bind_pos_z;
        uint32_t m_bind_mapid;
        uint32_t m_bind_zoneid;
        std::list<ItemSet> m_itemsets;
        //Duel
        uint32_t m_duelCountdownTimer;
        uint8_t m_duelStatus;
        uint8_t m_duelState;
        // Rested State Stuff
        uint32_t m_timeLogoff;
        // Played time
        // 0 = played on level
        // 1 = played total
        // 2 = played session
        uint32_t m_playedtime[3];


        uint8_t m_isResting;
        uint8_t m_restState;
        uint32_t m_restAmount;
        //combat mods
        float m_blockfromspellPCT;
        float m_critfromspell;
        float m_spellcritfromspell;
        float m_hitfromspell;
        //stats mods
        uint32_t m_healthfromspell;
        uint32_t m_manafromspell;
        uint32_t m_healthfromitems;
        uint32_t m_manafromitems;

        uint32_t armor_proficiency;
        uint32_t weapon_proficiency;
        // Talents
        uint32_t m_talentresettimes;
        // STATUS
        uint8_t m_status;
        // guid of current target
        uint64_t m_curTarget;
        // guid of current selection
        uint64_t m_curSelection;
        // Raid
        uint8_t m_targetIcon;
        // Player Reputation
        ReputationMap m_reputation;
        // Pointer to this char's game client
        WorldSession* m_session;
        // Channels
        std::set<Channel*> m_channels;
        // Visible objects
        std::set<uint64_t> m_visibleObjects;
        // Groups/Raids
        uint32_t m_GroupInviter;
        uint8_t m_StableSlotCount;

        // Fishing related
        Object* m_SummonedObject;

        // other system
        LocationVector myCorpseLocation;
        int32_t myCorpseInstanceId;

        uint32_t m_lastHonorResetTime;
        uint32_t _fields[PLAYER_END];
        int hearth_of_wild_pct; // druid hearth of wild talent used on shapeshifting. We either know what is last talent level or memo on learn

        uint32_t m_indoorCheckTimer;
        void RemovePendingPlayer();

    public:

        void addDeletedSpell(uint32_t id) { mDeletedSpells.insert(id); }

        std::map<uint32_t, uint32_t> m_forcedReactions;

        uint32_t m_flyhackCheckTimer;
        //void _FlyhackCheck(); disabled not working not used. Zyres.

        bool m_passOnLoot;
        uint32_t m_tradeSequence;
        bool m_changingMaps;

        /////////////////////////////////////////////////////////////////////////////////////////
        // SOCIAL
        /////////////////////////////////////////////////////////////////////////////////////////
    private:

        void Social_SendFriendList(uint32_t flag);

        void Social_AddFriend(const char* name, const char* note);
        void Social_RemoveFriend(uint32_t guid);

        void Social_AddIgnore(const char* name);
        void Social_RemoveIgnore(uint32_t guid);

        void Social_SetNote(uint32_t guid, const char* note);

    public:

        bool Social_IsIgnoring(PlayerInfo* m_info);
        bool Social_IsIgnoring(uint32_t guid);

        void Social_TellFriendsOnline();
        void Social_TellFriendsOffline();

        /////////////////////////////////////////////////////////////////////////////////////////
        // end social
        /////////////////////////////////////////////////////////////////////////////////////////

        uint32_t m_outStealthDamageBonusPct;
        uint32_t m_outStealthDamageBonusPeriod;
        uint32_t m_outStealthDamageBonusTimer;

        //\todo sort out where all the publics and privates go. This will do for now..

    private:

        PlayerInfo* m_playerInfo;
        uint8_t m_RaidDifficulty;
        bool m_XpGain;
        bool resettalents;
        std::list< Item* > m_GarbageItems;

        void RemoveGarbageItems();

        uint32_t ChampioningFactionID;

        uint32_t m_timeSyncCounter;
        uint32_t m_timeSyncTimer;
        uint32_t m_timeSyncClient;
        uint32_t m_timeSyncServer;

    public:

        void SetChampioningFaction(uint32_t f) { ChampioningFactionID = f; }
        void AddGarbageItem(Item* it);
        uint32_t CheckDamageLimits(uint32_t dmg, uint32_t spellid);

        PlayerInfo* getPlayerInfo() const { return m_playerInfo; }

        void LoadFieldsFromString(const char* string, uint16_t firstField, uint32_t fieldsNum);
#if VERSION_STRING > TBC
        void UpdateGlyphs();
#endif

        // Avenging Wrath
        bool mAvengingWrath;
        void AvengingWrath() { mAvengingWrath = true; }

        void ToggleXpGain();
        bool CanGainXp();

#if VERSION_STRING > TBC
        AchievementMgr & GetAchievementMgr() { return m_achievementMgr; }
        AchievementMgr m_achievementMgr;
#endif

        /////////////////////////////////////////////////////////////////////////////////////////
        // Talent Specs
        /////////////////////////////////////////////////////////////////////////////////////////
        uint16_t m_maxTalentPoints;
        uint8_t m_talentSpecsCount;
        uint8_t m_talentActiveSpec;
#if VERSION_STRING >= Cata
        uint32_t m_FirstTalentTreeLock;
#endif

#ifdef FT_DUAL_SPEC
    PlayerSpec m_specs[MAX_SPEC_COUNT];
#else
    PlayerSpec m_spec;
#endif

    PlayerSpec& getActiveSpec();

        uint8_t m_roles;
        uint32_t GroupUpdateFlags;

    public:

        void SendUpdateDataToSet(ByteBuffer* groupbuf, ByteBuffer* nongroupbuf, bool sendtoself);

        bool CanBuyAt(MySQLStructure::VendorRestrictions const* vendor);
        bool CanTrainAt(Trainer*);

        void SetRoles(uint8_t role) { m_roles = role; }
        uint8_t GetRoles() { return m_roles; }
        void SetBattlegroundEntryPoint();

        uint32_t GetGroupUpdateFlags() { return GroupUpdateFlags; }
        void SetGroupUpdateFlags(uint32_t flags);
        void AddGroupUpdateFlag(uint32_t flag);
        uint16_t GetGroupStatus();
        void SendUpdateToOutOfRangeGroupMembers();

        void SendTeleportPacket(float x, float y, float z, float o);
        void SendTeleportAckPacket(float x, float y, float z, float o);

        void SendCinematicCamera(uint32_t id);

        void SetMover(Unit* target);

        // command
        float go_last_x_rotation;
        float go_last_y_rotation;

        // AGPL End
};
