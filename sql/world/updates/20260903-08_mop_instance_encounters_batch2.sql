-- instance_encounters entries for the council-fight members added in the previous update
-- (plus the 5 that already existed: Protector Kaolan, Elder Regail, Elder Asani, Korven the
-- Prime, Secured Stockpile of Pandaren Spoils). Completes Terrace of Endless Spring and Siege
-- of Orgrimmar; brings Mogu'shan Vaults to 5/6 (still missing Subetai the Swift, the fourth
-- Spirit King, who isn't in the reference at all).

INSERT INTO `instance_encounters`
(`entry`, `creditType`, `creditEntry`, `lastEncounterDungeon`, `comment`, `mapid`)
VALUES
-- The Stone Guard, Mogu'shan Vaults (1008)
(19058, 0, 60047, 0, 'Amethyst Guardian (Stone Guard)', 1008),
(19059, 0, 60051, 0, 'Cobalt Guardian (Stone Guard)', 1008),
(19060, 0, 60043, 0, 'Jade Guardian (Stone Guard)', 1008),
(19061, 0, 59915, 0, 'Jasper Guardian (Stone Guard)', 1008),
-- The Spirit Kings, Mogu'shan Vaults (1008) - Subetai the Swift not available in the reference
(19062, 0, 60701, 0, 'Zian of the Endless Shadow (Spirit Kings)', 1008),
(19063, 0, 60709, 0, 'Qiang the Merciless (Spirit Kings)', 1008),
(19064, 0, 60708, 0, 'Meng the Demented (Spirit Kings)', 1008),
-- Will of the Emperor, Mogu'shan Vaults (1008)
(19065, 0, 60399, 0, 'Qin-xi (Will of the Emperor)', 1008),
(19066, 0, 60400, 0, 'Jan-xi (Will of the Emperor)', 1008),
-- Protectors of the Endless, Terrace of Endless Spring (996) - now complete
(19067, 0, 60583, 0, 'Protector Kaolan (Protectors of the Endless)', 996),
(19068, 0, 60585, 0, 'Elder Regail (Protectors of the Endless)', 996),
(19069, 0, 60586, 0, 'Elder Asani (Protectors of the Endless)', 996),
-- Paragons of the Klaxxi, Siege of Orgrimmar (1136)
(19070, 0, 62152, 0, "Kil'ruk the Wind-Reaver (Paragons of the Klaxxi)", 1136),
(19071, 0, 62540, 0, "Kaz'tik the Manipulator (Paragons of the Klaxxi)", 1136),
(19072, 0, 63071, 0, 'Skeer the Bloodseeker (Paragons of the Klaxxi)', 1136),
(19073, 0, 62180, 0, 'Korven the Prime (Paragons of the Klaxxi)', 1136),
(19074, 0, 62773, 0, 'Iyyokuk the Lucid (Paragons of the Klaxxi)', 1136),
(19075, 0, 64210, 0, "Ka'roz the Locust (Paragons of the Klaxxi)", 1136),
(19076, 0, 63072, 0, "Rik'kal the Dissector (Paragons of the Klaxxi)", 1136),
(19077, 0, 63785, 0, 'Hisek the Swarmkeeper (Paragons of the Klaxxi)', 1136),
(19078, 0, 62151, 0, 'Xaril the Poisoned Mind (Paragons of the Klaxxi)', 1136),
-- The Fallen Protectors, Siege of Orgrimmar (1136)
(19079, 0, 58466, 0, 'Rook Stonetoe (Fallen Protectors)', 1136),
(19080, 0, 58470, 0, 'He Softfoot (Fallen Protectors)', 1136),
(19081, 0, 58468, 0, 'Sun Tenderheart (Fallen Protectors)', 1136),
-- Spoils of Pandaria, Siege of Orgrimmar (1136)
(19082, 0, 71889, 0, 'Spoils of Pandaria', 1136);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('176', '20260903-08_mop_instance_encounters_batch2');
