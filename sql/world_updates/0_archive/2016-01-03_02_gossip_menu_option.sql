--
-- Updates for gossip_menu_option
--
INSERT INTO `gossip_menu_option` (`entry`, `option_text`)
VALUES
   (603, 'I have reason to believe you\'re involved in cultist activity.'),
   (604, 'What do you know about the Cult of the Damned?'),
   (605, 'How long have you worked for the Cult of the Damned?');

--
-- Update world db version
--   
UPDATE `world_db_version` SET `LastUpdate` = '2016-01-03_02_gossip_menu_option' WHERE `LastUpdate` = '2016-01-03_01_creature_proto_difficulty';

