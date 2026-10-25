-- 2 tables: the battle pet journal and the battle slots of every account
CREATE TABLE IF NOT EXISTS `battle_pets` (
  `account_id` int unsigned NOT NULL,
  `id` int unsigned NOT NULL,
  `species` int unsigned NOT NULL DEFAULT '0',
  `breed` smallint unsigned NOT NULL DEFAULT '0',
  `display_id` int unsigned NOT NULL DEFAULT '0',
  `level` smallint unsigned NOT NULL DEFAULT '1',
  `experience` smallint unsigned NOT NULL DEFAULT '0',
  `health` int unsigned NOT NULL DEFAULT '0',
  `quality` tinyint unsigned NOT NULL DEFAULT '0',
  `flags` smallint unsigned NOT NULL DEFAULT '0',
  `name` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `name_timestamp` bigint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `battle_pet_slots` (
  `account_id` int unsigned NOT NULL,
  `slot` tinyint unsigned NOT NULL,
  `pet_id` int unsigned NOT NULL DEFAULT '0',
  `locked` tinyint unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`account_id`,`slot`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `logon_db_version` (`id`, `LastUpdate`) VALUES (8, '20261025-00_battle_pets');
