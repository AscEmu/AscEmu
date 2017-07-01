--
-- Rewrite guild system drop unused tables
--
DROP TABLE IF EXISTS `guilds`;
DROP TABLE IF EXISTS `guild_ranks`;
DROP TABLE IF EXISTS `guild_bankitems`;
DROP TABLE IF EXISTS `guild_banklogs`;
DROP TABLE IF EXISTS `guild_banktabs`;
DROP TABLE IF EXISTS `guild_data`;
DROP TABLE IF EXISTS `guild_logs`;

--
-- Table structure for `guild`
--
DROP TABLE IF EXISTS `guild`;
CREATE TABLE `guild` (
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
-- Table structure for `guild_achievement`
--
DROP TABLE IF EXISTS `guild_achievement`;
CREATE TABLE `guild_achievement` (
  `guildId` int(10) unsigned NOT NULL,
  `achievement` smallint(5) unsigned NOT NULL,
  `date` int(10) unsigned NOT NULL default '0',
  `guids` text NOT NULL,
  PRIMARY KEY  (`guildId`,`achievement`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for `guild_achievement_progress`
--
DROP TABLE IF EXISTS `guild_achievement_progress`;
CREATE TABLE `guild_achievement_progress` (
  `guildId` int(10) unsigned NOT NULL,
  `criteria` smallint(5) unsigned NOT NULL,
  `counter` int(10) unsigned NOT NULL,
  `date` int(10) unsigned NOT NULL default '0',
  `completedGuid` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guildId`,`criteria`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for `guild_bank_eventlog`
--
DROP TABLE IF EXISTS `guild_bank_eventlog`;
CREATE TABLE `guild_bank_eventlog` (
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
-- Table structure for `guild_bank_item`
--
DROP TABLE IF EXISTS `guild_bank_item`;
CREATE TABLE `guild_bank_item` (
  `guildId` int(10) unsigned NOT NULL default '0',
  `tabId` tinyint(3) unsigned NOT NULL default '0',
  `slotId` tinyint(3) unsigned NOT NULL default '0',
  `itemGuid` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guildId`,`tabId`,`slotId`),
  KEY `guildid_key` (`guildId`),
  KEY `Idx_item_guid` (`itemGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for `guild_bank_right`
--
DROP TABLE IF EXISTS `guild_bank_right`;
CREATE TABLE `guild_bank_right` (
  `guildId` int(10) unsigned NOT NULL default '0',
  `tabId` tinyint(3) unsigned NOT NULL default '0',
  `rid` tinyint(3) unsigned NOT NULL default '0',
  `gbright` tinyint(3) unsigned NOT NULL default '0',
  `slotPerDay` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guildId`,`tabId`,`rid`),
  KEY `guildid_key` (`guildId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for `guild_bank_tab`
--
DROP TABLE IF EXISTS `guild_bank_tab`;
CREATE TABLE `guild_bank_tab` (
  `guildId` int(10) unsigned NOT NULL default '0',
  `tabId` tinyint(3) unsigned NOT NULL default '0',
  `tabName` varchar(16) NOT NULL default '',
  `tabIcon` varchar(100) NOT NULL default '',
  `tabText` varchar(500) default NULL,
  PRIMARY KEY  (`guildId`,`tabId`),
  KEY `guildid_key` (`guildId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for `guild_eventlog`
--
DROP TABLE IF EXISTS `guild_eventlog`;
CREATE TABLE `guild_eventlog` (
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
-- Table structure for `guild_finder_applicant`
--
DROP TABLE IF EXISTS `guild_finder_applicant`;
CREATE TABLE `guild_finder_applicant` (
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
-- Table structure for `guild_finder_guild_settings`
--
DROP TABLE IF EXISTS `guild_finder_guild_settings`;
CREATE TABLE `guild_finder_guild_settings` (
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
-- Table structure for `guild_member`
--
DROP TABLE IF EXISTS `guild_member`;
CREATE TABLE `guild_member` (
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
-- Table structure for `guild_member_withdraw`
--
DROP TABLE IF EXISTS `guild_member_withdraw`;
CREATE TABLE `guild_member_withdraw` (
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
-- Table structure for `guild_newslog`
--
DROP TABLE IF EXISTS `guild_newslog`;
CREATE TABLE `guild_newslog` (
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
-- Table structure for `guild_rank`
--
DROP TABLE IF EXISTS `guild_rank`;
CREATE TABLE `guild_rank` (
  `guildId` int(10) unsigned NOT NULL default '0',
  `rid` tinyint(3) unsigned NOT NULL,
  `rname` varchar(20) NOT NULL default '',
  `rights` mediumint(8) unsigned NOT NULL default '0',
  `bankMoneyPerDay` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guildId`,`rid`),
  KEY `Idx_rid` (`rid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Guild System';

--
-- Update character_db_version
--
UPDATE `character_db_version` SET `LastUpdate`='2017-06-30_01_guild' WHERE `LastUpdate`='2017-06-25_01_clienbtaddons';
