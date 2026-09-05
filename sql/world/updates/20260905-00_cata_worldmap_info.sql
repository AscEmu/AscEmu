
INSERT INTO `worldmap_info`
(`entry`, `build`, `screenid`, `type`, `maxplayers`, `minlevel`, `minlevel_heroic`, `area_name`, `flags`, `viewingDistance`)
VALUES
-- Continent
(870, 18414, 0, 0, 0, 85, 0, 'Pandaria', 1, 500),
-- Battlegrounds
(726, 15595, 262, 3, 20, 85, 0, 'Twin Peaks', 1, 500),
(761, 15595, 270, 3, 30, 85, 0, 'Battle for Gilneas', 1, 500),
-- 5-man dungeons
(643, 15595, 255, 4, 5, 85, 85, 'Throne of the Tides', 0, 80),
(644, 15595, 256, 4, 5, 85, 85, 'Halls of Origination', 0, 80),
(645, 15595, 267, 4, 5, 85, 85, 'Blackrock Caverns', 0, 80),
(657, 15595, 259, 4, 5, 85, 85, 'The Vortex Pinnacle', 0, 80),
(670, 15595, 257, 4, 5, 85, 85, 'Grim Batol', 0, 80),
(725, 15595, 258, 4, 5, 85, 85, 'The Stonecore', 0, 80),
(755, 15595, 264, 4, 5, 85, 85, "Lost City of the Tol'vir", 0, 80),
-- Zul'Gurub's minlevel matches the already-present Zul'Aman row's own real minlevel (70, entry 568,
-- build 13914) - both are Cata 4.1 heroic revamps of pre-existing lower-level content, not new
-- level-85 designs, so they share the same real entry level rather than the expansion cap.
(859, 15595, 161, 4, 5, 70, 70, "Zul'Gurub", 0, 80),
(938, 15595, 286, 4, 5, 85, 85, 'End Time', 0, 80),
(939, 15595, 287, 4, 5, 85, 85, 'Well of Eternity', 0, 80),
(940, 15595, 288, 4, 5, 85, 85, 'Hour of Twilight', 0, 80),
-- Raids
(669, 15595, 260, 1, 25, 85, 0, 'Blackwing Descent', 0, 80),
(671, 15595, 266, 1, 25, 85, 0, 'The Bastion of Twilight', 0, 80),
(720, 15595, 275, 1, 25, 85, 0, 'Firelands', 0, 80),
(754, 15595, 271, 1, 25, 85, 0, 'Throne of the Four Winds', 0, 80),
(757, 15595, 269, 1, 25, 85, 0, 'Baradin Hold', 0, 80),
(967, 15595, 279, 1, 25, 85, 0, 'Dragon Soul', 0, 80);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('184', '20260905-00_cata_worldmap_info');
