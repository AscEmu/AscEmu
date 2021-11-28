/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Magtheridons_Lair.h"
#include "Management/Faction.h"
#include "Server/Script/CreatureAIScript.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Magtheridons Lair Instance
class MagtheridonsLairInstanceScript : public InstanceScript
{
public:
    explicit MagtheridonsLairInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        Instance = (MagtheridonsLairInstanceScript*)pMapMgr->GetScript();
        door = nullptr;
        hall = nullptr;
        cubes.clear();
        columns.clear();

        magtheridon = nullptr;
        worldTrigger = nullptr;
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new MagtheridonsLairInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
        case GO_MANTICRON_CUBE:
            cubes.push_back(pGameObject);
            break;
        case GO_MAGTHERIDON_DOOR:
            door = pGameObject;
            break;
        case GO_MAGTHERIDON_HALL:
            hall = pGameObject;
            break;
        case GO_MAGTHERIDON_COLUMN_0:
        case GO_MAGTHERIDON_COLUMN_1:
        case GO_MAGTHERIDON_COLUMN_2:
        case GO_MAGTHERIDON_COLUMN_3:
        case GO_MAGTHERIDON_COLUMN_4:
        case GO_MAGTHERIDON_COLUMN_5:
            columns.push_back(pGameObject);
            break;
        default:
            break;
        }
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
        case NPC_MAGTHERIDON:
            magtheridon = pCreature;
            break;
        case NPC_WORLD_TRIGGER:
            worldTrigger = pCreature;
            break;
        default:
            break;
        }
    }

    void setLocalData(uint32_t type, uint32_t data) override
    {
        switch (type)
        {
        case DATA_MANTICRON_CUBE:
            for (auto cube : cubes)
            {
                if (cube)
                {
                    if (data == ACTION_ENABLE)
                        cube->removeFlags(GO_FLAG_NOT_SELECTABLE);
                    else
                        cube->setFlags(GO_FLAG_NOT_SELECTABLE);
                }
            }
            break;
        case DATA_COLLAPSE:
            if (hall)
            {
                if (data == ACTION_ENABLE)
                    hall->setState(GO_STATE_OPEN);
                else
                    hall->setState(GO_STATE_CLOSED);
            }
            break;
        case DATA_COLLAPSE_2:
            for (auto column : columns)
            {
                if (column)
                {
                    if (data == ACTION_ENABLE)
                        column->setState(GO_STATE_OPEN);
                    else
                        column->setState(GO_STATE_CLOSED);
                }
            }
            break;
        case DATA_DOOR:
            if (door)
            {
                if (data == ACTION_ENABLE)
                    door->setState(GO_STATE_OPEN);
                else
                    door->setState(GO_STATE_CLOSED);
            }
            break;
        default:
            break;
        }
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
        case ACTION_START_CHANNELERS_EVENT:
            setLocalData(DATA_DOOR, ACTION_DISABLE);
            magtheridon->GetScript()->DoAction(ACTION_START_CHANNELERS_EVENT);
            break;
        }
    }


public:
    MagtheridonsLairInstanceScript* Instance;
    GameObject* door;
    GameObject* hall;
    std::list<GameObject*> columns;
    std::list<GameObject*> cubes;
    Creature* magtheridon;
    Creature* worldTrigger;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Gameobject: manticron Cubes
class ManticronCubeGO : public GameObjectAIScript
{
public:
    static GameObjectAIScript* Create(GameObject* GO) { return new ManticronCubeGO(GO); }
    explicit ManticronCubeGO(GameObject* pGameObject) : GameObjectAIScript(pGameObject) 
    {
        Instance = nullptr;
    }

    void OnActivate(Player* pPlayer) override
    {
        Instance = (MagtheridonsLairInstanceScript*)_gameobject->GetMapMgr()->GetScript();

        // We check if player has aura that prevents anyone from using this GO
        if (pPlayer->getAuraWithId(SPELL_MIND_EXHAUSTION) || pPlayer->getAuraWithId(SPELL_SHADOW_GRASP))
            return;

        if (!Instance && !Instance->magtheridon)
            return;

        if (Creature* trigger = _gameobject->GetMapMgr()->GetInterface()->findNearestCreature(pPlayer, NPC_HELFIRE_RAID_TRIGGER, 10.0f))
            trigger->castSpell(Instance->magtheridon, SPELL_SHADOW_GRASP_VISUAL, true);

        pPlayer->setTargetGuid(Instance->magtheridon->getGuid());
        pPlayer->castSpell(pPlayer, SPELL_SHADOW_GRASP, true);
    }

protected:
    MagtheridonsLairInstanceScript* Instance;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Cube Trigger Npc
class CubeTriggerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CubeTriggerAI(c); }
    explicit CubeTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Room Triger ( Roof falling down )
class RoomTrigger : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RoomTrigger(c); }
    explicit RoomTrigger(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Scripted Spells not autocastet
        debrisVisual = addAISpell(SPELL_DEBRIS_VISUAL, 0.0f, TARGET_SELF);
        debrisDamage = addAISpell(SPELL_DEBRIS_DAMAGE, 0.0f, TARGET_SOURCE);
    }

    void OnLoad() override
    {
        _castAISpell(debrisVisual);
        scriptEvents.addEvent(SPELL_DEBRIS_DAMAGE, 5000);
        despawn(20000, 0);
    }

    void AIUpdate() override
    {
        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        if(scriptEvents.getFinishedEvent() == SPELL_DEBRIS_DAMAGE)
            _castAISpell(debrisDamage);

    }

protected:
    CreatureAISpells* debrisVisual;
    CreatureAISpells* debrisDamage;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Hellfire Warder
class HellfireWarderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireWarderAI(c); }
    explicit HellfireWarderAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto shadowBoltVolley = addAISpell(HW_SHADOW_BOLT_VOLLEY, 15.0f, TARGET_VARIOUS, 0, 5, false, true);
        shadowBoltVolley->setAttackStopTimer(1000);

        auto wordPain = addAISpell(SHADOW_WORD_PAIN, 6.0f, TARGET_RANDOM_SINGLE, 0, 7, false, true);
        wordPain->setAttackStopTimer(1000);
        wordPain->setMinMaxDistance(0.0f, 30.0f);

        auto unstableAfflection = addAISpell(UNSTABLE_AFFLICTION, 6.0f, TARGET_RANDOM_SINGLE, 0, 7, false, true);
        unstableAfflection->setAttackStopTimer(1000);
        unstableAfflection->setMinMaxDistance(0.0f, 30.0f);

        auto deathCoil = addAISpell(DEATH_COIL, 5.0f, TARGET_RANDOM_SINGLE, 0, 8, false, true);
        deathCoil->setAttackStopTimer(1000);
        deathCoil->setMinMaxDistance(0.0f, 30.0f);

        auto rainOfFire = addAISpell(RAIN_OF_FIRE, 5.0f, TARGET_RANDOM_DESTINATION, 0, 6, false, true);
        rainOfFire->setAttackStopTimer(1000);
        rainOfFire->setMinMaxDistance(0.0f, 30.0f);

        auto fear = addAISpell(HW_FEAR, 4.0f, TARGET_RANDOM_SINGLE, 0, 10, false, true);
        fear->setAttackStopTimer(1000);
        fear->setMinMaxDistance(0.0f, 30.0f);

        auto shadowBurst = addAISpell(SHADOW_BURST, 4.0f, TARGET_VARIOUS, 0, 8);
        shadowBurst->setAttackStopTimer(1000);
        shadowBurst->setMinMaxDistance(0.0f, 30.0f);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Hellfire Channeler
class HellfireChannelerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HellfireChannelerAI(c); }
    explicit HellfireChannelerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        shadowGasp = addAISpell(SPELL_SHADOW_GRASP_C, 0.0f, TARGET_SELF);
        shadowBolt = addAISpell(SPELL_SHADOW_BOLT_VOLLEY, 0.0f, TARGET_DESTINATION);
        fear = addAISpell(SPELL_FEAR, 0.0f, TARGET_RANDOM_SINGLE);
        darkMending = addAISpell(SPELL_DARK_MENDING, 8.0f, TARGET_RANDOM_FRIEND, 0, Util::getRandomUInt(10, 20));
        darkMending->setAttackStopTimer(1000);
        darkMending->setMinMaxDistance(0.0f, 30.0f);
        darkMending->setMinMaxPercentHp(0, 70);
        summonAbyssal = addAISpell(SPELL_BURNING_ABYSSAL, 0.0f, TARGET_ATTACKING);
        soulTransfer = addAISpell(SPELL_SOUL_TRANSFER, 0.0f, TARGET_SOURCE);
    }

    void OnLoad() override
    {
        _castAISpell(shadowGasp);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->getAIInterface()->setReactState(REACT_DEFENSIVE);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getInstanceScript()->DoAction(ACTION_START_CHANNELERS_EVENT);

        scriptEvents.addEvent(EVENT_SHADOWBOLT, 20000);
        scriptEvents.addEvent(EVENT_ABYSSAL, 30000);
        scriptEvents.addEvent(EVENT_FEAR1, Util::getRandomUInt(15000, 20000));
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getInstanceScript()->setLocalData(DATA_DOOR, ACTION_ENABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if(!_isInCombat() && !getCreature()->hasAurasWithId(SPELL_SHADOW_GRASP_C))
            _castAISpell(shadowGasp);

        scriptEvents.updateEvents(time_passed, getScriptPhase());

        if (_isCasting())
            return;

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_SHADOWBOLT:
                _castAISpell(shadowBolt);
                scriptEvents.addEvent(EVENT_SHADOWBOLT, Util::getRandomUInt(15000, 20000));
                break;
            case EVENT_FEAR1:
                _castAISpell(fear);
                scriptEvents.addEvent(EVENT_FEAR1, Util::getRandomUInt(25000, 40000));
                break;
            case EVENT_ABYSSAL:
                _castAISpell(summonAbyssal);
                scriptEvents.addEvent(EVENT_ABYSSAL, 60000);
                break;
            default:
                break;
            }
        }
    }

    void OnDied(Unit* /*killer*/) override
    {
         _castAISpell(soulTransfer);
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t /*fAmount*/) override
    {
        if (!getCreature()->getAIInterface()->getAllowedToEnterCombat())
            getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
    }

protected:
    CreatureAISpells* shadowGasp;
    CreatureAISpells* shadowBolt;
    CreatureAISpells* fear;
    CreatureAISpells* darkMending;
    CreatureAISpells* summonAbyssal;
    CreatureAISpells* soulTransfer;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Magtheridon
class MagtheridonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MagtheridonAI(c); }
    explicit MagtheridonAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        shadowCage = addAISpell(SPELL_SHADOW_CAGE_C, 0.0f, TARGET_SELF);
        shadowCage->mIsTriggered = true;
        cleave = addAISpell(SPELL_CLEAVE, 0.0f, TARGET_ATTACKING);
        blaseTarget = addAISpell(SPELL_BLAZE_TARGET, 0.0f, TARGET_VARIOUS);
        quake = addAISpell(SPELL_QUAKE, 0.0f, TARGET_VARIOUS);
        blastNova = addAISpell(SPELL_BLAST_NOVA, 0.0f, TARGET_VARIOUS);
        berserk = addAISpell(SPELL_BERSERK, 0.0f, TARGET_SELF);
        shake = addAISpell(SPELL_CAMERA_SHAKE, 0.0f, TARGET_VARIOUS);
    }

    void OnLoad() override
    {
        Reset();
    }

    void CombatStart()
    {
        scriptEvents.removeEvent(EVENT_START_FIGHT);
        scriptEvents.removeEvent(EVENT_NEARLY_EMOTE);
        scriptEvents.addEvent(EVENT_RELEASED, 6000);

        getCreature()->sendChatMessageAlternateEntry(17257, CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, "%s breaks free!");
        sendDBChatMessage(SAY_FREE);
        _removeAura(SPELL_SHADOW_CAGE_C);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        scriptEvents.resetEvents();
        getInstanceScript()->setLocalData(DATA_DOOR, ACTION_ENABLE);
        getInstanceScript()->setLocalData(DATA_MANTICRON_CUBE, ACTION_DISABLE);
        getInstanceScript()->setLocalData(DATA_COLLAPSE, ACTION_DISABLE);
        getInstanceScript()->setLocalData(DATA_COLLAPSE_2, ACTION_DISABLE);

        Reset();
    }

    void OnDied(Unit* /*killer*/) override
    {
        sendDBChatMessage(SAY_DEATH);
        getInstanceScript()->setLocalData(DATA_MANTICRON_CUBE, ACTION_DISABLE);
    }

    void OnTargetDied(Unit* target) override
    {
        if (target->getObjectTypeId() == TYPEID_PLAYER)
            sendDBChatMessage(SAY_SLAY);
    }  

    void AIUpdate(unsigned long time_passed) override
    {
        if(isScriptPhase(PHASE_BANISH))
            _castAISpell(shadowCage);

        if (!isScriptPhase(PHASE_BANISH) && !isScriptPhase(PHASE_1) && !getCreature()->getAIInterface()->getCurrentTarget())
            return;

        scriptEvents.updateEvents(time_passed, getScriptPhase());

        if (_isCasting())
            return;

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_BERSERK :
                _castAISpell(berserk);
                break;
            case EVENT_CLEAVE:
                _castAISpell(cleave);
                scriptEvents.addEvent(EVENT_CLEAVE, 10000);
                break;
            case EVENT_BLAZE:
                _castAISpell(blaseTarget);
                scriptEvents.addEvent(EVENT_BLAZE, 20000);
                break;
            case EVENT_QUAKE:
                _castAISpell(quake);
                scriptEvents.addEvent(EVENT_QUAKE, 60000);
                break;
            case EVENT_START_FIGHT:
                CombatStart();
                break;
            case EVENT_RELEASED:
                getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
                getCreature()->getAIInterface()->setImmuneToPC(false);
                getCreature()->getAIInterface()->setImmuneToNPC(false);
                getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                setScriptPhase(PHASE_2);

                getInstanceScript()->setLocalData(DATA_MANTICRON_CUBE, ACTION_ENABLE);
                scriptEvents.addEvent(EVENT_CLEAVE, 10000);
                scriptEvents.addEvent(EVENT_BLAST_NOVA, 60000);
                scriptEvents.addEvent(EVENT_BLAZE, 20000);
                scriptEvents.addEvent(EVENT_QUAKE, 35000);
                scriptEvents.addEvent(EVENT_BERSERK, 12000000);
                break;
            case EVENT_COLLAPSE:
                getInstanceScript()->setLocalData(DATA_COLLAPSE_2, ACTION_ENABLE);
                scriptEvents.addEvent(EVENT_DEBRIS_KNOCKDOWN, 4000);
                break;
            case EVENT_DEBRIS_KNOCKDOWN:
                {
                    if (Creature* trigger = ((MagtheridonsLairInstanceScript*)getInstanceScript())->worldTrigger)
                    {
                        trigger->castSpell(trigger, SPELL_DEBRIS_KNOCKDOWN, true);
                        getCreature()->getAIInterface()->setReactState(REACT_AGGRESSIVE);
                    }
                    scriptEvents.addEvent(EVENT_DEBRIS, 20000);
                }
                break;
            case EVENT_DEBRIS:
                {
                    Unit* target = getBestPlayerTarget(TargetFilter_NotCurrent);
                    if(!target)
                        target = getBestPlayerTarget(TargetFilter_Current);

                    if (target)
                    {
                        /*Creature* debris = */spawnCreature(NPC_MAGTHERIDON_ROOM, target->GetPosition());
                        target->castSpell(target, SPELL_DEBRIS_VISUAL, true); // hackfix invis creatures cant have visual spells atm -_-
                        scriptEvents.addEvent(EVENT_DEBRIS, 20000);
                    }
                }
                break;
            case EVENT_NEARLY_EMOTE:
                getCreature()->sendChatMessageAlternateEntry(17257, CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, "%s is nearly free of his bonds!");
                break;
            case EVENT_BLAST_NOVA:
                getCreature()->sendChatMessageAlternateEntry(17257, CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, "%s begins to cast Blast Nova!");
                _castAISpell(blastNova);
                scriptEvents.addEvent(EVENT_BLAST_NOVA, 55000);
                break;
            case EVENT_TAUNT:
                switch (Util::getRandomUInt(0, 5))
                {
                case 0:
                    sendDBChatMessage(SAY_TAUNT01);
                    break;
                case 1:
                    sendDBChatMessage(SAY_TAUNT02);
                    break;
                case 2:
                    sendDBChatMessage(SAY_TAUNT03);
                    break;
                case 3:
                    sendDBChatMessage(SAY_TAUNT04);
                    break;
                case 4:
                    sendDBChatMessage(SAY_TAUNT05);
                    break;
                case 5:
                    sendDBChatMessage(SAY_TAUNT06);
                    break;
                }
                
                scriptEvents.addEvent(EVENT_TAUNT, Util::getRandomUInt(240000, 300000));
                break;
            default:
                break;
            }
        }

        // When we have 5 Auras from the Cubes Banish us
        if (getCreature()->getAuraCountForId(SPELL_SHADOW_GRASP_VISUAL) == 5)
            _castAISpell(shadowCage);
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_START_CHANNELERS_EVENT && isScriptPhase(PHASE_BANISH))
        {
            setScriptPhase(PHASE_1);
            getCreature()->sendChatMessageAlternateEntry(17257, CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, "%s's bonds begin to weaken!");

            scriptEvents.addEvent(EVENT_START_FIGHT, 120000);
            scriptEvents.addEvent(EVENT_NEARLY_EMOTE, 60000);
            scriptEvents.removeEvent(EVENT_TAUNT);

            getInstanceScript()->setData(DATA_MAGTHERIDON, InProgress);
        }
    }

    void OnHitBySpell(uint32_t spellId, Unit* /*caster*/) override
    {
        if (spellId == SPELL_SHADOW_CAGE)
            sendDBChatMessage(SAY_BANISHED);
    }

    void OnDamageTaken(Unit* /*_attacker*/, uint32_t /*_amount*/) override
    {
        if (_getHealthPercent() < 30 && !isScriptPhase(PHASE_3))
        {
            setScriptPhase(PHASE_3);
            getCreature()->getAIInterface()->setReactState(REACT_PASSIVE);
            //me->AttackStop();
            sendDBChatMessage(SAY_COLLAPSE);
            getInstanceScript()->setLocalData(DATA_COLLAPSE, ACTION_ENABLE);
            _castAISpell(shake);
            scriptEvents.addEvent(EVENT_COLLAPSE, 6000);
        }
    }

    void Reset()
    {
        _castAISpell(shadowCage);
        setScriptPhase(PHASE_BANISH);
        scriptEvents.addEvent(EVENT_TAUNT, 240000);
    }

protected:
    CreatureAISpells* shadowCage;
    CreatureAISpells* cleave;
    CreatureAISpells* blaseTarget;
    CreatureAISpells* quake;
    CreatureAISpells* blastNova;
    CreatureAISpells* berserk;
    CreatureAISpells* shake;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Soul Transfer
class SoulTransfer : public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t /*effectIndex*/, std::vector<uint64_t>* effectTargets) override
    {
        // Hackfix shouldnt only cast on Channelers
        effectTargets->clear();

        std::vector<Player*> players;
        for (const auto& itr : spell->getUnitCaster()->getInRangeObjectsSet())
        {
            float distance = spell->getUnitCaster()->CalcDistance(itr);
            if (itr->isCreature() && itr->getEntry() == NPC_HELLFIRE_CHANNELLER && distance <= 100.0f)
            {
                effectTargets->push_back(itr->getGuid());
            }
        }
    }
};

void SetupMagtheridonsLair(ScriptMgr* mgr)
{
    // Instance
    mgr->register_instance_script(MAP_MAGTHERIDONS_LAIR, &MagtheridonsLairInstanceScript::Create);

    // GameObjects
    mgr->register_gameobject_script(GO_MANTICRON_CUBE, &ManticronCubeGO::Create);

    // Creatures
    mgr->register_creature_script(NPC_HELFIRE_RAID_TRIGGER, &CubeTriggerAI::Create);
    mgr->register_creature_script(NPC_MAGTHERIDON_ROOM, &RoomTrigger::Create);
    mgr->register_creature_script(NPC_HELLFIRE_WARDER, &HellfireWarderAI::Create);
    mgr->register_creature_script(NPC_HELLFIRE_CHANNELLER, &HellfireChannelerAI::Create);

    // Spells
    mgr->register_spell_script(SPELL_SOUL_TRANSFER, new SoulTransfer);

    // Boss
    mgr->register_creature_script(NPC_MAGTHERIDON, &MagtheridonAI::Create);
}
