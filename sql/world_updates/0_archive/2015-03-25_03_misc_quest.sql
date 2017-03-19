/*
********************************************************************
Updates for creature_spawns (misc quests)
********************************************************************
*/

DELETE FROM `creature_spawns` WHERE `id` = 199005;

INSERT INTO creature_spawns
   (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`)
VALUES
   (199005, 26225, 571, 3602.77, 3655.36, 35.94, 2.55, 0, 10553, 1975, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
   