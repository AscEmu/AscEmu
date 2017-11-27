/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
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
#include "Instance_TheMechanar.h"

class ArcaneServantAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ArcaneServantAI);
        ArcaneServantAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto arcaneVolley = addAISpell(SP_ARCANE_VOLLEY, 10.0f, TARGET_ATTACKING);
            arcaneVolley->setAttackStopTimer(1000);

            auto arcaneExplosion = addAISpell(SP_ARCANE_EXPLOSION, 15.0f, TARGET_ATTACKING, 0, 0, false, true);
            arcaneExplosion->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class BloodwarderCenturionAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(BloodwarderCenturionAI);
        BloodwarderCenturionAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shieldBash = addAISpell(SP_CENTURION_SHIELD_BASH, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
            shieldBash->setAttackStopTimer(1000);

            auto unstableAffliction = addAISpell(SP_CENTURION_UNSTABLE_AFFLICTION, 6.0f, TARGET_ATTACKING, 0, 0, false, true);
            unstableAffliction->setAttackStopTimer(1000);

            auto meltArmor = addAISpell(SP_CENTURION_MELT_ARMOR, 6.0f, TARGET_ATTACKING, 0, 0, false, true);
            meltArmor->setAttackStopTimer(1000);

            auto chillingTouch = addAISpell(SP_CENTURION_CHILLING_TOUCH, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            chillingTouch->setAttackStopTimer(1000);

            etherealTeleport = addAISpell(SP_CENTURION_ETHEREAL_TELEPORT, 0.0f, TARGET_SELF, 0, 0, false, true);
            etherealTeleport->setAttackStopTimer(1000);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            getCreature()->CastSpell(getCreature(), etherealTeleport->mSpellInfo, etherealTeleport->mIsTriggered);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

    protected:

        CreatureAISpells* etherealTeleport;
};

class BloodwarderPhysicianAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(BloodwarderPhysicianAI);
        BloodwarderPhysicianAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto holyShock = addAISpell(SP_PHYSICIAN_HOLY_SHOCK, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            holyShock->setAttackStopTimer(1000);

            auto anesthetic = addAISpell(SP_PHYSICIAN_ANESTHETIC, 6.0f, TARGET_ATTACKING, 0, 0, false, true);
            anesthetic->setAttackStopTimer(1000);

            auto bandage = addAISpell(SP_PHYSICIAN_BANDAGE, 6.0f, TARGET_ATTACKING, 0, 0, false, true);
            bandage->setAttackStopTimer(1000);

            etherealTeleport = addAISpell(SP_PHYSICIAN_ETHEREAL_TELEPORT_PHYS, 0.0f, TARGET_SELF, 0, 0, false, true);
            etherealTeleport->setAttackStopTimer(1000);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            getCreature()->CastSpell(getCreature(), etherealTeleport->mSpellInfo, etherealTeleport->mIsTriggered);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

    protected:

        CreatureAISpells* etherealTeleport;
};

class BloodwarderSlayerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(BloodwarderSlayerAI);
        BloodwarderSlayerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto whirlwind = addAISpell(SP_SLAYER_WHIRLWIND, 15.0f, TARGET_ATTACKING, 0, 0, false, true);
            whirlwind->setAttackStopTimer(1000);

            auto solarStrike = addAISpell(SP_SLAYER_SOLAR_STRIKE, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            solarStrike->setAttackStopTimer(1000);

            auto meltArmor = addAISpell(SP_SLAYER_MELT_ARMOR, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
            meltArmor->setAttackStopTimer(1000);

            auto chillingTouch = addAISpell(SP_SLAYER_CHILLING_TOUCH, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
            chillingTouch->setAttackStopTimer(1000);

            auto mortalStrike = addAISpell(SP_SLAYER_MORTAL_STRIKE, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            mortalStrike->setAttackStopTimer(1000);;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class MechanarCrusherAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MechanarCrusherAI);
        MechanarCrusherAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto disarm = addAISpell(SP_MECH_CRUSHER_DISARM, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            disarm->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class MechanarDrillerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MechanarDrillerAI);
        MechanarDrillerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto fluid = addAISpell(SP_MECH_DRILLER_GLOB_OF_MACHINE_FLUID, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
            fluid->setAttackStopTimer(1000);

            auto armor = addAISpell(SP_MECH_DRILLER_ARMOR, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            armor->setAttackStopTimer(1000);

            auto cripplingPoison = addAISpell(SP_MECH_DRILLER_CRIPPLING_POISON, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            cripplingPoison->setAttackStopTimer(1000);

            auto pound = addAISpell(SP_MECH_DRILLER_POUND, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            pound->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class MechanarTinkererAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MechanarTinkererAI);
        MechanarTinkererAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto netherbomb = addAISpell(SP_MECH_TINKERER_NETHERBOMB, 5.0f, TARGET_DESTINATION);
            netherbomb->setAttackStopTimer(1000);

            auto prayerOfMendinf = addAISpell(SP_MECH_TINKERER_PRAYER_OF_MENDING, 8.0f, TARGET_VARIOUS, 0, 0, false, true);
            prayerOfMendinf->setAttackStopTimer(1000);

            auto maniacalCharge = addAISpell(SP_MECH_TINKERER_MANIACAL_CHARGE, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
            maniacalCharge->setAttackStopTimer(1000);

            netherExplosion = addAISpell(SP_MECH_TINKERER_NETHER_EXPLOSION, 0.0f, TARGET_VARIOUS);
            netherExplosion->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            getCreature()->CastSpell(getCreature(), netherExplosion->mSpellInfo, netherExplosion->mIsTriggered);
        }

    protected:

        CreatureAISpells* netherExplosion;
};

class MechanarWreckerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MechanarWreckerAI);
        MechanarWreckerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto wreckerPound = addAISpell(SP_MECH_WRECKER_POUND, 12.0f, TARGET_ATTACKING);
            wreckerPound->setAttackStopTimer(1000);

            auto mechanicFluid = addAISpell(SP_MECH_WRECKER_GLOB_OF_MACHINE_FLUID, 7.0f, TARGET_ATTACKING, 0, 0, false, true);
            mechanicFluid->setAttackStopTimer(1000);

            auto prayerOfMending = addAISpell(SP_MECH_WRECKER_PRAYER_OF_MENDING, 6.0f, TARGET_ATTACKING, 0, 0, false, true);
            prayerOfMending->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

// Raging FlamesAI
class RagingFlamesAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(RagingFlamesAI);
        RagingFlamesAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            // disabled to prevent crashes
            // addAISpell(SP_RAGING_FLAMES, 0.0f, TARGET_ATTACKING, 0, 0, false, true);

            auto flamesInferno = addAISpell(SP_RAGING_FLAMES_INFERNO, 9.0f, TARGET_VARIOUS, 0, 0, false, true);
            flamesInferno->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class SunseekerAstromageAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SunseekerAstromageAI);
        SunseekerAstromageAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto scorch = addAISpell(SP_SS_ASTROMAGE_SCORCH, 12.0f, TARGET_ATTACKING);
            scorch->setAttackStopTimer(1000);

            auto solarburn = addAISpell(SP_SS_ASTROMAGE_SOLARBURN, 10.0f, TARGET_ATTACKING);
            solarburn->setAttackStopTimer(1000);

            auto fireShield = addAISpell(SP_SS_ASTROMAGE_FIRE_SHIELD, 8.0f, TARGET_SELF, 0, 0, false, true);
            fireShield->setAttackStopTimer(1000);

            etherealTeleport = addAISpell(SP_SS_ASTROMAGE_ETHEREAL_TELEPORT, 0.0f, TARGET_SELF, 0, 0, false, true);
            etherealTeleport->setAttackStopTimer(1000);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            getCreature()->CastSpell(getCreature(), etherealTeleport->mSpellInfo, etherealTeleport->mIsTriggered);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

    protected:

        CreatureAISpells* etherealTeleport;
};

class SunseekerEngineerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SunseekerEngineerAI);
        SunseekerEngineerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shrinkRay = addAISpell(SP_SS_ENGINEER_SUPER_SHRINK_RAY, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            shrinkRay->setAttackStopTimer(1000);

            auto deathRay = addAISpell(SP_SS_ENGINEER_DEATH_RAY, 13.0f, TARGET_ATTACKING);
            deathRay->setAttackStopTimer(1000);

            auto growthRay = addAISpell(SP_SS_ENGINEER_GROWTH_RAY, 7.0f, TARGET_SELF, 0, 0, false, true);
            growthRay->setAttackStopTimer(1000);

            etherealTeleport = addAISpell(SP_SS_ENGINEER_ETHEREAL_TELEPORT, 0.0f, TARGET_SELF, 0, 0, false, true);
            etherealTeleport->setAttackStopTimer(1000);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            getCreature()->CastSpell(getCreature(), etherealTeleport->mSpellInfo, etherealTeleport->mIsTriggered);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

    protected:

        CreatureAISpells* etherealTeleport;
};

class SunseekerNetherbinderAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SunseekerNetherbinderAI);
        SunseekerNetherbinderAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto arcaneNova = addAISpell(SP_SS_NETHERBINDER_ARCANE_NOVA, 10.0f, TARGET_VARIOUS, 0, 0, false, true);
            arcaneNova->setAttackStopTimer(1000);

            auto starfire = addAISpell(SP_SS_NETHERBINDER_STARFIRE, 13.0f, TARGET_ATTACKING);
            starfire->setAttackStopTimer(1000);

            auto arcaneGolem = addAISpell(SP_SS_NETHERBINDER_SUMMON_ARCANE_GOLEM1, 5.0f, TARGET_SELF, 0, 0, false, true);
            arcaneGolem->setAttackStopTimer(1000);

            auto arcaneGolem2 = addAISpell(SP_SS_NETHERBINDER_SUMMON_ARCANE_GOLEM2, 5.0f, TARGET_SELF, 0, 0, false, true);
            arcaneGolem2->setAttackStopTimer(1000);

            auto dispelMagic = addAISpell(SP_SS_NETHERBINDER_DISPEL_MAGIC, 8.0f, TARGET_SELF, 0, 0, false, true);
            dispelMagic->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class TempestForgeDestroyerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TempestForgeDestroyerAI);
        TempestForgeDestroyerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto knockdown = addAISpell(SP_TEMPEST_DESTROYER_KNOCKDOWN, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            knockdown->setAttackStopTimer(1000);

            auto chargedFirst = addAISpell(SP_TEMPEST_DESTROYER_CHARGED_FIST, 12.0f, TARGET_VARIOUS);
            chargedFirst->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class TempestForgePatrollerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TempestForgePatrollerAI);
        TempestForgePatrollerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto arcaneMissile = addAISpell(SP_TEMPEST_PAT_CHARGED_ARCANE_MISSILE, 12.0f, TARGET_ATTACKING, 0, 0, false, true);
            arcaneMissile->setAttackStopTimer(1000);

            auto knockdown = addAISpell(SP_TEMPEST_PAT_KNOCKDOWN, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            knockdown->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

///////////////////////////////////////////////////////////////////////////////////
// Boss AIs
//////////////////////////////////////////////////////////////////////////////////

class GatewatcherGyroKillAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GatewatcherGyroKillAI);
        GatewatcherGyroKillAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto sawBlade = addAISpell(SP_GW_GYRO_KILL_SAW_BLADE, 13.0f, TARGET_ATTACKING, 0, 0, false, true);
            sawBlade->setAttackStopTimer(1000);
            sawBlade->addDBEmote(SAY_GW_GYRO_KILL_02);
            sawBlade->addDBEmote(SAY_GW_GYRO_KILL_03);

            auto shadowPower = addAISpell(SP_GW_GYRO_KILL_SHADOW_POWER, 7.0f, TARGET_SELF);
            shadowPower->setAttackStopTimer(1000);

            auto machineFluid = addAISpell(SP_GW_GYRO_KILL_STEAM_OF_MACHINE_FLUID, 9.0f, TARGET_VARIOUS, 0, 0, false, true);
            machineFluid->setAttackStopTimer(1000);

            addEmoteForEvent(Event_OnCombatStart, SAY_GW_GYRO_KILL_05);
            addEmoteForEvent(Event_OnTargetDied, SAY_GW_GYRO_KILL_06);
            addEmoteForEvent(Event_OnTargetDied, SAY_GW_GYRO_KILL_04);
            addEmoteForEvent(Event_OnDied, SAY_GW_GYRO_KILL_01);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class GatewatcherIronHandAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GatewatcherIronHandAI);
        GatewatcherIronHandAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto hammer = addAISpell(SP_GW_IRON_HAND_JACK_HAMMER, 7.0f, TARGET_VARIOUS);
            hammer->setAttackStopTimer(1000);
            hammer->addDBEmote(SAY_GW_GYRO_KILL_02);
            hammer->addDBEmote(SAY_GW_GYRO_KILL_03);

            auto punch = addAISpell(SP_GW_IRON_HAND_HAMMER_PUNCH, 9.0f, TARGET_ATTACKING, 0, 0, false, true);
            punch->setAttackStopTimer(1000);

            auto machineFluid = addAISpell(SP_GW_IRON_HAND_STREAM_OF_MACHINE_FLUID, 7.0f, TARGET_VARIOUS, 0, 0, false, true);
            machineFluid->setAttackStopTimer(1000);

            auto shadowPower = addAISpell(SP_GW_IRON_HAND_SHADOW_POWER, 5.0f, TARGET_SELF, 0, 0, false, true);
            shadowPower->setAttackStopTimer(1000);

            addEmoteForEvent(Event_OnCombatStart, SAY_GW_GYRO_KILL_01);
            addEmoteForEvent(Event_OnTargetDied, SAY_GW_GYRO_KILL_04);
            addEmoteForEvent(Event_OnTargetDied, SAY_GW_GYRO_KILL_05);
            addEmoteForEvent(Event_OnDied, SAY_GW_GYRO_KILL_06);
        }
};

class MechanoLordCapacitusAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MechanoLordCapacitusAI);
        MechanoLordCapacitusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto headCrack = addAISpell(SP_MECH_LORD_HEAD_CRACK, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            headCrack->setAttackStopTimer(1000);

            auto damageShield = addAISpell(SP_MECH_LORD_REFLECTIVE_DAMAGE_SHIELD, 7.0f, TARGET_SELF);
            damageShield->setAttackStopTimer(1000);
            damageShield->addEmote("Think you can hurt me, huh? Think I'm afraid a' you?", CHAT_MSG_MONSTER_YELL, 11165);

            auto reflectiveMagicShield = addAISpell(SP_MECH_LORD_REFLECTIVE_MAGIC_SHIELD, 7.0f, TARGET_SELF);
            reflectiveMagicShield->setAttackStopTimer(1000);
            reflectiveMagicShield->addEmote("Go ahead, gimme your best shot. I can take it!", CHAT_MSG_MONSTER_YELL, 11166);

            auto seedOfCorruption = addAISpell(SP_MECH_LORD_SEED_OF_CORRUPTION, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
            seedOfCorruption->setAttackStopTimer(1000);

            addEmoteForEvent(Event_OnCombatStart, SAY_MECH_LORD_06);
            addEmoteForEvent(Event_OnTargetDied, SAY_MECH_LORD_03);
            addEmoteForEvent(Event_OnTargetDied, SAY_MECH_LORD_02);
            addEmoteForEvent(Event_OnDied, SAY_MECH_LORD_01);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class NethermancerSepethreaAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(NethermancerSepethreaAI);
        NethermancerSepethreaAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            summonRaginFlames = addAISpell(SP_NETH_SEPETHREA_SUMMON_RAGIN_FLAMES, 0.0f, TARGET_SELF, 0, 0, false, true);
            summonRaginFlames->setAttackStopTimer(1000);

            auto frostAttack = addAISpell(SP_NETH_SEPETHREA_FROST_ATTACK, 9.0f, TARGET_ATTACKING, 0, 0, false, true);
            frostAttack->setAttackStopTimer(1000);

            auto arcaneBlast = addAISpell(SP_NETH_SEPETHREA_ARCANE_BLAST, 3.0f, TARGET_ATTACKING, 0, 0, false, true);
            arcaneBlast->setAttackStopTimer(1000);

            auto dragonsBreath = addAISpell(SP_NETH_SEPETHREA_DRAGONS_BREATH, 8.0f, TARGET_VARIOUS, 0, 0, false, true);
            dragonsBreath->setAttackStopTimer(1000);
            dragonsBreath->addDBEmote(SAY_NETH_SEPETHREA_03);
            dragonsBreath->addDBEmote(SAY_NETH_SEPETHREA_04);

            addEmoteForEvent(Event_OnCombatStart, SAY_NETH_SEPETHREA_01);
            addEmoteForEvent(Event_OnTargetDied, SAY_NETH_SEPETHREA_05);
            addEmoteForEvent(Event_OnTargetDied, SAY_NETH_SEPETHREA_06);
            addEmoteForEvent(Event_OnDied, SAY_NETH_SEPETHREA_07);

            SummonTimer = 0;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            SummonTimer = 4;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            SummonTimer = 4;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            SummonTimer = 4;
        }

        void AIUpdate() override
        {
            SummonTimer--;

            if (!SummonTimer && getScriptPhase() == 1)
            {
                getCreature()->CastSpell(getCreature(), summonRaginFlames->mSpellInfo, summonRaginFlames->mIsTriggered);
                sendDBChatMessage(SAY_NETH_SEPETHREA_02);
            }
        }

    protected:

        int SummonTimer;
        CreatureAISpells* summonRaginFlames;
};


// hmm... he switches weapons and there is sound for it, but I must know when he does that, how it looks like etc.
// before adding weapon switching =/    (Sound: 11199; speech: "I prefer to be hands-on...";)
class PathaleonTheCalculatorAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(PathaleonTheCalculatorAI);
        PathaleonTheCalculatorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto manaTrap = addAISpell(SP_PATHALEON_MANA_TRAP, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            manaTrap->setAttackStopTimer(1000);

            auto domination = addAISpell(SP_PATHALEON_DOMINATION, 4.0f, TARGET_ATTACKING, 0, 0, false, true);
            domination->setAttackStopTimer(1000);
            domination->addDBEmote(SAY_PATHALEON_02);
            domination->addDBEmote(SAY_PATHALEON_03);

            auto silence = addAISpell(SP_PATHALEON_SILENCE, 6.0f, TARGET_VARIOUS, 0, 0, false, true);
            silence->setAttackStopTimer(1000);

            summonNetherWraith1 = addAISpell(SP_PATHALEON_SUMMON_NETHER_WRAITH1, 0.0f, TARGET_SELF, 0, 0, false, true);
            summonNetherWraith1->setAttackStopTimer(1000);

            summonNetherWraith2 = addAISpell(SP_PATHALEON_SUMMON_NETHER_WRAITH2, 0.0f, TARGET_SELF, 0, 0, false, true);
            summonNetherWraith2->setAttackStopTimer(1000);

            summonNetherWraith3 = addAISpell(SP_PATHALEON_SUMMON_NETHER_WRAITH3, 0.0f, TARGET_SELF, 0, 0, false, true);
            summonNetherWraith3->setAttackStopTimer(1000);

            summonNetherWraith4 = addAISpell(SP_PATHALEON_SUMMON_NETHER_WRAITH4, 0.0f, TARGET_SELF, 0, 0, false, true);
            summonNetherWraith4->setAttackStopTimer(1000);

            addEmoteForEvent(Event_OnCombatStart, SAY_PATHALEON_01);
            addEmoteForEvent(Event_OnTargetDied, SAY_PATHALEON_06);
            addEmoteForEvent(Event_OnTargetDied, SAY_PATHALEON_07);
            addEmoteForEvent(Event_OnDied, SAY_PATHALEON_08);

            SummonTimer = 0;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            SummonTimer = RandomUInt(30, 45);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            SummonTimer = RandomUInt(30, 45);
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            SummonTimer = RandomUInt(30, 45);
        }

        void AIUpdate() override
        {
            SummonTimer--;

            if (!SummonTimer && getScriptPhase() == 1)
            {
                getCreature()->CastSpell(getCreature(), summonNetherWraith1->mSpellInfo, summonNetherWraith1->mIsTriggered);
                getCreature()->CastSpell(getCreature(), summonNetherWraith2->mSpellInfo, summonNetherWraith2->mIsTriggered);
                getCreature()->CastSpell(getCreature(), summonNetherWraith3->mSpellInfo, summonNetherWraith3->mIsTriggered);
                getCreature()->CastSpell(getCreature(), summonNetherWraith4->mSpellInfo, summonNetherWraith4->mIsTriggered);
                SummonTimer = RandomUInt(30, 45);    // 30 - 45
                sendDBChatMessage(SAY_PATHALEON_04);
            }
        }

    protected:

        uint32 SummonTimer;
        CreatureAISpells* summonNetherWraith1;
        CreatureAISpells* summonNetherWraith2;
        CreatureAISpells* summonNetherWraith3;
        CreatureAISpells* summonNetherWraith4;
};

// \todo Data needed for: Nether Wraith, Mechanar Crusher (maybe not enough?)
void SetupTheMechanar(ScriptMgr* mgr)
{
    //Creatures
    mgr->register_creature_script(CN_ARCANE_SERVANT, &ArcaneServantAI::Create);
    mgr->register_creature_script(CN_BLOODWARDER_CENTURION, &BloodwarderCenturionAI::Create);
    mgr->register_creature_script(CN_BLOODWARDER_PHYSICIAN, &BloodwarderPhysicianAI::Create);
    mgr->register_creature_script(CN_BLOODWARDER_SLAYER, &BloodwarderSlayerAI::Create);
    mgr->register_creature_script(CN_MECHANAR_CRUSHER, &MechanarCrusherAI::Create);
    mgr->register_creature_script(CN_MECHANAR_DRILLER, &MechanarDrillerAI::Create);
    mgr->register_creature_script(CN_MECHANAR_TINKERER, &MechanarTinkererAI::Create);
    mgr->register_creature_script(CN_MECHANAR_WRECKER, &MechanarWreckerAI::Create);
    mgr->register_creature_script(CN_RAGING_FLAMES, &RagingFlamesAI::Create);
    mgr->register_creature_script(CN_SUNSEEKER_ASTROMAGE, &SunseekerAstromageAI::Create);
    mgr->register_creature_script(CN_SUNSEEKER_ENGINEER, &SunseekerEngineerAI::Create);
    mgr->register_creature_script(CN_SUNSEEKER_NETHERBINDER, &SunseekerNetherbinderAI::Create);
    mgr->register_creature_script(CN_TEMPEST_FORGE_DESTROYER, &TempestForgeDestroyerAI::Create);
    mgr->register_creature_script(CN_TEMPEST_FORGE_PATROLLER, &TempestForgePatrollerAI::Create);
    mgr->register_creature_script(CN_GATEWATCHER_GYRO_KILL, &GatewatcherGyroKillAI::Create);
    mgr->register_creature_script(CN_GATEWATCHER_IRON_HAND, &GatewatcherIronHandAI::Create);
    mgr->register_creature_script(CN_MECHANO_LORD_CAPACITUS, &MechanoLordCapacitusAI::Create);
    mgr->register_creature_script(CN_NETHERMANCER_SEPETHREA, &NethermancerSepethreaAI::Create);
    mgr->register_creature_script(CN_PATHALEON_THE_CALCULATOR, &PathaleonTheCalculatorAI::Create);
}
