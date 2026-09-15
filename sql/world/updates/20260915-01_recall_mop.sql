-- .recall port: extend the teleport locations to Mop (issue 1357)
-- Every location was checked against the map and vmap data of 5.4.8: terrain height, the model spawns within
-- 150 yards and their model files are identical to the data of the build the row was made for.

-- Cataclysm locations that are unchanged in Mop, excluded rows: 867 AzsharaCrater (map 37 has no Mop data),
-- 894 Bfd and 1728 ST (instance model changed), 1617 ScottTest (test map without data), 4312 IsleOfDread (floating)
UPDATE `recall` SET `max_build` = 18414 WHERE `max_build` = 15595 AND `id` NOT IN (867, 894, 1617, 1728, 4312);

-- WotLK locations without a Cataclysm row whose surroundings are unchanged in Mop
UPDATE `recall` SET `max_build` = 18414 WHERE `max_build` = 12340 AND `id` IN (
    10, 13, 14, 15, 16, 24, 37, 40, 44, 47, 49, 53, 54, 55, 56, 57, 63, 73, 89, 97, 99, 102, 106, 112, 113,
    115, 117, 118, 122, 126, 132, 137, 139, 140, 143, 144, 145, 150, 152, 153, 158, 160, 163, 168, 169, 175, 177, 180, 182, 188,
    190, 191, 200, 209, 210, 211, 215, 218, 221, 223, 227, 230, 232, 234, 236, 242, 243, 244, 245, 251, 252, 253, 254, 255, 256,
    257, 260, 266, 280, 281, 283, 287, 293, 301, 303, 307, 311, 317, 319, 320, 322, 323, 324, 326, 339, 341, 346, 347, 355, 358,
    363, 366, 370, 373, 383, 385, 392, 408, 411, 412, 413, 417, 423, 426, 434, 435, 440, 441, 445, 446, 447, 450, 452, 454, 455,
    458, 459, 463, 466, 475, 477, 484, 485, 486, 487, 489, 494, 499, 502, 503, 505, 511, 513, 523, 528, 534, 537, 539, 544, 546,
    548, 549, 557, 563, 564, 575, 576, 579, 585, 601, 615, 621, 634, 638, 646, 651, 660, 662, 664, 665, 666, 667, 669, 671, 673,
    680, 681, 683, 684, 686, 701, 704, 705, 706, 707, 708, 709, 730, 734, 737, 741, 745, 747, 749, 750, 754, 759, 765, 767, 768,
    774, 776, 778, 779, 780, 781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 795, 796, 797, 798, 799, 800
);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('195', '20260915-01_recall_mop');
