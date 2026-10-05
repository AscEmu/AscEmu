
SET FOREIGN_KEY_CHECKS = 0;

-- -----------------------------------------------------------------------------
-- Creature tables
-- -----------------------------------------------------------------------------

ALTER TABLE `creature_difficulty`
    DROP PRIMARY KEY,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `entry`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD PRIMARY KEY (`entry`,`min_build`) USING BTREE;

ALTER TABLE `creature_formations`
    DROP PRIMARY KEY,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `memberGUID`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD PRIMARY KEY (`memberGUID`,`min_build`) USING BTREE;

ALTER TABLE `creature_group_spawn`
    DROP PRIMARY KEY,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `spawnId`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD PRIMARY KEY (`groupId`,`spawnId`,`min_build`) USING BTREE;

ALTER TABLE `creature_initial_equip`
    DROP PRIMARY KEY,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `creature_entry`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD PRIMARY KEY (`creature_entry`,`min_build`) USING BTREE;

ALTER TABLE `creature_movement_override`
    DROP PRIMARY KEY,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `SpawnId`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD PRIMARY KEY (`SpawnId`,`min_build`) USING BTREE;

ALTER TABLE `creature_properties_movement`
    DROP PRIMARY KEY,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `CreatureId`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD PRIMARY KEY (`CreatureId`,`min_build`) USING BTREE;

ALTER TABLE `creature_timed_emotes`
    DROP PRIMARY KEY,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `rowid`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD PRIMARY KEY (`spawnid`,`rowid`,`min_build`) USING BTREE;

ALTER TABLE `creature_waypoints`
    DROP PRIMARY KEY,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `point`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD PRIMARY KEY (`id`,`point`,`min_build`) USING BTREE;

ALTER TABLE `currency_creature_onkill`
    DROP PRIMARY KEY,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `currency_id`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD PRIMARY KEY (`creature_id`,`currency_id`,`min_build`) USING BTREE;

-- -----------------------------------------------------------------------------
-- Loot tables
-- -----------------------------------------------------------------------------

ALTER TABLE `loot_creatures`
    DROP INDEX `UNIQUE`,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `itemid`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD UNIQUE INDEX `UNIQUE` (`entryid`,`itemid`,`min_build`) USING BTREE,
    ADD INDEX `idx_loot_creatures_build` (`min_build`,`max_build`) USING BTREE;

ALTER TABLE `loot_fishing`
    DROP INDEX `UNIQUE`,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `itemid`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD UNIQUE INDEX `UNIQUE` (`itemid`,`entryid`,`min_build`) USING BTREE,
    ADD INDEX `idx_loot_fishing_build` (`min_build`,`max_build`) USING BTREE;

ALTER TABLE `loot_gameobjects`
    DROP PRIMARY KEY,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `itemid`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD PRIMARY KEY (`entryid`,`itemid`,`min_build`) USING BTREE,
    ADD INDEX `idx_loot_gameobjects_build` (`min_build`,`max_build`) USING BTREE;

ALTER TABLE `loot_items`
    DROP INDEX `UNIQUE`,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `itemid`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD UNIQUE INDEX `UNIQUE` (`entryid`,`itemid`,`min_build`) USING BTREE,
    ADD INDEX `idx_loot_items_build` (`min_build`,`max_build`) USING BTREE;

ALTER TABLE `loot_pickpocketing`
    DROP INDEX `UNIQUE`,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `itemid`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD UNIQUE INDEX `UNIQUE` (`entryid`,`itemid`,`min_build`) USING BTREE,
    ADD INDEX `idx_loot_pickpocketing_build` (`min_build`,`max_build`) USING BTREE;

ALTER TABLE `loot_skinning`
    DROP INDEX `UNIQUE`,
    ADD COLUMN `min_build` INT UNSIGNED NOT NULL DEFAULT 12340 AFTER `itemid`,
    ADD COLUMN `max_build` INT UNSIGNED NOT NULL DEFAULT 18414 AFTER `min_build`,
    ADD UNIQUE INDEX `UNIQUE` (`entryid`,`itemid`,`min_build`) USING BTREE,
    ADD INDEX `idx_loot_skinning_build` (`min_build`,`max_build`) USING BTREE;

INSERT INTO `world_db_version` (`LastUpdate`)
VALUES ('20261004-00_creature_loot_build_ranges');

SET FOREIGN_KEY_CHECKS = 1;
