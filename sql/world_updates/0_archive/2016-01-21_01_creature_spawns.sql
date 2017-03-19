--
-- Delete existing spawns for instances FoS,PoS,HoR.
--
DELETE FROM `creature_spawns` WHERE `id` IN(134010, 134007, 134112, 134148, 141019, 141018, 134101, 141020, 141021, 133779, 141022, 134102, 134149, 141023);

--
-- Update world db version
--   
UPDATE `world_db_version` SET `LastUpdate` = '2016-01-21_01_creature_spawns' WHERE `LastUpdate` = '2016-01-20_01_creature_spawns';
