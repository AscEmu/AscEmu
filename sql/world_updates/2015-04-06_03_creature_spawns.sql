/*
********************************************************************
Updates for creature_spawns (ICC 1. block ally, 2. horde)
********************************************************************
*/

DELETE FROM `creature_spawns` WHERE `entry` IN (37190,37190,37190,37119,37200,37997,37998,37999,38182,38283,38840,37200,39371);
DELETE FROM `creature_spawns` WHERE `entry` IN (37189,37189,37189,39372,37187,37991,37992,37993,38181,38284,38841,37187);

UPDATE `creature_spawns` SET `faction` = 2131 WHERE `entry` = 37965;
UPDATE `creature_spawns` SET `faction` = 2050 WHERE `entry` = 37967;
UPDATE `creature_spawns` SET `faction` = 2144 WHERE `entry` = 37996;
UPDATE `creature_spawns` SET `faction` = 2131 WHERE `entry` = 37928;
   
UPDATE `world_db_version` SET `LastUpdate` = '2015-04-06_03_creature_spawns';
