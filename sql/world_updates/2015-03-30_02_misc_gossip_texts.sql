/*
********************************************************************
Updates for npc_script_texts
********************************************************************
*/

INSERT INTO npc_script_text
   (`entry`, `text`, `creature_entry`, `id`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `broadcast_id`)
VALUES
   (8748, 'Ha! Much better!', 20886, 0, 14, 0, 0, 0, 0, 11240, 0),
   (8749, 'Drakkari gonna kill anybody who trespass on these lands!', 29304, 0, 14, 0, 0, 0, 0, 14443, 0),
   (8750, 'None can stand against the Serpent Lords!', 3671, 0, 14, 0, 0, 0, 0, 5786, 0),
   (8751, 'You will never wake the dreamer!', 3669, 0, 14, 0, 0, 0, 0, 5785, 0),
   (8752, 'The coils of death... Will crush you!', 3670, 0, 14, 0, 0, 0, 0, 5787, 0),
   (8753, 'I am the serpent king, i can do anything!', 3673, 0, 14, 0, 0, 0, 0, 5788, 0);

   
UPDATE `world_db_version` SET `LastUpdate` = '2015-03-30_02_misc_gossip_texts';
