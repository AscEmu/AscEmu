-- Throne of Thunder boss NPCs (12 encounters + the heroic-only Ra-den bonus boss). Unlike the
-- earlier Mop boss batches, none of this raid exists in the Mop reference database at all
-- (that reference predates this raid's release), so these came directly from Wowhead: names and
-- entry/npc ids are real and verified there, but display model, faction, and creature type
-- could not be retrieved (Wowhead loads that data dynamically, a plain page fetch doesn't see
-- it) - male_displayid is left at 0 (renders as the default placeholder model) rather than
-- guessing a real-looking but wrong appearance. Health/mana/damage follow the same stub
-- convention as the rest of this Mop boss import (minhealth/maxhealth 20/30, mana 0).

INSERT INTO `creature_properties`
(`entry`, `build`, `male_displayid`, `female_displayid`, `name`, `subname`, `type_flags`, `type`, `family`, `rank`,
 `base_attack_mod`, `range_attack_mod`, `minlevel`, `maxlevel`, `faction`, `minhealth`, `maxhealth`, `mana`, `scale`,
 `attacktime`, `mindamage`, `maxdamage`, `respawntime`, `combat_reach`, `bounding_radius`, `auras`, `boss`, `walk_speed`, `run_speed`, `fly_speed`)
VALUES
(68476, 18414, 0, 0, 'Horridon', '', 2097260, 1, 0, 3, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
-- Council of Elders
(69131, 18414, 0, 0, 'Frost King Malakk', '', 2097260, 7, 0, 1, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(69134, 18414, 0, 0, "Kazra'jin", '', 2097260, 7, 0, 1, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(69078, 18414, 0, 0, 'Sul the Sandcrawler', '', 2097260, 7, 0, 1, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(69132, 18414, 0, 0, "High Priestess Mar'li", '', 2097260, 7, 0, 1, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(67977, 18414, 0, 0, 'Tortos', '', 2097260, 1, 0, 3, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(68065, 18414, 0, 0, 'Megaera', '', 2097260, 2, 0, 3, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(69712, 18414, 0, 0, 'Ji-Kun', '', 2097260, 1, 0, 3, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(68036, 18414, 0, 0, 'Durumu the Forgotten', '', 2097260, 7, 0, 3, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(69017, 18414, 0, 0, 'Primordius', '', 2097260, 4, 0, 3, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(69427, 18414, 0, 0, 'Dark Animus', '', 2097260, 7, 0, 3, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(68078, 18414, 0, 0, 'Iron Qon', '', 2097260, 7, 0, 3, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
-- Twin Empyreans / Twin Consorts
(68904, 18414, 0, 0, 'Suen', '', 2097260, 9, 0, 1, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(68905, 18414, 0, 0, "Lu'lin", '', 2097260, 9, 0, 1, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(68397, 18414, 0, 0, 'Lei Shen', '', 270532716, 7, 0, 3, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
-- Ra-den, heroic-only bonus boss
(69473, 18414, 0, 0, 'Ra-den', '', 2097260, 9, 0, 3, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('177', '20260903-09_mop_throne_of_thunder_bosses');
