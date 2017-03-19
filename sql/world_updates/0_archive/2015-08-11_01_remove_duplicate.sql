-- ----------------------------
-- DELETE command_overrides and clientaddons.
-- ----------------------------
DROP TABLE IF EXISTS `command_overrides`;
DROP TABLE IF EXISTS `clientaddons`;


UPDATE `world_db_version` SET `LastUpdate` = '2015-08-11_01_remove_duplicate' WHERE `LastUpdate` = '2015-08-02_01_creature_waypoints';
  