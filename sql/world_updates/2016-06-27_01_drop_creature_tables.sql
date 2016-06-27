--
-- Drop creature tables
--
DROP TABLE IF EXISTS `creature_names`;
DROP TABLE IF EXISTS `creature_proto`;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-06-27_01_drop_creature_tables' WHERE `LastUpdate` = '2016-06-26_02_creature_properties';
