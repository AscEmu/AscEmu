/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_HallsOfLightning.h"

enum 
{
    // Main Spells
    SPELL_TEMPER = 52238,

    // Molten Golem Spells
    SPELL_BLAST_WAVE = 23113,
    // 24 seconds + up to 6
    TIMER_STOMP = 24000,

    DISPRESE = 52770,
    SPELL_SUMMON_SPARK = 52746,

    PULSING_SHOCKWAVE_AURA = 59414,
    ARC_LIGHTNING = 52921,
    // 14 seconds + random up to 8
    TIMER_NOVA = 14000,
    TIMER_RESPOND = 18000,

    TIMER_STANCE_CHANGE = 18000,
};

enum GENERAL_STANCES
{
    STANCE_BATTLE = 1,
    STANCE_BERSERKER = 2,
    STANCE_DEFENSIVE = 3,
};

class HallsOfLightningScript : public InstanceScript
{
public:

    uint32 mGeneralDoorsGUID;
    uint32 mVolkhanDoorsGUID;
    uint32 mLokenDoorsGUID;
    uint32 mIonarDoors1GUID;
    uint32 mIonarDoors2GUID;

    explicit HallsOfLightningScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        mGeneralDoorsGUID = 0;
        mVolkhanDoorsGUID = 0;
        mLokenDoorsGUID = 0;
        mIonarDoors1GUID = 0;
        mIonarDoors2GUID = 0;
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new HallsOfLightningScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_GENERAL_DOORS:
                mGeneralDoorsGUID = pGameObject->getGuidLow();
                break;
            case GO_VOLKHAN_DOORS:
                mVolkhanDoorsGUID = pGameObject->getGuidLow();
                break;
            case GO_LOKEN_DOORS:
                mLokenDoorsGUID = pGameObject->getGuidLow();
                break;
            case GO_IONAR_DOORS1:
                mIonarDoors1GUID = pGameObject->getGuidLow();
                break;
            case GO_IONAR_DOORS2:
                mIonarDoors2GUID = pGameObject->getGuidLow();
                break;
        }
    }

    void OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/) override
    {
        GameObject* pDoors = NULL;
        switch (pVictim->getEntry())
        {
            case CN_GENERAL_BJARNGRIM:
            {
                pDoors = GetGameObjectByGuid(mGeneralDoorsGUID);
                if (pDoors)
                    pDoors->setState(GO_STATE_OPEN);
            }
            break;
            case CN_VOLKHAN:
            {
                pDoors = GetGameObjectByGuid(mVolkhanDoorsGUID);
                    if (pDoors)
                        pDoors->setState(GO_STATE_OPEN);
            }
            break;
            case CN_LOKEN:
            {
                pDoors = GetGameObjectByGuid(mLokenDoorsGUID);
                if (pDoors)
                    pDoors->setState(GO_STATE_OPEN);
            }
            break;
            case CN_IONAR:
            {
                pDoors = GetGameObjectByGuid(mIonarDoors1GUID);
                if (pDoors)
                    pDoors->setState(GO_STATE_OPEN);

                pDoors = GetGameObjectByGuid(mIonarDoors2GUID);
                if (pDoors)
                    pDoors->setState(GO_STATE_OPEN);
            }
            break;
        }
    }
};

class GeneralBjarngrimAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GeneralBjarngrimAI);
    explicit GeneralBjarngrimAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Battle Stance
        auto mortalStrike = addAISpell(SPELL_MORTAL_STRIKE, 25.0f, TARGET_ATTACKING, 0, 5);
        mortalStrike->setAvailableForScriptPhase({ 1 });

        auto whirlwind = addAISpell(SPELL_WHIRLWIND, 90.0f, TARGET_SELF, 8, 30);
        whirlwind->setAvailableForScriptPhase({ 1 });

        // Berserker Stance
        auto cleave = addAISpell(SPELL_CLEAVE, 30.0f, TARGET_ATTACKING, 0, 5);
        cleave->setAvailableForScriptPhase({ 2 });

        // Defensive Stance
        auto reflection = addAISpell(SPELL_SPELL_REFLECTION, 20.0f, TARGET_SELF, 0, 10);
        reflection->setAvailableForScriptPhase({ 3 });

        auto intercept = addAISpell(SPELL_INTERCEPT, 40.0f, TARGET_RANDOM_SINGLE, 0, 6);
        intercept->setAvailableForScriptPhase({ 3 });

        auto pummel = addAISpell(SPELL_PUMMEL, 40.0f, TARGET_ATTACKING, 0, 5);
        pummel->setAvailableForScriptPhase({ 3 });

        mStanceTimer = INVALIDATE_TIMER;

        addEmoteForEvent(Event_OnCombatStart, 758);      // I am the greatest of my father's sons! Your end has come!
        addEmoteForEvent(Event_OnTargetDied, 762);        // So ends your curse.
        addEmoteForEvent(Event_OnTargetDied, 763);        // Flesh... is... weak!
        addEmoteForEvent(Event_OnDied, 765);      // How can it be...? Flesh is not... stronger!
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStanceTimer = _addTimer(TIMER_STANCE_CHANGE + (Util::getRandomUInt(7) * 1000));
        switchStance(Util::getRandomUInt(2));
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mStanceTimer))
        {
            switch (getScriptPhase())
            {
                case STANCE_BATTLE:
                    switchStance(Util::getRandomUInt(1) + 2);
                    break;
                case STANCE_BERSERKER:
                    if (Util::getRandomUInt(1) == 1)
                        switchStance(STANCE_BATTLE);
                    else
                        switchStance(STANCE_DEFENSIVE);
                    break;
                case STANCE_DEFENSIVE:
                    switchStance(Util::getRandomUInt(1) + 1);
                    break;
            }
            _resetTimer(mStanceTimer, TIMER_STANCE_CHANGE + (Util::getRandomUInt(7) * 1000));
        }
    }

    // case for scriptPhase and spell emotes
    void switchStance(int32 pStance)
    {
        switch (pStance)
        {
            case STANCE_BATTLE:
                _applyAura(SPELL_BATTLE_AURA);
                _applyAura(SPELL_BATTLE_STANCE);
                sendDBChatMessage(760);      // Defend yourself, for all the good it will do!
                sendAnnouncement("General Bjarngrim switches to Battle Stance!");
                setScriptPhase(1);
                break;
            case STANCE_BERSERKER:
                _applyAura(SPELL_BERSERKER_AURA);
                _applyAura(SPELL_BERSERKER_STANCE);
                sendDBChatMessage(761);      // GRAAAAAH! Behold the fury of iron and steel!
                sendAnnouncement("General Bjarngrim switches to Berserker Stance!");
                setScriptPhase(2);
                break;
            case STANCE_DEFENSIVE:
                _applyAura(SPELL_DEFENSIVE_AURA);
                _applyAura(SPELL_DEFENSIVE_STANCE);
                sendDBChatMessage(759);      // Give me your worst!
                sendAnnouncement("General Bjarngrim switches to Defensive Stance!");
                setScriptPhase(3);
                break;
        }
    }

private:

    int32 mStanceTimer;
};

class Volkhan : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Volkhan);
    explicit Volkhan(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (_isHeroic())
        {
            addAISpell(59529, 15.0f, TARGET_RANDOM_FRIEND, 2, 15);
            mStomp = addAISpell(59529, 0.0f, TARGET_SELF, 3, 0);
        }
        else
        {
            addAISpell(52237, 15.0f, TARGET_RANDOM_FRIEND, 2, 15);
            mStomp = addAISpell(52237, 0.0f, TARGET_SELF, 3, 0);
        }

        mStomp->addEmote("I will crush you beneath my boots!", CHAT_MSG_MONSTER_YELL, 13963);
        mStomp->addEmote("All my work... undone!", CHAT_MSG_MONSTER_YELL, 13964);

        m_cVolkhanWP.x = 1328.666870f;
        m_cVolkhanWP.y = -97.022758f;
        m_cVolkhanWP.z = 56.675297f;
        m_cVolkhanWP.o = 2.235341f;

        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        AddWaypoint(CreateWaypoint(1, 0, Movement::WP_MOVE_TYPE_RUN, m_cVolkhanWP));

        mStompTimerId = 0;
        mPhase = 0;
        m_bStomp = false;

        addEmoteForEvent(Event_OnCombatStart, 769);      // It is you who have destroyed my children? You... shall... pay!
        addEmoteForEvent(Event_OnTargetDied, 774);      // The armies of iron will conquer all!
        addEmoteForEvent(Event_OnTargetDied, 775);      // Feh! Pathetic!
        addEmoteForEvent(Event_OnTargetDied, 776);      // You have cost me too much work!
        addEmoteForEvent(Event_OnDied, 777);      // The master was right... to be concerned.
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStompTimerId = _addTimer(TIMER_STOMP + (Util::getRandomUInt(6) * 1000));
        mPhase = 0;
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mStompTimerId))
        {
            if (m_bStomp == false)
            {
                m_bStomp = true;
                sendAnnouncement("Volkhan prepares to shatter his Brittle Golems!");
                _castAISpell(mStomp);
                _resetTimer(mStompTimerId, 3000);
            }
            else
            {
                DoStomp();
                _resetTimer(mStompTimerId, TIMER_STOMP + (Util::getRandomUInt(6) * 1000));
            }
        }

        if (_getHealthPercent() <= (100 - (20 * mPhase)))
        {
            ForceWaypointMove(1);
            sendAnnouncement("Volkhan runs to his anvil!");
            ++mPhase;
        } 
    }

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == 1)
        {
            switch (Util::getRandomUInt(2))
            {
                case 0:
                    sendDBChatMessage(770);      // Life from lifelessness... death for you.
                    break;
                case 1:
                    sendDBChatMessage(771);      // Nothing is wasted in the process. You will see....
                    break;
            }

            Creature* pAnvil = getNearestCreature(CN_VOLKHANS_ANVIL);
            if (pAnvil)
                getCreature()->castSpell(pAnvil, SPELL_TEMPER, true);
            else
                getCreature()->castSpell(getCreature(), SPELL_TEMPER, true);

            setCanEnterCombat(true);
            getCreature()->GetAIInterface()->AttackReaction(getNearestPlayer(), 1);   // hackfix
        }
    }

    void DoStomp()
    {
        for (const auto& itr : getCreature()->getInRangeObjectsSet())
        {
            if (itr && itr->isCreature() && itr->getEntry() == CN_BRITTLE_GOLEM)
            {
                Creature* pCreature = static_cast<Creature*>(itr);
                if (_isHeroic())
                    pCreature->castSpell(pCreature, 59527, true);
                else
                    pCreature->castSpell(pCreature, 52429, true);

                pCreature->Despawn(1000, 0);
            }
        }

        m_bStomp = false;
    }

    CreatureAISpells* mStomp;
    Movement::Location m_cVolkhanWP;
    bool m_bStomp;
    uint32_t mStompTimerId;
    int32 mPhase;
};

class MoltenGolem : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MoltenGolem);
    explicit MoltenGolem(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SPELL_BLAST_WAVE, 25.0f, TARGET_SELF, 0, 20);

        if (_isHeroic())
            addAISpell(59530, 15.0f, TARGET_ATTACKING, 0, 15);
        else
            addAISpell(52433, 15.0f, TARGET_ATTACKING, 0, 15);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        spawnCreature(CN_BRITTLE_GOLEM, getCreature()->GetPosition());
        despawn();
    }
};

class BrittleGolem : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BrittleGolem);
    explicit BrittleGolem(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        setCanEnterCombat(false);
        setRooted(true);
    }
};

class VolkhansAnvil : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(VolkhansAnvil);
    explicit VolkhansAnvil(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setRooted(true);
    }

};

//\todo missing spark phase
class IonarAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IonarAI);
    explicit IonarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (_isHeroic())
        {
            addAISpell(59800, 20.0f, TARGET_RANDOM_SINGLE, 2, 5);
            addAISpell(59795, 15.0f, TARGET_RANDOM_SINGLE, 0, 12);
        }
        else
        {
            addAISpell(52780, 20.0f, TARGET_RANDOM_SINGLE, 2, 5);
            addAISpell(52658, 15.0f, TARGET_RANDOM_SINGLE, 0, 12);
        }

        addEmoteForEvent(Event_OnCombatStart, 738);     // You wish to confront the master? You must first weather the storm!
        addEmoteForEvent(Event_OnTargetDied, 741);      // Shocking, I know.
        addEmoteForEvent(Event_OnTargetDied, 742);      // You attempt the impossible.
        addEmoteForEvent(Event_OnTargetDied, 743);      // Your spark of life is... extinguished.
        addEmoteForEvent(Event_OnDied, 744);            // Master... you have guests
    }
};

class LokenAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LokenAI);
    explicit LokenAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (_isHeroic())
            mNova = addAISpell(59835, 0.0f, TARGET_SELF, 4, 0);
        else
            mNova = addAISpell(52960, 0.0f, TARGET_SELF, 4, 0);

        mNova->addDBEmote(802);      // You cannot hide from fate!
        mNova->addDBEmote(803);      // Come closer. I will make it quick.
        mNova->addDBEmote(804);      // Your flesh cannot hold out for long.
        
        addAISpell(ARC_LIGHTNING, 25.0f, TARGET_RANDOM_SINGLE, 0, 6);

        sendChatMessage(CHAT_MSG_MONSTER_YELL, 14160, "I have witnessed the rise and fall of empires. The birth and extinction of entire species. Over countless millennia the foolishness of mortals has remained the only constant. Your presence here confirms this.");

        mNovaTimerId = 0;
        mRespondTimer = _addTimer(TIMER_RESPOND);
        RegisterAIUpdateEvent(1000);
        mSpeech = 1;

        addEmoteForEvent(Event_OnCombatStart, 801);      // What hope is there for you? None!
        addEmoteForEvent(Event_OnTargetDied, 805);      // Only mortal...
        addEmoteForEvent(Event_OnTargetDied, 806);      // I... am... FOREVER!
        addEmoteForEvent(Event_OnTargetDied, 807);      // What little time you had, you wasted!
        addEmoteForEvent(Event_OnDied, 811);      // My death... heralds the end of this world.
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mSpeech = 1;

        if (_isHeroic())
            _applyAura(59836);
        else
            _applyAura(52961);

        mNovaTimerId = _addTimer(TIMER_NOVA);
        _castOnInrangePlayers(PULSING_SHOCKWAVE_AURA);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        _removeAuraOnPlayers(PULSING_SHOCKWAVE_AURA);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        _removeAuraOnPlayers(PULSING_SHOCKWAVE_AURA);
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mNovaTimerId))
        {
            sendAnnouncement("Loken begins to cast Lightning Nova!");
            _castAISpell(mNova);
            _resetTimer(mNovaTimerId, TIMER_NOVA + (Util::getRandomUInt(8) * 1000));
        }

        if (mSpeech == 4)
            return;

        // scriptPhase
        if (_getHealthPercent() <= (100 - (25 * mSpeech)))
        {
            switch (mSpeech) //rand() % 2
            {
                case 1:
                    sendDBChatMessage(808);      // You stare blindly into the abyss!
                    break;
                case 2:
                    sendDBChatMessage(809);      // Your ignorance is profound. Can you not see where this path leads?
                    break;
                case 3:
                    sendDBChatMessage(810);      // You cross the precipice of oblivion!
                    break;
            }

            ++mSpeech;
        }

        if (_isTimerFinished(mRespondTimer))
        {
            sendDBChatMessage(800);      // My master has shown me the future, and you have no place in it. Azeroth..
            _removeTimer(mRespondTimer);
            RemoveAIUpdateEvent();
        }       
    }

    CreatureAISpells* mNova;

    uint32_t mNovaTimerId;
    uint32 mRespondTimer;
    uint8 mSpeech;
};

void SetupHallsOfLightning(ScriptMgr* mgr)
{
#ifndef UseNewMapScriptsProject
    mgr->register_instance_script(MAP_HALLS_OF_LIGHTNING, &HallsOfLightningScript::Create);
#endif
    mgr->register_creature_script(CN_GENERAL_BJARNGRIM, &GeneralBjarngrimAI::Create);

    mgr->register_creature_script(CN_VOLKHAN, &Volkhan::Create);
    mgr->register_creature_script(CN_MOLTEN_GOLEM, &MoltenGolem::Create);
    mgr->register_creature_script(CN_BRITTLE_GOLEM, &BrittleGolem::Create);
    mgr->register_creature_script(CN_VOLKHANS_ANVIL, &VolkhansAnvil::Create);

    mgr->register_creature_script(CN_IONAR, &IonarAI::Create);
    mgr->register_creature_script(CN_LOKEN, &LokenAI::Create);
}
