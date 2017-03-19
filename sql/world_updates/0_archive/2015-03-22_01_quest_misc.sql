/*
********************************************************************
Updates for gossip_menu_option
********************************************************************
*/

DELETE FROM `gossip_menu_option` WHERE `entry` = 191;
INSERT INTO gossip_menu_option
   (`entry`, `option_text`)
VALUES
   (191, 'May I use a drake to fly elsewhere?');
