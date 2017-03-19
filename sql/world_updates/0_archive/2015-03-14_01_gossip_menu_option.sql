/*
********************************************************************
Updates for gossip_meu_option
********************************************************************
*/

DELETE FROM `gossip_menu_option` WHERE `entry` IN (423,424);
INSERT INTO `gossip_menu_option` (`entry`, `option_text`) VALUES
   (423, 'Please Teleport me to Darnassus.'),
   (424, 'I wish to leave this horrible place.');
