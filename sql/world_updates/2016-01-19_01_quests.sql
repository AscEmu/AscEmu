--
-- Update Quest: Massacre At Light's Point
--
UPDATE `quests` SET `ReqKillMobOrGOCount1` = 100 WHERE `entry` = 12701;

--
-- Update world db version
--   
UPDATE `world_db_version` SET `LastUpdate` = '2016-01-19_01_quests' WHERE `LastUpdate` = '2016-01-13_01_creature_proto_difficulty';

