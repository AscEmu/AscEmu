-- Display ids for the Throne of Thunder bosses added earlier today, which had none available
-- from any reference and were left at 0. Supplied by Zyres. Jin'rokh the Breaker already had
-- the correct value (14994, from the Mop reference database) - included here too for completeness.

UPDATE `creature_properties` SET `male_displayid` = 14994 WHERE `entry` = 14902 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 46559 WHERE `entry` = 67977 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 47189 WHERE `entry` = 68036 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 46627 WHERE `entry` = 68078 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 46770 WHERE `entry` = 68397 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 47325 WHERE `entry` = 68476 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 46974 WHERE `entry` = 68904 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 46975 WHERE `entry` = 68905 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 47009 WHERE `entry` = 69017 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 47505 WHERE `entry` = 69078 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 47506 WHERE `entry` = 69131 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 47730 WHERE `entry` = 69132 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 47229 WHERE `entry` = 69134 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 47527 WHERE `entry` = 69427 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 47739 WHERE `entry` = 69473 AND `build` = 18414;
UPDATE `creature_properties` SET `male_displayid` = 120300 WHERE `entry` = 69712 AND `build` = 18414;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('180', '20260903-12_mop_throne_of_thunder_displayids');
