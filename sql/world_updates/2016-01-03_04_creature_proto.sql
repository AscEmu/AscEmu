--
-- Update creature_proto /disable regeneration for these npcs
--
UPDATE `creature_proto` SET `npcflags` = 134217728 WHERE `entry` = 12423; -- Guard Roberts
UPDATE `creature_proto` SET `npcflags` = 134217728 WHERE `entry` = 16483; -- Draenei Survivor

--
-- Update world db version
--   
UPDATE `world_db_version` SET `LastUpdate` = '2016-01-03_04_creature_proto' WHERE `LastUpdate` = '2016-01-03_03_creature_proto';

