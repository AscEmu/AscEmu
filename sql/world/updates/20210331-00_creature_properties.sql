-- FIX: this npc is not set correctly
DELETE FROM `creature_properties` WHERE `entry`=41200;
-- FIX: this npc is not set correctly
DELETE FROM `creature_properties` WHERE `entry`=44775;
-- FIX: this npc is not set correctly
DELETE FROM `creature_properties` WHERE  `entry`=46464;
-- FIX: this npc is not set correctly
DELETE FROM `creature_properties` WHERE  `entry`=39519;
-- FIX: this npc is not set correctly
DELETE FROM `creature_properties` WHERE  `entry`=35374;
-- FIX: this npc is not set correctly
DELETE FROM `creature_properties` WHERE  `entry`=34795;
-- FIX: this npc is not set correctly
DELETE FROM `creature_properties` WHERE  `entry`=44140;
-- FIX: this npc is not set correctly
DELETE FROM `creature_properties` WHERE  `entry`=43718;
-- FIX: this npc is not set correctly
DELETE FROM `creature_properties` WHERE  `entry`=43594;
-- FIX: this npc is not set correctly
DELETE FROM `creature_properties` WHERE  `entry`=43515;
-- FIX: this npc is not set correctly
DELETE FROM `creature_properties` WHERE  `entry`=41318;
-- FIX: this npc is not set correctly
DELETE FROM `creature_properties` WHERE  `entry`=50373;
-- FIX: this npc is not set correctly
DELETE FROM `creature_properties` WHERE  `entry`=40350;

-- update world_db_version
REPLACE INTO `world_db_version` VALUES ('67', '20210331-00_creature_properties');
