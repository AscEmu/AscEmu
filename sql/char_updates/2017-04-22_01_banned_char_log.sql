--
-- Add table banned_char_log
--
DROP TABLE IF EXISTS `banned_char_log`;
CREATE TABLE `banned_char_log` (
  `banned_by` varchar(50) NOT NULL,
  `banned_player` varchar(50) NOT NULL,
  `timestamp` int(11) NOT NULL,
  `banned_until` int(11) NOT NULL,
  `reason` varchar(150) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Update char_db_version
--
UPDATE `character_db_version` SET `LastUpdate` = '2017-04-22_01_banned_char_log' WHERE `LastUpdate` = '2017-03-21_01_characters';
