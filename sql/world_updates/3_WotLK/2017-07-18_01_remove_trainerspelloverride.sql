--
-- Drop trainerspelloverride (not used)
--
DROP TABLE IF EXISTS `trainerspelloverride`;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2017-07-18_01_remove_trainerspelloverride' WHERE `LastUpdate` = '2017-07-14_01_locales_npc_monstersay';
