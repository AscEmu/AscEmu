/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CreatureDefines.hpp"
#include "Objects/Units/UnitDefines.hpp"
#include "Map/Map.h"
#include "Objects/Units/Unit.h"
#include "Objects/Object.h"
#include "Management/Group.h"
#include "Movement/MovementDefines.h"
#include "Server/UpdateFieldInclude.h"

class CreatureAIScript;
class GossipScript;
class AuctionHouse;
struct Trainer;
class GameEvent;
struct QuestRelation;
struct QuestProperties;
class CreatureGroup;

enum MovementGeneratorType : uint8_t;

uint8 get_byte(uint32 buffer, uint32 index);


class SERVER_DECL Creature : public Unit
{
public:

    Creature(uint64_t guid);
    virtual ~Creature();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Essential functions

    void Update(unsigned long time_passed);             // hides function Unit::Update
    void AddToWorld();                                  // hides virtual function Object::AddToWorld
    void AddToWorld(MapMgr* pMapMgr);                   // hides virtual function Object::AddToWorld
    // void PushToWorld(MapMgr*);                       // not used
    void RemoveFromWorld(bool free_guid);               // hides virtual function Unit::RemoveFromWorld
    // void OnPrePushToWorld();                         // not used
    void OnPushToWorld() override;                      // overrides virtual function Unit::OnPushToWorld
    // void OnPreRemoveFromWorld();                     // not used
    // void OnRemoveFromWorld();                        // not used

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
#if VERSION_STRING >= Cata
    bool isTransmog() const;
    bool isReforger() const;
    bool isVoidStorage() const;
#endif

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

    std::vector<CreatureItem>* getSellItems();

    uint32_t getWaypointPath() const { return _waypointPathId; }
    void loadPath(uint32_t pathid) { _waypointPathId = pathid; }

    // nodeId, pathId
    std::pair<uint32_t, uint32_t> getCurrentWaypointInfo() const { return _currentWaypointNodeInfo; }
    void updateCurrentWaypointInfo(uint32_t nodeId, uint32_t pathId) { _currentWaypointNodeInfo = { nodeId, pathId }; }

    virtual void setDeathState(DeathState s);

    void addToInRangeObjects(Object* pObj) override;
    void onRemoveInRangeObject(Object* pObj) override;
    void clearInRangeSets() override;

    void registerDatabaseGossip();

    bool isReturningHome() const;
    void searchFormation();
    CreatureGroup* getFormation() { return m_formation; }
    void setFormation(CreatureGroup* formation) { m_formation = formation; }
    bool isFormationLeader() const;
    void signalFormationMovement();
    bool isFormationLeaderMoveAllowed() const;

    uint32_t getSpawnId() { return spawnid; }

    void motion_Initialize();
    void immediateMovementFlagsUpdate();
    void updateMovementFlags();
    CreatureMovementData const& getMovementTemplate();
    bool canWalk() { return getMovementTemplate().isGroundAllowed(); }
    bool canSwim() override { return getMovementTemplate().isSwimAllowed() || isPet(); }
    bool canFly()  override { return getMovementTemplate().isFlightAllowed() || IsFlying(); }
    bool canHover() { return getMovementTemplate().Ground == CreatureGroundMovementType::Hover || isHovering(); }

    MovementGeneratorType getDefaultMovementType() const override { return m_defaultMovementType; }
    void setDefaultMovementType(MovementGeneratorType mgt) { m_defaultMovementType = mgt; }

 //AGPL Starts

        bool Teleport(const LocationVector& vec, MapMgr* map) override;

        bool Load(MySQLStructure::CreatureSpawn* spawn, uint8 mode, MySQLStructure::MapInfo const* info);
        void Load(CreatureProperties const* c_properties, float x, float y, float z, float o = 0);

        void RemoveFromWorld(bool addrespawnevent, bool free_guid);

        // remove auras, guardians, scripts
        void PrepareForRemove();

        // Creation
        void Create(uint32 mapid, float x, float y, float z, float ang);
        void CreateWayPoint(uint32 WayPointID, uint32 mapid, float x, float y, float z, float ang);

        // Creature inventory
        uint32 GetItemIdBySlot(uint32 slot) { return m_SellItems->at(slot).itemid; }
        uint32 GetItemAmountBySlot(uint32 slot) { return m_SellItems->at(slot).amount; }

        bool HasItems();
        void InitSummon(Object* summoner);
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

        int32 m_speedFromHaste = 0;
        int32 FlatResistanceMod[TOTAL_SPELL_SCHOOLS] = {0};
        int32 BaseResistanceModPct[TOTAL_SPELL_SCHOOLS] = {0};
        int32 ResistanceModPct[TOTAL_SPELL_SCHOOLS] = {0};

        int32 FlatStatMod[5] = {0};
        int32 StatModPct[5] = {0};
        int32 TotalStatModPct[5] = {0};

        int32 ModDamageDone[TOTAL_SPELL_SCHOOLS] = {0};
        float ModDamageDonePct[TOTAL_SPELL_SCHOOLS] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
        void CalcResistance(uint8_t type);
        void CalcStat(uint8_t type);

        bool m_canRegenerateHP = true;
        void RegenerateHealth();
        uint8_t BaseAttackType = SCHOOL_NORMAL;

        // Looting
        bool HasLootForPlayer(Player* plr);

        bool Skinned = false;
        uint16_t GetRequiredLootSkill();

        // Misc
        uint32 GetSQL_id();
        void SetTransportHomePosition(float x, float y, float z, float o) { m_transportHomePosition.x = x, m_transportHomePosition.y = y, m_transportHomePosition.z = z, m_transportHomePosition.o = o; }
        void SetTransportHomePosition(const LocationVector &pos) { m_transportHomePosition = pos; }
        void GetTransportHomePosition(float &x, float &y, float &z, float &ori) { x = m_transportHomePosition.x, y = m_transportHomePosition.y, z = m_transportHomePosition.z, ori = m_transportHomePosition.o; }
        LocationVector GetTransportHomePosition() { return m_transportHomePosition; }
        LocationVector m_transportHomePosition;

        void SendScriptTextChatMessage(uint32 textid, Unit* target = nullptr);
        void SendTimedScriptTextChatMessage(uint32 textid, uint32 delay = 0, Unit* target = nullptr);

        // Serialization
        void SaveToDB();
        void DeleteFromDB();

        void OnRemoveCorpse();
        void OnRespawn(MapMgr* m);

        void BuildPetSpellList(WorldPacket & data);

    private:
        // Waypoint path
        uint32_t _waypointPathId = 0;
        std::pair<uint32_t/*nodeId*/, uint32_t/*pathId*/> _currentWaypointNodeInfo = {0, 0};

    protected:

        virtual void SafeDelete();      // use DeleteMe() instead of SafeDelete() to avoid crashes like InWorld Creatures deleted.

    public:

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

        void CallScriptUpdate(unsigned long time_passed);

        CreatureProperties const* GetCreatureProperties();
        void SetCreatureProperties(CreatureProperties const* creature_properties);

        Trainer* GetTrainer();

        DBC::Structures::CreatureFamilyEntry const* myFamily = nullptr;

        bool IsExotic();
        bool isCritter() override;

        void ChannelLinkUpGO(uint32 SqlId);
        void ChannelLinkUpCreature(uint32 SqlId);

        uint32 spawnid = 0;
        uint32 original_emotestate = 0;

        MySQLStructure::CreatureSpawn* m_spawn = nullptr;

        virtual void Despawn(uint32 delay, uint32 respawntime);
        void TriggerScriptEvent(int);

        AuctionHouse* auctionHouse = nullptr;

        void DeleteMe();
        bool CanAddToWorld();

        // scriptdev2
        uint32 GetNpcTextId();

        Player* m_escorter = nullptr;
        bool IsInLimboState();

        void SetLimboState(bool set);
    static uint32 GetLineByFamily(DBC::Structures::CreatureFamilyEntry const* family);
        void RemoveLimboState(Unit* healer);

        bool m_corpseEvent = false;
        MapCell* m_respawnCell = nullptr;
        bool m_noRespawn = false;
        uint32 m_respawnTimeOverride = 0;

        float GetBaseParry();
        bool isattackable(MySQLStructure::CreatureSpawn* spawn);

        void die(Unit* pAttacker, uint32 damage, uint32 spellid) override;

        uint32 GetType();

        void SetType(uint32 t);

    protected:

        CreatureAIScript* _myScriptClass = nullptr;
        bool m_limbostate = false;
        Trainer* mTrainer = nullptr;

        // Movement
        MovementGeneratorType m_defaultMovementType = IDLE_MOTION_TYPE;

        // Vendor data
        std::vector<CreatureItem>* m_SellItems = nullptr;

        // Taxi data
        uint32 mTaxiNode = 0;

        // Quest data
        std::list<QuestRelation*>* m_quests = nullptr;

        uint32 m_enslaveCount = 0;
        uint32 m_enslaveSpell = 0;

        bool m_PickPocketed = false;
        uint32 _fields[getSizeOfStructure(WoWUnit)];
        uint32 m_healthfromspell = 0;

        CreatureProperties const* creature_properties = nullptr;

    private:

        uint32 m_Creature_type = 0;

        uint16_t m_movementFlagUpdateTimer = 1000;

        // Formation var
        CreatureGroup* m_formation = nullptr;

        float_t m_wanderDistance = 0.0f;
};
