INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `probability`, `broadcast_id`) VALUES ('10759', 'Okay, okay... gimmie a minute to rest now. You gone and beat me up good.', '6784', '1', '12', '100', '3043');
INSERT INTO `creature_quest_starter` (`id`, `quest`) VALUES ('6784', '590');
UPDATE `quest_properties` SET `PrevQuestId`='8' WHERE (`entry`='590') AND (`build`='4044');

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('106', '20220521-00_npc_script_text');
