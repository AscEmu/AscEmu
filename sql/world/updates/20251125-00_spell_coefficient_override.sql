DROP TABLE IF EXISTS `spell_coefficient_override`;
CREATE TABLE `spell_coefficient_override` (
    `spell_id` INT UNSIGNED NOT NULL DEFAULT '0',
    `effectIndex` TINYINT UNSIGNED NOT NULL DEFAULT '0',
    `min_build` INT UNSIGNED NOT NULL DEFAULT '12340',
    `max_build` INT UNSIGNED NOT NULL DEFAULT '12340',
    `sp_coefficient` FLOAT NULL DEFAULT NULL,
    `ap_coefficient` FLOAT NULL DEFAULT NULL,
    `flags` TINYINT UNSIGNED NOT NULL DEFAULT '0',
    `description` VARCHAR(100) NULL DEFAULT NULL COLLATE 'utf8mb4_unicode_ci',
    PRIMARY KEY (`spell_id`, `effectIndex`, `min_build`) USING BTREE,
    INDEX `spell_id` (`spell_id`) USING BTREE
) ENGINE=InnoDB COMMENT='Spell System' COLLATE='utf8mb4_unicode_ci';

SET @classic := 5875;
SET @tbc := 8606;
SET @wotlk := 12340;
SET @cata := 15595;
SET @mop := 18414;

INSERT INTO `spell_coefficient_override` VALUES
    -- Seal of Command 29% holy sp coeff classic - tbc
    ('20424', '0', @classic, @tbc, '0.29', null, '0', 'Seal of Command proc'),
    -- Seal of Command 5% ap coeff wotlk
    ('20424', '0', @wotlk, @wotlk, null, '0.05', '0', 'Seal of Command proc'),
    -- Seal of Light/Insight 15% holy sp coeff + 15% ap coeff wotlk - mop (missing from DBC)
    ('20167', '0', @wotlk, @mop, '0.15', '0.15', '0', 'Seal of Light/Insight proc'),
    -- Seal of Righteousness 10.8% holy sp coeff (two-handed weapon) classic - tbc
    ('25742', '0', @classic, @tbc, '0.108', null, '0', 'Seal of Righteousness proc'),
    -- Seal of Righteousness 4.4% holy sp coeff + 2.2% ap coeff wotlk (missing from DBC)
    ('25742', '0', @wotlk, @wotlk, '0.044', '0.022', '0', 'Seal of Righteousness proc'),
    -- Seal of Righteousness 2.2% holy sp coeff + 1.1% ap coeff cata (missing from DBC)
    ('25742', '0', @cata, @cata, '0.022', '0.011', '0', 'Seal of Righteousness proc'),
    ('101423', '0', @cata, @cata, '0.022', '0.011', '0', 'Seal of Righteousness aoe proc'),
    -- Seal of Justice 1% holy sp coeff + 0.5% ap coeff cata (missing from DBC)
    ('20170', '1', @cata, @cata, '0.01', '0.005', '0', 'Seal of Justice proc'),
    -- Seal of Vengeance dot 3.4% holy sp coeff tbc
    ('31803', '0', @tbc, @tbc, '0.034', null, '0', 'Seal of Vengeance dot'),
    -- Seal of Vengeance/Corruption dot 2.5% ap coeff wotlk (sp coeff in DBC)
    ('31803', '0', @wotlk, @wotlk, null, '0.025', '0', 'Seal of Vengeance dot'),
    ('53742', '0', @wotlk, @wotlk, null, '0.025', '0', 'Seal of Corruption dot'),
    -- Seal of Vengeance direct proc 2.2% holy sp coeff tbc
    ('42463', '0', @tbc, @tbc, '0.022', null, '0', 'Seal of Vengeance direct proc'),
    -- Judgement of Righteousness 50% holy sp coeff classic
    ('20187', '0', @classic, @classic, '0.5', null, '0', 'Judgement of Righteousness'),
    -- Judgement of Righteousness 72.8% holy sp coeff tbc
    ('20187', '0', @tbc, @tbc, '0.728', null, '0', 'Judgement of Righteousness'),
    -- Judgement of Righteousness 20% ap coeff wotlk - cata (sp coeff in DBC)
    ('20187', '0', @wotlk, @cata, null, '0.2', '0', 'Judgement of Righteousness'),
    -- Judgement of Command 42.86% holy sp coeff classic - tbc
    ('20467', '0', @classic, @tbc, '0.4286', null, '0', 'Judgement of Command'),
    -- Judgement of Command 13% holy sp coeff + 8% ap coeff wotlk (missing from DBC)
    ('20467', '1', @wotlk, @wotlk, '0.13', '0.08', '0', 'Judgement of Command'),
    -- Judgement of Vengeance 42.86% holy sp coeff tbc
    ('31804', '0', @tbc, @tbc, '0.4286', null, '0', 'Judgement of Vengeance'),
    -- Judgement of Vengeance/Corruption 14% ap coeff wotlk (sp coeff in DBC)
    ('31804', '0', @wotlk, @wotlk, null, '0.14', '0', 'Judgement of Vengeance'),
    ('53733', '0', @wotlk, @wotlk, null, '0.14', '0', 'Judgement of Corruption'),
    -- Judgement of Blood 42.86% holy sp coeff tbc
    ('31898', '0', @tbc, @tbc, '0.4286', null, '0', 'Judgement of Blood'),
    -- Judgement of Blood/Martyr 18% holy sp coeff + 11% ap coeff wotlk (missing from DBC)
    ('31898', '1', @wotlk, @wotlk, '0.18', '0.11', '0', 'Judgement of Blood'),
    ('53726', '1', @wotlk, @wotlk, '0.18', '0.11', '0', 'Judgement of the Martyr'),
    -- Generic Judgement damage 16% ap coeff wotlk (sp coeff in DBC)
    ('54158', '0', @wotlk, @wotlk, null, '0.16', '0', 'Judgement'),

    -- Death and Decay 4.805% ap coeff wotlk
    ('52212', '0', @wotlk, @wotlk, null, '0.04805', '0', 'Death and Decay'),

    -- Earth Shield 28.57% nature sp coeff tbc
    ('974', '0', @tbc, @tbc, '0.2857', null, '0', 'Earth Shield'),
    -- Earth Shield 42.05% nature sp coeff wotlk (missing from DBC)
    ('974', '0', @wotlk, @wotlk, '0.4205', null, '0', 'Earth Shield'),
    -- Earth Shield 15.2% nature sp coeff cata and mop (missing from DBC)
    ('974', '0', @cata, @mop, '0.152', null, '0', 'Earth Shield');

-- Add missing spell ranks
DELETE FROM `spell_ranks` WHERE `first_spell` IN ('20167', '20168', '20425', '20267', '20268');
INSERT INTO `spell_ranks` VALUES
    ('20167', @classic, @tbc, '20167', '1', 'Seal of Light'),
    ('20333', @classic, @tbc, '20167', '2', 'Seal of Light'),
    ('20334', @classic, @tbc, '20167', '3', 'Seal of Light'),
    ('20340', @classic, @tbc, '20167', '4', 'Seal of Light'),
    ('27161', @tbc, @tbc, '20167', '5', 'Seal of Light'),
    ('20168', @classic, @tbc, '20168', '1', 'Seal of Wisdom'),
    ('20350', @classic, @tbc, '20168', '2', 'Seal of Wisdom'),
    ('20351', @classic, @tbc, '20168', '3', 'Seal of Wisdom'),
    ('27167', @tbc, @tbc, '20168', '4', 'Seal of Wisdom'),
    ('20425', @classic, @tbc, '20425', '1', 'Judgement of Command'),
    ('20962', @classic, @tbc, '20425', '2', 'Judgement of Command'),
    ('20961', @classic, @tbc, '20425', '3', 'Judgement of Command'),
    ('20967', @classic, @tbc, '20425', '4', 'Judgement of Command'),
    ('20968', @classic, @tbc, '20425', '5', 'Judgement of Command'),
    ('27172', @tbc, @tbc, '20425', '6', 'Judgement of Command'),
    ('20267', @classic, @tbc, '20267', '1', 'Judgement of Light'),
    ('20341', @classic, @tbc, '20267', '2', 'Judgement of Light'),
    ('20342', @classic, @tbc, '20267', '3', 'Judgement of Light'),
    ('20343', @classic, @tbc, '20267', '4', 'Judgement of Light'),
    ('27163', @tbc, @tbc, '20267', '5', 'Judgement of Light'),
    ('20268', @classic, @tbc, '20268', '1', 'Judgement of Wisdom'),
    ('20352', @classic, @tbc, '20268', '2', 'Judgement of Wisdom'),
    ('20353', @classic, @tbc, '20268', '3', 'Judgement of Wisdom'),
    ('27165', @tbc, @tbc, '20268', '4', 'Judgement of Wisdom');

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('158', '20251125-00_spell_coefficient_override');
