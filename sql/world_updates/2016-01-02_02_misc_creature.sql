--
-- Update Festival Loremaster Alliance
--
UPDATE `creature_proto` SET `maxhealth`=4104 WHERE `entry`=16817;

--
-- Update Festival Talespinner Horde
--
UPDATE `creature_proto` SET `minhealth`=3052, `maxhealth`=4104 WHERE `entry`=16818;

--
-- Update Thrallmar Wolf Rider
--
UPDATE `creature_spawns` SET `mountdisplayid`=14334 WHERE `id`=67348;

--
-- Relocation of portals. (Isle of Conquest Portal)
--
UPDATE `creature_spawns` SET `position_z`=654.312 WHERE `id`=132927;
UPDATE `creature_spawns` SET `position_z`=661.101 WHERE `id`=132928;

--
-- Update Howling Fjord Flame Warden
--
UPDATE `creature_proto` SET `minlevel`=72, `maxlevel`=72, `minhealth`=4278, `maxhealth`=4608, `mindamage`=126, `maxdamage`=167, `rangedmindamage`=89, `rangedmaxdamage`=130, `armor`=7.387 WHERE `entry`=32804;

--
-- Update Eye of Thrallmar
--
UPDATE `creature_proto` SET `fly_speed`=25 WHERE `entry`=16598;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-01-02_02_misc_creature' WHERE `LastUpdate` = '2016-01-02_01_creature_update';
