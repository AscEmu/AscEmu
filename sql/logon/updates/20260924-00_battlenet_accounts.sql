
SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for battlenet_accounts
-- ----------------------------
DROP TABLE IF EXISTS `battlenet_accounts`;
CREATE TABLE `battlenet_accounts`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `email` varchar(320) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `srp_version` tinyint UNSIGNED NOT NULL DEFAULT 1,
  `srp_salt` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `srp_verifier` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `battle_tag` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `country` char(2) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'CH',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `uq_battlenet_accounts_email`(`email` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 5 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for battlenet_game_accounts
-- ----------------------------
DROP TABLE IF EXISTS `battlenet_game_accounts`;
CREATE TABLE `battlenet_game_accounts`  (
  `battlenet_account_id` int UNSIGNED NOT NULL,
  `game_account_id` int UNSIGNED NOT NULL,
  PRIMARY KEY (`battlenet_account_id`, `game_account_id`) USING BTREE,
  UNIQUE INDEX `uq_battlenet_game_accounts_game_account`(`game_account_id` ASC) USING BTREE,
  INDEX `idx_battlenet_game_accounts_bnet`(`battlenet_account_id` ASC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;


INSERT INTO `ascemu_logon`.`logon_db_version` (`id`, `LastUpdate`) VALUES (6, '20260924-00_battlenet_accounts')
