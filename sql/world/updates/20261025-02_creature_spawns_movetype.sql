-- 51 creature spawns with invalid movetype set to idle
UPDATE `creature_spawns` SET `movetype` = 0, `wander_distance` = 0 WHERE `movetype` = 4 AND `entry` = 28833 AND `id` IN (119001,119002,119003,119004,119005,119006);
UPDATE `creature_spawns` SET `movetype` = 0, `wander_distance` = 0 WHERE `movetype` = 4 AND `entry` = 28834 AND `id` IN (119007,119008);
UPDATE `creature_spawns` SET `movetype` = 0, `wander_distance` = 0 WHERE `movetype` = 5 AND `entry` = 51714 AND `id` IN (660944,660948,660958,660962,661039,661040,661042,661045,661049,661065,661066,661067,661068,661070,661071,661078,661079,661086,661088,661095,661097,661099,661100,661101,661106,661107,661118,661121,661124,680644,680646,680650);
UPDATE `creature_spawns` SET `movetype` = 0, `wander_distance` = 0 WHERE `movetype` = 5 AND `entry` = 51752 AND `id` IN (661109,661110,661111,661112,661113,661114,661115,661116,661117,680651);
UPDATE `creature_spawns` SET `movetype` = 0, `wander_distance` = 0 WHERE `movetype` = 10 AND `entry` = 222332 AND `id` IN (748032);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('269', '20261025-02_creature_spawns_movetype');
