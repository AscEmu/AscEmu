--
-- Rename tables
--
RENAME TABLE `event_names` TO `event_properties`;
RENAME TABLE `gameobject_names` TO `gameobject_properties`;
RENAME TABLE `items` TO `item_properties`;
RENAME TABLE `itempages` TO `item_pages`;
RENAME TABLE `items_linked_itemsets` TO `itemset_linked_itemsetbonus`;
RENAME TABLE `quests` TO `quest_properties`;
RENAME TABLE `teleport_coords` TO `spell_teleport_coords`;

RENAME TABLE `creature_names_localized` TO `locales_creature`;
RENAME TABLE `gameobject_names_localized` TO `locales_gameobject`;
RENAME TABLE `items_localized` TO `locales_item`;
RENAME TABLE `itempages_localized` TO `locales_item_pages`;
RENAME TABLE `npc_monstersay_localized` TO `locales_npc_monstersay`;
RENAME TABLE `npc_script_text_localized` TO `locales_npc_script_text`;
RENAME TABLE `npc_text_localized` TO `locales_npc_text`;
RENAME TABLE `worldbroadcast_localized` TO `locales_worldbroadcast`;
RENAME TABLE `worldstring_tables_localized` TO `locales_worldstring_table`;
RENAME TABLE `worldmap_info_localized` TO `locales_worldmap_info`;
RENAME TABLE `quests_localized` TO `locales_quest`;
RENAME TABLE `gossip_menu_option_localized` TO `locales_gossip_menu_option`;

--
-- Drope items_extendedcost unused!
--
DROP TABLE IF EXISTS `items_extendedcost`;
   
--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-06-26_01_table_rename' WHERE `LastUpdate` = '2016-05-22_01_drop_unit_display_sizes';
