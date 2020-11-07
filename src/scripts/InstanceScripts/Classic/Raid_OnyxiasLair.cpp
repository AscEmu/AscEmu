/*
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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

class OnyxiasLairInstanceScript : public InstanceScript
{
public:

    explicit OnyxiasLairInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new OnyxiasLairInstanceScript(pMapMgr); }


};

// This script covers Onyxia's mind
class OnyxiaAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(OnyxiaAI)
    explicit OnyxiaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_phase = 1;
        m_entry = pCreature->getEntry();
        m_useSpell = true;
        m_eFlamesCooldown = 1;
        m_whelpCooldown = 7;
        m_aoeFearCooldown = 30;
        m_fCastCount = 5;

        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        AddWaypoint(CreateWaypoint(1, 2000, Movement::WP_MOVE_TYPE_RUN, coords[1]));
        AddWaypoint(CreateWaypoint(2, 0, Movement::WP_MOVE_TYPE_FLY, coords[2]));
        AddWaypoint(CreateWaypoint(3, 0, Movement::WP_MOVE_TYPE_FLY, coords[3]));
        AddWaypoint(CreateWaypoint(4, 0, Movement::WP_MOVE_TYPE_FLY, coords[4]));
        AddWaypoint(CreateWaypoint(5, 0, Movement::WP_MOVE_TYPE_FLY, coords[5]));
        AddWaypoint(CreateWaypoint(6, 0, Movement::WP_MOVE_TYPE_FLY, coords[6]));
        AddWaypoint(CreateWaypoint(7, 0, Movement::WP_MOVE_TYPE_FLY, coords[7]));
        AddWaypoint(CreateWaypoint(8, 0, Movement::WP_MOVE_TYPE_FLY, coords[8]));

        infoFear = sSpellMgr.getSpellInfo(AOE_FEAR);
        infoCleave = sSpellMgr.getSpellInfo(CLEAVE);
        infoFBreath = sSpellMgr.getSpellInfo(FLAME_BREATH);
        infoKAway = sSpellMgr.getSpellInfo(KNOCK_AWAY);
        infoSFireball = sSpellMgr.getSpellInfo(SCRIPTABLE_FIREBALL);
        infoWBuffet = sSpellMgr.getSpellInfo(WING_BUFFET);
        infoDeepBreath = sSpellMgr.getSpellInfo(DEEP_BREATH);

        if (!infoFear || !infoCleave || !infoFBreath
                || !infoKAway || !infoSFireball || !infoWBuffet || !infoDeepBreath)
            m_useSpell = false;

        getCreature()->GetAIInterface()->setOutOfCombatRange(200000);

        m_fBreath = false;
        m_kAway = false;
        m_wBuffet = false;
        m_Cleave = false;
        m_currentWP = 0;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        m_phase = 1;
        m_eFlamesCooldown = 1;
        m_whelpCooldown = 7;
        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        getCreature()->setStandState(STANDSTATE_STAND);
        sendDBChatMessage(1725);     //How fortuitous, usually I must leave my lair to feed!
        if (m_useSpell)
            RegisterAIUpdateEvent(getCreature()->getBaseAttackTime(MELEE));

        m_fBreath = false;
        m_kAway = false;
        m_wBuffet = false;
        m_Cleave = false;
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        getCreature()->GetAIInterface()->setWayPointToMove(0);

        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
        getCreature()->GetAIInterface()->unsetSplineFlying();
        getCreature()->GetAIInterface()->m_canMove = true;
        getCreature()->setStandState(STANDSTATE_SLEEP);
        /*if (_unit->m_pacified > 0)
            _unit->m_pacified--;*/
        if (m_useSpell)
            RemoveAIUpdateEvent();
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        m_phase = 1;
        m_eFlamesCooldown = 1;
        m_whelpCooldown = 7;
    }

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        switch (iWaypointId)
        {
            case 1:
                {
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(2);
                    Fly();
                }
                break;
            case 2:
                {
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(3);
                }
                break;
            case 3:
                {
                    getCreature()->GetAIInterface()->m_canMove = false;
                    getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
                    setAIAgent(AGENT_SPELL);
                    //_unit->m_pacified--;
                    getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                    getCreature()->GetAIInterface()->setWayPointToMove(0);

                    getCreature()->setMoveHover(true);
                    m_currentWP = 3;
                }
                break;
            case 8:
                {
                    getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
                    setAIAgent(AGENT_NULL);
                    getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                    getCreature()->GetAIInterface()->setWayPointToMove(0);
                    /*_unit->m_pacified--;
                    if (_unit->m_pacified > 0)
                        _unit->m_pacified--;*/
                    getCreature()->setMoveHover(false);
                    Land();
                }
                break;
            default:
                {
                    getCreature()->GetAIInterface()->m_canMove = false;
                    getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
                    getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                    getCreature()->GetAIInterface()->setWayPointToMove(0);

                    getCreature()->setMoveHover(true);
                    //_unit->m_pacified--;
                }
                break;
        }
    }

    void AIUpdate() override
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
        if (getCreature()->getHealthPct() <= 65)
        {
            m_phase = 2;
            getCreature()->setModCastSpeed(0.01f);
            if (getCreature()->isCastingSpell())
                getCreature()->interruptSpell();

            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            //_unit->m_pacified++;
            getCreature()->GetAIInterface()->StopMovement(0);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
            getCreature()->GetAIInterface()->setWayPointToMove(1);

            return;
        }
        uint32_t val = Util::getRandomUInt(1000);
        SpellCast(val);
    }

    void PhaseTwo()
    {
        if (getCreature()->getHealthPct() <= 40)
        {
            m_phase = 3;
            getCreature()->setModCastSpeed(1.0f);
            if (getCreature()->isCastingSpell())
                getCreature()->interruptSpell();
            getCreature()->GetAIInterface()->m_canMove = true;
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            //_unit->m_pacified++;
            getCreature()->GetAIInterface()->StopMovement(0);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
            getCreature()->GetAIInterface()->setWayPointToMove(8);

            return;
        }
        if (getCreature()->GetAIInterface()->getWaypointScriptType() == Movement::WP_MOVEMENT_SCRIPT_WANTEDWP)
            return;
        m_eFlamesCooldown--;
        if (!m_eFlamesCooldown && getCreature()->GetAIInterface()->getNextTarget())//_unit->getAttackTarget())
        {
            getCreature()->castSpell(getCreature()->GetAIInterface()->getNextTarget(), infoSFireball, false);//(_unit->getAttackTarget(),
            m_eFlamesCooldown = 4;
            m_fCastCount--;
        }
        if (!m_fCastCount)
        {
            uint32_t val = Util::getRandomUInt(1250);
            if (val < 250)//Move left
            {
                m_currentWP++;
                if (m_currentWP >= 8)
                    m_currentWP = 3;

                getCreature()->GetAIInterface()->m_canMove = true;
                getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
                //_unit->m_pacified++;
                getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(m_currentWP);
                m_fCastCount = 5;
            }
            else if (val > 1000)//Move right
            {
                m_currentWP--;
                if (m_currentWP < 3)
                    m_currentWP = 7;

                getCreature()->GetAIInterface()->m_canMove = true;
                getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
                //_unit->m_pacified++;
                getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(m_currentWP);
                m_fCastCount = 5;
            }
            else if (val < 350)
            {
                //Deep breath
                getCreature()->castSpell(getCreature(), infoDeepBreath, false);
                m_fCastCount = 5;
            }
            else
                m_fCastCount = 5;
        }
        m_whelpCooldown--;
        if (!m_whelpCooldown)
        {
            Creature* cre = NULL;
            for (uint8_t i = 0; i < 6; i++)
            {
                cre = spawnCreature(11262, whelpCoords[i].x, whelpCoords[i].y, whelpCoords[i].z, whelpCoords[i].o);
                if (cre)
                {
                    cre->GetAIInterface()->MoveTo(14.161f, -177.874f, -85.649f);
                    cre->SetOrientation(0.23f);
                    cre->GetAIInterface()->setOutOfCombatRange(100000);
                }
                cre = spawnCreature(11262, whelpCoords[5 - i].x, whelpCoords[5 - i].y, whelpCoords[5 - i].z, whelpCoords[5 - i].o);
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
            getCreature()->castSpell(getCreature(), infoFear, false);//(_unit->getAttackTarget(),
            m_aoeFearCooldown = 30;
            return;
        }
        uint32_t val = Util::getRandomUInt(1000);
        SpellCast(val);
        m_whelpCooldown--;
        m_aoeFearCooldown--;
        if (!m_whelpCooldown)
        {
            Creature* cre = NULL;
            for (uint8_t i = 0; i < 6; i++)
            {
                cre = spawnCreature(11262, whelpCoords[i].x, whelpCoords[i].y, whelpCoords[i].z, whelpCoords[i].o);
                if (cre)
                {
                    cre->GetAIInterface()->MoveTo(14.161f, -177.874f, -85.649f);
                    cre->SetOrientation(0.23f);
                    cre->GetAIInterface()->setOutOfCombatRange(100000);
                }
                cre = spawnCreature(11262, whelpCoords[5 - i].x, whelpCoords[5 - i].y, whelpCoords[5 - i].z, whelpCoords[5 - i].o);
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

    void Fly()
    {
        getCreature()->Emote(EMOTE_ONESHOT_LIFTOFF);
        //Do we need hover really? Check it :D
        //_unit->SetHover(true);
        getCreature()->GetAIInterface()->setSplineFlying();
    }

    void Land()
    {
        getCreature()->Emote(EMOTE_ONESHOT_LAND);
        //Do we need hover really? Check it :D
        //_unit->SetHover(false);
        getCreature()->GetAIInterface()->unsetSplineFlying();
    }

    void SpellCast(uint32_t val)
    {
        if (!getCreature()->isCastingSpell() && getCreature()->GetAIInterface()->getNextTarget())//_unit->getAttackTarget())
        {
            if (m_fBreath)
            {
                getCreature()->castSpell(getCreature(), infoFBreath, false);
                m_fBreath = false;
                return;
            }
            else if (m_kAway)
            {
                getCreature()->castSpell(getCreature()->GetAIInterface()->getNextTarget(), infoKAway, false);
                m_kAway = false;
                return;
            }
            else if (m_wBuffet)
            {
                getCreature()->castSpell(getCreature(), infoWBuffet, false);
                m_wBuffet = false;
                return;
            }
            else if (m_Cleave)
            {
                getCreature()->castSpell(getCreature()->GetAIInterface()->getNextTarget(), infoCleave, false);
                m_Cleave = false;
                return;
            }

            if (val >= 100 && val <= 225)
            {
                getCreature()->setAttackTimer(MELEE, 6000);//6000
                m_fBreath = true;
                //_unit->castSpell(_unit, infoFBreath, false);
            }
            else if (val > 225 && val <= 300)
            {
                getCreature()->setAttackTimer(MELEE, 4000);//2000
                m_kAway = true;
                //_unit->castSpell(_unit->GetAIInterface()->GetNextTarget(), infoKAway, false);
            }
            else if (val > 300 && val <= 375)
            {
                getCreature()->setAttackTimer(MELEE, 4000);//3000
                m_wBuffet = true;
                //_unit->castSpell(_unit, infoWBuffet, false);
            }
            else if (val > 375 && val < 450)
            {
                getCreature()->setAttackTimer(MELEE, 4000);//2000
                m_Cleave = true;
                // _unit->castSpell(_unit->GetAIInterface()->GetNextTarget(), infoCleave, false);
            }
        }
    }

    inline bool HasEntry() { return (m_entry != 0) ? true : false; }

protected:

    bool m_fBreath;
    bool m_kAway;
    bool m_wBuffet;
    bool m_Cleave;
    uint32_t m_entry;
    uint32_t m_phase;
    bool m_useSpell;
    uint32_t m_eFlamesCooldown;
    uint32_t m_whelpCooldown;
    uint32_t m_aoeFearCooldown;
    uint32_t m_fCastCount;
    uint32_t m_currentWP;
    SpellInfo const* infoFear, *infoWBuffet, *infoCleave, *infoFBreath, *infoKAway, *infoSFireball, *infoDeepBreath;
};

void SetupOnyxiasLair(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ONYXIAS_LAIR, &OnyxiasLairInstanceScript::Create);

    // Onyxia
    mgr->register_creature_script(CN_ONYXIA, &OnyxiaAI::Create);
}
