
-- worldstring_tables rows for the four battlegrounds added this cycle were never added - their
-- GetNameID() placeholders (108/120/699/708) pointed at ids with no row, so
-- WorldSession::localizedWorldSrv() logged "is a bad WorldString TEXT!" and then dereferenced the
-- null lookup result, crashing the WorldMap thread on every win/loss announcement. Same id space
-- and plain-name convention as the existing rows (38 Alterac Valley, 39 Warsong Gulch, ...).
-- Ids 108/120/699/708 were free (checked against the whole table, max existing entry was 503).
INSERT INTO `worldstring_tables` (`entry`, `text`) VALUES
(108, 'Twin Peaks'),
(120, 'Battle for Gilneas'),
(699, 'Temple of Kotmogu'),
(708, 'Silvershard Mines');

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('188', '20260905-04_mop_battleground_worldstrings');
