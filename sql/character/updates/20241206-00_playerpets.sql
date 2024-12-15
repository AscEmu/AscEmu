ALTER TABLE `playerpets` DROP COLUMN `happinessupdate`;
ALTER TABLE `playerpets` MODIFY COLUMN `petnumber` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `ownerguid`;
ALTER TABLE `playerpets` MODIFY COLUMN `type` TINYINT UNSIGNED NOT NULL DEFAULT 1 AFTER `petnumber`;
ALTER TABLE `playerpets` ADD COLUMN `model` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `entry`;
ALTER TABLE `playerpets` MODIFY COLUMN `level` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `model`;
ALTER TABLE `playerpets` ADD COLUMN `slot` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `xp`;
ALTER TABLE `playerpets` MODIFY COLUMN `alive` TINYINT UNSIGNED NOT NULL DEFAULT 1 AFTER `active`;
ALTER TABLE `playerpets` MODIFY COLUMN `actionbar` LONGTEXT NOT NULL AFTER `alive`;
ALTER TABLE `playerpets` MODIFY COLUMN `petstate` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `spellid`;
ALTER TABLE `playerpets` MODIFY COLUMN `renamable` TINYINT UNSIGNED NOT NULL DEFAULT 1 AFTER `current_happiness`;

UPDATE `playerpets` SET `slot`='5' WHERE `active` >= '20';
UPDATE `playerpets` SET `slot`='0' WHERE `active` >= '10';
UPDATE `playerpets` SET `active`='0';

UPDATE `character_db_version` SET LastUpdate = '20241206-00_playerpets';
