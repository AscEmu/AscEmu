ALTER TABLE `ai_agents` ADD COLUMN `comments` TEXT NOT NULL AFTER `Misc2`;

--
-- Update world_db_version
--

UPDATE `world_db_version` SET `LastUpdate`='2017-01-11_01_ai_agents' WHERE  `LastUpdate`='2017-01-08_01_npc_script_text';