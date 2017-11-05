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
#include "Raid_OnyxiasLair.h"


// This script covers Onyxia's mind
class OnyxiaAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(OnyxiaAI);
        OnyxiaAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            m_phase = 1;
            m_entry = pCreature->GetEntry();
            m_useSpell = true;
            m_eFlamesCooldown = 1;
            m_whelpCooldown = 7;
            m_aoeFearCooldown = 30;
            m_fCastCount = 5;
            _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(1, 2000, Movement::WP_MOVE_TYPE_RUN));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(2, 0, Movement::WP_MOVE_TYPE_FLY));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(3, 0, Movement::WP_MOVE_TYPE_FLY));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(4, 0, Movement::WP_MOVE_TYPE_FLY));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(5, 0, Movement::WP_MOVE_TYPE_FLY));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(6, 0, Movement::WP_MOVE_TYPE_FLY));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(7, 0, Movement::WP_MOVE_TYPE_FLY));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(8, 0, Movement::WP_MOVE_TYPE_FLY));

            infoFear = sSpellCustomizations.GetSpellInfo(AOE_FEAR);
            infoCleave = sSpellCustomizations.GetSpellInfo(CLEAVE);
            infoFBreath = sSpellCustomizations.GetSpellInfo(FLAME_BREATH);
            infoKAway = sSpellCustomizations.GetSpellInfo(KNOCK_AWAY);
            infoSFireball = sSpellCustomizations.GetSpellInfo(SCRIPTABLE_FIREBALL);
            infoWBuffet = sSpellCustomizations.GetSpellInfo(WING_BUFFET);
            infoDeepBreath = sSpellCustomizations.GetSpellInfo(DEEP_BREATH);

            if (!infoFear || !infoCleave || !infoFBreath
                    || !infoKAway || !infoSFireball || !infoWBuffet || !infoDeepBreath)
                m_useSpell = false;

            _unit->GetAIInterface()->setOutOfCombatRange(200000);

            m_fBreath = false;
            m_kAway = false;
            m_wBuffet = false;
            m_Cleave = false;
            m_currentWP = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            m_phase = 1;
            m_eFlamesCooldown = 1;
            m_whelpCooldown = 7;
            _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
            _unit->SetStandState(STANDSTATE_STAND);
            sendDBChatMessage(1725);     //How fortuitous, usually I must leave my lair to feed!
            if (m_useSpell)
                RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));

            m_fBreath = false;
            m_kAway = false;
            m_wBuffet = false;
            m_Cleave = false;
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
            _unit->GetAIInterface()->setWayPointToMove(0);
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
            _unit->GetAIInterface()->unsetSplineFlying();
            _unit->GetAIInterface()->m_canMove = true;
            _unit->SetStandState(STANDSTATE_SLEEP);
            /*if (_unit->m_pacified > 0)
                _unit->m_pacified--;*/
            if (m_useSpell)
                RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            if (m_useSpell)
                RemoveAIUpdateEvent();

            m_phase = 1;
            m_eFlamesCooldown = 1;
            m_whelpCooldown = 7;

            /*Add Loot?
            mKiller->
            */
        }

        void OnReachWP(uint32 iWaypointId, bool bForwards)
        {
            switch (iWaypointId)
            {
                case 1:
                    {
                        _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        _unit->GetAIInterface()->setWayPointToMove(2);
                        Fly();
                    }
                    break;
                case 2:
                    {
                        _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        _unit->GetAIInterface()->setWayPointToMove(3);
                    }
                    break;
                case 3:
                    {
                        _unit->GetAIInterface()->m_canMove = false;
                        _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
                        setAIAgent(AGENT_SPELL);
                        //_unit->m_pacified--;
                        _unit->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                        _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                        _unit->GetAIInterface()->setWayPointToMove(0);

                        _unit->setMoveHover(true);
                        m_currentWP = 3;
                    }
                    break;
                case 8:
                    {
                        _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
                        setAIAgent(AGENT_NULL);
                        _unit->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                        _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                        _unit->GetAIInterface()->setWayPointToMove(0);
                        /*_unit->m_pacified--;
                        if (_unit->m_pacified > 0)
                            _unit->m_pacified--;*/
                        _unit->setMoveHover(false);
                        Land();
                    }
                    break;
                default:
                    {
                        _unit->GetAIInterface()->m_canMove = false;
                        _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
                        _unit->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                        _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                        _unit->GetAIInterface()->setWayPointToMove(0);

                        _unit->setMoveHover(true);
                        //_unit->m_pacified--;
                    }
                    break;
            }
        }

        void AIUpdate()
        {
            switch (m_phase)
            {
                case 1:
                    {
                        PhaseOne();
                    }
                    break;
                case 2:
                    {
                        PhaseTwo();
                    }
                    break;
                case 3:
                    {
                        PhaseThree();
                    }
                    break;
                default:
                    {
                        m_phase = 1;
                    }
            }
        }

        void PhaseOne()
        {
            if (_unit->GetHealthPct() <= 65)
            {
                m_phase = 2;
                _unit->SetCastSpeedMod(0.01f);
                if (_unit->GetCurrentSpell() != NULL)
                    _unit->GetCurrentSpell()->cancel();

                _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
                //_unit->m_pacified++;
                _unit->GetAIInterface()->StopMovement(0);
                _unit->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                _unit->GetAIInterface()->setWayPointToMove(1);

                return;
            }
            uint32 val = RandomUInt(1000);
            SpellCast(val);
        }

        void PhaseTwo()
        {
            if (_unit->GetHealthPct() <= 40)
            {
                m_phase = 3;
                _unit->SetCastSpeedMod(1.0f);
                if (_unit->GetCurrentSpell() != NULL)
                    _unit->GetCurrentSpell()->cancel();
                _unit->GetAIInterface()->m_canMove = true;
                _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
                //_unit->m_pacified++;
                _unit->GetAIInterface()->StopMovement(0);
                _unit->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                _unit->GetAIInterface()->setWayPointToMove(8);

                return;
            }
            if (_unit->GetAIInterface()->getWaypointScriptType() == Movement::WP_MOVEMENT_SCRIPT_WANTEDWP)
                return;
            m_eFlamesCooldown--;
            if (!m_eFlamesCooldown && _unit->GetAIInterface()->getNextTarget())//_unit->getAttackTarget())
            {
                _unit->CastSpell(_unit->GetAIInterface()->getNextTarget(), infoSFireball, false);//(_unit->getAttackTarget(),
                m_eFlamesCooldown = 4;
                m_fCastCount--;
            }
            if (!m_fCastCount)
            {
                uint32 val = RandomUInt(1250);
                if (val < 250)//Move left
                {
                    m_currentWP++;
                    if (m_currentWP >= 8)
                        m_currentWP = 3;

                    _unit->GetAIInterface()->m_canMove = true;
                    _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
                    //_unit->m_pacified++;
                    _unit->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                    _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    _unit->GetAIInterface()->setWayPointToMove(m_currentWP);
                    m_fCastCount = 5;
                }
                else if (val > 1000)//Move right
                {
                    m_currentWP--;
                    if (m_currentWP < 3)
                        m_currentWP = 7;

                    _unit->GetAIInterface()->m_canMove = true;
                    _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
                    //_unit->m_pacified++;
                    _unit->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                    _unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    _unit->GetAIInterface()->setWayPointToMove(m_currentWP);
                    m_fCastCount = 5;
                }
                else if (val < 350)
                {
                    //Deep breath
                    _unit->CastSpell(_unit, infoDeepBreath, false);
                    m_fCastCount = 5;
                }
                else
                    m_fCastCount = 5;
            }
            m_whelpCooldown--;
            if (!m_whelpCooldown)
            {
                Creature* cre = NULL;
                for (uint8 i = 0; i < 6; i++)
                {
                    cre = _unit->GetMapMgr()->GetInterface()->SpawnCreature(11262,
                            whelpCoords[i].x, whelpCoords[i].y,
                            whelpCoords[i].z, whelpCoords[i].o,
                            true, false, 0, 0);
                    if (cre)
                    {
                        cre->GetAIInterface()->MoveTo(14.161f, -177.874f, -85.649f);
                        cre->SetOrientation(0.23f);
                        cre->GetAIInterface()->setOutOfCombatRange(100000);
                    }
                    cre = _unit->GetMapMgr()->GetInterface()->SpawnCreature(11262,
                            whelpCoords[5 - i].x, whelpCoords[5 - i].y,
                            whelpCoords[5 - i].z, whelpCoords[5 - i].o,
                            true, false, 0, 0);
                    if (cre)
                    {
                        cre->GetAIInterface()->MoveTo(27.133f, -232.030f, -84.188f);
                        cre->SetOrientation(0.44f);
                        cre->GetAIInterface()->setOutOfCombatRange(100000);
                    }
                }
                m_whelpCooldown = 30;
            }
        }

        void PhaseThree()
        {
            if (!m_aoeFearCooldown)
            {
                _unit->CastSpell(_unit, infoFear, false);//(_unit->getAttackTarget(),
                m_aoeFearCooldown = 30;
                return;
            }
            uint32 val = RandomUInt(1000);
            SpellCast(val);
            m_whelpCooldown--;
            m_aoeFearCooldown--;
            if (!m_whelpCooldown)
            {
                Creature* cre = NULL;
                for (uint8 i = 0; i < 6; i++)
                {
                    cre = _unit->GetMapMgr()->GetInterface()->SpawnCreature(11262,
                            whelpCoords[i].x, whelpCoords[i].y,
                            whelpCoords[i].z, whelpCoords[i].o,
                            true, false, 0, 0);
                    if (cre)
                    {
                        cre->GetAIInterface()->MoveTo(14.161f, -177.874f, -85.649f);
                        cre->SetOrientation(0.23f);
                        cre->GetAIInterface()->setOutOfCombatRange(100000);
                    }
                    cre = _unit->GetMapMgr()->GetInterface()->SpawnCreature(11262,
                            whelpCoords[5 - i].x, whelpCoords[5 - i].y,
                            whelpCoords[5 - i].z, whelpCoords[5 - i].o,
                            true, false, 0, 0);
                    if (cre)
                    {
                        cre->GetAIInterface()->MoveTo(27.133f, -232.030f, -84.188f);
                        cre->SetOrientation(0.23f);
                        cre->GetAIInterface()->setOutOfCombatRange(100000);
                    }
                }
                m_whelpCooldown = 300;
            }
        }

        inline Movement::WayPoint* CreateWaypoint(int id, uint32 waittime, uint32 flags)
        {
            //WayPoint* wp = new WayPoint;
            //WayPoint * wp = _unit->GetMapMgr()->GetInterface()->CreateWaypoint();
            //WayPoint * wp = sStructFactory.CreateWaypoint();
            Movement::WayPoint* wp = _unit->CreateWaypointStruct();
            wp->id = id;
            wp->x = coords[id].x;
            wp->y = coords[id].y;
            wp->z = coords[id].z;
            wp->o = coords[id].o;
            wp->waittime = waittime;
            wp->flags = flags;
            wp->forwardemoteoneshot = false;
            wp->forwardemoteid = 0;
            wp->backwardemoteoneshot = false;
            wp->backwardemoteid = 0;
            wp->forwardskinid = 0;
            wp->backwardskinid = 0;
            return wp;
        }

        void Fly()
        {
            _unit->Emote(EMOTE_ONESHOT_LIFTOFF);
            //Do we need hover really? Check it :D
            //_unit->SetHover(true);
            _unit->GetAIInterface()->setSplineFlying();
        }

        void Land()
        {
            _unit->Emote(EMOTE_ONESHOT_LAND);
            //Do we need hover really? Check it :D
            //_unit->SetHover(false);
            _unit->GetAIInterface()->unsetSplineFlying();
        }

        void SpellCast(uint32 val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())//_unit->getAttackTarget())
            {
                if (m_fBreath)
                {
                    _unit->CastSpell(_unit, infoFBreath, false);
                    m_fBreath = false;
                    return;
                }
                else if (m_kAway)
                {
                    _unit->CastSpell(_unit->GetAIInterface()->getNextTarget(), infoKAway, false);
                    m_kAway = false;
                    return;
                }
                else if (m_wBuffet)
                {
                    _unit->CastSpell(_unit, infoWBuffet, false);
                    m_wBuffet = false;
                    return;
                }
                else if (m_Cleave)
                {
                    _unit->CastSpell(_unit->GetAIInterface()->getNextTarget(), infoCleave, false);
                    m_Cleave = false;
                    return;
                }

                if (val >= 100 && val <= 225)
                {
                    _unit->setAttackTimer(6000, false);//6000
                    m_fBreath = true;
                    //_unit->CastSpell(_unit, infoFBreath, false);
                }
                else if (val > 225 && val <= 300)
                {
                    _unit->setAttackTimer(4000, false);//2000
                    m_kAway = true;
                    //_unit->CastSpell(_unit->GetAIInterface()->GetNextTarget(), infoKAway, false);
                }
                else if (val > 300 && val <= 375)
                {
                    _unit->setAttackTimer(4000, false);//3000
                    m_wBuffet = true;
                    //_unit->CastSpell(_unit, infoWBuffet, false);
                }
                else if (val > 375 && val < 450)
                {
                    _unit->setAttackTimer(4000, false);//2000
                    m_Cleave = true;
                    // _unit->CastSpell(_unit->GetAIInterface()->GetNextTarget(), infoCleave, false);
                }
            }
        }

        inline bool HasEntry() { return (m_entry != 0) ? true : false; }

    protected:

        bool m_fBreath;
        bool m_kAway;
        bool m_wBuffet;
        bool m_Cleave;
        uint32 m_entry;
        uint32 m_phase;
        bool m_useSpell;
        uint32 m_eFlamesCooldown;
        uint32 m_whelpCooldown;
        uint32 m_aoeFearCooldown;
        uint32 m_fCastCount;
        uint32 m_currentWP;
        SpellInfo* infoFear, *infoWBuffet, *infoCleave, *infoFBreath, *infoKAway, *infoSFireball, *infoDeepBreath;
};

void SetupOnyxiasLair(ScriptMgr* mgr)
{
    // Onyxia
    mgr->register_creature_script(CN_ONYXIA, &OnyxiaAI::Create);
}
