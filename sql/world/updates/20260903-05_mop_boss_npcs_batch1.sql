-- Boss NPC templates for 31 of the Mop bosses that had no build=18414 creature_properties row
-- at all (some existing entries only had a stale Classic/Cata-build row under the same id -
-- reused character, e.g. Darkmaster Gandling/Jandice Barov/Rattlegore in the Mop Scholomance
-- revamp - those get a new (entry, 18414) row here alongside their old one, which is untouched).
-- Identity fields (model, subname, level, faction, type, rank) are transferred from a Mop
-- reference database's creature_template, which stores them directly the same way AscEmu does.
-- Health/mana/damage are NOT reliably transferable - the reference computes those at runtime
-- from a level/class formula AscEmu doesn't have, and even our own existing Mop boss rows are
-- inconsistent here (some real, many stub 20/30 health). Per instruction, these follow that same stub convention:
-- minhealth/maxhealth 20/30, mana 0, damage a low placeholder - all still need real tuning.
-- Multi-member "council" encounters (Stone Guard, Spirit Kings, Council of Elders, Twin
-- Empyreans, Paragons of the Klaxxi, etc.) are stored per-individual-member in the reference,
-- not under the encounter's collective name, and are intentionally left for a follow-up pass.

INSERT INTO `creature_properties`
(`entry`, `build`, `male_displayid`, `female_displayid`, `name`, `subname`, `type_flags`, `type`, `family`, `rank`,
 `base_attack_mod`, `range_attack_mod`, `minlevel`, `maxlevel`, `faction`, `minhealth`, `maxhealth`, `mana`, `scale`,
 `attacktime`, `mindamage`, `maxdamage`, `respawntime`, `combat_reach`, `bounding_radius`, `auras`, `boss`, `walk_speed`, `run_speed`, `fly_speed`)
VALUES
-- Temple of the Jade Serpent / Shado-Pan Monastery / Siege of Niuzao Temple / Gate of the Setting Sun / Scarlet Halls / Scarlet Monastery / Scholomance dungeon bosses
(59479, 18414, 42969, 42971, 'Yan-Zhu the Uncasked', '', 2149580904, 4, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(61567, 18414, 43197, 0, "Vizier Jin'bak", '', 2097256, 7, 0, 1, 3, 3, 92, 92, 14, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(61634, 18414, 42169, 0, "Commander Vo'jak", '', 2097224, 7, 0, 1, 3, 3, 92, 92, 16, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(61485, 18414, 43120, 0, 'General Pa''valak', '', 270532680, 7, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(62205, 18414, 43151, 0, "Wing Leader Ner'onok", '', 2147483752, 7, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(61445, 18414, 42060, 0, 'Haiyan the Unstoppable', '', 270532680, 7, 0, 1, 3, 3, 89, 89, 14, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(61243, 18414, 41920, 0, 'Gekkan', '', 2097224, 7, 0, 1, 3, 3, 89, 89, 16, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(61398, 18414, 41987, 0, 'Xin the Weaponmaster', '', 2149580872, 7, 0, 1, 3, 3, 89, 89, 14, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(56589, 18414, 43275, 0, "Striker Ga'dok", '', 2097224, 7, 0, 1, 3, 3, 92, 92, 14, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(56877, 18414, 39519, 0, 'Raigonn', '', 2147483720, 7, 0, 1, 3, 3, 92, 92, 1771, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(59303, 18414, 42264, 0, 'Houndmaster Braun', '', 2097224, 7, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(58632, 18414, 40293, 0, 'Armsmaster Harlan', 'The Ravager', 2097224, 7, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(59150, 18414, 40597, 0, 'Flameweaver Koegler', '', 2097224, 7, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(59789, 18414, 27705, 0, 'Thalnos the Soulrender', '', 2097224, 6, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(59223, 18414, 41154, 0, 'Brother Korloff', '', 2097224, 7, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(3977, 18414, 2043, 0, 'High Inquisitor Whitemane', '', 0, 7, 0, 1, 3, 3, 90, 90, 67, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(58633, 18414, 40301, 0, 'Instructor Chillheart', '', 2097224, 6, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(10503, 18414, 11073, 0, 'Jandice Barov', '', 72, 6, 0, 1, 3, 3, 90, 90, 233, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(11622, 18414, 12073, 0, 'Rattlegore', '', 72, 6, 0, 1, 3, 3, 90, 90, 233, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(38895, 18414, 31252, 0, 'Lilian Voss', '', 0, 7, 0, 0, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(1853, 18414, 11070, 0, 'Darkmaster Gandling', '', 72, 7, 0, 1, 3, 3, 90, 90, 21, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
-- Mogu'shan Vaults / Heart of Fear / Throne of Thunder single-entity raid bosses
(62511, 18414, 43126, 0, "Amber-Shaper Un'sok", '', 2097260, 7, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(62543, 18414, 43141, 0, "Blade Lord Ta'yak", '', 2097260, 7, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(60410, 18414, 41399, 0, 'Elegon', '', 2097260, 9, 0, 1, 3, 3, 93, 93, 14, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(60016, 18414, 41192, 0, 'Feng the Accursed', 'Keeper of Champion Spirits', 108, 6, 0, 3, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(60257, 18414, 41496, 0, "Gara'jal the Spiritbinder", '', 0, 7, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(62164, 18414, 42368, 0, 'Garalon', '', 2097260, 1, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(62837, 18414, 42730, 0, 'Grand Empress Shek''zeer', '', 108, 7, 0, 3, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(62980, 18414, 42807, 0, 'Imperial Vizier Zor''lok', 'Voice of the Empress', 270532716, 7, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(14902, 18414, 14994, 0, "Jin'rokh the Breaker", '', 0, 7, 0, 1, 3, 3, 90, 90, 1574, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(62397, 18414, 42645, 0, "Wind Lord Mel'jarak", '', 2097260, 7, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('173', '20260903-05_mop_boss_npcs_batch1');
