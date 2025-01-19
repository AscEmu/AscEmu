/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_MagistersTerrace.h"

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Magister's Terrace
class InstanceMagistersTerraceScript : public InstanceScript
{
public:
    explicit InstanceMagistersTerraceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new InstanceMagistersTerraceScript(pMapMgr); }
};

// Fel Crystal Spawn Locations
static LocationVector FelCrystals[] =
{
    { 225.969f, -20.0775f, -2.9731f, 0.942478f },
    { 226.314f, 20.2183f, -2.98127f, 5.32325f },
    { 247.888f, -14.6252f, 3.80777f, 2.33874f },
    { 248.053f, 14.592f, 3.74882f, 3.94444f },
    { 263.149f, 0.309245f, 1.32057f, 3.15905f }
};

class SelinFireheartAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SelinFireheartAI(c); }
    explicit SelinFireheartAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SF_DRAINLIFE, 8.0f, TARGET_RANDOM_SINGLE, 0, 35);

        if (isHeroic())
            addAISpell(SF_DRAINMANA, 8.0f, TARGET_RANDOM_SINGLE, 0, 35);

        ManaRage = sSpellMgr.getSpellInfo(FC_MANARAGE);
        ManaRageTrigger = addAISpell(FC_MANARAGE_TRIGGER, 0.0f, TARGET_SELF, 0, 0);
        FelExplosion = addAISpell(SF_FELEXPLOSION, 0.0f, TARGET_SELF, 0, 0);
        mEnableFelExplosion = false;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        // Selin Fireheart starts with 0 mana and drains it from the felcrystals in the room
        // \todo  Set it so mana regen is off
        getCreature()->setPower(POWER_TYPE_MANA, 0);
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

        getCreature()->stopMoving();

        if (!FelCrystal->isCastingSpell())
            FelCrystal->castSpell(getCreature(), ManaRage, false);

        // Mana Rage giving of mana doesnt work so we give 10%(3231) / AIUpdate() Event.
        _castAISpell(ManaRageTrigger);
        uint32_t mana = getCreature()->getPower(POWER_TYPE_MANA) + 3231;
        if (mana >= getCreature()->getMaxPower(POWER_TYPE_MANA))
            mana = getCreature()->getMaxPower(POWER_TYPE_MANA);

        getCreature()->setPower(POWER_TYPE_MANA, mana);

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
        for (uint8_t x = 0; x < 5; x++)
        {
            FC = getNearestCreature(FelCrystals[x].x, FelCrystals[x].y, FelCrystals[x].z, CN_TRASH_FELCRYSTALS);
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
        getCreature()->setPower(POWER_TYPE_MANA, getCreature()->getPower(POWER_TYPE_MANA) - 3231);
    }

    SpellInfo const* ManaRage;
    CreatureAISpells* ManaRageTrigger;
    CreatureAISpells* FelExplosion;
    bool mEnableFelExplosion;
};

class VexallusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VexallusAI(c); }
    explicit VexallusAI(Creature* pCreature) : CreatureAIScript(pCreature)
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
    uint8_t mSummon;
};

class Priestess_DelrissaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Priestess_DelrissaAI(c); }
    explicit Priestess_DelrissaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mClearHateList = _addTimer(15000);
        mKilledPlayers = 0;
    }

    void OnTargetDied(Unit* pTarget) override
    {
        if (!pTarget || !pTarget->isPlayer())
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
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
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
    uint8_t mKilledPlayers;
    int32_t mClearHateList;
};

void SetupMagistersTerrace(ScriptMgr* mgr)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Instance
    mgr->register_instance_script(MAP_MAGISTERS_TERRACE, &InstanceMagistersTerraceScript::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Bosses
    mgr->register_creature_script(BOSS_SELIN_FIREHEART, &SelinFireheartAI::Create);
    mgr->register_creature_script(BOSS_VEXALLUS, &VexallusAI::Create);
    mgr->register_creature_script(BOSS_PRIEST_DELRISSA, &Priestess_DelrissaAI::Create);
}
