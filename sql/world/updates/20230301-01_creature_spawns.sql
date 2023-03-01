-- fix cata Deathguard Elite
UPDATE `creature_spawns` SET `max_build`=15595 WHERE  `id`=48107 AND `min_build`=12340;
-- fix cata Maggot Eye
UPDATE `creature_spawns` SET `max_build`=15595 WHERE  `id`=12555 AND `min_build`=12340;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('120', '20230301-01_creature_spawns');
