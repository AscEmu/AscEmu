/*
********************************************************************
Updates for gossip_meu_option
********************************************************************
*/

DELETE FROM `gossip_menu_option` WHERE `entry` BETWEEN 506 AND 531;
INSERT INTO `gossip_menu_option` (`entry`, `option_text`) VALUES
   (507, 'What do you know of it'),
   (508, 'I am listening, Demitrian.'),
   (509, 'Continue, please.'),
   (510, 'A battle?'),
   (511, '<Nod>'),
   (512, 'Caught unaware? How?'),
   (513, 'Oh what did Ragnaros do next?'),
   (514, 'Teleport me to Dalaran.'),
   (515, 'Teleport to Light\'s Hammer.'),
   (516, 'Teleport to Oratory of The Damned.'),
   (517, 'Teleport to Rampart of Skulls.'),
   (518, 'Teleport to Deathbringer\'s Rise.'),
   (519, 'Teleport to the Upper Spire.'),
   (520, 'Teleport to Sindragosa\'s Lair.'),
   (521, 'Expedition Base Camp.'),
   (522, 'Formation Grounds'),
   (523, 'Colossal Forge'),
   (524, 'Scrapyard'),
   (525, 'Antechamber of Ulduar'),
   (526, 'Shattered Walkway'),
   (527, 'Conservatory of Life'),
   (528, 'Spark of Imagination'),
   (529, 'Prison of Yogg-Saron'),
   (530, 'Rut\'Theran Ferry');

   
UPDATE `world_db_version` SET `LastUpdate` = '2015-04-06_02_gossip_menu_option';

