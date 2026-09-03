-- GM recall/teleport locations for Mists of Pandaria (map 870, the Pandaria continent) and
-- the handful of new Cataclysm locations not already covered by existing recall rows
-- (Deepholm, Vashj'ir/Maelstrom, Tol Barad). Excludes anything on a pre-existing map id
-- (e.g. old-world Blackrock Depths/Molten Core/Maraudon teleports already present).

INSERT INTO `recall`
(`min_build`, `max_build`, `name`, `MapId`, `positionX`, `positionY`, `positionZ`, `Orientation`)
VALUES
-- Mists of Pandaria (map 870)
(18414, 18414, 'Shrine of Two Moons', 870, 1665.49, 929.159, 471.178, 3.33285),
(18414, 18414, 'Shrine of Seven Stars', 870, 821.489, 253.009, 503.92, 0.598881),
-- orientation in the reference row was out of the valid 0-2pi range (likely a source data
-- error) - left at 0 rather than inventing a value
(18414, 18414, 'Temple of the Jade Serpent', 870, 960.715, -2463.43, 471.178, 0),
(18414, 18414, 'Isle of Giants', 870, 6031.41, 1117.96, 55.935, 3.33285),
(18414, 18414, 'Timeless Isle', 870, -698.285, -5040.82, -6.27675, 4.58871),
(18414, 18414, 'Shado-Pan Monastery', 870, 3599.62, 2548.4, 768.69, 5.87886),
(18414, 18414, 'Heart of Fear', 870, 171.724, 4039.32, 255.915, 1.77186),
(18414, 18414, 'Siege of Niuzao Temple', 870, 1339.8, 4977.48, 123.78, 1.02457),
(18414, 18414, 'Kun-Lai Summit', 870, 3817.37, 1794.45, 950.351, 5.02719),
(18414, 18414, "Mogu'shan Vaults", 870, 4011.61, 1077.24, 501.029, 2.34062),
(18414, 18414, 'Black Market Auction House', 870, 800.708, -343.901, 400.784, 2.22925),
(18414, 18414, 'Siege of Orgrimmar', 870, 1219.81, 625.647, 335.074, 5.44086),
(18414, 18414, "Mogu'shan Palace", 870, 1382.32, 447.216, 478.939, 5.50291),
(18414, 18414, 'Terrace of Endless Spring', 870, 951.789, -60.7127, 508.539, 0.870627),
(18414, 18414, 'Gate of the Setting Sun', 870, 668.318, 2079.53, 371.402, 0.0278959),
(18414, 18414, 'Stormstout Brewery', 870, -707.816, 1266.88, 136.024, 3.30319),
(18414, 18414, 'Pandaria', 870, 3001.38, -542.47, 248.18, 5.1),
(18414, 18414, 'Peak of Serenity', 870, 3826.87, 1795.74, 950.352, 0.299085),
-- Cataclysm
(15595, 15595, 'Deepholm', 646, 915.004, 503.842, -49.23, 0.196921),
(15595, 15595, 'The Stonecore Entrance', 646, 1023.72, 644.747, 156.671, 4.9),
(15595, 15595, 'The Maelstrom', 730, 851.307, 1067.76, -10.0183, 4.50656),
(15595, 15595, 'Tol Barad Peninsula', 732, -281.595, 1362.91, 22.7936, 1.84159),
(15595, 15595, 'Baradin Hold Entrance', 732, -1208.18, 980.94, 119.728, 3.21604);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('169', '20260903-01_mop_cata_recall');
