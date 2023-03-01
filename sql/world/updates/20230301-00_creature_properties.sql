-- https://www.wowhead.com/wotlk/npc=23947/westguard-ranged-trigger

UPDATE `creature_properties` SET `isTriggerNpc` = 15 WHERE `entry`IN(23867, 23917, 23916, 23947);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('119', '20230301-00_creature_properties');
