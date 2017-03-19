/*
********************************************************************
Misc updates for Ulduar Displacement Pad
********************************************************************
*/

DELETE FROM npc_text WHERE entry IN (31024,14424);

INSERT INTO npc_text
   (`entry`, `prob0`, `text0_0`, `text0_1`, `lang0`, `EmoteDelay0_0`, `Emote0_0`, `EmoteDelay0_1`, `Emote0_1`, `EmoteDelay0_2`, `Emote0_2`, `prob1`, `text1_0`, `text1_1`, `lang1`, `EmoteDelay1_0`, `Emote1_0`, `EmoteDelay1_1`, `Emote1_1`, `EmoteDelay1_2`, `Emote1_2`, `prob2`, `text2_0`, `text2_1`, `lang2`, `EmoteDelay2_0`, `Emote2_0`, `EmoteDelay2_1`, `Emote2_1`, `EmoteDelay2_2`, `Emote2_2`, `prob3`, `text3_0`, `text3_1`, `lang3`, `EmoteDelay3_0`, `Emote3_0`, `EmoteDelay3_1`, `Emote3_1`, `EmoteDelay3_2`, `Emote3_2`, `prob4`, `text4_0`, `text4_1`, `lang4`, `EmoteDelay4_0`, `Emote4_0`, `EmoteDelay4_1`, `Emote4_1`, `EmoteDelay4_2`, `Emote4_2`, `prob5`, `text5_0`, `text5_1`, `lang5`, `EmoteDelay5_0`, `Emote5_0`, `EmoteDelay5_1`, `Emote5_1`, `EmoteDelay5_2`, `Emote5_2`, `prob6`, `text6_0`, `text6_1`, `lang6`, `EmoteDelay6_0`, `Emote6_0`, `EmoteDelay6_1`, `Emote6_1`, `EmoteDelay6_2`, `Emote6_2`, `prob7`, `text7_0`, `text7_1`, `lang7`, `EmoteDelay7_0`, `Emote7_0`, `EmoteDelay7_1`, `Emote7_1`, `EmoteDelay7_2`, `Emote7_2`)
VALUES
   (14424, 1, 'The Displacement Pad appears to be active and stable.', 'The Displacement Pad appears to be active and stable.', 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0);

UPDATE `world_db_version` SET `LastUpdate` = '2015-04-09_01_npc_text';
