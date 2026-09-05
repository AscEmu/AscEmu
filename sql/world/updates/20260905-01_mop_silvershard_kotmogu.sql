
-- worldmap_info rows for Silvershard Mines (727) and Temple of Kotmogu (998), the two remaining
-- Mop battlegrounds - placeholder implementations, registered and enterable with real starting
-- coordinates, no scoring/win mechanics yet. type=3 matches every other battleground row already
-- in this table, screenid=0 matches the established convention every other Mop-content row uses,
-- and minlevel=90 matches Mop's own level-cap convention already used for the Mop dungeon/raid rows.
INSERT INTO `worldmap_info`
(`entry`, `build`, `screenid`, `type`, `maxplayers`, `minlevel`, `minlevel_heroic`, `area_name`, `flags`, `viewingDistance`)
VALUES
(727, 18414, 0, 3, 20, 90, 0, 'Silvershard Mines', 1, 500),
(998, 18414, 0, 3, 20, 90, 0, 'Temple of Kotmogu', 1, 500);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('185', '20260905-01_mop_silvershard_kotmogu');
