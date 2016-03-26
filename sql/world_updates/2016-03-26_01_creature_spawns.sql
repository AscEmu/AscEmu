--
-- Delete creature_spawns for Orgrim's Hammer
--
DELETE FROM `creature_spawns` WHERE `id` = 125806;

--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-03-26_01_creature_spawns' WHERE `LastUpdate` = '2016-03-18_01_misc';