-- FIX: Table `creature_formations` includes formation data for invalid spawn id 151206. Skipped!
DELETE FROM `creature_formations` WHERE  `spawn_id`=151206;

-- FIX: Table `creature_formations` includes formation data for invalid spawn id 151193. Skipped!
DELETE FROM `creature_formations` WHERE  `spawn_id`=151193;

-- Set last world sql update
UPDATE `world_db_version` SET `LastUpdate` = '2017-12-30_01_console_fix' WHERE `LastUpdate` = '2017-12-10_01_creature_spawns';