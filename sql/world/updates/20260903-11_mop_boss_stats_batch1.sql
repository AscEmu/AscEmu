-- Real health/mana/level values for a subset of the Mop boss NPCs added earlier today,
-- replacing their stub placeholders (minhealth/maxhealth 20/30, mana 0). Supplied by Zyres.
-- Entries still on the stub values after this: the Stone Guard (4), Will of the Emperor (2),
-- Elegon, three of the four Spirit Kings, Wind Lord Mel'jarak, Amber-Shaper Un'sok, Blade Lord
-- Ta'yak, Imperial Vizier Zor'lok, Tortos, Durumu the Forgotten, Lei Shen, Horridon, and most
-- other Throne of Thunder bosses - not covered by this batch.

UPDATE `creature_properties` SET `mana` = 4296 WHERE `entry` = 1853 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 34, `maxlevel` = 93, `mana` = 5035 WHERE `entry` = 3977 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 43, `maxlevel` = 43, `mana` = 4296 WHERE `entry` = 10503 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 43, `maxlevel` = 43, `mana` = 4296 WHERE `entry` = 11622 AND `build` = 18414;
UPDATE `creature_properties` SET `minhealth` = 9156, `maxhealth` = 9156 WHERE `entry` = 14902 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 1, `maxlevel` = 1, `mana` = 60, `minhealth` = 84, `maxhealth` = 84 WHERE `entry` = 38895 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 91, `maxlevel` = 93, `mana` = 21620 WHERE `entry` = 56589 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 92, `maxlevel` = 93 WHERE `entry` = 56877 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 90, `maxlevel` = 90, `minhealth` = 1969705, `maxhealth` = 1969705 WHERE `entry` = 58466 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 90, `maxlevel` = 90, `mana` = 21262, `minhealth` = 1575765, `maxhealth` = 1575765 WHERE `entry` = 58468 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 90, `maxlevel` = 90, `minhealth` = 1969705, `maxhealth` = 1969705 WHERE `entry` = 58470 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 31, `maxlevel` = 93 WHERE `entry` = 58632 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 43, `maxlevel` = 93, `mana` = 3575 WHERE `entry` = 58633 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 31, `maxlevel` = 93, `mana` = 17560 WHERE `entry` = 59150 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 34, `maxlevel` = 93 WHERE `entry` = 59223 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 31, `maxlevel` = 93 WHERE `entry` = 59303 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 86, `maxlevel` = 93 WHERE `entry` = 59479 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 33, `maxlevel` = 93 WHERE `entry` = 59789 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 89, `maxlevel` = 93 WHERE `entry` = 61243 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 89, `maxlevel` = 93, `mana` = 193840 WHERE `entry` = 61398 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 89, `maxlevel` = 93 WHERE `entry` = 61445 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 91, `maxlevel` = 93 WHERE `entry` = 61485 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 91, `maxlevel` = 93, `mana` = 60504 WHERE `entry` = 61567 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 91, `maxlevel` = 93 WHERE `entry` = 61634 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 90, `maxlevel` = 90, `minhealth` = 393941, `maxhealth` = 393941 WHERE `entry` = 62151 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 91, `maxlevel` = 93 WHERE `entry` = 62205 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 90, `maxlevel` = 90, `mana` = 9916, `minhealth` = 393941, `maxhealth` = 393941 WHERE `entry` = 62773 AND `build` = 18414;
UPDATE `creature_properties` SET `mana` = 150 WHERE `entry` = 62837 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 90, `maxlevel` = 90, `minhealth` = 3939410, `maxhealth` = 3939410 WHERE `entry` = 63071 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 90, `maxlevel` = 90, `minhealth` = 393941, `maxhealth` = 393941 WHERE `entry` = 63072 AND `build` = 18414;
UPDATE `creature_properties` SET `minlevel` = 90, `maxlevel` = 90, `mana` = 21262, `minhealth` = 315153, `maxhealth` = 315153 WHERE `entry` = 63785 AND `build` = 18414;
UPDATE `creature_properties` SET `mana` = 1051400 WHERE `entry` = 68078 AND `build` = 18414;
UPDATE `creature_properties` SET `mana` = 65 WHERE `entry` = 68904 AND `build` = 18414;
UPDATE `creature_properties` SET `mana` = 64 WHERE `entry` = 68905 AND `build` = 18414;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('179', '20260903-11_mop_boss_stats_batch1');
