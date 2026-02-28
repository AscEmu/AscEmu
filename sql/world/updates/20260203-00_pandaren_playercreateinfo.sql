-- Pandaren (MoP 5.4.8 build 18414): playercreateinfo, playercreateinfo_items, worldmap_info
-- Adds starting position (Wandering Isle, map 860, zone 5736), basic starting equipment,
-- and worldmap_info for map 860 so the server creates the map and players can enter the world.
--
-- Required table structure (AscEmu):
--   worldmap_info: entry, screenid, type, maxplayers, minlevel, minlevel_heroic, repopx, repopy, repopz, repopentry, area_name, flags, cooldown, lvl_mod_a, required_quest_A, required_quest_H, required_item, heroic_keyid_1, heroic_keyid_2, viewingDistance, required_checkpoint, build
--   playercreateinfo: race, class, mapID, zoneID, positionX, positionY, positionZ, orientation, build
--   playercreateinfo_items: race, class, protoid, slotid, amount, build
-- Equipment slot ids: 0=head, 1=neck, 2=shoulders, 3=body, 4=chest, 5=waist, 6=legs, 7=feet, 8=wrists, 9=hands, 14=back, 15=mainhand, 16=offhand, 17=ranged, 18=tabard
--
-- Without worldmap_info for map 860, MapMgr never creates the WorldMap for 860, AddToWorld() fails, and the client hangs on loading.

SET @mop_build := 18414;
SET @map_id := 860;
SET @zone_id := 5736;
SET @pos_x := 1470.0;
SET @pos_y := 3460.0;
SET @pos_z := 182.0;
SET @pos_o := 6.28;

-- Avoid duplicate key if script is re-run (adjust if your table has different PK/unique)
DELETE FROM `playercreateinfo` WHERE `race` IN (24, 25, 26) AND `build` = @mop_build;
DELETE FROM `playercreateinfo_items` WHERE `race` IN (24, 25, 26) AND `build` = @mop_build;

-- worldmap_info: map 860 (The Wandering Isle) - required for server to create WorldMap so Pandaren can enter world
-- type 0 = INSTANCE_NULL (open world). Without this row, findWorldMap(860,0) returns nullptr and login hangs.
DELETE FROM `worldmap_info` WHERE `entry` = @map_id AND `build` = @mop_build;
INSERT INTO `worldmap_info` (`entry`, `screenid`, `type`, `maxplayers`, `minlevel`, `minlevel_heroic`, `repopx`, `repopy`, `repopz`, `repopentry`, `area_name`, `flags`, `cooldown`, `lvl_mod_a`, `required_quest_A`, `required_quest_H`, `required_item`, `heroic_keyid_1`, `heroic_keyid_2`, `viewingDistance`, `required_checkpoint`, `build`) VALUES
(860, 0, 0, 0, 1, 0, @pos_x, @pos_y, @pos_z, 860, 'The Wandering Isle', 0, 0, 0, 0, 0, 0, 0, 0, 100.0, 0, @mop_build);

-- playercreateinfo: starting position for all Pandaren race/class combinations
INSERT INTO `playercreateinfo` (`race`, `class`, `mapID`, `zoneID`, `positionX`, `positionY`, `positionZ`, `orientation`, `build`) VALUES
(24, 1, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(24, 3, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(24, 4, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(24, 5, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(24, 8, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(24, 10, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(25, 1, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(25, 3, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(25, 4, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(25, 5, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(25, 8, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(25, 10, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(26, 1, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(26, 3, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(26, 4, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(26, 5, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(26, 8, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build),
(26, 10, @map_id, @zone_id, @pos_x, @pos_y, @pos_z, @pos_o, @mop_build);

-- playercreateinfo_items: basic starting clothing (slot 3=body/shirt, 6=legs)
-- Item IDs 49 (Footpad's Shirt) and 48 (Footpad's Pants) are classic and usually present in world DB.
-- For MoP-specific look use 82900 (Pandaren Initiate's Shirt) etc. if your item_properties has them.
INSERT INTO `playercreateinfo_items` (`race`, `class`, `protoid`, `slotid`, `amount`, `build`) VALUES
(24, 1, 49, 3, 1, @mop_build),
(24, 1, 48, 6, 1, @mop_build),
(24, 3, 49, 3, 1, @mop_build),
(24, 3, 48, 6, 1, @mop_build),
(24, 4, 49, 3, 1, @mop_build),
(24, 4, 48, 6, 1, @mop_build),
(24, 5, 49, 3, 1, @mop_build),
(24, 5, 48, 6, 1, @mop_build),
(24, 8, 49, 3, 1, @mop_build),
(24, 8, 48, 6, 1, @mop_build),
(24, 10, 49, 3, 1, @mop_build),
(24, 10, 48, 6, 1, @mop_build),
(25, 1, 49, 3, 1, @mop_build),
(25, 1, 48, 6, 1, @mop_build),
(25, 3, 49, 3, 1, @mop_build),
(25, 3, 48, 6, 1, @mop_build),
(25, 4, 49, 3, 1, @mop_build),
(25, 4, 48, 6, 1, @mop_build),
(25, 5, 49, 3, 1, @mop_build),
(25, 5, 48, 6, 1, @mop_build),
(25, 8, 49, 3, 1, @mop_build),
(25, 8, 48, 6, 1, @mop_build),
(25, 10, 49, 3, 1, @mop_build),
(25, 10, 48, 6, 1, @mop_build),
(26, 1, 49, 3, 1, @mop_build),
(26, 1, 48, 6, 1, @mop_build),
(26, 3, 49, 3, 1, @mop_build),
(26, 3, 48, 6, 1, @mop_build),
(26, 4, 49, 3, 1, @mop_build),
(26, 4, 48, 6, 1, @mop_build),
(26, 5, 49, 3, 1, @mop_build),
(26, 5, 48, 6, 1, @mop_build),
(26, 8, 49, 3, 1, @mop_build),
(26, 8, 48, 6, 1, @mop_build),
(26, 10, 49, 3, 1, @mop_build),
(26, 10, 48, 6, 1, @mop_build);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('159', '20260203-00_pandaren_playercreateinfo');
