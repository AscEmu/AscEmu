--
-- Drop table itemnames (Used in WorldSession::HandleItemNameQueryOpcode but these items are also included in table items)
--
DROP TABLE IF EXISTS `itemnames`;
--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-02-08_01_itemnames' WHERE `LastUpdate` = '2016-01-07_05_misc_creature';
