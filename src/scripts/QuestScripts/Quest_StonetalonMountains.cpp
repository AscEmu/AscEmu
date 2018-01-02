/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
        creat->setUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        // Prevention "not starting from spawn after attacking"
        creat->GetAIInterface()->SetAllowedToEnterCombat(true);
        creat->SetFaction(1801);
    }
};

class KayaFlathoof : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(KayaFlathoof);
    KayaFlathoof(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pCreature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
    }

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        switch (iWaypointId)
        {
            case 15:
            {
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Thanks for your help. I'll continue from here!");
                if (getCreature()->m_escorter == NULL)
                    return;
                Player* plr = getCreature()->m_escorter;
                getCreature()->m_escorter = NULL;

                auto quest_entry = plr->GetQuestLogForEntry(6523);
                if (quest_entry == nullptr)
                    return;
                quest_entry->SendQuestComplete();
            }break;
            case 17:
            {
                getCreature()->Despawn(5000, 1000);
            }break;
        }
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        if (getCreature()->m_escorter == NULL)
            return;
        Player* plr = getCreature()->m_escorter;
        getCreature()->m_escorter = NULL;

        auto quest_entry = plr->GetQuestLogForEntry(6523);
        if (quest_entry != nullptr)
            quest_entry->Fail(false);
    }
};


void SetupStonetalonMountains(ScriptMgr* mgr)
{
    mgr->register_quest_script(6523, new ProtectKaya());
    mgr->register_creature_script(11856, KayaFlathoof::Create);
}
