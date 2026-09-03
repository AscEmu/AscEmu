-- Companion SQL for the Setup.h Cata map id fix: those constants only affect where the C++
-- instance scripts register, they don't touch any DB row, so there's nothing to migrate for
-- the fix itself. This file adds the Dragon Soul (now correctly map 967) encounter list,
-- cross-checked against Wowhead for boss order/names and two independent emulator reference
-- databases for the real creature/spell credit ids - every creature entry already exists in
-- our own creature_properties.

INSERT INTO `instance_encounters`
(`entry`, `creditType`, `creditEntry`, `lastEncounterDungeon`, `comment`, `mapid`)
VALUES
(1292, 0, 55265, 0, 'Morchok', 967),
(1294, 0, 55308, 0, "Warlord Zon'ozz", 967),
(1295, 0, 55312, 0, "Yor'sahj the Unsleeping", 967),
(1296, 0, 55689, 0, 'Hagara the Stormbinder', 967),
(1297, 0, 55294, 0, 'Ultraxion', 967),
(1298, 0, 56427, 0, 'Warmaster Blackhorn', 967),
-- Spine of Deathwing credits via a spell cast rather than a creature kill - the same
-- encounter covers the Madness of Deathwing transition that immediately follows it.
(1291, 1, 104574, 0, 'Spine of Deathwing / Madness of Deathwing', 967);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('170', '20260903-02_cata_map_id_fix_and_dragonsoul_encounters');
