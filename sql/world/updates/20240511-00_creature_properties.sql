UPDATE `creature_properties` SET `info_str`='Repair' WHERE  `entry`=43421 AND `build`=15595;

-- Update world db version
INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('138', '20240511-00_creature_properties');
