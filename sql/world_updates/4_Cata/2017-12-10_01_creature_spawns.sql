-- 
-- Delete creature_spawns Event : The Battle For The Undercity
--

DELETE FROM `creature_spawns` WHERE  `id`=139057;
DELETE FROM `creature_spawns` WHERE  `id`=139952;
    
-- Set last world sql update
UPDATE `world_db_version` SET `LastUpdate` = '2017-12-10_01_creature_spawns' WHERE `LastUpdate` = '2017-11-09_01_npc_script_text';