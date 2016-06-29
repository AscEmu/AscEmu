/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "ObjectMgr.h"
#include "Master.h"

void ParseBanArgs(char* args, char** BanDuration, char** BanReason)
{
    char* pBanDuration = strchr(args, ' ');
    char* pReason = NULL;
    if (pBanDuration != NULL)
    {
        if (isdigit(*(pBanDuration + 1)))       // this is the duration of the ban
        {
            *pBanDuration = 0;                  // NULL-terminate the first string (character/account/ip)
            ++pBanDuration;                     // point to next arg
            pReason = strchr(pBanDuration + 1, ' ');
            if (pReason != NULL)                // BanReason is OPTIONAL
            {
                *pReason = 0;                   // BanReason was given, so NULL-terminate the duration string
                ++pReason;                      // and point to the ban reason
            }
        }
        else                                    // no duration was given (didn't start with a digit) - so this arg must be ban reason and duration defaults to permanent
        {
            pReason = pBanDuration;
            pBanDuration = NULL;
            *pReason = 0;
            ++pReason;
        }
    }
    *BanDuration = pBanDuration;
    *BanReason = pReason;
}

int32 GetSpellIDFromLink(const char* spelllink)
{
    if (spelllink == NULL)
        return 0;

    const char* ptr = strstr(spelllink, "|Hspell:");
    if (ptr == NULL)
    {
        return 0;
    }

    return atol(ptr + 8);       // spell id is just past "|Hspell:" (8 bytes)
}

bool ChatHandler::HandleWorldPortCommand(const char* args, WorldSession* m_session)
{
    float x, y, z, o = 0;
    uint32 mapid;
    if (sscanf(args, "%u %f %f %f %f", (unsigned int*)&mapid, &x, &y, &z, &o) < 4)
        return false;

    if (x >= _maxX || x <= _minX || y <= _minY || y >= _maxY)
        return false;

    LocationVector vec(x, y, z, o);
    m_session->GetPlayer()->SafeTeleport(mapid, 0, vec);
    return true;
}

bool ChatHandler::HandleLearnCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
    {
        return false;
    }

    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr)
    {
        plr = m_session->GetPlayer();
        SystemMessage(m_session, "Auto-targeting self.");
    }

    if (!plr)
    {
        return false;
    }

    if (stricmp(args, "all") == 0)
    {
        sGMLog.writefromsession(m_session, "taught %s all spells.", plr->GetName());
        SystemMessage(m_session, "Taught %s all spells.", plr->GetName());
        static uint32 spellarray[DRUID + 1][512] =
        {
            { 0 }, // N/A
            { 6673, 2457, 78, 100, 772, 6343, 1715, 284, 2687, 71, 6546, 7386, 355, 5242, 7384, 72, 1160, 6572, 285, 694, 2565, 676, 8198, 845, 6547, 20230, 12678, 6192, 5246, 7405, 6190, 5308, 1608, 6574, 1161, 6178, 871, 8204, 2458, 7369, 20252, 6548, 1464, 11549, 18499, 20658, 11564, 11554, 7379, 8380, 1680, 6552, 8820, 8205, 11608, 20660, 11565, 11572, 23922, 11550, 11555, 11600, 11578, 11604, 11596, 20661, 11566, 23923, 11580, 11609, 1719, 11573, 11551, 11556, 11601, 23924, 11605, 20662, 11567, 11597, 11581, 25289, 20569, 25286, 11574, 25288, 23925, 25241, 25202, 34428, 25269, 23920, 25234, 29707, 25258, 25225, 25264, 25231, 469, 25208, 2048, 25242, 25203, 25236, 30324, 3411, 30357, 30356, 46845, 47519, 47449, 47470, 47501, 47439, 47474, 55694, 47487, 47450, 47465, 47520, 47467, 47436, 47502, 47437, 47475, 47440, 47471, 57755, 57823, 47488, 674, 750, 64382, 0 }, // Warrior
            { 465, 635, 21084, 19740, 20271, 498, 639, 853, 1152, 10290, 1022, 633, 19834, 53408, 7328, 19742, 647, 31789, 62124, 7294, 25780, 1044, 31785, 26573, 643, 879, 19750, 5502, 19835, 19746, 1026, 20164, 19850, 5588, 5599, 10322, 10326, 19939, 1038, 10298, 5614, 53407, 19876, 20116, 10291, 19752, 1042, 2800, 20165, 19836, 19888, 19852, 642, 19940, 5615, 19891, 10324, 10299, 10278, 3472, 20166, 20922, 1032, 5589, 19895, 19837, 4987, 19941, 19853, 10312, 19897, 24275, 6940, 10328, 10300, 19899, 20772, 20923, 10292, 19942, 2812, 10310, 19838, 10313, 25782, 24274, 19896, 19854, 25894, 10308, 10329, 19898, 10301, 19943, 25291, 25290, 20924, 10293, 10314, 19900, 25898, 25916, 25899, 25918, 24239, 25292, 10318, 53651, 20773, 32223, 27135, 27151, 27142, 27143, 27137, 27150, 33776, 27138, 27152, 27180, 27139, 27154, 31884, 27140, 27173, 27149, 27153, 27141, 27136, 48935, 54428, 48937, 48816, 48949, 48931, 48800, 48933, 48941, 48784, 48805, 48818, 48781, 53600, 54043, 48943, 48936, 48945, 48938, 48947, 48817, 48788, 48932, 48942, 48801, 48785, 48934, 48950, 48819, 48806, 48782, 53601, 61411, 750, 20217, 53736, 33388, 0 }, // Paladin
            { 75, 2973, 1494, 13163, 1978, 3044, 1130, 5116, 14260, 13165, 883, 2641, 6991, 982, 13549, 1515, 19883, 14281, 20736, 136, 2974, 6197, 1002, 1513, 13795, 1495, 14261, 14318, 2643, 13550, 19884, 14282, 5118, 34074, 781, 1499, 3111, 14323, 3043, 1462, 14262, 19885, 14302, 3045, 13551, 19880, 14283, 14319, 13809, 3661, 13161, 5384, 14269, 14288, 14326, 1543, 14263, 19878, 13813, 13552, 14284, 14303, 3662, 3034, 14320, 13159, 14310, 14324, 14264, 19882, 1510, 14289, 13553, 14285, 14316, 13542, 14270, 20043, 14304, 14327, 14321, 14265, 13554, 56641, 19879, 14294, 14286, 13543, 14317, 14290, 20190, 14305, 14266, 14322, 14325, 14271, 13555, 14295, 14287, 25296, 19263, 14311, 13544, 25294, 25295, 19801, 27025, 34120, 27014, 27023, 34026, 27021, 27016, 27022, 27044, 27045, 27046, 34600, 27019, 34477, 36916, 49066, 53351, 48995, 49051, 49055, 49044, 49000, 61846, 48989, 49047, 58431, 61005, 53271, 49071, 53338, 49067, 48996, 49052, 49056, 49045, 49001, 61847, 60192, 61006, 48990, 53339, 49048, 58434, 62757, 674, 8737, 0 }, // Hunter
            { 2098, 1752, 1784, 53, 921, 1776, 1757, 5277, 6760, 6770, 5171, 2983, 2589, 1766, 8647, 703, 1758, 6761, 1966, 1804, 8676, 2590, 51722, 1943, 1725, 8631, 1759, 1856, 2836, 6762, 8724, 1833, 8649, 2591, 6768, 8639, 2070, 1842, 8632, 408, 1760, 8623, 8725, 2094, 8696, 8721, 8650, 8640, 8633, 8621, 8624, 8637, 1860, 11267, 6774, 1857, 11279, 11273, 11197, 11289, 17347, 11293, 11299, 11297, 11268, 26669, 8643, 11280, 11303, 11274, 11290, 11294, 11300, 11198, 11269, 17348, 11305, 11281, 25300, 31016, 25302, 11275, 26839, 32645, 26861, 26889, 26679, 26865, 27448, 27441, 31224, 26866, 26863, 26867, 32684, 48689, 48673, 26884, 26864, 5938, 26862, 51724, 48658, 48667, 48656, 57992, 48671, 48690, 48675, 57934, 48674, 48637, 48669, 48659, 48668, 48672, 48691, 48657, 57993, 51723, 48676, 48660, 48638, 0 }, // Rogue
            { 2050, 1243, 585, 2052, 589, 17, 591, 586, 139, 2053, 8092, 2006, 594, 588, 1244, 592, 528, 8122, 6074, 598, 2054, 8102, 527, 600, 970, 2944, 6346, 2061, 14914, 15237, 7128, 453, 6075, 9484, 2055, 8103, 2096, 2010, 984, 15262, 8129, 1245, 3747, 9472, 6076, 992, 19276, 6063, 15430, 8104, 8124, 15263, 602, 605, 6065, 596, 976, 1004, 552, 9473, 6077, 6064, 1706, 8105, 10880, 2767, 19277, 988, 15264, 15431, 2791, 6066, 9474, 6078, 6060, 2060, 1006, 8106, 996, 9485, 15265, 10898, 10888, 10957, 10892, 19278, 10915, 27799, 10909, 10927, 10963, 10945, 10881, 10933, 15266, 10937, 10899, 21562, 10916, 10951, 10960, 10928, 10893, 19279, 10964, 27800, 10946, 15267, 10900, 10934, 10917, 27683, 10890, 10929, 10958, 10965, 10947, 20770, 10894, 19280, 25314, 15261, 27801, 60931, 10952, 10938, 10901, 21564, 10961, 25316, 27681, 25315, 10955, 25233, 25363, 32379, 25210, 25372, 32546, 25217, 25221, 25367, 25384, 34433, 25235, 25467, 25213, 25331, 25308, 33076, 25435, 25433, 25431, 25375, 25364, 32375, 25389, 25218, 25392, 39374, 32999, 25222, 32996, 25368, 48040, 48119, 48134, 48299, 48070, 48062, 48126, 48112, 48122, 48075, 48077, 48045, 48065, 48067, 48157, 48124, 48072, 48169, 48168, 48170, 48120, 48063, 48135, 48171, 48300, 48071, 48127, 48113, 48123, 47951, 48078, 53023, 48161, 48066, 48162, 48074, 48068, 48158, 48125, 14752, 14818, 14819, 27841, 25312, 48073, 64843, 64901, 0 }, // Priest
            { 53341, 53331, 53343, 54447, 53342, 54446, 53323, 53344, 62158, 48778, 48266, 45902, 52375, 50977, 49576, 49142, 45477, 45462, 53428, 49998, 50842, 46584, 48263, 47528, 48721, 45524, 49926, 47476, 51325, 43265, 49917, 49896, 49020, 3714, 49892, 48792, 51426, 49999, 49927, 45529, 56222, 57330, 49918, 49913, 49939, 48743, 49936, 49903, 51423, 56815, 55265, 48707, 49893, 51427, 49928, 49914, 51326, 45463, 49919, 48265, 49940, 61999, 49937, 49904, 51424, 55270, 49929, 51428, 49915, 51327, 49923, 47568, 57623, 49920, 49894, 49941, 49909, 51425, 51429, 55271, 49916, 42650, 49930, 51328, 49938, 49895, 49924, 49921, 70164, 0 }, // Death Knight
            { 30669, 30670, 30671, 331, 403, 8017, 8042, 8071, 2484, 332, 8044, 529, 324, 8018, 5730, 8050, 8024, 3599, 8075, 2008, 1535, 547, 370, 8045, 548, 8154, 526, 325, 8019, 57994, 8052, 8027, 913, 6390, 8143, 8056, 8033, 2645, 5394, 8004, 915, 6363, 52127, 2870, 8498, 8166, 131, 20609, 8046, 8181, 939, 905, 10399, 8155, 8160, 6196, 8030, 943, 8190, 5675, 8184, 8053, 8227, 8038, 8008, 6391, 52129, 546, 556, 51730, 8177, 6375, 10595, 20608, 6364, 36936, 8232, 421, 8499, 959, 6041, 945, 8012, 8512, 8058, 6495, 10406, 52131, 20610, 10412, 16339, 8010, 10585, 10495, 8170, 8249, 10478, 10456, 10391, 6392, 8161, 1064, 930, 51988, 10447, 6377, 8005, 8134, 6365, 8235, 52134, 11314, 10537, 10466, 10392, 10600, 10407, 10622, 16341, 10472, 10586, 10496, 20776, 2860, 10413, 10526, 16355, 10395, 10431, 10427, 52136, 51991, 10462, 15207, 10437, 10486, 11315, 10448, 10467, 10442, 10623, 10479, 10408, 52138, 10605, 16342, 10396, 15208, 10432, 10587, 10497, 10538, 16387, 10473, 16356, 10428, 20777, 10414, 51992, 29228, 10463, 25357, 10468, 10601, 10438, 25361, 16362, 25422, 25546, 25448, 24398, 25439, 25391, 25469, 25508, 25489, 3738, 25552, 25570, 25528, 2062, 25500, 25420, 25557, 25560, 25449, 25525, 25423, 2894, 25563, 25464, 25505, 25590, 25454, 25567, 25574, 25533, 33736, 25442, 51993, 25547, 25457, 25396, 25472, 25509, 58649, 58785, 58794, 58755, 58771, 58699, 58580, 58801, 49275, 49235, 49237, 58731, 58751, 55458, 49270, 49230, 61649, 58737, 49232, 58652, 58741, 49272, 51505, 49280, 58746, 58703, 58581, 57622, 58789, 58795, 58756, 58773, 57960, 58803, 49276, 49236, 58734, 58582, 58753, 49231, 49238, 49277, 55459, 49271, 51994, 61657, 58739, 49233, 58656, 58790, 58745, 58796, 58757, 49273, 51514, 60043, 49281, 58774, 58749, 58704, 58643, 58804, 66842, 66843, 66844, 8737, 0 }, // Shaman
            { 1459, 133, 168, 5504, 116, 587, 2136, 143, 5143, 205, 118, 5505, 7300, 122, 597, 604, 145, 130, 1449, 1460, 2137, 837, 5144, 2120, 1008, 3140, 475, 1953, 10, 5506, 12051, 543, 54648, 7301, 7322, 1463, 12824, 8437, 990, 2138, 6143, 2948, 5145, 2139, 8450, 8400, 2121, 120, 865, 8406, 1461, 6141, 759, 8494, 8444, 8455, 8438, 6127, 8412, 8457, 8401, 7302, 45438, 8416, 6129, 8422, 8461, 8407, 8492, 6117, 8445, 8427, 8451, 8402, 8495, 8439, 3552, 8413, 8408, 8417, 10138, 8458, 8423, 6131, 7320, 12825, 8446, 10169, 10156, 10159, 10144, 10148, 8462, 10185, 10179, 10191, 10201, 10197, 13031, 22782, 10205, 10211, 10053, 10173, 10149, 10215, 10160, 10139, 10223, 10180, 10219, 10186, 10145, 10177, 13032, 10192, 10206, 10170, 10202, 10199, 10150, 10230, 23028, 10157, 10212, 33041, 10216, 10181, 10161, 10054, 13033, 22783, 10207, 25345, 10187, 28612, 10140, 10174, 10225, 10151, 28609, 25304, 10220, 10193, 61305, 28272, 12826, 61025, 28271, 27078, 27080, 25306, 30482, 27130, 27075, 27071, 30451, 33042, 27086, 27134, 27087, 37420, 27073, 27070, 30455, 33944, 27088, 27085, 27101, 66, 27131, 33946, 38699, 27128, 27072, 27124, 27125, 27127, 27082, 27126, 38704, 33717, 27090, 33043, 27079, 38692, 32796, 38697, 33405, 43987, 27074, 30449, 42894, 43023, 43045, 53140, 42930, 42925, 42913, 43019, 42858, 42939, 42872, 42832, 53142, 42843, 42955, 42949, 42917, 42841, 44614, 43038, 42896, 42920, 43015, 43017, 42985, 43010, 42833, 42914, 42859, 42846, 42931, 42926, 43012, 42842, 43008, 43024, 43020, 43046, 42897, 43002, 42921, 42995, 42940, 42956, 61316, 61024, 42950, 42873, 47610, 43039, 55342, 58659, 53142, 0 }, // Mage
            { 59671, 687, 348, 686, 688, 172, 702, 1454, 695, 980, 5782, 6201, 696, 1120, 707, 697, 1108, 755, 705, 6222, 704, 689, 1455, 5697, 693, 1014, 5676, 706, 3698, 1094, 5740, 698, 1088, 712, 6202, 6205, 699, 126, 6223, 5138, 8288, 5500, 1714, 132, 1456, 17919, 710, 6366, 6217, 7658, 3699, 1106, 20752, 1086, 709, 1098, 5784, 1949, 2941, 691, 1490, 7646, 6213, 6229, 7648, 5699, 6219, 17920, 17951, 2362, 3700, 11687, 7641, 11711, 7651, 8289, 20755, 11733, 5484, 11665, 7659, 11707, 6789, 11683, 17921, 11739, 11671, 11725, 11693, 11659, 17952, 11729, 11721, 11699, 11688, 11677, 18647, 17727, 11712, 6353, 20756, 11719, 17925, 11734, 11667, 1122, 17922, 11708, 11675, 11694, 11660, 11740, 11672, 11700, 11684, 17928, 17953, 11717, 6215, 11689, 17924, 11730, 11713, 17926, 11726, 11678, 17923, 25311, 20757, 17728, 603, 11722, 11735, 54785, 11695, 11668, 25309, 50589, 18540, 11661, 50581, 28610, 27224, 23161, 27219, 28176, 25307, 29722, 27211, 27216, 27210, 27250, 28172, 29858, 27218, 27217, 27259, 27230, 27223, 27213, 27222, 29893, 27226, 27228, 30909, 27220, 28189, 27215, 27212, 27209, 27238, 30910, 27260, 32231, 30459, 27243, 30545, 47812, 50511, 47886, 61191, 47819, 47890, 47871, 47863, 47859, 60219, 47892, 47837, 47814, 47808, 47810, 47835, 47897, 47824, 47884, 47793, 47856, 47813, 47855, 47888, 47865, 47860, 47857, 47823, 47891, 47878, 47864, 57595, 47893, 47820, 47815, 47809, 60220, 47867, 47889, 48018, 48020, 47811, 47838, 57946, 58887, 47836, 61290, 47825, 0 }, // Warlock
            { 0 }, // N/A
            { 5185, 1126, 5176, 8921, 774, 467, 5177, 339, 5186, 5487, 99, 6795, 5232, 6807, 8924, 16689, 1058, 18960, 5229, 8936, 50769, 5211, 8946, 5187, 782, 5178, 1066, 8925, 1430, 779, 1062, 770, 16857, 2637, 6808, 16810, 8938, 768, 1082, 1735, 5188, 6756, 5215, 20484, 1079, 2912, 8926, 2090, 5221, 2908, 5179, 1822, 8939, 2782, 50768, 780, 1075, 5217, 2893, 1850, 5189, 6809, 8949, 5209, 3029, 8998, 5195, 8927, 16811, 2091, 9492, 6798, 778, 17390, 5234, 20739, 8940, 6800, 740, 783, 5180, 9490, 22568, 6778, 6785, 5225, 8972, 8928, 1823, 3627, 8950, 769, 8914, 22842, 9005, 8941, 50767, 9493, 6793, 5201, 5196, 8903, 18657, 16812, 8992, 8955, 6780, 9000, 20719, 22827, 16914, 29166, 8907, 8929, 20742, 8910, 8918, 9747, 9749, 17391, 9745, 6787, 9750, 8951, 22812, 9758, 1824, 9752, 9754, 9756, 8983, 9821, 9833, 9823, 9839, 9829, 8905, 9849, 9852, 22828, 16813, 9856, 50766, 9845, 21849, 9888, 17401, 9884, 9880, 9866, 20747, 9875, 9862, 9892, 9898, 9834, 9840, 9894, 9907, 17392, 9904, 9857, 9830, 9901, 9908, 9910, 9912, 22829, 9889, 9827, 9850, 9853, 18658, 33986, 33982, 9881, 9835, 17329, 9867, 9841, 9876, 31709, 31018, 21850, 25297, 17402, 9885, 20748, 9858, 25299, 50765, 9896, 25298, 9846, 9863, 53223, 27001, 26984, 26998, 26978, 22570, 24248, 26987, 26981, 33763, 27003, 26997, 26992, 33357, 26980, 26993, 27011, 33745, 27006, 27005, 27000, 26996, 27008, 26986, 26989, 33943, 33987, 33983, 27009, 27004, 26979, 26994, 26982, 50764, 26985, 33786, 26991, 27012, 26990, 26988, 27002, 26995, 26983, 53225, 48559, 48442, 49799, 40120, 62078, 50212, 48576, 48450, 48573, 48464, 48561, 48569, 48567, 48479, 48578, 48377, 49802, 53307, 48459, 48563, 48565, 48462, 48440, 52610, 48571, 48446, 53226, 48575, 48476, 48475, 48560, 49803, 48443, 48562, 53308, 48577, 53312, 48574, 48465, 48570, 48378, 48480, 48579, 48477, 50213, 48461, 48470, 48467, 48568, 48451, 48564, 48566, 48469, 48463, 50464, 48441, 50763, 49800, 48572, 48447, 53227, 61384, 62600, 0 }, // Druid
        };

        uint32 c = plr->getClass();
        for (uint32 i = 0; spellarray[c][i] != 0; ++i)
        {
            plr->addSpell(spellarray[c][i]);
        }

        static uint32 paladinspellarray[RACE_DRAENEI + 1][5] =
        {
            { 0 },        // RACE 0
            { 23214, 13819, 31801, 53720, 0 },        // HUMAN  Charger, Warhorse, Seal of Vengeance, Seal of Martyr
            { 0 },        // ORC
            { 23214, 13819, 31801, 53720, 0 },        // DWARF  Charger, Warhorse, Seal of Vengeance, Seal of Martyr
            { 0 },        // NIGHTELF
            { 0 },        // UNDEAD
            { 0 },        // TAUREN
            { 0 },        // GNOME
            { 0 },        // TROLL
            { 34767, 34769, 31892, 53736, 0 },        // BLOODELF  Summon Charger, Summon Warhorse, Seal of Blood, Seal of Corruption
            { 23214, 13819, 31801, 53720, 0 },        // DRAENEI  Charger, Warhorse, Seal of Vengeance, Seal of Martyr
        };

        static uint32 shamanspellarray[RACE_DRAENEI + 1][2] =
        {
            { 0 },        // RACE 0
            { 0 },        // HUMAN
            { 2825, 0 },              // ORC Bloodlust
            { 0 },        // DWARF
            { 0 },        // NIGHTELF
            { 0 },        // UNDEAD
            { 2825, 0 },              // TAUREN Bloodlust
            { 0 },        // GNOME
            { 2825, 0 },              // TROLL Bloodlust
            { 0 },        // BLOODELF
            { 32182, 0 },             // DRAENEI Heroism
        };

        static uint32 magespellarray[RACE_DRAENEI + 1][13] =
        {
            { 0 },        // RACE 0
            { 3561, 3562, 3565, 10059, 11416, 11419, 32266, 32271, 33690, 33691, 49359, 49360, 0 },        // HUMAN
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },        // ORC
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },        // DWARF
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },        // NIGHTELF
            { 3563, 3566, 3567, 11417, 11418, 11420, 32267, 32272, 35715, 35717, 49358, 49361, 0 },        // UNDEAD
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },        // TAUREN
            { 3561, 3562, 3565, 10059, 11416, 11419, 32266, 32271, 33690, 33691, 49359, 49360, 0 },        // GNOME
            { 3563, 3566, 3567, 11417, 11418, 11420, 32267, 32272, 35715, 35717, 49358, 49361, 0 },        // TROLL
            { 3563, 3566, 3567, 11417, 11418, 11420, 32267, 32272, 35715, 35717, 49358, 49361, 0 },        // BLOODELF
            { 3561, 3562, 3565, 10059, 11416, 11419, 32266, 32271, 33690, 33691, 49359, 49360, 0 },        // DRAENEI
        };

        uint32 r = plr->getRace();
        switch (c)
        {
            case PALADIN:
                for (uint32 i = 0; paladinspellarray[r][i] != 0; ++i)
                {
                    plr->addSpell(paladinspellarray[r][i]);
                }
                break;
            case MAGE:
                for (uint32 i = 0; magespellarray[r][i] != 0; ++i)
                {
                    plr->addSpell(magespellarray[r][i]);
                }
                break;
            case SHAMAN:
                for (uint32 i = 0; shamanspellarray[r][i] != 0; ++i)
                {
                    plr->addSpell(shamanspellarray[r][i]);
                }
                break;
        }
        return true;
    }

    uint32 spell = atol((char*)args);
    if (spell == 0)
    {
        spell = GetSpellIDFromLink(args);
    }

    SpellEntry* sp = dbcSpell.LookupEntryForced(spell);
    if (!sp)
    {
        SystemMessage(m_session, "Invalid spell %u", spell);
        return true;
    }

    if (!plr->GetSession()->HasGMPermissions() && (sp->Effect[0] == SPELL_EFFECT_INSTANT_KILL || sp->Effect[1] == SPELL_EFFECT_INSTANT_KILL || sp->Effect[2] == SPELL_EFFECT_INSTANT_KILL))
    {
        SystemMessage(m_session, "don't be an idiot and teach players instakill spells. this action has been logged.");
        return true;
    }

    if (plr->HasSpell(spell))     // check to see if char already knows
    {
        std::string OutStr = plr->GetName();
        OutStr += " already knows that spell.";

        SystemMessage(m_session, OutStr.c_str());
        return true;
    }

    plr->addSpell(spell);
    sGMLog.writefromsession(m_session, "Taught %s spell %u", plr->GetName(), spell);
    BlueSystemMessage(plr->GetSession(), "%s taught you Spell %u", m_session->GetPlayer()->GetName(), spell);
    GreenSystemMessage(m_session, "Taught %s Spell %u", plr->GetName(), spell);

    return true;
}

bool ChatHandler::HandleBanCharacterCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    char* pCharacter = (char*)args;
    PlayerInfo* pInfo = NULL;
    char* pReason;
    char* pDuration;
    int32 BanTime = 0;
    ParseBanArgs(pCharacter, &pDuration, &pReason);
    if (pDuration != NULL)
    {
        BanTime = GetTimePeriodFromString(pDuration);
        if (BanTime < 0) // if time is 0, ban is permanent
            return false;
    }

    Player* pPlayer = objmgr.GetPlayer(pCharacter, false);
    if (pPlayer == NULL)
    {
        pInfo = objmgr.GetPlayerInfoByName(pCharacter);
        if (pInfo == NULL)
        {
            SystemMessage(m_session, "Player not found.");
            return true;
        }
        SystemMessage(m_session, "Banning player '%s' in database for '%s'.", pCharacter, (pReason == NULL) ? "No reason." : pReason);
        std::string escaped_reason = (pReason == NULL) ? "No reason." : CharacterDatabase.EscapeString(std::string(pReason));
        CharacterDatabase.Execute("UPDATE characters SET banned = %u, banReason = '%s' WHERE guid = %u",
                                  BanTime ? BanTime + (uint32)UNIXTIME : 1, escaped_reason.c_str(), pInfo->guid);
    }
    else
    {
        SystemMessage(m_session, "Banning player '%s' ingame for '%s'.", pCharacter, (pReason == NULL) ? "No reason." : pReason);
        std::string sReason = (pReason == NULL) ? "No Reason." : std::string(pReason);
        uint32 uBanTime = BanTime ? BanTime + (uint32)UNIXTIME : 1;
        pPlayer->SetBanned(uBanTime, sReason);
        pInfo = pPlayer->getPlayerInfo();
    }
    SystemMessage(m_session, "This ban is due to expire %s%s.", BanTime ? "on " : "", BanTime ? ConvertTimeStampToDataTime(BanTime + (uint32)UNIXTIME).c_str() : "Never");

    sGMLog.writefromsession(m_session, "banned %s, reason %s, for %s", pCharacter, (pReason == NULL) ? "No reason" : pReason, BanTime ? ConvertTimeStampToString(BanTime).c_str() : "ever");
    char msg[200];
    snprintf(msg, 200, "%sGM: %s has been banned by %s for %s. Reason: %s", MSG_COLOR_RED, pCharacter, m_session->GetPlayer()->GetName(), BanTime ? ConvertTimeStampToString(BanTime).c_str() : "ever", (pReason == NULL) ? "No reason." : pReason);
    sWorld.SendWorldText(msg, NULL);
    if (sWorld.m_banTable && pInfo)
    {
        CharacterDatabase.Execute("INSERT INTO %s VALUES('%s', '%s', %u, %u, '%s')", sWorld.m_banTable, m_session->GetPlayer()->GetName(), pInfo->name, (uint32)UNIXTIME, (uint32)UNIXTIME + BanTime, (pReason == NULL) ? "No reason." : CharacterDatabase.EscapeString(std::string(pReason)).c_str());
    }

    if (pPlayer)
    {
        SystemMessage(m_session, "Kicking %s.", pPlayer->GetName());
        pPlayer->Kick();
    }
    return true;
}

bool ChatHandler::HandleBanAllCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    Player* pBanned;
    std::string pAcc;
    std::string pIP;
    std::string pArgs = args;
    char* pCharacter = (char*)args;
    char* pReason;
    char* pDuration;
    ParseBanArgs(pCharacter, &pDuration, &pReason);
    int32 BanTime = 0;
    if (pDuration != NULL)
    {
        BanTime = GetTimePeriodFromString(pDuration);
        if (BanTime < 0)
            return false;
    }
    pBanned = objmgr.GetPlayer(pCharacter, false);
    if (!pBanned || !pBanned->IsInWorld())
    {
        RedSystemMessage(m_session, "Player \'%s\' is not online or does not exists!", pCharacter);
        return true;
    }
    if (pBanned == m_session->GetPlayer())
    {
        RedSystemMessage(m_session, "You cannot ban yourself!");
        return true;
    }
    if (pBanned->GetSession() == NULL)
    {
        RedSystemMessage(m_session, "Player does not have a session!");
        return true;
    }
    if (pBanned->GetSession()->GetSocket() == NULL)
    {
        RedSystemMessage(m_session, "Player does not have a socket!");
        return true;
    }
    pAcc = pBanned->GetSession()->GetAccountName();
    pIP = pBanned->GetSession()->GetSocket()->GetRemoteIP();
    if (pIP == m_session->GetSocket()->GetRemoteIP())           //This check is there incase a gm tries to ban someone on their LAN etc.
    {
        RedSystemMessage(m_session, "That player has the same IP as you - ban failed");
        return true;
    }

    //Checks complete. time to fire it up?
    HandleBanCharacterCommand(pArgs.c_str(), m_session);
    char pIPCmd[256];
    snprintf(pIPCmd, 254, "%s %s %s", pIP.c_str(), pDuration, pReason);
    HandleIPBanCommand(pIPCmd, m_session);
    char pAccCmd[256];
    snprintf(pAccCmd, 254, "%s %s %s", pAcc.c_str(), pDuration, pReason);
    HandleAccountBannedCommand((const char*)pAccCmd, m_session);
    //GreenSystemMessage(m_session,"Successfully banned player %s with ip %s and account %s",pCharacter,pIP.c_str(),pAcc.c_str());
    return true;
}

bool ChatHandler::HandleUnBanCharacterCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    char Character[255];
    if (sscanf(args, "%s", Character) != 1)
    {
        RedSystemMessage(m_session, "A character name and reason is required.");
        return true;
    }

    // Check if player is in world.
    Player* pPlayer = ObjectMgr::getSingleton().GetPlayer(Character, false);
    if (pPlayer != 0)
    {
        GreenSystemMessage(m_session, "Unbanned player %s ingame.", pPlayer->GetName());
        pPlayer->UnSetBanned();
    }
    else
    {
        GreenSystemMessage(m_session, "Player %s not found ingame.", Character);
    }

    // Ban in database
    CharacterDatabase.Execute("UPDATE characters SET banned = 0 WHERE name = '%s'", CharacterDatabase.EscapeString(std::string(Character)).c_str());

    SystemMessage(m_session, "Unbanned character %s in database.", Character);
    sGMLog.writefromsession(m_session, "unbanned %s", Character);
    return true;
}

bool ChatHandler::HandleAddSkillCommand(const char* args, WorldSession* m_session)
{
    char buf[256];
    Player* target = objmgr.GetPlayer((uint32)m_session->GetPlayer()->GetSelection());

    if (!target)
    {
        SystemMessage(m_session, "Select A Player first.");
        return true;
    }

    uint32 skillline;
    uint16 cur, max;

    char* pSkillline = strtok((char*)args, " ");
    if (!pSkillline)
        return false;

    char* pCurrent = strtok(NULL, " ");
    if (!pCurrent)
        return false;

    char* pMax = strtok(NULL, " ");
    if (!pMax)
        return false;

    skillline = (uint32)atol(pSkillline);
    cur = (uint16)atol(pCurrent);
    max = (uint16)atol(pMax);

    target->_AddSkillLine(skillline, cur, max);

    snprintf(buf, 256, "SkillLine: %u CurrentValue %u Max Value %u Added.", (unsigned int)skillline, (unsigned int)cur, (unsigned int)max);
    sGMLog.writefromsession(m_session, "added skill line %u (%u/%u) to %s", skillline, cur, max, target->GetName());
    SystemMessage(m_session, buf);

    return true;
}

bool ChatHandler::HandleIncreaseWeaponSkill(const char* args, WorldSession* m_session)
{
    char* pMin = strtok((char*)args, " ");
    uint32 cnt = 0;
    if (!pMin)
        cnt = 1;
    else
        cnt = atol(pMin);

    Player* pr = GetSelectedPlayer(m_session, true, true);

    uint32 SubClassSkill = 0;
    if (!pr) pr = m_session->GetPlayer();
    if (!pr) return false;
    Item* it = pr->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

    ItemProperties const* proto = nullptr;
    if (!it)
        it = pr->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
    if (it)
        proto = it->GetItemProperties();
    if (proto)
    {
        switch (proto->SubClass)
        {
            // Weapons
            case 0:    // 1 handed axes
                SubClassSkill = SKILL_AXES;
                break;
            case 1:    // 2 handed axes
                SubClassSkill = SKILL_2H_AXES;
                break;
            case 2:    // bows
                SubClassSkill = SKILL_BOWS;
                break;
            case 3:    // guns
                SubClassSkill = SKILL_GUNS;
                break;
            case 4:    // 1 handed mace
                SubClassSkill = SKILL_MACES;
                break;
            case 5:    // 2 handed mace
                SubClassSkill = SKILL_2H_MACES;
                break;
            case 6:    // polearms
                SubClassSkill = SKILL_POLEARMS;
                break;
            case 7: // 1 handed sword
                SubClassSkill = SKILL_SWORDS;
                break;
            case 8: // 2 handed sword
                SubClassSkill = SKILL_2H_SWORDS;
                break;
            case 9: // obsolete
                SubClassSkill = 136;
                break;
            case 10: //1 handed exotic
                SubClassSkill = 136;
                break;
            case 11: // 2 handed exotic
                SubClassSkill = 0;
                break;
            case 12: // fist
                SubClassSkill = SKILL_FIST_WEAPONS;
                break;
            case 13: // misc
                SubClassSkill = 0;
                break;
            case 15: // daggers
                SubClassSkill = SKILL_DAGGERS;
                break;
            case 16: // thrown
                SubClassSkill = SKILL_THROWN;
                break;
            case 18: // crossbows
                SubClassSkill = SKILL_CROSSBOWS;
                break;
            case 19: // wands
                SubClassSkill = SKILL_WANDS;
                break;
            case 20: // fishing
                SubClassSkill = SKILL_FISHING;
                break;
        }
    }
    else
    {
        SubClassSkill = 162;
    }

    if (!SubClassSkill)
    {
        RedSystemMessage(m_session, "Can't find skill ID :-/");
        return false;
    }

    uint32 skill = SubClassSkill;

    BlueSystemMessage(m_session, "Modifying skill line %d. Advancing %d times.", skill, cnt);
    sGMLog.writefromsession(m_session, "increased weapon skill (%u) of %s by %u", skill, pr->GetName(), cnt);

    if (!pr->_HasSkillLine(skill))
    {
        SystemMessage(m_session, "Does not have skill line, adding.");
        pr->_AddSkillLine(skill, 1, 450);
    }
    else
    {
        pr->_AdvanceSkillLine(skill, cnt);
    }
    return true;
}


bool ChatHandler::HandleResetTalentsCommand(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr)
        return true;

    plr->Reset_Talents();

    SystemMessage(m_session, "Reset talents of %s.", plr->GetName());
    BlueSystemMessage(plr->GetSession(), "%s reset all your talents.", m_session->GetPlayer()->GetName());
    sGMLog.writefromsession(m_session, "reset talents of %s", plr->GetName());
    return true;
}

bool ChatHandler::HandleResetSpellsCommand(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr)
        return true;

    plr->Reset_Spells();

    SystemMessage(m_session, "Reset spells of %s to level 1.", plr->GetName());
    BlueSystemMessage(plr->GetSession(), "%s reset all your spells to starting values.", m_session->GetPlayer()->GetName());
    sGMLog.writefromsession(m_session, "reset spells of %s", plr->GetName());
    return true;
}

bool ChatHandler::HandleCreatePetCommand(const char* args, WorldSession* m_session)
{
    if ((args == NULL) || (strlen(args) < 2))
        return false;

    uint32 entry = atol(args);
    if (entry == 0)
        return false;

    CreatureProperties const* cp = sMySQLStore.GetCreatureProperties(entry);
    if (cp == nullptr)
        return false;

    Player* p = m_session->GetPlayer();

    p->DismissActivePets();
    p->RemoveFieldSummon();

    float followangle = -M_PI_FLOAT * 2;
    LocationVector v(p->GetPosition());

    v.x += (3 * (cosf(followangle + p->GetOrientation())));
    v.y += (3 * (sinf(followangle + p->GetOrientation())));

    Pet* pet = objmgr.CreatePet(entry);

    if (!pet->CreateAsSummon(entry, cp, NULL, p, NULL, 1, 0, &v, true))
    {
        pet->DeleteMe();
        return true;
    }

    pet->GetAIInterface()->SetUnitToFollowAngle(followangle);

    return true;
}

bool ChatHandler::HandleAddPetSpellCommand(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr)
        return false;

    if (plr->GetSummon() == NULL)
    {
        RedSystemMessage(m_session, "%s has no pet.", plr->GetName());
        return true;
    }

    uint32 SpellId = atol(args);
    SpellEntry* spell = dbcSpell.LookupEntryForced(SpellId);
    if (!SpellId || !spell)
    {
        RedSystemMessage(m_session, "Invalid spell id requested.");
        return true;
    }

    std::list<Pet*> summons = plr->GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        (*itr)->AddSpell(spell, true);
    }
    GreenSystemMessage(m_session, "Added spell %u to %s's pet.", SpellId, plr->GetName());
    return true;
}

bool ChatHandler::HandleRemovePetSpellCommand(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, false, true);
    if (!plr)
        return false;

    if (plr->GetSummon() == NULL)
    {
        RedSystemMessage(m_session, "%s has no pet.", plr->GetName());
        return true;
    }

    uint32 SpellId = atol(args);
    SpellEntry* spell = dbcSpell.LookupEntryForced(SpellId);
    if (!SpellId || !spell)
    {
        RedSystemMessage(m_session, "Invalid spell id requested.");
        return true;
    }

    std::list<Pet*> summons = plr->GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        (*itr)->RemoveSpell(SpellId);
    }
    GreenSystemMessage(m_session, "Removed spell %u from %s's pet.", SpellId, plr->GetName());
    return true;
}

bool ChatHandler::HandleRenamePetCommand(const char* args, WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();
    Pet* pPet = plr->GetSummon();
    if (pPet == NULL)
    {
        RedSystemMessage(m_session, "You have no pet.");
        return true;
    }

    if (strlen(args) < 1)
    {
        RedSystemMessage(m_session, "You must specify a name.");
        return true;
    }

    GreenSystemMessage(m_session, "Renamed your pet to %s.", args);
    pPet->Rename(args);//support for only 1st pet
    return true;
}

bool ChatHandler::HandleDismissPetCommand(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, false, true);
    Pet* pPet = NULL;
    if (plr)
    {
        if (plr->GetSummon() == NULL)
        {
            RedSystemMessage(m_session, "Player has no pet.");
            return true;
        }
        else
        {
            plr->DismissActivePets();
        }
    }
    else
    {
        // no player selected, see if it is a pet
        Creature* pCrt = GetSelectedCreature(m_session, false);
        if (!pCrt)
        {
            // show usage string
            return false;
        }
        if (pCrt->IsPet())
        {
            pPet = static_cast< Pet* >(pCrt);
        }
        if (!pPet)
        {
            RedSystemMessage(m_session, "No player or pet selected.");
            return true;
        }
        plr = pPet->GetPetOwner();
        pPet->Dismiss();
    }

    GreenSystemMessage(m_session, "Dismissed %s's pet.", plr->GetName());
    plr->GetSession()->SystemMessage("%s dismissed your pet.", m_session->GetPlayer()->GetName());
    return true;
}

bool ChatHandler::HandlePetLevelCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        return false;
    }

    int32 newLevel = atol(args);
    if (newLevel < 1)
    {
        return false;
    }

    Player* plr = GetSelectedPlayer(m_session, false, true);
    Pet* pPet = NULL;
    if (plr)
    {
        pPet = plr->GetSummon();
        if (!pPet)
        {
            RedSystemMessage(m_session, "Player has no pet.");
            return true;
        }
    }
    else
    {
        // no player selected, see if it is a pet
        Creature* pCrt = GetSelectedCreature(m_session, false);
        if (!pCrt)
        {
            // show usage string
            return false;
        }
        if (pCrt->IsPet())
        {
            pPet = static_cast< Pet* >(pCrt);
        }
        if (!pPet)
        {
            RedSystemMessage(m_session, "No player or pet selected.");
            return true;
        }
        plr = pPet->GetPetOwner();
    }

    // Should GMs be allowed to set a pet higher than its owner?  I don't think so
    if ((uint32)newLevel > plr->getLevel())
    {
        newLevel = plr->getLevel();
    }

    //support for only 1 pet
    pPet->setLevel(newLevel);
    pPet->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
    pPet->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, pPet->GetNextLevelXP(newLevel));
    pPet->ApplyStatsForLevel();
    pPet->UpdateSpellList();

    GreenSystemMessage(m_session, "Set %s's pet to level %lu.", plr->GetName(), newLevel);
    plr->GetSession()->SystemMessage("%s set your pet to level %lu.", m_session->GetPlayer()->GetName(), newLevel);
    return true;
}

bool ChatHandler::HandleAdvanceAllSkillsCommand(const char* args, WorldSession* m_session)
{
    uint32 amt = args ? atol(args) : 0;
    if (!amt)
    {
        RedSystemMessage(m_session, "An amount to increment is required.");
        return true;
    }

    Player* plr = GetSelectedPlayer(m_session, false, true);
    if (!plr)
        return true;


    plr->_AdvanceAllSkills(amt);
    GreenSystemMessage(plr->GetSession(), "Advanced all your skill lines by %u points.", amt);
    sGMLog.writefromsession(m_session, "advanced all skills by %u on %s", amt, plr->GetName());
    return true;
}

bool ChatHandler::HandleMassSummonCommand(const char* args, WorldSession* m_session)
{
    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    Player* summoner = m_session->GetPlayer();
    Player* plr;
    int faction = -1;
    char Buffer[170];
    if (*args == 'a' || *args == 'A')
    {
        faction = 0;
        snprintf(Buffer, 170, "%s%s Has requested a mass summon of all Alliance players. Do not feel obliged to accept the summon, as it is most likely for an event or a test of sorts", MSG_COLOR_GOLD, m_session->GetPlayer()->GetName());

    }
    else if (*args == 'h' || *args == 'H')
    {
        faction = 1;
        snprintf(Buffer, 170, "%s%s Has requested a mass summon of all Horde players. Do not feel obliged to accept the summon, as it is most likely for an event or a test of sorts", MSG_COLOR_GOLD, m_session->GetPlayer()->GetName());
    }
    else  snprintf(Buffer, 170, "%s%s Has requested a mass summon of all players. Do not feel obliged to accept the summon, as it is most likely for an event or a test of sorts", MSG_COLOR_GOLD, m_session->GetPlayer()->GetName());

    uint32 c = 0;

    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        plr = itr->second;
        if (plr->GetSession() && plr->IsInWorld())
        {
            //plr->SafeTeleport(summoner->GetMapId(), summoner->GetInstanceID(), summoner->GetPosition());
            /* let's do this the blizz way */
            if (faction > -1 && plr->GetTeam() == static_cast<uint32>(faction))
            {
                plr->SummonRequest(summoner->GetLowGUID(), summoner->GetZoneId(), summoner->GetMapId(), summoner->GetInstanceID(), summoner->GetPosition());
                ++c;
            }
            else if (faction == -1)
            {
                plr->SummonRequest(summoner->GetLowGUID(), summoner->GetZoneId(), summoner->GetMapId(), summoner->GetInstanceID(), summoner->GetPosition());
                ++c;
            }

        }
    }
    sGMLog.writefromsession(m_session, "requested a mass summon of %u players.", c);
    objmgr._playerslock.ReleaseReadLock();
    return true;
}

bool ChatHandler::HandleCastAllCommand(const char* args, WorldSession* m_session)
{
    if (!args || strlen(args) < 2)
    {
        RedSystemMessage(m_session, "No spellid specified.");
        return true;
    }
    Player* plr;
    uint32 spellid = atol(args);
    SpellEntry* info = dbcSpell.LookupEntryForced(spellid);
    if (!info)
    {
        RedSystemMessage(m_session, "Invalid spell specified.");
        return true;
    }

    // this makes sure no moron casts a learn spell on everybody and wrecks the server
    for (uint8 i = 0; i < 3; i++)
    {
        if (info->Effect[i] == SPELL_EFFECT_LEARN_SPELL)  //SPELL_EFFECT_LEARN_SPELL - 36
        {
            sGMLog.writefromsession(m_session, "used wrong / learnall castall command, spellid %u", spellid);
            RedSystemMessage(m_session, "Learn spell specified.");
            return true;
        }
    }

    sGMLog.writefromsession(m_session, "used castall command, spellid %u", spellid);

    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        plr = itr->second;
        if (plr->GetSession() && plr->IsInWorld())
        {
            if (plr->GetMapMgr() != m_session->GetPlayer()->GetMapMgr())
            {
                sEventMgr.AddEvent(static_cast< Unit* >(plr), &Unit::EventCastSpell, static_cast< Unit* >(plr), info, EVENT_PLAYER_CHECKFORCHEATS, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            }
            else
            {
                Spell* sp = sSpellFactoryMgr.NewSpell(plr, info, true, 0);
                SpellCastTargets targets(plr->GetGUID());
                sp->prepare(&targets);
            }
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    BlueSystemMessage(m_session, "Casted spell %u on all players!", spellid);
    return true;
}

bool ChatHandler::HandleResetSkillsCommand(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr)
        return true;

    plr->_RemoveAllSkills();

    // Load skills from create info.
    PlayerCreateInfo* info = objmgr.GetPlayerCreateInfo(plr->getRace(), plr->getClass());
    if (!info) return true;

    for (std::list<CreateInfo_SkillStruct>::iterator ss = info->skills.begin(); ss != info->skills.end(); ++ss)
    {
        auto skill_line = sSkillLineStore.LookupEntry(ss->skillid);
        if (skill_line == nullptr)
            continue;

        if (skill_line->type != SKILL_TYPE_LANGUAGE && ss->skillid && ss->currentval && ss->maxval)
            plr->_AddSkillLine(ss->skillid, ss->currentval, ss->maxval);
    }
    //Chances depend on stats must be in this order!
    plr->UpdateStats();
    plr->UpdateChances();
    plr->_UpdateMaxSkillCounts();
    plr->_AddLanguages(false);
    BlueSystemMessage(m_session, "Reset skills to default.");
    sGMLog.writefromsession(m_session, "reset skills of %s to default", plr->GetName());
    return true;
}

bool ChatHandler::HandlePlayerInfo(const char* args, WorldSession* m_session)
{
    Player* plr;
    if (strlen(args) >= 2) // char name can be 2 letters
    {
        plr = objmgr.GetPlayer(args, false);
        if (!plr)
        {
            RedSystemMessage(m_session, "Unable to locate player %s.", args);
            return true;
        }
    }
    else
        plr = GetSelectedPlayer(m_session, true, true);

    if (!plr) return true;
    if (!plr->GetSession())
    {
        RedSystemMessage(m_session, "ERROR: this player hasn't got any session !");
        return true;
    }
    if (!plr->GetSession()->GetSocket())
    {
        RedSystemMessage(m_session, "ERROR: this player hasn't got any socket !");
        return true;
    }
    WorldSession* sess = plr->GetSession();

    //    char* infos = new char[128];
    static const char* classes[12] =
    { "None", "Warrior", "Paladin", "Hunter", "Rogue", "Priest", "Death Knight", "Shaman", "Mage", "Warlock", "None", "Druid" };
    static const char* races[12] =
    { "None", "Human", "Orc", "Dwarf", "Night Elf", "Undead", "Tauren", "Gnome", "Troll", "None", "Blood Elf", "Draenei" };

    char playedLevel[64];
    char playedTotal[64];

    int seconds = (plr->GetPlayedtime())[0];
    int mins = 0;
    int hours = 0;
    int days = 0;
    if (seconds >= 60)
    {
        mins = seconds / 60;
        if (mins)
        {
            seconds -= mins * 60;
            if (mins >= 60)
            {
                hours = mins / 60;
                if (hours)
                {
                    mins -= hours * 60;
                    if (hours >= 24)
                    {
                        days = hours / 24;
                        if (days)
                            hours -= days * 24;
                    }
                }
            }
        }
    }
    snprintf(playedLevel, 64, "[%d days, %d hours, %d minutes, %d seconds]", days, hours, mins, seconds);

    seconds = (plr->GetPlayedtime())[1];
    mins = 0;
    hours = 0;
    days = 0;
    if (seconds >= 60)
    {
        mins = seconds / 60;
        if (mins)
        {
            seconds -= mins * 60;
            if (mins >= 60)
            {
                hours = mins / 60;
                if (hours)
                {
                    mins -= hours * 60;
                    if (hours >= 24)
                    {
                        days = hours / 24;
                        if (days)
                            hours -= days * 24;
                    }
                }
            }
        }
    }
    snprintf(playedTotal, 64, "[%d days, %d hours, %d minutes, %d seconds]", days, hours, mins, seconds);

    GreenSystemMessage(m_session, "%s is a %s %s %s", plr->GetName(),
                       (plr->getGender() ? "Female" : "Male"), races[plr->getRace()], classes[plr->getClass()]);

    BlueSystemMessage(m_session, "%s has played %s at this level", (plr->getGender() ? "She" : "He"), playedLevel);
    BlueSystemMessage(m_session, "and %s overall", playedTotal);

    BlueSystemMessage(m_session, "%s is connecting from account '%s'[%u] with permissions '%s'",
                      (plr->getGender() ? "She" : "He"), sess->GetAccountName().c_str(), sess->GetAccountId(), sess->GetPermissions());

    const char* client;

    // Clean code says you need to work from highest combined bit to lowest. Second, you need to check if both flags exists.
    if (sess->HasFlag(ACCOUNT_FLAG_XPACK_02) && sess->HasFlag(ACCOUNT_FLAG_XPACK_01))
        client = "TBC and WotLK";
    else if (sess->HasFlag(ACCOUNT_FLAG_XPACK_02))
        client = "Wrath of the Lich King";
    else if (sess->HasFlag(ACCOUNT_FLAG_XPACK_01))
        client = "WoW Burning Crusade";
    else
        client = "WoW";

    BlueSystemMessage(m_session, "%s uses %s (build %u)", (plr->getGender() ? "She" : "He"),
                      client, sess->GetClientBuild());

    BlueSystemMessage(m_session, "%s IP is '%s', and has a latency of %ums", (plr->getGender() ? "Her" : "His"),
                      sess->GetSocket()->GetRemoteIP().c_str(), sess->GetLatency());

    return true;
}

bool ChatHandler::HandleGlobalPlaySoundCommand(const char* args, WorldSession* m_session)
{
    if (*args == '\0')
        return false;

    uint32 sound = atoi(args);
    if (sound == 0)
        return false;

    sWorld.PlaySoundToAll(sound);
    BlueSystemMessage(m_session, "Broadcasted sound %u to server.", sound);
    sGMLog.writefromsession(m_session, "used play all command soundid %u", sound);

    return true;
}

bool ChatHandler::HandleIPBanCommand(const char* args, WorldSession* m_session)
{
    char* pIp = (char*)args;
    char* pReason;
    char* pDuration;
    ParseBanArgs(pIp, &pDuration, &pReason);
    int32 timeperiod = 0;
    if (pDuration != NULL)
    {
        timeperiod = GetTimePeriodFromString(pDuration);
        if (timeperiod < 0)
            return false;
    }

    uint32 o1, o2, o3, o4;
    if (sscanf(pIp, "%3u.%3u.%3u.%3u", (unsigned int*)&o1, (unsigned int*)&o2, (unsigned int*)&o3, (unsigned int*)&o4) != 4
        || o1 > 255 || o2 > 255 || o3 > 255 || o4 > 255)
    {
        RedSystemMessage(m_session, "Invalid IPv4 address [%s]", pIp);
        return true;    // error in syntax, but we wont remind client of command usage
    }

    time_t expire_time;
    if (timeperiod == 0)        // permanent ban
        expire_time = 0;
    else
        expire_time = UNIXTIME + (time_t)timeperiod;
    std::string IP = pIp;
    std::string::size_type i = IP.find("/");
    if (i == std::string::npos)
    {
        RedSystemMessage(m_session, "Lack of CIDR address assumes a 32bit match (if you don't understand, don't worry, it worked)");
        IP.append("/32");
    }

    //temporal IP or real pro flooder who will change it tomorrow ?
    char emptystring = 0;
    if (pReason == NULL)
        pReason = &emptystring;

    SystemMessage(m_session, "Adding [%s] to IP ban table, expires %s.Reason is :%s", pIp, (expire_time == 0) ? "Never" : ctime(&expire_time), pReason);
    sLogonCommHandler.IPBan_Add(IP.c_str(), (uint32)expire_time, pReason);
    sWorld.DisconnectUsersWithIP(IP.substr(0, IP.find("/")).c_str(), m_session);
    sGMLog.writefromsession(m_session, "banned ip address %s, expires %s", pIp, (expire_time == 0) ? "Never" : ctime(&expire_time));
    return true;
}

bool ChatHandler::HandleIPUnBanCommand(const char* args, WorldSession* m_session)
{
    std::string pIp = args;
    if (pIp.length() == 0)
        return false;

    if (pIp.find("/") == std::string::npos)
    {
        RedSystemMessage(m_session, "Lack of CIDR address assumes a 32bit match (if you don't understand, don't worry, it worked)");
        pIp.append("/32");
    }
    /**
    * We can afford to be less fussy with the validity of the IP address given since
    * we are only attempting to remove it.
    * Sadly, we can only blindly execute SQL statements on the logonserver so we have
    * no idea if the address existed and so the account/IPBanner cache requires reloading.
    */

    SystemMessage(m_session, "Deleting [%s] from ip ban table if it exists", pIp.c_str());
    sLogonCommHandler.IPBan_Remove(pIp.c_str());
    sGMLog.writefromsession(m_session, "unbanned ip address %s", pIp.c_str());
    return true;
}

bool ChatHandler::HandleRemoveItemCommand(const char* args, WorldSession* m_session)
{
    uint32 item_id;
    int32 count, ocount;
    int argc = sscanf(args, "%u %u", (unsigned int*)&item_id, (unsigned int*)&count);
    if (argc == 1)
        count = 1;
    else if (argc != 2 || !count)
        return false;

    ocount = count;
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr) return true;

    // loop until they're all gone.
    int32 loop_count = 0;
    int32 start_count = plr->GetItemInterface()->GetItemCount(item_id, true);
    int32 start_count2 = start_count;
    if (count > start_count)
        count = start_count;

    while (start_count >= count && (count > 0) && loop_count < 20)     // Prevent a loop here.
    {
        plr->GetItemInterface()->RemoveItemAmt(item_id, count);
        start_count2 = plr->GetItemInterface()->GetItemCount(item_id, true);
        count -= (start_count - start_count2);
        start_count = start_count2;
        ++loop_count;
    }

    ItemProperties const* iProto = sMySQLStore.GetItemProperties(item_id);
    if (iProto)
    {
        sGMLog.writefromsession(m_session, "used remove item %s (id: %u) count %u from %s", iProto->Name.c_str(), item_id, ocount, plr->GetName());
        BlueSystemMessage(m_session, "Removing %u copies of item %s (id: %u) from %s's inventory.", ocount, GetItemLinkByProto(iProto, m_session->language).c_str(), item_id, plr->GetName());
        BlueSystemMessage(plr->GetSession(), "%s removed %u copies of item %s from your inventory.", m_session->GetPlayer()->GetName(), ocount, GetItemLinkByProto(iProto, plr->GetSession()->language).c_str());
    }
    else RedSystemMessage(m_session, "Cannot remove non valid item id: %u .", item_id);

    return true;
}

void ChatHandler::SendHighlightedName(WorldSession* m_session, const char* prefix, const char* full_name, std::string & lowercase_name, std::string & highlight, uint32 id)
{
    char message[1024];
    char start[50];
    start[0] = 0;
    message[0] = 0;

    snprintf(start, 50, "%s %u: %s", prefix, (unsigned int)id, MSG_COLOR_WHITE);

    auto highlight_length = highlight.length();
    std::string fullname = std::string(full_name);
    size_t offset = (size_t)lowercase_name.find(highlight);
    auto remaining = fullname.size() - offset - highlight_length;

    strcat(message, start);
    strncat(message, fullname.c_str(), offset);
    strcat(message, MSG_COLOR_LIGHTRED);
    strncat(message, (fullname.c_str() + offset), highlight_length);
    strcat(message, MSG_COLOR_WHITE);
    if (remaining > 0)
        strncat(message, (fullname.c_str() + offset + highlight_length), remaining);

    SystemMessage(m_session, message);
}

void ChatHandler::SendItemLinkToPlayer(ItemProperties const* iProto, WorldSession* pSession, bool ItemCount, Player* owner, uint32 language)
{
    if (!iProto || !pSession)
        return;
    if (ItemCount && owner == NULL)
        return;

    if (ItemCount)
    {
        int8 count = static_cast<int8>(owner->GetItemInterface()->GetItemCount(iProto->ItemId, true));
        //int8 slot = owner->GetItemInterface()->GetInventorySlotById(iProto->ItemId); //DISABLED due to being a retarded concept
        if (iProto->ContainerSlots > 0)
        {
            SystemMessage(pSession, "Item %u %s Count %u ContainerSlots %u", iProto->ItemId, GetItemLinkByProto(iProto, language).c_str(), count, iProto->ContainerSlots);
        }
        else
        {
            SystemMessage(pSession, "Item %u %s Count %u", iProto->ItemId, GetItemLinkByProto(iProto, language).c_str(), count);
        }
    }
    else
    {
        if (iProto->ContainerSlots > 0)
        {
            SystemMessage(pSession, "Item %u %s ContainerSlots %u", iProto->ItemId, GetItemLinkByProto(iProto, language).c_str(), iProto->ContainerSlots);
        }
        else
        {
            SystemMessage(pSession, "Item %u %s", iProto->ItemId, GetItemLinkByProto(iProto, language).c_str());
        }
    }
}

struct spell_thingo
{
    uint32 type;
    uint32 target;
};

std::list<SpellEntry*> aiagent_spells;
std::map<uint32, spell_thingo> aiagent_extra;

bool ChatHandler::HandleAIAgentDebugBegin(const char* args, WorldSession* m_session)
{
    QueryResult* result = WorldDatabase.Query("SELECT DISTINCT spell FROM ai_agents");
    if (!result) return false;

    do
    {
        SpellEntry* se = dbcSpell.LookupEntryForced(result->Fetch()[0].GetUInt32());
        if (se)
            aiagent_spells.push_back(se);
    }
    while (result->NextRow());
    delete result;

    for (std::list<SpellEntry*>::iterator itr = aiagent_spells.begin(); itr != aiagent_spells.end(); ++itr)
    {
        result = WorldDatabase.Query("SELECT * FROM ai_agents WHERE spell = %u", (*itr)->Id);
        ARCEMU_ASSERT(result != NULL);
        spell_thingo t;
        t.type = result->Fetch()[6].GetUInt32();
        t.target = result->Fetch()[7].GetUInt32();
        delete result;
        aiagent_extra[(*itr)->Id] = t;
    }

    GreenSystemMessage(m_session, "Loaded %u spells for testing.", aiagent_spells.size());
    return true;
}

SpellCastTargets SetTargets(SpellEntry* sp, uint32 type, uint32 targettype, Unit* dst, Creature* src)
{
    SpellCastTargets targets;
    targets.m_unitTarget = 0;
    targets.m_itemTarget = 0;
    targets.m_srcX = 0;
    targets.m_srcY = 0;
    targets.m_srcZ = 0;
    targets.m_destX = 0;
    targets.m_destY = 0;
    targets.m_destZ = 0;

    if (targettype == TTYPE_SINGLETARGET)
    {
        targets.m_targetMask = TARGET_FLAG_UNIT;
        targets.m_unitTarget = dst->GetGUID();
    }
    else if (targettype == TTYPE_SOURCE)
    {
        targets.m_targetMask = TARGET_FLAG_SOURCE_LOCATION;
        targets.m_srcX = src->GetPositionX();
        targets.m_srcY = src->GetPositionY();
        targets.m_srcZ = src->GetPositionZ();
    }
    else if (targettype == TTYPE_DESTINATION)
    {
        targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;
        targets.m_destX = dst->GetPositionX();
        targets.m_destY = dst->GetPositionY();
        targets.m_destZ = dst->GetPositionZ();
    }

    return targets;
};

bool ChatHandler::HandleAIAgentDebugContinue(const char* args, WorldSession* m_session)
{
    uint32 count = atoi(args);
    if (!count)
        return false;

    Creature* pCreature = GetSelectedCreature(m_session, true);
    if (!pCreature)
        return true;

    Player* pPlayer = m_session->GetPlayer();

    for (uint32 i = 0; i < count; ++i)
    {
        if (!aiagent_spells.size())
            break;

        SpellEntry* sp = *aiagent_spells.begin();
        aiagent_spells.erase(aiagent_spells.begin());
        BlueSystemMessage(m_session, "Casting %u, " MSG_COLOR_SUBWHITE "%u remaining.", sp->Id, aiagent_spells.size());

        std::map<uint32, spell_thingo>::iterator it = aiagent_extra.find(sp->Id);
        ARCEMU_ASSERT(it != aiagent_extra.end());

        SpellCastTargets targets;
        if (it->second.type == STYPE_BUFF)
            targets = SetTargets(sp, it->second.type, it->second.type, pCreature, pCreature);
        else
            targets = SetTargets(sp, it->second.type, it->second.type, pPlayer, pCreature);

        pCreature->GetAIInterface()->CastSpell(pCreature, sp, targets);
    }

    if (!aiagent_spells.size())
        RedSystemMessage(m_session, "Finished.");
    /*else
        BlueSystemMessage(m_session, "Got %u remaining.", aiagent_spells.size());*/
    return true;
}

bool ChatHandler::HandleAIAgentDebugSkip(const char* args, WorldSession* m_session)
{
    uint32 count = atoi(args);
    if (!count) return false;

    for (uint32 i = 0; i < count; ++i)
    {
        if (!aiagent_spells.size())
            break;

        aiagent_spells.erase(aiagent_spells.begin());
    }
    BlueSystemMessage(m_session, "Erased %u spells.", count);
    return true;
}

bool ChatHandler::HandleRenameGuildCommand(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr || !plr->GetGuildId() || !plr->GetGuild() || !args || !strlen(args))
        return false;

    Guild* pGuild = objmgr.GetGuildByGuildName(std::string(args));

    if (pGuild)
    {
        RedSystemMessage(m_session, "Guild name %s is already taken.", args);
        return false;
    }

    GreenSystemMessage(m_session, "Changed guild name of %s to %s. This will take effect next restart.", plr->GetGuild()->GetGuildName(), args);
    CharacterDatabase.Execute("UPDATE guilds SET `guildName` = \'%s\' WHERE `guildId` = '%u'", CharacterDatabase.EscapeString(std::string(args)).c_str(), plr->GetGuild()->GetGuildId());
    sGMLog.writefromsession(m_session, "Changed guild name of '%s' to '%s'", plr->GetGuild()->GetGuildName(), args);
    return true;
}

//People seem to get stuck in guilds from time to time. This should be helpful. -DGM
bool ChatHandler::HandleGuildRemovePlayerCommand(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr || !plr->GetGuildId() || !plr->GetGuild() || plr->GetGuild()->GetGuildLeader() == plr->GetLowGUID())
        return false;

    GreenSystemMessage(m_session, "Kicked %s from Guild: %s", plr->GetName(), plr->GetGuild()->GetGuildName());

    if (plr->GetLowGUID() != m_session->GetPlayer()->GetLowGUID())
        sGMLog.writefromsession(m_session, "Kicked %s from Guild %s", plr->GetName(), plr->GetGuild()->GetGuildName());

    plr->GetGuild()->RemoveGuildMember(plr->getPlayerInfo(), plr->GetSession());
    return true;
}

//-DGM
bool ChatHandler::HandleGuildDisbandCommand(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr || !plr->GetGuildId() || !plr->GetGuild())
        return false;

    GreenSystemMessage(m_session, "Disbanded Guild: %s", plr->GetGuild()->GetGuildName());
    sGMLog.writefromsession(m_session, "Disbanded Guild %s", plr->GetGuild()->GetGuildName());
    plr->GetGuild()->Disband();
    return true;
}

bool ChatHandler::HandleGuildJoinCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    Player* ptarget = GetSelectedPlayer(m_session, true, true);
    if (!ptarget) return false;

    if (ptarget->IsInGuild())
    {
        RedSystemMessage(m_session, "%s is already in a guild.", ptarget->GetName());
        return true;
    }

    Guild* pGuild = NULL;
    pGuild = objmgr.GetGuildByGuildName(std::string(args));

    if (pGuild)
    {
        pGuild->getLock().Acquire();
        uint32 memberCount = pGuild->GetNumMembers();
        pGuild->getLock().Release();

        if (memberCount >= MAX_GUILD_MEMBERS)
        {
            m_session->SystemMessage("That guild is full.");
            return true;
        }

        pGuild->AddGuildMember(ptarget->getPlayerInfo(), m_session, -2);
        GreenSystemMessage(m_session, "You have joined the guild '%s'", pGuild->GetGuildName());
        sGMLog.writefromsession(m_session, "Force joined guild '%s'", pGuild->GetGuildName());
        return true;
    }

    return false;
}

//-DGM
bool ChatHandler::HandleGuildMembersCommand(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr || !plr->GetGuildId() || !plr->GetGuild())
        return false;
    GreenSystemMessage(m_session, "Now showing guild members for %s", plr->GetGuild()->GetGuildName());
    plr->GetGuild()->Lock();
    for (GuildMemberMap::iterator itr = plr->GetGuild()->GetGuildMembersBegin(); itr != plr->GetGuild()->GetGuildMembersEnd(); ++itr)
    {
        GuildMember* member = itr->second;
        if (!member || !member->pPlayer)
            continue;

        BlueSystemMessage(m_session, "%s (Rank: %s)", member->pPlayer->name, member->pRank->szRankName);
    }
    plr->GetGuild()->Unlock();
    return true;
}

bool ChatHandler::HandleDispelAllCommand(const char* args, WorldSession* m_session)
{
    uint32 pos = 0;
    if (*args)
        pos = atoi(args);

    Player* plr;

    sGMLog.writefromsession(m_session, "used dispelall command, pos %u", pos);

    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        plr = itr->second;
        if (plr->GetSession() && plr->IsInWorld())
        {
            if (plr->GetMapMgr() != m_session->GetPlayer()->GetMapMgr())
            {
                sEventMgr.AddEvent(static_cast< Unit* >(plr), &Unit::DispelAll, pos ? true : false, EVENT_PLAYER_CHECKFORCHEATS, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            }
            else
            {
                plr->DispelAll(pos ? true : false);
            }
        }
    }
    sGMLog.writefromsession(m_session, "used mass dispel");
    objmgr._playerslock.ReleaseReadLock();

    BlueSystemMessage(m_session, "Dispel action done.");
    return true;
}

bool ChatHandler::HandleCollisionTestIndoor(const char* args, WorldSession* m_session)
{
    if (sWorld.Collision)
    {
        Player* plr = m_session->GetPlayer();
        const LocationVector & loc = plr->GetPosition();
        bool res = CollideInterface.IsIndoor(plr->GetMapId(), loc.x, loc.y, loc.z + 2.0f);
        SystemMessage(m_session, "Result was: %s.", res ? "indoors" : "outside");
        return true;
    }
    else
    {
        SystemMessage(m_session, "Collision is not enabled.");
        return true;
    }
}

bool ChatHandler::HandleCollisionTestLOS(const char* args, WorldSession* m_session)
{
    if (sWorld.Collision)
    {
        Object* pObj = NULL;
        Creature* pCreature = GetSelectedCreature(m_session, false);
        Player* pPlayer = GetSelectedPlayer(m_session, true, true);
        if (pCreature)
            pObj = pCreature;
        else if (pPlayer)
            pObj = pPlayer;

        if (pObj == NULL)
        {
            SystemMessage(m_session, "Invalid target.");
            return true;
        }

        const LocationVector & loc2 = pObj->GetPosition();
        const LocationVector & loc1 = m_session->GetPlayer()->GetPosition();
        bool res = CollideInterface.CheckLOS(pObj->GetMapId(), loc1.x, loc1.y, loc1.z, loc2.x, loc2.y, loc2.z);
        bool res2 = CollideInterface.CheckLOS(pObj->GetMapId(), loc1.x, loc1.y, loc1.z + 2.0f, loc2.x, loc2.y, loc2.z + 2.0f);
        bool res3 = CollideInterface.CheckLOS(pObj->GetMapId(), loc1.x, loc1.y, loc1.z + 5.0f, loc2.x, loc2.y, loc2.z + 5.0f);
        SystemMessage(m_session, "Result was: %s %s %s.", res ? "in LOS" : "not in LOS", res2 ? "in LOS" : "not in LOS", res3 ? "in LOS" : "not in LOS");
        return true;
    }
    else
    {
        SystemMessage(m_session, "Collision is not enabled.");
        return true;
    }
}

bool ChatHandler::HandleGetDeathState(const char* args, WorldSession* m_session)
{
    Player* SelectedPlayer = GetSelectedPlayer(m_session, true, true);
    if (!SelectedPlayer)
        return true;

    SystemMessage(m_session, "Death State: %d", SelectedPlayer->getDeathState());
    return true;
}


bool ChatHandler::HandleCollisionGetHeight(const char* args, WorldSession* m_session)
{
    if (sWorld.Collision)
    {
        Player* plr = m_session->GetPlayer();
        float radius = 5.0f;

        float posX = plr->GetPositionX();
        float posY = plr->GetPositionY();
        float posZ = plr->GetPositionZ();
        float ori = plr->GetOrientation();

        LocationVector src(posX, posY, posZ);

        LocationVector dest(posX + (radius * (cosf(ori))), posY + (radius * (sinf(ori))), posZ);
        //LocationVector destest(posX+(radius*(cosf(ori))),posY+(radius*(sinf(ori))),posZ);


        float z = CollideInterface.GetHeight(plr->GetMapId(), posX, posY, posZ + 2.0f);
        float z2 = CollideInterface.GetHeight(plr->GetMapId(), posX, posY, posZ + 5.0f);
        float z3 = CollideInterface.GetHeight(plr->GetMapId(), posX, posY, posZ);
        float z4 = plr->GetMapMgr()->GetADTLandHeight(plr->GetPositionX(), plr->GetPositionY());
        bool fp = CollideInterface.GetFirstPoint(plr->GetMapId(), src, dest, dest, -1.5f);

        SystemMessage(m_session, "Results were: %f(offset2.0f) | %f(offset5.0f) | %f(org) | landheight:%f | target radius5 FP:%d", z, z2, z3, z4, fp);
        return true;
    }
    else
    {
        SystemMessage(m_session, "Collision is not enabled.");
        return true;
    }
}

bool ChatHandler::HandleFixScaleCommand(const char* args, WorldSession* m_session)
{
    Creature* pCreature = GetSelectedCreature(m_session, true);
    if (pCreature == NULL)
        return true;

    float sc = (float)atof(args);
    if (sc < 0.1f)
    {
        return false;
    }

    pCreature->SetScale(sc);
    const_cast<CreatureProperties*>(pCreature->GetCreatureProperties())->Scale = sc;
    WorldDatabase.Execute("UPDATE creature_proto SET scale = '%f' WHERE entry = %u", sc, pCreature->GetEntry());
    return true;
}
