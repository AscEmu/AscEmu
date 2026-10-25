-- 6 wotlk gameobject spawns end at build 12340, the cata rows follow
UPDATE `gameobject_spawns` SET `max_build` = 12340 WHERE `min_build` = 12340 AND `id` IN (32368,42691,42753,45135,87072,87205);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('270', '20261025-03_gameobject_spawns');
