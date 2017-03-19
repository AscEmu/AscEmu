/*
********************************************************************
Updates for npc_script_text
********************************************************************
*/

DELETE FROM `npc_script_text` WHERE `entry` = 8729;
INSERT INTO npc_script_text
   (`entry`, `text`, `creature_entry`, `id`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `broadcast_id`)
VALUES
   (8729, 'Who dares interfere with the Sons of Arugal?', 4274, 0, 14, 0, 100, 0, 0, 5791, 0);

