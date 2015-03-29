/*
********************************************************************
Updates for npc_script_text
********************************************************************
*/

UPDATE `gameobject_spawns` SET `Flags` = 0 WHERE `id` = 51572;

UPDATE `npc_script_text` SET `duration` = 3500, `emote` = 1 WHERE `entry` = 8730;
UPDATE `npc_script_text` SET `duration` = 3500, `emote` = 1 WHERE `entry` = 8731;
UPDATE `npc_script_text` SET `duration` = 2500, `emote` = 25 WHERE `entry` = 8732;
UPDATE `npc_script_text` SET `duration` = 2500, `emote` = 25 WHERE `entry` = 8733;
