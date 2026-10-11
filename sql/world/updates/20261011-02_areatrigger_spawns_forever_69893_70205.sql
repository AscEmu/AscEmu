CREATE TABLE IF NOT EXISTS `areatrigger_spawns` (
  `entry` INT UNSIGNED NOT NULL,
  `map` INT UNSIGNED NOT NULL DEFAULT 0,
  `position_x` FLOAT NOT NULL DEFAULT 0,
  `position_y` FLOAT NOT NULL DEFAULT 0,
  `position_z` FLOAT NOT NULL DEFAULT 0,
  `radius` FLOAT NOT NULL DEFAULT 0,
  `box_x` FLOAT NOT NULL DEFAULT 0,
  `box_y` FLOAT NOT NULL DEFAULT 0,
  `box_z` FLOAT NOT NULL DEFAULT 0,
  `orientation` FLOAT NOT NULL DEFAULT 0,
  `min_build` INT UNSIGNED NOT NULL DEFAULT 0,
  `max_build` INT UNSIGNED NOT NULL DEFAULT 2147483647,
  PRIMARY KEY (`entry`, `min_build`, `max_build`),
  KEY `idx_areatrigger_spawns_map_build` (`map`, `min_build`, `max_build`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Stormwind Stockades - Entering. This legacy trigger is intentionally absent
-- from Forever AreaTrigger.db2, so its source volume must come from world data.
INSERT INTO `areatrigger_spawns`
(`entry`, `map`, `position_x`, `position_y`, `position_z`, `radius`, `box_x`, `box_y`, `box_z`, `orientation`, `min_build`, `max_build`)
VALUES
(101, 0, -8761.85, 848.557, 87.8052, 0, 4.972, 9.694, 7.444, 0.6632, 69893, 70205)
ON DUPLICATE KEY UPDATE
`map` = VALUES(`map`),
`position_x` = VALUES(`position_x`),
`position_y` = VALUES(`position_y`),
`position_z` = VALUES(`position_z`),
`radius` = VALUES(`radius`),
`box_x` = VALUES(`box_x`),
`box_y` = VALUES(`box_y`),
`box_z` = VALUES(`box_z`),
`orientation` = VALUES(`orientation`);

-- Ensure AscEmu has the matching action/destination for the source trigger.
INSERT INTO `areatriggers`
(`entry`, `type`, `map`, `screen`, `name`, `position_x`, `position_y`, `position_z`, `orientation`, `required_honor_rank`, `required_level`, `min_build`, `max_build`)
VALUES
(101, 1, 34, 0, 'Stormwind Stockades - Entering', 48.9849, 0.483882, -16.3942, 0, 0, 15, 69893, 70205)
ON DUPLICATE KEY UPDATE
`type` = VALUES(`type`),
`map` = VALUES(`map`),
`screen` = VALUES(`screen`),
`name` = VALUES(`name`),
`position_x` = VALUES(`position_x`),
`position_y` = VALUES(`position_y`),
`position_z` = VALUES(`position_z`),
`orientation` = VALUES(`orientation`),
`required_honor_rank` = VALUES(`required_honor_rank`),
`required_level` = VALUES(`required_level`),
`min_build` = VALUES(`min_build`),
`max_build` = VALUES(`max_build`);

INSERT INTO `world_db_version` (`LastUpdate`)
SELECT '20261011-02_areatrigger_spawns_forever_69893_70205'
WHERE NOT EXISTS (
    SELECT 1 FROM `world_db_version`
    WHERE `LastUpdate` = '20261011-02_areatrigger_spawns_forever_69893_70205'
);
