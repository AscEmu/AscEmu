/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Spell/SpellClickInfo.hpp"
#include "Spell/Definitions/TeleportCoords.hpp"
#include "MySQLStructures.h"
#include "Macros/MapsMacros.hpp"
#include "Movement/Spline/SplineChain.h"
#include "Objects/GameObjectProperties.hpp"
#include "Management/QuestProperties.hpp"
#include "Management/ItemProperties.hpp"
#include "Objects/Units/Creatures/CreatureDefines.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"

class QueryResult;
class SpellInfo;
struct SplineChainLink;

typedef std::pair<uint32_t, uint16_t> ChainKeyType;

// Custom Functions to make par a keyvariable in an unordered map
struct ChainKeyTypeHash
{
    std::size_t operator()(const ChainKeyType& key) const
    {
        std::size_t hash = std::hash<uint32_t>{}(key.first);
        hash ^= std::hash<uint16_t>{}(key.second) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        return hash;
    }
};

// Custom Functions to make par a keyvariable in an unordered map
struct ChainKeyTypeEqual
{
    bool operator()(const ChainKeyType& lhs, const ChainKeyType& rhs) const
    {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }
};

// Zyres: Define base tables
struct MySQLAdditionalTable
{
    std::string mainTable;
    std::vector<std::string> tableVector;
};

extern SERVER_DECL std::vector<MySQLAdditionalTable> MySQLAdditionalTables;

class SERVER_DECL MySQLDataStore
{
private:
    MySQLDataStore() = default;
    ~MySQLDataStore() = default;

public:
    static MySQLDataStore& getInstance();
    void finalize();

    MySQLDataStore(MySQLDataStore&&) = delete;
    MySQLDataStore(MySQLDataStore const&) = delete;
    MySQLDataStore& operator=(MySQLDataStore&&) = delete;
    MySQLDataStore& operator=(MySQLDataStore const&) = delete;

    // maps
    typedef std::unordered_map<uint32_t, MySQLStructure::ItemPage> ItemPageContainer;
    typedef std::unordered_map<uint32_t, ItemProperties> ItemPropertiesContainer;
    typedef std::unordered_map<uint32_t, CreatureProperties> CreaturePropertiesContainer;
    typedef std::unordered_map<uint32_t, CreaturePropertiesMovement> CreaturePropertiesMovementContainer;

    typedef std::multimap<uint32_t, std::unique_ptr<MySQLStructure::CreatureAIScripts>> AIScriptsMap;

    typedef std::unordered_map<uint32_t, GameObjectProperties> GameObjectPropertiesContainer;
    typedef std::unordered_map<uint32_t, QuestProperties> QuestPropertiesContainer;

    typedef std::unordered_map<uint32_t, MySQLStructure::GameObjectSpawnExtra> GameObjectSpawnExtraContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::GameObjectSpawnOverrides> GameObjectSpawnOverrideContainer;

    typedef std::unordered_map<uint32_t, SpawnGroupTemplateData> SpawnGroupDataContainer;
    typedef std::multimap<uint32_t, SpawnGroupTemplateData*> SpawnGroupLinkContainer;

    typedef std::unordered_map<ChainKeyType, std::vector<SplineChainLink>, ChainKeyTypeHash, ChainKeyTypeEqual> SplineChainContainer;

    typedef std::unordered_map<uint32_t, MySQLStructure::CreatureDifficulty> CreatureDifficultyContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::DisplayBoundingBoxes> DisplayBoundingBoxesContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::VendorRestrictions> VendorRestrictionContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::NpcGossipText> NpcGossipTextContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::NpcScriptText> NpcScriptTextContainer;
    typedef std::unordered_map<uint32_t, std::vector<MySQLStructure::NpcScriptText>> NpcScriptTextByIdContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::GossipMenuOption> GossipMenuOptionContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::Graveyards> GraveyardsContainer;
    typedef std::unordered_map<uint32_t, TeleportCoords> TeleportCoordsContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::FishingZones> FishingZonesContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::MapInfo> WorldMapInfoContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::ZoneGuards> ZoneGuardsContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::Battlemasters> BattleMastersContainer;
    typedef std::vector<MySQLStructure::TotemDisplayIds> TotemDisplayIdContainer;
    typedef std::unordered_map<uint32_t, SpellClickInfo> SpellClickInfoContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::WorldStringTable> WorldStringContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::PointsOfInterest> PointsOfInterestContainer;

    typedef std::unordered_map<int32_t, MySQLStructure::ItemSetLinkedItemSetBonus> ItemSetDefinedSetBonusContainer;

    typedef std::vector<uint32_t> PlayerXPperLevel;

    typedef std::map<uint32_t, std::unique_ptr<std::list<SpellInfo const*>>> SpellOverrideIdMap;

    typedef std::map<uint32_t, uint32_t> NpcGossipTextIdMap;

    typedef std::unordered_map<uint32_t, MySQLStructure::PetLevelAbilities> PetLevelAbilitiesContainer;

    typedef std::unordered_map<uint32_t, MySQLStructure::WorldBroadCast> WorldBroadcastContainer;

    typedef std::unordered_map<uint32_t, MySQLStructure::AreaTrigger> AreaTriggerContainer;

    typedef std::list<MySQLStructure::WordFilterCharacterNames> WordFilterCharacterNamesSet;
    typedef std::list<MySQLStructure::WordFilterChat> WordFilterChatSet;

    //////////////////////////////////////////////////////////////////////////////////////////
    // locales
    typedef std::vector<MySQLStructure::LocalesAchievementReward> LocalesAchievementRewardContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesCreature> LocalesCreatureContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesGameobject> LocalesGameobjectContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesGossipMenuOption> LocalesGossipMenuOptionContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesItem> LocalesItemContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesItemPages> LocalesItemPagesContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesNpcScriptText> LocalesNpcScriptTextContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesNpcGossipText> LocalesNpcGossipTextContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesPointsOfInterest> LocalesPointsOfInterestContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesQuest> LocalesQuestContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesWorldbroadcast> LocalesWorldbroadcastContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesWorldmapInfo> LocalesWorldmapInfoContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesWorldStringTable> LocalesWorldStringTableContainer;

    typedef std::unordered_map<uint32_t, MySQLStructure::CreatureAITexts*> CreatureAiTextContainer;

    //typedef std::map<uint32_t, std::set<SpellInfo const*>> PetDefaultSpellsMap;     Zyres 2017/07/16 not used

    typedef std::set<std::unique_ptr<MySQLStructure::ProfessionDiscovery>> ProfessionDiscoverySet;

    typedef std::unordered_map<uint32_t, MySQLStructure::TransportData> TransportDataContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::TransportEntrys> TransportEntryContrainer;
    typedef std::vector<uint32_t> TransportMapContainer;

    typedef std::unordered_map<uint32_t, MySQLStructure::GossipMenuInit> GossipMenuInitMap;
    typedef std::multimap<uint32_t, MySQLStructure::GossipMenuItems> GossipMenuItemsContainer;

    typedef std::vector<MySQLStructure::CreatureSpawn*> CreatureSpawnsMap;
    typedef std::vector<MySQLStructure::GameobjectSpawn*> GameobjectSpawnsMap;

    typedef std::vector<std::unique_ptr<MySQLStructure::RecallStruct>> RecallMap;

    // helper
    MySQLStructure::ItemPage const* getItemPage(uint32_t entry);
    uint32_t getItemPageEntryByText(std::string _text);
    ItemPageContainer const* getItemPagesStore() { return &_itemPagesStore; }
    ItemProperties const* getItemProperties(uint32_t entry);
    ItemPropertiesContainer const* getItemPropertiesStore();
    std::string getItemLinkByProto(ItemProperties const* iProto, uint32_t language = 0);

    CreatureProperties const* getCreatureProperties(uint32_t entry);
    CreaturePropertiesContainer const* getCreaturePropertiesStore() { return &_creaturePropertiesStore; }

    CreaturePropertiesMovement const* getCreaturePropertiesMovement(uint32_t entry);

    GameObjectProperties const* getGameObjectProperties(uint32_t entry);
    GameObjectPropertiesContainer const* getGameObjectPropertiesStore();

    MySQLStructure::GameObjectSpawnExtra const* getGameObjectExtra(uint32_t lowguid) const;
    MySQLStructure::GameObjectSpawnOverrides const* getGameObjectOverride(uint32_t lowguid) const;

    QuestProperties const* getQuestProperties(uint32_t entry);
    QuestPropertiesContainer const* getQuestPropertiesStore();

    uint32_t getCreatureDifficulty(uint32_t entry, uint8_t difficulty_type);
    CreatureDifficultyContainer const* getCreatureDifficultyStore() { return &_creatureDifficultyStore; }

    MySQLStructure::DisplayBoundingBoxes const* getDisplayBounding(uint32_t entry);
    DisplayBoundingBoxesContainer const* getDisplayBoundingBoxesStore() { return &_displayBoundingBoxesStore; }

    MySQLStructure::VendorRestrictions const* getVendorRestriction(uint32_t entry);
    VendorRestrictionContainer const* getVendorRestrictionsStore() { return &_vendorRestrictionsStore; }

    MySQLStructure::NpcGossipText const* getNpcGossipText(uint32_t entry) const;
    NpcGossipTextContainer const* getNpcGossipTextStore() { return &_npcGossipTextStore; }

    MySQLStructure::NpcScriptText const* getNpcScriptText(uint32_t entry);
    NpcScriptTextContainer const* getNpcScriptTextStore() { return &_npcScriptTextStore; }

    MySQLStructure::NpcScriptText const* getNpcScriptTextById(uint32_t entry, uint8_t index);
    NpcScriptTextByIdContainer const* getNpcScriptTextStoreById() { return &_npcScriptTextStoreById; }

    MySQLStructure::GossipMenuOption const* getGossipMenuOption(uint32_t entry);
    GossipMenuOptionContainer const* getGossipMenuOptionStore() { return &_gossipMenuOptionStore; }

    MySQLStructure::Graveyards const* getGraveyard(uint32_t entry);
    GraveyardsContainer const* getGraveyardsStore() { return &_graveyardsStore; }

    TeleportCoords const* getTeleportCoord(uint32_t entry);
    TeleportCoordsContainer const* getTeleportCoordsStore() { return &_teleportCoordsStore; }

    MySQLStructure::FishingZones const* getFishingZone(uint32_t entry);
    FishingZonesContainer const* getFischingZonesStore() { return &_fishingZonesStore; }

    MySQLStructure::MapInfo const* getWorldMapInfo(uint32_t entry);
    WorldMapInfoContainer const* getWorldMapInfoStore() { return &_worldMapInfoStore; }

    MySQLStructure::ZoneGuards const* getZoneGuard(uint32_t entry);
    ZoneGuardsContainer const* getZoneGuardsStore() { return &_zoneGuardsStore; }

    MySQLStructure::Battlemasters const* getBattleMaster(uint32_t entry);
    BattleMastersContainer const* getBattleMastersStore() { return &_battleMastersStore; }

    MySQLStructure::TotemDisplayIds const* getTotemDisplayId(uint8_t race, uint32_t entry);
    TotemDisplayIdContainer const* getTotemDisplayIdsStore() { return &_totemDisplayIdsStore; }

    std::vector<SpellClickInfo> const getSpellClickInfo(uint32_t creature_id);
    SpellClickInfoContainer const* getSpellClickSpellsStore() { return &_spellClickInfoStore; }

    MySQLStructure::WorldStringTable const* getWorldString(uint32_t entry);
    WorldStringContainer const* getWorldStringsStore() { return &_worldStringsStore; }

    MySQLStructure::PointsOfInterest const* getPointOfInterest(uint32_t entry);
    PointsOfInterestContainer const* getPointOfInterestStore() { return &_pointsOfInterestStore; }

    uint32_t getItemSetLinkedBonus(int32_t itemset);

    PlayerCreateInfo const* getPlayerCreateInfo(uint8_t player_race, uint8_t player_class);
    CreateInfo_Levelstats const* getPlayerLevelstats(uint32_t level, uint8_t player_race, uint8_t player_class);
    CreateInfo_ClassLevelStats const* getPlayerClassLevelStats(uint32_t level, uint8_t player_class);
    uint32_t getPlayerXPForLevel(uint32_t level);

    uint32_t getGossipTextIdForNpc(uint32_t entry);

    MySQLStructure::PetLevelAbilities const* getPetLevelAbilities(uint32_t level);
    PetLevelAbilitiesContainer const* getPetAbilitiesStore() { return &_petLevelAbilitiesStore; }

    MySQLStructure::WorldBroadCast const* getWorldBroadcastById(uint32_t level);
    WorldBroadcastContainer* getWorldBroadcastStore() { return &_worldBroadcastStore; }

    MySQLStructure::AreaTrigger const* getAreaTrigger(uint32_t entry);
    AreaTriggerContainer const* getAreaTriggersStore() { return &_areaTriggerStore; }
    MySQLStructure::AreaTrigger const* getMapEntranceTrigger(uint32_t mapId);
    MySQLStructure::AreaTrigger const* getMapGoBackTrigger(uint32_t mapId);

    std::unique_ptr<std::vector<MySQLStructure::CreatureAIScripts>> getCreatureAiScripts(uint32_t entry);

    SpawnGroupTemplateData* getSpawnGroupDataBySpawn(uint32_t spawnId);
    SpawnGroupTemplateData* getSpawnGroupDataByGroup(uint32_t groupId);
    std::vector<Creature*> const getSpawnGroupDataByBoss(uint32_t bossId);

    std::vector<SplineChainLink> const* getSplineChain(uint32_t entry, uint16_t chainId) const;
    std::vector<SplineChainLink> const* getSplineChain(Creature const* pCreature, uint16_t id) const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // locales
    MySQLStructure::LocalesAchievementReward const* getLocalizedAchievementReward(uint32_t entry, uint32_t gender, uint32_t sessionLocale);
    MySQLStructure::LocalesCreature const* getLocalizedCreature(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesGameobject const* getLocalizedGameobject(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesGossipMenuOption const* getLocalizedGossipMenuOption(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesItem const* getLocalizedItem(uint32_t entry, uint32_t sessionLocale);
    char* getLocalizedItemName(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesItemPages const* getLocalizedItemPages(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesNpcScriptText const* getLocalizedNpcScriptText(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesNpcGossipText const* getLocalizedNpcGossipText(uint32_t entry, uint32_t sessionLocale) const;
    MySQLStructure::LocalesPointsOfInterest const* getLocalizedPointsOfInterest(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesQuest const* getLocalizedQuest(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesWorldbroadcast const* getLocalizedWorldbroadcast(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesWorldmapInfo const* getLocalizedWorldmapInfo(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesWorldStringTable const* getLocalizedWorldStringTable(uint32_t entry, uint32_t sessionLocale);

    // locales helper
    std::string getLocaleGossipMenuOptionOrElse(uint32_t entry, uint32_t sessionLocale);
    std::string getLocaleGossipTitleOrElse(uint32_t entry, uint32_t sessionLocale);

    //std::set<SpellInfo const*>* getDefaultPetSpellsByEntry(uint32_t entry);     Zyres 2017/07/16 not used
    
    GossipMenuInitMap const* getGossipMenuInitTextId() { return &_gossipMenuInitStore; }

    RecallMap const& getRecallStore() const { return _recallStore; }
    MySQLStructure::RecallStruct const* getRecallByName(std::string const& name) const;

    bool isCharacterNameAllowed(std::string charName);

    bool isTransportMap(uint32_t mapId) const
    {
        if (std::find(_transportMapStore.begin(), _transportMapStore.end(), mapId) != _transportMapStore.end())
            return true;
        return false;
    }

    // config
    void loadAdditionalTableConfig();

    // helpers
    std::unique_ptr<QueryResult> getWorldDBQuery(const char* query, ...);

    // loads
    void loadItemPagesTable();
    void addItemPage(uint32_t _entry, std::string _text, uint32_t _nextPage = 0);
    void loadItemPropertiesTable();
    void loadItemPropertiesSpellsTable();
    void loadItemPropertiesStatsTable();

    void loadCreaturePropertiesMovementTable();
    void loadCreaturePropertiesTable();

    void loadGameObjectPropertiesTable();
    void loadGameObjectSpawnsExtraTable();
    void loadGameObjectSpawnsOverrideTable();

    void loadQuestPropertiesTable();
    void loadGameObjectQuestItemBindingTable();
    void loadGameObjectQuestPickupBindingTable();

    void loadCreatureAIScriptsTable();
    void loadCreatureDifficultyTable();
    void loadDisplayBoundingBoxesTable();
    void loadVendorRestrictionsTable();

    void loadSpawnGroupIds();
    void loadCreatureGroupSpawns();

    void loadCreatureSplineChains();

    void loadNpcTextTable();
    void loadNpcScriptTextTable();
    void loadGossipMenuOptionTable();
    void loadGraveyardsTable();
    void loadTeleportCoordsTable();
    void loadFishingTable();
    void loadWorldMapInfoTable();
    void loadZoneGuardsTable();
    void loadBattleMastersTable();
    void loadTotemDisplayIdsTable();
    void loadSpellClickSpellsTable();

    void loadWorldStringsTable();

    void loadPointsOfInterestTable();

    void loadItemSetLinkedSetBonusTable();
    void loadCreatureInitialEquipmentTable();

    // player create info
    void loadPlayerCreateInfoTable();
    void loadPlayerCreateInfoBars();
    void loadPlayerCreateInfoItems();
    void loadPlayerCreateInfoSkills();
    void loadPlayerCreateInfoSpellLearn();
    void loadPlayerCreateInfoSpellCast();
    void loadPlayerCreateInfoLevelstats();
    void loadPlayerCreateInfoClassLevelstats();
    void loadPlayerXpToLevelTable();

    void loadSpellOverrideTable();

    void loadNpcGossipTextIdTable();
    void loadPetLevelAbilitiesTable();

    void loadBroadcastTable();

    void loadAreaTriggerTable();

    void loadWordFilterCharacterNames();
    void loadWordFilterChat();

    //////////////////////////////////////////////////////////////////////////////////////////
    // locales
    void loadLocalesAchievementReward();
    void loadLocalesCreature();
    void loadLocalesGameobject();
    void loadLocalesGossipMenuOption();
    void loadLocalesItem();
    void loadLocalesItemPages();
    void loadLocalesNpcScriptText();
    void loadLocalesNpcText();
    void loadLocalesPointsOfInterest();
    void loadLocalesQuest();
    void loadLocalesWorldbroadcast();
    void loadLocalesWorldmapInfo();
    void loadLocalesWorldStringTable();

    //void loadDefaultPetSpellsTable();   Zyres 2017 / 07 / 16 not used

    void loadProfessionDiscoveriesTable();

    void loadTransportDataTable();
    void loadTransportEntrys();
    void loadTransportMaps();

    void loadGossipMenuItemsTable();

    void loadCreatureSpawns();
    void loadGameobjectSpawns();

    void loadRecallTable();

    ItemPageContainer _itemPagesStore;
    ItemPropertiesContainer _itemPropertiesStore;
    CreaturePropertiesContainer _creaturePropertiesStore;
    CreaturePropertiesMovementContainer _creaturePropertiesMovementStore;
    GameObjectPropertiesContainer _gameobjectPropertiesStore;
    QuestPropertiesContainer _questPropertiesStore;

    GameObjectSpawnExtraContainer _gameObjectSpawnExtraStore;
    GameObjectSpawnOverrideContainer _gameObjectSpawnOverrideStore;

    // Spawn Groups
    SpawnGroupDataContainer _spawnGroupDataStore;
    SpawnGroupLinkContainer _spawnGroupMapStore;

    // Spline Chains
    SplineChainContainer _splineChainsStore;

    AIScriptsMap _creatureAIScriptStore;
    CreatureDifficultyContainer _creatureDifficultyStore;
    DisplayBoundingBoxesContainer _displayBoundingBoxesStore;
    VendorRestrictionContainer _vendorRestrictionsStore;
    NpcGossipTextContainer _npcGossipTextStore;
    NpcScriptTextContainer _npcScriptTextStore;
    NpcScriptTextByIdContainer _npcScriptTextStoreById;
    GossipMenuOptionContainer _gossipMenuOptionStore;
    GraveyardsContainer _graveyardsStore;
    TeleportCoordsContainer _teleportCoordsStore;
    FishingZonesContainer _fishingZonesStore;
    WorldMapInfoContainer _worldMapInfoStore;
    ZoneGuardsContainer _zoneGuardsStore;
    BattleMastersContainer _battleMastersStore;
    TotemDisplayIdContainer _totemDisplayIdsStore;
    SpellClickInfoContainer _spellClickInfoStore;

    WorldStringContainer _worldStringsStore;

    PointsOfInterestContainer _pointsOfInterestStore;

    ItemSetDefinedSetBonusContainer _definedItemSetBonusStore;

    std::array<std::array<std::unique_ptr<PlayerCreateInfo>, MAX_PLAYER_CLASSES>, DBC_NUM_RACES> _playerCreateInfoStoreNew = {{ nullptr }};
    std::array<CreateInfo_ClassLevelStatsVector, MAX_PLAYER_CLASSES> _playerClassLevelStatsStore;
    PlayerXPperLevel _playerXPperLevelStore;

    SpellOverrideIdMap _spellOverrideIdStore;

    NpcGossipTextIdMap _npcGossipTextIdStore;

    PetLevelAbilitiesContainer _petLevelAbilitiesStore;

    WorldBroadcastContainer _worldBroadcastStore;

    AreaTriggerContainer _areaTriggerStore;

    WordFilterCharacterNamesSet _wordFilterCharacterNamesStore;
    WordFilterChatSet _wordFilterChatStore;

    //////////////////////////////////////////////////////////////////////////////////////////
    // locales
    LocalesAchievementRewardContainer _localesAchievementRewardStore;
    LocalesCreatureContainer _localesCreatureStore;
    LocalesGameobjectContainer _localesGameobjectStore;
    LocalesGossipMenuOptionContainer _localesGossipMenuOptionStore;
    LocalesItemContainer _localesItemStore;
    LocalesItemPagesContainer _localesItemPagesStore;
    LocalesNpcScriptTextContainer _localesNpcScriptTextStore;
    LocalesNpcGossipTextContainer _localesNpcGossipTextStore;
    LocalesPointsOfInterestContainer _localesPointsOfInterestStore;
    LocalesQuestContainer _localesQuestStore;
    LocalesWorldbroadcastContainer _localesWorldbroadcastStore;
    LocalesWorldmapInfoContainer _localesWorldmapInfoStore;
    LocalesWorldStringTableContainer _localesWorldStringTableStore;

    //PetDefaultSpellsMap _defaultPetSpellsStore;   Zyres 2017/07/16 not used

    ProfessionDiscoverySet _professionDiscoveryStore;

    TransportDataContainer _transportDataStore;
    TransportEntryContrainer _transportEntryStore;
    TransportMapContainer _transportMapStore;

    GossipMenuInitMap _gossipMenuInitStore;
    GossipMenuItemsContainer _gossipMenuItemsStores;

    std::array<CreatureSpawnsMap, (MAX_NUM_MAPS + 1)> _creatureSpawnsStore;
    std::array<GameobjectSpawnsMap, (MAX_NUM_MAPS + 1)> _gameobjectSpawnsStore;

    RecallMap _recallStore;
};

#define sMySQLStore MySQLDataStore::getInstance()
