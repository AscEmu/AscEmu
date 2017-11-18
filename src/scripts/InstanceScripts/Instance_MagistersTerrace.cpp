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
#include <Spell/Definitions/PowerType.h>

//////////////////////////////////////////////////////////////////////////////////////////
//Magister's Terrace
class InstanceMagistersTerraceScript : public InstanceScript
{
    public:

        InstanceMagistersTerraceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
        {}

        static InstanceScript* Create(MapMgr* pMapMgr) { return new InstanceMagistersTerraceScript(pMapMgr); }
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

class SelinFireheartAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SelinFireheartAI);
    SelinFireheartAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(SF_DRAINLIFE, Target_RandomPlayer, 8, 0, 35);

        if (_isHeroic())
            AddSpell(SF_DRAINMANA, Target_RandomPlayer, 8, 0, 35);

        ManaRage = sSpellCustomizations.GetSpellInfo(FC_MANARAGE);
        ManaRageTrigger = AddSpell(FC_MANARAGE_TRIGGER, Target_Self, 0, 0, 0);
        FelExplosion = AddSpell(SF_FELEXPLOSION, Target_Self, 0, 0, 0);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        /*
            Selin Fireheart starts with 0 mana and drains it from the felcrystals in the room
            \todo  Set it so mana regen is off
            */
        getCreature()->setUInt32Value(UNIT_FIELD_POWER1, 0);
        
    }

    /*
        During the AIUpdate() Selin will spam FelExplosion until hes out of mana
        He will then attempt to gain mana from a FelCrystal thats in the room by running to them
        */
    void AIUpdate() override
    {
        // 10% of his mana according to wowhead is 3231 which is whats needed to cast FelExplosion
        if (_getManaPercent() < 10 || FelExplosion->mEnabled == false)
            Mana();
        else if (!_isCasting())// Mana is greater than 10%
            CastFelExplosion();

        
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
        if (getCreature()->GetDistance2dSq(FelCrystal) > 100)
        {
            moveTo(FelCrystal->GetPositionX(), FelCrystal->GetPositionY(), FelCrystal->GetPositionZ());
            FelCrystal = NULL;
            return;
        }

        getCreature()->GetAIInterface()->StopMovement(0);

        if (!FelCrystal->isCastingNonMeleeSpell())
            FelCrystal->CastSpell(getCreature(), ManaRage, false);

        // Mana Rage giving of mana doesnt work so we give 10%(3231) / AIUpdate() Event.
        CastSpellNowNoScheduling(ManaRageTrigger);
        uint32 mana = getCreature()->GetPower(POWER_TYPE_MANA) + 3231;
        if (mana >= getCreature()->GetMaxPower(POWER_TYPE_MANA))
            mana = getCreature()->GetMaxPower(POWER_TYPE_MANA);

        getCreature()->setUInt32Value(UNIT_FIELD_POWER1, mana);

        // Re-Enable FelExplosion
        if (_getManaPercent() >= 100)
            PreventActions(true);

        FelCrystal = NULL;
    }

    void PreventActions(bool Allow)
    {
        FelExplosion->mEnabled = Allow;
        _setMeleeDisabled(!Allow);
        _setRangedDisabled(!Allow);
        _setCastDisabled(!Allow);
        _setTargetingDisabled(Allow);
    }

    Unit* FindFelCrystal()
    {
        /*
            Find a FelCrystal
            */
        Unit* FC = NULL;
        for (uint8 x = 0; x < 5; x++)
        {
            FC = getNearestCreature(FelCrystals[x].x, FelCrystals[x].y, FelCrystals[x].z, FelCrystals[x].addition);
            if (!FC || !FC->isAlive() || FC->GetInstanceID() != getCreature()->GetInstanceID())
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
        getCreature()->setUInt32Value(UNIT_FIELD_POWER1, getCreature()->GetPower(POWER_TYPE_MANA) - 3231);
    }

    SpellInfo* ManaRage;
    SpellDesc* ManaRageTrigger;
    SpellDesc* FelExplosion;
};


// Vexallus
class VexallusAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(VexallusAI);
    VexallusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddPhaseSpell(1, AddSpell(VEXALLUS_CHAIN_LIGHTNING, Target_Current, 19, 0, 8, 0, 0));
        AddPhaseSpell(1, AddSpell(VEXALLUS_ARCANE_SHOCK, Target_ClosestPlayer, 12, 0, 20, 0, 0, true, "Un...con...tainable.", CHAT_MSG_MONSTER_YELL, 12392));
        AddPhaseSpell(2, AddSpell(VEXALLUS_OVERLOAD, Target_Self, 85, 0, 3, 0, 0));
        mPureEnergy = AddSpell(VEXALLUS_SUMMON_PURE_ENERGY, Target_Self, 85, 0, 3);

        mSummon = 0;

        // new
        addEmoteForEvent(Event_OnCombatStart, 3003);     // Drain... life!
        addEmoteForEvent(Event_OnTargetDied, 3006);     // Con...sume.
        addEmoteForEvent(Event_OnDied, 4861);     // It is... not over.
    }

    void AIUpdate() override
    {
        if ((_getHealthPercent() <= 85 && mSummon == 0) ||
            (_getHealthPercent() <= 70 && mSummon == 1) ||
            (_getHealthPercent() <= 55 && mSummon == 2) ||
            (_getHealthPercent() <= 40 && mSummon == 3) ||
            (_getHealthPercent() <= 25 && mSummon == 4))
        {
            CastSpell(mPureEnergy);
            ++mSummon;
            //spawnCreature(CN_PURE_ENERGY, 231, -207, 6, 0, true);
        }

        if (_getHealthPercent() <= 10 && isScriptPhase(1))
            setScriptPhase(2);
    }

    SpellDesc* mPureEnergy;
    uint8 mSummon;
};


//Priestess Delrissa
class Priestess_DelrissaAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Priestess_DelrissaAI);
    Priestess_DelrissaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(PRIESTESS_DELRISSA_DISPEL_MAGIC, Target_RandomFriendly, 35, 0, 5, 0, 30);
        AddSpell(PRIESTESS_DELRISSA_FLASH_HEAL, Target_RandomFriendly, 40, 1.5, 7, 0, 40);
        AddSpell(PRIESTESS_DELRISSA_SHADOWWORD_PAIN, Target_RandomPlayer, 45, 0, 18, 0, 30);
        AddSpell(PRIESTESS_DELRISSA_POWERWORD_SHIELD, Target_RandomFriendly, 32, 0, 15, 0, 40);
        AddSpell(PRIESTESS_DELRISSA_RENEW, Target_RandomFriendly, 30, 0, 18, 0, 40);

        mClearHateList = _addTimer(15000);
        mKilledPlayers = 0;

        // new
        addEmoteForEvent(Event_OnCombatStart, 3022);     // Annihilate them.
        addEmoteForEvent(Event_OnDied, 3032);     // Not what I had... planned.
    }

    void OnTargetDied(Unit* pTarget) override
    {
        if (!pTarget || !pTarget->IsPlayer())
            return;

        ++mKilledPlayers;

        if (mKilledPlayers == 1)
            sendDBChatMessage(3027);     // I call that a good start.
        else if (mKilledPlayers == 2)
            sendDBChatMessage(3028);     // I could have sworn there were more of you...
        else if (mKilledPlayers == 3)
            sendDBChatMessage(3029);     // Not really much of a "group" anymore, is it?
        else if (mKilledPlayers == 4)
            sendDBChatMessage(3030);     // One is such a lonely number.
        else if (mKilledPlayers == 5)
            sendDBChatMessage(3031);     // It's been a kick, really.  
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        sendDBChatMessage(3031);     // It's been a kick, really.
        mKilledPlayers = 0;
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mClearHateList))
        {
            _clearHateList();
            AggroRandomUnit();
            _resetTimer(mClearHateList, 15000);
        }
    }

    protected:

        uint8 mKilledPlayers;
        int32 mClearHateList;
};


//Kagani Nightstrike
class KaganiNightstrikeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KaganiNightstrikeAI);
    KaganiNightstrikeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(KAGANI_KIDNEY_SHOT, Target_Current, 80, 0, 25, 0, 30);
        AddSpell(KAGANI_GOUGE, Target_ClosestPlayer, 20, 0, 18, 0, 30);
        AddSpell(KAGANI_EVISCERATE, Target_Current, 8, 0, 45, 0, 30);
    }
};

//Ellrys Duskhallow
class EllrysDuskhallowAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EllrysDuskhallowAI);
    EllrysDuskhallowAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(ELLRYS_IMMOLATE, Target_Current, 75, 2, 15, 0, 30);
        AddSpell(ELLRYS_SHADOWBOLT, Target_RandomPlayer, 75, 3, 5, 4, 40);
        AddSpell(ELLRYS_CURSE_OF_AGONY, Target_RandomPlayer, 75, 0, 4, 0, 30);
        AddSpell(ELLRYS_FEAR, Target_RandomPlayer, 75, 1.5, 9, 0, 20);
    }
};

//Eramas Brightblaze
class EramasBrightblazeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EramasBrightblazeAI);
    EramasBrightblazeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(ERAMAS_KNOCKDOWN, Target_Current, 25, 0, 5, 0, 5);
        AddSpell(ERAMAS_SNAP_KICK, Target_SecondMostHated, 40, 0, 2, 0, 5);
    }
};

//Yazzai
class YazzaiAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(YazzaiAI);
    YazzaiAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(YAZZAI_POLYMORPH, Target_RandomPlayer, 30, 1.5, 16, 0, 30);
        AddSpell(YAZZAI_ICE_BLOCK, Target_Self, 20, 0, 300, 0, 1);
        AddSpell(YAZZAI_BLIZZARD, Target_RandomPlayer, 25, 0, 20, 0, 30);
        AddSpell(YAZZAI_CONE_OF_COLD, Target_Self, 10, 0, 19, 0, 1);
        AddSpell(YAZZAI_FROSTBOLT, Target_RandomPlayer, 80, 3, 14, 0, 40);
    }
};

//Warlord Salaris
class WarlordSalarisAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(WarlordSalarisAI);
    WarlordSalarisAI(Creature* pCreature) : CreatureAIScript(pCreature)
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
class GaraxxasAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GaraxxasAI);
    GaraxxasAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(GARAXXAS_AIMED_SHOT, Target_RandomPlayer, 90, 3, 6, 5, 35);
        AddSpell(GARAXXAS_SHOOT, Target_RandomPlayer, 90, 2.5, 5, 5, 30);
        AddSpell(GARAXXAS_CONCUSSIV_SHOT, Target_RandomPlayer, 40, 0, 8, 5, 35);
        AddSpell(GARAXXAS_MULTI_SHOT, Target_RandomPlayer, 25, 0, 12, 5, 30);
        AddSpell(GARAXXAS_WING_CLIP, Target_Current, 30, 0, 9, 0, 5);
    }
};

//Apoko
class ApokoAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ApokoAI);
    ApokoAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(APOKO_FROST_SHOCK, Target_RandomPlayer, 40, 0, 8, 0, 20);
        AddSpell(APOKO_LESSER_HEALING_WAVE, Target_RandomFriendly, 50, 1.5, 10, 0, 40);
        AddSpell(APOKO_PURGE, Target_RandomUnit, 20, 0, 40, 0, 30);
    }
};

//Zelfan
class ZelfanAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ZelfanAI);
    ZelfanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(ZELFAN_GOBLIN_DRAGON_GUN, Target_Current, 90, 0, 15, 0, 5);
        AddSpell(ZELFAN_HIGH_EXPLOSIV_SHEEP, Target_Self, 90, 2, 80);
        AddSpell(ZELFAN_ROCKET_LAUNCH, Target_RandomPlayer, 99, 3.5, 60, 0, 45);
    }
};

//Trash mobs

//Coilskar Witch
class CoilskarWitchAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CoilskarWitchAI);
    CoilskarWitchAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(COILSKAR_WITCH_FORKED_LIGHTNING, Target_Current, 60, 2, 12, 0, 30);
        AddSpell(COILSKAR_WITCH_FROST_ARROW, Target_RandomPlayer, 15, 0, 16, 0, 40);
        AddSpell(COILSKAR_WITCH_MANA_SHIELD, Target_Self, 6, 0, 40, 0, 0);
        AddSpell(COILSKAR_WITCH_SHOOT, Target_RandomPlayer, 75, 1.5, 4, 5, 30);
    }
};

//Sister of Torment
class SisterOfTormentAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SisterOfTormentAI);
    SisterOfTormentAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(SISTER_OF_TORMENT_LASH_OF_PAIN, Target_Current, 60, 0, 8, 0, 5);
        AddSpell(SISTER_OF_TORMENT_DEADLY_EMBRACE, Target_RandomPlayer, 20, 1.5, 16, 0, 20);
    }
};

//Sunblade Blood Knight
class SunbladeBloodKnightAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SunbladeBloodKnightAI);
    SunbladeBloodKnightAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(BLOOD_KNIGHT_JUDGEMENT_OF_WRATH, Target_Current, 20, 0, 30, 0, 5);
        AddSpell(BLOOD_KNIGHT_SEAL_OF_WRATH, Target_Self, 99, 0, 30, 0, 0);
        AddSpell(BLOOD_KNIGHT_HOLY_LIGHT, Target_Self, 10, 2, 30, 0, 40);
    }
};

//Sunblade Imp
class SunbladeImpAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SunbladeImpAI);
    SunbladeImpAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(IMP_FIREBOLT, Target_Current, 100, 2, (int32)2.5, 0, 30);
    }
};

//Sunblade Mage Guard
class SunbladeMageGuardAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SunbladeMageGuardAI);
    SunbladeMageGuardAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(MAGE_GUARD_GLAVE_THROW, Target_Current, 60, 0, 25, 0, 5);
        AddSpell(MAGE_GUARD_MAGIC_DAMPENING_FIELD, Target_RandomPlayer, 20, 1, 35, 0, 20);
    }
};

//Sunblade Magister
class SunbladeMagisterAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SunbladeMagisterAI);
    SunbladeMagisterAI(Creature* pCreature) : CreatureAIScript(pCreature)
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
