/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CreatureDefines.hpp"
#include "Objects/Units/UnitDefines.hpp"
#include "Objects/Units/Unit.hpp"
#include "Objects/Object.hpp"
#include "Movement/MovementDefines.h"
#include "Server/UpdateFieldInclude.h"

namespace WDB::Structures
{
    struct CreatureFamilyEntry;
}

class CreatureAIScript;
class GossipScript;
class AuctionHouse;
struct Trainer;
class GameEvent;
struct QuestRelation;
struct QuestProperties;
class CreatureGroup;

namespace MySQLStructure
{
    struct CreatureSpawn;
    struct MapInfo;
}

enum MovementGeneratorType : uint8_t;

uint8_t get_byte(uint32_t buffer, uint32_t index);

class SERVER_DECL Creature : public Unit
{
public:
    Creature(uint64_t guid);
    virtual ~Creature();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Essential functions

    virtual void Update(unsigned long time_passed);     // hides function Unit::Update
    virtual void onPreAttachToWorld() override;
    virtual void onAttachToWorld() override;

    virtual void onPreDetachFromWorld() override;
    virtual void onDetachFromWorld() override;

    void OnLoaded();

    virtual void sendSpellsToController(Unit* controller, uint32_t duration);

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
    bool isPvpFlagSet() const override;
    void setPvpFlag() override;
    void removePvpFlag() override;

    bool isFfaPvpFlagSet() const override;
    void setFfaPvpFlag() override;
    void removeFfaPvpFlag() override;

    bool isSanctuaryFlagSet() const override;
    void setSanctuaryFlag() override;
    void removeSanctuaryFlag() override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Owner
    // Returns unit charmer or unit owner
    Unit* getUnitOwner() override;
    // Returns unit charmer or unit owner
    Unit const* getUnitOwner() const override;
    // Returns unit charmer, unit owner or self
    Unit* getUnitOwnerOrSelf() override;
    // Returns unit charmer, unit owner or self
    Unit const* getUnitOwnerOrSelf() const override;
    // Returns player charmer or player owner
    Player* getPlayerOwner() override;
    // Returns player charmer or player owner
    Player const* getPlayerOwner() const override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    float_t getMaxWanderDistance() const;
    void setMaxWanderDistance(float_t dist);

#if VERSION_STRING < WotLK
    uint32_t getVirtualItemEntry(uint8_t slot) const;
    void setVirtualItemEntry(uint8_t slot, uint32_t itemId);
#endif
    void toggleDualwield(bool);

    std::vector<CreatureItem>* getSellItems();

    uint32_t getWaypointPath() const { return _waypointPathId; }
    void loadPath(uint32_t pathid) { _waypointPathId = pathid; }

    // nodeId, pathId
    std::pair<uint32_t, uint32_t> getCurrentWaypointInfo() const { return _currentWaypointNodeInfo; }
    void updateCurrentWaypointInfo(uint32_t nodeId, uint32_t pathId) { _currentWaypointNodeInfo = { nodeId, pathId }; }

    virtual void setDeathState(DeathState s);

    void onRemoveInRangeObject(Object* pObj) override;

    void registerDatabaseGossip();

    bool isReturningHome() const;
    void searchFormation();
    CreatureGroup* getFormation() { return m_formation; }
    void setFormation(CreatureGroup* formation) { m_formation = formation; }
    bool isFormationLeader() const;
    void signalFormationMovement();
    bool isFormationLeaderMoveAllowed() const;

    uint32_t getSpawnId() { return m_spawnId; }
    void setSpawnId(uint32_t spawnId) { m_spawnId = spawnId; }

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

    bool isDungeonBoss() { return (GetCreatureProperties()->extra_a9_flags & 0x10000000) != 0; }

private:
#if VERSION_STRING < WotLK
    uint32_t m_virtualItemEntry[TOTAL_WEAPON_DAMAGE_TYPES] = { 0 };
#endif

public:
 //AGPL Starts

        bool teleport(const LocationVector& vec, WorldMap* map);

        void Load(CreatureProperties const* c_properties, float x, float y, float z, float o = 0);
        bool LoadFromDB(MySQLStructure::CreatureSpawn* spawn, WorldMap* map, bool addToWorld);

        // remove auras, guardians, scripts
        virtual void PrepareForRemove();

        // Creation
        void Create(uint32_t mapid, float x, float y, float z, float ang);
        void CreateWayPoint(uint32_t WayPointID, uint32_t mapid, float x, float y, float z, float ang);

        // Creature inventory
        uint32_t GetItemIdBySlot(uint32_t slot) { return m_SellItems->at(slot).itemid; }
        uint32_t GetItemAmountBySlot(uint32_t slot) { return m_SellItems->at(slot).amount; }

        bool HasItems();
        void InitSummon(Object* summoner);

        bool updateEntry(uint32_t entry);

        void SummonExpire()
        {
            despawn();
        }

        int32_t GetSlotByItemId(uint32_t itemid);

        uint32_t GetItemAmountByItemId(uint32_t itemid);

        void GetSellItemBySlot(uint32_t slot, CreatureItem& ci);

        void GetSellItemByItemId(uint32_t itemid, CreatureItem& ci);

        WDB::Structures::ItemExtendedCostEntry const* GetItemExtendedCostByItemId(uint32_t itemid);

        std::vector<CreatureItem>::iterator GetSellItemBegin();

        std::vector<CreatureItem>::iterator GetSellItemEnd();

        size_t GetSellItemCount();

        void RemoveVendorItem(uint32_t itemid);
        void AddVendorItem(uint32_t itemid, uint32_t amount, WDB::Structures::ItemExtendedCostEntry const* ec);
        void ModAvItemAmount(uint32_t itemid, uint32_t value);
        void UpdateItemAmount(uint32_t itemid);

        // Quests
        void _LoadQuests();
        bool HasQuests();
        bool HasQuest(uint32_t id, uint32_t type);
        void AddQuest(std::unique_ptr<QuestRelation> Q);
        void DeleteQuest(QuestRelation const* Q);
        QuestProperties const* FindQuest(uint32_t quest_id, uint8_t quest_relation);
        uint16_t GetQuestRelation(uint32_t quest_id);
        uint32_t NumOfQuests();
        std::list<std::unique_ptr<QuestRelation>>::iterator QuestsBegin();
        std::list<std::unique_ptr<QuestRelation>>::iterator QuestsEnd();
        void SetQuestList(std::list<std::unique_ptr<QuestRelation>>* qst_lst);

        uint32_t GetHealthFromSpell();

        void SetHealthFromSpell(uint32_t value);

        int32_t m_speedFromHaste = 0;
        int32_t FlatResistanceMod[TOTAL_SPELL_SCHOOLS] = {0};
        int32_t BaseResistanceModPct[TOTAL_SPELL_SCHOOLS] = {0};
        int32_t ResistanceModPct[TOTAL_SPELL_SCHOOLS] = {0};

        int32_t FlatStatMod[5] = {0};
        int32_t StatModPct[5] = {0};
        int32_t TotalStatModPct[5] = {0};

        int32_t ModDamageDone[TOTAL_SPELL_SCHOOLS] = {0};
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
        void SetTransportHomePosition(float x, float y, float z, float o) { m_transportHomePosition.x = x, m_transportHomePosition.y = y, m_transportHomePosition.z = z, m_transportHomePosition.o = o; }
        void SetTransportHomePosition(const LocationVector &pos) { m_transportHomePosition = pos; }
        void GetTransportHomePosition(float &x, float &y, float &z, float &ori) { x = m_transportHomePosition.x, y = m_transportHomePosition.y, z = m_transportHomePosition.z, ori = m_transportHomePosition.o; }
        LocationVector GetTransportHomePosition() { return m_transportHomePosition; }
        LocationVector m_transportHomePosition;

        void SendScriptTextChatMessage(uint32_t textid, Unit* target = nullptr);
        void SendScriptTextChatMessageByIndex(uint32_t textid, Unit* target = nullptr);
        void SendTimedScriptTextChatMessage(uint32_t textid, uint32_t delay = 0, Unit* target = nullptr);

        // Serialization
        void SaveToDB();
        void DeleteFromDB();

        void OnRemoveCorpse();
        void OnRespawn();

    private:
        // Waypoint path
        uint32_t _waypointPathId = 0;
        std::pair<uint32_t/*nodeId*/, uint32_t/*pathId*/> _currentWaypointNodeInfo = {0, 0};

    public:
        // Demon
        void EnslaveExpire();

        // Pet
        uint32_t GetEnslaveCount();

        void SetEnslaveCount(uint32_t count);

        uint32_t GetEnslaveSpell();

        void SetEnslaveSpell(uint32_t spellId);
        bool RemoveEnslave();

        int32_t GetDamageDoneMod(uint8_t school) const override;

        float GetDamageDonePctMod(uint8_t school) const override;

        bool IsPickPocketed();

        void SetPickPocketed(bool val = true);

        CreatureAIScript* GetScript();
        void LoadScript();

        void CallScriptUpdate(unsigned long time_passed);

        CreatureProperties const* GetCreatureProperties() const;
        void SetCreatureProperties(CreatureProperties const* creature_properties);

        Trainer const* GetTrainer();

        WDB::Structures::CreatureFamilyEntry const* myFamily = nullptr;

        bool IsExotic();
        bool isCritter() override;

        void ChannelLinkUpGO(uint32_t SqlId);
        void ChannelLinkUpCreature(uint32_t SqlId);

        uint32_t m_spawnId = 0;
        uint32_t original_emotestate = 0;

        MySQLStructure::CreatureSpawn* m_spawn = nullptr;

        void despawn(uint32_t delayMs = 0, uint32_t respawnDelayMs = 0);
        void Despawn(uint32_t delayMs = 0, uint32_t respawnDelayMs = 0) { despawn(delayMs, respawnDelayMs); }
        void TriggerScriptEvent(int);

        AuctionHouse* auctionHouse = nullptr;

        bool CanAddToWorld();

        // scriptdev2
        uint32_t GetNpcTextId();

        Player* m_escorter = nullptr;
        bool IsInLimboState();

        void SetLimboState(bool set);
        static uint32_t GetLineByFamily(WDB::Structures::CreatureFamilyEntry const* family);
        void RemoveLimboState(Unit* healer);

        bool m_noRespawn = false;

        float GetBaseParry();
        bool isattackable(MySQLStructure::CreatureSpawn* spawn);

        void die(Unit* pAttacker, uint32_t damage, uint32_t spellid) override;

        uint32_t GetType();

        void SetType(uint32_t t);

        void setRespawnTime(uint32_t respawn);
        time_t getRespawnTime() { return m_respawnTime; }

    protected:
        time_t m_corpseRemoveTime = 0;      // (msecs)timer for death or corpse disappearance
        time_t m_respawnTime = 0;           // (secs) time of next respawn
        uint32_t m_respawnDelay = 300;      // (secs) delay between corpse disappearance and respawning
        uint32_t m_corpseDelay = 60;        // (secs) delay between death and corpse disappearance

        CreatureAIScript* _myScriptClass = nullptr;
        bool m_limbostate = false;
        Trainer const* mTrainer = nullptr;

        // Movement
        MovementGeneratorType m_defaultMovementType = IDLE_MOTION_TYPE;

        // Vendor data
        std::vector<CreatureItem>* m_SellItems = nullptr;

        // Taxi data
        uint32_t mTaxiNode = 0;

        // Quest data
        std::list<std::unique_ptr<QuestRelation>>* m_quests = nullptr;

        uint32_t m_enslaveCount = 0;
        uint32_t m_enslaveSpell = 0;

        bool m_PickPocketed = false;
        uint32_t _fields[getSizeOfStructure(WoWUnit)];
        uint32_t m_healthfromspell = 0;

        CreatureProperties const* creature_properties = nullptr;

    private:
        uint32_t m_Creature_type = 0;

        uint16_t m_movementFlagUpdateTimer = 1000;

        // Formation var
        CreatureGroup* m_formation = nullptr;

        float_t m_wanderDistance = 0.0f;
};
