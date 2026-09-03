-- Dungeon Journal / LFG metadata for the 9 Mists of Pandaria dungeons and 5 raids registered
-- in src/scripts/InstanceScripts/Mop/. Entrance coordinates (used as the repop point when
-- releasing spirit inside the instance) are taken from each instance's own real entrance
-- teleport target, cross-checked against the Mop map ids used by the instance script stubs.
-- repopentry is intentionally left at 0 - it is parsed into WorldMapInfo but never read
-- anywhere in AscEmu, so no value there has any effect.

INSERT INTO `worldmap_info`
(`entry`, `build`, `screenid`, `type`, `maxplayers`, `minlevel`, `minlevel_heroic`, `repopx`, `repopy`, `repopz`, `repopentry`, `area_name`, `flags`, `cooldown`, `lvl_mod_a`, `required_quest_A`, `required_quest_H`, `required_item`, `heroic_keyid_1`, `heroic_keyid_2`, `viewingDistance`, `required_checkpoint`)
VALUES
-- Dungeons
(960, 18414, 0, 4, 5, 90, 90, 953.37, -2487.5, 180.431, 0, 'Temple of the Jade Serpent', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
(961, 18414, 0, 4, 5, 90, 90, -732.115, 1266.13, 116.108, 0, 'Stormstout Brewery', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
(959, 18414, 0, 4, 5, 90, 90, 3657.29, 2551.92, 766.966, 0, 'Shado-Pan Monastery', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
(1011, 18414, 0, 4, 5, 90, 90, 1455.33, 5100.01, 149.415, 0, 'Siege of Niuzao Temple', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
(994, 18414, 0, 4, 5, 90, 90, -3969.67, -2542.71, 26.7537, 0, "Mogu'shan Palace", 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
(962, 18414, 0, 4, 5, 90, 90, 722.097, 2108.08, 402.978, 0, 'Gate of the Setting Sun', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
(1001, 18414, 0, 4, 5, 90, 90, 820.743, 607.812, 13.6389, 0, 'Scarlet Halls', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
(1004, 18414, 0, 4, 5, 90, 90, 1124.64, 512.467, 0.989549, 0, 'Scarlet Monastery', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
(1007, 18414, 0, 4, 5, 90, 90, 199.876, 125.346, 138.43, 0, 'Scholomance', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
-- Raids
(1008, 18414, 0, 1, 25, 90, 0, 3861.55, 1045.11, 490.17, 0, "Mogu'shan Vaults", 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
(1009, 18414, 0, 1, 25, 90, 0, -2378.92, 459.879, 422.441, 0, 'Heart of Fear', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
(996, 18414, 0, 1, 25, 90, 0, -1020.99, -3145.79, 28.3346, 0, 'Terrace of Endless Spring', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
(1098, 18414, 0, 1, 25, 90, 0, 5892.45, 6610.35, 106.108, 0, 'Throne of Thunder', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
(1136, 18414, 0, 1, 25, 90, 0, 1440.89, 263.047, 283.558, 0, 'Siege of Orgrimmar', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('168', '20260903-00_mop_worldmap_info');
