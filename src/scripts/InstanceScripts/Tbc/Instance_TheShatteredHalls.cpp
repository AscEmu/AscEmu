/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TheShatteredHalls.h"
#include "Objects/Faction.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Hellfire Citadel: The Shattered Halls
class InstanceTheShatteredHallsScript : public InstanceScript
{
public:

    explicit InstanceTheShatteredHallsScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {}

    static InstanceScript* Create(MapMgr* pMapMgr) { return new InstanceTheShatteredHallsScript(pMapMgr); }
};

class FelOrcConvertAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FelOrcConvertAI)
    explicit FelOrcConvertAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // not casted
        auto hemorrhage = addAISpell(SP_FEL_ORC_CONVERTER_HEMORRHAGE, 0.0f, TARGET_RANDOM_SINGLE, 0, 25, false, true);
        hemorrhage->setAttackStopTimer(1000);
    }
};

class ShatteredHandHeathenAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandHeathenAI)
    explicit ShatteredHandHeathenAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // not casted
        auto bloodthirst = addAISpell(SP_HAND_HEATHEN_BLOODTHIRST, 0.0f, TARGET_ATTACKING, 0, 25, false, true);
        bloodthirst->setAttackStopTimer(1000);

        enrage = addAISpell(SP_HAND_HEATHEN_ENRAGE, 0.0f, TARGET_SELF, 0, 70, false, true);
        enrage->setAttackStopTimer(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->castSpell(getCreature(), enrage->mSpellInfo, enrage->mIsTriggered);
    }

protected:

    CreatureAISpells* enrage;
};

class ShatteredHandLegionnaireAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandLegionnaireAI)
    explicit ShatteredHandLegionnaireAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // not casted
        auraOfDiscipline = addAISpell(SP_HAND_LEGI_AURA_OF_DISCIPLINE, 0.0f, TARGET_VARIOUS, 0, 0, false, true);
        auraOfDiscipline->setAttackStopTimer(1000);

        auto pummel = addAISpell(SP_HAND_LEGI_PUMMEL, 0.0f, TARGET_ATTACKING, 0, 25, false, true);
        pummel->setAttackStopTimer(1000);

        auto enrage = addAISpell(SP_HAND_LEGI_ENRAGE, 0.0f, TARGET_ATTACKING, 0, 70, false, true);
        enrage->setAttackStopTimer(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->castSpell(getCreature(), auraOfDiscipline->mSpellInfo, auraOfDiscipline->mIsTriggered);
    }

protected:

    CreatureAISpells* auraOfDiscipline;
};

class ShatteredHandSavageAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandSavageAI)
    explicit ShatteredHandSavageAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // not casted
        auto sliceAndDice = addAISpell(SP_HAND_SAVAGE_SLICE_AND_DICE, 0.0f, TARGET_SELF, 0, 35, false, true);
        sliceAndDice->setAttackStopTimer(1000);

        enrage = addAISpell(SP_HAND_SAVAGE_ENRAGE, 0.0f, TARGET_SELF, 0, 70, false, true);
        enrage->setAttackStopTimer(1000);

        auto deathblow = addAISpell(SP_HAND_SAVAGE_DEATHBLOW, 0.0f, TARGET_ATTACKING, 0, 25, false, true);
        deathblow->setAttackStopTimer(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->castSpell(getCreature(), enrage->mSpellInfo, enrage->mIsTriggered);
    }

protected:

    CreatureAISpells* enrage;
};

class ShadowmoonAcolyteAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowmoonAcolyteAI)
    explicit ShadowmoonAcolyteAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto heal = addAISpell(SP_SHADOWMOON_ACOLYTE_HEAL, 5.0f, TARGET_SELF, 0, 35, false, true);
        heal->setAttackStopTimer(1000);

        auto shield = addAISpell(SP_SHADOWMOON_ACOLYTE_PW_SHIELD, 5.0f, TARGET_SELF, 0, 45, false, true);
        shield->setAttackStopTimer(1000);

        auto mindBlast = addAISpell(SP_SHADOWMOON_ACOLYTE_MIND_BLAST, 5.0f, TARGET_ATTACKING, 0, 10);
        mindBlast->setAttackStopTimer(1000);

        auto resistShadow = addAISpell(SP_SHADOWMOON_ACOLYTE_RESIST_SHADOW, 5.0f, TARGET_SELF, 0, 65, false, true);
        resistShadow->setAttackStopTimer(1000);
    }
};

class ShatteredHandAssassinAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandAssassinAI)
    explicit ShatteredHandAssassinAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto sap = addAISpell(SP_SHATT_HAND_ASSASSIN_SAP, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
        sap->setAttackStopTimer(1000);

        stealth = addAISpell(SP_SHATT_HAND_ASSASSIN_STEALTH, 0.0f, TARGET_SELF, 0, 0, false, true);
        stealth->setAttackStopTimer(1000);

        auto cheapShot = addAISpell(SP_SHATT_HAND_ASSASSIN_CHEAP_SHOT, 5.0f, TARGET_ATTACKING, 0, 25, false, true);
        cheapShot->setAttackStopTimer(1000);

        getCreature()->castSpell(getCreature(), stealth->mSpellInfo, stealth->mIsTriggered);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->castSpell(getCreature(), stealth->mSpellInfo, stealth->mIsTriggered);
    }

protected:

    CreatureAISpells* stealth;
};

class ShatteredHandGladiatorAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandGladiatorAI)
    explicit ShatteredHandGladiatorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // not casted
        auto sap = addAISpell(SP_SHATT_HAND_GLADI_MORTAL_STRIKE, 0.0f, TARGET_ATTACKING, 0, 15, false, true);
        sap->setAttackStopTimer(1000);
    }
};

// he patrols with Rabid Warhounds
class ShatteredHandHoundmasterAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandHoundmasterAI)
    explicit ShatteredHandHoundmasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // not casted
        auto sap = addAISpell(SP_SHATT_HAND_HOUNDMASTER_VOLLEY, 0.0f, TARGET_DESTINATION, 0, 30);
        sap->setAttackStopTimer(1000);
    }
};

class ShatteredHandReaverAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandReaverAI)
    explicit ShatteredHandReaverAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto cleave = addAISpell(SP_SHATT_HAND_REAVER_CLEAVE, 7.0f, TARGET_VARIOUS, 0, 10, false, true);
        cleave->setAttackStopTimer(1000);

        auto uppercut = addAISpell(SP_SHATT_HAND_REAVER_UPPERCUT, 7.0f, TARGET_ATTACKING, 0, 35, false, true);
        uppercut->setAttackStopTimer(1000);

        auto enrage = addAISpell(SP_SHATT_HAND_REAVER_ENRAGE, 5.0f, TARGET_SELF, 0, 70, false, true);
        enrage->setAttackStopTimer(1000);
    }
};

class ShatteredHandSentryAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandSentryAI)
    explicit ShatteredHandSentryAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto hamstering = addAISpell(SP_SHATT_HAND_SENTRY_HAMSTERING, 7.0f, TARGET_ATTACKING, 0, 20, false, true);
        hamstering->setAttackStopTimer(1000);

        auto charge = addAISpell(SP_SHATT_HAND_SENTRY_CHARGE, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
        charge->setAttackStopTimer(1000);
    }
};

class ShatteredHandSharpshooterAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandSharpshooterAI)
    explicit ShatteredHandSharpshooterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto scatterShot = addAISpell(SP_SHATT_HAND_SHARP_SCATTER_SHOT, 7.0f, TARGET_ATTACKING, 0, 20, false, true);
        scatterShot->setAttackStopTimer(1000);

        auto arrow = addAISpell(SP_SHATT_HAND_SHARP_IMMO_ARROW, 7.0f, TARGET_ATTACKING, 0, 5);
        arrow->setAttackStopTimer(1000);

        auto sharpShot = addAISpell(SP_SHATT_HAND_SHARP_SHOT, 7.0f, TARGET_ATTACKING, 0, 5, false, true);
        sharpShot->setAttackStopTimer(1000);

        auto incendiaryShot = addAISpell(SP_SHATT_HAND_SHARP_INCENDIARY_SHOT, 7.0f, TARGET_ATTACKING, 0, 35, false, true);
        incendiaryShot->setAttackStopTimer(1000);
    }
};

// Self Visual - Sleep Until Cancelled (DND) 16093 ?
class ShatteredHandBrawlerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShatteredHandBrawlerAI)
    explicit ShatteredHandBrawlerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto shatteredHand = addAISpell(SP_CURSE_OF_THE_SHATTERED_HAND, 7.0f, TARGET_ATTACKING, 0, 35, false, true);
        shatteredHand->setAttackStopTimer(1000);

        auto brawlerKick = addAISpell(SP_SHATT_HAND_BRAWLER_KICK, 7.0f, TARGET_ATTACKING, 0, 25, false, true);
        brawlerKick->setAttackStopTimer(1000);

        auto brawlerTrash = addAISpell(SP_SHATT_HAND_BRAWLER_TRASH, 7.0f, TARGET_SELF, 0, 20, false, true);
        brawlerTrash->setAttackStopTimer(1000);
    }
};

// Grand Warlock Nethekurse Encounter
static Movement::Location Darkcasters[] =
{
    { 160.563004f, 272.989014f, -13.189000f },
    { 176.201004f, 264.669006f, -13.141600f },
    { 194.951004f, 265.657990f, -13.181700f }
};

class ShadowmoonDarkcasterAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowmoonDarkcasterAI)
    explicit ShadowmoonDarkcasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        Unit* GrandWarlock = NULL;
        GrandWarlock = getNearestCreature(178.811996f, 292.377991f, -8.190210f, CN_GRAND_WARLOCK_NETHEKURSE);
        if (GrandWarlock)
        {
            GrandWarlock->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
            GrandWarlock->GetAIInterface()->SetAllowedToEnterCombat(false);
        }
    }

    void OnCombatStart(Creature* /*mTarget*/)
    {
        Creature* GrandWarlock = NULL;
        GrandWarlock = getNearestCreature(178.811996f, 292.377991f, -8.190210f, CN_GRAND_WARLOCK_NETHEKURSE);
        if (GrandWarlock)
        {
            switch (Util::getRandomUInt(3))        // must be verified + emotes?
            {
                case 0:
                    GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_02);
                    break;
                case 1:
                    GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_03);
                    break;
                case 2:
                    GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_04);
                    break;
                case 3:
                    GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_05);
                    break;
            }
        }
    }

    void OnDied(Creature* /*mKiller*/)
    {
        Creature* GrandWarlock = NULL;
        GrandWarlock = getNearestCreature(178.811996f, 292.377991f, -8.190210f, CN_GRAND_WARLOCK_NETHEKURSE);
        if (GrandWarlock)    // any emotes needed?
        {
            uint32_t Counter = 0;
            for (uint8_t i = 0; i < 3; i++)
            {
                Creature* Servant = NULL;
                Servant = getNearestCreature(Darkcasters[i].x, Darkcasters[i].y, Darkcasters[i].z, CN_SHADOWMOON_DARKCASTER);
                if (!Servant)
                    continue;
                if (!Servant->isAlive())
                    continue;
                Counter++;
            }

            if (Counter == 0)
            {
                GrandWarlock->GetAIInterface()->HandleEvent(EVENT_ENTERCOMBAT, GrandWarlock, 0);
            }

            switch (Util::getRandomUInt(2))    // those need to be verified too
            {
                case 0:
                    GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_06);
                    break;
                case 1:
                    GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_07);
                    break;
                case 2:
                    GrandWarlock->SendScriptTextChatMessage(SAY_GRAND_WARLOCK_08);
                    break;
            }
        }
    }
};

// \todo It has much more sounds (like for servant dies etc.). For future makes researches on them.
class GrandWarlockNethekurseAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GrandWarlockNethekurseAI)
    explicit GrandWarlockNethekurseAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto deathCoil = addAISpell(SP_GRAND_WARLOCK_NETH_DEATH_COIL, 10.0f, TARGET_ATTACKING, 0, 15, false, true);
        deathCoil->setAttackStopTimer(1000);
        deathCoil->setMinMaxDistance(0.0f, 40.0f);

        addEmoteForEvent(Event_OnCombatStart, SAY_GRAND_WARLOCK_13);
        addEmoteForEvent(Event_OnCombatStart, SAY_GRAND_WARLOCK_14);
        addEmoteForEvent(Event_OnCombatStart, SAY_GRAND_WARLOCK_15);
        addEmoteForEvent(Event_OnTargetDied, SAY_GRAND_WARLOCK_16);
        addEmoteForEvent(Event_OnTargetDied, SAY_GRAND_WARLOCK_17);
        addEmoteForEvent(Event_OnDied, SAY_GRAND_WARLOCK_18);
    }
};

// Note: This boss appears only in Heroic mode and I don't have much infos about it =/
class BloodGuardPorungAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BloodGuardPorungAI)
    explicit BloodGuardPorungAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto cleave = addAISpell(SP_BLOOD_GUARD_PORUNG_CLEAVE, 10.0f, TARGET_VARIOUS, 0, 15, false, true);
        cleave->setAttackStopTimer(1000);
    }
};

class WarbringerOmroggAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(WarbringerOmroggAI)
    explicit WarbringerOmroggAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SP_WARBRINGER_OMROGG_THUNDERCLAP, 25.0f, TARGET_SELF, 1, 12);
        addAISpell(SP_WARBRINGER_OMROGG_FEAR, 7.0f, TARGET_SELF, 0, 20);

        mBlastWave = addAISpell(SP_WARBRINGER_OMROGG_BLAST_WAVE, 100.0f, TARGET_SELF, 1, 15);
        mBlastWaveTimer = mSpeechTimer = mSpeechId = mAggroShiftTimer = INVALIDATE_TIMER;
        mRightHead = nullptr;
        mLeftHead = nullptr;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mAggroShiftTimer = _addTimer(20000 + Util::getRandomUInt(10) * 1000);
        mBlastWaveTimer = mSpeechTimer = mSpeechId = INVALIDATE_TIMER;

        mLeftHead = spawnCreatureAndGetAIScript(19523, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
        mRightHead = spawnCreatureAndGetAIScript(19524, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
        
        if (mLeftHead == nullptr || mRightHead == nullptr)
            return;
        
        mLeftHead->getCreature()->GetAIInterface()->SetUnitToFollow(getCreature());
        mRightHead->getCreature()->GetAIInterface()->SetUnitToFollow(getCreature());

        switch (Util::getRandomUInt(2))
        {
            case 0:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 10308, "If you nice me let you live.");
                mSpeechTimer = _addTimer(4000);
                mSpeechId = 1;
                break;
            case 1:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 10309, "Me hungry!");
                mSpeechTimer = _addTimer(2500);
                mSpeechId = 2;
                break;
            case 2:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 10306, "Smash!");
                mSpeechTimer = _addTimer(2000);
                mSpeechId = 3;
                break;
        }
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (isAlive())
        {
            if (mLeftHead != nullptr)
            {
                mLeftHead->despawn(1000);
                mLeftHead = nullptr;
            }

            if (mRightHead != nullptr)
            {
                mRightHead->despawn(1000);
                mRightHead = nullptr;
            }
        }
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        if (mLeftHead == nullptr || mRightHead == nullptr || mSpeechTimer != INVALIDATE_TIMER)
            return;

        switch (Util::getRandomUInt(1))
        {
            case 0:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 10320, "I'm tired. You kill the next one!");
                break;
            case 1:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 10310, "This one die easy!");
                mSpeechTimer = _addTimer(3000);
                mSpeechId = 0;
                break;
        }
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (mLeftHead == nullptr || mRightHead == nullptr)
            return;

        sendChatMessage(CHAT_MSG_MONSTER_YELL, 10311, "This all... your fault!");
        mLeftHead->despawn(1000);
        mLeftHead = nullptr;
        mRightHead->RegisterAIUpdateEvent(3000);
        mRightHead->despawn(4000);
        mRightHead = nullptr;
    }

    void AIUpdate() override
    {
        if (mSpeechTimer != INVALIDATE_TIMER && _isTimerFinished(mSpeechTimer))
        {
            bool ResetSpeech = true;
            _removeTimer(mSpeechTimer);
            if (mLeftHead != nullptr && mRightHead != nullptr)
            {
                switch (mSpeechId)
                {
                    case 0:
                        mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10321, "That's because I do all the hard work!");
                        break;
                    case 1:
                        mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10318, "No, we will NOT let you live!");
                        break;
                    case 2:
                        mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10319, "You always hungry. That why we so fat!");
                        break;
                    case 3:
                        mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10317, "Why don't you let me do the talking!");
                        break;
                    case 4:
                        mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10313, "I'm not done yet, idiot!");
                        break;
                    case 5:
                        mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10316, "Bored, he's almost dead!");
                        break;
                    case 6:
                        mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10314, "That's not funny!");
                        mSpeechTimer = _addTimer(6000);
                        mSpeechId = 8;
                        ResetSpeech = false;
                        break;
                    case 7:
                        mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10315, "What are you doing!?");
                        break;
                    case 8:
                        mLeftHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10304, "Ha ha ha!");
                        break;
                }
            }

            if (ResetSpeech)
                mSpeechId = -1;
        }
        else if (_isTimerFinished(mAggroShiftTimer))
        {
            _resetTimer(mAggroShiftTimer, 20000 + Util::getRandomUInt(10) * 1000);
            ShiftAggro();
        }

        if (mBlastWaveTimer != INVALIDATE_TIMER && _isTimerFinished(mBlastWaveTimer))
        {
            _removeTimer(mBlastWaveTimer);
            _castAISpell(mBlastWave);
        }
    }

    void ShiftAggro()
    {
        Unit* pTarget = getBestPlayerTarget(TargetFilter_NotCurrent);
        if (pTarget != nullptr)
        {
            _clearHateList();
            getCreature()->GetAIInterface()->setNextTarget(pTarget);
            getCreature()->GetAIInterface()->modThreatByPtr(pTarget, 1000);

            if (mLeftHead == nullptr || mRightHead == nullptr || mSpeechTimer != INVALIDATE_TIMER)
                return;

            switch (Util::getRandomUInt(6))
            {
                case 0:
                    mLeftHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10301, "We kill his friend!");
                    break;
                case 1:
                    mLeftHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10300, "Me not like this one...");
                    mSpeechTimer = _addTimer(3000);
                    mSpeechId = 4;
                    break;
                case 2:
                    mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10312, "Hey, you numbskull!");
                    break;
                case 3:
                    mLeftHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10305, "Me get bored.");
                    mSpeechTimer = _addTimer(3000);
                    mSpeechId = 5;
                    break;
                case 4:
                    mRightHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10315, "What are you doing!?");
                    break;
                case 5:
                    mLeftHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10303, "You stay here. Me go kill someone else!");
                    mSpeechTimer = _addTimer(4000);
                    mSpeechId = 6;
                    break;
                case 6:
                    mLeftHead->sendChatMessage(CHAT_MSG_MONSTER_YELL, 10302, "Me kill someone else!");
                    mSpeechTimer = _addTimer(3000);
                    mSpeechId = 7;
                    break;
            }
        }
    }

    CreatureAIScript* mLeftHead;
    CreatureAIScript* mRightHead;
    int32_t mAggroShiftTimer;
    uint32_t mBlastWaveTimer;
    uint32_t mSpeechTimer;
    int32_t mSpeechId;
    CreatureAISpells* mBlastWave;
};

class HeadAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HeadAI)
    explicit HeadAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        _setScale(4.0f);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->m_noRespawn = true;
    }

    void AIUpdate() override
    {
        if (getCreature()->getEntry() != CN_RIGHT_HEAD)
            return;

        sendChatMessage(CHAT_MSG_MONSTER_YELL, 10322, "I... hate... you!");
        RemoveAIUpdateEvent();                                // Dangerous!
    }

    void Destroy() override
    {
        Creature* pUnit = getNearestCreature(CN_WARBRINGER_OMROGG);
        if (pUnit != NULL && pUnit->GetScript() != NULL)
        {
            WarbringerOmroggAI* pAI = static_cast< WarbringerOmroggAI* >(pUnit->GetScript());
            if (pAI->mLeftHead == (CreatureAIScript*)(this))
                pAI->mLeftHead = NULL;
            if (pAI->mRightHead == (CreatureAIScript*)(this))
                pAI->mRightHead = NULL;
        }
    }
};

// Should call for support? does he use only one ability?
class WarchiefKargathBladefistAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(WarchiefKargathBladefistAI)
    explicit WarchiefKargathBladefistAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto bladeDance = addAISpell(SP_WARCHIEF_LARAGATH_BLADE_DANCE, 10.0f, TARGET_VARIOUS, 0, 30, false, true);
        bladeDance->setAttackStopTimer(1500);

        addEmoteForEvent(Event_OnCombatStart, SAY_WARCHIEF_KARGATH_01);
        addEmoteForEvent(Event_OnCombatStart, SAY_WARCHIEF_KARGATH_02);
        addEmoteForEvent(Event_OnCombatStart, SAY_WARCHIEF_KARGATH_03);
        addEmoteForEvent(Event_OnTargetDied, SAY_WARCHIEF_KARGATH_04);
        addEmoteForEvent(Event_OnTargetDied, SAY_WARCHIEF_KARGATH_05);
        addEmoteForEvent(Event_OnDied, SAY_WARCHIEF_KARGATH_06);
    }
};

// \todo Shattered Hand Executioner 17301, Shattered Hand Champion 17671,
// Shattered Hand Centurion 17465, Shattered Hand Blood Guard 17461,
// hattered Hand Archer 17427, Sharpshooter Guard 17622, Shattered Hand Zealot 17462
// (lack of infos or don't have any spells!) more?
void SetupTheShatteredHalls(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_HC_SHATTERED_HALLS, &InstanceTheShatteredHallsScript::Create);

    //Creatures
    mgr->register_creature_script(CN_FEL_ORC_CONVERT, &FelOrcConvertAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_HEATHEN, &ShatteredHandHeathenAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_LEGIONNAIRE, &ShatteredHandLegionnaireAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_SAVAGE, &ShatteredHandSavageAI::Create);
    mgr->register_creature_script(CN_SHADOWMOON_ACOLYTE, &ShadowmoonAcolyteAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_ASSASSIN, &ShatteredHandAssassinAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_GLADIATOR, &ShatteredHandGladiatorAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_HOUNDMASTER, &ShatteredHandHoundmasterAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_REAVER, &ShatteredHandReaverAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_SENTRY, &ShatteredHandSentryAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_SHARPSHOOTER, &ShatteredHandSharpshooterAI::Create);
    mgr->register_creature_script(CN_SHATTERED_HAND_BRAWLER, &ShatteredHandBrawlerAI::Create);
    mgr->register_creature_script(CN_SHADOWMOON_DARKCASTER, &ShadowmoonDarkcasterAI::Create);
    mgr->register_creature_script(CN_GRAND_WARLOCK_NETHEKURSE, &GrandWarlockNethekurseAI::Create);
    mgr->register_creature_script(CN_BLOOD_GUARD_PORUNG, &BloodGuardPorungAI::Create);
    mgr->register_creature_script(CN_WARBRINGER_OMROGG, &WarbringerOmroggAI::Create);
    mgr->register_creature_script(CN_LEFT_HEAD, &HeadAI::Create);
    mgr->register_creature_script(CN_RIGHT_HEAD, &HeadAI::Create);
    mgr->register_creature_script(CN_WARCHIEF_KARGATH_BLADEFIST, &WarchiefKargathBladefistAI::Create);
}
