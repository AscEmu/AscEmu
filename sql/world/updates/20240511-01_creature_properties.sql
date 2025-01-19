ALTER TABLE `creature_properties`
	CHANGE COLUMN `info_str` `icon_name` VARCHAR(100) NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `subname`;

-- Update world db version
INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('139', '20240511-01_creature_properties');
