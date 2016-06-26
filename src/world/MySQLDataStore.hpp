/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef MYSQL_DATA_LOADS_HPP
#define MYSQL_DATA_LOADS_HPP

#include "Singleton.h"

extern SERVER_DECL std::set<std::string> CreatureSpawnsTables;
extern SERVER_DECL std::set<std::string> GameObjectSpawnsTables;
extern SERVER_DECL std::set<std::string> GameObjectPropertiesTables;
extern SERVER_DECL std::set<std::string> CreaturePropertiesTables;
extern SERVER_DECL std::set<std::string> ItemPropertiesTables;
extern SERVER_DECL std::set<std::string> QuestPropertiesTables;

class SERVER_DECL MySQLDataStore : public Singleton < MySQLDataStore >
{
public:

    MySQLDataStore();
    ~MySQLDataStore();

    //maps
    typedef std::unordered_map<uint32, ItemPage> ItemPageContainer;
    typedef std::unordered_map<uint32, ItemProperties> ItemPropertiesContainer;
    typedef std::unordered_map<uint32, CreatureProperties> CreaturePropertiesContainer;
    typedef std::unordered_map<uint32, GameObjectProperties> GameObjectPropertiesContainer;
    typedef std::unordered_map<uint32, QuestProperties> QuestPropertiesContainer;

    typedef std::unordered_map<uint32, DisplayBounding> DisplayBoundingBoxesContainer;
    typedef std::unordered_map<uint32, VendorRestrictionEntry> VendorRestrictionContainer;
    typedef std::unordered_map<uint32, AreaTrigger> AreaTriggerContainer;
    typedef std::unordered_map<uint32, NpcText> NpcTextContainer;
    typedef std::unordered_map<uint32, NpcScriptText> NpcScriptTextContainer;
    typedef std::unordered_map<uint32, GossipMenuOption> GossipMenuOptionContainer;
    typedef std::unordered_map<uint32, GraveyardTeleport> GraveyardsContainer;
    typedef std::unordered_map<uint32, TeleportCoords> TeleportCoordsContainer;
    typedef std::unordered_map<uint32, FishingZoneEntry> FishingZonesContainer;
    typedef std::unordered_map<uint32, MapInfo> WorldMapInfoContainer;
    typedef std::unordered_map<uint32, ZoneGuardEntry> ZoneGuardsContainer;
    typedef std::unordered_map<uint32, BGMaster> BattleMastersContainer;
    typedef std::unordered_map<uint32, TotemDisplayIdEntry> TotemDisplayIdContainer;
    typedef std::unordered_map<uint32, SpellClickSpell> SpellClickSpellContainer;
    typedef std::unordered_map<uint32, WorldStringTable> WorldStringContainer;
    typedef std::unordered_map<uint32, WorldBroadCast> WorldBroadCastContainer;
    typedef std::unordered_map<uint32, PointOfInterest> PointOfInterestContainer;


    //helper
    ItemPage const* GetItemPage(uint32 entry);
    ItemPageContainer const* GetItemPagesStore() { return &_itemPagesStore; }
    ItemProperties const* GetItemProperties(uint32 entry);
    ItemPropertiesContainer const* GetItemPropertiesStore() { return &_itemPropertiesStore; }

    CreatureProperties const* GetCreatureProperties(uint32 entry);
    CreaturePropertiesContainer const* GetCreaturePropertiesStore() { return &_creaturePropertiesStore; }

    GameObjectProperties const* GetGameObjectProperties(uint32 entry);
    GameObjectPropertiesContainer const* GetGameObjectPropertiesStore() { return &_gameobjectPropertiesStore; }

    QuestProperties const* GetQuestProperties(uint32 entry);
    QuestPropertiesContainer const* GetQuestPropertiesStore() { return &_questPropertiesStore; }

    DisplayBounding const* GetDisplayBounding(uint32 entry);
    DisplayBoundingBoxesContainer const* GetDisplayBoundingBoxesStore() { return &_displayBoundingBoxesStore; }

    VendorRestrictionEntry const* GetVendorRestriction(uint32 entry);
    VendorRestrictionContainer const* GetVendorRestrictionsStore() { return &_vendorRestrictionsStore; }

    AreaTrigger const* GetAreaTrigger(uint32 entry);
    AreaTriggerContainer const* GetAreaTriggersStore() { return &_areaTriggersStore; }

    NpcText const* GetNpcText(uint32 entry);
    NpcTextContainer const* GetNpcTextStore() { return &_npcTextStore; }

    NpcScriptText const* GetNpcScriptText(uint32 entry);
    NpcScriptTextContainer const* GetNpcScriptTextStore() { return &_npcScriptTextStore; }

    GossipMenuOption const* GetGossipMenuOption(uint32 entry);
    GossipMenuOptionContainer const* GetGossipMenuOptionStore() { return &_gossipMenuOptionStore; }

    GraveyardTeleport const* GetGraveyard(uint32 entry);
    GraveyardsContainer const* GetGraveyardsStore() { return &_graveyardsStore; }

    TeleportCoords const* GetTeleportCoord(uint32 entry);
    TeleportCoordsContainer const* GetTeleportCoordsStore() { return &_teleportCoordsStore; }

    FishingZoneEntry const* GetFishingZone(uint32 entry);
    FishingZonesContainer const* GetFischingZonesStore() { return &_fishingZonesStore; }

    MapInfo const* GetWorldMapInfo(uint32 entry);
    WorldMapInfoContainer const* GetWorldMapInfoStore() { return &_worldMapInfoStore; }

    ZoneGuardEntry const* GetZoneGuard(uint32 entry);
    ZoneGuardsContainer const* GetZoneGuardsStore() { return &_zoneGuardsStore; }

    BGMaster const* GetBattleMaster(uint32 entry);
    BattleMastersContainer const* GetBattleMastersStore() { return &_battleMastersStore; }

    TotemDisplayIdEntry const* GetTotemDisplayId(uint32 entry);
    TotemDisplayIdContainer const* GetTotemDisplayIdsStore() { return &_totemDisplayIdsStore; }

    SpellClickSpell const* GetSpellClickSpell(uint32 entry);
    SpellClickSpellContainer const* GetSpellClickSpellsStore() { return &_spellClickSpellsStore; }

    WorldStringTable const* GetWorldString(uint32 entry);
    WorldStringContainer const* GetWorldStringsStore() { return &_worldStringsStore; }

    WorldBroadCast const* GetWorldBroadcast(uint32 entry);
    WorldBroadCastContainer const* GetWorldBroadcastStore() { return &_worldBroadcastStore; }

    PointOfInterest const* GetPointOfInterest(uint32 entry);
    PointOfInterestContainer const* GetPointOfInterestStore() { return &_pointOfInterestStore; }

    //Config
    void LoadAdditionalTableConfig();

    //Loads
    void LoadItemPagesTable();
    void LoadItemPropertiesTable();

    void LoadCreaturePropertiesTable();

    void LoadGameObjectPropertiesTable();

    void LoadQuestPropertiesTable();
    void LoadGameObjectQuestItemBindingTable();
    void LoadGameObjectQuestPickupBindingTable();

    void LoadDisplayBoundingBoxesTable();
    void LoadVendorRestrictionsTable();
    void LoadAreaTriggersTable();
    void LoadNpcTextTable();
    void LoadNpcScriptTextTable();
    void LoadGossipMenuOptionTable();
    void LoadGraveyardsTable();
    void LoadTeleportCoordsTable();
    void LoadFishingTable();
    void LoadWorldMapInfoTable();
    void LoadZoneGuardsTable();
    void LoadBattleMastersTable();
    void LoadTotemDisplayIdsTable();
    void LoadSpellClickSpellsTable();

    void LoadWorldStringsTable();
    void LoadWorldBroadcastTable();
    void LoadPointOfInterestTable();


    ItemPageContainer _itemPagesStore;
    ItemPropertiesContainer _itemPropertiesStore;
    CreaturePropertiesContainer _creaturePropertiesStore;
    GameObjectPropertiesContainer _gameobjectPropertiesStore;
    QuestPropertiesContainer _questPropertiesStore;

    DisplayBoundingBoxesContainer _displayBoundingBoxesStore;
    VendorRestrictionContainer _vendorRestrictionsStore;
    AreaTriggerContainer _areaTriggersStore;
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
    WorldBroadCastContainer _worldBroadcastStore;
    PointOfInterestContainer _pointOfInterestStore;

};

#define sMySQLStore MySQLDataStore::getSingleton()

#endif MYSQL_DATA_LOADS_HPP
