--
-- drop tables
--
DROP TABLE IF EXISTS `totemdisplayids`;

--
-- create tables
--
CREATE TABLE IF NOT EXISTS `totemdisplayids` (
  `displayid` int(10) unsigned NOT NULL DEFAULT '0',
  `draeneiid` int(10) unsigned NOT NULL DEFAULT '0',
  `trollid` int(10) unsigned NOT NULL DEFAULT '0',
  `orcid` int(10) unsigned NOT NULL DEFAULT '0',
  `taurenid` int(10) unsigned NOT NULL DEFAULT '0',
  `dwarfid` int(10) unsigned NOT NULL DEFAULT '0',
  `goblinid` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`displayid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

/*Data for the table `totemdisplayids` */
INSERT INTO `totemdisplayids` (`displayid`, `draeneiid`, `trollid`, `orcid`, `taurenid`, `dwarfid`, `goblinid`) VALUES
    (4587, 19075, 30763, 30759, 4587, 30755, 30784),
    (4588, 19073, 30761, 30757, 4588, 30753, 30782),
    (4589, 19074, 30762, 30758, 4589, 30754, 30783),
    (4590, 19071, 30760, 30756, 4590, 30736, 30781);

--
-- Update world_db_version`
--
UPDATE `world_db_version` SET `LastUpdate` = '2017-06-19_01_totemdisplayids' WHERE `LastUpdate` = '2017-06-15_01_remove_tables';