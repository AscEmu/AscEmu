-- instance_encounters for the 16 Throne of Thunder bosses added in the previous update.
-- Completes Throne of Thunder (13/13 encounters, including the heroic-only Ra-den bonus boss),
-- which brings every Mop dungeon/raid instance_encounters set to either complete or limited
-- only by what plain doesn't exist in any available reference (Subetai the Swift, the fourth
-- Spirit King in Mogu'shan Vaults).

INSERT INTO `instance_encounters`
(`entry`, `creditType`, `creditEntry`, `lastEncounterDungeon`, `comment`, `mapid`)
VALUES
(19083, 0, 68476, 0, 'Horridon', 1098),
-- Council of Elders
(19084, 0, 69131, 0, 'Frost King Malakk (Council of Elders)', 1098),
(19085, 0, 69134, 0, "Kazra'jin (Council of Elders)", 1098),
(19086, 0, 69078, 0, 'Sul the Sandcrawler (Council of Elders)', 1098),
(19087, 0, 69132, 0, "High Priestess Mar'li (Council of Elders)", 1098),
(19088, 0, 67977, 0, 'Tortos', 1098),
(19089, 0, 68065, 0, 'Megaera', 1098),
(19090, 0, 69712, 0, 'Ji-Kun', 1098),
(19091, 0, 68036, 0, 'Durumu the Forgotten', 1098),
(19092, 0, 69017, 0, 'Primordius', 1098),
(19093, 0, 69427, 0, 'Dark Animus', 1098),
(19094, 0, 68078, 0, 'Iron Qon', 1098),
-- Twin Empyreans / Twin Consorts
(19095, 0, 68904, 0, 'Suen (Twin Empyreans)', 1098),
(19096, 0, 68905, 0, "Lu'lin (Twin Empyreans)", 1098),
(19097, 0, 68397, 0, 'Lei Shen', 1098),
-- Ra-den, heroic-only bonus boss after Lei Shen
(19098, 0, 69473, 0, 'Ra-den', 1098);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('178', '20260903-10_mop_throne_of_thunder_encounters');
