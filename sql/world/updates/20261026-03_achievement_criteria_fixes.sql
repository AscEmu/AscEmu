-- 8 achievement criteria requirements: anniversaries 4 and 5 for Cata, 3 disabled criteria, item quality of 3 item statistics
UPDATE `achievement_criteria_requirement` SET `max_build` = 15595 WHERE `criteria_id` IN (9058, 12562) AND `min_build` = 12340 AND `requirement` = 42;

REPLACE INTO `achievement_criteria_requirement` (`criteria_id`, `min_build`, `max_build`, `requirement`, `param1`, `param2`) VALUES
(5343, 12340, 15595, 32, 4, 0),
(5344, 12340, 15595, 32, 4, 0),
(5345, 12340, 15595, 32, 5, 0),
(10800, 15595, 15595, 60, 0, 0),
(10801, 15595, 15595, 60, 0, 0),
(11238, 12340, 15595, 60, 0, 0);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('275', '20261026-03_achievement_criteria_fixes');
