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
class WatchkeeperGargolmarAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(WatchkeeperGargolmarAI, MoonScriptCreatureAI);
    WatchkeeperGargolmarAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
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
    }

    void OnCombatStart(Unit* pTarget)
    {
        switch (RandomUInt(2))
        {
            case 0:
                sendDBChatMessage(4873);     // What have we here?
                break;
            case 1:
                sendDBChatMessage(4874);     // This may hurt a little....
                break;
            case 2:
                sendDBChatMessage(4875);     // I'm going to enjoy this...
                break;
        }
    }

    void OnTargetDied(Unit* pTarget)
    {
        switch (RandomUInt(1))
        {
            case 0:
                sendDBChatMessage(4876);     // Say farewell!
                break;
            case 1:
                sendDBChatMessage(4877);     // Much too easy.
                break;
        }
    }

    void OnDied(Unit* mKiller)
    {
        sendDBChatMessage(4878);      // Hahah.. <cough> ..argh!
        ParentClass::OnDied(mKiller);
    }

    void AIUpdate()
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

        ParentClass::AIUpdate();
    }

    bool mCalledForHelp;
    bool _retaliation;
    SpellDesc* mRetaliation;
};


//Omor the Unscarred
class OmorTheUnscarredAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(OmorTheUnscarredAI, MoonScriptCreatureAI);
        OmorTheUnscarredAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            SpellDesc* pShield = AddSpell(OMOR_DEMONIC_SHIELD, Target_Self, 30, 0, 25);
            pShield->mEnabled = false;
            SpellDesc* pSummon = AddSpell(OMOR_SUMMON_FIENDISH_HOUND, Target_Self, 8, 1, 20);
            pSummon->AddEmote("Achor-she-ki! Feast my pet! Eat your fill!", CHAT_MSG_MONSTER_YELL, 10277);
            AddSpell(OMOR_SHADOW_WHIP, Target_RandomPlayer, 10, 0, 30);
            if (!_isHeroic())
            {
                AddSpell(OMOR_SHADOW_BOLT, Target_RandomPlayer, 8, 3, 15, 10, 60, true);
                SpellDesc* pAura = AddSpell(OMOR_TREACHEROUS_AURA, Target_RandomPlayer, 8, 2, 35, 0, 60, true);
                pAura->AddEmote("A-Kreesh!", CHAT_MSG_MONSTER_YELL, 10278);
            }
            else
            {
                AddSpell(OMOR_SHADOW_BOLT2, Target_RandomPlayer, 8, 3, 15, 10, 60, true);
                SpellDesc* pAura = AddSpell(OMOR_BANE_OF_TREACHERY, Target_RandomPlayer, 8, 2, 35, 0, 60, true);
                pAura->AddEmote("A-Kreesh!", CHAT_MSG_MONSTER_YELL, 10278);
            }
        }

        void OnCombatStart(Unit* pTarget)
        {
            switch (RandomUInt(2))
            {
                case 0:
                    sendDBChatMessage(4856);     // I will not be defeated!
                    break;
                case 1:
                    sendDBChatMessage(4855);     // You dare stand against ME?
                    break;
                case 2:
                    sendDBChatMessage(4857);     // Your insolence will be your death!
                    break;
            }
            ParentClass::OnCombatStart(pTarget);
            setRooted(true);
        }

        void OnTargetDied(Unit* pKiller)
        {
            sendDBChatMessage(4860);     // Die, weakling!
        }

        void OnDied(Unit* pKiller)
        {
            sendDBChatMessage(4861);     // It is... not over.
        }

        void OnCombatStop(Unit* pTarget)
        {
            ParentClass::OnCombatStop(pTarget);
            if (isAlive())
            {
                sendDBChatMessage(4862);     // I am victorious!
            }
        }

        void AIUpdate()
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

            ParentClass::AIUpdate();
            setRooted(true);
    }
};

void SetupHellfireRamparts(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_WATCHKEEPER_GARGOLMAR, &WatchkeeperGargolmarAI::Create);
    mgr->register_creature_script(CN_OMOR_THE_UNSCARRED, &OmorTheUnscarredAI::Create);
}
