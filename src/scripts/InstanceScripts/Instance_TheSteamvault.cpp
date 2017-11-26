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
#include "Instance_TheSteamvault.h"
#include "Objects/Faction.h"


/////////////////////////////////////////////////////////////
// Boss AIs
/////////////////////////////////////////////////////////////

class HydromancerThespiaAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HydromancerThespiaAI);
        HydromancerThespiaAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto envelopingWinds = addAISpell(ENVELOPING_WINDS, 9.0f, TARGET_RANDOM_SINGLE, 0, 10, false, true);
            envelopingWinds->setAttackStopTimer(1000);
            envelopingWinds->setMinMaxDistance(0.0f, 35.0f);

            auto lightningCloud = addAISpell(LIGHTNING_CLOUD, 8.0f, TARGET_RANDOM_DESTINATION, 0, 25, false, true);
            lightningCloud->setAttackStopTimer(1000);
            lightningCloud->setMinMaxDistance(0.0f, 30.0f);

            auto lungBurst = addAISpell(LUNG_BURST, 9.0f, TARGET_RANDOM_SINGLE, 0, 15, false, true);
            lungBurst->setAttackStopTimer(1000);
            lungBurst->setMinMaxDistance(0.0f, 40.0f);

            addEmoteForEvent(Event_OnCombatStart, SAY_HYDROMACER_THESPIA_02);
            addEmoteForEvent(Event_OnCombatStart, SAY_HYDROMACER_THESPIA_03);
            addEmoteForEvent(Event_OnCombatStart, SAY_HYDROMACER_THESPIA_04);
            addEmoteForEvent(Event_OnTargetDied, SAY_HYDROMACER_THESPIA_05);
            addEmoteForEvent(Event_OnTargetDied, SAY_HYDROMACER_THESPIA_06);
            addEmoteForEvent(Event_OnDied, SAY_HYDROMACER_THESPIA_07);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

//---- Steamrigger encounter ----//

static Movement::Location SpawnCoords[] =
{
    { -300.037842f, -115.296227f, -7.865229f, 4.197916f },
    { -330.083008f, -121.505997f, -7.985120f, 5.061450f },
    { -346.530273f, -147.167892f, -6.703687f, 0.010135f }
};

// Steamrigger MechanicAI
// Should they really fight?
class SteamriggerMechanicAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SteamriggerMechanicAI);
        SteamriggerMechanicAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            getCreature()->m_noRespawn = true;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            getCreature()->GetAIInterface()->ResetUnitToFollow();
            getCreature()->GetAIInterface()->SetUnitToFollowAngle(0.0f);

            getCreature()->interruptSpell();
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        }

        void OnTargetDied(Unit* mTarget) override
        {
            getCreature()->GetAIInterface()->RemoveThreatByPtr(mTarget);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            getCreature()->GetAIInterface()->ResetUnitToFollow();
            getCreature()->GetAIInterface()->SetUnitToFollowAngle(0.0f);
        }

        void OnDamageTaken(Unit* /*mAttacker*/, uint32 /*fAmount*/) override
        {
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
        }
};


// Must spawn 3 Steamrigger Mechanics when his health is on 75%, 50% and 25%
class MekgineerSteamriggerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MekgineerSteamriggerAI);
        std::vector <Unit*> Gnomes;

        MekgineerSteamriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shrinkRay = addAISpell(SUPER_SHRINK_RAY, 9.0f, TARGET_RANDOM_SINGLE, 0, 20, false, true);
            shrinkRay->setAttackStopTimer(1000);
            shrinkRay->setMinMaxDistance(0.0f, 40.0f);

            auto sawBlade = addAISpell(SAW_BLADE, 10.0f, TARGET_RANDOM_SINGLE, 0, 15, false, true);
            sawBlade->setAttackStopTimer(1000);
            sawBlade->setMinMaxDistance(0.0f, 40.0f);

            auto electrifiedNet = addAISpell(ELECTRIFIED_NET, 8.0f, TARGET_RANDOM_SINGLE, 0, 15, false, true);
            electrifiedNet->setAttackStopTimer(1000);
            electrifiedNet->setMinMaxDistance(0.0f, 40.0f);

            auto enrage = addAISpell(ENRAGE, 8.0f, TARGET_SELF, 0, 300, false, true);
            enrage->setAttackStopTimer(1000);
            enrage->addDBEmote(SAY_MEKGINEER_STEAMRIGGER_08);

            GnomeCounter = 0;

            addEmoteForEvent(Event_OnCombatStart, SAY_MEKGINEER_STEAMRIGGER_02);
            addEmoteForEvent(Event_OnCombatStart, SAY_MEKGINEER_STEAMRIGGER_03);
            addEmoteForEvent(Event_OnCombatStart, SAY_MEKGINEER_STEAMRIGGER_04);
            addEmoteForEvent(Event_OnCombatStart, SAY_MEKGINEER_STEAMRIGGER_05);
            addEmoteForEvent(Event_OnTargetDied, SAY_MEKGINEER_STEAMRIGGER_06);
            addEmoteForEvent(Event_OnTargetDied, SAY_MEKGINEER_STEAMRIGGER_07);
            addEmoteForEvent(Event_OnDied, SAY_MEKGINEER_STEAMRIGGER_09);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            GnomeCounter = 0;

            Gnomes.clear();
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            for (size_t i = 0; i < Gnomes.size(); i++)
            {
                Creature* Gnome = NULL;
                Gnome = static_cast<Creature*>(Gnomes[i]);
                if (!Gnome)
                    continue;

                if (!Gnome->IsInWorld() || !Gnome->isAlive())
                    continue;

                Gnome->Despawn(0, 0);
            }

            Gnomes.clear();

            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void AIUpdate() override
        {
            if (Gnomes.size())
            {
                Unit* Gnome = NULL;
                for (std::vector<Unit*>::iterator itr = Gnomes.begin(); itr < Gnomes.end(); ++itr)
                {
                    Gnome = *itr;
                    if (!Gnome->isAlive() || !Gnome->IsInWorld())
                    {
                        itr = Gnomes.erase(itr);
                        continue;
                    }

                    if (Gnome->GetAIInterface()->getNextTarget())
                        continue;

                    if (Gnome->GetAIInterface()->getUnitToFollow() == NULL)
                    {
                        Gnome->GetAIInterface()->SetUnitToFollow(getCreature());
                        Gnome->GetAIInterface()->SetFollowDistance(15.0f);
                    }

                    if (getCreature()->GetDistance2dSq(Gnome) > 250.0f)
                    {
                        if (Gnome->isCastingNonMeleeSpell())
                            Gnome->interruptSpell();

                        continue;
                    }

                    if (!Gnome->isCastingNonMeleeSpell())
                    {
                        Gnome->GetAIInterface()->StopMovement(1);
                        Gnome->CastSpell(getCreature(), REPAIR, false);    // core problem? casted on self (and effect is applied on caster instead of _unit)
                    }
                }
            }

            if ((getCreature()->GetHealthPct() <= 75 && GnomeCounter == 0) || (getCreature()->GetHealthPct() <= 50 && GnomeCounter == 1) || (getCreature()->GetHealthPct() <= 25 && GnomeCounter == 2))
            {
                Unit* Gnome = NULL;
                for (uint8 i = 0; i < 3; i++)
                {
                    Gnome = spawnCreature(CN_STEAMRIGGER_MECHANIC, SpawnCoords[i].x, SpawnCoords[i].y, SpawnCoords[i].z, SpawnCoords[i].o, getCreature()->GetFaction());
                    if (Gnome)
                    {
                        Gnome->GetAIInterface()->SetUnitToFollow(getCreature());
                        Gnome->GetAIInterface()->SetFollowDistance(15.0f);
                        Gnomes.push_back(Gnome);
                    }
                }

                sendDBChatMessage(SAY_MEKGINEER_STEAMRIGGER_01);

                GnomeCounter++;
            }
        }

    protected:

        uint8 GnomeCounter;
};

//---- Warlord Kalitresh Encounter ----//

static Movement::Location Distiller[] =
{
    {  },
    { -113.183952f, -488.599335f, 8.196310f, 6.134734f },
    {  -76.880989f, -489.164673f, 8.236189f, 2.103285f },
    {  -74.309753f, -520.418884f, 8.255078f, 4.612630f },
    { -116.220764f, -520.139771f, 8.198921f, 5.127069f }
};

static Movement::Location DistillerMoveTo[] =
{
    {  },
    { -108.092949f, -491.747803f, 8.198845f,  0.621336f },
    {  -81.165871f, -492.459869f, 8.255936f,  6.531955f },
    {  -79.170982f, -518.544800f, 8.241381f, -2.281880f },
    { -112.033188f, -517.945190f, 8.205022f, -0.949258f }
};

// Naga DistillerAI
class NagaDistillerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(NagaDistillerAI);

        NagaDistillerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            _setMeleeDisabled(true);
            getCreature()->GetAIInterface()->m_canMove = false;
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            getCreature()->SetChannelSpellTargetGUID(0);
            getCreature()->SetChannelSpellId(0);
        }
};

// Warlord Kalitresh AI
class WarlordKalitreshAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(WarlordKalitreshAI);
        WarlordKalitreshAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto impale = addAISpell(IMPALE, 8.0f, TARGET_RANDOM_SINGLE, 0, 10, false, true);
            impale->setAttackStopTimer(1000);
            impale->setMinMaxDistance(0.0f, 40.0f);

            auto crack = addAISpell(HEAD_CRACK, 8.0f, TARGET_RANDOM_SINGLE, 0, 10, false, true);
            crack->setAttackStopTimer(1000);
            crack->setMinMaxDistance(0.0f, 40.0f);

            auto reflection = addAISpell(SPELL_REFLECTION, 8.0f, TARGET_SELF, 0, 25, false, true);
            reflection->setAttackStopTimer(1000);

            auto rage = addAISpell(WARLORDS_RAGE, 2.0f, TARGET_SELF, 0, 240, false, true);
            rage->setAttackStopTimer(1000);
            rage->addEmote("This is not nearly over!", CHAT_MSG_MONSTER_YELL, 10391);

            DistillerNumber = 0;
            RagePhaseTimer = 0;
            EnrageTimer = 0;
            RagePhase = 0;

            addEmoteForEvent(Event_OnCombatStart, SAY_WARLORD_KALITRESH_03);
            addEmoteForEvent(Event_OnCombatStart, SAY_WARLORD_KALITRESH_04);
            addEmoteForEvent(Event_OnCombatStart, SAY_WARLORD_KALITRESH_05);
            addEmoteForEvent(Event_OnTargetDied, SAY_WARLORD_KALITRESH_06);
            addEmoteForEvent(Event_OnTargetDied, SAY_WARLORD_KALITRESH_07);
            addEmoteForEvent(Event_OnDied, SAY_WARLORD_KALITRESH_08);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            RagePhaseTimer = (uint32)time(NULL) + RandomUInt(15) + 10;
            DistillerNumber = 0;
            EnrageTimer = 0;
            RagePhase = 0;

            GameObject* Gate = NULL;
            Gate = getNearestGameObject(-95.774361f, -439.608612f, 3.382976f, 183049);
            if (Gate)
                Gate->SetState(GO_STATE_CLOSED);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            GameObject* Gate = getNearestGameObject(-95.774361f, -439.608612f, 3.382976f, 183049);
            if (Gate)
                Gate->SetState(GO_STATE_OPEN);

            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
            getCreature()->GetAIInterface()->m_canMove = true;
            getCreature()->GetAIInterface()->ResetUnitToFollow();
            getCreature()->GetAIInterface()->SetFollowDistance(0.0f);
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            if (getCreature()->getAuraWithId(37076))
                getCreature()->RemoveAura(37076);
            if (getCreature()->getAuraWithId(36453))
                getCreature()->RemoveAura(36453);

            Unit* pDistiller = NULL;
            pDistiller = GetClosestDistiller();
            if (pDistiller)
            {
                pDistiller->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                pDistiller->SetChannelSpellTargetGUID(0);
                pDistiller->SetChannelSpellId(0);
                pDistiller->GetAIInterface()->WipeTargetList();
                pDistiller->GetAIInterface()->WipeHateList();
            }

            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            uint32 t = (uint32)time(NULL);
            if (t > RagePhaseTimer)
            {
                if (EnrageTimer != 0)
                    EnrageTimer++;

                Unit* pDistiller = NULL;
                pDistiller = GetClosestDistiller();
                if (!pDistiller || (pDistiller->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9) && RagePhase != 0))
                {
                    getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
                    getCreature()->GetAIInterface()->m_canMove = true;
                    getCreature()->GetAIInterface()->ResetUnitToFollow();
                    getCreature()->GetAIInterface()->SetFollowDistance(0.0f);

                    RagePhaseTimer = t + RandomUInt(15) + 20;
                    EnrageTimer = 0;
                    RagePhase = 0;

                    if (getCreature()->getAuraWithId(31543))
                        getCreature()->RemoveAura(31543);
                    if (getCreature()->getAuraWithId(37076))
                        getCreature()->RemoveAura(37076);
                }

                else
                {
                    if (EnrageTimer == 0 && RagePhase == 0)
                    {
                        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
                        getCreature()->GetAIInterface()->SetUnitToFollow(pDistiller);
                        getCreature()->GetAIInterface()->SetFollowDistance(10.0f);

                        getCreature()->GetAIInterface()->StopMovement(0);
                        getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                        getCreature()->GetAIInterface()->MoveTo(DistillerMoveTo[DistillerNumber].x, DistillerMoveTo[DistillerNumber].y, DistillerMoveTo[DistillerNumber].z);

                        if (getCreature()->GetDistance2dSq(pDistiller) <= 100.0f)
                        {
                            pDistiller->setUInt64Value(UNIT_FIELD_FLAGS, 0);
                            pDistiller->SetChannelSpellTargetGUID(getCreature()->GetGUID());
                            pDistiller->SetChannelSpellId(31543);

                            getCreature()->GetAIInterface()->StopMovement(0);
                            getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                            getCreature()->GetAIInterface()->m_canMove = false;
                            sendDBChatMessage(SAY_WARLORD_KALITRESH_02);

                            if (!getCreature()->getAuraWithId(36453))
                                getCreature()->CastSpell(getCreature(), 31543, true);

                            EnrageTimer = t + 3;
                            RagePhase = 1;
                        }
                    }

                    else if (t > EnrageTimer && RagePhase == 1)
                    {
                        EnrageTimer = t + 6;    // 9
                        RagePhase = 2;
                    }

                    else if (t > EnrageTimer && RagePhase == 2)
                    {
                        pDistiller->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                        pDistiller->SetChannelSpellTargetGUID(0);
                        pDistiller->SetChannelSpellId(0);
                        pDistiller->GetAIInterface()->WipeTargetList();
                        pDistiller->GetAIInterface()->WipeHateList();

                        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
                        getCreature()->GetAIInterface()->m_canMove = true;
                        getCreature()->GetAIInterface()->ResetUnitToFollow();
                        getCreature()->GetAIInterface()->SetFollowDistance(0.0f);
                        getCreature()->CastSpell(getCreature(), 36453, true);

                        RagePhaseTimer = t + RandomUInt(15) + 20;
                        DistillerNumber = 0;
                        EnrageTimer = 0;
                        RagePhase = 0;
                    }
                }
            }
        }

        Unit* GetClosestDistiller()
        {
            float distance = 50.0f;
            Unit* pDistiller = NULL;
            Unit* Unit2Check = NULL;

            for (uint8 i = 1; i < 5; i++)
            {
                Unit2Check = getNearestCreature(Distiller[i].x, Distiller[i].y, Distiller[i].z, 17954);
                if (!Unit2Check)
                    continue;

                if (!Unit2Check->isAlive())
                    continue;

                float Dist2Unit = getCreature()->GetDistance2dSq(Unit2Check);
                if (Dist2Unit > distance * distance)
                    continue;

                pDistiller = Unit2Check;
                distance = sqrt(Dist2Unit);
                DistillerNumber = i;
            }

            return pDistiller;
        }

    protected:

        uint32 DistillerNumber;
        uint32 RagePhaseTimer;
        uint32 EnrageTimer;
        uint32 RagePhase;
};

/** \todo Check all spells/creatures and evenatually balance them (if needed!)
   also add spawns correctly (no core support for now and hacky Onyxia way does
   not work (in Onyxia also). Change target type when there will be more core
   support. Also very imporant thing is to force spawns to heal random or target
   you choosed. When that will work, I will add AI for mechanics who should
   heal one of instance bosses.*/
// Don't have infos about: Second Fragment Guardian (22891) | Dreghood Slave (17799 -
// should they really have enrage ? [8269]), Driller should use Warlord's Rage spell
// (31543) to force Warlord to enrage, but I need more infos about targeting target
// you want to.
void SetupTheSteamvault(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_HYDROMANCER_THESPIA, &HydromancerThespiaAI::Create);
    mgr->register_creature_script(CN_STEAMRIGGER_MECHANIC, &SteamriggerMechanicAI::Create);
    mgr->register_creature_script(CN_MEKGINEER_STEAMRIGGER, &MekgineerSteamriggerAI::Create);
    mgr->register_creature_script(CN_NAGA_DISTILLER, &NagaDistillerAI::Create);
    mgr->register_creature_script(CN_WARLORD_KALITRESH, &WarlordKalitreshAI::Create);
}
