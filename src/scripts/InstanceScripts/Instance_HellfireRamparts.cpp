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
#include "Instance_HellfireRamparts.h"


// Watchkeeper GargolmarAI
/// \todo "Do you smell that? Fresh meat has somehow breached our citadel. Be wary of any intruders." should be on some areatrigger
class WatchkeeperGargolmarAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(WatchkeeperGargolmarAI);
    WatchkeeperGargolmarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(WATCHKEEPER_SURGE, Target_RandomUnit, 20, 0, 15, 5, 40, false, "Back off, pup!", CHAT_MSG_MONSTER_YELL, 10330);
        AddSpell(WATCHKEEPER_OVERPOWER, Target_Current, 10, 0, 5);
        mRetaliation = AddSpell(WATCHKEEPER_RETALIATION, Target_Self, 0, 0, 0);

        if (_isHeroic())
            AddSpell(WATCHKEEPER_MORTAL_WOUND_H, Target_Current, 15, 0, 12);
        else
            AddSpell(WATCHKEEPER_MORTAL_WOUND, Target_Current, 15, 0, 12);

        mCalledForHelp = 0;
        _retaliation = false;

        // new
        addEmoteForEvent(Event_OnCombatStart, 4873);     // What have we here?
        addEmoteForEvent(Event_OnCombatStart, 4874);     // This may hurt a little....
        addEmoteForEvent(Event_OnCombatStart, 4875);     // I'm going to enjoy this...
        addEmoteForEvent(Event_OnTargetDied, 4876);     // Say farewell!
        addEmoteForEvent(Event_OnTargetDied, 4877);     // Much too easy.
        addEmoteForEvent(Event_OnDied, 4878);      // Hahah.. <cough> ..argh!
    }

    //case for scriptPhase
    void AIUpdate() override
    {
        if (getCreature()->GetHealthPct() <= 40 && !mCalledForHelp)
        {
            sendDBChatMessage(4871);      // Heal me, quickly!
            mCalledForHelp = true;
        }

        if (getCreature()->GetHealthPct() <= 20 && !_retaliation)
        {
            _retaliation = true;
            getCreature()->setAttackTimer(1500, false);
            CastSpellNowNoScheduling(mRetaliation);
        }

        
    }

    bool mCalledForHelp;
    bool _retaliation;
    SpellDesc* mRetaliation;
};


//Omor the Unscarred
class OmorTheUnscarredAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(OmorTheUnscarredAI);
        OmorTheUnscarredAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            SpellDesc* pShield = AddSpell(OMOR_DEMONIC_SHIELD, Target_Self, 30, 0, 25);
            pShield->mEnabled = false;
            SpellDesc* pSummon = AddSpell(OMOR_SUMMON_FIENDISH_HOUND, Target_Self, 8, 1, 20);
            pSummon->addEmote("Achor-she-ki! Feast my pet! Eat your fill!", CHAT_MSG_MONSTER_YELL, 10277);
            AddSpell(OMOR_SHADOW_WHIP, Target_RandomPlayer, 10, 0, 30);
            if (!_isHeroic())
            {
                AddSpell(OMOR_SHADOW_BOLT, Target_RandomPlayer, 8, 3, 15, 10, 60, true);
                SpellDesc* pAura = AddSpell(OMOR_TREACHEROUS_AURA, Target_RandomPlayer, 8, 2, 35, 0, 60, true);
                pAura->addEmote("A-Kreesh!", CHAT_MSG_MONSTER_YELL, 10278);
            }
            else
            {
                AddSpell(OMOR_SHADOW_BOLT2, Target_RandomPlayer, 8, 3, 15, 10, 60, true);
                SpellDesc* pAura = AddSpell(OMOR_BANE_OF_TREACHERY, Target_RandomPlayer, 8, 2, 35, 0, 60, true);
                pAura->addEmote("A-Kreesh!", CHAT_MSG_MONSTER_YELL, 10278);
            }

            // new
            addEmoteForEvent(Event_OnCombatStart, 4856);     // I will not be defeated!
            addEmoteForEvent(Event_OnCombatStart, 4855);     // You dare stand against ME?
            addEmoteForEvent(Event_OnCombatStart, 4857);     // Your insolence will be your death!
            addEmoteForEvent(Event_OnTargetDied, 4860);     // Die, weakling!
            addEmoteForEvent(Event_OnDied, 4861);     // It is... not over.
        }

        void OnCombatStart(Unit* /*pTarget*/) override
        {           
            setRooted(true);
        }

        void OnCombatStop(Unit* /*pTarget*/) override
        {
            if (isAlive())
            {
                sendDBChatMessage(4862);     // I am victorious!
            }
        }

        void AIUpdate() override
        {
            SpellDesc* pShield = FindSpellById(OMOR_DEMONIC_SHIELD);
            if (_getHealthPercent() <= 20 && pShield != NULL && !pShield->mEnabled)
            {
                pShield->mEnabled = true;
            }

            Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
            if (pTarget != NULL)
            {
                if (getRangeToObject(pTarget) > 10.0f)
                {
                    pTarget = GetBestPlayerTarget(TargetFilter_Closest);
                    if (pTarget != NULL)
                    {
                        if (getRangeToObject(pTarget) > 10.0f)
                        {
                            pTarget = NULL;
                        }
                        else
                        {
                            _clearHateList();
                            getCreature()->GetAIInterface()->AttackReaction(pTarget, 500);
                            getCreature()->GetAIInterface()->setNextTarget(pTarget);
                        }
                    }
                    else
                        return;
                }

                if (pTarget == NULL)
                {
                    SpellDesc* pWhip = FindSpellById(OMOR_SHADOW_WHIP);    // used for now
                    if (pWhip != NULL)
                    {
                        pWhip->mLastCastTime = 0;
                        CastSpellNowNoScheduling(pWhip);
                        return;
                    }
                }
            }

            
            setRooted(true);
    }
};

void SetupHellfireRamparts(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_WATCHKEEPER_GARGOLMAR, &WatchkeeperGargolmarAI::Create);
    mgr->register_creature_script(CN_OMOR_THE_UNSCARRED, &OmorTheUnscarredAI::Create);
}
