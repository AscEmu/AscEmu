UPDATE `creature_properties` SET `invisibility_type`='1' WHERE `invisibility_type` > '0';
ALTER TABLE `creature_properties` CHANGE COLUMN `invisibility_type` `isTriggerNpc` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `money`;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('105', '20220513-00_creature_properties');
