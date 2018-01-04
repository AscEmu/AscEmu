-- Barbershop in UnderCity (Horde)
UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=5496;

-- Barbershop in Orgrimmar (Horde)
UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=4958;

-- Barbershop in Orgrimmar (Horde)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=4718;

-- Barbershop in Orgrimmar (Horde)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=5136;

-- Barbershop in Stormwind City (Alliance)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=54964;

-- Barbershop in Stormwind City (Alliance)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=54965;

-- Barbershop in Stormwind City (Alliance)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=54966;

-- Barbershop in Ironforge (Alliance)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=54962;

-- Barbershop in Ironforge (Alliance)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=54960;

-- Barbershop in Dalaran (Neutral)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=57093;

-- Barbershop in Dalaran (Neutral)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=57136;

-- Barbershop in Dalaran (Neutral)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=57144;

-- Barbershop in Area 52 (Neutral)

DELETE FROM `gameobject_properties` WHERE  `entry`=190703;
DELETE FROM `gameobject_spawns` WHERE  `id`=165966;
INSERT INTO `gameobject_properties` (`entry`, `type`, `display_id`, `name`, `UnkStr`, `parameter_0`) VALUES ('190703', '32', '7896', 'Barbershop Chair', '', '2');
INSERT INTO `gameobject_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides`) VALUES ('165966', '190703', '530', '2969.838867', '3752.694336', '144.238983', '-1.26536', '0', '0', '0', '0', '1', '0', '1735', '1', '0', '1', '0');

-- Barbershop in Cosmowrench (Neutral)

DELETE FROM `gameobject_properties` WHERE  `entry`=190705;
DELETE FROM `gameobject_spawns` WHERE  `id`=165967;
INSERT INTO `gameobject_properties` (`entry`, `type`, `display_id`, `name`, `UnkStr`, `parameter_0`) VALUES ('190705', '32', '7896', 'Barbershop Chair', '', '2');
INSERT INTO `gameobject_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides`) VALUES ('165967', '190705', '530', '2968.990967', '1773.833618', '139.418091', '-1.26536', '0', '0', '0', '0', '1', '0', '1735', '1', '0', '1', '0');

-- Highback Chair in Stormwind City (Alliance)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=62000;
UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=61998;
UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=61997;

-- Set last world sql update
UPDATE `world_db_version` SET `LastUpdate` = '2018-01-01_01_gameobject_spawns' WHERE `LastUpdate` = '2017-12-30_01_console_fix';