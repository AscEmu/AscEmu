--
-- Update creature_properties column names
--
ALTER TABLE `creature_properties` CHANGE `flags1` `type_flags` int(10);
ALTER TABLE `creature_properties` CHANGE `attacktype` `attack_school` tinyint(4);
ALTER TABLE `creature_properties` CHANGE `unknown_float1` `base_attack_mod` float;
ALTER TABLE `creature_properties` CHANGE `unknown_float2` `range_attack_mod` float;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2017-09-10_01_creature_properties' WHERE `LastUpdate` = '2017-07-18_01_remove_trainerspelloverride';
