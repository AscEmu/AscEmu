/*
********************************************************************
Updates for gossip_meu_option
********************************************************************
*/

DELETE FROM `gossip_menu_option` WHERE `entry` IN (425,426,427);
INSERT INTO `gossip_menu_option` (`entry`, `option_text`) VALUES
   (425, 'I need a pack of Incendiary Bombs.'),
   (426, 'I am ready to go to Durnholde Keep.'),
   (427, '[PH] Start escort.');

