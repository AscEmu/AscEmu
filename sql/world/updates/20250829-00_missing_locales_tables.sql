-- locales_achievement_reward definition

DROP TABLE IF EXISTS `locales_achievement_reward`;

CREATE TABLE `locales_achievement_reward` (
  `entry` mediumint unsigned NOT NULL DEFAULT '0',
  `gender` tinyint NOT NULL DEFAULT '2',
  `language_code` varchar(5) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `subject` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `text` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  UNIQUE KEY `locales_achievement_reward_entry_IDX` (`entry`,`gender`,`language_code`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- locales_points_of_interest definition

DROP TABLE IF EXISTS `locales_points_of_interest`;

CREATE TABLE `locales_points_of_interest` (
  `entry` int unsigned NOT NULL DEFAULT '0',
  `language_code` varchar(5) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `icon_name` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  UNIQUE KEY `locales_points_of_interest_entry_IDX` (`entry`,`language_code`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Points of interest';

-- Remove unused table trainer_defs

DROP TABLE IF EXISTS `trainer_defs`;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('143', '20250829-00_missing_locales_tables');
