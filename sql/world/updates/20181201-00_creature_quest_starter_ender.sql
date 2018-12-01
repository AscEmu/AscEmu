-- Set up quest_starter build
ALTER TABLE `creature_quest_starter`
ADD COLUMN `min_build` INT(6) NOT NULL DEFAULT 12340 AFTER `quest`,
ADD COLUMN `max_build` INT(6) NOT NULL DEFAULT 12340 AFTER `min_build`,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`id`,`quest`,`min_build`),
ADD UNIQUE KEY `unique_index` (`id`, `quest`,`min_build`) USING BTREE;

-- Set up quest_ender build
ALTER TABLE `creature_quest_finisher`
ADD COLUMN `min_build` INT(6) NOT NULL DEFAULT 12340 AFTER `quest`,
ADD COLUMN `max_build` INT(6) NOT NULL DEFAULT 12340 AFTER `min_build`,
DROP PRIMARY KEY,
ADD PRIMARY KEY (`id`,`quest`,`min_build`),
ADD UNIQUE KEY `unique_index` (`id`, `quest`,`min_build`) USING BTREE;
