--
-- Removed incorrect spawn for Arena Battlemaster
-- 
DELETE FROM `creature_spawns` WHERE `id`=138202;

--
-- Removed wrong gameobject_spawns for event Call to Arms: Eye of the Storm!
--
DELETE FROM `event_gameobject_spawns` WHERE `id`=202236;
DELETE FROM `event_gameobject_spawns` WHERE `id`=202235;
DELETE FROM `event_gameobject_spawns` WHERE `id`=202251;
DELETE FROM `event_gameobject_spawns` WHERE `id`=202252;
DELETE FROM `event_gameobject_spawns` WHERE `id`=202253;

--
-- Update Battlemaster locations
--
UPDATE `creature_spawns` SET `position_x`=-5038.14, `position_y`=-1270.11 WHERE `id`=132983;
UPDATE `creature_spawns` SET `position_x`=1333.66, `position_y`=326.39, `position_z`=-63.71, `orientation`=3.33 WHERE `id`=132977;
UPDATE `creature_spawns` SET `position_x`=1329.72, `position_y`=341.36, `position_z`=-63.71, `orientation`=3.39 WHERE `id`=132976;
UPDATE `creature_spawns` SET `position_x`=9856.14, `position_y`=-7568.66, `position_z`=19.2509, `orientation`=3.3646 WHERE `id`=141542;
UPDATE `creature_spawns` SET `position_x`=-1971.35, `position_y`=5266.7, `position_z`=-38.8463, `orientation`=3.58873 WHERE `id`=132996;
REPLACE INTO `creature_spawns` VALUES (101565, 25155, 530, -1898.13, 5397.97, -12.4282, 4.99995, 0, 22956, 1956, 256, 512, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
REPLACE INTO `creature_spawns` VALUES (101558, 25153, 530, -1898.53, 5394.28, -12.4283, 0.92766, 0, 22957, 1956, 256, 66048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
REPLACE INTO `creature_spawns` VALUES (101545, 25142, 530, -1895.45, 5395.29, -12.4272, 2.86681, 0, 22763, 1956, 0, 16777472, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11021, 0, 1);

--
-- Update location for Grizzle Halfmane <Alterac Valley Battlemaster>
--
UPDATE `event_creature_spawns` SET `position_x`=1332.04, `position_y`=333.97, `position_z`=-63.71, `orientation`=3.39 WHERE `id`=150541;

--
-- Updat for player start positions
--
UPDATE `playercreateinfo` SET `positionX`=10353.7, `positionY`=-6366.13, `positionZ`=35.39 WHERE `Index`=50;
UPDATE `playercreateinfo` SET `positionX`=10353.7, `positionY`=-6366.13, `positionZ`=35.39 WHERE `Index`=51;
UPDATE `playercreateinfo` SET `positionX`=10353.7, `positionY`=-6366.13, `positionZ`=35.39 WHERE `Index`=52;
UPDATE `playercreateinfo` SET `positionX`=10353.7, `positionY`=-6366.13, `positionZ`=35.39 WHERE `Index`=53;
UPDATE `playercreateinfo` SET `positionX`=10353.7, `positionY`=-6366.13, `positionZ`=35.39 WHERE `Index`=54;
UPDATE `playercreateinfo` SET `positionX`=10353.7, `positionY`=-6366.13, `positionZ`=35.39 WHERE `Index`=55;

--
-- Corrected scale for Skyguard Nether Ray
--
UPDATE `creature_proto` SET `scale`=0.7 WHERE `entry`=22987;

--
-- Removed 2 waypoints for Stormwind City Patroller
--
DELETE FROM `creature_waypoints` WHERE `spawnid`=14650 AND `waypointid`=7;
DELETE FROM `creature_waypoints` WHERE `spawnid`=14650 AND `waypointid`=8;
UPDATE `creature_waypoints` SET `waypointid`=7 WHERE `spawnid`=14650 AND `waypointid`=9;
UPDATE `creature_waypoints` SET `waypointid`=8 WHERE `spawnid`=14650 AND `waypointid`=10;

--
-- Update waypoints for Skyguard Nether Ray
--
UPDATE `creature_waypoints` SET `position_z`=-12.42 WHERE  `spawnid`=92881 AND `waypointid`=1;
UPDATE `creature_waypoints` SET `position_x`=-1846.14, `position_y`=5296.16, `position_z`=-12.42, `waittime`=3000 WHERE  `spawnid`=92881 AND `waypointid`=2;
UPDATE `creature_waypoints` SET `position_z`=-12.42 WHERE  `spawnid`=92881 AND `waypointid`=3;
UPDATE `creature_waypoints` SET `position_z`=-12.42 WHERE  `spawnid`=92881 AND `waypointid`=4;
UPDATE `creature_waypoints` SET `position_x`=-1848.91 WHERE  `spawnid`=92881 AND `waypointid`=1;
UPDATE `creature_waypoints` SET `position_x`=-1842.41 WHERE  `spawnid`=92881 AND `waypointid`=4;

--
-- Added two missing worldstrings
--
INSERT INTO `worldstring_tables` (`entry`, `text`) VALUES (1, 'I wish to learn Dual Specialization.');
INSERT INTO `worldstring_tables` (`entry`, `text`) VALUES (0, 'Are you sure you wish to purchase a Dual Talent Specialization.');
UPDATE `worldstring_tables` SET `text`='Strand of the Ancients' WHERE  `entry`=34;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2015-12-13_01_misc_updates' WHERE `LastUpdate` = '2015-12-08_01_creature_channel_spell';
