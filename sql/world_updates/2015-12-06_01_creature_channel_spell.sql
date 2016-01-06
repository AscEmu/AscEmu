--
-- Updates Entrys 30078 and 27306 With Correct invisibility_type
--
REPLACE INTO `creature_proto` VALUES (30078, 1, 1, 35, 2215, 2215, 0, 1, 0, 2000, 0, 85, 112, 0, 0, 0, 0, 300000, 2966, 0, 0, 0, 0, 0, 0, 4, 1.806, '', 0, 0, 0, 2.5, 8, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
REPLACE INTO `creature_proto` VALUES (27306, 1, 1, 35, 42, 42, 0, 1, 0, 2000, 0, 2, 2, 0, 0, 0, 0, 300000, 7, 0, 0, 0, 0, 0, 0, 3, 2, '48387', 0, 0, 0, 2.5, 8, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

--
-- Added 2 30078 Spawns to Ulduar's Protection Bubble
--
INSERT INTO `creature_spawns` VALUES (145453, 27306, 603, -666.024, -3.625, 519.563, 0.432717, 0, 11686, 35, 33555200, 16777472, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1);
INSERT INTO `creature_spawns` VALUES (145454, 27306, 603, -641.688, -100.817, 519.205, 6.03707, 0, 11686, 35, 33555200, 16777472, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1);

--
-- 2 Battle-Mages added Channel spell with correct aura linked to 30078 Spawns IDs in Ulduar
--
REPLACE INTO `creature_spawns` VALUES (131951, 33662, 603, -675.228, -18.3325, 426.383, 0.942478, 0, 28886, 2008, 33536, 66048, 0, 1, 0, 0, 48310, 0, 145453, 0, 0, 0, 45726, 45727, 0, 0, 1);
REPLACE INTO `creature_spawns` VALUES (131949, 33662, 603, -673.13, -77.4619, 426.448, 5.44543, 0, 28886, 2008, 33536, 66048, 0, 1, 0, 0, 48310, 0, 145454, 0, 0, 0, 45726, 45727, 0, 0, 1);

--
-- Deleted ID: 49857 from Gameobject_Spawns. Added ID: 49857 to gameobject_staticspawns (Dome No longer Disappears)
--
DELETE FROM `gameobject_spawns` WHERE id = 49857;
INSERT INTO `gameobject_staticspawns` VALUES (49857, 194484, 603, -805.1, -78.23, 605.22, -2.65, 0, 0, 0, 0, 0, 0, 0, 3, 0, 1, 0);


--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2015-12-06_01_creature_channel_spell' WHERE `LastUpdate` = '2015-11-28_01_event_names';
