/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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
#include "Macros/MapsMacros.hpp"
#include "Macros/PlayerMacros.hpp"
#include "Units/Creatures/AIInterface.h" //?? what?
#include "WorldConf.h"
#include "Management/AuctionHouse.h"
#include "Management/Guild/Guild.hpp"
#include "Management/ObjectUpdates/SplineManager.h"
#include "Management/ObjectUpdates/UpdateManager.h"
#include "Data/WoWPlayer.hpp"

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
    uint32 protoid;
    uint8 slot;
    uint32 amount;
};

struct CreateInfo_SkillStruct
{
    uint32 skillid;
    uint32 currentval;
    uint32 maxval;
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
    uint8 index;
    uint8 race;
    uint8 class_;
    uint32 mapId;
    uint32 zoneId;
    float positionX;
    float positionY;
    float positionZ;
    float orientation;
    uint8 strength;
    uint8 ability;
    uint8 stamina;
    uint8 intellect;
    uint8 spirit;
    uint32 health;
    uint32 mana;
    uint32 rage;
    uint32 focus;
    uint32 energy;
    uint32 attackpower;
    float mindmg;
    float maxdmg;
    uint32_t taximask[DBC_TAXI_MASK_SIZE];
    std::list<CreateInfo_ItemStruct> items;
    std::list<CreateInfo_SkillStruct> skills;
    std::list<CreateInfo_ActionBarStruct> actionbars;
    std::set<uint32> spell_list;
};

struct DamageSplit
{
    Player* caster;
    Aura* aura;
    uint32 miscVal;
    union
    {
        uint32 damage;
        float damagePCT;
    };
};

struct LoginAura
{
    uint32 id;
    uint32 dur;
    bool positive;
    uint32 charges;
};

struct FactionReputation
{
    int32 standing;
    uint8 flag;
    int32 baseStanding;
    int32 CalcStanding() { return standing - baseStanding; }
    bool Positive() { return standing >= 0; }
};

typedef std::unordered_map<uint32, uint32> PlayerInstanceMap;
class SERVER_DECL PlayerInfo
{
    public:

        ~PlayerInfo();

        uint32 guid;        // there is a filed for that
        uint32 acct;
        char* name;         // Part of Player class
        uint8_t race;       // Part of PlayerCreateInfo
        uint8_t gender;     // there is a field for that in playerbytes3
        uint8 cl;           // class? Part of PlayerCreateInfo. It is determind on player creation.
        uint32 team;        // team? there is a field for that since bc. Investigate further for what this is used.
        uint8 role;         // bg related?

        time_t lastOnline;
        uint32 lastZone;
        uint32 lastLevel;
        Group* m_Group;
        int8 subGroup;
        Mutex savedInstanceIdsLock;
        PlayerInstanceMap savedInstanceIds[NUM_INSTANCE_MODES];

        Player* m_loggedInPlayer;
        uint32 m_guild;
        uint32 guildRank;
};

struct PlayerPet
{
    std::string name;
    uint32 entry;
    uint32 xp;
    bool active;
    bool alive;
    char stablestate;
    uint32 number;
    uint32 level;
    uint32 happinessupdate;
    std::string actionbar;
    time_t reset_time;
    uint32 reset_cost;
    uint32 spellid;
    uint32 petstate;
    uint32 talentpoints;
    uint32 current_power;
    uint32 current_hp;
    uint32 current_happiness;
    uint32 renamable;
    uint32 type;
};

struct WeaponModifier
{
    uint32 wclass;
    uint32 subclass;
    float value;
};

struct classScriptOverride
{
    uint32 id;
    uint32 effect;
    uint32 aura;
    uint32 damage;
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
    uint32 CurrentValue;
    uint32 MaximumValue;
    uint32 BonusValue;
    float GetSkillUpChance();
    void Reset(uint32 Id);
};

class ArenaTeam;

struct PlayerCooldown
{
    uint32 ExpireTime;
    uint32 ItemId;
    uint32 SpellId;
};

class PlayerSpec
{
    public:

        PlayerSpec()
        {
            tp = 0;
            for (uint8 i = 0; i < PLAYER_ACTION_BUTTON_COUNT; i++)
            {
                mActions[i].Action = 0;
                mActions[i].Type = 0;
                mActions[i].Misc = 0;
            }
        }

        void SetTP(uint32 points){ tp = points; }

        uint32 GetTP() const{ return tp; }

        void Reset()
        {
            tp += static_cast<uint32>(talents.size());
            talents.clear();
        }

        void AddTalent(uint32 talentid, uint8 rankid);
        bool HasTalent(uint32 talentid, uint8 rankid)
        {
            std::map<uint32, uint8>::iterator itr = talents.find(talentid);
            if (itr != talents.end())
                return itr->second == rankid;

            return false;
        }

        std::map<uint32, uint8> talents;
#ifdef FT_GLYPHS
        uint16 glyphs[GLYPHS_COUNT];
#endif
        ActionButton mActions[PLAYER_ACTION_BUTTON_COUNT];
    private:

        uint32 tp;
};


typedef std::set<uint32>                            SpellSet;
typedef std::list<classScriptOverride*>             ScriptOverrideList;
typedef std::map<uint32, ScriptOverrideList* >      SpellOverrideMap;
typedef std::map<uint32, FactionReputation*>        ReputationMap;
typedef std::map<SpellInfo const*, std::pair<uint32, uint32> >StrikeSpellMap;
typedef std::map<uint32, OnHitSpell >               StrikeSpellDmgMap;
typedef std::map<uint32, PlayerSkill>               SkillMap;
typedef std::map<uint32, PlayerCooldown>            PlayerCooldownMap;

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
    bool hasTaxiCheat;
    bool hasCooldownCheat;
    bool hasCastTimeCheat;
    bool hasGodModeCheat;
    bool hasPowerCheat;
    bool hasFlyCheat;
    bool hasAuraStackCheat;
    bool hasItemStackCheat;
    bool hasTriggerpassCheat;
};

enum GlyphSlotMask
{
#if VERSION_STRING < Cata
    GS_MASK_1 = 0x001,
    GS_MASK_2 = 0x002,
    GS_MASK_3 = 0x008,
    GS_MASK_4 = 0x004,
    GS_MASK_5 = 0x010,
    GS_MASK_6 = 0x020
#else
    GS_MASK_1 = 0x001,
    GS_MASK_2 = 0x002,
    GS_MASK_3 = 0x040,

    GS_MASK_4 = 0x004,
    GS_MASK_5 = 0x008,
    GS_MASK_6 = 0x080,

    GS_MASK_7 = 0x010,
    GS_MASK_8 = 0x020,
    GS_MASK_9 = 0x100,

    GS_MASK_LEVEL_25 = GS_MASK_1 | GS_MASK_2 | GS_MASK_3,
    GS_MASK_LEVEL_50 = GS_MASK_4 | GS_MASK_5 | GS_MASK_6,
    GS_MASK_LEVEL_75 = GS_MASK_7 | GS_MASK_8 | GS_MASK_9
#endif
};

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
    uint32_t getVisibleItemEnchantment(uint32_t slot) const;
    void setVisibleItemEnchantment(uint32_t slot, uint32_t enchantment);
#else
    uint32_t getVisibleItemEnchantment(uint32_t slot, uint32_t pos) const;
    void setVisibleItemEnchantment(uint32_t slot, uint32_t pos, uint32_t enchantment);
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

    uint32_t getValueFromSkillInfoIndex(uint32_t index) const;
    void setValueBySkillInfoIndex(uint32_t index, uint32_t value);

    uint32_t getFreeTalentPoints() const;
#if VERSION_STRING < Cata
    void setFreeTalentPoints(uint32_t points);
#endif

    uint32_t getFreePrimaryProfessionPoints() const;
    void setFreePrimaryProfessionPoints(uint32_t points);

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
    uint32_t max_level = 60;

    std::string m_name;

    uint32_t m_team = 0;
    uint32_t m_bgTeam = 0;

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

    // This timer ticks even if the player is not eating or drinking
    uint16_t m_foodDrinkSpellVisualTimer = 5000;

#if VERSION_STRING == Classic
    // Classic doesn't have these in unit or playerdata
    float m_manaRegeneration = 0.0f;
    float m_manaRegenerationWhileCasting = 0.0f;
#endif

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Database stuff
    bool loadSpells(QueryResult* result);
    bool loadReputations(QueryResult* result);

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
    void addSpellCooldown(SpellInfo const* spellInfo, Item const* itemCaster, Spell* castingSpell = nullptr, int32_t cooldownTime = 0);
    void addGlobalCooldown(SpellInfo const* spellInfo, Spell* castingSpell, const bool sendPacket = false);
    void sendSpellCooldownPacket(SpellInfo const* spellInfo, const uint32_t duration, const bool isGcd);
    void clearCooldownForSpell(uint32_t spellId);
    void clearGlobalCooldown();
    void resetAllCooldowns();

#if VERSION_STRING >= WotLK
    // Glyphs
    // Initializes glyph slots or updates them on levelup
    void updateGlyphs();
#endif

    bool m_FirstCastAutoRepeat = false;

private:
    bool m_canDualWield2H = false;

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
    uint32_t m_talentPointsFromQuests = 0;

public:
    /////////////////////////////////////////////////////////////////////////////////////////
    // Tutorials
    uint32_t getTutorialValueById(uint8_t id);
    void setTutorialValueForId(uint8_t id, uint32_t value);

    void loadTutorials();
    void saveTutorials();

protected:
    uint32_t m_Tutorials[8] = {0};

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
    TradeData* m_TradeData = nullptr;

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

    PlayerCheat m_cheats = {false};
    float m_goLastXRotation = 0.0f;
    float m_goLastYRotation = 0.0f;

    bool m_saveAllChangesCommand = false;

    bool m_XpGainAllowed = true;

private:
    bool m_disableAppearing = false;
    bool m_disableSummoning = false;

    uint64_t m_GMSelectedGO = 0;

    uint32_t m_banned = 0;
    std::string m_banreason;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Items
    void unEquipOffHandIfRequired();
    bool hasOffHandWeapon() const;

    bool hasItem(uint32_t itemId, uint32_t amount = 1, bool checkBankAlso = false) const;

    // Player's item storage
    ItemInterface* getItemInterface() const;
private:
    ItemInterface* m_itemInterface = nullptr;

public:

    //////////////////////////////////////////////////////////////////////////////////////////
    // Difficulty
    void setDungeonDifficulty(uint8_t diff);
    uint8_t getDungeonDifficulty();

    void setRaidDifficulty(uint8_t diff);
    uint8_t getRaidDifficulty();

private:
    uint8_t m_dungeonDifficulty = 0;
    uint8_t m_raidDifficulty = 0;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Corpse
    void setCorpseData(LocationVector position, int32_t instanceId);
    LocationVector getCorpseLocation() const;
    int32_t getCorpseInstanceId() const;

    void setAllowedToCreateCorpse(bool allowed);
    bool isAllowedToCreateCorpse() const;

private:
    struct CorpseData
    {
        LocationVector location = {0,0,0,0};
        int32_t instanceId = 0;
    };
    CorpseData m_corpseData;

    bool isCorpseCreationAllowed = true;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Bind
    void setBindPoint(float x, float y, float z, uint32_t mapId, uint32_t zoneId);

    LocationVector getBindPosition() const;
    uint32_t getBindMapId() const;
    uint32_t getBindZoneId() const;

private:
    struct BindData
    {
        LocationVector location = { 0, 0, 0 };
        uint32_t mapId = 0;
        uint32_t zoneId = 0;
    };
    BindData m_bindData;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Battleground Entry
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

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Guild
    void setInvitedByGuildId(uint32_t GuildId);
    uint32_t getInvitedByGuildId() const;

    Guild* getGuild();
    bool isInGuild();

    uint32_t getGuildRankFromDB();

private:
    uint32_t m_invitedByGuildId = 0;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Group
    void setGroupInviterId(uint32_t inviterId);
    uint32_t getGroupInviterId() const;
    bool isAlreadyInvitedToGroup() const;

    bool isInGroup() const;

    Group* getGroup();
    bool isGroupLeader() const;

    int8_t getSubGroupSlot() const;

private:
    uint32_t m_GroupInviter = 0;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Quests
    void setQuestLogInSlot(QuestLogEntry* entry, uint32_t slotId);

    bool hasAnyQuestInQuestSlot() const;
    bool hasTimedQuestInQuestSlot() const;
    bool hasQuestInQuestLog(uint32_t questId) const;
    uint8_t getFreeQuestSlot() const;

    QuestLogEntry* getQuestLogByQuestId(uint32_t questId) const;
    QuestLogEntry* getQuestLogBySlotId(uint32_t slotId) const;


private:
    QuestLogEntry* m_questlog[MAX_QUEST_LOG_SIZE] = {nullptr};

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Hackdetection

    //Speed
    //Fly
    //Teleport
    //NoClip
    //Waterwalk
    //Size
    //Wallclimb
    //Itemstacking (spell/attack power stacking)
private:

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

    void setVisibleItemFields(uint32_t slot, Item* item);

private:
    uint16_t m_spellAreaUpdateTimer = 1000;
    uint16_t m_pendingPacketTimer = 100;
    uint16_t m_partyUpdateTimer = 1000;

public:
    //MIT End
    //AGPL Start

    friend class WorldSession;
    friend class Pet;

    public:

        bool Teleport(const LocationVector& vec, MapMgr* map) override;

        Player(uint32 guid);
        ~Player();
    PlayerCache* m_cache;

        void EventGroupFullUpdate();

        // Skill System
        void _AdvanceSkillLine(uint32 SkillLine, uint32 Count = 1);
        void _AddSkillLine(uint32 SkillLine, uint32 Current, uint32 Max);
        uint32 _GetSkillLineMax(uint32 SkillLine);
        uint32 _GetSkillLineCurrent(uint32 SkillLine, bool IncludeBonus = true);
        void _RemoveSkillLine(uint32 SkillLine);
        void _UpdateMaxSkillCounts();
        void _ModifySkillBonus(uint32 SkillLine, int32 Delta);
        void _ModifySkillBonusByType(uint32 SkillType, int32 Delta);
        bool _HasSkillLine(uint32 SkillLine);
        void RemoveSpellsFromLine(uint32 skill_line);
        void _RemoveAllSkills();
        void _RemoveLanguages();
        void _AddLanguages(bool All);
        void _AdvanceAllSkills(uint32 count);
        void _ModifySkillMaximum(uint32 SkillLine, uint32 NewMax);
        void _LearnSkillSpells(uint32 SkillLine, uint32 Current);

        void UpdatePvPCurrencies();
        void FillRandomBattlegroundReward(bool wonBattleground, uint32 &honorPoints, uint32 &arenaPoints);
        void ApplyRandomBattlegroundReward(bool wonBattleground);

        LfgMatch* m_lfgMatch;
        uint32 m_lfgInviterGuid;

        // Scripting
        void SendChatMessage(uint8 type, uint32 lang, const char* msg, uint32 delay = 0) override;
        void SendChatMessageToPlayer(uint8 type, uint32 lang, const char* msg, Player* plr) override;
    protected:

        void _UpdateSkillFields();

        SkillMap m_skills;

        // COOLDOWNS
        uint32 m_lastPotionId;
        PlayerCooldownMap m_cooldownMap[NUM_COOLDOWN_TYPES];
        uint32 m_globalCooldown;

        /***********************************************************************************
            AFTER THIS POINT, public and private ARE PASSED AROUND LIKE A CHEAP WH*RE :P
            Let's keeps thing clean (use encapsulation) above this line. Thanks.
        ***********************************************************************************/

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

        void RemoveItemByGuid(uint64 GUID);

        //! Okay to remove from world
        bool ok_to_remove;
        void OnLogin();//custom stuff on player login.

        void SendEquipmentSetList();
        void SendEquipmentSetSaved(uint32 setID, uint32 setGUID);
        
        void SendEmptyPetSpellList();

        void SendInitialWorldstates();

        void OutPacket(uint16 opcode, uint16 len, const void* data);
        void SendPacket(WorldPacket* packet);
        void SendMessageToSet(WorldPacket* data, bool self, bool myteam_only = false);
        void OutPacketToSet(uint16 Opcode, uint16 Len, const void* Data, bool self);


        /////////////////////////////////////////////////////////////////////////////////////////
        // static void CharChange_Looks(uint64 GUID, uint8 gender, uint8 skin, uint8 face, uint8 hairStyle, uint8 hairColor, uint8 facialHair)
        // Updates database with characters new looks, gender, and name after character customization is called at login.
        //
        // \param uint64 GUID          - GUID of the character to customized
        // \param uint8 gender         - New gender of the character customized
        // \param uint8 skin           - New skin colour of the character customized
        // \param uint8 face           - New face selection of the character customized
        // \param uint8 hairStyle      - New hair style selected for the character customized
        // \param uint8 hairColor      - New hair color selected for the character customized
        // \param uint8 facialHair     - New facial hair selected for the character customized
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        static void CharChange_Looks(uint64 GUID, uint8 gender, uint8 skin, uint8 face, uint8 hairStyle, uint8 hairColor, uint8 facialHair);


        /////////////////////////////////////////////////////////////////////////////////////////
        // static void CharChange_Language(uint64 GUID, uint8 race)
        // Updates the characters racial languages
        //
        // \param uint64 GUID         -  GUID of the character to customized
        // \param uint8 race          -  New race to be usedd for racial language change
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        static void CharChange_Language(uint64 GUID, uint8 race);


        void AddToWorld();
        void AddToWorld(MapMgr* pMapMgr);
        void RemoveFromWorld();
        bool Create(CharCreate& charCreateContent);

        void Update(unsigned long time_passed);
        void BuildFlagUpdateForNonGroupSet(uint32 index, uint32 flag);
        void BuildPetSpellList(WorldPacket & data);
        void SetAFKReason(std::string reason) { m_cache->SetStringValue(CACHE_AFK_DND_REASON, reason); };

        void GiveXP(uint32 xp, const uint64 & guid, bool allowbonus);       /// to stop rest xp being given
        void ModifyBonuses(uint32 type, int32 val, bool apply);
        void CalcExpertise();
        std::map<uint32, uint32> m_wratings;

        ArenaTeam* m_arenaTeams[NUM_ARENA_TEAM_TYPES];

        /////////////////////////////////////////////////////////////////////////////////////////
        // Taxi
        /////////////////////////////////////////////////////////////////////////////////////////
        TaxiPath* GetTaxiPath() { return m_CurrentTaxiPath; }
        bool isOnTaxi() const { return m_onTaxi; }
        const uint32 & GetTaximask(uint32_t index) const { return m_taximask[index]; }
        void LoadTaxiMask(const char* data);
        void TaxiStart(TaxiPath* path, uint32 modelid, uint32 start_node);
        void JumpToEndTaxiNode(TaxiPath* path);
        void EventDismount(uint32 money, float x, float y, float z);
        void EventTaxiInterpolate();

        void SetTaxiState(bool state) { m_onTaxi = state; }
        void SetTaximask(uint32_t index, uint32 value) { m_taximask[index] = value; }
        void SetTaxiPath(TaxiPath* path) { m_CurrentTaxiPath = path; }
        void SetTaxiPos() { m_taxi_pos_x = m_position.x; m_taxi_pos_y = m_position.y; m_taxi_pos_z = m_position.z; }
        void UnSetTaxiPos() { m_taxi_pos_x = 0; m_taxi_pos_y = 0; m_taxi_pos_z = 0; }

        // Taxi related variables
        std::vector<TaxiPath*> m_taxiPaths;
        TaxiPath* m_CurrentTaxiPath;
        uint32 taxi_model_id;
        uint32 lastNode;
        uint32 m_taxi_ride_time;
        uint32_t m_taximask[DBC_TAXI_MASK_SIZE];
        float m_taxi_pos_x;
        float m_taxi_pos_y;
        float m_taxi_pos_z;
        bool m_onTaxi;
        uint32 m_taxiMapChangeNode;

        /////////////////////////////////////////////////////////////////////////////////////////
        // Quests
        /////////////////////////////////////////////////////////////////////////////////////////
        uint32 GetQuestSharer() { return m_questSharer; }

        void SetQuestSharer(uint32 guid) { m_questSharer = guid; }

        void PushToRemovedQuests(uint32 questid) { m_removequests.insert(questid);}
        void PushToFinishedDailies(uint32 questid) { DailyMutex.Acquire(); m_finishedDailies.insert(questid); DailyMutex.Release();}
        bool HasFinishedDaily(uint32 questid) { return (m_finishedDailies.find(questid) == m_finishedDailies.end() ? false : true); }
        void AddToFinishedQuests(uint32 quest_id);
        void AreaExploredOrEventHappens(uint32 questId);   // scriptdev2

        bool HasFinishedQuest(uint32 quest_id);

        void EventTimedQuestExpire(uint32 questid);

        //////////////////////////////////////////////////////////////////////////////////////////
        // void ClearQuest(uint32 id)
        // Clears the finished status of a quest
        //
        // \param uint32 id  -  Identifier of the quest
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void ClearQuest(uint32 id);

        bool GetQuestRewardStatus(uint32 quest_id);
        bool HasQuestForItem(uint32 itemid);
        bool HasQuestSpell(uint32 spellid);
        void RemoveQuestSpell(uint32 spellid);
        bool HasQuestMob(uint32 entry);
        
        void RemoveQuestMob(uint32 entry);
        void AddQuestKill(uint32 questid, uint8 reqid, uint32 delay = 0);

        /////////////////////////////////////////////////////////////////////////////////////////
        // void AcceptQuest(uint64 guid, uint32 quest_id)
        // Checks if the quest is acceptable from that questgiver and accepts it.
        //
        // \param uin64 guid      -  guid of the questgiver
        // \param uint32 quest_id -  id of the quest
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void AcceptQuest(uint64 guid, uint32 quest_id);

        //Quest related variables

        std::set<uint32> m_removequests;
        std::set<uint32> m_finishedQuests;
        Mutex DailyMutex;
        std::set<uint32> m_finishedDailies;
        uint32 m_questSharer;
        std::set<uint32> quest_spells;
        std::set<uint32> quest_mobs;

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

        const uint64 & GetSelection() const { return m_curSelection; }
        const uint64 & GetTarget() const { return m_curTarget; }
        void SetSelection(const uint64 & guid) { m_curSelection = guid; }
        void SetTarget(const uint64 & guid) { m_curTarget = guid; }

        /////////////////////////////////////////////////////////////////////////////////////////
        // Spells
        /////////////////////////////////////////////////////////////////////////////////////////
        bool HasSpell(uint32 spell);
        bool HasDeletedSpell(uint32 spell);
        void smsg_InitialSpells();
        void addSpell(uint32 spell_idy);
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
        /////////////////////////////////////////////////////////////////////////////////////////
        // Reputation
        /////////////////////////////////////////////////////////////////////////////////////////
        void ModStanding(uint32 Faction, int32 Value);
        int32 GetStanding(uint32 Faction);
        int32 GetBaseStanding(uint32 Faction);
        void SetStanding(uint32 Faction, int32 Value);
        void SetAtWar(uint32 Faction, bool Set);

        Standing GetStandingRank(uint32 Faction);
        bool IsHostileBasedOnReputation(DBC::Structures::FactionEntry const* dbc);
        void UpdateInrangeSetsBasedOnReputation();
        void Reputation_OnKilledUnit(Unit* pUnit, bool InnerLoop);
        void Reputation_OnTalk(DBC::Structures::FactionEntry const* dbc);
        static Standing GetReputationRankFromStanding(int32 Standing_);
        void SetFactionInactive(uint32 faction, bool set);
        bool AddNewFaction(DBC::Structures::FactionEntry const* dbc, int32 standing, bool base);
        void OnModStanding(DBC::Structures::FactionEntry const* dbc, FactionReputation* rep);
        uint32 GetExaltedCount();

        // Factions
        void smsg_InitialFactions();
        uint32 GetInitialFactionId();
        // factions variables
        int32 pctReputationMod;

        /////////////////////////////////////////////////////////////////////////////////////////
        // PVP
        /////////////////////////////////////////////////////////////////////////////////////////
        uint32 GetMaxPersonalRating();

        bool HasTitle(RankTitles title)
        {
#if VERSION_STRING > Classic
            const auto index = static_cast<uint8_t>(title / 32);

            return (getKnownTitles(index) & 1ULL << static_cast<uint64_t>((title % 32))) != 0;
#else
            return false;
#endif
        }
        void SetKnownTitle(RankTitles title, bool set);
        void SendAvailSpells(DBC::Structures::SpellShapeshiftFormEntry const* shapeshift_form, bool active);

        /////////////////////////////////////////////////////////////////////////////////////////
        // Duel
        /////////////////////////////////////////////////////////////////////////////////////////
        void RequestDuel(Player* pTarget);
        void DuelBoundaryTest();
        void EndDuel(uint8 WinCondition);
        void DuelCountdown();
        void cancelDuel();
        void SetDuelStatus(uint8 status) { m_duelStatus = status; }
        uint8 GetDuelStatus() { return m_duelStatus; }
        void SetDuelState(uint8 state) { m_duelState = state; }
        uint8 GetDuelState() { return m_duelState; }
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
        uint32 GeneratePetNumber();
        void RemovePlayerPet(uint32 pet_number);
        void AddPlayerPet(PlayerPet* pet, uint32 index) { m_Pets[index] = pet; }
        PlayerPet* GetPlayerPet(uint32 idx)
        {
            std::map<uint32, PlayerPet*>::iterator itr = m_Pets.find(idx);
            if (itr != m_Pets.end())
                return itr->second;

            return nullptr;
        }
        void SpawnPet(uint32 pet_number);
        void SpawnActivePet();
        void DismissActivePets();
        uint8 GetPetCount() { return (uint8)m_Pets.size(); }
        void SetStableSlotCount(uint8 count) { m_StableSlotCount = count; }
        uint8 GetStableSlotCount() { return m_StableSlotCount; }

        uint32 GetUnstabledPetNumber();
        void EventSummonPet(Pet* new_pet);   // if we charmed or simply summoned a pet, this function should get called
        void EventDismissPet();              // if pet/charm died or whatever happened we should call this function

        /////////////////////////////////////////////////////////////////////////////////////////
        // Item Interface
        /////////////////////////////////////////////////////////////////////////////////////////
        void ApplyItemMods(Item* item, int16 slot, bool apply, bool justdrokedown = false) { _ApplyItemMods(item, slot, apply, justdrokedown); }


        /////////////////////////////////////////////////////////////////////////////////////////
        // Loot
        /////////////////////////////////////////////////////////////////////////////////////////
        const uint64 & GetLootGUID() const { return m_lootGuid; }
        void SetLootGUID(const uint64 & guid) { m_lootGuid = guid; }
        void SendLoot(uint64 guid, uint8 loot_type, uint32 mapid);
        void SendLootUpdate(Object* o);
        void TagUnit(Object* o);
        
        // loot variables
        uint64 m_lootGuid;
        uint64 m_currentLoot;
        bool bShouldHaveLootableOnCorpse;

        /////////////////////////////////////////////////////////////////////////////////////////
        // World Session
        /////////////////////////////////////////////////////////////////////////////////////////
        WorldSession* GetSession() const { return m_session; }
        void SetSession(WorldSession* s) { m_session = s; }

        void SendDelayedPacket(WorldPacket* data, bool bDeleteOnSend);
        float offhand_dmg_mod;

        // Talents
        // These functions build a specific type of A9 packet
        uint32 buildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target);
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

        bool LoadSkills(QueryResult* result);
        bool SaveSkills(bool NewCharacter, QueryBuffer* buf);

        bool m_FirstLogin;

        /////////////////////////////////////////////////////////////////////////////////////////
        // Drunk system
        /////////////////////////////////////////////////////////////////////////////////////////
        void SetDrunkValue(uint16 newDrunkValue, uint32 itemid = 0);
        uint16 GetDrunkValue() const { return m_drunk; }
        static DrunkenState GetDrunkenstateByValue(uint16 value);
        void HandleSobering();

        uint32 m_drunkTimer;
        uint16 m_drunk;

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
        void RepopAtGraveyard(float ox, float oy, float oz, uint32 mapid);

        uint64 m_resurrecter;

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
        float GetSkillUpChance(uint32 id);

        float SpellHasteRatingBonus;
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

        uint32 CalcTalentResetCost(uint32 resetnum);
        
        uint32 GetTalentResetTimes() { return m_talentresettimes; }
        void SetTalentResetTimes(uint32 value) { m_talentresettimes = value; }

        void SetPlayerStatus(uint8 pStatus) { m_status = pStatus; }
    uint8 GetPlayerStatus() const;

        // Showing Units WayPoints
        AIInterface* waypointunit;

        uint32 m_nextSave;

        // Rest
        uint32 SubtractRestXP(uint32 amount);
        void AddCalculatedRestXP(uint32 seconds);
        void ApplyPlayerRestState(bool apply);
        void UpdateRestState();

        int m_lifetapbonus;
        bool m_requiresNoAmmo;

        bool m_bUnlimitedBreath;
        uint32 m_UnderwaterTime;
        uint32 m_UnderwaterState;
        // Visible objects
        bool IsVisible(uint64 pObj) { return !(m_visibleObjects.find(pObj) == m_visibleObjects.end()); }
        void addToInRangeObjects(Object* pObj);
        void onRemoveInRangeObject(Object* pObj);
        void clearInRangeSets();
        void AddVisibleObject(uint64 pObj) { m_visibleObjects.insert(pObj); }
        void RemoveVisibleObject(uint64 pObj) { m_visibleObjects.erase(pObj); }
        void RemoveVisibleObject(std::set< uint64 >::iterator itr) { m_visibleObjects.erase(itr); }
        std::set< uint64 >::iterator FindVisible(uint64 obj) { return m_visibleObjects.find(obj); }
        void RemoveIfVisible(uint64 obj);

        // Misc
        void EventCannibalize(uint32 amount);
        bool m_AllowAreaTriggerPort;
        void EventAllowTiggerPort(bool enable);
        uint32 m_modblockabsorbvalue;
        uint32 m_modblockvaluefromspells;
        void SendInitialLogonPackets();
        void Reset_Spells();
        // Battlegrounds xD
        CBattleground* m_bg;
        CBattleground* m_pendingBattleground;
        
        bool m_bgHasFlag;
        bool m_bgIsQueued;
        uint32 m_bgQueueType;
        uint32 m_bgQueueInstanceId;

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
        uint32 FlatResistanceModifierPos[TOTAL_SPELL_SCHOOLS];
        uint32 FlatResistanceModifierNeg[TOTAL_SPELL_SCHOOLS];
        uint32 BaseResistanceModPctPos[TOTAL_SPELL_SCHOOLS];
        uint32 BaseResistanceModPctNeg[TOTAL_SPELL_SCHOOLS];
        uint32 ResistanceModPctPos[TOTAL_SPELL_SCHOOLS];
        uint32 ResistanceModPctNeg[TOTAL_SPELL_SCHOOLS];
        float m_resist_critical[2];             // when we are a victim we can have talents to decrease chance for critical hit. This is a negative value and it's added to critchances
        float m_resist_hit[2];                  // 0 = melee; 1= ranged;
        int32 m_resist_hit_spell[TOTAL_SPELL_SCHOOLS]; // spell resist per school
        uint32 m_modphyscritdmgPCT;
        uint32 m_RootedCritChanceBonus;         // Class Script Override: Shatter
        uint32 m_IncreaseDmgSnaredSlowed;

        // SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
        uint32 m_ModInterrMRegenPCT;
        // SPELL_AURA_MOD_POWER_REGEN
        int32 m_ModInterrMRegen;
        // SPELL_AURA_REGEN_MANA_STAT_PCT
        int32_t m_modManaRegenFromStat[STAT_COUNT];
        float m_RegenManaOnSpellResist;
        uint32 m_casted_amount[TOTAL_SPELL_SCHOOLS];   // Last casted spells amounts. Need for some spells. Like Ignite etc. DOesn't count HoTs and DoTs. Only directs

        uint32 FlatStatModPos[5];
        uint32 FlatStatModNeg[5];
        uint32 StatModPctPos[5];
        uint32 StatModPctNeg[5];
        uint32 TotalStatModPctPos[5];
        uint32 TotalStatModPctNeg[5];
        int32 IncreaseDamageByType[12];         // mod dmg by creature type
        float IncreaseDamageByTypePCT[12];
        float IncreaseCricticalByTypePCT[12];
        int32 DetectedRange;
        float PctIgnoreRegenModifier;
        uint32 m_retainedrage;

        uint32* GetPlayedtime() { return m_playedtime; };
        void CalcStat(uint8_t t);
        float CalcRating(PlayerCombatRating t);
        void RegenerateHealth(bool inCombat);

        uint32 SoulStone;
        uint32 SoulStoneReceiver;
        void removeSoulStone();

        uint32 GetSoulStoneReceiver() { return SoulStoneReceiver; }
        void SetSoulStoneReceiver(uint32 StoneGUID) { SoulStoneReceiver = StoneGUID; }
        uint32 GetSoulStone() { return SoulStone; }
        void SetSoulStone(uint32 StoneID) { SoulStone = StoneID; }

        uint64 misdirectionTarget;

        uint64 GetMisdirectionTarget() { return misdirectionTarget; }
        void SetMisdirectionTarget(uint64 PlayerGUID) { misdirectionTarget = PlayerGUID; }

        bool bReincarnation;

        std::map<uint32, WeaponModifier> damagedone;
        std::map<uint32, WeaponModifier> tocritchance;
        bool cannibalize;
        uint8 cannibalizeCount;
        int32 rageFromDamageDealt;
        int32 rageFromDamageTaken;


        void _Relocate(uint32 mapid, const LocationVector & v, bool sendpending, bool force_new_world, uint32 instance_id);

        void AddItemsToWorld();
        void RemoveItemsFromWorld();
        void UpdateKnownCurrencies(uint32 itemId, bool apply);

        uint32 m_MountSpellId;
        uint32 mountvehicleid;

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


        void addVehicleComponent(uint32 creature_entry, uint32 vehicleid);

        void removeVehicleComponent();

        bool bHasBindDialogOpen;
        uint32 TrackingSpell;
        void _EventCharmAttack();
        void _Kick();
        void Kick(uint32 delay = 0);
        void SoftDisconnect();
        uint32 m_KickDelay;
        uint64 m_CurrentCharm;

        Object* GetSummonedObject() { return m_SummonedObject; };
        void SetSummonedObject(Object* t_SummonedObject) { m_SummonedObject = t_SummonedObject; };

        void ClearCooldownsOnLine(uint32 skill_line, uint32 called_from);

        void Phase(uint8 command = PHASE_SET, uint32 newphase = 1);

        void ProcessPendingUpdates();
        bool CompressAndSendUpdateBuffer(uint32 size, const uint8* update_buffer);

        uint32 GetArmorProficiency() { return armor_proficiency; }
        uint32 GetWeaponProficiency() { return weapon_proficiency; }

        bool ExitInstance();
        void SaveEntryPoint(uint32 mapId);

        // Cheat section
        void SpeedCheatDelay(uint32 ms_delay);
        void SpeedCheatReset();

        void ZoneUpdate(uint32 ZoneId);
        void UpdateChannels(uint16 AreaID);
        uint32 GetAreaID() { return m_AreaID; }
        void SetAreaID(uint32 area) { m_AreaID = area; }
        bool IsInCity();

        // Instance IDs
        uint32 GetPersistentInstanceId(uint32 mapId, uint8 difficulty)
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
        void SetPersistentInstanceId(uint32 mapId, uint8 difficulty, uint32 instanceId);

    public:

        bool m_Autojoin;
        bool m_AutoAddMem;
        void SendMirrorTimer(MirrorTimerTypes Type, uint32 max, uint32 current, int32 regen);
        
        BGScore m_bgScore;
        
        void UpdateChanceFields();
        //Honor Variables
        time_t m_fallDisabledUntil;
        uint32 m_honorToday;
        uint32 m_honorYesterday;

        void RolloverHonor();
        uint32 m_honorPoints;
        uint32 m_honorRolloverTime;
        uint32 m_killsToday;
        uint32 m_killsYesterday;
        uint32 m_killsLifetime;
        uint32 m_arenaPoints;
        uint32 m_honorless;
        uint32 m_lastSeenWeather;
        std::set<Object*> m_visibleFarsightObjects;
        void EventTeleport(uint32 mapid, float x, float y, float z);
        void EventTeleportTaxi(uint32 mapid, float x, float y, float z);
        void BroadcastMessage(const char* Format, ...);
        std::map<uint32, std::set<uint32> > SummonSpells;
        std::map<uint32, std::map<SpellInfo const*, uint16>*> PetSpells;
        void AddSummonSpell(uint32 Entry, uint32 SpellID);
        void RemoveSummonSpell(uint32 Entry, uint32 SpellID);
        std::set<uint32>* GetSummonSpells(uint32 Entry);
        uint32 m_UnderwaterMaxTime;
        uint32 m_UnderwaterLastDmg;
        
        
        uint32 m_resurrectHealth, m_resurrectMana;
        uint32 m_resurrectInstanceID, m_resurrectMapId;
        LocationVector m_resurrectPosition;
        bool blinked;
        uint32 m_explorationTimer;
        // DBC stuff
        DBC::Structures::ChrRacesEntry const* myRace;
        DBC::Structures::ChrClassesEntry const* myClass;
        Creature* linkTarget;

        bool SafeTeleport(uint32 MapID, uint32 InstanceID, float X, float Y, float Z, float O);
        bool SafeTeleport(uint32 MapID, uint32 InstanceID, const LocationVector & vec);
        void SafeTeleport(MapMgr* mgr, const LocationVector & vec);
        void EjectFromInstance();
        bool raidgrouponlysent;

        void EventSafeTeleport(uint32 MapID, uint32 InstanceID, const LocationVector vec)
        {
            SafeTeleport(MapID, InstanceID, vec);
        }

        // Hack fix here!
        void ForceZoneUpdate();

        bool HasAreaExplored(::DBC::Structures::AreaTableEntry const*);
        bool HasOverlayUncovered(uint32 overlayID);

        /////////////////////////////////////////////////////////////////////////////////////////
        //  PVP Stuff
        /////////////////////////////////////////////////////////////////////////////////////////
        uint32 m_pvpTimer;


        /////////////////////////////////////////////////////////////////////////////////////////
        // EASY FUNCTIONS - MISC
        /////////////////////////////////////////////////////////////////////////////////////////

        //\todo fix this
        void ModPrimaryProfessionPoints(int32 amt)
        {
#if VERSION_STRING < Cata
            int32_t value = getFreePrimaryProfessionPoints();
            value += amt;

            if (value < 0)
                value = 0;

            setFreePrimaryProfessionPoints(value);

#else
            if (amt == 0) { return; }
#endif
        }

        void AddHonor(uint32 honorPoints, bool sendUpdate);
        void UpdateHonor();

        void AddArenaPoints(uint32 arenaPoints, bool sendUpdate);
        void UpdateArenaPoints();

#if VERSION_STRING > TBC
        uint32 GetGlyph(uint32 spec, uint32 slot) const { return m_specs[spec].glyphs[slot]; }
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
        // void HandleSpellLoot(uint32 itemid)
        // Generates loot for the spell loot item (clams for example) , then adds the generated loot to the Player
        //
        // \param uint32 itemid   -  unique numerical identifier of the item the Player is looting
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void HandleSpellLoot(uint32 itemid);

        void Die(Unit* pAttacker, uint32 damage, uint32 spellid) override;
        void HandleKnockback(Object* caster, float horizontal, float vertical) override;

        uint32 LastHonorResetTime() const { return m_lastHonorResetTime; }
        void LastHonorResetTime(uint32 val) { m_lastHonorResetTime = val; }
        uint32 OnlineTime;
        bool tutorialsDirty;
        LevelInfo* lvlinfo;
        uint32 load_health;
        uint32 load_mana;
        void CompleteLoading();
        void OnPushToWorld() override;
        void OnPrePushToWorld() override;
        void OnWorldPortAck();
        uint32 m_TeleportState;
        bool m_beingPushed;
        bool CanSignCharter(Charter* charter, Player* requester);
        Charter* m_charters[NUM_CHARTER_TYPES];
        uint32 flying_aura;
        bool resend_speed;
        uint32 login_flags;

        FactionReputation* reputationByListId[128];

        uint64 m_comboTarget;
        int8 m_comboPoints;
        bool m_retainComboPoints;
        int8 m_spellcomboPoints;        // rogue talent Ruthlessness will change combopoints while consuming them. solutions 1) add post cast prochandling, 2) delay adding the CP
        void UpdateComboPoints();

        void AddComboPoints(uint64 target, int8 count);

        void NullComboPoints() { if (!m_retainComboPoints) { m_comboTarget = 0; m_comboPoints = 0; m_spellcomboPoints = 0; } UpdateComboPoints(); }
        uint32 m_speedChangeCounter;

        void SendAreaTriggerMessage(const char* message, ...);

        // Water level related stuff (they are public because they need to be accessed fast)
        // Nose level of the character (needed for proper breathing)
        float m_noseLevel;

        void RemoteRevive();

        LocationVector m_last_group_position;
        int32 m_rap_mod_pct;
        void SummonRequest(uint32 Requestor, uint32 ZoneID, uint32 MapID, uint32 InstanceID, const LocationVector & Position);

        bool m_deathVision;
        SpellInfo const* last_heal_spell;
        LocationVector m_sentTeleportPosition;

        bool InBattleground() const { return m_bgQueueInstanceId != 0; }
        bool InBattlegroundQueue() const { return m_bgIsQueued != 0; }

        void RemoveFromBattlegroundQueue();
        void FullHPMP();
        void RemoveTempEnchantsOnArena();
        uint32 m_arenateaminviteguid;

        void ResetTimeSync();
        void SendTimeSync();

        /////////////////////////////////////////////////////////////////////////////////////////
        // Spell Packet wrapper Please keep this separated
        /////////////////////////////////////////////////////////////////////////////////////////
        void SendWorldStateUpdate(uint32 WorldState, uint32 Value);

        /////////////////////////////////////////////////////////////////////////////////////////
        // End of SpellPacket wrapper
        /////////////////////////////////////////////////////////////////////////////////////////

        Mailbox m_mailBox;
        uint64 m_areaSpiritHealer_guid;
        bool m_finishingmovesdodge;

        bool IsAttacking() { return m_attacking; }

        static void InitVisibleUpdateBits();
        static UpdateMask m_visibleUpdateMask;

        void CopyAndSendDelayedPacket(WorldPacket* data);
        void PartLFGChannel();
        SpeedCheatDetector* SDetector;

    protected:

        LocationVector m_summonPos;
        uint32 m_summonInstanceId;
        uint32 m_summonMapId;
        uint32 m_summoner;

        void _SetCreateBits(UpdateMask* updateMask, Player* target) const;
        void _SetUpdateBits(UpdateMask* updateMask, Player* target) const;

        void _SaveQuestLogEntry(QueryBuffer* buf);
        void _LoadQuestLogEntry(QueryResult* result);

        void _LoadPet(QueryResult* result);
        void _LoadPetSpells(QueryResult* result);
        void _SavePet(QueryBuffer* buf);
        void _SavePetSpells(QueryBuffer* buf);
        void _ApplyItemMods(Item* item, int16 slot, bool apply, bool justdrokedown = false, bool skip_stat_apply = false);
        void _EventAttack(bool offhand);
        void _EventExploration();
        void CastSpellArea();

        // Water level related stuff
        void SetNoseLevel();

        /////////////////////////////////////////////////////////////////////////////////////////
        // Player Class systems, info and misc things
        /////////////////////////////////////////////////////////////////////////////////////////
        PlayerCreateInfo const* info;
        uint32 m_AttackMsgTimer;        // "too far away" and "wrong facing" timer
        bool m_attacking;
        
        uint32 m_AreaID;
        std::list<Pet*>  m_Summons;
        uint32 m_PetNumberMax;
        std::map<uint32, PlayerPet*> m_Pets;

        uint32 m_invitersGuid;      // It is guild inviters guid ,0 when its not used

        std::list<ItemSet> m_itemsets;
        //Duel
        uint32 m_duelCountdownTimer;
        uint8 m_duelStatus;
        uint8 m_duelState;
        // Rested State Stuff
        uint32 m_timeLogoff;
        // Played time
        // 0 = played on level
        // 1 = played total
        // 2 = played session
        uint32 m_playedtime[3];


        uint8 m_isResting;
        uint8 m_restState;
        uint32 m_restAmount;
        //combat mods
        float m_blockfromspellPCT;
        float m_critfromspell;
        float m_spellcritfromspell;
        float m_hitfromspell;
        //stats mods
        uint32 m_healthfromspell;
        uint32 m_manafromspell;
        uint32 m_healthfromitems;
        uint32 m_manafromitems;

        uint32 armor_proficiency;
        uint32 weapon_proficiency;
        // Talents
        uint32 m_talentresettimes;
        // STATUS
        uint8 m_status;
        // guid of current target
        uint64 m_curTarget;
        // guid of current selection
        uint64 m_curSelection;
        // Raid
        uint8 m_targetIcon;
        // Player Reputation
        ReputationMap m_reputation;
        // Pointer to this char's game client
        WorldSession* m_session;
        // Channels
        std::set<Channel*> m_channels;
        // Visible objects
        std::set<uint64> m_visibleObjects;
        // Groups/Raids
        uint8 m_StableSlotCount;

        // Fishing related
        Object* m_SummonedObject;

        uint32 m_lastHonorResetTime;
        uint32 _fields[getSizeOfStructure(WoWPlayer)];
        int hearth_of_wild_pct;        // druid hearth of wild talent used on shapeshifting. We either know what is last talent level or memo on learn

        uint32 m_indoorCheckTimer;
        void RemovePendingPlayer();

    public:

        void addDeletedSpell(uint32 id) { mDeletedSpells.insert(id); }

        std::map<uint32, uint32> m_forcedReactions;

        bool m_passOnLoot;
        uint32 m_tradeSequence;
        bool m_changingMaps;

        /////////////////////////////////////////////////////////////////////////////////////////
        // SOCIAL
        /////////////////////////////////////////////////////////////////////////////////////////
    private:

        void Social_SendFriendList(uint32 flag);

        void Social_AddFriend(const char* name, const char* note);
        void Social_RemoveFriend(uint32 guid);

        void Social_AddIgnore(const char* name);
        void Social_RemoveIgnore(uint32 guid);

        void Social_SetNote(uint32 guid, const char* note);

    public:

        bool Social_IsIgnoring(PlayerInfo* m_info);
        bool Social_IsIgnoring(uint32 guid);

        void Social_TellFriendsOnline();
        void Social_TellFriendsOffline();

        /////////////////////////////////////////////////////////////////////////////////////////
        // end social
        /////////////////////////////////////////////////////////////////////////////////////////

        uint32 m_outStealthDamageBonusPct;
        uint32 m_outStealthDamageBonusPeriod;
        uint32 m_outStealthDamageBonusTimer;

        //\todo sort out where all the publics and privates go. This will do for now..

    private:

        PlayerInfo* m_playerInfo;

        bool resettalents;
        std::list< Item* > m_GarbageItems;

        void RemoveGarbageItems();

        uint32 ChampioningFactionID;

        uint32 m_timeSyncCounter;
        uint32 m_timeSyncTimer;
        uint32 m_timeSyncClient;
        uint32 m_timeSyncServer;

    public:

        void SetChampioningFaction(uint32 f) { ChampioningFactionID = f; }
        void AddGarbageItem(Item* it);
        uint32 CheckDamageLimits(uint32 dmg, uint32 spellid);

        PlayerInfo* getPlayerInfo() const { return m_playerInfo; }

        void LoadFieldsFromString(const char* string, uint16 firstField, uint32 fieldsNum);

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
        uint16 m_maxTalentPoints;
        uint8 m_talentSpecsCount;
        uint8 m_talentActiveSpec;
#if VERSION_STRING >= Cata
        uint32 m_FirstTalentTreeLock;
#endif

#ifdef FT_DUAL_SPEC
    PlayerSpec m_specs[MAX_SPEC_COUNT];
#else
    PlayerSpec m_spec;
#endif

    PlayerSpec& getActiveSpec();

        uint8 m_roles;
        uint32 GroupUpdateFlags;

    public:

        void SendUpdateDataToSet(ByteBuffer* groupbuf, ByteBuffer* nongroupbuf, bool sendtoself);

        bool CanBuyAt(MySQLStructure::VendorRestrictions const* vendor);
        bool CanTrainAt(Trainer*);

        void SetRoles(uint8 role) { m_roles = role; }
        uint8 GetRoles() { return m_roles; }

        uint32 GetGroupUpdateFlags() { return GroupUpdateFlags; }
        void SetGroupUpdateFlags(uint32 flags);
        void AddGroupUpdateFlag(uint32 flag);
        uint16 GetGroupStatus();
        void SendUpdateToOutOfRangeGroupMembers();

        void SendTeleportPacket(float x, float y, float z, float o);
        void SendTeleportAckPacket(float x, float y, float z, float o);

        void SendCinematicCamera(uint32 id);

        void SetMover(Unit* target);

        // AGPL End
};
