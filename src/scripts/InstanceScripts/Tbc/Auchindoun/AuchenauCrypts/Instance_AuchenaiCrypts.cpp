/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_AuchenaiCrypts.h"

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

class AuchenaiCryptsInstanceScript : public InstanceScript
{
public:
    explicit AuchenaiCryptsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new AuchenaiCryptsInstanceScript(pMapMgr); }
};

// Shirrak the Dead WatcherAI
// Hmmm... next boss without sounds?
class ShirrakTheDeadWatcherAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShirrakTheDeadWatcherAI(c); }
    explicit ShirrakTheDeadWatcherAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto inhibitMagic = addAISpell(INHIBIT_MAGIC, 7.0f, TARGET_SELF, 0, 10, false, true);
        inhibitMagic->setAttackStopTimer(1000);

        auto bite = addAISpell(CARNIVOROUS_BITE, 15.0f, TARGET_ATTACKING, 0, 10, false, true);
        bite->setAttackStopTimer(1000);

        auto focusFire = addAISpell(FOCUS_FIRE, 8.0f, TARGET_RANDOM_DESTINATION, 0, 15);
        focusFire->setMinMaxDistance(0.0f, 40.0f);
        focusFire->setAttackStopTimer(1000);

        auto attractMagic = addAISpell(ATTRACT_MAGIC, 10.0f, TARGET_VARIOUS, 0, 15, false, true);
        attractMagic->setAttackStopTimer(1000);
    }
};

// Avatar of the MartyredAI
class AvatarOfTheMartyredAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AvatarOfTheMartyredAI(c); }
    explicit AvatarOfTheMartyredAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto sunderArmor = addAISpell(SUNDER_ARMOR, 15.0f, TARGET_ATTACKING, 0, 10, false, true);
        sunderArmor->setAttackStopTimer(1000);

        auto mortalStrike = addAISpell(MORTAL_STRIKE, 10.0f, TARGET_ATTACKING, 0, 10, false, true);
        mortalStrike->setAttackStopTimer(1000);

        phaseIn = addAISpell(PHASE_IN, 0.0f, TARGET_ATTACKING, 0, 10, false, true);
        phaseIn->setAttackStopTimer(1000);

        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->m_noRespawn = true;

        Appear = true;
    }

    void AIUpdate() override
    {
        if (Appear)
        {
            getCreature()->castSpell(getCreature(), phaseIn->mSpellInfo, phaseIn->mIsTriggered);
            getCreature()->getAIInterface()->setAllowedToEnterCombat(true);

            Appear = false;
        }
    }

protected:
    bool Appear;
    CreatureAISpells* phaseIn;
};

// Exarch MaladaarAI
class EXARCH_MALADAAR_AI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EXARCH_MALADAAR_AI(c); }
    explicit EXARCH_MALADAAR_AI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto souldScream = addAISpell(SOUL_SCREAM, 10.0f, TARGET_VARIOUS, 0, 15, false, true);
        souldScream->addEmote("Let your mind be clouded.", CHAT_MSG_MONSTER_YELL, 10510); // dunno for sure if it should be here, but still gives better effect of fight :)
        souldScream->setAttackStopTimer(1000);

        auto ribbonOfSoul = addAISpell(RIBBON_OF_SOULS, 15.0f, TARGET_RANDOM_SINGLE, 0, 15);
        ribbonOfSoul->addEmote("Stare into the darkness of your soul!", CHAT_MSG_MONSTER_YELL, 10511); // not sure if it's really "stand"
        ribbonOfSoul->setMinMaxDistance(0.0f, 40.0f);
        ribbonOfSoul->setAttackStopTimer(2000);

        auto stolenSoul = addAISpell(STOLEN_SOUL, 7.0f, TARGET_RANDOM_SINGLE, 0, 15);
        stolenSoul->setMinMaxDistance(0.0f, 40.0f);
        stolenSoul->setAttackStopTimer(1000);

        summonAvatar = addAISpell(SUMMON_AVATAR, 0.0f, TARGET_SELF);
        summonAvatar->setAttackStopTimer(1000);

        Avatar = false;

        addEmoteForEvent(Event_OnCombatStart, SAY_MALADAAR_01);
        addEmoteForEvent(Event_OnCombatStart, SAY_MALADAAR_02);
        addEmoteForEvent(Event_OnCombatStart, SAY_MALADAAR_03);
        addEmoteForEvent(Event_OnTargetDied, SAY_MALADAAR_04);
        addEmoteForEvent(Event_OnTargetDied, SAY_MALADAAR_05);
        addEmoteForEvent(Event_OnDied, SAY_MALADAAR_06);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        Avatar = false;

        RegisterAIUpdateEvent(getCreature()->getBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        Avatar = false;
    }

    void AIUpdate() override
    {
        // case for scriptphase
        if (getCreature()->getHealthPct() <= 25 && !Avatar && !getCreature()->isStunned())
        {
            sendDBChatMessage(SAY_MALADAAR_07);

            getCreature()->setAttackTimer(MELEE, 3500);
            getCreature()->pauseMovement(2000);

            getCreature()->castSpell(getCreature(), summonAvatar->mSpellInfo, summonAvatar->mIsTriggered);
            Avatar = true;
        }
    }

protected:
    bool Avatar;
    CreatureAISpells* summonAvatar;
};

void SetupAuchenaiCrypts(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_AUCHENAI_CRYPTS, &AuchenaiCryptsInstanceScript::Create);

    mgr->register_creature_script(CN_SHIRRAK_THE_DEAD_WATCHER, &ShirrakTheDeadWatcherAI::Create);
    mgr->register_creature_script(CN_AVATAR_OF_THE_MARTYRED, &AvatarOfTheMartyredAI::Create);
    mgr->register_creature_script(CN_EXARCH_MALADAAR, &EXARCH_MALADAAR_AI::Create);
}
