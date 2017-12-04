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

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Instance_BlackrockDepths.h"


class AmbassadorFlamelash : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AmbassadorFlamelash);
        AmbassadorFlamelash(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto fireBlast = addAISpell(15573, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            fireBlast->setAttackStopTimer(1000);
        }
};


class AnubShiah : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AnubShiah);
        AnubShiah(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto unknown = addAISpell(11661, 20.0f, TARGET_ATTACKING, 0, 20, false, true);
            unknown->setAttackStopTimer(1000);

            auto envelopingWeb = addAISpell(11661, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            envelopingWeb->setAttackStopTimer(1000);
        }
};


class BaelGar : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(BaelGar);
        BaelGar(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto magmaSplash = addAISpell(13879, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            magmaSplash->setAttackStopTimer(1000);

            auto summonSpawnOdBaelGar = addAISpell(13895, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            summonSpawnOdBaelGar->setAttackStopTimer(1000);
        }
};


class EmperorDagranThaurissan : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(EmperorDagranThaurissan);
        EmperorDagranThaurissan(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto handOfThaurissan = addAISpell(17492, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            handOfThaurissan->setAttackStopTimer(1000);

            auto mortalStrike = addAISpell(24573, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            mortalStrike->setAttackStopTimer(1000);

            auto cleave = addAISpell(20691, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            cleave->setAttackStopTimer(1000);
        }
};


class Eviscerator : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(Eviscerator);
        Eviscerator(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto viciousRend = addAISpell(14331, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            viciousRend->setAttackStopTimer(1000);

            auto shadowBoltVolley = addAISpell(20741, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            shadowBoltVolley->setAttackStopTimer(1000);
        }
};


class FineousDarkvire : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(FineousDarkvire);
        FineousDarkvire(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto kick = addAISpell(15614, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            kick->setAttackStopTimer(1000);

            auto holyStrike = addAISpell(13953, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            holyStrike->setAttackStopTimer(1000);
        }
};


class GeneralAngerforge : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GeneralAngerforge);
        GeneralAngerforge(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto mightyBlow = addAISpell(14099, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            mightyBlow->setAttackStopTimer(1000);

            auto hanstering = addAISpell(9080, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            hanstering->setAttackStopTimer(1000);

            auto cleave = addAISpell(20691, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            cleave->setAttackStopTimer(1000);
        }
};


class GolemLordArgelmach : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GolemLordArgelmach);
        GolemLordArgelmach(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto chainLightning = addAISpell(16033, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            chainLightning->setAttackStopTimer(1000);

            auto shock = addAISpell(16034, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            shock->setAttackStopTimer(1000);

            auto unknown = addAISpell(10432, 20.0f, TARGET_SELF, 0, 10, false, true);
            unknown->setAttackStopTimer(1000);
        }
};


class GoroshTheDervish : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GoroshTheDervish);
        GoroshTheDervish(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto whirlwind = addAISpell(15589, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            whirlwind->setAttackStopTimer(1000);

            auto mortalStrike = addAISpell(24573, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            mortalStrike->setAttackStopTimer(1000);
        }
};


class Grizzle : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(Grizzle);
        Grizzle(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto groundTremor = addAISpell(6524, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            groundTremor->setAttackStopTimer(1000);

            auto cleave = addAISpell(20691, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            cleave->setAttackStopTimer(1000);
        }
};


class HedrumTheCreeper : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HedrumTheCreeper);
        HedrumTheCreeper(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto banefulPoison = addAISpell(15475, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            banefulPoison->setAttackStopTimer(1000);

            auto webExplosion = addAISpell(15474, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            webExplosion->setAttackStopTimer(1000);

            auto paralyzingPoison = addAISpell(3609, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            paralyzingPoison->setAttackStopTimer(1000);
        }
};

class HighInterrogatorGerstahn : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HighInterrogatorGerstahn);
        HighInterrogatorGerstahn(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto unknown = addAISpell(10894, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown->setAttackStopTimer(1000);

            auto psychicScream = addAISpell(8122, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            psychicScream->setAttackStopTimer(1000);

            auto unknown2 = addAISpell(10876, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown2->setAttackStopTimer(1000);
        }
};

class HoundmasterGrebmar : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HoundmasterGrebmar);
        HoundmasterGrebmar(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto demoralizingShout = addAISpell(23511, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            demoralizingShout->setAttackStopTimer(1000);

            auto rend = addAISpell(17153, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            rend->setAttackStopTimer(1000);
        }
};


class HurleyBlackbreath : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HurleyBlackbreath);
        HurleyBlackbreath(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto flameBreath = addAISpell(17294, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            flameBreath->setAttackStopTimer(1000);

            auto rupture = addAISpell(15583, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            rupture->setAttackStopTimer(1000);

            auto mightyBlow = addAISpell(14099, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            mightyBlow->setAttackStopTimer(1000);
        }
};


class LordIncendius : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LordIncendius);
        LordIncendius(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto fireStorm = addAISpell(13899, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            fireStorm->setAttackStopTimer(1000);

            auto fieryBurst = addAISpell(13900, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            fieryBurst->setAttackStopTimer(1000);

            auto mightyBlow = addAISpell(14099, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            mightyBlow->setAttackStopTimer(1000);
        }
};


class LordRoccor : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LordRoccor);
        LordRoccor(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto unknown = addAISpell(10448, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown->setAttackStopTimer(1000);

            auto groundTremor = addAISpell(6524, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            groundTremor->setAttackStopTimer(1000);

            auto unknown2 = addAISpell(10414, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown2->setAttackStopTimer(1000);
        }
};


class Magmus : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(Magmus);
        Magmus(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto fieryBurst = addAISpell(13900, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            fieryBurst->setAttackStopTimer(1000);

            auto warStomp = addAISpell(24375, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            warStomp->setAttackStopTimer(1000);
        }
};

class OkThorTheBreaker : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(OkThorTheBreaker);
        OkThorTheBreaker(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto arcaneExplosion = addAISpell(15453, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            arcaneExplosion->setAttackStopTimer(1000);

            auto arcaneBolt = addAISpell(15451, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            arcaneBolt->setAttackStopTimer(1000);
        }
};

class Phalanx : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(Phalanx);
        Phalanx(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto thunderclap = addAISpell(8732, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            thunderclap->setAttackStopTimer(1000);

            auto fireballVolley = addAISpell(22425, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            fireballVolley->setAttackStopTimer(1000);

            auto mightyBlow = addAISpell(14099, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            mightyBlow->setAttackStopTimer(1000);
        }
};

class PrincessMoiraBronzebeard : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(PrincessMoiraBronzebeard);
        PrincessMoiraBronzebeard(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto unknown = addAISpell(10947, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown->setAttackStopTimer(1000);

            auto frostNova = addAISpell(22645, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            frostNova->setAttackStopTimer(1000);
        }
};

class PyromancerLoregrain : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(PyromancerLoregrain);
        PyromancerLoregrain(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto unknown = addAISpell(10448, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            unknown->setAttackStopTimer(1000);

            auto moltenBlast = addAISpell(15095, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
            moltenBlast->setAttackStopTimer(1000);
        }
};

void SetupBlackrockDepths(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_AMBASSADOR_FLAMELASH, &AmbassadorFlamelash::Create);
    mgr->register_creature_script(CN_ANUB_SHIAH, &AnubShiah::Create);
    mgr->register_creature_script(CN_BAEL_GAR, &BaelGar::Create);
    mgr->register_creature_script(CN_DRAGRAN_THAURISSAN, &EmperorDagranThaurissan::Create);
    mgr->register_creature_script(CN_EVISCERATOR, &Eviscerator::Create);
    mgr->register_creature_script(CN_FINEOUS_DARKVIRE, &FineousDarkvire::Create);
    mgr->register_creature_script(CN_GENERAL_ANGERFORGE, &GeneralAngerforge::Create);
    mgr->register_creature_script(CN_GOLEM_LORD_ARGELMACH, &GolemLordArgelmach::Create);
    mgr->register_creature_script(CN_GRIZZLE, &Grizzle::Create);
    mgr->register_creature_script(CN_HEDRUM_THE_CREEPER, &HedrumTheCreeper::Create);
    mgr->register_creature_script(CN_INTERROGATOR_GERSTAHN, &HighInterrogatorGerstahn::Create);
    mgr->register_creature_script(CN_HOUNDMASTER_GREBMAR, &HoundmasterGrebmar::Create);
    mgr->register_creature_script(CN_HURLEY_BLACKBREATH, &HurleyBlackbreath::Create);
    mgr->register_creature_script(CN_LORD_INCENDIUS, &LordIncendius::Create);
    mgr->register_creature_script(CN_LORD_ROCCOR, &LordRoccor::Create);
    mgr->register_creature_script(CN_MAGMUS, &Magmus::Create);
    mgr->register_creature_script(CN_OKTHOR_THE_BREAKER, &OkThorTheBreaker::Create);
    mgr->register_creature_script(CN_PHALANX, &Phalanx::Create);
    mgr->register_creature_script(CN_MOIRA_BRONZEBART, &PrincessMoiraBronzebeard::Create);
    mgr->register_creature_script(CN_PYROMANCER_LOREGRAIN, &PyromancerLoregrain::Create);
}
