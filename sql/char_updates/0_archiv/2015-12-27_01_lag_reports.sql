--
-- Create table lag_reports
--
CREATE TABLE IF NOT EXISTS `lag_reports` (
  `lag_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `player` int(10) unsigned NOT NULL,
  `account` int(10) unsigned NOT NULL,
  `lag_type` smallint(2) unsigned NOT NULL,
  `map_id` int(5) unsigned DEFAULT '0',
  `position_x` float DEFAULT '0',
  `position_y` float DEFAULT '0',
  `position_z` float DEFAULT '0',
  `timestamp` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`lag_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Update character_db_version
--
UPDATE `character_db_version` SET `LastUpdate` = '2015-12-27_01_lag_reports' WHERE `LastUpdate` = '2015-11-06_01_event_save';
