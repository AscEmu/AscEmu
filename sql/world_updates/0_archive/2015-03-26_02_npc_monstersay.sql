/*
********************************************************************
Updates for creature_spawns (misc quests)
********************************************************************
*/

DELETE FROM `npc_monstersay` WHERE `entry` IN (25830,25831,25832,25833);

INSERT INTO npc_monstersay
   (`entry`, `event`, `chance`, `language`, `type`, `monstername`, `text0`, `text1`, `text2`, `text3`, `text4`)
VALUES
   (25830, 0, 100, 0, 12, 'Twonky', 'Twonky!', NULL, NULL, NULL, NULL),
   (25831, 0, 100, 0, 12, 'ED-210', 'ED-210 online!', NULL, NULL, NULL, NULL),
   (25832, 0, 100, 0, 12, 'Max Blasto', 'I am the herald of Mechazod. You will be crushed!', NULL, NULL, NULL, NULL),
   (25833, 0, 100, 0, 12, 'The Grinder', 'Your meddling is at an end. Mechazod will relieve your curse once I am done with you.', NULL, NULL, NULL, NULL);

