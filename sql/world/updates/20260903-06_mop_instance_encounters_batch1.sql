-- instance_encounters entries for the 31 boss NPCs added in the previous update. Completes
-- Stormstout Brewery, Siege of Niuzao Temple, Mogu'shan Palace, Gate of the Setting Sun,
-- Scarlet Halls, Scarlet Monastery, Scholomance and Heart of Fear; partially covers Mogu'shan
-- Vaults (3/6 - missing the three multi-member council fights) and Throne of Thunder (1/13).

INSERT INTO `instance_encounters`
(`entry`, `creditType`, `creditEntry`, `lastEncounterDungeon`, `comment`, `mapid`)
VALUES
-- Stormstout Brewery (961) - now complete
(19027, 0, 59479, 0, 'Yan-Zhu the Uncasked', 961),
-- Siege of Niuzao Temple (1011) - complete
(19028, 0, 61567, 0, "Vizier Jin'bak", 1011),
(19029, 0, 61634, 0, "Commander Vo'jak", 1011),
(19030, 0, 61485, 0, "General Pa'valak", 1011),
(19031, 0, 62205, 0, "Wing Leader Ner'onok", 1011),
-- Mogu'shan Palace (994) - complete
(19032, 0, 61445, 0, 'Trial of the King (Haiyan the Unstoppable)', 994),
(19033, 0, 61243, 0, 'Gekkan', 994),
(19034, 0, 61398, 0, 'Xin the Weaponmaster', 994),
-- Gate of the Setting Sun (962) - now complete
(19035, 0, 56589, 0, "Striker Ga'dok", 962),
(19036, 0, 56877, 0, 'Raigonn', 962),
-- Scarlet Halls (1001) - complete
(19037, 0, 59303, 0, 'Houndmaster Braun', 1001),
(19038, 0, 58632, 0, 'Armsmaster Harlan', 1001),
(19039, 0, 59150, 0, 'Flameweaver Koegler', 1001),
-- Scarlet Monastery, Mists revamp (1004) - complete
(19040, 0, 59789, 0, 'Thalnos the Soulrender', 1004),
(19041, 0, 59223, 0, 'Brother Korloff', 1004),
(19042, 0, 3977, 0, 'High Inquisitor Whitemane', 1004),
-- Scholomance, Mists revamp (1007) - complete
(19043, 0, 58633, 0, 'Instructor Chillheart', 1007),
(19044, 0, 10503, 0, 'Jandice Barov', 1007),
(19045, 0, 11622, 0, 'Rattlegore', 1007),
(19046, 0, 38895, 0, 'Lilian Voss', 1007),
(19047, 0, 1853, 0, 'Darkmaster Gandling', 1007),
-- Mogu'shan Vaults (1008) - missing The Stone Guard, The Spirit Kings, Will of the Emperor (council fights)
(19048, 0, 60016, 0, 'Feng the Accursed', 1008),
(19049, 0, 60257, 0, "Gara'jal the Spiritbinder", 1008),
(19050, 0, 60410, 0, 'Elegon', 1008),
-- Heart of Fear (1009) - complete
(19051, 0, 62980, 0, "Imperial Vizier Zor'lok", 1009),
(19052, 0, 62543, 0, "Blade Lord Ta'yak", 1009),
(19053, 0, 62164, 0, 'Garalon', 1009),
(19054, 0, 62397, 0, "Wind Lord Mel'jarak", 1009),
(19055, 0, 62511, 0, "Amber-Shaper Un'sok", 1009),
(19056, 0, 62837, 0, "Grand Empress Shek'zeer", 1009),
-- Throne of Thunder (1098) - only Jin'rokh the Breaker so far, 12 bosses still missing
(19057, 0, 14902, 0, "Jin'rokh the Breaker", 1098);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('174', '20260903-06_mop_instance_encounters_batch1');
