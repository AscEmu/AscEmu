/*
********************************************************************
Updates for gossip_meu_option
********************************************************************
*/

INSERT INTO `gossip_menu_option` (`entry`, `option_text`) VALUES
   (428, 'What is this place?'),
   (429, 'Where is Medivh?'),
   (430, 'How do you navigate the tower?'),
   (431, 'Please teleport me to the Guardian\'s Library.'),
   (432, 'I\'m not an actor.'),
   (433, 'Ok, I\'ll give it a try, then.'),
   (434, 'What phat lewts you have Grandmother!');
   
UPDATE `npc_script_text` SET `sound` = 9198 WHERE `entry` = 1998;

INSERT INTO npc_script_text
   (`entry`, `text`, `creature_entry`, `id`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `broadcast_id`)
VALUES
   (8735, 'Wretched, meddling insects! Release me, and perhaps I will grant you a merciful death!', 17257, 0, 14, 0, 0, 0, 0, 10247, 0),
   (8736, 'Vermin! Leeches! Take my blood and choke on it!', 17257, 0, 14, 0, 0, 0, 0, 10248, 0),
   (8737, 'Illidan is an arrogant fool! I will crush him and reclaim Outland as my own!', 17257, 0, 14, 0, 0, 0, 0, 10249, 0),
   (8738, 'Away, you mindless parasites! My blood is my own!', 17257, 0, 14, 0, 0, 0, 0, 10250, 0),
   (8739, 'How long do you believe your pathetic sorcery can hold me?', 17257, 0, 14, 0, 0, 0, 0, 10251, 0),
   (8740, 'My blood will be the end of you!', 17257, 0, 14, 0, 0, 0, 0, 10252, 0),
   (8741, 'My blood will be the end of you!', 17257, 0, 14, 0, 0, 0, 0, 10253, 0),
   (8742, 'Thank you for releasing me. Now... die!', 17257, 0, 14, 0, 0, 0, 0, 10254, 0),
   (8743, 'I... am... unleashed!', 17257, 0, 14, 0, 0, 0, 0, 10255, 0),
   (8744, 'Not again... NOT AGAIN!', 17257, 0, 14, 0, 0, 0, 0, 10256, 0),
   (8745, 'The Legion... will consume you... all....', 17257, 0, 14, 0, 0, 0, 0, 10258, 0),
   (8746, 'Did you think me weak? Soft? Who is the weak one now?!', 17257, 0, 14, 0, 0, 0, 0, 10255, 0),
   (8747, 'I will not be taken so easily. Let the walls of this prison tremble... and FALL!!!', 17257, 0, 14, 0, 0, 0, 0, 10257, 0);
   
UPDATE `world_db_version` SET `LastUpdate` = '2015-03-30_01_misc_gossip_texts';

