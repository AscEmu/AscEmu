/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_HellfireRamparts.h"

#include "Server/Script/CreatureAIScript.h"

class HellfireRampartsInstanceScript : public InstanceScript
{
public:
    explicit HellfireRampartsInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new HellfireRampartsInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

// \todo "Do you smell that? Fresh meat has somehow breached our citadel. Be wary of any intruders." should be on some areatrigger
class WatchkeeperGargolmarAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WatchkeeperGargolmarAI(c); }
    explicit WatchkeeperGargolmarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto surge = addAISpell(WATCHKEEPER_SURGE, 20.0f, TARGET_RANDOM_SINGLE, 0, 15);
        surge->addEmote("Back off, pup!", CHAT_MSG_MONSTER_YELL, 10330);
        surge->setMinMaxDistance(5.0f, 40.0f);;

        addAISpell(WATCHKEEPER_OVERPOWER, 10.0f, TARGET_ATTACKING, 0, 5);
        mRetaliation = addAISpell(WATCHKEEPER_RETALIATION, 0.0f, TARGET_SELF);

        if (_isHeroic())
            addAISpell(WATCHKEEPER_MORTAL_WOUND_H, 15.0f, TARGET_ATTACKING, 0, 12);
        else
            addAISpell(WATCHKEEPER_MORTAL_WOUND, 15.0f, TARGET_ATTACKING, 0, 12);

        mCalledForHelp = 0;
        _retaliation = false;

        addEmoteForEvent(Event_OnCombatStart, 4873);    // What have we here?
        addEmoteForEvent(Event_OnCombatStart, 4874);    // This may hurt a little....
        addEmoteForEvent(Event_OnCombatStart, 4875);    // I'm going to enjoy this...
        addEmoteForEvent(Event_OnTargetDied, 4876);     // Say farewell!
        addEmoteForEvent(Event_OnTargetDied, 4877);     // Much too easy.
        addEmoteForEvent(Event_OnDied, 4878);           // Hahah.. <cough> ..argh!
    }

    //case for scriptPhase
    void AIUpdate() override
    {
        if (getCreature()->getHealthPct() <= 40 && !mCalledForHelp)
        {
            sendDBChatMessage(4871);      // Heal me, quickly!
            mCalledForHelp = true;
        }

        if (getCreature()->getHealthPct() <= 20 && !_retaliation)
        {
            _retaliation = true;
            getCreature()->setAttackTimer(MELEE, 1500);
            _castAISpell(mRetaliation);
        }
    }

    bool mCalledForHelp;
    bool _retaliation;
    CreatureAISpells* mRetaliation;
};

class OmorTheUnscarredAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OmorTheUnscarredAI(c); }
    explicit OmorTheUnscarredAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pShield = addAISpell(OMOR_DEMONIC_SHIELD, 30.0f, TARGET_SELF, 0, 25);
        pShield->setMinMaxPercentHp(0, 20);

        auto pSummon = addAISpell(OMOR_SUMMON_FIENDISH_HOUND, 8.0f, TARGET_SELF, 1, 20);
        pSummon->addEmote("Achor-she-ki! Feast my pet! Eat your fill!", CHAT_MSG_MONSTER_YELL, 10277);

        pWhip = addAISpell(OMOR_SHADOW_WHIP, 10.0f, TARGET_RANDOM_SINGLE, 0, 30);
        pWhip->setMinMaxDistance(10.0f, 60.0f);

        if (!_isHeroic())
        {
            auto shadowBolt = addAISpell(OMOR_SHADOW_BOLT, 8.0f, TARGET_RANDOM_SINGLE, 3, 15, false, true);
            shadowBolt->setMinMaxDistance(10.0f, 60.0f);

            auto pAura = addAISpell(OMOR_TREACHEROUS_AURA, 8.0f, TARGET_RANDOM_SINGLE, 2, 35, false, true);
            pAura->setMinMaxDistance(0.0f, 60.0f);
            pAura->addEmote("A-Kreesh!", CHAT_MSG_MONSTER_YELL, 10278);
        }
        else
        {
            auto shadowBolt = addAISpell(OMOR_SHADOW_BOLT2, 8.0f, TARGET_RANDOM_SINGLE, 3, 15, false, true);
            shadowBolt->setMinMaxDistance(10.0f, 60.0f);

            auto pAura = addAISpell(OMOR_BANE_OF_TREACHERY, 8.0f, TARGET_RANDOM_SINGLE, 2, 35, false, true);
            pAura->setMinMaxDistance(0.0f, 60.0f);
            pAura->addEmote("A-Kreesh!", CHAT_MSG_MONSTER_YELL, 10278);
        }

        addEmoteForEvent(Event_OnCombatStart, 4856);     // I will not be defeated!
        addEmoteForEvent(Event_OnCombatStart, 4855);     // You dare stand against ME?
        addEmoteForEvent(Event_OnCombatStart, 4857);     // Your insolence will be your death!
        addEmoteForEvent(Event_OnTargetDied, 4860);      // Die, weakling!
        addEmoteForEvent(Event_OnDied, 4861);            // It is... not over.
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {           
        setRooted(true);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (isAlive())
            sendDBChatMessage(4862);     // I am victorious!
    }

    CreatureAISpells* pShield;
    CreatureAISpells* pWhip;
};

void SetupHellfireRamparts(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HC_RAMPARTS, &HellfireRampartsInstanceScript::Create);
    
    mgr->register_creature_script(CN_WATCHKEEPER_GARGOLMAR, &WatchkeeperGargolmarAI::Create);
    mgr->register_creature_script(CN_OMOR_THE_UNSCARRED, &OmorTheUnscarredAI::Create);
}
