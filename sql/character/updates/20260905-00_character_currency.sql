CREATE TABLE IF NOT EXISTS `character_currency` (
  `guid` int unsigned NOT NULL,
  `currency` int unsigned NOT NULL,
  `quantity` int unsigned NOT NULL DEFAULT '0',
  `weekly_quantity` int unsigned NOT NULL DEFAULT '0',
  `tracked_quantity` int unsigned NOT NULL DEFAULT '0',
  `flags` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`currency`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `character_db_version` (`id`, `LastUpdate`) VALUES ('18', '20260905-00_character_currency');
