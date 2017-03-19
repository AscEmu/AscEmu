--
-- Update event_tables columns
--
ALTER TABLE `event_creature_spawns` CHANGE `eventEntry` `event_entry` INT(3) unsigned NOT NULL;
ALTER TABLE `event_gameobject_spawns` CHANGE `eventEntry` `event_entry` INT(3) unsigned NOT NULL;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2015-11-06_02_event_tables' WHERE `LastUpdate` = '2015-11-06_01_misc_spawns';
