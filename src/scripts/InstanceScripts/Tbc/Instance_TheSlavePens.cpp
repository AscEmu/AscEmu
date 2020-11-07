/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TheSlavePens.h"
#include "Objects/Faction.h"

class TheSlavePensInstanceScript : public InstanceScript
{
public:

    explicit TheSlavePensInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TheSlavePensInstanceScript(pMapMgr); }


};

//////////////////////////////////////////////////////////////////////////////////////////
// Boss AIs

class TotemsAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TotemsAI)
    explicit TotemsAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        uint32_t Despawn = 30000;
        uint32_t AIUpdate = 1000;

        SpellID = 1;
        switch (getCreature()->getEntry())
        {
            case CN_MENNUS_HEALING_WARD:
                SpellID = 34977;
                break;
            case CN_TAINED_EARTHGRAB_TOTEM:
                SpellID = 20654;
                AIUpdate = 5000;
                break;
            case CN_TAINED_STONESKIN_TOTEM:
                Despawn = 60000;
                SpellID = 25509;    // temporary spell
                AIUpdate = 0;
                break;
            default:    // for Corrupted Nova Totem and it's also safe case
                {
                    getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
                    Despawn = 6000;
                    SpellID = 33132;
                    AIUpdate = 5000;
                }
        }

        if (AIUpdate != 0)
            RegisterAIUpdateEvent(AIUpdate);

        setAIAgent(AGENT_SPELL);
        getCreature()->Despawn(Despawn, 0);
        getCreature()->m_noRespawn = true;

        getCreature()->castSpell(getCreature(), SpellID, true);
    }

    void AIUpdate() override
    {
        if (getCreature()->hasUnitFlags(UNIT_FLAG_NON_ATTACKABLE))
            getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);

        getCreature()->castSpell(getCreature(), SpellID, true);
    }

protected:

    uint32_t SpellID;
};

uint32_t Totems[4] = { 20208, 18176, 18177, 14662 };

class MennuTheBetrayerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MennuTheBetrayerAI)

    bool SummonedTotems[4];

    explicit MennuTheBetrayerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        for (uint8_t i = 0; i < 4; i++)
            SummonedTotems[i] = false;

        auto lighningBolt = addAISpell(LIGHTNING_BOLT, 10.0f, TARGET_ATTACKING, 0, 15);
        lighningBolt->setAttackStopTimer(5000);

        auto healingWard = addAISpell(MENNUS_HEALING_WARD, 10.0f, TARGET_SELF, 0, 20, false, true);
        healingWard->setAttackStopTimer(1000);

        earthgrabTotem = addAISpell(TAINTED_EARTHGRAB_TOTEM, 0.0f, TARGET_SELF, 0, 20, false, true);
        earthgrabTotem->setAttackStopTimer(1000);

        stoneskinTotem = addAISpell(TAINTED_STONESKIN_TOTEM, 0.0f, TARGET_SELF, 0, 20, false, true);
        stoneskinTotem->setAttackStopTimer(1000);

        novaTotem = addAISpell(CORRUPTED_NOVA_TOTEM, 0.0f, TARGET_SELF, 0, 20, false, true);
        novaTotem->setAttackStopTimer(1000);

        TotemCounter = 0;

        addEmoteForEvent(Event_OnCombatStart, SAY_MENNU_BETRAYER_01);
        addEmoteForEvent(Event_OnCombatStart, SAY_MENNU_BETRAYER_02);
        addEmoteForEvent(Event_OnCombatStart, SAY_MENNU_BETRAYER_03);
        addEmoteForEvent(Event_OnTargetDied, SAY_MENNU_BETRAYER_04);
        addEmoteForEvent(Event_OnTargetDied, SAY_MENNU_BETRAYER_05);
        addEmoteForEvent(Event_OnDied, SAY_MENNU_BETRAYER_06);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        for (uint8_t i = 0; i < 4; i++)
            SummonedTotems[i] = false;

        TotemCounter = 0;
    }

    void AIUpdate() override
    {
        if (getScriptPhase() == 1 && TotemCounter != 0 && !getCreature()->isCastingSpell())
        {
            TotemSpawning();
        }
    }

    // Random totem spawning
    void TotemSpawning()
    {
        getCreature()->setAttackTimer(MELEE, 1500);

        bool Spawned = false;
        uint32_t Counter = 0;
        while(!Spawned)
        {
            if (Counter >= 2)
            {
                for (uint8_t i = 0; i < 4; i++)
                {
                    if (!SummonedTotems[i])
                    {
                        spawnCreature(Totems[i], getCreature()->GetPosition());
                        getCreature()->castSpell(getCreature(), earthgrabTotem->mSpellInfo, earthgrabTotem->mIsTriggered);
                        getCreature()->castSpell(getCreature(), stoneskinTotem->mSpellInfo, stoneskinTotem->mIsTriggered);
                        getCreature()->castSpell(getCreature(), novaTotem->mSpellInfo, novaTotem->mIsTriggered);
                        SummonedTotems[i] = true;
                        TotemCounter++;
                        break;
                    }
                }

                Spawned = true;
            }

            uint32_t i = Util::getRandomUInt(3);
            if (SummonedTotems[i])
                Counter++;
            else
            {
                spawnCreature(Totems[i], getCreature()->GetPosition());
                switch (i)
                {
                    case 1:
                        getCreature()->castSpell(getCreature(), earthgrabTotem->mSpellInfo, earthgrabTotem->mIsTriggered);
                        break;
                    case 2:
                        getCreature()->castSpell(getCreature(), stoneskinTotem->mSpellInfo, stoneskinTotem->mIsTriggered);
                        break;
                    case 3:
                        getCreature()->castSpell(getCreature(), novaTotem->mSpellInfo, novaTotem->mIsTriggered);
                        break;
                    default:
                        break;
                }

                SummonedTotems[i] = true;
                TotemCounter++;
                Spawned = true;
            }
        }

        if (TotemCounter == 4)
        {
            for (uint8_t i = 0; i < 4; i++)
                SummonedTotems[i] = false;

            TotemCounter = 0;
        }
    }

protected:

    uint32_t TotemCounter;
    CreatureAISpells* earthgrabTotem;
    CreatureAISpells* stoneskinTotem;
    CreatureAISpells* novaTotem;
};

class RokmarTheCracklerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(RokmarTheCracklerAI)
    explicit RokmarTheCracklerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto grievousWound = addAISpell(GRIEVOUS_WOUND, 8.0f, TARGET_ATTACKING, 0, 20, false, true);
        grievousWound->setAttackStopTimer(5000);

        auto waterSpit = addAISpell(WATER_SPIT, 16.0f, TARGET_VARIOUS, 0, 10);
        waterSpit->setAttackStopTimer(5000);

        auto ensnaringMoss = addAISpell(ENSNARING_MOSS, 8.0f, TARGET_RANDOM_SINGLE, 0, 35);
        ensnaringMoss->setAttackStopTimer(5000);
        ensnaringMoss->setMinMaxDistance(0.0f, 30.0f);

        enrage = addAISpell(ENRAGE, 0.0f, TARGET_SELF, 0, 0, false, true);
        enrage->setAttackStopTimer(5000);

        Enraged = false;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        Enraged = false;
    }

    void AIUpdate() override
    {
        if (getCreature()->getHealthPct() <= 20 && !Enraged && getScriptPhase() == 1)
        {
            getCreature()->castSpell(getCreature(), enrage->mSpellInfo, enrage->mIsTriggered);

            Enraged = true;
        }
    }

protected:

    bool Enraged;
    CreatureAISpells* enrage;
};

class QuagmirranAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(QuagmirranAI)
    explicit QuagmirranAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto acidGeyser = addAISpell(ACID_GEYSER, 10.0f, TARGET_RANDOM_SINGLE, 0, 20, false, true);
        acidGeyser->setAttackStopTimer(5000);
        acidGeyser->setMinMaxDistance(0.0f, 40.0f);

        auto poisonBoltVolley = addAISpell(POISON_BOLT_VOLLEY, 15.0f, TARGET_VARIOUS, 0, 10, false, true);
        poisonBoltVolley->setAttackStopTimer(2000);

        auto cleave = addAISpell(CLEAVE, 6.0f, TARGET_ATTACKING, 0, 15, false, true);
        cleave->setAttackStopTimer(2000);
    }
};

// \note Coilfang Slavemaster was already scripted in SteamVaults, so I haven't
// copied/pasted it here.
// Still many NPCs left and I don't have infos if any of those use any spell
void SetupTheSlavePens(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_CF_SLAVE_PENS, &TheSlavePensInstanceScript::Create);

    mgr->register_creature_script(CN_MENNUS_HEALING_WARD, &TotemsAI::Create);
    mgr->register_creature_script(CN_TAINED_EARTHGRAB_TOTEM, &TotemsAI::Create);
    mgr->register_creature_script(CN_TAINED_STONESKIN_TOTEM, &TotemsAI::Create);
    mgr->register_creature_script(CN_CORRUPTED_NOVA_TOTEM, &TotemsAI::Create);
    mgr->register_creature_script(CN_MENNU_THE_BETRAYER, &MennuTheBetrayerAI::Create);
    mgr->register_creature_script(CN_ROKMAR_THE_CRACKLER, &RokmarTheCracklerAI::Create);
    mgr->register_creature_script(CN_QUAGMIRRAN, &QuagmirranAI::Create);
}
