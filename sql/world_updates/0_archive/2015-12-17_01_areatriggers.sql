--
-- Delete invalid areatrigger
--
DELETE FROM `areatriggers` WHERE `entry` = 4776;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2015-12-17_01_areatriggers' WHERE `LastUpdate` = '2015-12-16_01_items_itemsets';
