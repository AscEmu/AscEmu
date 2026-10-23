/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_AhnKahetTheOldKingdom.h"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class AhnKahetTheOldKingdomInstanceScript : public InstanceScript
{
public:
    explicit AhnKahetTheOldKingdomInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new AhnKahetTheOldKingdomInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Herald Volazj
namespace volazj
{
    enum Spells : uint32_t
    {
        SPELL_MIND_FLAY             = 57941,
        SPELL_MIND_FLAY_H           = 59974,
        SPELL_SHADOW_BOLT_VOLLEY    = 57942,
        SPELL_SHADOW_BOLT_VOLLEY_H  = 59975,
        SPELL_SHIVER                = 57949,
        SPELL_SHIVER_H              = 59978,
        SPELL_INSANITY_VISUAL       = 57561,
        SPELL_TWISTED_VISAGE_VISUAL = 57551
    };

    // npc_script_text of entry 29311
    enum Texts : uint32_t
    {
        SAY_AGGRO       = 0,
        WHISPER_AGGRO   = 1,
        SAY_INSANITY    = 2,
        SAY_SLAY_1      = 3,
        SAY_SLAY_2      = 4,
        SAY_SLAY_3      = 5,
        WHISPER_DEATH   = 6,
        SAY_DEATH       = 7
    };

    constexpr uint32_t NPC_TWISTED_VISAGE = 30625;
    constexpr uint32_t DATA_VISAGE_CLASS = 1;

    // one phase per player, the boss stays in all of them
    constexpr uint8_t MAX_INSANITY_PLAYERS = 5;
    constexpr uint32_t insanityPhase(uint8_t index) { return 16u << index; }
    constexpr uint32_t ALL_INSANITY_PHASES = 1 | 16 | 32 | 64 | 128 | 256;
}

// the twisted visage fights with two spells of the copied class
class TwistedVisageAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature) { return new TwistedVisageAI(pCreature); }
    explicit TwistedVisageAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        castSpellOnSelf(volazj::SPELL_TWISTED_VISAGE_VISUAL, true);
    }

    void SetCreatureData(uint32_t type, uint32_t data) override
    {
        if (type != volazj::DATA_VISAGE_CLASS)
            return;

        switch (data)
        {
            case WARRIOR:       addClassSpells(57789, 57832); break;    // mortal strike, thunder clap
            case PALADIN:       addClassSpells(57774, 57798); break;    // judgement of light, consecration
            case HUNTER:        addClassSpells(57589, 57635); break;    // shoot, disengage
            case ROGUE:         addClassSpells(57640, 57641); break;    // sinister strike, eviscerate
            case PRIEST:        addClassSpells(57778, 57779); break;    // shadow word pain, mind flay
            case DEATHKNIGHT:   addClassSpells(57599, 57602); break;    // plague strike, death grip
            case SHAMAN:        addClassSpells(57783, 57781); break;    // earth shock, lightning bolt
            case MAGE:          addClassSpells(57628, 57629); break;    // fireball, frost nova
            case WARLOCK:       addClassSpells(57644, 57645); break;    // shadow bolt, corruption
            case DRUID:         addClassSpells(57648, 57647); break;    // wrath, moonfire
            default:
                break;
        }
    }

private:
    void addClassSpells(uint32_t first, uint32_t second)
    {
        addAISpell(first, 40.0f, TARGET_ATTACKING, 0, 6);
        addAISpell(second, 30.0f, TARGET_ATTACKING, 0, 10);
    }
};

class HeraldVolazjAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature) { return new HeraldVolazjAI(pCreature); }
    explicit HeraldVolazjAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addEmoteForEventByIndex(Event_OnCombatStart, volazj::SAY_AGGRO);
        addEmoteForEventByIndex(Event_OnTargetDied, volazj::SAY_SLAY_1);
        addEmoteForEventByIndex(Event_OnTargetDied, volazj::SAY_SLAY_2);
        addEmoteForEventByIndex(Event_OnTargetDied, volazj::SAY_SLAY_3);
        addEmoteForEventByIndex(Event_OnDied, volazj::SAY_DEATH);

        addAISpell(isHeroic() ? volazj::SPELL_MIND_FLAY_H : volazj::SPELL_MIND_FLAY, 25.0f, TARGET_ATTACKING, 3, 20);
        addAISpell(isHeroic() ? volazj::SPELL_SHADOW_BOLT_VOLLEY_H : volazj::SPELL_SHADOW_BOLT_VOLLEY, 30.0f, TARGET_ATTACKING, 0, 5);
        addAISpell(isHeroic() ? volazj::SPELL_SHIVER_H : volazj::SPELL_SHIVER, 25.0f, TARGET_RANDOM_SINGLE, 0, 15);
    }

    void InitOrReset() override
    {
        m_insanityCount = 0;
        m_insanityActive = false;
        getCreature()->setPhase(PHASE_SET, volazj::ALL_INSANITY_PHASES);
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        endInsanity();
        summons.despawnAll();
    }

    void OnDied(Unit* /*_killer*/) override
    {
        endInsanity();
        summons.despawnAll();
    }

    void OnDamageTaken(Unit* /*_attacker*/, uint32_t amount) override
    {
        if (m_insanityActive || m_insanityCount >= 2)
            return;

        const uint32_t health = getCreature()->getHealth();
        const uint32_t maxHealth = getCreature()->getMaxHealth();
        if (maxHealth == 0)
            return;

        const uint32_t before = 100 * health / maxHealth;
        const uint32_t after = amount >= health ? 0 : 100 * (health - amount) / maxHealth;
        if ((before >= 66 && after < 66) || (before >= 33 && after < 33))
            startInsanity();
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
    }

    void OnSummonDies(Creature* summon, Unit* /*_killer*/) override
    {
        summons.despawn(summon);

        // the players of a cleared phase join the next phase that still has visages
        const uint32_t phase = summon->GetPhase();
        uint32_t nextPhase = 0;
        for (const auto guid : summons)
        {
            if (Creature* visage = getCreature()->getWorldMapCreature(guid))
            {
                if (visage->GetPhase() == phase)
                    return;
                nextPhase = visage->GetPhase();
            }
        }

        for (const auto& entry : getCreature()->getWorldMap()->getPlayers())
            if (entry.second->GetPhase() == phase)
                entry.second->setPhase(PHASE_SET, nextPhase != 0 ? nextPhase : 1);

        if (summons.empty())
            endInsanity();
    }

private:
    void startInsanity()
    {
        m_insanityActive = true;
        ++m_insanityCount;

        sendDBChatMessageByIndex(volazj::SAY_INSANITY);
        castSpellOnSelf(volazj::SPELL_INSANITY_VISUAL, true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setControlled(true, UNIT_STATE_STUNNED);

        std::vector<Player*> players;
        for (const auto& entry : getCreature()->getWorldMap()->getPlayers())
            if (entry.second->isAlive() && players.size() < volazj::MAX_INSANITY_PLAYERS)
                players.push_back(entry.second);

        // every player gets a phase of their own with a copy of the whole group
        for (uint8_t i = 0; i < players.size(); ++i)
        {
            const uint32_t phase = volazj::insanityPhase(i);
            players[i]->setPhase(PHASE_SET, phase);

            for (Player* copied : players)
            {
                Creature* visage = summonCreature(volazj::NPC_TWISTED_VISAGE, getCreature()->GetPosition(), CORPSE_DESPAWN);
                if (visage == nullptr)
                    continue;

                visage->setPhase(PHASE_SET, phase);
                visage->setDisplayId(copied->getDisplayId());
                visage->setNativeDisplayId(copied->getDisplayId());
                if (visage->GetScript())
                    visage->GetScript()->SetCreatureData(volazj::DATA_VISAGE_CLASS, copied->getClass());

                visage->getAIInterface()->onHostileAction(players[i]);
            }
        }
    }

    void endInsanity()
    {
        if (!m_insanityActive)
            return;

        m_insanityActive = false;
        _removeAura(volazj::SPELL_INSANITY_VISUAL);
        getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setControlled(false, UNIT_STATE_STUNNED);

        for (const auto& entry : getCreature()->getWorldMap()->getPlayers())
            entry.second->setPhase(PHASE_SET, 1);
    }

    uint8_t m_insanityCount = 0;
    bool m_insanityActive = false;
};

void SetupAhnKahetTheOldKingdom(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_AHN_KAHET, &AhnKahetTheOldKingdomInstanceScript::Create);
    mgr->register_creature_script(29311, &HeraldVolazjAI::Create);
    mgr->register_creature_script(volazj::NPC_TWISTED_VISAGE, &TwistedVisageAI::Create);
}
