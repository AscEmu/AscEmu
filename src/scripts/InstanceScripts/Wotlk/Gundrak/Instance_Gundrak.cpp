/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_Gundrak.h"

#include "Setup.h"
#include "Map/Maps/MapScriptInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Utilities/Random.hpp"

namespace gundrak
{
    constexpr uint32_t DATA_COLOSSUS = 1;
    constexpr uint32_t DATA_SLADRAN = 2;

    enum Actions : int32_t
    {
        ACTION_UNFREEZE_COLOSSUS = 1
    };

    enum Data : uint32_t
    {
        DATA_COLOSSUS_PHASE = 1
    };

    enum ColossusPhases : uint32_t
    {
        COLOSSUS_PHASE_NORMAL = 0,
        COLOSSUS_PHASE_FIRST_ELEMENTAL = 1,
        COLOSSUS_PHASE_SECOND_ELEMENTAL = 2
    };

    enum Moves : uint32_t
    {
        MOVE_TO_COLOSSUS = 1,
        MOVE_TO_SLADRAN = 2
    };

    // the statues light up a while after the altar was used
    constexpr uint32_t STATUE_ACTIVATION_MS = 3500;
    constexpr float MOJO_RANGE = 60.0f;

    const LocationVector snakeSpawns[5] =
    {
        { 1783.81f, 646.637f, 133.948f, 3.71755f },
        { 1775.03f, 606.586f, 134.165f, 1.43117f },
        { 1717.39f, 630.041f, 129.282f, 5.96903f },
        { 1765.66f, 646.542f, 134.02f, 5.11381f },
        { 1716.76f, 635.159f, 129.282f, 0.191986f }
    };
}

ObjectData const gundrakCreatureData[] =
{
    { CN_DRAKKARI_COLOSSUS, gundrak::DATA_COLOSSUS },
    { CN_SLADRAN, gundrak::DATA_SLADRAN },
    { 0, 0 }
};

class GundrakScript : public InstanceScript
{
public:
    WoWGuid mSladranAltarGUID;
    WoWGuid mSladranStatueGUID;
    WoWGuid mColossusAltarGUID;
    WoWGuid mColossusStatueGUID;
    WoWGuid mMoorabiAltarGUID;
    WoWGuid mMoorabiStatueGUID;
    WoWGuid mGalDarahStatueGUID;

    WoWGuid mEckDoorsGUID;

    WoWGuid mTrapDoorGUID;
    WoWGuid mCoilisionGUID;

    WoWGuid mCombatDoorsGUID;
    WoWGuid mDoor1GUID;
    WoWGuid mDoor2GUID;

    uint8_t mStatueCount;

    // one pending timer per altar, then the bridge
    uint32_t mStatueTimers[3];
    bool mStatueDone[3];
    uint32_t mBridgeTimer;
    bool mBridgeDone;

    explicit GundrakScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setupInstanceData(gundrakCreatureData, nullptr);

        mSladranAltarGUID = 0;
        mSladranStatueGUID = 0;
        mColossusAltarGUID = 0;
        mColossusStatueGUID = 0;
        mMoorabiAltarGUID = 0;
        mMoorabiStatueGUID = 0;
        mGalDarahStatueGUID = 0;

        mEckDoorsGUID = 0;

        mTrapDoorGUID = 0;
        mCoilisionGUID = 0;

        mCombatDoorsGUID = 0;
        mDoor1GUID = 0;
        mDoor2GUID = 0;

        mStatueCount = 0;
        for (uint8_t i = 0; i < 3; ++i)
        {
            mStatueTimers[i] = 0;
            mStatueDone[i] = false;
        }
        mBridgeTimer = 0;
        mBridgeDone = false;
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new GundrakScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_ALTAR1_SLADRAN:
            {
                mSladranAltarGUID = pGameObject->GetNewGUID();
                pGameObject->setFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
            case GO_STATUE1_SLADRAN:
            {
                mSladranStatueGUID = pGameObject->GetNewGUID();
                pGameObject->setFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
            case GO_ALTAR2_COLOSSUS:
            {
                mColossusAltarGUID = pGameObject->GetNewGUID();
                pGameObject->setFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
            case GO_STATUE2_COLOSSUS:
            {
                mColossusStatueGUID = pGameObject->GetNewGUID();
                pGameObject->setFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
            case GO_ALTAR3_MOORABI:
            {
                mMoorabiAltarGUID = pGameObject->GetNewGUID();
                pGameObject->setFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
            case GO_STATUE3_MOORABI:
            {
                mMoorabiStatueGUID = pGameObject->GetNewGUID();
                pGameObject->setFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
            case GO_STATUE4_GAL_DARAH:
            {
                mGalDarahStatueGUID = pGameObject->GetNewGUID();
                pGameObject->setFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
            case GO_ECKDOOR:
            {
                mEckDoorsGUID = pGameObject->GetNewGUID();
                pGameObject->setFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
            case GO_TRAPDOOR:
            {
                mTrapDoorGUID = pGameObject->GetNewGUID();
                pGameObject->setFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
            case GO_COILISION:
            {
                mCoilisionGUID = pGameObject->GetNewGUID();
                pGameObject->setFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
            case GO_GAL_DARAH_DOOR1:
            {
                mDoor1GUID = pGameObject->GetNewGUID();
                pGameObject->setFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
            case GO_GAL_DARAH_DOOR2:
            {
                mDoor2GUID = pGameObject->GetNewGUID();
                pGameObject->setFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
            case 192568:
                mCombatDoorsGUID = pGameObject->GetNewGUID();
                break;
        }
    }

    void OnGameObjectActivate(GameObject* pGameObject, Player* /*pPlayer*/) override
    {
        uint8_t altar = 0;
        switch (pGameObject->getEntry())
        {
            case GO_ALTAR1_SLADRAN:
                altar = 0;
                break;
            case GO_ALTAR2_COLOSSUS:
                altar = 1;
                break;
            case GO_ALTAR3_MOORABI:
                altar = 2;
                break;
            default:
                return;
        }

        if (mStatueDone[altar] || mStatueTimers[altar] != 0)
            return;

        pGameObject->setFlags(GO_FLAG_NOT_SELECTABLE);
        pGameObject->setState(GO_STATE_OPEN);
        mStatueTimers[altar] = addTimer(gundrak::STATUE_ACTIVATION_MS);
    }

    void UpdateEvent() override
    {
        for (uint8_t i = 0; i < 3; ++i)
        {
            if (mStatueDone[i] || mStatueTimers[i] == 0 || !isTimerFinished(mStatueTimers[i]))
                continue;

            removeTimer(mStatueTimers[i]);
            mStatueDone[i] = true;
            activateStatue(i);
        }

        if (!mBridgeDone && mBridgeTimer != 0 && isTimerFinished(mBridgeTimer))
        {
            removeTimer(mBridgeTimer);
            mBridgeDone = true;
            openBridge();
        }
    }

    void OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/) override
    {
        GameObject* pDoors = NULL;
        GameObject* pAltar = NULL;
        switch (pVictim->getEntry())
        {
            case CN_MOORABI:
            {
                pAltar = getGameObjectByGuid(mMoorabiAltarGUID);
                if (pAltar)
                    pAltar->removeFlags(GO_FLAG_NOT_SELECTABLE);

                if (mInstance->getDifficulty() != InstanceDifficulty::DUNGEON_HEROIC)
                    return;

                pDoors = getGameObjectByGuid(mEckDoorsGUID);
                if (pDoors)
                    pDoors->setState(GO_STATE_OPEN);
            }
            break;
            case CN_GAL_DARAH:
            {
                pDoors = getGameObjectByGuid(mDoor1GUID);
                if (pDoors)
                    pDoors->setState(GO_STATE_OPEN);

                pDoors = getGameObjectByGuid(mDoor2GUID);
                if (pDoors)
                    pDoors->setState(GO_STATE_OPEN);
            }
            break;
            case CN_SLADRAN:
            {
                pAltar = getGameObjectByGuid(mSladranAltarGUID);
                if (pAltar)
                    pAltar->removeFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
            case CN_DRAKKARI_COLOSSUS:
            {
                pAltar = getGameObjectByGuid(mColossusAltarGUID);
                if (pAltar)
                    pAltar->removeFlags(GO_FLAG_NOT_SELECTABLE);
            }
            break;
        }
    }

private:
    // the altar trigger fires the beam, the statue switches
    void activateStatue(uint8_t altar)
    {
        const WoWGuid altarGuids[3] = { mSladranAltarGUID, mColossusAltarGUID, mMoorabiAltarGUID };
        const WoWGuid statueGuids[3] = { mSladranStatueGUID, mColossusStatueGUID, mMoorabiStatueGUID };
        const uint32_t beams[3] = { ALTAR_FIRE_BEAM_SNAKE, ALTAR_FIRE_BEAM_ELEMENTAL, ALTAR_FIRE_BEAM_MAMMOTH };

        if (GameObject* pAltar = getGameObjectByGuid(altarGuids[altar]))
            if (Creature* trigger = getWorldMap()->getInterface()->getCreatureNearestCoords(pAltar->GetPositionX(), pAltar->GetPositionY(), pAltar->GetPositionZ(), CN_ALTAR_TRIGGER))
                trigger->castSpell(trigger, beams[altar], true);

        if (GameObject* pStatue = getGameObjectByGuid(statueGuids[altar]))
            pStatue->setState(GO_STATE_CLOSED);

        ++mStatueCount;
        if (mStatueCount >= 3 && !mBridgeDone && mBridgeTimer == 0)
            mBridgeTimer = addTimer(gundrak::STATUE_ACTIVATION_MS);
    }

    void openBridge()
    {
        const WoWGuid statueGuids[4] = { mSladranStatueGUID, mColossusStatueGUID, mMoorabiStatueGUID, mGalDarahStatueGUID };
        for (const auto& guid : statueGuids)
            if (GameObject* pStatue = getGameObjectByGuid(guid))
                pStatue->setState(GO_STATE_DESTROYED);

        if (GameObject* pTrapDoors = getGameObjectByGuid(mTrapDoorGUID))
            pTrapDoors->setState(GO_STATE_CLOSED);
        if (GameObject* pCoilision = getGameObjectByGuid(mCoilisionGUID))
            pCoilision->setState(GO_STATE_OPEN);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Slad'ran: snakes from 30 %, constrictors from 25 %
class SladranAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SladranAI(c); }
    explicit SladranAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        CreatureAISpells* sdPoisonNova = nullptr;
        if (isHeroic())
        {
            addAISpell(SLADRAN_POWERFUL_BITE_HC, 25.0f, TARGET_ATTACKING, 0, 6);
            addAISpell(SLADRAN_VENOM_BOLT_HC, 18.0f, TARGET_RANDOM_SINGLE, 2, 8);

            sdPoisonNova = addAISpell(SLADRAN_POISON_NOVA_HC, 10.0f, TARGET_SELF, 4, 16);
        }
        else
        {
            addAISpell(SLADRAN_POWERFUL_BITE, 25.0f, TARGET_ATTACKING, 0, 6);
            addAISpell(SLADRAN_VENOM_BOLT, 18.0f, TARGET_RANDOM_SINGLE, 2, 8);

            sdPoisonNova = addAISpell(SLADRAN_POISON_NOVA, 10.0f, TARGET_SELF, 4, 16);
        }

        if (sdPoisonNova != nullptr)
            sdPoisonNova->setAnnouncement("Slad'ran begins to cast Poison Nova!");

        addEmoteForEvent(Event_OnCombatStart, 8754);    // Drakkari gonna kill anybody who trespass on these lands!
        addEmoteForEvent(Event_OnTargetDied, 4217);     // You not breathin'? Good.
        addEmoteForEvent(Event_OnTargetDied, 4218);     // Ssscared now?
        addEmoteForEvent(Event_OnTargetDied, 4219);     // I eat you next, mon.
        addEmoteForEvent(Event_OnDied, 4220);           // I sssee now... Ssscourge wasss not... our greatessst enemy....
    }

    void InitOrReset() override
    {
        m_snakes = false;
        m_constrictors = false;
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        summons.despawnAll();
    }

    void OnDied(Unit* /*_killer*/) override
    {
        summons.despawnAll();
    }

    void OnDamageTaken(Unit* /*_attacker*/, uint32_t amount) override
    {
        const uint32_t health = getCreature()->getHealth();
        const uint32_t maxHealth = getCreature()->getMaxHealth();
        if (maxHealth == 0)
            return;

        const uint32_t after = amount >= health ? 0 : 100 * (health - amount) / maxHealth;
        if (!m_snakes && after < 30)
        {
            m_snakes = true;
            sendDBChatMessage(SLADRAN_SAY_SNAKES);
            addAIFunction([this](CreatureAIFunc) { summonWave(CN_SLADRAN_VIPER); }, DoLoopScheduler(5s, 100.0f));
        }

        if (m_snakes && !m_constrictors && after < 25)
        {
            m_constrictors = true;
            sendDBChatMessage(SLADRAN_SAY_CONSTRICTORS);
            removeAllFunctionsFromScheduler();
            addAIFunction([this](CreatureAIFunc) { summonWave(CN_SLADRAN_CONSTRICTOR); }, DoLoopScheduler(5s, 100.0f));
        }
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
    }

private:
    void summonWave(uint32_t entry)
    {
        const uint8_t count = isHeroic() ? 5 : 3;
        for (uint8_t i = 0; i < count; ++i)
            summonCreature(entry, gundrak::snakeSpawns[i], CORPSE_TIMED_DESPAWN, 20000);
    }

    bool m_snakes = false;
    bool m_constrictors = false;
};

// vipers and constrictors run to Slad'ran, then fight; a full grip wraps the player
class SladranSnakeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SladranSnakeAI(c); }
    explicit SladranSnakeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (pCreature->getEntry() == CN_SLADRAN_VIPER)
            addAISpell(SLADRAN_VENOMOUS_BITE, 40.0f, TARGET_ATTACKING, 0, 10);
        else
            addAISpell(SLADRAN_GRIP, 50.0f, TARGET_ATTACKING, 0, 4);
    }

    void OnLoad() override
    {
        if (Creature* boss = getInstanceScript() != nullptr ? getInstanceScript()->getCreatureFromData(gundrak::DATA_SLADRAN) : nullptr)
            movePoint(gundrak::MOVE_TO_SLADRAN, boss->GetPosition());
    }

    void OnReachWP(uint32_t /*type*/, uint32_t id) override
    {
        if (id != gundrak::MOVE_TO_SLADRAN)
            return;

        if (Unit* target = getBestPlayerTarget())
            getCreature()->getAIInterface()->onHostileAction(target);
    }

    void OnSpellHitTarget(Object* target, SpellInfo const* info) override
    {
        if (info == nullptr || info->getId() != SLADRAN_GRIP || target == nullptr || !target->isPlayer())
            return;

        // five grips wrap the player and use the snake up
        auto* player = static_cast<Player*>(target);
        Aura* grip = player->getAuraWithId(SLADRAN_GRIP);
        if (grip == nullptr || grip->getStackCount() < 5)
            return;

        player->removeAllAurasById(SLADRAN_GRIP);
        player->castSpell(player, SLADRAN_SNAKE_WRAP, true);
        despawn(500, 0);
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        despawn(1000, 0);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Moorabi: transforms into a mammoth unless the cast is interrupted
class MoorabiAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MoorabiAI(c); }
    explicit MoorabiAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addEmoteForEvent(Event_OnCombatStart, MOORABI_SAY_AGGRO);
        addEmoteForEvent(Event_OnTargetDied, MOORABI_SAY_SLAY_1);
        addEmoteForEvent(Event_OnTargetDied, MOORABI_SAY_SLAY_2);
        addEmoteForEvent(Event_OnDied, MOORABI_SAY_DEATH);

        // troll form
        addAISpell(MOORABI_GROUND_TREMOR, 50.0f, TARGET_SELF, 0, 10)->setAvailableForScriptPhase({ 1 });
        addAISpell(MOORABI_NUMBING_SHOUT, 50.0f, TARGET_SELF, 0, 10)->setAvailableForScriptPhase({ 1 });
        addAISpell(MOORABI_DETERMINED_STAB, 50.0f, TARGET_ATTACKING, 0, 8)->setAvailableForScriptPhase({ 1 });

        auto transformation = addAISpell(MOORABI_TRANSFORMATION, 100.0f, TARGET_SELF, 10, 10);
        transformation->setAvailableForScriptPhase({ 1 });
        transformation->addDBEmote(MOORABI_EMOTE_TRANSFORM);
        transformation->addDBEmote(MOORABI_SAY_TRANSFORM);

        // mammoth form
        auto quake = addAISpell(MOORABI_QUAKE, 50.0f, TARGET_SELF, 0, 10);
        quake->setAvailableForScriptPhase({ 2 });
        quake->addDBEmote(MOORABI_SAY_QUAKE);
        addAISpell(MOORABI_NUMBING_ROAR, 50.0f, TARGET_SELF, 0, 10)->setAvailableForScriptPhase({ 2 });
        addAISpell(MOORABI_DETERMINED_GORE, 50.0f, TARGET_ATTACKING, 0, 8)->setAvailableForScriptPhase({ 2 });
    }

    void InitOrReset() override
    {
        setScriptPhase(1);
    }

    void OnCombatStart(Unit* /*_target*/) override
    {
        castSpellOnSelf(MOORABI_MOJO_FRENZY, true);
    }

    void OnHitBySpell(uint32_t spellId, Unit* /*_caster*/) override
    {
        if (spellId != MOORABI_TRANSFORMATION || isScriptPhase(2))
            return;

        setScriptPhase(2);
        _removeAura(MOORABI_MOJO_FRENZY);
        sendDBChatMessage(MOORABI_EMOTE_TRANSFORMED);
        sendDBChatMessage(MOORABI_SAY_TRANSFORMED);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Drakkari Colossus: the living mojos wake him, at 50 % and 5 % the elemental leaves him
class DrakkariColossusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DrakkariColossusAI(c); }
    explicit DrakkariColossusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(COLOSSUS_MIGHTY_BLOW, 40.0f, TARGET_ATTACKING, 0, 10);
    }

    void InitOrReset() override
    {
        m_phase = gundrak::COLOSSUS_PHASE_NORMAL;
        m_frozen = false;
        castSpellOnSelf(isHeroic() ? COLOSSUS_MORTAL_STRIKES_HC : COLOSSUS_MORTAL_STRIKES, true);
        setReactState(REACT_PASSIVE);
    }

    void OnCombatStart(Unit* /*_target*/) override
    {
        // attacked before the mojos joined him
        if (!m_frozen)
            setReactState(REACT_AGGRESSIVE);
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        summons.despawnAll();
    }

    void OnDied(Unit* /*_killer*/) override
    {
        summons.despawnAll();
    }

    void OnDamageTaken(Unit* /*_attacker*/, uint32_t amount) override
    {
        if (m_frozen || m_phase == gundrak::COLOSSUS_PHASE_SECOND_ELEMENTAL)
            return;

        const uint32_t health = getCreature()->getHealth();
        const uint32_t maxHealth = getCreature()->getMaxHealth();
        if (maxHealth == 0)
            return;

        const uint32_t after = amount >= health ? 0 : 100 * (health - amount) / maxHealth;
        const uint32_t threshold = m_phase == gundrak::COLOSSUS_PHASE_FIRST_ELEMENTAL ? 5 : 50;
        if (after >= threshold)
            return;

        m_phase = m_phase == gundrak::COLOSSUS_PHASE_FIRST_ELEMENTAL ? gundrak::COLOSSUS_PHASE_SECOND_ELEMENTAL : gundrak::COLOSSUS_PHASE_FIRST_ELEMENTAL;
        freeze();
        summonCreature(CN_DRAKKARI_ELEMENTAL, getCreature()->GetPosition(), CORPSE_DESPAWN);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);

        // the second elemental starts with half of its health
        if (summon->getEntry() == CN_DRAKKARI_ELEMENTAL && m_phase == gundrak::COLOSSUS_PHASE_SECOND_ELEMENTAL)
            summon->setHealth(summon->getMaxHealth() / 2);
    }

    void DoAction(int32_t action) override
    {
        if (action != gundrak::ACTION_UNFREEZE_COLOSSUS)
            return;

        m_frozen = false;
        _removeAura(COLOSSUS_FREEZE_ANIM);
        getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);

        if (Unit* target = getBestPlayerTarget())
            getCreature()->getAIInterface()->onHostileAction(target);
    }

    uint32_t GetCreatureData(uint32_t type) const override
    {
        if (type == gundrak::DATA_COLOSSUS_PHASE)
            return m_phase;
        return 0;
    }

private:
    void freeze()
    {
        m_frozen = true;
        setReactState(REACT_PASSIVE);
        stopMovement();
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(COLOSSUS_FREEZE_ANIM, true);
    }

    uint32_t m_phase = gundrak::COLOSSUS_PHASE_NORMAL;
    bool m_frozen = false;
};

// the elemental merges back at 50 % in the first phase, its death ends the fight in the second
class DrakkariElementalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DrakkariElementalAI(c); }
    explicit DrakkariElementalAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ELEMENTAL_SURGE, 30.0f, TARGET_RANDOM_SINGLE, 0, 10);
    }

    void OnLoad() override
    {
        m_merging = false;
        setReactState(REACT_PASSIVE);
        castSpellOnSelf(ELEMENTAL_SPAWN_EFFECT, true);
        addAIFunction([this](CreatureAIFunc)
        {
            setReactState(REACT_AGGRESSIVE);
            if (Unit* target = getBestPlayerTarget())
                getCreature()->getAIInterface()->onHostileAction(target);
        }, DoOnceScheduler(2s, 100.0f));
    }

    void OnCombatStart(Unit* /*_target*/) override
    {
        castSpellOnSelf(ELEMENTAL_MOJO_VOLLEY, true);
    }

    void OnDamageTaken(Unit* /*_attacker*/, uint32_t amount) override
    {
        if (m_merging)
            return;

        Creature* colossus = getColossus();
        if (colossus == nullptr || colossus->GetScript() == nullptr || colossus->GetScript()->GetCreatureData(gundrak::DATA_COLOSSUS_PHASE) != gundrak::COLOSSUS_PHASE_FIRST_ELEMENTAL)
            return;

        const uint32_t health = getCreature()->getHealth();
        if (amount < health && 100 * (health - amount) / getCreature()->getMaxHealth() >= 50)
            return;

        // back into the colossus
        m_merging = true;
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        castSpellOnSelf(ELEMENTAL_SURGE_VISUAL, true);
        movePoint(gundrak::MOVE_TO_COLOSSUS, colossus->GetPosition());
    }

    void OnReachWP(uint32_t /*type*/, uint32_t id) override
    {
        if (id != gundrak::MOVE_TO_COLOSSUS)
            return;

        if (Creature* colossus = getColossus())
            if (colossus->GetScript() != nullptr)
                colossus->GetScript()->DoAction(gundrak::ACTION_UNFREEZE_COLOSSUS);

        despawn(0, 0);
    }

    void OnDied(Unit* killer) override
    {
        Creature* colossus = getColossus();
        if (colossus == nullptr || !colossus->isAlive() || colossus->GetScript() == nullptr)
            return;

        if (colossus->GetScript()->GetCreatureData(gundrak::DATA_COLOSSUS_PHASE) == gundrak::COLOSSUS_PHASE_SECOND_ELEMENTAL)
            colossus->die(killer, colossus->getHealth(), 0);
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        if (!m_merging)
            despawn(1000, 0);
    }

private:
    Creature* getColossus()
    {
        return getInstanceScript() != nullptr ? getInstanceScript()->getCreatureFromData(gundrak::DATA_COLOSSUS) : nullptr;
    }

    bool m_merging = false;
};

// the living mojos next to the colossus join him when attacked
class LivingMojoAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LivingMojoAI(c); }
    explicit LivingMojoAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(MOJO_WAVE, 30.0f, TARGET_ATTACKING, 0, 15);
        addAISpell(MOJO_PUDDLE, 30.0f, TARGET_ATTACKING, 0, 18);
    }

    void OnCombatStart(Unit* /*_target*/) override
    {
        Creature* colossus = getInstanceScript() != nullptr ? getInstanceScript()->getCreatureFromData(gundrak::DATA_COLOSSUS) : nullptr;
        if (colossus == nullptr || !colossus->isAlive() || colossus->getAIInterface()->getReactState() != REACT_PASSIVE)
            return;

        if (getCreature()->getDistanceSq(colossus) > gundrak::MOJO_RANGE * gundrak::MOJO_RANGE)
            return;

        // the whole group walks into the colossus
        for (Object* object : getCreature()->getInRangeObjectsSet())
        {
            if (!object->isCreature() || object->getEntry() != CN_LIVING_MOJO)
                continue;

            auto* mojo = static_cast<Creature*>(object);
            if (mojo->GetScript() != nullptr && mojo->getDistanceSq(colossus) <= gundrak::MOJO_RANGE * gundrak::MOJO_RANGE)
                mojo->GetScript()->DoAction(gundrak::ACTION_UNFREEZE_COLOSSUS);
        }
        DoAction(gundrak::ACTION_UNFREEZE_COLOSSUS);
    }

    void DoAction(int32_t action) override
    {
        if (action != gundrak::ACTION_UNFREEZE_COLOSSUS || m_joining)
            return;

        Creature* colossus = getInstanceScript() != nullptr ? getInstanceScript()->getCreatureFromData(gundrak::DATA_COLOSSUS) : nullptr;
        if (colossus == nullptr)
            return;

        m_joining = true;
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        movePoint(gundrak::MOVE_TO_COLOSSUS, colossus->GetPosition());
    }

    void OnReachWP(uint32_t /*type*/, uint32_t id) override
    {
        if (id != gundrak::MOVE_TO_COLOSSUS)
            return;

        if (Creature* colossus = getInstanceScript() != nullptr ? getInstanceScript()->getCreatureFromData(gundrak::DATA_COLOSSUS) : nullptr)
            if (colossus->GetScript() != nullptr)
                colossus->GetScript()->DoAction(gundrak::ACTION_UNFREEZE_COLOSSUS);

        despawn(0, 0);
    }

private:
    bool m_joining = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gal'darah
// Status: 20% done, missing rihno part, need vehicle support for that, missing stampade script
class GalDarahAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GalDarahAI(c); }
    explicit GalDarahAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (isHeroic())
            addAISpell(GALDARAH_STAMPEDE_HC, 20.0f, TARGET_SELF, 0, 12);
        else
            addAISpell(GALDARAH_STAMPEDE, 20.0f, TARGET_SELF, 0, 12);

        addEmoteForEvent(Event_OnCombatStart, 4199);    // I'm gonna spill your guts, mon!
        addEmoteForEvent(Event_OnTargetDied, 4200);     // What a rush!
        addEmoteForEvent(Event_OnTargetDied, 4201);     // Who needs gods when we ARE gods?
        addEmoteForEvent(Event_OnTargetDied, 4202);     // I told ya so!
        addEmoteForEvent(Event_OnDied, 4203);           // Even the mighty... can fall.
    }
};

void SetupGundrak(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_GUNDRAK, &GundrakScript::Create);

    mgr->register_creature_script(CN_SLADRAN, &SladranAI::Create);
    mgr->register_creature_script(CN_SLADRAN_VIPER, &SladranSnakeAI::Create);
    mgr->register_creature_script(CN_SLADRAN_CONSTRICTOR, &SladranSnakeAI::Create);
    mgr->register_creature_script(CN_MOORABI, &MoorabiAI::Create);
    mgr->register_creature_script(CN_DRAKKARI_COLOSSUS, &DrakkariColossusAI::Create);
    mgr->register_creature_script(CN_DRAKKARI_ELEMENTAL, &DrakkariElementalAI::Create);
    mgr->register_creature_script(CN_LIVING_MOJO, &LivingMojoAI::Create);
    mgr->register_creature_script(CN_GAL_DARAH, &GalDarahAI::Create);
};
