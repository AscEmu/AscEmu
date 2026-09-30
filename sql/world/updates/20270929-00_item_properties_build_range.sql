-- AscEmu world database update
-- Convert item_properties from build to min_build/max_build.
-- Existing build values are preserved exactly:
-- min_build = old build
-- max_build = old build

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

ALTER TABLE `item_properties`
    DROP PRIMARY KEY,
    DROP INDEX `unique_index`,
    CHANGE COLUMN `build` `min_build` INT UNSIGNED NOT NULL DEFAULT 12340,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `min_build`,
    ADD PRIMARY KEY (`entry`, `min_build`, `max_build`) USING BTREE,
    ADD UNIQUE INDEX `unique_index` (`entry`, `min_build`, `max_build`) USING BTREE;

UPDATE `item_properties`
SET `max_build` = `min_build`;

INSERT INTO `world_db_version` (`LastUpdate`)
VALUES ('20270929-00_item_properties_build_range');

SET FOREIGN_KEY_CHECKS = 1;
