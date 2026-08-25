CREATE TABLE IF NOT EXISTS `character_cuf_profiles` (
  `ownerguid` int unsigned NOT NULL,
  `id` tinyint unsigned NOT NULL,
  `name` varchar(12) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `frameHeight` smallint unsigned NOT NULL DEFAULT '0',
  `frameWidth` smallint unsigned NOT NULL DEFAULT '0',
  `sortBy` tinyint unsigned NOT NULL DEFAULT '0',
  `healthText` tinyint unsigned NOT NULL DEFAULT '0',
  `boolOptions` int unsigned NOT NULL DEFAULT '0',
  `topPoint` tinyint unsigned NOT NULL DEFAULT '0',
  `bottomPoint` tinyint unsigned NOT NULL DEFAULT '0',
  `leftPoint` tinyint unsigned NOT NULL DEFAULT '0',
  `topOffset` smallint unsigned NOT NULL DEFAULT '0',
  `bottomOffset` smallint unsigned NOT NULL DEFAULT '0',
  `leftOffset` smallint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ownerguid`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `character_db_version` (`id`, `LastUpdate`) VALUES ('17', '20260825-00_character_cuf_profiles');
