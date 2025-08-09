UPDATE `creature_spawns` SET `standstate`=(bytes1) & 0xFF WHERE `standstate`='0';
ALTER TABLE `creature_spawns` ADD COLUMN `sheath_state` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `mountdisplayid`;
UPDATE `creature_spawns` SET `sheath_state`=(bytes2) & 0xFF;
ALTER TABLE `creature_spawns` ADD COLUMN `pvp_flagged` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `flags`;
UPDATE `creature_spawns` SET `pvp_flagged`=((bytes2 >> 8) & 0xFF) & 0x1;
ALTER TABLE `creature_spawns` DROP COLUMN `bytes1`, DROP COLUMN `bytes2`;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('142', '20250516-00_creature_spawns');
