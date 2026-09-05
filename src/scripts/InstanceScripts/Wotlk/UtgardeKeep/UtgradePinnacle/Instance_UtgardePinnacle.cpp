/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_UtgardePinnacle.h"

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Utilities/CommonTime.hpp"

enum PinnacleData
{
    DATA_SVALA = 0
};

class UtgardePinnacleInstanceScript : public InstanceScript
{
public:
    explicit UtgardePinnacleInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(1);
    }
    static InstanceScript* Create(WorldMap* pMapMgr) { return new UtgardePinnacleInstanceScript(pMapMgr); }
};

// The human "Svala" seen praying to the Image of Arthas before the fight - on completion she
// despawns and the real boss, Svala Sorrowgrave, is spawned in her place.
class HumanSvalaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HumanSvalaAI(c); }
    explicit HumanSvalaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mInstance = getInstanceScript();
        setReactState(REACT_PASSIVE);
    }

    void InitOrReset() override
    {
        if (mInstance != nullptr && mInstance->getBossState(DATA_SVALA) == EncounterStates::NotStarted)
        {
            mInstance->setBossState(DATA_SVALA, EncounterStates::InProgress);
            addAIFunction(&HumanSvalaAI::introSummonArthas, DoOnceScheduler(3s));
        }
    }

protected:
    InstanceScript* mInstance;
    Creature* mArthasImage = nullptr;

    void introSummonArthas(CreatureAIFunc)
    {
        mArthasImage = summonCreature(CN_IMAGE_OF_ARTHAS, getCreature()->GetPositionX(), getCreature()->GetPositionY(),
            getCreature()->GetPositionZ(), getCreature()->GetOrientation(), TIMED_DESPAWN, 55000);

        addAIFunction(&HumanSvalaAI::introSvalaSpeaks, DoOnceScheduler(9s));
    }

    void introSvalaSpeaks(CreatureAIFunc)
    {
        sendDBChatMessage(SAY_SVALA_INTRO_00);
        addAIFunction(&HumanSvalaAI::introArthasReplies, DoOnceScheduler(10s));
    }

    void introArthasReplies(CreatureAIFunc)
    {
        if (mArthasImage != nullptr)
            mArthasImage->SendScriptTextChatMessage(SAY_ARTHAS_INTRO_00);

        castSpellOnSelf(SPELL_TRANSFORMING_CHANNEL, true);
        addAIFunction(&HumanSvalaAI::introFloat, DoOnceScheduler(7s));
    }

    void introFloat(CreatureAIFunc)
    {
        castSpellOnSelf(SPELL_TRANSFORMING_FLOATING, true);
        addAIFunction(&HumanSvalaAI::introTransform, DoOnceScheduler(4s));
    }

    void introTransform(CreatureAIFunc)
    {
        castSpellOnSelf(SPELL_TRANSFORMING, true);

        if (mInstance != nullptr)
            mInstance->spawnCreature(CN_SVALA_SORROWGRAVE, SVALA_SORROWGRAVE_SPAWN_X, SVALA_SORROWGRAVE_SPAWN_Y,
                SVALA_SORROWGRAVE_SPAWN_Z, SVALA_SORROWGRAVE_SPAWN_O);

        getCreature()->despawn(0);
    }
};

class SvalaSorrowgraveAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SvalaSorrowgraveAI(c); }
    explicit SvalaSorrowgraveAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mInstance = getInstanceScript();
        setReactState(REACT_PASSIVE);

        addEmoteForEvent(Event_OnTargetDied, SAY_SVALA_SORROWGRAVE_KILL_01);
        addEmoteForEvent(Event_OnTargetDied, SAY_SVALA_SORROWGRAVE_KILL_02);
        addEmoteForEvent(Event_OnTargetDied, SAY_SVALA_SORROWGRAVE_KILL_03);
        addEmoteForEvent(Event_OnDied, SAY_SVALA_SORROWGRAVE_DEATH);
    }

    void InitOrReset() override
    {
        if (mInstance != nullptr && mInstance->getBossState(DATA_SVALA) != EncounterStates::Performed)
            addAIFunction(&SvalaSorrowgraveAI::finishIntroAndEngage, DoOnceScheduler(11s));
    }

    void OnCombatStart(Unit* /*_target*/) override
    {
        sendDBChatMessage(SAY_SVALA_SORROWGRAVE_AGGRO);

        const bool heroic = isHeroic();

        DoLoopScheduler mSinisterStrikeArgs;
        mSinisterStrikeArgs.setInitialCooldown(7s);
        addAISpell(SpellDesc(heroic ? H_SINISTER_STRIKE : N_SINISTER_STRIKE, FilterArgs(TargetFilter_Current)), mSinisterStrikeArgs);

        DoLoopScheduler mCallFlamesArgs;
        mCallFlamesArgs.setInitialCooldown(10s);
        addAISpell(SpellDesc(CALL_FLAMES, FilterArgs(TargetFilter_Player)), mCallFlamesArgs);
    }

protected:
    InstanceScript* mInstance;

    void finishIntroAndEngage(CreatureAIFunc)
    {
        sendDBChatMessage(SAY_SVALA_INTRO_01);
        addAIFunction(&SvalaSorrowgraveAI::finishIntroArthas, DoOnceScheduler(10s));
    }

    void finishIntroArthas(CreatureAIFunc)
    {
        if (mInstance != nullptr)
        {
            for (auto* arthasImage : mInstance->getCreatureSetForEntry(CN_IMAGE_OF_ARTHAS))
                arthasImage->SendScriptTextChatMessage(SAY_ARTHAS_INTRO_01);
        }

        addAIFunction(&SvalaSorrowgraveAI::finishIntroEngage, DoOnceScheduler(13s));
    }

    void finishIntroEngage(CreatureAIFunc)
    {
        sendDBChatMessage(SAY_SVALA_INTRO_02);

        if (mInstance != nullptr)
            mInstance->setBossState(DATA_SVALA, EncounterStates::Performed);

        setReactState(REACT_AGGRESSIVE);

        if (Player* target = getNearestPlayer())
            getCreature()->getAIInterface()->onHostileAction(target);
    }
};

class GortokPalehoofAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GortokPalehoofAI(c); }
    explicit GortokPalehoofAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addEmoteForEvent(Event_OnCombatStart, SAY_GROTOK_PALEHOOF_01);
        addEmoteForEvent(Event_OnTargetDied, SAY_GROTOK_PALEHOOF_02);
        addEmoteForEvent(Event_OnTargetDied, SAY_GROTOK_PALEHOOF_03);
    }
};

class SkadiTheRuthlessAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SkadiTheRuthlessAI(c); }
    explicit SkadiTheRuthlessAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addEmoteForEvent(Event_OnCombatStart, SAY_SKADI_RUTHLESS_START);
        addEmoteForEvent(Event_OnTargetDied, SAY_SKADI_RUTHLESS_KILL_01);
        addEmoteForEvent(Event_OnTargetDied, SAY_SKADI_RUTHLESS_KILL_02);
        addEmoteForEvent(Event_OnDied, SAY_SKADI_RUTHLESS_DIE);
    }
};

class KingYmironAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KingYmironAI(c); }
    explicit KingYmironAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addEmoteForEvent(Event_OnCombatStart, SAY_KING_YMIRON_START);
        addEmoteForEvent(Event_OnTargetDied, SAY_KING_YMIRON_KILL_01);
        addEmoteForEvent(Event_OnTargetDied, SAY_KING_YMIRON_KILL_02);
        addEmoteForEvent(Event_OnTargetDied, SAY_KING_YMIRON_KILL_03);
        addEmoteForEvent(Event_OnTargetDied, SAY_KING_YMIRON_KILL_04);
        addEmoteForEvent(Event_OnDied, SAY_KING_YMIRON_DIE);
    }
};

void SetupUtgardePinnacle(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_UTGARDE_PINNACLE, &UtgardePinnacleInstanceScript::Create);

    //Bosses
    mgr->register_creature_script(CN_SVALA_HUMAN, &HumanSvalaAI::Create);
    mgr->register_creature_script(CN_SVALA_SORROWGRAVE, &SvalaSorrowgraveAI::Create);
    mgr->register_creature_script(CN_GORTOK_PALEHOOF, &GortokPalehoofAI::Create);
    mgr->register_creature_script(CN_SKADI_THE_RUTHLESS, &SkadiTheRuthlessAI::Create);
    mgr->register_creature_script(CN_KING_YMIRON, &KingYmironAI::Create);
}
