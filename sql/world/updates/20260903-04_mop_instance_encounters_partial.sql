-- instance_encounters for Mists of Pandaria - only for bosses whose creature_properties row
-- actually exists at build=18414 (the Mop build this project targets). Boss order/names come
-- from Wowhead/Warcraft Wiki; roughly two thirds of the ~70 total Mop dungeon/raid bosses have
-- no build=18414 creature template at all yet (some only exist under an old Classic/Cata build
-- number reused for the same character name, which would be the wrong creature to credit) -
-- those are intentionally left out rather than guessed or given placeholder creatures. Coverage
-- by instance: Temple of the Jade Serpent and Shado-Pan Monastery are complete; Terrace of
-- Endless Spring and Siege of Orgrimmar are missing a few; Siege of Niuzao Temple, Mogu'shan
-- Palace, Scarlet Halls, Scarlet Monastery, Scholomance, Mogu'shan Vaults, Heart of Fear and
-- Throne of Thunder have no usable creature data at all yet and are entirely absent here.
-- Where a boss has several build=18414 creature template variants (different raid/challenge
-- mode difficulties), the lowest entry id was picked as a single representative.

INSERT INTO `instance_encounters`
(`entry`, `creditType`, `creditEntry`, `lastEncounterDungeon`, `comment`, `mapid`)
VALUES
-- Temple of the Jade Serpent (960) - complete
(19001, 0, 56448, 0, 'Wise Mari', 960),
(19002, 0, 56843, 0, 'Lorewalker Stonestep', 960),
(19003, 0, 56732, 0, 'Liu Flameheart', 960),
(19004, 0, 56439, 0, 'Sha of Doubt', 960),
-- Stormstout Brewery (961) - missing Yan-Zhu the Uncasked
(19005, 0, 56637, 0, 'Ook-Ook', 961),
(19006, 0, 56717, 0, 'Hoptallus', 961),
-- Shado-Pan Monastery (959) - complete
(19007, 0, 56747, 0, 'Gu Cloudstrike', 959),
(19008, 0, 56541, 0, 'Master Snowdrift', 959),
(19009, 0, 56719, 0, 'Sha of Violence', 959),
(19010, 0, 56884, 0, 'Taran Zhu', 959),
-- Gate of the Setting Sun (962) - missing Striker Ga'dok, Raigonn
(19011, 0, 56906, 0, "Saboteur Kip'tilak", 962),
(19012, 0, 56636, 0, "Commander Ri'mok", 962),
-- Terrace of Endless Spring (996) - missing Protectors of the Endless
(19013, 0, 62442, 0, 'Tsulong', 996),
(19014, 0, 62983, 0, 'Lei Shi', 996),
(19015, 0, 60999, 0, 'Sha of Fear', 996),
-- Siege of Orgrimmar (1136) - missing Fallen Protectors, Spoils of Pandaria, Paragons of the Klaxxi
(19016, 0, 71543, 0, 'Immerseus', 1136),
(19017, 0, 71965, 0, 'Norushen', 1136),
(19018, 0, 71734, 0, 'Sha of Pride', 1136),
(19019, 0, 72249, 0, 'Galakras', 1136),
(19020, 0, 71466, 0, 'Iron Juggernaut', 1136),
(19021, 0, 72451, 0, "Kor'kron Dark Shaman", 1136),
(19022, 0, 54870, 0, 'General Nazgrim', 1136),
(19023, 0, 71454, 0, 'Malkorok', 1136),
(19024, 0, 71529, 0, 'Thok the Bloodthirsty', 1136),
(19025, 0, 71504, 0, 'Siegecrafter Blackfuse', 1136),
(19026, 0, 71865, 0, 'Garrosh Hellscream', 1136);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('172', '20260903-04_mop_instance_encounters_partial');
