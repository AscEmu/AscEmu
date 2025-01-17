/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/QuestLogEntry.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Movement/MovementManager.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Quest: "Protect Kaya" (Entry: 6523)
// Kaya Flathoof (Entry: 11856)
class ProtectKaya : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        float SSX = mTarget->GetPositionX();
        float SSY = mTarget->GetPositionY();
        float SSZ = mTarget->GetPositionZ();

        Creature* creat = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(SSX, SSY, SSZ, 11856);
        if (creat == nullptr)
            return;
        creat->m_escorter = mTarget;

        creat->getMovementManager()->movePath(creat->getWaypointPath(), false);
        creat->pauseMovement(10);

        creat->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Lets go");
        creat->setNpcFlags(UNIT_NPC_FLAG_NONE);
        // Prevention "not starting from spawn after attacking"
        creat->getAIInterface()->setAllowedToEnterCombat(true);
        creat->setFaction(1801);
    }
};

class KayaFlathoof : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KayaFlathoof(c); }
    explicit KayaFlathoof(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        stopMovement();
    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        switch (iWaypointId)
        {
            case 15:
            {
                getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Thanks for your help. I'll continue from here!");
                if (getCreature()->m_escorter == nullptr)
                    return;

                Player* player = getCreature()->m_escorter;
                getCreature()->m_escorter = nullptr;

                if (auto* questLog = player->getQuestLogByQuestId(6523))
                    questLog->sendQuestComplete();
            }break;
            case 17:
            {
                getCreature()->Despawn(5000, 1000);
            }break;
        }
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        if (Player* player = getCreature()->m_escorter)
        {
            getCreature()->m_escorter = nullptr;

            if (auto* questLog = player->getQuestLogByQuestId(6523))
                questLog->sendQuestFailed();
        }
    }
};

void SetupStonetalonMountains(ScriptMgr* mgr)
{
    mgr->register_quest_script(6523, new ProtectKaya());
    mgr->register_creature_script(11856, KayaFlathoof::Create);
}
