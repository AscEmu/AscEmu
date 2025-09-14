/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"

class LazyPeon : public CreatureAIScript
{
    static constexpr uint32_t  QUEST_LAZY_PEONS = 5441;
    static constexpr uint32_t  GO_LUMBERPILE = 175784;
    static constexpr uint32_t  SPELL_BUFF_SLEEP = 17743;
    static constexpr uint32_t  SPELL_AWAKEN_PEON = 19938;


public:
    static CreatureAIScript* Create(Creature* creature) { return new LazyPeon(creature); }
    explicit LazyPeon(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        work = false;

        addAIFunction([this](CreatureAIFunc pThis)
            {
                castSpellOnSelf(SPELL_BUFF_SLEEP);
                repeatFunctionFromScheduler(pThis, 3min);
            }, DoOnceScheduler(5s));
    }

    void AIUpdate(unsigned long /*time_passed*/) override
    {
        // when at woodpile do chopping emote
        if (work)
            getCreature()->emote(EMOTE_ONESHOT_WORK_CHOPWOOD);
    }

    void OnReachWP(uint32_t /*type*/, uint32_t id) override
    {
        // Woodpile Reached, prepare flag for emote
        if (id == 1)
            work = true;
    }

    void OnHitBySpell(uint32_t spellId, Unit* caster) override
    {
        if (!caster || !caster->isPlayer())
            return;

        if (spellId != SPELL_AWAKEN_PEON)
            return;

        handleWakeUp(caster);
    }

    void handleWakeUp(Unit* caster)
    {
        // Remove Zzz aura
        _removeAura(SPELL_BUFF_SLEEP);

        // Send chat Message
        sendDBChatMessageByIndex(0, caster);

        // Quest Credit for Player
        if (auto* questLog = caster->ToPlayer()->getQuestLogByQuestId(QUEST_LAZY_PEONS))
        {
            if (questLog->getMobCountByIndex(0) < 5)
            {
                questLog->setMobCountForIndex(0, questLog->getMobCountByIndex(0) + 1);
                questLog->sendUpdateAddKill(0);
                questLog->updatePlayerFields();
            }
        }

        // Move to closest wood pile
        if (GameObject* Lumberpile = findNearestGameObject(GO_LUMBERPILE, 20.0f))
            movePoint(1, Lumberpile->GetPositionX() - 1, Lumberpile->GetPositionY(), Lumberpile->GetPositionZ());
    }

protected:
    bool work{ false };
};

void SetupDurotar(ScriptMgr* mgr)
{
    mgr->register_creature_script(10556, &LazyPeon::Create);
}
