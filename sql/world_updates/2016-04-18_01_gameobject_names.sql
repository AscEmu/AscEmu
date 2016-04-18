--
-- Eye Of Acherus GO
--
UPDATE `gameobject_names` SET `type` = 10, `parameter_0` = 1635, `parameter_1` = 12641, `parameter_10` = 51888, `parameter_13` = 1 WHERE `entry` = 191609;

--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-04-18_01_gameobject_names' WHERE `LastUpdate` = '2016-03-30_02_creature_proto';
