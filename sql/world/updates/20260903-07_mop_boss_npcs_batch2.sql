-- Second batch of Mop boss NPCs - covers the multi-member "council" encounters, whose
-- individual members are stored under their own names rather than the encounter's collective
-- name. 5 of the ~25 candidates here (Protector Kaolan, Elder Regail, Elder Asani, Korven the
-- Prime, Secured Stockpile of Pandaren Spoils) already had a build=18414 row and are left
-- untouched. Some reference names came back in French in this snapshot (Zian des Ombres /
-- Meng le D.../ Qiang l...) - renamed to their real English names here, ids unchanged.
-- Still missing after this: Subetai the Swift (4th Spirit King), all of Council of Elders
-- and Twin Empyreans, and every Throne of Thunder boss past Jin'rokh - none of those exist in
-- this reference at all (that raid likely postdates this snapshot).

INSERT INTO `creature_properties`
(`entry`, `build`, `male_displayid`, `female_displayid`, `name`, `subname`, `type_flags`, `type`, `family`, `rank`,
 `base_attack_mod`, `range_attack_mod`, `minlevel`, `maxlevel`, `faction`, `minhealth`, `maxhealth`, `mana`, `scale`,
 `attacktime`, `mindamage`, `maxdamage`, `respawntime`, `combat_reach`, `bounding_radius`, `auras`, `boss`, `walk_speed`, `run_speed`, `fly_speed`)
VALUES
-- The Stone Guard, Mogu'shan Vaults (1008)
(60047, 18414, 41892, 0, 'Amethyst Guardian', '', 4, 1, 0, 3, 3, 3, 93, 93, 16, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(60051, 18414, 41893, 0, 'Cobalt Guardian', '', 4, 1, 0, 3, 3, 3, 93, 93, 16, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(60043, 18414, 41894, 0, 'Jade Guardian', '', 4, 1, 0, 3, 3, 3, 93, 93, 16, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(59915, 18414, 42644, 0, 'Jasper Guardian', '', 4, 1, 0, 3, 3, 3, 93, 93, 16, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
-- The Spirit Kings, Mogu'shan Vaults (1008) - Subetai the Swift still missing
(60701, 18414, 41566, 0, 'Zian of the Endless Shadow', '', 2097260, 10, 0, 1, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(60709, 18414, 41812, 0, 'Qiang the Merciless', '', 270532716, 10, 0, 1, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(60708, 18414, 41813, 0, 'Meng the Demented', '', 2097260, 10, 0, 1, 3, 3, 93, 93, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
-- Will of the Emperor, Mogu'shan Vaults (1008)
(60399, 18414, 41391, 0, 'Qin-xi', '', 2097260, 7, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(60400, 18414, 41392, 0, 'Jan-xi', '', 2097260, 7, 0, 1, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
-- Paragons of the Klaxxi, Siege of Orgrimmar (1136) - Korven the Prime already existed
(62152, 18414, 42539, 0, "Kil'ruk the Wind-Reaver", '', 0, 7, 0, 0, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(62540, 18414, 44355, 0, "Kaz'tik the Manipulator", '', 0, 7, 0, 0, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(63071, 18414, 43894, 0, 'Skeer the Bloodseeker', '', 262144, 7, 0, 0, 3, 3, 90, 90, 7, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(62773, 18414, 42800, 0, 'Iyyokuk the Lucid', '', 0, 7, 0, 0, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(64210, 18414, 11686, 0, "Ka'roz the Locust", '', 1611661328, 7, 0, 0, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(63072, 18414, 43825, 0, "Rik'kal the Dissector", '', 0, 7, 0, 0, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(63785, 18414, 43224, 0, 'Hisek the Swarmkeeper', '', 0, 7, 0, 0, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(62151, 18414, 43893, 0, 'Xaril the Poisoned Mind', '', 0, 7, 0, 0, 3, 3, 90, 90, 35, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
-- The Fallen Protectors, Siege of Orgrimmar (1136)
(58466, 18414, 40177, 0, 'Rook Stonetoe', '', 266240, 7, 0, 0, 3, 3, 90, 90, 2481, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(58470, 18414, 40181, 0, 'He Softfoot', '', 266240, 7, 0, 0, 3, 3, 90, 90, 2481, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14),
(58468, 18414, 42711, 0, 'Sun Tenderheart', '', 266240, 7, 0, 0, 3, 3, 90, 90, 2481, 20, 30, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('175', '20260903-07_mop_boss_npcs_batch2');
