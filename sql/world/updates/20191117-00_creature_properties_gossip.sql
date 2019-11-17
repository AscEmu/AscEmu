ALTER TABLE `creature_properties`
ADD COLUMN `gossipId` int(6) NOT NULL DEFAULT 0 AFTER `waypointid`;

-- StormwindGuard
UPDATE `creature_properties` SET `gossipId`=114 WHERE `entry` IN(68, 1976, 29712);
-- DarnassusGuard
UPDATE `creature_properties` SET `gossipId`=122 WHERE `entry`=4262;
-- UndercityGuard
UPDATE `creature_properties` SET `gossipId`=142 WHERE `entry`=5624;
-- UndercityGuardOverseer
UPDATE `creature_properties` SET `gossipId`=163 WHERE `entry`=36213;
-- ThunderbluffGuard
UPDATE `creature_properties` SET `gossipId`=152 WHERE `entry`=3084;
-- GoldshireGuard
UPDATE `creature_properties` SET `gossipId`=132 WHERE `entry`=1423;
-- TeldrassilGuard
UPDATE `creature_properties` SET `gossipId`=172 WHERE `entry`=3571;
-- SilvermoonGuard
UPDATE `creature_properties` SET `gossipId`=180 WHERE `entry`=16222;
-- ExodarGuard
UPDATE `creature_properties` SET `gossipId`=191 WHERE `entry` IN(16733, 20674);
-- OrgrimmarGuard
UPDATE `creature_properties` SET `gossipId`=724 WHERE `entry`=3296;
-- BloodhoofGuard
UPDATE `creature_properties` SET `gossipId`=751 WHERE `entry` IN(3222, 3224, 3220, 3219, 3217, 3215, 3218, 3221, 3223, 3212);
-- RazorHillGuard
UPDATE `creature_properties` SET `gossipId`=989 WHERE `entry`=5953;
-- BrillGuard
UPDATE `creature_properties` SET `gossipId`=1003 WHERE `entry` IN(5725, 1738, 1652, 1746, 1745, 1743, 1744, 1496, 1742);
-- IronforgeGuard
UPDATE `creature_properties` SET `gossipId`=1012 WHERE `entry`=5595;
-- KharanosGuard
UPDATE `creature_properties` SET `gossipId`=1035 WHERE `entry`=727;
-- FalconwingGuard
UPDATE `creature_properties` SET `gossipId`=1047 WHERE `entry`=16221;
-- AzureWatchGuard
UPDATE `creature_properties` SET `gossipId`=1058 WHERE `entry`=18038;
-- ShattrathGuard
UPDATE `creature_properties` SET `gossipId`=1068 WHERE `entry` IN(19687, 18568, 18549);
-- DalaranGuard
UPDATE `creature_properties` SET `gossipId`=1095 WHERE `entry` IN(32675, 32676, 32677, 32678, 32679, 32680, 32681, 32683, 32684, 32685, 32686, 32687, 32688, 32689, 32690, 32691, 32692, 32693);

INSERT INTO `world_db_version` VALUES ('50', '20191117-00_creature_properties_gossip');
