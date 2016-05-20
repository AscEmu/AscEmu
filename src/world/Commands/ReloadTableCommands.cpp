/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

//.server reload gameobjects
bool ChatHandler::HandleReloadGameobjectsCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadGameObjectNamesTable();
    GreenSystemMessage(m_session, "WorldDB table 'gameobject_names' reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload creatures
bool ChatHandler::HandleReloadCreaturesCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadCreatureNamesTable();
    sMySQLStore.LoadCreatureProtoTable();
    GreenSystemMessage(m_session, "WorldDB creature tables reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload areatriggers
bool ChatHandler::HandleReloadAreaTriggersCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    AreaTriggerStorage.Reload();
    GreenSystemMessage(m_session, "WorldDB table 'areatriggers' reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload command_overrides
bool ChatHandler::HandleReloadCommandOverridesCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sCommandTableStorag.Dealloc();
    sCommandTableStorag.Init();
    sCommandTableStorag.Load();
    GreenSystemMessage(m_session, "CharactersDB 'command_overrides' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload fishing
bool ChatHandler::HandleReloadFishingCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    FishingZoneStorage.Reload();
    GreenSystemMessage(m_session, "WorldDB 'fishing' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload gossip_menu_option
bool ChatHandler::HandleReloadGossipMenuOptionCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    GossipMenuOptionStorage.Reload();
    GreenSystemMessage(m_session, "WorldDB 'gossip_menu_option' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload graveyards
bool ChatHandler::HandleReloadGraveyardsCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    GraveyardStorage.Reload();
    GreenSystemMessage(m_session, "WorldDB 'graveyards' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload items
bool ChatHandler::HandleReloadItemsCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadItemsTable();

    GreenSystemMessage(m_session, "WorldDB table 'items' reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload itempages
bool ChatHandler::HandleReloadItempagesCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadItemPagesTable();
    GreenSystemMessage(m_session, "WorldDB 'itempages' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload npc_script_text
bool ChatHandler::HandleReloadNpcScriptTextCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    CreatureTextStorage.Reload();
    GreenSystemMessage(m_session, "WorldDB 'npc_script_text' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload npc_text
bool ChatHandler::HandleReloadNpcTextCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    NpcTextStorage.Reload();
    GreenSystemMessage(m_session, "WorldDB 'npc_text' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload player_xp_for_level
bool ChatHandler::HandleReloadPlayerXpForLevelCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    objmgr.LoadXpToLevelTable();
    GreenSystemMessage(m_session, "WorldDB 'player_xp_for_level' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload points_of_interest
bool ChatHandler::HandleReloadPointsOfInterestCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    PointOfInterestStorage.Reload();
    GreenSystemMessage(m_session, "WorldDB 'points_of_interest' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload quests
bool ChatHandler::HandleReloadQuestsCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadQuestsTable();
    GreenSystemMessage(m_session, "WorldDB 'quests' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload teleport_coords
bool ChatHandler::HandleReloadTeleportCoordsCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    TeleportCoordStorage.Reload();
    GreenSystemMessage(m_session, "WorldDB 'teleport_coords' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload unit_display_sizes
bool ChatHandler::HandleReloadUnitDisplaySizesCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    UnitModelSizeStorage.Reload();
    GreenSystemMessage(m_session, "WorldDB 'unit_display_sizes' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload worldbroadcast
bool ChatHandler::HandleReloadWorldbroadcastCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    WorldBroadCastStorage.Reload();
    GreenSystemMessage(m_session, "WorldDB 'worldbroadcast' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload worldmap_info
bool ChatHandler::HandleReloadWorldmapInfoCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    WorldMapInfoStorage.Reload();
    GreenSystemMessage(m_session, "WorldDB 'worldmap_info' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload worldstring_tables
bool ChatHandler::HandleReloadWorldstringTablesCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    WorldStringTableStorage.Reload();
    GreenSystemMessage(m_session, "WorldDB 'worldstring_tables' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload zoneguards
bool ChatHandler::HandleReloadZoneguardsCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    ZoneGuardStorage.Reload();
    GreenSystemMessage(m_session, "WorldDB 'zoneguards' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}
