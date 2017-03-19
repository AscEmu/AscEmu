INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `broadcast_id`) VALUES (8760, 'Aah, fresh meat!', 1716, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `broadcast_id`) VALUES (8759, 'I\'ll crush you!', 1717, 0, 12, 0, 100, 0, 0, 0, 0);

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2017-01-08_01_npc_script_text' WHERE `LastUpdate` = '2016-11-03_01_misc_spelltargetconstraints';
