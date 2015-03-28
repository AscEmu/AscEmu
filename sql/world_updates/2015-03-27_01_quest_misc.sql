/*
********************************************************************
Updates for creature_spawns
********************************************************************
*/

UPDATE `gameobject_spawns` SET `Flags` = 0 WHERE `id` = 51572;

INSERT INTO npc_script_text
   (`entry`, `text`, `creature_entry`, `id`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `broadcast_id`)
VALUES
   (8730, 'You were looking for me, child? Why do you come to kill me, $n? I only wish to help.', 25834, 1, 12, 0, 100, 0, 0, 0, 0),
   (8731, 'Now that I have been reassembled, we can return to a time of perfection...the time of the Titans!', 25834, 2, 12, 0, 100, 29, 0, 0, 0),
   (8732, 'But, I can see it in your eyes, hear it in your pulse rate. You would destroy me despite my offer of immortality!', 25834, 3, 12, 0, 100, 29, 0, 0, 0),
   (8733, 'Very well. It saddens me that it has come to this. I look upon all of you as if you were my children. I will slay you if I must!', 25834, 4, 12, 0, 100, 0, 0, 0, 0);
