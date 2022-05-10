/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Map/Maps/InstanceDefines.hpp"
#include "Objects/Units/Creatures/CreatureDefines.hpp"
#include "Server/EventableObject.h"
#include "Storage/DBC/DBCStructures.hpp"
#if VERSION_STRING >= Cata
#include "Storage/DB2/DB2Structures.h"
#endif

#include <map>
#include <vector>
#include <set>

struct ItemProperties;
class Player;
class Unit;
class WorldMap;

#define MAX_NR_LOOT_ITEMS 16
#define MAX_NR_LOOT_QUESTITEMS 32
#define MAX_NR_LOOT_EQUIPABLE 2
#define MAX_NR_LOOT_NONEQUIPABLE 3

enum PartyLootMethod
{
    PARTY_LOOT_FREE_FOR_ALL         = 0,                        // Party Loot Method set to Free for All
    PARTY_LOOT_ROUND_ROBIN          = 1,                        // Party Loot Method set to Round Robin
    PARTY_LOOT_MASTER_LOOTER        = 2,                        // Party Loot Method set to Master Looter
    PARTY_LOOT_GROUP                = 3,                        // Party Loot Method set to Group
    PARTY_LOOT_NEED_BEFORE_GREED    = 4                         // Party Loot Method set to Need before Greed
};

enum LootSlotType
{
    LOOT_SLOT_TYPE_ALLOW_LOOT       = 0,                        // player can loot the item.
    LOOT_SLOT_TYPE_ROLL_ONGOING     = 1,                        // roll is ongoing. player cannot loot.
    LOOT_SLOT_TYPE_MASTER           = 2,                        // item can only be distributed by group loot master.
    LOOT_SLOT_TYPE_LOCKED           = 3,                        // item is shown in red. player cannot loot.
    LOOT_SLOT_TYPE_OWNER            = 4                         // ignore binding confirmation and etc, for single player looting
};

enum LootRollType
{
    ROLL_PASS                       = 0,                        // Player Picks Passe for an Item
    ROLL_NEED                       = 1,                        // Player Picks Need for an Item
    ROLL_GREED                      = 2,                        // Player Picks Greed for an Item
    ROLL_DISENCHANT                 = 3                         // Player Picks Disenchant for an Item
};

class LootRoll : public EventableObject
{
    public:

        LootRoll(uint32_t timer, uint32_t groupcount, uint64_t guid, uint8_t slotid, uint32_t itemid, uint32_t itemunk1, uint32_t itemunk2, WorldMap* mgr);
        ~LootRoll();

        // player rolled on the item
        void playerRolled(Player* player, uint8_t choice);

        // finish roll for item
        void finalize();

    private:

        std::map<uint32_t, uint8_t> m_NeedRolls;
        std::map<uint32_t, uint8_t> m_GreedRolls;
        std::set<uint32_t> m_passRolls;
        uint32_t _groupcount;
        uint8_t _slotid;
        uint32_t _itemid;
        uint32_t _randomsuffixid;
        uint32_t _randompropertyid;
        uint32_t _remaining;
        uint64_t _guid;
        WorldMap* _mgr;
};

typedef std::vector<std::pair<DBC::Structures::ItemRandomPropertiesEntry const*, float>> RandomPropertyVector;
typedef std::vector<std::pair<DBC::Structures::ItemRandomSuffixEntry const*, float>> RandomSuffixVector;

struct LootStoreItem
{
    uint32_t itemId;                    // the item that drops
    ItemProperties const* itemproto;    // Item properties
    std::vector<float> chance;          // chance to drop the Item
    uint32_t mincount;                  // minimum quantity to drop
    uint32_t maxcount;                  // maximum quantity to drop

    // rolls a chance and determines if the item can be added to the loottable
    bool roll(uint8_t difficulty);

    // determines if a item is a quest drop
    bool needs_quest;                   // quest drop (QuestId in Properties set)

    explicit LootStoreItem(ItemProperties const* _itemproto, std::vector<float> _chance, uint32_t _mincount, uint32_t _maxcount);
};


typedef std::set<uint32_t> LooterSet;

struct LootItem
{
    uint32_t itemId;                                                    // LootItem entry
    ItemProperties const* itemproto;                                    // LootItem itemproperties
    uint8_t count;                                                      // LootItem count
    DBC::Structures::ItemRandomPropertiesEntry const* iRandomProperty;  // LootItem randomProperty
    DBC::Structures::ItemRandomSuffixEntry const* iRandomSuffix;        // LootItem randomSuffix

    LootRoll* roll;                                                     // Lootitem roll ongoing

    LooterSet allowedLooters;                                           // LootItem allowed Players to Loot

    bool is_passed;                                                     // LootItem passes on rolling
    bool is_looted;                                                     // LootItem is looted
    bool is_blocked;                                                    // LootItem is blocked from looting
    bool is_ffa;                                                        // LootItem is free for all
    bool is_underthreshold;                                             // LootItem is under group threshold
    bool needs_quest;                                                   // LootItem needs a quest to be shown

    explicit LootItem(LootStoreItem const& lootItem);

    LootItem()
    {
        itemId = 0;
        itemproto = nullptr;
        count = 0;
        iRandomProperty = nullptr;
        iRandomSuffix = nullptr;

        roll = nullptr;

        is_passed = true;
        is_looted = true;
        is_blocked = true;
        is_ffa = true;
        is_underthreshold = true;
        needs_quest = true;
    }

    // return true when the Player is allowed to loot the item
    bool allowedForPlayer(Player* player) const;

    // adds an allowed Looter for the current Item
    void addAllowedLooter(Player* player);

    // gets all allowed Looters for our current Item
    const LooterSet & getAllowedLooters() const { return allowedLooters; }
};

struct Personaltem
{
    uint8_t index;  // Inedx from Personal Items;
    bool is_looted; // Personal Item is looted

    Personaltem()
        : index(0), is_looted(false) {}

    Personaltem(uint8_t _index, bool _islooted = false)
        : index(_index), is_looted(_islooted) {}
};

struct Loot;
class LootTemplate;

typedef std::vector<Personaltem> PersonaltemList;
typedef std::vector<LootItem> LootItemList;
typedef std::map<uint32_t, PersonaltemList*> PersonaltemMap;
typedef std::vector<LootStoreItem> LootStoreItemList;
typedef std::unordered_map<uint32_t, LootTemplate*> LootTemplateMap;

class LootTemplate
{
public:
    // Adds an entry to our LootTempelate
    void addEntry(LootStoreItem& item);

    // Generats the Loot for our current LootTempelate
    void generateLoot(Loot& loot, uint8_t lootDifficulty) const;

    // Returns True when in our LootTempelate is atleast 1 QuestItem
    bool hasQuestDrop(LootTemplateMap const& store) const;

    // Return true if our LootTempelate has atleast 1 Quest drop for the given Player
    bool hasQuestDropForPlayer(LootTemplateMap const& store, Player* player) const;

private:
    // Loot entrys on our current LootTempelate
    LootStoreItemList Entries;
};

class SERVER_DECL LootMgr
{
    private:

        LootMgr() = default;
        ~LootMgr() = default;

    public:

        static LootMgr& getInstance();
        void initialize();
        void finalize();

        LootMgr(LootMgr&&) = delete;
        LootMgr(LootMgr const&) = delete;
        LootMgr& operator=(LootMgr&&) = delete;
        LootMgr& operator=(LootMgr const&) = delete;

        // fills Creature loot for the given loot owner
        void fillCreatureLoot(Unit* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type);

        // fills GameObject loot for the given loot owner
        void fillGOLoot(Unit* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type);

        // fills Item loot for the given loot owner
        void fillItemLoot(Unit* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type);

        // fills Fishing loot for the given loot owner
        void fillFishingLoot(Unit* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type);

        // fills Skinning loot for the given loot owner
        void fillSkinningLoot(Unit* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type);

        // fills Pickpocketing loot for the given loot owner
        void fillPickpocketingLoot(Unit* lootOwner, Loot* loot, uint32_t loot_id, uint8_t type);

        // return true when the Creature is Lootable (has Loot in the Template)
        bool isCreatureLootable(uint32_t loot_id);

        // return true when the Creature is Pickpocketable (has Loot in the Tempelate)
        bool isPickpocketable(uint32_t creatureId);

        // return true when the Creature is Skinnable (has Loot in the Tempelate)
        bool isSkinnable(uint32_t creatureId);

        // return true when Gameobject is Fishable (has Loot in the Tempelate)
        bool isFishable(uint32_t zoneid);

        // Load Loot Tables
        void loadLoot();
        void loadLootProp();
        void loadLootTables(const char* szTableName, LootTemplateMap* LootTable);
        void loadAndGenerateLoot(uint8_t type);
        void addLoot(Loot* loot, uint32_t itemid, std::vector<float> chance, uint32_t mincount, uint32_t maxcount, uint8_t lootDifficulty);

        DBC::Structures::ItemRandomPropertiesEntry const* GetRandomProperties(ItemProperties const* proto);
        DBC::Structures::ItemRandomSuffixEntry const* GetRandomSuffix(ItemProperties const* proto);

        LootTemplateMap CreatureLoot;
        LootTemplateMap FishingLoot;
        LootTemplateMap SkinningLoot;
        LootTemplateMap GOLoot;
        LootTemplateMap ItemLoot;
        LootTemplateMap PickpocketingLoot;

        bool is_loading;

    private:
        std::map<uint32_t, RandomPropertyVector> _randomprops;
        std::map<uint32_t, RandomSuffixVector> _randomsuffix;
};

struct Loot
{
    PersonaltemMap const& getPlayerQuestItems() const { return PlayerQuestItems; }
    PersonaltemMap const& getPlayerFFAItems() const { return PlayerFFAItems; }

    std::vector<LootItem> items;            // LootItems
    std::vector<LootItem> quest_items;      // PersonalItems

    uint32_t gold = 0;                      // Loot money
    uint8_t unlootedCount = 0;              // Unlooted Items count
    uint64_t roundRobinPlayer = 0;          // Current Round Robin Player from the Group

    Loot(uint32_t _gold = 0) : gold(_gold) {}
    ~Loot() { clear(); }

    // fill Current Loot Store for xx Items
    bool fillLoot(uint32_t lootId, LootTemplateMap const& tempelateStore, Player* lootOwner, bool personal, uint8_t lootMode = InstanceDifficulty::DUNGEON_NORMAL);
    
    // fill Current loot Store for xx money
    void generateGold(CreatureProperties const* property, uint8_t difficulty);

    // adds an Item to our current Loot Store
    void addLootItem(LootStoreItem const & item);

    // clear our Loot
    void clear();

    // return true when all Items and Money are Looted
    bool isLooted() const { return gold == 0 && unlootedCount == 0; }

    // adds an Looter to the Current Loot (Players which are on the Loot Window)
    void addLooter(uint32_t GUID) { PlayersLooting.insert(GUID); }

    // removes an Looter from the Current Loot (Players which are on the Loot Window)
    void removeLooter(uint32_t GUID) { PlayersLooting.erase(GUID); }

    // returns an LootItem from given Slot and marks it as Looted
    LootItem* lootItemInSlot(uint32_t lootslot, Player* player, Personaltem** qitem = nullptr, Personaltem** ffaitem = nullptr);

    // returns an LootItem from given Slot
    LootItem* getlootItemInSlot(uint32_t lootslot, Player* player);

    // returns the highestIndex of our Lootables for Player xx
    uint32_t getMaxSlotInLootFor(Player* player) const;

    // return true when the Loot holds an Item for Player xx
    bool hasItemFor(Player* player) const;

    // return true when the Loot contains Items over the Group threshold
    bool hasOverThresholdItem() const;

    // Notify all Looters an Item got Removed
    void itemRemoved(uint8_t slot);

    // Notify all Looters Money got Removed
    void moneyRemoved();

private:
    // Fills not normal Loot for Player xx
    void fillNotNormalLootFor(Player* player, bool presentAtLooting);

    // Adds Currency Items to the Player when Present
    void processCurrencyItems(Player* player);

    // All Players currently Looting
    std::set<uint32_t> PlayersLooting;

    // Personal Loot
    PersonaltemMap PlayerQuestItems;
    PersonaltemMap PlayerFFAItems;
};

#define sLootMgr LootMgr::getInstance()
