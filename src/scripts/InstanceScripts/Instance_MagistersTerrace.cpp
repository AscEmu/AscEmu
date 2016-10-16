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
#include "Instance_MagistersTerrace.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Magister's Terrace
class InstanceMagistersTerraceScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceMagistersTerraceScript, MoonInstanceScript);
        InstanceMagistersTerraceScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            // Way to select bosses
            BuildEncounterMap();
            if (mEncounters.size() == 0)
                return;

            for (EncounterMap::iterator Iter = mEncounters.begin(); Iter != mEncounters.end(); ++Iter)
            {
                if ((*Iter).second.mState != State_Finished)
                    continue;
            }
        }

        void OnGameObjectPushToWorld(GameObject* pGameObject) { }

        void SetInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = (EncounterState)pData;
        }

        uint32 GetInstanceData(uint32 pType, uint32 pIndex)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return 0;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return 0;

            return (*Iter).second.mState;
        }

        void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
        {
            EncounterMap::iterator Iter = mEncounters.find(pCreature->GetEntry());
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = State_Finished;

            return;
        }
};

// Selin Firehart Encounter
// Fel Crystal Spawn Locations
static LocationExtra FelCrystals[] =
{
    { 225.969f, -20.0775f, -2.9731f, 0.942478f, CN_TRASH_FELCRYSTALS },
    { 226.314f, 20.2183f, -2.98127f, 5.32325f, CN_TRASH_FELCRYSTALS },
    { 247.888f, -14.6252f, 3.80777f, 2.33874f, CN_TRASH_FELCRYSTALS },
    { 248.053f, 14.592f, 3.74882f, 3.94444f, CN_TRASH_FELCRYSTALS },
    { 263.149f, 0.309245f, 1.32057f, 3.15905f, CN_TRASH_FELCRYSTALS }
};

class SelinFireheartAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SelinFireheartAI, MoonScriptCreatureAI);
    SelinFireheartAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SF_DRAINLIFE, Target_RandomPlayer, 8, 0, 35);

        if (_unit->GetMapMgr()->iInstanceMode == MODE_HEROIC)
            AddSpell(SF_DRAINMANA, Target_RandomPlayer, 8, 0, 35);

        ManaRage = sSpellCustomizations.GetSpellInfo(FC_MANARAGE);
        ManaRageTrigger = AddSpell(FC_MANARAGE_TRIGGER, Target_Self, 0, 0, 0);
        FelExplosion = AddSpell(SF_FELEXPLOSION, Target_Self, 0, 0, 0);
    }

    void OnCombatStart(Unit* pTarget)
    {
        /*
            Selin Fireheart starts with 0 mana and drains it from the felcrystals in the room
            \todo  Set it so mana regen is off
            */
        _unit->SetUInt32Value(UNIT_FIELD_POWER1, 0);
        ParentClass::OnCombatStart(pTarget);
    }

    /*
        During the AIUpdate() Selin will spam FelExplosion until hes out of mana
        He will then attempt to gain mana from a FelCrystal thats in the room by running to them
        */
    void AIUpdate()
    {
        // 10% of his mana according to wowhead is 3231 which is whats needed to cast FelExplosion
        if (GetManaPercent() < 10 || FelExplosion->mEnabled == false)
            Mana();
        else if (!IsCasting())// Mana is greater than 10%
            CastFelExplosion();

        ParentClass::AIUpdate();
    }

    void Mana()
    {
        /*
            Attempt to get a Fel Crystal and move to it if not in range.
            Once in range we get the FelCrystal to cast Mana Rage on Selin
            */
        Unit* FelCrystal = NULL;
        PreventActions(false);

        FelCrystal = FindFelCrystal();

        if (!FelCrystal || !FelCrystal->isAlive())
        {
            PreventActions(true);
            FelCrystal = NULL;
            return;
        }

        // Not in range
        if (_unit->GetDistance2dSq(FelCrystal) > 100)
        {
            MoveTo(FelCrystal->GetPositionX(), FelCrystal->GetPositionY(), FelCrystal->GetPositionZ());
            FelCrystal = NULL;
            return;
        }

        _unit->GetAIInterface()->StopMovement(0);

        if (!FelCrystal->GetCurrentSpell())
            FelCrystal->CastSpell(_unit, ManaRage, false);

        // Mana Rage giving of mana doesnt work so we give 10%(3231) / AIUpdate() Event.
        CastSpellNowNoScheduling(ManaRageTrigger);
        uint32 mana = _unit->GetPower(POWER_TYPE_MANA) + 3231;
        if (mana >= _unit->GetMaxPower(POWER_TYPE_MANA))
            mana = _unit->GetMaxPower(POWER_TYPE_MANA);

        _unit->SetUInt32Value(UNIT_FIELD_POWER1, mana);

        // Re-Enable FelExplosion
        if (GetManaPercent() >= 100)
            PreventActions(true);

        FelCrystal = NULL;
    }

    void PreventActions(bool Allow)
    {
        FelExplosion->mEnabled = Allow;
        SetAllowMelee(Allow);
        SetAllowRanged(Allow);
        SetAllowSpell(Allow);
        SetAllowTargeting(Allow);
    }

    Unit* FindFelCrystal()
    {
        /*
            Find a FelCrystal
            */
        Unit* FC = NULL;
        for (uint8 x = 0; x < 5; x++)
        {
            FC = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(FelCrystals[x].x, FelCrystals[x].y, FelCrystals[x].z, FelCrystals[x].addition);
            if (!FC || !FC->isAlive() || FC->GetInstanceID() != _unit->GetInstanceID())
                FC = NULL;
            else
                break;
        }
        return FC;
    }

    void CastFelExplosion()
    {
        CastSpellNowNoScheduling(FelExplosion);

        // No Idea why the mana isnt taken when the spell is cast so had to manually take it -_-
        _unit->SetUInt32Value(UNIT_FIELD_POWER1, _unit->GetPower(POWER_TYPE_MANA) - 3231);
    }

    SpellInfo* ManaRage;
    SpellDesc* ManaRageTrigger;
    SpellDesc* FelExplosion;
};


// Vexallus
class VexallusAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(VexallusAI, MoonScriptBossAI);
    VexallusAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddPhaseSpell(1, AddSpell(VEXALLUS_CHAIN_LIGHTNING, Target_Current, 19, 0, 8, 0, 0));
        AddPhaseSpell(1, AddSpell(VEXALLUS_ARCANE_SHOCK, Target_ClosestPlayer, 12, 0, 20, 0, 0, true, "Un...con...tainable.", Text_Yell, 12392));
        AddPhaseSpell(2, AddSpell(VEXALLUS_OVERLOAD, Target_Self, 85, 0, 3, 0, 0));
        mPureEnergy = AddSpell(VEXALLUS_SUMMON_PURE_ENERGY, Target_Self, 85, 0, 3);

        mSummon = 0;
    }

    void OnCombatStart(Unit* pTarget)
    {
        _unit->SendScriptTextChatMessage(3003);     // Drain... life!

        SetPhase(1);
        ParentClass::OnCombatStart(pTarget);
    }

    void OnTargetDied(Unit* pTarget)
    {
        _unit->SendScriptTextChatMessage(3006);     // Con...sume.
    }

    void AIUpdate()
    {
        if ((GetHealthPercent() <= 85 && mSummon == 0) ||
            (GetHealthPercent() <= 70 && mSummon == 1) ||
            (GetHealthPercent() <= 55 && mSummon == 2) ||
            (GetHealthPercent() <= 40 && mSummon == 3) ||
            (GetHealthPercent() <= 25 && mSummon == 4))
        {
            CastSpell(mPureEnergy);
            ++mSummon;
            //SpawnCreature(CN_PURE_ENERGY, 231, -207, 6, 0, true);
        }

        if (GetHealthPercent() <= 10 && GetPhase() == 1)
            SetPhase(2);


        ParentClass::AIUpdate();
    }

    SpellDesc* mPureEnergy;
    uint8 mSummon;
};


//Priestess Delrissa
class Priestess_DelrissaAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(Priestess_DelrissaAI, MoonScriptBossAI);
    Priestess_DelrissaAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(PRIESTESS_DELRISSA_DISPEL_MAGIC, Target_RandomFriendly, 35, 0, 5, 0, 30);
        AddSpell(PRIESTESS_DELRISSA_FLASH_HEAL, Target_RandomFriendly, 40, 1.5, 7, 0, 40);
        AddSpell(PRIESTESS_DELRISSA_SHADOWWORD_PAIN, Target_RandomPlayer, 45, 0, 18, 0, 30);
        AddSpell(PRIESTESS_DELRISSA_POWERWORD_SHIELD, Target_RandomFriendly, 32, 0, 15, 0, 40);
        AddSpell(PRIESTESS_DELRISSA_RENEW, Target_RandomFriendly, 30, 0, 18, 0, 40);

        mClearHateList = AddTimer(15000);
        mKilledPlayers = 0;
    };

    void OnCombatStart(Unit* pTarget)
    {
        _unit->SendScriptTextChatMessage(3022);     // Annihilate them.
        //AggroRandomUnit();    // Want to aggro random unit ? Set it instead of calling premade
        // method that in this case recursively loops this procedure

        ParentClass::OnCombatStart(pTarget);
    };

    void OnTargetDied(Unit* pTarget)
    {
        if (!pTarget || !pTarget->IsPlayer())
            return;

        ++mKilledPlayers;

        if (mKilledPlayers == 1)
            _unit->SendScriptTextChatMessage(3027);     // I call that a good start.
        else if (mKilledPlayers == 2)
            _unit->SendScriptTextChatMessage(3028);     // I could have sworn there were more of you...
        else if (mKilledPlayers == 3)
            _unit->SendScriptTextChatMessage(3029);     // Not really much of a "group" anymore, is it?
        else if (mKilledPlayers == 4)
            _unit->SendScriptTextChatMessage(3030);     // One is such a lonely number.
        else if (mKilledPlayers == 5)
            _unit->SendScriptTextChatMessage(3031);     // It's been a kick, really.

        ParentClass::OnTargetDied(pTarget);
    }

    void OnCombatStop(Unit* pTarget)
    {
        _unit->SendScriptTextChatMessage(3031);     // It's been a kick, really.
        mKilledPlayers = 0;

        ParentClass::OnCombatStop(pTarget);
    }

    void OnDied(Unit* pKiller)
    {
        _unit->SendScriptTextChatMessage(3032);     // Not what I had... planned.
    }

    void AIUpdate()
    {
        if (IsTimerFinished(mClearHateList))
        {
            ClearHateList();
            AggroRandomUnit();
            ResetTimer(mClearHateList, 15000);
        };

        ParentClass::AIUpdate();
    };

    protected:

        uint8 mKilledPlayers;
        int32 mClearHateList;
};


//Kagani Nightstrike
class KaganiNightstrikeAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(KaganiNightstrikeAI, MoonScriptBossAI);
    KaganiNightstrikeAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(KAGANI_KIDNEY_SHOT, Target_Current, 80, 0, 25, 0, 30);
        AddSpell(KAGANI_GOUGE, Target_ClosestPlayer, 20, 0, 18, 0, 30);
        AddSpell(KAGANI_EVISCERATE, Target_Current, 8, 0, 45, 0, 30);
    }
};

//Ellrys Duskhallow
class EllrysDuskhallowAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(EllrysDuskhallowAI, MoonScriptBossAI);
    EllrysDuskhallowAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(ELLRYS_IMMOLATE, Target_Current, 75, 2, 15, 0, 30);
        AddSpell(ELLRYS_SHADOWBOLT, Target_RandomPlayer, 75, 3, 5, 4, 40);
        AddSpell(ELLRYS_CURSE_OF_AGONY, Target_RandomPlayer, 75, 0, 4, 0, 30);
        AddSpell(ELLRYS_FEAR, Target_RandomPlayer, 75, 1.5, 9, 0, 20);
    }

};

//Eramas Brightblaze
class EramasBrightblazeAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(EramasBrightblazeAI, MoonScriptBossAI);
    EramasBrightblazeAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(ERAMAS_KNOCKDOWN, Target_Current, 25, 0, 5, 0, 5);
        AddSpell(ERAMAS_SNAP_KICK, Target_SecondMostHated, 40, 0, 2, 0, 5);
    }

};

//Yazzai
class YazzaiAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(YazzaiAI, MoonScriptBossAI);
    YazzaiAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(YAZZAI_POLYMORPH, Target_RandomPlayer, 30, 1.5, 16, 0, 30);
        AddSpell(YAZZAI_ICE_BLOCK, Target_Self, 20, 0, 300, 0, 1);
        AddSpell(YAZZAI_BLIZZARD, Target_RandomPlayer, 25, 0, 20, 0, 30);
        AddSpell(YAZZAI_CONE_OF_COLD, Target_Self, 10, 0, 19, 0, 1);
        AddSpell(YAZZAI_FROSTBOLT, Target_RandomPlayer, 80, 3, 14, 0, 40);
    }

};

//Warlord Salaris
class WarlordSalarisAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(WarlordSalarisAI, MoonScriptBossAI);
    WarlordSalarisAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        //AddSpell(uint32 pSpellId, TargetType pTargetType, float pChance, float pCastTime, int32 pCooldown, float pMinRange, float pMaxRange
        AddSpell(SALARIS_INTERCEPT, Target_RandomPlayer, 25, 0, 8, 8, 25);
        AddSpell(SALARIS_DISARM, Target_Current, 100, 0, 60, 0, 5);
        AddSpell(SALARIS_PIERCING_HOWL, Target_Self, 22, 0, 17, 0, 1);
        AddSpell(SALARIS_FRIGHTENING_SHOUT, Target_RandomPlayer, 30, 0, 9, 0, 10);
        AddSpell(SALARIS_HAMSTRING, Target_ClosestPlayer, 10, 0, 20, 0, 5);
        AddSpell(SALARIS_MORTAL_STRIKE, Target_Current, 100, 0, 6, 0, 5);
    }

};

//Geraxxas
class GaraxxasAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(GaraxxasAI, MoonScriptBossAI);
    GaraxxasAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(GARAXXAS_AIMED_SHOT, Target_RandomPlayer, 90, 3, 6, 5, 35);
        AddSpell(GARAXXAS_SHOOT, Target_RandomPlayer, 90, 2.5, 5, 5, 30);
        AddSpell(GARAXXAS_CONCUSSIV_SHOT, Target_RandomPlayer, 40, 0, 8, 5, 35);
        AddSpell(GARAXXAS_MULTI_SHOT, Target_RandomPlayer, 25, 0, 12, 5, 30);
        AddSpell(GARAXXAS_WING_CLIP, Target_Current, 30, 0, 9, 0, 5);
    }

};

//Apoko
class ApokoAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(ApokoAI, MoonScriptCreatureAI);
    ApokoAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(APOKO_FROST_SHOCK, Target_RandomPlayer, 40, 0, 8, 0, 20);
        AddSpell(APOKO_LESSER_HEALING_WAVE, Target_RandomFriendly, 50, 1.5, 10, 0, 40);
        AddSpell(APOKO_PURGE, Target_RandomUnit, 20, 0, 40, 0, 30);
    }

};

//Zelfan
class ZelfanAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(ZelfanAI, MoonScriptCreatureAI);
    ZelfanAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(ZELFAN_GOBLIN_DRAGON_GUN, Target_Current, 90, 0, 15, 0, 5);
        AddSpell(ZELFAN_HIGH_EXPLOSIV_SHEEP, Target_Self, 90, 2, 80);
        AddSpell(ZELFAN_ROCKET_LAUNCH, Target_RandomPlayer, 99, 3.5, 60, 0, 45);
    }

};

//Trash mobs

//Coilskar Witch
class CoilskarWitchAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(CoilskarWitchAI, MoonScriptBossAI);
    CoilskarWitchAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(COILSKAR_WITCH_FORKED_LIGHTNING, Target_Current, 60, 2, 12, 0, 30);
        AddSpell(COILSKAR_WITCH_FROST_ARROW, Target_RandomPlayer, 15, 0, 16, 0, 40);
        AddSpell(COILSKAR_WITCH_MANA_SHIELD, Target_Self, 6, 0, 40, 0, 0);
        AddSpell(COILSKAR_WITCH_SHOOT, Target_RandomPlayer, 75, 1.5, 4, 5, 30);
    }

};

//Sister of Torment
class SisterOfTormentAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SisterOfTormentAI, MoonScriptBossAI);
    SisterOfTormentAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(SISTER_OF_TORMENT_LASH_OF_PAIN, Target_Current, 60, 0, 8, 0, 5);
        AddSpell(SISTER_OF_TORMENT_DEADLY_EMBRACE, Target_RandomPlayer, 20, 1.5, 16, 0, 20);
    }

};

//Sunblade Blood Knight
class SunbladeBloodKnightAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SunbladeBloodKnightAI, MoonScriptBossAI);
    SunbladeBloodKnightAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(BLOOD_KNIGHT_JUDGEMENT_OF_WRATH, Target_Current, 20, 0, 30, 0, 5);
        AddSpell(BLOOD_KNIGHT_SEAL_OF_WRATH, Target_Self, 99, 0, 30, 0, 0);
        AddSpell(BLOOD_KNIGHT_HOLY_LIGHT, Target_Self, 10, 2, 30, 0, 40);
    }

};

//Sunblade Imp
class SunbladeImpAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SunbladeImpAI, MoonScriptBossAI);
    SunbladeImpAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(IMP_FIREBOLT, Target_Current, 100, 2, (int32)2.5, 0, 30);
    }

};

//Sunblade Mage Guard
class SunbladeMageGuardAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SunbladeMageGuardAI, MoonScriptBossAI);
    SunbladeMageGuardAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(MAGE_GUARD_GLAVE_THROW, Target_Current, 60, 0, 25, 0, 5);
        AddSpell(MAGE_GUARD_MAGIC_DAMPENING_FIELD, Target_RandomPlayer, 20, 1, 35, 0, 20);
    }

};

//Sunblade Magister
class SunbladeMagisterAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SunbladeMagisterAI, MoonScriptBossAI);
    SunbladeMagisterAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(MAGISTER_FROSTBOLT, Target_Current, 65, 2, 4, 0, 30);
        AddSpell(MAGISTER_ARCANE_NOVA, Target_Self, 12, 1.5, 40, 0, 0);
    }

};

void SetupMagistersTerrace(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_MAGISTERS_TERRACE, &InstanceMagistersTerraceScript::Create);

    //Bosses
    mgr->register_creature_script(BOSS_SELIN_FIREHEART, &SelinFireheartAI::Create);
    mgr->register_creature_script(BOSS_VEXALLUS, &VexallusAI::Create);
    mgr->register_creature_script(BOSS_PRIEST_DELRISSA, &Priestess_DelrissaAI::Create);
    //Priestess Delrissa Encounter Creature AI
    mgr->register_creature_script(CN_KAGANI_NIGHTSTRIKE, &KaganiNightstrikeAI::Create);
    mgr->register_creature_script(CN_ELLRYS_DUSKHALLOW, &EllrysDuskhallowAI::Create);
    mgr->register_creature_script(CN_ERAMAS_BRIGHTBLAZE, &EramasBrightblazeAI::Create);
    mgr->register_creature_script(CN_YAZZAI, &YazzaiAI::Create);
    mgr->register_creature_script(CN_WARLORD_SALARIS, &WarlordSalarisAI::Create);
    mgr->register_creature_script(CN_GARAXXAS, &GaraxxasAI::Create);
    mgr->register_creature_script(CN_APOKO, &ApokoAI::Create);
    mgr->register_creature_script(CN_ZELFAN, &ZelfanAI::Create);
    //Trash Mobs
    mgr->register_creature_script(CN_COILSKAR_WITCH, &CoilskarWitchAI::Create);
    mgr->register_creature_script(CN_SISTER_OF_TORMENT, &SisterOfTormentAI::Create);
    mgr->register_creature_script(CN_SB_IMP, &SunbladeImpAI::Create);
    mgr->register_creature_script(CN_SB_MAGE_GUARD, &SunbladeMageGuardAI::Create);
    mgr->register_creature_script(CN_SB_MAGISTER, &SunbladeMagisterAI::Create);
}
