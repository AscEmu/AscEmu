/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Movement/MovementDefines.h"
#include "Movement/MovementManager.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellScript.hpp"
#include "Utilities/Random.hpp"

#include <numbers>

// Quest 6661 Deeprun Rat Roundup: the flute turns a Deeprun Rat into an enthralled rat that follows the player
class MelodiousRapture : public SpellScript
{
    static constexpr uint32_t NPC_DEEPRUN_RAT = 13016;
    static constexpr uint32_t NPC_ENTHRALLED_DEEPRUN_RAT = 13017;
    static constexpr uint32_t SPELL_MELODIOUS_RAPTURE_VISUAL = 21051;
    static constexpr uint32_t QUEST_DEEPRUN_RAT_ROUNDUP = 6661;

public:
    SpellCastResult onCanCast(Spell* spell, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/) override
    {
        const auto target = spell->getUnitTarget();
        if (target == nullptr || target->getEntry() != NPC_DEEPRUN_RAT)
            return SPELL_FAILED_BAD_TARGETS;

        return SPELL_CAST_SUCCESS;
    }

    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t effIndex) override
    {
        if (effIndex != 0)
            return SpellScriptCheckDummy::DUMMY_NOT_HANDLED;

        const auto player = spell->getPlayerCaster();
        const auto target = spell->getUnitTarget();
        if (player == nullptr || target == nullptr || !target->isCreature())
            return SpellScriptCheckDummy::DUMMY_OK;

        const auto rat = static_cast<Creature*>(target);
        if (rat->getEntry() != NPC_DEEPRUN_RAT || !rat->updateEntry(NPC_ENTHRALLED_DEEPRUN_RAT))
            return SpellScriptCheckDummy::DUMMY_OK;

        rat->castSpell(rat, SPELL_MELODIOUS_RAPTURE_VISUAL, true);
        const float angle = Util::getRandomFloat(std::numbers::pi_v<float> * 0.8f, std::numbers::pi_v<float> * 1.2f);
        rat->getMovementManager()->moveFollow(player, Util::getRandomFloat(0.5f, 3.0f), ChaseAngle(angle));
        player->addQuestKill(QUEST_DEEPRUN_RAT_ROUNDUP, 0);
        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

void SetupDunMorogh(ScriptMgr* mgr)
{
    mgr->register_spell_script(21050, new MelodiousRapture);
}
