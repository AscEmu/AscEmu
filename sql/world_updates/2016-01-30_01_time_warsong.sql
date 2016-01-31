--
-- Time Warsong Gulch
--
INSERT INTO `ascemu_world`.`worldstate_templates` (`map`, `zone`, `field`, `value`) VALUES (489, 3277, 4247, 1);
INSERT INTO `ascemu_world`.`worldstate_templates` (`map`, `zone`, `field`, `value`) VALUES (489, 3277, 4248, 25);

--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-01-30_01_time_warsong' WHERE `LastUpdate` = '2016-01-25_02_loot_creatures';
