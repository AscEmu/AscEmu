/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Instance_AuchenaiCrypts.h"
#include "Objects/Faction.h"

// Shirrak the Dead WatcherAI
// Hmmm... next boss without sounds?
class SHIRRAKTHEDEADWATCHERAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SHIRRAKTHEDEADWATCHERAI);
        SHIRRAKTHEDEADWATCHERAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

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

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// Avatar of the MartyredAI
class AvatarOfTheMartyredAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AvatarOfTheMartyredAI);
        AvatarOfTheMartyredAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto sunderArmor = addAISpell(SUNDER_ARMOR, 15.0f, TARGET_ATTACKING, 0, 10, false, true);
            sunderArmor->setAttackStopTimer(1000);

            auto mortalStrike = addAISpell(MORTAL_STRIKE, 10.0f, TARGET_ATTACKING, 0, 10, false, true);
            mortalStrike->setAttackStopTimer(1000);

            phaseIn = addAISpell(PHASE_IN, 0.0f, TARGET_ATTACKING, 0, 10, false, true);
            phaseIn->setAttackStopTimer(1000);

            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            getCreature()->m_noRespawn = true;

            Appear = true;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void AIUpdate() override
        {
            if (Appear)
            {
                getCreature()->CastSpell(getCreature(), phaseIn->mSpellInfo, phaseIn->mIsTriggered);
                getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);

                Appear = false;
            }
        }

    protected:

        bool Appear;
        CreatureAISpells* phaseIn;
};


// Exarch MaladaarAI
class EXARCHMALADAARAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(EXARCHMALADAARAI);
        EXARCHMALADAARAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

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

            // new
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

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            Avatar = false;
        }

        void AIUpdate() override
        {
            // case for scriptphase
            if (getCreature()->GetHealthPct() <= 25 && !Avatar && !getCreature()->IsStunned())
            {
                sendDBChatMessage(SAY_MALADAAR_07);

                getCreature()->setAttackTimer(3500, false);
                getCreature()->GetAIInterface()->StopMovement(2000);

                getCreature()->CastSpell(getCreature(), summonAvatar->mSpellInfo, summonAvatar->mIsTriggered);
                Avatar = true;
            }
        }

    protected:

        bool Avatar;
        CreatureAISpells* summonAvatar;
};

void SetupAuchenaiCrypts(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_SHIRRAK_THE_DEAD_WATCHER, &SHIRRAKTHEDEADWATCHERAI::Create);
    mgr->register_creature_script(CN_AVATAR_OF_THE_MARTYRED, &AvatarOfTheMartyredAI::Create);
    mgr->register_creature_script(CN_EXARCH_MALADAAR, &EXARCHMALADAARAI::Create);
}
