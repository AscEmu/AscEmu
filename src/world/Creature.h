/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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

#ifndef _WOWSERVER_CREATURE_H
#define _WOWSERVER_CREATURE_H

#include "CreatureDefines.hpp"
#include "UnitDefines.hpp"
#include "Map.h"
#include "Unit.h"

class CreatureTemplate;
class GossipScript;

SERVER_DECL bool Rand(float chance);
SERVER_DECL bool Rand(uint32 chance);
SERVER_DECL bool Rand(int32 chance);

class CreatureAIScript;
class GossipScript;
class AuctionHouse;
struct Trainer;
#define CALL_SCRIPT_EVENT(obj, func) if (obj->IsInWorld() && obj->IsCreature() && static_cast<Creature*>(obj)->GetScript() != NULL) static_cast<Creature*>(obj)->GetScript()->func


uint8 get_byte(uint32 buffer, uint32 index);

///////////////////
/// Creature object

class SERVER_DECL Creature : public Unit
{
    public:

        Creature(uint64 guid);
        virtual ~Creature();

        GameEvent* mEvent = nullptr;
        
        /// For derived subclasses of Creature
        bool IsVehicle()
        {
            if (proto->vehicleid != 0)
                return true;
            else
                return false;
        }
        
        void AddVehicleComponent(uint32 creature_entry, uint32 vehicleid);
        void RemoveVehicleComponent();

        bool Load(CreatureSpawn* spawn, uint32 mode, MapInfo* info);
        void Load(CreatureProto* proto_, float x, float y, float z, float o = 0);

        void AddToWorld();
        void AddToWorld(MapMgr* pMapMgr);
        void RemoveFromWorld(bool addrespawnevent, bool free_guid);
        void RemoveFromWorld(bool free_guid);

        void PrepareForRemove();    /// remove auras, guardians, scripts

        /// Creation
        void Create(const char* creature_name, uint32 mapid, float x, float y, float z, float ang);
        void CreateWayPoint(uint32 WayPointID, uint32 mapid, float x, float y, float z, float ang);

        /// Updates
        void Update(unsigned long time_passed);

        /// Creature inventory
        inline uint32 GetItemIdBySlot(uint32 slot) { return m_SellItems->at(slot).itemid; }
        inline uint32 GetItemAmountBySlot(uint32 slot) { return m_SellItems->at(slot).amount; }

        inline bool HasItems() { return ((m_SellItems != NULL) ? true : false); }
        inline CreatureProto* GetProto() { return proto; }

        bool IsPvPFlagged();
        void SetPvPFlag();
        void RemovePvPFlag();

        bool IsFFAPvPFlagged();
        void SetFFAPvPFlag();
        void RemoveFFAPvPFlag();

        bool IsSanctuaryFlagged();
        void SetSanctuaryFlag();
        void RemoveSanctuaryFlag();

        void SetSpeeds(uint8 type, float speed);

        int32 GetSlotByItemId(uint32 itemid)
        {
            uint32 slot = 0;
            for (std::vector<CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
            {
                if (itr->itemid == itemid)
                    return slot;
                else
                    ++slot;
            }
            return -1;
        }

        uint32 GetItemAmountByItemId(uint32 itemid)
        {
            for (std::vector<CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
            {
                if (itr->itemid == itemid)
                    return ((itr->amount < 1) ? 1 : itr->amount);
            }
            return 0;
        }

        inline void GetSellItemBySlot(uint32 slot, CreatureItem & ci)
        {
            ci = m_SellItems->at(slot);
        }

        void GetSellItemByItemId(uint32 itemid, CreatureItem & ci)
        {
            for (std::vector<CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
            {
                if (itr->itemid == itemid)
                {
                    ci = (*itr);
                    return;
                }
            }
            ci.amount = 0;
            ci.max_amount = 0;
            ci.available_amount = 0;
            ci.incrtime = 0;
            ci.itemid = 0;
        }

        ItemExtendedCostEntry* GetItemExtendedCostByItemId(uint32 itemid)
        {
            for (std::vector<CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
            {
                if (itr->itemid == itemid)
                    return itr->extended_cost;
            }
            return NULL;
        }

        inline std::vector<CreatureItem>::iterator GetSellItemBegin() { return m_SellItems->begin(); }
        inline std::vector<CreatureItem>::iterator GetSellItemEnd()   { return m_SellItems->end(); }
        inline size_t GetSellItemCount() { return m_SellItems->size(); }
        void RemoveVendorItem(uint32 itemid)
        {
            for (std::vector<CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
            {
                if (itr->itemid == itemid)
                {
                    m_SellItems->erase(itr);
                    return;
                }
            }
        }
        void AddVendorItem(uint32 itemid, uint32 amount, ItemExtendedCostEntry* ec);
        void ModAvItemAmount(uint32 itemid, uint32 value);
        void UpdateItemAmount(uint32 itemid);
        /// Quests
        void _LoadQuests();
        bool HasQuests() { return m_quests != NULL; };
        bool HasQuest(uint32 id, uint32 type)
        {
            if (!m_quests) return false;
            for (std::list<QuestRelation*>::iterator itr = m_quests->begin(); itr != m_quests->end(); ++itr)
            {
                if ((*itr)->qst->id == id && (*itr)->type & type)
                    return true;
            }
            return false;
        }
        void AddQuest(QuestRelation* Q);
        void DeleteQuest(QuestRelation* Q);
        Quest* FindQuest(uint32 quest_id, uint8 quest_relation);
        uint16 GetQuestRelation(uint32 quest_id);
        uint32 NumOfQuests();
        std::list<QuestRelation*>::iterator QuestsBegin() { return m_quests->begin(); };
        std::list<QuestRelation*>::iterator QuestsEnd() { return m_quests->end(); };
        void SetQuestList(std::list<QuestRelation*>* qst_lst) { m_quests = qst_lst; };

        inline uint32 isVendor()         const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR); }
        inline uint32 isTrainer()        const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER); }
        inline uint32 isClass()          const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER_CLASS); }
        inline uint32 isProf()           const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER_PROF); }
        inline uint32 isQuestGiver()     const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER); }
        inline uint32 isGossip()         const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP); }
        inline uint32 isTaxi()           const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TAXIVENDOR); }
        inline uint32 isCharterGiver()   const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_ARENACHARTER); }
        inline uint32 isGuildBank()      const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GUILD_BANK); }
        inline uint32 isBattleMaster()   const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BATTLEFIELDPERSON); }
        inline uint32 isBanker()         const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER); }
        inline uint32 isInnkeeper()      const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER); }
        inline uint32 isSpiritHealer()   const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER); }
        inline uint32 isTabardDesigner() const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TABARDCHANGER); }
        inline uint32 isAuctioner()      const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER); }
        inline uint32 isStableMaster()   const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_STABLEMASTER); }
        inline uint32 isArmorer()        const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_ARMORER); }

        inline uint32 GetHealthFromSpell() { return m_healthfromspell; }
        void SetHealthFromSpell(uint32 value) { m_healthfromspell = value;}

        int32 m_speedFromHaste;
        int32 FlatResistanceMod[SCHOOL_COUNT];
        int32 BaseResistanceModPct[SCHOOL_COUNT];
        int32 ResistanceModPct[SCHOOL_COUNT];

        int32 FlatStatMod[5];
        int32 StatModPct[5];
        int32 TotalStatModPct[5];

        int32 ModDamageDone[SCHOOL_COUNT];
        float ModDamageDonePct[SCHOOL_COUNT];
        void CalcResistance(uint32 type);
        void CalcStat(uint32 type);

        bool m_canRegenerateHP;
        void RegenerateHealth();
        void RegenerateMana();
        int BaseAttackType;

        bool CanSee(Unit* obj)      /// Invisibility & Stealth Detection - Partha
        {
            if (!obj)
                return false;

            if (obj->m_invisible)    /// Invisibility - Detection of Players and Units
            {
                if (obj->getDeathState() == CORPSE)  /// can't see dead players' spirits
                    return false;

                if (m_invisDetect[obj->m_invisFlag] < 1)    /// can't see invisible without proper detection
                    return false;
            }

            if (obj->IsStealth())       /// Stealth Detection ( I Hate Rogues :P )
            {
                if (isInFront(obj))     /// stealthed player is in front of creature
                {
                    // Detection Range = 5yds + (Detection Skill - Stealth Skill)/5
                        detectRange = 5.0f + getLevel() + (0.2f * (float)(GetStealthDetectBonus()) - obj->GetStealthLevel());

                    if (detectRange < 1.0f) detectRange = 1.0f;     /// Minimum Detection Range = 1yd
                }
                else /// stealthed player is behind creature
                {
                    if (GetStealthDetectBonus() > 1000) return true;    /// immune to stealth
                    else detectRange = 0.0f;
                }

                detectRange += GetBoundingRadius();         /// adjust range for size of creature
                detectRange += obj->GetBoundingRadius();    /// adjust range for size of stealthed player

                if (GetDistance2dSq(obj) > detectRange * detectRange)
                    return false;
            }

            return true;
        }

        /// Make this unit face another unit
        bool setInFront(Unit* target);

        /// Looting
        void generateLoot();
        bool HasLootForPlayer(Player* plr);

        bool Skinned;
        uint32 GetRequiredLootSkill();

        // Misc
        inline void setEmoteState(uint8 emote) { m_emoteState = emote; };
        inline uint32 GetSQL_id() { return spawnid; };

        virtual void setDeathState(DeathState s);

        uint32 GetOldEmote() { return m_oldEmote; }

        void SendChatMessage(uint8 type, uint32 lang, const char* msg, uint32 delay = 0);
        void SendScriptTextChatMessage(uint32 textid);
        void SendTimedScriptTextChatMessage(uint32 textid, uint32 delay = 0);
        void SendChatMessageToPlayer(uint8 type, uint32 lang, const char* msg, Player* plr);

        // Serialization
        void SaveToDB();
        void LoadAIAgents(CreatureTemplate* t);
        void DeleteFromDB();

        void OnJustDied();
        void OnRemoveCorpse();
        void OnRespawn(MapMgr* m);

        void BuildPetSpellList(WorldPacket & data);

    protected:
        virtual void SafeDelete();      /// use DeleteMe() instead of SafeDelete() to avoid crashes like InWorld Creatures deleted.
    public:
        // In Range
        void AddInRangeObject(Object* pObj);
        void OnRemoveInRangeObject(Object* pObj);
        void ClearInRangeSet();

        // Demon
        void EnslaveExpire();

        // Pet
        void UpdatePet();
        uint32 GetEnslaveCount() { return m_enslaveCount; }
        void SetEnslaveCount(uint32 count) { m_enslaveCount = count; }
        uint32 GetEnslaveSpell() { return m_enslaveSpell; }
        void SetEnslaveSpell(uint32 spellId) { m_enslaveSpell = spellId; }
        bool RemoveEnslave();

        Object* GetPlayerOwner();

        Group* GetGroup();

        int32 GetDamageDoneMod(uint32 school)
        {
            if (school >= SCHOOL_COUNT)
                return 0;

            return ModDamageDone[ school ];
        }

        float GetDamageDonePctMod(uint32 school)
        {
            if (school >= SCHOOL_COUNT)
                return 0;

            return ModDamageDonePct[ school ];
        }

        inline bool IsPickPocketed() { return m_PickPocketed; }
        inline void SetPickPocketed(bool val = true) { m_PickPocketed = val; }

        inline CreatureAIScript* GetScript() { return _myScriptClass; }
        void LoadScript();

        void CallScriptUpdate();

        inline CreatureInfo* GetCreatureInfo() { return creature_info; }
        inline void SetCreatureInfo(CreatureInfo* ci) { creature_info = ci; }
        void SetCreatureProto(CreatureProto* cp) { proto = cp; }

        inline Trainer* GetTrainer() { return mTrainer; }
        void RegenerateFocus();

        CreatureFamilyEntry* myFamily;

        inline bool IsExotic()
        {
            if ((GetCreatureInfo()->Flags1 & CREATURE_FLAG1_EXOTIC) != 0)
                return true;

            return false;
        }


        bool isCritter();
        bool isTrainingDummy()
        {

            if (GetProto()->isTrainingDummy)
                return true;
            else
                return false;
        }

        void FormationLinkUp(uint32 SqlId);
        void ChannelLinkUpGO(uint32 SqlId);
        void ChannelLinkUpCreature(uint32 SqlId);
        bool haslinkupevent;
        WayPoint* CreateWaypointStruct();
        uint32 spawnid;
        uint32 original_emotestate;

        CreatureSpawn* m_spawn;

        void OnPushToWorld();
        virtual void Despawn(uint32 delay, uint32 respawntime);
        void TriggerScriptEvent(int);

        AuctionHouse* auctionHouse;

        void DeleteMe();
        bool CanAddToWorld();

        // scriptdev2
        uint32 GetNpcTextId();

        WayPointMap* m_custom_waypoint_map;
        void LoadWaypointGroup(uint32 pWaypointGroup);
        void LoadCustomWaypoint(float pX, float pY, float pZ, float pO, uint32 pWaitTime, uint32 pFlags, bool pForwardEmoteOneshot, uint32 pForwardEmoteId, bool pBackwardEmoteOneshot, uint32 pBackwardEmoteId, uint32 pForwardSkinId, uint32 pBackwardSkinId);
        void SwitchToCustomWaypoints();
        Player* m_escorter;
        void DestroyCustomWaypointMap();
        bool IsInLimboState() { return m_limbostate; }
        void SetLimboState(bool set) { m_limbostate = set; };
        uint32 GetLineByFamily(CreatureFamilyEntry* family) {return family->skilline ? family->skilline : 0;};
        void RemoveLimboState(Unit* healer);
        void SetGuardWaypoints();
        bool m_corpseEvent;
        MapCell* m_respawnCell;
        bool m_noRespawn;
        uint32 m_respawnTimeOverride;

        float GetBaseParry();
        bool isattackable(CreatureSpawn* spawn);

        void DealDamage(Unit* pVictim, uint32 damage, uint32 targetEvent, uint32 unitEvent, uint32 spellId, bool no_remove_auras = false);
        void TakeDamage(Unit* pAttacker, uint32 damage, uint32 spellid, bool no_remove_auras = false);
        void Die(Unit* pAttacker, uint32 damage, uint32 spellid);

        void HandleMonsterSayEvent(MONSTER_SAY_EVENTS Event);

        uint32 GetType() { return m_Creature_type; }
        void SetType(uint32 t) { m_Creature_type = t; }

    protected:
        CreatureAIScript* _myScriptClass;
        bool m_limbostate;
        Trainer* mTrainer;

        void _LoadGoods();
        void _LoadGoods(std::list<CreatureItem*>* lst);
        void _LoadMovement();

        /// Vendor data
        std::vector<CreatureItem>* m_SellItems;

        /// Taxi data
        uint32 mTaxiNode;

        /// Quest data
        std::list<QuestRelation*>* m_quests;

        uint32 m_enslaveCount;
        uint32 m_enslaveSpell;

        bool m_PickPocketed;
        uint32 _fields[UNIT_END];
        uint32 m_healthfromspell;

        CreatureInfo* creature_info;
        CreatureProto* proto;
    private:
        uint32 m_Creature_type;
};

#endif // _WOWSERVER_CREATURE_H
