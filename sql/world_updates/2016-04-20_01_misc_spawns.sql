--
-- Update creature_spawns phase 
--
UPDATE `creature_spawns` SET `phase` = 1 WHERE `id` = 128045;
UPDATE `creature_spawns` SET `phase` = 1 WHERE `id` = 133202;
UPDATE `creature_spawns` SET `phase` = 1 WHERE `id` = 128044;
UPDATE `creature_spawns` SET `phase` = 1 WHERE `id` = 132239;
UPDATE `creature_spawns` SET `phase` = 1 WHERE `id` = 133210;

--
-- Update gameobject_spawns phase 
--
UPDATE `gameobject_spawns` SET `phase` = 1 WHERE `id` = 48515;
UPDATE `gameobject_spawns` SET `phase` = 1 WHERE `id` = 47747;
UPDATE `gameobject_spawns` SET `phase` = 1 WHERE `id` = 47748;
UPDATE `gameobject_spawns` SET `phase` = 1 WHERE `id` = 47749;
--
-- Update world_db_version
--

UPDATE `world_db_version` SET `LastUpdate` = '2016-04-20_01_misc_spawns' WHERE `LastUpdate` = '2016-04-18_01_gameobject_names';
