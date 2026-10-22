ALTER TABLE `creature_difficulty` ADD COLUMN `build` int unsigned NOT NULL DEFAULT 5875 AFTER `entry`, DROP PRIMARY KEY, ADD PRIMARY KEY (`entry`, `build`);

ALTER TABLE `creature_properties_movement` ADD COLUMN `build` int unsigned NOT NULL DEFAULT 5875 AFTER `CreatureId`, DROP PRIMARY KEY, ADD PRIMARY KEY (`CreatureId`, `build`);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('215', '20261022-15_creature_tables_build');
