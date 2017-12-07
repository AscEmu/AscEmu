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
        addAISpell(SF_DRAINLIFE, 8.0f, TARGET_RANDOM_SINGLE, 0, 35);

        if (_isHeroic())
            addAISpell(SF_DRAINMANA, 8.0f, TARGET_RANDOM_SINGLE, 0, 35);

        ManaRage = sSpellCustomizations.GetSpellInfo(FC_MANARAGE);
        ManaRageTrigger = addAISpell(FC_MANARAGE_TRIGGER, 0.0f, TARGET_SELF, 0, 0);
        FelExplosion = addAISpell(SF_FELEXPLOSION, 0.0f, TARGET_SELF, 0, 0);
        mEnableFelExplosion = false;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        // Selin Fireheart starts with 0 mana and drains it from the felcrystals in the room
        // \todo  Set it so mana regen is off
        getCreature()->setUInt32Value(UNIT_FIELD_POWER1, 0);
    }

    // During the AIUpdate() Selin will spam FelExplosion until hes out of mana
    // He will then attempt to gain mana from a FelCrystal thats in the room by running to them
    void AIUpdate() override
    {
        // 10% of his mana according to wowhead is 3231 which is whats needed to cast FelExplosion
        if (_getManaPercent() < 10 || mEnableFelExplosion == false)
            Mana();
        else if (!_isCasting())// Mana is greater than 10%
            CastFelExplosion();
    }

    void Mana()
    {
        // Attempt to get a Fel Crystal and move to it if not in range.
        // Once in range we get the FelCrystal to cast Mana Rage on Selin
        Unit* FelCrystal = nullptr;
        PreventActions(false);

        FelCrystal = FindFelCrystal();

        if (!FelCrystal || !FelCrystal->isAlive())
        {
            PreventActions(true);
            FelCrystal = nullptr;
            return;
        }

        // Not in range
        if (getCreature()->GetDistance2dSq(FelCrystal) > 100)
        {
            moveTo(FelCrystal->GetPositionX(), FelCrystal->GetPositionY(), FelCrystal->GetPositionZ());
            FelCrystal = nullptr;
            return;
        }

        getCreature()->GetAIInterface()->StopMovement(0);

        if (!FelCrystal->isCastingNonMeleeSpell())
            FelCrystal->CastSpell(getCreature(), ManaRage, false);

        // Mana Rage giving of mana doesnt work so we give 10%(3231) / AIUpdate() Event.
        _castAISpell(ManaRageTrigger);
        uint32 mana = getCreature()->GetPower(POWER_TYPE_MANA) + 3231;
        if (mana >= getCreature()->GetMaxPower(POWER_TYPE_MANA))
            mana = getCreature()->GetMaxPower(POWER_TYPE_MANA);

        getCreature()->setUInt32Value(UNIT_FIELD_POWER1, mana);

        // Re-Enable FelExplosion
        if (_getManaPercent() >= 100)
            PreventActions(true);

        FelCrystal = nullptr;
    }

    void PreventActions(bool Allow)
    {
        mEnableFelExplosion = Allow;
        _setMeleeDisabled(!Allow);
        _setRangedDisabled(!Allow);
        _setCastDisabled(!Allow);
        _setTargetingDisabled(Allow);
    }

    Unit* FindFelCrystal()
    {
        // Find a FelCrystal
        Unit* FC = nullptr;
        for (uint8 x = 0; x < 5; x++)
        {
            FC = getNearestCreature(FelCrystals[x].x, FelCrystals[x].y, FelCrystals[x].z, FelCrystals[x].addition);
            if (!FC || !FC->isAlive() || FC->GetInstanceID() != getCreature()->GetInstanceID())
                FC = nullptr;
            else
                break;
        }
        return FC;
    }

    void CastFelExplosion()
    {
        _castAISpell(FelExplosion);

        // No Idea why the mana isnt taken when the spell is cast so had to manually take it -_-
        getCreature()->setUInt32Value(UNIT_FIELD_POWER1, getCreature()->GetPower(POWER_TYPE_MANA) - 3231);
    }

    SpellInfo* ManaRage;
    CreatureAISpells* ManaRageTrigger;
    CreatureAISpells* FelExplosion;
    bool mEnableFelExplosion;
};

class VexallusAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(VexallusAI);
    VexallusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto chainLighning = addAISpell(VEXALLUS_CHAIN_LIGHTNING, 19.0f, TARGET_ATTACKING, 0, 8);
        chainLighning->setAvailableForScriptPhase({ 1 });

        auto arcaneShock = addAISpell(VEXALLUS_ARCANE_SHOCK, 12.0f, TARGET_ATTACKING, 0, 20, false, true);
        arcaneShock->addEmote("Un...con...tainable.", CHAT_MSG_MONSTER_YELL, 12392);
        arcaneShock->setAvailableForScriptPhase({ 1 });

        auto overload = addAISpell(VEXALLUS_OVERLOAD, 85.0f, TARGET_SELF, 0, 3);
        overload->setAvailableForScriptPhase({ 2 });

        mPureEnergy = addAISpell(VEXALLUS_SUMMON_PURE_ENERGY, 0.0f, TARGET_SELF, 0, 3);

        mSummon = 0;

        addEmoteForEvent(Event_OnCombatStart, 3003);    // Drain... life!
        addEmoteForEvent(Event_OnTargetDied, 3006);     // Con...sume.
        addEmoteForEvent(Event_OnDied, 4861);           // It is... not over.
    }

    void AIUpdate() override
    {
        if ((_getHealthPercent() <= 85 && mSummon == 0) ||
            (_getHealthPercent() <= 70 && mSummon == 1) ||
            (_getHealthPercent() <= 55 && mSummon == 2) ||
            (_getHealthPercent() <= 40 && mSummon == 3) ||
            (_getHealthPercent() <= 25 && mSummon == 4))
        {
            _castAISpell(mPureEnergy);
            ++mSummon;
            //spawnCreature(CN_PURE_ENERGY, 231, -207, 6, 0, true);
        }

        if (_getHealthPercent() <= 10 && isScriptPhase(1))
            setScriptPhase(2);
    }

    CreatureAISpells* mPureEnergy;
    uint8 mSummon;
};

class Priestess_DelrissaAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Priestess_DelrissaAI);
    Priestess_DelrissaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto dispelMagic = addAISpell(PRIESTESS_DELRISSA_DISPEL_MAGIC, 35.0f, TARGET_RANDOM_FRIEND, 0, 5);
        dispelMagic->setMinMaxDistance(0.0f, 30.0f);

        auto flashHeal = addAISpell(PRIESTESS_DELRISSA_FLASH_HEAL, 40.0f, TARGET_RANDOM_FRIEND, 2, 7);
        flashHeal->setMinMaxDistance(0.0f, 40.0f);

        auto shadowwordPain = addAISpell(PRIESTESS_DELRISSA_SHADOWWORD_PAIN, 45.0f, TARGET_RANDOM_SINGLE, 0, 18);
        shadowwordPain->setMinMaxDistance(0.0f, 30.0f);

        auto powerwordShield = addAISpell(PRIESTESS_DELRISSA_POWERWORD_SHIELD, 32.0f, TARGET_RANDOM_FRIEND, 0, 15);
        powerwordShield->setMinMaxDistance(0.0f, 40.0f);

        auto renew = addAISpell(PRIESTESS_DELRISSA_RENEW, 30.0f, TARGET_RANDOM_FRIEND, 0, 18);
        renew->setMinMaxDistance(0.0f, 40.0f);

        mClearHateList = _addTimer(15000);
        mKilledPlayers = 0;

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
            _resetTimer(mClearHateList, 15000);
        }
    }

    protected:

        uint8 mKilledPlayers;
        int32 mClearHateList;
};

class KaganiNightstrikeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KaganiNightstrikeAI);
    KaganiNightstrikeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto kidneyShot = addAISpell(KAGANI_KIDNEY_SHOT, 80.0f, TARGET_ATTACKING, 0, 25);
        kidneyShot->setMinMaxDistance(0.0f, 30.0f);

        auto gouge = addAISpell(KAGANI_GOUGE, 20.0f, TARGET_ATTACKING, 0, 18);
        gouge->setMinMaxDistance(0.0f, 30.0f);

        auto eviscerate = addAISpell(KAGANI_EVISCERATE, 8.0f, TARGET_ATTACKING, 0, 45);
        eviscerate->setMinMaxDistance(0.0f, 30.0f);
    }
};

class EllrysDuskhallowAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EllrysDuskhallowAI);
    EllrysDuskhallowAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ELLRYS_IMMOLATE, 75.0f, TARGET_ATTACKING, 2, 15);

        auto shadowBolt = addAISpell(ELLRYS_SHADOWBOLT, 75.0f, TARGET_RANDOM_SINGLE, 3, 5);
        shadowBolt->setMinMaxDistance(0.0f, 40.0f);

        auto curseOfAgony = addAISpell(ELLRYS_CURSE_OF_AGONY, 75.0f, TARGET_RANDOM_SINGLE, 0, 4);
        curseOfAgony->setMinMaxDistance(0.0f, 30.0f);

        auto fear = addAISpell(ELLRYS_FEAR, 75.0f, TARGET_RANDOM_SINGLE, 2, 9);
        fear->setMinMaxDistance(0.0f, 20.0f);
    }
};

class EramasBrightblazeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EramasBrightblazeAI);
    EramasBrightblazeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ERAMAS_KNOCKDOWN, 25.0f, TARGET_ATTACKING, 0, 5);
        addAISpell(ERAMAS_SNAP_KICK, 40.0f, TARGET_VARIOUS, 0, 2);
    }
};

class YazzaiAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(YazzaiAI);
    YazzaiAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto polymorph = addAISpell(YAZZAI_POLYMORPH, 30.0f, TARGET_RANDOM_SINGLE, 2, 16);
        polymorph->setMinMaxDistance(8.0f, 30.0f);

        addAISpell(YAZZAI_ICE_BLOCK, 20.0f, TARGET_SELF, 0, 300);
        auto blizzard = addAISpell(YAZZAI_BLIZZARD, 25.0f, TARGET_RANDOM_SINGLE, 0, 20);
        blizzard->setMinMaxDistance(8.0f, 30.0f);

        addAISpell(YAZZAI_CONE_OF_COLD, 10.0f, TARGET_SELF, 0, 19);
        auto frostBolt = addAISpell(YAZZAI_FROSTBOLT, 80.0f, TARGET_RANDOM_SINGLE, 3, 14);
        frostBolt->setMinMaxDistance(8.0f, 40.0f);
    }
};

class WarlordSalarisAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(WarlordSalarisAI);
    WarlordSalarisAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto intercept = addAISpell(SALARIS_INTERCEPT, 25.0f, TARGET_RANDOM_SINGLE, 0, 8);
        intercept->setMinMaxDistance(8.0f, 25.0f);

        addAISpell(SALARIS_DISARM, 100.0f, TARGET_ATTACKING, 0, 60);

        addAISpell(SALARIS_PIERCING_HOWL, 22.0f, TARGET_SELF, 0, 17);

        auto shout = addAISpell(SALARIS_FRIGHTENING_SHOUT, 30.0f, TARGET_RANDOM_SINGLE, 0, 9);
        shout->setMinMaxDistance(0.0f, 10.0f);

        addAISpell(SALARIS_HAMSTRING, 10.0f, TARGET_ATTACKING, 0, 20);
        addAISpell(SALARIS_MORTAL_STRIKE, 100.0f, TARGET_ATTACKING, 0, 6);
    }
};

class GaraxxasAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GaraxxasAI);
    GaraxxasAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto aimedShot = addAISpell(GARAXXAS_AIMED_SHOT, 90.0f, TARGET_RANDOM_SINGLE, 3, 6);
        aimedShot->setMinMaxDistance(5.0f, 35.0f);

        auto shoot = addAISpell(GARAXXAS_SHOOT, 90.0f, TARGET_RANDOM_SINGLE, 3, 5);
        shoot->setMinMaxDistance(5.0f, 30.0f);

        auto concussivShot = addAISpell(GARAXXAS_CONCUSSIV_SHOT, 40.0f, TARGET_RANDOM_SINGLE, 0, 8);
        concussivShot->setMinMaxDistance(5.0f, 35.0f);

        auto multiShot = addAISpell(GARAXXAS_MULTI_SHOT, 25.0f, TARGET_RANDOM_SINGLE, 0, 12);
        multiShot->setMinMaxDistance(5.0f, 30.0f);

        addAISpell(GARAXXAS_WING_CLIP, 30.0f, TARGET_ATTACKING, 0, 9);
    }
};

class ApokoAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ApokoAI);
    ApokoAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto frostShock = addAISpell(APOKO_FROST_SHOCK, 40.0f, TARGET_RANDOM_SINGLE, 0, 8);
        frostShock->setMinMaxDistance(0.0f, 20.0f);

        auto healingWave = addAISpell(APOKO_LESSER_HEALING_WAVE, 50.0f, TARGET_RANDOM_FRIEND, 2, 10);
        healingWave->setMinMaxDistance(0.0f, 40.0f);

        auto purge = addAISpell(APOKO_PURGE, 20.0f, TARGET_RANDOM_SINGLE, 0, 40);
        purge->setMinMaxDistance(0.0f, 30.0f);
    }
};

class ZelfanAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ZelfanAI);
    ZelfanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ZELFAN_GOBLIN_DRAGON_GUN, 90.0f, TARGET_ATTACKING, 0, 15);
        addAISpell(ZELFAN_HIGH_EXPLOSIV_SHEEP, 90.0f, TARGET_SELF, 2, 80);

        auto rocketLaunch = addAISpell(ZELFAN_ROCKET_LAUNCH, 99.0f, TARGET_RANDOM_SINGLE, 4, 60);
        rocketLaunch->setMinMaxDistance(0.0f, 45.0f);
    }
};

//Trash mobs

class CoilskarWitchAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CoilskarWitchAI);
    CoilskarWitchAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(COILSKAR_WITCH_FORKED_LIGHTNING, 60.0f, TARGET_ATTACKING, 2, 12);

        auto frostArrow = addAISpell(COILSKAR_WITCH_FROST_ARROW, 15.0f, TARGET_RANDOM_SINGLE, 0, 16);
        frostArrow->setMinMaxDistance(0.0f, 40.0f);

        addAISpell(COILSKAR_WITCH_MANA_SHIELD, 6.0f, TARGET_SELF, 0, 40);

        auto witchShoot = addAISpell(COILSKAR_WITCH_SHOOT, 75.0f, TARGET_RANDOM_SINGLE, 2, 4);
        witchShoot->setMinMaxDistance(5.0f, 30.0f);
    }
};

class SisterOfTormentAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SisterOfTormentAI);
    SisterOfTormentAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SISTER_OF_TORMENT_LASH_OF_PAIN, 60.0f, TARGET_ATTACKING, 0, 8);

        auto deadlyEmbracy = addAISpell(SISTER_OF_TORMENT_DEADLY_EMBRACE, 20.0f, TARGET_RANDOM_SINGLE, 2, 16);
        deadlyEmbracy->setMinMaxDistance(0.0f, 20.0f);
    }
};

class SunbladeBloodKnightAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SunbladeBloodKnightAI);
    SunbladeBloodKnightAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BLOOD_KNIGHT_JUDGEMENT_OF_WRATH, 20.0f, TARGET_ATTACKING, 0, 30);
        addAISpell(BLOOD_KNIGHT_SEAL_OF_WRATH, 99.0f, TARGET_SELF, 0, 30);

        auto holyLight = addAISpell(BLOOD_KNIGHT_HOLY_LIGHT, 10.0f, TARGET_SELF, 2, 30);
        holyLight->setMinMaxDistance(0.0f, 40.0f);
    }
};

class SunbladeImpAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SunbladeImpAI);
    SunbladeImpAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(IMP_FIREBOLT, 100.0f, TARGET_ATTACKING, 2, 3);
    }
};

class SunbladeMageGuardAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SunbladeMageGuardAI);
    SunbladeMageGuardAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(MAGE_GUARD_GLAVE_THROW, 60.0f, TARGET_ATTACKING, 0, 25);

        auto magicDampening = addAISpell(MAGE_GUARD_MAGIC_DAMPENING_FIELD, 20.0f, TARGET_RANDOM_SINGLE, 1, 35);
        magicDampening->setMinMaxDistance(0.0f, 20.0f);
    }
};

class SunbladeMagisterAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SunbladeMagisterAI);
    SunbladeMagisterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(MAGISTER_FROSTBOLT, 65.0f, TARGET_ATTACKING, 2, 4);
        addAISpell(MAGISTER_ARCANE_NOVA, 12.0f, TARGET_SELF, 2, 40);
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
