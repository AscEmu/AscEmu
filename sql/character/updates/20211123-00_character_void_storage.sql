-- ----------------------------
-- Table structure for `character_void_storage`
-- ----------------------------
DROP TABLE IF EXISTS `character_void_storage`;
CREATE TABLE `character_void_storage` (
  `itemId` bigint unsigned NOT NULL,
  `playerGuid` int unsigned NOT NULL,
  `itemEntry` mediumint unsigned NOT NULL,
  `slot` tinyint unsigned NOT NULL,
  `creatorGuid` int unsigned NOT NULL DEFAULT '0',
  `randomProperty` int unsigned NOT NULL DEFAULT '0',
  `suffixFactor` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemId`),
  UNIQUE KEY `idx_player_slot` (`playerGuid`,`slot`),
  KEY `idx_player` (`playerGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

UPDATE `character_db_version` SET LastUpdate = '20211123-00_character_void_storage';
