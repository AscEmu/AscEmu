--
-- Create table gm_survey
--
DROP TABLE IF EXISTS `gm_survey`;
CREATE TABLE `gm_survey` (
  `survey_id` int(10) unsigned NOT NULL,
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `main_survey` int(10) unsigned NOT NULL DEFAULT '0',
  `comment` longtext NOT NULL,
  `create_time` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`survey_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='GM Survey';

--
-- Create table gm_survey_answers
--
DROP TABLE IF EXISTS `gm_survey_answers`;
CREATE TABLE `gm_survey_answers` (
  `survey_id` int(10) unsigned NOT NULL,
  `question_id` int(10) unsigned NOT NULL DEFAULT '0',
  `answer_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`survey_id`,`question_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='GM Survey';

--
-- Update char_db_version
--
UPDATE `character_db_version` SET `LastUpdate` = '2016-03-28_01_gm_survey';