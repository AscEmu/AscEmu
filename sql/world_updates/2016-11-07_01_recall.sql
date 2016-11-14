--
-- New recall locations
--
REPLACE INTO `recall` VALUES
   (803, 'DarkmoonFaire', 974, -4202.26, 6346.8, 9.7129, 3.25583),
   (804, 'RuinsOfGilneas', 0, -1496.84, 1617.89, 20.4854, 0.817762),
   (805, 'GilneasCity', 0, -1703.84, 1419.06, 21.6703, 0.719589),
   (806, 'TheForsakenFront', 0, -198.45, 1273.47, 40.4208, 2.0155),
   (807, 'BaradinHold', 732, -1208.18, 980.94, 119.728, 3.21604),
   (808, 'TolBaradPeninsula', 732, -281.595, 1362.91, 22.7936, 1.84159),
   (809, 'TwilightHighlands', 0, -3493.13, -4912.95, 77.1887, 0.522433),
   (810, 'MountHyjal', 1, 5075.76, -3201.27, 1889.44, 1.41445);

   
--
-- worldmap_info
--
REPLACE INTO `worldmap_info` VALUES
   (974, 0, 0, 0, 1, 0, 0, 0, 0, 0, 'Darkmoon Faire', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0),
   (732, 0, 0, 0, 1, 0, 0, 0, 0, 0, 'Baradin Hold', 0, 0, 0, 0, 0, 0, 0, 0, 80, 0);

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-11-07_01_recall' WHERE `LastUpdate` = '2016-10-16_02_spell_defines';
