/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_GruulsLair.h"
#include "Server/Script/CreatureAIScript.h"

class GruulsLairInstanceScript : public InstanceScript
{
public:
    explicit GruulsLairInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        Instance = (GruulsLairInstanceScript*)pMapMgr->GetScript();
        mDoorMaulgar = nullptr;
        mDoorGruul = nullptr;
        mKrosh = nullptr;
        mOlm = nullptr;
        mKiggler = nullptr;
        mBlindeye = nullptr;
        mMaulgar = nullptr;
        mGruul = nullptr;
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new GruulsLairInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_MAULGAR_DOOR:
                mDoorMaulgar = pGameObject;
                break;
            case GO_GRUUL_DOOR:
                mDoorGruul = pGameObject;
                break;
            default:
                break;
        }

        SetGameobjectStates();
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case NPC_MAULGAR:
                mMaulgar = pCreature;
                break;
            case NPC_KROSH_FIREHAND:
                mKrosh = pCreature;
                break;
            case NPC_OLM_THE_SUMMONER:
                mOlm = pCreature;
                break;
            case NPC_KIGGLER_THE_CRAZED:
                mKiggler = pCreature;
                break;
            case NPC_BLINDEYE_THE_SEER:
                mBlindeye = pCreature;
                break;
            case NPC_GRUUL_THE_DRAGONKILLER:
                mGruul = pCreature;
                break;
            default:
                break;
        }

        if (mMaulgar && mMaulgar->GetScript())
        {
            if (mKrosh && mKrosh->GetScript() && !mKrosh->GetScript()->getLinkedCreatureAIScript())
                mKrosh->GetScript()->setLinkedCreatureAIScript(mMaulgar->GetScript());

            if (mOlm && mOlm->GetScript() && !mOlm->GetScript()->getLinkedCreatureAIScript())
                mOlm->GetScript()->setLinkedCreatureAIScript(mMaulgar->GetScript());

            if (mKiggler && mKiggler->GetScript() && !mKiggler->GetScript()->getLinkedCreatureAIScript())
                mKiggler->GetScript()->setLinkedCreatureAIScript(mMaulgar->GetScript());

            if (mBlindeye && mBlindeye->GetScript() && !mBlindeye->GetScript()->getLinkedCreatureAIScript())
                mBlindeye->GetScript()->setLinkedCreatureAIScript(mMaulgar->GetScript());
        }
    }

    void OnEncounterStateChange(uint32_t entry, uint32_t state) override
    {
        switch (entry)
        {
            case NPC_MAULGAR:
            {
                switch (state)
                {
                case NotStarted:
                    setLocalData(DATA_DOOR_MAULGAR, ACTION_DISABLE);
                    break;
                case InProgress:
                    setLocalData(DATA_DOOR_MAULGAR, ACTION_DISABLE);
                    break;
                case Finished:
                    setLocalData(DATA_DOOR_MAULGAR, ACTION_ENABLE);
                    break;
                default:
                    break;
                }
            }
                break;
            case NPC_GRUUL_THE_DRAGONKILLER:
            {
                switch (state)
                {
                case NotStarted:
                    setLocalData(DATA_DOOR_GRUUL, ACTION_ENABLE);
                    break;
                case InProgress:
                    setLocalData(DATA_DOOR_GRUUL, ACTION_DISABLE);
                    break;
                case Finished:
                    setLocalData(DATA_DOOR_GRUUL, ACTION_DISABLE);
                    break;
                default:
                    break;
                }
            }
                break;
            default:
                break;
        }
    }

    void SetGameobjectStates()
    {
        switch (getData(NPC_MAULGAR))
        {
            case NotStarted:
                setLocalData(DATA_DOOR_MAULGAR, ACTION_DISABLE);
                break;
            case InProgress:
                setLocalData(DATA_DOOR_MAULGAR, ACTION_DISABLE);
                break;
            case Finished:
                setLocalData(DATA_DOOR_MAULGAR, ACTION_ENABLE);
                break;
            default:
                break;
        }

        switch (getData(NPC_GRUUL_THE_DRAGONKILLER))
        {
            case NotStarted:
                setLocalData(DATA_DOOR_GRUUL, ACTION_ENABLE);
                break;
            case InProgress:
                setLocalData(DATA_DOOR_MAULGAR, ACTION_DISABLE);
                break;
            case Finished:
                setLocalData(DATA_DOOR_GRUUL, ACTION_DISABLE);
                break;
            default:
                break;
        }
    }

    void setLocalData(uint32_t type, uint32_t data) override
    {
        switch (type)
        {
            case DATA_DOOR_MAULGAR:
            {
                if (mDoorMaulgar)
                {
                    if (data == ACTION_ENABLE)
                        mDoorMaulgar->setState(GO_STATE_OPEN);
                    else
                        mDoorMaulgar->setState(GO_STATE_CLOSED);
                }
            }
                break;
            case DATA_DOOR_GRUUL:
            {
                if (mDoorGruul)
                {
                    if (data == ACTION_ENABLE)
                        mDoorGruul->setState(GO_STATE_OPEN);
                    else
                        mDoorGruul->setState(GO_STATE_CLOSED);
                }
            }
                break;
            default:
                break;
        }
    }

public:
    GruulsLairInstanceScript* Instance;
    GameObject* mDoorMaulgar;
    GameObject* mDoorGruul;
    Creature* mKrosh;
    Creature* mOlm;
    Creature* mKiggler;
    Creature* mBlindeye;
    Creature* mMaulgar;
    Creature* mGruul;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Lair Brute
class LairBruteAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LairBruteAI(c); }
    explicit LairBruteAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SPELL_CLEAVE, 20.0f, TARGET_ATTACKING, 0, 15);
        addAISpell(SPELL_MORTALSTRIKE, 8.0f, TARGET_ATTACKING, 0, 20);
        addAISpell(SPELL_CHARGE, 7.0f, TARGET_ATTACKING, 0, 35);
    }

    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId == SPELL_CHARGE)
        {
            Unit* pCurrentTarget = getCreature()->getThreatManager().getCurrentVictim();
            if (pCurrentTarget != nullptr)
            {
                getCreature()->GetAIInterface()->onHostileAction(pCurrentTarget);
                getCreature()->getThreatManager().addThreat(pCurrentTarget, 500.f);
                getCreature()->GetAIInterface()->setCurrentTarget(pCurrentTarget);
                getCreature()->getThreatManager().clearThreat(pCurrentTarget);
            }
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Gronn Priest
class GronnPriestAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GronnPriestAI(c); }
    explicit GronnPriestAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SPELL_PSYCHICSCREAM, 8.0f, TARGET_SELF, 0, 20);

        auto renew = addAISpell(SPELL_RENEW, 8.0f, TARGET_RANDOM_FRIEND, 0, 25);
        renew->setMinMaxDistance(0.0f, 100.0f);

        auto heal = addAISpell(SPELL_HEAL_GP, 8.0f, TARGET_RANDOM_FRIEND, 2, 30);
        heal->setMinMaxDistance(0.0f, 100.0f);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Wild Fell Stalker
class WildFelStalkerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WildFelStalkerAI(c); }
    explicit WildFelStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SPELL_WILD_BITE, 10.0f, TARGET_ATTACKING, 0, 10);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Maulgar
class HighKingMaulgarAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighKingMaulgarAI(c); }
    explicit HighKingMaulgarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mArcingSmash = addAISpell(SPELL_ARCING_SMASH, 8.0f, TARGET_ATTACKING, 0, 15);

        mMightyBlow = addAISpell(SPELL_MIGHTY_BLOW, 7.0f, TARGET_ATTACKING, 0, 30);

        mWhrilwind = addAISpell(SPELL_WHIRLWIND, 7.0f, TARGET_ATTACKING, 0, 55);

        mFlurry = addAISpell(SPELL_FLURRY, 2.0f, TARGET_ATTACKING, 0, 60);
        mFlurry->addDBEmote(MAUL_SAY_ENRAGE);

        mBerserker = addAISpell(SPELL_BERSERKER_C, 10.0f, TARGET_RANDOM_SINGLE, 0, 20);
        mBerserker->setAvailableForScriptPhase({ 2 });
        mBerserker->setMinMaxDistance(0.0f, 40.0f);

        mRoar = addAISpell(SPELL_ROAR, 8.0f, TARGET_SELF, 0, 40);
        mRoar->setAvailableForScriptPhase({ 2 });

        mDualWield = addAISpell(SPELL_DUAL_WIELD, 0.0f, TARGET_SELF);

        emoteVector.clear();
        emoteVector.push_back(MAUL_SAY_OGRE_DEATH_01);
        emoteVector.push_back(MAUL_SAY_OGRE_DEATH_02);
        emoteVector.push_back(MAUL_SAY_OGRE_DEATH_03);

        addEmoteForEvent(Event_OnCombatStart, MAUL_SAY_AGGRO);
        addEmoteForEvent(Event_OnTargetDied, MAUL_SAY_SLAY_01);
        addEmoteForEvent(Event_OnTargetDied, MAUL_SAY_SLAY_02);
        addEmoteForEvent(Event_OnTargetDied, MAUL_SAY_SLAY_03);
        addEmoteForEvent(Event_OnDied, MAUL_SAY_DEATH);
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t /*fAmount*/) override
    {
        if (isScriptPhase(PHASE_1) && _getHealthPercent() <= 50)
            setScriptPhase(PHASE_2);
    }

    void OnScriptPhaseChange(uint32_t phaseId) override
    {
        switch (phaseId)
        {
        case PHASE_2:
            _castAISpell(mFlurry);
            break;
        default:
            break;
        }
    }

    void DoAction(int32 actionId) override
    {
        if (actionId == ACTION_ADD_DEATH)
            sendRandomDBChatMessage(emoteVector, nullptr);
    }

protected:
    std::vector<uint32_t> emoteVector;
    CreatureAISpells* mArcingSmash;
    CreatureAISpells* mMightyBlow;
    CreatureAISpells* mWhrilwind;
    CreatureAISpells* mBerserker;
    CreatureAISpells* mRoar;
    CreatureAISpells* mFlurry;
    CreatureAISpells* mDualWield;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Kiggler
class KigglerTheCrazedAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KigglerTheCrazedAI(c); }
    explicit KigglerTheCrazedAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mGreaterPolymorph = addAISpell(SPELL_GREATER_POLYMORPH, 8.0f, TARGET_RANDOM_SINGLE, 0, 15);
        mLightningBolt = addAISpell(SPELL_LIGHTNING_BOLT, 70.0f, TARGET_ATTACKING, 0, 15);
        mArcaneShock = addAISpell(SPELL_ARCANE_SHOCK, 10.0f, TARGET_RANDOM_SINGLE, 0, 20);
        mArcaneExplosion = addAISpell(SPELL_ARCANE_EXPLOSION, 10.0f, TARGET_SELF, 0, 30);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getLinkedCreatureAIScript()->DoAction(ACTION_ADD_DEATH);
    }

    void AIUpdate(unsigned long /*time_passed*/) override
    {
        Unit* pTarget = getCreature()->GetAIInterface()->getCurrentTarget();
        if (pTarget != NULL)
        {
            if (getRangeToObject(pTarget) <= 40.0f)
            {
                //setAIAgent(AGENT_SPELL);
                setRooted(true);
            }
            else
                setRooted(false);
        }
    }

protected:
    CreatureAISpells* mGreaterPolymorph;
    CreatureAISpells* mLightningBolt;
    CreatureAISpells* mArcaneShock;
    CreatureAISpells* mArcaneExplosion;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Blind eye
class BlindeyeTheSeerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BlindeyeTheSeerAI(c); }
    explicit BlindeyeTheSeerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mPrayerOfHealing = addAISpell(SPELL_PRAYER_OH, 5.0f, TARGET_SELF, 0, 35);
        mGreaterPowerWordShield = addAISpell(SPELL_GREATER_PW_SHIELD, 8.0f, TARGET_SELF, 0, 40);
        mHeal = addAISpell(SPELL_HEAL, 8.0f, TARGET_RANDOM_FRIEND, 0, 25);
        mHeal->setMinMaxPercentHp(0.0f, 70.0f);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getLinkedCreatureAIScript()->DoAction(ACTION_ADD_DEATH);
    }

protected:
    CreatureAISpells* mPrayerOfHealing;
    CreatureAISpells* mGreaterPowerWordShield;
    CreatureAISpells* mHeal;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Olm
class OlmTheSummonerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OlmTheSummonerAI(c); }
    explicit OlmTheSummonerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mDeathCoil = addAISpell(SPELL_DEATH_COIL, 7.0f, TARGET_RANDOM_SINGLE, 0, 10);
        mSummon = addAISpell(SPELL_SUMMON_WFH, 7.0f, TARGET_SELF, 3, 20);
        mDarkDecay = addAISpell(SPELL_DARK_DECAY, 10.0f, TARGET_RANDOM_SINGLE, 0, 15);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getLinkedCreatureAIScript()->DoAction(ACTION_ADD_DEATH);
    }

protected:
    CreatureAISpells* mDeathCoil;
    CreatureAISpells* mSummon;
    CreatureAISpells* mDarkDecay;
};

/* He will first spellshield on himself, and recast every 30 sec,
   then spam great fireball to the target, also if there is any unit
   close to him (15yr) he'll cast blast wave
*/

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Krosh
class KroshFirehandAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KroshFirehandAI(c); }
    explicit KroshFirehandAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mBlastWave = addAISpell(SPELL_BLAST_WAVE, 0.0f, TARGET_SELF, 0, 15);
        mGreaterFireball = addAISpell(SPELL_GREATER_FIREBALL, 100.0f, TARGET_ATTACKING, 3, 0);
        mSpellShield = addAISpell(SPELL_SPELLSHIELD, 100.0f, TARGET_SELF, 0, 30);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        _castAISpell(mSpellShield);
        scriptEvents.addEvent(EVENT_BLASTWAVE, 20000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (_isCasting())
            return;

        scriptEvents.updateEvents(time_passed, getScriptPhase());

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
                case EVENT_BLASTWAVE:
                {
                    Unit* unit = getBestUnitTarget(TargetFilter_Closest);
                    if (unit && getRangeToObject(unit) < 15.0f)
                        _castAISpell(mBlastWave);

                    scriptEvents.addEvent(EVENT_BLASTWAVE, 20000);
                }
                    break;
                default:
                    break;
            }
        }
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getLinkedCreatureAIScript()->DoAction(ACTION_ADD_DEATH);
    }

protected:
    CreatureAISpells* mSpellShield;
    CreatureAISpells* mGreaterFireball;
    CreatureAISpells* mBlastWave;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Gruul the Dragonkiller
class GruulTheDragonkillerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GruulTheDragonkillerAI(c); }
    explicit GruulTheDragonkillerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mGrowth = addAISpell(SPELL_GROWTH, 100.0f, TARGET_SELF, 0, 30);
        mGrowth->setMaxStackCount(30);
        mGrowth->addDBEmote(GRUUL_EMOTE_GROW);

        mCaveIn = addAISpell(SPELL_CAVE_IN, 7.0f, TARGET_RANDOM_DESTINATION, 0, 25);

        mGroundSlam = addAISpell(SPELL_GROUND_SLAM, 8.0f, TARGET_SELF, 1, 35);
        mGroundSlam->addDBEmote(GRUUL_SAY_SLAM_01);
        mGroundSlam->addDBEmote(GRUUL_SAY_SLAM_02);

        mReverberation = addAISpell(SPELL_REVERBERATION, TARGET_SELF, 4, 0, 30);

        mShatter = addAISpell(SPELL_SHATTER, 0.0f, TARGET_SELF, 0, 1, 0);
        mShatter->addDBEmote(GRUUL_SAY_SHATTER_01);
        mShatter->addDBEmote(GRUUL_SAY_SHATTER_02);

        mHurtfulStrike = addAISpell(SPELL_HURTFUL_STRIKE, 0.0f, TARGET_ATTACKING);

        addEmoteForEvent(Event_OnCombatStart, GRUUL_SAY_AGGRO);
        addEmoteForEvent(Event_OnTargetDied, GRUUL_SAY_SLAY_01);
        addEmoteForEvent(Event_OnTargetDied, GRUUL_SAY_SLAY_02);
        addEmoteForEvent(Event_OnTargetDied, GRUUL_SAY_SLAY_03);
        addEmoteForEvent(Event_OnDied, GRUUL_SAY_DEATH);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_HURTFUL_STRIKE, 8000);
    }

    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId == SPELL_GROUND_SLAM)
            scriptEvents.addEvent(EVENT_SHATTER, 5000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (_isCasting())
            return;

        scriptEvents.updateEvents(time_passed, getScriptPhase());

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
                case EVENT_SHATTER:
                    _castAISpell(mShatter);
                    break;
                case EVENT_HURTFUL_STRIKE:
                {
                    Unit* pCurrentTarget = getCreature()->GetAIInterface()->getCurrentTarget();
                    if (pCurrentTarget != nullptr)
                    {
                        Unit* pTarget = pCurrentTarget;
                        for (const auto& itr : getCreature()->getInRangePlayersSet())
                        {
                            Player* pPlayer = static_cast<Player*>(itr);
                            if (!pPlayer || !pPlayer->isAlive())
                                continue;
                            if (pPlayer->hasUnitFlags(UNIT_FLAG_FEIGN_DEATH))
                                continue;
                            if (getRangeToObject(pPlayer) > 8.0f)
                                continue;
                            if (getCreature()->getThreatManager().getThreat(pPlayer) >= getCreature()->getThreatManager().getThreat(pCurrentTarget))
                                continue;

                            pTarget = static_cast<Unit*>(pPlayer);
                        }

                        if (pTarget == pCurrentTarget)
                            _castAISpell(mHurtfulStrike);
                        else
                            getCreature()->castSpell(pTarget, SPELL_HURTFUL_STRIKE, true);
                    }
                    scriptEvents.addEvent(EVENT_HURTFUL_STRIKE, 8000);
                }
                    break;
                default:
                    break;
            }
        }
    }

protected:
    CreatureAISpells* mGrowth;
    CreatureAISpells* mHurtfulStrike;
    CreatureAISpells* mGroundSlam;
    CreatureAISpells* mCaveIn;
    CreatureAISpells* mShatter;
    CreatureAISpells* mReverberation;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Effect: Ground Slam
bool GroundSlamEffect(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* target = pSpell->GetUnitTarget();

    if (!target || !target->isPlayer())
        return true;

    target->HandleKnockback(target, Util::getRandomFloat(10.0f, 15.0f), Util::getRandomFloat(10.0f, 15.0f));

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Effect: Shatter
bool ShatterEffect(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* target = pSpell->GetUnitTarget();

    if (!target)
        return true;

    target->RemoveAura(SPELL_STONED);
    target->castSpell(target, SPELL_SHATTER_EFFECT, true);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Shatter Damage
class ShatterDamage : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg) override
    {
        if (effIndex != EFF_INDEX_0 || spell->GetUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        float radius = spell->GetRadius(EFF_INDEX_0);
        auto distance = spell->GetUnitTarget()->GetDistance2dSq(spell->getCaster());

        if (distance < 1.0f)
            distance = 1.0f;

        *dmg = float2int32(*dmg * ((radius - distance ) / radius));

        return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
    }
};

void SetupGruulsLair(ScriptMgr* mgr)
{
    // Instance
    mgr->register_instance_script(MAP_GRUULS_LAIR, &GruulsLairInstanceScript::Create);

    // Spells
    mgr->register_script_effect(SPELL_GROUND_SLAM, &GroundSlamEffect);
    mgr->register_script_effect(SPELL_SHATTER, &ShatterEffect);
    mgr->register_spell_script(SPELL_SHATTER_EFFECT, new ShatterDamage);

    // Creatures
    mgr->register_creature_script(NPC_LAIR_BRUTE, &LairBruteAI::Create);
    mgr->register_creature_script(NPC_GRONN_PRIEST, &GronnPriestAI::Create);
    mgr->register_creature_script(NPC_WILD_FEL_STALKER, &WildFelStalkerAI::Create);

    // Boss
    mgr->register_creature_script(NPC_KIGGLER_THE_CRAZED, &KigglerTheCrazedAI::Create);
    mgr->register_creature_script(NPC_BLINDEYE_THE_SEER, &BlindeyeTheSeerAI::Create);
    mgr->register_creature_script(NPC_OLM_THE_SUMMONER, &OlmTheSummonerAI::Create);
    mgr->register_creature_script(NPC_KROSH_FIREHAND, &KroshFirehandAI::Create);
    mgr->register_creature_script(NPC_MAULGAR,    &HighKingMaulgarAI::Create);
    mgr->register_creature_script(NPC_GRUUL_THE_DRAGONKILLER, &GruulTheDragonkillerAI::Create);
}
