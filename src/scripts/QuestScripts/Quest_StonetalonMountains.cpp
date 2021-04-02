/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

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

        Creature* creat = mTarget->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(SSX, SSY, SSZ, 11856);
        if (creat == NULL)
            return;
        creat->m_escorter = mTarget;
        creat->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_QUEST);
        creat->GetAIInterface()->StopMovement(10);
        creat->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Lets go");
        creat->setNpcFlags(UNIT_NPC_FLAG_NONE);
        // Prevention "not starting from spawn after attacking"
        creat->GetAIInterface()->SetAllowedToEnterCombat(true);
        creat->SetFaction(1801);
    }
};

class KayaFlathoof : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KayaFlathoof)
    explicit KayaFlathoof(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pCreature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
    }

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        switch (iWaypointId)
        {
            case 15:
            {
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Thanks for your help. I'll continue from here!");
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
