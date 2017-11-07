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


//////////////////////////////////////////////////////////////////////////////////////////
//    EMERALD DREAM DRAGONS!
//////////////////////////////////////////////////////////////////////////////////////////


// Emeriss AI
//\todo Check Putrid Mushroom
const uint32 CN_EMERISS = 14889;

const uint32 SLEEP = 24777;
const uint32 NOXIOUS_BREATH = 24818;
const uint32 TAIL_SWEEP = 15847;
const uint32 MARK_OF_NATURE = 25040;   //If a player is killed by Emeriss, they will be afflicted by a 15 minute debuff called Mark of Nature. If resurrected during this time, they will be slept for 2 minutes rather than 4 seconds if they are hit with Sleep.
const uint32 VOLATILE_INFECTION = 24928;
const uint32 CORRUPTION_OF_EARTH = 24910;
const uint32 PUTRID_MUSHROOM = 24904; //31690 - Putrid mushroom //Summon Putrid Mushroom = 24904     //Despawn Putrid Mushroom = 24958
const uint32 TELEPORT = 15734;

class EmerissAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(EmerissAI);
        SP_AI_Spell spells[7];
        bool m_spellcheck[7];

        EmerissAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 7;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SLEEP);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(NOXIOUS_BREATH);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 15.0f;
            spells[1].attackstoptimer = 2000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(TAIL_SWEEP);
            spells[2].targettype = TARGET_ATTACKING;        //if players are behind it's tail
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 7.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(VOLATILE_INFECTION);
            spells[3].targettype = TARGET_VARIOUS;
            spells[3].instant = false;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 10.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(CORRUPTION_OF_EARTH);
            spells[4].targettype = TARGET_ATTACKING;
            spells[4].instant = true;
            spells[4].cooldown = -1;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;
            spells[4].speech = "Taste your world's corruption!";

            spells[5].info = sSpellCustomizations.GetSpellInfo(PUTRID_MUSHROOM);
            spells[5].targettype = TARGET_DESTINATION;
            spells[5].instant = true;
            spells[5].cooldown = -1;
            spells[5].perctrigger = 0.0f;
            spells[5].attackstoptimer = 1000;

            spells[6].info = sSpellCustomizations.GetSpellInfo(MARK_OF_NATURE);
            spells[6].targettype = TARGET_VARIOUS;
            spells[6].instant = true;
            spells[6].cooldown = -1;
            spells[6].perctrigger = 0.0f;
            spells[6].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Hope is a DISEASE of the soul! This land shall wither and die!");
            RegisterAIUpdateEvent(1000); //Attack time is to slow on this boss
            CastTime();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                getCreature()->CastSpell(mTarget, spells[6].info, spells[6].instant);
                getCreature()->CastSpellAoF(mTarget->GetPosition(), spells[5].info, spells[5].instant);
                //When a player dies a Putrid Mushroom spawns at their corpse. This deals 600 Nature damage per second to any surrounding player.
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void AIUpdate()
        {
            // M4ksiu: Someone who wrote this hadn't thought about it much, so it should be rewritten
            Unit* Target = getCreature()->GetAIInterface()->getNextTarget();
            if (Target != NULL && !getCreature()->isInRange(Target, 20.0f))
                getCreature()->CastSpell(Target, TELEPORT, true);

            if (getCreature()->GetHealthPct() == 25 || getCreature()->GetHealthPct() == 50 || getCreature()->GetHealthPct() == 75)
            {
                getCreature()->CastSpell(getCreature(), spells[4].info, spells[4].instant);
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[4].speech.c_str());
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

        uint8 nrspells;
};

// Taerar AI
const uint32 CN_TAERAR = 14890;
const uint32 CN_SHADESTAERAR = 15302;

const uint32 ARCANE_BLAST = 24857;
const uint32 BELLOWING_ROAR = 22686;           //Mass fear
const uint32 SUMMON_SHADE = 24843;

class TaerarAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(TaerarAI);
        SP_AI_Spell spells[7];
        bool m_spellcheck[7];

        TaerarAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 7;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SLEEP);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 3.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(NOXIOUS_BREATH);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 5.0f;
            spells[1].attackstoptimer = 2000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(TAIL_SWEEP);
            spells[2].targettype = TARGET_ATTACKING;        //if players are behind it's tail
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 3.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(ARCANE_BLAST);
            spells[3].targettype = TARGET_VARIOUS;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 3.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(BELLOWING_ROAR);
            spells[4].targettype = TARGET_ATTACKING;
            spells[4].instant = true;
            spells[4].cooldown = -1;
            spells[4].perctrigger = 2.0f;
            spells[4].attackstoptimer = 1000;

            spells[5].info = sSpellCustomizations.GetSpellInfo(SUMMON_SHADE);
            spells[5].targettype = TARGET_DESTINATION;
            spells[5].instant = true;
            spells[5].cooldown = -1;
            spells[5].perctrigger = 0.0f;
            spells[5].attackstoptimer = 1000;
            spells[5].speech = "Children of Madness - I release you upon this world!";

            spells[6].info = sSpellCustomizations.GetSpellInfo(MARK_OF_NATURE);
            spells[6].targettype = TARGET_VARIOUS;
            spells[6].instant = true;
            spells[6].cooldown = -1;
            spells[6].perctrigger = 0.0f;
            spells[6].attackstoptimer = 1000;

            Shades = false;
            Shade_timer = 0;
            Summoned = 0;

        }

        void OnCombatStart(Unit* mTarget)
        {
            Shades = false;
            Shade_timer = 0;
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Peace is but a fleeting dream! Let the NIGHTMARE reign!");
            RegisterAIUpdateEvent(1000); //Attack time is to slow on this boss
            CastTime();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                getCreature()->CastSpell(mTarget, spells[6].info, spells[6].instant);
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            Shades = false;
            Shade_timer = 0;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
            CastTime();
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnDied(Unit* mKiller)
        {
            Shades = false;
            Shade_timer = 0;
        }

        void SummonShades(Unit* mTarget)
        {
            Summoned = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(CN_SHADESTAERAR, mTarget->GetPositionX(), mTarget->GetPositionY(), mTarget->GetPositionZ(), 0, true, false, getCreature()->GetFaction(), 50);
            Summoned->GetAIInterface()->setNextTarget(mTarget);
        }

        void AIUpdate()
        {
            // M4ksiu: Someone who wrote this hadn't thought about it much, so it should be rewritten
            Unit* Target = getCreature()->GetAIInterface()->getNextTarget();
            if (Target != NULL && !getCreature()->isInRange(Target, 20.0f))
                getCreature()->CastSpell(Target, TELEPORT, true);

            if (Shades && Shade_timer == 0)
            {
                //Become unbanished again
                getCreature()->SetFaction(14);
                //_unit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                getCreature()->GetAIInterface()->setNextTarget(getCreature()->GetAIInterface()->getNextTarget());
                Shades = false;
            }
            else if (Shades)
            {
                Shade_timer--;
                //Do nothing while banished
                return;
            }
            if (getCreature()->GetHealthPct() == 25 || getCreature()->GetHealthPct() == 50 || getCreature()->GetHealthPct() == 75)
            {
                //Inturrupt any spell casting
                getCreature()->InterruptSpell();
                //Root self
                getCreature()->CastSpell(getCreature(), 23973, true);
                getCreature()->SetFaction(35);
                //_unit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                //Summon shades
                SummonShades(getCreature()->GetAIInterface()->getNextTarget());
                SummonShades(getCreature()->GetAIInterface()->getNextTarget());
                SummonShades(getCreature()->GetAIInterface()->getNextTarget());
                Shades = true;
                Shade_timer = 60;
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[5].speech.c_str());
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

        Creature* Summoned;
        bool Shades;
        int Shade_timer;
        uint8 nrspells;
};

// Shades of Taerar AI
const uint32 CN_SHADEOFTAERAR = 15302;

const uint32 POSION_CLOUD = 24840;
const uint32 POSION_BREATH = 20667;

class ShadeofTaerarAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(ShadeofTaerarAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        ShadeofTaerarAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(POSION_CLOUD);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 18.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(POSION_BREATH);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 16.0f;
            spells[1].attackstoptimer = 2000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* mTarget)
        {
            //You died kek
        }

        void OnCombatStop(Unit* mTarget)
        {
            getCreature()->Despawn(15, 0);
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
            CastTime();
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void AIUpdate()
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

        uint8 nrspells;
};



// Ysondre AI
const uint32 CN_YSONDRE = 14887;
const uint32 CN_YDRUIDS = 15260;

const uint32 LIGHTNING_WAVE = 24819;
const uint32 SUMMON_DRUIDS = 24795; // Summon Demented Druid Spirit

class YsondreAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(YsondreAI);
        SP_AI_Spell spells[6];
        bool m_spellcheck[6];

        YsondreAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 6;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SLEEP);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 3.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(NOXIOUS_BREATH);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 5.0f;
            spells[1].attackstoptimer = 2000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(TAIL_SWEEP);
            spells[2].targettype = TARGET_ATTACKING;        //if players are behind it's tail
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 3.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(LIGHTNING_WAVE);
            spells[3].targettype = TARGET_VARIOUS;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 3.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(SUMMON_DRUIDS);
            spells[4].targettype = TARGET_DESTINATION;
            spells[4].instant = true;
            spells[4].cooldown = -1;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;
            spells[4].speech = "Come forth, ye Dreamers - and claim your vengeance!";

            spells[5].info = sSpellCustomizations.GetSpellInfo(MARK_OF_NATURE);
            spells[5].targettype = TARGET_VARIOUS;
            spells[5].instant = true;
            spells[5].cooldown = -1;
            spells[5].perctrigger = 0.0f;
            spells[5].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "The strands of LIFE have been severed! The Dreamers must be avenged!");
            RegisterAIUpdateEvent(1000); //Attack time is to slow on this boss
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                getCreature()->CastSpell(mTarget, spells[5].info, spells[5].instant);
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void AIUpdate()
        {
            // M4ksiu: Someone who wrote this hadn't thought about it much, so it should be rewritten
            Unit* Target = getCreature()->GetAIInterface()->getNextTarget();
            if (Target != NULL && !getCreature()->isInRange(Target, 20.0f))
                getCreature()->CastSpell(Target, TELEPORT, true);

            if (getCreature()->GetHealthPct() == 25 || getCreature()->GetHealthPct() == 50 || getCreature()->GetHealthPct() == 75)
            {
                // Summon 6 druids
                getCreature()->CastSpellAoF(getCreature()->GetAIInterface()->getNextTarget()->GetPosition(), spells[4].info, spells[4].instant);
                getCreature()->CastSpellAoF(getCreature()->GetAIInterface()->getNextTarget()->GetPosition(), spells[4].info, spells[4].instant);
                getCreature()->CastSpellAoF(getCreature()->GetAIInterface()->getNextTarget()->GetPosition(), spells[4].info, spells[4].instant);
                getCreature()->CastSpellAoF(getCreature()->GetAIInterface()->getNextTarget()->GetPosition(), spells[4].info, spells[4].instant);
                getCreature()->CastSpellAoF(getCreature()->GetAIInterface()->getNextTarget()->GetPosition(), spells[4].info, spells[4].instant);
                getCreature()->CastSpellAoF(getCreature()->GetAIInterface()->getNextTarget()->GetPosition(), spells[4].info, spells[4].instant);

                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[4].speech.c_str());
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

        uint8 nrspells;
};

// Demented Druid Spirit AI
const uint32 CN_DEMENTEDDRUID = 15260;

const uint32 MOONFIRE = 27737;

class DementedDruidSpiritAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(DementedDruidSpiritAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        DementedDruidSpiritAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(MOONFIRE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 11.0f;
            spells[0].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* mTarget)
        {
            //You died kek
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            getCreature()->Despawn(15, 0);
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void AIUpdate()
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

        uint8 nrspells;
};

// Lethon AI
const uint32 CN_LETHON = 14888;

const uint32 SHADOW_WHIRL = 24837;
const uint32 SUMMON_SHADES = 24810;

class LethonAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(LethonAI);
        SP_AI_Spell spells[6];
        bool m_spellcheck[6];

        LethonAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 6;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SLEEP);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 3.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(NOXIOUS_BREATH);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 5.0f;
            spells[1].attackstoptimer = 2000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(TAIL_SWEEP);
            spells[2].targettype = TARGET_ATTACKING;        //if players are behind it's tail
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 3.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SHADOW_WHIRL);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 2.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(SUMMON_SHADES);
            spells[4].targettype = TARGET_SELF;
            spells[4].instant = true;
            spells[4].cooldown = -1;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;
            spells[4].speech = "Your wicked souls shall feed my power!";

            spells[5].info = sSpellCustomizations.GetSpellInfo(MARK_OF_NATURE);
            spells[5].targettype = TARGET_VARIOUS;
            spells[5].instant = true;
            spells[5].cooldown = -1;
            spells[5].perctrigger = 0.0f;
            spells[5].attackstoptimer = 1000;

            Shade1 = false;
            Shade2 = false;
            Shade3 = false;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "I can sense the SHADOW on your hearts. There can be no rest for the wicked!");
            RegisterAIUpdateEvent(1000); //Attack time is to slow on this boss
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                getCreature()->CastSpell(mTarget, spells[5].info, spells[5].instant); //Mark of nature
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            Shade1 = false;
            Shade2 = false;
            Shade3 = false;
            CastTime();
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void AIUpdate()
        {
            std::list<Player*> mTargets;
            // \todo Someone who wrote this hadn't thought about it much, so it should be rewritten
            Unit* Target = getCreature()->GetAIInterface()->getNextTarget();
            if (Target != NULL && !getCreature()->isInRange(Target, 20.0f))
                getCreature()->CastSpell(Target, TELEPORT, true);


            //Made it like this because if lethon gets healed, he should spawn the adds again at the same pct. (Only spawn once at 75,50,25)
            switch (getCreature()->GetHealthPct())
            {
                case 25:
                {
                    if (!Shade3)
                        Shade3 = true;
                }break;
                case 50:
                {
                    if (!Shade2)
                        Shade2 = true;
                }break;
                case 75:
                {
                    if (!Shade1)
                        Shade1 = true;
                }break;
            }
            // Summon a spirit for each player
            std::list<Player*>::iterator itr = mTargets.begin();
            for (; itr != mTargets.end(); ++itr)
            {
                getCreature()->CastSpellAoF((*itr)->GetPosition(), spells[4].info, spells[4].instant);
            }
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[4].speech.c_str());

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

        bool Shade1; //75%
        bool Shade2; //50%
        bool Shade3; //25%
        uint8 nrspells;
};


//Shades of Lethon

const uint32 CN_LSHADE = 15261;

class ShadeofLethonAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(ShadeofLethonAI);

        ShadeofLethonAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {

        }

        void OnCombatStart(Unit* mTarget)
        {
            CheckDist();
            RegisterAIUpdateEvent(1000); //they cant attack anyway, update every sec instead
        }

        void OnTargetDied(Unit* mTarget)
        {
            //Will nevah happenz! haha
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
            getCreature()->Despawn(15, 0);
        }

        void CheckDist()
        {
            float distance;
            std::list<Creature*> mTargets;
            std::list<Creature*>::iterator itr = mTargets.begin();
            for (; itr != mTargets.end(); ++itr)
            {
                if ((*itr)->GetGUID() == CN_LETHON)
                {
                    distance = (*itr)->getDistanceSq((*itr)->GetPositionX(), (*itr)->GetPositionY(), (*itr)->GetPositionZ());
                    if (distance < 5.0)
                    {
                        (*itr)->SetHealth(((*itr)->getUInt32Value(UNIT_FIELD_MAXHEALTH) / 100)); //Heal him 1%
//                    if ((*itr)->GetUInt32Value(UNIT_FIELD_HEALTH) > (*itr)->GetUInt32Value(UNIT_FIELD_MAXHEALTH))
//                        (*itr)->SetUInt32Value(UNIT_FIELD_HEALTH, (*itr)->GetUInt32Value(UNIT_FIELD_MAXHEALTH)); //Do i need to do this....?
                        getCreature()->Despawn(1, 0);
                    }
                    else
                        getCreature()->GetAIInterface()->_CalcDestinationAndMove((*itr), distance);
                }
                else
                    OnCombatStop(getCreature());
            }
        }

        void AIUpdate()
        {
            //Repeat this, if they move Lethon while the ghosts move, they need to update his position
            CheckDist();
        }

};



//////////////////////////////////////////////////////////////////////////////////////////
// Rest of World Bosses
//////////////////////////////////////////////////////////////////////////////////////////

// Highlord Kruul
const uint32 CN_KRUUL = 18338;
const uint32 CN_HOUNDS = 19207;

const uint32 SHADOW_VOLLEY = 21341;
const uint32 CLEAVE = 20677;
const uint32 THUNDER_CLAP = 23931;
const uint32 TWISTED_REFLECTION = 21063;
const uint32 VOID_BOLT = 21066;
const uint32 RAGE = 21340;
const uint32 CAPTURE_SOUL = 21053;

class KruulAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(KruulAI);
        SP_AI_Spell spells[7];
        bool m_spellcheck[7];

        KruulAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 7;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SHADOW_VOLLEY);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 5.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(CLEAVE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 12.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(THUNDER_CLAP);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 10.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(TWISTED_REFLECTION);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 7.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(VOID_BOLT);
            spells[4].targettype = TARGET_ATTACKING;
            spells[4].instant = false;
            spells[4].cooldown = -1;
            spells[4].perctrigger = 5.0f;
            spells[4].attackstoptimer = 1000;

            spells[5].info = sSpellCustomizations.GetSpellInfo(CAPTURE_SOUL);
            spells[5].targettype = TARGET_VARIOUS;
            spells[5].instant = true;
            spells[5].cooldown = -1;
            spells[5].perctrigger = 0.0f;
            spells[5].attackstoptimer = 1000;

            spells[6].info = sSpellCustomizations.GetSpellInfo(RAGE);
            spells[6].targettype = TARGET_SELF;
            spells[6].instant = true;
            spells[6].cooldown = -1;
            spells[6].perctrigger = 0.0f;
            spells[6].attackstoptimer = 1000;

            hounds_timer = 0;
            enrage = 0;
            Rand = 0;
            RandX = 0;
            RandY = 0;
            enrage = 0;
            Summoned = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            hounds_timer = 45;
            enrage = 0;

            switch (RandomUInt(4))
            {
                case 0:
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Azeroth has cowered too long under our shadow! Now, feel the power of the Burning Crusade, and despair!");
                    break;
                case 1:
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Your fate is sealed, Azeroth! I will find the Aspect Shards, and then you will not stand against our might!");
                    break;
                case 2:
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Cower, little worms! Your heroes are nothing! Your saviors will be our first feast!");
                    break;
                case 3:
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Where? Where are the Shards! You cannot hide them from us!");
                    break;
                case 4:
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Your world will die, mortals! Your doom is now at hand!");
                    break;
            }
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
            CastTime();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Your own strength feeds me, $N!");
                getCreature()->CastSpell(getCreature(), spells[5].info, spells[5].instant); // Either himself or target? :P
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            hounds_timer = 45;
            enrage = 0;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
            CastTime();
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnDied(Unit* mKiller)
        {
            hounds_timer = 45;
            enrage = 0;
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Ha! This place is not yet worthy of my infliction.");
            CastTime();
        }

        void SummonHounds(Unit* mTarget)
        {
            Rand = RandomUInt(15);
            switch (RandomUInt(1))
            {
                case 0:
                    RandX = 0 - Rand;
                    break;
                case 1:
                    RandX = 0 + Rand;
                    break;
            }

            Rand = RandomUInt(15);
            switch (RandomUInt(1))
            {
                case 0:
                    RandY = 0 - Rand;
                    break;
                case 1:
                    RandY = 0 + Rand;
                    break;
            }
            Rand = 0;
            Summoned = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(CN_HOUNDS, (float)RandX, (float)RandY, 0, 0, true, false, getCreature()->GetFaction(), 50);
            Summoned->GetAIInterface()->setNextTarget(mTarget);
        }

        void AIUpdate()
        {
            if (hounds_timer == 0)
            {
                SummonHounds(getCreature()->GetAIInterface()->getNextTarget());
                SummonHounds(getCreature()->GetAIInterface()->getNextTarget());
                SummonHounds(getCreature()->GetAIInterface()->getNextTarget());
                hounds_timer = 45;
            }
            else
                hounds_timer--;

            if (enrage == 60)
                getCreature()->CastSpell(getCreature(), spells[6].info, spells[6].instant);
            else
            {
                enrage++;
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

        int Rand;
        int RandX;
        int RandY;
        int hounds_timer;
        int enrage;
        Creature* Summoned;
        uint8 nrspells;
};

// Doom Lord Kazzak (Lord Kazzak does not exist anymore, he is promoted and replaced by Highlord Kruul instead)
///\todo  Death Messages. Both himself and player needs.
/*
Remaining:
11338,10,"A_GRULLAIR_Kazzak_Slay02" (443956),"GRULLAIR_Kazzak_Slay02.wav"
11340,10,"A_GRULLAIR_Kazzak_Death01" (444060),"GRULLAIR_Kazzak_Death01.wav"
*/

const uint32 CN_KAZZAK = 18728;

const uint32 MARK_OF_KAZZAK = 21056;

class KazzakAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(KazzakAI);
        SP_AI_Spell spells[8];
        bool m_spellcheck[8];

        KazzakAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 8;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(SHADOW_VOLLEY);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 5.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(CLEAVE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 12.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(THUNDER_CLAP);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 10.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(TWISTED_REFLECTION);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 7.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(VOID_BOLT);
            spells[4].targettype = TARGET_ATTACKING;
            spells[4].instant = false;
            spells[4].cooldown = -1;
            spells[4].perctrigger = 5.0f;
            spells[4].attackstoptimer = 1000;

            spells[5].info = sSpellCustomizations.GetSpellInfo(MARK_OF_KAZZAK);
            spells[5].targettype = TARGET_VARIOUS;
            spells[5].instant = true;
            spells[5].cooldown = -1;
            spells[5].perctrigger = 5.0f;
            spells[5].attackstoptimer = 1000;

            spells[6].info = sSpellCustomizations.GetSpellInfo(CAPTURE_SOUL);
            spells[6].targettype = TARGET_DESTINATION;
            spells[6].instant = true;
            spells[6].cooldown = -1;
            spells[6].perctrigger = 0.0f;
            spells[6].attackstoptimer = 1000;

            spells[7].info = sSpellCustomizations.GetSpellInfo(RAGE);
            spells[7].targettype = TARGET_SELF;
            spells[7].instant = true;
            spells[7].cooldown = -1;
            spells[7].perctrigger = 0.0f;
            spells[7].attackstoptimer = 1000;

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));

            //Spawn intro.
            sendDBChatMessage(373);      // I remember well the sting of defeat at the conclusion...

            enrage = 0;

        }

        void OnCombatStart(Unit* mTarget)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    sendDBChatMessage(374);      // All mortals will perish!
                    break;
                case 1:
                    sendDBChatMessage(375);      // The Legion will conquer all!
                    break;
            }
            CastTime();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        sendDBChatMessage(379);      // Contemptible wretch!
                        break;
                    case 1:
                        sendDBChatMessage(378);      // Kirel narak!
                        break;
                }
                getCreature()->CastSpell(getCreature(), spells[6].info, spells[6].instant);
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            sendDBChatMessage(380);      // The universe will be remade.

            enrage = 0;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            CastTime();
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnDied(Unit* mKiller)
        {
            sendDBChatMessage(381);      // The Legion... will never... fall.
            CastTime();
        }

        void RandomSpeech()
        {
            switch (RandomUInt(20))        // 10% chance should do, he talks a lot tbh =P
            {
                case 0:
                    sendDBChatMessage(383);      // Invaders, you dangle upon the precipice of oblivion! The Burning...
                    break;
                case 1:
                    sendDBChatMessage(384);      // Impudent whelps, you only delay the inevitable. Where one has fallen, ten shall rise. Such is the will of Kazzak...
                    break;
                default:
                    break;
            }
        }

        void AIUpdate()
        {
            if (getCreature()->CombatStatus.IsInCombat())
            {
                if (enrage == 180)
                {
                    getCreature()->CastSpell(getCreature(), spells[7].info, spells[7].instant);
                    enrage = 0;
                }
                else
                {
                    enrage++;
                    float val = RandomFloat(100.0f);
                    SpellCast(val);
                }
            }
            else
            {
                RandomSpeech();        //awesome idea m4ksui
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

        int enrage;
        uint8 nrspells;
};

// Azuregos

const uint32 CN_AZUREGOS = 6109;

const uint32 MARK_OF_FROST = 23183;
const uint32 MANA_STORM = 21097;
const uint32 REFLECT = 30969;           //Might not be the right, this one doesn't work on dots?
const uint32 ACLEAVE = 8255;            //This is Strong Cleave, maybe it should be 27794, normal cleave 250+ damage
const uint32 CONE_OF_COLD = 30095;
const uint32 MASS_TELEPORT = 16807;
//\todo check for gossip azuregos ghost (AQ) "You seek to harm me $R?
// Answer (race)
// I am a treasure hunter in search of powerful artifacts. Give them to me and you will not be harmed.
// give me all your money and you won't get hurt!
class AzuregosAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(AzuregosAI);
        SP_AI_Spell spells[5];
        bool m_spellcheck[5];

        AzuregosAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 5;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(MANA_STORM);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 7.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(ACLEAVE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 15.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(CONE_OF_COLD);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 10.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(REFLECT);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].casttime = 45;
            spells[3].cooldown = 45;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(MARK_OF_FROST);
            spells[4].targettype = TARGET_DESTINATION;
            spells[4].instant = true;
            spells[4].cooldown = -1;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;

            masstele = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "This Place is under my Protection! The mysteries of the arcane shall remain untouched.");
            masstele = 60;
            RegisterAIUpdateEvent(1000);
            CastTime();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "So is the price of curiosity.");
                getCreature()->CastSpell(mTarget, spells[4].info, spells[4].instant);
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            masstele = 60;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
            CastTime();
        }

        void OnDied(Unit* mKiller)
        {
            masstele = 60;
            CastTime();
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }


        void AIUpdate()
        {
            if (masstele == 0)
            {

                getCreature()->CastSpell(getCreature()->GetAIInterface()->getNextTarget(), MASS_TELEPORT, true);
                getCreature()->GetAIInterface()->WipeHateList();
                masstele = 60;
            }
            else
            {
                masstele--;
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

        int masstele;
        uint8 nrspells;
};

// Doomwalker

const uint32 CN_DOOMWALKER = 17711;

const uint32 EARTHQUAKE = 32686;
const uint32 MARK_OF_DEATH = 37128;
const uint32 CHAIN_LIGHTNING = 33665;
const uint32 OVERRUN = 32636;
const uint32 ENRAGE = 33653;
const uint32 AURA_OF_DEATH = 37131;
const uint32 SUNDER_ARMOR = 33661;

class DoomwalkerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(DoomwalkerAI);
        SP_AI_Spell spells[6];
        bool m_spellcheck[6];

        DoomwalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 6;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(EARTHQUAKE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 55;
            spells[0].casttime = 55;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(MARK_OF_DEATH);
            spells[1].targettype = TARGET_DESTINATION;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(CHAIN_LIGHTNING);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = false;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 8.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(OVERRUN);
            spells[3].targettype = TARGET_VARIOUS;
            spells[3].instant = true;
            spells[3].casttime = 45;
            spells[3].cooldown = 45;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(ENRAGE);
            spells[4].targettype = TARGET_SELF;
            spells[4].instant = true;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;

            spells[5].info = sSpellCustomizations.GetSpellInfo(SUNDER_ARMOR);
            spells[5].targettype = TARGET_ATTACKING;
            spells[5].instant = true;
            spells[5].cooldown = 15;
            spells[5].perctrigger = 0.0f;
            spells[5].attackstoptimer = 1000;

            enraged = false;
        }

        void OnCombatStart(Unit* mTarget)
        {
            sendDBChatMessage(302);      // Do not proceed. You will be eliminated.
            RegisterAIUpdateEvent(1000);
            CastTime();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                switch (RandomUInt(2))
                {
                    case 0:
                        sendDBChatMessage(307);      // Threat level zero.
                        break;
                    case 1:
                        sendDBChatMessage(308);      // Directive accomplished.
                        break;
                    case 2:
                        sendDBChatMessage(309);      // Target exterminated.
                        break;
                }
                getCreature()->CastSpell(mTarget, spells[1].info, spells[1].instant);
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            getCreature()->RemoveAura(AURA_OF_DEATH);
            enraged = false;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
            CastTime();
        }

        void OnDied(Unit* mKiller)
        {
            sendDBChatMessage(310);      // System failure in five, f-o-u-r...
            getCreature()->RemoveAura(AURA_OF_DEATH);
            CastTime();
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }


        void AIUpdate()
        {
            if (getCreature()->GetHealthPct() == 20 && enraged == false)  //if he stays to long on 20% it could double activate without this check?
            {
                getCreature()->CastSpell(getCreature(), spells[4].info, spells[4].instant);
                enraged = true;
            }
            //_unit->CastSpell(_unit, AURA_OF_DEATH, true); //Repulse this every AIUpdate :) Spell is bugged atm, it also kills him methinks, not only those with Mark of Death
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
                        if (m_spellcheck[0] == true)  //Earthquake
                        {
                            getCreature()->GetAIInterface()->WipeHateList();
                            switch (RandomUInt(1))
                            {
                                case 0:
                                    sendDBChatMessage(303);      // Tectonic disruption commencing.
                                    break;
                                case 1:
                                    sendDBChatMessage(304);      // Magnitude set. Release.
                                    break;
                            }
                        }
                        if (m_spellcheck[3] == true)  //Overrun
                        {
                            getCreature()->GetAIInterface()->WipeHateList();
                            switch (RandomUInt(1))
                            {
                                case 0:
                                    sendDBChatMessage(305);      // Trajectory locked.
                                    break;
                                case 1:
                                    sendDBChatMessage(306);      // Engage maximum speed.
                                    break;
                            }
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

        bool enraged; //Just to make sure
        uint8 nrspells;
};

// Teremus The Devourer

const uint32 CN_TEREMUS = 7846;

const uint32 FLAME_BREATH = 20712;
const uint32 SOUL_CONSUMPTION = 12667;
const uint32 RESIST_ALL = 18114;

class TeremusAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(TeremusAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        TeremusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(FLAME_BREATH);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SOUL_CONSUMPTION);
            spells[1].targettype = TARGET_DESTINATION;
            spells[1].instant = false;
            spells[1].perctrigger = 9.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(RESIST_ALL);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = false;
            spells[2].perctrigger = 5.0f;
            spells[2].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(1000);
            CastTime();
        }

        void OnTargetDied(Unit* mTarget)
        {

        }
        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
            CastTime();
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }


        void AIUpdate()
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

        uint8 nrspells;
};

void SetupWorldBosses(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_EMERISS, &EmerissAI::Create);
    mgr->register_creature_script(CN_TAERAR, &TaerarAI::Create);
    mgr->register_creature_script(CN_SHADEOFTAERAR, &ShadeofTaerarAI::Create);
    mgr->register_creature_script(CN_YSONDRE, &YsondreAI::Create);
    mgr->register_creature_script(CN_DEMENTEDDRUID, &DementedDruidSpiritAI::Create);
    mgr->register_creature_script(CN_LETHON, &LethonAI::Create);
    mgr->register_creature_script(CN_LSHADE, &ShadeofLethonAI::Create);
    mgr->register_creature_script(CN_KRUUL, &KruulAI::Create);
    mgr->register_creature_script(CN_KAZZAK, &KazzakAI::Create);
    mgr->register_creature_script(CN_AZUREGOS, &AzuregosAI::Create);
    mgr->register_creature_script(CN_DOOMWALKER, &DoomwalkerAI::Create);
    mgr->register_creature_script(CN_TEREMUS, &TeremusAI::Create);
}
