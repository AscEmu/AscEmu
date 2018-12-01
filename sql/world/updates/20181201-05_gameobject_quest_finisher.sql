-- Set up quest_finisher for specific builds

-- BC & Cata
REPLACE INTO `gameobject_quest_finisher` (`id`, `quest`, `min_build`, `max_build`) VALUES (142487, 2952, 8606, 15595),
(142487, 2953, 8606, 15595),
(175084, 4603, 8606, 15595),
(175084, 4605, 8606, 15595),
(175085, 4604, 8606, 15595),
(175085, 4606, 8606, 15595),
(180025, 7937, 8606, 15595),
(180055, 7944, 8606, 15595),
(180056, 7945, 8606, 15595),
(180743, 8744, 8606, 15595),
(180746, 8767, 8606, 15595),
(180746, 8788, 8606, 15595),
(180747, 8768, 8606, 15595),
(180748, 8769, 8606, 15595),
(180793, 8803, 8606, 15595),
(180024, 7938, 8606, 8606),
(180570, 8322, 8606, 8606),
(180717, 8743, 8606, 8606),
(182485, 9941, 8606, 8606),
(182485, 9942, 8606, 8606),
(183435, 10169, 8606, 8606),
(184477, 9929, 8606, 8606),
(184477, 9930, 8606, 8606),
(184830, 10842, 8606, 8606),
(185913, 11074, 8606, 8606),
(187236, 11528, 8606, 8606),
(187882, 11691, 8606, 8606);

-- Add Cata quest starter
REPLACE INTO `gameobject_quest_finisher` (`id`, `quest`, `min_build`, `max_build`) VALUES (33, 26356, 15595, 15595),
(34, 26355, 15595, 15595),
(35, 26353, 15595, 15595),
(36, 26354, 15595, 15595),
(61, 26797, 15595, 15595),
(259, 25804, 15595, 15595),
(261, 25802, 15595, 15595),
(2059, 26854, 15595, 15595),
(2076, 26300, 15595, 15595),
(2076, 26301, 15595, 15595),
(2076, 26325, 15595, 15595),
(2076, 26330, 15595, 15595),
(2076, 26332, 15595, 15595),
(2083, 26609, 15595, 15595),
(2688, 26042, 15595, 15595),
(2701, 26039, 15595, 15595),
(2702, 26041, 15595, 15595),
(4141, 29021, 15595, 15595),
(4141, 29022, 15595, 15595),
(4141, 29023, 15595, 15595),
(112948, 25818, 15595, 15595),
(138492, 26049, 15595, 15595),
(138492, 26341, 15595, 15595),
(138492, 26909, 15595, 15595),
(138492, 26912, 15595, 15595),
(142151, 25803, 15595, 15595),
(142195, 25365, 15595, 15595),
(152097, 27022, 15595, 15595),
(164955, 24724, 15595, 15595),
(164955, 24725, 15595, 15595),
(164956, 24728, 15595, 15595),
(164956, 24729, 15595, 15595),
(164957, 24726, 15595, 15595),
(164957, 24727, 15595, 15595),
(176392, 27053, 15595, 15595),
(177544, 27391, 15595, 15595),
(179485, 27118, 15595, 15595),
(179517, 27111, 15595, 15595),
(180715, 8763, 15595, 15595),
(180715, 8799, 15595, 15595),
(182947, 29643, 15595, 15595),
(187236, 28878, 15595, 15595),
(187236, 29385, 15595, 15595),
(187559, 11580, 15595, 15595),
(187564, 11581, 15595, 15595),
(187914, 11732, 15595, 15595),
(187916, 11734, 15595, 15595),
(187917, 11735, 15595, 15595),
(187919, 11736, 15595, 15595),
(187920, 11737, 15595, 15595),
(187921, 11738, 15595, 15595),
(187922, 11739, 15595, 15595),
(187923, 11740, 15595, 15595),
(187924, 11741, 15595, 15595),
(187925, 11742, 15595, 15595),
(187926, 11743, 15595, 15595),
(187927, 11744, 15595, 15595),
(187928, 11745, 15595, 15595),
(187929, 11746, 15595, 15595),
(187930, 11747, 15595, 15595),
(187931, 11748, 15595, 15595),
(187932, 11749, 15595, 15595),
(187933, 11750, 15595, 15595),
(187934, 11751, 15595, 15595),
(187935, 11752, 15595, 15595),
(187936, 11753, 15595, 15595),
(187937, 11754, 15595, 15595),
(187938, 11755, 15595, 15595),
(187939, 11756, 15595, 15595),
(187940, 11757, 15595, 15595),
(187941, 11758, 15595, 15595),
(187942, 11759, 15595, 15595),
(187943, 11760, 15595, 15595),
(187944, 11761, 15595, 15595),
(187945, 11762, 15595, 15595),
(187946, 11763, 15595, 15595),
(187947, 11764, 15595, 15595),
(187948, 11765, 15595, 15595),
(187949, 11799, 15595, 15595),
(187950, 11800, 15595, 15595),
(187951, 11801, 15595, 15595),
(187952, 11802, 15595, 15595),
(187953, 11803, 15595, 15595),
(187954, 11766, 15595, 15595),
(187955, 11767, 15595, 15595),
(187956, 11768, 15595, 15595),
(187957, 11769, 15595, 15595),
(187958, 11770, 15595, 15595),
(187959, 11771, 15595, 15595),
(187960, 11772, 15595, 15595),
(187961, 11773, 15595, 15595),
(187962, 11774, 15595, 15595),
(187963, 11775, 15595, 15595),
(187964, 11776, 15595, 15595),
(187965, 11777, 15595, 15595),
(187966, 11778, 15595, 15595),
(187967, 11779, 15595, 15595),
(187968, 11780, 15595, 15595),
(187969, 11781, 15595, 15595),
(187970, 11782, 15595, 15595),
(187971, 11783, 15595, 15595),
(187972, 11784, 15595, 15595),
(187973, 11785, 15595, 15595),
(187974, 11786, 15595, 15595),
(187975, 11787, 15595, 15595),
(189303, 12286, 15595, 15595),
(190034, 12331, 15595, 15595),
(190035, 12345, 15595, 15595),
(190036, 12332, 15595, 15595),
(190037, 12333, 15595, 15595),
(190038, 12334, 15595, 15595),
(190039, 12335, 15595, 15595),
(190040, 12336, 15595, 15595),
(190041, 12337, 15595, 15595),
(190043, 12339, 15595, 15595),
(190044, 12343, 15595, 15595),
(190045, 12341, 15595, 15595),
(190046, 12342, 15595, 15595),
(190047, 12340, 15595, 15595),
(190048, 12344, 15595, 15595),
(190050, 12347, 15595, 15595),
(190051, 12348, 15595, 15595),
(190052, 12349, 15595, 15595),
(190053, 12350, 15595, 15595),
(190054, 12351, 15595, 15595),
(190055, 12352, 15595, 15595),
(190056, 12353, 15595, 15595),
(190057, 12354, 15595, 15595),
(190058, 12355, 15595, 15595),
(190059, 12356, 15595, 15595),
(190060, 12357, 15595, 15595),
(190061, 12358, 15595, 15595),
(190062, 12359, 15595, 15595),
(190063, 12360, 15595, 15595),
(190064, 12361, 15595, 15595),
(190065, 12362, 15595, 15595),
(190066, 12363, 15595, 15595),
(190067, 12364, 15595, 15595),
(190068, 12365, 15595, 15595),
(190069, 12366, 15595, 15595),
(190070, 12367, 15595, 15595),
(190071, 12368, 15595, 15595),
(190072, 12369, 15595, 15595),
(190073, 12370, 15595, 15595),
(190074, 12371, 15595, 15595),
(190075, 12373, 15595, 15595),
(190076, 12374, 15595, 15595),
(190077, 12375, 15595, 15595),
(190078, 12376, 15595, 15595),
(190079, 12377, 15595, 15595),
(190080, 12378, 15595, 15595),
(190081, 12379, 15595, 15595),
(190082, 12380, 15595, 15595),
(190083, 12381, 15595, 15595),
(190084, 12382, 15595, 15595),
(190085, 12383, 15595, 15595),
(190086, 12384, 15595, 15595),
(190087, 12385, 15595, 15595),
(190088, 12386, 15595, 15595),
(190089, 12387, 15595, 15595),
(190090, 12388, 15595, 15595),
(190091, 12389, 15595, 15595),
(190096, 12390, 15595, 15595),
(190097, 12391, 15595, 15595),
(190098, 12392, 15595, 15595),
(190099, 12393, 15595, 15595),
(190100, 12394, 15595, 15595),
(190101, 12395, 15595, 15595),
(190102, 12396, 15595, 15595),
(190103, 12397, 15595, 15595),
(190104, 12398, 15595, 15595),
(190105, 12399, 15595, 15595),
(190106, 12400, 15595, 15595),
(190107, 12401, 15595, 15595),
(190108, 12402, 15595, 15595),
(190109, 12403, 15595, 15595),
(190110, 12404, 15595, 15595),
(190111, 12404, 15595, 15595),
(190112, 12406, 15595, 15595),
(190113, 12407, 15595, 15595),
(190114, 12408, 15595, 15595),
(190115, 12409, 15595, 15595),
(190116, 12409, 15595, 15595),
(190936, 12718, 15595, 15595),
(191761, 13843, 15595, 15595),
(191878, 12940, 15595, 15595),
(191879, 12941, 15595, 15595),
(191880, 12946, 15595, 15595),
(191881, 12947, 15595, 15595),
(191882, 12944, 15595, 15595),
(191883, 12945, 15595, 15595),
(192018, 12950, 15595, 15595),
(194032, 13440, 15595, 15595),
(194033, 13441, 15595, 15595),
(194034, 13450, 15595, 15595),
(194035, 13442, 15595, 15595),
(194036, 13443, 15595, 15595),
(194037, 13451, 15595, 15595),
(194038, 13444, 15595, 15595),
(194039, 13453, 15595, 15595),
(194040, 13445, 15595, 15595),
(194042, 13454, 15595, 15595),
(194043, 13455, 15595, 15595),
(194044, 13446, 15595, 15595),
(194045, 13447, 15595, 15595),
(194046, 13457, 15595, 15595),
(194048, 13458, 15595, 15595),
(194049, 13449, 15595, 15595),
(194056, 13433, 15595, 15595),
(194057, 13434, 15595, 15595),
(194058, 13435, 15595, 15595),
(194059, 13436, 15595, 15595),
(194060, 13437, 15595, 15595),
(194061, 13438, 15595, 15595),
(194062, 13439, 15595, 15595),
(194063, 13448, 15595, 15595),
(194064, 13473, 15595, 15595),
(194065, 13452, 15595, 15595),
(194066, 13456, 15595, 15595),
(194067, 13459, 15595, 15595),
(194068, 13460, 15595, 15595),
(194069, 13461, 15595, 15595),
(194070, 13462, 15595, 15595),
(194071, 13463, 15595, 15595),
(194072, 13472, 15595, 15595),
(194073, 13464, 15595, 15595),
(194074, 13465, 15595, 15595),
(194075, 13466, 15595, 15595),
(194076, 13467, 15595, 15595),
(194077, 13468, 15595, 15595),
(194078, 13469, 15595, 15595),
(194079, 13470, 15595, 15595),
(194080, 13471, 15595, 15595),
(194081, 13474, 15595, 15595),
(194084, 13501, 15595, 15595),
(194105, 13521, 15595, 15595),
(194119, 13548, 15595, 15595),
(194122, 13528, 15595, 15595),
(195431, 14190, 15595, 15595),
(195433, 14189, 15595, 15595),
(195433, 14191, 15595, 15595),
(195433, 14193, 15595, 15595),
(195433, 14360, 15595, 15595),
(195435, 14192, 15595, 15595),
(195445, 14195, 15595, 15595),
(195445, 14196, 15595, 15595),
(195497, 14213, 15595, 15595),
(195497, 14217, 15595, 15595),
(195497, 14358, 15595, 15595),
(195497, 14359, 15595, 15595),
(195517, 14219, 15595, 15595),
(195517, 14357, 15595, 15595),
(195600, 14247, 15595, 15595),
(195642, 14267, 15595, 15595),
(195642, 14270, 15595, 15595),
(195676, 14308, 15595, 15595),
(196394, 14320, 15595, 15595),
(196832, 14428, 15595, 15595),
(196833, 14429, 15595, 15595),
(202135, 24702, 15595, 15595),
(202264, 24734, 15595, 15595),
(202335, 24941, 15595, 15595),
(202474, 25069, 15595, 15595),
(202474, 25070, 15595, 15595),
(202474, 25566, 15595, 15595),
(202598, 25183, 15595, 15595),
(202613, 25204, 15595, 15595),
(202697, 25300, 15595, 15595),
(202701, 25296, 15595, 15595),
(202701, 25308, 15595, 15595),
(202701, 25314, 15595, 15595),
(202706, 25297, 15595, 15595),
(202712, 25301, 15595, 15595),
(202712, 25303, 15595, 15595),
(202714, 27393, 15595, 15595),
(202759, 25377, 15595, 15595),
(202859, 25467, 15595, 15595),
(202916, 25503, 15595, 15595),
(202975, 25524, 15595, 15595),
(202975, 25526, 15595, 15595),
(203134, 25643, 15595, 15595),
(203140, 27394, 15595, 15595),
(203301, 25883, 15595, 15595),
(203305, 25862, 15595, 15595),
(203395, 27668, 15595, 15595),
(204050, 26335, 15595, 15595),
(204351, 26519, 15595, 15595),
(204450, 26650, 15595, 15595),
(204450, 26662, 15595, 15595),
(204450, 26663, 15595, 15595),
(204450, 26664, 15595, 15595),
(204578, 26678, 15595, 15595),
(204817, 26725, 15595, 15595),
(204824, 26753, 15595, 15595),
(204825, 26722, 15595, 15595),
(204959, 26869, 15595, 15595),
(205134, 26260, 15595, 15595),
(205143, 27039, 15595, 15595),
(205198, 27092, 15595, 15595),
(205207, 27101, 15595, 15595),
(205258, 27170, 15595, 15595),
(205350, 27231, 15595, 15595),
(205874, 27602, 15595, 15595),
(205874, 27623, 15595, 15595),
(205875, 27460, 15595, 15595),
(206293, 27760, 15595, 15595),
(206293, 27761, 15595, 15595),
(206293, 27777, 15595, 15595),
(206335, 27796, 15595, 15595),
(206335, 27883, 15595, 15595),
(206336, 27797, 15595, 15595),
(206336, 27884, 15595, 15595),
(206374, 27912, 15595, 15595),
(206374, 27913, 15595, 15595),
(206504, 27930, 15595, 15595),
(206569, 27947, 15595, 15595),
(206569, 27951, 15595, 15595),
(206569, 28241, 15595, 15595),
(206569, 28242, 15595, 15595),
(206585, 27989, 15595, 15595),
(206585, 27994, 15595, 15595),
(206585, 28100, 15595, 15595),
(207104, 28335, 15595, 15595),
(207104, 28385, 15595, 15595),
(207125, 28322, 15595, 15595),
(207125, 28456, 15595, 15595),
(207179, 28464, 15595, 15595),
(207291, 28630, 15595, 15595),
(207406, 28798, 15595, 15595),
(207407, 28799, 15595, 15595),
(207408, 28800, 15595, 15595),
(207409, 28801, 15595, 15595),
(207410, 28802, 15595, 15595),
(207411, 28803, 15595, 15595),
(207412, 28804, 15595, 15595),
(207982, 28910, 15595, 15595),
(207983, 28911, 15595, 15595),
(207984, 28912, 15595, 15595),
(207985, 28913, 15595, 15595),
(207986, 28914, 15595, 15595),
(207987, 28915, 15595, 15595),
(207988, 28916, 15595, 15595),
(207989, 28917, 15595, 15595),
(207990, 28918, 15595, 15595),
(207991, 28919, 15595, 15595),
(207992, 28920, 15595, 15595),
(207993, 28921, 15595, 15595),
(208089, 28943, 15595, 15595),
(208090, 28944, 15595, 15595),
(208093, 28947, 15595, 15595),
(208094, 28948, 15595, 15595),
(208115, 28951, 15595, 15595),
(208116, 28952, 15595, 15595),
(208117, 28953, 15595, 15595),
(208118, 28958, 15595, 15595),
(208119, 28989, 15595, 15595),
(208121, 28956, 15595, 15595),
(208122, 28960, 15595, 15595),
(208123, 28961, 15595, 15595),
(208124, 28963, 15595, 15595),
(208125, 28964, 15595, 15595),
(208126, 28968, 15595, 15595),
(208127, 28970, 15595, 15595),
(208128, 28977, 15595, 15595),
(208129, 28980, 15595, 15595),
(208130, 28978, 15595, 15595),
(208131, 28979, 15595, 15595),
(208132, 28985, 15595, 15595),
(208133, 28983, 15595, 15595),
(208134, 28988, 15595, 15595),
(208135, 28991, 15595, 15595),
(208136, 28990, 15595, 15595),
(208137, 28955, 15595, 15595),
(208138, 28965, 15595, 15595),
(208139, 28967, 15595, 15595),
(208140, 28992, 15595, 15595),
(208141, 28981, 15595, 15595),
(208142, 28982, 15595, 15595),
(208143, 28957, 15595, 15595),
(208144, 28959, 15595, 15595),
(208145, 28962, 15595, 15595),
(208146, 28966, 15595, 15595),
(208147, 28969, 15595, 15595),
(208148, 28971, 15595, 15595),
(208149, 28972, 15595, 15595),
(208150, 28973, 15595, 15595),
(208151, 28974, 15595, 15595),
(208152, 28975, 15595, 15595),
(208153, 28976, 15595, 15595),
(208154, 28984, 15595, 15595),
(208155, 28986, 15595, 15595),
(208156, 28987, 15595, 15595),
(208157, 28993, 15595, 15595),
(208158, 28994, 15595, 15595),
(208159, 28995, 15595, 15595),
(208160, 28996, 15595, 15595),
(208161, 28998, 15595, 15595),
(208162, 28999, 15595, 15595),
(208163, 29000, 15595, 15595),
(208164, 29001, 15595, 15595),
(208165, 29002, 15595, 15595),
(208166, 29003, 15595, 15595),
(208167, 29004, 15595, 15595),
(208168, 29005, 15595, 15595),
(208169, 29006, 15595, 15595),
(208170, 29007, 15595, 15595),
(208171, 29008, 15595, 15595),
(208172, 29009, 15595, 15595),
(208173, 29010, 15595, 15595),
(208174, 29011, 15595, 15595),
(208175, 29012, 15595, 15595),
(208176, 29013, 15595, 15595),
(208177, 29014, 15595, 15595),
(208178, 29016, 15595, 15595),
(208179, 29017, 15595, 15595),
(208180, 29018, 15595, 15595),
(208181, 29019, 15595, 15595),
(208183, 29020, 15595, 15595),
(208184, 29030, 15595, 15595),
(208187, 29031, 15595, 15595),
(208188, 29036, 15595, 15595),
(208316, 29071, 15595, 15595),
(208317, 29073, 15595, 15595),
(208420, 29155, 15595, 15595),
(208420, 29253, 15595, 15595),
(208549, 29261, 15595, 15595),
(208550, 29262, 15595, 15595),
(208825, 29314, 15595, 15595),
(209072, 29403, 15595, 15595),
(209076, 29413, 15595, 15595),
(209094, 29427, 15595, 15595),
(209095, 29429, 15595, 15595);

-- Update all other quest starter
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 131474 AND `quest` = 2278;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 142487 AND `quest` = 2945;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 142487 AND `quest` = 2951;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 152097 AND `quest` = 3525;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 164869 AND `quest` = 4083;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 164886 AND `quest` = 3363;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 164888 AND `quest` = 4443;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 175084 AND `quest` = 4601;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 175085 AND `quest` = 4602;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 179880 AND `quest` = 7761;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 180503 AND `quest` = 8307;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 181150 AND `quest` = 9161;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 181698 AND `quest` = 9529;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 181748 AND `quest` = 9565;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 181756 AND `quest` = 9550;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 181758 AND `quest` = 9561;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 183770 AND `quest` = 10243;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 184300 AND `quest` = 10316;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 184825 AND `quest` = 10555;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 185126 AND `quest` = 10793;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 185165 AND `quest` = 10819;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 185165 AND `quest` = 10820;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 24776 AND `quest` = 264;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 2908 AND `quest` = 749;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 32569 AND `quest` = 1393;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 35251 AND `quest` = 1454;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 55 AND `quest` = 37;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 56 AND `quest` = 45;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 7510 AND `quest` = 2399;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 7923 AND `quest` = 941;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 10076 AND `quest` = 944;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 112877 AND `quest` = 2201;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 112888 AND `quest` = 461;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 112948 AND `quest` = 290;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 113791 AND `quest` = 63;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 12666 AND `quest` = 949;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 138492 AND `quest` = 635;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 138492 AND `quest` = 656;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 141980 AND `quest` = 2701;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 142127 AND `quest` = 2742;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 142151 AND `quest` = 284;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 142179 AND `quest` = 2866;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 142194 AND `quest` = 2882;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 142195 AND `quest` = 2902;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 142343 AND `quest` = 2946;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 142343 AND `quest` = 2954;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 142343 AND `quest` = 2966;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 144063 AND `quest` = 2879;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 148498 AND `quest` = 3372;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 148504 AND `quest` = 3913;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 148512 AND `quest` = 3373;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 148836 AND `quest` = 3446;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 148838 AND `quest` = 3447;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 149047 AND `quest` = 3454;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 149502 AND `quest` = 3481;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 151286 AND `quest` = 3505;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 1557 AND `quest` = 410;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 1561 AND `quest` = 72;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 1585 AND `quest` = 280;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 1586 AND `quest` = 431;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 1593 AND `quest` = 438;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 1599 AND `quest` = 460;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 1609 AND `quest` = 465;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 161504 AND `quest` = 3844;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 1627 AND `quest` = 477;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164689 AND `quest` = 3802;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164885 AND `quest` = 4119;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164885 AND `quest` = 4447;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164886 AND `quest` = 2523;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164887 AND `quest` = 996;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164887 AND `quest` = 998;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164888 AND `quest` = 4117;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164909 AND `quest` = 4125;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164953 AND `quest` = 4131;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164954 AND `quest` = 4135;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164955 AND `quest` = 4381;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164955 AND `quest` = 4385;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164956 AND `quest` = 4383;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164956 AND `quest` = 4384;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164957 AND `quest` = 4382;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 164957 AND `quest` = 4386;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 17182 AND `quest` = 983;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 17183 AND `quest` = 1001;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 17184 AND `quest` = 1002;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 17185 AND `quest` = 1003;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 1728 AND `quest` = 524;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 173265 AND `quest` = 4449;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 173265 AND `quest` = 4451;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 174848 AND `quest` = 4561;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 174848 AND `quest` = 4661;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 175524 AND `quest` = 4812;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 175586 AND `quest` = 4863;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 175587 AND `quest` = 4861;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 175704 AND `quest` = 3367;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 175894 AND `quest` = 5021;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 175926 AND `quest` = 5058;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176091 AND `quest` = 5084;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176192 AND `quest` = 5164;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176317 AND `quest` = 5265;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176361 AND `quest` = 5216;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176361 AND `quest` = 5218;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176361 AND `quest` = 5219;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176361 AND `quest` = 5221;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176361 AND `quest` = 5222;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176361 AND `quest` = 5224;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176361 AND `quest` = 5225;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176361 AND `quest` = 5227;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176361 AND `quest` = 5229;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176361 AND `quest` = 5231;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176361 AND `quest` = 5233;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176361 AND `quest` = 5235;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 176631 AND `quest` = 5463;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 1767 AND `quest` = 553;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 177491 AND `quest` = 5902;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 177491 AND `quest` = 5904;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 177544 AND `quest` = 5942;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 177675 AND `quest` = 6024;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 177786 AND `quest` = 6161;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 179485 AND `quest` = 1193;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 179517 AND `quest` = 7877;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 179551 AND `quest` = 7486;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 180633 AND `quest` = 8305;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 180642 AND `quest` = 8577;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 180652 AND `quest` = 8597;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 181073 AND `quest` = 9029;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 181643 AND `quest` = 9476;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 181649 AND `quest` = 9469;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 182947 AND `quest` = 10094;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 183877 AND `quest` = 10216;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 19024 AND `quest` = 1028;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 19599 AND `quest` = 1089;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2059 AND `quest` = 419;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2076 AND `quest` = 584;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2076 AND `quest` = 585;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2076 AND `quest` = 586;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 20805 AND `quest` = 1190;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2083 AND `quest` = 595;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2289 AND `quest` = 619;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2553 AND `quest` = 624;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2555 AND `quest` = 625;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2556 AND `quest` = 626;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 257 AND `quest` = 250;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 259 AND `quest` = 285;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 261 AND `quest` = 281;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2652 AND `quest` = 631;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2688 AND `quest` = 652;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 269 AND `quest` = 403;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2701 AND `quest` = 642;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2702 AND `quest` = 651;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 270 AND `quest` = 310;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2703 AND `quest` = 645;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2734 AND `quest` = 321;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 287 AND `quest` = 200;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2875 AND `quest` = 738;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 288 AND `quest` = 328;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 2933 AND `quest` = 779;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 31 AND `quest` = 94;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 33 AND `quest` = 140;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 34 AND `quest` = 139;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 35 AND `quest` = 136;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 36 AND `quest` = 138;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 3643 AND `quest` = 67;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 4141 AND `quest` = 894;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 4141 AND `quest` = 900;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 4141 AND `quest` = 901;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 50961 AND `quest` = 1437;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 51708 AND `quest` = 254;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 5620 AND `quest` = 926;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 59 AND `quest` = 95;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 61 AND `quest` = 231;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 61934 AND `quest` = 1526;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 76 AND `quest` = 248;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 12340 WHERE `id` = 89931 AND `quest` = 1714;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 186585 AND `quest` = 11253;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 187565 AND `quest` = 11605;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 187565 AND `quest` = 11607;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 187565 AND `quest` = 11609;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 187565 AND `quest` = 11610;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 188085 AND `quest` = 11901;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 188365 AND `quest` = 12042;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 188419 AND `quest` = 12030;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 188419 AND `quest` = 12031;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 190535 AND `quest` = 12565;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 190535 AND `quest` = 12567;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 190602 AND `quest` = 12615;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 190602 AND `quest` = 12618;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 190657 AND `quest` = 12655;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 190657 AND `quest` = 12656;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 190768 AND `quest` = 12691;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 190777 AND `quest` = 12581;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 190917 AND `quest` = 12711;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 191760 AND `quest` = 13415;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 191760 AND `quest` = 13416;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 191761 AND `quest` = 12889;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 191766 AND `quest` = 12902;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 192060 AND `quest` = 12922;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 192071 AND `quest` = 12981;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 192078 AND `quest` = 12977;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 192079 AND `quest` = 13003;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 192080 AND `quest` = 13006;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 192524 AND `quest` = 13046;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 193195 AND `quest` = 13263;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 193195 AND `quest` = 13389;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 193400 AND `quest` = 13262;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 193400 AND `quest` = 13388;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 194555 AND `quest` = 13604;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 194555 AND `quest` = 13614;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 194555 AND `quest` = 13622;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 194555 AND `quest` = 13629;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 194555 AND `quest` = 13817;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 194555 AND `quest` = 13818;
UPDATE `gameobject_quest_finisher` SET `min_build` = 12340, `max_build` = 15595 WHERE `id` = 201742 AND `quest` = 24545;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 142487 AND `quest` = 2952;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 142487 AND `quest` = 2953;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 175084 AND `quest` = 4603;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 175084 AND `quest` = 4605;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 175085 AND `quest` = 4604;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 175085 AND `quest` = 4606;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 180025 AND `quest` = 7937;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 180055 AND `quest` = 7944;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 180056 AND `quest` = 7945;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 180743 AND `quest` = 8744;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 180746 AND `quest` = 8767;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 180746 AND `quest` = 8788;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 180747 AND `quest` = 8768;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 180748 AND `quest` = 8769;
UPDATE `gameobject_quest_finisher` SET `min_build` = 8606, `max_build` = 15595 WHERE `id` = 180793 AND `quest` = 8803;


INSERT INTO `world_db_version` VALUES ('44', '20181201-05_gameobject_quest_finisher');
