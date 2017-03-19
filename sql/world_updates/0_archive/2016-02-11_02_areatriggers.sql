--
-- Update areatrigger (ICC entrance) type = 4 (multimode)
--
UPDATE `areatriggers` SET `type` = 4 WHERE `entry` = 5670;

--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-02-11_02_areatriggers' WHERE `LastUpdate` = '2016-02-11_01_creature_proto';
