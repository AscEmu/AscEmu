CREATE TABLE IF NOT EXISTS `account_achievement` (
    `account_id` INT UNSIGNED NOT NULL,
    `achievement` INT UNSIGNED NOT NULL,
    `date` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`account_id`, `achievement`),
    KEY `idx_achievement` (`achievement`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `account_achievement_progress` (
    `account_id` INT UNSIGNED NOT NULL,
    `criteria` INT UNSIGNED NOT NULL,
    `counter` INT UNSIGNED NOT NULL DEFAULT 0,
    `date` INT UNSIGNED NOT NULL DEFAULT 0,
    `player_guid` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`account_id`, `criteria`),
    KEY `idx_criteria` (`criteria`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

SET @add_player_guid := IF(
    EXISTS (
        SELECT 1
        FROM `information_schema`.`COLUMNS`
        WHERE `TABLE_SCHEMA` = DATABASE()
          AND `TABLE_NAME` = 'account_achievement_progress'
          AND `COLUMN_NAME` = 'player_guid'
    ),
    'SELECT 1',
    'ALTER TABLE `account_achievement_progress` ADD COLUMN `player_guid` BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER `date`'
);
PREPARE stmt FROM @add_player_guid;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

INSERT INTO `character_db_version` (`LastUpdate`)
SELECT '20261009-00_account_achievements'
WHERE NOT EXISTS (SELECT 1 FROM `character_db_version` WHERE `LastUpdate` = '20261009-00_account_achievements');
