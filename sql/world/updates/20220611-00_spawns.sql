UPDATE `creature_spawns` SET `max_build`='18414' WHERE `map`='609';
UPDATE `gameobject_spawns` SET `max_build`='18414' WHERE `map`='609';

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('109', '20220611-00_spawns');
