/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatDefines.hpp"
#include "Chat/ChatCommandHandler.hpp"
#include "Management/HonorHandler.h"
#include "Management/ItemInterface.h"
#include "Management/ObjectMgr.hpp"
#include "Management/SkillNameMgr.h"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/InstanceMap.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Container.hpp"
#include "Objects/Item.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/Spec.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Strings.hpp"

//.character clearcooldowns
bool ChatCommandHandler::HandleCharClearCooldownsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (player_target != m_session->GetPlayer())
    {
        sGMLog.writefromsession(m_session, "Cleared all cooldowns for player %s", player_target->getName().c_str());
    }

    player_target->resetAllCooldowns();
    blueSystemMessage(m_session, "Cleared all spell cooldowns.");

    return true;
}

//.character demorph
bool ChatCommandHandler::HandleCharDeMorphCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    player_target->deMorph();

    return true;
}

//.character levelup
bool ChatCommandHandler::HandleCharLevelUpCommand(const char* args, WorldSession* m_session)
{
    uint32_t levels = atoi(args);

    if (levels == 0)
    {
        redSystemMessage(m_session, "Command must be in format: .character levelup <level>.");
        redSystemMessage(m_session, "A 0 level is not allowed.");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    sGMLog.writefromsession(m_session, "used level up command on %s, with %u levels", player_target->getName().c_str(), levels);

    levels += player_target->getLevel();

    if (levels > worldConfig.player.playerLevelCap)
        levels = worldConfig.player.playerLevelCap;

    auto level_info = sObjectMgr.getLevelInfo(player_target->getRace(), player_target->getClass(), levels);
    if (level_info == nullptr)
    {
        redSystemMessage(m_session, "No LevelInfo for Leve: {}, Race: {}, Class: {}", levels, player_target->getRace(), player_target->getClass());
        return true;
    }

    player_target->applyLevelInfo(levels);

    if (player_target != m_session->GetPlayer())
    {
        blueSystemMessage(m_session, "{} leveled up to level: {}", player_target->getName().c_str(), levels);
        blueSystemMessage(player_target->getSession(), "{} leveled you up to {}.", m_session->GetPlayer()->getName().c_str(), levels);
        sGMLog.writefromsession(m_session, "leveled player %s to level %u", player_target->getName().c_str(), levels);
    }
    else
    {
        blueSystemMessage(m_session, "You leveled yourself to {}", levels);
    }

    player_target->sendFriendStatus(true);

    return true;
}

int32_t ChatCommandHandler::getSpellIDFromLink(const char* spelllink)
{
    if (spelllink == NULL)
        return 0;

    const char* ptr = strstr(spelllink, "|Hspell:");
    if (ptr == NULL)
    {
        return 0;
    }

    return std::stoul(ptr + 8);       // spell id is just past "|Hspell:" (8 bytes)
}

uint16_t ChatCommandHandler::getItemIDFromLink(const char* itemlink, uint32_t* itemid)
{
    if (itemlink == NULL)
    {
        *itemid = 0;
        return 0;
    }
    uint16_t slen = (uint16_t)strlen(itemlink);
    const char* ptr = strstr(itemlink, "|Hitem:");
    if (ptr == NULL)
    {
        *itemid = 0;
        return slen;
    }
    ptr += 7;                       // item id is just past "|Hitem:" (7 bytes)
    *itemid = atoi(ptr);
    ptr = strstr(itemlink, "|r");   // the end of the item link
    if (ptr == NULL)                // item link was invalid
    {
        *itemid = 0;
        return slen;
    }
    ptr += 2;
    return (ptr - itemlink) & 0x0000ffff;
}

//.character unlearn
bool ChatCommandHandler::HandleCharUnlearnCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    uint32_t spell_id = std::stoul(args);
    if (spell_id == 0)
    {
        spell_id = getSpellIDFromLink(args);
        if (spell_id == 0)
        {
            redSystemMessage(m_session, "You must specify a spell id.");
            return true;
        }
    }

    sGMLog.writefromsession(m_session, "removed spell %u from %s", spell_id, player_target->getName().c_str());
    if (player_target->hasSpell(spell_id))
    {
        greenSystemMessage(player_target->getSession(), "Removed spell {}.", spell_id);
        greenSystemMessage(m_session, "Removed spell {} from {}.", spell_id, player_target->getName());
        player_target->removeSpell(spell_id, false);
    }
    else
    {
        redSystemMessage(m_session, "That player does not have spell {} learnt.", spell_id);
    }
    return true;
}

//.character learnskill
bool ChatCommandHandler::HandleCharLearnSkillCommand(const char* args, WorldSession* m_session)
{
    uint16_t skill;
    uint16_t min;
    uint16_t max;

    if (sscanf(args, "%hu %hu %hu", &skill, &min, &max) < 1)
    {
        redSystemMessage(m_session, "Command must be at least in format: .character learnskill <skillid>.");
        redSystemMessage(m_session, "Optional: .character learnskill <skillid> <min> <max>");
        return true;
    }

    if (min == 0)
        min = 1;

    if (max == 0)
        max = 1;

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (player_target->hasSkillLine(skill))
    {
        if (player_target == m_session->GetPlayer())
            redSystemMessage(m_session, "You already know this skill line");
        else
            redSystemMessage(m_session, "Player already knows this skill line");

        return true;
    }

    player_target->addSkillLine(skill, min, max);

    if (player_target == m_session->GetPlayer())
    {
        blueSystemMessage(m_session, "Adding skill line {}", skill);
    }
    else
    {
        systemMessage(player_target->getSession(), "{} taught you skill line {}.", m_session->GetPlayer()->getName(), skill);
        blueSystemMessage(m_session, "Skill line {} added to player: {}", skill, player_target->getName());
        sGMLog.writefromsession(m_session, "used add skill of %u %u %u on %s", skill, min, max, player_target->getName().c_str());
    }

    return true;
}

//.character advanceskill
bool ChatCommandHandler::HandleCharAdvanceSkillCommand(const char* args, WorldSession* m_session)
{
    uint16_t skill;
    uint16_t amount;

    if (sscanf(args, "%hu %hu", &skill, &amount) < 1)
    {
        redSystemMessage(m_session, "Command must be at least in format: .character advanceskill <skillid>.");
        redSystemMessage(m_session, "Optional: .character advanceskill <skillid> <amount>");
        return true;
    }

    if (amount == 0)
        amount = 1;

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    blueSystemMessage(m_session, "Modifying skill line {}. Advancing {} times.", skill, amount);
    sGMLog.writefromsession(m_session, "used modify skill of %u %u on %s", skill, amount, player_target->getName().c_str());

    if (!player_target->hasSkillLine(skill))
    {
        systemMessage(m_session, "Does not have skill line, adding.");
        player_target->addSkillLine(skill, amount, 0);
    }
    else
    {
        player_target->advanceSkillLine(skill, amount);
    }

    return true;
}

//.character removeskill
bool ChatCommandHandler::HandleCharRemoveSkillCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        redSystemMessage(m_session, "Command must be at least in format: .character removeskill <skillid>.");
        return true;
    }

    auto skill = static_cast<uint16_t>(std::stoul(args));
    if (skill == 0)
    {
        redSystemMessage(m_session, "%u is not a valid skill!", skill);
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (player_target->hasSkillLine(skill))
    {
        player_target->removeSkillLine(skill);

        blueSystemMessage(m_session, "Removing skill line {}", skill);
        sGMLog.writefromsession(m_session, "used remove skill of %u on %s", skill, player_target->getName().c_str());
        systemMessage(player_target->getSession(), "{} removed skill line {} from you. ", m_session->GetPlayer()->getName(), skill);
    }
    else
    {
        blueSystemMessage(m_session, "Player doesn't have skill line {}", skill);
    }
    return true;
}

//.character removeauras
bool ChatCommandHandler::HandleCharRemoveAurasCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    blueSystemMessage(m_session, "Removing all auras...");
    for (uint16_t i = AuraSlots::REMOVABLE_SLOT_START; i < AuraSlots::REMOVABLE_SLOT_END; ++i)
    {
        if (auto* const aur = player_target->getAuraWithAuraSlot(i))
            aur->removeAura();
    }

    if (player_target != m_session->GetPlayer())
        sGMLog.writefromsession(m_session, "Removed all of %s's auras.", player_target->getName().c_str());

    return true;
}

//.character removesickness
bool ChatCommandHandler::HandleCharRemoveSickessCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    player_target->removeAllAurasById(15007);

    if (player_target != m_session->GetPlayer())
    {
        blueSystemMessage(m_session, "Removed resurrection sickness from {}", player_target->getName());
        blueSystemMessage(player_target->getSession(), "{} removed your resurection sickness.", m_session->GetPlayer()->getName());
        sGMLog.writefromsession(m_session, "removed resurrection sickness from player %s", player_target->getName().c_str());
    }
    else
    {
        blueSystemMessage(m_session, "Removed resurrection sickness from you");
    }

    return true;
}

//.character learn
bool ChatCommandHandler::HandleCharLearnCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    SpellInfo const* spell_entry = nullptr;

    if (AscEmu::Util::Strings::isEqual(args, "all"))
    {
        sGMLog.writefromsession(m_session, "taught %s all spells.", selected_player->getName().c_str());
        systemMessage(m_session, "Taught {} all spells.", selected_player->getName());

        static uint32_t spellarray[DRUID + 1][512] =
        {
#if VERSION_STRING < Cata
            { 0 }, // N/A
            { 6673, 2457, 78, 100, 772, 6343, 1715, 284, 2687, 71, 6546, 7386, 355, 5242, 7384, 72, 1160, 6572, 285, 694, 2565, 676, 8198, 845, 6547, 20230, 12678, 6192, 5246, 7405, 6190, 5308, 1608, 6574, 1161, 6178, 871, 8204, 2458, 7369, 20252, 6548, 1464, 11549, 18499, 20658, 11564, 11554, 7379, 8380, 1680, 6552, 8820, 8205, 11608, 20660, 11565, 11572, 23922, 11550, 11555, 11600, 11578, 11604, 11596, 20661, 11566, 23923, 11580, 11609, 1719, 11573, 11551, 11556, 11601, 23924, 11605, 20662, 11567, 11597, 11581, 25289, 20569, 25286, 11574, 25288, 23925, 25241, 25202, 34428, 25269, 23920, 25234, 29707, 25258, 25225, 25264, 25231, 469, 25208, 2048, 25242, 25203, 25236, 30324, 3411, 30357, 30356, 46845, 47519, 47449, 47470, 47501, 47439, 47474, 55694, 47487, 47450, 47465, 47520, 47467, 47436, 47502, 47437, 47475, 47440, 47471, 57755, 57823, 47488, 674, 750, 64382, 0 }, // Warrior
            { 465, 643, 1032, 10290, 10291, 10292, 10293, 27149, 48941, 48942, 635, 21084, 19740, 20271, 498, 639, 853, 1152, 1022, 633, 19834, 53408, 7328, 19742, 647, 31789, 62124, 7294, 25780, 1044, 31785, 26573, 879, 19750, 5502, 19835, 19746, 1026, 20164, 19850, 5588, 5599, 10322, 10326, 19939, 1038, 10298, 5614, 53407, 19876, 20116, 19752, 1042, 2800, 20165, 19836, 19888, 19852, 642, 19940, 5615, 19891, 10324, 10299, 10278, 3472, 20166, 20922, 5589, 19895, 19837, 4987, 19941, 19853, 10312, 19897, 24275, 6940, 10328, 10300, 19899, 20772, 20923, 19942, 2812, 10310, 19838, 10313, 25782, 24274, 19896, 19854, 25894, 10308, 10329, 19898, 10301, 19943, 25291, 25290, 20924, 10314, 19900, 25898, 25916, 25899, 25918, 24239, 25292, 10318, 53651, 20773, 32223, 27135, 27151, 27142, 27143, 27137, 27150, 33776, 27138, 27152, 27180, 27139, 27154, 31884, 27140, 27173, 27153, 27141, 27136, 48935, 54428, 48937, 48816, 48949, 48931, 48800, 48933, 48784, 48805, 48818, 48781, 53600, 54043, 48943, 48936, 48945, 48938, 48947, 48817, 48788, 48932, 48801, 48785, 48934, 48950, 48819, 48806, 48782, 53601, 61411, 750, 20217, 53736, 33388, 0 }, // Paladin
            { 75, 2973, 1494, 13163, 1978, 3044, 1130, 5116, 14260, 13165, 883, 2641, 6991, 982, 13549, 1515, 19883, 14281, 20736, 136, 2974, 6197, 1002, 1513, 13795, 1495, 14261, 14318, 2643, 13550, 19884, 14282, 5118, 34074, 781, 1499, 3111, 14323, 3043, 1462, 14262, 19885, 14302, 3045, 13551, 19880, 14283, 14319, 13809, 3661, 13161, 5384, 14269, 14288, 14326, 1543, 14263, 19878, 13813, 13552, 14284, 14303, 3662, 3034, 14320, 13159, 14310, 14324, 14264, 19882, 1510, 14289, 13553, 14285, 14316, 13542, 14270, 20043, 14304, 14327, 14321, 14265, 13554, 56641, 19879, 14294, 14286, 13543, 14317, 14290, 20190, 14305, 14266, 14322, 14325, 14271, 13555, 14295, 14287, 25296, 19263, 14311, 13544, 25294, 25295, 19801, 27025, 34120, 27014, 27023, 34026, 27021, 27016, 27022, 27044, 27045, 27046, 34600, 27019, 34477, 36916, 49066, 53351, 48995, 49051, 49055, 49044, 49000, 61846, 48989, 49047, 58431, 61005, 53271, 49071, 53338, 49067, 48996, 49052, 49056, 49045, 49001, 61847, 60192, 61006, 48990, 53339, 49048, 58434, 62757, 674, 8737, 0 }, // Hunter
            { 2098, 1752, 1784, 53, 921, 1776, 1757, 5277, 6760, 6770, 5171, 2983, 2589, 1766, 8647, 703, 1758, 6761, 1966, 1804, 8676, 2590, 51722, 1943, 1725, 8631, 1759, 1856, 2836, 6762, 8724, 1833, 8649, 2591, 6768, 8639, 2070, 1842, 8632, 408, 1760, 8623, 8725, 2094, 8696, 8721, 8650, 8640, 8633, 8621, 8624, 8637, 1860, 11267, 6774, 1857, 11279, 11273, 11197, 11289, 17347, 11293, 11299, 11297, 11268, 26669, 8643, 11280, 11303, 11274, 11290, 11294, 11300, 11198, 11269, 17348, 11305, 11281, 25300, 31016, 25302, 11275, 26839, 32645, 26861, 26889, 26679, 26865, 27448, 27441, 31224, 26866, 26863, 26867, 32684, 48689, 48673, 26884, 26864, 5938, 26862, 51724, 48658, 48667, 48656, 57992, 48671, 48690, 48675, 57934, 48674, 48637, 48669, 48659, 48668, 48672, 48691, 48657, 57993, 51723, 48676, 48660, 48638, 0 }, // Rogue
            { 2050, 1243, 585, 2052, 589, 17, 591, 586, 139, 2053, 8092, 2006, 594, 588, 1244, 592, 528, 8122, 6074, 598, 2054, 8102, 527, 600, 970, 2944, 6346, 2061, 14914, 15237, 7128, 453, 6075, 9484, 2055, 8103, 2096, 2010, 984, 15262, 8129, 1245, 3747, 9472, 6076, 992, 19276, 6063, 15430, 8104, 8124, 15263, 602, 605, 6065, 596, 976, 1004, 552, 9473, 6077, 6064, 1706, 8105, 10880, 2767, 19277, 988, 15264, 15431, 2791, 6066, 9474, 6078, 6060, 2060, 1006, 8106, 996, 9485, 15265, 10898, 10888, 10957, 10892, 19278, 10915, 27799, 10909, 10927, 10963, 10945, 10881, 10933, 15266, 10937, 10899, 21562, 10916, 10951, 10960, 10928, 10893, 19279, 10964, 27800, 10946, 15267, 10900, 10934, 10917, 27683, 10890, 10929, 10958, 10965, 10947, 20770, 10894, 19280, 25314, 15261, 27801, 60931, 10952, 10938, 10901, 21564, 10961, 25316, 27681, 25315, 10955, 25233, 25363, 32379, 25210, 25372, 32546, 25217, 25221, 25367, 25384, 34433, 25235, 25467, 25213, 25331, 25308, 33076, 25435, 25433, 25431, 25375, 25364, 32375, 25389, 25218, 25392, 39374, 32999, 25222, 32996, 25368, 48040, 48119, 48134, 48299, 48070, 48062, 48126, 48112, 48122, 48075, 48077, 48045, 48065, 48067, 48157, 48124, 48072, 48169, 48168, 48170, 48120, 48063, 48135, 48171, 48300, 48071, 48127, 48113, 48123, 47951, 48078, 53023, 48161, 48066, 48162, 48074, 48068, 48158, 48125, 14752, 14818, 14819, 27841, 25312, 48073, 64843, 64901, 0 }, // Priest
            { 53341, 53331, 53343, 54447, 53342, 54446, 53323, 53344, 62158, 48778, 48266, 45902, 52375, 50977, 49576, 49142, 45477, 45462, 53428, 49998, 50842, 46584, 48263, 47528, 48721, 45524, 49926, 47476, 51325, 43265, 49917, 49896, 49020, 3714, 49892, 48792, 51426, 49999, 49927, 45529, 56222, 57330, 49918, 49913, 49939, 48743, 49936, 49903, 51423, 56815, 55265, 48707, 49893, 51427, 49928, 49914, 51326, 45463, 49919, 48265, 49940, 61999, 49937, 49904, 51424, 55270, 49929, 51428, 49915, 51327, 49923, 47568, 57623, 49920, 49894, 49941, 49909, 51425, 51429, 55271, 49916, 42650, 49930, 51328, 49938, 49895, 49924, 49921, 70164, 0 }, // Death Knight
            { 30669, 30670, 30671, 331, 403, 8017, 8042, 8071, 2484, 332, 8044, 529, 324, 8018, 5730, 8050, 8024, 3599, 8075, 2008, 1535, 547, 370, 8045, 548, 8154, 526, 325, 8019, 57994, 8052, 8027, 913, 6390, 8143, 8056, 8033, 2645, 5394, 8004, 915, 6363, 52127, 2870, 8498, 8166, 131, 20609, 8046, 8181, 939, 905, 10399, 8155, 8160, 6196, 8030, 943, 8190, 5675, 8184, 8053, 8227, 8038, 8008, 6391, 52129, 546, 556, 51730, 8177, 6375, 10595, 20608, 6364, 36936, 8232, 421, 8499, 959, 6041, 945, 8012, 8512, 8058, 6495, 10406, 52131, 20610, 10412, 16339, 8010, 10585, 10495, 8170, 8249, 10478, 10456, 10391, 6392, 8161, 1064, 930, 51988, 10447, 6377, 8005, 8134, 6365, 8235, 52134, 11314, 10537, 10466, 10392, 10600, 10407, 10622, 16341, 10472, 10586, 10496, 20776, 2860, 10413, 10526, 16355, 10395, 10431, 10427, 52136, 51991, 10462, 15207, 10437, 10486, 11315, 10448, 10467, 10442, 10623, 10479, 10408, 52138, 10605, 16342, 10396, 15208, 10432, 10587, 10497, 10538, 16387, 10473, 16356, 10428, 20777, 10414, 51992, 29228, 10463, 25357, 10468, 10601, 10438, 25361, 16362, 25422, 25546, 25448, 24398, 25439, 25391, 25469, 25508, 25489, 3738, 25552, 25570, 25528, 2062, 25500, 25420, 25557, 25560, 25449, 25525, 25423, 2894, 25563, 25464, 25505, 25590, 25454, 25567, 25574, 25533, 33736, 25442, 51993, 25547, 25457, 25396, 25472, 25509, 58649, 58785, 58794, 58755, 58771, 58699, 58580, 58801, 49275, 49235, 49237, 58731, 58751, 55458, 49270, 49230, 61649, 58737, 49232, 58652, 58741, 49272, 51505, 49280, 58746, 58703, 58581, 57622, 58789, 58795, 58756, 58773, 57960, 58803, 49276, 49236, 58734, 58582, 58753, 49231, 49238, 49277, 55459, 49271, 51994, 61657, 58739, 49233, 58656, 58790, 58745, 58796, 58757, 49273, 51514, 60043, 49281, 58774, 58749, 58704, 58643, 58804, 66842, 66843, 66844, 8737, 0 }, // Shaman
            { 1459, 133, 168, 5504, 116, 587, 2136, 143, 5143, 205, 118, 5505, 7300, 122, 597, 604, 145, 130, 1449, 1460, 2137, 837, 5144, 2120, 1008, 3140, 475, 1953, 10, 5506, 12051, 543, 54648, 7301, 7322, 1463, 12824, 8437, 990, 2138, 6143, 2948, 5145, 2139, 8450, 8400, 2121, 120, 865, 8406, 1461, 6141, 759, 8494, 8444, 8455, 8438, 6127, 8412, 8457, 8401, 7302, 45438, 8416, 6129, 8422, 8461, 8407, 8492, 6117, 8445, 8427, 8451, 8402, 8495, 8439, 3552, 8413, 8408, 8417, 10138, 8458, 8423, 6131, 7320, 12825, 8446, 10169, 10156, 10159, 10144, 10148, 8462, 10185, 10179, 10191, 10201, 10197, 13031, 22782, 10205, 10211, 10053, 10173, 10149, 10215, 10160, 10139, 10223, 10180, 10219, 10186, 10145, 10177, 13032, 10192, 10206, 10170, 10202, 10199, 10150, 10230, 23028, 10157, 10212, 33041, 10216, 10181, 10161, 10054, 13033, 22783, 10207, 25345, 10187, 28612, 10140, 10174, 10225, 10151, 28609, 25304, 10220, 10193, 61305, 28272, 12826, 61025, 28271, 27078, 27080, 25306, 30482, 27130, 27075, 27071, 30451, 33042, 27086, 27134, 27087, 37420, 27073, 27070, 30455, 33944, 27088, 27085, 27101, 66, 27131, 33946, 38699, 27128, 27072, 27124, 27125, 27127, 27082, 27126, 38704, 33717, 27090, 33043, 27079, 38692, 32796, 38697, 33405, 43987, 27074, 30449, 42894, 43023, 43045, 53140, 42930, 42925, 42913, 43019, 42858, 42939, 42872, 42832, 53142, 42843, 42955, 42949, 42917, 42841, 44614, 43038, 42896, 42920, 43015, 43017, 42985, 43010, 42833, 42914, 42859, 42846, 42931, 42926, 43012, 42842, 43008, 43024, 43020, 43046, 42897, 43002, 42921, 42995, 42940, 42956, 61316, 61024, 42950, 42873, 47610, 43039, 55342, 58659, 53142, 0 }, // Mage
            { 59671, 687, 348, 686, 688, 172, 702, 1454, 695, 980, 5782, 6201, 696, 1120, 707, 697, 1108, 755, 705, 6222, 704, 689, 1455, 5697, 693, 1014, 5676, 706, 3698, 1094, 5740, 698, 1088, 712, 6202, 6205, 699, 126, 6223, 5138, 8288, 5500, 1714, 132, 1456, 17919, 710, 6366, 6217, 7658, 3699, 1106, 20752, 1086, 709, 1098, 5784, 1949, 2941, 691, 1490, 7646, 6213, 6229, 7648, 5699, 6219, 17920, 17951, 2362, 3700, 11687, 7641, 11711, 7651, 8289, 20755, 11733, 5484, 11665, 7659, 11707, 6789, 11683, 17921, 11739, 11671, 11725, 11693, 11659, 17952, 11729, 11721, 11699, 11688, 11677, 18647, 17727, 11712, 6353, 20756, 11719, 17925, 11734, 11667, 1122, 17922, 11708, 11675, 11694, 11660, 11740, 11672, 11700, 11684, 17928, 17953, 11717, 6215, 11689, 17924, 11730, 11713, 17926, 11726, 11678, 17923, 25311, 20757, 17728, 603, 11722, 11735, 54785, 11695, 11668, 25309, 50589, 18540, 11661, 50581, 28610, 27224, 23161, 27219, 28176, 25307, 29722, 27211, 27216, 27210, 27250, 28172, 29858, 27218, 27217, 27259, 27230, 27223, 27213, 27222, 29893, 27226, 27228, 30909, 27220, 28189, 27215, 27212, 27209, 27238, 30910, 27260, 32231, 30459, 27243, 30545, 47812, 50511, 47886, 61191, 47819, 47890, 47871, 47863, 47859, 60219, 47892, 47837, 47814, 47808, 47810, 47835, 47897, 47824, 47884, 47793, 47856, 47813, 47855, 47888, 47865, 47860, 47857, 47823, 47891, 47878, 47864, 57595, 47893, 47820, 47815, 47809, 60220, 47867, 47889, 48018, 48020, 47811, 47838, 57946, 58887, 47836, 61290, 47825, 0 }, // Warlock
            { 0 }, // N/A
            { 5185, 1126, 5176, 8921, 774, 467, 5177, 339, 5186, 5487, 99, 6795, 5232, 6807, 8924, 16689, 1058, 18960, 5229, 8936, 50769, 5211, 8946, 5187, 782, 5178, 1066, 8925, 1430, 779, 1062, 770, 16857, 2637, 6808, 16810, 8938, 768, 1082, 1735, 5188, 6756, 5215, 20484, 1079, 2912, 8926, 2090, 5221, 2908, 5179, 1822, 8939, 2782, 50768, 780, 1075, 5217, 2893, 1850, 5189, 6809, 8949, 5209, 3029, 8998, 5195, 8927, 16811, 2091, 9492, 6798, 778, 17390, 5234, 20739, 8940, 6800, 740, 783, 5180, 9490, 22568, 6778, 6785, 5225, 8972, 8928, 1823, 3627, 8950, 769, 8914, 22842, 9005, 8941, 50767, 9493, 6793, 5201, 5196, 8903, 18657, 16812, 8992, 8955, 6780, 9000, 20719, 22827, 16914, 29166, 8907, 8929, 20742, 8910, 8918, 9747, 9749, 17391, 9745, 6787, 9750, 8951, 22812, 9758, 1824, 9752, 9754, 9756, 8983, 9821, 9833, 9823, 9839, 9829, 8905, 9849, 9852, 22828, 16813, 9856, 50766, 9845, 21849, 9888, 17401, 9884, 9880, 9866, 20747, 9875, 9862, 9892, 9898, 9834, 9840, 9894, 9907, 17392, 9904, 9857, 9830, 9901, 9908, 9910, 9912, 22829, 9889, 9827, 9850, 9853, 18658, 33986, 33982, 9881, 9835, 17329, 9867, 9841, 9876, 31709, 31018, 21850, 25297, 17402, 9885, 20748, 9858, 25299, 50765, 9896, 25298, 9846, 9863, 53223, 27001, 26984, 26998, 26978, 22570, 24248, 26987, 26981, 33763, 27003, 26997, 26992, 33357, 26980, 26993, 27011, 33745, 27006, 27005, 27000, 26996, 27008, 26986, 26989, 33943, 33987, 33983, 27009, 27004, 26979, 26994, 26982, 50764, 26985, 33786, 26991, 27012, 26990, 26988, 27002, 26995, 26983, 53225, 48559, 48442, 49799, 40120, 62078, 50212, 48576, 48450, 48573, 48464, 48561, 48569, 48567, 48479, 48578, 48377, 49802, 53307, 48459, 48563, 48565, 48462, 48440, 52610, 48571, 48446, 53226, 48575, 48476, 48475, 48560, 49803, 48443, 48562, 53308, 48577, 53312, 48574, 48465, 48570, 48378, 48480, 48579, 48477, 50213, 48461, 48470, 48467, 48568, 48451, 48564, 48566, 48469, 48463, 50464, 48441, 50763, 49800, 48572, 48447, 53227, 61384, 62600, 0 }, // Druid
#else
            { 0 }, // N/A
            { 750, 84614, 84615, 12809, 71, 7376, 20243, 676, 49410, 3411, 12975, 3127, 6572, 2565, 23922, 871, 46968, 23920, 12678, 7386, 355, 50720, 6673, 18499, 2458, 7381, 23881, 23885, 23880, 1161, 845, 469, 12292, 1160, 55694, 5308, 60970, 6544, /*52174,*/ 1134, 20252, /*20253,*/ 5246, 12323, 6552, 85288, 96103, 85384, 97462, 1719, 1464, 34428, 1680, 2457, 21156, 46924, 100, 86346, 1715, 78, 57755, 12294, 76858, 7384, 772, 94009, 20230, 64382, 64380, 88161, 12328, 85388, 6343, 86526, 72, 86479, 0 }, // Warrior
            { 750, 53563, 53652, 4987, 19746, 26573, 54968, 82326, 54428, 879, 19750, 635, 64891, 82327, 20473, 25912, 25914, 2812, 20187, 633, 53651, 7328, 20165, 10326, 85673, 53600, 26017, 20154, 82242, 19891, 31789, 25780, 20911, 84628, 84629, 20164, 86150, 20217, 465, 498, 642, 96231, 20271, 31804, 96231, 20066, 7294, 85285, 84963, 85256, 85696, 31801, 86525, 53385, 24275, 35395, 31935, 49410, 75806, 85043, 86150, 853, 53595, 1044, 1022, 62124, 6940, 1038, 20925, 31884, 19740, 31803, 32223, 86474, 0 }, // Paladin
            { 883, 83242, 83243, 83244, 83245, 82661, 82243, 1978, 34600, 82948, 1494, 19878, 19879, 19880, 19882, 19885, 19883, 19884, 19503, 19306, 19263, 781, 53301, 13813, 13812, 82939, 5384, 1499, 60192, 13809, 82941, 13795, 13797, 82945, 34477, 77767, 51755, 51753, 82654, 93321, 79682, 86100, 2974, 19386, 24131, 3674, 19434, 82928, 3044, 75, 53209, 5116, 20736, 1543, 1130, 53351, 2643, 3045, 34490, 56641, 19801, 19506, 53254, 5118, 13165, 13159, 20043, 1462, 19574, 2641, 6197, 6991, 24406, 19577, 34026, 53271, 136, 982, 1513, 1515, 13481, 34471, 34692, 77769, 674, 86528, 2973, 9077, 8737, 86472, 0 }, // Hunter
            { 1804, 2094, 31224, 2836, 1842, 1725, 16511, 921, 14183, 1784, 57934, 57933, 1856, 53, 74001, 5277, 51723, 1966, 1776, 1766, 76577, 1860, 6770, 51713, 36554, 73981, 84617, 5938, 1752, 79327, 2983, 8676, 1833, 26679, 51722, 32645, 2098, 8647, 703, 408, 1329, /*5374, 27576,*/ 73651, 82245, 74001, 1943, 5171, 79140, 79140, 2842, 86092, 86531, 86476, 0 }, // Rogue
            { 724, 7001, 596, 33076, 33110, 139, 2006, 585, 20711, 453, 589, 586, 528, 605, 34433, 15286, 34914, 34919, 32546, 70772, 34861, 2096, 19236, 64844, /*63544,*/ 87151, 527, 6346, 56131, 56160, 84733, 588, 2061, 101062, 56161, 2060, 47788, 48153, 2050, 14914, 84733, 15237, 23455, 64904, 88625, 88625, 88685, 88684, 8122, 27683, 32379, 15407, 48045, 49821, 8092, 2944, 47585, 1706, 8129, 32375, 47540, 47666, 47750, 47758, 47757, 10060, 62618, 21562, 17, 9484, 73413, 64901, 64843, 73325, 73510, 89745, 86475, 0 }, // Priest
            { 48778, 48707, 51052, 42650, 63560, 82246, 50842, 48982, 47476, 77606, 86524, 85948, 81229, 77575, 50977, 43265, 49224, 49610, 49611, 45462, 52373, 61999, 46584, 55090, 49206, 49194, 50536, 49016, 48265, 45524, 47568, 48266, 49143, 57330, 49184, 49203, 48792, 50887, 45477, 52372, 55610, 49039, 47528, 49020, 3714, 56815, 56816, 81229, 49182, 49500, 49501, 48721, 48263, 45902, 52374, 45529, 49028, 56222, 48743, 49998, 45470, 49410, 55050, 57532, 86061, 73975, 47632, 47633, 52375, 47541, 53428, 86471, 0 }, // Death Knight
            { 8737, 131, 331, 324, 370, 403, 421, 546, 556, 974, 51730, 55533, 26364, 8512, 8232, 3738, 73899, 8017, 30823, 8071, 32175, 32176, 8075, 73680, 86629, 2062, 51523, 51524, 8184, 6196, 51533, 8227, 8024, 8033, 2645, 8177, 60103, 3599, 45284, 8190, 45297, 5675, 16190, 20608, 21169, 61295, 98008, 36936, 8143, 52127, 8042, 5730, 51490, 57994, 2008, 1064, 70809, 2484, 61882, 77478, 2894, 1535, 8349, 8050, 8056, 51514, 51505, 77451, 86529, 66844, 66842, 66843, 76780, 79206, 73899, 73680, 51886, 5394, 8004, 77472, 73920, 87718, 86477, 0 }, // Shaman
            { 10, 66, 120, 122, 116, 118, 130, 475, 543, 1463, 55342, 84721, 28271, 43987, 30449, 31589, 28272, 61025, 61305, 61721, 61780, 61780, 95969, 11426, 45438, 1449, 5143, 7268, 1953, 42955, 2139, 61316, 6117, 12051, 54646, 54648, 30455, 11113, 30482, 34913, 11366, 92315, 2948, 759, 44425, 30451, 1459, 11129, 31661, 2136, 2120, /*88148,*/ 44614, 12654, 12355, 44457, 44461, 12472, 44572, /*71757,*/ 7302, 42208, 12484, 12485, 11958, 89744, 80353, 82731, 82691, 82676, 86473, 0 }, // Mage
            { 126, 172, 348, 689, 698, 702, 980, 710, 755, 603, 687, 693, 1490, 1714, 6789, 6229, 29858, 18540, 30146, 691, 688, 1122, 712, 697, 5697, 1098, 85403, 19483, 20153, /*50590,*/ 50589, /*22703,*/ 29893, 28176, 5784, 71521, 6201, 29886, 54785, 54786, 48018, 48020, 47193, 23161, 89420, 1120, 5782, 50796, 17962, 29722, 91711, 5740, 42223, 5676, 47897, /*47960,*/ 30283, 6353, 1949, 5857, 48181, 5484, 1454, 18094, 18095, 27243, 27285, 86121, 86213, 30108, 91986, 85112, 80240, 18662, 80398, 93375, 93376, 77801, 77799, 79268, 19028, 74434, 86091, 86478, 0 }, // Warlock
            { 0 }, // N/A
            { 99, 339, 770, 774, 740, 783, 779, 33831, 24858, 24905, 16979, 49376, 80861, 33878, 33876, 84736, 24907, 33891, 50516, 61391, 48438, 50334, 81283, 81291, 62078, 5217, 5221, 78675, 80313, 80951, 48505, 50288, 78674, 93402, 61336, 5225, 1079, 40120, 52610, 2912, 467, 6785, 81170, 9005, 18960, 2908, 64801, 8936, 50769, 20484, 2782, 44203, 1066, 5421, 1178, 8998, 1850, 5229, 20719, 16857, 60089, 5209, 5211, 33943, 22842, 6807, 1126, 50464, 33745, 5185, 33763, 22570, 16914, 42231, 2637, 33786, 22812, 29166, 16689, 768, 1082, 5487, 22568, 1822, 5215, 6795, 8921, 86530, 16864, 80964, 80965, 88747, 88751, 77764, 62600, 77758, 77761, 5570, 86470, 0 }, // Druid
#endif
        };

        uint8_t player_class = selected_player->getClass();
        for (uint32_t i = 0; spellarray[player_class][i] != 0; ++i)
        {
            spell_entry = sSpellMgr.getSpellInfo(spellarray[player_class][i]);
            if (spell_entry == nullptr)
                continue;

            if (selected_player->hasSpell(spellarray[player_class][i]))
                continue;

            selected_player->addSpell(spellarray[player_class][i]);
        }

#if VERSION_STRING < Cata
        static uint32_t paladinspellarray[DBC_NUM_RACES][5] =
        {
            { 0 },                                  // RACE 0
            { 23214, 13819, 31801, 53720, 0 },      // HUMAN  Charger, Warhorse, Seal of Vengeance, Seal of Martyr
            { 0 },                                  // ORC
            { 23214, 13819, 31801, 53720, 0 },      // DWARF  Charger, Warhorse, Seal of Vengeance, Seal of Martyr
            { 0 },                                  // NIGHTELF
            { 0 },                                  // UNDEAD
            { 0 },                                  // TAUREN
            { 0 },                                  // GNOME
            { 0 },                                  // TROLL
#if VERSION_STRING > Classic
            { 34767, 34769, 31892, 53736, 0 },      // BLOODELF  Summon Charger, Summon Warhorse, Seal of Blood, Seal of Corruption
            { 23214, 13819, 31801, 53720, 0 },      // DRAENEI  Charger, Warhorse, Seal of Vengeance, Seal of Martyr
#endif
        };
#else
        static uint32_t paladinspellarray[DBC_NUM_RACES][3] =
        {
            { 0 },                  // RACE 0
            { 13819, 23214, 0 },    // HUMAN  Summon Warhorse, Summon Charger
            { 0 },                  // ORC
            { 13819, 23214, 0 },    // DWARF  Summon Warhorse, Summon Charger
            { 0 },                  // NIGHTELF
            { 0 },                  // UNDEAD
            { 69820, 69826, 0 },    // TAUREN Summon Sunwalker Kodo, Summon Great Sunwalker Kodo
            { 0 },                  // GNOME
            { 0 },                  // TROLL
            { 0 },                  // GOBLIN
            { 34767, 34769, 0 },    // BLOODELF  Summon Thalassian Charger, Summon Thalassian Warhorse
            { 73629, 73630, 0 },    // DRAENEI  Summon Exarch's Elekk, Summon Great Exarch's Elekk
            { 0 },                  // RACE 12
            { 0 },                  // RACE 13
            { 0 },                  // RACE 14
            { 0 },                  // RACE 15
            { 0 },                  // RACE 16
            { 0 },                  // RACE 17
            { 0 },                  // RACE 18
            { 0 },                  // RACE 19
            { 0 },                  // RACE 20
            { 0 },                  // RACE 21
            { 0 },                  // WORGEN
        };
#endif

        static uint32_t shamanspellarray[DBC_NUM_RACES][2] =
        {
#if VERSION_STRING < Cata
            { 0 },                                  // RACE 0
            { 0 },                                  // HUMAN
            { 2825, 0 },                            // ORC Bloodlust
            { 0 },                                  // DWARF
            { 0 },                                  // NIGHTELF
            { 0 },                                  // UNDEAD
            { 2825, 0 },                            // TAUREN Bloodlust
            { 0 },                                  // GNOME
            { 2825, 0 },                            // TROLL Bloodlust
#if VERSION_STRING > Classic
            { 0 },                                  // BLOODELF
            { 32182, 0 },                           // DRAENEI Heroism
#endif
#else
            { 0 },                                  // RACE 0
            { 0 },                                  // HUMAN
            { 2825, 0 },                            // ORC Bloodlust
            { 0 },                                  // DWARF
            { 0 },                                  // NIGHTELF
            { 0 },                                  // UNDEAD
            { 2825, 0 },                            // TAUREN Bloodlust
            { 0 },                                  // GNOME
            { 2825, 0 },                            // TROLL Bloodlust
            { 0 },                                  // BLOODELF
            { 32182, 0 },                           // DRAENEI Heroism
            { 0 },                                  // RACE 12
            { 0 },                                  // RACE 13
            { 0 },                                  // RACE 14
            { 0 },                                  // RACE 15
            { 0 },                                  // RACE 16
            { 0 },                                  // RACE 17
            { 0 },                                  // RACE 18
            { 0 },                                  // RACE 19
            { 0 },                                  // RACE 20
            { 0 },                                  // RACE 21
            { 0 },                                  // WORGEN
#endif
        };

#if VERSION_STRING < Cata
        static uint32_t magespellarray[DBC_NUM_RACES][13] =
        {
            { 0 },                                                                                  // RACE 0
            { 3561, 3562, 3565, 10059, 11416, 11419, 32266, 32271, 33690, 33691, 49359, 49360, 0 }, // HUMAN
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                                              // ORC
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                                              // DWARF
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                                              // NIGHTELF
            { 3563, 3566, 3567, 11417, 11418, 11420, 32267, 32272, 35715, 35717, 49358, 49361, 0 }, // UNDEAD
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                                              // TAUREN
            { 3561, 3562, 3565, 10059, 11416, 11419, 32266, 32271, 33690, 33691, 49359, 49360, 0 }, // GNOME
            { 3563, 3566, 3567, 11417, 11418, 11420, 32267, 32272, 35715, 35717, 49358, 49361, 0 }, // TROLL
#if VERSION_STRING > Classic
            { 3563, 3566, 3567, 11417, 11418, 11420, 32267, 32272, 35715, 35717, 49358, 49361, 0 }, // BLOODELF
            { 3561, 3562, 3565, 10059, 11416, 11419, 32266, 32271, 33690, 33691, 49359, 49360, 0 }, // DRAENEI
#endif
        };
#else
        static uint32_t magespellarray[DBC_NUM_RACES][17] =
        {
            { 0 },      // RACE 0
            { 3561, 3562, 3565, 10059, 11416, 11419, 32266, 32271, 53142, 33690, 33691, 49360, 49359, 53140, 88342, 88345, 0 },       // HUMAN
            { 3563, 3566, 3567, 11417, 11418, 11420, 32267, 32272, 53142, 35715, 35717, 49361, 49358, 53140, 88344, 88346, 0 },       // ORC
            { 3561, 3562, 3565, 10059, 11416, 11419, 32266, 32271, 53142, 33690, 33691, 49360, 49359, 53140, 88342, 88345, 0 },       // DWARF
            { 3561, 3562, 3565, 10059, 11416, 11419, 32266, 32271, 53142, 33690, 33691, 49360, 49359, 53140, 88342, 88345, 0 },       // NIGHTELF
            { 3563, 3566, 3567, 11417, 11418, 11420, 32267, 32272, 53142, 35715, 35717, 49361, 49358, 53140, 88344, 88346, 0 },       // UNDEAD
            { 0 },       // TAUREN
            { 3561, 3562, 3565, 10059, 11416, 11419, 32266, 32271, 53142, 33690, 33691, 49360, 49359, 53140, 88342, 88345, 0 },       // GNOME
            { 3563, 3566, 3567, 11417, 11418, 11420, 32267, 32272, 53142, 35715, 35717, 49361, 49358, 53140, 88344, 88346, 0 },       // TROLL
            { 3563, 3566, 3567, 11417, 11418, 11420, 32267, 32272, 53142, 35715, 35717, 49361, 49358, 53140, 88344, 88346, 0 },       // GOBLIN
            { 3563, 3566, 3567, 11417, 11418, 11420, 32267, 32272, 53142, 35715, 35717, 49361, 49358, 53140, 88344, 88346, 0 },       // BLOODELF
            { 3561, 3562, 3565, 10059, 11416, 11419, 32266, 32271, 53142, 33690, 33691, 49360, 49359, 53140, 88342, 88345, 0 },       // DRAENEI
            { 0 },       // RACE 12
            { 0 },       // RACE 13
            { 0 },       // RACE 14
            { 0 },       // RACE 15
            { 0 },       // RACE 16
            { 0 },       // RACE 17
            { 0 },       // RACE 18
            { 0 },       // RACE 19
            { 0 },       // RACE 20
            { 0 },       // RACE 21
            { 3561, 3562, 3565, 10059, 11416, 11419, 32266, 32271, 53142, 33690, 33691, 49360, 49359, 53140, 88342, 88345, 0 },       // WORGEN
        };
#endif

        uint8_t player_race = selected_player->getRace();
        switch (player_class)
        {
            case PALADIN:
                for (uint32_t i = 0; paladinspellarray[player_race][i] != 0; ++i)
                {
                    spell_entry = sSpellMgr.getSpellInfo(paladinspellarray[player_race][i]);
                    if (spell_entry == nullptr)
                        continue;

                    if (selected_player->hasSpell(paladinspellarray[player_race][i]))
                        continue;

                    selected_player->addSpell(paladinspellarray[player_race][i]);
                }
                break;
            case MAGE:
                for (uint32_t i = 0; magespellarray[player_race][i] != 0; ++i)
                {
                    spell_entry = sSpellMgr.getSpellInfo(magespellarray[player_race][i]);
                    if (spell_entry == nullptr)
                        continue;

                    if (selected_player->hasSpell(magespellarray[player_race][i]))
                        continue;

                    selected_player->addSpell(magespellarray[player_race][i]);
                }
                break;
            case SHAMAN:
                for (uint32_t i = 0; shamanspellarray[player_race][i] != 0; ++i)
                {
                    spell_entry = sSpellMgr.getSpellInfo(shamanspellarray[player_race][i]);
                    if (spell_entry == nullptr)
                        continue;

                    if (selected_player->hasSpell(shamanspellarray[player_race][i]))
                        continue;

                    selected_player->addSpell(shamanspellarray[player_race][i]);
                }
                break;
        }
        return true;
    }

    uint32_t spell = std::stoul(args);
    if (spell == 0)
    {
        spell = getSpellIDFromLink(args);
    }

    spell_entry = sSpellMgr.getSpellInfo(spell);
    if (spell_entry == nullptr)
    {
        systemMessage(m_session, "Invalid spell {}", spell);
        return true;
    }

    if (!selected_player->getSession()->HasGMPermissions() && (spell_entry->getEffect(0) == SPELL_EFFECT_INSTANT_KILL || spell_entry->getEffect(1) == SPELL_EFFECT_INSTANT_KILL || spell_entry->getEffect(2) == SPELL_EFFECT_INSTANT_KILL))
    {
        systemMessage(m_session, "don't be an idiot and teach players instakill spells. this action has been logged.");
        sGMLog.writefromsession(m_session, "is an idiot and tried to tought player %s instakill spell %u", selected_player->getName().c_str(), spell);
        return true;
    }

    if (selected_player->hasSpell(spell))
    {
        systemMessage(m_session, "{} already knows that spell.", selected_player->getName());
        return true;
    }

    selected_player->addSpell(spell);

    sGMLog.writefromsession(m_session, "Taught %s spell %u", selected_player->getName().c_str(), spell);
    blueSystemMessage(selected_player->getSession(), "{} taught you Spell {}", m_session->GetPlayer()->getName(), spell);
    greenSystemMessage(m_session, "Taught {} Spell {}", selected_player->getName(), spell);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .character add commands
//.character add honorpoints
bool ChatCommandHandler::HandleCharAddHonorPointsCommand(const char* args, WorldSession* m_session)
{
    uint32_t honor_amount = args ? std::stoul(args) : 1;

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    blueSystemMessage(m_session, "{} honor points added to Player {}.", honor_amount, player_target->getName());
    greenSystemMessage(player_target->getSession(), "{} added %u honor points to your character.", m_session->GetPlayer()->getName(), honor_amount);
    sGMLog.writefromsession(m_session, "added %u honor points to character %s", honor_amount, player_target->getName().c_str());

    HonorHandler::AddHonorPointsToPlayer(player_target, honor_amount);

    return true;
}


//.character add honorkill
bool ChatCommandHandler::HandleCharAddHonorKillCommand(const char* args, WorldSession* m_session)
{
    uint32_t kill_amount = args ? std::stoul(args) : 1;
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    blueSystemMessage(m_session, "{} honor kill points added to Player {}.", kill_amount, player_target->getName());
    greenSystemMessage(player_target->getSession(), "{} added %u honor kill points to your character.", m_session->GetPlayer()->getName(), kill_amount);
    sGMLog.writefromsession(m_session, "added %u honor kill points to character %s", kill_amount, player_target->getName().c_str());

    player_target->incrementKills(kill_amount);

#if VERSION_STRING != Classic
    player_target->setFieldKills(uint32_t(player_target->getKillsToday() | (player_target->getKillsYesterday() << 16)));
    player_target->setLifetimeHonorableKills(player_target->getKillsLifetime());
#endif

    return true;
}

//.character add item
bool ChatCommandHandler::HandleCharAddItemCommand(const char* args, WorldSession* m_session)
{
    uint32_t itemid = 0;
    uint32_t count = 1;
    int32_t randomprop = 0;
    int32_t numadded = 0;

    // check for item link
    uint16_t ofs = getItemIDFromLink(args, &itemid);

    if (itemid)
    {
        sscanf(args + ofs, "%u %d", &count, &randomprop); // these may be empty
    }
    else if (sscanf(args, "%u %u %d", &itemid, &count, &randomprop) < 1)
    {
        redSystemMessage(m_session, "Command must be at least in format: .character add item <itemID>.");
        redSystemMessage(m_session, "Optional: .character add item <itemID> <amount> <randomprop>");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    auto item_proto = sMySQLStore.getItemProperties(itemid);
    if (item_proto != nullptr)
    {
        numadded -= player_target->getItemInterface()->GetItemCount(itemid);
        bool result = player_target->getItemInterface()->AddItemById(itemid, count, randomprop);
        numadded += player_target->getItemInterface()->GetItemCount(itemid);
        if (result == true)
        {
            if (count == 0)
            {
                sGMLog.writefromsession(m_session, "used add item command, item id %u [%s], quantity %u, to %s", item_proto->ItemId, item_proto->Name.c_str(), numadded, player_target->getName().c_str());
            }
            else
            {
                sGMLog.writefromsession(m_session, "used add item command, item id %u [%s], quantity %u (only %i added due to full inventory), to %s", item_proto->ItemId, item_proto->Name.c_str(), count, numadded, player_target->getName().c_str());
            }

            systemMessage(m_session, "Added item {} (id: {}), quantity {}, to {}'s inventory.", sMySQLStore.getItemLinkByProto(item_proto, m_session->language), item_proto->ItemId, numadded, player_target->getName());
            systemMessage(player_target->getSession(), "{} added item {}, quantity {}, to your inventory.", m_session->GetPlayer()->getName(), sMySQLStore.getItemLinkByProto(item_proto, player_target->getSession()->language), numadded);
        }
        else
        {
            systemMessage(player_target->getSession(), "Failed to add item.");
        }
        return true;
    }

    redSystemMessage(m_session, "Item {} is not a valid item!", itemid);
    return true;
}

//.character add itemset
bool ChatCommandHandler::HandleCharAddItemSetCommand(const char* args, WorldSession* m_session)
{
    int32_t setid = atoi(args);
    if (!setid)
    {
        redSystemMessage(m_session, "You must specify a setid.");
        return true;
    }

    auto player = GetSelectedPlayer(m_session, true, true);
    if (player == nullptr)
        return true;

    /*auto item_set_list = sObjectMgr.GetListForItemSet(setid);
    if (!item_set_list)
    {
        redSystemMessage(m_session, "Invalid item set.");
        return true;
    }*/

    blueSystemMessage(m_session, "Searching item set {}...", setid);
    sGMLog.writefromsession(m_session, "used add item set command, set %u, target %s", setid, player->getName().c_str());

    uint32_t itemset_items_count = 0;

    MySQLDataStore::ItemPropertiesContainer const* its = sMySQLStore.getItemPropertiesStore();
    for (MySQLDataStore::ItemPropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        ItemProperties const* it = sMySQLStore.getItemProperties(itr->second.ItemId);
        if (it == nullptr)
            continue;

        if (it->ItemSet != setid)
            continue;

        if (!player->getItemInterface()->AddItemById(it->ItemId, 1, 0))
        {
            m_session->SendNotification("No free slots left!");
            return true;
        }

        systemMessage(m_session, "Added item: {} [{}]", it->Name, it->ItemId);
        ++itemset_items_count;
    }

    if (itemset_items_count > 0)
        greenSystemMessage(m_session, "Added set to inventory complete.");
    else
        redSystemMessage(m_session, "Itemset ID: {} is not defined in tems table!", setid);

    return true;
}

//.character add copper
bool ChatCommandHandler::HandleCharAddCopperCommand(const char* args, WorldSession* m_session)
{
    if (*args == 0)
    {
        redSystemMessage(m_session, "You must specify how many copper you will add.");
        redSystemMessage(m_session, "10000 = 1 gold, 1000 = 1 silver, 1 = 1 copper.");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    int32_t total = atoi(args);

    uint32_t gold = (uint32_t)std::floor(Util::int32abs<float>(total) / 10000.0f);
    uint32_t silver = (uint32_t)std::floor(Util::int32abs<float>(total) / 100.0f) % 100;
    uint32_t copper = Util::int32abs<uint32_t>(total) % 100;

#if VERSION_STRING < Cata
    uint32_t newgold = player_target->getCoinage() + total;
#else
    uint64_t newgold = player_target->getCoinage() + total;
#endif

    if (newgold == 0)
    {
        blueSystemMessage(m_session, "Taking all gold from {}'s backpack...", player_target->getName());
        greenSystemMessage(player_target->getSession(), "{} took all gold from your backpack.", m_session->GetPlayer()->getName());
    }
    else
    {
        if (total >= 0)
        {
            if (worldConfig.player.isGoldCapEnabled)
            {
                if ((player_target->getCoinage() + newgold) > worldConfig.player.limitGoldAmount)
                {
                    redSystemMessage(m_session, "Maximum amount of gold is {} and {} already has {}", (worldConfig.player.limitGoldAmount / 10000), player_target->getName(), (player_target->getCoinage() / 10000));
                    return true;
                }
            }

            blueSystemMessage(m_session, "Adding {} gold, {} silver, {} copper to {}'s backpack...", gold, silver, copper, player_target->getName());
            greenSystemMessage(player_target->getSession(), "{} added {} gold, {} silver, {} copper to your backpack.", m_session->GetPlayer()->getName(), gold, silver, copper);
            sGMLog.writefromsession(m_session, "added %u gold, %u silver, %u copper to %s's backpack.", gold, silver, copper , player_target->getName().c_str());
        }
        else
        {
            blueSystemMessage(m_session, "Taking {} gold, {} silver, {} copper from {}'s backpack...", gold, silver, copper, player_target->getName());
            greenSystemMessage(player_target->getSession(), "{} took {} gold, {} silver, {} copper from your backpack.", m_session->GetPlayer()->getName(), gold, silver, copper);
            sGMLog.writefromsession(m_session, "took %u gold, %u silver, %u copper from %s's backpack.", gold, silver, copper, player_target->getName().c_str());
        }
    }

    player_target->setCoinage(newgold);

    return true;
}

//.character add silver
bool ChatCommandHandler::HandleCharAddSilverCommand(const char* args, WorldSession* m_session)
{
    if (*args == 0)
    {
        redSystemMessage(m_session, "You must specify how many silver you will add.");
        redSystemMessage(m_session, "1000 = 1 gold, 1 = 1 silver");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    int32_t total = atoi(args) * 100;

    uint32_t gold = (uint32_t)std::floor(Util::int32abs<float>(total) / 10000.0f);
    uint32_t silver = (uint32_t)std::floor(Util::int32abs<float>(total) / 100.0f) % 100;

#if VERSION_STRING < Cata
    uint32_t newgold = player_target->getCoinage() + total;
#else
    uint64_t newgold = player_target->getCoinage() + total;
#endif

    if (newgold == 0)
    {
        blueSystemMessage(m_session, "Taking all gold from {}'s backpack...", player_target->getName());
        greenSystemMessage(player_target->getSession(), "{} took all gold from your backpack.", m_session->GetPlayer()->getName());
    }
    else
    {
        if (total >= 0)
        {
            if (worldConfig.player.isGoldCapEnabled)
            {
                if ((player_target->getCoinage() + newgold) > worldConfig.player.limitGoldAmount)
                {
                    redSystemMessage(m_session, "Maximum amount of gold is {} and {} already has {}", (worldConfig.player.limitGoldAmount / 10000), player_target->getName(), (player_target->getCoinage() / 10000));
                    return true;
                }
            }

            blueSystemMessage(m_session, "Adding {} gold, {} silver to {}'s backpack...", gold, silver, player_target->getName());
            greenSystemMessage(player_target->getSession(), "{} added {} gold, {} silver to your backpack.", m_session->GetPlayer()->getName(), gold, silver);
            sGMLog.writefromsession(m_session, "added %u gold, %u silver to %s's backpack.", gold, silver, player_target->getName().c_str());
        }
        else
        {
            blueSystemMessage(m_session, "Taking {} gold, {} silver from {}'s backpack...", gold, silver, player_target->getName());
            greenSystemMessage(player_target->getSession(), "{} took {} gold, {} silver from your backpack.", m_session->GetPlayer()->getName(), gold, silver);
            sGMLog.writefromsession(m_session, "took %u gold, %u silver from %s's backpack.", gold, silver, player_target->getName().c_str());
        }
    }

    player_target->setCoinage(newgold);

    return true;
}

//.character add gold
bool ChatCommandHandler::HandleCharAddGoldCommand(const char* args, WorldSession* m_session)
{
    if (*args == 0)
    {
        redSystemMessage(m_session, "You must specify how many gold you will add.");
        redSystemMessage(m_session, "1 = 1 gold.");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    int32_t total = atoi(args) * 10000;

    uint32_t gold = (uint32_t)std::floor(Util::int32abs<float>(total) / 10000.0f);

#if VERSION_STRING < Cata
    uint32_t newgold = player_target->getCoinage() + total;
#else
    uint64_t newgold = player_target->getCoinage() + total;
#endif

    if (newgold == 0)
    {
        blueSystemMessage(m_session, "Taking all gold from {}'s backpack...", player_target->getName());
        greenSystemMessage(player_target->getSession(), "{} took all gold from your backpack.", m_session->GetPlayer()->getName());
    }
    else
    {
        if (total >= 0)
        {
            if (worldConfig.player.isGoldCapEnabled)
            {
                if ((player_target->getCoinage() + newgold) > worldConfig.player.limitGoldAmount)
                {
                    redSystemMessage(m_session, "Maximum amount of gold is {} and {} already has {}", (worldConfig.player.limitGoldAmount / 10000), player_target->getName(), (player_target->getCoinage() / 10000));
                    return true;
                }
            }

            blueSystemMessage(m_session, "Adding {} gold to {}'s backpack...", gold, player_target->getName());
            greenSystemMessage(player_target->getSession(), "{} added {} gold to your backpack.", m_session->GetPlayer()->getName(), gold);
            sGMLog.writefromsession(m_session, "added %u gold to %s's backpack.", gold, player_target->getName().c_str());
        }
        else
        {
            blueSystemMessage(m_session, "Taking {} gold from {}'s backpack...", gold, player_target->getName());
            greenSystemMessage(player_target->getSession(), "{} took {} gold from your backpack.", m_session->GetPlayer()->getName(), gold);
            sGMLog.writefromsession(m_session, "took %u gold from %s's backpack.", gold, player_target->getName().c_str());
        }
    }

    player_target->setCoinage(newgold);

    return true;
}

#if VERSION_STRING >= TBC // support classic
//.character resetskills
bool ChatCommandHandler::HandleCharResetSkillsCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    selected_player->removeAllSkills();
    selected_player->learnInitialSkills();

    selected_player->updateStats();
    selected_player->updateChances();
    selected_player->updateSkillMaximumValues();

    if (selected_player != m_session->GetPlayer())
    {
        systemMessage(m_session, "Reset skills of {}.", selected_player->getName());
        blueSystemMessage(selected_player->getSession(), "{} reset all your skills.", m_session->GetPlayer()->getName());
        sGMLog.writefromsession(m_session, "reset skills of %s", selected_player->getName().c_str());
    }
    else
    {
        blueSystemMessage(m_session, "Your skills are reset to default.");
    }

    return true;
}
#endif

//.character removeitem
bool ChatCommandHandler::HandleCharRemoveItemCommand(const char* args, WorldSession* m_session)
{
    uint32_t item_id;
    int32_t count, ocount;

    int argc = sscanf(args, "%u %u", &item_id, (unsigned int*)&count);
    if (argc == 1)
        count = 1;
    else if (argc != 2 || !count)
        return false;

    ocount = count;
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    int32_t loop_count = 0;
    int32_t start_count = selected_player->getItemInterface()->GetItemCount(item_id, true);
    int32_t start_count2 = start_count;
    if (count > start_count)
        count = start_count;

    while (start_count >= count && (count > 0) && loop_count < 20)
    {
        selected_player->getItemInterface()->RemoveItemAmt(item_id, count);
        start_count2 = selected_player->getItemInterface()->GetItemCount(item_id, true);
        count -= (start_count - start_count2);
        start_count = start_count2;
        ++loop_count;
    }

    ItemProperties const* item_properties = sMySQLStore.getItemProperties(item_id);
    if (item_properties != nullptr)
    {
        if (selected_player != m_session->GetPlayer())
        {
            sGMLog.writefromsession(m_session, "used remove item %s (id: %u) count %u from %s", item_properties->Name.c_str(), item_id, ocount, selected_player->getName().c_str());
            blueSystemMessage(m_session, "Removing {} copies of item {} (id: {}) from {}'s inventory.", ocount, sMySQLStore.getItemLinkByProto(item_properties, m_session->language), item_id, selected_player->getName());
            blueSystemMessage(selected_player->getSession(), "{} removed {} copies of item {} from your inventory.", m_session->GetPlayer()->getName(), ocount, sMySQLStore.getItemLinkByProto(item_properties, selected_player->getSession()->language));
        }
        else
        {
            blueSystemMessage(m_session, "Removing {} copies of item {} (id: {}) from your inventory.", ocount, sMySQLStore.getItemLinkByProto(item_properties, m_session->language), item_id);
        }
    }
    else
    {
        redSystemMessage(m_session, "Cannot remove non valid item id: {} .", item_id);
    }

    return true;
}

//.character resettalents
bool ChatCommandHandler::HandleCharResetTalentsCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    selected_player->resetTalents();

    if (selected_player != m_session->GetPlayer())
    {
        systemMessage(m_session, "Reset talents of {}.", selected_player->getName());
        blueSystemMessage(selected_player->getSession(), "{} reset all your talents.", m_session->GetPlayer()->getName());
        sGMLog.writefromsession(m_session, "reset talents of %s", selected_player->getName().c_str());
    }
    else
    {
        blueSystemMessage(m_session, "All your talents are reset.");
    }

    return true;
}

//.character advanceallskills
bool ChatCommandHandler::HandleAdvanceAllSkillsCommand(const char* args, WorldSession* m_session)
{
    auto amt = static_cast<uint16_t>(std::stoul(args));
    if (!amt)
    {
        redSystemMessage(m_session, "An amount to increment is required.");
        return true;
    }

    Player* selected_player = GetSelectedPlayer(m_session, false, true);
    if (selected_player == nullptr)
        return true;

    selected_player->advanceAllSkills(amt);

    if (selected_player != m_session->GetPlayer())
    {
        greenSystemMessage(selected_player->getSession(), "{} advanced all your skill lines by {} points.", m_session->GetPlayer()->getName(),  amt);
        sGMLog.writefromsession(m_session, "advanced all skills by %u on %s", amt, selected_player->getName().c_str());
    }
    else
    {
        greenSystemMessage(m_session, "Advanced all your skill lines by {} points.", amt);
    }

    return true;
}

//.character increaseweaponskill
bool ChatCommandHandler::HandleCharIncreaseWeaponSkill(const char* args, WorldSession* m_session)
{
    char* pMin = strtok((char*)args, " ");
    uint16_t cnt = 0;
    if (!pMin)
        cnt = 1;
    else
        cnt = static_cast<uint16_t>(std::stoul(pMin));

    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    uint16_t SubClassSkill = 0;

    Item* item = selected_player->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    ItemProperties const* proto = nullptr;
    if (item == nullptr)
        item = selected_player->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);

    if (item != nullptr)
        proto = item->getItemProperties();

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
#if VERSION_STRING <= Cata
            case 16: // thrown
                SubClassSkill = SKILL_THROWN;
                break;
#endif
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
        redSystemMessage(m_session, "Can't find skill ID!");
        return false;
    }

    auto skill = SubClassSkill;

    if (selected_player != m_session->GetPlayer())
    {
        blueSystemMessage(selected_player->getSession(), "{} modified your skill line {}. Advancing {} times.", m_session->GetPlayer()->getName(), skill, cnt);
        blueSystemMessage(m_session, "Modifying skill line {}. Advancing {} times for {}.", skill, cnt, selected_player->getName());
        sGMLog.writefromsession(m_session, "increased weapon skill (%u) of %s by %u", skill, selected_player->getName().c_str(), cnt);
    }
    else
    {
        blueSystemMessage(m_session, "Modifying skill line {}. Advancing {} times.", skill, cnt);
    }

    if (!selected_player->hasSkillLine(skill))
    {
        systemMessage(m_session, "Does not have skill line {}, adding.", skill);
        selected_player->addSkillLine(skill, cnt, 0);
    }
    else
    {
        selected_player->advanceSkillLine(skill, cnt);
    }

    return true;
}

//.character resetreputation
bool ChatCommandHandler::HandleCharResetReputationCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    selected_player->initialiseReputation();

    if (selected_player != m_session->GetPlayer())
    {
        systemMessage(selected_player->getSession(), "{} resets your reputation. Relog for changes to take effect.", m_session->GetPlayer()->getName());
        sGMLog.writefromsession(m_session, "used reset reputation for %s", selected_player->getName().c_str());
        systemMessage(m_session, "Reputation reset for {}", selected_player->getName());
    }
    else
    {
        systemMessage(m_session, "Done. Relog for changes to take effect.");
    }

    return true;
}

//.character resetspells
bool ChatCommandHandler::HandleCharResetSpellsCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    selected_player->resetSpells();

    if (selected_player != m_session->GetPlayer())
    {
        systemMessage(m_session, "Reset spells of {} to level 1.", selected_player->getName());
        blueSystemMessage(selected_player->getSession(), "{} reset all your spells to starting values.", m_session->GetPlayer()->getName());
        sGMLog.writefromsession(m_session, "reset spells of %s", selected_player->getName().c_str());
    }
    else
    {
        blueSystemMessage(m_session, "Your spells are reset.");
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .character set commands
//.character set allexplored
bool ChatCommandHandler::HandleCharSetAllExploredCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    systemMessage(m_session, "{} has explored all zones now.", player_target->getName());
    greenSystemMessage(player_target->getSession(), "{} sets all areas as explored for you.", m_session->GetPlayer()->getName());
    sGMLog.writefromsession(m_session, "sets all areas as explored for player %s", player_target->getName().c_str());

    for (uint8_t i = 0; i < WOWPLAYER_EXPLORED_ZONES_COUNT; ++i)
    {
        player_target->setExploredZone(i, 0xFFFFFFFF);
    }

#if VERSION_STRING > TBC
    player_target->getAchievementMgr()->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA); // update
#endif
    return true;
}

//.character set gender
bool ChatCommandHandler::HandleCharSetGenderCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    uint32_t displayId = player_target->getNativeDisplayId();
    uint8_t gender;
    if (*args == 0)
    {
        gender = player_target->getGender() == 1 ? 0 : 1;
    }
    else
    {
        gender = (uint8_t)atoi(args);
        if (gender > 1)
            gender = 1;
    }

    if (gender == player_target->getGender())
    {
        systemMessage(m_session, "{}'s gender is already set to {}({}).", player_target->getName(), gender ? "Female" : "Male", gender);
        return true;
    }

    player_target->setGender(gender);
    systemMessage(m_session, "Set {}'s gender to {}({}).", player_target->getName(), gender ? "Female" : "Male", gender);

#if VERSION_STRING > Classic
    if (player_target->getGender() == 0)
    {
        player_target->setDisplayId((player_target->getRace() == RACE_BLOODELF) ? ++displayId : --displayId);
        player_target->setNativeDisplayId(displayId);
    }
    else
    {
        player_target->setDisplayId((player_target->getRace() == RACE_BLOODELF) ? --displayId : ++displayId);
        player_target->setNativeDisplayId(displayId);
    }
#endif

    player_target->eventModelChange();

    return true;
}

//.character set itemsrepaired
bool ChatCommandHandler::HandleCharSetItemsRepairedCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    for (uint8_t i = 0; i < MAX_INVENTORY_SLOT; i++)
    {
        auto player_item = player_target->getItemInterface()->GetInventoryItem(static_cast<uint16_t>(i));
        if (player_item != nullptr)
        {
            if (player_item->isContainer())
            {
                auto item_container = static_cast<Container*>(player_item);
                for (uint32_t j = 0; j < item_container->getItemProperties()->ContainerSlots; ++j)
                {
                    player_item = item_container->getItem(static_cast<uint16_t>(j));
                    if (player_item != nullptr)
                    {
                        player_item->setDurabilityToMax();
                        player_item->m_isDirty = true;
                    }
                }
            }
            else
            {
                if (player_item->getItemProperties()->MaxDurability > 0 && i < INVENTORY_SLOT_BAG_END && player_item->getDurability() <= 0)
                {
                    player_item->setDurabilityToMax();
                    player_item->m_isDirty = true;
                    player_target->applyItemMods(player_item, static_cast<uint16_t>(i), true);
                }
                else
                {
                    player_item->setDurabilityToMax();
                    player_item->m_isDirty = true;
                }
            }
        }
    }

    if (player_target != m_session->GetPlayer())
    {
        blueSystemMessage(m_session, "All items has been repaired for Player {}", player_target->getName());
        greenSystemMessage(player_target->getSession(), "{} repaired all your items.", m_session->GetPlayer()->getName());
        sGMLog.writefromsession(m_session, "repaired all items for player %s.", player_target->getName().c_str());
    }
    else
    {
        blueSystemMessage(m_session, "You repaired all your items.");
    }

    return true;
}

//.character set level
bool ChatCommandHandler::HandleCharSetLevelCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    uint32_t new_level = args ? std::stoul(args) : 0;
    if (new_level == 0 || new_level > worldConfig.player.playerLevelCap)
    {
        redSystemMessage(m_session, "Level {} is not a valid level! Check out your world.conf!", new_level);
        return true;
    }

    auto level_info = sObjectMgr.getLevelInfo(player_target->getRace(), player_target->getClass(), new_level);
    if (level_info == nullptr)
    {
        redSystemMessage(m_session, "Level information not found in table playercreateinfo!");
        return true;
    }

    if (player_target != m_session->GetPlayer())
    {
        blueSystemMessage(m_session, "Setting the level of {} to {}.", player_target->getName(), new_level);
        greenSystemMessage(player_target->getSession(), "{} set your level to %u.", m_session->GetPlayer()->getName(), new_level);
        sGMLog.writefromsession(m_session, "set level on %s, level %u", player_target->getName().c_str(), new_level);
    }
    else
    {
        blueSystemMessage(m_session, "You set your own level to {}.", new_level);
    }

    player_target->applyLevelInfo(new_level);

    return true;
}

//.character set name
bool ChatCommandHandler::HandleCharSetNameCommand(const char* args, WorldSession* m_session)
{
    if (!args)
        return false;

    constexpr std::size_t maxNameLength = 100;

    std::string current_name;
    std::string new_name_cmd;

    std::istringstream iss(std::string{ args });

    if (!(iss >> current_name >> new_name_cmd))
        return false;

    if (current_name.empty() || current_name.size() > maxNameLength ||
        new_name_cmd.empty() || new_name_cmd.size() > maxNameLength)
    {
        return false;
    }

    auto isAlphaAscii = [](unsigned char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        };

    if (!std::all_of(new_name_cmd.begin(), new_name_cmd.end(), isAlphaAscii))
    {
        redSystemMessage(m_session, "That name is invalid or contains invalid characters.");
        return true;
    }

    std::string new_name = new_name_cmd;
    AscEmu::Util::Strings::capitalize(new_name);

    const auto pi = sObjectMgr.getCachedCharacterInfoByName(current_name);
    if (pi == nullptr)
    {
        redSystemMessage(m_session, "Player not found with this name.");
        return true;
    }

    if (sObjectMgr.getCachedCharacterInfoByName(new_name) != nullptr)
    {
        redSystemMessage(m_session, "New name {} is already in use.", new_name);
        return true;
    }

    sObjectMgr.updateCachedCharacterInfoName(pi, new_name);

    Player* plr = sObjectMgr.getPlayer(pi->guid);
    if (plr != nullptr)
    {
        plr->setName(new_name);
        blueSystemMessage(plr->getSession(), "{} changed your name to '{}'.", m_session->GetPlayer()->getName(), new_name);
        plr->saveToDB(false);
    }
    else
    {
        CharacterDatabase.Execute("UPDATE characters SET name = '%s' WHERE guid = %u", CharacterDatabase.EscapeString(new_name).c_str(), pi->guid);
    }

    greenSystemMessage(m_session, "Changed name of '{}' to '{}'.", current_name, new_name);
    sGMLog.writefromsession(m_session, "renamed character %s (GUID: %u) to %s", current_name.c_str(), pi->guid, new_name.c_str());
    sPlrLog.writefromsession(m_session, "GM renamed character %s (GUID: %u) to %s", current_name.c_str(), pi->guid, new_name.c_str());
    return true;
}

//.character set phase
bool ChatCommandHandler::HandleCharSetPhaseCommand(const char* args, WorldSession* m_session)
{
    uint32_t phase = atoi(args);
    if (phase == 0)
    {
        redSystemMessage(m_session, "No phase set. Use .character set phase <phase>");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    player_target->setPhase(PHASE_SET, phase);

    if (player_target != m_session->GetPlayer())
    {
        blueSystemMessage(m_session, "Setting the phase of {} to {}.", player_target->getName(), phase);
        greenSystemMessage(player_target->getSession(), "{} set your phase to {}.", m_session->GetPlayer()->getName(), phase);
        sGMLog.writefromsession(m_session, "set phase on %s, phase %u", player_target->getName().c_str(), phase);
    }
    else
    {
        blueSystemMessage(m_session, "You set your own phase to {}.", phase);
    }

    return true;
}

//.character set speed
bool ChatCommandHandler::HandleCharSetSpeedCommand(const char* args, WorldSession* m_session)
{
    float speed = float(atof(args));
    if (speed == 0.0f || speed > 255.0f || speed < 0.1f)
    {
        redSystemMessage(m_session, "Invalid speed set. Value range 0.1f ... 255.0f Use .character set speed <speed>");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (player_target != m_session->GetPlayer())
    {
        blueSystemMessage(m_session, "Setting the speed of {} to {}.", player_target->getName(), speed);
        greenSystemMessage(player_target->getSession(), "{} set your speed to {}.", m_session->GetPlayer()->getName(), speed);
        sGMLog.writefromsession(m_session, "modified speed of %s to %3.2f.", player_target->getName().c_str(), speed);
    }
    else
    {
        blueSystemMessage(m_session, "Setting your speed to {}.", speed);
    }

    player_target->setSpeedRate(TYPE_RUN, speed, true);
    player_target->setSpeedRate(TYPE_SWIM, speed, true);
    player_target->setSpeedRate(TYPE_RUN_BACK, speed / 2, true);
    player_target->setSpeedRate(TYPE_FLY, speed * 2, true);

    return true;
}

//.character set standing
bool ChatCommandHandler::HandleCharSetStandingCommand(const char* args, WorldSession* m_session)
{
    uint32_t faction;
    int32_t standing;

    if (sscanf(args, "%u %d", &faction, (unsigned int*)&standing) != 2)
    {
        redSystemMessage(m_session, "No faction or standing value entered.");
        redSystemMessage(m_session, "Use: .character set standstate <factionid> <standing>");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    player_target->setFactionStanding(faction, standing);

    if (player_target != m_session->GetPlayer())
    {
        blueSystemMessage(m_session, "Setting standing of {} to {} on {}.", faction, standing, player_target->getName());
        greenSystemMessage(player_target->getSession(), "{} set your standing of faction {} to {}.", m_session->GetPlayer()->getName(), faction, standing);
        sGMLog.writefromsession(m_session, "set standing of faction %u to %u for %s", faction, standing, player_target->getName().c_str());
    }
    else
    {
        blueSystemMessage(m_session, "Setting standing of {} to {} on you.", faction, standing);
    }

    return true;
}

//.character set talentpoints
bool ChatCommandHandler::HandleCharSetTalentpointsCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        redSystemMessage(m_session, "No amount of talentpoints entered.");
#ifdef FT_DUAL_SPEC
        redSystemMessage(m_session, "Use: .character set talentpoints <primary_amount> <secondary_amount>");
#else
        redSystemMessage(m_session, "Use: .character set talentpoints <amount>");
#endif
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    uint32_t primary_amount = 0;
    [[maybe_unused]]uint32_t secondary_amount = 0;

    std::stringstream ss(args);
    ss >> primary_amount;
#ifdef FT_DUAL_SPEC
    ss >> secondary_amount;
#endif

#ifdef FT_DUAL_SPEC
    player_target->m_specs[SPEC_PRIMARY].setTalentPoints(primary_amount);
    player_target->m_specs[SPEC_SECONDARY].setTalentPoints(secondary_amount);
#else
    player_target->m_spec.setTalentPoints(primary_amount);
#endif

    player_target->smsg_TalentsInfo(false);

    if (player_target != m_session->GetPlayer())
    {
#ifdef FT_DUAL_SPEC
        blueSystemMessage(m_session, "Setting talentpoints primary: {}, secondary: {} for player {}.", primary_amount, secondary_amount, player_target->getName());
        greenSystemMessage(player_target->getSession(), "{} set your talenpoints to primary: {}, secondary: {}.", m_session->GetPlayer()->getName(), primary_amount, secondary_amount);
        sGMLog.writefromsession(m_session, "set talenpoints primary: %u, secondary: %u for player %s", primary_amount, secondary_amount, player_target->getName().c_str());
#else
        blueSystemMessage(m_session, "Setting talent points {} for player {}.", primary_amount, player_target->getName());
        greenSystemMessage(player_target->getSession(), "{} set your talent points to {}.", m_session->GetPlayer()->getName(), primary_amount);
        sGMLog.writefromsession(m_session, "set talent points %u for player %s", primary_amount, player_target->getName().c_str());
#endif
    }
    else
    {
#ifdef FT_DUAL_SPEC
        blueSystemMessage(m_session, "Your talent points were set - primary: {}, secondary: {}.", primary_amount, secondary_amount);
#else
        blueSystemMessage(m_session, "Your talent points were set to {}.", primary_amount);
#endif
    }

    return true;
}

//.character set title
bool ChatCommandHandler::HandleCharSetTitleCommand(const char* args, WorldSession* m_session)
{
#if VERSION_STRING > Classic
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    int32_t title = std::stoul(args);
    if (title > int32_t(PVPTITLE_END) || title < -int32_t(PVPTITLE_END))
    {
        redSystemMessage(m_session, "Argument {} is out of range!", title);
        return false;
    }
    if (title == 0)
    {
        player_target->setKnownTitles(0, 0);
#if VERSION_STRING > TBC
        player_target->setKnownTitles(1, 0);
        player_target->setKnownTitles(2, 0);
#endif
    }
    else if (title > 0)
    {
        player_target->setKnownPvPTitle(static_cast<RankTitles>(title), true);
    }
    else
    {
        player_target->setKnownPvPTitle(static_cast<RankTitles>(-title), false);
    }

#if VERSION_STRING > Classic
    player_target->setChosenTitle(0);  // better remove chosen one
#endif
    systemMessage(m_session, "Title has been {}.", title > 0 ? "set" : "reset");

    std::stringstream logtext;
    logtext << "has ";
    if (title > 0)
        logtext << "set the title field of " << player_target->getName().c_str() << "to " << title << ".";
    else
        logtext << "reset the title field of " << player_target->getName().c_str();

    sGMLog.writefromsession(m_session, logtext.str().c_str());

    return true;
#else
    redSystemMessage(m_session, "This command is not available for Classic!");
    return true;
#endif
}

//.character set forcerename
bool ChatCommandHandler::HandleCharSetForceRenameCommand(const char* args, WorldSession* m_session)
{
    if (strlen(args) > 100)
        return false;

    std::string tmp = std::string(args);
    const auto pi = sObjectMgr.getCachedCharacterInfoByName(tmp);
    if (pi == nullptr)
    {
        redSystemMessage(m_session, "Player with that name not found.");
        return true;
    }

    Player* plr = sObjectMgr.getPlayer(pi->guid);
    if (plr == nullptr)
    {
        CharacterDatabase.Execute("UPDATE characters SET login_flags = %u WHERE guid = %u", (uint32_t)LOGIN_FORCED_RENAME, pi->guid);
    }
    else
    {
        plr->setLoginFlag(LOGIN_FORCED_RENAME);
        plr->saveToDB(false);
        blueSystemMessage(plr->getSession(), "{} forced your character to be renamed next logon.", m_session->GetPlayer()->getName());
    }

    CharacterDatabase.Execute("INSERT INTO banned_names VALUES('%s')", CharacterDatabase.EscapeString(pi->name).c_str());
    greenSystemMessage(m_session, "Forcing {} to rename his character next logon.", args);
    sGMLog.writefromsession(m_session, "forced %s to rename his charater (%u)", pi->name.c_str(), pi->guid);
    return true;
}

//.character set customize
bool ChatCommandHandler::HandleCharSetCustomizeCommand(const char* args, WorldSession* m_session)
{
    // prevent buffer overflow
    if (strlen(args) > 100)
        return false;

    std::string tmp = std::string(args);
    const auto pi = sObjectMgr.getCachedCharacterInfoByName(tmp);
    if (pi == nullptr)
    {
        redSystemMessage(m_session, "Player with that name not found.");
        return true;
    }

    Player* plr = sObjectMgr.getPlayer(pi->guid);
    if (plr == nullptr)
    {
        CharacterDatabase.Execute("UPDATE characters SET login_flags = %u WHERE guid = %u", (uint32_t)LOGIN_CUSTOMIZE_LOOKS, pi->guid);
    }
    else
    {
        plr->setLoginFlag(LOGIN_CUSTOMIZE_LOOKS);
        plr->saveToDB(false);
        blueSystemMessage(plr->getSession(), "{} flagged your character for customization at next login.", m_session->GetPlayer()->getName());
    }

    greenSystemMessage(m_session, "{} flagged to customize his character next logon.", args);
    sGMLog.writefromsession(m_session, "flagged %s for customization for charater (%u)", pi->name.c_str(), pi->guid);
    return true;
}

//.character set factionchange
bool ChatCommandHandler::HandleCharSetFactionChangeCommand(const char* args, WorldSession* m_session)
{
    // prevent buffer overflow
    if (strlen(args) > 100)
        return false;

    std::string tmp = std::string(args);
    const auto pi = sObjectMgr.getCachedCharacterInfoByName(tmp);
    if (pi == nullptr)
    {
        redSystemMessage(m_session, "Player with that name not found.");
        return true;
    }

    Player* plr = sObjectMgr.getPlayer(pi->guid);
    if (plr == nullptr)
    {
        CharacterDatabase.Execute("UPDATE characters SET login_flags = %u WHERE guid = %u", (uint32_t)LOGIN_CUSTOMIZE_FACTION, pi->guid);
    }
    else
    {
        plr->setLoginFlag(LOGIN_CUSTOMIZE_FACTION);
        plr->saveToDB(false);
        blueSystemMessage(plr->getSession(), "{} flagged your character for a faction change at next login.", m_session->GetPlayer()->getName());
    }

    greenSystemMessage(m_session, "{} flagged for a faction change next logon.", args);
    sGMLog.writefromsession(m_session, "flagged %s for a faction change for charater (%u)", pi->name.c_str(), pi->guid);
    return true;
}

//.character set racechange
bool ChatCommandHandler::HandleCharSetRaceChangeCommand(const char* args, WorldSession* m_session)
{
    // prevent buffer overflow
    if (strlen(args) > 100)
        return false;

    std::string tmp = std::string(args);
    const auto pi = sObjectMgr.getCachedCharacterInfoByName(tmp);
    if (pi == nullptr)
    {
        redSystemMessage(m_session, "Player with that name not found.");
        return true;
    }

    Player* plr = sObjectMgr.getPlayer(pi->guid);
    if (plr == nullptr)
    {
        CharacterDatabase.Execute("UPDATE characters SET login_flags = %u WHERE guid = %u", (uint32_t)LOGIN_CUSTOMIZE_RACE, pi->guid);
    }
    else
    {
        plr->setLoginFlag(LOGIN_CUSTOMIZE_RACE);
        plr->saveToDB(false);
        blueSystemMessage(plr->getSession(), "{} flagged your character for a race change at next login.", m_session->GetPlayer()->getName());
    }

    greenSystemMessage(m_session, "{} flagged for a race change next logon.", args);
    sGMLog.writefromsession(m_session, "flagged %s for a race change for charater (%u)", pi->name.c_str(), pi->guid);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .character list commands
//.character list skills
bool ChatCommandHandler::HandleCharListSkillsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    uint16_t nobonus = 0;
    int16_t bonus = 0;
    uint16_t max = 0;

    blueSystemMessage(m_session, "===== {} has skills =====", player_target->getName());

    for (uint16_t SkillId = 0; SkillId <= SkillNameManager->maxskill; SkillId++)
    {
        if (player_target->hasSkillLine(SkillId))
        {
            char* SkillName = SkillNameManager->SkillNames[SkillId].get();
            if (!SkillName)
            {
                redSystemMessage(m_session, "Invalid skill: {}", SkillId);
                continue;
            }
            nobonus = player_target->getSkillLineCurrent(SkillId, false);
            bonus = player_target->getSkillLineCurrent(SkillId, true) - nobonus;
            max = player_target->getSkillLineMax(SkillId);
            blueSystemMessage(m_session, " {}: Value: {}, MaxValue: {}. (+ {} bonus)", SkillName, nobonus, max, bonus);
        }
    }
    return true;
}

//.character list spells
bool ChatCommandHandler::handleCharListSpellsCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    // No args or 0 = list only active spells
    // 1 = list deleted / inactive spells
    const auto arg = static_cast<uint8_t>(std::stoul(args));
    std::string spellString = "";
    uint8_t itemsPerRow = 0;

    const auto& spellSet = arg == 1 ? player_target->getDeletedSpellSet() : player_target->getSpellSet();
    for (auto itr = spellSet.cbegin(); itr != spellSet.cend();)
    {
        spellString += std::to_string(*itr);
        ++itr;
        if (itr != spellSet.cend())
            spellString += ", ";
        ++itemsPerRow;
        if (itemsPerRow >= 9)
        {
            // Add a line break after 9 spells
            spellString += "\n";
            itemsPerRow = 0;
        }
    }

    if (arg == 1)
        blueSystemMessage(m_session, "===== {} has {} deleted spells =====", player_target->getName(), spellSet.size());
    else
        blueSystemMessage(m_session, "===== {} has {} spells =====", player_target->getName(), spellSet.size());

    SendMultilineMessage(m_session, spellString.c_str());
    return true;
}

//.character list standing
bool ChatCommandHandler::HandleCharListStandingCommand(const char* args, WorldSession* m_session)
{
    uint32_t faction = atoi(args);
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    int32_t standing = player_target->getFactionStanding(faction);
    int32_t bstanding = player_target->getBaseFactionStanding(faction);

    systemMessage(m_session, "==== {} standing ====", player_target->getName());
    systemMessage(m_session, "Reputation for faction {}:", faction);
    systemMessage(m_session, " Base Standing: {}", bstanding);
    systemMessage(m_session, " Standing: {}", standing);
    return true;
}

//.character list items
bool ChatCommandHandler::HandleCharListItemsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    systemMessage(m_session, "==== {} has items ====", player_target->getName());
    int itemcount = 0;
    ItemIterator itr(player_target->getItemInterface());
    itr.BeginSearch();
    for (; !itr.End(); itr.Increment())
    {
        if (!(*itr))
            return false;
        itemcount++;
        sendItemLinkToPlayer((*itr)->getItemProperties(), m_session, true, player_target, m_session->language);
    }
    itr.EndSearch();

    return true;
}

//.character list kills
bool ChatCommandHandler::HandleCharListKillsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    systemMessage(m_session, "==== {} kills ====", player_target->getName());
    systemMessage(m_session, "All Kills: {}", player_target->getKillsLifetime());
    systemMessage(m_session, "Kills today: {}", player_target->getKillsToday());
    systemMessage(m_session, "Kills yesterday: {}", player_target->getKillsYesterday());

    return true;
}

//.character list instances
bool ChatCommandHandler::HandleCharListInstanceCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    uint32_t count = 0;
    std::stringstream ss;
    ss << "Show persistent instances of " << MSG_COLOR_CYAN << player_target->getName().c_str() << "|r\n";

    for (uint32_t mapId = 0; mapId < MAX_NUM_MAPS; mapId++)
    {
        const auto save = player_target->getInstanceSave(mapId, false);

        if (save)
        {
            count++;
            MySQLStructure::MapInfo const* mapInfo = sMySQLStore.getWorldMapInfo(mapId);
            if (mapInfo)
            {
                ss << " - " << MSG_COLOR_CYAN << mapInfo->mapid << "|r";
                ss << " (" << MSG_COLOR_CYAN << mapInfo->name << "|r)";

                ss << " [" << GetMapTypeString(static_cast<uint8_t>(mapInfo->type)) << "]";
                if (mapInfo->isMultimodeDungeon())
                {
                    ss << " [" << GetDifficultyString(save->getDifficulty()) << "]";
                }
                ss << " - ";

                InstanceMap* instance = sMapMgr.findInstanceMap(save->getInstanceId());
                if (instance == nullptr)
                    ss << MSG_COLOR_LIGHTRED << "Shut Down|r";
                else
                {
                    if (!instance->hasPlayers())
                        ss << MSG_COLOR_LIGHTRED << "Idle|r";
                    else
                        ss << MSG_COLOR_GREEN << "In use|r";
                }

            }
        }
        ss << "\n";
    }

    if (count == 0)
        ss << "Player is not assigned to any persistent instances.\n";
    else
        ss << "Player is assigned to " << MSG_COLOR_CYAN << count << "|r persistent instances.\n";

    SendMultilineMessage(m_session, ss.str().c_str());
    sGMLog.writefromsession(m_session, "used show instances command on %s,", player_target->getName().c_str());
    return true;
}
