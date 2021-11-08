DROP TABLE IF EXISTS `playercreateinfo_spell_learn`;
CREATE TABLE `playercreateinfo_spell_learn` (
  `min_build` smallint NOT NULL DEFAULT '5875',
  `max_build` smallint NOT NULL DEFAULT '18414',
  `raceMask` int unsigned NOT NULL DEFAULT '0',
  `classMask` int NOT NULL DEFAULT '0',
  `spellid` int unsigned NOT NULL DEFAULT '0',
  `note` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT '',
  PRIMARY KEY (`min_build`,`max_build`,`raceMask`,`classMask`,`spellid`) USING BTREE,
  UNIQUE KEY `unique_index` (`min_build`,`max_build`,`raceMask`,`classMask`,`spellid`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Player System';

INSERT INTO `playercreateinfo_spell_learn` VALUES (15595,18414,512,4,80483,'Arcane Torrent - Focus'),(12340,18414,0,0,61437,'Opening'),(12340,18414,0,2,60091,'Judgement Anti-Parry/Dodge Passive'),(12340,18414,1024,128,59548,'Gift of the Naaru'),(12340,18414,1024,64,59547,'Gift of the Naaru'),(12340,18414,1024,32,59545,'Gift of the Naaru'),(12340,18414,1024,16,59544,'Gift of the Naaru'),(12340,18414,1024,4,59543,'Gift of the Naaru'),(12340,18414,1024,2,59542,'Gift of the Naaru'),(12340,18414,1024,128,59541,'Shadow Resistance'),(12340,18414,1024,64,59540,'Shadow Resistance'),(12340,18414,1024,32,59539,'Shadow Resistance'),(12340,18414,1024,16,59538,'Shadow Resistance'),(12340,18414,1024,4,59536,'Shadow Resistance'),(12340,18414,1024,2,59535,'Shadow Resistance'),(12340,18414,1024,1,59221,'Shadow Resistance'),(12340,18414,8,0,58984,'Shadowmeld'),(12340,12340,0,32,1180,'Daggers'),(12340,18414,0,256,58284,'Chaos Bolt Passive'),(12340,18414,2,32,54562,'Command'),(8606,8606,512,0,28734,'Mana Tap'),(15595,18414,0,35,49410,'Forceful Deflection'),(12340,18414,0,0,45927,'Summon Friend'),(12340,18414,0,32,45903,'Offensive State (DND)'),(15595,18414,0,2,35395,'Crusader Strike'),(8606,18414,0,4,34082,'Advantaged State (DND)'),(15595,18414,2,384,33702,'Blood Fury'),(5875,5875,2,0,20572,'Blood Fury'),(8606,12340,1133,1,198,'One-Handed Maces'),(12340,18414,1024,1,28880,'Gift of the Naaru'),(15595,18414,512,402,28730,'Arcane Torrent - Mana'),(8606,12340,1024,4,201,'One-Handed Swords'),(8606,12340,650,4,264,'Bows'),(8606,18414,512,8,25046,'Arcane Torrent - Energy'),(5875,8606,1,0,20600,'Perception'),(5875,8606,2,1243,21563,'Command'),(5875,5875,5,2,199,'Two-Handed Maces'),(8606,12340,1029,2,199,'Two-Handed Maces'),(5875,5875,16,1,202,'Two-Handed Swords'),(8606,8606,1040,1,202,'Two-Handed Swords'),(8606,8606,512,2,202,'Two-Handed Swords'),(15595,18414,0,44,674,'Dual Wield'),(12340,12340,0,32,196,'One-Handed Axes'),(8606,8606,1024,0,20579,'Shadow Resistance'),(5875,18414,2,4,20576,'Command'),(5875,18414,2,256,20575,'Command'),(12340,18414,2,45,20572,'Blood Fury'),(8606,12340,512,2,201,'One-Handed Swords'),(5875,5875,109,1,198,'One-Handed Maces'),(12340,18414,0,32,18630,'Heavy Runecloth Bandage'),(12340,18414,0,32,18629,'Runecloth Bandage'),(15595,18414,0,1101,15590,'Fist Weapons'),(12340,18414,0,32,10841,'Heavy Mageweave Bandage'),(12340,12340,0,37,197,'Two-Handed Axes'),(5875,12340,167,1,196,'One-Handed Axes'),(5875,12340,166,4,196,'One-Handed Axes'),(8606,12340,1029,2,198,'One-Handed Maces'),(12340,18414,0,1135,9077,'Leather'),(5875,18414,0,0,6603,'Auto Attack'),(12340,18414,1024,39,6562,'Heroic Presence'),(15595,18414,0,1147,198,'One-Handed Maces'),(5875,12340,32,1024,198,'One-Handed Maces'),(15595,18414,0,13,5011,'Crossbows'),(5875,8606,128,8,26297,'Berserking'),(5875,8606,128,1,26296,'Berserking'),(5875,5875,8,4,1180,'Daggers'),(8606,12340,520,4,1180,'Daggers'),(5875,12340,8,1024,1180,'Daggers'),(5875,12340,216,1,1180,'Daggers'),(15595,18414,0,4,3044,'Arcane Shot'),(12340,12340,0,32,201,'One-Handed Swords'),(15595,18414,0,9,2567,'Thrown'),(15595,18414,2102248,4460,1843,'Disarm'),(15595,18414,0,1533,1180,'Daggers'),(15595,18414,0,4,982,'Revive Pet'),(15595,18414,0,4,883,'Call Pet'),(12340,18414,0,35,8737,'Mail'),(15595,18414,0,256,688,'Summon Imp'),(5875,8606,128,212,20554,'Berserking'),(12340,12340,0,35,202,'Two-Handed Swords'),(15595,18414,0,13,266,'Guns'),(15595,18414,0,13,264,'Bows'),(15595,18414,0,1493,227,'Staves'),(8606,12340,1115,1,201,'One-Handed Swords'),(15595,18414,0,39,202,'Two-Handed Swords'),(15595,18414,0,431,201,'One-Handed Swords'),(15595,18414,0,1063,200,'Polearms'),(15595,18414,0,1123,199,'Two-Handed Maces'),(15595,18414,0,103,197,'Two-Handed Axes'),(15595,18414,0,111,196,'One-Handed Axes'),(12340,12340,0,40,674,'Dual Wield'),(12340,18414,0,32,61455,'Runic Focus'),(12340,18414,0,32,59921,'Frost Fever'),(12340,18414,0,32,59879,'Blood Plague'),(12340,18414,1,0,59752,'Every Man for Himself'),(12340,18414,4,0,59224,'Mace Specialization'),(12340,12340,1,0,58985,'Perception'),(5875,8606,8,0,20580,'Shadowmeld'),(12340,18414,128,0,58943,'Da Voodoo Shuffle'),(12340,18414,0,32,52665,'Sigil'),(12340,18414,512,32,50613,'Arcane Torrent - Runic Power'),(12340,18414,0,32,49576,'Death Grip'),(12340,12340,0,32,49410,'Forceful Deflection'),(12340,18414,0,32,48266,'Blood Presence'),(12340,18414,0,32,47541,'Death Coil'),(12340,18414,0,32,45902,'Blood Strike'),(12340,18414,0,32,45477,'Icy Touch'),(12340,18414,0,32,45462,'Plague Strike'),(8606,12340,2,256,33702,'Blood Fury'),(8606,18414,2,64,33697,'Blood Fury'),(12340,18414,0,32,33391,'Journeyman Riding'),(8606,18414,0,1,32215,'Victorious State'),(8606,8606,1024,0,28880,'Gift of the Naaru'),(8606,18414,1024,208,28878,'Heroic Presence'),(8606,18414,512,0,28877,'Arcane Affinity'),(8606,18414,1024,0,28875,'Gemcutting'),(8606,12340,512,406,28730,'Arcane Torrent - Mana'),(5875,18414,0,1024,27764,'Idol'),(5875,18414,0,64,27763,'Totem'),(5875,18414,0,2,27762,'Libram'),(12340,18414,128,0,26297,'Berserking'),(5875,15595,128,0,26290,'Bow Specialization'),(12340,18414,0,4,24949,'Defensive State 2 (DND)'),(5875,18414,0,0,22810,'Opening - No Text'),(5875,18414,0,0,22027,'Remove Insignia'),(5875,18414,0,0,21652,'Closing'),(5875,18414,0,0,21651,'Opening'),(5875,18414,0,8,21184,'Rogue Passive (DND)'),(5875,18414,0,2,21084,'Seal of Righteousness'),(5875,18414,8,0,21009,'Elusiveness'),(5875,18414,1,0,20864,'Mace Specialization'),(5875,18414,1,0,20599,'Diplomacy'),(5875,18414,1,0,20598,'The Human Spirit'),(5875,18414,1,0,20597,'Sword Specialization'),(5875,18414,4,0,20596,'Frost Resistance'),(5875,18414,4,0,20595,'Gun Specialization'),(5875,18414,4,0,20594,'Stoneform'),(5875,18414,64,0,20593,'Engineering Specialization'),(5875,18414,64,0,20592,'Arcane Resistance'),(5875,18414,64,0,20591,'Expansive Mind'),(5875,18414,64,0,20589,'Escape Artist'),(5875,18414,8,0,20585,'Wisp Spirit'),(5875,18414,8,0,20583,'Nature Resistance'),(5875,18414,8,0,20582,'Quickness'),(5875,18414,16,0,20579,'Shadow Resistance'),(5875,18414,16,0,20577,'Cannibalize'),(5875,18414,2,0,20574,'Axe Specialization'),(5875,18414,2,0,20573,'Hardiness'),(8606,8606,2,13,20572,'Blood Fury'),(5875,15595,128,0,20558,'Throwing Specialization'),(5875,18414,128,0,20557,'Beast Slaying'),(5875,18414,128,0,20555,'Regeneration'),(5875,18414,32,0,20552,'Cultivation'),(5875,18414,32,0,20551,'Nature Resistance'),(5875,18414,32,0,20550,'Endurance'),(5875,18414,32,0,20549,'War Stomp'),(5875,18414,0,8,16092,'Defensive State (DND)'),(5875,18414,0,4,13358,'Defensive State (DND)'),(12340,18414,0,32,10846,'First Aid'),(12340,18414,0,32,10840,'Mageweave Bandage'),(5875,18414,0,0,9125,'Generic'),(5875,18414,0,67,9116,'Shield'),(5875,18414,0,0,9078,'Cloth'),(5875,8606,0,1103,9077,'Leather'),(5875,8606,0,3,8737,'Mail'),(5875,18414,0,0,8386,'Attacking'),(12340,18414,0,32,7934,'Anti-Venom'),(12340,18414,0,32,7929,'Heavy Silk Bandage'),(12340,18414,0,32,7928,'Silk Bandage'),(5875,18414,16,0,7744,'Will of the Forsaken'),(5875,18414,0,0,7355,'Stuck'),(5875,18414,0,0,7267,'Grovel'),(5875,18414,0,0,7266,'Duel'),(8606,8606,1024,7,6562,'Heroic Presence'),(5875,18414,0,0,6478,'Opening'),(5875,18414,0,0,6477,'Opening'),(5875,18414,0,0,6247,'Opening'),(5875,18414,0,0,6246,'Closing'),(5875,18414,0,0,6233,'Closing'),(5875,18414,0,1,5301,'Defensive State (DND)'),(5875,18414,16,0,5227,'Underwater Breathing'),(5875,18414,0,1024,5185,'Healing Touch'),(5875,18414,0,1024,5176,'Wrath'),(8606,12340,1024,4,5011,'Crossbows'),(5875,18414,0,400,5009,'Wands'),(5875,18414,0,0,3365,'Opening'),(12340,18414,0,32,3278,'Heavy Wool Bandage'),(12340,18414,0,32,3277,'Wool Bandage'),(12340,18414,0,32,3276,'Heavy Linen Bandage'),(12340,18414,0,32,3275,'Linen Bandage'),(12340,12340,0,32,3127,'Parry'),(5875,18414,0,0,3050,'Detect'),(5875,12340,32,1,199,'Two-Handed Maces'),(5875,18414,0,4,2973,'Raptor Strike'),(5875,12340,128,1,2567,'Thrown'),(5875,12340,0,8,2567,'Thrown'),(5875,12340,4,0,2481,'Find Treasure'),(5875,18414,0,0,2479,'Honorless Target'),(5875,18414,0,1,2457,'Battle Stance'),(5875,18414,0,0,2382,'Generic'),(5875,18414,0,8,2098,'Eviscerate'),(5875,18414,0,16,2050,'Lesser Heal'),(5875,18414,0,8,1752,'Sinister Strike'),(5875,12340,0,264,1180,'Daggers'),(8606,18414,512,0,822,'Magic Resistance'),(12340,18414,0,32,750,'Plate Mail'),(5875,18414,0,256,687,'Demon Skin'),(5875,18414,0,256,686,'Shadow Bolt'),(5875,18414,0,2,635,'Holy Light'),(5875,18414,0,16,585,'Smite'),(5875,18414,0,0,522,'SPELLDEFENSE (DND)'),(5875,18414,0,64,403,'Lightning Bolt'),(5875,18414,0,64,331,'Healing Wave'),(5875,12340,36,4,266,'Guns'),(5875,5875,138,4,264,'Bows'),(5875,12340,0,1216,227,'Staves'),(5875,18414,0,0,204,'Defense'),(5875,18414,0,0,203,'Unarmed'),(12340,12340,0,272,227,'Staves'),(5875,5875,91,1,201,'One-Handed Swords'),(12340,12340,0,32,200,'Polearms'),(5875,12340,0,80,198,'One-Handed Maces'),(5875,8606,6,1,197,'Two-Handed Axes'),(5875,5875,5,2,198,'One-Handed Maces'),(5875,18414,0,128,168,'Frost Armor'),(5875,18414,0,128,133,'Fireball'),(5875,18414,0,67,107,'Block'),(5875,18414,0,0,81,'Dodge'),(5875,12340,0,1,78,'Heroic Strike'),(5875,18414,0,4,75,'Auto Shot'),(12340,12340,0,64,75461,'Flame Shock periodic crit'),(12340,12340,0,256,75445,'Immolate periodic crit'),(12340,18414,0,32,56816,'Rune Strike Dummy'),(18414,18414,33554432,0,143369,'Languages - Pandaren Horde'),(18414,18414,16777216,0,143368,'Languages - Pandaren Alliance'),(18414,18414,0,0,134732,'Battle Fatigue'),(18414,18414,1024,512,132295,'Shadow Resistance'),(18414,18414,8388608,0,131701,'Languages - Pandaren Neutral'),(18414,18414,512,512,129597,'Arcane Torrent - Chi'),(18414,18414,0,0,113873,'Remove Talent'),(18414,18414,0,0,111621,'Tome of the Clear Mind'),(18414,18414,8388608,0,107079,'Quaking Palm'),(18414,18414,8388608,0,107076,'Bouncy'),(18414,18414,8388608,0,107074,'Inner Peace'),(18414,18414,8388608,0,107073,'Gourmand'),(18414,18414,8388608,0,107072,'Epicurean'),(15595,18414,0,16,101062,'Flash Heal'),(15595,18414,0,0,96220,'Opening'),(15595,18414,2097152,32,94293,'Enable Worgen Altered Form'),(15595,18414,4,0,92682,'Explorer'),(15595,18414,64,0,92680,'Shortblade Specialization'),(15595,15595,0,128,92315,'Pyroblast!'),(15595,18414,0,0,89964,'Clear Glyph'),(15595,18414,0,32,89832,'Death Strike Enabler'),(15595,18414,0,1,88163,'Attack'),(15595,15595,0,1,88161,'Strike'),(15595,18414,2097152,32,87840,'Running Wild'),(15595,15595,0,4,87816,'General Hunter Passives'),(15595,18414,0,256,87330,'Suppression'),(15595,18414,0,4,87324,'Focused Aim'),(15595,18414,0,256,86213,'Soul Swap Exhale'),(15595,18414,0,384,85801,'DPS Caster Crit Damage Bonus'),(15595,15595,0,1024,87324,'Celestial Focus'),(15595,15595,0,1024,84736,'Nature\'s Focus'),(15595,18414,0,16,84733,'Holy Focus'),(15595,18414,0,4,82928,'Aimed Shot!'),(15595,18414,0,32,82246,'Parry'),(15595,18414,0,1024,81170,'Ravage!'),(15595,18414,256,0,79749,'Languages (Goblin)'),(15595,18414,512,0,79748,'Languages (Thalassian)'),(15595,18414,16,0,79747,'Languages (Forsaken)'),(15595,18414,2,0,79743,'Languages (Orcish)'),(15595,18414,32,0,79746,'Languages (Taurahe)'),(15595,18414,128,0,79744,'Languages (Zandali)'),(15595,18414,2097152,0,79742,'Languages (Common)'),(15595,18414,1024,0,79741,'Languages (Draenei)'),(15595,18414,64,0,79740,'Languages (Gnomish)'),(15595,18414,4,0,79739,'Languages (Dwarvish)'),(15595,18414,1,0,79738,'Languages (Common)'),(15595,18414,8,0,76252,'Languages (Darnassian)'),(12340,18414,0,128,71761,'Deep Freeze Immunity State'),(15595,18414,512,1,69179,'Arcane Torrent - Rage'),(15595,18414,256,0,69041,'Rocket Barrage'),(15595,18414,256,0,69042,'Time Is Money'),(15595,18414,256,0,69044,'Best Deals Anywhere'),(15595,18414,256,0,69045,'Better Living Through Chemistry'),(15595,18414,256,32,69046,'Pack Hobgoblin'),(15595,18414,256,0,69070,'Rocket Jump'),(15595,18414,2097152,32,68975,'Viciousness'),(15595,18414,2097152,32,68976,'Aberration'),(15595,18414,2097152,32,68978,'Flayer'),(15595,18414,2097152,32,68992,'Darkflight'),(15595,18414,2097152,32,68996,'Two Forms'),(12340,18414,0,32,63644,'Activating Secondary Spec'),(12340,18414,0,32,63645,'Activating Primary Spec'),(12340,18414,0,0,68398,'Opening'),(12340,18414,2,1179,21563,'Command'),(12340,18414,2,64,65222,'Command'),(15595,18414,0,4,77442,'Focus'),(15595,18414,0,128,79684,'Offensive State (DND)'),(15595,18414,0,16,76301,'Weapon Skills'),(15595,18414,0,1024,76300,'Weapon Skills'),(15595,18414,0,256,76299,'Weapon Skills'),(15595,18414,0,128,76298,'Weapon Skills'),(15595,18414,0,8,76297,'Weapon Skills'),(15595,18414,0,64,76296,'Weapon Skills'),(15595,18414,0,2,76294,'Weapon Skills'),(15595,18414,0,32,76292,'Weapon Skills'),(15595,18414,0,1,76290,'Weapon Skills'),(15595,18414,0,4,76249,'Weapon Skills'),(15595,18414,0,4,76250,'Armor Skills'),(15595,18414,0,1,76268,'Armor Skills'),(15595,18414,0,2,76271,'Armor Skills'),(15595,18414,0,8,76273,'Armor Skills'),(15595,18414,0,1024,76275,'Armor Skills'),(15595,18414,0,128,76276,'Armor Skills'),(15595,18414,0,256,76277,'Armor Skills'),(15595,18414,0,16,76279,'Armor Skills'),(15595,18414,0,32,76282,'Armor Skills'),(15595,18414,0,64,76272,'Armor Skills'),(15595,18414,0,2,20208,'Paladin pushback resistance'),(15595,18414,0,256,93375,'Control Demon'),(15595,18414,0,4,93321,'Control Pet'),(18414,18414,0,1,78,'Heroic Strike');

DROP TABLE IF EXISTS `playercreateinfo_skills`;
CREATE TABLE `playercreateinfo_skills` (
  `min_build` smallint NOT NULL DEFAULT '5875',
  `max_build` smallint NOT NULL DEFAULT '18414',
  `raceMask` int unsigned NOT NULL DEFAULT '0',
  `classMask` int unsigned NOT NULL DEFAULT '0',
  `skillid` smallint unsigned NOT NULL DEFAULT '0',
  `level` smallint unsigned NOT NULL DEFAULT '0',
  `note` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`max_build`,`min_build`,`raceMask`,`classMask`,`skillid`) USING BTREE,
  UNIQUE KEY `unique_index` (`max_build`,`min_build`,`raceMask`,`classMask`,`skillid`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Player System';

INSERT INTO `playercreateinfo_skills` VALUES (5875,15595,0,1,26,0,'Warrior: Arms'),(5875,15595,0,1,256,0,'Warrior: Fury'),(5875,15595,0,1,257,0,'Warrior: Protection'),(5875,15595,0,2,594,0,'Paladin: Holy'),(5875,15595,0,2,267,0,'Paladin: Protection'),(5875,15595,0,2,184,0,'Paladin: Retribution'),(5875,15595,0,4,50,0,'Hunter: Beast Mastery'),(5875,15595,0,4,163,0,'Hunter: Marksmanship'),(5875,15595,0,4,51,0,'Hunter: Survival'),(5875,15595,0,8,38,0,'Rogue: Combat'),(5875,15595,0,8,253,0,'Rogue: Assassination'),(5875,15595,0,8,39,0,'Rogue: Subtlety'),(5875,15595,0,16,56,0,'Priest: Holy'),(5875,15595,0,16,613,0,'Priest: Discipline'),(5875,15595,0,16,78,0,'Priest: Shadow'),(5875,15595,0,64,375,0,'Shaman: Elemental'),(5875,15595,0,64,373,0,'Shaman: Enhancement'),(5875,15595,0,64,374,0,'Shaman: Restoration'),(5875,15595,0,128,237,0,'Mage: Arcane'),(5875,15595,0,128,8,0,'Mage: Fire'),(5875,15595,0,128,6,0,'Mage: Frost'),(5875,15595,0,256,355,0,'Warlock: Affliction'),(5875,15595,0,256,354,0,'Warlock: Demonology'),(5875,15595,0,256,593,0,'Warlock: Destruction'),(5875,15595,0,1024,574,0,'Druid: Balance'),(5875,15595,0,1024,134,0,'Druid: Feral Combat'),(5875,15595,0,1024,573,0,'Druid: Restoration'),(5875,18414,1,0,754,0,'Racial: Human'),(5875,18414,4,0,101,0,'Racial: Dwarf'),(5875,18414,8,0,126,0,'Racial: Night Elf'),(5875,18414,64,0,753,0,'Racial: Gnome'),(5875,18414,2,0,125,0,'Racial: Orc'),(5875,18414,16,0,220,0,'Racial: Undead'),(5875,18414,32,0,124,0,'Racial: Tauren'),(5875,18414,128,0,733,0,'Racial: Troll'),(8606,18414,512,0,137,0,'Language Thalassian'),(5875,18414,128,0,315,0,'Language Troll'),(8606,12340,690,0,109,0,'Language Orcish'),(8606,18414,1024,0,759,0,'Language Draenei'),(15595,18414,256,0,792,0,'Language Goblin'),(8606,18414,1024,0,760,0,'Racial: Draenei'),(8606,18414,512,0,756,0,'Racial: Blood Elf'),(12340,18414,0,0,777,0,'Mounts'),(12340,18414,0,0,778,0,'Companion Pets'),(5875,18414,32,0,115,0,'Language Taurahe'),(5875,5875,178,0,109,0,'Language Orcish'),(12340,15595,0,32,770,0,'Death Knight - Blood'),(12340,15595,0,32,771,0,'Death Knight - Frost'),(12340,15595,0,32,772,0,'Death Knight - Unholy'),(15595,15595,2098253,0,98,0,'Language Common'),(5875,18414,8,0,113,0,'Language Darnassian'),(15595,18414,2097152,0,791,0,'Language Gilnean'),(5875,18414,64,0,313,0,'Language Gnomish'),(15595,15595,946,0,109,0,'Language Orcish'),(5875,18414,16,0,673,0,'Language Gutterspeak'),(5875,5875,77,0,98,0,'Language Common'),(8606,12340,1101,0,98,0,'Language Common'),(5875,18414,4,0,111,0,'Language Dwarven'),(15595,18414,256,0,790,0,'Racial - Goblin'),(15595,18414,2097152,0,789,0,'Racial - Worgen'),(15595,18414,0,0,810,0,'All - Glyphs'),(15595,15595,0,256,802,0,'Warlock - General'),(15595,15595,0,64,801,0,'Shaman - General'),(15595,18414,0,2,800,0,'Paladin - General'),(15595,15595,0,128,799,0,'Mage - General'),(15595,18414,0,1024,798,0,'Druid - General'),(15595,15595,0,8,797,0,'Rogue - General'),(15595,18414,0,32,796,0,'Death Knight - General'),(15595,18414,0,4,795,0,'Hunter - General'),(15595,15595,0,1,803,0,'Warrior - General'),(15595,18414,0,16,804,0,'Priest - General'),(18414,18414,18875469,0,98,0,'Language Common'),(18414,18414,33555378,0,109,0,'Language Orcish'),(18414,18414,8388608,0,905,0,'Language Pandaren'),(18414,18414,33554432,0,907,0,'Language Pandaren Horde'),(18414,18414,16777216,0,906,0,'Language Pandaren Alliance'),(18414,18414,0,8,921,0,'Rogue - General'),(18414,18414,0,128,904,0,'Mage - General'),(18414,18414,0,1,840,0,'Warrior - General'),(18414,18414,0,256,849,0,'Warlock - General'),(18414,18414,0,64,924,0,'Shaman - General'),(18414,18414,0,512,829,0,'Monk - General'),(18414,18414,58720256,0,899,0,'Racial: Pandaren');

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('96', '20211108-00_playercreateinfo');
