--
-- drop unused tables
--
DROP TABLE IF EXISTS `banned_phrases`;
DROP TABLE IF EXISTS `map_checkpoint`;
DROP TABLE IF EXISTS `weather_season`;
DROP TABLE IF EXISTS `world_instance_entrance`;

--
-- Update world_db_version`
--
UPDATE `world_db_version` SET `LastUpdate` = '2017-06-15_01_remove_tables' WHERE `LastUpdate` = '2017-05-31_01_worldbroadcast';
