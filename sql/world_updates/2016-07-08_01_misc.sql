--
-- Removed invalid loot_gameobjects entry
--
DELETE FROM `loot_gameobjects` WHERE `entryid` = 175264;
DELETE FROM `loot_gameobjects` WHERE `entryid` = 192552;

--
-- Update event_creature_spawns (invalid event entry)
--
UPDATE `event_creature_spawns` SET `event_entry` = 62 WHERE `event_entry` = 100;


--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-07-08_01_misc' WHERE `LastUpdate` = '2016-07-06_01_item_pages';
