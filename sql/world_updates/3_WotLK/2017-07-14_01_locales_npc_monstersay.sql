--
-- Update locales_npc_monstersay
--
DROP TABLE IF EXISTS `locales_npc_monstersay`;
CREATE TABLE `locales_npc_monstersay` (
  `entry` int(10) NOT NULL DEFAULT '0',
  `type` int(1) unsigned NOT NULL DEFAULT '0',
  `language_code` varchar(5) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'enGB',
  `monstername` varchar(100) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `text0` varchar(255) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `text1` varchar(255) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `text2` varchar(255) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `text3` varchar(255) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `text4` varchar(255) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`entry`,`type`,`language_code`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='NPC System localized';

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2017-07-14_01_locales_npc_monstersay' WHERE `LastUpdate` = '2017-06-15_01_remove_tables';

