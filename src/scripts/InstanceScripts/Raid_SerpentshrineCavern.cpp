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
#include "Raid_SerpentshrineCavern.h"
#include "Objects/Faction.h"
#include <Spell/Definitions/PowerType.h>

const uint32 SP_SCALDING_WATER = 37284; //This "dot" is supposed to tick whenever you get in the water, it doesnt tick anymore after u kill lurker.

//------------------------------------
//    -= Hydross the Unstable =-
//------------------------------------
const uint32 WATER_TOMB = 38235;
const uint32 VILE_SLUDGE = 38246;
const uint32 MARK_OF_HYDROSS1 = 38215;
const uint32 MARK_OF_HYDROSS2 = 38216;
const uint32 MARK_OF_HYDROSS3 = 38217;
const uint32 MARK_OF_HYDROSS4 = 38218;
const uint32 MARK_OF_HYDROSS5 = 38231;
const uint32 MARK_OF_HYDROSS6 = 40584;

const uint32 MARK_OF_CORRUPTION1 = 38219;
const uint32 MARK_OF_CORRUPTION2 = 38220;
const uint32 MARK_OF_CORRUPTION3 = 38221;
const uint32 MARK_OF_CORRUPTION4 = 38222;
const uint32 MARK_OF_CORRUPTION5 = 38230;
const uint32 MARK_OF_CORRUPTION6 = 40583;

const uint32 HYDROSS_ENRAGE = 13048;
const uint32 PURIFY_ELEMENTAL = 36461;

class HydrossTheUnstableAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HydrossTheUnstableAI);
        HydrossTheUnstableAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            /*spells[0].info = sSpellCustomizations.GetSpellInfo(WATER_TOMB);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = true;
            spells[0].cooldown = 7;
            spells[0].attackstoptimer = 1000;
            m_spellcheck[0] = false;

            spells[1].info = sSpellCustomizations.GetSpellInfo(VILE_SLUDGE);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = true;
            spells[1].cooldown = 15;
            spells[1].attackstoptimer = 1000;
            m_spellcheck[1] = false;*/

            //frost attack type
            const_cast<CreatureProperties*>(getCreature()->GetCreatureProperties())->attackSchool = 4;
            //frost immunity
            getCreature()->SchoolImmunityList[SCHOOL_FROST] = 1;

            MarkCount = 0;
            form = false;
            MarkTimer = 0;
            minspell = 0;
            maxspell = 0;
            Enraged = false;
            EnrageTimer = 0;
        }


        void OnCombatStart(Unit* /*mTarget*/) override
        {
            MarkCount = 0;
            form = false;
            MarkTimer = 10;
            minspell = 0;
            maxspell = 0;
            Enraged = false;
            EnrageTimer = 600;

            sendDBChatMessage(4749);     // I cannot allow you to interfere!

            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            getCreature()->SetDisplayId(20162);
            getCreature()->SchoolImmunityList[SCHOOL_FROST] = 1;
            getCreature()->SchoolImmunityList[SCHOOL_NATURE] = 0;
        }

        void OnTargetDied(Unit* /*mTarget*/) override
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                if (!form)
                {
                    switch (Util::getRandomUInt(1))
                    {
                        case 0:
                            sendDBChatMessage(4751);     // They have forced me to this...
                            break;

                        case 1:
                            sendDBChatMessage(4752);     // I had no choice.");
                            break;
                    }
                }
                else
                {
                    switch (Util::getRandomUInt(1))
                    {
                        case 0:
                            sendDBChatMessage(4755);     // I will purge you from this place.
                            break;

                        case 1:
                            sendDBChatMessage(4756);     // You are no better than they!
                            break;
                    }
                }
            }
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            sendDBChatMessage(4757);     // You are the disease, not I..
        }

        void AIUpdate() override
        {
            if (!form) //water form
            {
                //Mark of Hydross
                if (MarkCount < 6)
                {
                    MarkTimer--;
                    if (!MarkTimer)
                    {
//                    uint32 spellid = 0; UNUSED???
                        switch (MarkCount)
                        {
                            case 0:
                                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(MARK_OF_HYDROSS1), true);
                                break;

                            case 1:
                                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(MARK_OF_HYDROSS2), true);
                                break;

                            case 2:
                                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(MARK_OF_HYDROSS3), true);
                                break;

                            case 3:
                                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(MARK_OF_HYDROSS4), true);
                                break;

                            case 4:
                                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(MARK_OF_HYDROSS5), true);
                                break;

                            case 5:
                                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(MARK_OF_HYDROSS6), true);
                                break;
                        }

                        MarkCount++;
                        MarkTimer = 20;
                    }
                }

                //Switch check
                float distx = getCreature()->GetSpawnX() - getCreature()->GetPositionX();
                float disty = getCreature()->GetSpawnY() - getCreature()->GetPositionY();
                float dist = sqrt((distx * distx) + (disty * disty));
                if (dist > 25)
                {
                    //switch to poison form
                    MarkTimer = 10;
                    MarkCount = 0;
                    minspell = 1;
                    maxspell = 1;
                    form = true;
                    getCreature()->SetDisplayId(5498);
                    sendDBChatMessage(4754);     // Aaghh, the poison...
                    getCreature()->PlaySoundToSet(11297);
                    const_cast<CreatureProperties*>(getCreature()->GetCreatureProperties())->attackSchool = 3;
                    getCreature()->SchoolImmunityList[SCHOOL_FROST] = 0;
                    getCreature()->SchoolImmunityList[SCHOOL_NATURE] = 1;

                    //Summon 4 elementals
                    Creature* summon;
                    float posx = getCreature()->GetPositionX();
                    float posy = getCreature()->GetPositionY();
                    float posz = getCreature()->GetPositionZ();
                    float orientation = getCreature()->GetOrientation();

                    summon = spawnCreature(CN_TAINTED_SPAWN_OF_HYDROSS, posx + 6.93f, posy - 11.25f, posz, orientation);
                    if (summon)
                        summon->SchoolImmunityList[SCHOOL_NATURE] = 1;

                    summon = spawnCreature(CN_TAINTED_SPAWN_OF_HYDROSS, posx - 6.93f, posy + 11.25f, posz, orientation);
                    if (summon)
                        summon->SchoolImmunityList[SCHOOL_NATURE] = 1;

                    summon = spawnCreature(CN_TAINTED_SPAWN_OF_HYDROSS, posx - 12.57f, posy - 4.72f, posz, orientation);
                    if (summon)
                        summon->SchoolImmunityList[SCHOOL_NATURE] = 1;

                    summon = spawnCreature(CN_TAINTED_SPAWN_OF_HYDROSS, posx + 12.57f, posy + 4.72f, posz, orientation);
                    if (summon)
                        summon->SchoolImmunityList[SCHOOL_NATURE] = 1;
                }
            }
            else //poison form
            {
                //Mark of Corruption
                if (MarkCount < 6)
                {
                    MarkTimer--;
                    if (!MarkTimer)
                    {
//                    uint32 spellid = 0; UNUSED???
                        switch (MarkCount)
                        {
                            case 0:
                                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(MARK_OF_CORRUPTION1), true);
                                break;

                            case 1:
                                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(MARK_OF_CORRUPTION2), true);
                                break;

                            case 2:
                                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(MARK_OF_CORRUPTION3), true);
                                break;

                            case 3:
                                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(MARK_OF_CORRUPTION4), true);
                                break;

                            case 4:
                                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(MARK_OF_CORRUPTION5), true);
                                break;

                            case 5:
                                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(MARK_OF_CORRUPTION6), true);
                                break;
                        }

                        MarkCount++;
                        MarkTimer = 20;
                    }
                }

                //Switch check
                float distx = getCreature()->GetSpawnX() - getCreature()->GetPositionX();
                float disty = getCreature()->GetSpawnY() - getCreature()->GetPositionY();
                float dist = sqrt((distx * distx) + (disty * disty));
                if (dist < 25)
                {
                    //switch to water form
                    MarkTimer = 10;
                    MarkCount = 0;
                    minspell = 0;
                    maxspell = 0;
                    form = false;
                    getCreature()->SetDisplayId(20162);
                    sendDBChatMessage(4750);     // Better, much better.
                    getCreature()->PlaySoundToSet(11290);
                    const_cast<CreatureProperties*>(getCreature()->GetCreatureProperties())->attackSchool = 4;
                    getCreature()->SchoolImmunityList[SCHOOL_FROST] = 1;
                    getCreature()->SchoolImmunityList[SCHOOL_NATURE] = 0;

                    //Summon 4 elementals
                    Creature* summon;
                    float posx = getCreature()->GetPositionX();
                    float posy = getCreature()->GetPositionY();
                    float posz = getCreature()->GetPositionZ();
                    float orientation = getCreature()->GetOrientation();

                    summon = spawnCreature(CN_PURE_SPAWN_OF_HYDROSS, posx + 6.93f, posy - 11.25f, posz, orientation);
                    if (summon)
                        summon->SchoolImmunityList[SCHOOL_FROST] = 1;

                    summon = spawnCreature(CN_PURE_SPAWN_OF_HYDROSS, posx - 6.93f, posy + 11.25f, posz, orientation);
                    if (summon)
                        summon->SchoolImmunityList[SCHOOL_FROST] = 1;

                    summon = spawnCreature(CN_PURE_SPAWN_OF_HYDROSS, posx - 12.57f, posy - 4.72f, posz, orientation);
                    if (summon)
                        summon->SchoolImmunityList[SCHOOL_FROST] = 1;

                    summon = spawnCreature(CN_PURE_SPAWN_OF_HYDROSS, posx + 12.57f, posy + 4.72f, posz, orientation);
                    if (summon)
                        summon->SchoolImmunityList[SCHOOL_FROST] = 1;
                }
            }

            //Enrage
            if (!Enraged)
            {
                EnrageTimer--;
                if (!EnrageTimer)
                {
                    getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(HYDROSS_ENRAGE), true);
                    Enraged = true;
                }
            }
        }

       
    private:
        int minspell;
        int maxspell;
        bool form; //false = water | true = poison
        uint32 MarkTimer;
        uint32 MarkCount;
        bool Enraged;
        uint32 EnrageTimer;
};

//------------------------------------
//    -= The Lurker Below=-
//------------------------------------
const uint32 WHIRL = 37363;
const uint32 GEYSER = 37478;
const uint32 SPOUT = 37433;
const uint32 SUBMERGE = 37433; // Didn't find the spell id
//const uint32 WATER_BOLT  = 37138;

class LurkerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LurkerAI);
        LurkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            //spells[0].info = sSpellCustomizations.GetSpellInfo(WHIRL);
            //spells[0].targettype = TARGET_ATTACKING;
            //spells[0].instant = true;
            //spells[0].cooldown = 30;
            //spells[0].perctrigger = 10.0f;
            //spells[0].attackstoptimer = 1000; // 1sec
            //m_spellcheck[0] = true;

            //spells[1].info = sSpellCustomizations.GetSpellInfo(GEYSER);
            //spells[1].targettype = TARGET_VARIOUS;
            //spells[1].instant = true;
            //spells[1].cooldown = 10;
            //spells[1].perctrigger = 10.0f;
            //spells[1].attackstoptimer = 2000; // 2sec

            //spells[2].info = sSpellCustomizations.GetSpellInfo(SPOUT);
            //spells[2].targettype = TARGET_ATTACKING;
            //spells[2].instant = false;
            //spells[2].cooldown = 60;
            //spells[2].perctrigger = 5.0f;
            //spells[2].attackstoptimer = 2000; // 2sec

            //spells[3].info = sSpellCustomizations.GetSpellInfo(SUBMERGE);
            //spells[3].targettype = TARGET_SELF;
            //spells[3].instant = true;
            //spells[3].cooldown = 120;
            //spells[3].perctrigger = 10.0f;
            //spells[3].attackstoptimer = 2000; // 2sec
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }
};


/* \todo
 - Some phase timers
 - Insidious whispers
*/
const uint32 MIND_BLAST = 37531;
const uint32 WHIRLWINDLEO = 40653;
const uint32 INSIDIOUS_WHISPER = 37676;
const uint32 CHAOS_BLAST_ANIMATION = 37674;
const uint32 CHAOS_BLAST_EFFECT = 37675;
const uint32 LEO_ENRAGE = 41447; //13048
const uint32 LEO_BANISH = 37546;

uint32 LeotherasEventGreyheartToKill[1000000];

class LeotherasAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LeotherasAI);
        LeotherasAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            ////Insidious Whisper (inner demons)
            ////"We all have our demons..."
            //////Debuff that summons an Inner Demon from up to five raid members. Each Inner Demon can be attacked only by the person it spawned from. If you do not kill your Inner Demon before Leotheras gets back into humanoid form you will become Mind Controlled for 10 minutes and can't get out of it unless killed. Inner Demons take increased damage from arcane, nature, and holy spells.
            //spells[0].info = sSpellCustomizations.GetSpellInfo(INSIDIOUS_WHISPER);
            //spells[0].targettype = TARGET_VARIOUS;
            //spells[0].instant = true;
            //spells[0].perctrigger = 2.0f;
            //spells[0].attackstoptimer = 2000;
            //m_spellcheck[0] = false;

            info_chaos_blast = sSpellCustomizations.GetSpellInfo(CHAOS_BLAST_ANIMATION);
            info_whirlwind = sSpellCustomizations.GetSpellInfo(WHIRLWINDLEO);

            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);

            SwitchTimer = 0;
            WhirlwindTimer = 0;
            EnrageTimer = 0;
            Enraged = false;
            mInWhirlwind = false;
            IsMorphing = false;
            Phase = 0;              //nightelf form
            FinalPhaseSubphase = 0;
            FinalPhaseTimer = 0;

            LeotherasEventGreyheartToKill[getCreature()->GetInstanceID()] = 0;
            FirstCheck();
        }

        void FirstCheck()
        {
            //count greyheart spellbinders
            Creature* creature = NULL;
            for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
            {
                if ((*itr)->IsCreature())
                {
                    creature = static_cast<Creature*>((*itr));

                    if (creature->GetCreatureProperties()->Id == CN_GREYHEART_SPELLBINDER && creature->isAlive())
                        LeotherasEventGreyheartToKill[getCreature()->GetInstanceID()]++;
                }
            }

            //no greyheart spellbinder found, release him
            if (!LeotherasEventGreyheartToKill[getCreature()->GetInstanceID()])
            {
                //remove banish & blocks
                getCreature()->RemoveAllAuras();
                getCreature()->setUInt32Value(UNIT_FIELD_FLAGS, 0);
                getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
                getCreature()->GetAIInterface()->m_canMove = true;
                getCreature()->SetStandState(STANDSTATE_STAND);
            }
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            if (LeotherasEventGreyheartToKill[getCreature()->GetInstanceID()] != 0)
                return;

            SwitchTimer = 40 + Util::getRandomUInt(5); //wowwiki says 45, bosskillers says 40
            WhirlwindTimer = 15;
            EnrageTimer = 599; //10 minutes

            sendDBChatMessage(4772);     // Finally my banishment ends!

            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            //despawn shadow of leotheras
            Creature* shadow = NULL;
            shadow = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), CN_SHADOW_OF_LEOTHERAS);
            if (shadow)
            {
                shadow->Despawn(0, 0);
            }

            SwitchToHumanForm();
        }

        void OnTargetDied(Unit* /*mTarget*/) override
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                if (Phase) //blood elf form
                {
                    switch (Util::getRandomUInt(2))
                    {
                        case 0:
                            sendDBChatMessage(4778);     // Kill! KILL!
                            break;
                        case 1:
                            sendDBChatMessage(4779);     // That's right! Yes!
                            break;
                        case 2:
                            sendDBChatMessage(4780);     // Who's the master now?
                            break;
                    }
                }
                else //demon form
                {
                    switch (Util::getRandomUInt(2))
                    {
                        case 0:
                            sendDBChatMessage(4775);     // I have no equal.
                            break;
                        case 1:
                            sendDBChatMessage(4776);     // Perish, mortal.
                            break;
                        case 2:
                            sendDBChatMessage(4777);     // Yes, YES! Ahahah!
                            break;
                    }
                }
            }
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            sendDBChatMessage(4783);     // You cannot kill me! Fools, I'll be back! I'll... aarghh...
        }

        void SwitchToHumanForm()
        {
            getCreature()->SetDisplayId(20514);
            getCreature()->SetEquippedItem(MELEE, (getCreature()->m_spawn != NULL) ? getCreature()->m_spawn->Item1SlotDisplay : 0);
            getCreature()->SetEquippedItem(OFFHAND, (getCreature()->m_spawn != NULL) ?  getCreature()->m_spawn->Item2SlotDisplay : 0);
        }

        void SwitchToDemonForm()
        {
            getCreature()->SetDisplayId(20125);
            getCreature()->SetEquippedItem(MELEE, 0);
            getCreature()->SetEquippedItem(OFFHAND, 0);
        }

        void AIUpdate() override
        {
            if (Phase == 0 || Phase == 3) //nightelf phase
            {
                if (!IsMorphing)
                {
                    //whirlwind
                    WhirlwindTimer--;
                    if (!WhirlwindTimer)
                    {
                        if (!mInWhirlwind)
                        {
                            getCreature()->CastSpell(getCreature(), info_whirlwind, true);
                            getCreature()->setAttackTimer(15000, false);
                            getCreature()->GetAIInterface()->ClearHateList(); //reset aggro
                            WhirlwindTimer = 15;
                            mInWhirlwind = true;
                        }
                        else
                        {
                            getCreature()->GetAIInterface()->ClearHateList(); //reset aggro
                            WhirlwindTimer = 15;
                            mInWhirlwind = false;
                        }
                    }
                }

                if (Phase == 0)
                {
                    //switch to demon form
                    SwitchTimer--;
                    if (!SwitchTimer)
                    {
                        //switch to AGENT_SPELL
                        setAIAgent(AGENT_SPELL);
                        sendDBChatMessage(4773);     // Be gone trifling elf. I'm in control now
                        SwitchToDemonForm();
                        Phase = 1;
                        SwitchTimer = 60; //60 seconds
                    }

                    //15% Leotheras/Demon split
                    //wait until he returns nightelf (blizzlike)
                    if (getCreature()->GetHealthPct() <= 15 && !mInWhirlwind)
                        Phase = 3;
                }
                else
                {
                    Creature* shadow = NULL;
                    switch (FinalPhaseSubphase)
                    {
                        case 0:
                            IsMorphing = true;
                            getCreature()->setAttackTimer(15000, false);
                            getCreature()->SetStandState(STANDSTATE_KNEEL);
                            getCreature()->setUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
                            getCreature()->GetAIInterface()->m_canMove = false;
                            sendDBChatMessage(4781);     // No... no! What have you done? I am the master! Do you hear me? I am... aaggh! Can't... contain him.
                            FinalPhaseTimer = 10;
                            FinalPhaseSubphase++;
                            break;

                        case 1:
                            FinalPhaseTimer--;
                            if (!FinalPhaseTimer)
                            {
                                getCreature()->SetStandState(STANDSTATE_STAND);
                                shadow = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), CN_SHADOW_OF_LEOTHERAS);

                                if (shadow == NULL)
                                    shadow = spawnCreature(CN_SHADOW_OF_LEOTHERAS, getCreature()->GetPosition());

                                FinalPhaseTimer = 5;
                                FinalPhaseSubphase++;
                            }
                            break;

                        case 2:
                            FinalPhaseTimer--;
                            if (!FinalPhaseTimer)
                            {
                                getCreature()->setUInt32Value(UNIT_FIELD_FLAGS, 0);
                                getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
                                getCreature()->GetAIInterface()->m_canMove = true;
                                IsMorphing = false;
                                FinalPhaseSubphase++;
                            }
                            break;

                        default:
                            break;
                    }
                }

                //Enrage
                if (!Enraged)
                {
                    EnrageTimer--;
                    if (!EnrageTimer)
                    {
                        getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(LEO_ENRAGE), true);
                        Enraged = true;
                    }
                }
            }
            else if (Phase == 1) //demon form
            {
                //Chaos Blast
                if (getCreature()->GetAIInterface()->getNextTarget())
                {
                    if (!getCreature()->isCastingNonMeleeSpell())
                    {
                        if (Util::getRandomUInt(1))
                        {
                            getCreature()->CastSpell(getCreature()->GetAIInterface()->getNextTarget(), info_chaos_blast, false);
                        }
                    }

                    //move if needed
                    if (getCreature()->GetAIInterface()->getNextTarget()->GetDistance2dSq(getCreature()) >= 400) //20 yards
                    {
                        getCreature()->GetAIInterface()->setSplineRun();
                        getCreature()->GetAIInterface()->_CalcDestinationAndMove(getCreature()->GetAIInterface()->getNextTarget(), 5.0f);
                    }
                }

                //switch
                SwitchTimer--;
                if (!SwitchTimer)
                {
                    setAIAgent(AGENT_MELEE);
                    SwitchToHumanForm();
                    Phase = 0;
                    WhirlwindTimer = 10 + Util::getRandomUInt(5);
                    SwitchTimer = 40 + Util::getRandomUInt(5); //wowwiki says 45, bosskillers says 40
                    getCreature()->GetAIInterface()->ClearHateList(); //reset aggro
                }
            }
        }

    protected:
        uint32 SwitchTimer;
        uint32 WhirlwindTimer;
        uint32 EnrageTimer;
        bool Enraged;
        bool mInWhirlwind;
        bool IsMorphing;
        uint32 Phase;
        SpellInfo* info_whirlwind;
        SpellInfo* info_chaos_blast;
        uint32 FinalPhaseSubphase;
        uint32 FinalPhaseTimer;
};

class GreyheartSpellbinderAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GreyheartSpellbinderAI);
        GreyheartSpellbinderAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            /*spells[0].info = sSpellCustomizations.GetSpellInfo(MIND_BLAST);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = true;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 2000;
            m_spellcheck[0] = false;*/

            Unit* Leotheras = NULL;
            Leotheras = getNearestCreature(376.543f, -438.631f, 29.7083f, CN_LEOTHERAS_THE_BLIND);
            if (Leotheras)
            {
                getCreature()->SetChannelSpellTargetGUID(Leotheras->GetGUID());
                getCreature()->SetChannelSpellId(30166);//wrong
            }
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            getCreature()->SetChannelSpellTargetGUID(0);
            getCreature()->SetChannelSpellId(0);

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            LeotherasEventGreyheartToKill[getCreature()->GetInstanceID()]--;

            //start the event
            if (LeotherasEventGreyheartToKill[getCreature()->GetInstanceID()] == 0)
            {
                Unit* Leotheras = NULL;
                Leotheras = getNearestCreature(376.543f, -438.631f, 29.7083f, CN_LEOTHERAS_THE_BLIND);
                if (Leotheras)
                {
                    //remove banish & blocks
                    Leotheras->RemoveAllAuras();
                    Leotheras->setUInt32Value(UNIT_FIELD_FLAGS, 0);
                    Leotheras->GetAIInterface()->SetAllowedToEnterCombat(true);
                    Leotheras->GetAIInterface()->m_canMove = true;
                    Leotheras->SetStandState(STANDSTATE_STAND);

                    //attack nearest player
                    Player* NearestPlayer = NULL;
                    float NearestDist = 0;
                    for (std::set< Object* >::iterator itr = getCreature()->GetInRangePlayerSetBegin(); itr != getCreature()->GetInRangePlayerSetEnd(); ++itr)
                    {
                        if (isHostile(getCreature(), (*itr)) && ((*itr)->GetDistance2dSq(getCreature()) < NearestDist || !NearestDist))
                        {
                            NearestDist = (*itr)->GetDistance2dSq(getCreature());
                            NearestPlayer = static_cast< Player* >(*itr);
                        }
                    }

                    if (NearestPlayer)
                        Leotheras->GetAIInterface()->AttackReaction(NearestPlayer, 1, 0);
                }
            }
        }
};

class ShadowofLeotherasAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ShadowofLeotherasAI);
        ShadowofLeotherasAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            info_chaos_blast = sSpellCustomizations.GetSpellInfo(CHAOS_BLAST_ANIMATION);

            getCreature()->setUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);

            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "At last I am liberated. It has been too long since I have tasted true freedom!");
            getCreature()->PlaySoundToSet(11309);

            sEventMgr.AddEvent(static_cast<Object*>(getCreature()), &Object::EventSetUInt32Value, (uint16)UNIT_FIELD_FLAGS, (uint32)0, EVENT_CREATURE_UPDATE, 7500, 0, 1);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_SPELL);
            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            RemoveAIUpdateEvent();
        }

        void AIUpdate() override
        {
            //Chaos Blast
            if (getCreature()->GetAIInterface()->getNextTarget())
            {
                if (!getCreature()->isCastingNonMeleeSpell())
                {
                    if (Util::getRandomUInt(1))
                    {
                        getCreature()->CastSpell(getCreature()->GetAIInterface()->getNextTarget(), info_chaos_blast, false);
                    }
                }

                //move if needed
                if (getCreature()->GetAIInterface()->getNextTarget()->GetDistance2dSq(getCreature()) >= 400) //20 yards
                {
                    getCreature()->GetAIInterface()->setSplineRun();
                    getCreature()->GetAIInterface()->_CalcDestinationAndMove(getCreature()->GetAIInterface()->getNextTarget(), 5.0f);
                }
            }
        }

    protected:
        SpellInfo* info_chaos_blast;
};



//------------------------------------
//    -= Fathom-Lord Karathress =-
//------------------------------------
const uint32 CATACLYSMIC_BOLT = 38441;
const uint32 BLESSING_OF_THE_TIDES = 38449;
const uint32 KARATHRESS_ENRAGE = 39249;

class KarathressAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(KarathressAI);
        KarathressAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            info_cataclysmic_bolt = sSpellCustomizations.GetSpellInfo(CATACLYSMIC_BOLT);
            AdvisorsLeft = 3;
            BlessingOfTidesCounter = 0;

            EnrageTimer = 0;
            Enraged = false;
            CataclysmicBoltTimer = 0;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            CataclysmicBoltTimer = 10;
            EnrageTimer = 600;
            Enraged = false;
            sendDBChatMessage(4740);     // Guards, attention!We have visitors ...
            RegisterAIUpdateEvent(1000);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            sendDBChatMessage(4748);     // Her ... excellency ... awaits!

            //spawn seer olum and despawn him in 3 minutes
            Creature* olum = NULL;
            olum = spawnCreature(CN_SEER_OLUM, 451.099f, -544.984f, -7.36327f, 0.174533f);
            if (olum)
                olum->Despawn(180000, 0);
        }

        void OnTargetDied(Unit* /*mTarget*/) override
        {
            sendDBChatMessage(4747);     // I am rid of you.
        }

        void AIUpdate() override
        {
            //Cataclysmic Bolt
            CataclysmicBoltTimer--;
            if (CataclysmicBoltTimer == 0)
            {
                // trying to be blizzlike: cast this bolt random on casters only
                CataclysmicBoltTimer = 10;
                Unit* RandomTarget = NULL;
                std::vector<Unit*> TargetTable;
                for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                {
                    if (isHostile(getCreature(), (*itr)) && (*itr)->IsUnit())
                    {
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) <= 80.0f && getCreature()->GetPowerType() == POWER_TYPE_MANA)
                            TargetTable.push_back(RandomTarget);
                    }
                }

                if (!TargetTable.size())
                    return;

                auto random_index = Util::getRandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;
                //let's force this effect
                info_cataclysmic_bolt->setEffectBasePoints(random_target->getUInt32Value(UNIT_FIELD_MAXHEALTH) / 2, 0);
                getCreature()->CastSpell(random_target, info_cataclysmic_bolt, true);
                TargetTable.clear();
            }

            //Blessing of the Tides
            if (getCreature()->GetHealthPct() <= 70 && AdvisorsLeft > 0)
            {
                if (BlessingOfTidesCounter < AdvisorsLeft)
                {
                    sendDBChatMessage(4741);     // Your overconfidence will be your undoing! Guards, lend me your strength!
                    getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(BLESSING_OF_THE_TIDES), true);
                    BlessingOfTidesCounter++;
                }
            }

            //Enrage
            if (!Enraged)
            {
                EnrageTimer--;
                if (!EnrageTimer)
                {
                    getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(KARATHRESS_ENRAGE), true);
                    Enraged = true;
                }
            }
        }

        uint32 AdvisorsLeft;

    private:
        SpellInfo* info_cataclysmic_bolt;
        uint32 CataclysmicBoltTimer;
        uint32 EnrageTimer;
        uint32 BlessingOfTidesCounter;
        bool Enraged;
};

//Fathom-Guard Sharkkis (Hunter)
const uint32 MULTI_SHOT = 29576;
const uint32 LEECHING_THROW = 29436;
const uint32 THE_BEAST_WITHIN = 38373;

class FathomGuardSharkissAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(FathomGuardSharkissAI);
        FathomGuardSharkissAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(MULTI_SHOT, 10.0f, TARGET_ATTACKING);
            addAISpell(LEECHING_THROW, 10.0f, TARGET_ATTACKING);
            addAISpell(THE_BEAST_WITHIN, 10.0f, TARGET_ATTACKING, 0, 40);

            CurrentPet = NULL;
            SummonPetTimer = 0;
        }

        void OnCombatStart(Unit* /*pTarget*/) override
        {
            CurrentPet = NULL;
            SummonPetTimer = 5;
        }

        void OnDied(Unit* /*pKiller*/) override
        {
            Creature* FLK = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), CN_FATHOM_LORD_KARATHRESS);
            if (FLK)
            {
                FLK->CastSpell(FLK, sSpellCustomizations.GetSpellInfo(38455), true); //Power of Sharkkis
                FLK->SendScriptTextChatMessage(4743);     // I am more powerful than ever!
                if (static_cast< KarathressAI* >(FLK->GetScript())->AdvisorsLeft > 0)
                    static_cast< KarathressAI* >(FLK->GetScript())->AdvisorsLeft--;
                FLK->RemoveAura(BLESSING_OF_THE_TIDES);
            }
        }

        void AIUpdate() override
        {
            //Summon Pet
            if (!CurrentPet || !CurrentPet->isAlive())
            {
                SummonPetTimer--;
                if (!SummonPetTimer)
                {
                    switch (Util::getRandomUInt(1))
                    {
                        case 0:
                            CurrentPet = spawnCreature(CN_FATHOM_LURKER, getCreature()->GetPosition());
                            break;
                        case 1:
                            CurrentPet = spawnCreature(CN_FATHOM_SPOREBAT, getCreature()->GetPosition());
                            break;
                    }

                    SummonPetTimer = 5;
                }
            }
        }

    private:
        uint32 SummonPetTimer;
        Creature* CurrentPet;
};

//Fathom-Guard Tidalvess (Shaman)
const uint32 SPITFIRE_TOTEM = 38236;
const uint32 POISON_CLEANSING_TOTEM = 38306;
const uint32 EARTHBIND_TOTEM = 38304;
const uint32 FROST_SHOCK = 38234;
const uint32 WINDFURY = 38229;

class FathomGuardTidalvessAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(FathomGuardTidalvessAI);
        FathomGuardTidalvessAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            //totems
            addAISpell(SPITFIRE_TOTEM, 10.0f, TARGET_SELF);
            addAISpell(POISON_CLEANSING_TOTEM, 10.0f, TARGET_SELF);
            addAISpell(EARTHBIND_TOTEM, 10.0f, TARGET_SELF);

            addAISpell(FROST_SHOCK, 10.0f, TARGET_ATTACKING);
            addAISpell(WINDFURY, 10.0f, TARGET_ATTACKING);
        }

        void OnDied(Unit* /*pKiller*/) override
        {
            Creature* FLK = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), CN_FATHOM_LORD_KARATHRESS);
            if (FLK)
            {
                FLK->CastSpell(FLK, sSpellCustomizations.GetSpellInfo(38452), true); //Power of Tidalvess
                FLK->SendScriptTextChatMessage(4742);     // Go on, kill them! I'll be the better for it!
                if (static_cast< KarathressAI* >(FLK->GetScript())->AdvisorsLeft > 0)
                    static_cast< KarathressAI* >(FLK->GetScript())->AdvisorsLeft--;
                FLK->RemoveAura(BLESSING_OF_THE_TIDES);
            }
        }
};

//Fathom-Guard Caribdis (Priest)
const uint32 HEAL = 33144;
const uint32 TIDAL_SURGE = 38358;
const uint32 SUMMON_CYCLONE = 38337;

class FathomGuardCaribdisAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(FathomGuardCaribdisAI);
        FathomGuardCaribdisAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(TIDAL_SURGE, 20.0f, TARGET_SELF, 0, 10);
            addAISpell(SUMMON_CYCLONE, 2.0f, TARGET_SELF, 0, 0);
            HealingWaveTimer = 0;
        }

        void OnCombatStart(Unit* /*pTarget*/) override
        {
            HealingWaveTimer = 15;
        }

        void OnDied(Unit* /*pKiller*/) override
        {
            Creature* FLK = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), CN_FATHOM_LORD_KARATHRESS);
            if (FLK)
            {
                FLK->CastSpell(FLK, sSpellCustomizations.GetSpellInfo(38451), true); //Power of Caribdis
                FLK->SendScriptTextChatMessage(4744);     // More knowledge, more power!
                if (static_cast< KarathressAI* >(FLK->GetScript())->AdvisorsLeft > 0)
                    static_cast< KarathressAI* >(FLK->GetScript())->AdvisorsLeft--;
                FLK->RemoveAura(BLESSING_OF_THE_TIDES);
            }
        }

    private:
        uint32 HealingWaveTimer;
};


const uint32 TIDAL_WAVE = 37730;
const uint32 WATERY_GRAVE = 38049;
const uint32 EARTHQUAKE = 37764;
const uint32 SUMMON_WATER_GLOBULE = 37854;

class MorogrimAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MorogrimAI);
        MorogrimAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            sendDBChatMessage(4784);     // Flood of the deep, take you!

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            sendDBChatMessage(4792);     // Great... currents of... Ageon.
        }

        void OnTargetDied(Unit* /*mTarget*/) override
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                switch (Util::getRandomUInt(2))
                {
                    case 0:
                        sendDBChatMessage(4791);     // Only the strong survive.
                        break;
                    case 1:
                        sendDBChatMessage(4790);     // Struggling only makes it worse.
                        break;
                    case 2:
                        sendDBChatMessage(4789);     // It is done!
                        getCreature()->PlaySoundToSet(11326);
                        break;
                }
            }
        }
};

class TidewalkerLurkerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TidewalkerLurkerAI);
        TidewalkerLurkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            Unit* target = FindTargetForSpell();
            if (target)
            {
                getCreature()->GetAIInterface()->AttackReaction(target, 1, 0);
            }
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            getCreature()->Despawn(1, 0);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            getCreature()->Despawn(1, 0);
        }

        Unit* FindTargetForSpell()
        {
            Unit* target = NULL;
            float distance = 150.0f;

            Unit* pUnit;
            float dist;

            for (const auto& itr : getCreature()->GetInRangeOppFactsSet())
            {
                if (!itr || !itr->IsUnit())
                    continue;

                pUnit = static_cast<Unit*>(itr);

                if (pUnit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH))
                    continue;

                if (!pUnit->isAlive() || getCreature() == pUnit)
                    continue;

                dist = getCreature()->GetDistance2dSq(pUnit);

                if (dist > distance * distance)
                    continue;

                target = pUnit;
                break;
            }

            return target;
        }
};



/* \todo
 - Toxic Sporebats
 - Coilfang Strider spawn points
 - Some vashj spells and cooldowns are wrong
 - Right Shield generators coords
*/

const uint32 SHOOT = 37770;
const uint32 ENTANGLE = 38316;
const uint32 SPAWN_ELEMENTAL = 37053;
const uint32 EEMULTI_SHOT = 29576;
const uint32 STATIC_CHARGE = 38280;
const uint32 FORKED_LIGHTNING = 40088;
const uint32 VASHJ_SHIELD = 38112;
const uint32 POISON_SPIT = 40078;
const uint32 TOXIC_SPORES = 38575;
const uint32 SHOCK_BLAST = 38509;

class EnchantedElementalAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(EnchantedElementalAI);
        EnchantedElementalAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);

            Unit* Vashj = NULL;
            Vashj = getNearestCreature(29.798161f, -923.358276f, 42.900517f, CN_LADY_VASHJ);
            if (Vashj)
            {
                getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(1);
            }
        }

        void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
        {
            switch (iWaypointId)
            {
                case 1:
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(2);
                    break;

                case 2:
                    Unit* Vashj = NULL;
                    Vashj = getNearestCreature(29.798161f, -923.358276f, 42.900517f, CN_LADY_VASHJ);
                    if (Vashj)
                    {
                        //Increase Lady Vashj attack by 5%
                        Vashj->ModFloatValue(UNIT_FIELD_MINDAMAGE, (Vashj->GetMinDamage() / 100) * 5);
                        Vashj->ModFloatValue(UNIT_FIELD_MAXDAMAGE, (Vashj->GetMaxDamage() / 100) * 5);
                    }

                    //despawn
                    getCreature()->Despawn(1, 0);
                    break;
            }
        }
};

class VashjAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(VashjAI);
        VashjAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            info_multishot = sSpellCustomizations.GetSpellInfo(EEMULTI_SHOT);
            info_shot = sSpellCustomizations.GetSpellInfo(SHOOT);

            Movement::WayPoint* wp = getCreature()->CreateWaypointStruct();
            wp->id = 1;
            wp->x = 29.798161f;
            wp->y = -923.358276f;
            wp->z = 42.900517f;
            wp->o = 0.0f;
            wp->waittime = 0;
            wp->flags = Movement::WP_MOVE_TYPE_RUN;
            wp->forwardemoteoneshot = false;
            wp->forwardemoteid = 0;
            wp->backwardemoteoneshot = false;
            wp->backwardemoteid = 0;
            wp->forwardskinid = 0;
            wp->backwardskinid = 0;

            getCreature()->GetAIInterface()->addWayPoint(wp);
            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);

            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);

            TaintedElementalTimer = 0;
            Phase = 0;
            EnchantedElementalTimer = 0;
            CoilfangStriderTimer = 0;
            CoilfangEliteTimer = 0;
            SporebatTimer = 0;
            ForkedLightningTimer = 0;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            Phase = 1;
            EnchantedElementalTimer = 10;
            CoilfangStriderTimer = 60;
            CoilfangEliteTimer = 40;
            TaintedElementalTimer = 50;
            SporebatTimer = 0;
            ForkedLightningTimer = 5;

            switch (Util::getRandomUInt(3))
            {
                case 0:
                    sendDBChatMessage(4759);     // I'll split you from stem to stern!");
                    break;
                case 1:
                    sendDBChatMessage(4760);     // Victory to Lord Illidan!
                    break;
                case 2:
                    sendDBChatMessage(4761);     // I spit on you, surface filth!
                    break;
                case 3:
                    sendDBChatMessage(4762);     // Death to the outsiders!
                    break;
            }

            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            //despawn enchanted elemental, tainted elemental, coilfang elite, coilfang strider
            Creature* creature = NULL;
            for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
            {
                if ((*itr)->IsCreature())
                {
                    creature = static_cast<Creature*>((*itr));

                    if ((creature->GetCreatureProperties()->Id == CN_ENCHANTED_ELEMENTAL ||
                            creature->GetCreatureProperties()->Id == CN_TAINTED_ELEMENTAL ||
                            creature->GetCreatureProperties()->Id == CN_COILFANG_STRIDER ||
                            creature->GetCreatureProperties()->Id == CN_COILFANG_ELITE ||
                            creature->GetCreatureProperties()->Id == CN_SHIELD_GENERATOR_CHANNEL)
                            && creature->isAlive())
                        creature->Despawn(500, 0);
                }
            }

            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);
            getCreature()->RemoveAura(VASHJ_SHIELD);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
            getCreature()->GetAIInterface()->m_canMove = true;
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            sendDBChatMessage(4771);     // Lord Illidan, I... I am... sorry.
        }

        void OnTargetDied(Unit* /*mTarget*/) override
        {
            switch (Util::getRandomUInt(1))
            {
                case 0:
                    sendDBChatMessage(4768);     // Your time ends now!
                    break;
                case 1:
                    sendDBChatMessage(4769);     // You have failed!
                    break;
            }
        }

        void AIUpdate() override
        {
            switch (Phase)
            {
                case 1:
                case 3:
                    PhaseOneAndThree();
                    break;
                case 2:
                    PhaseTwo();
                    break;
            }
        }

        void PhaseOneAndThree()
        {
            if (Phase != 3)
            {
                if (getCreature()->GetHealthPct() <= 70)
                {
                    getCreature()->RemoveAllAuras();
                    getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
                    getCreature()->GetAIInterface()->StopMovement(0);
                    getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(1);
                    sendDBChatMessage(4764);     // The time is now! Leave none standing!
                    getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                    getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(VASHJ_SHIELD), true);
                    getCreature()->GetAIInterface()->setOutOfCombatRange(3000);
                    Phase = 2;
                }
            }

            //if nobody is in range, shot or multishot
            bool InRange = false;
            for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
            {
                if (isHostile(getCreature(), (*itr)) && getCreature()->GetDistance2dSq((*itr)) < 100) //10 yards
                {
                    InRange = true;
                    break;
                }
            }
            if (!InRange)
            {
                Shoot(getCreature()->GetAIInterface()->getNextTarget());
            }
        }

        void PhaseTwo()
        {
            //WORKAROUND
            getCreature()->setAttackTimer(2000, false);

            //Forked Lightning
            ForkedLightningTimer--;
            if (!ForkedLightningTimer)
            {
                ForkedLightningTimer = 2 + Util::getRandomUInt(5);
            }

            //spawn creatures
            EnchantedElementalTimer--;
            if (!EnchantedElementalTimer)
            {
                uint32 pos = Util::getRandomUInt(7);
                Creature* elemental = NULL;
                elemental = spawnCreature(CN_ENCHANTED_ELEMENTAL, ElementalSpawnPoints[pos].x, ElementalSpawnPoints[pos].y, ElementalSpawnPoints[pos].z, ElementalSpawnPoints[pos].o);
                if (elemental)
                {
                    Movement::WayPoint* wp = getCreature()->CreateWaypointStruct();
                    wp->id = 1;
                    wp->x = ElementalSpawnPoints2[pos].x;
                    wp->y = ElementalSpawnPoints2[pos].y;
                    wp->z = ElementalSpawnPoints2[pos].z;
                    wp->o = ElementalSpawnPoints2[pos].o;
                    wp->waittime = 0;
                    wp->flags = Movement::WP_MOVE_TYPE_WALK;
                    wp->forwardemoteoneshot = false;
                    wp->forwardemoteid = 0;
                    wp->backwardemoteoneshot = false;
                    wp->backwardemoteid = 0;
                    wp->forwardskinid = 0;
                    wp->backwardskinid = 0;
                    elemental->GetAIInterface()->addWayPoint(wp);

                    wp = getCreature()->CreateWaypointStruct();
                    wp->id = 2;
                    wp->x = 29.798161f;
                    wp->y = -923.358276f;
                    wp->z = 42.900517f;
                    wp->o = 0.0f;
                    wp->waittime = 0;
                    wp->flags = Movement::WP_MOVE_TYPE_WALK;
                    wp->forwardemoteoneshot = false;
                    wp->forwardemoteid = 0;
                    wp->backwardemoteoneshot = false;
                    wp->backwardemoteid = 0;
                    wp->forwardskinid = 0;
                    wp->backwardskinid = 0;
                    elemental->GetAIInterface()->addWayPoint(wp);
                }
                EnchantedElementalTimer = 10 + Util::getRandomUInt(5);
            }
            CoilfangStriderTimer--;
            if (!CoilfangStriderTimer)
            {
                Creature* summoned = NULL;
                summoned = spawnCreature(CN_COILFANG_STRIDER, -29.761278f, -980.252930f, 41.097122f, 0.0f);
                if (summoned)
                {
                    //attack nearest target
                    Unit* nearest = NULL;
                    float nearestdist = 0;
                    for (std::set<Object*>::iterator itr = summoned->GetInRangeSetBegin(); itr != summoned->GetInRangeSetEnd(); ++itr)
                    {
                        if ((*itr)->IsUnit() && isHostile(summoned, (*itr)) && (summoned->GetDistance2dSq((*itr)) < nearestdist || !nearestdist))
                        {
                            nearestdist = summoned->GetDistance2dSq((*itr));
                            nearest = static_cast<Unit*>((*itr));
                        }
                    }
                    if (nearest)
                        summoned->GetAIInterface()->AttackReaction(nearest, 1, 0);
                }
                CoilfangStriderTimer = 60;
            }
            CoilfangEliteTimer--;
            if (!CoilfangEliteTimer)
            {
                uint32 pos = Util::getRandomUInt(3);
                Creature* summoned = NULL;
                summoned = spawnCreature(CN_COILFANG_ELITE, CoilfangEliteSpawnPoints[pos].x, CoilfangEliteSpawnPoints[pos].y, CoilfangEliteSpawnPoints[pos].z, CoilfangEliteSpawnPoints[pos].o);
                if (summoned)
                {
                    //attack nearest target
                    Unit* nearest = NULL;
                    float nearestdist = 0;
                    for (std::set<Object*>::iterator itr = summoned->GetInRangeSetBegin(); itr != summoned->GetInRangeSetEnd(); ++itr)
                    {
                        if ((*itr)->IsUnit() && isHostile(summoned, (*itr)) && (summoned->GetDistance2dSq((*itr)) < nearestdist || !nearestdist))
                        {
                            nearestdist = summoned->GetDistance2dSq((*itr));
                            nearest = static_cast<Unit*>((*itr));
                        }
                    }
                    if (nearest)
                        summoned->GetAIInterface()->AttackReaction(nearest, 1, 0);
                }
                CoilfangEliteTimer = 45;
            }
            TaintedElementalTimer--;
            if (!TaintedElementalTimer)
            {
                uint32 pos = Util::getRandomUInt(7);
                spawnCreature(CN_TAINTED_ELEMENTAL, ElementalSpawnPoints[pos].x, ElementalSpawnPoints[pos].y, ElementalSpawnPoints[pos].z, ElementalSpawnPoints[pos].o);
                TaintedElementalTimer = 120;
            }

            if (getCreature()->GetHealthPct() <= 50)
            {
                //despawn enchanted elementals
                Creature* creature = NULL;
                for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                {
                    if ((*itr)->IsCreature())
                    {
                        creature = static_cast<Creature*>((*itr));

                        if (creature->GetCreatureProperties()->Id == CN_ENCHANTED_ELEMENTAL && creature->isAlive())
                            creature->Despawn(0, 0);
                    }
                }

                getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);
                getCreature()->RemoveAura(VASHJ_SHIELD);
                sendDBChatMessage(4765);     // You may want to take cover.
                getCreature()->GetAIInterface()->m_canMove = true;
                Phase = 3;
            }
        }

        void Shoot(Unit* target)
        {
            switch (Util::getRandomUInt(1))
            {
                case 0: //shoot
                    getCreature()->CastSpell(target, info_shot, true);
                    break;
                case 1: //multishot
                    getCreature()->CastSpell(target, info_multishot, true);
                    break;
            }

            switch (Util::getRandomUInt(5))
            {
                case 0:
                    sendDBChatMessage(4766);     // "Straight to the heart!
                    break;
                case 1:
                    sendDBChatMessage(4767);     // Seek your mark!
                    break;
                default:
                    break;
            }
        }

        void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
        {
            switch (iWaypointId)
            {
                case 1:
                    getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                    getCreature()->GetAIInterface()->m_canMove = false;

                    //setup shield
                    Creature* channel = NULL;
                    for (uint8 i = 0; i < 4; i++)
                    {
                        channel = spawnCreature(CN_SHIELD_GENERATOR_CHANNEL, ShieldGeneratorCoords[i][0],  ShieldGeneratorCoords[i][1],  ShieldGeneratorCoords[i][2], 0);
                        if (channel)
                        {
                            channel->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            channel->GetAIInterface()->m_canMove = false;
                            channel->SetChannelSpellTargetGUID(getCreature()->GetGUID());
                            channel->SetChannelSpellId(VASHJ_SHIELD);
                        }
                    }
                    break;
            }
        }

        uint32 TaintedElementalTimer;
        uint32 Phase;

    protected:
        uint32 EnchantedElementalTimer;
        uint32 CoilfangStriderTimer;
        uint32 CoilfangEliteTimer;
        uint32 SporebatTimer;
        uint32 ForkedLightningTimer;
        SpellInfo* info_multishot;
        SpellInfo* info_shot;
};

class TaintedElementalAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TaintedElementalAI);
        TaintedElementalAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            spell_poison_spit = new AI_Spell();
            spell_poison_spit->agent = AGENT_SPELL;
            spell_poison_spit->procChance = 0;
            spell_poison_spit->spell = sSpellCustomizations.GetSpellInfo(POISON_SPIT);
            spell_poison_spit->spellType = STYPE_DAMAGE;
            spell_poison_spit->spelltargetType = TTYPE_SINGLETARGET;
            spell_poison_spit->cooldown = 2000;
            spell_poison_spit->cooldowntime = 0;
            spell_poison_spit->procCount = 0;
            spell_poison_spit->procCounter = 0;
            spell_poison_spit->minrange = 0;
            spell_poison_spit->maxrange = 30;

            getCreature()->GetAIInterface()->m_canMove = false;
        }

        ~TaintedElementalAI()
        {
            if (this->spell_poison_spit)
                delete this->spell_poison_spit;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_SPELL);
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            Creature* Vashj = NULL;
            Vashj = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), CN_LADY_VASHJ);
            if (Vashj)
            {
                if (static_cast< VashjAI* >(Vashj->GetScript())->TaintedElementalTimer > 50)
                    static_cast< VashjAI* >(Vashj->GetScript())->TaintedElementalTimer = 50;
            }
        }

        void AIUpdate() override
        {
            ///\todo  Despawn after 15 secs
            if (!getCreature()->isCastingNonMeleeSpell())
                getCreature()->GetAIInterface()->SetNextSpell(spell_poison_spit);
        }

        void Destroy() override
        {
            // Could someone confirm it's nowhere saved, so we won't have any crashes related
            // to using deleted pointers ?
            if (spell_poison_spit != NULL)
            {
                delete spell_poison_spit;
                spell_poison_spit = NULL;
            }

            delete this;
        }

    private:
        AI_Spell* spell_poison_spit;
};

class TaintedCoreGO : public GameObjectAIScript
{
    public:
        TaintedCoreGO(GameObject* pGameObject) : GameObjectAIScript(pGameObject)
        {
        }

        void OnActivate(Player* pPlayer) override
        {
            Creature* Vashj = NULL;
            Vashj = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(29.798161f, -923.358276f, 42.900517f, CN_LADY_VASHJ);
            if (Vashj != NULL && static_cast< VashjAI* >(Vashj->GetScript())->Phase == 2)
            {
                Vashj->ModHealth(static_cast<int32>((Vashj->getUInt32Value(UNIT_FIELD_MAXHEALTH) / 100) * 5));
                Creature* channel = NULL;
                channel = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), CN_SHIELD_GENERATOR_CHANNEL);
                if (channel != NULL && channel->IsInWorld())
                    channel->Despawn(0, 0);
            }
        }

        static GameObjectAIScript* Create(GameObject* pGameObject) { return new TaintedCoreGO(pGameObject); }
};

class ToxicSporeBatAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ToxicSporeBatAI);
        ToxicSporeBatAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Waypoints
            m_entry = pCreature->GetEntry();
            for (uint8_t i = 0; i < 12; i++)
                AddWaypoint(CreateWaypoint(i, 0, Movement::WP_MOVE_TYPE_FLY, fly[i]));


            /*spells[0].info = sSpellCustomizations.GetSpellInfo(TOXIC_SPORES);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = 0;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;*/

            // Additional Settings

            Phase = 0;
            FlameQuills = false;
            Meteor = false;
            PositionChange = Util::getRandomUInt(15, 23);
            PhoenixSummon = Util::getRandomUInt(17, 23);
            getCreature()->GetAIInterface()->setSplineFlying();
            getCreature()->GetAIInterface()->StopMovement(0);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
            getCreature()->GetAIInterface()->setWayPointToMove(1);
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));

            QuillsCount = 0;
            NextWP = 0;
            FlyWay = 0;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            //_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Phase 1 Test!");
            getCreature()->PlaySoundToSet(11243);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);

            Phase = 1;
            FlameQuills = false;
            Meteor = false;
            PositionChange = Util::getRandomUInt(30, 45);    // 30-45sec /*** if attack time 1000 (%15+31) ***/
            PhoenixSummon = Util::getRandomUInt(34, 44);    // 34-44sec /*** if attack time 1000 (%11+34) ***/
            FlyWay = Util::getRandomUInt(1);
            switch (FlyWay)
            {
                case 0:    // Clock like
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(6);
                    break;

                case 1:    // hmm... other?
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(9);
                    break;
            }
        }


        void OnCombatStop(Unit* /*mTarget*/) override
        {
            Phase = 0;
            FlameQuills = false;
            Meteor = false;
            PhoenixSummon = Util::getRandomUInt(17, 23);
            PositionChange = Util::getRandomUInt(15, 23);
            getCreature()->GetAIInterface()->StopMovement(0);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
            getCreature()->GetAIInterface()->setWayPointToMove(1);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            Phase = 0;
            FlameQuills = false;
            Meteor = false;
            PositionChange = Util::getRandomUInt(15, 23);
            PhoenixSummon = Util::getRandomUInt(17, 23);
        }

        void AIUpdate() override
        {
            if (FlameQuills == true)
            {
                QuillsCount++;
                if (QuillsCount == 9)
                {
                    FlameQuills = false;
                    switch (FlyWay)
                    {
                        case 0:    // Clock like
                            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                            getCreature()->GetAIInterface()->setWayPointToMove(6);
                            break;

                        case 1:    // hmm... other?
                            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                            getCreature()->GetAIInterface()->setWayPointToMove(9);
                            break;
                    }
                }
                //getCreature()->CastSpell(getCreature(), spells[0].info, spells[0].instant);
            }

            if (Meteor != true)
            {
                switch (Phase)
                {
                    case 0:
                        return;
                    case 1:
                        {
                            PhaseOne();
                        }
                        break;
                    case 2:
                        {
                            //PhaseTwo();
                        }
                        break;
                    default:
                        {
                            Phase = 0;
                        }
                }
            }
        }

        void PhaseOne()
        {
            PositionChange--;
            PhoenixSummon--;

            /*if (getCreature()->GetHealthPct() == 0)
            {
                Phase = 2;
                getCreature()->CastSpell(getCreature(), spells[0].info, spells[0].instant);
            }

            if (!PhoenixSummon--)
            {
                getCreature()->CastSpell(getCreature(), spells[0].info, spells[0].instant);
                PhoenixSummon = Util::getRandomUInt(17, 23);
            }*/

            if (!PositionChange)
            {
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(NextWP);
                PositionChange = Util::getRandomUInt(15, 23);    // added 4 sec fit time + time needed to move to next pos.
            }

            else
            {
                uint32 val = Util::getRandomUInt(100);

                if (val > 0 && val < 5)    // Flame Quills wp here!
                {
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(10);
                }
            }
        }

        void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
        {
            if (Phase == 1)
            {
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(6);
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Phase 1 Test!");
                getCreature()->PlaySoundToSet(11243);
            }

            switch (iWaypointId)
            {
                case 1:    // First fly point
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(2);
                    break;

                case 2:
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(3);
                    break;

                case 3:
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(4);
                    break;

                case 4:
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(5);
                    break;

                case 5:
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(1);    // Last fly point (flyback to point 1 - reset)
                    break;

                case 6:
                    {
                        getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                        getCreature()->GetAIInterface()->m_canMove = false;
                        switch (FlyWay)
                        {
                            case 0:
                                NextWP = 7;
                                break;

                            case 1:
                                NextWP = 9;
                                break;
                        }
                    }
                    break;

                case 7:
                    {
                        getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                        getCreature()->GetAIInterface()->m_canMove = false;
                        switch (FlyWay)
                        {
                            case 0:
                                NextWP = 8;
                                break;

                            case 1:
                                NextWP = 6;
                                break;
                        }
                    }
                    break;

                case 8:
                    {
                        getCreature()->GetAIInterface()->m_canMove = false;
                        getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                        switch (FlyWay)
                        {
                            case 0:
                                NextWP = 9;
                                break;

                            case 1:
                                NextWP = 7;
                                break;
                        }
                    }
                    break;

                case 9:
                    {
                        getCreature()->GetAIInterface()->m_canMove = false;
                        getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                        switch (FlyWay)
                        {
                            case 0:
                                NextWP = 6;
                                break;

                            case 1:
                                NextWP = 8;
                                break;
                        }
                    }
                    break;

                case 10:
                    {
                        getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                        if (Phase == 1)
                        {
                            FlameQuills = true;
                            QuillsCount = 0;
                        }

                        if (Phase == 2)
                        {
                            Meteor = true;
                        }

                    }
                    break;
            }
        }

    protected:
        bool FlameQuills;
        uint32 QuillsCount;
        bool Meteor;
        uint32 PositionChange;
        uint32 PhoenixSummon;
        uint32 NextWP;
        uint32 m_entry;
        uint32 FlyWay;
        uint32 Phase;
};



const uint32 CA_MULTI_SHOT = 27021;

class CoilfangAmbusherAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangAmbusherAI);
        CoilfangAmbusherAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(CA_MULTI_SHOT, 10.0f, TARGET_SELF);
        }
};

const uint32 SHADOW_BOLT = 27209;
const uint32 WHIRLWIND_KNOCKBACK = 34109;

class CoilfangFathomWitchAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangFathomWitchAI);
        CoilfangFathomWitchAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(SHADOW_BOLT, 2.0f, TARGET_ATTACKING);
            addAISpell(WHIRLWIND_KNOCKBACK, 2.0f, TARGET_SELF);
        }
};

const uint32 CLEAVE = 38260;

class CoilfangGuardianAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangGuardianAI);
        CoilfangGuardianAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(CLEAVE, 3.0f, TARGET_RANDOM_DESTINATION);
        }
};

const uint32 HOLY_NOVA = 38589;
const uint32 SMITE = 25364;
const uint32 SPIRIT_OF_REDEMPTION = 35618;

class CoilfangPriestessAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangPriestessAI);
        CoilfangPriestessAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(HOLY_NOVA, 2.0f, TARGET_SELF);
            addAISpell(SMITE, 1.0f, TARGET_ATTACKING);
            addAISpell(SPIRIT_OF_REDEMPTION, 2.0f, TARGET_SELF);
        }
};

const uint32 ACID_GEYSER = 37959;
const uint32 ATROPIC_BLOW = 39015;
const uint32 SPORE_QUAKE = 38976;
const uint32 TOXIC_POOL = 38718;
const uint32 FRENZY = 37605;
const uint32 RAMPANT_INFECTION = 39042;
const uint32 PARASITE = 39044;

class UnderbogColossusAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(UnderbogColossusAI);
        UnderbogColossusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            //these mobs pick from a random set of abilities
            switch (Util::getRandomUInt(2))
            {
                case 0:
                    addAISpell(RAMPANT_INFECTION, 5.0f, TARGET_SELF);
                    addAISpell(SPORE_QUAKE, 2.0f, TARGET_SELF);
                    break;
                case 1:
                    addAISpell(ACID_GEYSER, 10.0f, TARGET_RANDOM_DESTINATION);
                    addAISpell(PARASITE, 2.0f, TARGET_ATTACKING);
                    break;
                case 2:
                    addAISpell(FRENZY, 10.0f, TARGET_SELF);
                    break;
            }
        }

        void OnDied(Unit* /*pKiller*/) override
        {
            //There will also be a choice of abilities he might use as he dies:
            switch (Util::getRandomUInt(2))
            {
                case 0:
                    //cast toxic pool
                    getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(TOXIC_POOL), true);
                    break;
                case 1:
                    //spawn two colossus lurkers
                    spawnCreature(22347, getCreature()->GetPosition());
                    spawnCreature(22347, getCreature()->GetPosition());
                    break;
                default:
                    break;

                    //\todo Many small adds
                    //\todo Refreshing mist
            }
        }
};

const uint32 TW_CLEAVE = 38260;
const uint32 TW_BLOODTHIRST = 30335; //INSTANT
const uint32 TW_FRENZY = 37605;

class TidewalkerWarriorAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TidewalkerWarriorAI);
        TidewalkerWarriorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(TW_CLEAVE, 1.0f, TARGET_RANDOM_DESTINATION);
            addAISpell(TW_BLOODTHIRST, 1.0f, TARGET_ATTACKING, 0, 0, false, true);
            addAISpell(TW_FRENZY, 2.0f, TARGET_SELF);
        }
};

const uint32 CSERP_CLEAVE = 38260;
const uint32 CSERP_REFLECTION = 36096;
const uint32 CSERP_DEVOTION = 38603;

class CoilfangSerpentguardAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangSerpentguardAI);
        CoilfangSerpentguardAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(CSERP_CLEAVE, 1.0f, TARGET_RANDOM_DESTINATION);
            addAISpell(CSERP_REFLECTION, 0.5f, TARGET_SELF);
            addAISpell(CSERP_DEVOTION, 1.0f, TARGET_SELF);
        }
};

const uint32 CSHATT_ARMOR = 38591;

class CoilfangShattererAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangShattererAI);
        CoilfangShattererAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(CSHATT_ARMOR, 2.0f, TARGET_ATTACKING);
        }
};

const uint32 CSTRID_SCREAM = 10890;

class CoilfangStriderAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CoilfangStriderAI);
        CoilfangStriderAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(CSTRID_SCREAM, 2.0f, TARGET_ATTACKING);
        }
};

class SerpentshrineCavern : public InstanceScript
{
    public:
        // Console & Bridge parts
        uint32 mBridgePart[3];

        SerpentshrineCavern(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
        {
            for (uint8 i = 0; i < 3; ++i)
                mBridgePart[i] = 0;
        }

        static InstanceScript* Create(MapMgr* pMapMgr) { return new SerpentshrineCavern(pMapMgr); }

        void OnGameObjectPushToWorld(GameObject* pGameObject) override
        {
            switch (pGameObject->GetEntry())
            {
                case 184203:
                    mBridgePart[0] = pGameObject->GetLowGUID();
                    break;
                case 184204:
                    mBridgePart[1] = pGameObject->GetLowGUID();
                    break;
                case 184205:
                    mBridgePart[2] = pGameObject->GetLowGUID();
                    break;
            }
        }

        void OnGameObjectActivate(GameObject* pGameObject, Player* /*pPlayer*/) override
        {
            if (pGameObject->GetGameObjectProperties()->entry != 184568)
                return;

            GameObject* pBridgePart = NULL;

            for (uint8 i = 0; i < 3; ++i)
            {
                pBridgePart = GetGameObjectByGuid(mBridgePart[i]);
                if (pBridgePart != NULL)
                    pBridgePart->SetState(GO_STATE_OPEN);
            }

            pGameObject->SetState(GO_STATE_OPEN);
        }
};

void SetupSerpentshrineCavern(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_HYDROSS_THE_UNSTABLE, &HydrossTheUnstableAI::Create);
    mgr->register_creature_script(CN_THE_LURKER_BELOW, &LurkerAI::Create);

    //Leotheras the Blind event
    mgr->register_creature_script(CN_LEOTHERAS_THE_BLIND, &LeotherasAI::Create);
    mgr->register_creature_script(CN_GREYHEART_SPELLBINDER, &GreyheartSpellbinderAI::Create);
    mgr->register_creature_script(CN_SHADOW_OF_LEOTHERAS, &ShadowofLeotherasAI::Create);


    //Morogrim Tidewalker event
    mgr->register_creature_script(CN_MOROGRIM_TIDEWALKER, &MorogrimAI::Create);
    mgr->register_creature_script(CN_TIDEWALKER_LURKER, &TidewalkerLurkerAI::Create);

    //Fathom-Lord Karathress event
    mgr->register_creature_script(CN_FATHOM_LORD_KARATHRESS, &KarathressAI::Create);
    mgr->register_creature_script(CN_FATHOM_GUARD_CARIBDIS, &FathomGuardCaribdisAI::Create);
    mgr->register_creature_script(CN_FATHOM_GUARD_TIDALVESS, &FathomGuardTidalvessAI::Create);
    mgr->register_creature_script(CN_FATHOM_GUARD_SHARKKIS, &FathomGuardSharkissAI::Create);

    //Lady Vashj event
    mgr->register_creature_script(CN_LADY_VASHJ, &VashjAI::Create);
    //mgr->register_creature_script(CN_TOXIC_SPORE_BAT, &ToxicSporeBatAI::Create);
    mgr->register_creature_script(CN_COILFANG_STRIDER, &CoilfangStriderAI::Create);
    mgr->register_creature_script(CN_ENCHANTED_ELEMENTAL, &EnchantedElementalAI::Create);
    mgr->register_creature_script(CN_TAINTED_ELEMENTAL, &TaintedElementalAI::Create);

    // Shield Generator
    mgr->register_gameobject_script(185051, &TaintedCoreGO::Create);

    // Serpentsrine Cavern instance script
#ifndef UseNewMapScriptsProject
    mgr->register_instance_script(MAP_CF_SERPENTSHRINE_CA, &SerpentshrineCavern::Create);
#endif

    //Trash mobs
    mgr->register_creature_script(CN_COILFANG_AMBUSHER, &CoilfangAmbusherAI::Create);
    mgr->register_creature_script(CN_COILFANG_FATHOM_WITCH, &CoilfangFathomWitchAI::Create);
    mgr->register_creature_script(CN_COILFANG_GUARDIAN, &CoilfangGuardianAI::Create);
    mgr->register_creature_script(CN_COILFANG_PRIESTESS, &CoilfangPriestessAI::Create);
    mgr->register_creature_script(CN_UNDERBOG_COLOSSUS, &UnderbogColossusAI::Create);
    mgr->register_creature_script(CN_TIDEWALKER_WARRIOR, &TidewalkerWarriorAI::Create);
    mgr->register_creature_script(CN_COILFANG_SERPENTGUARD, &CoilfangSerpentguardAI::Create);
    mgr->register_creature_script(CN_COILFANG_SHATTERER, &CoilfangShattererAI::Create);
}
