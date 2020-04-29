ALTER TABLE `account_data` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `account_permissions` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `arenateams` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `auctions` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `banned_char_log` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `banned_names` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `calendar_events` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `calendar_invites` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `characters` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `character_achievement` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `character_achievement_progress` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `character_db_version` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `charters` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `clientaddons` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `command_overrides` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `corpses` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `equipmentsets` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `event_save` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `gm_survey` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `gm_survey_answers` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `gm_tickets` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `groups` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `guilds` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `guild_bank_items` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `guild_bank_logs` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `guild_bank_rights` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `guild_bank_tabs` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `guild_finder_applicant` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `guild_finder_guild_settings` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `guild_logs` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `guild_members` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `guild_members_withdraw` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `guild_news_log` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `guild_ranks` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `instanceids` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `instances` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `lag_reports` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `lfg_data` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `mailbox` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `playerbugreports` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `playercooldowns` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `playerdeletedspells` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `playeritems` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `playerpets` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `playerpetspells` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `playerreputations` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `playerskills` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `playerspells` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `playersummons` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `playersummonspells` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `questlog` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `server_settings` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `social_friends` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `social_ignores` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `tutorials` COLLATE='utf8mb4_unicode_ci';

-- TABLE `account_permissions`
ALTER TABLE `account_permissions`
    CHANGE COLUMN `permissions` `permissions` VARCHAR(100) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `id`,
    CHANGE COLUMN `name` `name` VARCHAR(50) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `permissions`;

-- TABLE `arenateams`
ALTER TABLE `arenateams`
    CHANGE COLUMN `name` `name` VARCHAR(100) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `leader`,
    CHANGE COLUMN `data` `data` VARCHAR(100) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `rating`,
    CHANGE COLUMN `player_data1` `player_data1` VARCHAR(60) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `ranking`,
    CHANGE COLUMN `player_data2` `player_data2` VARCHAR(60) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `player_data1`,
    CHANGE COLUMN `player_data3` `player_data3` VARCHAR(60) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `player_data2`,
    CHANGE COLUMN `player_data4` `player_data4` VARCHAR(60) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `player_data3`,
    CHANGE COLUMN `player_data5` `player_data5` VARCHAR(60) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `player_data4`,
    CHANGE COLUMN `player_data6` `player_data6` VARCHAR(60) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `player_data5`,
    CHANGE COLUMN `player_data7` `player_data7` VARCHAR(60) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `player_data6`,
    CHANGE COLUMN `player_data8` `player_data8` VARCHAR(60) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `player_data7`,
    CHANGE COLUMN `player_data9` `player_data9` VARCHAR(60) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `player_data8`,
    CHANGE COLUMN `player_data10` `player_data10` VARCHAR(60) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `player_data9`;

-- TABLE `banned_char_log`
ALTER TABLE `banned_char_log`
    CHANGE COLUMN `banned_by` `banned_by` VARCHAR(50) NOT NULL COLLATE 'utf8mb4_unicode_ci' FIRST,
    CHANGE COLUMN `banned_player` `banned_player` VARCHAR(50) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `banned_by`,
    CHANGE COLUMN `reason` `reason` VARCHAR(100) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `banned_until`;

-- TABLE `banned_names`
ALTER TABLE `banned_names`
    CHANGE COLUMN `name` `name` VARCHAR(30) NOT NULL COLLATE 'utf8mb4_unicode_ci' FIRST;

-- TABLE `calendar_events`
ALTER TABLE `calendar_events`
    CHANGE COLUMN `title` `title` VARCHAR(100) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `creator`,
    CHANGE COLUMN `description` `description` VARCHAR(100) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `title`;

-- TABLE `calendar_invites`
ALTER TABLE `calendar_invites`
    CHANGE COLUMN `text` `text` VARCHAR(100) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `rank`;

-- TABLE `characters`
ALTER TABLE `characters`
    CHANGE COLUMN `name` `name` VARCHAR(21) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `acct`,
    CHANGE COLUMN `exploration_data` `exploration_data` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `active_cheats`,
    CHANGE COLUMN `taximask` `taximask` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `zoneId`,
    CHANGE COLUMN `banReason` `banReason` VARCHAR(100) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `banned`,
    CHANGE COLUMN `playedtime` `playedtime` TEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `restTime`,
    CHANGE COLUMN `actions1` `actions1` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `transporter_odiff`,
    CHANGE COLUMN `actions2` `actions2` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `actions1`,
    CHANGE COLUMN `auras` `auras` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `actions2`,
    CHANGE COLUMN `finished_quests` `finished_quests` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `auras`,
    CHANGE COLUMN `finisheddailies` `finisheddailies` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `finished_quests`,
    CHANGE COLUMN `glyphs1` `glyphs1` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `drunkValue`,
    CHANGE COLUMN `talents1` `talents1` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `glyphs1`,
    CHANGE COLUMN `glyphs2` `glyphs2` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `talents1`,
    CHANGE COLUMN `talents2` `talents2` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `glyphs2`,
    CHANGE COLUMN `talentpoints` `talentpoints` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `currentspec`,
    CHANGE COLUMN `data` `data` LONGTEXT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `CanGainXp`;

-- TABLE `character_db_version`
ALTER TABLE `character_db_version`
    CHANGE COLUMN `LastUpdate` `LastUpdate` VARCHAR(100) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' FIRST;

-- TABLE `charters`
ALTER TABLE `charters`
    CHANGE COLUMN `guildName` `guildName` VARCHAR(32) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `leaderGuid`;

-- TABLE `clientaddons`
ALTER TABLE `clientaddons`
    CHANGE COLUMN `name` `name` VARCHAR(50) NULL DEFAULT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `id`,
    CHANGE COLUMN `version` `version` VARCHAR(60) NULL DEFAULT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `timestamp`;

-- TABLE `command_overrides`
ALTER TABLE `command_overrides`
    CHANGE COLUMN `command_name` `command_name` VARCHAR(100) NOT NULL COLLATE 'utf8mb4_unicode_ci' FIRST,
    CHANGE COLUMN `access_level` `access_level` VARCHAR(10) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `command_name`;

-- TABLE `corpses`
ALTER TABLE `corpses`
    CHANGE COLUMN `data` `data` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `instanceId`;

-- TABLE `equipmentsets`
ALTER TABLE `equipmentsets`
    CHANGE COLUMN `setname` `setname` VARCHAR(16) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `setid`,
    CHANGE COLUMN `iconname` `iconname` VARCHAR(100) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `setname`;

-- TABLE `gm_survey`
ALTER TABLE `gm_survey`
    CHANGE COLUMN `comment` `comment` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `main_survey`;

-- TABLE `gm_tickets`
ALTER TABLE `gm_tickets`
    CHANGE COLUMN `name` `name` VARCHAR(255) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `playerGuid`,
    CHANGE COLUMN `message` `message` TEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `posZ`,
    CHANGE COLUMN `timestamp` `timestamp` TEXT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `message`,
    CHANGE COLUMN `comment` `comment` TEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `assignedto`;

-- TABLE `groups`
ALTER TABLE `groups`
    CHANGE COLUMN `instanceids` `instanceids` TEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `timestamp`;

-- TABLE `guilds`
ALTER TABLE `guilds`
    CHANGE COLUMN `guildName` `guildName` VARCHAR(32) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `guildId`,
    CHANGE COLUMN `guildInfo` `guildInfo` VARCHAR(100) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `backgroundColor`,
    CHANGE COLUMN `motd` `motd` VARCHAR(100) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `guildInfo`;

-- TABLE `guild_bank_tabs`
ALTER TABLE `guild_bank_tabs`
    CHANGE COLUMN `tabName` `tabName` VARCHAR(100) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `tabId`,
    CHANGE COLUMN `tabIcon` `tabIcon` VARCHAR(100) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `tabName`,
    CHANGE COLUMN `tabInfo` `tabInfo` VARCHAR(100) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `tabIcon`;

-- TABLE `guild_finder_applicant`
ALTER TABLE `guild_finder_applicant`
    CHANGE COLUMN `comment` `comment` VARCHAR(100) NULL DEFAULT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `interests`;

-- TABLE `guild_finder_guild_settings`
ALTER TABLE `guild_finder_guild_settings`
    CHANGE COLUMN `comment` `comment` VARCHAR(100) NULL DEFAULT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `listed`;

-- TABLE `guild_members`
ALTER TABLE `guild_members`
    CHANGE COLUMN `publicNote` `publicNote` VARCHAR(31) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `guildRank`,
    CHANGE COLUMN `officerNote` `officerNote` VARCHAR(31) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `publicNote`;

-- TABLE `guild_ranks`
ALTER TABLE `guild_ranks`
    CHANGE COLUMN `rankName` `rankName` VARCHAR(100) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `rankId`;

-- TABLE `instances`
ALTER TABLE `instances`
    CHANGE COLUMN `killed_npc_guids` `killed_npc_guids` TEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `expiration`;

-- TABLE `mailbox`
ALTER TABLE `mailbox`
    CHANGE COLUMN `subject` `subject` VARCHAR(100) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `sender_guid`,
    CHANGE COLUMN `body` `body` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `subject`,
    CHANGE COLUMN `attached_item_guids` `attached_item_guids` VARCHAR(100) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `money`;

-- TABLE `playerbugreports`
ALTER TABLE `playerbugreports`
    CHANGE COLUMN `Type` `Type` TEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `Suggestion`,
    CHANGE COLUMN `Content` `Content` TEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `Type`;

-- TABLE `playerpets`
ALTER TABLE `playerpets`
	CHANGE COLUMN `name` `name` VARCHAR(21) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `petnumber`,
	CHANGE COLUMN `actionbar` `actionbar` VARCHAR(100) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `level`;

-- TABLE `playersummons`
ALTER TABLE `playersummons`
    CHANGE COLUMN `name` `name` VARCHAR(64) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `entry`;

-- TABLE `server_settings`
ALTER TABLE `server_settings`
    CHANGE COLUMN `setting_id` `setting_id` VARCHAR(100) NOT NULL COLLATE 'utf8mb4_unicode_ci' FIRST;

-- TABLE `social_friends`
ALTER TABLE `social_friends`
    CHANGE COLUMN `note` `note` VARCHAR(100) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `friend_guid`;

-- TABLE `playeritems`
ALTER TABLE `playeritems`
	CHANGE COLUMN `enchantments` `enchantments` LONGTEXT NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `slot`,
	CHANGE COLUMN `text` `text` TEXT(65535) NOT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `refund_costid`;

UPDATE `character_db_version` SET LastUpdate = '20200221-00_utf8mb4_unicode_ci';
