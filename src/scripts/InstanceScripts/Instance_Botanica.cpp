/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "Setup.h"
#include "Instance_Botanica.h"

// Bloodwarder Protector AI
class BloodProtectorAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(BloodProtectorAI);
        BloodProtectorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto crystalStrike = addAISpell(CRYSTAL_STRIKE, 10.0f, TARGET_ATTACKING);
            crystalStrike->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// Bloodwarder Mender AI
// \todo Script me!

    //Healer
    //Casts Shadow Word: Pain and Mind Blast
    //Mind Control these for Holy Fury buff (+295 spell damage for 30 minutes, shows as DIVINE fury on the pet bar). Can be spellstolen.

// Bloodwarder Greenkeeper AI
class BloodGreenkeeperAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(BloodGreenkeeperAI);
        BloodGreenkeeperAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto greenkeeperFury = addAISpell(GREENKEEPER_FURY, 10.0f, TARGET_ATTACKING);
            greenkeeperFury->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// Sunseeker Chemist AI
class SunchemistAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SunchemistAI);
        SunchemistAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto flameBreath = addAISpell(FLAME_BREATH, 10.0f, TARGET_VARIOUS);
            flameBreath->setAttackStopTimer(1000);

            auto poisonCloud = addAISpell(POISON_CLOUD, 5.0f, TARGET_VARIOUS);
            poisonCloud->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// Sunseeker Researcher AI
class SunResearcherAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SunResearcherAI);
        SunResearcherAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            poisonShield = addAISpell(POISON_SHIELD, 0.0f, TARGET_SELF);
            poisonShield->setAttackStopTimer(1000);

            auto mindShock = addAISpell(MIND_SHOCK, 5.0f, TARGET_ATTACKING);
            mindShock->setAttackStopTimer(2000);

            auto frostShock = addAISpell(FROST_SHOCK, 5.0f, TARGET_ATTACKING);
            frostShock->setAttackStopTimer(2000);

            auto flameShock = addAISpell(FLAME_SHOCK, 10.0f, TARGET_ATTACKING);
            flameShock->setAttackStopTimer(1000);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            getCreature()->CastSpell(getCreature(), poisonShield->mSpellInfo, true);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

    protected:

        CreatureAISpells* poisonShield;
};


// Commander Sarannis AI
class CommanderSarannisAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CommanderSarannisAI);
        CommanderSarannisAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            GuardAdds = false;

            enableCreatureAISpellSystem = true;

            auto arcaneResonance = addAISpell(ARCANE_RESONANCE, 7.0f, TARGET_ATTACKING);
            arcaneResonance->setAttackStopTimer(1000);
            arcaneResonance->addDBEmote(SAY_COMMANDER_SARANNIS_04);
            arcaneResonance->addDBEmote(SAY_COMMANDER_SARANNIS_05);

            auto arcaneDevastation = addAISpell(ARCANE_DEVASTATION, 15.0f, TARGET_ATTACKING);
            arcaneDevastation->setAttackStopTimer(1000);

            summonReinforcements = addAISpell(SUMMON_REINFORCEMENTS, 0.0f, TARGET_SELF);
            summonReinforcements->setAttackStopTimer(2000);
            summonReinforcements->addDBEmote(SAY_COMMANDER_SARANNIS_06);

            addEmoteForEvent(Event_OnCombatStart, SAY_COMMANDER_SARANNIS_01);
            addEmoteForEvent(Event_OnTargetDied, SAY_COMMANDER_SARANNIS_02);
            addEmoteForEvent(Event_OnTargetDied, SAY_COMMANDER_SARANNIS_03);
            addEmoteForEvent(Event_OnDied, SAY_COMMANDER_SARANNIS_07);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            GuardAdds = false;
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            GuardAdds = false;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            GuardAdds = false;
        }

        void AIUpdate() override
        {
            if (getCreature()->GetHealthPct() <= 50 && GuardAdds == false)
            {
                GuardAdds = true;
                getCreature()->CastSpell(getCreature(), summonReinforcements->mSpellInfo, true);
            }
        }

    protected:

        bool GuardAdds;
        CreatureAISpells* summonReinforcements;
};


// High Botanist Freywinn AI
class HighBotanistFreywinnAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HighBotanistFreywinnAI);
        HighBotanistFreywinnAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            PlantTimer = 10;

            enableCreatureAISpellSystem = true;

            plantRedSeedling = addAISpell(PLANT_RED_SEEDLING, 0.0f, TARGET_SELF);
            plantRedSeedling->setAttackStopTimer(1000);

            plantGreenSeedling = addAISpell(PLANT_GREEN_SEEDLING, 0.0f, TARGET_SELF);
            plantGreenSeedling->setAttackStopTimer(1000);

            plantWhiteSeedling = addAISpell(PLANT_WHITE_SEEDLING, 0.0f, TARGET_SELF);
            plantWhiteSeedling->setAttackStopTimer(1000);

            plantBlueSeedling = addAISpell(PLANT_BLUE_SEEDLING, 0.0f, TARGET_SELF);
            plantBlueSeedling->setAttackStopTimer(1000);

            auto summonFrayerProtector = addAISpell(SUMMON_FRAYER_PROTECTOR, 5.0f, TARGET_SELF);
            summonFrayerProtector->setAttackStopTimer(1000);

            auto treeForm = addAISpell(TREE_FORM, 5.0f, TARGET_SELF, 0, 40, false, true);
            treeForm->setAttackStopTimer(1000);
            treeForm->addDBEmote(SAY_HIGH_BOTANIS_FREYWIN_01);
            treeForm->addDBEmote(SAY_HIGH_BOTANIS_FREYWIN_05);

            auto tranquility = addAISpell(TRANQUILITY, 0.0f, TARGET_VARIOUS);
            tranquility->setAttackStopTimer(1000);

            addEmoteForEvent(Event_OnCombatStart, SAY_HIGH_BOTANIS_FREYWIN_04);
            addEmoteForEvent(Event_OnTargetDied, SAY_HIGH_BOTANIS_FREYWIN_03);
            addEmoteForEvent(Event_OnTargetDied, SAY_HIGH_BOTANIS_FREYWIN_02);
            addEmoteForEvent(Event_OnDied, SAY_HIGH_BOTANIS_FREYWIN_06);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            PlantTimer = 10;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            PlantTimer = 10;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            PlantTimer = 10;
        }

        void AIUpdate() override
        {
            PlantTimer--;
            if (!PlantTimer)
            {
                PlantColorSeedling();
            }
        }

        void PlantColorSeedling()
        {
            PlantTimer = RandomUInt(5, 10);    //5-10 sec (as in my DB attack time is 1000)

            switch (RandomUInt(3))
            {
                case 0:
                {
                    getCreature()->CastSpell(getCreature(), plantRedSeedling->mSpellInfo, true);
                }
                break;
                case 1:
                {
                    getCreature()->CastSpell(getCreature(), plantGreenSeedling->mSpellInfo, true);
                }
                break;
                case 2:
                {
                    getCreature()->CastSpell(getCreature(), plantWhiteSeedling->mSpellInfo, true);
                }
                break;
                case 3:
                {
                    getCreature()->CastSpell(getCreature(), plantBlueSeedling->mSpellInfo, true);
                }
                break;
            }
        }

    protected:

        uint32 PlantTimer;

        CreatureAISpells* plantRedSeedling;
        CreatureAISpells* plantGreenSeedling;
        CreatureAISpells* plantWhiteSeedling;
        CreatureAISpells* plantBlueSeedling;
};


// Thorngrin the Tender AI
class ThorngrinTheTenderAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ThorngrinTheTenderAI);
        ThorngrinTheTenderAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            Enraged = false;

            enableCreatureAISpellSystem = true;

            auto hellfire = addAISpell(HELLFIRE, 9.0f, TARGET_VARIOUS);
            hellfire->setAttackStopTimer(1000);
            hellfire->addDBEmote(SAY_THORNIN_06);
            hellfire->addDBEmote(SAY_THORNIN_07);

            auto sacrifice = addAISpell(SACRIFICE, 6.0f, TARGET_ATTACKING);
            sacrifice->setAttackStopTimer(1000);
            sacrifice->addDBEmote(SAY_THORNIN_04);
            sacrifice->addDBEmote(SAY_THORNIN_05);

            enrage = addAISpell(ENRAGE, 0.0f, TARGET_SELF);
            enrage->setAttackStopTimer(1000);

            addEmoteForEvent(Event_OnCombatStart, SAY_THORNIN_01);
            addEmoteForEvent(Event_OnTargetDied, SAY_THORNIN_02);
            addEmoteForEvent(Event_OnTargetDied, SAY_THORNIN_03);
            addEmoteForEvent(Event_OnDied, SAY_THORNIN_08);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            Enraged = false;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            Enraged = false;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            Enraged = false;
        }

        void AIUpdate() override
        {
            if (getCreature()->GetHealthPct() <= 20 && Enraged == false)
            {
                Enraged = true;
                getCreature()->CastSpell(getCreature(), enrage->mSpellInfo, true);
            }
        }

    protected:

        bool Enraged;
        CreatureAISpells* enrage;
};


// Laj AI
class LajAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LajAI);
        LajAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            TeleportTimer = 20;    // It's sth about that

            enableCreatureAISpellSystem = true;

            auto allergicReaction = addAISpell(ALERGIC_REACTION, 10.0f, TARGET_ATTACKING);
            allergicReaction->setAttackStopTimer(1000);

            auto summonThornLasher = addAISpell(SUMMON_THORN_LASHER, 6.0f, TARGET_SELF);
            summonThornLasher->setAttackStopTimer(1000);

            auto summonThornFlayer = addAISpell(SUMMON_THORN_FLAYER, 6.0f, TARGET_SELF);
            summonThornFlayer->setAttackStopTimer(1000);

            teleportSelf = addAISpell(TELEPORT_SELF, 0.0f, TARGET_SELF);
            teleportSelf->setAttackStopTimer(1000);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            TeleportTimer = 20;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            TeleportTimer = 20;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            TeleportTimer = 20;
        }

        void AIUpdate() override
        {
            TeleportTimer--;

            if (!TeleportTimer)
            {
                getCreature()->SetPosition(-204.125000f, 391.248993f, -11.194300f, 0.017453f);    // \todo hmm doesn't work :S
                getCreature()->CastSpell(getCreature(), teleportSelf->mSpellInfo, true);
                TeleportTimer = 20;
            }
        }

    protected:

        uint32 TeleportTimer;
        CreatureAISpells* teleportSelf;
};


// Warp Splinter AI
class WarpSplinterAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(WarpSplinterAI);
        WarpSplinterAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            SummonTimer = 20;    // It's sth about that

            enableCreatureAISpellSystem = true;

            auto stomp = addAISpell(STOMP, 8.0f, TARGET_VARIOUS);
            stomp->setAttackStopTimer(1000);

            summonSaplings = addAISpell(SUMMON_SAPLINGS, 0.0f, TARGET_SELF);
            summonSaplings->setAttackStopTimer(1000);
            summonSaplings->addDBEmote(SAY_WARP_SPLINTER_04);
            summonSaplings->addDBEmote(SAY_WARP_SPLINTER_05);

            auto arcaneVolley = addAISpell(ARCANE_VOLLEY, 12.0f, TARGET_SELF);
            arcaneVolley->setAttackStopTimer(1000);

            addEmoteForEvent(Event_OnCombatStart, SAY_WARP_SPLINTER_01);
            addEmoteForEvent(Event_OnTargetDied, SAY_WARP_SPLINTER_02);
            addEmoteForEvent(Event_OnTargetDied, SAY_WARP_SPLINTER_03);
            addEmoteForEvent(Event_OnDied, SAY_WARP_SPLINTER_06);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            SummonTimer = 20;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            SummonTimer = 20;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            SummonTimer = 20;
        }

        void AIUpdate() override
        {
            SummonTimer--;

            if (!SummonTimer)
            {
                getCreature()->CastSpell(getCreature(), summonSaplings->mSpellInfo, true);
                SummonTimer = 20;
            }
        }

    protected:

        uint32 SummonTimer;
        CreatureAISpells* summonSaplings;
};

void SetupBotanica(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_BLOOD_PROTECTOR, &BloodProtectorAI::Create);
    mgr->register_creature_script(CN_BLOOD_GREENKEEPER, &BloodGreenkeeperAI::Create);
    mgr->register_creature_script(CN_SUN_CHEMIST, &SunchemistAI::Create);
    mgr->register_creature_script(CN_SUN_RESEARCHER, &SunResearcherAI::Create);
    mgr->register_creature_script(CN_COMMANDER_SARANNIS, &CommanderSarannisAI::Create);
    mgr->register_creature_script(CN_HIGH_BOTANIST_FREYWINN, &HighBotanistFreywinnAI::Create);
    mgr->register_creature_script(CN_THORNGRIN_THE_TENDER, &ThorngrinTheTenderAI::Create);
    mgr->register_creature_script(CN_LAJ, &LajAI::Create);
    mgr->register_creature_script(CN_WARP_SPLINTER, &WarpSplinterAI::Create);
}
