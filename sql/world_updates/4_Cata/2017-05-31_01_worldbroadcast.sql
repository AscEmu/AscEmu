--
-- New worldbroadcast table
--
DROP TABLE IF EXISTS `worldbroadcast`;
CREATE TABLE IF NOT EXISTS `worldbroadcast` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `interval` int(3) NOT NULL DEFAULT '1' COMMENT 'Time in minutes',
  `random_interval` int(3) NOT NULL DEFAULT '0' COMMENT 'Random time in minutes (interval + rand(random_interval)',
  `text` varchar(255) CHARACTER SET utf8 NOT NULL COMMENT 'Text to worldbroadcast',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Update world_db_version`
--
UPDATE `world_db_version` SET `LastUpdate` = '2017-05-31_01_worldbroadcast' WHERE `LastUpdate` = '2017-05-05_02_areatriggers';
