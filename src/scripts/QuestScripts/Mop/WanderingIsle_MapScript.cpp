/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

#include "WanderingIsle.hpp"

#include "Logging/Logger.hpp"
#include "Movement/MovementManager.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/LocationVector.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Map script: phase groups and area triggers of The Wandering Isle.
//
// Mop phasing works with sets of phase ids per object: objects without any phase are seen by
// everyone without a phase or with the default phase 169, everything else needs a common id.
// AscEmu keeps a single value per object, so the value is expanded to the set it stands for:
//  - 1 (spawn data default) = no phase
//  - a player in 524 (Zhao-Ren fight) still carries 169
//  - the class copies of the first quest (592..598) and the Li Fei phases (631/632) replace 169

// Area phasing: the academy (area 5834) shows a class copy of the first quests until the
// class variant of "The Lesson of the Iron Bough" has been rewarded.
struct AcademyClassPhase
{
    uint8_t playerClass;
    uint32_t phase;
    uint32_t quest;
};

const AcademyClassPhase academyClassPhases[7] =
{
    { WARRIOR, 592, 30038 },
    { MAGE, 593, 30033 },
    { HUNTER, 594, 30034 },
    { PRIEST, 595, 30035 },
    { ROGUE, 596, 30036 },
    { SHAMAN, 597, 30037 },
    { MONK, 598, 30027 }
};

const uint32_t AREA_SHANG_XI_ACADEMY = 5834;

class TheWanderingIsleScript : public InstanceScript
{
public:
    explicit TheWanderingIsleScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        modifyUpdateEvent(1000);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheWanderingIsleScript(pMapMgr); }

    void UpdateEvent() override
    {
        for (const auto& itr : getWorldMap()->getPlayers())
            updateAcademyPhase(itr.second);
    }

    bool arePhasesLinked(Object const* objectA, Object const* objectB) const override
    {
        const uint32_t phaseA = objectA->GetPhase();
        const uint32_t phaseB = objectB->GetPhase();

        if (phaseA == phaseB)
            return true;

        if (phaseA == PHASE_UNPHASED)
            return phaseB == PHASE_DEFAULT_PLAYER || carriesDefaultPhase(objectB);

        if (phaseB == PHASE_UNPHASED)
            return phaseA == PHASE_DEFAULT_PLAYER || carriesDefaultPhase(objectA);

        if (phaseA == PHASE_DEFAULT_PLAYER)
            return carriesDefaultPhase(objectB);

        if (phaseB == PHASE_DEFAULT_PLAYER)
            return carriesDefaultPhase(objectA);

        return false;
    }

    void OnAreaTrigger(Player* pPlayer, uint32_t pAreaId) override
    {
        switch (pAreaId)
        {
            case AT_DAWNING_SPAN:
                onDawningSpan(pPlayer);
                break;
            case AT_CHAMBER_OF_WHISPERS_ENTRANCE:
                onChamberOfWhispersEntrance(pPlayer);
                break;
            case AT_MANDORI_VILLAGE_WUGOU:
                onMandoriVillage(pPlayer, 55539, LocationVector(927.5729f, 3610.2399f, 196.4969f), 4000);
                break;
            case AT_MANDORI_VILLAGE_SHU:
                onMandoriVillage(pPlayer, 55558, LocationVector(880.8524f, 3606.0269f, 192.22139f), 7000);
                break;
            case AT_SHRINE_OF_INNER_LIGHT:
                onShrineOfInnerLight(pPlayer);
                break;
            case AT_DAWNING_VALLEY:
                onDawningValley(pPlayer);
                break;
            case AT_WU_SONG_VILLAGE:
                onWuSongVillage(pPlayer);
                break;
            case AT_FUS_POND:
                onFusPond(pPlayer);
                break;
            case AT_DAWNING_VALLEY_2:
                onDawningValley2(pPlayer);
                break;
            case AT_POOL_OF_REFLECTION:
                pPlayer->castSpell(pPlayer, 108590, true);
                break;
            case AT_THE_DAWNING_VALLEY:
                onTheDawningValley(pPlayer);
                break;
            case AT_TEMPLE_OF_FIVE_DAWNS:
                onTempleOfFiveDawnsTrigger(pPlayer);
                break;
            default:
                break;
        }
    }

private:
    // players keep 169 while the Zhao-Ren phase is applied on top of it, spawned objects in 524 do not
    static bool carriesDefaultPhase(Object const* object)
    {
        return object->isPlayer() && object->GetPhase() == PHASE_WIND_TEMPLE;
    }

    static void updateAcademyPhase(Player* player)
    {
        if (player == nullptr || !player->IsInWorld())
            return;

        for (const auto& classPhase : academyClassPhases)
        {
            if (classPhase.playerClass != player->getClass())
                continue;

            const bool wantsClassPhase = player->getAreaId() == AREA_SHANG_XI_ACADEMY && !player->hasQuestFinished(classPhase.quest);
            if (wantsClassPhase && player->GetPhase() != classPhase.phase)
            {
                sLogger.debug("WanderingIsle: player {} area {} phase {} -> class phase {}", player->getGuidLow(), player->getAreaId(), player->GetPhase(), classPhase.phase);
                player->setPhase(PHASE_SET, classPhase.phase);
            }
            else if (!wantsClassPhase && player->GetPhase() == classPhase.phase)
            {
                sLogger.debug("WanderingIsle: player {} area {} phase {} -> default phase {}", player->getGuidLow(), player->getAreaId(), player->GetPhase(), PHASE_DEFAULT_PLAYER);
                player->setPhase(PHASE_SET, PHASE_DEFAULT_PLAYER);
            }
            return;
        }
    }

    // at 8287
    void onDawningSpan(Player* player)
    {
        if (getIsleQuestStatus(player, QUEST_TEMPLE_OF_THE_FIVE_DAWNS) != ISLE_QUEST_COMPLETE || player->hasAurasWithId(116219))
            return;

        if (Creature* lorewalkerZan = findNearestCreature(player, 64885, 25.0f))
        {
            player->castSpell(player, 116219, true);
            lorewalkerZan->SendScriptTextChatMessage(TEXT_LOREWALKER_ZAN, player);
        }
    }

    // at 7041
    void onChamberOfWhispersEntrance(Player* player)
    {
        if (getIsleQuestStatus(player, QUEST_THE_CHAMBER_OF_WHISPERS) == ISLE_QUEST_INCOMPLETE && !player->hasAurasWithId(104571))
            player->castSpell(player, 104593, true);
    }

    // at 7858 (Wugou) / 7116 (Shu): the companions leave the player when Mandori Village is reached
    void onMandoriVillage(Player* player, uint32_t companionEntry, LocationVector const& leavePos, uint32_t despawnDelay)
    {
        if (!hasIsleQuestInLog(player, QUEST_THE_DAWNING_VALLEY))
            return;

        if (Creature* companion = findNearestCreature(player, companionEntry, 25.0f))
        {
            companion->getMovementManager()->movePoint(0, leavePos);
            companion->Despawn(despawnDelay, 0);
        }
    }

    // at 7736
    void onShrineOfInnerLight(Player* player)
    {
        if (getIsleQuestStatus(player, QUEST_THE_SPIRITS_GUARDIAN) != ISLE_QUEST_COMPLETE || player->hasAurasWithId(92571))
            return;

        if (Creature* huojinMonk = findNearestCreature(player, 60176, 15.0f))
        {
            huojinMonk->castSpell(player, 92571, true);
            huojinMonk->SendScriptTextChatMessage(TEXT_HUOJIN_MONK_SHRINE);
        }
    }

    // at 7750
    void onDawningValley(Player* player)
    {
        if (getIsleQuestStatus(player, QUEST_THE_PASSION_OF_SHEN_ZIN_SU) != ISLE_QUEST_INCOMPLETE || player->hasAurasWithId(116220))
            return;

        Creature* chiaHui = findNearestCreature(player, 60248, 45.0f);
        if (chiaHui == nullptr)
            return;

        // the villagers only greet Huo when he actually walks with the player
        if (findNearestCreature(chiaHui, 54958, 45.0f) != nullptr)
            chiaHui->SendScriptTextChatMessage(TEXT_CHIA_HUI);

        if (Creature* brewerLin = findNearestCreature(player, 60253, 45.0f))
        {
            if (findNearestCreature(brewerLin, 54958, 45.0f) != nullptr)
                brewerLin->SendScriptTextChatMessage(TEXT_BREWER_LIN);
        }

        chiaHui->castSpell(player, 116220, true);
    }

    // at 7749
    void onWuSongVillage(Player* player)
    {
        if (getIsleQuestStatus(player, QUEST_JI_OF_THE_HUOJIN) != ISLE_QUEST_COMPLETE || player->hasAurasWithId(116219))
            return;

        if (Creature* jiFirepaw = findNearestCreature(player, 54568, 15.0f))
        {
            jiFirepaw->castSpell(player, 116219, true);
            jiFirepaw->SendScriptTextChatMessage(TEXT_JI_TAKE_THAT);
        }
    }

    // at 7748
    void onFusPond(Player* player)
    {
        if (getIsleQuestStatus(player, QUEST_AYSA_OF_THE_TUSHUI) == ISLE_QUEST_COMPLETE)
        {
            // only the Lorvo standing at the pond (spawn 655948) speaks
            if (Creature* lorvo = findNearestCreature(player, 54943, 15.0f))
            {
                if (lorvo->isAlive() && lorvo->GetPositionX() < 1205.0f)
                {
                    lorvo->SendScriptTextChatMessage(TEXT_LORVO_SHHH);
                    return;
                }
            }
        }

        if (getIsleQuestStatus(player, QUEST_THE_MISSING_DRIVER) == ISLE_QUEST_COMPLETE)
        {
            if (Creature* aysa = findNearestCreature(player, 54567, 15.0f))
                aysa->SendScriptTextChatMessage(TEXT_AYSA_LORVO_PASSED, player);
        }
    }

    // at 7747
    void onDawningValley2(Player* player)
    {
        if (getIsleQuestStatus(player, QUEST_AYSA_OF_THE_TUSHUI) != ISLE_QUEST_COMPLETE || player->hasAurasWithId(116220))
            return;

        if (Creature* traineeGuang = findNearestCreature(player, 60244, 45.0f))
        {
            traineeGuang->SendScriptTextChatMessage(TEXT_TRAINEE_GUANG, player);
            traineeGuang->castSpell(player, 116220, true);
        }
    }

    // at 7746
    void onTheDawningValley(Player* player)
    {
        if (getIsleQuestStatus(player, QUEST_THE_DISCIPLES_CHALLENGE) != ISLE_QUEST_INCOMPLETE || player->hasAurasWithId(102429))
            return;

        if (Creature* traineeNim = findNearestCreature(player, 60183, 25.0f))
        {
            player->castSpell(player, 102429, false);
            traineeNim->SendScriptTextChatMessage(TEXT_TRAINEE_NIM, player);
        }
    }
};

void SetupWanderingIsle(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_WANDERING_ISLE, &TheWanderingIsleScript::Create);

    SetupWanderingIsleAcademy(mgr);
    SetupWanderingIsleCaveOfMeditation(mgr);
    SetupWanderingIsleTempleOfFiveDawns(mgr);
    SetupWanderingIsleSingingPools(mgr);
    SetupWanderingIsleChamberOfWhispers(mgr);
    SetupWanderingIsleWoodOfStaves(mgr);
}
