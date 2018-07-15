/*
Update guilds table to handle cata specific fields too
*/

ALTER TABLE `guilds`
ADD COLUMN `guildLevel` INT(10) NOT NULL DEFAULT 1 AFTER `bankBalance`,
ADD COLUMN `guildExperience` BIGINT(20) NOT NULL DEFAULT 0 AFTER `guildLevel`,
ADD COLUMN `todayExperience` BIGINT(20) NOT NULL DEFAULT 0 AFTER `guildExperience`;

UPDATE `character_db_version` SET LastUpdate = '20180709-00_guilds';