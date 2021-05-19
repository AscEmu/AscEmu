ALTER TABLE `creature_spawns`
ADD COLUMN `wander_distance` int(10) NOT NULL DEFAULT 20 AFTER `event_entry`;

-- Set wander distance to 0 in spawns that do not use random movement
UPDATE `creature_spawns` SET `wander_distance`='0' WHERE NOT `movetype`='1';

INSERT INTO `world_db_version` VALUES ('76', '20210525-00_creature_spawns');
