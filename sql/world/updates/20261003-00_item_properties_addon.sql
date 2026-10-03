-- Item property addon overrides and Forever quest item proxy support.

CREATE TABLE IF NOT EXISTS `item_properties_addon` (
    `itemid` INT UNSIGNED NOT NULL,
    `min_build` INT UNSIGNED NOT NULL DEFAULT 0,
    `max_build` INT UNSIGNED NOT NULL DEFAULT 0,
    `food_type` INT UNSIGNED NOT NULL DEFAULT 0,
    `min_money_loot` INT UNSIGNED NOT NULL DEFAULT 0,
    `max_money_loot` INT UNSIGNED NOT NULL DEFAULT 0,
    `random_bonus_list_template_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `quest_log_item_id` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`itemid`, `min_build`, `max_build`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

ALTER TABLE `quest_properties`
    MODIFY COLUMN `build` INT NOT NULL DEFAULT 12340;

INSERT INTO `item_properties_addon`
    (`itemid`, `min_build`, `max_build`, `quest_log_item_id`)
VALUES
    (247834, 69893, 70124, 247857)
ON DUPLICATE KEY UPDATE
    `quest_log_item_id` = VALUES(`quest_log_item_id`);

INSERT INTO `world_db_version` (`LastUpdate`)
VALUES ('20261003-00_item_properties_addon');
