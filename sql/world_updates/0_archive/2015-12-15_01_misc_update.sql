
-- Magister Surdiel <Wintergrasp Battle-Mage>
UPDATE `creature_spawns` SET `position_x`=5925.71, `position_y`=573.495, `position_z`=661.08, `orientation`=5.67371 WHERE  `id`=129229;

-- High Arcanist Savor
UPDATE `creature_spawns` SET `position_x`=5956.71, `position_y`=568.404, `position_z`=660.48, `orientation`=4.10528 WHERE  `id`=123263;

-- Horde Warbringer
UPDATE `creature_spawns` SET `position_x`=5952.36, `position_y`=570.346, `position_z`=660.48, `orientation`=4.57024 WHERE  `id`=62656;

-- Changes to phase for Isle of Conquest Portal Niche Horde
UPDATE `gameobject_spawns` SET `phase`=1 WHERE  `id`=51377;

-- Changes to phase for Isle of Conquest Portal Niche Alliance
UPDATE `gameobject_spawns` SET `phase`=1 WHERE  `id`=51375;

-- Added banner Horder (Isle of Conquest Portal Niche Horde)
INSERT INTO `gameobject_spawns` VALUES (141714, 195533, 571, 5913.95, 554.53, 661.11, 0.99, 0, 0, 0, 1, 1, 0, 0, 1.5, 0, 1, 0);

-- Added banner Horder (Isle of Conquest Portal Niche Alliance)
INSERT INTO `gameobject_spawns` VALUES (141715, 195532, 571, 5674.47, 807.51, 654.31, 4.53, 0, 0, 0, 1, 1, 0, 0, 1.5, 0, 1, 0);


--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2015-12-15_01_misc_update' WHERE `LastUpdate` = '2015-12-14_02_items_itemset';
