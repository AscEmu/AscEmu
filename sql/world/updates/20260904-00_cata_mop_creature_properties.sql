
INSERT INTO `creature_properties`
(`entry`, `build`, `male_displayid`, `female_displayid`, `name`, `subname`, `type_flags`, `type`, `family`, `rank`,
 `base_attack_mod`, `range_attack_mod`, `minlevel`, `maxlevel`, `faction`, `minhealth`, `maxhealth`, `mana`, `scale`,
 `attacktime`, `mindamage`, `maxdamage`, `respawntime`, `combat_reach`, `bounding_radius`, `auras`, `boss`, `walk_speed`, `run_speed`, `fly_speed`, `armor`)
VALUES
-- Scarlet Halls / Scarlet Monastery Mop trash + boss
(58685, 18414, 40764, 40765, 'Scarlet Evangelist', '', 0, 7, 0, 1, 1, 3, 31, 93, 67, 915018, 915018, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 14392),
(59175, 18414, 40629, 43808, 'Master Archer', '', 0, 7, 0, 1, 1, 3, 31, 93, 2102, 152503, 152503, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 14392),
(58876, 18414, 30212, 0, 'Starving Hound', '', 0, 1, 52, 1, 1, 3, 31, 93, 16, 381258, 381258, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 14392),
(58605, 18414, 41282, 0, 'Scarlet Judicator', '', 0, 7, 0, 1, 1, 3, 34, 93, 67, 915018, 915018, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 14392),
(59705, 18414, 40725, 40724, 'Scarlet Flamethrower', '', 0, 7, 0, 1, 1, 3, 34, 93, 14, 915018, 915018, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 14392),
(60033, 18414, 24653, 24653, 'Frenzied Spirit', '', 0, 6, 0, 1, 1, 3, 34, 93, 1814, 305006, 305006, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 14392),
(60040, 18414, 41220, 0, 'Commander Durand', '', 2097224, 7, 0, 3, 1, 3, 34, 93, 67, 1830036, 1830036, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14, 14392),
-- Scholomance Mop bosses + trash
(58757, 18414, 40271, 40272, 'Scholomance Acolyte', '', 0, 7, 0, 1, 1, 3, 43, 93, 16, 1830036, 1830036, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 14392),
(59368, 18414, 40743, 0, 'Krastinovian Carver', '', 0, 7, 0, 1, 1, 3, 43, 93, 21, 1067521, 1067521, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 14392),
(59467, 18414, 40824, 0, 'Candlestick Mage', 'Illusionist Apprentice', 0, 7, 0, 1, 1, 3, 43, 93, 16, 1067521, 1067521, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 14392),
(58722, 18414, 40256, 0, 'Lilian Voss', '', 2097224, 7, 0, 3, 1, 3, 43, 93, 1665, 4575090, 4575090, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14, 14392),
(59080, 18414, 40322, 0, 'Darkmaster Gandling', '', 2097224, 7, 0, 3, 1, 3, 43, 93, 21, 5337605, 5337605, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14, 14392),
(59153, 18414, 31092, 0, 'Rattlegore', '', 2097224, 6, 0, 3, 1, 3, 43, 93, 233, 4575090, 4575090, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14, 14392),
(59184, 18414, 43460, 0, 'Jandice Barov', '', 2097224, 6, 0, 3, 1, 3, 43, 93, 233, 3812575, 3812575, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14, 14392),
-- Mogu'shan Palace bosses + trash
(61304, 18414, 41813, 0, 'Mogu Defier', '', 0, 10, 0, 1, 1, 3, 89, 93, 14, 61001200, 61001200, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 14392),
(61442, 18414, 42058, 0, 'Kuai the Brute', '', 2097224, 7, 0, 3, 1, 3, 89, 93, 14, 1723515, 1723515, 0, 1, 1500, 8321, 12024, 0, 1, 1, '', 1, 2.5, 8, 14, 12775),
(61444, 18414, 42059, 0, 'Ming the Cunning', '', 2097224, 7, 0, 3, 1, 3, 89, 93, 14, 1723515, 1723515, 9692, 1, 2000, 9352, 12526, 0, 1, 1, '', 1, 2.5, 8, 14, 12700),
-- Mogu'shan Vaults trash + boss
(60057, 18414, 41207, 41208, 'Mogu Ambusher', '', 0, 7, 0, 1, 1, 3, 90, 90, 14, 219994, 219994, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 13162),
(60710, 18414, 41565, 0, 'Subetai the Swift', 'Bandit King', 2097260, 10, 0, 3, 1, 3, 90, 90, 16, 30500600, 30500600, 0, 1, 1500, 5174, 12574, 0, 1, 1, '', 1, 2.5, 8, 14, 14392),
-- Siege of Niuzao Temple trash
(61514, 18414, 43381, 0, "Sra'thik Fleshrender", '', 0, 7, 0, 1, 1, 3, 90, 90, 16, 43999, 43999, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 13162),
-- Heart of Fear trash
(62966, 18414, 43315, 0, "Zan'thik General", '', 0, 7, 0, 1, 1, 3, 90, 90, 16, 439989, 439989, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 13162),
-- Throne of Thunder trash + boss
(69065, 18414, 47245, 47370, 'Zandalari Beastcaller', '', 0, 7, 0, 1, 1, 3, 93, 93, 16, 205328, 205328, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 13162),
(69156, 18414, 47549, 0, 'Zandalari Skyscreamer', '', 0, 1, 0, 1, 1, 3, 93, 93, 16, 305006, 305006, 0, 1, 1500, 100, 200, 0, 1, 1, '', 0, 2.5, 8, 14, 14392), -- not in reference, neutral modifier used
(69465, 18414, 47552, 0, "Jin'rokh the Breaker", '', 2097260, 7, 0, 3, 1, 3, 93, 93, 14, 305006, 305006, 0, 1, 1500, 100, 200, 0, 1, 1, '', 1, 2.5, 8, 14, 14392); -- not in reference, neutral modifier used


UPDATE `creature_properties` SET `minhealth`=309960, `maxhealth`=309960, `mana`=0, `armor`=10419 WHERE `entry`=39366 AND `build`=15595; -- NPC_SUN_TOUCHED_SERVANT
UPDATE `creature_properties` SET `minhealth`=154980, `maxhealth`=154980, `mana`=0, `armor`=10419 WHERE `entry`=39369 AND `build`=15595; -- NPC_SUN_TOUCHED_SPRITE
UPDATE `creature_properties` SET `minhealth`=23247, `maxhealth`=23247, `mana`=0, `armor`=10419 WHERE `entry`=39370 AND `build`=15595; -- NPC_SUN_TOUCHED_SPRITELING
UPDATE `creature_properties` SET `minhealth`=387450, `maxhealth`=387450, `mana`=0, `armor`=10419 WHERE `entry`=39373 AND `build`=15595; -- NPC_SUN_TOUCHED_SPEAKER
UPDATE `creature_properties` SET `minhealth`=2074850, `maxhealth`=2074850, `mana`=0, `armor`=10553 WHERE `entry`=39378 AND `build`=13202; -- BOSS_RAJH
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=0, `armor`=10099 WHERE `entry`=39381 AND `build`=15595; -- NPC_CRIMSONBORNE_GUARDIAN
UPDATE `creature_properties` SET `minhealth`=1659880, `maxhealth`=1659880, `mana`=32585, `armor`=11853 WHERE `entry`=39425 AND `build`=15595; -- BOSS_TEMPLE_GUARDIAN_ANHUUR
UPDATE `creature_properties` SET `minhealth`=1659880, `maxhealth`=1659880, `mana`=0, `armor`=10522 WHERE `entry`=39428 AND `build`=15595; -- BOSS_EARTHRAGER_PTAH
UPDATE `creature_properties` SET `minhealth`=13948, `maxhealth`=13948, `mana`=0, `armor`=10419 WHERE `entry`=39444 AND `build`=15595; -- NPC_PIT_VIPER
UPDATE `creature_properties` SET `minhealth`=1659880, `maxhealth`=1659880, `mana`=300000, `armor`=11853 WHERE `entry`=39587 AND `build`=13202; -- BOSS_ISISET
UPDATE `creature_properties` SET `minhealth`=123804, `maxhealth`=123804, `mana`=0, `armor`=9729 WHERE `entry`=39616 AND `build`=15595; -- NPC_NAZJAR_INVADER
UPDATE `creature_properties` SET `minhealth`=1603900, `maxhealth`=1603900, `mana`=0, `armor`=10470 WHERE `entry`=39625 AND `build`=15595; -- BOSS_GENERAL_UMBRISS
UPDATE `creature_properties` SET `minhealth`=893580, `maxhealth`=893580, `mana`=0, `armor`=10356 WHERE `entry`=39665 AND `build`=13164; -- BOSS_ROMOGG_BONECRUSHER
UPDATE `creature_properties` SET `minhealth`=893580, `maxhealth`=893580, `mana`=104225, `armor`=10253 WHERE `entry`=39679 AND `build`=13202; -- BOSS_CORLA_HERALD_OF_TWILIGHT
UPDATE `creature_properties` SET `minhealth`=893580, `maxhealth`=893580, `mana`=0, `armor`=10356 WHERE `entry`=39698 AND `build`=13202; -- BOSS_KARSH_STEELBENDER
UPDATE `creature_properties` SET `minhealth`=893580, `maxhealth`=893580, `mana`=0, `armor`=10356 WHERE `entry`=39700 AND `build`=13202; -- BOSS_BEAUTY
UPDATE `creature_properties` SET `minhealth`=837731, `maxhealth`=837731, `mana`=0, `armor`=10356 WHERE `entry`=39705 AND `build`=13202; -- BOSS_ASCENDANT_LORD_OBSIDIUS
UPDATE `creature_properties` SET `minhealth`=123804, `maxhealth`=123804, `mana`=0, `armor`=9729 WHERE `entry`=39708 AND `build`=15595; -- NPC_TWILIGHT_FLAME_CALLER
UPDATE `creature_properties` SET `minhealth`=1659880, `maxhealth`=1659880, `mana`=0, `armor`=10553 WHERE `entry`=39731 AND `build`=13202; -- BOSS_AMMUNAE
UPDATE `creature_properties` SET `minhealth`=1659880, `maxhealth`=1659880, `mana`=32585, `armor`=11853 WHERE `entry`=39732 AND `build`=13202; -- BOSS_SETESH
UPDATE `creature_properties` SET `minhealth`=2074850, `maxhealth`=2074850, `mana`=0, `armor`=10522 WHERE `entry`=39788 AND `build`=15595; -- BOSS_ANRAPHET
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=0, `armor`=10099 WHERE `entry`=39854 AND `build`=15595; -- NPC_AZUREBORNE_GUARDIAN
UPDATE `creature_properties` SET `minhealth`=206388, `maxhealth`=206388, `mana`=95120, `armor`=8670 WHERE `entry`=39855 AND `build`=15595; -- NPC_AZUREBORNE_SEER
UPDATE `creature_properties` SET `minhealth`=206388, `maxhealth`=206388, `mana`=95120, `armor`=8670 WHERE `entry`=39890 AND `build`=15595; -- NPC_TWILIGHT_EARTHSHAPER
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=0, `armor`=10099 WHERE `entry`=39956 AND `build`=15595; -- NPC_TWILIGHT_ENFORCER
UPDATE `creature_properties` SET `minhealth`=206388, `maxhealth`=206388, `mana`=95120, `armor`=8670 WHERE `entry`=39962 AND `build`=15595; -- NPC_TWILIGHT_STORMBREAKER
UPDATE `creature_properties` SET `minhealth`=123804, `maxhealth`=123804, `mana`=0, `armor`=9729 WHERE `entry`=39978 AND `build`=15595; -- NPC_TWILIGHT_TORTURER
UPDATE `creature_properties` SET `minhealth`=123804, `maxhealth`=123804, `mana`=0, `armor`=9729 WHERE `entry`=39980 AND `build`=15595; -- NPC_TWILIGHT_SADIST
UPDATE `creature_properties` SET `minhealth`=123804, `maxhealth`=123804, `mana`=19970, `armor`=9706 WHERE `entry`=39982 AND `build`=15595; -- NPC_CRAZED_MAGE
UPDATE `creature_properties` SET `minhealth`=123804, `maxhealth`=123804, `mana`=0, `armor`=9729 WHERE `entry`=39985 AND `build`=15595; -- NPC_MAD_PRISONER
UPDATE `creature_properties` SET `minhealth`=297496, `maxhealth`=297496, `mana`=0, `armor`=10033 WHERE `entry`=39994 AND `build`=15595; -- NPC_CONFLAGRATION
UPDATE `creature_properties` SET `minhealth`=123804, `maxhealth`=123804, `mana`=19970, `armor`=9706 WHERE `entry`=40017 AND `build`=15595; -- NPC_TWILIGHT_ELEMENT_WARDEN
UPDATE `creature_properties` SET `minhealth`=515968, `maxhealth`=515968, `mana`=0, `armor`=10099 WHERE `entry`=40166 AND `build`=15595; -- NPC_ENSLAVED_GRONN_BRUTE
UPDATE `creature_properties` SET `minhealth`=206388, `maxhealth`=206388, `mana`=95120, `armor`=8670 WHERE `entry`=40167 AND `build`=15595; -- NPC_TWILIGHT_BEGUILER
UPDATE `creature_properties` SET `minhealth`=2004875, `maxhealth`=2004875, `mana`=0, `armor`=10470 WHERE `entry`=40177 AND `build`=15595; -- BOSS_FORGEMASTER_THRONGUS
UPDATE `creature_properties` SET `minhealth`=309960, `maxhealth`=309960, `mana`=31178, `armor`=11213 WHERE `entry`=40311 AND `build`=15595; -- NPC_DUSTBONE_TORMENTOR
UPDATE `creature_properties` SET `minhealth`=641560, `maxhealth`=641560, `mana`=90040, `armor`=11533 WHERE `entry`=40319 AND `build`=15595; -- BOSS_DRAHGA_SHADOWBURNER
UPDATE `creature_properties` SET `minhealth`=2806825, `maxhealth`=2806825, `mana`=0, `armor`=10470 WHERE `entry`=40484 AND `build`=13164; -- BOSS_ERUDAX
UPDATE `creature_properties` SET `minhealth`=483431, `maxhealth`=483431, `mana`=0, `armor`=10033 WHERE `entry`=40577 AND `build`=15595; -- NPC_NAZJAR_SENTINEL
UPDATE `creature_properties` SET `minhealth`=1787160, `maxhealth`=1787160, `mana`=0, `armor`=10356 WHERE `entry`=40586 AND `build`=13202; -- BOSS_LADY_NAZJAR
UPDATE `creature_properties` SET `minhealth`=1116975, `maxhealth`=1116975, `mana`=0, `armor`=10356 WHERE `entry`=40765 AND `build`=15595; -- BOSS_COMMANDER_ULTHOK
UPDATE `creature_properties` SET `minhealth`=309960, `maxhealth`=309960, `mana`=0, `armor`=10419 WHERE `entry`=40787 AND `build`=15595; -- NPC_DUSTBONE_HORROR_SIMPLE
UPDATE `creature_properties` SET `minhealth`=670185, `maxhealth`=670185, `mana`=0, `armor`=10356 WHERE `entry`=40788 AND `build`=13202; -- BOSS_MINDBENDER_GURSHA
UPDATE `creature_properties` SET `minhealth`=309960, `maxhealth`=309960, `mana`=0, `armor`=10419 WHERE `entry`=40808 AND `build`=15595; -- NPC_DUSTBONE_HORROR_SUBMERGING
UPDATE `creature_properties` SET `minhealth`=3095, `maxhealth`=3095, `mana`=0, `armor`=9729 WHERE `entry`=40923 AND `build`=15595; -- NPC_UNSTABLE_CORRUPTION
UPDATE `creature_properties` SET `minhealth`=247608, `maxhealth`=247608, `mana`=0, `armor`=9729 WHERE `entry`=40925 AND `build`=15595; -- NPC_TAINTED_SENTRY
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=0, `armor`=10099 WHERE `entry`=41073 AND `build`=15595; -- NPC_TWILIGHT_ARMSMASTER
UPDATE `creature_properties` SET `minhealth`=123804, `maxhealth`=123804, `mana`=3994, `armor`=9706 WHERE `entry`=41096 AND `build`=15595; -- NPC_NAZJAR_SPIRITMENDER
UPDATE `creature_properties` SET `minhealth`=5582980, `maxhealth`=5582980, `mana`=0, `armor`=1057 WHERE `entry`=41270 AND `build`=15595; -- NPC_ONYXIA
UPDATE `creature_properties` SET `minhealth`=22761380, `maxhealth`=22761380, `mana`=0, `armor`=1057 WHERE `entry`=41376 AND `build`=15595; -- BOSS_NEFARIAN
UPDATE `creature_properties` SET `minhealth`=19755160, `maxhealth`=19755160, `mana`=0, `armor`=1057 WHERE `entry`=41378 AND `build`=15595; -- BOSS_MALORIAK
UPDATE `creature_properties` SET `minhealth`=26111168, `maxhealth`=26111168, `mana`=0, `armor`=1057 WHERE `entry`=41442 AND `build`=15595; -- BOSS_ATRAMEDES
UPDATE `creature_properties` SET `minhealth`=26798304, `maxhealth`=26798304, `mana`=0, `armor`=1057 WHERE `entry`=41570 AND `build`=15595; -- BOSS_MAGMAW
UPDATE `creature_properties` SET `minhealth`=25767600, `maxhealth`=25767600, `mana`=0, `armor`=10794 WHERE `entry`=42166 AND `build`=15595; -- NPC_ARCANOTRON
UPDATE `creature_properties` SET `minhealth`=25767600, `maxhealth`=25767600, `mana`=0, `armor`=10794 WHERE `entry`=42178 AND `build`=15595; -- NPC_MAGMATRON
UPDATE `creature_properties` SET `minhealth`=25767600, `maxhealth`=25767600, `mana`=0, `armor`=10794 WHERE `entry`=42179 AND `build`=15595; -- NPC_ELECTRON
UPDATE `creature_properties` SET `minhealth`=25767600, `maxhealth`=25767600, `mana`=0, `armor`=10794 WHERE `entry`=42180 AND `build`=15595; -- NPC_TOXITRON
UPDATE `creature_properties` SET `minhealth`=2450848, `maxhealth`=2450848, `mana`=0, `armor`=10099 WHERE `entry`=42188 AND `build`=13164; -- BOSS_OZRUK
UPDATE `creature_properties` SET `minhealth`=1612400, `maxhealth`=1612400, `mana`=0, `armor`=10099 WHERE `entry`=42333 AND `build`=13164; -- BOSS_HIGH_PRIESTESS_AZIL
UPDATE `creature_properties` SET `minhealth`=312753, `maxhealth`=312753, `mana`=8338, `armor`=10253 WHERE `entry`=42691 AND `build`=15595; -- NPC_STONECORE_RIFT_CONJURER
UPDATE `creature_properties` SET `minhealth`=590491, `maxhealth`=590491, `mana`=0, `armor`=10643 WHERE `entry`=42692 AND `build`=15595; -- NPC_STONECORE_BRUISER
UPDATE `creature_properties` SET `minhealth`=312753, `maxhealth`=312753, `mana`=0, `armor`=10356 WHERE `entry`=42696 AND `build`=15595; -- NPC_STONECORE_WARBRINGER
UPDATE `creature_properties` SET `minhealth`=312753, `maxhealth`=312753, `mana`=12507, `armor`=10253 WHERE `entry`=42789 AND `build`=15595; -- NPC_STONECORE_MAGMALORD
UPDATE `creature_properties` SET `minhealth`=4649400, `maxhealth`=4649400, `mana`=0, `armor`=10419 WHERE `entry`=42800 AND `build`=15595; -- NPC_GOLEM_SENTRY
UPDATE `creature_properties` SET `minhealth`=312753, `maxhealth`=312753, `mana`=0, `armor`=10356 WHERE `entry`=42808 AND `build`=15595; -- NPC_STONECORE_FLAYER
UPDATE `creature_properties` SET `minhealth`=536810, `maxhealth`=536810, `mana`=0, `armor`=10643 WHERE `entry`=42810 AND `build`=15595; -- NPC_CRYSTALSPAWN_GIANT
UPDATE `creature_properties` SET `minhealth`=2450848, `maxhealth`=2450848, `mana`=0, `armor`=10099 WHERE `entry`=43214 AND `build`=15595; -- BOSS_SLABHIDE
UPDATE `creature_properties` SET `minhealth`=20699972, `maxhealth`=20699972, `mana`=0, `armor`=1057 WHERE `entry`=43296 AND `build`=15595; -- BOSS_CHIMAERON
UPDATE `creature_properties` SET `minhealth`=26798304, `maxhealth`=26798304, `mana`=473500, `armor`=12173 WHERE `entry`=43324 AND `build`=15595; -- BOSS_CHOGALL
UPDATE `creature_properties` SET `minhealth`=312753, `maxhealth`=312753, `mana`=0, `armor`=10356 WHERE `entry`=43430 AND `build`=15595; -- NPC_STONECORE_BERSERKER
UPDATE `creature_properties` SET `minhealth`=1612400, `maxhealth`=1612400, `mana`=0, `armor`=10099 WHERE `entry`=43438 AND `build`=15595; -- BOSS_CORBORUS
UPDATE `creature_properties` SET `minhealth`=250201, `maxhealth`=250201, `mana`=9160, `armor`=8340 WHERE `entry`=43537 AND `build`=15595; -- NPC_STONECORE_EARTHSHAPER
UPDATE `creature_properties` SET `minhealth`=1603900, `maxhealth`=1603900, `mana`=31514, `armor`=11533 WHERE `entry`=43612 AND `build`=15595; -- BOSS_HIGH_PROPHET_BARIM
UPDATE `creature_properties` SET `minhealth`=1603900, `maxhealth`=1603900, `mana`=31514, `armor`=11533 WHERE `entry`=43614 AND `build`=15595; -- BOSS_LOCKMAW
UPDATE `creature_properties` SET `minhealth`=6871360, `maxhealth`=6871360, `mana`=2367500, `armor`=12173 WHERE `entry`=43686 AND `build`=15595; -- BOSS_IGNACIOUS
UPDATE `creature_properties` SET `minhealth`=6871360, `maxhealth`=6871360, `mana`=2367500, `armor`=12173 WHERE `entry`=43687 AND `build`=15595; -- BOSS_FELUDIUS
UPDATE `creature_properties` SET `minhealth`=4724060, `maxhealth`=4724060, `mana`=2367500, `armor`=12173 WHERE `entry`=43688 AND `build`=15595; -- BOSS_ARION
UPDATE `creature_properties` SET `minhealth`=4724060, `maxhealth`=4724060, `mana`=2367500, `armor`=12173 WHERE `entry`=43689 AND `build`=15595; -- BOSS_TERRASTRA
UPDATE `creature_properties` SET `minhealth`=1999376, `maxhealth`=1999376, `mana`=0, `armor`=10099 WHERE `entry`=43873 AND `build`=15595; -- BOSS_ALTAIRUS
UPDATE `creature_properties` SET `minhealth`=2579840, `maxhealth`=2579840, `mana`=109075, `armor`=10893 WHERE `entry`=43875 AND `build`=13164; -- BOSS_ASAAD
UPDATE `creature_properties` SET `minhealth`=1805888, `maxhealth`=1805888, `mana`=218150, `armor`=10893 WHERE `entry`=43878 AND `build`=15595; -- BOSS_GRAND_VIZIER_ERTAN
UPDATE `creature_properties` SET `minhealth`=9668620, `maxhealth`=9668620, `mana`=0, `armor`=10033 WHERE `entry`=44566 AND `build`=15595; -- BOSS_OZUMAT
UPDATE `creature_properties` SET `minhealth`=2004875, `maxhealth`=2004875, `mana`=0, `armor`=10470 WHERE `entry`=44577 AND `build`=13164; -- BOSS_GENERAL_HUSAM
UPDATE `creature_properties` SET `minhealth`=25939384, `maxhealth`=25939384, `mana`=0, `armor`=1057 WHERE `entry`=44600 AND `build`=15595; -- BOSS_HALFUS_WYRMBREAKER
UPDATE `creature_properties` SET `minhealth`=2004875, `maxhealth`=2004875, `mana`=31514, `armor`=11533 WHERE `entry`=44819 AND `build`=13164; -- BOSS_SIAMAT
UPDATE `creature_properties` SET `minhealth`=206388, `maxhealth`=206388, `mana`=66584, `armor`=8670 WHERE `entry`=44897 AND `build`=15595; -- NPC_PYGMY_SCOUT
UPDATE `creature_properties` SET `minhealth`=206388, `maxhealth`=206388, `mana`=66584, `armor`=8670 WHERE `entry`=44898 AND `build`=15595; -- NPC_PYGMY_FIREBREATHER
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=0, `armor`=10099 WHERE `entry`=44922 AND `build`=15595; -- NPC_OATHSWORN_AXEMASTER
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=0, `armor`=10099 WHERE `entry`=44924 AND `build`=15595; -- NPC_OATHSWORN_MYRMIDON
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=0, `armor`=10099 WHERE `entry`=44926 AND `build`=15595; -- NPC_OATHSWORN_WANDERER
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=0, `armor`=10099 WHERE `entry`=44932 AND `build`=15595; -- NPC_OATHSWORN_PATHFINDER
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=30541, `armor`=10893 WHERE `entry`=44976 AND `build`=15595; -- NPC_NEFERSET_PLAGUEBRINGER
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=30541, `armor`=10893 WHERE `entry`=44977 AND `build`=15595; -- NPC_NEFERSET_TORTURER
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=30541, `armor`=10893 WHERE `entry`=44980 AND `build`=15595; -- NPC_NEFERSET_THEURGIST
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=0, `armor`=10099 WHERE `entry`=44981 AND `build`=15595; -- NPC_OATHSWORN_SKINNER
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=30541, `armor`=10893 WHERE `entry`=44982 AND `build`=15595; -- NPC_NEFERSET_DARKCASTER
UPDATE `creature_properties` SET `minhealth`=257984, `maxhealth`=257984, `mana`=30541, `armor`=10893 WHERE `entry`=45062 AND `build`=15595; -- NPC_OATHSWORN_SCORPID_KEEPER
UPDATE `creature_properties` SET `minhealth`=85239, `maxhealth`=85239, `mana`=0, `armor`=10419 WHERE `entry`=45065 AND `build`=15595; -- NPC_TOLVIR_MERCHANT
UPDATE `creature_properties` SET `minhealth`=1162350, `maxhealth`=1162350, `mana`=0, `armor`=10419 WHERE `entry`=45261 AND `build`=15595; -- NPC_TWILIGHT_SHADOW_KNIGHT
UPDATE `creature_properties` SET `minhealth`=1162350, `maxhealth`=1162350, `mana`=89080, `armor`=11213 WHERE `entry`=45264 AND `build`=15595; -- NPC_TWILIGHT_CROSSFIRE
UPDATE `creature_properties` SET `minhealth`=1659880, `maxhealth`=1659880, `mana`=0, `armor`=10522 WHERE `entry`=45265 AND `build`=15595; -- NPC_TWILIGHT_SOUL_BLADE
UPDATE `creature_properties` SET `minhealth`=1162350, `maxhealth`=1162350, `mana`=89080, `armor`=11213 WHERE `entry`=45266 AND `build`=15595; -- NPC_TWILIGHT_DARK_MENDER
UPDATE `creature_properties` SET `minhealth`=464940, `maxhealth`=464940, `mana`=44540, `armor`=11213 WHERE `entry`=45267 AND `build`=15595; -- NPC_TWILIGHT_PHASE_TWISTER
UPDATE `creature_properties` SET `minhealth`=223395, `maxhealth`=223395, `mana`=0, `armor`=10356 WHERE `entry`=45477 AND `build`=15595; -- NPC_GUST_SOLDIER
UPDATE `creature_properties` SET `minhealth`=4294600, `maxhealth`=4294600, `mana`=45, `armor`=12173 WHERE `entry`=45870 AND `build`=15595; -- BOSS_ANSHAL
UPDATE `creature_properties` SET `minhealth`=7300820, `maxhealth`=7300820, `mana`=45, `armor`=12173 WHERE `entry`=45871 AND `build`=15595; -- BOSS_NEZIR
UPDATE `creature_properties` SET `minhealth`=4294600, `maxhealth`=4294600, `mana`=45, `armor`=12173 WHERE `entry`=45872 AND `build`=15595; -- BOSS_ROHASH
UPDATE `creature_properties` SET `minhealth`=223395, `maxhealth`=223395, `mana`=20845, `armor`=10253 WHERE `entry`=45912 AND `build`=15595; -- NPC_WILD_VORTEX
UPDATE `creature_properties` SET `minhealth`=223395, `maxhealth`=223395, `mana`=0, `armor`=10356 WHERE `entry`=45915 AND `build`=15595; -- NPC_ARMORED_MISTRAL
UPDATE `creature_properties` SET `minhealth`=536810, `maxhealth`=536810, `mana`=42580, `armor`=10643 WHERE `entry`=45917 AND `build`=15595; -- NPC_CLOUD_PRINCE
UPDATE `creature_properties` SET `minhealth`=223395, `maxhealth`=223395, `mana`=0, `armor`=10356 WHERE `entry`=45922 AND `build`=15595; -- NPC_EMPYREAN_ASSASSIN
UPDATE `creature_properties` SET `minhealth`=223395, `maxhealth`=223395, `mana`=20845, `armor`=10253 WHERE `entry`=45924 AND `build`=15595; -- NPC_TURBULENT_SQUALL
UPDATE `creature_properties` SET `minhealth`=223395, `maxhealth`=223395, `mana`=20845, `armor`=10253 WHERE `entry`=45926 AND `build`=15595; -- NPC_SERVANT_OF_ASAAD
UPDATE `creature_properties` SET `minhealth`=223395, `maxhealth`=223395, `mana`=0, `armor`=10356 WHERE `entry`=45928 AND `build`=15595; -- NPC_EXECUTOR_OF_THE_CALIPH
UPDATE `creature_properties` SET `minhealth`=178715, `maxhealth`=178715, `mana`=45800, `armor`=8340 WHERE `entry`=45930 AND `build`=15595; -- NPC_MINISTER_OF_AIR
UPDATE `creature_properties` SET `minhealth`=178715, `maxhealth`=178715, `mana`=45800, `armor`=8340 WHERE `entry`=45935 AND `build`=15595; -- NPC_TEMPLE_ADEPT
UPDATE `creature_properties` SET `minhealth`=25767600, `maxhealth`=25767600, `mana`=0, `armor`=1057 WHERE `entry`=45992 AND `build`=15595; -- BOSS_VALIONA
UPDATE `creature_properties` SET `minhealth`=25767600, `maxhealth`=25767600, `mana`=0, `armor`=1057 WHERE `entry`=45993 AND `build`=15595; -- BOSS_THERALION
UPDATE `creature_properties` SET `minhealth`=24049760, `maxhealth`=24049760, `mana`=0, `armor`=1057 WHERE `entry`=46753 AND `build`=15595; -- BOSS_ALAKIR
UPDATE `creature_properties` SET `minhealth`=21473000, `maxhealth`=21473000, `mana`=0, `armor`=1057 WHERE `entry`=47120 AND `build`=15595; -- BOSS_ARGALOTH
UPDATE `creature_properties` SET `minhealth`=154980, `maxhealth`=154980, `mana`=0, `armor`=10419 WHERE `entry`=47829 AND `build`=15595; -- NPC_FEL_FLAMES
UPDATE `creature_properties` SET `minhealth`=309960, `maxhealth`=309960, `mana`=0, `armor`=10419 WHERE `entry`=48139 AND `build`=15595; -- NPC_TEMPLE_SWIFTSTALKER
UPDATE `creature_properties` SET `minhealth`=309960, `maxhealth`=309960, `mana`=31178, `armor`=11213 WHERE `entry`=48140 AND `build`=15595; -- NPC_TEMPLE_RUNECASTER
UPDATE `creature_properties` SET `minhealth`=309960, `maxhealth`=309960, `mana`=0, `armor`=10419 WHERE `entry`=48141 AND `build`=15595; -- NPC_TEMPLE_SHADOWLANCER
UPDATE `creature_properties` SET `minhealth`=619920, `maxhealth`=619920, `mana`=31178, `armor`=11213 WHERE `entry`=48143 AND `build`=15595; -- NPC_TEMPLE_FIRESHAPER
UPDATE `creature_properties` SET `minhealth`=2074850, `maxhealth`=2074850, `mana`=0, `armor`=10522 WHERE `entry`=49045 AND `build`=15595; -- BOSS_AUGH
UPDATE `creature_properties` SET `minhealth`=2074850, `maxhealth`=2074850, `mana`=69825, `armor`=11853 WHERE `entry`=49817 AND `build`=15595; -- NPC_BOUND_INFERNO
UPDATE `creature_properties` SET `minhealth`=2074850, `maxhealth`=2074850, `mana`=69825, `armor`=11853 WHERE `entry`=49821 AND `build`=15595; -- NPC_BOUND_ZEPHYR
UPDATE `creature_properties` SET `minhealth`=2074850, `maxhealth`=2074850, `mana`=69825, `armor`=11853 WHERE `entry`=49825 AND `build`=15595; -- NPC_BOUND_DELUGE
UPDATE `creature_properties` SET `minhealth`=2074850, `maxhealth`=2074850, `mana`=69825, `armor`=11853 WHERE `entry`=49826 AND `build`=15595; -- NPC_BOUND_RUMBLER
UPDATE `creature_properties` SET `minhealth`=35743, `maxhealth`=35743, `mana`=0, `armor`=10356 WHERE `entry`=52019 AND `build`=15595; -- NPC_VP_SKYFALL_STAR
UPDATE `creature_properties` SET `minhealth`=4149700, `maxhealth`=4149700, `mana`=232750, `armor`=11853 WHERE `entry`=52053 AND `build`=15595; -- BOSS_ZANZIL
UPDATE `creature_properties` SET `minhealth`=3734730, `maxhealth`=3734730, `mana`=23275, `armor`=11853 WHERE `entry`=52059 AND `build`=15595; -- BOSS_HIGH_PRIESTESS_KILNARA
UPDATE `creature_properties` SET `minhealth`=387450, `maxhealth`=387450, `mana`=44540, `armor`=11213 WHERE `entry`=52076 AND `build`=15595; -- NPC_GURUBASHI_CAULDRON_MIXER_A
UPDATE `creature_properties` SET `minhealth`=387450, `maxhealth`=387450, `mana`=44540, `armor`=11213 WHERE `entry`=52082 AND `build`=15595; -- NPC_GURUBASHI_CAULDRON_MIXER_B
UPDATE `creature_properties` SET `minhealth`=77490, `maxhealth`=77490, `mana`=0, `armor`=10419 WHERE `entry`=52085 AND `build`=15595; -- NPC_RAZZASHI_ADDER
UPDATE `creature_properties` SET `minhealth`=774900, `maxhealth`=774900, `mana`=22270, `armor`=11213 WHERE `entry`=52086 AND `build`=15595; -- NPC_HAKKARI_WITCH_DOCTOR
UPDATE `creature_properties` SET `minhealth`=387450, `maxhealth`=387450, `mana`=44540, `armor`=11213 WHERE `entry`=52088 AND `build`=15595; -- NPC_GURUBASHI_CAULDRON_MIXER_C
UPDATE `creature_properties` SET `minhealth`=5809580, `maxhealth`=5809580, `mana`=0, `armor`=10522 WHERE `entry`=52148 AND `build`=13914; -- BOSS_JINDO_THE_GODBREAKER
UPDATE `creature_properties` SET `minhealth`=4813652, `maxhealth`=4813652, `mana`=0, `armor`=10522 WHERE `entry`=52151 AND `build`=15595; -- BOSS_BLOODLORD_MANDOKIR
UPDATE `creature_properties` SET `minhealth`=4979640, `maxhealth`=4979640, `mana`=46550, `armor`=11853 WHERE `entry`=52155 AND `build`=15595; -- BOSS_HIGH_PRIEST_VENOXIS
UPDATE `creature_properties` SET `minhealth`=2074850, `maxhealth`=2074850, `mana`=0, `armor`=10522 WHERE `entry`=52258 AND `build`=15595; -- BOSS_GRILEK
UPDATE `creature_properties` SET `minhealth`=2074850, `maxhealth`=2074850, `mana`=0, `armor`=10522 WHERE `entry`=52269 AND `build`=15595; -- BOSS_RENATAKI
UPDATE `creature_properties` SET `minhealth`=2074850, `maxhealth`=2074850, `mana`=9310, `armor`=11853 WHERE `entry`=52271 AND `build`=15595; -- BOSS_HAZZARAH
UPDATE `creature_properties` SET `minhealth`=1000, `maxhealth`=1000, `mana`=0, `armor`=10522 WHERE `entry`=52284 AND `build`=15595; -- NPC_NIGHTMARE_ILLUSION
UPDATE `creature_properties` SET `minhealth`=2074850, `maxhealth`=2074850, `mana`=0, `armor`=10522 WHERE `entry`=52286 AND `build`=15595; -- BOSS_WUSHOOLAY
UPDATE `creature_properties` SET `minhealth`=200488, `maxhealth`=200488, `mana`=0, `armor`=10470 WHERE `entry`=52322 AND `build`=15595; -- NPC_WITCH_DOCTOR_QUIN
UPDATE `creature_properties` SET `minhealth`=77490, `maxhealth`=77490, `mana`=0, `armor`=10419 WHERE `entry`=52323 AND `build`=15595; -- NPC_CHOSEN_OF_HETHISS
UPDATE `creature_properties` SET `minhealth`=387450, `maxhealth`=387450, `mana`=0, `armor`=10419 WHERE `entry`=52325 AND `build`=15595; -- NPC_GURUBASHI_BLOOD_DRINKER
UPDATE `creature_properties` SET `minhealth`=387450, `maxhealth`=387450, `mana`=0, `armor`=10419 WHERE `entry`=52327 AND `build`=15595; -- NPC_GURUBASHI_SHADOW_HUNTER
UPDATE `creature_properties` SET `minhealth`=387450, `maxhealth`=387450, `mana`=31178, `armor`=11213 WHERE `entry`=52339 AND `build`=15595; -- NPC_LESSER_PRIEST_OF_BETHEKK
UPDATE `creature_properties` SET `minhealth`=387450, `maxhealth`=387450, `mana`=0, `armor`=10419 WHERE `entry`=52345 AND `build`=15595; -- NPC_PRIDE_OF_BETHEKK
UPDATE `creature_properties` SET `minhealth`=23653290, `maxhealth`=23653290, `mana`=0, `armor`=10522 WHERE `entry`=52363 AND `build`=15595; -- BOSS_OCCUTHAR
UPDATE `creature_properties` SET `minhealth`=82994, `maxhealth`=82994, `mana`=0, `armor`=10522 WHERE `entry`=52369 AND `build`=15595; -- NPC_FOCUS_FIRE_DUMMY
UPDATE `creature_properties` SET `minhealth`=801950, `maxhealth`=801950, `mana`=45020, `armor`=11533 WHERE `entry`=52380 AND `build`=15595; -- NPC_VENOMANCER_MAURI
UPDATE `creature_properties` SET `minhealth`=801950, `maxhealth`=801950, `mana`=45020, `armor`=11533 WHERE `entry`=52381 AND `build`=15595; -- NPC_VENOMANCER_TKULU
UPDATE `creature_properties` SET `minhealth`=99593, `maxhealth`=99593, `mana`=0, `armor`=10522 WHERE `entry`=52389 AND `build`=15595; -- NPC_EYE_OF_OCCUTHAR
UPDATE `creature_properties` SET `minhealth`=50246820, `maxhealth`=50246820, `mana`=0, `armor`=1057 WHERE `entry`=52409 AND `build`=15595; -- BOSS_RAGNAROS
UPDATE `creature_properties` SET `minhealth`=1202925, `maxhealth`=1202925, `mana`=0, `armor`=10470 WHERE `entry`=52419 AND `build`=15595; -- NPC_TIKI_TORCH
UPDATE `creature_properties` SET `minhealth`=17693752, `maxhealth`=17693752, `mana`=4100, `armor`=12173 WHERE `entry`=52498 AND `build`=15595; -- BOSS_BETHTILAC
UPDATE `creature_properties` SET `minhealth`=38651400, `maxhealth`=38651400, `mana`=50, `armor`=12173 WHERE `entry`=52530 AND `build`=15595; -- BOSS_ALYSRAZOR
UPDATE `creature_properties` SET `minhealth`=13099990, `maxhealth`=13099990, `mana`=0, `armor`=1057 WHERE `entry`=52558 AND `build`=15595; -- BOSS_LORD_RHYOLITH
UPDATE `creature_properties` SET `minhealth`=38221940, `maxhealth`=38221940, `mana`=0, `armor`=10794 WHERE `entry`=52571 AND `build`=15595; -- BOSS_MAJORDOMO_STAGHELM
UPDATE `creature_properties` SET `minhealth`=309960, `maxhealth`=309960, `mana`=96970, `armor`=8835 WHERE `entry`=52598 AND `build`=15595; -- NPC_GURUBASHI_SOUL_EATER
UPDATE `creature_properties` SET `minhealth`=641560, `maxhealth`=641560, `mana`=0, `armor`=10470 WHERE `entry`=52606 AND `build`=15595; -- NPC_GURUBASHI_WARMONGER
UPDATE `creature_properties` SET `minhealth`=641560, `maxhealth`=641560, `mana`=0, `armor`=10470 WHERE `entry`=52956 AND `build`=15595; -- NPC_ZANDALARI_JUGGERNAUT
UPDATE `creature_properties` SET `minhealth`=641560, `maxhealth`=641560, `mana`=45020, `armor`=11533 WHERE `entry`=52958 AND `build`=15595; -- NPC_ZANDALARI_HIEROPHANT
UPDATE `creature_properties` SET `minhealth`=641560, `maxhealth`=641560, `mana`=45020, `armor`=11533 WHERE `entry`=52962 AND `build`=15595; -- NPC_ZANDALARI_ARCHON
UPDATE `creature_properties` SET `minhealth`=3874500, `maxhealth`=3874500, `mana`=0, `armor`=10419 WHERE `entry`=53115 AND `build`=15595; -- NPC_MOLTEN_LORD
UPDATE `creature_properties` SET `minhealth`=542430, `maxhealth`=542430, `mana`=44540, `armor`=11213 WHERE `entry`=53187 AND `build`=15595; -- NPC_FLAMEWAKER_ANIMATOR
UPDATE `creature_properties` SET `minhealth`=3099600, `maxhealth`=3099600, `mana`=0, `armor`=10419 WHERE `entry`=53188 AND `build`=15595; -- NPC_FLAMEWAKER_SUBJUGATOR
UPDATE `creature_properties` SET `minhealth`=31565310, `maxhealth`=31565310, `mana`=0, `armor`=1057 WHERE `entry`=53494 AND `build`=15595; -- BOSS_BALEROC
UPDATE `creature_properties` SET `minhealth`=2169720, `maxhealth`=2169720, `mana`=40, `armor`=11213 WHERE `entry`=53635 AND `build`=15595; -- NPC_CINDERWEB_DRONE
UPDATE `creature_properties` SET `minhealth`=1859760, `maxhealth`=1859760, `mana`=96970, `armor`=8835 WHERE `entry`=53639 AND `build`=15595; -- NPC_FLAMEWAKER_CAUTERIZER
UPDATE `creature_properties` SET `minhealth`=309960, `maxhealth`=309960, `mana`=0, `armor`=10419 WHERE `entry`=53642 AND `build`=15595; -- NPC_CINDERWEB_SPINNER
UPDATE `creature_properties` SET `minhealth`=20442296, `maxhealth`=20442296, `mana`=0, `armor`=1057 WHERE `entry`=53691 AND `build`=15595; -- BOSS_SHANNOX
UPDATE `creature_properties` SET `minhealth`=6639520, `maxhealth`=6639520, `mana`=93100, `armor`=11853 WHERE `entry`=54123 AND `build`=15595; -- BOSS_ECHO_OF_SYLVANAS
UPDATE `creature_properties` SET `minhealth`=7137484, `maxhealth`=7137484, `mana`=0, `armor`=10522 WHERE `entry`=54431 AND `build`=15595; -- BOSS_ECHO_OF_BAINE
UPDATE `creature_properties` SET `minhealth`=18258680, `maxhealth`=18258680, `mana`=465500, `armor`=11853 WHERE `entry`=54432 AND `build`=15005; -- BOSS_MUROZOND
UPDATE `creature_properties` SET `minhealth`=6224550, `maxhealth`=6224550, `mana`=93100, `armor`=11853 WHERE `entry`=54445 AND `build`=15595; -- BOSS_ECHO_OF_JAINA
UPDATE `creature_properties` SET `minhealth`=309960, `maxhealth`=309960, `mana`=0, `armor`=10419 WHERE `entry`=54511 AND `build`=15595; -- NPC_TIME_TWISTED_GEIST
UPDATE `creature_properties` SET `minhealth`=6639520, `maxhealth`=6639520, `mana`=93100, `armor`=11853 WHERE `entry`=54544 AND `build`=15595; -- BOSS_ECHO_OF_TYRANDE
UPDATE `creature_properties` SET `minhealth`=1549800, `maxhealth`=1549800, `mana`=22270, `armor`=11213 WHERE `entry`=54663 AND `build`=15595; -- BOSS_EARTHCALLER_TORUNSCAR
UPDATE `creature_properties` SET `minhealth`=1549800, `maxhealth`=1549800, `mana`=26724, `armor`=11213 WHERE `entry`=54664 AND `build`=15595; -- BOSS_TAWN_WINTERBLUFF
UPDATE `creature_properties` SET `minhealth`=1549800, `maxhealth`=1549800, `mana`=26724, `armor`=11213 WHERE `entry`=54665 AND `build`=15595; -- BOSS_HARGOTH_DIMBLAZE
UPDATE `creature_properties` SET `minhealth`=1549800, `maxhealth`=1549800, `mana`=26724, `armor`=11213 WHERE `entry`=54666 AND `build`=15595; -- BOSS_STORMCALLER_JALARA
UPDATE `creature_properties` SET `minhealth`=77490, `maxhealth`=77490, `mana`=0, `armor`=10419 WHERE `entry`=54688 AND `build`=15595; -- NPC_TIME_TWISTED_NIGHTSABER
UPDATE `creature_properties` SET `minhealth`=412640, `maxhealth`=412640, `mana`=93100, `armor`=11853 WHERE `entry`=54938 AND `build`=15005; -- BOSS_ARCHBISHOP_BENEDICTUS
UPDATE `creature_properties` SET `minhealth`=6639520, `maxhealth`=6639520, `mana`=0, `armor`=10553 WHERE `entry`=54968 AND `build`=15595; -- BOSS_ASIRA_DAWNSLAYER
UPDATE `creature_properties` SET `minhealth`=9544310, `maxhealth`=9544310, `mana`=0, `armor`=10522 WHERE `entry`=55085 AND `build`=15005; -- BOSS_PEROTHARN
UPDATE `creature_properties` SET `minhealth`=36000172, `maxhealth`=36000172, `mana`=4735, `armor`=12173 WHERE `entry`=55265 AND `build`=15595; -- BOSS_MORCHOK
UPDATE `creature_properties` SET `minhealth`=6639, `maxhealth`=6639, `mana`=0, `armor`=1057 WHERE `entry`=55293 AND `build`=15595; -- BOSS_ULTRAXION
UPDATE `creature_properties` SET `minhealth`=68198248, `maxhealth`=68198248, `mana`=0, `armor`=1057 WHERE `entry`=55308 AND `build`=15595; -- BOSS_WARLORD_ZONOZZ
UPDATE `creature_properties` SET `minhealth`=47240600, `maxhealth`=47240600, `mana`=0, `armor`=1057 WHERE `entry`=55312 AND `build`=15595; -- BOSS_YORSAHJ_THE_UNSLEEPING
UPDATE `creature_properties` SET `minhealth`=232470, `maxhealth`=232470, `mana`=0, `armor`=10419 WHERE `entry`=55519 AND `build`=15595; -- NPC_DOOMGUARD_ANNIHILATOR
UPDATE `creature_properties` SET `minhealth`=2130975, `maxhealth`=2130975, `mana`=0, `armor`=10419 WHERE `entry`=55656 AND `build`=15595; -- NPC_DREADLORD_DEFENDER
UPDATE `creature_properties` SET `minhealth`=34356800, `maxhealth`=34356800, `mana`=473500, `armor`=12173 WHERE `entry`=55689 AND `build`=15595; -- BOSS_HAGARA_THE_STORMBINDER
UPDATE `creature_properties` SET `minhealth`=25767600, `maxhealth`=25767600, `mana`=0, `armor`=1057 WHERE `entry`=55869 AND `build`=15595; -- BOSS_ALIZABAL
UPDATE `creature_properties` SET `minhealth`=127120160, `maxhealth`=127120160, `mana`=0, `armor`=1057 WHERE `entry`=56173 AND `build`=15595; -- BOSS_MADNESS_OF_DEATHWING
UPDATE `creature_properties` SET `minhealth`=2324700, `maxhealth`=2324700, `mana`=0, `armor`=10419 WHERE `entry`=56350 AND `build`=15595; -- NPC_DISCIPLE_OF_HATE
UPDATE `creature_properties` SET `minhealth`=20614080, `maxhealth`=20614080, `mana`=0, `armor`=1057 WHERE `entry`=56427 AND `build`=15595; -- BOSS_WARMASTER_BLACKHORN
UPDATE `creature_properties` SET `minhealth`=3487050, `maxhealth`=3487050, `mana`=4454, `armor`=11213 WHERE `entry`=57158 AND `build`=15595; -- NPC_EARTHEN_DESTROYER
UPDATE `creature_properties` SET `minhealth`=2324700, `maxhealth`=2324700, `mana`=0, `armor`=10071 WHERE `entry`=57159 AND `build`=15595; -- NPC_EARTHEN_SOLDIER
UPDATE `creature_properties` SET `minhealth`=3487050, `maxhealth`=3487050, `mana`=4454, `armor`=11213 WHERE `entry`=57160 AND `build`=15595; -- NPC_ANCIENT_WATER_LORD
UPDATE `creature_properties` SET `minhealth`=3227880, `maxhealth`=3227880, `mana`=100520, `armor`=9165 WHERE `entry`=57280 AND `build`=15595; -- NPC_TWILIGHT_SIEGE_CAPTAIN


INSERT INTO `creature_initial_equip` (`creature_entry`, `itemslot_1`, `itemslot_2`, `itemslot_3`)
VALUES
(39378, 1728, 0, 0),
(39381, 55150, 55150, 0),
(39425, 28914, 0, 0),
(39616, 34058, 0, 0),
(39625, 65663, 0, 0),
(39665, 54780, 54780, 0),
(39679, 52838, 0, 0),
(39698, 58176, 0, 0),
(39705, 60969, 0, 0),
(39708, 49653, 0, 0),
(39854, 50425, 0, 0),
(39855, 52837, 0, 0),
(39890, 40606, 0, 0),
(39956, 53008, 0, 0),
(39962, 13750, 0, 0),
(39978, 52520, 0, 0),
(39980, 52517, 52517, 6886),
(39985, 54827, 0, 0),
(40017, 49724, 0, 0),
(40167, 40606, 0, 0),
(40319, 50225, 0, 0),
(52269, 62456, 62475, 0),
(52286, 19988, 0, 0),
(56448, 64880, 0, 0),
(56589, 82849, 0, 0),
(56636, 85576, 0, 0),
(56747, 77221, 0, 0),
(56927, 80580, 0, 0),
(58466, 82272, 0, 0),
(58605, 80589, 80589, 0),
(58632, 7612, 0, 0),
(58685, 43619, 0, 0),
(58722, 49340, 49340, 0),
(58757, 80290, 0, 0),
(58824, 0, 0, 74588),
(59080, 79321, 0, 0),
(59150, 71039, 0, 0),
(59175, 56696, 0, 55306),
(59184, 13069, 0, 0),
(59303, 39140, 39140, 0),
(59368, 75039, 75039, 0),
(59467, 13069, 0, 0),
(59705, 0, 0, 80271),
(59752, 72822, 72822, 0),
(59789, 58470, 0, 0),
(60016, 82767, 82767, 0),
(60040, 80537, 0, 0),
(60057, 1905, 1905, 0),
(60399, 80939, 0, 0),
(60400, 80289, 0, 0),
(60583, 81390, 0, 0),
(60701, 76369, 0, 0),
(60709, 86777, 0, 0),
(60710, 80283, 80283, 60790),
(61243, 72484, 0, 0),
(61304, 68838, 32375, 0),
(61398, 67153, 0, 0),
(61442, 82349, 82349, 0),
(61444, 82788, 0, 0),
(61445, 82789, 0, 0),
(61485, 85401, 0, 0),
(61502, 74756, 0, 0),
(61514, 85399, 85399, 0),
(61634, 84769, 84767, 0),
(62397, 85401, 0, 0),
(62511, 86983, 0, 0),
(62543, 85401, 0, 0),
(62966, 85401, 0, 0),
(62980, 85947, 0, 0),
(63071, 85576, 0, 0),
(68078, 93234, 0, 0),
(68397, 94906, 0, 0),
(68905, 93776, 93780, 0),
(69065, 2522, 2522, 33790),
(71515, 7612, 0, 0),
(71859, 101737, 101737, 0),
(71865, 101441, 0, 0),
(72728, 94649, 0, 0);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('181', '20260904-00_cata_mop_creature_properties');
