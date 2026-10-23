/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// \ todo Finish Kritkhir Encounter, needs more blizzlike | Anuburak | Add's AI and trash

#include "Instance_AzjolNerub.h"

#include "Setup.h"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Utilities/Random.hpp"

namespace hadronox
{
    constexpr uint32_t DATA_HADRONOX = 1;

    enum Actions : int32_t
    {
        ACTION_CRUSHER_ENGAGED = 1,
        ACTION_HADRONOX_MOVE = 2
    };

    enum Data : uint32_t
    {
        DATA_ENTERED_COMBAT = 1,
        DATA_WEBBED_DOORS = 2,
        DATA_PACK_ID = 3
    };

    enum Moves : uint32_t
    {
        MOVE_STEP = 1,
        MOVE_OUTSIDE = 10,
        MOVE_DOWNSTAIRS = 11,
        MOVE_DOWNSTAIRS_2 = 12,
        MOVE_HADRONOX = 13
    };

    // the climb from the bottom to the web gate
    constexpr uint8_t STEP_COUNT = 4;
    const LocationVector steps[STEP_COUNT] =
    {
        { 515.5848f, 544.2007f, 673.6272f },
        { 562.191f, 514.068f, 696.4448f },
        { 610.3828f, 518.6407f, 695.9385f },
        { 530.42f, 560.003f, 733.0308f }
    };

    // the three doors the waves come from, then the way down to Hadronox (an x of 0 skips the point)
    constexpr uint8_t SPAWN_COUNT = 3;
    const LocationVector spawnPoints[SPAWN_COUNT] =
    {
        { 477.016f, 618.4f, 771.515f },
        { 583.091f, 617.371f, 771.551f },
        { 581.448f, 608.841f, 739.405f }
    };
    const LocationVector outsideMoves[SPAWN_COUNT] =
    {
        { 485.314606f, 611.418640f, 771.428406f },
        { 575.760437f, 611.516418f, 771.427368f },
        { 588.930725f, 598.233276f, 739.142151f }
    };
    const LocationVector downstairsMoves[SPAWN_COUNT] =
    {
        { 513.574341f, 587.022156f, 736.229065f },
        { 537.920410f, 580.436157f, 732.796692f },
        { 601.289246f, 583.259644f, 725.443054f }
    };
    const LocationVector downstairsMoves2[SPAWN_COUNT] =
    {
        { 571.498718f, 576.978333f, 727.582947f },
        { 571.498718f, 576.978333f, 727.582947f },
        { 0.0f, 0.0f, 0.0f }
    };

    // below this height the waves walk to the second step first
    constexpr float LOWER_LEVEL_Z = 702.0f;
    constexpr uint32_t WAVE_ENTRIES[3] = { CN_ANUBAR_CHAMPION, CN_ANUBAR_CRYPT_FIEND, CN_ANUBAR_NECROMANCER };

    struct PackMember
    {
        uint32_t entry;
        LocationVector position;
        uint32_t despawnMs;
    };

    // the second and third crusher pack, the first one is spawned by the database
    const PackMember packTwo[3] =
    {
        { CN_ANUBAR_CRUSHER, { 493.477f, 603.344f, 760.563f, 5.44024f }, 45000 },
        { CN_CRUSHER_CHAMPION, { 490.442f, 604.335f, 763.182f, 5.6256f }, 30000 },
        { CN_CRUSHER_NECROMANCER, { 488.825f, 609.282f, 767.588f, 5.59029f }, 30000 }
    };
    const PackMember packThree[3] =
    {
        { CN_ANUBAR_CRUSHER, { 566.979f, 602.571f, 759.642f, 3.88597f }, 45000 },
        { CN_CRUSHER_CRYPT_FIEND, { 569.348f, 604.999f, 763.214f, 4.17983f }, 30000 },
        { CN_CRUSHER_NECROMANCER, { 572.474f, 607.411f, 767.178f, 3.94417f }, 30000 }
    };
}

ObjectData const azjolNerubCreatureData[] =
{
    { BOSS_HADRONOX, hadronox::DATA_HADRONOX },
    { 0, 0 }
};

class AzjolNerubInstanceScript : public InstanceScript
{
public:
    explicit AzjolNerubInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setupInstanceData(azjolNerubCreatureData, nullptr);
    }
    static InstanceScript* Create(WorldMap* pMapMgr) { return new AzjolNerubInstanceScript(pMapMgr); }
};

// Krikthir The Gatewatcher
class KrikthirAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KrikthirAI(c); }
    explicit KrikthirAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (!isHeroic())
        {
            addAISpell(KRIKTHIR_CURSEOFFATIGUE, 100.0f, TARGET_SELF, 0, 10);

            auto curseOfFattigue = addAISpell(KRIKTHIR_MINDFLAY, 100.0f, TARGET_RANDOM_SINGLE, 0, 7);
            curseOfFattigue->setMinMaxDistance(0.0f, 30.0f);
        }
        else
        {
            addAISpell(KRIKTHIR_CURSEOFFATIGUE_HC, 100.0f, TARGET_SELF, 0, 10);

            auto curseOfFattigue = addAISpell(KRIKTHIR_MINDFLAY_HC, 100.0f, TARGET_RANDOM_SINGLE, 0, 7);
            curseOfFattigue->setMinMaxDistance(0.0f, 30.0f);
        }

        mEnraged = false;

        addEmoteForEvent(Event_OnCombatStart, 3908);    // This kingdom belongs to the Scourge. Only the dead may enter!
        addEmoteForEvent(Event_OnTargetDied, 3910);     // As Anub'arak commands!
        addEmoteForEvent(Event_OnTargetDied, 3909);     // You were foolish to come.
        addEmoteForEvent(Event_OnDied, 3911);           // I should be grateful... but I long ago lost the capacity....
    }

    void AIUpdate() override
    {
        // case for scriptphase
        if (getCreature()->getHealthPct() <= 10 && mEnraged == false)
        {
            _applyAura(KRIKTHIR_ENRAGE);
            mEnraged = true;
        }
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        GameObject* Doors = getNearestGameObject(192395);
        if (Doors != nullptr)
            Doors->despawn(0, 0);
    }
    bool mEnraged;
};

// Hadronox
class HadronoxAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HadronoxAI(c); }
    explicit HadronoxAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (!isHeroic())
        {
            addAISpell(HADRONOX_WEBGRAB, 22.0f, TARGET_RANDOM_SINGLE, 0, 14);

            auto leechpoison = addAISpell(HADRONOX_LEECHPOISON, 14.0f, TARGET_SELF, 0, 25);
            leechpoison->setMinMaxDistance(0.0f, 20.0f);

            auto acidcloud = addAISpell(HADRONOX_ACIDCLOUD, 18.0f, TARGET_RANDOM_SINGLE, 0, 20);
            acidcloud->setMinMaxDistance(0.0f, 60.0f);
        }
        else
        {
            addAISpell(HADRONOX_WEBGRAB_HC, 22.0f, TARGET_RANDOM_SINGLE, 0, 14);

            auto leechpoison = addAISpell(HADRONOX_LEECHPOISON_HC, 14.0f, TARGET_SELF, 0, 25);
            leechpoison->setMinMaxDistance(0.0f, 20.0f);

            auto acidcloud = addAISpell(HADRONOX_ACIDCLOUD_HC, 18.0f, TARGET_RANDOM_SINGLE, 0, 20);
            acidcloud->setMinMaxDistance(0.0f, 60.0f);
        }

        addAISpell(HADRONOX_PIERCEARMOR, 20.0f, TARGET_ATTACKING, 0, 5);
    }

    void InitOrReset() override
    {
        m_step = 0;
        m_enteredCombat = false;
        m_doorsWebbed = false;
        m_playerCombat = false;
        m_checkTimer = 0;
        for (uint8_t i = 0; i < hadronox::SPAWN_COUNT; ++i)
            m_waveTimers[i] = Util::getRandomUInt(2000, 17000);

        setStep(0);
    }

    void AIUpdate(unsigned long timePassed) override
    {
        if (!getCreature()->isAlive())
            return;

        updatePlayerCombat(timePassed);
        updateWaves(timePassed);
    }

    void OnReachWP(uint32_t /*type*/, uint32_t id) override
    {
        if (id != hadronox::MOVE_STEP)
            return;

        setReactState(REACT_AGGRESSIVE);
        if (m_step < hadronox::STEP_COUNT - 1)
            return;

        // at the gate the doors get webbed, the waves stop
        castSpellAOE(HADRONOX_WEB_FRONT_DOORS, true);
        castSpellAOE(HADRONOX_WEB_SIDE_DOORS, true);
        m_doorsWebbed = true;
    }

    void DoAction(int32_t action) override
    {
        switch (action)
        {
            case hadronox::ACTION_CRUSHER_ENGAGED:
                if (m_enteredCombat)
                    break;
                m_enteredCombat = true;
                summonPack(hadronox::packTwo, 2);
                summonPack(hadronox::packThree, 3);
                break;
            case hadronox::ACTION_HADRONOX_MOVE:
                if (m_step < hadronox::STEP_COUNT - 1)
                    setStep(m_step + 1);
                break;
            default:
                break;
        }
    }

    uint32_t GetCreatureData(uint32_t type) const override
    {
        if (type == hadronox::DATA_ENTERED_COMBAT)
            return m_enteredCombat ? 1 : 0;
        if (type == hadronox::DATA_WEBBED_DOORS)
            return m_doorsWebbed ? 1 : 0;
        return 0;
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
    }

    void OnDied(Unit* /*_killer*/) override
    {
        summons.despawnAll();
    }

private:
    void setStep(uint8_t step)
    {
        if (m_playerCombat)
            return;

        m_step = step;
        setReactState(REACT_PASSIVE);
        getCreature()->SetSpawnLocation(hadronox::steps[step]);
        stopMovement();
        movePoint(hadronox::MOVE_STEP, hadronox::steps[step]);
    }

    void summonPack(hadronox::PackMember const* pack, uint32_t packId)
    {
        for (uint8_t i = 0; i < 3; ++i)
        {
            Creature* member = summonCreature(pack[i].entry, pack[i].position, CORPSE_TIMED_DESPAWN, pack[i].despawnMs);
            if (member != nullptr && member->GetScript() != nullptr)
                member->GetScript()->SetCreatureData(hadronox::DATA_PACK_ID, packId);
        }
    }

    bool isInCombatWithPlayer()
    {
        Unit* target = getCreature()->getAIInterface()->getCurrentTarget();
        return target != nullptr && target->getPlayerOwnerOrSelf() != nullptr;
    }

    void updatePlayerCombat(unsigned long timePassed)
    {
        m_checkTimer += timePassed;
        if (m_checkTimer < 1000)
            return;
        m_checkTimer = 0;

        const bool playerCombat = isInCombatWithPlayer();
        if (playerCombat == m_playerCombat)
            return;

        m_playerCombat = playerCombat;
        if (m_playerCombat)
        {
            // players interrupt the climb
            stopMovement();
            setReactState(REACT_AGGRESSIVE);
            moveChase(getCreature()->getAIInterface()->getCurrentTarget());
            return;
        }

        // the players are gone, the encounter starts over
        summons.despawnAll();
        despawn(25000, 1000);
    }

    void updateWaves(unsigned long timePassed)
    {
        if (m_doorsWebbed || !getCreature()->getWorldMap()->hasPlayers())
            return;

        for (uint8_t i = 0; i < hadronox::SPAWN_COUNT; ++i)
        {
            if (m_waveTimers[i] > timePassed)
            {
                m_waveTimers[i] -= static_cast<uint32_t>(timePassed);
                continue;
            }

            m_waveTimers[i] = Util::getRandomUInt(20000, 30000);
            summonCreature(hadronox::WAVE_ENTRIES[Util::getRandomUInt(0, 2)], hadronox::spawnPoints[i], CORPSE_DESPAWN);
        }
    }

    uint8_t m_step = 0;
    bool m_enteredCombat = false;
    bool m_doorsWebbed = false;
    bool m_playerCombat = false;
    uint32_t m_checkTimer = 0;
    uint32_t m_waveTimers[hadronox::SPAWN_COUNT] = { 0, 0, 0 };
};

// the Anub'ar waves walk down from the doors and attack Hadronox
class HadronoxFoeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HadronoxFoeAI(c); }
    explicit HadronoxFoeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        switch (pCreature->getEntry())
        {
            case CN_ANUBAR_CHAMPION:
            case CN_CRUSHER_CHAMPION:
                addAISpell(ANUBAR_REND, 30.0f, TARGET_ATTACKING, 0, 14);
                addAISpell(ANUBAR_PUMMEL, 25.0f, TARGET_ATTACKING, 0, 15);
                break;
            case CN_ANUBAR_CRYPT_FIEND:
            case CN_CRUSHER_CRYPT_FIEND:
                addAISpell(ANUBAR_CRUSHING_WEBS, 30.0f, TARGET_ATTACKING, 0, 14);
                addAISpell(ANUBAR_INFECTED_WOUND, 25.0f, TARGET_ATTACKING, 0, 20);
                break;
            case CN_ANUBAR_NECROMANCER:
            case CN_CRUSHER_NECROMANCER:
                addAISpell(ANUBAR_SHADOW_BOLT, 40.0f, TARGET_ATTACKING, 0, 4);
                addAISpell(Util::getRandomUInt(0, 1) ? ANUBAR_ANIMATE_BONES_2 : ANUBAR_ANIMATE_BONES_1, 20.0f, TARGET_ATTACKING, 0, 40);
                break;
            default:
                break;
        }

        if (isWave())
            addAISpell(ANUBAR_TAUNT, 15.0f, TARGET_ATTACKING, 0, 30);
    }

    void OnLoad() override
    {
        if (!isWave())
            return;

        // the door nearest to the spawn decides the way down
        float best = 0.0f;
        for (uint8_t i = 0; i < hadronox::SPAWN_COUNT; ++i)
        {
            const float dist = getCreature()->getDistanceSq(hadronox::outsideMoves[i]);
            if (i == 0 || dist < best)
            {
                best = dist;
                m_spawn = i;
            }
        }

        // no path for the first step, the web wall is in the way
        movePoint(hadronox::MOVE_OUTSIDE, hadronox::outsideMoves[m_spawn], false);
    }

    void OnReachWP(uint32_t /*type*/, uint32_t id) override
    {
        switch (id)
        {
            case hadronox::MOVE_OUTSIDE:
                movePoint(hadronox::MOVE_DOWNSTAIRS, hadronox::downstairsMoves[m_spawn]);
                break;
            case hadronox::MOVE_DOWNSTAIRS:
                if (hadronox::downstairsMoves2[m_spawn].x > 0.0f)
                {
                    movePoint(hadronox::MOVE_DOWNSTAIRS_2, hadronox::downstairsMoves2[m_spawn]);
                    break;
                }
                attackHadronox(false);
                break;
            case hadronox::MOVE_DOWNSTAIRS_2:
                attackHadronox(false);
                break;
            case hadronox::MOVE_HADRONOX:
                attackHadronox(true);
                break;
            default:
                break;
        }
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        if (isWave())
            despawn(1000, 0);
    }

    void OnDied(Unit* /*_killer*/) override
    {
        // leech poison feeds Hadronox when its victim dies
        if (getCreature()->getAuraWithId(HADRONOX_LEECHPOISON) == nullptr && getCreature()->getAuraWithId(HADRONOX_LEECHPOISON_HC) == nullptr)
            return;

        if (Creature* boss = getInstanceScript() != nullptr ? getInstanceScript()->getCreatureFromData(hadronox::DATA_HADRONOX) : nullptr)
            if (boss->isAlive())
                boss->castSpell(boss, HADRONOX_LEECHPOISON_HEAL, true);
    }

private:
    bool isWave()
    {
        return getCreature()->isSummon() && (getCreature()->getEntry() == CN_ANUBAR_CHAMPION || getCreature()->getEntry() == CN_ANUBAR_CRYPT_FIEND || getCreature()->getEntry() == CN_ANUBAR_NECROMANCER);
    }

    void attackHadronox(bool direct)
    {
        Creature* boss = getInstanceScript() != nullptr ? getInstanceScript()->getCreatureFromData(hadronox::DATA_HADRONOX) : nullptr;
        if (boss == nullptr || !boss->isAlive())
        {
            despawn(1000, 0);
            return;
        }

        if (!direct && boss->GetPositionZ() < hadronox::LOWER_LEVEL_Z)
        {
            movePoint(hadronox::MOVE_HADRONOX, hadronox::steps[2]);
            return;
        }

        getCreature()->getAIInterface()->onHostileAction(boss);
    }

    uint8_t m_spawn = 0;
};

// the crusher packs start the event and move Hadronox up when they die
class AnubarCrusherAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AnubarCrusherAI(c); }
    explicit AnubarCrusherAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(CRUSHER_SMASH, 30.0f, TARGET_ATTACKING, 0, 17);
    }

    void InitOrReset() override
    {
        m_frenzied = false;
    }

    void SetCreatureData(uint32_t type, uint32_t data) override
    {
        if (type == hadronox::DATA_PACK_ID)
            m_pack = data;
    }

    void OnCombatStart(Unit* /*_target*/) override
    {
        if (m_pack != 1)
            return;

        if (Creature* boss = getInstanceScript() != nullptr ? getInstanceScript()->getCreatureFromData(hadronox::DATA_HADRONOX) : nullptr)
            if (boss->GetScript() != nullptr && boss->GetScript()->GetCreatureData(hadronox::DATA_ENTERED_COMBAT) == 0)
                boss->GetScript()->DoAction(hadronox::ACTION_CRUSHER_ENGAGED);
    }

    void OnDamageTaken(Unit* /*_attacker*/, uint32_t amount) override
    {
        if (m_frenzied || getCreature()->getHealth() > amount + getCreature()->getMaxHealth() / 4)
            return;

        m_frenzied = true;
        castSpellOnSelf(CRUSHER_FRENZY, true);
    }

    void OnDied(Unit* /*_killer*/) override
    {
        if (Creature* boss = getInstanceScript() != nullptr ? getInstanceScript()->getCreatureFromData(hadronox::DATA_HADRONOX) : nullptr)
            if (boss->GetScript() != nullptr)
                boss->GetScript()->DoAction(hadronox::ACTION_HADRONOX_MOVE);
    }

private:
    uint32_t m_pack = 1;
    bool m_frenzied = false;
};

// Watcher Gashra
class GashraAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GashraAI(c); }
    explicit GashraAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(GASHRA_WEBWRAP, 22.0f, TARGET_RANDOM_SINGLE, 0, 35);
        addAISpell(GASHRA_INFECTEDBITE, 35.0f, TARGET_ATTACKING, 0, 12);
    }
};

// Watcher Narjil
class NarjilAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NarjilAI(c); }
    explicit NarjilAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(NARJIL_WEBWRAP, 22.0f, TARGET_RANDOM_SINGLE, 0, 35);
        addAISpell(NARJIL_INFECTEDBITE, 35.0f, TARGET_ATTACKING, 0, 12);
        addAISpell(NARJIL_BLINDINGWEBS, 16.0f, TARGET_ATTACKING, 0, 9);
    }
};

// Watcher Silthik
class SilthikAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SilthikAI(c); }
    explicit SilthikAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(NARJIL_WEBWRAP, 22.0f, TARGET_RANDOM_SINGLE, 0, 35);
        addAISpell(NARJIL_INFECTEDBITE, 35.0f, TARGET_ATTACKING, 0, 12);
        addAISpell(SILTHIK_POISONSPRAY, 30.0f, TARGET_RANDOM_SINGLE, 0, 15);
    }
};

// Anub'ar Shadowcaster (anub shadowcaster)
class AnubShadowcasterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AnubShadowcasterAI(c); }
    explicit AnubShadowcasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHADOWCASTER_SHADOWBOLT, 36.0f, TARGET_RANDOM_SINGLE, 0, 8);
        addAISpell(SHADOWCASTER_SHADOW_NOVA, 22.0f, TARGET_SELF, 0, 15);
    }
};

void SetupAzjolNerub(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_AZJOL_NERUB, &AzjolNerubInstanceScript::Create);

    // Bosses
    mgr->register_creature_script(BOSS_KRIKTHIR, &KrikthirAI::Create);
    mgr->register_creature_script(BOSS_HADRONOX, &HadronoxAI::Create);

    // Hadronox event
    mgr->register_creature_script(CN_ANUBAR_CRUSHER, &AnubarCrusherAI::Create);
    mgr->register_creature_script(CN_ANUBAR_CHAMPION, &HadronoxFoeAI::Create);
    mgr->register_creature_script(CN_ANUBAR_CRYPT_FIEND, &HadronoxFoeAI::Create);
    mgr->register_creature_script(CN_ANUBAR_NECROMANCER, &HadronoxFoeAI::Create);
    mgr->register_creature_script(CN_CRUSHER_CHAMPION, &HadronoxFoeAI::Create);
    mgr->register_creature_script(CN_CRUSHER_CRYPT_FIEND, &HadronoxFoeAI::Create);
    mgr->register_creature_script(CN_CRUSHER_NECROMANCER, &HadronoxFoeAI::Create);

    // watchers
    mgr->register_creature_script(CN_GASHRA, &GashraAI::Create);
    mgr->register_creature_script(CN_NARJIL, &NarjilAI::Create);
    mgr->register_creature_script(CN_SILTHIK, &SilthikAI::Create);
}
