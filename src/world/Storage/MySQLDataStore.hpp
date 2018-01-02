/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Singleton.h"
#include "Server/Packets/Handlers/NPCHandler.h"
#include "../../scripts/Battlegrounds/IsleOfConquest.h"
#include "Objects/ObjectMgr.h"
#include "Spell/Definitions/SpellClickSpell.h"
#include "Spell/Definitions/TeleportCoords.h"
#include "MySQLStructures.h"

extern SERVER_DECL std::set<std::string> CreatureSpawnsTables;
extern SERVER_DECL std::set<std::string> GameObjectSpawnsTables;
extern SERVER_DECL std::set<std::string> GameObjectPropertiesTables;
extern SERVER_DECL std::set<std::string> CreaturePropertiesTables;
extern SERVER_DECL std::set<std::string> ItemPropertiesTables;
extern SERVER_DECL std::set<std::string> QuestPropertiesTables;


class SERVER_DECL MySQLDataStore : public Singleton <MySQLDataStore>
{
public:

    MySQLDataStore();
    ~MySQLDataStore();

    //maps
    typedef std::unordered_map<uint32_t, MySQLStructure::ItemPage> ItemPageContainer;
    typedef std::unordered_map<uint32_t, ItemProperties> ItemPropertiesContainer;
    typedef std::unordered_map<uint32_t, CreatureProperties> CreaturePropertiesContainer;
    typedef std::unordered_map<uint32_t, GameObjectProperties> GameObjectPropertiesContainer;
    typedef std::unordered_map<uint32_t, QuestProperties> QuestPropertiesContainer;

    typedef std::unordered_map<uint32_t, MySQLStructure::CreatureDifficulty> CreatureDifficultyContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::DisplayBoundingBoxes> DisplayBoundingBoxesContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::VendorRestrictions> VendorRestrictionContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::NpcText> NpcTextContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::NpcScriptText> NpcScriptTextContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::GossipMenuOption> GossipMenuOptionContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::Graveyards> GraveyardsContainer;
    typedef std::unordered_map<uint32_t, TeleportCoords> TeleportCoordsContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::FishingZones> FishingZonesContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::MapInfo> WorldMapInfoContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::ZoneGuards> ZoneGuardsContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::Battlemasters> BattleMastersContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::TotemDisplayIds> TotemDisplayIdContainer;
    typedef std::unordered_map<uint32_t, SpellClickSpell> SpellClickSpellContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::WorldStringTable> WorldStringContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::PointsOfInterest> PointsOfInterestContainer;

    typedef std::unordered_map<int32_t, MySQLStructure::ItemSetLinkedItemSetBonus> ItemSetDefinedSetBonusContainer;

    typedef std::unordered_map<uint32_t, PlayerCreateInfo> PlayerCreateInfoContainer;
    typedef std::vector<uint32_t> PlayerXPperLevel;

    typedef std::map<uint32_t, std::list<SpellInfo*>*> SpellOverrideIdMap;

    typedef std::map<uint32_t, uint32_t> NpcGossipTextIdMap;

    typedef std::unordered_map<uint32_t, MySQLStructure::PetLevelAbilities> PetLevelAbilitiesContainer;

    typedef std::unordered_map<uint32_t, MySQLStructure::WorldBroadCast> WorldBroadcastContainer;

    typedef std::unordered_map<uint32_t, MySQLStructure::AreaTrigger> AreaTriggerContainer;

    typedef std::list<MySQLStructure::WordFilterCharacterNames> WordFilterCharacterNamesSet;
    typedef std::list<MySQLStructure::WordFilterChat> WordFilterChatSet;

    typedef std::unordered_map<uint32_t, MySQLStructure::CreatureFormation> CreatureFormationsMap;

    //////////////////////////////////////////////////////////////////////////////////////////
    // locales
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesCreature> LocalesCreatureContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesGameobject> LocalesGameobjectContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesGossipMenuOption> LocalesGossipMenuOptionContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesItem> LocalesItemContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesItemPages> LocalesItemPagesContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesNPCMonstersay> LocalesNPCMonstersayContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesNpcScriptText> LocalesNpcScriptTextContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesNpcText> LocalesNpcTextContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesQuest> LocalesQuestContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesWorldbroadcast> LocalesWorldbroadcastContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesWorldmapInfo> LocalesWorldmapInfoContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::LocalesWorldStringTable> LocalesWorldStringTableContainer;

    typedef std::unordered_map<uint32_t, MySQLStructure::NpcMonsterSay*> NpcMonstersayContainer;

    //typedef std::map<uint32_t, std::set<SpellInfo*>> PetDefaultSpellsMap;     Zyres 2017/07/16 not used

    typedef std::set<MySQLStructure::ProfessionDiscovery*> ProfessionDiscoverySet;

    typedef std::unordered_map<uint32_t, MySQLStructure::TransportCreatures> TransportCreaturesContainer;
    typedef std::unordered_map<uint32_t, MySQLStructure::TransportData> TransportDataContainer;

    typedef std::unordered_map<uint32_t, MySQLStructure::GossipMenuInit> GossipMenuInitMap;
    typedef std::multimap<uint32_t, MySQLStructure::GossipMenuItems> GossipMenuItemsContainer;

    //helper
    MySQLStructure::ItemPage const* getItemPage(uint32_t entry);
    ItemPageContainer const* getItemPagesStore() { return &_itemPagesStore; }
    ItemProperties const* getItemProperties(uint32_t entry);
    ItemPropertiesContainer const* getItemPropertiesStore() { return &_itemPropertiesStore; }

    CreatureProperties const* getCreatureProperties(uint32_t entry);
    CreaturePropertiesContainer const* getCreaturePropertiesStore() { return &_creaturePropertiesStore; }

    GameObjectProperties const* getGameObjectProperties(uint32_t entry);
    GameObjectPropertiesContainer const* getGameObjectPropertiesStore() { return &_gameobjectPropertiesStore; }

    QuestProperties const* getQuestProperties(uint32_t entry);
    QuestPropertiesContainer const* getQuestPropertiesStore() { return &_questPropertiesStore; }

    uint32_t getCreatureDifficulty(uint32_t entry, uint8_t difficulty_type);
    CreatureDifficultyContainer const* getCreatureDifficultyStore() { return &_creatureDifficultyStore; }

    MySQLStructure::DisplayBoundingBoxes const* getDisplayBounding(uint32_t entry);
    DisplayBoundingBoxesContainer const* getDisplayBoundingBoxesStore() { return &_displayBoundingBoxesStore; }

    MySQLStructure::VendorRestrictions const* getVendorRestriction(uint32_t entry);
    VendorRestrictionContainer const* getVendorRestrictionsStore() { return &_vendorRestrictionsStore; }

    MySQLStructure::NpcText const* getNpcText(uint32_t entry);
    NpcTextContainer const* getNpcTextStore() { return &_npcTextStore; }

    MySQLStructure::NpcScriptText const* getNpcScriptText(uint32_t entry);
    NpcScriptTextContainer const* getNpcScriptTextStore() { return &_npcScriptTextStore; }

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

    MySQLStructure::TotemDisplayIds const* getTotemDisplayId(uint32_t entry);
    TotemDisplayIdContainer const* getTotemDisplayIdsStore() { return &_totemDisplayIdsStore; }

    SpellClickSpell const* getSpellClickSpell(uint32_t entry);
    SpellClickSpellContainer const* getSpellClickSpellsStore() { return &_spellClickSpellsStore; }

    MySQLStructure::WorldStringTable const* getWorldString(uint32_t entry);
    WorldStringContainer const* getWorldStringsStore() { return &_worldStringsStore; }

    MySQLStructure::PointsOfInterest const* getPointOfInterest(uint32_t entry);
    PointsOfInterestContainer const* getPointOfInterestStore() { return &_pointsOfInterestStore; }

    uint32_t getItemSetLinkedBonus(int32_t itemset);

    PlayerCreateInfo const* getPlayerCreateInfo(uint8_t player_race, uint8_t player_class);
    uint32_t getPlayerXPForLevel(uint32_t level);

    uint32_t getGossipTextIdForNpc(uint32_t entry);

    MySQLStructure::PetLevelAbilities const* getPetLevelAbilities(uint32_t level);
    PetLevelAbilitiesContainer const* getPetAbilitiesStore() { return &_petLevelAbilitiesStore; }

    MySQLStructure::WorldBroadCast const* getWorldBroadcastById(uint32_t level);
    WorldBroadcastContainer* getWorldBroadcastStore() { return &_worldBroadcastStore; }

    MySQLStructure::AreaTrigger const* getAreaTrigger(uint32_t entry);
    AreaTriggerContainer const* getAreaTriggersStore() { return &_areaTriggerStore; }
    MySQLStructure::AreaTrigger const* getMapEntranceTrigger(uint32_t mapId);

    CreatureFormationsMap const* getCreatureFormationsStore() { return &_creatureFormationsStore; }
    MySQLStructure::CreatureFormation const* getCreatureFormationBySpawnId(uint32_t spawnId);

    //////////////////////////////////////////////////////////////////////////////////////////
    // locales
    MySQLStructure::LocalesCreature const* getLocalizedCreature(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesGameobject const* getLocalizedGameobject(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesGossipMenuOption const* getLocalizedGossipMenuOption(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesItem const* getLocalizedItem(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesItemPages const* getLocalizedItemPages(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesNPCMonstersay const* getLocalizedMonsterSay(uint32_t entry, uint32_t sessionLocale, uint32_t event);
    MySQLStructure::LocalesNpcScriptText const* getLocalizedNpcScriptText(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesNpcText const* getLocalizedNpcText(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesQuest const* getLocalizedQuest(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesWorldbroadcast const* getLocalizedWorldbroadcast(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesWorldmapInfo const* getLocalizedWorldmapInfo(uint32_t entry, uint32_t sessionLocale);
    MySQLStructure::LocalesWorldStringTable const* getLocalizedWorldStringTable(uint32_t entry, uint32_t sessionLocale);

    MySQLStructure::NpcMonsterSay* getMonstersayEventForCreature(uint32_t entry, MONSTER_SAY_EVENTS Event);
    //std::set<SpellInfo*>* getDefaultPetSpellsByEntry(uint32_t entry);     Zyres 2017/07/16 not used

    TransportCreaturesContainer* getTransportCreaturesStore() { return &_transportCreaturesStore; }
    
    GossipMenuInitMap const* getGossipMenuInitTextId() { return &_gossipMenuInitStore; }

    bool isCharacterNameAllowed(std::string charName);

    //Config
    void loadAdditionalTableConfig();

    //Loads
    void loadItemPagesTable();
    void loadItemPropertiesTable();

    void loadCreaturePropertiesTable();

    void loadGameObjectPropertiesTable();

    void loadQuestPropertiesTable();
    void loadGameObjectQuestItemBindingTable();
    void loadGameObjectQuestPickupBindingTable();

    void loadCreatureDifficultyTable();
    void loadDisplayBoundingBoxesTable();
    void loadVendorRestrictionsTable();

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

    //player create info
    void loadPlayerCreateInfoTable();
    void loadPlayerCreateInfoSkillsTable();
    void loadPlayerCreateInfoSpellsTable();
    void loadPlayerCreateInfoItemsTable();
    void loadPlayerCreateInfoBarsTable(uint32_t player_info_index);
    void loadPlayerXpToLevelTable();

    void loadSpellOverrideTable();

    void loadNpcGossipTextIdTable();
    void loadPetLevelAbilitiesTable();

    void loadBroadcastTable();

    void loadAreaTriggerTable();

    void loadWordFilterCharacterNames();
    void loadWordFilterChat();

    void loadCreatureFormationsTable();

    //////////////////////////////////////////////////////////////////////////////////////////
    // locales
    void loadLocalesCreature();
    void loadLocalesGameobject();
    void loadLocalesGossipMenuOption();
    void loadLocalesItem();
    void loadLocalesItemPages();
    void loadLocalesNPCMonstersay();
    void loadLocalesNpcScriptText();
    void loadLocalesNpcText();
    void loadLocalesQuest();
    void loadLocalesWorldbroadcast();
    void loadLocalesWorldmapInfo();
    void loadLocalesWorldStringTable();

    void loadNpcMonstersayTable();
    //void loadDefaultPetSpellsTable();   Zyres 2017 / 07 / 16 not used

    void loadProfessionDiscoveriesTable();

    void loadTransportCreaturesTable();
    void loadTransportDataTable();

    void loadGossipMenuItemsTable();


    ItemPageContainer _itemPagesStore;
    ItemPropertiesContainer _itemPropertiesStore;
    CreaturePropertiesContainer _creaturePropertiesStore;
    GameObjectPropertiesContainer _gameobjectPropertiesStore;
    QuestPropertiesContainer _questPropertiesStore;

    CreatureDifficultyContainer _creatureDifficultyStore;
    DisplayBoundingBoxesContainer _displayBoundingBoxesStore;
    VendorRestrictionContainer _vendorRestrictionsStore;
    NpcTextContainer _npcTextStore;
    NpcScriptTextContainer _npcScriptTextStore;
    GossipMenuOptionContainer _gossipMenuOptionStore;
    GraveyardsContainer _graveyardsStore;
    TeleportCoordsContainer _teleportCoordsStore;
    FishingZonesContainer _fishingZonesStore;
    WorldMapInfoContainer _worldMapInfoStore;
    ZoneGuardsContainer _zoneGuardsStore;
    BattleMastersContainer _battleMastersStore;
    TotemDisplayIdContainer _totemDisplayIdsStore;
    SpellClickSpellContainer _spellClickSpellsStore;

    WorldStringContainer _worldStringsStore;

    PointsOfInterestContainer _pointsOfInterestStore;

    ItemSetDefinedSetBonusContainer _definedItemSetBonusStore;

    PlayerCreateInfoContainer _playerCreateInfoStore;
    PlayerXPperLevel _playerXPperLevelStore;

    SpellOverrideIdMap _spellOverrideIdStore;

    NpcGossipTextIdMap _npcGossipTextIdStore;

    PetLevelAbilitiesContainer _petLevelAbilitiesStore;

    WorldBroadcastContainer _worldBroadcastStore;

    AreaTriggerContainer _areaTriggerStore;

    WordFilterCharacterNamesSet _wordFilterCharacterNamesStore;
    WordFilterChatSet _wordFilterChatStore;

    CreatureFormationsMap _creatureFormationsStore;

    //////////////////////////////////////////////////////////////////////////////////////////
    // locales
    LocalesCreatureContainer _localesCreatureStore;
    LocalesGameobjectContainer _localesGameobjectStore;
    LocalesGossipMenuOptionContainer _localesGossipMenuOptionStore;
    LocalesItemContainer _localesItemStore;
    LocalesItemPagesContainer _localesItemPagesStore;
    LocalesNPCMonstersayContainer _localesNPCMonstersayStore;
    LocalesNpcScriptTextContainer _localesNpcScriptTextStore;
    LocalesNpcTextContainer _localesNpcTextStore;
    LocalesQuestContainer _localesQuestStore;
    LocalesWorldbroadcastContainer _localesWorldbroadcastStore;
    LocalesWorldmapInfoContainer _localesWorldmapInfoStore;
    LocalesWorldStringTableContainer _localesWorldStringTableStore;

    NpcMonstersayContainer _npcMonstersayContainer[NUM_MONSTER_SAY_EVENTS];
    //PetDefaultSpellsMap _defaultPetSpellsStore;   Zyres 2017/07/16 not used

    ProfessionDiscoverySet _professionDiscoveryStore;

    TransportCreaturesContainer _transportCreaturesStore;
    TransportDataContainer _transportDataStore;

    GossipMenuInitMap _gossipMenuInitStore;
    GossipMenuItemsContainer _gossipMenuItemsStores;
};

#define sMySQLStore MySQLDataStore::getSingleton()
