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

#ifndef WOWSERVER_CREATURE_H
#define WOWSERVER_CREATURE_H

#include "CreatureDefines.hpp"
#include "Units/UnitDefines.hpp"
#include "Map/Map.h"
#include "Units/Unit.h"
#include "Objects/Object.h"
#include "Management/Group.h"

class CreatureAIScript;
class GossipScript;
class AuctionHouse;
struct Trainer;
class GameEvent;
struct QuestRelation;
struct QuestProperties;
class CreatureGroup;

enum MovementGeneratorType : uint8;

uint8 get_byte(uint32 buffer, uint32 index);


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

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    float_t getMaxWanderDistance() const;
    void setMaxWanderDistance(float_t dist);

 //MIT end

        bool Teleport(const LocationVector& vec, MapMgr* map) override;

        Creature(uint64 guid);
        virtual ~Creature();

        void addVehicleComponent(uint32 creature_entry, uint32 vehicleid);
        void removeVehicleComponent();

        bool Load(MySQLStructure::CreatureSpawn* spawn, uint8 mode, MySQLStructure::MapInfo const* info);
        void Load(CreatureProperties const* c_properties, float x, float y, float z, float o = 0);

        void AddToWorld();
        void AddToWorld(MapMgr* pMapMgr);
        void RemoveFromWorld(bool addrespawnevent, bool free_guid);
        void RemoveFromWorld(bool free_guid);

        // remove auras, guardians, scripts
        void PrepareForRemove();

        // Creation
        void Create(uint32 mapid, float x, float y, float z, float ang);
        void CreateWayPoint(uint32 WayPointID, uint32 mapid, float x, float y, float z, float ang);

        // Updates
        void Update(unsigned long time_passed);

        // Creature inventory
        uint32 GetItemIdBySlot(uint32 slot) { return m_SellItems->at(slot).itemid; }
        uint32 GetItemAmountBySlot(uint32 slot) { return m_SellItems->at(slot).amount; }

        bool HasItems();
        void SummonExpire()
        {
            DeleteMe();
        }

        int32 GetSlotByItemId(uint32 itemid);

        uint32 GetItemAmountByItemId(uint32 itemid);

        void GetSellItemBySlot(uint32 slot, CreatureItem& ci);

        void GetSellItemByItemId(uint32 itemid, CreatureItem& ci);

#if VERSION_STRING < Cata
        DBC::Structures::ItemExtendedCostEntry const* GetItemExtendedCostByItemId(uint32 itemid);
#else
        DB2::Structures::ItemExtendedCostEntry const* GetItemExtendedCostByItemId(uint32 itemid);
#endif

        std::vector<CreatureItem>::iterator GetSellItemBegin();

        std::vector<CreatureItem>::iterator GetSellItemEnd();

    //MIT
        std::vector<CreatureItem>* getSellItems();
    //MIT end

        size_t GetSellItemCount();

        void RemoveVendorItem(uint32 itemid);
#if VERSION_STRING < Cata
        void AddVendorItem(uint32 itemid, uint32 amount, DBC::Structures::ItemExtendedCostEntry const* ec);
#else
        void AddVendorItem(uint32 itemid, uint32 amount, DB2::Structures::ItemExtendedCostEntry const* ec);
#endif
        void ModAvItemAmount(uint32 itemid, uint32 value);
        void UpdateItemAmount(uint32 itemid);

        // Quests
        void _LoadQuests();
        bool HasQuests();
        bool HasQuest(uint32 id, uint32 type);
        void AddQuest(QuestRelation* Q);
        void DeleteQuest(QuestRelation* Q);
        QuestProperties const* FindQuest(uint32 quest_id, uint8 quest_relation);
        uint16 GetQuestRelation(uint32 quest_id);
        uint32 NumOfQuests();
        std::list<QuestRelation*>::iterator QuestsBegin();
        std::list<QuestRelation*>::iterator QuestsEnd();
        void SetQuestList(std::list<QuestRelation*>* qst_lst);

        uint32 GetHealthFromSpell();

        void SetHealthFromSpell(uint32 value);

        int32 m_speedFromHaste;
        int32 FlatResistanceMod[TOTAL_SPELL_SCHOOLS];
        int32 BaseResistanceModPct[TOTAL_SPELL_SCHOOLS];
        int32 ResistanceModPct[TOTAL_SPELL_SCHOOLS];

        int32 FlatStatMod[5];
        int32 StatModPct[5];
        int32 TotalStatModPct[5];

        int32 ModDamageDone[TOTAL_SPELL_SCHOOLS];
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
        uint32 GetRequiredLootSkill();

        // Misc
        uint32 GetSQL_id();
        void SetTransportHomePosition(float x, float y, float z, float o) { m_transportHomePosition.x = x, m_transportHomePosition.y = y, m_transportHomePosition.z = z, m_transportHomePosition.o = o; }
        void SetTransportHomePosition(const LocationVector &pos) { m_transportHomePosition = pos; }
        void GetTransportHomePosition(float &x, float &y, float &z, float &ori) { x = m_transportHomePosition.x, y = m_transportHomePosition.y, z = m_transportHomePosition.z, ori = m_transportHomePosition.o; }
        LocationVector GetTransportHomePosition() { return m_transportHomePosition; }
        LocationVector m_transportHomePosition;

        uint32_t getWaypointPath() const { return _waypointPathId; }
        void loadPath(uint32_t pathid) { _waypointPathId = pathid; }

        // nodeId, pathId
        std::pair<uint32_t, uint32_t> getCurrentWaypointInfo() const { return _currentWaypointNodeInfo; }
        void updateCurrentWaypointInfo(uint32_t nodeId, uint32_t pathId) { _currentWaypointNodeInfo = { nodeId, pathId }; }

        virtual void setDeathState(DeathState s);

        void SendChatMessage(uint8 type, uint32 lang, const char* msg, uint32 delay = 0);
        void SendScriptTextChatMessage(uint32 textid);
        void SendTimedScriptTextChatMessage(uint32 textid, uint32 delay = 0);
        void SendChatMessageToPlayer(uint8 type, uint32 lang, const char* msg, Player* plr);

        // Serialization
        void SaveToDB();
        void DeleteFromDB();

        void OnRemoveCorpse();
        void OnRespawn(MapMgr* m);

        void BuildPetSpellList(WorldPacket & data);

    private:
        // Waypoint path
        uint32_t _waypointPathId;
        std::pair<uint32_t/*nodeId*/, uint32_t/*pathId*/> _currentWaypointNodeInfo;

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
        uint32 GetEnslaveCount();

        void SetEnslaveCount(uint32 count);

        uint32 GetEnslaveSpell();

        void SetEnslaveSpell(uint32 spellId);
        bool RemoveEnslave();

        int32 GetDamageDoneMod(uint16_t school) override;

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

        bool isReturningHome() const;
        void searchFormation();
        CreatureGroup* getFormation() { return m_formation; }
        void setFormation(CreatureGroup* formation) { m_formation = formation; }
        bool isFormationLeader() const;
        void signalFormationMovement();
        bool isFormationLeaderMoveAllowed() const;

        uint32_t getSpawnId() { return spawnid; }


        void ChannelLinkUpGO(uint32 SqlId);
        void ChannelLinkUpCreature(uint32 SqlId);

        uint32 spawnid;
        uint32 original_emotestate;

        void Motion_Initialize();
        void immediateMovementFlagsUpdate();
        void updateMovementFlags();
        CreatureMovementData const& GetMovementTemplate();
        bool CanWalk() { return GetMovementTemplate().IsGroundAllowed(); }
        bool CanSwim() override { return GetMovementTemplate().IsSwimAllowed() || isPet(); }
        bool CanFly()  override { return GetMovementTemplate().IsFlightAllowed() || IsFlying(); }
        bool CanHover() { return GetMovementTemplate().Ground == CreatureGroundMovementType::Hover || IsHovering(); }

        MovementGeneratorType GetDefaultMovementType() const override { return m_defaultMovementType; }
        void SetDefaultMovementType(MovementGeneratorType mgt) { m_defaultMovementType = mgt; }

        MySQLStructure::CreatureSpawn* m_spawn;

        void OnPushToWorld() override;
        virtual void Despawn(uint32 delay, uint32 respawntime);
        void TriggerScriptEvent(int);

        AuctionHouse* auctionHouse;

        void DeleteMe();
        bool CanAddToWorld();

        // scriptdev2
        uint32 GetNpcTextId();

        Player* m_escorter;
        bool IsInLimboState();

        void SetLimboState(bool set);
    static uint32 GetLineByFamily(DBC::Structures::CreatureFamilyEntry const* family);
        void RemoveLimboState(Unit* healer);

        bool m_corpseEvent;
        MapCell* m_respawnCell;
        bool m_noRespawn;
        uint32 m_respawnTimeOverride;

        float GetBaseParry();
        bool isattackable(MySQLStructure::CreatureSpawn* spawn);

        void Die(Unit* pAttacker, uint32 damage, uint32 spellid) override;

        void HandleMonsterSayEvent(MONSTER_SAY_EVENTS Event);

        uint32 GetType();

        void SetType(uint32 t);

    protected:

        CreatureAIScript* _myScriptClass;
        bool m_limbostate;
        Trainer* mTrainer;

        // Movement
        MovementGeneratorType m_defaultMovementType;

        // Vendor data
        std::vector<CreatureItem>* m_SellItems;

        // Taxi data
        uint32 mTaxiNode;

        // Quest data
        std::list<QuestRelation*>* m_quests;

        uint32 m_enslaveCount;
        uint32 m_enslaveSpell;

        bool m_PickPocketed;
        uint32 _fields[getSizeOfStructure(WoWUnit)];
        uint32 m_healthfromspell;

        CreatureProperties const* creature_properties;

    private:

        uint32 m_Creature_type;

        uint16_t m_movementFlagUpdateTimer = 1000;

        // Formation var
        CreatureGroup* m_formation;

        float_t m_wanderDistance = 0.0f;

        // old EasyFunctions.h
};

#endif // _WOWSERVER_CREATURE_H
