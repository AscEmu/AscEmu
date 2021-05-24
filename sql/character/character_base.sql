/*
********************************************************************
AscEmu char structure
Last update: 20210524-00
*********************************************************************
*/

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

CREATE TABLE IF NOT EXISTS `account_data` (
  `acct` int NOT NULL,
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `account_data`;
/*!40000 ALTER TABLE `account_data` DISABLE KEYS */;
/*!40000 ALTER TABLE `account_data` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `account_permissions` (
  `id` int unsigned NOT NULL,
  `permissions` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `name` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `account_permissions`;
/*!40000 ALTER TABLE `account_permissions` DISABLE KEYS */;
/*!40000 ALTER TABLE `account_permissions` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `arenateams` (
  `id` int NOT NULL,
  `type` int NOT NULL,
  `leader` int NOT NULL,
  `name` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `emblemstyle` int NOT NULL,
  `emblemcolour` bigint NOT NULL,
  `borderstyle` int NOT NULL,
  `bordercolour` bigint NOT NULL,
  `backgroundcolour` bigint NOT NULL,
  `rating` int NOT NULL,
  `data` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `ranking` int NOT NULL,
  `player_data1` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `player_data2` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `player_data3` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `player_data4` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `player_data5` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `player_data6` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `player_data7` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `player_data8` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `player_data9` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `player_data10` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `arenateams`;
/*!40000 ALTER TABLE `arenateams` DISABLE KEYS */;
/*!40000 ALTER TABLE `arenateams` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `auctions` (
  `auctionId` int NOT NULL AUTO_INCREMENT,
  `auctionhouse` int DEFAULT NULL,
  `item` bigint DEFAULT NULL,
  `owner` bigint DEFAULT NULL,
  `startbid` int DEFAULT NULL,
  `buyout` int DEFAULT NULL,
  `time` int DEFAULT NULL,
  `bidder` bigint DEFAULT NULL,
  `bid` int DEFAULT NULL,
  `deposit` int DEFAULT NULL,
  PRIMARY KEY (`auctionId`),
  KEY `b` (`auctionhouse`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `auctions`;
/*!40000 ALTER TABLE `auctions` DISABLE KEYS */;
/*!40000 ALTER TABLE `auctions` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `banned_char_log` (
  `banned_by` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `banned_player` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `timestamp` int NOT NULL,
  `banned_until` int NOT NULL,
  `reason` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `banned_char_log`;
/*!40000 ALTER TABLE `banned_char_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `banned_char_log` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `banned_names` (
  `name` varchar(30) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `banned_names`;
/*!40000 ALTER TABLE `banned_names` DISABLE KEYS */;
/*!40000 ALTER TABLE `banned_names` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `calendar_events` (
  `entry` int unsigned NOT NULL DEFAULT '0',
  `creator` int unsigned NOT NULL DEFAULT '0',
  `title` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `description` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `type` tinyint unsigned NOT NULL DEFAULT '4',
  `dungeon` int NOT NULL DEFAULT '-1',
  `date` int unsigned NOT NULL DEFAULT '0',
  `flags` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `calendar_events`;
/*!40000 ALTER TABLE `calendar_events` DISABLE KEYS */;
/*!40000 ALTER TABLE `calendar_events` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `calendar_invites` (
  `id` bigint unsigned NOT NULL DEFAULT '0',
  `event` bigint unsigned NOT NULL DEFAULT '0',
  `invitee` int unsigned NOT NULL DEFAULT '0',
  `sender` int unsigned NOT NULL DEFAULT '0',
  `status` tinyint unsigned NOT NULL DEFAULT '0',
  `statustime` int unsigned NOT NULL DEFAULT '0',
  `rank` tinyint unsigned NOT NULL DEFAULT '0',
  `text` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `calendar_invites`;
/*!40000 ALTER TABLE `calendar_invites` DISABLE KEYS */;
/*!40000 ALTER TABLE `calendar_invites` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `characters` (
  `guid` int unsigned NOT NULL DEFAULT '0',
  `acct` int unsigned NOT NULL DEFAULT '0',
  `name` varchar(21) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `race` smallint NOT NULL,
  `class` smallint NOT NULL,
  `gender` tinyint(1) NOT NULL,
  `custom_faction` int NOT NULL DEFAULT '0',
  `level` int NOT NULL,
  `xp` int NOT NULL,
  `active_cheats` int unsigned NOT NULL DEFAULT '0',
  `exploration_data` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `watched_faction_index` bigint NOT NULL,
  `selected_pvp_title` int NOT NULL,
  `available_pvp_titles` bigint unsigned NOT NULL DEFAULT '0',
  `available_pvp_titles1` bigint NOT NULL,
  `available_pvp_titles2` bigint unsigned NOT NULL DEFAULT '0',
  `gold` int NOT NULL,
  `ammo_id` int NOT NULL,
  `available_prof_points` int NOT NULL,
  `current_hp` int NOT NULL,
  `current_power` int NOT NULL,
  `pvprank` int NOT NULL,
  `bytes` int NOT NULL,
  `bytes2` int NOT NULL,
  `player_flags` int NOT NULL,
  `player_bytes` int NOT NULL,
  `positionX` float NOT NULL DEFAULT '0',
  `positionY` float NOT NULL DEFAULT '0',
  `positionZ` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `mapId` int unsigned NOT NULL DEFAULT '0',
  `zoneId` int unsigned NOT NULL DEFAULT '0',
  `taximask` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `banned` int unsigned NOT NULL DEFAULT '0',
  `banReason` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `timestamp` int DEFAULT NULL,
  `online` int DEFAULT NULL,
  `bindpositionX` float NOT NULL DEFAULT '0',
  `bindpositionY` float NOT NULL DEFAULT '0',
  `bindpositionZ` float NOT NULL DEFAULT '0',
  `bindmapId` int unsigned NOT NULL DEFAULT '0',
  `bindzoneId` int unsigned NOT NULL DEFAULT '0',
  `isResting` int NOT NULL DEFAULT '0',
  `restState` int NOT NULL DEFAULT '0',
  `restTime` int NOT NULL DEFAULT '0',
  `playedtime` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `deathstate` int NOT NULL DEFAULT '0',
  `TalentResetTimes` int NOT NULL DEFAULT '0',
  `first_login` tinyint(1) NOT NULL DEFAULT '0',
  `login_flags` int unsigned NOT NULL DEFAULT '0',
  `arenaPoints` int NOT NULL,
  `totalstableslots` int unsigned NOT NULL DEFAULT '0',
  `instance_id` int NOT NULL,
  `entrypointmap` int NOT NULL,
  `entrypointx` float NOT NULL,
  `entrypointy` float NOT NULL,
  `entrypointz` float NOT NULL,
  `entrypointo` float NOT NULL,
  `entrypointinstance` int NOT NULL,
  `taxi_path` int NOT NULL,
  `taxi_lastnode` int NOT NULL,
  `taxi_mountid` int NOT NULL,
  `transporter` int NOT NULL,
  `transporter_xdiff` float NOT NULL,
  `transporter_ydiff` float NOT NULL,
  `transporter_zdiff` float NOT NULL,
  `transporter_odiff` float NOT NULL,
  `actions1` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `actions2` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `auras` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `finished_quests` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `finisheddailies` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `honorRolloverTime` int NOT NULL DEFAULT '0',
  `killsToday` int NOT NULL DEFAULT '0',
  `killsYesterday` int NOT NULL DEFAULT '0',
  `killsLifeTime` int NOT NULL DEFAULT '0',
  `honorToday` int NOT NULL DEFAULT '0',
  `honorYesterday` int NOT NULL DEFAULT '0',
  `honorPoints` int NOT NULL DEFAULT '0',
  `drunkValue` int NOT NULL DEFAULT '0',
  `glyphs1` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `talents1` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `glyphs2` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `talents2` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `numspecs` int NOT NULL DEFAULT '1',
  `currentspec` int NOT NULL DEFAULT '0',
  `talentpoints` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `firsttalenttree` int DEFAULT NULL,
  `phase` int unsigned NOT NULL DEFAULT '1',
  `CanGainXp` int unsigned NOT NULL DEFAULT '1',
  `data` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `resettalents` int unsigned NOT NULL DEFAULT '0',
  `rbg_daily` tinyint(1) NOT NULL DEFAULT '0' COMMENT 'Boolean already done a daily rbg?',
  `dungeon_difficulty` smallint unsigned NOT NULL DEFAULT '0',
  `raid_difficulty` smallint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`),
  KEY `acct` (`acct`),
  KEY `name` (`name`),
  KEY `b` (`banned`),
  KEY `c` (`online`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `characters`;
/*!40000 ALTER TABLE `characters` DISABLE KEYS */;
/*!40000 ALTER TABLE `characters` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `character_achievement` (
  `guid` int unsigned NOT NULL DEFAULT '0',
  `achievement` int unsigned NOT NULL DEFAULT '0',
  `date` int unsigned DEFAULT NULL,
  PRIMARY KEY (`guid`,`achievement`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `character_achievement`;
/*!40000 ALTER TABLE `character_achievement` DISABLE KEYS */;
/*!40000 ALTER TABLE `character_achievement` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `character_achievement_progress` (
  `guid` int unsigned NOT NULL DEFAULT '0',
  `criteria` int unsigned NOT NULL DEFAULT '0',
  `counter` int DEFAULT NULL,
  `date` int unsigned DEFAULT NULL,
  PRIMARY KEY (`guid`,`criteria`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `character_achievement_progress`;
/*!40000 ALTER TABLE `character_achievement_progress` DISABLE KEYS */;
/*!40000 ALTER TABLE `character_achievement_progress` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `character_db_version` (
  `LastUpdate` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`LastUpdate`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `character_db_version`;
/*!40000 ALTER TABLE `character_db_version` DISABLE KEYS */;
INSERT INTO `character_db_version` (`LastUpdate`) VALUES
	('20201216-00_rename_event_properties');
/*!40000 ALTER TABLE `character_db_version` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `charters` (
  `charterId` int NOT NULL,
  `charterType` int NOT NULL DEFAULT '0',
  `leaderGuid` int unsigned NOT NULL DEFAULT '0',
  `guildName` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `itemGuid` bigint unsigned NOT NULL DEFAULT '0',
  `signer1` int unsigned NOT NULL DEFAULT '0',
  `signer2` int unsigned NOT NULL DEFAULT '0',
  `signer3` int unsigned NOT NULL DEFAULT '0',
  `signer4` int unsigned NOT NULL DEFAULT '0',
  `signer5` int unsigned NOT NULL DEFAULT '0',
  `signer6` int unsigned NOT NULL DEFAULT '0',
  `signer7` int unsigned NOT NULL DEFAULT '0',
  `signer8` int unsigned NOT NULL DEFAULT '0',
  `signer9` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`charterId`),
  UNIQUE KEY `leaderGuid` (`charterType`,`leaderGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `charters`;
/*!40000 ALTER TABLE `charters` DISABLE KEYS */;
/*!40000 ALTER TABLE `charters` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `clientaddons` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `crc` bigint DEFAULT NULL,
  `banned` int NOT NULL DEFAULT '0',
  `showinlist` int NOT NULL DEFAULT '0',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `version` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `index` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `clientaddons`;
/*!40000 ALTER TABLE `clientaddons` DISABLE KEYS */;
/*!40000 ALTER TABLE `clientaddons` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `command_overrides` (
  `command_name` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `access_level` varchar(10) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  PRIMARY KEY (`command_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `command_overrides`;
/*!40000 ALTER TABLE `command_overrides` DISABLE KEYS */;
/*!40000 ALTER TABLE `command_overrides` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `corpses` (
  `guid` bigint unsigned NOT NULL DEFAULT '0',
  `positionX` float NOT NULL DEFAULT '0',
  `positionY` float NOT NULL DEFAULT '0',
  `positionZ` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `zoneId` int NOT NULL DEFAULT '38',
  `mapId` int NOT NULL DEFAULT '0',
  `instanceId` int NOT NULL DEFAULT '0',
  `data` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  PRIMARY KEY (`guid`),
  KEY `b` (`instanceId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `corpses`;
/*!40000 ALTER TABLE `corpses` DISABLE KEYS */;
/*!40000 ALTER TABLE `corpses` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `equipmentsets` (
  `ownerguid` int unsigned NOT NULL DEFAULT '1',
  `setGUID` int unsigned NOT NULL DEFAULT '1',
  `setid` int unsigned NOT NULL DEFAULT '1',
  `setname` varchar(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `iconname` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `head` int unsigned NOT NULL DEFAULT '0',
  `neck` int unsigned NOT NULL DEFAULT '0',
  `shoulders` int unsigned NOT NULL DEFAULT '0',
  `body` int unsigned NOT NULL DEFAULT '0',
  `chest` int unsigned NOT NULL DEFAULT '0',
  `waist` int unsigned NOT NULL DEFAULT '0',
  `legs` int unsigned NOT NULL DEFAULT '0',
  `feet` int unsigned NOT NULL DEFAULT '0',
  `wrists` int unsigned NOT NULL DEFAULT '0',
  `hands` int unsigned NOT NULL DEFAULT '0',
  `finger1` int unsigned NOT NULL DEFAULT '0',
  `finger2` int unsigned NOT NULL DEFAULT '0',
  `trinket1` int unsigned NOT NULL DEFAULT '0',
  `trinket2` int unsigned NOT NULL DEFAULT '0',
  `back` int unsigned NOT NULL DEFAULT '0',
  `mainhand` int unsigned NOT NULL DEFAULT '0',
  `offhand` int unsigned NOT NULL DEFAULT '0',
  `ranged` int unsigned NOT NULL DEFAULT '0',
  `tabard` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ownerguid`,`setGUID`,`setid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `equipmentsets`;
/*!40000 ALTER TABLE `equipmentsets` DISABLE KEYS */;
/*!40000 ALTER TABLE `equipmentsets` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `gameevent_save` (
  `event_entry` int unsigned NOT NULL,
  `state` tinyint unsigned NOT NULL DEFAULT '0',
  `next_start` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`event_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `gameevent_save`;
/*!40000 ALTER TABLE `gameevent_save` DISABLE KEYS */;
/*!40000 ALTER TABLE `gameevent_save` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `gm_survey` (
  `survey_id` int unsigned NOT NULL,
  `guid` int unsigned NOT NULL DEFAULT '0',
  `main_survey` int unsigned NOT NULL DEFAULT '0',
  `comment` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `create_time` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`survey_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='GM Survey';

DELETE FROM `gm_survey`;
/*!40000 ALTER TABLE `gm_survey` DISABLE KEYS */;
/*!40000 ALTER TABLE `gm_survey` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `gm_survey_answers` (
  `survey_id` int unsigned NOT NULL,
  `question_id` int unsigned NOT NULL DEFAULT '0',
  `answer_id` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`survey_id`,`question_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='GM Survey';

DELETE FROM `gm_survey_answers`;
/*!40000 ALTER TABLE `gm_survey_answers` DISABLE KEYS */;
/*!40000 ALTER TABLE `gm_survey_answers` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `gm_tickets` (
  `ticketid` int NOT NULL,
  `playerGuid` int NOT NULL,
  `name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `level` int NOT NULL DEFAULT '0',
  `map` int NOT NULL DEFAULT '0',
  `posX` float NOT NULL DEFAULT '0',
  `posY` float NOT NULL DEFAULT '0',
  `posZ` float NOT NULL DEFAULT '0',
  `message` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `timestamp` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `deleted` int unsigned NOT NULL DEFAULT '0',
  `assignedto` int NOT NULL DEFAULT '0',
  `comment` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  UNIQUE KEY `guid` (`ticketid`),
  UNIQUE KEY `guid_2` (`ticketid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `gm_tickets`;
/*!40000 ALTER TABLE `gm_tickets` DISABLE KEYS */;
/*!40000 ALTER TABLE `gm_tickets` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `groups` (
  `group_id` int NOT NULL,
  `group_type` tinyint NOT NULL,
  `subgroup_count` tinyint NOT NULL,
  `loot_method` tinyint NOT NULL,
  `loot_threshold` tinyint NOT NULL,
  `difficulty` int NOT NULL DEFAULT '0',
  `raiddifficulty` int NOT NULL DEFAULT '0',
  `assistant_leader` int NOT NULL DEFAULT '0',
  `main_tank` int NOT NULL DEFAULT '0',
  `main_assist` int NOT NULL DEFAULT '0',
  `group1member1` int NOT NULL,
  `group1member2` int NOT NULL,
  `group1member3` int NOT NULL,
  `group1member4` int NOT NULL,
  `group1member5` int NOT NULL,
  `group2member1` int NOT NULL,
  `group2member2` int NOT NULL,
  `group2member3` int NOT NULL,
  `group2member4` int NOT NULL,
  `group2member5` int NOT NULL,
  `group3member1` int NOT NULL,
  `group3member2` int NOT NULL,
  `group3member3` int NOT NULL,
  `group3member4` int NOT NULL,
  `group3member5` int NOT NULL,
  `group4member1` int NOT NULL,
  `group4member2` int NOT NULL,
  `group4member3` int NOT NULL,
  `group4member4` int NOT NULL,
  `group4member5` int NOT NULL,
  `group5member1` int NOT NULL,
  `group5member2` int NOT NULL,
  `group5member3` int NOT NULL,
  `group5member4` int NOT NULL,
  `group5member5` int NOT NULL,
  `group6member1` int NOT NULL,
  `group6member2` int NOT NULL,
  `group6member3` int NOT NULL,
  `group6member4` int NOT NULL,
  `group6member5` int NOT NULL,
  `group7member1` int NOT NULL,
  `group7member2` int NOT NULL,
  `group7member3` int NOT NULL,
  `group7member4` int NOT NULL,
  `group7member5` int NOT NULL,
  `group8member1` int NOT NULL,
  `group8member2` int NOT NULL,
  `group8member3` int NOT NULL,
  `group8member4` int NOT NULL,
  `group8member5` int NOT NULL,
  `timestamp` int NOT NULL,
  `instanceids` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  PRIMARY KEY (`group_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `groups`;
/*!40000 ALTER TABLE `groups` DISABLE KEYS */;
/*!40000 ALTER TABLE `groups` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `guilds` (
  `guildId` bigint NOT NULL AUTO_INCREMENT,
  `guildName` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `leaderGuid` bigint NOT NULL DEFAULT '0',
  `emblemStyle` int NOT NULL DEFAULT '0',
  `emblemColor` int NOT NULL DEFAULT '0',
  `borderStyle` int NOT NULL DEFAULT '0',
  `borderColor` int NOT NULL DEFAULT '0',
  `backgroundColor` int NOT NULL DEFAULT '0',
  `guildInfo` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `motd` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `createdate` int NOT NULL,
  `bankBalance` bigint unsigned NOT NULL,
  `guildLevel` int NOT NULL DEFAULT '1',
  `guildExperience` bigint NOT NULL DEFAULT '0',
  `todayExperience` bigint NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`),
  UNIQUE KEY `guildId` (`guildId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `guilds`;
/*!40000 ALTER TABLE `guilds` DISABLE KEYS */;
/*!40000 ALTER TABLE `guilds` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `guild_bank_items` (
  `guildId` int NOT NULL,
  `tabId` int NOT NULL,
  `slotId` int NOT NULL,
  `itemGuid` int NOT NULL,
  PRIMARY KEY (`guildId`,`tabId`,`slotId`),
  KEY `a` (`guildId`),
  KEY `b` (`tabId`),
  KEY `c` (`slotId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `guild_bank_items`;
/*!40000 ALTER TABLE `guild_bank_items` DISABLE KEYS */;
/*!40000 ALTER TABLE `guild_bank_items` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `guild_bank_logs` (
  `guildId` int unsigned NOT NULL DEFAULT '0',
  `logGuid` int unsigned NOT NULL DEFAULT '0' COMMENT 'Log id for this guild',
  `tabId` tinyint unsigned NOT NULL DEFAULT '0',
  `eventType` tinyint unsigned NOT NULL DEFAULT '0',
  `playerGuid` int unsigned NOT NULL DEFAULT '0',
  `itemOrMoney` int unsigned NOT NULL DEFAULT '0',
  `itemStackCount` smallint unsigned NOT NULL DEFAULT '0',
  `destTabId` tinyint unsigned NOT NULL DEFAULT '0',
  `timeStamp` int unsigned NOT NULL DEFAULT '0' COMMENT 'UNIX time',
  PRIMARY KEY (`guildId`,`logGuid`,`tabId`),
  KEY `guildid_key` (`guildId`),
  KEY `Idx_PlayerGuid` (`playerGuid`),
  KEY `Idx_LogGuid` (`logGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `guild_bank_logs`;
/*!40000 ALTER TABLE `guild_bank_logs` DISABLE KEYS */;
/*!40000 ALTER TABLE `guild_bank_logs` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `guild_bank_rights` (
  `guildId` int unsigned NOT NULL DEFAULT '0',
  `tabId` tinyint unsigned NOT NULL DEFAULT '0',
  `rankId` tinyint unsigned NOT NULL DEFAULT '0',
  `bankRight` tinyint unsigned NOT NULL DEFAULT '0',
  `slotPerDay` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`,`tabId`,`rankId`),
  KEY `guildid_key` (`guildId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `guild_bank_rights`;
/*!40000 ALTER TABLE `guild_bank_rights` DISABLE KEYS */;
/*!40000 ALTER TABLE `guild_bank_rights` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `guild_bank_tabs` (
  `guildId` int NOT NULL,
  `tabId` int NOT NULL,
  `tabName` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `tabIcon` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `tabInfo` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  PRIMARY KEY (`guildId`,`tabId`),
  KEY `a` (`guildId`),
  KEY `b` (`tabId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `guild_bank_tabs`;
/*!40000 ALTER TABLE `guild_bank_tabs` DISABLE KEYS */;
/*!40000 ALTER TABLE `guild_bank_tabs` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `guild_finder_applicant` (
  `guildId` int unsigned DEFAULT NULL,
  `playerGuid` int unsigned DEFAULT NULL,
  `availability` tinyint unsigned DEFAULT '0',
  `classRole` tinyint unsigned DEFAULT '0',
  `interests` tinyint unsigned DEFAULT '0',
  `comment` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `submitTime` int unsigned DEFAULT NULL,
  UNIQUE KEY `guildId` (`guildId`,`playerGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `guild_finder_applicant`;
/*!40000 ALTER TABLE `guild_finder_applicant` DISABLE KEYS */;
/*!40000 ALTER TABLE `guild_finder_applicant` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `guild_finder_guild_settings` (
  `guildId` int unsigned NOT NULL,
  `availability` tinyint unsigned NOT NULL DEFAULT '0',
  `classRoles` tinyint unsigned NOT NULL DEFAULT '0',
  `interests` tinyint unsigned NOT NULL DEFAULT '0',
  `level` tinyint unsigned NOT NULL DEFAULT '1',
  `listed` tinyint unsigned NOT NULL DEFAULT '0',
  `comment` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`guildId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `guild_finder_guild_settings`;
/*!40000 ALTER TABLE `guild_finder_guild_settings` DISABLE KEYS */;
/*!40000 ALTER TABLE `guild_finder_guild_settings` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `guild_logs` (
  `guildId` int unsigned NOT NULL,
  `logGuid` int unsigned NOT NULL COMMENT 'Log id for this guild',
  `eventType` tinyint unsigned NOT NULL,
  `playerGuid1` int unsigned NOT NULL,
  `playerGuid2` int unsigned NOT NULL,
  `newRank` tinyint unsigned NOT NULL,
  `timeStamp` int unsigned NOT NULL COMMENT 'UNIX time',
  PRIMARY KEY (`guildId`,`logGuid`),
  KEY `Idx_PlayerGuid1` (`playerGuid1`),
  KEY `Idx_PlayerGuid2` (`playerGuid2`),
  KEY `Idx_LogGuid` (`logGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `guild_logs`;
/*!40000 ALTER TABLE `guild_logs` DISABLE KEYS */;
/*!40000 ALTER TABLE `guild_logs` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `guild_members` (
  `guildId` int unsigned NOT NULL,
  `playerid` int unsigned NOT NULL,
  `guildRank` tinyint unsigned NOT NULL,
  `publicNote` varchar(31) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `officerNote` varchar(31) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  UNIQUE KEY `guid_key` (`playerid`),
  KEY `guildid_key` (`guildId`),
  KEY `guildid_rank_key` (`guildId`,`guildRank`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `guild_members`;
/*!40000 ALTER TABLE `guild_members` DISABLE KEYS */;
/*!40000 ALTER TABLE `guild_members` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `guild_members_withdraw` (
  `guid` int unsigned NOT NULL,
  `tab0` int unsigned NOT NULL DEFAULT '0',
  `tab1` int unsigned NOT NULL DEFAULT '0',
  `tab2` int unsigned NOT NULL DEFAULT '0',
  `tab3` int unsigned NOT NULL DEFAULT '0',
  `tab4` int unsigned NOT NULL DEFAULT '0',
  `tab5` int unsigned NOT NULL DEFAULT '0',
  `tab6` int unsigned NOT NULL DEFAULT '0',
  `tab7` int unsigned NOT NULL DEFAULT '0',
  `money` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `guild_members_withdraw`;
/*!40000 ALTER TABLE `guild_members_withdraw` DISABLE KEYS */;
/*!40000 ALTER TABLE `guild_members_withdraw` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `guild_news_log` (
  `guildId` int unsigned NOT NULL DEFAULT '0',
  `logGuid` int unsigned NOT NULL DEFAULT '0' COMMENT 'Log id for this guild',
  `eventType` tinyint unsigned NOT NULL DEFAULT '0',
  `playerGuid` int unsigned NOT NULL DEFAULT '0',
  `flags` int unsigned NOT NULL DEFAULT '0',
  `value` int unsigned NOT NULL DEFAULT '0',
  `timeStamp` int unsigned NOT NULL DEFAULT '0' COMMENT 'UNIX time',
  PRIMARY KEY (`guildId`,`logGuid`),
  KEY `guildid_key` (`guildId`),
  KEY `Idx_PlayerGuid` (`playerGuid`),
  KEY `Idx_LogGuid` (`logGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `guild_news_log`;
/*!40000 ALTER TABLE `guild_news_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `guild_news_log` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `guild_ranks` (
  `guildId` int unsigned NOT NULL DEFAULT '0',
  `rankId` int NOT NULL DEFAULT '0',
  `rankName` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `rankRights` int unsigned NOT NULL DEFAULT '0',
  `goldLimitPerDay` int NOT NULL DEFAULT '0',
  `bankTabFlags0` int NOT NULL DEFAULT '0',
  `itemStacksPerDay0` int NOT NULL DEFAULT '0',
  `bankTabFlags1` int NOT NULL DEFAULT '0',
  `itemStacksPerDay1` int NOT NULL DEFAULT '0',
  `bankTabFlags2` int NOT NULL DEFAULT '0',
  `itemStacksPerDay2` int NOT NULL DEFAULT '0',
  `bankTabFlags3` int NOT NULL DEFAULT '0',
  `itemStacksPerDay3` int NOT NULL DEFAULT '0',
  `bankTabFlags4` int NOT NULL DEFAULT '0',
  `itemStacksPerDay4` int NOT NULL DEFAULT '0',
  `bankTabFlags5` int NOT NULL DEFAULT '0',
  `itemStacksPerDay5` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`,`rankId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `guild_ranks`;
/*!40000 ALTER TABLE `guild_ranks` DISABLE KEYS */;
/*!40000 ALTER TABLE `guild_ranks` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `instanceids` (
  `playerguid` int unsigned NOT NULL DEFAULT '0',
  `mapid` int unsigned NOT NULL DEFAULT '0',
  `mode` int unsigned NOT NULL DEFAULT '0',
  `instanceid` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`playerguid`,`mapid`,`mode`),
  KEY `ix_instanceid` (`playerguid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Player / InstanceID - Reference Table';

DELETE FROM `instanceids`;
/*!40000 ALTER TABLE `instanceids` DISABLE KEYS */;
/*!40000 ALTER TABLE `instanceids` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `instances` (
  `id` int NOT NULL,
  `mapid` int NOT NULL,
  `creation` int NOT NULL,
  `expiration` int NOT NULL,
  `killed_npc_guids` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `difficulty` int NOT NULL,
  `creator_group` int NOT NULL,
  `creator_guid` int NOT NULL,
  `persistent` tinyint NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `a` (`mapid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `instances`;
/*!40000 ALTER TABLE `instances` DISABLE KEYS */;
/*!40000 ALTER TABLE `instances` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `lag_reports` (
  `lag_id` int unsigned NOT NULL AUTO_INCREMENT,
  `player` int unsigned NOT NULL,
  `account` int unsigned NOT NULL,
  `lag_type` smallint unsigned NOT NULL,
  `map_id` int unsigned DEFAULT '0',
  `position_x` float DEFAULT '0',
  `position_y` float DEFAULT '0',
  `position_z` float DEFAULT '0',
  `timestamp` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`lag_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `lag_reports`;
/*!40000 ALTER TABLE `lag_reports` DISABLE KEYS */;
/*!40000 ALTER TABLE `lag_reports` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `lfg_data` (
  `guid` bigint NOT NULL,
  `dungeon` int NOT NULL,
  `state` int NOT NULL,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `lfg_data`;
/*!40000 ALTER TABLE `lfg_data` DISABLE KEYS */;
/*!40000 ALTER TABLE `lfg_data` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `mailbox` (
  `message_id` int NOT NULL DEFAULT '0',
  `message_type` int NOT NULL DEFAULT '0',
  `player_guid` int NOT NULL DEFAULT '0',
  `sender_guid` bigint unsigned NOT NULL DEFAULT '0',
  `subject` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `body` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `money` int NOT NULL DEFAULT '0',
  `attached_item_guids` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `cod` int NOT NULL DEFAULT '0',
  `stationary` bigint unsigned NOT NULL DEFAULT '0',
  `expiry_time` int NOT NULL DEFAULT '0',
  `delivery_time` int NOT NULL DEFAULT '0',
  `checked_flag` int unsigned NOT NULL DEFAULT '0',
  `deleted_flag` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`message_id`),
  KEY `b` (`player_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `mailbox`;
/*!40000 ALTER TABLE `mailbox` DISABLE KEYS */;
/*!40000 ALTER TABLE `mailbox` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `playerbugreports` (
  `UID` int unsigned NOT NULL,
  `AccountID` int unsigned NOT NULL,
  `TimeStamp` int unsigned NOT NULL,
  `Suggestion` int unsigned NOT NULL,
  `Type` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `Content` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  PRIMARY KEY (`UID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `playerbugreports`;
/*!40000 ALTER TABLE `playerbugreports` DISABLE KEYS */;
/*!40000 ALTER TABLE `playerbugreports` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `playercooldowns` (
  `player_guid` int NOT NULL,
  `cooldown_type` int NOT NULL COMMENT '0 is spell, 1 is item, 2 is spell category',
  `cooldown_misc` int NOT NULL COMMENT 'spellid/itemid/category',
  `cooldown_expire_time` int NOT NULL COMMENT 'expiring time in unix epoch format',
  `cooldown_spellid` int NOT NULL COMMENT 'spell that cast it',
  `cooldown_itemid` int NOT NULL COMMENT 'item that cast it'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `playercooldowns`;
/*!40000 ALTER TABLE `playercooldowns` DISABLE KEYS */;
/*!40000 ALTER TABLE `playercooldowns` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `playerdeletedspells` (
  `GUID` int unsigned NOT NULL,
  `SpellID` int unsigned NOT NULL,
  PRIMARY KEY (`GUID`,`SpellID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `playerdeletedspells`;
/*!40000 ALTER TABLE `playerdeletedspells` DISABLE KEYS */;
/*!40000 ALTER TABLE `playerdeletedspells` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `playeritems` (
  `ownerguid` int unsigned NOT NULL DEFAULT '0',
  `guid` bigint NOT NULL AUTO_INCREMENT,
  `entry` int unsigned NOT NULL DEFAULT '0',
  `wrapped_item_id` int NOT NULL DEFAULT '0',
  `wrapped_creator` int NOT NULL DEFAULT '0',
  `creator` int unsigned NOT NULL DEFAULT '0',
  `count` int unsigned NOT NULL DEFAULT '0',
  `charges` int NOT NULL DEFAULT '0',
  `flags` int unsigned NOT NULL DEFAULT '0',
  `randomprop` int unsigned NOT NULL DEFAULT '0',
  `randomsuffix` int NOT NULL,
  `itemtext` int unsigned NOT NULL DEFAULT '0',
  `durability` int unsigned NOT NULL DEFAULT '0',
  `containerslot` int DEFAULT '-1',
  `slot` int NOT NULL DEFAULT '0',
  `enchantments` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `duration_expireson` int unsigned NOT NULL DEFAULT '0',
  `refund_purchasedon` int unsigned NOT NULL DEFAULT '0',
  `refund_costid` int unsigned NOT NULL DEFAULT '0',
  `text` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  PRIMARY KEY (`guid`),
  KEY `ownerguid` (`ownerguid`),
  KEY `itemtext` (`itemtext`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `playeritems`;
/*!40000 ALTER TABLE `playeritems` DISABLE KEYS */;
/*!40000 ALTER TABLE `playeritems` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `playerpets` (
  `ownerguid` bigint NOT NULL DEFAULT '0',
  `petnumber` int NOT NULL DEFAULT '0',
  `name` varchar(21) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `entry` int unsigned NOT NULL DEFAULT '0',
  `xp` int NOT NULL DEFAULT '0',
  `active` tinyint(1) NOT NULL DEFAULT '0',
  `level` int NOT NULL DEFAULT '0',
  `actionbar` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `happinessupdate` int NOT NULL DEFAULT '0',
  `reset_time` int unsigned NOT NULL DEFAULT '0',
  `reset_cost` int NOT NULL DEFAULT '0',
  `spellid` int unsigned NOT NULL DEFAULT '0',
  `petstate` int unsigned NOT NULL DEFAULT '0',
  `alive` tinyint(1) NOT NULL DEFAULT '1',
  `talentpoints` int unsigned NOT NULL DEFAULT '0',
  `current_power` int unsigned NOT NULL DEFAULT '1',
  `current_hp` int unsigned NOT NULL DEFAULT '1',
  `current_happiness` int unsigned NOT NULL DEFAULT '1000000',
  `renamable` int unsigned NOT NULL DEFAULT '1',
  `type` int unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`ownerguid`,`petnumber`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `playerpets`;
/*!40000 ALTER TABLE `playerpets` DISABLE KEYS */;
/*!40000 ALTER TABLE `playerpets` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `playerpetspells` (
  `ownerguid` bigint NOT NULL DEFAULT '0',
  `petnumber` int NOT NULL DEFAULT '0',
  `spellid` int NOT NULL DEFAULT '0',
  `flags` int NOT NULL DEFAULT '0',
  KEY `a` (`ownerguid`),
  KEY `b` (`petnumber`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `playerpetspells`;
/*!40000 ALTER TABLE `playerpetspells` DISABLE KEYS */;
/*!40000 ALTER TABLE `playerpetspells` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `playerreputations` (
  `guid` int unsigned NOT NULL,
  `faction` int unsigned NOT NULL,
  `flag` int unsigned NOT NULL DEFAULT '0',
  `basestanding` int NOT NULL DEFAULT '0',
  `standing` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`faction`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `playerreputations`;
/*!40000 ALTER TABLE `playerreputations` DISABLE KEYS */;
/*!40000 ALTER TABLE `playerreputations` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `playerskills` (
  `GUID` int unsigned NOT NULL,
  `SkillID` int unsigned NOT NULL,
  `CurrentValue` int unsigned NOT NULL,
  `MaximumValue` int unsigned NOT NULL,
  PRIMARY KEY (`GUID`,`SkillID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `playerskills`;
/*!40000 ALTER TABLE `playerskills` DISABLE KEYS */;
/*!40000 ALTER TABLE `playerskills` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `playerspells` (
  `GUID` int unsigned NOT NULL,
  `SpellID` int unsigned NOT NULL,
  PRIMARY KEY (`GUID`,`SpellID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `playerspells`;
/*!40000 ALTER TABLE `playerspells` DISABLE KEYS */;
/*!40000 ALTER TABLE `playerspells` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `playersummons` (
  `ownerguid` int unsigned NOT NULL DEFAULT '0',
  `entry` int unsigned NOT NULL DEFAULT '0',
  `name` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  KEY `a` (`ownerguid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `playersummons`;
/*!40000 ALTER TABLE `playersummons` DISABLE KEYS */;
/*!40000 ALTER TABLE `playersummons` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `playersummonspells` (
  `ownerguid` bigint NOT NULL DEFAULT '0',
  `entryid` int NOT NULL DEFAULT '0',
  `spellid` int NOT NULL DEFAULT '0',
  KEY `a` (`ownerguid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `playersummonspells`;
/*!40000 ALTER TABLE `playersummonspells` DISABLE KEYS */;
/*!40000 ALTER TABLE `playersummonspells` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `questlog` (
  `player_guid` bigint unsigned NOT NULL DEFAULT '0',
  `quest_id` bigint unsigned NOT NULL DEFAULT '0',
  `slot` int unsigned NOT NULL DEFAULT '0',
  `expirytime` int unsigned NOT NULL DEFAULT '0',
  `explored_area1` bigint unsigned NOT NULL DEFAULT '0',
  `explored_area2` bigint unsigned NOT NULL DEFAULT '0',
  `explored_area3` bigint unsigned NOT NULL DEFAULT '0',
  `explored_area4` bigint unsigned NOT NULL DEFAULT '0',
  `mob_kill1` bigint NOT NULL DEFAULT '0',
  `mob_kill2` bigint NOT NULL DEFAULT '0',
  `mob_kill3` bigint NOT NULL DEFAULT '0',
  `mob_kill4` bigint NOT NULL DEFAULT '0',
  `completed` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`player_guid`,`quest_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `questlog`;
/*!40000 ALTER TABLE `questlog` DISABLE KEYS */;
/*!40000 ALTER TABLE `questlog` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `server_settings` (
  `setting_id` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `setting_value` int NOT NULL,
  PRIMARY KEY (`setting_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `server_settings`;
/*!40000 ALTER TABLE `server_settings` DISABLE KEYS */;
INSERT INTO `server_settings` (`setting_id`, `setting_value`) VALUES
	('last_arena_update_time', 1455039763),
	('last_daily_update_time', 0);
/*!40000 ALTER TABLE `server_settings` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `social_friends` (
  `character_guid` int NOT NULL,
  `friend_guid` int NOT NULL,
  `note` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  PRIMARY KEY (`character_guid`,`friend_guid`),
  KEY `a` (`character_guid`),
  KEY `b` (`friend_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `social_friends`;
/*!40000 ALTER TABLE `social_friends` DISABLE KEYS */;
/*!40000 ALTER TABLE `social_friends` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `social_ignores` (
  `character_guid` int NOT NULL,
  `ignore_guid` int NOT NULL,
  PRIMARY KEY (`character_guid`,`ignore_guid`),
  KEY `a` (`character_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `social_ignores`;
/*!40000 ALTER TABLE `social_ignores` DISABLE KEYS */;
/*!40000 ALTER TABLE `social_ignores` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `tutorials` (
  `playerId` bigint unsigned NOT NULL DEFAULT '0',
  `tut0` bigint unsigned NOT NULL DEFAULT '0',
  `tut1` bigint unsigned NOT NULL DEFAULT '0',
  `tut2` bigint unsigned NOT NULL DEFAULT '0',
  `tut3` bigint unsigned NOT NULL DEFAULT '0',
  `tut4` bigint unsigned NOT NULL DEFAULT '0',
  `tut5` bigint unsigned NOT NULL DEFAULT '0',
  `tut6` bigint unsigned NOT NULL DEFAULT '0',
  `tut7` bigint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`playerId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `tutorials`;
/*!40000 ALTER TABLE `tutorials` DISABLE KEYS */;
/*!40000 ALTER TABLE `tutorials` ENABLE KEYS */;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IFNULL(@OLD_FOREIGN_KEY_CHECKS, 1) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40111 SET SQL_NOTES=IFNULL(@OLD_SQL_NOTES, 1) */;
