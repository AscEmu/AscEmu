/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

//.character clearcooldowns
bool ChatHandler::HandleCharClearCooldownsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (player_target != m_session->GetPlayer())
    {
        sGMLog.writefromsession(m_session, "Cleared all cooldowns for player %s", player_target->GetName());
    }

    uint64 guid = player_target->GetGUID();
    switch (player_target->getClass())
    {
        case WARRIOR:
        {
            player_target->ClearCooldownsOnLine(26, guid);
            player_target->ClearCooldownsOnLine(256, guid);
            player_target->ClearCooldownsOnLine(257, guid);
            BlueSystemMessage(m_session, "Cleared all Warrior cooldowns.");
            break;
        }
        case PALADIN:
        {
            player_target->ClearCooldownsOnLine(594, guid);
            player_target->ClearCooldownsOnLine(267, guid);
            player_target->ClearCooldownsOnLine(184, guid);
            BlueSystemMessage(m_session, "Cleared all Paladin cooldowns.");
            break;
        }
        case HUNTER:
        {
            player_target->ClearCooldownsOnLine(50, guid);
            player_target->ClearCooldownsOnLine(51, guid);
            player_target->ClearCooldownsOnLine(163, guid);
            BlueSystemMessage(m_session, "Cleared all Hunter cooldowns.");
            break;
        }
        case ROGUE:
        {
            player_target->ClearCooldownsOnLine(253, guid);
            player_target->ClearCooldownsOnLine(38, guid);
            player_target->ClearCooldownsOnLine(39, guid);
            BlueSystemMessage(m_session, "Cleared all Rogue cooldowns.");
            break;
        }
        case PRIEST:
        {
            player_target->ClearCooldownsOnLine(56, guid);
            player_target->ClearCooldownsOnLine(78, guid);
            player_target->ClearCooldownsOnLine(613, guid);
            BlueSystemMessage(m_session, "Cleared all Priest cooldowns.");
            break;
        }
        case DEATHKNIGHT:
        {
            player_target->ClearCooldownsOnLine(770, guid);
            player_target->ClearCooldownsOnLine(771, guid);
            player_target->ClearCooldownsOnLine(772, guid);
            BlueSystemMessage(m_session, "Cleared all Death Knight cooldowns.");
            break;
        }
        case SHAMAN:
        {
            player_target->ClearCooldownsOnLine(373, guid);
            player_target->ClearCooldownsOnLine(374, guid);
            player_target->ClearCooldownsOnLine(375, guid);
            BlueSystemMessage(m_session, "Cleared all Shaman cooldowns.");
            break;
        }
        case MAGE:
        {
            player_target->ClearCooldownsOnLine(6, guid);
            player_target->ClearCooldownsOnLine(8, guid);
            player_target->ClearCooldownsOnLine(237, guid);
            BlueSystemMessage(m_session, "Cleared all Mage cooldowns.");
            break;
        }
        case WARLOCK:
        {
            player_target->ClearCooldownsOnLine(355, guid);
            player_target->ClearCooldownsOnLine(354, guid);
            player_target->ClearCooldownsOnLine(593, guid);
            BlueSystemMessage(m_session, "Cleared all Warlock cooldowns.");
            break;
        }
        case DRUID:
        {
            player_target->ClearCooldownsOnLine(573, guid);
            player_target->ClearCooldownsOnLine(574, guid);
            player_target->ClearCooldownsOnLine(134, guid);
            BlueSystemMessage(m_session, "Cleared all Druid cooldowns.");
            break;
        }
    }
 
    return true;
}

//.character demorph
bool ChatHandler::HandleCharDeMorphCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    player_target->DeMorph();

    return true;
}

//.character levelup
bool ChatHandler::HandleCharLevelUpCommand(const char* args, WorldSession* m_session)
{
    uint32 levels = atoi(args);

    if (levels == 0)
    {
        RedSystemMessage(m_session, "Command must be in format: .character levelup <level>.");
        RedSystemMessage(m_session, "A 0 level is not allowed.");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    sGMLog.writefromsession(m_session, "used level up command on %s, with %u levels", player_target->GetName(), levels);

    levels += player_target->getLevel();

    if (levels > sWorld.m_levelCap)
        levels = sWorld.m_levelCap;

    auto level_info = objmgr.GetLevelInfo(player_target->getRace(), player_target->getClass(), levels);
    if (level_info == nullptr)
    {
        RedSystemMessage(m_session, "No LevelInfo for Leve: %u, Race: %u, Class: %u", levels, player_target->getRace(), player_target->getClass());
        return true;
    }

    player_target->ApplyLevelInfo(level_info, levels);

    if (player_target->getClass() == WARLOCK)
    {
        std::list<Pet*> summons = player_target->GetSummons();
        for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
        {
            if ((*itr)->IsInWorld() && (*itr)->isAlive())
            {
                (*itr)->setLevel(levels);
                (*itr)->ApplyStatsForLevel();
                (*itr)->UpdateSpellList();
            }
        }
    }

    if (player_target != m_session->GetPlayer())
    {
        BlueSystemMessage(m_session, "%s leveled up to level: %u", player_target->GetName(), levels);
        BlueSystemMessage(player_target->GetSession(), "%s leveled you up to %u.", m_session->GetPlayer()->GetName(), levels);
        sGMLog.writefromsession(m_session, "leveled player %s to level %u", player_target->GetName(), levels);
    }
    else
    {
        BlueSystemMessage(m_session, "You leveled yourself to %u", levels);
    }

    player_target->Social_TellFriendsOnline();

    return true;
}

//.character unlearn
bool ChatHandler::HandleCharUnlearnCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    uint32 spell_id = atol(args);
    if (spell_id == 0)
    {
        spell_id = GetSpellIDFromLink(args);
        if (spell_id == 0)
        {
            RedSystemMessage(m_session, "You must specify a spell id.");
            return true;
        }
    }

    sGMLog.writefromsession(m_session, "removed spell %u from %s", spell_id, player_target->GetName());
    if (player_target->HasSpell(spell_id))
    {
        GreenSystemMessage(player_target->GetSession(), "Removed spell %u.", spell_id);
        GreenSystemMessage(m_session, "Removed spell %u from %s.", spell_id, player_target->GetName());
        player_target->removeSpell(spell_id, false, false, 0);
    }
    else
    {
        RedSystemMessage(m_session, "That player does not have spell %u learnt.", spell_id);
    }
    return true;
}

//.character learnskill
bool ChatHandler::HandleCharLearnSkillCommand(const char* args, WorldSession* m_session)
{
    uint32 skill;
    uint32 min;
    uint32 max;

    if (sscanf(args, "%u %u %u", &skill, &min, &max) < 1)
    {
        RedSystemMessage(m_session, "Command must be at least in format: .character learnskill <skillid>.");
        RedSystemMessage(m_session, "Optional: .character learnskill <skillid> <min> <max>");
        return true;
    }

    if (min == 0)
        min = 1;

    if (max == 0)
        max = 1;

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    player_target->_AddSkillLine(skill, min, max);

    if (player_target == m_session->GetPlayer())
    {
        BlueSystemMessage(m_session, "Adding skill line %d", skill);
    }
    else
    {
        SystemMessage(player_target->GetSession(), "%s taught you skill line %u.", m_session->GetPlayer()->GetName(), skill);
        BlueSystemMessage(m_session, "Skill line %u added to player: %s", skill, player_target->GetName());
        sGMLog.writefromsession(m_session, "used add skill of %u %u %u on %s", skill, min, max, player_target->GetName());
    }

    return true;
}

//.character advanceskill
bool ChatHandler::HandleCharAdvanceSkillCommand(const char* args, WorldSession* m_session)
{
    uint32 skill;
    uint32 amount;

    if (sscanf(args, "%u %u", &skill, &amount) < 1)
    {
        RedSystemMessage(m_session, "Command must be at least in format: .character advanceskill <skillid>.");
        RedSystemMessage(m_session, "Optional: .character advanceskill <skillid> <amount>");
        return true;
    }

    if (amount == 0)
        amount = 1;

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    BlueSystemMessage(m_session, "Modifying skill line %u. Advancing %u times.", skill, amount);
    sGMLog.writefromsession(m_session, "used modify skill of %u %u on %s", skill, amount, player_target->GetName());

    if (!player_target->_HasSkillLine(skill))
    {
        SystemMessage(m_session, "Does not have skill line, adding.");
        player_target->_AddSkillLine(skill, 1, 525);
    }
    else
    {
        player_target->_AdvanceSkillLine(skill, amount);
    }

    return true;
}

//.character removeskill
bool ChatHandler::HandleCharRemoveSkillCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        RedSystemMessage(m_session, "Command must be at least in format: .character removeskill <skillid>.");
        return true;
    }

    uint32 skill = atoi(args);
    if (skill == 0)
    {
        RedSystemMessage(m_session, "%u is not a valid skill!", skill);
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (player_target->_HasSkillLine(skill))
    {
        player_target->_RemoveSkillLine(skill);

        BlueSystemMessage(m_session, "Removing skill line %u", skill);
        sGMLog.writefromsession(m_session, "used remove skill of %u on %s", skill, player_target->GetName());
        SystemMessage(player_target->GetSession(), "%s removed skill line %u from you. ", m_session->GetPlayer()->GetName(), skill);
    }
    else
    {
        BlueSystemMessage(m_session, "Player doesn't have skill line %d", skill);
    }
    return true;
}

//.character removeauras
bool ChatHandler::HandleCharRemoveAurasCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    BlueSystemMessage(m_session, "Removing all auras...");
    for (uint32 i = MAX_REMOVABLE_AURAS_START; i < MAX_REMOVABLE_AURAS_END; ++i)
    {
        if (player_target->m_auras[i] != 0)
            player_target->m_auras[i]->Remove();
    }

    if (player_target != m_session->GetPlayer())
        sGMLog.writefromsession(m_session, "Removed all of %s's auras.", player_target->GetName());

    return true;
}

//.character removesickness
bool ChatHandler::HandleCharRemoveSickessCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    player_target->RemoveAura(15007);

    if (player_target != m_session->GetPlayer())
    {
        BlueSystemMessage(m_session, "Removed resurrection sickness from %s", player_target->GetName());
        BlueSystemMessage(player_target->GetSession(), "%s removed your resurection sickness.", m_session->GetPlayer()->GetName());
        sGMLog.writefromsession(m_session, "removed resurrection sickness from player %s", player_target->GetName());
    }
    else
    {
        BlueSystemMessage(m_session, "Removed resurrection sickness from you");
    }

    return true;
}

//.character learn
bool ChatHandler::HandleCharLearnCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    OLD_SpellEntry* spell_entry = nullptr;

    if (stricmp(args, "all") == 0)
    {
        sGMLog.writefromsession(m_session, "taught %s all spells.", selected_player->GetName());
        SystemMessage(m_session, "Taught %s all spells.", selected_player->GetName());

        static uint32 spellarray[DRUID + 1][512] =
        {
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
        };

        uint8 player_class = selected_player->getClass();
        for (uint32 i = 0; spellarray[player_class][i] != 0; ++i)
        {
            spell_entry = dbcSpell.LookupEntryForced(spellarray[player_class][i]);
            if (spell_entry == nullptr)
                continue;

            if (selected_player->HasSpell(spellarray[player_class][i]))
                continue;

            selected_player->addSpell(spellarray[player_class][i]);
        }

        static uint32 paladinspellarray[RACE_WORGEN + 1][3] =
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

        static uint32 shamanspellarray[RACE_WORGEN + 1][2] =
        {
            { 0 },                  // RACE 0
            { 0 },                  // HUMAN
            { 2825, 0 },            // ORC Bloodlust
            { 32182, 0 },           // DWARF Heroism
            { 0 },                  // NIGHTELF
            { 0 },                  // UNDEAD
            { 2825, 0 },            // TAUREN Bloodlust
            { 0 },                  // GNOME
            { 2825, 0 },            // TROLL Bloodlust
            { 2825, 0 },            // GOBLIN Bloodlust
            { 0 },                  // BLOODELF
            { 32182, 0 },           // DRAENEI Heroism
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

        static uint32 magespellarray[RACE_WORGEN + 1][17] =
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

        uint8 player_race = selected_player->getRace();
        switch (player_class)
        {
            case PALADIN:
                for (uint32 i = 0; paladinspellarray[player_race][i] != 0; ++i)
                {
                    spell_entry = dbcSpell.LookupEntryForced(paladinspellarray[player_race][i]);
                    if (spell_entry == nullptr)
                        continue;

                    if (selected_player->HasSpell(paladinspellarray[player_race][i]))
                        continue;

                    selected_player->addSpell(paladinspellarray[player_race][i]);
                }
                break;
            case MAGE:
                for (uint32 i = 0; magespellarray[player_race][i] != 0; ++i)
                {
                    spell_entry = dbcSpell.LookupEntryForced(magespellarray[player_race][i]);
                    if (spell_entry == nullptr)
                        continue;

                    if (selected_player->HasSpell(magespellarray[player_race][i]))
                        continue;

                    selected_player->addSpell(magespellarray[player_race][i]);
                }
                break;
            case SHAMAN:
                for (uint32 i = 0; shamanspellarray[player_race][i] != 0; ++i)
                {
                    spell_entry = dbcSpell.LookupEntryForced(shamanspellarray[player_race][i]);
                    if (spell_entry == nullptr)
                        continue;

                    if (selected_player->HasSpell(shamanspellarray[player_race][i]))
                        continue;

                    selected_player->addSpell(shamanspellarray[player_race][i]);
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

    spell_entry = dbcSpell.LookupEntryForced(spell);
    if (spell_entry == nullptr)
    {
        SystemMessage(m_session, "Invalid spell %u", spell);
        return true;
    }

    if (!selected_player->GetSession()->HasGMPermissions() && (spell_entry->Effect[0] == SPELL_EFFECT_INSTANT_KILL || spell_entry->Effect[1] == SPELL_EFFECT_INSTANT_KILL || spell_entry->Effect[2] == SPELL_EFFECT_INSTANT_KILL))
    {
        SystemMessage(m_session, "don't be an idiot and teach players instakill spells. this action has been logged.");
        sGMLog.writefromsession(m_session, "is an idiot and tried to tought player %s instakill spell %u", selected_player->GetName(), spell);
        return true;
    }

    if (selected_player->HasSpell(spell))
    {
        SystemMessage(m_session, "%s already knows that spell.", selected_player->GetName());
        return true;
    }

    selected_player->addSpell(spell);

    sGMLog.writefromsession(m_session, "Taught %s spell %u", selected_player->GetName(), spell);
    BlueSystemMessage(selected_player->GetSession(), "%s taught you Spell %u", m_session->GetPlayer()->GetName(), spell);
    GreenSystemMessage(m_session, "Taught %s Spell %u", selected_player->GetName(), spell);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .character add commands
//.character add honorpoints
bool ChatHandler::HandleCharAddHonorPointsCommand(const char* args, WorldSession* m_session)
{
    uint32 honor_amount = args ? atol(args) : 1;

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    BlueSystemMessage(m_session, "%u honor points added to Player %s.", honor_amount, player_target->GetName());
    GreenSystemMessage(player_target->GetSession(), "%s added %u honor points to your character.", m_session->GetPlayer()->GetName(), honor_amount);
    sGMLog.writefromsession(m_session, "added %u honor points to character %s", honor_amount, player_target->GetName());

    HonorHandler::AddHonorPointsToPlayer(player_target, honor_amount);

    return true;
}


//.character add honorkill
bool ChatHandler::HandleCharAddHonorKillCommand(const char* args, WorldSession* m_session)
{
    uint32 kill_amount = args ? atol(args) : 1;
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    BlueSystemMessage(m_session, "%u honor kill points added to Player %s.", kill_amount, player_target->GetName());
    GreenSystemMessage(player_target->GetSession(), "%s added %u honor kill points to your character.", m_session->GetPlayer()->GetName(), kill_amount);
    sGMLog.writefromsession(m_session, "added %u honor kill points to character %s", kill_amount, player_target->GetName());

    player_target->m_killsToday += kill_amount;
    player_target->m_killsLifetime += kill_amount;
    player_target->SetUInt32Value(PLAYER_FIELD_KILLS, uint16(player_target->m_killsToday) | (player_target->m_killsYesterday << 16));
    player_target->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORBALE_KILLS, player_target->m_killsLifetime);

    return true;
}

//.character add item
bool ChatHandler::HandleCharAddItemCommand(const char* args, WorldSession* m_session)
{
    uint32 itemid = 0;
    uint32 count = 1;
    int32 randomprop = 0;
    int32 numadded = 0;

    if (sscanf(args, "%u %u %d", &itemid, &count, &randomprop) < 1)
    {
        RedSystemMessage(m_session, "Command must be at least in format: .character add item <itemID>.");
        RedSystemMessage(m_session, "Optional: .character add item <itemID> <amount> <randomprop>");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    auto item_proto = sMySQLStore.GetItemProperties(itemid);
    if (item_proto != nullptr)
    {
        numadded -= player_target->GetItemInterface()->GetItemCount(itemid);
        bool result = player_target->GetItemInterface()->AddItemById(itemid, count, randomprop);
        numadded += player_target->GetItemInterface()->GetItemCount(itemid);
        if (result == true)
        {
            if (count == 0)
            {
                sGMLog.writefromsession(m_session, "used add item command, item id %u [%s], quantity %u, to %s", item_proto->ItemId, item_proto->Name.c_str(), numadded, player_target->GetName());
            }
            else
            {
                sGMLog.writefromsession(m_session, "used add item command, item id %u [%s], quantity %u (only %lu added due to full inventory), to %s", item_proto->ItemId, item_proto->Name.c_str(), numadded, numadded, player_target->GetName());
            }

            SystemMessage(m_session, "Added item %s (id: %u), quantity %u, to %s's inventory.", GetItemLinkByProto(item_proto, m_session->language).c_str(), item_proto->ItemId, numadded, player_target->GetName());
            SystemMessage(player_target->GetSession(), "%s added item %s, quantity %u, to your inventory.", m_session->GetPlayer()->GetName(), GetItemLinkByProto(item_proto, player_target->GetSession()->language).c_str(), numadded);
        }
        else
        {
            SystemMessage(player_target->GetSession(), "Failed to add item.");
        }
        return true;
    }
    else
    {
        RedSystemMessage(m_session, "Item %u is not a valid item!", itemid);
        return true;
    }
}

//.character add itemset
bool ChatHandler::HandleCharAddItemSetCommand(const char* args, WorldSession* m_session)
{
    int32 setid = atoi(args);
    if (!setid)
    {
        RedSystemMessage(m_session, "You must specify a setid.");
        return true;
    }

    auto player = GetSelectedPlayer(m_session, true, true);
    if (player == nullptr)
        return true;

    /*auto item_set_list = objmgr.GetListForItemSet(setid);
    if (!item_set_list)
    {
        RedSystemMessage(m_session, "Invalid item set.");
        return true;
    }*/

    BlueSystemMessage(m_session, "Searching item set %u...", setid);
    sGMLog.writefromsession(m_session, "used add item set command, set %u, target %s", setid, player->GetName());

    uint32 itemset_items_count = 0;

    MySQLDataStore::ItemPropertiesContainer const* its = sMySQLStore.GetItemPropertiesStore();
    for (MySQLDataStore::ItemPropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        ItemProperties const* it = sMySQLStore.GetItemProperties(itr->second.ItemId);
        if (it == nullptr)
            continue;

        if (it->ItemSet != setid)
        {
            continue;
        }
        else
        {
            auto item = objmgr.CreateItem(it->ItemId, m_session->GetPlayer());
            if (item == nullptr)
                continue;

            if (it->Bonding == ITEM_BIND_ON_PICKUP)
            {
                if (it->Flags & ITEM_FLAG_ACCOUNTBOUND)
                    item->AccountBind();
                else
                    item->SoulBind();
            }

            if (!player->GetItemInterface()->AddItemToFreeSlot(item))
            {
                m_session->SendNotification("No free slots left!");
                item->DeleteMe();
                return true;
            }
            else
            {
                SystemMessage(m_session, "Added item: %s [%u]", it->Name.c_str(), it->ItemId);
                SlotResult* le = player->GetItemInterface()->LastSearchResult();
                player->SendItemPushResult(false, true, false, true, le->ContainerSlot, le->Slot, 1, item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());
                ++itemset_items_count;
            }

        }
    }

    if (itemset_items_count > 0)
        GreenSystemMessage(m_session, "Added set to inventory complete.");
    else
        RedSystemMessage(m_session, "Itemset ID: %d is not defined in tems table!", setid);

    return true;
}

//.character add copper
bool ChatHandler::HandleCharAddCopperCommand(const char* args, WorldSession* m_session)
{
    if (*args == 0)
    {
        RedSystemMessage(m_session, "You must specify how many copper you will add.");
        RedSystemMessage(m_session, "10000 = 1 gold, 1000 = 1 silver, 1 = 1 copper.");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    int32 total = atoi(args);

    uint32 gold = (uint32)std::floor((float)int32abs(total) / 10000.0f);
    uint32 silver = (uint32)std::floor(((float)int32abs(total) / 100.0f)) % 100;
    uint32 copper = int32abs2uint32(total) % 100;

    int32 newgold = player_target->GetGold() + total;
    if (newgold < 0)
    {
        BlueSystemMessage(m_session, "Taking all gold from %s's backpack...", player_target->GetName());
        GreenSystemMessage(player_target->GetSession(), "%s took all gold from your backpack.", m_session->GetPlayer()->GetName());
        newgold = 0;
    }
    else
    {
        if (total >= 0)
        {
            if (sWorld.GoldCapEnabled)
            {
                if ((player_target->GetGold() + newgold) > sWorld.GoldLimit)
                {
                    RedSystemMessage(m_session, "Maximum amount of gold is %u and %s already has %u", (sWorld.GoldLimit / 10000), player_target->GetName(), (player_target->GetGold() / 10000));
                    return true;
                }
            }

            BlueSystemMessage(m_session, "Adding %u gold, %u silver, %u copper to %s's backpack...", gold, silver, copper, player_target->GetName());
            GreenSystemMessage(player_target->GetSession(), "%s added %u gold, %u silver, %u copper to your backpack.", m_session->GetPlayer()->GetName(), gold, silver, copper);
            sGMLog.writefromsession(m_session, "added %u gold, %u silver, %u copper to %s's backpack.", gold, silver, copper , player_target->GetName());
        }
        else
        {
            BlueSystemMessage(m_session, "Taking %u gold, %u silver, %u copper from %s's backpack...", gold, silver, copper, player_target->GetName());
            GreenSystemMessage(player_target->GetSession(), "%s took %u gold, %u silver, %u copper from your backpack.", m_session->GetPlayer()->GetName(), gold, silver, copper);
            sGMLog.writefromsession(m_session, "took %u gold, %u silver, %u copper from %s's backpack.", gold, silver, copper, player_target->GetName());
        }
    }

    player_target->SetGold(newgold);

    return true;
}

//.character add silver
bool ChatHandler::HandleCharAddSilverCommand(const char* args, WorldSession* m_session)
{
    if (*args == 0)
    {
        RedSystemMessage(m_session, "You must specify how many silver you will add.");
        RedSystemMessage(m_session, "1000 = 1 gold, 1 = 1 silver");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    int32 total = atoi(args) * 100;

    uint32 gold = (uint32)std::floor((float)int32abs(total) / 10000.0f);
    uint32 silver = (uint32)std::floor(((float)int32abs(total) / 100.0f)) % 100;

    int32 newgold = player_target->GetGold() + total;
    if (newgold < 0)
    {
        BlueSystemMessage(m_session, "Taking all gold from %s's backpack...", player_target->GetName());
        GreenSystemMessage(player_target->GetSession(), "%s took all gold from your backpack.", m_session->GetPlayer()->GetName());
        newgold = 0;
    }
    else
    {
        if (total >= 0)
        {
            if (sWorld.GoldCapEnabled)
            {
                if ((player_target->GetGold() + newgold) > sWorld.GoldLimit)
                {
                    RedSystemMessage(m_session, "Maximum amount of gold is %u and %s already has %u", (sWorld.GoldLimit / 10000), player_target->GetName(), (player_target->GetGold() / 10000));
                    return true;
                }
            }

            BlueSystemMessage(m_session, "Adding %u gold, %u silver to %s's backpack...", gold, silver, player_target->GetName());
            GreenSystemMessage(player_target->GetSession(), "%s added %u gold, %u silver to your backpack.", m_session->GetPlayer()->GetName(), gold, silver);
            sGMLog.writefromsession(m_session, "added %u gold, %u silver to %s's backpack.", gold, silver, player_target->GetName());
        }
        else
        {
            BlueSystemMessage(m_session, "Taking %u gold, %u silver from %s's backpack...", gold, silver, player_target->GetName());
            GreenSystemMessage(player_target->GetSession(), "%s took %u gold, %u silver from your backpack.", m_session->GetPlayer()->GetName(), gold, silver);
            sGMLog.writefromsession(m_session, "took %u gold, %u silver from %s's backpack.", gold, silver, player_target->GetName());
        }
    }

    player_target->SetGold(newgold);

    return true;
}

//.character add gold
bool ChatHandler::HandleCharAddGoldCommand(const char* args, WorldSession* m_session)
{
    if (*args == 0)
    {
        RedSystemMessage(m_session, "You must specify how many gold you will add.");
        RedSystemMessage(m_session, "1 = 1 gold.");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    int32 total = atoi(args) * 10000;

    uint32 gold = (uint32)std::floor((float)int32abs(total) / 10000.0f);

    int32 newgold = player_target->GetGold() + total;
    if (newgold < 0)
    {
        BlueSystemMessage(m_session, "Taking all gold from %s's backpack...", player_target->GetName());
        GreenSystemMessage(player_target->GetSession(), "%s took all gold from your backpack.", m_session->GetPlayer()->GetName());
        newgold = 0;
    }
    else
    {
        if (total >= 0)
        {
            if (sWorld.GoldCapEnabled)
            {
                if ((player_target->GetGold() + newgold) > sWorld.GoldLimit)
                {
                    RedSystemMessage(m_session, "Maximum amount of gold is %u and %s already has %u", (sWorld.GoldLimit / 10000), player_target->GetName(), (player_target->GetGold() / 10000));
                    return true;
                }
            }

            BlueSystemMessage(m_session, "Adding %u gold to %s's backpack...", gold, player_target->GetName());
            GreenSystemMessage(player_target->GetSession(), "%s added %u gold to your backpack.", m_session->GetPlayer()->GetName(), gold);
            sGMLog.writefromsession(m_session, "added %u gold to %s's backpack.", gold, player_target->GetName());
        }
        else
        {
            BlueSystemMessage(m_session, "Taking %u gold from %s's backpack...", gold, player_target->GetName());
            GreenSystemMessage(player_target->GetSession(), "%s took %u gold from your backpack.", m_session->GetPlayer()->GetName(), gold);
            sGMLog.writefromsession(m_session, "took %u gold from %s's backpack.", gold, player_target->GetName());
        }
    }

    player_target->SetGold(newgold);

    return true;
}

//.character resetskills
bool ChatHandler::HandleCharResetSkillsCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    selected_player->_RemoveAllSkills();

    PlayerCreateInfo const* player_info = sMySQLStore.GetPlayerCreateInfo(selected_player->getRace(), selected_player->getClass());
    if (player_info == nullptr)
        return true;

    for (std::list<CreateInfo_SkillStruct>::const_iterator ss = player_info->skills.begin(); ss != player_info->skills.end(); ++ss)
    {
        auto skill_line = sSkillLineStore.LookupEntry(ss->skillid);
        if (skill_line == nullptr)
            continue;

        if (skill_line->type != SKILL_TYPE_LANGUAGE && ss->skillid && ss->currentval && ss->maxval)
            selected_player->_AddSkillLine(ss->skillid, ss->currentval, ss->maxval);
    }

    selected_player->UpdateStats();
    selected_player->UpdateChances();
    selected_player->_UpdateMaxSkillCounts();
    selected_player->_AddLanguages(false);

    if (selected_player != m_session->GetPlayer())
    {
        SystemMessage(m_session, "Reset skills of %s.", selected_player->GetName());
        BlueSystemMessage(selected_player->GetSession(), "%s reset all your skills.", m_session->GetPlayer()->GetName());
        sGMLog.writefromsession(m_session, "reset skills of %s", selected_player->GetName());
    }
    else
    {
        BlueSystemMessage(m_session, "Your skills are reset to default.");
    }

    return true;
}

//.character removeitem
bool ChatHandler::HandleCharRemoveItemCommand(const char* args, WorldSession* m_session)
{
    uint32 item_id;
    int32 count, ocount;

    int argc = sscanf(args, "%u %u", (unsigned int*)&item_id, (unsigned int*)&count);
    if (argc == 1)
        count = 1;
    else if (argc != 2 || !count)
        return false;

    ocount = count;
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    int32 loop_count = 0;
    int32 start_count = selected_player->GetItemInterface()->GetItemCount(item_id, true);
    int32 start_count2 = start_count;
    if (count > start_count)
        count = start_count;

    while (start_count >= count && (count > 0) && loop_count < 20)
    {
        selected_player->GetItemInterface()->RemoveItemAmt(item_id, count);
        start_count2 = selected_player->GetItemInterface()->GetItemCount(item_id, true);
        count -= (start_count - start_count2);
        start_count = start_count2;
        ++loop_count;
    }

    ItemProperties const* item_properties = sMySQLStore.GetItemProperties(item_id);
    if (item_properties != nullptr)
    {
        if (selected_player != m_session->GetPlayer())
        {
            sGMLog.writefromsession(m_session, "used remove item %s (id: %u) count %u from %s", item_properties->Name.c_str(), item_id, ocount, selected_player->GetName());
            BlueSystemMessage(m_session, "Removing %u copies of item %s (id: %u) from %s's inventory.", ocount, GetItemLinkByProto(item_properties, m_session->language).c_str(), item_id, selected_player->GetName());
            BlueSystemMessage(selected_player->GetSession(), "%s removed %u copies of item %s from your inventory.", m_session->GetPlayer()->GetName(), ocount, GetItemLinkByProto(item_properties, selected_player->GetSession()->language).c_str());
        }
        else
        {
            BlueSystemMessage(m_session, "Removing %u copies of item %s (id: %u) from your inventory.", ocount, GetItemLinkByProto(item_properties, m_session->language).c_str(), item_id);
        }
    }
    else
    {
        RedSystemMessage(m_session, "Cannot remove non valid item id: %u .", item_id);
    }

    return true;
}

//.character resettalents
bool ChatHandler::HandleCharResetTalentsCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    selected_player->Reset_Talents();

    if (selected_player != m_session->GetPlayer())
    {
        SystemMessage(m_session, "Reset talents of %s.", selected_player->GetName());
        BlueSystemMessage(selected_player->GetSession(), "%s reset all your talents.", m_session->GetPlayer()->GetName());
        sGMLog.writefromsession(m_session, "reset talents of %s", selected_player->GetName());
    }
    else
    {
        BlueSystemMessage(m_session, "All your talents are reset.");
    }

    return true;
}

//.character advanceallskills
bool ChatHandler::HandleAdvanceAllSkillsCommand(const char* args, WorldSession* m_session)
{
    uint32 amt = args ? atol(args) : 0;
    if (!amt)
    {
        RedSystemMessage(m_session, "An amount to increment is required.");
        return true;
    }

    Player* selected_player = GetSelectedPlayer(m_session, false, true);
    if (selected_player == nullptr)
        return true;

    selected_player->_AdvanceAllSkills(amt);

    if (selected_player != m_session->GetPlayer())
    {
        GreenSystemMessage(selected_player->GetSession(), "%s advanced all your skill lines by %u points.", m_session->GetPlayer()->GetName(),  amt);
        sGMLog.writefromsession(m_session, "advanced all skills by %u on %s", amt, selected_player->GetName());
    }
    else
    {
        GreenSystemMessage(m_session, "Advanced all your skill lines by %u points.", amt);
    }

    return true;
}

//.character increaseweaponskill
bool ChatHandler::HandleCharIncreaseWeaponSkill(const char* args, WorldSession* m_session)
{
    char* pMin = strtok((char*)args, " ");
    uint32 cnt = 0;
    if (!pMin)
        cnt = 1;
    else
        cnt = atol(pMin);

    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    uint32 SubClassSkill = 0;

    Item* item = selected_player->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    ItemProperties const* proto = nullptr;
    if (item == nullptr)
        item = selected_player->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);

    if (item != nullptr)
        proto = item->GetItemProperties();

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
        RedSystemMessage(m_session, "Can't find skill ID!");
        return false;
    }

    uint32 skill = SubClassSkill;

    if (selected_player != m_session->GetPlayer())
    {
        BlueSystemMessage(selected_player->GetSession(), "%s modified your skill line %d. Advancing %d times.", m_session->GetPlayer()->GetName(), skill, cnt);
        BlueSystemMessage(m_session, "Modifying skill line %d. Advancing %d times for %s.", skill, cnt, selected_player->GetName());
        sGMLog.writefromsession(m_session, "increased weapon skill (%u) of %s by %u", skill, selected_player->GetName(), cnt);
    }
    else
    {
        BlueSystemMessage(m_session, "Modifying skill line %d. Advancing %d times.", skill, cnt);
    }

    if (!selected_player->_HasSkillLine(skill))
    {
        SystemMessage(m_session, "Does not have skill line %u, adding.", skill);
        selected_player->_AddSkillLine(skill, 1, 450);
    }
    else
    {
        selected_player->_AdvanceSkillLine(skill, cnt);
    }

    return true;
}

//.character resetreputation
bool ChatHandler::HandleCharResetReputationCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    selected_player->_InitialReputation();

    if (selected_player != m_session->GetPlayer())
    {
        SystemMessage(selected_player->GetSession(), "%s resets your reputation. Relog for changes to take effect.", m_session->GetPlayer()->GetName());
        sGMLog.writefromsession(m_session, "used reset reputation for %s", selected_player->GetName());
        SystemMessage(m_session, "Reputation reset for %s", selected_player->GetName());
    }
    else
    {
        SystemMessage(m_session, "Done. Relog for changes to take effect.");
    }

    return true;
}

//.character resetspells
bool ChatHandler::HandleCharResetSpellsCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    selected_player->Reset_Spells();

    if (selected_player != m_session->GetPlayer())
    {
        SystemMessage(m_session, "Reset spells of %s to level 1.", selected_player->GetName());
        BlueSystemMessage(selected_player->GetSession(), "%s reset all your spells to starting values.", m_session->GetPlayer()->GetName());
        sGMLog.writefromsession(m_session, "reset spells of %s", selected_player->GetName());
    }
    else
    {
        BlueSystemMessage(m_session, "Your spells are reset.");
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .character set commands
//.character set allexplored
bool ChatHandler::HandleCharSetAllExploredCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    SystemMessage(m_session, "%s has explored all zones now.", player_target->GetName());
    GreenSystemMessage(player_target->GetSession(), "%s sets all areas as explored for you.", m_session->GetPlayer()->GetName());
    sGMLog.writefromsession(m_session, "sets all areas as explored for player %s", player_target->GetName());

    for (uint8 i = 0; i < PLAYER_EXPLORED_ZONES_LENGTH; ++i)
    {
        player_target->SetFlag(PLAYER_EXPLORED_ZONES_1 + i, 0xFFFFFFFF);
    }

#ifdef ENABLE_ACHIEVEMENTS
    player_target->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA); // update
#endif
    return true;
}

//.character set gender
bool ChatHandler::HandleCharSetGenderCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    uint32 displayId = player_target->GetNativeDisplayId();
    uint8 gender;
    if (*args == 0)
    {
        gender = player_target->getGender() == 1 ? 0 : 1;
    }
    else
    {
        gender = (uint8)atoi((char*)args);
        if (gender > 1)
            gender = 1;
    }

    if (gender == player_target->getGender())
    {
        SystemMessage(m_session, "%s's gender is already set to %s(%u).", player_target->GetName(), gender ? "Female" : "Male", gender);
        return true;
    }
    else
    {
        player_target->setGender(gender);
        SystemMessage(m_session, "Set %s's gender to %s(%u).", player_target->GetName(), gender ? "Female" : "Male", gender);
    }

    if (player_target->getGender() == 0)
    {
        player_target->SetDisplayId((player_target->getRace() == RACE_BLOODELF) ? ++displayId : --displayId);
        player_target->SetNativeDisplayId(displayId);
    }
    else
    {
        player_target->SetDisplayId((player_target->getRace() == RACE_BLOODELF) ? --displayId : ++displayId);
        player_target->SetNativeDisplayId(displayId);
    }

    player_target->EventModelChange();

    return true;
}

//.character set itemsrepaired
bool ChatHandler::HandleCharSetItemsRepairedCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    for (uint8 i = 0; i < MAX_INVENTORY_SLOT; i++)
    {
        auto player_item = player_target->GetItemInterface()->GetInventoryItem(static_cast<uint16>(i));
        if (player_item != nullptr)
        {
            if (player_item->IsContainer())
            {
                auto item_container = static_cast<Container*>(player_item);
                for (uint32 j = 0; j < item_container->GetItemProperties()->ContainerSlots; ++j)
                {
                    player_item = item_container->GetItem(static_cast<uint16>(j));
                    if (player_item != nullptr)
                    {
                        player_item->SetDurabilityToMax();
                        player_item->m_isDirty = true;
                    }
                }
            }
            else
            {
                if (player_item->GetItemProperties()->MaxDurability > 0 && i < INVENTORY_SLOT_BAG_END && player_item->GetDurability() <= 0)
                {
                    player_item->SetDurabilityToMax();
                    player_item->m_isDirty = true;
                    player_target->ApplyItemMods(player_item, static_cast<uint16>(i), true);
                }
                else
                {
                    player_item->SetDurabilityToMax();
                    player_item->m_isDirty = true;
                }
            }
        }
    }

    if (player_target != m_session->GetPlayer())
    {
        BlueSystemMessage(m_session, "All items has been repaired for Player %s", player_target->GetName());
        GreenSystemMessage(player_target->GetSession(), "%s repaired all your items.", m_session->GetPlayer()->GetName());
        sGMLog.writefromsession(m_session, "repaired all items for player %s.", player_target->GetName());
    }
    else
    {
        BlueSystemMessage(m_session, "You repaired all your items.", player_target->GetName());
    }

    return true;
}

//.character set level
bool ChatHandler::HandleCharSetLevelCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    uint32 new_level = args ? atol(args) : 0;
    if (new_level == 0 || new_level > sWorld.m_levelCap)
    {
        RedSystemMessage(m_session, "Level %u is not a valid level! Check out your world.conf!", new_level);
        return true;
    }

    auto level_info = objmgr.GetLevelInfo(player_target->getRace(), player_target->getClass(), new_level);
    if (level_info == nullptr)
    {
        RedSystemMessage(m_session, "Level information not found in table playercreateinfo!");
        return true;
    }

    if (player_target != m_session->GetPlayer())
    {
        BlueSystemMessage(m_session, "Setting the level of %s to %u.", player_target->GetName(), new_level);
        GreenSystemMessage(player_target->GetSession(), "%s set your level to %u.", m_session->GetPlayer()->GetName(), new_level);
        sGMLog.writefromsession(m_session, "set level on %s, level %u", player_target->GetName(), new_level);
    }
    else
    {
        BlueSystemMessage(m_session, "You set your own level to %u.", new_level);
    }

    player_target->ApplyLevelInfo(level_info, new_level);

    if (player_target->getClass() == WARLOCK)
    {
        std::list<Pet*> player_summons = player_target->GetSummons();
        for (std::list<Pet*>::iterator itr = player_summons.begin(); itr != player_summons.end(); ++itr)
        {
            Pet* single_summon = *itr;
            if (single_summon->IsInWorld() && single_summon->isAlive())
            {
                single_summon->setLevel(new_level);
                single_summon->ApplyStatsForLevel();
                single_summon->UpdateSpellList();
            }
        }
    }

    return true;
}

//.character set name
bool ChatHandler::HandleCharSetNameCommand(const char* args, WorldSession* m_session)
{
    if (strlen(args) > 100)
        return false;

    char current_name[100];
    char new_name_cmd[100];

    if (sscanf(args, "%s %s", &current_name, &new_name_cmd) != 2)
        return false;

    static const char* bannedCharacters = "\t\v\b\f\a\n\r\\\"\'\? <>[](){}_=+-|/!@#$%^&*~`.,0123456789\0";
    static const char* allowedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    size_t nlen = strlen(new_name_cmd);

    for (size_t i = 0; i < nlen; ++i)
    {
        const char* p = allowedCharacters;
        for (; *p != 0; ++p)
        {
            if (new_name_cmd[i] == *p)
                goto cont;
        }
        RedSystemMessage(m_session, "That name is invalid or contains invalid characters.");
        return true;
    cont:
        continue;
    }

    for (size_t i = 0; i < nlen; ++i)
    {
        const char* p = bannedCharacters;
        while (*p != 0 && new_name_cmd[i] != *p && new_name_cmd[i] != 0)
            ++p;

        if (*p != 0)
        {
            RedSystemMessage(m_session, "That name is invalid or contains invalid characters.");
            return true;
        }
    }

    std::string new_name = new_name_cmd;
    PlayerInfo* pi = objmgr.GetPlayerInfoByName(current_name);
    if (pi == nullptr)
    {
        RedSystemMessage(m_session, "Player not found with this name.");
        return true;
    }

    if (objmgr.GetPlayerInfoByName(new_name.c_str()) != nullptr)
    {
        RedSystemMessage(m_session, "New name %s is already in use.", new_name.c_str());
        return true;
    }

    objmgr.RenamePlayerInfo(pi, pi->name, new_name.c_str());

    free(pi->name);
    pi->name = strdup(new_name.c_str());

    Player* plr = objmgr.GetPlayer(pi->guid);
    if (plr != nullptr)
    {
        plr->SetName(new_name);
        BlueSystemMessage(plr->GetSession(), "%s changed your name to '%s'.", m_session->GetPlayer()->GetName(), new_name.c_str());
        plr->SaveToDB(false);
    }
    else
    {
        CharacterDatabase.Execute("UPDATE characters SET name = '%s' WHERE guid = %u", CharacterDatabase.EscapeString(new_name).c_str(), (uint32)pi->guid);
    }

    GreenSystemMessage(m_session, "Changed name of '%s' to '%s'.", current_name, new_name_cmd);
    sGMLog.writefromsession(m_session, "renamed character %s (GUID: %u) to %s", current_name, pi->guid, new_name_cmd);
    sPlrLog.writefromsession(m_session, "GM renamed character %s (GUID: %u) to %s", current_name, pi->guid, new_name_cmd);
    return true;
}

//.character set phase
bool ChatHandler::HandleCharSetPhaseCommand(const char* args, WorldSession* m_session)
{
    uint32 phase = atoi(args);
    if (phase == 0)
    {
        RedSystemMessage(m_session, "No phase set. Use .character set phase <phase>");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    player_target->Phase(PHASE_SET, phase);

    if (player_target->GetSession())
    {
        WorldPacket data(SMSG_SET_PHASE_SHIFT, 4);
        data << phase;
        player_target->GetSession()->SendPacket(&data);
    }

    if (player_target != m_session->GetPlayer())
    {
        BlueSystemMessage(m_session, "Setting the phase of %s to %u.", player_target->GetName(), phase);
        GreenSystemMessage(player_target->GetSession(), "%s set your phase to %u.", m_session->GetPlayer()->GetName(), phase);
        sGMLog.writefromsession(m_session, "set phase on %s, phase %u", player_target->GetName(), phase);
    }
    else
    {
        BlueSystemMessage(m_session, "You set your own phase to %u.", phase);
    }

    return true;
}

//.character set speed
bool ChatHandler::HandleCharSetSpeedCommand(const char* args, WorldSession* m_session)
{
    float speed = atof(args);
    if (speed == 0.0f || speed > 255.0f || speed < 0.1f)
    {
        RedSystemMessage(m_session, "Invalid speed set. Value range 0.1f ... 255.0f Use .character set speed <speed>");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    if (player_target != m_session->GetPlayer())
    {
        BlueSystemMessage(m_session, "Setting the speed of %s to %3.2f.", player_target->GetName(), speed);
        GreenSystemMessage(player_target->GetSession(), "%s set your speed to %3.2f.", m_session->GetPlayer()->GetName(), speed);
        sGMLog.writefromsession(m_session, "modified speed of %s to %3.2f.", player_target->GetName(), speed);
    }
    else
    {
        BlueSystemMessage(m_session, "Setting your speed to %3.2f.", speed);
    }

    player_target->SetSpeeds(WALK, speed);
    player_target->SetSpeeds(RUN, speed);
    player_target->SetSpeeds(SWIM, speed);
    player_target->SetSpeeds(RUNBACK, speed);
    player_target->SetSpeeds(FLY, speed);

    return true;
}

//.character set standing
bool ChatHandler::HandleCharSetStandingCommand(const char* args, WorldSession* m_session)
{
    uint32 faction;
    int32 standing;

    if (sscanf(args, "%u %d", (unsigned int*)&faction, (unsigned int*)&standing) != 2)
    {
        RedSystemMessage(m_session, "No faction or standing value entered.");
        RedSystemMessage(m_session, "Use: .character set standstate <factionid> <standing>");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    player_target->SetStanding(faction, standing);

    if (player_target != m_session->GetPlayer())
    {
        BlueSystemMessage(m_session, "Setting standing of %u to %d on %s.", faction, standing, player_target->GetName());
        GreenSystemMessage(player_target->GetSession(), "%s set your standing of faction %u to %d.", m_session->GetPlayer()->GetName(), faction, standing);
        sGMLog.writefromsession(m_session, "set standing of faction %u to %u for %s", faction, standing, player_target->GetName());
    }
    else
    {
        BlueSystemMessage(m_session, "Setting standing of %u to %d on you.", faction, standing);
    }

    return true;
}

//.character set talentpoints
bool ChatHandler::HandleCharSetTalentpointsCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        RedSystemMessage(m_session, "No amount of talentpoints entered.");
        RedSystemMessage(m_session, "Use: .character set talentpoints <primary_amount> <secondary_amount>");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    uint32 primary_amount = 0;
    uint32 secondary_amnount = 0;

    std::stringstream ss(args);
    ss >> primary_amount;
    ss >> secondary_amnount;

    player_target->m_specs[SPEC_PRIMARY].SetTP(primary_amount);
    player_target->m_specs[SPEC_SECONDARY].SetTP(secondary_amnount);

    player_target->smsg_TalentsInfo(false);

    if (player_target != m_session->GetPlayer())
    {
        BlueSystemMessage(m_session, "Setting talentpoints primary: %u, secondary: %u for player %s.", primary_amount, secondary_amnount, player_target->GetName());
        GreenSystemMessage(player_target->GetSession(), "%s set your talenpoints to primary: %u, secondary: %u.", m_session->GetPlayer()->GetName(), primary_amount, secondary_amnount);
        sGMLog.writefromsession(m_session, "set talenpoints primary: %u, secondary: %u for player %s", primary_amount, secondary_amnount, player_target->GetName());
    }
    else
    {
        BlueSystemMessage(m_session, "Your talenpoints were set - primary: %u, secondary: %u.", primary_amount, secondary_amnount);
    }

    return true;
}

//.character set title
bool ChatHandler::HandleCharSetTitleCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    int32 title = atol(args);
    if (title > int32(PVPTITLE_END) || title < -int32(PVPTITLE_END))
    {
        RedSystemMessage(m_session, "Argument %i is out of range!", title);
        return false;
    }
    if (title == 0)
    {
        player_target->SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES, 0);
        player_target->SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES1, 0);
        player_target->SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES2, 0);
    }
    else if (title > 0)
    {
        player_target->SetKnownTitle(static_cast<RankTitles>(title), true);
    }
    else
    {
        player_target->SetKnownTitle(static_cast<RankTitles>(-title), false);
    }

    player_target->SetChosenTitle(0);  // better remove chosen one
    SystemMessage(m_session, "Title has been %s.", title > 0 ? "set" : "reset");

    std::stringstream logtext;
    logtext << "has ";
    if (title > 0)
        logtext << "set the title field of " << player_target->GetName() << "to " << title << ".";
    else
        logtext << "reset the title field of " << player_target->GetName();

    sGMLog.writefromsession(m_session, logtext.str().c_str());

    return true;
}

//.character set forcerename
bool ChatHandler::HandleCharSetForceRenameCommand(const char* args, WorldSession* m_session)
{
    if (strlen(args) > 100)
        return false;

    std::string tmp = std::string(args);
    PlayerInfo* pi = objmgr.GetPlayerInfoByName(tmp.c_str());
    if (pi == nullptr)
    {
        RedSystemMessage(m_session, "Player with that name not found.");
        return true;
    }

    Player* plr = objmgr.GetPlayer((uint32)pi->guid);
    if (plr == nullptr)
    {
        CharacterDatabase.Execute("UPDATE characters SET login_flags = %u WHERE guid = %u", (uint32)LOGIN_FORCED_RENAME, (uint32)pi->guid);
    }
    else
    {
        plr->login_flags = LOGIN_FORCED_RENAME;
        plr->SaveToDB(false);
        BlueSystemMessage(plr->GetSession(), "%s forced your character to be renamed next logon.", m_session->GetPlayer()->GetName());
    }

    CharacterDatabase.Execute("INSERT INTO banned_names VALUES('%s')", CharacterDatabase.EscapeString(std::string(pi->name)).c_str());
    GreenSystemMessage(m_session, "Forcing %s to rename his character next logon.", args);
    sGMLog.writefromsession(m_session, "forced %s to rename his charater (%u)", pi->name, pi->guid);
    return true;
}

//.character set customize
bool ChatHandler::HandleCharSetCustomizeCommand(const char* args, WorldSession* m_session)
{
    // prevent buffer overflow
    if (strlen(args) > 100)
        return false;

    std::string tmp = std::string(args);
    PlayerInfo* pi = objmgr.GetPlayerInfoByName(tmp.c_str());
    if (pi == nullptr)
    {
        RedSystemMessage(m_session, "Player with that name not found.");
        return true;
    }

    Player* plr = objmgr.GetPlayer((uint32)pi->guid);
    if (plr == nullptr)
    {
        CharacterDatabase.Execute("UPDATE characters SET login_flags = %u WHERE guid = %u", (uint32)LOGIN_CUSTOMIZE_LOOKS, (uint32)pi->guid);
    }
    else
    {
        plr->login_flags = LOGIN_CUSTOMIZE_LOOKS;
        plr->SaveToDB(false);
        BlueSystemMessage(plr->GetSession(), "%s flagged your character for customization at next login.", m_session->GetPlayer()->GetName());
    }

    GreenSystemMessage(m_session, "%s flagged to customize his character next logon.", args);
    sGMLog.writefromsession(m_session, "flagged %s for customization for charater (%u)", pi->name, pi->guid);
    return true;
}

//.character set factionchange
bool ChatHandler::HandleCharSetFactionChangeCommand(const char* args, WorldSession* m_session)
{
    // prevent buffer overflow
    if (strlen(args) > 100)
        return false;

    std::string tmp = std::string(args);
    PlayerInfo* pi = objmgr.GetPlayerInfoByName(tmp.c_str());
    if (pi == nullptr)
    {
        RedSystemMessage(m_session, "Player with that name not found.");
        return true;
    }

    Player* plr = objmgr.GetPlayer((uint32)pi->guid);
    if (plr == nullptr)
    {
        CharacterDatabase.Execute("UPDATE characters SET login_flags = %u WHERE guid = %u", (uint32)LOGIN_CUSTOMIZE_FACTION, (uint32)pi->guid);
    }
    else
    {
        plr->login_flags = LOGIN_CUSTOMIZE_FACTION;
        plr->SaveToDB(false);
        BlueSystemMessage(plr->GetSession(), "%s flagged your character for a faction change at next login.", m_session->GetPlayer()->GetName());
    }

    GreenSystemMessage(m_session, "%s flagged for a faction change next logon.", args);
    sGMLog.writefromsession(m_session, "flagged %s for a faction change for charater (%u)", pi->name, pi->guid);
    return true;
}

//.character set racechange
bool ChatHandler::HandleCharSetRaceChangeCommand(const char* args, WorldSession* m_session)
{
    // prevent buffer overflow
    if (strlen(args) > 100)
        return false;

    std::string tmp = std::string(args);
    PlayerInfo* pi = objmgr.GetPlayerInfoByName(tmp.c_str());
    if (pi == nullptr)
    {
        RedSystemMessage(m_session, "Player with that name not found.");
        return true;
    }

    Player* plr = objmgr.GetPlayer((uint32)pi->guid);
    if (plr == nullptr)
    {
        CharacterDatabase.Execute("UPDATE characters SET login_flags = %u WHERE guid = %u", (uint32)LOGIN_CUSTOMIZE_RACE, (uint32)pi->guid);
    }
    else
    {
        plr->login_flags = LOGIN_CUSTOMIZE_RACE;
        plr->SaveToDB(false);
        BlueSystemMessage(plr->GetSession(), "%s flagged your character for a race change at next login.", m_session->GetPlayer()->GetName());
    }

    GreenSystemMessage(m_session, "%s flagged for a race change next logon.", args);
    sGMLog.writefromsession(m_session, "flagged %s for a race change for charater (%u)", pi->name, pi->guid);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .character list commands
//.character list skills
bool ChatHandler::HandleCharListSkillsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    uint32 nobonus = 0;
    int32 bonus = 0;
    uint32 max = 0;

    BlueSystemMessage(m_session, "===== %s has skills =====", player_target->GetName());

    for (uint32 SkillId = 0; SkillId <= SkillNameManager->maxskill; SkillId++)
    {
        if (player_target->_HasSkillLine(SkillId))
        {
            char* SkillName = SkillNameManager->SkillNames[SkillId];
            if (!SkillName)
            {
                RedSystemMessage(m_session, "Invalid skill: %u", SkillId);
                continue;
            }
            nobonus = player_target->_GetSkillLineCurrent(SkillId, false);
            bonus = player_target->_GetSkillLineCurrent(SkillId, true) - nobonus;
            max = player_target->_GetSkillLineMax(SkillId);
            BlueSystemMessage(m_session, " %s: Value: %u, MaxValue: %u. (+ %d bonus)", SkillName, nobonus, max, bonus);
        }
    }
    return true;
}

//.character list standing
bool ChatHandler::HandleCharListStandingCommand(const char* args, WorldSession* m_session)
{
    uint32 faction = atoi(args);
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    int32 standing = player_target->GetStanding(faction);
    int32 bstanding = player_target->GetBaseStanding(faction);

    SystemMessage(m_session, "==== %s standing ====", player_target->GetName());
    SystemMessage(m_session, "Reputation for faction %u:", faction);
    SystemMessage(m_session, "Base Standing: %d", bstanding);
    SystemMessage(m_session, "Standing: %d", standing);
    return true;
}

//.character list items
bool ChatHandler::HandleCharListItemsCommand(const char* /*args*/, WorldSession* m_session)
{
    std::string q;

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    SystemMessage(m_session, "==== %s has items ====", player_target->GetName());
    int itemcount = 0;
    ItemIterator itr(player_target->GetItemInterface());
    itr.BeginSearch();
    for (; !itr.End(); itr.Increment())
    {
        if (!(*itr))
            return false;
        itemcount++;
        SendItemLinkToPlayer((*itr)->GetItemProperties(), m_session, true, player_target, m_session->language);
    }
    itr.EndSearch();

    return true;
}

//.character list kills
bool ChatHandler::HandleCharListKillsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    SystemMessage(m_session, "==== %s kills ====", player_target->GetName());
    SystemMessage(m_session, "All Kills: %u", player_target->m_killsLifetime);
    SystemMessage(m_session, "Kills today: %u", player_target->m_killsToday);
    SystemMessage(m_session, "Kills yesterday: %u", player_target->m_killsYesterday);

    return true;
}

//.character list instances
bool ChatHandler::HandleCharListInstanceCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    uint32 count = 0;
    std::stringstream ss;
    ss << "Show persistent instances of " << MSG_COLOR_CYAN << player_target->GetName() << "|r\n";
    player_target->getPlayerInfo()->savedInstanceIdsLock.Acquire();
    for (uint32 difficulty = 0; difficulty < NUM_INSTANCE_MODES; difficulty++)
    {
        for (PlayerInstanceMap::iterator itr = player_target->getPlayerInfo()->savedInstanceIds[difficulty].begin(); itr != player_target->getPlayerInfo()->savedInstanceIds[difficulty].end(); ++itr)
        {
            count++;
            ss << " - " << MSG_COLOR_CYAN << (*itr).second << "|r";
            MapInfo const* mapInfo = sMySQLStore.GetWorldMapInfo((*itr).first);
            if (mapInfo != NULL)
                ss << " (" << MSG_COLOR_CYAN << mapInfo->name << "|r)";
            Instance* pInstance = sInstanceMgr.GetInstanceByIds((*itr).first, (*itr).second);
            if (pInstance == NULL)
                ss << " - " << MSG_COLOR_RED << "Expired!|r";
            else
            {
                ss << " [" << GetMapTypeString(static_cast<uint8>(pInstance->m_mapInfo->type)) << "]";
                if (pInstance->m_mapInfo->type == INSTANCE_MULTIMODE)
                {
                    ss << " [" << GetDifficultyString(static_cast<uint8>(pInstance->m_difficulty)) << "]";
                }
                ss << " - ";
                if (pInstance->m_mapMgr == NULL)
                    ss << MSG_COLOR_LIGHTRED << "Shut Down|r";
                else
                {
                    if (!pInstance->m_mapMgr->HasPlayers())
                        ss << MSG_COLOR_LIGHTRED << "Idle|r";
                    else
                        ss << MSG_COLOR_GREEN << "In use|r";
                }
            }
            ss << "\n";
        }
    }
    player_target->getPlayerInfo()->savedInstanceIdsLock.Release();

    if (count == 0)
        ss << "Player is not assigned to any persistent instances.\n";
    else
        ss << "Player is assigned to " << MSG_COLOR_CYAN << count << "|r persistent instances.\n";

    SendMultilineMessage(m_session, ss.str().c_str());
    sGMLog.writefromsession(m_session, "used show instances command on %s,", player_target->GetName());
    return true;
}
