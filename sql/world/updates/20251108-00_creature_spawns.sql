-- double spawn
DELETE FROM `creature_spawns` WHERE `id`=5403;
DELETE FROM `creature_spawns` WHERE `id`=20663;
DELETE FROM `creature_spawns` WHERE `id`=34043;
DELETE FROM `creature_spawns` WHERE `id`=45483;
DELETE FROM `creature_spawns` WHERE `id`=51331;
DELETE FROM `creature_spawns` WHERE `id`=64730;
DELETE FROM `creature_spawns` WHERE `id`=122484;
DELETE FROM `creature_spawns` WHERE `id`=134171;
DELETE FROM `creature_spawns` WHERE `id`=134729;
DELETE FROM `creature_spawns` WHERE `id`=138439;
DELETE FROM `creature_spawns` WHERE `id`=263711;
DELETE FROM `creature_spawns` WHERE `id`=284231;
DELETE FROM `creature_spawns` WHERE `id`=284390;
DELETE FROM `creature_spawns` WHERE `id`=284550;
DELETE FROM `creature_spawns` WHERE `id`=285764;
DELETE FROM `creature_spawns` WHERE `id`=285766;
DELETE FROM `creature_spawns` WHERE `id`=286798;
DELETE FROM `creature_spawns` WHERE `id`=288327;
DELETE FROM `creature_spawns` WHERE `id`=288328;
DELETE FROM `creature_spawns` WHERE `id`=289416;
DELETE FROM `creature_spawns` WHERE `id`=289642;
DELETE FROM `creature_spawns` WHERE `id`=298161;
DELETE FROM `creature_spawns` WHERE `id`=313793;
DELETE FROM `creature_spawns` WHERE `id`=313892;
DELETE FROM `creature_spawns` WHERE `id`=313903;

-- fix Anduin Wrynn
DELETE FROM `creature_spawns` WHERE `id`=58983 AND `min_build`=12340;
UPDATE `creature_spawns` SET `id`=58983, `position_x`='-8440.71', `position_y`='332.994', `position_z`='123.57928', `orientation`='2.25725' WHERE  `id`=58982 AND `min_build`=12340;
UPDATE `creature_spawns` SET `orientation`=6.280503 WHERE  `id`=57799 AND `min_build`=12340;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('156', '20251108-00_creature_spawns');

