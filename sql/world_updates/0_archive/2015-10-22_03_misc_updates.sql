--
-- Update creature_spawns phase 
--
UPDATE `creature_spawns` SET `phase` = 3, `position_x` = 5941.3, `position_y` = 507.821, `position_z` = 650.263, `orientation` = 2.70526 WHERE `id` = 128045;
UPDATE `creature_spawns` SET `phase` = 3, `position_x` = 5939.31, `position_y` = 513.155, `position_z` = 650.263, `orientation` = 2.56563 WHERE `id` = 133202;
UPDATE `creature_spawns` SET `phase` = 3, `position_x` = 5937.39, `position_y` = 504.512, `position_z` = 650.263, `orientation` = 2.04204  WHERE `id` = 128044;
UPDATE `creature_spawns` SET `phase` = 3, `position_x` = 5931.55, `position_y` = 507.388, `position_z` = 650.263, `orientation` = 1.67552 WHERE `id` = 132239;
UPDATE `creature_spawns` SET `phase` = 3 WHERE `id` = 133210;

--
-- Update gameobject_spawns phase 
--
UPDATE `gameobject_spawns` SET `phase` = 3 WHERE `id` = 48515;
UPDATE `gameobject_spawns` SET `phase` = 3 WHERE `id` = 47747;
UPDATE `gameobject_spawns` SET `phase` = 3 WHERE `id` = 47748;
UPDATE `gameobject_spawns` SET `phase` = 3 WHERE `id` = 47749;
--
-- Update world_db_version
--

UPDATE `world_db_version` SET `LastUpdate` = '2015-10-22_03_misc_updates' WHERE `LastUpdate` = '2015-10-22_02_gameobject_spawns';
