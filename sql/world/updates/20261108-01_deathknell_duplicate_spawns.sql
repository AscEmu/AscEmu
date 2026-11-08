-- 3 duplicate 4.3.4 spawns of the Deathknell graves (second Aradne, two grave targets)
DELETE FROM `creature_spawns` WHERE `id` IN (542800, 490467, 490468) AND `min_build` = 15595 AND `max_build` = 15595;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('289', '20261108-01_deathknell_duplicate_spawns');
