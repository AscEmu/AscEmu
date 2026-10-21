-- One account for every client version: the SRP credential of the Battle.net web login (6.x and later
-- clients) moves into `accounts`, the separate Battle.net account tables are dropped. Existing accounts
-- get their SRP credential with the next password change (account set password).

ALTER TABLE `accounts`
  ADD COLUMN `srp_salt` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'Battle.net login (SRP) salt, hex' AFTER `encrypted_password`,
  ADD COLUMN `srp_verifier` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'Battle.net login (SRP) verifier, hex' AFTER `srp_salt`;

DROP TABLE IF EXISTS `battlenet_game_accounts`;
DROP TABLE IF EXISTS `battlenet_accounts`;

INSERT INTO `logon_db_version` (`id`, `LastUpdate`) VALUES (7, '20261021-00_accounts_srp');
