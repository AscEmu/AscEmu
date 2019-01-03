/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Instance_DireMaul.h"

class AlzzinTheWildshaper : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AlzzinTheWildshaper);
    explicit AlzzinTheWildshaper(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto mangle = addAISpell(22689, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        mangle->setAttackStopTimer(3000);

        auto wither = addAISpell(22662, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        wither->setAttackStopTimer(3000);

        auto viliciousBite = addAISpell(19319, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        viliciousBite->setAttackStopTimer(3000);
    }
};


class CaptainKromcrush : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CaptainKromcrush);
    explicit CaptainKromcrush(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto mortalStrike = addAISpell(15708, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        mortalStrike->setAttackStopTimer(3000);

        auto demoralizingShout = addAISpell(23511, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        demoralizingShout->setAttackStopTimer(3000);
    }
};


class ChoRushTheObserver : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ChoRushTheObserver);
    explicit ChoRushTheObserver(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto unknown = addAISpell(10947, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknown->setAttackStopTimer(3000);

        auto unknown2 = addAISpell(10151, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknown2->setAttackStopTimer(3000);
    }
};


class GuardFengus : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GuardFengus);
    explicit GuardFengus(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto strike = addAISpell(15580, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        strike->setAttackStopTimer(3000);

        auto shieldSlam = addAISpell(15655, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        shieldSlam->setAttackStopTimer(3000);

        auto bruisingBlow = addAISpell(22572, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        bruisingBlow->setAttackStopTimer(3000);

        auto cleave = addAISpell(20691, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        cleave->setAttackStopTimer(3000);
    }
};


class GuardMolDar : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GuardMolDar);
    explicit GuardMolDar(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto strike = addAISpell(15580, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        strike->setAttackStopTimer(3000);

        auto shieldSlam = addAISpell(15655, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        shieldSlam->setAttackStopTimer(3000);

        auto cleave = addAISpell(20691, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        cleave->setAttackStopTimer(3000);
    }
};


class GuardSlipKik : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GuardSlipKik);
    explicit GuardSlipKik(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto strike = addAISpell(15580, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        strike->setAttackStopTimer(3000);

        auto knockout = addAISpell(17307, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        knockout->setAttackStopTimer(3000);

        auto cleave = addAISpell(20691, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        cleave->setAttackStopTimer(3000);
    }
};


class Hydrospawn : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Hydrospawn);
    explicit Hydrospawn(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto riptide = addAISpell(22419, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        riptide->setAttackStopTimer(3000);

        auto submersion = addAISpell(22420, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        submersion->setAttackStopTimer(3000);

        auto massiveGeyser = addAISpell(22421, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        massiveGeyser->setAttackStopTimer(3000);
    }
};


class MassiveGeyser : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MassiveGeyser);
    explicit MassiveGeyser(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto water = addAISpell(22422, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        water->setAttackStopTimer(3000);
    }
};


class IllyanaRavenoak : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IllyanaRavenoak);
    explicit IllyanaRavenoak(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto concussiveShot = addAISpell(5116, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        concussiveShot->setAttackStopTimer(3000);

        auto unknown = addAISpell(20904, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknown->setAttackStopTimer(3000);

        auto unknown2 = addAISpell(14290, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknown2->setAttackStopTimer(3000);

        auto unknown3 = addAISpell(14295, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknown3->setAttackStopTimer(3000);
    }
};


class Immolthar : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Immolthar);
    explicit Immolthar(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto infectedBite = addAISpell(16128, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        infectedBite->setAttackStopTimer(3000);

        auto trample = addAISpell(15550, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        trample->setAttackStopTimer(3000);

        auto eyeOfImmolThar = addAISpell(22899, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        eyeOfImmolThar->setAttackStopTimer(3000);
    }
};


class EyeOfImmolThar : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EyeOfImmolThar);
    explicit EyeOfImmolThar(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto eyeOfImmolThar = addAISpell(22909, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        eyeOfImmolThar->setAttackStopTimer(3000);
    }
};


class KingGordok : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KingGordok);
    explicit KingGordok(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto mortalStrike = addAISpell(15708, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        mortalStrike->setAttackStopTimer(3000);

        auto warStomp = addAISpell(24375, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        warStomp->setAttackStopTimer(3000);
    }
};


class Lethtendris : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Lethtendris);
    explicit Lethtendris(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto unknow = addAISpell(11668, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknow->setAttackStopTimer(3000);

        auto shadowBoltVolley = addAISpell(14887, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        shadowBoltVolley->setAttackStopTimer(3000);
    }
};


class LordHelNurath : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LordHelNurath);
    explicit LordHelNurath(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto unknow = addAISpell(10984, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknow->setAttackStopTimer(3000);
    }
};


class MagisterKalendris : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MagisterKalendris);
    explicit MagisterKalendris(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto unknow = addAISpell(10894, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknow->setAttackStopTimer(3000);

        auto unknow2 = addAISpell(10947, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknow2->setAttackStopTimer(3000);

        auto unknow3 = addAISpell(18807, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknow3->setAttackStopTimer(3000);
    }
};


class PrinceTortheldrin : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(PrinceTortheldrin);
    explicit PrinceTortheldrin(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto cleave = addAISpell(20691, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        cleave->setAttackStopTimer(3000);

        auto arcaneBlast = addAISpell(22920, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        arcaneBlast->setAttackStopTimer(3000);
    }
};


class Pusillin : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Pusillin);
    explicit Pusillin(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto blastWave = addAISpell(22424, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        blastWave->setAttackStopTimer(3000);

        auto unknown = addAISpell(10151, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknown->setAttackStopTimer(3000);

        auto fireBlast = addAISpell(16144, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        fireBlast->setAttackStopTimer(3000);
    }
};


class SkarrTheUnbreakable : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SkarrTheUnbreakable);
    explicit SkarrTheUnbreakable(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto cleave = addAISpell(20691, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        cleave->setAttackStopTimer(3000);

        auto warStomp = addAISpell(24375, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        warStomp->setAttackStopTimer(3000);
    }
};


class TendrisWarpwood : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TendrisWarpwood);
    explicit TendrisWarpwood(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto trample = addAISpell(15550, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        trample->setAttackStopTimer(3000);

        auto graspingVines = addAISpell(22924, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        graspingVines->setAttackStopTimer(3000);

        auto entangle = addAISpell(22994, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        entangle->setAttackStopTimer(3000);
    }
};


class ZevrimThornhoof : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ZevrimThornhoof);
    explicit ZevrimThornhoof(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto intensePain = addAISpell(22478, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        intensePain->setAttackStopTimer(3000);

        auto sacrifice = addAISpell(22651, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        sacrifice->setAttackStopTimer(3000);
    }
};


class GordokMageLord : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GordokMageLord);
    explicit GordokMageLord(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto fireBlast = addAISpell(20832, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        fireBlast->setAttackStopTimer(3000);

        auto flamestrike = addAISpell(16102, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        flamestrike->setAttackStopTimer(3000);

        auto frostbolt = addAISpell(15530, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        frostbolt->setAttackStopTimer(3000);

        auto bloodlust = addAISpell(16170, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        bloodlust->setAttackStopTimer(3000);
    }
};


class GordokReaver : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GordokReaver);
    explicit GordokReaver(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto bruisingBlow = addAISpell(22572, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        bruisingBlow->setAttackStopTimer(3000);

        auto uppercut = addAISpell(22916, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        uppercut->setAttackStopTimer(3000);
    }
};


class GordokBrute : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GordokBrute);
    explicit GordokBrute(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto mortalStrike = addAISpell(13737, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        mortalStrike->setAttackStopTimer(3000);

        auto cleave = addAISpell(20677, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        cleave->setAttackStopTimer(3000);

        auto sunderArmor = addAISpell(24317, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        sunderArmor->setAttackStopTimer(3000);
    }
};


class FelLash : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FelLash);
    explicit FelLash(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto arcaneExplosion = addAISpell(22460, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        arcaneExplosion->setAttackStopTimer(3000);

        auto arcaneMissiles = addAISpell(22272, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        arcaneMissiles->setAttackStopTimer(3000);
    }
};

void SetupDireMaul(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_CAPTAIN_KROMCRUSH, &CaptainKromcrush::Create);
    mgr->register_creature_script(CN_CHO_RUSH_OBSERVER, &ChoRushTheObserver::Create);
    mgr->register_creature_script(CN_GUARD_FENGUS, &GuardFengus::Create);
    mgr->register_creature_script(CN_GUARD_MOL_DAR, &GuardMolDar::Create);
    mgr->register_creature_script(CN_GUARD_SLIP_KIK, &GuardSlipKik::Create);
    mgr->register_creature_script(CN_HYDROSPAWN, &Hydrospawn::Create);
    mgr->register_creature_script(CN_MASSIVE_GEYSER, &MassiveGeyser::Create);
    mgr->register_creature_script(CN_ILLYANA_RAVENOAK, &IllyanaRavenoak::Create);
    mgr->register_creature_script(CN_IMMOLTHAR, &Immolthar::Create);
    mgr->register_creature_script(CN_EYE_OF_IMMOLTHAR, &EyeOfImmolThar::Create);
    mgr->register_creature_script(CN_KING_GORDOK, &KingGordok::Create);
    mgr->register_creature_script(CN_LETHENDRIS, &Lethtendris::Create);
    mgr->register_creature_script(CN_LORD_HEL_NURATH, &LordHelNurath::Create);
    mgr->register_creature_script(CN_MAGISTER_KALENDRIS, &MagisterKalendris::Create);
    mgr->register_creature_script(CN_PRINCE_TORTHELDRIN, &PrinceTortheldrin::Create);
    mgr->register_creature_script(CN_PUSILLIN, &Pusillin::Create);
    mgr->register_creature_script(CN_SKARR_THE_UNBREAKABLE, &SkarrTheUnbreakable::Create);
    mgr->register_creature_script(CN_TENDRIS_WARPWOOD, &TendrisWarpwood::Create);
    mgr->register_creature_script(CN_ZEVRIM_THORNHOOF, &ZevrimThornhoof::Create);
    mgr->register_creature_script(CN_GORDOK_MAGE_LORD, &GordokMageLord::Create);
    mgr->register_creature_script(CN_GORDOK_REAVER, &GordokReaver::Create);
    mgr->register_creature_script(CN_GORDOK_BRUTE, &GordokBrute::Create);
    mgr->register_creature_script(CN_FEL_LASH, &FelLash::Create);
}
