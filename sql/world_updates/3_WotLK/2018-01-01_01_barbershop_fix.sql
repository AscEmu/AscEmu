-- Barbershop in UnderCity (Horde)
UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=5496;
SELECT `id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides` FROM `gameobject_spawns` WHERE `id`=5496;

-- Barbershop in Orgrimmar (Horde)
UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=4958;
SELECT `id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides` FROM `gameobject_spawns` WHERE `id`=4958;

-- Barbershop in Orgrimmar (Horde)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=4718;
SELECT `id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides` FROM `gameobject_spawns` WHERE `id`=4718;

-- Barbershop in Orgrimmar (Horde)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=5136;
SELECT `id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides` FROM `gameobject_spawns` WHERE  `id`=5136;

-- Barbershop in Stormwind City (Alliance)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=54964;
SELECT `id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides` FROM `gameobject_spawns` WHERE  `id`=54964;

-- Barbershop in Stormwind City (Alliance)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=54965;
SELECT `id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides` FROM `gameobject_spawns` WHERE  `id`=54965;

-- Barbershop in Stormwind City (Alliance)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=54966;
SELECT `id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides` FROM `gameobject_spawns` WHERE  `id`=54966;

-- Barbershop in Ironforge (Alliance)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=54962;
SELECT `id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides` FROM `gameobject_spawns` WHERE  `id`=54962;

-- Barbershop in Ironforge (Alliance)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=54960;
SELECT `id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides` FROM `gameobject_spawns` WHERE  `id`=54960;

-- Barbershop in Dalaran (Neutral)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=57093;
SELECT `id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides` FROM `gameobject_spawns` WHERE  `id`=57093;

-- Barbershop in Dalaran (Neutral)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=57136;
SELECT `id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides` FROM `gameobject_spawns` WHERE  `id`=57136;

-- Barbershop in Dalaran (Neutral)

UPDATE `gameobject_spawns` SET `orientation1`='0', `orientation2`='0', `orientation3`='0', `orientation4`='0' WHERE `id`=57144;
SELECT `id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `state`, `flags`, `faction`, `scale`, `respawnNpcLink`, `phase`, `overrides` FROM `gameobject_spawns` WHERE  `id`=57144;


-- -- Barbershop in  Area 52 (Neutral)

--  no chair unk_id


-- Barbershop in Cosmowrench (Neutral)

-- no chair unk_id

-- Set last world sql update
UPDATE `world_db_version` SET `LastUpdate` = '2018-01-01_01_barbershop_fix' WHERE `LastUpdate` = '2017-12-30_01_console_fix';