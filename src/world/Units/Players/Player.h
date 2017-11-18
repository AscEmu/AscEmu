/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
#include "Server/Packets/Handlers/PlayerCache.h"
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

#if VERSION_STRING == Cata
#include "GameCata/Management/Guild.h"
#endif


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
struct GuildRank;
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

#pragma pack(push,1)
struct ActionButton
{
    uint16 Action;
    uint8 Type;
    uint8 Misc;
};
#pragma pack(pop)

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

struct CreateInfo_ActionBarStruct
{
    uint32 button;
    uint32 action;
    uint32 type;
    uint32 misc;
};

struct PlayerCreateInfo
{
    uint8 index;
    uint8 race;
    uint32 factiontemplate;
    uint8 class_;
    uint32 mapId;
    uint32 zoneId;
    float positionX;
    float positionY;
    float positionZ;
    float orientation;
    uint16 displayId;
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
    uint32 introid;
    uint32 taximask[12];
    std::list<CreateInfo_ItemStruct> items;
    std::list<CreateInfo_SkillStruct> skills;
    std::list<CreateInfo_ActionBarStruct> actionbars;
    std::set<uint32> spell_list;
    //uint32 item[10];
    //uint8 item_slot[10];
    //uint16 spell[10];
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

        uint32 guid;
        uint32 acct;
        char* name;
        uint32 race;
        uint32 gender;
        uint32 cl;
        uint32 team;
        uint8 role;

        time_t lastOnline;
        uint32 lastZone;
        uint32 lastLevel;
        Group* m_Group;
        int8 subGroup;
        Mutex savedInstanceIdsLock;
        PlayerInstanceMap savedInstanceIds[NUM_INSTANCE_MODES];

        Player* m_loggedInPlayer;
#if VERSION_STRING != Cata
        Guild* guild;
        GuildRank* guildRank;
        GuildMember* guildMember;
#else
        uint32 m_guild;
        uint32 guildRank;
#endif
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
    uint32 wclass;
    uint32 subclass;
    float value;
};

struct PetActionBar
{
    uint32 spell[10];
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

enum SPELL_INDEX2
{
    SPELL_TYPE2_PALADIN_AURA        = 1,
    SPELL_TYPE3_DEATH_KNIGHT_AURA   = 1
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
            {
                return itr->second == rankid;
            }
            else
            {
                return false;
            }
        }

        std::map<uint32, uint8> talents;
        uint16 glyphs[GLYPHS_COUNT];
        ActionButton mActions[PLAYER_ACTION_BUTTON_COUNT];
    private:

        uint32 tp;
};


typedef std::set<uint32>                            SpellSet;
typedef std::list<classScriptOverride*>             ScriptOverrideList;
typedef std::set<uint32>                            SaveSet;
typedef std::map<uint64, ByteBuffer*>               SplineMap;
typedef std::map<uint32, ScriptOverrideList* >      SpellOverrideMap;
typedef std::map<uint32, uint32>                    SpellOverrideExtraAuraMap;
typedef std::map<uint32, FactionReputation*>        ReputationMap;
typedef std::map<uint32, uint64>                    SoloSpells;
typedef std::map<SpellInfo*, std::pair<uint32, uint32> >StrikeSpellMap;
typedef std::map<uint32, OnHitSpell >               StrikeSpellDmgMap;
typedef std::map<uint32, PlayerSkill>               SkillMap;
typedef std::set<Player**>                          ReferenceSet;
typedef std::map<uint32, PlayerCooldown>            PlayerCooldownMap;

// AGPL End

// MIT Start
#if VERSION_STRING == Cata
class TradeData
{
    void updateTrade(bool for_trader = true);

    Player* m_player;
    Player* m_tradeTarget;
    bool m_accepted;
    bool m_acceptProccess;
    uint64_t m_money;
    uint32_t m_spell;
    uint64_t m_spellCastItem;
    uint64_t m_items[TRADE_SLOT_COUNT];

    public:

        TradeData(Player* player, Player* trader) : m_player(player), m_tradeTarget(trader), m_accepted(false), m_acceptProccess(false), m_money(0), m_spell(0) {}

        Player* getTradeTarget() const { return m_tradeTarget; }
        TradeData* getTargetTradeData() const;

        Item* getTradeItem(TradeSlots slot) const;
        bool hasTradeItem(uint64 item_guid) const;

        uint32_t getSpell() const { return m_spell; }
        Item* getSpellCastItem() const;
        bool hasSpellCastItem() const { return !m_spellCastItem; }

        uint64_t getMoney() const { return m_money; }

        void setAccepted(bool state, bool send_both = false);
        bool isAccepted() const { return m_accepted; }

        void setInAcceptProcess(bool state) { m_acceptProccess = state; }
        bool isInAcceptProcess() const { return m_acceptProccess; }

        void setItem(TradeSlots slot, Item* item);
        void setSpell(uint32_t spell_id, Item* cast_item = nullptr);
        void setMoney(uint64_t money);

};
#endif

class SERVER_DECL Player : public Unit
{

public:

    //////////////////////////////////////////////////////////////////////////////////////////
    // Movement
    void sendForceMovePacket(UnitSpeedType speed_type, float speed);
    void sendMoveSetSpeedPaket(UnitSpeedType speed_type, float speed);

    void handleFall(MovementInfo const& movement_info);
    bool isPlayerJumping(MovementInfo const& movement_info, uint16_t opcode);
    void handleBreathing(MovementInfo const& movement_info, WorldSession* session);
    void handleAuraInterruptForMovementFlags(MovementInfo const& movement_info);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spells
    bool isSpellFitByClassAndRace(uint32_t spell_id);
#if VERSION_STRING == Cata
    uint32_t getFreePrimaryProfessionPoints() const { return getUInt32Value(PLAYER_CHARACTER_POINTS); }
#endif
    void updateAutoRepeatSpell();
    bool m_FirstCastAutoRepeat;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Auction
    void sendAuctionCommandResult(Auction* auction, uint32_t Action, uint32_t ErrorCode, uint32_t bidError = 0);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Trade
#if VERSION_STRING == Cata
private:

    TradeData* m_TradeData;

public:

    Player* getTradeTarget() const { return m_TradeData ? m_TradeData->getTradeTarget() : nullptr; }
    TradeData* getTradeData() const { return m_TradeData; }
    void cancelTrade(bool sendback);

#endif
    //////////////////////////////////////////////////////////////////////////////////////////
    // Messages
public:

    void sendReportToGmMessage(std::string playerName, std::string damageLog);

#if VERSION_STRING == Cata
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
    // Misc
    bool isGMFlagSet();

    void sendMovie(uint32_t movieId);
    //MIT End
    //AGPL Start

    friend class WorldSession;
    friend class Pet;

    public:

        bool Teleport(const LocationVector& vec, MapMgr* map) override;

        Player(uint32 guid);
        ~Player();
    PlayerCache* m_cache;

        virtual bool IsMage() { return false; }
        virtual bool IsDeathKnight() { return false; }
        virtual bool IsPriest() { return false; }
        virtual bool IsRogue() { return false; }
        virtual bool IsShaman() { return false; }
        virtual bool IsHunter() { return false; }
        virtual bool IsWarlock() { return false; }
        virtual bool IsWarrior() { return false; }
        virtual bool IsPaladin() { return false; }
        virtual bool IsDruid() { return false; }

        void HandleUpdateFieldChanged(uint32 index)
        {
            if (index == PLAYER_FLAGS)
                m_cache->SetUInt32Value(CACHE_PLAYER_FLAGS, getUInt32Value(PLAYER_FLAGS));
        }

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

        // Summon and Appear Blocking
        void DisableSummon(bool disable) { disableSummon = disable; }
        bool IsSummonDisabled() { return disableSummon; }
        void DisableAppear(bool disable) { disableAppear = disable; }
        bool IsAppearDisabled() { return disableAppear; }

        // Scripting
        void SendChatMessage(uint8 type, uint32 lang, const char* msg, uint32 delay = 0) override;
        void SendChatMessageToPlayer(uint8 type, uint32 lang, const char* msg, Player* plr) override;
    protected:

        void _UpdateSkillFields();

        SkillMap m_skills;

        // Summon and Appear Blocking
        bool disableAppear;
        bool disableSummon;

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
        void Cooldown_AddStart(SpellInfo* pSpell);
        void Cooldown_Add(SpellInfo* pSpell, Item* pItemCaster);
        void Cooldown_AddItem(ItemProperties const* pProto, uint32 x);
        bool Cooldown_CanCast(SpellInfo* pSpell);
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
        void SendMeetingStoneQueue(uint32 DungeonId, uint8 Status);
        void SendDungeonDifficulty();
        void SendRaidDifficulty();
        void SendInstanceDifficulty(uint32 difficulty);
        void SendExploreXP(uint32 areaid, uint32 xp);
        void SendDestroyObject(uint64 GUID);
        void SendEquipmentSetList();
        void SendEquipmentSetSaved(uint32 setID, uint32 setGUID);
        void SendEquipmentSetUseResult(uint8 result);
        void SendEmptyPetSpellList();


        //////////////////////////////////////////////////////////////////////////////////////////
        // void SendTotemCreated(uint8 slot, uint64 GUID, uint32 duration, uint32 spellid)
        // Notifies the client about the creation of a Totem/Summon
        // (client will show a right-clickable icon with a timer that can cancel the summon)
        //
        // \param uint8 slot       -  Summon slot number
        // \param uint64 GUID      -  GUID of the summon
        // \param uint32 duration  -  Duration of the summon (the timer of the icon)
        // \param uint32 spellid   -  ID of the spell that created this summon
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void SendTotemCreated(uint8 slot, uint64 GUID, uint32 duration, uint32 spellid);

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
        bool Create(WorldPacket & data);

        void Update(unsigned long time_passed);
        void BuildFlagUpdateForNonGroupSet(uint32 index, uint32 flag);
        void BuildPetSpellList(WorldPacket & data);
        void SetAFKReason(std::string reason) { m_cache->SetStringValue(CACHE_AFK_DND_REASON, reason); };
        const char* GetName() { return m_name.c_str(); }
        std::string* GetNameString() { return &m_name; }

        void GiveXP(uint32 xp, const uint64 & guid, bool allowbonus);       /// to stop rest xp being given
        void ModifyBonuses(uint32 type, int32 val, bool apply);
        void CalcExpertise();
        std::map<uint32, uint32> m_wratings;

        ArenaTeam* m_arenaTeams[NUM_ARENA_TEAM_TYPES];

        /////////////////////////////////////////////////////////////////////////////////////////
        // Taxi
        /////////////////////////////////////////////////////////////////////////////////////////
        TaxiPath* GetTaxiPath() { return m_CurrentTaxiPath; }
        bool GetTaxiState() { return m_onTaxi; }
        const uint32 & GetTaximask(uint8 index) const { return m_taximask[index]; }
        void LoadTaxiMask(const char* data);
        void TaxiStart(TaxiPath* path, uint32 modelid, uint32 start_node);
        void JumpToEndTaxiNode(TaxiPath* path);
        void EventDismount(uint32 money, float x, float y, float z);
        void EventTaxiInterpolate();

        void SetTaxiState(bool state) { m_onTaxi = state; }
        void SetTaximask(uint8 index, uint32 value) { m_taximask[index] = value; }
        void SetTaxiPath(TaxiPath* path) { m_CurrentTaxiPath = path; }
        void SetTaxiPos() { m_taxi_pos_x = m_position.x; m_taxi_pos_y = m_position.y; m_taxi_pos_z = m_position.z; }
        void UnSetTaxiPos() { m_taxi_pos_x = 0; m_taxi_pos_y = 0; m_taxi_pos_z = 0; }

        // Taxi related variables
        std::vector<TaxiPath*> m_taxiPaths;
        TaxiPath* m_CurrentTaxiPath;
        uint32 taxi_model_id;
        uint32 lastNode;
        uint32 m_taxi_ride_time;
        uint32 m_taximask[12];
        float m_taxi_pos_x;
        float m_taxi_pos_y;
        float m_taxi_pos_z;
        bool m_onTaxi;
        uint32 m_taxiMapChangeNode;

        /////////////////////////////////////////////////////////////////////////////////////////
        // Quests
        /////////////////////////////////////////////////////////////////////////////////////////
        bool HasQuests()
        {
            for (uint8 i = 0; i < 25; ++i)
            {
                if (m_questlog[i] != 0)
                    return true;
            }
            return false;
        }

        int32 GetOpenQuestSlot();
        QuestLogEntry* GetQuestLogForEntry(uint32 quest);
        QuestLogEntry* GetQuestLogInSlot(uint32 slot) { return m_questlog[slot]; }
        uint32 GetQuestSharer() { return m_questSharer; }

        void SetQuestSharer(uint32 guid) { m_questSharer = guid; }
        void SetQuestLogSlot(QuestLogEntry* entry, uint32 slot);

        void PushToRemovedQuests(uint32 questid) { m_removequests.insert(questid);}
        void PushToFinishedDailies(uint32 questid) { DailyMutex.Acquire(); m_finishedDailies.insert(questid); DailyMutex.Release();}
        bool HasFinishedDaily(uint32 questid) { return (m_finishedDailies.find(questid) == m_finishedDailies.end() ? false : true); }
        void AddToFinishedQuests(uint32 quest_id);
        void AreaExploredOrEventHappens(uint32 questId);   // scriptdev2

        bool HasFinishedQuest(uint32 quest_id);

        void EventTimedQuestExpire(uint32 questid);


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
        bool HasQuest(uint32 entry);
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
        QuestLogEntry* m_questlog[MAX_QUEST_LOG_SIZE];
        std::set<uint32> m_removequests;
        std::set<uint32> m_finishedQuests;
        Mutex DailyMutex;
        std::set<uint32> m_finishedDailies;
        uint32 m_questSharer;
        std::set<uint32> quest_spells;
        std::set<uint32> quest_mobs;

        void EventPortToGM(Player* p);
        /*! \deprecated This function returns a uint32 (the underlying type of the enum) instead of a PlayerTeam (the enum itself)
         *  \todo Move existing code using GetTeam to GetTeamReal, then refactor to remove GetTeam and rename GetTeamReal to GetTeam
         *  \sa Player::GetTeamReal */
        uint32 GetTeam() { return m_team; }

        PlayerTeam GetTeamReal();

        uint32 GetTeamInitial();
        void SetTeam(uint32 t) { m_team = t; m_bgTeam = t; }

        void ResetTeam();
        bool IsTeamHorde() { return m_team == TEAM_HORDE; }
        bool IsTeamAlliance() { return m_team == TEAM_ALLIANCE; }

        bool IsInFeralForm()
        {
            int s = GetShapeShift();
            if (s <= 0)
                return false;

            // Fight forms that do not use player's weapon
            return (s == FORM_BEAR || s == FORM_DIREBEAR || s == FORM_CAT);     //Shady: actually ghostwolf form doesn't use weapon too.
        }

        void CalcDamage();

        int32 GetDamageDoneMod(uint16_t school)
        {
            if (school >= SCHOOL_COUNT)
                return 0;

            return static_cast<int32>(GetPosDamageDoneMod(school)) - static_cast<int32>(GetNegDamageDoneMod(school));
        }

        float GetDamageDonePctMod(uint32 school)
        {
            if (school >= SCHOOL_COUNT)
                return 0;

            return m_floatValues[PLAYER_FIELD_MOD_DAMAGE_DONE_PCT + school];
        }

        uint32 GetMainMeleeDamage(uint32 AP_owerride);          /// I need this for windfury
        uint32 GetMaxLevel()
        {
#if VERSION_STRING == Classic
            return 60;      // world levelcap!
#else
            return getUInt32Value(PLAYER_FIELD_MAX_LEVEL);
#endif
        }

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
        void smsg_TalentsInfo(bool SendPetTalents);
        void ActivateSpec(uint8 spec);
        void addSpell(uint32 spell_idy);
        void removeSpellByHashName(uint32 hash);
        bool removeSpell(uint32 SpellID, bool MoveToDeleted, bool SupercededSpell, uint32 SupercededSpellID);
        bool removeDeletedSpell(uint32 SpellID);
        void SendPreventSchoolCast(uint32 SpellSchool, uint32 unTimeMs);

        // PLEASE DO NOT INLINE!
        void AddOnStrikeSpell(SpellInfo* sp, uint32 delay)
        {
            m_onStrikeSpells.insert(std::map<SpellInfo*, std::pair<uint32, uint32>>::value_type(sp, std::make_pair(delay, 0)));
        }
        void RemoveOnStrikeSpell(SpellInfo* sp)
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
        // Actionbar
        /////////////////////////////////////////////////////////////////////////////////////////
        void setAction(uint8 button, uint16 action, uint8 type, uint8 misc);
        void SendInitialActions();

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
        uint8 GetPVPRank()
        {
            return (uint8)((getUInt32Value(PLAYER_BYTES_3) >> 24) & 0xFF);
        }
        void SetPVPRank(int newrank)
        {
            setUInt32Value(PLAYER_BYTES_3, ((getUInt32Value(PLAYER_BYTES_3) & 0x00FFFFFF) | (uint8(newrank) << 24)));
        }
        uint32 GetMaxPersonalRating();

        bool HasTitle(RankTitles title)
        {
            return (getUInt64Value(PLAYER_FIELD_KNOWN_TITLES + ((title >> 6) << 1)) & (uint64(1) << (title % 64))) != 0;
        }
        void SetKnownTitle(RankTitles title, bool set);
        void SendAvailSpells(DBC::Structures::SpellShapeshiftFormEntry const* shapeshift_form, bool active);

        /////////////////////////////////////////////////////////////////////////////////////////
        // Groups
        /////////////////////////////////////////////////////////////////////////////////////////
        void SetInviter(uint32 pInviter) { m_GroupInviter = pInviter; }
        uint32 GetInviter() { return m_GroupInviter; }
        bool InGroup() { return (m_playerInfo->m_Group != NULL && !m_GroupInviter); }

        bool IsGroupLeader();
        int HasBeenInvited() { return m_GroupInviter != 0; }
        Group* GetGroup() { return m_playerInfo ? m_playerInfo->m_Group : NULL; }
        int8 GetSubGroup() { return m_playerInfo->subGroup; }
        bool IsGroupMember(Player* plyr);

        bool IsBanned();
        void SetBanned() { m_banned = 4;}
        void SetBanned(std::string Reason) { m_banned = 4; m_banreason = Reason;}
        void SetBanned(uint32 timestamp, std::string & Reason) { m_banned = timestamp; m_banreason = Reason; }
        void UnSetBanned() { m_banned = 0; }
        std::string GetBanReason() {return m_banreason;}

        /////////////////////////////////////////////////////////////////////////////////////////
        // Guilds
        /////////////////////////////////////////////////////////////////////////////////////////
#if VERSION_STRING != Cata
        Guild* GetGuild() { return m_playerInfo->guild; }

        //\todo fix this
        bool IsInGuild()
        {
            return (m_uint32Values[PLAYER_GUILDID] != 0) ? true : false;
        }
        uint32 GetGuildId()
        {
            return m_uint32Values[PLAYER_GUILDID];
        }
        void SetGuildId(uint32 guildId);
        uint32 GetGuildRank() { return m_uint32Values[PLAYER_GUILDRANK]; }
        GuildRank* GetGuildRankS() { return m_playerInfo->guildRank; }
        void SetGuildRank(uint32 guildRank);
        uint32 GetGuildInvitersGuid() { return m_invitersGuid; }
        void SetGuildInvitersGuid(uint32 guid) { m_invitersGuid = guid; }
        void UnSetGuildInvitersGuid() { m_invitersGuid = 0; }
        GuildMember* GetGuildMember() { return m_playerInfo->guildMember; }
#else
        uint32 m_GuildId;
        uint32 m_GuildIdInvited;

        void SetGuildId(uint32 guildId);
        void SetGuildRank(uint32 guildRank);
        void SetInGuild(uint32 guildId);

        void SetRank(uint8 rankId) { setUInt32Value(PLAYER_GUILDRANK, rankId); }
        uint8 GetRank() const { return uint8(getUInt32Value(PLAYER_GUILDRANK)); }

        void SetGuildLevel(uint32 level) { setUInt32Value(PLAYER_GUILDLEVEL, level); }
        uint32 GetGuildLevel() { return getUInt32Value(PLAYER_GUILDLEVEL); }

        void SetGuildIdInvited(uint32 GuildId) { m_GuildIdInvited = GuildId; }
        uint32 GetGuildId() const { return getUInt32Value(OBJECT_FIELD_DATA); /* return only lower part */ }
        Guild* GetGuild();
        bool IsInGuild() { return GetGuild() != nullptr; }

        static uint32 GetGuildIdFromDB(uint64 guid);
        static int8 GetRankFromDB(uint64 guid);
        uint32 GetGuildRank() { return (uint32)GetRankFromDB(GetGUID()); }

        int GetGuildIdInvited() { return m_GuildIdInvited; }

        std::string GetGuildName();
#endif

        /////////////////////////////////////////////////////////////////////////////////////////
        // Duel
        /////////////////////////////////////////////////////////////////////////////////////////
        void RequestDuel(Player* pTarget);
        void DuelBoundaryTest();
        void EndDuel(uint8 WinCondition);
        void DuelCountdown();
        void SetDuelStatus(uint8 status) { m_duelStatus = status; }
        uint8 GetDuelStatus() { return m_duelStatus; }
        void SetDuelState(uint8 state) { m_duelState = state; }
        uint8 GetDuelState() { return m_duelState; }
        // duel variables
        Player* DuelingWith;
        void SetDuelArbiter(uint64 guid) { setUInt64Value(PLAYER_DUEL_ARBITER, guid); }
        uint64 GetDuelArbiter() { return getUInt64Value(PLAYER_DUEL_ARBITER); }
        void SetDuelTeam(uint32 team) { setUInt32Value(PLAYER_DUEL_TEAM, team); }
        uint32 GetDuelTeam() { return getUInt32Value(PLAYER_DUEL_TEAM); }

        /////////////////////////////////////////////////////////////////////////////////////////
        // Trade
        /////////////////////////////////////////////////////////////////////////////////////////
#if VERSION_STRING != Cata
        void SendTradeUpdate(void);
        void ResetTradeVariables()
        {
            mTradeGold = 0;
            memset(&mTradeItems, 0, sizeof(Item*) * 8);
            mTradeStatus = 0;
            mTradeTarget = 0;
            m_tradeSequence = 2;
        }
#endif

        /////////////////////////////////////////////////////////////////////////////////////////
        // Pets
        /////////////////////////////////////////////////////////////////////////////////////////
        void AddSummon(Pet* pet) { m_Summons.push_front(pet); }
        Pet* GetSummon()     // returns 1st summon
        {
            if (!m_Summons.empty())
                return m_Summons.front();
            else
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
            else
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
        ItemInterface* GetItemInterface() { return m_ItemInterface; }       /// Player Inventory Item storage
        void ApplyItemMods(Item* item, int16 slot, bool apply, bool justdrokedown = false) { _ApplyItemMods(item, slot, apply, justdrokedown); }
        bool HasItemCount(uint32 item, uint32 count, bool inBankAlso = false) const;
        /// item interface variables
        ItemInterface* m_ItemInterface;
        int32 GetVisibleBase(int16 slot)
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
        const uint64 & GetLootGUID() const { return m_lootGuid; }
        void SetLootGUID(const uint64 & guid) { m_lootGuid = guid; }
        void SendLoot(uint64 guid, uint8 loot_type, uint32 mapid);
        void SendLootUpdate(Object* o);
        void TagUnit(Object* o);
        void SendPartyKillLog(uint64 GUID);
        // loot variables
        uint64 m_lootGuid;
        uint64 m_currentLoot;
        bool bShouldHaveLootableOnCorpse;

        /////////////////////////////////////////////////////////////////////////////////////////
        // World Session
        /////////////////////////////////////////////////////////////////////////////////////////
        WorldSession* GetSession() const { return m_session; }
        void SetSession(WorldSession* s) { m_session = s; }
        void SetBindPoint(float x, float y, float z, uint32 m, uint32 v) { m_bind_pos_x = x; m_bind_pos_y = y; m_bind_pos_z = z; m_bind_mapid = m; m_bind_zoneid = v;}

        void SendDelayedPacket(WorldPacket* data, bool bDeleteOnSend);
        float offhand_dmg_mod;

        // Talents
        // These functions build a specific type of A9 packet
        uint32 BuildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target);
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

        bool LoadSpells(QueryResult* result);
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

        bool m_isMoving;            /// moving + strafing + jumping
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
        void ModAttackSpeed(int32 mod, ModType type);
        void UpdateAttackSpeed();
        float GetDefenseChance(uint32 opLevel);
        float GetDodgeChance();
        float GetBlockChance();
        float GetParryChance();
        void UpdateChances();
        void UpdateStats();
        uint32 GetBlockDamageReduction();
        void ApplyFeralAttackPower(bool apply, Item* item = NULL);

        bool canCast(SpellInfo* m_spellInfo);

        float GetSpellCritFromSpell() { return m_spellcritfromspell; }
        float GetHitFromSpell() { return m_hitfromspell; }
        void SetSpellCritFromSpell(float value) { m_spellcritfromspell = value; }
        void SetHitFromSpell(float value) { m_hitfromspell = value; }

        uint32 GetHealthFromSpell() { return m_healthfromspell; }
        uint32 GetManaFromSpell() { return m_manafromspell; }
        void SetHealthFromSpell(uint32 value) { m_healthfromspell = value;}
        void SetManaFromSpell(uint32 value) { m_manafromspell = value;}

        uint32 CalcTalentResetCost(uint32 resetnum);
        void SendTalentResetConfirm();
        void SendPetUntrainConfirm();

        uint32 GetTalentResetTimes() { return m_talentresettimes; }
        void SetTalentResetTimes(uint32 value) { m_talentresettimes = value; }

        void SetPlayerStatus(uint8 pStatus) { m_status = pStatus; }
        uint8 GetPlayerStatus() { return m_status; }

        const float & GetBindPositionX() const { return m_bind_pos_x; }
        const float & GetBindPositionY() const { return m_bind_pos_y; }
        const float & GetBindPositionZ() const { return m_bind_pos_z; }

        const uint32 & GetBindMapId() const { return m_bind_mapid; }
        const uint32 & GetBindZoneId() const { return m_bind_zoneid; }

        void delayAttackTimer(int32 delay)
        {
            if (!delay)
                return;

            m_attackTimer += delay;
            m_attackTimer_1 += delay;
        }

        void SetShapeShift(uint8 ss);

        uint32 m_furorChance;

        // Showing Units WayPoints
        AIInterface* waypointunit;

        uint32 m_nextSave;
        // Tutorials
        uint32 GetTutorialInt(uint32 intId);
        void SetTutorialInt(uint32 intId, uint32 value);

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
        bool CanSee(Object* obj);
        bool IsVisible(uint64 pObj) { return !(m_visibleObjects.find(pObj) == m_visibleObjects.end()); }
        void AddInRangeObject(Object* pObj);
        void OnRemoveInRangeObject(Object* pObj);
        void ClearInRangeSet();
        void AddVisibleObject(uint64 pObj) { m_visibleObjects.insert(pObj); }
        void RemoveVisibleObject(uint64 pObj) { m_visibleObjects.erase(pObj); }
        void RemoveVisibleObject(std::set< uint64 >::iterator itr) { m_visibleObjects.erase(itr); }
        std::set< uint64 >::iterator FindVisible(uint64 obj) { return m_visibleObjects.find(obj); }
        void RemoveIfVisible(uint64 obj);

        // Misc
        void EventCannibalize(uint32 amount);
        bool m_AllowAreaTriggerPort;
        void EventAllowTiggerPort(bool enable);
        void UpdatePowerAmm();
        uint32 m_modblockabsorbvalue;
        uint32 m_modblockvaluefromspells;
        void SendInitialLogonPackets();
        void Reset_Spells();
        void Reset_Talents();
        void Reset_AllTalents();
        // Battlegrounds xD
        CBattleground* m_bg;
        CBattleground* m_pendingBattleground;
        uint32 m_bgEntryPointMap;
        float m_bgEntryPointX;
        float m_bgEntryPointY;
        float m_bgEntryPointZ;
        float m_bgEntryPointO;
        int32 m_bgEntryPointInstance;
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

        int32 CanShootRangedWeapon(uint32 spellid, Unit* target, bool autoshot);
        uint32 m_AutoShotAttackTimer;
        void _InitialReputation();
        void EventActivateGameObject(GameObject* obj);
        void EventDeActivateGameObject(GameObject* obj);
        void UpdateNearbyGameObjects();

        void CalcResistance(uint32 type);
        float res_M_crit_get() { return m_resist_critical[0]; }
        void res_M_crit_set(float newvalue) { m_resist_critical[0] = newvalue; }
        float res_R_crit_get() { return m_resist_critical[1]; }
        void res_R_crit_set(float newvalue) { m_resist_critical[1] = newvalue; }
        uint32 FlatResistanceModifierPos[SCHOOL_COUNT];
        uint32 FlatResistanceModifierNeg[SCHOOL_COUNT];
        uint32 BaseResistanceModPctPos[SCHOOL_COUNT];
        uint32 BaseResistanceModPctNeg[SCHOOL_COUNT];
        uint32 ResistanceModPctPos[SCHOOL_COUNT];
        uint32 ResistanceModPctNeg[SCHOOL_COUNT];
        float m_resist_critical[2];             // when we are a victim we can have talents to decrease chance for critical hit. This is a negative value and it's added to critchances
        float m_resist_hit[2];                  // 0 = melee; 1= ranged;
        int32 m_resist_hit_spell[SCHOOL_COUNT]; // spell resist per school
        float m_attack_speed[3];
        float SpellHealDoneByAttribute[5][SCHOOL_COUNT];
        uint32 m_modphyscritdmgPCT;
        uint32 m_RootedCritChanceBonus;         // Class Script Override: Shatter
        uint32 m_IncreaseDmgSnaredSlowed;

        uint32 m_ModInterrMRegenPCT;
        int32 m_ModInterrMRegen;
        float m_RegenManaOnSpellResist;
        uint32 m_casted_amount[SCHOOL_COUNT];   // Last casted spells amounts. Need for some spells. Like Ignite etc. DOesn't count HoTs and DoTs. Only directs

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
        void CalcStat(uint32 t);
        float CalcRating(PlayerCombatRating t);
        void RegenerateMana(bool is_interrupted);
        void RegenerateHealth(bool inCombat);
        void RegenerateEnergy();
        void LooseRage(int32 value);

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
        bool ignoreShapeShiftChecks;
        bool ignoreAuraStateCheck;

        std::map<uint32, WeaponModifier> damagedone;
        std::map<uint32, WeaponModifier> tocritchance;
        bool cannibalize;
        uint8 cannibalizeCount;
        int32 rageFromDamageDealt;
        int32 rageFromDamageTaken;
        // GameObject commands
        GameObject * GetSelectedGo();

        uint64 m_GM_SelectedGO;

        void _Relocate(uint32 mapid, const LocationVector & v, bool sendpending, bool force_new_world, uint32 instance_id);

        void AddItemsToWorld();
        void RemoveItemsFromWorld();
        void UpdateKnownCurrencies(uint32 itemId, bool apply);

        uint32 m_ShapeShifted;
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

        bool IsVehicle()
        {
            if (mountvehicleid != 0)
                return true;
            else
                return false;
        }


        void AddVehicleComponent(uint32 creature_entry, uint32 vehicleid);

        void RemoveVehicleComponent();

        void SendMountResult(uint32 result);

        void SendDismountResult(uint32 result);

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
        void ResetAllCooldowns();
        void ClearCooldownForSpell(uint32 spell_id);

        void Phase(uint8 command = PHASE_SET, uint32 newphase = 1);

        bool bProcessPending;
        Mutex _bufferS;
        void PushUpdateData(ByteBuffer* data, uint32 updatecount);
        void PushCreationData(ByteBuffer* data, uint32 updatecount);
        void PushOutOfRange(const WoWGuid & guid);
        void ProcessPendingUpdates();
        bool CompressAndSendUpdateBuffer(uint32 size, const uint8* update_buffer);
        void ClearAllPendingUpdates();

        uint32 GetArmorProficiency() { return armor_proficiency; }
        uint32 GetWeaponProficiency() { return weapon_proficiency; }

        void AddSplinePacket(uint64 guid, ByteBuffer* packet);
        ByteBuffer* GetAndRemoveSplinePacket(uint64 guid);
        void ClearSplinePackets();
        bool ExitInstance();
        void SaveEntryPoint(uint32 mapId);

        // Cheat section
        void SpeedCheatDelay(uint32 ms_delay);
        void SpeedCheatReset();

        bool TaxiCheat;
        bool CooldownCheat;
        bool CastTimeCheat;
        bool GodModeCheat;
        bool PowerCheat;
        bool FlyCheat;
        bool ItemStackCheat;
        bool AuraStackCheat;
        bool TriggerpassCheat;

        bool SaveAllChangesCommand;

        void ZoneUpdate(uint32 ZoneId);
        void UpdateChannels(uint16 AreaID);
        uint32 GetAreaID() { return m_AreaID; }
        void SetAreaID(uint32 area) { m_AreaID = area; }
        bool IsInCity();

        // Instance IDs
        uint32 GetPersistentInstanceId(uint32 mapId, uint8 difficulty)
        {
            if (mapId >= NUM_MAPS || difficulty >= NUM_INSTANCE_MODES || m_playerInfo == NULL)
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

        // DualWield2H (ex: Titan's grip)
        bool DualWield2H;
        void ResetDualWield2H();

    public:

        bool m_Autojoin;
        bool m_AutoAddMem;
        void SendMirrorTimer(MirrorTimerTypes Type, uint32 max, uint32 current, int32 regen);
        void StopMirrorTimer(MirrorTimerTypes Type);
        BGScore m_bgScore;
        uint32 m_bgTeam;
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
        void ApplyLevelInfo(LevelInfo* Info, uint32 Level);
        void BroadcastMessage(const char* Format, ...);
        std::map<uint32, std::set<uint32> > SummonSpells;
        std::map<uint32, std::map<SpellInfo*, uint16>*> PetSpells;
        void AddSummonSpell(uint32 Entry, uint32 SpellID);
        void RemoveSummonSpell(uint32 Entry, uint32 SpellID);
        std::set<uint32>* GetSummonSpells(uint32 Entry);
        LockedQueue<WorldPacket*> delayedPackets;
        uint32 m_UnderwaterMaxTime;
        uint32 m_UnderwaterLastDmg;
        LocationVector getMyCorpseLocation() const { return myCorpseLocation; }
        bool bCorpseCreateable;
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

        void SetDungeonDifficulty(uint8 diff);
        uint8 GetDungeonDifficulty();

        void SetRaidDifficulty(uint8 diff);
        uint8 GetRaidDifficulty();

        void EventSafeTeleport(uint32 MapID, uint32 InstanceID, LocationVector vec)
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

        bool IsPvPFlagged();
        void SetPvPFlag();
        void RemovePvPFlag();

        bool IsFFAPvPFlagged();
        void SetFFAPvPFlag();
        void RemoveFFAPvPFlag();

        bool IsSanctuaryFlagged();
        void SetSanctuaryFlag();
        void RemoveSanctuaryFlag();

        /////////////////////////////////////////////////////////////////////////////////////////
        // Player gold
        /////////////////////////////////////////////////////////////////////////////////////////
        void SetGold(int32 coins)
        {
            setUInt32Value(PLAYER_FIELD_COINAGE, coins);
        }
        void ModGold(int32 coins)
        {
            ModUnsigned32Value(PLAYER_FIELD_COINAGE, coins);
        }
        uint32 GetGold()
        {
            return getUInt32Value(PLAYER_FIELD_COINAGE);
        }
        bool HasGold(uint32 coins)
        {
            return (getUInt32Value(PLAYER_FIELD_COINAGE) >= coins);
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DEPRICATED FUNCTIONS USE
        // SetGold and ModGold
        /////////////////////////////////////////////////////////////////////////////////////////
        void GiveGold(int32 coins)
        {
            ModUnsigned32Value(PLAYER_FIELD_COINAGE , coins);
        }
        void TakeGold(int32 coins)
        {
            ModUnsigned32Value(PLAYER_FIELD_COINAGE, -coins);
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // EASY FUNCTIONS - MISC
        /////////////////////////////////////////////////////////////////////////////////////////

        void SetChosenTitle(uint32 id)
        {
#if VERSION_STRING > Classic
            setUInt32Value(PLAYER_CHOSEN_TITLE, id);
#endif
        }

        void SetInventorySlot(uint16_t slot, uint64 guid) { setUInt64Value(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2), guid); }

        void SetFarsightTarget(uint64 guid) { setUInt64Value(PLAYER_FARSIGHT, guid); }
        uint64 GetFarsightTarget() { return getUInt64Value(PLAYER_FARSIGHT); }

        void SetXp(uint32 xp) { setUInt32Value(PLAYER_XP, xp); }
        uint32 GetXp() { return getUInt32Value(PLAYER_XP); }
        uint32 GetXpToLevel() { return getUInt32Value(PLAYER_NEXT_LEVEL_XP); }
        void SetNextLevelXp(uint32 xp) { setUInt32Value(PLAYER_NEXT_LEVEL_XP, xp); }

        //\todo fix this
        void SetTalentPointsForAllSpec(uint32 amt)
        {
            m_specs[0].SetTP(amt);
            m_specs[1].SetTP(amt);
#if VERSION_STRING != Cata
            setUInt32Value(PLAYER_CHARACTER_POINTS1, amt);
#else
            setUInt32Value(PLAYER_CHARACTER_POINTS, amt);
#endif
            smsg_TalentsInfo(false);
        }

        void AddTalentPointsToAllSpec(uint32 amt)
        {
            m_specs[0].SetTP(m_specs[0].GetTP() + amt);
            m_specs[1].SetTP(m_specs[1].GetTP() + amt);
#if VERSION_STRING != Cata
            setUInt32Value(PLAYER_CHARACTER_POINTS1, getUInt32Value(PLAYER_CHARACTER_POINTS1) + amt);
#else
            setUInt32Value(PLAYER_CHARACTER_POINTS, getUInt32Value(PLAYER_CHARACTER_POINTS) + amt);
#endif
            smsg_TalentsInfo(false);
        }

        void SetCurrentTalentPoints(uint32 points)
        {
            m_specs[m_talentActiveSpec].SetTP(points);
#if VERSION_STRING != Cata
            setUInt32Value(PLAYER_CHARACTER_POINTS1, points);
#else
            setUInt32Value(PLAYER_CHARACTER_POINTS, points);
#endif
            smsg_TalentsInfo(false);
        }

        uint32 GetCurrentTalentPoints()
        {
#if VERSION_STRING != Cata
            uint32 points = getUInt32Value(PLAYER_CHARACTER_POINTS1);
#else
            uint32 points = getUInt32Value(PLAYER_CHARACTER_POINTS);
#endif
            Arcemu::Util::ArcemuAssert(points == m_specs[m_talentActiveSpec].GetTP());
            return points;
        }

        //\todo fix this
        void SetPrimaryProfessionPoints(uint32 amt)
        {
#if VERSION_STRING != Cata
            setUInt32Value(PLAYER_CHARACTER_POINTS2, amt);
#else
            if (amt == 0) { return; }
#endif
        }
        //\todo fix this
        void ModPrimaryProfessionPoints(int32 amt)
        {
#if VERSION_STRING != Cata
            ModUnsigned32Value(PLAYER_CHARACTER_POINTS2, amt);
#else
            if (amt == 0) { return; }
#endif
        }
        uint32 GetPrimaryProfessionPoints()
        {
#if VERSION_STRING != Cata
            return getUInt32Value(PLAYER_CHARACTER_POINTS2);
#else
            return 0;
#endif
        }

        void ModPosDamageDoneMod(uint32 school, uint32 value) { ModUnsigned32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + school, value); }
        uint32 GetPosDamageDoneMod(uint16_t school) { return getUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + school); }

        void ModNegDamageDoneMod(uint16_t school, uint32 value) { ModUnsigned32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + school, value); }
        uint32 GetNegDamageDoneMod(uint16_t school) { return getUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + school); }

        void ModHealingDoneMod(uint32 value)
        {
#if VERSION_STRING > Classic
            ModUnsigned32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS, value);
#endif
        }
        uint32 GetHealingDoneMod()
        {
#if VERSION_STRING > Classic
            return getUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS);
#else
            return 0;
#endif
        }

        //\todo fix this
        void SetAmmoId(uint32 id)
        {
#if VERSION_STRING < Cata
            setUInt32Value(PLAYER_AMMO_ID, id);
#else
            if (id == 0) { return; }
#endif
        }
        uint32 GetAmmoId()
        {
#if VERSION_STRING < Cata
            return getUInt32Value(PLAYER_AMMO_ID);
#else
            return 0;
#endif
        }

        void SetHonorCurrency(uint32 value)
        {
#if VERSION_STRING == Cata
            if (value == 0) { return; }
#elif VERSION_STRING == Classic
#else
            setUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, value);
#endif
        }
        void ModHonorCurrency(uint32 value)
        {
#if VERSION_STRING == Cata
            if (value == 0) { return; }
#elif VERSION_STRING == Classic
#else
            ModUnsigned32Value(PLAYER_FIELD_HONOR_CURRENCY, value);
#endif
        }
        uint32 GetHonorCurrency()
        {
#if VERSION_STRING == Cata
            return 0;
#elif VERSION_STRING == Classic
            return 0;
#else
            return getUInt32Value(PLAYER_FIELD_HONOR_CURRENCY);
#endif
        }

        void AddHonor(uint32 honorPoints, bool sendUpdate);
        void UpdateHonor();

        //\todo fix this
        void SetArenaCurrency(uint32 value)
        {
#if VERSION_STRING == Cata
            if (value == 0) { return; }
#elif VERSION_STRING == Classic
#else
            setUInt32Value(PLAYER_FIELD_ARENA_CURRENCY, value);
#endif
        }
        void ModArenaCurrency(uint32 value)
        {
#if VERSION_STRING == Cata
            if (value == 0) { return; }
#elif VERSION_STRING == Classic
#else
            ModUnsigned32Value(PLAYER_FIELD_ARENA_CURRENCY, value);
#endif
        }
        uint32 GetArenaCurrency()
        {
#if VERSION_STRING == Cata
            return 0;
#elif VERSION_STRING == Classic
            return 0;
#else
            return getUInt32Value(PLAYER_FIELD_ARENA_CURRENCY);
#endif
        }

        void AddArenaPoints(uint32 arenaPoints, bool sendUpdate);
        void UpdateArenaPoints();

#if VERSION_STRING > TBC
        void SetGlyph(uint32 slot, uint32 id) { setUInt32Value(PLAYER_FIELD_GLYPHS_1 + slot, id); }
        uint32 GetGlyph(uint32 slot) { return getUInt32Value(PLAYER_FIELD_GLYPHS_1 + slot); }
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


        //////////////////////////////////////////////////////////////////////////////////////////
        // void LearnTalent(uint32 talentid, uint32 rank, bool isPreviewed)
        // Teaches a talentspell to the Player and decreases the available talent points
        //
        // \param uint32 talentid     -   unique numeric identifier of the talent (index of talent.dbc)
        // \param uint32 rank         -   rank of the talent
        // \param bool isPreviewed     -   true if called from the preview system
        //
        // \return none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void LearnTalent(uint32 talentid, uint32 rank, bool isPreviewed = false);


        void DealDamage(Unit* pVictim, uint32 damage, uint32 targetEvent, uint32 unitEvent, uint32 spellId, bool no_remove_auras = false);
        void TakeDamage(Unit* pAttacker, uint32 damage, uint32 spellid, bool no_remove_auras = false);
        void Die(Unit* pAttacker, uint32 damage, uint32 spellid);
        void HandleKnockback(Object* caster, float horizontal, float vertical);

        uint32 LastHonorResetTime() const { return m_lastHonorResetTime; }
        void LastHonorResetTime(uint32 val) { m_lastHonorResetTime = val; }
        uint32 OnlineTime;
        bool tutorialsDirty;
        LevelInfo* lvlinfo;
        void CalculateBaseStats();
        uint32 load_health;
        uint32 load_mana;
        void CompleteLoading();
        void OnPushToWorld();
        void OnPrePushToWorld();
        void OnWorldPortAck();
        uint32 m_TeleportState;
        bool m_beingPushed;
        bool CanSignCharter(Charter* charter, Player* requester);
        Charter* m_charters[NUM_CHARTER_TYPES];
        uint32 flying_aura;
        bool resend_speed;
        uint32 login_flags;
        uint8 iInstanceType;
        void SetName(std::string & name) { m_name = name; }

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

        // Trade Target
#if VERSION_STRING != Cata
        Player* GetTradeTarget();

        Item* getTradeItem(uint32 slot) {return mTradeItems[slot];};
#endif

        // Water level related stuff (they are public because they need to be accessed fast)
        // Nose level of the character (needed for proper breathing)
        float m_noseLevel;

        void RemoteRevive();

        LocationVector m_last_group_position;
        int32 m_rap_mod_pct;
        void SummonRequest(uint32 Requestor, uint32 ZoneID, uint32 MapID, uint32 InstanceID, const LocationVector & Position);

        bool m_deathVision;
        SpellInfo* last_heal_spell;
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
        void SendLevelupInfo(uint32 level, uint32 Hp, uint32 Mana, uint32 Stat0, uint32 Stat1, uint32 Stat2, uint32 Stat3, uint32 Stat4);
        void SendLogXPGain(uint64 guid, uint32 NormalXP, uint32 RestedXP, bool type);
        void SendWorldStateUpdate(uint32 WorldState, uint32 Value);
        void SendCastResult(uint32 SpellId, uint8 ErrorMessage, uint8 MultiCast, uint32 Extra);
        void Gossip_SendPOI(float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, const char* Name);
        void Gossip_SendSQLPOI(uint32 id);
        void SendSpellCooldownEvent(uint32 SpellId);
        void SendSpellModifier(uint8 spellgroup, uint8 spelltype, int32 v, bool is_pct);
        void SendItemPushResult(bool created, bool recieved, bool sendtoset, bool newitem,  uint8 destbagslot, uint32 destslot, uint32 count, uint32 entry, uint32 suffix, uint32 randomprop, uint32 stack);
        void SendSetProficiency(uint8 ItemClass, uint32 Proficiency);
        void SendLoginVerifyWorld(uint32 MapId, float X, float Y, float Z, float O);

        void SendNewDrunkState(uint32 state, uint32 itemid);

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

        // Update system components
        ByteBuffer bUpdateBuffer;
        ByteBuffer bCreationBuffer;
        uint32 mUpdateCount;
        uint32 mCreationCount;
        uint32 mOutOfRangeIdCount;
        ByteBuffer mOutOfRangeIds;
        SplineMap _splineMap;
        // End update system

        void _LoadTutorials(QueryResult* result);
        void _SaveTutorials(QueryBuffer* buf);
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

        /// Water level related stuff
        void SetNoseLevel();

        /////////////////////////////////////////////////////////////////////////////////////////
        // Trade
        /////////////////////////////////////////////////////////////////////////////////////////
#if VERSION_STRING != Cata
        Item* mTradeItems[8];
        uint32 mTradeGold;
        uint32 mTradeTarget;
        uint32 mTradeStatus;
#endif

        /////////////////////////////////////////////////////////////////////////////////////////
        // Player Class systems, info and misc things
        /////////////////////////////////////////////////////////////////////////////////////////
        PlayerCreateInfo const* info;
        uint32 m_AttackMsgTimer;        // "too far away" and "wrong facing" timer
        bool m_attacking;
        std::string m_name;             // max 21 character name
        uint32 m_Tutorials[8];

        // Character Ban
        uint32 m_banned;
        std::string m_banreason;
        uint32 m_AreaID;
        std::list<Pet*>  m_Summons;
        uint32 m_PetNumberMax;
        std::map<uint32, PlayerPet*> m_Pets;

        uint32 m_invitersGuid;      // It is guild inviters guid ,0 when its not used

        // bind
        float m_bind_pos_x;
        float m_bind_pos_y;
        float m_bind_pos_z;
        uint32 m_bind_mapid;
        uint32 m_bind_zoneid;
        std::list<ItemSet> m_itemsets;
        //Duel
        uint32 m_duelCountdownTimer;
        uint8 m_duelStatus;
        uint8 m_duelState;
        // Rested State Stuff
        uint32 m_timeLogoff;
        // Played time
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
        uint32 m_GroupInviter;
        uint8 m_StableSlotCount;

        // Fishing related
        Object* m_SummonedObject;

        // other system
        LocationVector myCorpseLocation;
        int32 myCorpseInstanceId;

        uint32 m_lastHonorResetTime;
        uint32 _fields[PLAYER_END];
        int hearth_of_wild_pct;        // druid hearth of wild talent used on shapeshifting. We either know what is last talent level or memo on learn

        uint32 m_team;

        uint32 m_indoorCheckTimer;
        void RemovePendingPlayer();

    public:

        void addDeletedSpell(uint32 id) { mDeletedSpells.insert(id); }

        std::map<uint32, uint32> m_forcedReactions;

        uint32 m_flyhackCheckTimer;
        //void _FlyhackCheck(); disabled not working not used. Zyres.

        bool m_passOnLoot;
        uint32 m_tradeSequence;
        bool m_changingMaps;

        void PlaySoundToPlayer(uint64_t from_guid, uint32_t sound_id);
        void PlaySound(uint32 sound_id);

        void SendGuildMOTD();

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

        bool m_castFilterEnabled;
        uint32 m_castFilter[3];    // spell group relation of only spells that player can currently cast

        uint32 m_outStealthDamageBonusPct;
        uint32 m_outStealthDamageBonusPeriod;
        uint32 m_outStealthDamageBonusTimer;

        //\todo sort out where all the publics and privates go. This will do for now..

    private:

        PlayerInfo* m_playerInfo;
        uint8 m_RaidDifficulty;
        bool m_XpGain;
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

        void LoadFieldsFromString(const char* string, uint32 firstField, uint32 fieldsNum);
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
        uint16 m_maxTalentPoints;
        uint8 m_talentSpecsCount;
        uint8 m_talentActiveSpec;
#if VERSION_STRING == Cata
        uint32 m_FirstTalentTreeLock;
        uint32 CalcTalentPointsHaveSpent(uint32 spec);
#endif

        PlayerSpec m_specs[MAX_SPEC_COUNT];

        uint8 m_roles;
		uint32 GroupUpdateFlags;

    public:

        void SendUpdateDataToSet(ByteBuffer* groupbuf, ByteBuffer* nongroupbuf, bool sendtoself);

        bool CanBuyAt(MySQLStructure::VendorRestrictions const* vendor);
        bool CanTrainAt(Trainer*);

        Object* GetPlayerOwner() { return this; };

        void SetRoles(uint8 role) { m_roles = role; }
		uint8 GetRoles() { return m_roles; }
        void SetBattlegroundEntryPoint();

        uint32 GetGroupUpdateFlags() { return GroupUpdateFlags; }
		void SetGroupUpdateFlags(uint32 flags);
		void AddGroupUpdateFlag(uint32 flag);
		uint16 GetGroupStatus();
		void SendUpdateToOutOfRangeGroupMembers();

        void SendTeleportPacket(float x, float y, float z, float o);
        void SendTeleportAckPacket(float x, float y, float z, float o);

        bool camControle;
        void SendCinematicCamera(uint32 id);
        void SetClientControl(Unit* target, uint8 allowMove);

        void SetMover(Unit* target);

        // command
        float go_last_x_rotation;
        float go_last_y_rotation;

        // AGPL End
};
