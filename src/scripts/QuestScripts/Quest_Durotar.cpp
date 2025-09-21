/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/QuestLogEntry.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"
#include "Utilities/Random.hpp"

#include <numbers>

class LazyPeon : public CreatureAIScript
{
    // Core Data
    static constexpr uint32_t  QUEST_LAZY_PEONS     = 5441;
    static constexpr uint32_t  GO_LUMBERPILE        = 175784;
    static constexpr uint32_t  SPELL_BUFF_SLEEP     = 17743;
    static constexpr uint32_t  SPELL_AWAKEN_PEON    = 19938;

    // Tuning
    static constexpr float SEARCH_RADIUS            = 30.0f;
    static constexpr float RANDOM_MIN_DIST          = 5.0f;
    static constexpr float RANDOM_MAX_DIST          = 15.0f;
    static constexpr float CHOP_OFFSET_X            = -1.0f;

    enum WaypointId : uint32_t
    {
        ToWoodpile = 1,
        Random = 2
    };

public:
    static CreatureAIScript* Create(Creature* creature) { return new LazyPeon(creature); }
    explicit LazyPeon(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void InitOrReset() override
    {
        getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
        // Hackfix for Updatemgr
        addAIFunction([this](CreatureAIFunc pThis) { startSleeping(); }, DoOnceScheduler(1s));
    }

    void OnReachWP(uint32_t /*type*/, uint32_t id) override
    {
        if (id == ToWoodpile)
        {
            // Woodpile Reached
            getCreature()->setEmoteState(EMOTE_STATE_WORK_CHOPWOOD);
            addAIFunction([this](CreatureAIFunc pThis) {handleMoveRandom(pThis); }, DoOnceScheduler(3min));
        }
        else if (id == Random)
        {
            // Random Position Reached go Back to Sleep
            startSleeping();
        }
    }

    void OnHitBySpell(uint32_t spellId, Unit* caster) override
    {
        if (!caster || !caster->isPlayer())
            return;

        if (spellId != SPELL_AWAKEN_PEON)
            return;

        removeAllFunctionsFromScheduler();
        handleWakeUp(caster);
    }

    void startSleeping()
    {
        castSpellOnSelf(SPELL_BUFF_SLEEP);
        // Start working after aura expires by itself
        addAIFunction([this](CreatureAIFunc PThis) { handleWakeUp(nullptr); }, DoOnceScheduler(2min));
    }

    void handleMoveRandom(CreatureAIFunc pThis)
    {
        getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);

        // Move to a random point near closest wood pile
        if (GameObject* Lumberpile = findNearestGameObject(GO_LUMBERPILE, SEARCH_RADIUS))
        {
            LocationVector randomPosition = Lumberpile->GetPosition();

            float distance  = Util::getRandomFloat(RANDOM_MIN_DIST, RANDOM_MAX_DIST);
            float angle     = Util::getRandomFloat(0.f, std::numbers::pi_v<float>);

            getCreature()->movePositionToFirstCollision(randomPosition, distance, angle);

            movePoint(Random, randomPosition);
        }
    }

    void handleWakeUp(Unit* caster)
    {
        // Remove Zzz aura
        _removeAura(SPELL_BUFF_SLEEP);

        // Caster is nullptr when peon wakes by itself
        if (caster != nullptr)
        {
            // Send chat Message
            sendDBChatMessageByIndex(0, caster);

            // Hackfix, clear AffectedUnits for Player. Quest Increment happens trough CastedSpell in QuestMgr.
            if (auto* questLog = caster->ToPlayer()->getQuestLogByQuestId(QUEST_LAZY_PEONS))
                questLog->clearAffectedUnits();
        }

        // Move to closest wood pile
        if (GameObject* Lumberpile = findNearestGameObject(GO_LUMBERPILE, SEARCH_RADIUS))
        {
            LocationVector dest
            {
                Lumberpile->GetPositionX() + CHOP_OFFSET_X,
                Lumberpile->GetPositionY(),
                Lumberpile->GetPositionZ()
            };
            const float orientation = faceFromTo(getCreature(), dest.x, dest.y, Lumberpile->GetPositionX(), Lumberpile->GetPositionY());

            movePoint(ToWoodpile, dest, true, orientation);
        }
    }

    static constexpr float toRad(float deg) noexcept
    {
        return deg * std::numbers::pi_v<float> / 180.0f;
    }

    static float faceFromTo(Creature* c, float fromX, float fromY, float toX, float toY)
    {
        if (!c) return 0.0f;
        const float deg = c->calcAngle(fromX, fromY, toX, toY);
        return deg * std::numbers::pi_v<float> / 180.0f;
    }
};

void SetupDurotar(ScriptMgr* mgr)
{
    mgr->register_creature_script(10556, &LazyPeon::Create);
}
