-- 2 tables: the account wide achievements and their criteria progress of every account
CREATE TABLE IF NOT EXISTS `account_achievement` (
  `account_id` int unsigned NOT NULL,
  `achievement` int unsigned NOT NULL,
  `date` bigint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`,`achievement`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `account_achievement_progress` (
  `account_id` int unsigned NOT NULL,
  `criteria` int unsigned NOT NULL,
  `counter` int unsigned NOT NULL DEFAULT '0',
  `date` bigint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`,`criteria`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `logon_db_version` (`id`, `LastUpdate`) VALUES (10, '20261026-00_account_achievements');
