/*
********************************************************************
AscEmu char structure
Last update: 15/09/2016
********************************************************************
*/

--
-- Table structure for table account_data
--
CREATE TABLE IF NOT EXISTS `account_data` (
  `acct` int(30) NOT NULL,
  `uiconfig0` blob,
  `uiconfig1` blob,
  `uiconfig2` blob,
  `uiconfig3` blob,
  `uiconfig4` blob,
  `uiconfig5` blob,
  `uiconfig6` blob,
  `uiconfig7` blob,
  `uiconfig8` blob,
  PRIMARY KEY (`acct`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table account_forced_permissions
--
CREATE TABLE IF NOT EXISTS `account_forced_permissions` (
  `login` varchar(50) NOT NULL,
  `permissions` varchar(100) NOT NULL,
  PRIMARY KEY (`login`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table arenateams
--
CREATE TABLE IF NOT EXISTS `arenateams` (
  `id` int(30) NOT NULL,
  `type` int(30) NOT NULL,
  `leader` int(30) NOT NULL,
  `name` varchar(150) NOT NULL,
  `emblemstyle` int(40) NOT NULL,
  `emblemcolour` bigint(40) NOT NULL,
  `borderstyle` int(40) NOT NULL,
  `bordercolour` bigint(40) NOT NULL,
  `backgroundcolour` bigint(40) NOT NULL,
  `rating` int(30) NOT NULL,
  `data` varchar(150) NOT NULL,
  `ranking` int(30) NOT NULL,
  `player_data1` varchar(60) NOT NULL,
  `player_data2` varchar(60) NOT NULL,
  `player_data3` varchar(60) NOT NULL,
  `player_data4` varchar(60) NOT NULL,
  `player_data5` varchar(60) NOT NULL,
  `player_data6` varchar(60) NOT NULL,
  `player_data7` varchar(60) NOT NULL,
  `player_data8` varchar(60) NOT NULL,
  `player_data9` varchar(60) NOT NULL,
  `player_data10` varchar(60) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table auctions
--
CREATE TABLE IF NOT EXISTS `auctions` (
  `auctionId` int(32) NOT NULL AUTO_INCREMENT,
  `auctionhouse` int(32) DEFAULT NULL,
  `item` bigint(10) DEFAULT NULL,
  `owner` bigint(10) DEFAULT NULL,
  `startbid` int(32) DEFAULT NULL,
  `buyout` int(32) DEFAULT NULL,
  `time` int(32) DEFAULT NULL,
  `bidder` bigint(10) DEFAULT NULL,
  `bid` int(32) DEFAULT NULL,
  `deposit` int(32) DEFAULT NULL,
  PRIMARY KEY (`auctionId`),
  KEY `b` (`auctionhouse`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table banned_names
--
CREATE TABLE IF NOT EXISTS `banned_names` (
  `name` varchar(30) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Table structure for table calendar_events
--
CREATE TABLE IF NOT EXISTS `calendar_events` (
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `creator` int(10) unsigned NOT NULL DEFAULT '0',
  `title` varchar(255) NOT NULL DEFAULT '',
  `description` varchar(255) NOT NULL DEFAULT '',
  `type` tinyint(1) unsigned NOT NULL DEFAULT '4',
  `dungeon` int(10) NOT NULL DEFAULT '-1',
  `date` int(10) unsigned NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table calendar_invites
--
CREATE TABLE IF NOT EXISTS `calendar_invites` (
  `id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `event` bigint(20) unsigned NOT NULL DEFAULT '0',
  `invitee` int(10) unsigned NOT NULL DEFAULT '0',
  `sender` int(10) unsigned NOT NULL DEFAULT '0',
  `status` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `statustime` int(10) unsigned NOT NULL DEFAULT '0',
  `rank` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `text` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table characters
--
CREATE TABLE IF NOT EXISTS `characters` (
  `guid` int(6) unsigned NOT NULL DEFAULT '0',
  `acct` int(20) unsigned NOT NULL DEFAULT '0',
  `name` varchar(21) NOT NULL DEFAULT '',
  `race` smallint(3) NOT NULL,
  `class` smallint(3) NOT NULL,
  `gender` tinyint(1) NOT NULL,
  `custom_faction` int(30) NOT NULL DEFAULT '0',
  `level` int(3) NOT NULL,
  `xp` int(30) NOT NULL,
  `active_cheats` int(10) unsigned NOT NULL DEFAULT '0',
  `exploration_data` longtext NOT NULL,
  `watched_faction_index` bigint(40) NOT NULL,
  `selected_pvp_title` int(30) NOT NULL,
  `available_pvp_titles` bigint(10) unsigned NOT NULL DEFAULT '0',
  `available_pvp_titles1` bigint(10) NOT NULL,
  `available_pvp_titles2` bigint(10) unsigned NOT NULL DEFAULT '0',
  `gold` int(30) NOT NULL,
  `ammo_id` int(30) NOT NULL,
  `available_prof_points` int(30) NOT NULL,
  `current_hp` int(30) NOT NULL,
  `current_power` int(30) NOT NULL,
  `pvprank` int(30) NOT NULL,
  `bytes` int(30) NOT NULL,
  `bytes2` int(30) NOT NULL,
  `player_flags` int(30) NOT NULL,
  `player_bytes` int(30) NOT NULL,
  `positionX` float NOT NULL DEFAULT '0',
  `positionY` float NOT NULL DEFAULT '0',
  `positionZ` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `mapId` int(8) unsigned NOT NULL DEFAULT '0',
  `zoneId` int(8) unsigned NOT NULL DEFAULT '0',
  `taximask` longtext NOT NULL,
  `banned` int(40) unsigned NOT NULL DEFAULT '0',
  `banReason` varchar(255) NOT NULL,
  `timestamp` int(30) DEFAULT NULL,
  `online` int(11) DEFAULT NULL,
  `bindpositionX` float NOT NULL DEFAULT '0',
  `bindpositionY` float NOT NULL DEFAULT '0',
  `bindpositionZ` float NOT NULL DEFAULT '0',
  `bindmapId` int(8) unsigned NOT NULL DEFAULT '0',
  `bindzoneId` int(8) unsigned NOT NULL DEFAULT '0',
  `isResting` int(3) NOT NULL DEFAULT '0',
  `restState` int(5) NOT NULL DEFAULT '0',
  `restTime` int(5) NOT NULL DEFAULT '0',
  `playedtime` text NOT NULL,
  `deathstate` int(5) NOT NULL DEFAULT '0',
  `TalentResetTimes` int(5) NOT NULL DEFAULT '0',
  `first_login` tinyint(1) NOT NULL DEFAULT '0',
  `login_flags` int(10) unsigned NOT NULL DEFAULT '0',
  `arenaPoints` int(10) NOT NULL,
  `totalstableslots` int(10) unsigned NOT NULL DEFAULT '0',
  `instance_id` int(10) NOT NULL,
  `entrypointmap` int(10) NOT NULL,
  `entrypointx` float NOT NULL,
  `entrypointy` float NOT NULL,
  `entrypointz` float NOT NULL,
  `entrypointo` float NOT NULL,
  `entrypointinstance` int(10) NOT NULL,
  `taxi_path` int(10) NOT NULL,
  `taxi_lastnode` int(10) NOT NULL,
  `taxi_mountid` int(10) NOT NULL,
  `transporter` int(10) NOT NULL,
  `transporter_xdiff` float NOT NULL,
  `transporter_ydiff` float NOT NULL,
  `transporter_zdiff` float NOT NULL,
  `transporter_odiff` float NOT NULL,
  `actions1` longtext NOT NULL,
  `actions2` longtext NOT NULL,
  `auras` longtext NOT NULL,
  `finished_quests` longtext NOT NULL,
  `finisheddailies` longtext NOT NULL,
  `honorRolloverTime` int(30) NOT NULL DEFAULT '0',
  `killsToday` int(10) NOT NULL DEFAULT '0',
  `killsYesterday` int(10) NOT NULL DEFAULT '0',
  `killsLifeTime` int(10) NOT NULL DEFAULT '0',
  `honorToday` int(10) NOT NULL DEFAULT '0',
  `honorYesterday` int(10) NOT NULL DEFAULT '0',
  `honorPoints` int(10) NOT NULL DEFAULT '0',
  `drunkValue` int(30) NOT NULL DEFAULT '0',
  `glyphs1` longtext NOT NULL,
  `talents1` longtext NOT NULL,
  `glyphs2` longtext NOT NULL,
  `talents2` longtext NOT NULL,
  `numspecs` int(10) NOT NULL DEFAULT '1',
  `currentspec` int(10) NOT NULL DEFAULT '0',
  `talentpoints` longtext NOT NULL,
  `firsttalenttree` int(11) DEFAULT NULL,
  `phase` int(10) unsigned NOT NULL DEFAULT '1',
  `CanGainXp` int(10) unsigned NOT NULL DEFAULT '1',
  `data` longtext,
  `resettalents` int(10) unsigned NOT NULL DEFAULT '0',
  `rbg_daily` tinyint(1) NOT NULL DEFAULT '0' COMMENT 'Boolean already done a daily rbg?',
  `dungeon_difficulty` SMALLINT(1) unsigned NOT NULL DEFAULT '0',
  `raid_difficulty` SMALLINT(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`),
  KEY `acct` (`acct`),
  KEY `name` (`name`),
  KEY `b` (`banned`),
  KEY `c` (`online`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table characters_insert_queue
--
CREATE TABLE IF NOT EXISTS `characters_insert_queue` (
  `insert_temp_guid` int(10) unsigned NOT NULL DEFAULT '0',
  `acct` int(10) unsigned NOT NULL DEFAULT '0',
  `name` varchar(21) COLLATE utf8_unicode_ci NOT NULL,
  `race` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `gender` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `custom_faction` int(30) NOT NULL DEFAULT '0',
  `level` int(10) unsigned NOT NULL DEFAULT '0',
  `xp` int(10) unsigned NOT NULL DEFAULT '0',
  `exploration_data` longtext COLLATE utf8_unicode_ci NOT NULL,
  `skills` longtext COLLATE utf8_unicode_ci NOT NULL,
  `watched_faction_index` int(10) unsigned NOT NULL DEFAULT '0',
  `selected_pvp_title` int(10) unsigned NOT NULL DEFAULT '0',
  `available_pvp_titles` int(10) unsigned NOT NULL DEFAULT '0',
  `gold` int(10) unsigned NOT NULL DEFAULT '0',
  `ammo_id` int(10) unsigned NOT NULL DEFAULT '0',
  `available_prof_points` int(10) unsigned NOT NULL DEFAULT '0',
  `available_talent_points` int(10) unsigned NOT NULL DEFAULT '0',
  `current_hp` int(10) unsigned NOT NULL DEFAULT '0',
  `current_power` int(10) unsigned NOT NULL DEFAULT '0',
  `pvprank` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `bytes` int(10) unsigned NOT NULL DEFAULT '0',
  `bytes2` int(10) unsigned NOT NULL DEFAULT '0',
  `player_flags` int(10) unsigned NOT NULL DEFAULT '0',
  `player_bytes` int(10) unsigned NOT NULL DEFAULT '0',
  `positionX` float NOT NULL DEFAULT '0',
  `positionY` float NOT NULL DEFAULT '0',
  `positionZ` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `mapId` int(10) unsigned NOT NULL DEFAULT '0',
  `zoneId` int(10) unsigned NOT NULL DEFAULT '0',
  `taximask` longtext COLLATE utf8_unicode_ci NOT NULL,
  `banned` int(40) NOT NULL,
  `banReason` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  `timestamp` int(11) NOT NULL,
  `online` int(11) DEFAULT NULL,
  `bindpositionX` float NOT NULL DEFAULT '0',
  `bindpositionY` float NOT NULL DEFAULT '0',
  `bindpositionZ` float NOT NULL DEFAULT '0',
  `bindmapId` int(10) unsigned NOT NULL DEFAULT '0',
  `bindzoneId` int(10) unsigned NOT NULL DEFAULT '0',
  `isResting` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `restState` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `restTime` int(10) unsigned NOT NULL DEFAULT '0',
  `playedtime` longtext COLLATE utf8_unicode_ci NOT NULL,
  `deathstate` int(10) unsigned NOT NULL DEFAULT '0',
  `TalentResetTimes` int(10) unsigned NOT NULL DEFAULT '0',
  `first_login` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `forced_rename_pending` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `publicNote` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `officerNote` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `guildid` int(10) unsigned NOT NULL DEFAULT '0',
  `guildRank` int(10) unsigned NOT NULL DEFAULT '0',
  `arenaPoints` int(10) NOT NULL,
  `totalstableslots` int(10) unsigned NOT NULL DEFAULT '0',
  `instance_id` int(10) unsigned NOT NULL DEFAULT '0',
  `entrypointmap` int(10) unsigned NOT NULL DEFAULT '0',
  `entrypointx` float NOT NULL DEFAULT '0',
  `entrypointy` float NOT NULL DEFAULT '0',
  `entrypointz` float NOT NULL DEFAULT '0',
  `entrypointo` float NOT NULL DEFAULT '0',
  `entrypointinstance` int(10) unsigned NOT NULL DEFAULT '0',
  `taxi_path` int(10) unsigned NOT NULL DEFAULT '0',
  `taxi_lastnode` int(10) unsigned NOT NULL DEFAULT '0',
  `taxi_mountid` int(10) unsigned NOT NULL DEFAULT '0',
  `transporter` int(10) unsigned NOT NULL DEFAULT '0',
  `transporter_xdiff` float NOT NULL DEFAULT '0',
  `transporter_ydiff` float NOT NULL DEFAULT '0',
  `transporter_zdiff` float NOT NULL DEFAULT '0',
  `spells` longtext COLLATE utf8_unicode_ci NOT NULL,
  `deleted_spells` longtext COLLATE utf8_unicode_ci NOT NULL,
  `reputation` longtext COLLATE utf8_unicode_ci NOT NULL,
  `actions` longtext COLLATE utf8_unicode_ci NOT NULL,
  `auras` longtext COLLATE utf8_unicode_ci NOT NULL,
  `finished_quests` longtext COLLATE utf8_unicode_ci NOT NULL,
  `honorPointsToAdd` int(10) NOT NULL,
  `killsToday` int(10) unsigned NOT NULL DEFAULT '0',
  `killsYesterday` int(10) unsigned NOT NULL DEFAULT '0',
  `killsLifeTime` int(10) unsigned NOT NULL DEFAULT '0',
  `honorToday` int(10) unsigned NOT NULL DEFAULT '0',
  `honorYesterday` int(10) unsigned NOT NULL DEFAULT '0',
  `honorPoints` int(10) unsigned NOT NULL DEFAULT '0',
  `difficulty` int(10) unsigned NOT NULL DEFAULT '0',
  UNIQUE KEY `guid` (`insert_temp_guid`),
  KEY `acct` (`acct`),
  KEY `guildid` (`guildid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Table structure for table character_achievement
--
CREATE TABLE IF NOT EXISTS `character_achievement` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `achievement` int(10) unsigned NOT NULL DEFAULT '0',
  `date` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`guid`,`achievement`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table character_achievement_progress
--
CREATE TABLE IF NOT EXISTS `character_achievement_progress` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `criteria` int(10) unsigned NOT NULL DEFAULT '0',
  `counter` int(10) DEFAULT NULL,
  `date` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`guid`,`criteria`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table character_db_version
--
CREATE TABLE IF NOT EXISTS `character_db_version` (
  `LastUpdate` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`LastUpdate`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Dumping data for table character_db_version: ~1 rows (approximately)
INSERT INTO `character_db_version` (`LastUpdate`) VALUES
	('2016-09-13_01_guild');

--
-- Table structure for table charters
--
CREATE TABLE IF NOT EXISTS `charters` (
  `charterId` int(30) NOT NULL,
  `charterType` int(30) NOT NULL DEFAULT '0',
  `leaderGuid` int(20) unsigned NOT NULL DEFAULT '0',
  `guildName` varchar(32) NOT NULL DEFAULT '',
  `itemGuid` bigint(40) unsigned NOT NULL DEFAULT '0',
  `signer1` int(10) unsigned NOT NULL DEFAULT '0',
  `signer2` int(10) unsigned NOT NULL DEFAULT '0',
  `signer3` int(10) unsigned NOT NULL DEFAULT '0',
  `signer4` int(10) unsigned NOT NULL DEFAULT '0',
  `signer5` int(10) unsigned NOT NULL DEFAULT '0',
  `signer6` int(10) unsigned NOT NULL DEFAULT '0',
  `signer7` int(10) unsigned NOT NULL DEFAULT '0',
  `signer8` int(10) unsigned NOT NULL DEFAULT '0',
  `signer9` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`charterId`),
  UNIQUE KEY `leaderGuid` (`charterType`,`leaderGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table clientaddons
--
CREATE TABLE IF NOT EXISTS `clientaddons` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `name` varchar(50) DEFAULT NULL,
  `crc` bigint(50) DEFAULT NULL,
  `banned` int(1) NOT NULL DEFAULT '0',
  `showinlist` int(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `index` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table command_overrides
--
CREATE TABLE IF NOT EXISTS `command_overrides` (
  `command_name` varchar(100) NOT NULL,
  `access_level` varchar(10) NOT NULL,
  PRIMARY KEY (`command_name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table corpses
--
CREATE TABLE IF NOT EXISTS `corpses` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `positionX` float NOT NULL DEFAULT '0',
  `positionY` float NOT NULL DEFAULT '0',
  `positionZ` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `zoneId` int(11) NOT NULL DEFAULT '38',
  `mapId` int(11) NOT NULL DEFAULT '0',
  `instanceId` int(11) NOT NULL DEFAULT '0',
  `data` longtext NOT NULL,
  PRIMARY KEY (`guid`),
  KEY `b` (`instanceId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table equipmentsets
--
CREATE TABLE IF NOT EXISTS `equipmentsets` (
  `ownerguid` int(10) unsigned NOT NULL DEFAULT '1',
  `setGUID` int(10) unsigned NOT NULL DEFAULT '1',
  `setid` int(10) unsigned NOT NULL DEFAULT '1',
  `setname` varchar(16) NOT NULL DEFAULT '',
  `iconname` varchar(100) NOT NULL DEFAULT '',
  `head` int(10) unsigned NOT NULL DEFAULT '0',
  `neck` int(10) unsigned NOT NULL DEFAULT '0',
  `shoulders` int(10) unsigned NOT NULL DEFAULT '0',
  `body` int(10) unsigned NOT NULL DEFAULT '0',
  `chest` int(10) unsigned NOT NULL DEFAULT '0',
  `waist` int(10) unsigned NOT NULL DEFAULT '0',
  `legs` int(10) unsigned NOT NULL DEFAULT '0',
  `feet` int(10) unsigned NOT NULL DEFAULT '0',
  `wrists` int(10) unsigned NOT NULL DEFAULT '0',
  `hands` int(10) unsigned NOT NULL DEFAULT '0',
  `finger1` int(10) unsigned NOT NULL DEFAULT '0',
  `finger2` int(10) unsigned NOT NULL DEFAULT '0',
  `trinket1` int(10) unsigned NOT NULL DEFAULT '0',
  `trinket2` int(10) unsigned NOT NULL DEFAULT '0',
  `back` int(10) unsigned NOT NULL DEFAULT '0',
  `mainhand` int(10) unsigned NOT NULL DEFAULT '0',
  `offhand` int(10) unsigned NOT NULL DEFAULT '0',
  `ranged` int(10) unsigned NOT NULL DEFAULT '0',
  `tabard` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ownerguid`,`setGUID`,`setid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table event_save
--
CREATE TABLE IF NOT EXISTS `event_save` (
  `event_entry` int(3) unsigned NOT NULL,
  `state` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `next_start` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`event_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table gm_survey
--
CREATE TABLE IF NOT EXISTS `gm_survey` (
  `survey_id` int(10) unsigned NOT NULL,
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `main_survey` int(10) unsigned NOT NULL DEFAULT '0',
  `comment` longtext NOT NULL,
  `create_time` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`survey_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='GM Survey';

--
-- Table structure for table gm_survey_answers
--
CREATE TABLE IF NOT EXISTS `gm_survey_answers` (
  `survey_id` int(10) unsigned NOT NULL,
  `question_id` int(10) unsigned NOT NULL DEFAULT '0',
  `answer_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`survey_id`,`question_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='GM Survey';

--
-- Table structure for table gm_tickets
--
CREATE TABLE IF NOT EXISTS `gm_tickets` (
  `ticketid` int(11) NOT NULL,
  `playerGuid` int(11) NOT NULL,
  `name` varchar(200) NOT NULL DEFAULT '',
  `level` int(6) NOT NULL DEFAULT '0',
  `map` int(11) NOT NULL DEFAULT '0',
  `posX` float NOT NULL DEFAULT '0',
  `posY` float NOT NULL DEFAULT '0',
  `posZ` float NOT NULL DEFAULT '0',
  `message` text NOT NULL,
  `timestamp` text,
  `deleted` int(10) unsigned NOT NULL DEFAULT '0',
  `assignedto` int(11) NOT NULL DEFAULT '0',
  `comment` text NOT NULL,
  UNIQUE KEY `guid` (`ticketid`),
  UNIQUE KEY `guid_2` (`ticketid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table groups
--
CREATE TABLE IF NOT EXISTS `groups` (
  `group_id` int(30) NOT NULL,
  `group_type` tinyint(2) NOT NULL,
  `subgroup_count` tinyint(2) NOT NULL,
  `loot_method` tinyint(2) NOT NULL,
  `loot_threshold` tinyint(2) NOT NULL,
  `difficulty` int(30) NOT NULL DEFAULT '0',
  `raiddifficulty` int(30) NOT NULL DEFAULT '0',
  `assistant_leader` int(30) NOT NULL DEFAULT '0',
  `main_tank` int(30) NOT NULL DEFAULT '0',
  `main_assist` int(30) NOT NULL DEFAULT '0',
  `group1member1` int(50) NOT NULL,
  `group1member2` int(50) NOT NULL,
  `group1member3` int(50) NOT NULL,
  `group1member4` int(50) NOT NULL,
  `group1member5` int(50) NOT NULL,
  `group2member1` int(50) NOT NULL,
  `group2member2` int(50) NOT NULL,
  `group2member3` int(50) NOT NULL,
  `group2member4` int(50) NOT NULL,
  `group2member5` int(50) NOT NULL,
  `group3member1` int(50) NOT NULL,
  `group3member2` int(50) NOT NULL,
  `group3member3` int(50) NOT NULL,
  `group3member4` int(50) NOT NULL,
  `group3member5` int(50) NOT NULL,
  `group4member1` int(50) NOT NULL,
  `group4member2` int(50) NOT NULL,
  `group4member3` int(50) NOT NULL,
  `group4member4` int(50) NOT NULL,
  `group4member5` int(50) NOT NULL,
  `group5member1` int(50) NOT NULL,
  `group5member2` int(50) NOT NULL,
  `group5member3` int(50) NOT NULL,
  `group5member4` int(50) NOT NULL,
  `group5member5` int(50) NOT NULL,
  `group6member1` int(50) NOT NULL,
  `group6member2` int(50) NOT NULL,
  `group6member3` int(50) NOT NULL,
  `group6member4` int(50) NOT NULL,
  `group6member5` int(50) NOT NULL,
  `group7member1` int(50) NOT NULL,
  `group7member2` int(50) NOT NULL,
  `group7member3` int(50) NOT NULL,
  `group7member4` int(50) NOT NULL,
  `group7member5` int(50) NOT NULL,
  `group8member1` int(50) NOT NULL,
  `group8member2` int(50) NOT NULL,
  `group8member3` int(50) NOT NULL,
  `group8member4` int(50) NOT NULL,
  `group8member5` int(50) NOT NULL,
  `timestamp` int(30) NOT NULL,
  `instanceids` text NOT NULL,
  PRIMARY KEY (`group_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table guild
--
CREATE TABLE IF NOT EXISTS `guild` (
  `guildId` int(10) unsigned NOT NULL default '0',
  `guildName` varchar(24) NOT NULL default '',
  `leaderGuid` int(10) unsigned NOT NULL default '0',
  `emblemStyle` tinyint(3) unsigned NOT NULL default '0',
  `emblemColor` tinyint(3) unsigned NOT NULL default '0',
  `borderStyle` tinyint(3) unsigned NOT NULL default '0',
  `borderColor` tinyint(3) unsigned NOT NULL default '0',
  `backgroundColor` tinyint(3) unsigned NOT NULL default '0',
  `guildInfo` text NOT NULL,
  `motd` varchar(128) NOT NULL default '',
  `createdate` int(10) unsigned NOT NULL default '0',
  `bankBalance` bigint(20) unsigned NOT NULL default '0',
  `guildLevel` int(10) unsigned default '1',
  `guildExperience` bigint(20) unsigned default '0',
  `todayExperience` bigint(20) unsigned default '0',
  PRIMARY KEY  (`guildId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Guild System';

--
-- Table structure for table guild_achievement
--
CREATE TABLE IF NOT EXISTS `guild_achievement` (
  `guildId` int(10) unsigned NOT NULL,
  `achievement` smallint(5) unsigned NOT NULL,
  `date` int(10) unsigned NOT NULL default '0',
  `guids` text NOT NULL,
  PRIMARY KEY  (`guildId`,`achievement`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table guild_achievement_progress
--
CREATE TABLE IF NOT EXISTS `guild_achievement_progress` (
  `guildId` int(10) unsigned NOT NULL,
  `criteria` smallint(5) unsigned NOT NULL,
  `counter` int(10) unsigned NOT NULL,
  `date` int(10) unsigned NOT NULL default '0',
  `completedGuid` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guildId`,`criteria`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table guild_bank_eventlog
--
CREATE TABLE IF NOT EXISTS `guild_bank_eventlog` (
  `guildId` int(10) unsigned NOT NULL default '0' COMMENT 'Guild Identificator',
  `logGuid` int(10) unsigned NOT NULL default '0' COMMENT 'Log record identificator - auxiliary column',
  `tabId` tinyint(3) unsigned NOT NULL default '0' COMMENT 'Guild bank TabId',
  `eventType` tinyint(3) unsigned NOT NULL default '0' COMMENT 'Event type',
  `playerGuid` int(10) unsigned NOT NULL default '0',
  `itemOrMoney` int(10) unsigned NOT NULL default '0',
  `itemStackCount` smallint(5) unsigned NOT NULL default '0',
  `destTabId` tinyint(3) unsigned NOT NULL default '0' COMMENT 'Destination Tab Id',
  `timeStamp` int(10) unsigned NOT NULL default '0' COMMENT 'Event UNIX time',
  PRIMARY KEY  (`guildId`,`logGuid`,`tabId`),
  KEY `guildid_key` (`guildid`),
  KEY `Idx_PlayerGuid` (`playerGuid`),
  KEY `Idx_LogGuid` (`logGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table guild_bank_item
--
CREATE TABLE IF NOT EXISTS `guild_bank_item` (
  `guildId` int(10) unsigned NOT NULL default '0',
  `tabId` tinyint(3) unsigned NOT NULL default '0',
  `slotId` tinyint(3) unsigned NOT NULL default '0',
  `itemGuid` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guildId`,`tabId`,`slotId`),
  KEY `guildid_key` (`guildId`),
  KEY `Idx_item_guid` (`itemGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table guild_bank_right
--
CREATE TABLE IF NOT EXISTS `guild_bank_right` (
  `guildId` int(10) unsigned NOT NULL default '0',
  `tabId` tinyint(3) unsigned NOT NULL default '0',
  `rid` tinyint(3) unsigned NOT NULL default '0',
  `gbright` tinyint(3) unsigned NOT NULL default '0',
  `slotPerDay` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guildId`,`tabId`,`rid`),
  KEY `guildid_key` (`guildId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table guild_bank_tab
--
CREATE TABLE IF NOT EXISTS `guild_bank_tab` (
  `guildId` int(10) unsigned NOT NULL default '0',
  `tabId` tinyint(3) unsigned NOT NULL default '0',
  `tabName` varchar(16) NOT NULL default '',
  `tabIcon` varchar(100) NOT NULL default '',
  `tabText` varchar(500) default NULL,
  PRIMARY KEY  (`guildId`,`tabId`),
  KEY `guildid_key` (`guildId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table guild_eventlog
--
CREATE TABLE IF NOT EXISTS `guild_eventlog` (
  `guildId` int(10) unsigned NOT NULL COMMENT 'Guild Identificator',
  `logGuid` int(10) unsigned NOT NULL COMMENT 'Log record identificator - auxiliary column',
  `eventType` tinyint(3) unsigned NOT NULL COMMENT 'Event type',
  `playerGuid1` int(10) unsigned NOT NULL COMMENT 'Player 1',
  `playerGuid2` int(10) unsigned NOT NULL COMMENT 'Player 2',
  `newRank` tinyint(3) unsigned NOT NULL COMMENT 'New rank(in case promotion/demotion)',
  `timeStamp` int(10) unsigned NOT NULL COMMENT 'Event UNIX time',
  PRIMARY KEY  (`guildid`,`LogGuid`),
  KEY `Idx_PlayerGuid1` (`playerGuid1`),
  KEY `Idx_PlayerGuid2` (`playerGuid2`),
  KEY `Idx_LogGuid` (`logGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Guild Eventlog';

--
-- Table structure for table guild_finder_applicant
--
CREATE TABLE IF NOT EXISTS `guild_finder_applicant` (
  `guildId` int(10) unsigned default NULL,
  `playerGuid` int(10) unsigned default NULL,
  `availability` tinyint(3) unsigned default '0',
  `classRole` tinyint(3) unsigned default '0',
  `interests` tinyint(3) unsigned default '0',
  `comment` varchar(255) default NULL,
  `submitTime` int(10) unsigned default NULL,
  UNIQUE KEY `guildId` (`guildId`,`playerGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table guild_finder_guild_settings
--
CREATE TABLE IF NOT EXISTS `guild_finder_guild_settings` (
  `guildId` int(10) unsigned NOT NULL,
  `availability` tinyint(3) unsigned NOT NULL default '0',
  `classRoles` tinyint(3) unsigned NOT NULL default '0',
  `interests` tinyint(3) unsigned NOT NULL default '0',
  `level` tinyint(3) unsigned NOT NULL default '1',
  `listed` tinyint(3) unsigned NOT NULL default '0',
  `comment` varchar(255) default NULL,
  PRIMARY KEY  (`guildId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table guild_member
--
CREATE TABLE IF NOT EXISTS `guild_member` (
  `guildId` int(10) unsigned NOT NULL COMMENT 'Guild Identificator',
  `playerGuid` int(10) unsigned NOT NULL,
  `rank` tinyint(3) unsigned NOT NULL,
  `pnote` varchar(31) NOT NULL default '',
  `offnote` varchar(31) NOT NULL default '',
  UNIQUE KEY `guid_key` (`playerGuid`),
  KEY `guildid_key` (`guildId`),
  KEY `guildid_rank_key` (`guildId`,`rank`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Guild System';

--
-- Table structure for table guild_member_withdraw
--
CREATE TABLE IF NOT EXISTS `guild_member_withdraw` (
  `guid` int(10) unsigned NOT NULL,
  `tab0` int(10) unsigned NOT NULL default '0',
  `tab1` int(10) unsigned NOT NULL default '0',
  `tab2` int(10) unsigned NOT NULL default '0',
  `tab3` int(10) unsigned NOT NULL default '0',
  `tab4` int(10) unsigned NOT NULL default '0',
  `tab5` int(10) unsigned NOT NULL default '0',
  `tab6` int(10) unsigned NOT NULL default '0',
  `tab7` int(10) unsigned NOT NULL default '0',
  `money` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Guild Member Daily Withdraws';

--
-- Table structure for table guild_newslog
--
CREATE TABLE IF NOT EXISTS `guild_newslog` (
  `guildId` int(10) unsigned NOT NULL default '0' COMMENT 'Guild Identificator',
  `logGuid` int(10) unsigned NOT NULL default '0' COMMENT 'Log record identificator - auxiliary column',
  `eventType` tinyint(3) unsigned NOT NULL default '0' COMMENT 'Event type',
  `playerGuid` int(10) unsigned NOT NULL default '0',
  `flags` int(10) unsigned NOT NULL default '0',
  `value` int(10) unsigned NOT NULL default '0',
  `timeStamp` int(10) unsigned NOT NULL default '0' COMMENT 'Event UNIX time',
  PRIMARY KEY  (`guildId`,`logGuid`),
  KEY `guildid_key` (`guildId`),
  KEY `Idx_PlayerGuid` (`playerGuid`),
  KEY `Idx_LogGuid` (`logGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table guild_rank
--
CREATE TABLE IF NOT EXISTS`guild_rank` (
  `guildId` int(10) unsigned NOT NULL default '0',
  `rid` tinyint(3) unsigned NOT NULL,
  `rname` varchar(20) NOT NULL default '',
  `rights` mediumint(8) unsigned NOT NULL default '0',
  `bankMoneyPerDay` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guildId`,`rid`),
  KEY `Idx_rid` (`rid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Guild System';

--
-- Table structure for table instanceids
--
CREATE TABLE IF NOT EXISTS `instanceids` (
  `playerguid` int(11) unsigned NOT NULL DEFAULT '0',
  `mapid` int(11) unsigned NOT NULL DEFAULT '0',
  `mode` int(11) unsigned NOT NULL DEFAULT '0',
  `instanceid` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`playerguid`,`mapid`,`mode`),
  KEY `ix_instanceid` (`playerguid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Player / InstanceID - Reference Table';

--
-- Table structure for table instances
--
CREATE TABLE IF NOT EXISTS `instances` (
  `id` int(30) NOT NULL,
  `mapid` int(30) NOT NULL,
  `creation` int(30) NOT NULL,
  `expiration` int(30) NOT NULL,
  `killed_npc_guids` text NOT NULL,
  `difficulty` int(30) NOT NULL,
  `creator_group` int(30) NOT NULL,
  `creator_guid` int(30) NOT NULL,
  `persistent` tinyint(4) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `a` (`mapid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table lag_reports
--
CREATE TABLE IF NOT EXISTS `lag_reports` (
  `lag_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `player` int(10) unsigned NOT NULL,
  `account` int(10) unsigned NOT NULL,
  `lag_type` smallint(2) unsigned NOT NULL,
  `map_id` int(5) unsigned DEFAULT '0',
  `position_x` float DEFAULT '0',
  `position_y` float DEFAULT '0',
  `position_z` float DEFAULT '0',
  `timestamp` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`lag_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table lfg_data
--
CREATE TABLE IF NOT EXISTS `lfg_data` (
  `guid` bigint(10) NOT NULL,
  `dungeon` int(10) NOT NULL,
  `state` int(10) NOT NULL,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table mailbox
--
CREATE TABLE IF NOT EXISTS `mailbox` (
  `message_id` int(30) NOT NULL DEFAULT '0',
  `message_type` int(30) NOT NULL DEFAULT '0',
  `player_guid` int(30) NOT NULL DEFAULT '0',
  `sender_guid` bigint(30) unsigned NOT NULL DEFAULT '0',
  `subject` varchar(255) NOT NULL DEFAULT '',
  `body` longtext NOT NULL,
  `money` int(30) NOT NULL DEFAULT '0',
  `attached_item_guids` varchar(200) NOT NULL DEFAULT '',
  `cod` int(30) NOT NULL DEFAULT '0',
  `stationary` bigint(20) NOT NULL DEFAULT '0',
  `expiry_time` int(30) NOT NULL DEFAULT '0',
  `delivery_time` int(30) NOT NULL DEFAULT '0',
  `checked_flag` int(30) unsigned NOT NULL DEFAULT '0',
  `deleted_flag` int(30) NOT NULL DEFAULT '0',
  PRIMARY KEY (`message_id`),
  KEY `b` (`player_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table mailbox_insert_queue
--
CREATE TABLE IF NOT EXISTS `mailbox_insert_queue` (
  `sender_guid` bigint(30) NOT NULL,
  `receiver_guid` int(30) NOT NULL,
  `subject` varchar(200) NOT NULL,
  `body` varchar(500) NOT NULL,
  `stationary` int(30) NOT NULL,
  `money` int(30) NOT NULL,
  `item_id` int(30) NOT NULL,
  `item_stack` int(30) NOT NULL,
  `item_id2` int(30) NOT NULL DEFAULT '0',
  `item_stack2` int(30) NOT NULL DEFAULT '0',
  `item_id3` int(30) NOT NULL DEFAULT '0',
  `item_stack3` int(30) NOT NULL DEFAULT '0',
  `item_id4` int(30) NOT NULL DEFAULT '0',
  `item_stack4` int(30) NOT NULL DEFAULT '0',
  `item_id5` int(30) NOT NULL DEFAULT '0',
  `item_stack5` int(30) NOT NULL DEFAULT '0',
  `item_id6` int(30) NOT NULL DEFAULT '0',
  `item_stack6` int(30) NOT NULL DEFAULT '0',
  `item_id7` int(30) NOT NULL DEFAULT '0',
  `item_stack7` int(30) NOT NULL DEFAULT '0',
  `item_id8` int(30) NOT NULL DEFAULT '0',
  `item_stack8` int(30) NOT NULL DEFAULT '0',
  `item_id9` int(30) NOT NULL DEFAULT '0',
  `item_stack9` int(30) NOT NULL DEFAULT '0',
  `item_id10` int(30) NOT NULL DEFAULT '0',
  `item_stack10` int(30) NOT NULL DEFAULT '0',
  `item_id11` int(30) NOT NULL DEFAULT '0',
  `item_stack11` int(30) NOT NULL DEFAULT '0',
  `item_id12` int(30) NOT NULL DEFAULT '0',
  `item_stack12` int(30) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table playerbugreports
--
CREATE TABLE IF NOT EXISTS `playerbugreports` (
  `UID` int(10) unsigned NOT NULL,
  `AccountID` int(10) unsigned NOT NULL,
  `TimeStamp` int(10) unsigned NOT NULL,
  `Suggestion` int(10) unsigned NOT NULL,
  `Type` text NOT NULL,
  `Content` text NOT NULL,
  PRIMARY KEY (`UID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table playercooldowns
--
CREATE TABLE IF NOT EXISTS `playercooldowns` (
  `player_guid` int(30) NOT NULL,
  `cooldown_type` int(30) NOT NULL COMMENT '0 is spell, 1 is item, 2 is spell category',
  `cooldown_misc` int(30) NOT NULL COMMENT 'spellid/itemid/category',
  `cooldown_expire_time` int(30) NOT NULL COMMENT 'expiring time in unix epoch format',
  `cooldown_spellid` int(30) NOT NULL COMMENT 'spell that cast it',
  `cooldown_itemid` int(30) NOT NULL COMMENT 'item that cast it'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table playerdeletedspells
--
CREATE TABLE IF NOT EXISTS `playerdeletedspells` (
  `GUID` int(10) unsigned NOT NULL,
  `SpellID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`GUID`,`SpellID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table playeritems
--
CREATE TABLE IF NOT EXISTS `playeritems` (
  `ownerguid` int(10) unsigned NOT NULL DEFAULT '0',
  `guid` bigint(10) NOT NULL AUTO_INCREMENT,
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `wrapped_item_id` int(30) NOT NULL DEFAULT '0',
  `wrapped_creator` int(30) NOT NULL DEFAULT '0',
  `creator` int(10) unsigned NOT NULL DEFAULT '0',
  `count` int(10) unsigned NOT NULL DEFAULT '0',
  `charges` int(10) NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `randomprop` int(10) unsigned NOT NULL DEFAULT '0',
  `randomsuffix` int(10) NOT NULL,
  `itemtext` int(10) unsigned NOT NULL DEFAULT '0',
  `durability` int(10) unsigned NOT NULL DEFAULT '0',
  `containerslot` int(11) DEFAULT '-1',
  `slot` int(10) NOT NULL DEFAULT '0',
  `enchantments` longtext NOT NULL,
  `duration_expireson` int(10) unsigned NOT NULL DEFAULT '0',
  `refund_purchasedon` int(10) unsigned NOT NULL DEFAULT '0',
  `refund_costid` int(10) unsigned NOT NULL DEFAULT '0',
  `text` text NOT NULL,
  PRIMARY KEY (`guid`),
  KEY `ownerguid` (`ownerguid`),
  KEY `itemtext` (`itemtext`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table playeritems_insert_queue
--
CREATE TABLE IF NOT EXISTS `playeritems_insert_queue` (
  `ownerguid` int(10) unsigned NOT NULL DEFAULT '0',
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `wrapped_item_id` int(30) NOT NULL DEFAULT '0',
  `wrapped_creator` int(30) NOT NULL DEFAULT '0',
  `creator` int(10) unsigned NOT NULL DEFAULT '0',
  `count` int(10) unsigned NOT NULL DEFAULT '0',
  `charges` int(10) unsigned NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `randomprop` int(10) unsigned NOT NULL DEFAULT '0',
  `randomsuffix` int(30) NOT NULL,
  `itemtext` int(10) unsigned NOT NULL DEFAULT '0',
  `durability` int(10) unsigned NOT NULL DEFAULT '0',
  `containerslot` int(11) NOT NULL DEFAULT '-1' COMMENT 'couldnt find this being used in source',
  `slot` tinyint(4) NOT NULL DEFAULT '0',
  `enchantments` longtext COLLATE utf8_unicode_ci NOT NULL,
  KEY `ownerguid` (`ownerguid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Table structure for table playerpets
--
CREATE TABLE IF NOT EXISTS `playerpets` (
  `ownerguid` bigint(20) NOT NULL DEFAULT '0',
  `petnumber` int(11) NOT NULL DEFAULT '0',
  `name` varchar(21) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `xp` int(11) NOT NULL DEFAULT '0',
  `active` tinyint(1) NOT NULL DEFAULT '0',
  `level` int(11) NOT NULL DEFAULT '0',
  `actionbar` varchar(200) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `happinessupdate` int(11) NOT NULL DEFAULT '0',
  `reset_time` int(10) unsigned NOT NULL DEFAULT '0',
  `reset_cost` int(10) NOT NULL DEFAULT '0',
  `spellid` int(10) unsigned NOT NULL DEFAULT '0',
  `petstate` int(10) unsigned NOT NULL DEFAULT '0',
  `alive` tinyint(1) NOT NULL DEFAULT '1',
  `talentpoints` int(10) unsigned NOT NULL DEFAULT '0',
  `current_power` int(10) unsigned NOT NULL DEFAULT '1',
  `current_hp` int(10) unsigned NOT NULL DEFAULT '1',
  `current_happiness` int(10) unsigned NOT NULL DEFAULT '1000000',
  `renamable` int(10) unsigned NOT NULL DEFAULT '1',
  `type` int(10) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`ownerguid`,`petnumber`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Table structure for table playerpetspells
--
CREATE TABLE IF NOT EXISTS `playerpetspells` (
  `ownerguid` bigint(20) NOT NULL DEFAULT '0',
  `petnumber` int(4) NOT NULL DEFAULT '0',
  `spellid` int(4) NOT NULL DEFAULT '0',
  `flags` int(4) NOT NULL DEFAULT '0',
  KEY `a` (`ownerguid`),
  KEY `b` (`petnumber`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table playerreputations
--
CREATE TABLE IF NOT EXISTS `playerreputations` (
  `guid` int(10) unsigned NOT NULL,
  `faction` int(10) unsigned NOT NULL,
  `flag` int(10) unsigned NOT NULL DEFAULT '0',
  `basestanding` int(11) NOT NULL DEFAULT '0',
  `standing` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`faction`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table playerskills
--
CREATE TABLE IF NOT EXISTS `playerskills` (
  `GUID` int(10) unsigned NOT NULL,
  `SkillID` int(10) unsigned NOT NULL,
  `CurrentValue` int(10) unsigned NOT NULL,
  `MaximumValue` int(10) unsigned NOT NULL,
  PRIMARY KEY (`GUID`,`SkillID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table playerspells
--
CREATE TABLE IF NOT EXISTS `playerspells` (
  `GUID` int(10) unsigned NOT NULL,
  `SpellID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`GUID`,`SpellID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table playersummons
--
CREATE TABLE IF NOT EXISTS `playersummons` (
  `ownerguid` int(11) unsigned NOT NULL DEFAULT '0',
  `entry` int(11) unsigned NOT NULL DEFAULT '0',
  `name` varchar(64) NOT NULL,
  KEY `a` (`ownerguid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table playersummonspells
--
CREATE TABLE IF NOT EXISTS `playersummonspells` (
  `ownerguid` bigint(20) NOT NULL DEFAULT '0',
  `entryid` int(4) NOT NULL DEFAULT '0',
  `spellid` int(4) NOT NULL DEFAULT '0',
  KEY `a` (`ownerguid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table questlog
--
CREATE TABLE IF NOT EXISTS `questlog` (
  `player_guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `quest_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `slot` int(20) unsigned NOT NULL DEFAULT '0',
  `expirytime` int(20) unsigned NOT NULL DEFAULT '0',
  `explored_area1` bigint(20) unsigned NOT NULL DEFAULT '0',
  `explored_area2` bigint(20) unsigned NOT NULL DEFAULT '0',
  `explored_area3` bigint(20) unsigned NOT NULL DEFAULT '0',
  `explored_area4` bigint(20) unsigned NOT NULL DEFAULT '0',
  `mob_kill1` bigint(20) NOT NULL DEFAULT '0',
  `mob_kill2` bigint(20) NOT NULL DEFAULT '0',
  `mob_kill3` bigint(20) NOT NULL DEFAULT '0',
  `mob_kill4` bigint(20) NOT NULL DEFAULT '0',
  `completed` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`player_guid`,`quest_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table server_settings
--
CREATE TABLE IF NOT EXISTS `server_settings` (
  `setting_id` varchar(200) NOT NULL,
  `setting_value` int(50) NOT NULL,
  PRIMARY KEY (`setting_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Dumping data for table server_settings: ~2 rows (approximately)
INSERT INTO `server_settings` (`setting_id`, `setting_value`) VALUES
	('last_arena_update_time', 1455039763),
	('last_daily_update_time', 0);

--
-- Table structure for table social_friends
--
CREATE TABLE IF NOT EXISTS `social_friends` (
  `character_guid` int(30) NOT NULL,
  `friend_guid` int(30) NOT NULL,
  `note` varchar(100) NOT NULL,
  PRIMARY KEY (`character_guid`,`friend_guid`),
  KEY `a` (`character_guid`),
  KEY `b` (`friend_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table social_ignores
--
CREATE TABLE IF NOT EXISTS `social_ignores` (
  `character_guid` int(30) NOT NULL,
  `ignore_guid` int(30) NOT NULL,
  PRIMARY KEY (`character_guid`,`ignore_guid`),
  KEY `a` (`character_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table tutorials
--
CREATE TABLE IF NOT EXISTS `tutorials` (
  `playerId` bigint(20) unsigned NOT NULL DEFAULT '0',
  `tut0` bigint(20) unsigned NOT NULL DEFAULT '0',
  `tut1` bigint(20) unsigned NOT NULL DEFAULT '0',
  `tut2` bigint(20) unsigned NOT NULL DEFAULT '0',
  `tut3` bigint(20) unsigned NOT NULL DEFAULT '0',
  `tut4` bigint(20) unsigned NOT NULL DEFAULT '0',
  `tut5` bigint(20) unsigned NOT NULL DEFAULT '0',
  `tut6` bigint(20) unsigned NOT NULL DEFAULT '0',
  `tut7` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`playerId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
