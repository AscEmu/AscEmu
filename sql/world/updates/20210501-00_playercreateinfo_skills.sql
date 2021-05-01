-- FIX: missing language skills #855
REPLACE INTO `playercreateinfo_skills` VALUES
(84, 109, 15595, 300, 300),
(84, 115, 15595, 300, 300),
(85, 109, 15595, 300, 300),
(85, 673, 15595, 300, 300),
(86, 109, 15595, 300, 300),
(86, 115, 15595, 300, 300),
(89, 109, 15595, 300, 300),
(93, 109, 15595, 300, 300),
(93, 315, 15595, 300, 300),
(94, 109, 15595, 300, 300),
(94, 315, 15595, 300, 300);

-- update world_db_version
REPLACE INTO `world_db_version` VALUES ('69', '20210501-00_playercreateinfo_skills');

