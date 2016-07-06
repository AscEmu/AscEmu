--
-- Added missing Item_pages
--

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
-- Dumping data for table ascemu_world2.item_pages: 1'646 rows
/*!40000 ALTER TABLE `item_pages` DISABLE KEYS */;
REPLACE INTO `item_pages` (`entry`, `text`, `next_page`) VALUES
	(3613, 'We live in tumultuous times. War, invasion, and famine have swept the world, but the real trial is yet to come: the end of Azeroth itself.\r\n\r\nHow will you cope with the loss and destruction of everything you hold dear? The answer is that you don\'t have to. Bring your friends and family to one of our gatherings and learn how you can survive the apocalypse together, with us.', 0),
	(3612, 'You\'ve no doubt heard the phrase "all good things must come to an end" and so it is with your life on Azeroth. It won\'t be long before the world erupts into chaos as the elements reclaim their birthright.\r\n\r\nBut you don\'t have to be afraid during these harrowing times. You don\'t have to die. We can help you ascend to a new way of life, beyond death, beyond fear, and beyond the powerlessness of mortality. Join us today.', 0),
	(3611, 'When the flames consume Azeroth, where will you be?\r\n\r\nWe know that no one wants to dwell on their impending doom, but it\'s worth thinking about where you\'ll be spending the hereafter.\r\n\r\nOur sages have predicted that the end of the world is nigh, and only the prepared will survive. We can help you survive the raging inferno that will cleanse this world of the wicked. Isn\'t it time you found some peace of mind?', 0),
	(3589, 'There\'s nothing like wooing the heart of a human girl. Infinitely forgiving, endlessly caring and fantastically fun, human girls have been the downfall of countless heroes throughout the ages. (See Chapter 3: "Jaina Proudmoore and the men who loved her")\n\nHowever, generation after generation has proven it takes something more than just money, looks or an epic suit of armor to attract the woman of your dreams. Not even the power of Gnomish invention can help you here. \n\nTo truly charm the heart of another, you should possess these qualities.\n\n* Be Fun & Friendly\n* Be a Challenge\n* Be a Man\n\n', 3590),
	(3588, 'Let\'s be honest. Since the end of the Third War, Night Elf girls have heard it all.  In fact, they\'d already heard it all long before you or I was born. \n\nIf you want to engage the mind of a Night Elf girl, you\'re going to have to stand out. Sure, we\'ve all heard the tales of Night Elf lasses dancing on mailboxes and stripping to pay for Nightsaber training. True or not, if you want to light that lovely lady\'s lips up with a smile, you gotta be unique, memorable and confident. \n\nStart off by showing that you\'re looking for more than a gal with looks. Sure, she can bounce, she can dance, but can she hold a decent conversation?  Does she even understand the proper use of a samophlange? Does she know how to have a fun time?\n\nThere\'s nothing worse than bringing a Night Elf to a party, only to watch her stand awkwardly by herself, breaking conversation only to lament the loss of her Highborne sister during the War of the Ancients.', 0),
	(3587, 'How to date a Dwarven woman:\n\n1. Ask her to buy you a drink.', 0),
	(3583, 'Are your tastes more exotic? \nDo you desire someone a little out of this world? \nAre hooves your thing, but succubi a little too much for you?\n\nRead on, my friend...', 3584),
	(3581, '"Roleplaying"\n\nGood roleplaying skills are essential. No Genius Gnomish gal wants a giant bore. Regale her with tales of your future cross-continental adventures:\n\n  "You and me, babe, we\'re gonna fly to Kalimdor, etch our names into the side of Teldrassil and spend the rest of our lives swinging from the trees in Un\'goro Crater."\n\n"Storytelling"\n\nShare stores of your exciting future together! The more implausible, the better. Nothing gets a Gnomish girl excited like an ambitious plan. It also makes for great conversation starters!\n\n  "With our brilliant minds combined, we could retake Gnomeregan. ... why haven\'t we retaken Gnomeregan anyways?"', 3582),
	(3560, 'No Data', 0),
	(2875, 'No Data', 0),
	(360, 'The earth trembled as the ancient trees in the enchanted forest were uprooted and toppled. The groves and glades tended by the sons and daughters of Cenarius and the stone towers of the children of the stars were brought to the rolling ground. There was our queen, radiant even in the desperation, in the chaos that was the battles. The enchanted sky changed colors with the discharge of magic, with the explosions that threatened to tear the world asunder. Brother fought brother. Chosen fought ', 366),
	(265, 'Miner Orwell Debt outstanding.  Payment due upon next delivery of ore from Azureload. Miner Fitzgerald Debt outstanding.  Payment due upon next delivery of ore from Azureload. Citizen Netherand All debts paid. Citizen May All debts paid. Foreman Bonds Debt outstanding.  Payment due upon next delivery of ore from Azureload. ', 0),
	(124, '  The Green Hills Of Stranglethorn$b$b              By Hemet Nesingwary', 125),
	(44, 'Having personally known Edwin VanCleef my entire life, I can tell you that facing him as a foe is quite a daunting task.  You see, he was my childhood friend, and I personally trained him in the ways of the shadows thinking that one day he might consider a career alongside me.  If VanCleef is heading up the Defias Brotherhood, may the Light have mercy on our souls. Master Mathias Shaw Stormwind Assassin\'s Guild ', 0),
	(2898, 'No Data', 0),
	(15, 'Hello Morgan,$B$BBusiness in Goldshire is brisk, so brisk that I haven\'t had time to send you any shipments!  $B$BI commissioned the person bearing this note to bring you a package of large wax candles (you know, the ones the Kobolds like to wear on their heads?). $B$BPlease give this person our thanks, and fair payment.', 0);
/*!40000 ALTER TABLE `item_pages` ENABLE KEYS */;
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;

--
-- Delete invalid gameobject_quest_item_binding
--

DELETE FROM `gameobject_quest_item_binding` WHERE entry IN(191819,193417,164,104565,104566,193418);

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-07-06_01_item_pages' WHERE `LastUpdate` = '2016-07-03_02_gameobject_spawns';
