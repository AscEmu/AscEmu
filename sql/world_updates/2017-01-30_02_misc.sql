--
-- update wrong faction for creature Mutated Owlkin
--
UPDATE `creature_spawns` SET `faction` = 7 WHERE `entry` = 16537;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate`='2017-01-30_02_misc' WHERE  `LastUpdate`='2017-01-30_01_worldstring_tables';
