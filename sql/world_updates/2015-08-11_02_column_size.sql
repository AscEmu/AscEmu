-- ----------------------------
-- Change columns size event_creature_spawns
-- ----------------------------
ALTER TABLE `event_creature_spawns` MODIFY `eventEntry` INT(3) unsigned NOT NULL;
ALTER TABLE `event_creature_spawns` MODIFY `position_x` FLOAT(0) NOT NULL;
ALTER TABLE `event_creature_spawns` MODIFY `position_y` FLOAT(0) NOT NULL;
ALTER TABLE `event_creature_spawns` MODIFY `position_z` FLOAT(0) NOT NULL;
ALTER TABLE `event_creature_spawns` MODIFY `orientation` FLOAT(0) NOT NULL;
ALTER TABLE `event_creature_spawns` MODIFY `waypoint_group` INT(10) unsigned DEFAULT NULL COMMENT 'waypoint group in table creature_waypoints_manual';

-- ----------------------------
-- Change columns size event_gameobject_spawns
-- ----------------------------
ALTER TABLE `event_gameobject_spawns` MODIFY `eventEntry` INT(3) unsigned NOT NULL;
ALTER TABLE `event_gameobject_spawns` MODIFY `position_x` FLOAT(0) NOT NULL;
ALTER TABLE `event_gameobject_spawns` MODIFY `position_y` FLOAT(0) NOT NULL;
ALTER TABLE `event_gameobject_spawns` MODIFY `position_z` FLOAT(0) NOT NULL;
ALTER TABLE `event_gameobject_spawns` MODIFY `Facing` FLOAT(0) NOT NULL;
ALTER TABLE `event_gameobject_spawns` MODIFY `orientation1` FLOAT(0) NOT NULL;
ALTER TABLE `event_gameobject_spawns` MODIFY `orientation2` FLOAT(0) NOT NULL;
ALTER TABLE `event_gameobject_spawns` MODIFY `orientation3` FLOAT(0) NOT NULL;
ALTER TABLE `event_gameobject_spawns` MODIFY `orientation4` FLOAT(0) NOT NULL;

-- ----------------------------
-- Change columns size/names creature_staticspawns
-- ----------------------------
ALTER TABLE `creature_staticspawns` MODIFY `id` INT(11) unsigned NOT NULL AUTO_INCREMENT;
ALTER TABLE `creature_staticspawns` MODIFY `entry` MEDIUMINT(10) unsigned NOT NULL;
ALTER TABLE `creature_staticspawns` CHANGE `Map` `map` SMALLINT(5) unsigned NOT NULL;
ALTER TABLE `creature_staticspawns` CHANGE `x` `position_x` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` CHANGE `y` `position_y` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` CHANGE `z` `position_z` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` CHANGE `o` `orientation` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` MODIFY `movetype` TINYINT(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` MODIFY `displayid` MEDIUMINT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` CHANGE `factionid` `faction` MEDIUMINT(10) unsigned NOT NULL DEFAULT '14';
ALTER TABLE `creature_staticspawns` MODIFY `flags` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` MODIFY `bytes0` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` MODIFY `bytes1` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` MODIFY `bytes2` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` MODIFY `emote_state` SMALLINT(5) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` MODIFY `npc_respawn_link` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` MODIFY `channel_spell` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` MODIFY `channel_target_sqlid` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` MODIFY `channel_target_sqlid_creature` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` MODIFY `standstate` TINYINT(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_staticspawns` MODIFY `CanFly` SMALLINT(3) unsigned NOT NULL DEFAULT '0';

-- ----------------------------
-- Change columns size/names gameobject_staticspawns
-- ----------------------------
ALTER TABLE `gameobject_staticspawns` MODIFY `id` INT(11) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Unique Spawn Identifier';
ALTER TABLE `gameobject_staticspawns` MODIFY `entry` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Gameobject Identifier';
ALTER TABLE `gameobject_staticspawns` MODIFY `map` INT(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier';
ALTER TABLE `gameobject_staticspawns` CHANGE `x` `position_x` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_staticspawns` CHANGE `y` `position_y` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_staticspawns` CHANGE `z` `position_z` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_staticspawns` MODIFY `facing` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_staticspawns` CHANGE `o` `orientation1` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_staticspawns` CHANGE `o1` `orientation2` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_staticspawns` CHANGE `o2` `orientation3` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_staticspawns` CHANGE `o3` `orientation4` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_staticspawns` MODIFY `state` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_staticspawns` MODIFY `flags` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_staticspawns` MODIFY `faction` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_staticspawns` MODIFY `scale` FLOAT(0) unsigned NOT NULL DEFAULT '1';
ALTER TABLE `gameobject_staticspawns` MODIFY `respawnNpcLink` INT(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_staticspawns` MODIFY `phase` INT(10) unsigned NOT NULL DEFAULT '1' COMMENT 'Phase mask';
ALTER TABLE `gameobject_staticspawns` MODIFY `overrides` INT(10) unsigned NOT NULL DEFAULT '0';

-- ----------------------------
-- Change columns size/names gameobject_spawns
-- ----------------------------
ALTER TABLE `gameobject_spawns` CHANGE `Entry` `entry` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Gameobject Identifier';
ALTER TABLE `gameobject_spawns` MODIFY `map` INT(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier';
ALTER TABLE `gameobject_spawns` MODIFY `position_x` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_spawns` MODIFY `position_y` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_spawns` MODIFY `position_z` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_spawns` CHANGE `Facing` `facing` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_spawns` MODIFY `orientation1` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_spawns` MODIFY `orientation2` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_spawns` MODIFY `orientation3` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_spawns` MODIFY `orientation4` FLOAT(0) NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_spawns` CHANGE `State` `state` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_spawns` CHANGE `Flags` `flags` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_spawns` CHANGE `Faction` `faction` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_spawns` CHANGE `Scale` `scale` FLOAT(0) unsigned NOT NULL DEFAULT '1';
ALTER TABLE `gameobject_spawns` CHANGE `stateNpcLink` `respawnNpcLink` INT(11) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_spawns` MODIFY `overrides` INT(10) unsigned NOT NULL DEFAULT '0';

UPDATE `world_db_version` SET `LastUpdate` = '2015-08-11_02_column_size' WHERE `LastUpdate` = '2015-08-11_01_remove_duplicate';
  