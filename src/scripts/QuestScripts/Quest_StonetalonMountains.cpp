/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/Script/CreatureAIScript.h"

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

        Creature* pCreature = mTarget->MAP_CREATURE_NEAREST_COORDS(SSX, SSY, SSZ, 11856);
        if (pCreature == nullptr)
        {
            return;
        }

        pCreature->m_escorter = mTarget;

        pCreature->getMovementManager()->movePath(pCreature->getWaypointPath(), false);
        pCreature->pauseMovement(10);

        pCreature->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Lets go");
        pCreature->setNpcFlags(UNIT_NPC_FLAG_NONE);
        // Prevention "not starting from spawn after attacking"
        pCreature->GetAIInterface()->setAllowedToEnterCombat(true);
        pCreature->SetFaction(1801);
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
