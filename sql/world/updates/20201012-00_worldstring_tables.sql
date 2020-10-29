-- update
UPDATE `worldstring_tables` SET `text`='This character is not allowed to play.' WHERE  `entry`=501;

-- update world_db_version

INSERT INTO `world_db_version` VALUES ('60', '20201012-00_worldstring_tables');
