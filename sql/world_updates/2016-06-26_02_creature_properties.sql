--
-- Create creature_properties based on creature_proto
--
CREATE TABLE `creature_properties` LIKE `creature_proto`; 
INSERT `creature_properties` SELECT * FROM `creature_proto`;

--
-- Add creature_names columns to creature_properties
--
ALTER TABLE `creature_properties` ADD `killcredit1` int(10) default 0 after `entry`;
ALTER TABLE `creature_properties` ADD `killcredit2` int(10) default 0 after `killcredit1`;
ALTER TABLE `creature_properties` ADD `male_displayid` int(10) default 0 after `killcredit2`;
ALTER TABLE `creature_properties` ADD `female_displayid` int(10) default 0 after `male_displayid`;
ALTER TABLE `creature_properties` ADD `male_displayid2` int(10) default 0 after `female_displayid`;
ALTER TABLE `creature_properties` ADD `female_displayid2` int(10) default 0 after `male_displayid2`;
ALTER TABLE `creature_properties` ADD `name` varchar(100) default '' after `female_displayid2`;
ALTER TABLE `creature_properties` ADD `subname` varchar(100) default '' after `name`;
ALTER TABLE `creature_properties` ADD `info_str` varchar(500) default '' after `subname`;
ALTER TABLE `creature_properties` ADD `flags1` int(10) default 0 after `info_str`;
ALTER TABLE `creature_properties` ADD `type` int(10) default 0 after `flags1`;
ALTER TABLE `creature_properties` ADD `family` int(10) default 0 after `type`;
ALTER TABLE `creature_properties` ADD `rank` int(10) default 0 after `family`;
ALTER TABLE `creature_properties` ADD `encounter` int(10) default 0 after `rank`;
ALTER TABLE `creature_properties` ADD `unknown_float1` float default 0 after `encounter`;
ALTER TABLE `creature_properties` ADD `unknown_float2` float default 0 after `unknown_float1`;
ALTER TABLE `creature_properties` ADD `leader` tinyint(3) default 0 after `unknown_float2`;
ALTER TABLE `creature_properties` ADD `questitem1` int(11) default 0 after `rooted`;
ALTER TABLE `creature_properties` ADD `questitem2` int(11) default 0 after `questitem1`;
ALTER TABLE `creature_properties` ADD `questitem3` int(11) default 0 after `questitem2`;
ALTER TABLE `creature_properties` ADD `questitem4` int(11) default 0 after `questitem3`;
ALTER TABLE `creature_properties` ADD `questitem5` int(11) default 0 after `questitem4`;
ALTER TABLE `creature_properties` ADD `questitem6` int(11) default 0 after `questitem5`;
ALTER TABLE `creature_properties` ADD `waypointid` int(10) default 0 after `questitem6`;

--
-- Fill new creature_properties (creature_names) columns
--
UPDATE `creature_properties` cp, `creature_names` cn
SET cp.killcredit1 = cn.killcredit1,
cp.killcredit2 = cn.killcredit2,
cp.male_displayid = cn.male_displayid,
cp.female_displayid = cn.female_displayid,
cp.male_displayid2 = cn.male_displayid2,
cp.female_displayid2 = cn.female_displayid2,
cp.name = cn.name,
cp.subname = cn.subname,
cp.info_str = cn.info_str,
cp.flags1 = cn.flags1,
cp.type = cn.type,
cp.family = cn.family,
cp.rank = cn.rank,
cp.encounter = cn.encounter,
cp.unknown_float1 = cn.unknown_float1,
cp.unknown_float2 = cn.unknown_float2,
cp.leader = cn.leader,
cp.questitem1 = cn.questitem1,
cp.questitem2 = cn.questitem2,
cp.questitem3 = cn.questitem3,
cp.questitem4 = cn.questitem4,
cp.questitem5 = cn.questitem5,
cp.questitem6 = cn.questitem6,
cp.waypointid = cn.waypointid
WHERE cp.entry = cn.entry;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-06-26_02_creature_properties' WHERE `LastUpdate` = '2016-06-26_01_table_rename';
