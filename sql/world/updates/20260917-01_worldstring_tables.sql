UPDATE `worldstring_tables` SET `text`='You must be at least level {} to pass through here.' WHERE  `entry`=31;
UPDATE `worldstring_tables` SET `text`='You must have the item, `{}` to pass through here.' WHERE  `entry`=35;
UPDATE `worldstring_tables` SET `text`='One minute until the battle for {} begins!' WHERE  `entry`=46;
UPDATE `worldstring_tables` SET `text`='Thirty seconds until the battle for {} begins!' WHERE  `entry`=47;
UPDATE `worldstring_tables` SET `text`='Fifteen seconds until the battle for {} begins!' WHERE  `entry`=48;
UPDATE `worldstring_tables` SET `text`='The battle for {} has begun!' WHERE  `entry`=49;
UPDATE `worldstring_tables` SET `text`='Your queue on battleground instance id {} is no longer valid. Reason: Instance Deleted.' WHERE  `entry`=52;
UPDATE `worldstring_tables` SET `text`='Your queue on battleground instance {} is no longer valid, the instance no longer exists.' WHERE  `entry`=54;
UPDATE `worldstring_tables` SET `text`='You must have the quest, \'{}\' completed to pass through here.' WHERE  `entry`=81;
UPDATE `worldstring_tables` SET `text`='You have been banned for: {}' WHERE  `entry`=502;




INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('197', '20260917-01_worldstring_tables');
