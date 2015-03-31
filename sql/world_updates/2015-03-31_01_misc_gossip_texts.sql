/*
********************************************************************
Updates for npc_script_texts
********************************************************************
*/

DELETE FROM `gossip_menu_option` WHERE `entry` BETWEEN 427 AND 435;
INSERT INTO `gossip_menu_option` (`entry`, `option_text`) VALUES
   (428, 'What is this place?'),
   (429, 'Where is Medivh?'),
   (430, 'How do you navigate the tower?'),
   (431, 'Please teleport me to the Guardian\'s Library.'),
   (432, 'I\'m not an actor.'),
   (433, 'Ok, I\'ll give it a try, then.'),
   (434, 'What phat lewts you have Grandmother!');
   
UPDATE `npc_script_text` SET `sound` = 9198 WHERE `entry` = 1998;

UPDATE `gameobject_spawns` SET `Flags` = 0 WHERE `id` = 51572;
UPDATE `gameobject_spawns` SET `Flags` = 0 WHERE `id` = 51572;

DELETE FROM `npc_script_text` WHERE `entry` BETWEEN 8729 AND 8759;
INSERT INTO npc_script_text
   (`entry`, `text`, `creature_entry`, `id`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `broadcast_id`)
VALUES
   (8730, 'Ours is the true horde! The only horde!', 30739, 0, 14, 0, 100, 0, 0, 10323, 0),
   (8731, 'I\'ll carve the meat from your bones!', 30739, 0, 14, 0, 100, 0, 0, 10324, 0),
   (8732, 'I am called Bladefists for a reason... as you will see!', 30739, 0, 14, 0, 100, 0, 0, 10325, 0),
   (8733, 'For the real horde!', 30739, 0, 14, 0, 100, 0, 0, 10326, 0),
   (8734, 'I am the only warchief!', 30739, 0, 14, 0, 100, 0, 0, 10327, 0),
   (8735, 'The true horde... will... prevail!', 30739, 0, 14, 0, 100, 0, 0, 10328, 0),
   (8736, 'You were looking for me, child? Why do you come to kill me, $n? I only wish to help.', 25834, 1, 12, 0, 100, 1, 3500, 0, 0),
   (8737, 'Now that I have been reassembled, we can return to a time of perfection...the time of the Titans!', 25834, 2, 12, 0, 100, 1, 3500, 0, 0),
   (8738, 'But, I can see it in your eyes, hear it in your pulse rate. You would destroy me despite my offer of immortality!', 25834, 3, 12, 0, 100, 25, 2500, 0, 0),
   (8739, 'Very well. It saddens me that it has come to this. I look upon all of you as if you were my children. I will slay you if I must!', 25834, 4, 12, 0, 100, 25, 2500, 0, 0),
   (8740, 'Wretched, meddling insects! Release me, and perhaps I will grant you a merciful death!', 17257, 0, 14, 0, 0, 0, 0, 10247, 0),
   (8741, 'Vermin! Leeches! Take my blood and choke on it!', 17257, 0, 14, 0, 0, 0, 0, 10248, 0),
   (8742, 'Illidan is an arrogant fool! I will crush him and reclaim Outland as my own!', 17257, 0, 14, 0, 0, 0, 0, 10249, 0),
   (8743, 'Away, you mindless parasites! My blood is my own!', 17257, 0, 14, 0, 0, 0, 0, 10250, 0),
   (8744, 'How long do you believe your pathetic sorcery can hold me?', 17257, 0, 14, 0, 0, 0, 0, 10251, 0),
   (8745, 'My blood will be the end of you!', 17257, 0, 14, 0, 0, 0, 0, 10252, 0),
   (8746, 'My blood will be the end of you!', 17257, 0, 14, 0, 0, 0, 0, 10253, 0),
   (8747, 'Thank you for releasing me. Now... die!', 17257, 0, 14, 0, 0, 0, 0, 10254, 0),
   (8748, 'I... am... unleashed!', 17257, 0, 14, 0, 0, 0, 0, 10255, 0),
   (8749, 'Not again... NOT AGAIN!', 17257, 0, 14, 0, 0, 0, 0, 10256, 0),
   (8750, 'The Legion... will consume you... all....', 17257, 0, 14, 0, 0, 0, 0, 10258, 0),
   (8751, 'Did you think me weak? Soft? Who is the weak one now?!', 17257, 0, 14, 0, 0, 0, 0, 10255, 0),
   (8752, 'I will not be taken so easily. Let the walls of this prison tremble... and FALL!!!', 17257, 0, 14, 0, 0, 0, 0, 10257, 0),
   (8753, 'Ha! Much better!', 20886, 0, 14, 0, 0, 0, 0, 11240, 0),
   (8754, 'Drakkari gonna kill anybody who trespass on these lands!', 29304, 0, 14, 0, 0, 0, 0, 14443, 0),
   (8755, 'None can stand against the Serpent Lords!', 3671, 0, 14, 0, 0, 0, 0, 5786, 0),
   (8756, 'You will never wake the dreamer!', 3669, 0, 14, 0, 0, 0, 0, 5785, 0),
   (8757, 'The coils of death... Will crush you!', 3670, 0, 14, 0, 0, 0, 0, 5787, 0),
   (8758, 'I am the serpent king, i can do anything!', 3673, 0, 14, 0, 0, 0, 0, 5788, 0);

   
UPDATE `world_db_version` SET `LastUpdate` = '2015-03-31_01_misc_gossip_texts';
