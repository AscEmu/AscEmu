
SET FOREIGN_KEY_CHECKS = 0;

ALTER TABLE `quest_properties`
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `entry`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`;

UPDATE `quest_properties` AS target
INNER JOIN (
    SELECT
        `entry`,
        `build`,
        CAST(`build` AS UNSIGNED) AS `range_min_build`,
        CASE
            WHEN LEAD(`build`) OVER (PARTITION BY `entry` ORDER BY `build`) IS NULL THEN 18414
            ELSE CAST(LEAD(`build`) OVER (PARTITION BY `entry` ORDER BY `build`) - 1 AS UNSIGNED)
        END AS `range_max_build`
    FROM `quest_properties`
) AS ranges
    ON ranges.`entry` = target.`entry`
   AND ranges.`build` = target.`build`
SET
    target.`min_build` = ranges.`range_min_build`,
    target.`max_build` = ranges.`range_max_build`;

ALTER TABLE `quest_properties`
    DROP INDEX `unique_index`,
    DROP PRIMARY KEY,
    DROP COLUMN `build`,
    ADD PRIMARY KEY (`entry`, `min_build`) USING BTREE,
    ADD UNIQUE INDEX `unique_index` (`entry`, `min_build`) USING BTREE,
    ADD INDEX `idx_quest_build_range` (`min_build`, `max_build`) USING BTREE;

INSERT INTO `world_db_version` (`LastUpdate`)
SELECT '20261005-01_quest_build_ranges'
WHERE NOT EXISTS (SELECT 1 FROM `world_db_version` WHERE `LastUpdate` = '20261005-01_quest_build_ranges');

SET FOREIGN_KEY_CHECKS = 1;
