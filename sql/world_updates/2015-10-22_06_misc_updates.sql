--
-- Added Children's Week spawns (THX Nupper)
--
INSERT INTO `event_creature_spawns`
VALUES
(10, 157038, 34520, 571, 5710.01, 641.867, 648.777, 0.0699411, 0, 25384, 35, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65535, NULL),
(10, 157039, 34520, 571, 5710.87, 637.163, 646.241, 5.84751, 0, 25384, 35, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65535, NULL),
(10, 157040, 33533, 571, 5712.48, 641.978, 648.741, 3.3342, 0, 25173, 35, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65535, NULL),
(10, 157041, 33533, 571, 5713.66, 637.908, 646.237, 5.08175, 0, 25173, 35, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65535, NULL);

--
-- DELETE weird circle of calling on GM-Island (THX Pure)
--
DELETE FROM `gameobject_spawns` WHERE `id` = 120874;


--
-- DELETE fun npcs (Don't know why they are in our world db)
--
DELETE FROM `creature_names` WHERE `entry` IN(100001,100002,100003,100004);
DELETE FROM `creature_proto` WHERE `entry` IN(100001,100002,100003,100004);
DELETE FROM `creature_spawns` WHERE `entry` IN(100001,100002,100003,100004);


--
-- Update world_db_version
--

UPDATE `world_db_version` SET `LastUpdate` = '2015-10-22_06_misc_updates' WHERE `LastUpdate` = '2015-10-22_05_misc_updates';
