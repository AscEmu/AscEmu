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

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Instance_TheUnderbog.h"
#include "Objects/Faction.h"


///////////////////////////////////////////////////////////
// Boss AIs
///////////////////////////////////////////////////////////

// HungarfenAI
class HungarfenAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(HungarfenAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        HungarfenAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(UNDERBOG_MUSHROOM);    // need to have more fun with it?
            spells[0].targettype = TARGET_RANDOM_DESTINATION;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 0.0f;//100.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(FOUL_SPORES);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = false;
            spells[1].cooldown = 0;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            FourSpores = false;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            spells[0].casttime = 0;

            FourSpores = false;

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            FourSpores = false;
        }

        void AIUpdate() override
        {
            if (getCreature()->GetHealthPct() <= 20 && !FourSpores)
            {
                getCreature()->GetAIInterface()->StopMovement(11000);
                getCreature()->setAttackTimer(1200, false);

                getCreature()->CastSpell(getCreature(), spells[1].info, spells[1].instant);

                FourSpores = true;
            }

            else if (!getCreature()->getAuraWithId(FOUL_SPORES))
            {
                uint32 t = (uint32)time(NULL);
                if (t > spells[0].casttime)
                {
                    // Not yet added
                    //CastSpellOnRandomTarget(0, 0.0f, 40.0f, 0, 100);

                    spells[0].casttime = t + spells[0].cooldown;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 /*i*/, float mindist2cast, float maxdist2cast, int /*minhp2cast*/, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;
                for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                {
                    if (isHostile(getCreature(), (*itr)) && (*itr) != getCreature() && (*itr)->IsUnit())
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && getCreature()->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast)
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;
                ///\todo Spawning mushroom

                TargetTable.clear();
            }
        }

    protected:

        bool FourSpores;
        uint8 nrspells;
};

// Ghaz'anAI
class GhazanAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GhazanAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        GhazanAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;

            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(ACID_SPIT);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = 20;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(TAIL_SWEEP);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 25;
            spells[1].perctrigger = 7.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(ACID_BREATH);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = true;
            spells[2].cooldown = 15;
            spells[2].perctrigger = 10.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(ENRAGE);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = 0;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            Enraged = false;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            Enraged = false;

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            if (getCreature()->GetHealthPct() <= 20 && !Enraged && !getCreature()->isCastingNonMeleeSpell())
            {
                getCreature()->CastSpell(getCreature(), spells[3].info, spells[3].instant);

                Enraged = true;
                return;
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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

                        if (spells[i].speech != "")
                        {
                            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            getCreature()->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        bool Enraged;
        uint8 nrspells;
};

// ClawAI
class ClawAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ClawAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        ClawAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(MAUL);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 15.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(CL_ECHOING_ROAR);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 30;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(FERAL_CHARGE);
            spells[2].targettype = TARGET_RANDOM_SINGLE;
            spells[2].instant = true;
            spells[2].cooldown = 3;
            spells[2].perctrigger = 18.0f;
            spells[2].attackstoptimer = 2000;
            spells[2].mindist2cast = 0.0f;
            spells[2].maxdist2cast = 40.0f;

            spells[3].info = sSpellCustomizations.GetSpellInfo(CL_ENRAGE);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = 30;
            spells[3].perctrigger = 15.0f;
            spells[3].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            spells[3].casttime = (uint32)time(NULL) + RandomUInt(10);

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* /*mTarget*/) override
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
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
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
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }

                        if (i == 1)
                        {
                            Unit* Swamplord = NULL;
                            Swamplord = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), 17826);
                            if (Swamplord && Swamplord->isAlive())
                            {
                                sendDBChatMessage(1462);
                            }
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

            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;        // From M4ksiu - Big THX to Capt.
                for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(getCreature(), (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(getCreature(), (*itr)) && (*itr) != getCreature())) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && getCreature()->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || isHostile(getCreature(), RandomTarget)))
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

// Swamplord Musel'ekAI
class SwamplordMuselekAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SwamplordMuselekAI);
        SP_AI_Spell spells[5];
        bool m_spellcheck[5];

        SwamplordMuselekAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(THROW_FREEZING_TRAP);
            spells[0].targettype = TARGET_RANDOM_SINGLE; // not sure to target type
            spells[0].instant = true;
            spells[0].cooldown = 30;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].mindist2cast = 0.0f;
            spells[0].maxdist2cast = 40.0f;

            spells[1].info = sSpellCustomizations.GetSpellInfo(KNOCK_AWAY_MUSELEK);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = 20;
            spells[1].perctrigger = 12.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(AIMED_SHOT);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].cooldown = 20;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 6500;

            spells[3].info = sSpellCustomizations.GetSpellInfo(MULTI_SHOT);
            spells[3].targettype = TARGET_ATTACKING; // changed from VARIOUS to prevent crashes
            spells[3].instant = true;
            spells[3].cooldown = 15;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(SHOT);
            spells[4].targettype = TARGET_ATTACKING;
            spells[4].instant = true;
            spells[4].cooldown = 0;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;

            // new
            addEmoteForEvent(Event_OnCombatStart, SAY_SWAMPLORD_MUSEL_02);
            addEmoteForEvent(Event_OnCombatStart, SAY_SWAMPLORD_MUSEL_03);
            addEmoteForEvent(Event_OnCombatStart, SAY_SWAMPLORD_MUSEL_04);
            addEmoteForEvent(Event_OnTargetDied, SAY_SWAMPLORD_MUSEL_05);
            addEmoteForEvent(Event_OnTargetDied, SAY_SWAMPLORD_MUSEL_06);
            addEmoteForEvent(Event_OnDied, SAY_SWAMPLORD_MUSEL_07);
        }

        void OnCombatStart(Unit* mTarget) override
        {
            for (uint8 i = 0; i < 5; i++)
                spells[i].casttime = 0;

            Unit* Bear = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), 17827);
            if (Bear && Bear->isAlive())
                Bear->GetAIInterface()->AttackReaction(mTarget, 1, 0);

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            if (getCreature()->GetAIInterface()->getNextTarget())
            {
                Unit* target = getCreature()->GetAIInterface()->getNextTarget();
                if (getCreature()->GetDistance2dSq(target) >= 100.0f && getCreature()->getDistanceSq(target) <= 900.0f && RandomUInt(3) != 1)
                {
                    getCreature()->GetAIInterface()->StopMovement(2000);
                    if (!getCreature()->isCastingNonMeleeSpell())
                    {
                        uint32 t = (uint32)time(NULL);
                        uint32 RangedSpell = RandomUInt(100);
                        if (RangedSpell <= 20 && t > spells[2].casttime)
                        {
                            getCreature()->CastSpell(target, spells[2].info, spells[2].instant);
                            getCreature()->setAttackTimer(spells[2].attackstoptimer, false);

                            spells[2].casttime = t + spells[2].cooldown;
                        }

                        if (RangedSpell > 20 && RangedSpell <= 40 && t > spells[3].casttime)
                        {
                            getCreature()->CastSpell(target, spells[3].info, spells[3].instant);
                            getCreature()->setAttackTimer(spells[3].attackstoptimer, false);

                            spells[3].casttime = t + spells[3].cooldown;
                        }

                        else
                        {
                            getCreature()->CastSpell(target, spells[4].info, spells[4].instant);
                            getCreature()->setAttackTimer(spells[4].attackstoptimer, false);
                        }
                    }
                }

                else if (getCreature()->GetDistance2dSq(target) < 100.0f)
                {
                    float val = RandomFloat(100.0f);
                    SpellCast(val);
                }
            }

            else return;
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        if (i != 1 || (i == 1 && getCreature()->GetDistance2dSq(target) <= 100.0f))
                        {
                            if (!spells[i].instant)
                                getCreature()->GetAIInterface()->StopMovement(1);

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
                        }

                        if (spells[i].speech != "")
                        {
                            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            getCreature()->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint8 nrspells;
};


// The Black StalkerAI
class TheBlackStalkerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TheBlackStalkerAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        TheBlackStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(CHAIN_LIGHTNING);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = false;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 12.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].mindist2cast = 0.0f;
            spells[0].maxdist2cast = 40.0f;

            spells[1].info = sSpellCustomizations.GetSpellInfo(LEVITATE);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = true;
            spells[1].cooldown = 30;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].mindist2cast = 0.0f;
            spells[1].maxdist2cast = 40.0f;

            spells[2].info = sSpellCustomizations.GetSpellInfo(STATIC_CHARGE);
            spells[2].targettype = TARGET_RANDOM_SINGLE;
            spells[2].instant = true;
            spells[2].cooldown = 25;
            spells[2].perctrigger = 8.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].mindist2cast = 0.0f;
            spells[2].maxdist2cast = 40.0f;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SUMMON_SPORE_STRIDER);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = 10;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            spells[3].casttime = (uint32)time(NULL) + spells[3].cooldown + RandomUInt(5);

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            uint32 t = (uint32)time(NULL);
            if (t > spells[3].casttime && !getCreature()->isCastingNonMeleeSpell())
            {
                getCreature()->CastSpell(getCreature(), spells[3].info, spells[3].instant);

                spells[3].casttime = (uint32)time(NULL) + spells[3].cooldown + RandomUInt(5);
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
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

            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;        // From M4ksiu - Big THX to Capt
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

/// \note Wasp/Stinger must be checked. Please check it (because for sure
// many spells/creatures with spells are missing and also you will find some dupes.
// No spells found for: Windcaller Claw, Spore Spider, Earthbinder Rayge
// Left Underbog Mushroom.
void SetupTheUnderbog(ScriptMgr* mgr)
{
    //Creatures
    mgr->register_creature_script(CN_HUNGARFEN, &HungarfenAI::Create);
    mgr->register_creature_script(CN_GHAZAN, &GhazanAI::Create);
    mgr->register_creature_script(CN_CLAW, &ClawAI::Create);
    mgr->register_creature_script(CN_SWAMPLORD_MUSELEK, &SwamplordMuselekAI::Create);
    mgr->register_creature_script(CN_THE_BLACK_STALKER, &TheBlackStalkerAI::Create);
}
