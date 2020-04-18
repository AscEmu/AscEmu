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

#ifndef WOWSERVER_CREATURE_H
#define WOWSERVER_CREATURE_H

#include "CreatureDefines.hpp"
#include "Units/UnitDefines.hpp"
#include "Map/Map.h"
#include "Units/Unit.h"
#include "Movement/UnitMovementManager.hpp"
#include "Objects/Object.h"
#include "Management/Group.h"

SERVER_DECL bool Rand(float chance);
SERVER_DECL bool Rand(uint32_t chance);
SERVER_DECL bool Rand(int32_t chance);

class CreatureAIScript;
class GossipScript;
class AuctionHouse;
struct Trainer;
class GameEvent;
struct QuestRelation;
struct QuestProperties;

#define CALL_SCRIPT_EVENT(obj, func) if (obj->IsInWorld() && obj->isCreature() && static_cast<Creature*>(obj)->GetScript() != NULL) static_cast<Creature*>(obj)->GetScript()->func

uint8_t get_byte(uint32_t buffer, uint32_t index);


//MIT start
class SERVER_DECL Creature : public Unit
{
public:

    GameEvent * mEvent = nullptr;

    // npc flag helper
    bool isVendor() const;
    bool isTrainer() const;
    bool isClassTrainer() const;
    bool isProfessionTrainer() const;
    bool isQuestGiver() const;
    bool isGossip() const;
    bool isTaxi() const;
    bool isCharterGiver() const;
    bool isGuildBank() const;
    bool isBattleMaster() const;
    bool isBanker() const;
    bool isInnkeeper() const;
    bool isSpiritHealer() const;
    bool isTabardDesigner() const;
    bool isAuctioneer() const;
    bool isStableMaster() const;
    bool isArmorer() const;

    //type helper
    bool isVehicle() const override;
    bool isTrainingDummy() override;

    //pvp helper
    bool isPvpFlagSet() override;
    void setPvpFlag() override;
    void removePvpFlag() override;

    bool isFfaPvpFlagSet() override;
    void setFfaPvpFlag() override;
    void removeFfaPvpFlag() override;

    bool isSanctuaryFlagSet() override;
    void setSanctuaryFlag() override;
    void removeSanctuaryFlag() override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Owner
    Player* getPlayerOwner() override;

 //MIT end

        bool Teleport(const LocationVector& vec, MapMgr* map) override;

        Creature(uint64_t guid);
        virtual ~Creature();

        void addVehicleComponent(uint32_t creature_entry, uint32_t vehicleid);
        void removeVehicleComponent();

        bool Load(MySQLStructure::CreatureSpawn* spawn, uint8_t mode, MySQLStructure::MapInfo const* info);
        void Load(CreatureProperties const* c_properties, float x, float y, float z, float o = 0);

        void AddToWorld();
        void AddToWorld(MapMgr* pMapMgr);
        void RemoveFromWorld(bool addrespawnevent, bool free_guid);
        void RemoveFromWorld(bool free_guid);

        // remove auras, guardians, scripts
        void PrepareForRemove();

        // Creation
        void Create(uint32_t mapid, float x, float y, float z, float ang);
        void CreateWayPoint(uint32_t WayPointID, uint32_t mapid, float x, float y, float z, float ang);

        // Updates
        void Update(unsigned long time_passed);

        // Creature inventory
        uint32_t GetItemIdBySlot(uint32_t slot) { return m_SellItems->at(slot).itemid; }
        uint32_t GetItemAmountBySlot(uint32_t slot) { return m_SellItems->at(slot).amount; }

        bool HasItems();
        void SummonExpire()
        {
            DeleteMe();
        }

        int32_t GetSlotByItemId(uint32_t itemid);

        uint32_t GetItemAmountByItemId(uint32_t itemid);

        void GetSellItemBySlot(uint32_t slot, CreatureItem& ci);

        void GetSellItemByItemId(uint32_t itemid, CreatureItem& ci);

#if VERSION_STRING < Cata
        DBC::Structures::ItemExtendedCostEntry const* GetItemExtendedCostByItemId(uint32_t itemid);
#else
        DB2::Structures::ItemExtendedCostEntry const* GetItemExtendedCostByItemId(uint32_t itemid);
#endif

        std::vector<CreatureItem>::iterator GetSellItemBegin();

        std::vector<CreatureItem>::iterator GetSellItemEnd();

    //MIT
        std::vector<CreatureItem>* getSellItems();
    //MIT end

        size_t GetSellItemCount();

        void RemoveVendorItem(uint32_t itemid);
#if VERSION_STRING < Cata
        void AddVendorItem(uint32_t itemid, uint32_t amount, DBC::Structures::ItemExtendedCostEntry const* ec);
#else
        void AddVendorItem(uint32_t itemid, uint32_t amount, DB2::Structures::ItemExtendedCostEntry const* ec);
#endif
        void ModAvItemAmount(uint32_t itemid, uint32_t value);
        void UpdateItemAmount(uint32_t itemid);

        // Quests
        void _LoadQuests();
        bool HasQuests();
        bool HasQuest(uint32_t id, uint32_t type);
        void AddQuest(QuestRelation* Q);
        void DeleteQuest(QuestRelation* Q);
        QuestProperties const* FindQuest(uint32_t quest_id, uint8_t quest_relation);
        uint16_t GetQuestRelation(uint32_t quest_id);
        uint32_t NumOfQuests();
        std::list<QuestRelation*>::iterator QuestsBegin();
        std::list<QuestRelation*>::iterator QuestsEnd();
        void SetQuestList(std::list<QuestRelation*>* qst_lst);

        uint32_t GetHealthFromSpell();

        void SetHealthFromSpell(uint32_t value);

        int32_t m_speedFromHaste;
        int32_t FlatResistanceMod[TOTAL_SPELL_SCHOOLS];
        int32_t BaseResistanceModPct[TOTAL_SPELL_SCHOOLS];
        int32_t ResistanceModPct[TOTAL_SPELL_SCHOOLS];

        int32_t FlatStatMod[5];
        int32_t StatModPct[5];
        int32_t TotalStatModPct[5];

        int32_t ModDamageDone[TOTAL_SPELL_SCHOOLS];
        float ModDamageDonePct[TOTAL_SPELL_SCHOOLS];
        void CalcResistance(uint8_t type);
        void CalcStat(uint8_t type);

        bool m_canRegenerateHP;
        void RegenerateHealth();
        int BaseAttackType;

        // Looting
        void generateLoot();
        bool HasLootForPlayer(Player* plr);

        bool Skinned;
        uint32_t GetRequiredLootSkill();

        // Misc
        uint32_t GetSQL_id();

        virtual void setDeathState(DeathState s);

        uint32_t GetOldEmote();

        void SendChatMessage(uint8_t type, uint32_t lang, const char* msg, uint32_t delay = 0);
        void SendScriptTextChatMessage(uint32_t textid);
        void SendTimedScriptTextChatMessage(uint32_t textid, uint32_t delay = 0);
        void SendChatMessageToPlayer(uint8_t type, uint32_t lang, const char* msg, Player* plr);

        // Serialization
        void SaveToDB();
        void DeleteFromDB();

        void OnRemoveCorpse();
        void OnRespawn(MapMgr* m);

        void BuildPetSpellList(WorldPacket & data);

    protected:

        virtual void SafeDelete();      // use DeleteMe() instead of SafeDelete() to avoid crashes like InWorld Creatures deleted.

    public:

        // In Range
        void addToInRangeObjects(Object* pObj) override;
        void onRemoveInRangeObject(Object* pObj) override;
        void clearInRangeSets() override;

        // Demon
        void EnslaveExpire();

        // Pet
        uint32_t GetEnslaveCount();

        void SetEnslaveCount(uint32_t count);

        uint32_t GetEnslaveSpell();

        void SetEnslaveSpell(uint32_t spellId);
        bool RemoveEnslave();

        Group* GetGroup() override;

        int32_t GetDamageDoneMod(uint16_t school) override;

        float GetDamageDonePctMod(uint16_t school) override;

        bool IsPickPocketed();

        void SetPickPocketed(bool val = true);

        CreatureAIScript* GetScript();
        void LoadScript();

        //MIT
        void registerDatabaseGossip();

        void CallScriptUpdate();

        CreatureProperties const* GetCreatureProperties();
        void SetCreatureProperties(CreatureProperties const* creature_properties);

        Trainer* GetTrainer();

        DBC::Structures::CreatureFamilyEntry const* myFamily;

        bool IsExotic();


        bool isCritter() override;

        void FormationLinkUp(uint32_t SqlId);
        void ChannelLinkUpGO(uint32_t SqlId);
        void ChannelLinkUpCreature(uint32_t SqlId);
        bool haslinkupevent;
        Movement::WayPoint* CreateWaypointStruct();
        uint32_t spawnid;
        uint32_t original_emotestate;

        MySQLStructure::CreatureSpawn* m_spawn;

        void OnPushToWorld() override;
        virtual void Despawn(uint32_t delay, uint32_t respawntime);
        void TriggerScriptEvent(int);

        AuctionHouse* auctionHouse;

        void DeleteMe();
        bool CanAddToWorld();

        // scriptdev2
        uint32_t GetNpcTextId();

        Movement::WayPointMap* m_custom_waypoint_map;
        void LoadWaypointGroup(uint32_t pWaypointGroup);
        void LoadCustomWaypoint(float pX, float pY, float pZ, float pO, uint32_t pWaitTime, uint32_t pFlags, bool pForwardEmoteOneshot, uint32_t pForwardEmoteId, bool pBackwardEmoteOneshot, uint32_t pBackwardEmoteId, uint32_t pForwardSkinId, uint32_t pBackwardSkinId);
        void SwitchToCustomWaypoints();
        Player* m_escorter;
        void DestroyCustomWaypointMap();
        bool IsInLimboState();

        void SetLimboState(bool set);
    static uint32_t GetLineByFamily(DBC::Structures::CreatureFamilyEntry const* family);
        void RemoveLimboState(Unit* healer);
        void SetGuardWaypoints();
        bool m_corpseEvent;
        MapCell* m_respawnCell;
        bool m_noRespawn;
        uint32_t m_respawnTimeOverride;

        float GetBaseParry();
        bool isattackable(MySQLStructure::CreatureSpawn* spawn);

        void DealDamage(Unit* pVictim, uint32_t damage, uint32_t targetEvent, uint32_t unitEvent, uint32_t spellId, bool no_remove_auras = false);
        void TakeDamage(Unit* pAttacker, uint32_t damage, uint32_t spellid, bool no_remove_auras = false);
        void Die(Unit* pAttacker, uint32_t damage, uint32_t spellid);

        void HandleMonsterSayEvent(MONSTER_SAY_EVENTS Event);

        uint32_t GetType();

        void SetType(uint32_t t);

    protected:

        CreatureAIScript* _myScriptClass;
        bool m_limbostate;
        Trainer* mTrainer;

        // Vendor data
        std::vector<CreatureItem>* m_SellItems;

        // Taxi data
        uint32_t mTaxiNode;

        // Quest data
        std::list<QuestRelation*>* m_quests;

        uint32_t m_enslaveCount;
        uint32_t m_enslaveSpell;

        bool m_PickPocketed;
        uint32_t _fields[UNIT_END];
        uint32_t m_healthfromspell;

        CreatureProperties const* creature_properties;

    private:

        uint32_t m_Creature_type;

        // old EasyFunctions.h
    public:

        void DeleteWaypoints()
        {
            if (m_custom_waypoint_map == nullptr)
                return;

            for (auto i = m_custom_waypoint_map->begin(); i != m_custom_waypoint_map->end(); ++i)
            {
                if ((*i) != nullptr)
                    delete(*i);
            }

            m_custom_waypoint_map->clear();
        }

        void CreateCustomWaypointMap()
        {
            if (m_custom_waypoint_map == nullptr)
                m_custom_waypoint_map = new Movement::WayPointMap;
            else
                DeleteWaypoints();
        }
};

#endif // _WOWSERVER_CREATURE_H
