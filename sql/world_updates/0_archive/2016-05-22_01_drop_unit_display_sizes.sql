--
-- unit_display_sizes is unused!
--
DROP TABLE IF EXISTS `unit_display_sizes`;
   
--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-05-22_01_drop_unit_display_sizes' WHERE `LastUpdate` = '2016-04-25_01_waypoint_manual';
