ALTER TABLE `characters` ADD COLUMN `enabled_actionbars` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `player_flags`;
UPDATE `characters` SET `enabled_actionbars`=(player_bytes >> 16) & 0xFF;
ALTER TABLE `characters` DROP COLUMN `player_bytes`;

-- Remove problematic version specific player flags
UPDATE `characters` SET `player_flags` = `player_flags` & ~(0x00000080 | 0x00008000 | 0x00010000 | 0x00040000);

INSERT INTO `character_db_version` (`id`, `LastUpdate`) VALUES ('14', '20250516-00_characters');
