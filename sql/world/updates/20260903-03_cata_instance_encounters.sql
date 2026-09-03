-- instance_encounters for the remaining 12 Cata dungeons and 4 Cata raids (Dragon Soul was
-- already added separately; Zul'Aman needs nothing here - it shares its map with the Tbc raid,
-- which already has all 5 of its (still valid) boss rows registered on map 568).
-- Boss order/names cross-checked against Wowhead; creditType/creditEntry values taken from a
-- Cata emulator reference database and verified against our own creature_properties (every
-- creature-type credit below already exists there) - Murozond (End Time's final boss) was the
-- only one missing from that reference, its npc id (54432, already in creature_properties) was
-- looked up directly and it's given a fresh entry id (19000) since the reference has none.
-- Map ids match the corrected Setup.h constants from the previous update in this series.

INSERT INTO `instance_encounters`
(`entry`, `creditType`, `creditEntry`, `lastEncounterDungeon`, `comment`, `mapid`)
VALUES
-- Blackrock Caverns (645)
(1040, 0, 39665, 0, "Rom'ogg Bonecrusher", 645),
(1038, 0, 39679, 0, 'Corla, Herald of Twilight', 645),
(1039, 0, 39698, 0, 'Karsh Steelbender', 645),
(1036, 0, 39705, 0, 'Ascendant Lord Obsidius', 645),
-- Grim Batol (670)
(1051, 0, 39625, 0, 'General Umbriss', 670),
(1050, 0, 40177, 0, 'Forgemaster Throngus', 670),
(1048, 0, 40319, 0, 'Drahga Shadowburner', 670),
(1049, 0, 40484, 0, 'Erudax', 670),
-- Halls of Origination (644)
(1080, 0, 39425, 0, 'Temple Guardian Anhuur', 644),
(1076, 0, 39428, 0, 'Earthrager Ptah', 644),
(1075, 0, 39788, 0, 'Anraphet', 644),
(1077, 0, 39587, 0, 'Isiset', 644),
(1078, 0, 39378, 0, 'Rajh', 644),
(1079, 0, 39732, 0, 'Setesh', 644),
-- Throne of the Tides (643)
(1045, 0, 40586, 0, "Lady Naz'jar", 643),
(1044, 0, 40765, 0, 'Commander Ulthok', 643),
(1046, 0, 40788, 0, "Mindbender Ghur'sha", 643),
(1047, 0, 42172, 0, 'Ozumat', 643),
-- The Stonecore (725)
(1056, 0, 43438, 0, 'Corborus', 725),
(1057, 0, 42333, 0, 'High Priestess Azil', 725),
(1058, 0, 42188, 0, 'Ozruk', 725),
(1059, 0, 43214, 0, 'Slabhide', 725),
-- The Vortex Pinnacle (657)
(1041, 0, 43873, 0, 'Altairus', 657),
(1042, 0, 43875, 0, 'Asaad', 657),
(1043, 0, 43878, 0, 'Grand Vizier Ertan', 657),
-- Lost City of the Tol'vir (755)
(1053, 0, 43612, 0, 'High Prophet Barim', 755),
(1054, 0, 43614, 0, 'Lockmaw', 755),
(1055, 0, 44819, 0, 'Siamat', 755),
-- Zul'Gurub, Cataclysm 4.1 revamp (859) - Renataki/Wushoolay/Gri'lek/Hazza'rah share one
-- generic credit slot (Cache of Madness), matching how only 1-2 of those 4 appear per run
(1178, 0, 52155, 0, 'High Priest Venoxis', 859),
(1180, 0, 52059, 0, 'High Priestess Kilnara', 859),
(1188, 0, 52271, 0, 'Cache of Madness', 859),
(1179, 0, 52151, 0, 'Bloodlord Mandokir', 859),
(1181, 0, 52053, 0, 'Zanzil the Outcast', 859),
(1182, 0, 52148, 0, "Jin'do the Godbreaker", 859),
-- End Time (938) - two of four random "Echo" mini-bosses appear per run, both credited via
-- the same shared spell-cast trigger rather than per-specific-boss creature kills
(1269, 1, 72959, 0, 'First Echo', 938),
(1268, 1, 72959, 0, 'Second Echo', 938),
(19000, 0, 54432, 0, 'Murozond', 938),
-- Hour of Twilight (940)
(1337, 0, 54590, 0, 'Arcurion', 940),
(1340, 0, 54968, 0, 'Asira Dawnslayer', 940),
(1339, 0, 54938, 0, 'Archbishop Benedictus', 940),
-- Baradin Hold (757)
(1033, 0, 47120, 0, 'Argaloth', 757),
(1250, 0, 52363, 0, "Occu'thar", 757),
(1332, 0, 55869, 0, 'Alizabal', 757),
-- Bastion of Twilight (671) - Sinestra has separate 10-/25-man creature templates, both kept
(1030, 0, 44600, 0, 'Halfus Wyrmbreaker', 671),
(1032, 0, 45992, 0, 'Theralion and Valiona', 671),
(1028, 0, 43735, 0, 'Ascendant Council', 671),
(1029, 0, 43324, 0, "Cho'gall", 671),
(1082, 0, 45213, 0, 'Sinestra', 671),
(1083, 0, 49744, 0, 'Sinestra (alternate raid size)', 671),
-- Blackwing Descent (669)
(1024, 0, 41570, 0, 'Magmaw', 669),
(1027, 1, 42180, 0, 'Omnotron Defense System', 669),
(1025, 0, 41378, 0, 'Maloriak', 669),
(1022, 0, 41442, 0, 'Atramedes', 669),
(1023, 0, 43296, 0, 'Chimaeron', 669),
(1026, 0, 41376, 0, "Nefarian's End", 669),
-- Firelands (720)
(1197, 0, 52498, 0, "Beth'tilac", 720),
(1204, 0, 52558, 0, 'Lord Rhyolith', 720),
(1206, 0, 52530, 0, 'Alysrazor', 720),
(1205, 0, 53691, 0, 'Shannox', 720),
(1200, 0, 53494, 0, 'Baleroc', 720),
(1185, 0, 52571, 0, 'Majordomo Staghelm', 720),
(1203, 0, 52409, 0, 'Ragnaros', 720);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('171', '20260903-03_cata_instance_encounters');
