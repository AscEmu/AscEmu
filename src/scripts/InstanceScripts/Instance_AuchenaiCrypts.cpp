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
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        SHIRRAKTHEDEADWATCHERAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(INHIBIT_MAGIC);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].perctrigger = 7.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].cooldown = 10;

            spells[1].info = sSpellCustomizations.GetSpellInfo(CARNIVOROUS_BITE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 15.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].cooldown = 10;

            spells[2].info = sSpellCustomizations.GetSpellInfo(FOCUS_FIRE);
            spells[2].targettype = TARGET_RANDOM_DESTINATION;   // changed from attack as it wasn't working anyway
            spells[2].instant = false;
            spells[2].perctrigger = 8.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].cooldown = 15;
            spells[2].mindist2cast = 0.0f;
            spells[2].maxdist2cast = 40.0f;

            spells[3].info = sSpellCustomizations.GetSpellInfo(ATTRACT_MAGIC);
            spells[3].targettype = TARGET_VARIOUS;
            spells[3].instant = true;
            spells[3].perctrigger = 10.0f;
            spells[3].attackstoptimer = 1000;
            spells[3].cooldown = 15;
        }

        void OnCombatStart(Unit* mTarget) override
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        if (!spells[i].instant)
                            getCreature()->GetAIInterface()->StopMovement(1);

                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;         // From M4ksiu - Big THX to Capt
                for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(getCreature(), (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(getCreature(), (*itr)) && (*itr) != getCreature())) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && getCreature()->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (getCreature()->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(getCreature(), RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (getCreature()->GetHealthPct() >= minhp2cast && getCreature()->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(getCreature());

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        getCreature()->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        getCreature()->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:

        uint8 nrspells;
};


// Avatar of the MartyredAI
class AvatarOfTheMartyredAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AvatarOfTheMartyredAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        AvatarOfTheMartyredAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SUNDER_ARMOR);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 15.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].cooldown = 10;

            spells[1].info = sSpellCustomizations.GetSpellInfo(MORTAL_STRIKE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 10.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].cooldown = 10;

            spells[2].info = sSpellCustomizations.GetSpellInfo(PHASE_IN);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].cooldown = -1;

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));

            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            getCreature()->m_noRespawn = true;

            Appear = true;
        }

        void OnCombatStart(Unit* mTarget) override
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            //RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            if (Appear)
            {
                getCreature()->CastSpell(getCreature(), spells[2].info, spells[2].instant);
                getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);

                Appear = false;
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        bool Appear;
        uint8 nrspells;
};


// Exarch MaladaarAI
class EXARCHMALADAARAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(EXARCHMALADAARAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        EXARCHMALADAARAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SOUL_SCREAM);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].cooldown = 15;
            spells[0].soundid = 10510;
            spells[0].speech = "Let your mind be clouded.";    // dunno for sure if it should be here, but still gives better effect of fight :)

            spells[1].info = sSpellCustomizations.GetSpellInfo(RIBBON_OF_SOULS);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = false;
            spells[1].perctrigger = 15.0f;
            spells[1].attackstoptimer = 2000;
            spells[1].cooldown = 15;
            spells[1].mindist2cast = 0.0f;
            spells[1].maxdist2cast = 40.0f;
            spells[1].soundid = 10511;
            spells[1].speech = "Stare into the darkness of your soul!"; // not sure if it's really "stand"

            spells[2].info = sSpellCustomizations.GetSpellInfo(STOLEN_SOUL);
            spells[2].targettype = TARGET_RANDOM_SINGLE;
            spells[2].instant = false;
            spells[2].perctrigger = 7.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].cooldown = 15;
            spells[2].mindist2cast = 0.0f;
            spells[2].maxdist2cast = 40.0f;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SUMMON_AVATAR);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = false;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;
            spells[3].cooldown = -1;

            Avatar = false;

            // new
            addEmoteForEvent(Event_OnCombatStart, SAY_MALADAAR_01);
            addEmoteForEvent(Event_OnCombatStart, SAY_MALADAAR_02);
            addEmoteForEvent(Event_OnCombatStart, SAY_MALADAAR_03);
            addEmoteForEvent(Event_OnTargetDied, SAY_MALADAAR_04);
            addEmoteForEvent(Event_OnTargetDied, SAY_MALADAAR_05);
            addEmoteForEvent(Event_OnDied, SAY_MALADAAR_06);
        }

        void OnCombatStart(Unit* mTarget) override
        {
            for (uint8 i = 0; i < 4; i++)
                spells[i].casttime = 0;

            Avatar = false;

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            RemoveAIUpdateEvent();

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

                getCreature()->CastSpell(getCreature(), spells[3].info, spells[3].instant);
                Avatar = true;
            }

            else
            {
                float val = RandomFloat(100.0f);
                SpellCast(val);
            }
        }

        void SpellCast(float val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        if (!spells[i].instant)
                            getCreature()->GetAIInterface()->StopMovement(1);

                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;
                for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(getCreature(), (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(getCreature(), (*itr)) && (*itr) != getCreature())) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && getCreature()->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (getCreature()->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(getCreature(), RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (getCreature()->GetHealthPct() >= minhp2cast && getCreature()->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(getCreature());

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        getCreature()->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        getCreature()->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:

        bool Avatar;
        uint8 nrspells;
};

void SetupAuchenaiCrypts(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_SHIRRAK_THE_DEAD_WATCHER, &SHIRRAKTHEDEADWATCHERAI::Create);
    mgr->register_creature_script(CN_AVATAR_OF_THE_MARTYRED, &AvatarOfTheMartyredAI::Create);
    mgr->register_creature_script(CN_EXARCH_MALADAAR, &EXARCHMALADAARAI::Create);
}
