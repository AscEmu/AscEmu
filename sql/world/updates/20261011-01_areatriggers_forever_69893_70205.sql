-- AscEmu
-- Make area triggers build-ranged and expose the existing Classic portal data
-- to Forever clients. Runtime validation against AreaTrigger.db2 discards rows
-- whose trigger ID no longer exists in the active client data.

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

ALTER TABLE `areatriggers`
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 8606 AFTER `entry`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    DROP PRIMARY KEY,
    ADD PRIMARY KEY (`entry`, `min_build`) USING BTREE,
    ADD UNIQUE INDEX `unique_index` (`entry`, `min_build`) USING BTREE,
    ADD INDEX `idx_areatriggers_build_range` (`min_build`, `max_build`) USING BTREE;

UPDATE `areatriggers`
SET `min_build` = 8606,
    `max_build` = 18414;

INSERT INTO `areatriggers`
    (`entry`, `min_build`, `max_build`, `type`, `map`, `screen`, `name`,
     `position_x`, `position_y`, `position_z`, `orientation`,
     `required_honor_rank`, `required_level`)
SELECT
    `entry`, 69893, 70205, `type`, `map`, `screen`, `name`,
    `position_x`, `position_y`, `position_z`, `orientation`,
    `required_honor_rank`, `required_level`
FROM `areatriggers`
WHERE `min_build` = 8606
  AND `max_build` = 18414;

INSERT INTO `world_db_version` (`LastUpdate`)
SELECT '20261011-01_areatriggers_forever_69893_70205'
WHERE NOT EXISTS (
    SELECT 1 FROM `world_db_version`
    WHERE `LastUpdate` = '20261011-01_areatriggers_forever_69893_70205'
);

SET FOREIGN_KEY_CHECKS = 1;
