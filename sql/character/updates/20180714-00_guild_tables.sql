/*
Update guilds table to handle cata specific fields too
*/

DROP TABLE IF EXISTS `guild_bankitems`;
DROP TABLE IF EXISTS `guild_banklogs`;
DROP TABLE IF EXISTS `guild_banktabs`;
DROP TABLE IF EXISTS `guild_data`;
DROP TABLE IF EXISTS `guild_logs`;
DROP TABLE IF EXISTS `guild_ranks`;
DROP TABLE IF EXISTS `guilds`;

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `guild_bank_items`
-- ----------------------------
DROP TABLE IF EXISTS `guild_bank_items`;
CREATE TABLE `guild_bank_items` (
  `guildId` int(30) NOT NULL,
  `tabId` int(30) NOT NULL,
  `slotId` int(30) NOT NULL,
  `itemGuid` int(30) NOT NULL,
  PRIMARY KEY (`guildId`,`tabId`,`slotId`),
  KEY `a` (`guildId`),
  KEY `b` (`tabId`),
  KEY `c` (`slotId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `guild_bank_logs`
-- ----------------------------
DROP TABLE IF EXISTS `guild_bank_logs`;
CREATE TABLE `guild_bank_logs` (
  `guildId` int(10) unsigned NOT NULL DEFAULT '0',
  `logGuid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Log id for this guild',
  `tabId` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `eventType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `playerGuid` int(10) unsigned NOT NULL DEFAULT '0',
  `itemOrMoney` int(10) unsigned NOT NULL DEFAULT '0',
  `itemStackCount` smallint(5) unsigned NOT NULL DEFAULT '0',
  `destTabId` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `timeStamp` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'UNIX time',
  PRIMARY KEY (`guildId`,`logGuid`,`tabId`),
  KEY `guildid_key` (`guildId`),
  KEY `Idx_PlayerGuid` (`playerGuid`),
  KEY `Idx_LogGuid` (`logGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `guild_bank_rights`
-- ----------------------------
DROP TABLE IF EXISTS `guild_bank_rights`;
CREATE TABLE `guild_bank_rights` (
  `guildId` int(10) unsigned NOT NULL DEFAULT '0',
  `tabId` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `rankId` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `bankRight` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `slotPerDay` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`,`tabId`,`rankId`),
  KEY `guildid_key` (`guildId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `guild_bank_tabs`
-- ----------------------------
DROP TABLE IF EXISTS `guild_bank_tabs`;
CREATE TABLE `guild_bank_tabs` (
  `guildId` int(30) NOT NULL,
  `tabId` int(30) NOT NULL,
  `tabName` varchar(200) NOT NULL,
  `tabIcon` varchar(200) NOT NULL,
  `tabInfo` varchar(200) NOT NULL,
  PRIMARY KEY (`guildId`,`tabId`),
  KEY `a` (`guildId`),
  KEY `b` (`tabId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `guild_logs`
-- ----------------------------
DROP TABLE IF EXISTS `guild_logs`;
CREATE TABLE `guild_logs` (
  `guildId` int(10) unsigned NOT NULL,
  `logGuid` int(10) unsigned NOT NULL COMMENT 'Log id for this guild',
  `eventType` tinyint(3) unsigned NOT NULL,
  `playerGuid1` int(10) unsigned NOT NULL,
  `playerGuid2` int(10) unsigned NOT NULL,
  `newRank` tinyint(3) unsigned NOT NULL,
  `timeStamp` int(10) unsigned NOT NULL COMMENT 'UNIX time',
  PRIMARY KEY (`guildId`,`logGuid`),
  KEY `Idx_PlayerGuid1` (`playerGuid1`),
  KEY `Idx_PlayerGuid2` (`playerGuid2`),
  KEY `Idx_LogGuid` (`logGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `guild_members`
-- ----------------------------
DROP TABLE IF EXISTS `guild_members`;
CREATE TABLE `guild_members` (
  `guildId` int(10) unsigned NOT NULL,
  `playerid` int(10) unsigned NOT NULL,
  `guildRank` tinyint(3) unsigned NOT NULL,
  `publicNote` varchar(31) NOT NULL DEFAULT '',
  `officerNote` varchar(31) NOT NULL DEFAULT '',
  UNIQUE KEY `guid_key` (`playerid`),
  KEY `guildid_key` (`guildId`),
  KEY `guildid_rank_key` (`guildId`,`guildRank`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `guild_members_withdraw`
-- ----------------------------
DROP TABLE IF EXISTS `guild_members_withdraw`;
CREATE TABLE `guild_members_withdraw` (
  `guid` int(10) unsigned NOT NULL,
  `tab0` int(10) unsigned NOT NULL DEFAULT '0',
  `tab1` int(10) unsigned NOT NULL DEFAULT '0',
  `tab2` int(10) unsigned NOT NULL DEFAULT '0',
  `tab3` int(10) unsigned NOT NULL DEFAULT '0',
  `tab4` int(10) unsigned NOT NULL DEFAULT '0',
  `tab5` int(10) unsigned NOT NULL DEFAULT '0',
  `tab6` int(10) unsigned NOT NULL DEFAULT '0',
  `tab7` int(10) unsigned NOT NULL DEFAULT '0',
  `money` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `guild_news_log`
-- ----------------------------
DROP TABLE IF EXISTS `guild_news_log`;
CREATE TABLE `guild_news_log` (
  `guildId` int(10) unsigned NOT NULL DEFAULT '0',
  `logGuid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Log id for this guild',
  `eventType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `playerGuid` int(10) unsigned NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `value` int(10) unsigned NOT NULL DEFAULT '0',
  `timeStamp` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'UNIX time',
  PRIMARY KEY (`guildId`,`logGuid`),
  KEY `guildid_key` (`guildId`),
  KEY `Idx_PlayerGuid` (`playerGuid`),
  KEY `Idx_LogGuid` (`logGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `guild_ranks`
-- ----------------------------
DROP TABLE IF EXISTS `guild_ranks`;
CREATE TABLE `guild_ranks` (
  `guildId` int(6) unsigned NOT NULL DEFAULT '0',
  `rankId` int(1) NOT NULL DEFAULT '0',
  `rankName` varchar(255) NOT NULL DEFAULT '',
  `rankRights` int(3) unsigned NOT NULL DEFAULT '0',
  `goldLimitPerDay` int(30) NOT NULL DEFAULT '0',
  `bankTabFlags0` int(30) NOT NULL DEFAULT '0',
  `itemStacksPerDay0` int(30) NOT NULL DEFAULT '0',
  `bankTabFlags1` int(30) NOT NULL DEFAULT '0',
  `itemStacksPerDay1` int(30) NOT NULL DEFAULT '0',
  `bankTabFlags2` int(30) NOT NULL DEFAULT '0',
  `itemStacksPerDay2` int(30) NOT NULL DEFAULT '0',
  `bankTabFlags3` int(30) NOT NULL DEFAULT '0',
  `itemStacksPerDay3` int(30) NOT NULL DEFAULT '0',
  `bankTabFlags4` int(30) NOT NULL DEFAULT '0',
  `itemStacksPerDay4` int(30) NOT NULL DEFAULT '0',
  `bankTabFlags5` int(30) NOT NULL DEFAULT '0',
  `itemStacksPerDay5` int(30) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`,`rankId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for `guilds`
-- ----------------------------
DROP TABLE IF EXISTS `guilds`;
CREATE TABLE `guilds` (
  `guildId` bigint(20) NOT NULL AUTO_INCREMENT,
  `guildName` varchar(32) NOT NULL DEFAULT '',
  `leaderGuid` bigint(20) NOT NULL DEFAULT '0',
  `emblemStyle` int(10) NOT NULL DEFAULT '0',
  `emblemColor` int(10) NOT NULL DEFAULT '0',
  `borderStyle` int(10) NOT NULL DEFAULT '0',
  `borderColor` int(10) NOT NULL DEFAULT '0',
  `backgroundColor` int(10) NOT NULL DEFAULT '0',
  `guildInfo` varchar(300) NOT NULL DEFAULT '',
  `motd` varchar(300) NOT NULL DEFAULT '',
  `createdate` int(30) NOT NULL,
  `bankBalance` bigint(30) unsigned NOT NULL,
  `guildLevel` int(10) NOT NULL DEFAULT '1',
  `guildExperience` bigint(20) NOT NULL DEFAULT '0',
  `todayExperience` bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`),
  UNIQUE KEY `guildId` (`guildId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

UPDATE `character_db_version` SET LastUpdate = '20180714-00_guild_tables';
