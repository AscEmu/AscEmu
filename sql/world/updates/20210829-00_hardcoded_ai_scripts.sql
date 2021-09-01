-- Update moved scripts to correct event
UPDATE `creature_ai_scripts` SET event = 5 WHERE min_build = 5875 AND event = 0;
-- fix Deviate Guardian aoe spell (not on load)
UPDATE `creature_ai_scripts` SET event = 5 WHERE entry = 3637 AND event = 0;
-- fix old ai_agents so it would work with new SYSTEM
UPDATE `creature_ai_scripts` SET event = 5 WHERE min_build = 12340 AND event = 0 AND action = 1;

-- Add Vectus and Theolen Krastinov from PR
-- 28371
INSERT INTO creature_ai_scripts VALUES ('5875', '12340', '10432', '4', '0', '5', '1', '1', '100', '8269', '10', '1', '0', '0', '0', '0', '25', '2290', '0', 'Vectus-Cast Frenzy at 25 percent');
INSERT INTO creature_ai_scripts VALUES ('5875', '12340', '10432', '4', '0', '5', '1', '0', '10', '19626', '10', '0', '0', '0', '10000', '0', '100', '0', '0', 'Vectus-Cast Fire Shield onAIUpdate');
INSERT INTO creature_ai_scripts VALUES ('5875', '12340', '10432', '4', '0', '5', '1', '0', '100', '16046', '8', '0', '0', '0', '14000', '0', '100', '0', '0', 'Vectus-cast Blast Wave onAIUpdate');

-- 28371
INSERT INTO creature_ai_scripts VALUES ('5875', '12340', '11261', '4', '0', '5', '1', '0', '100', '8269', '10', '1', '0', '0', '120000', '0', '100', '2289', '0', 'Theolen Krastinov-Cast Frenzy onAIUpdate');
INSERT INTO creature_ai_scripts VALUES ('5875', '12340', '11261', '4', '0', '5', '1', '0', '60', '16509', '10', '1', '2', '0', '8000', '0', '100', '0', '0', 'Theolen Krastinov-Cast Rend onAIUpdate');
INSERT INTO creature_ai_scripts VALUES ('5875', '12340', '11261', '4', '0', '5', '1', '0', '60', '18103', '10', '1', '2', '0', '10000', '0', '100', '0', '0', 'Theolen Krastinov-Cast Backhand onAIUpdate');
-- 28371
INSERT INTO creature_ai_scripts VALUES ('5875', '12340', '11261', '4', '0', '1', '1', '0', '100', '8269', '10', '1', '0', '0', '0', '0', '100', '2289', '0', 'Theolen Krastinov-cast Frenzy onEnterCombat');


-- Shadowfang Keep (33)
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 2529, 4, 0, 5, 1, 0, 5, 7124, 0, 0, 2, 300000, 300000, 0, 100, 0, 0, 'Son of Arugal - Arugals Gift');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3853, 4, 0, 5, 1, 0, 5, 7121, 0, 0, 0, 10000, 10000, 0, 100, 0, 0, 'Shadowfang Moonwalker - Anti-Magic Shield');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3855, 4, 0, 5, 1, 0, 8, 8140, 0, 0, 6, 15000, 15000, 0, 100, 0, 0, 'Shadowfang Darksoul - Befuddlement');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3855, 4, 0, 5, 1, 0, 5, 970, 0, 0, 6, 18000, 18000, 0, 100, 0, 0, 'Shadowfang Darksoul - Shadow Word: Pain');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3857, 4, 0, 5, 1, 0, 5, 7122, 0, 0, 2, 0, 0, 0, 100, 0, 0, 'Shadowfang Glutton - Blood Tap');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3861, 4, 0, 5, 1, 0, 5, 7127, 0, 0, 6, 60000, 60000, 0, 100, 0, 0, 'Bleak Worg - Wavering Will');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3863, 4, 0, 5, 1, 0, 5, 7132, 0, 0, 0, 240000, 240000, 0, 100, 0, 0, 'Lupine Horror - Summon Lupine Delusions');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3864, 4, 0, 5, 1, 0, 5, 7139, 0, 0, 2, 3000, 3000, 0, 100, 0, 0, 'Fel Steed - Fel Stomp');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3866, 4, 0, 5, 1, 0, 8, 7145, 0, 0, 2, 0, 0, 0, 100, 0, 0, 'Vile Bat - Diving Sweep');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3866, 4, 0, 5, 1, 0, 5, 6713, 0, 0, 2, 6000, 6000, 0, 100, 0, 0, 'Vile Bat - Disarm');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3868, 4, 0, 5, 1, 0, 5, 7140, 0, 0, 2, 5000, 5000, 0, 100, 0, 0, 'Blood Seeker - Expose Weakness');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3872, 4, 0, 5, 1, 0, 5, 9080, 0, 0, 2, 10000, 10000, 0, 100, 0, 0, 'Deathsworn Captain - Hamstring');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3872, 4, 0, 5, 1, 0, 8, 40505, 0, 0, 2, 10000, 10000, 0, 100, 0, 0, 'Deathsworn Captain - Cleave');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3873, 4, 0, 5, 1, 0, 5, 7054, 0, 0, 2, 300000, 300000, 0, 100, 0, 0, 'Tormented Officer - Forsaken Skills');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3875, 4, 0, 5, 1, 0, 5, 7057, 0, 0, 2, 300000, 300000, 0, 100, 0, 0, 'Haunted Servitor - Haunting Spirits');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3877, 4, 0, 5, 1, 0, 5, 7074, 0, 0, 0, 5000, 5000, 0, 100, 0, 0, 'Wailing Guardsman - Screams of the Past');

-- Uldaman (70)
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 2748, 4, 0, 5, 1, 0, 20, 6524, 0, 0, 2, 10000, 10000, 0, 100, 0, 0, 'Archaedas - Ground Tremor');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 4854, 4, 0, 5, 1, 0, 20, 8292, 0, 1, 2, 10000, 10000, 0, 100, 0, 0, 'Grimlok - Chain Bolt');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 4854, 4, 0, 5, 1, 0, 20, 10392, 0, 1, 2, 10000, 10000, 0, 100, 0, 0, 'Grimlok - Lightning Bolt');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 4854, 4, 0, 5, 1, 0, 20, 8066, 0, 1, 2, 10000, 10000, 0, 100, 0, 0, 'Grimlok - Shrink');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 6906, 4, 0, 5, 1, 0, 20, 15655, 0, 0, 2, 10000, 10000, 0, 100, 0, 0, 'Baelog - Shield Slam');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 6910, 4, 0, 5, 1, 0, 20, 10392, 0, 1, 2, 10000, 10000, 0, 100, 0, 0, 'Revelosh - Lightning Bolt');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 6910, 4, 0, 5, 1, 0, 20, 2860, 0, 1, 2, 10000, 10000, 0, 100, 0, 0, 'Revelosh - Chain Lightning');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 7228, 4, 0, 5, 1, 0, 20, 16169, 0, 1, 2, 10000, 10000, 0, 100, 0, 0, 'Ironaya - Arcing Smash');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 7228, 4, 0, 5, 1, 0, 20, 24375, 0, 1, 2, 10000, 10000, 0, 100, 0, 0, 'Ironaya - War Stomp');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 7291, 4, 0, 5, 1, 0, 20, 10448, 0, 1, 2, 10000, 10000, 0, 100, 0, 0, 'Galgann Firehammer - Flame Shock');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 7291, 4, 0, 5, 1, 0, 20, 18958, 0, 1, 2, 10000, 10000, 0, 100, 0, 0, 'Galgann Firehammer - Flame Lash');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 7291, 4, 0, 5, 1, 0, 20, 12470, 0, 1, 2, 10000, 10000, 0, 100, 0, 0, 'Galgann Firehammer - Fire Nova');

-- Wailing Caverns (43)
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3654, 4, 0, 5, 1, 0, 15, 8150, 0, 0, 0, 2500, 2500, 0, 100, 0, 0, 'Mutanus the Devourer - Thundercrack');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3654, 4, 0, 5, 1, 0, 15, 7399, 0, 0, 6, 4000, 4000, 0, 100, 0, 0, 'Mutanus the Devourer - Terrify');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3669, 4, 0, 5, 1, 0, 30, 9532, 0, 0, 2, 0, 0, 0, 100, 0, 0, 'Lord Cobrahn - Lightning Bolt');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3669, 4, 0, 5, 1, 0, 15, 34969, 0, 0, 2, 30000, 30000, 0, 100, 0, 0, 'Lord Cobrahn - Poison');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3669, 4, 0, 5, 1, 1, 100, 7965, 0, 0, 0, 300000, 300000, 0, 20, 0, 0, 'Lord Cobrahn - Cobrahn Serpent Form');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3670, 4, 0, 5, 1, 0, 30, 9532, 0, 0, 2, 0, 0, 0, 100, 0, 0, 'Lord Pythas - Lightning Bolt');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3670, 4, 0, 5, 1, 0, 10, 700, 0, 0, 6, 20000, 20000, 0, 100, 0, 0, 'Lord Pythas - Sleep');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3670, 4, 0, 5, 1, 0, 20, 8147, 0, 0, 0, 5000, 5000, 0, 100, 0, 0, 'Lord Pythas - Thunderclap');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3671, 4, 0, 5, 1, 0, 30, 9532, 0, 0, 2, 0, 0, 0, 100, 0, 0, 'Lady Anacondra - Lightning Bolt');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3671, 4, 0, 5, 1, 0, 10, 700, 0, 0, 6, 20000, 20000, 0, 100, 0, 0, 'Lady Anacondra - Sleep');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3673, 4, 0, 5, 1, 0, 30, 9532, 0, 0, 2, 0, 0, 0, 100, 0, 0, 'Lord Serpentis - Lightning Bolt');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3673, 4, 0, 5, 1, 0, 10, 700, 0, 0, 6, 20000, 20000, 0, 100, 0, 0, 'Lord Serpentis - Sleep');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3674, 4, 0, 5, 1, 0, 50, 6254, 0, 0, 2, 0, 0, 0, 100, 0, 0, 'Skum - Chained Bolt');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3840, 4, 0, 5, 1, 0, 5, 8041, 0, 0, 0, 10000, 10000, 0, 50, 0, 0, 'Druid of the Fang - Serpent Form');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3840, 4, 0, 5, 1, 0, 5, 5187, 0, 0, 0, 0, 0, 0, 5, 0, 0, 'Druid of the Fang - Healing Touch');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3840, 4, 0, 5, 1, 0, 30, 9532, 0, 0, 2, 0, 0, 0, 100, 0, 0, 'Druid of the Fang - Lightning Bolt');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3840, 4, 0, 5, 1, 0, 20, 8040, 0, 0, 6, 15000, 15000, 0, 100, 0, 0, 'Druid of the Fang - Druids Slumber');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 5775, 4, 0, 5, 1, 0, 30, 8142, 0, 0, 2, 10000, 10000, 0, 100, 0, 0, 'Verdan the Everliving - Grasping Vines');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3671, 4, 0, 1, 2, 1, 100, 0, 0, 0, 0, 0, 0, 0, 100, 8755, 0, 'Lady Anacomdra - onEnterCombat Say');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3673, 4, 0, 1, 2, 1, 100, 0, 0, 0, 0, 0, 0, 0, 100, 8758, 0, 'Lord Serpentis - onEnterCombat Say');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 3670, 4, 0, 1, 0, 1, 100, 0, 0, 0, 0, 0, 0, 0, 100, 8757, 0, 'Lord Pythas - onEnterCombat Say');

-- Blackwing Lair (469)
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11981, 4, 0, 5, 1, 0, 10, 37319, 0, 0, 1, 0, 0, 0, 100, 0, 0, 'Flamegor - Wing Buffet');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11981, 4, 0, 5, 1, 0, 20, 23462, 0, 0, 1, 0, 0, 0, 100, 0, 0, 'Flamegor - Fire Nova');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11983, 4, 0, 5, 1, 0, 10, 37319, 0, 0, 1, 0, 0, 0, 100, 0, 0, 'Firemaw - Wing Buffet');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11983, 4, 0, 5, 1, 0, 15, 23341, 0, 0, 1, 20000, 20000, 0, 100, 0, 0, 'Firemaw - Flame Buffet');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12017, 4, 0, 5, 1, 0, 15, 39001, 0, 0, 1, 8000, 8000, 0, 100, 0, 0, 'Broodlord Lashlayer - Blast Wave');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12459, 4, 0, 5, 1, 0, 10, 19717, 0, 0, 3, 6000, 6000, 0, 100, 0, 0, 'Blackwing Warlock - Rain of Fire');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12459, 4, 0, 5, 1, 0, 15, 36986, 0, 0, 3, 0, 0, 0, 100, 0, 0, 'Blackwing Warlock - Shadow Bolt');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12463, 4, 0, 5, 1, 0, 15, 22423, 0, 0, 2, 12000, 12000, 0, 100, 0, 0, 'Death Talon Flamescale - Flame Shock');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12465, 4, 0, 5, 1, 0, 40, 36742, 0, 0, 1, 3000, 3000, 0, 100, 0, 0, 'Death Talon Wyrmkin - Fireball Volley');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12467, 4, 0, 5, 1, 0, 15, 25050, 0, 0, 2, 120000, 120000, 0, 100, 0, 0, 'Death Talon Captain - Mark of Flames');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12467, 4, 0, 5, 1, 0, 4, 22438, 0, 0, 2, 30000, 30000, 0, 100, 0, 0, 'Death Talon Captain - Mark of Detonation');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 13020, 4, 0, 1, 1, 1, 100, 23513, 0, 0, 1, 180000, 180000, 0, 100, 0, 0, 'Vaelastrasz the Corrupt - Essence of the Red');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 13020, 4, 0, 5, 1, 0, 15, 18435, 0, 0, 1, 0, 0, 0, 100, 0, 0, 'Vaelastrasz the Corrupt - Flame Breath');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 13020, 4, 0, 5, 1, 0, 3, 18173, 0, 0, 2, 20000, 20000, 0, 100, 0, 0, 'Vaelastrasz the Corrupt - Burning Adrenaline');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 13996, 4, 0, 5, 1, 0, 25, 30217, 0, 0, 2, 3000, 3000, 0, 100, 0, 0, 'Blackwing Technician - Adamantite Grenade');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 14601, 4, 0, 5, 1, 0, 10, 37319, 0, 0, 1, 0, 0, 0, 100, 0, 0, 'Ebonroc - Wing Buffet');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 14601, 4, 0, 5, 1, 0, 15, 23340, 0, 0, 2, 8000, 8000, 0, 100, 0, 0, 'Ebonroc - Shadow of Ebonroc');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12017, 4, 0, 1, 2, 1, 100, 0, 0, 0, 0, 0, 0, 0, 100, 2287, 0, 'Broodlord Lashlayer - onEnterCombat Say');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 13020, 4, 0, 4, 2, 1, 100, 0, 0, 0, 0, 0, 0, 0, 100, 2296, 0, 'Vaelastrasz the Corrupt - onTargetDied Say');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 13020, 4, 0, 5, 2, 1, 100, 0, 0, 0, 0, 0, 0, 0, 15, 2295, 0, 'Vaelastrasz the Corrupt - on 15 percent Say');

-- Molten Core (409)
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11658, 4, 0, 5, 1, 0, 10, 31900, 0, 0, 2, 5000, 5000, 0, 100, 0, 0, 'Molten Giant - Stomp');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11658, 4, 0, 5, 1, 0, 10, 30056, 0, 0, 0, 5000, 5000, 0, 100, 0, 0, 'Molten Giant - Knockback');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11659, 4, 0, 5, 1, 0, 12, 19129, 0, 0, 0, 2000, 2000, 0, 100, 0, 0, 'Molten Destroyer - Massive Tremor');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11659, 4, 0, 5, 1, 0, 12, 13360, 0, 0, 2, 2000, 2000, 0, 100, 0, 0, 'Molten Destroyer - Knockdown');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11661, 4, 0, 5, 1, 0, 8, 25051, 0, 0, 2, 20000, 20000, 0, 100, 0, 0, 'Flamewaker - Sunder Armor');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11661, 4, 0, 5, 1, 0, 8, 20277, 0, 0, 0, 4000, 4000, 0, 100, 0, 0, 'Flamewaker - Fist of Ragnaros');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11661, 4, 0, 5, 1, 0, 14, 11998, 0, 0, 2, 0, 0, 0, 100, 0, 0, 'Flamewaker - Strike');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11666, 4, 0, 5, 1, 0, 10, 19631, 0, 0, 0, 60000, 60000, 0, 100, 0, 0, 'Firewalker - Melt Armor');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11666, 4, 0, 5, 1, 0, 10, 19635, 0, 0, 0, 60000, 60000, 0, 100, 0, 0, 'Firewalker - Incite Flames');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11666, 4, 0, 5, 1, 0, 10, 19636, 0, 0, 0, 6000, 6000, 0, 100, 0, 0, 'Firewalker - Fire Blossom');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11667, 4, 0, 0, 1, 0, 100, 19627, 0, 0, 0, 0, 0, 0, 100, 0, 0, 'Flameguard - Fire Shield');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11667, 4, 0, 3, 1, 1, 100, 19628, 0, 0, 0, 0, 0, 0, 100, 0, 0, 'Flameguard - Flames');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11668, 4, 0, 5, 1, 0, 20, 19392, 0, 0, 0, 10000, 10000, 0, 100, 0, 0, 'Firelord - Summon Lava Spawn');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11668, 4, 0, 5, 1, 0, 20, 19393, 0, 0, 6, 5000, 5000, 0, 100, 0, 0, 'Firelord - Soul Burn');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11669, 4, 0, 5, 1, 0, 25, 20602, 0, 0, 2, 0, 0, 0, 100, 0, 0, 'Flame Imp - Fire Nova');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11671, 4, 0, 5, 1, 0, 10, 19771, 0, 0, 6, 30000, 30000, 0, 100, 0, 0, 'Core Hound - Serrated Bite');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11673, 4, 0, 5, 1, 0, 20, 19272, 0, 0, 0, 3000, 3000, 0, 100, 0, 0, 'Ancient Core Hound - Lava Breath');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11673, 4, 0, 5, 1, 0, 20, 19319, 0, 0, 0, 0, 0, 0, 100, 0, 0, 'Ancient Core Hound - Vicious Bite');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11982, 4, 0, 5, 1, 0, 8, 19450, 0, 0, 0, 30000, 30000, 0, 100, 0, 0, 'Magmadar - Magma Spit');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11982, 4, 0, 5, 1, 0, 8, 19272, 0, 0, 0, 0, 0, 0, 100, 0, 0, 'Magmadar - Lava Breath');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11982, 4, 0, 5, 1, 0, 8, 19408, 0, 0, 0, 8000, 8000, 0, 100, 0, 0, 'Magmadar - Panic');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11982, 4, 0, 5, 1, 0, 8, 19411, 0, 0, 6, 0, 0, 0, 100, 0, 0, 'Magmadar - Lava Bomb');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11988, 4, 0, 5, 1, 0, 8, 20553, 0, 0, 0, 2000, 2000, 0, 100, 0, 0, 'Golemagg the Incinerator - Golemaggs Trust');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11988, 4, 0, 5, 1, 0, 8, 13880, 0, 0, 0, 30000, 30000, 0, 100, 0, 0, 'Golemagg the Incinerator - Magma Splash');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11988, 4, 0, 5, 1, 0, 8, 20228, 0, 0, 6, 12000, 12000, 0, 100, 0, 0, 'Golemagg the Incinerator - Pyroblast');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 11988, 4, 0, 5, 1, 0, 8, 19798, 0, 0, 0, 0, 0, 0, 100, 0, 0, 'Golemagg the Incinerator - Earthquake');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12056, 4, 0, 5, 1, 0, 8, 19698, 0, 0, 0, 0, 0, 0, 100, 0, 0, 'Baron Geddon - Inferno');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12056, 4, 0, 5, 1, 0, 8, 19659, 0, 0, 0, 300000, 300000, 0, 100, 0, 0, 'Baron Geddon - Ignite Mana');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12056, 4, 0, 5, 1, 0, 8, 20475, 0, 0, 6, 8000, 8000, 0, 100, 0, 0, 'Baron Geddon - Living Bomb');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12057, 4, 0, 5, 1, 0, 10, 19492, 0, 0, 0, 0, 0, 0, 100, 0, 0, 'Garr - Antimagic Pulse');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12057, 4, 0, 5, 1, 0, 10, 19496, 0, 0, 0, 15000, 15000, 0, 100, 0, 0, 'Garr - Magma Shackles');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12076, 4, 0, 5, 1, 0, 10, 19641, 0, 0, 0, 10000, 10000, 0, 100, 0, 0, 'Lava Elemental - Pyroclast Barrage');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12099, 4, 0, 5, 1, 0, 10, 20294, 0, 0, 2, 21000, 21000, 0, 100, 0, 0, 'Firesworn - Immolate');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12099, 4, 0, 3, 1, 1, 100, 19497, 0, 0, 0, 0, 0, 0, 100, 0, 0, 'Firesworn - onDied Eruption');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12100, 4, 0, 5, 1, 0, 20, 20691, 0, 0, 2, 0, 0, 0, 100, 0, 0, 'Lava Reaver - Cleave');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12101, 4, 0, 5, 1, 0, 20, 25787, 0, 0, 6, 5000, 5000, 0, 100, 0, 0, 'Lava Surger - Surge');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12118, 4, 0, 5, 1, 0, 8, 19702, 0, 0, 0, 10000, 10000, 0, 100, 0, 0, 'Lucifron - Impending Doom');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12118, 4, 0, 5, 1, 0, 8, 19703, 0, 0, 0, 300000, 300000, 0, 100, 0, 0, 'Lucifron - Lucifrons Curse');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12118, 4, 0, 5, 1, 0, 8, 20603, 0, 0, 0, 0, 0, 0, 100, 0, 0, 'Lucifron - Shadow Shock');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12119, 4, 0, 5, 1, 0, 8, 20691, 0, 0, 2, 0, 0, 0, 100, 0, 0, 'Flamewaker Protector - Cleave');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12119, 4, 0, 5, 1, 0, 4, 20740, 0, 0, 6, 15000, 15000, 0, 100, 0, 0, 'Flamewaker Protector - Dominate Mind');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12259, 4, 0, 5, 1, 0, 8, 29317, 0, 0, 6, 0, 0, 0, 100, 0, 0, 'Gehennas - Shadow Bolt');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12259, 4, 0, 5, 1, 0, 8, 19716, 0, 0, 0, 300000, 300000, 0, 100, 0, 0, 'Gehennas - Gehennas Curse');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12259, 4, 0, 5, 1, 0, 4, 19717, 0, 0, 7, 6000, 6000, 0, 100, 0, 0, 'Gehennas - Rain of Fire');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12264, 4, 0, 5, 1, 0, 8, 19713, 0, 0, 0, 300000, 300000, 0, 100, 0, 0, 'Shazzrah - Shazzrahs Curse');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12264, 4, 0, 5, 1, 0, 6, 19714, 0, 0, 0, 30000, 30000, 0, 100, 0, 0, 'Shazzrah - Magic Grounding');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12264, 4, 0, 5, 1, 0, 6, 19715, 0, 0, 0, 10000, 10000, 0, 100, 0, 0, 'Shazzrah - Counterspell');
INSERT INTO `creature_ai_scripts` VALUES (5875, 12340, 12264, 4, 0, 5, 1, 0, 5, 29883, 0, 0, 6, 15000, 15000, 0, 100, 0, 0, 'Shazzrah - Blink');

INSERT INTO `world_db_version` VALUES ('90', '20210829-00_hardcoded_ai_scripts');

