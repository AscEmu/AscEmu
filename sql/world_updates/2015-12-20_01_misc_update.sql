--
-- Incorrect entry  for Big Will
--
DELETE FROM `creature_names` WHERE `entry`=100000;

--
-- Defense System is part of the scripted dungeon.
--
DELETE FROM `creature_spawns` WHERE `id`=199003;

--
-- Update spawn identifiers in the database by the provisions of the forum.
--
UPDATE `creature_spawns` SET `id`=145455 WHERE `id`=199000;
UPDATE `creature_spawns` SET `id`=145456, `entry`=6238 WHERE `id`=199001;
UPDATE `creature_spawns` SET `id`=145457 WHERE `id`=199002;
UPDATE `creature_spawns` SET `id`=145458 WHERE `id`=199005;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2015-12-20_01_misc_update' WHERE `LastUpdate` = '2015-12-18_03_event_winter_veil';
