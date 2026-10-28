/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/QuestLogEntry.hpp"
#include "Management/QuestProperties.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/WorldSession.h"

#include <cstdint>
#include <list>
#include <vector>

class ScriptMgr;

//////////////////////////////////////////////////////////////////////////////////////////
// The Wandering Isle (map 860, zone 5736) - pandaren starting zone.

enum WanderingIsleMap
{
    MAP_THE_WANDERING_ISLE = 860,
    ZONE_THE_WANDERING_ISLE = 5736
};

// Phase ids used by the isle (Mop phase ids, not a bitmask - see TheWanderingIsleScript::arePhasesLinked)
enum WanderingIslePhases
{
    PHASE_UNPHASED = 1,                 // spawn data default, visible to everyone
    PHASE_DEFAULT_PLAYER = 169,         // default player phase
    PHASE_WIND_TEMPLE = 524,            // Zhao-Ren fight
    PHASE_STARTING_QUEST_FIRST = 592,   // 592..598 - per class copy of Master Shang Xi's first quest
    PHASE_STARTING_QUEST_LAST = 598,
    PHASE_LI_FEI_FIGHT = 631,           // quest 29421
    PHASE_LI_FEI_DONE = 632             // quest 29422
};

enum WanderingIsleAreaTriggers
{
    AT_CHAMBER_OF_WHISPERS_ENTRANCE = 7041,
    AT_MANDORI_VILLAGE_SHU = 7116,
    AT_SHRINE_OF_INNER_LIGHT = 7736,
    AT_THE_DAWNING_VALLEY = 7746,
    AT_DAWNING_VALLEY_2 = 7747,
    AT_FUS_POND = 7748,
    AT_WU_SONG_VILLAGE = 7749,
    AT_DAWNING_VALLEY = 7750,
    AT_POOL_OF_REFLECTION = 7783,
    AT_TEMPLE_OF_FIVE_DAWNS = 7835,
    AT_MANDORI_VILLAGE_WUGOU = 7858,
    AT_DAWNING_SPAN = 8287
};

enum WanderingIsleQuests
{
    QUEST_MUCH_TO_LEARN = 29404,
    QUEST_LESSON_OF_THE_IRON_STAFF = 29405,
    QUEST_DISCIPLE_OF_THE_SANDY_FIST = 29406,
    QUEST_THE_LESSON_OF_THE_BURNING_SCROLL = 29408,
    QUEST_THE_DISCIPLES_CHALLENGE = 29409,
    QUEST_AYSA_OF_THE_TUSHUI = 29410,
    QUEST_THE_WAY_OF_THE_TUSHUI = 29414,
    QUEST_THE_SPIRITS_GUARDIAN = 29420,
    QUEST_THE_MISSING_DRIVER = 29419,
    QUEST_ONLY_THE_WORTHY_SHALL_PASS = 29421,
    QUEST_HUO_THE_SPIRIT_OF_FIRE = 29422,
    QUEST_THE_PASSION_OF_SHEN_ZIN_SU = 29423,
    QUEST_JI_OF_THE_HUOJIN = 29522,
    QUEST_THE_LESSON_OF_STIFLED_PRIDE = 29524,
    QUEST_THE_SOURCE_OF_OUR_LIVELIHOOD = 29679,
    QUEST_NOT_IN_THE_FACE = 29774,
    QUEST_THE_DAWNING_VALLEY = 29775,
    QUEST_TEMPLE_OF_THE_FIVE_DAWNS = 29776,
    QUEST_BATTLE_FOR_THE_SKIES = 29786,
    QUEST_WORTHY_OF_PASSING = 29787,
    QUEST_PASSING_WISDOM = 29790,
    QUEST_THE_SUFFERING_OF_SHEN_ZIN_SU = 29791,
    QUEST_THE_CHAMBER_OF_WHISPERS = 29785
};

// npc_script_text entries of the isle (sql 20260909-03)
enum WanderingIsleTexts
{
    TEXT_ASPIRING_TRAINEE_TARGET_FIRST = 11000,     // 53565 group 0 id 0..7 (11000..11007)
    TEXT_ASPIRING_TRAINEE_CHAT_FIRST = 11008,       // 53565 group 1..7 (11008..11014)
    TEXT_AYSA_LORVO_PASSED = 11015,                 // 54567
    TEXT_JI_TAKE_THAT = 11016,                      // 54568
    TEXT_HUOJIN_TRAINEE_YIELD_FIRST = 11017,        // 54586 (11017..11019)
    TEXT_TUSHUI_TRAINEE_YIELD_FIRST = 11020,        // 54587 (11020..11022)
    TEXT_JAOMIN_RO_CHALLENGER = 11023,              // 54611
    TEXT_JAOMIN_RO_WELL_FOUGHT = 11024,             // 54611
    TEXT_LORVO_SHHH = 11025,                        // 54943
    TEXT_AYSA_POOL_FUN = 11026,                     // 54975
    TEXT_AYSA_MEDITATION_START = 11027,             // 59642
    TEXT_AYSA_MEDITATION_END = 11028,               // 59642
    TEXT_HUOJIN_MONK_SHRINE = 11029,                // 60176
    TEXT_TRAINEE_NIM = 11030,                       // 60183
    TEXT_TRAINEE_GUANG = 11031,                     // 60244
    TEXT_CHIA_HUI = 11032,                          // 60248
    TEXT_BREWER_LIN = 11033,                        // 60253
    TEXT_LOREWALKER_ZAN = 11034,                    // 64885
    TEXT_ASPIRING_TRAINEE_2_TARGET_FIRST = 11035,   // 65469 (11035..11038)
    TEXT_HUOJIN_TRAINEE_2_YIELD_FIRST = 11039,      // 65470 (11039..11041)
    TEXT_TUSHUI_TRAINEE_2_YIELD_FIRST = 11042       // 65471 (11042..11044)
};

// quest status as the isle scripts need it
enum WanderingIsleQuestStatus
{
    ISLE_QUEST_NONE,
    ISLE_QUEST_INCOMPLETE,
    ISLE_QUEST_COMPLETE,
    ISLE_QUEST_REWARDED
};

inline WanderingIsleQuestStatus getIsleQuestStatus(Player* player, uint32_t questId)
{
    if (player->hasQuestFinished(questId))
        return ISLE_QUEST_REWARDED;

    if (QuestLogEntry const* questLog = player->getQuestLogByQuestId(questId))
        return questLog->canBeFinished() ? ISLE_QUEST_COMPLETE : ISLE_QUEST_INCOMPLETE;

    return ISLE_QUEST_NONE;
}

inline bool hasIsleQuestInLog(Player* player, uint32_t questId)
{
    const auto status = getIsleQuestStatus(player, questId);
    return status == ISLE_QUEST_INCOMPLETE || status == ISLE_QUEST_COMPLETE;
}

// kill credit: the credit entry is one of the quest's ReqKillMobOrGOId slots
inline void giveIsleQuestCredit(Player* player, uint32_t questId, uint8_t reqIndex)
{
    if (player->hasQuestInQuestLog(questId))
        player->addQuestKill(questId, reqIndex);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Common base for the isle creature scripts. The academy exists once per phase (1, 592..598) at
// identical positions, so every nearby-search has to stay inside the own phase.
class WanderingIsleCreatureAI : public CreatureAIScript
{
public:
    explicit WanderingIsleCreatureAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void getCreaturesInPhase(uint32_t entry, float range, std::list<Creature*>& result)
    {
        std::list<Creature*> candidates;
        GetCreatureListWithEntryInGrid(candidates, entry, range);
        for (Creature* candidate : candidates)
        {
            if (candidate != getCreature() && candidate->isAlive() && getCreature()->isInSamePhase(candidate))
                result.push_back(candidate);
        }
    }

    Creature* findNearestCreatureInPhase(uint32_t entry, float range)
    {
        std::list<Creature*> candidates;
        getCreaturesInPhase(entry, range, candidates);

        Creature* nearest = nullptr;
        float nearestDistance = range;
        for (Creature* candidate : candidates)
        {
            const float distance = getCreature()->getDistance2d(candidate);
            if (nearest == nullptr || distance < nearestDistance)
            {
                nearest = candidate;
                nearestDistance = distance;
            }
        }
        return nearest;
    }

    // players in range and phase, optionally filtered by quest status
    std::vector<Player*> getPlayersInPhase(float range, uint32_t questId = 0, WanderingIsleQuestStatus status = ISLE_QUEST_NONE)
    {
        std::vector<Player*> result;
        for (Object* object : getCreature()->getInRangePlayersSet())
        {
            Player* player = static_cast<Player*>(object);
            if (player == nullptr || !player->isAlive() || !getCreature()->isInSamePhase(player) || getCreature()->getDistance2d(player) > range)
                continue;

            if (questId != 0 && getIsleQuestStatus(player, questId) != status)
                continue;

            result.push_back(player);
        }
        return result;
    }
};

// areatrigger 7835, implemented with the Temple of Five Dawns chapter
void onTempleOfFiveDawnsTrigger(Player* player);

void SetupWanderingIsleAcademy(ScriptMgr* mgr);
void SetupWanderingIsleCaveOfMeditation(ScriptMgr* mgr);
void SetupWanderingIsleTempleOfFiveDawns(ScriptMgr* mgr);
void SetupWanderingIsleSingingPools(ScriptMgr* mgr);
void SetupWanderingIsleChamberOfWhispers(ScriptMgr* mgr);
void SetupWanderingIsleWoodOfStaves(ScriptMgr* mgr);
