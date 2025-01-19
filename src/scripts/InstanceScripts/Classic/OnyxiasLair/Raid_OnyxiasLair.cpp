/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Raid_OnyxiasLair.h"

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Utilities/Random.hpp"

class OnyxiasLairInstanceScript : public InstanceScript
{
public:
    explicit OnyxiasLairInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) { }
    static InstanceScript* Create(WorldMap* pMapMgr) { return new OnyxiasLairInstanceScript(pMapMgr); }
};

class OnyxiaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OnyxiaAI(c); }
    explicit OnyxiaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_phase = 1;
        m_entry = pCreature->getEntry();
        m_useSpell = true;
        m_eFlamesCooldown = 1;
        m_whelpCooldown = 7;
        m_aoeFearCooldown = 30;
        m_fCastCount = 5;

        stopMovement();
        addWaypoint(1, createWaypoint(1, 2000, WAYPOINT_MOVE_TYPE_RUN, OnyxiasLair::coords[1]));
        addWaypoint(1, createWaypoint(2, 0, WAYPOINT_MOVE_TYPE_TAKEOFF, OnyxiasLair::coords[2]));
        addWaypoint(1, createWaypoint(3, 0, WAYPOINT_MOVE_TYPE_TAKEOFF, OnyxiasLair::coords[3]));
        addWaypoint(1, createWaypoint(4, 0, WAYPOINT_MOVE_TYPE_TAKEOFF, OnyxiasLair::coords[4]));
        addWaypoint(1, createWaypoint(5, 0, WAYPOINT_MOVE_TYPE_TAKEOFF, OnyxiasLair::coords[5]));
        addWaypoint(1, createWaypoint(6, 0, WAYPOINT_MOVE_TYPE_TAKEOFF, OnyxiasLair::coords[6]));
        addWaypoint(1, createWaypoint(7, 0, WAYPOINT_MOVE_TYPE_TAKEOFF, OnyxiasLair::coords[7]));
        addWaypoint(1, createWaypoint(8, 0, WAYPOINT_MOVE_TYPE_TAKEOFF, OnyxiasLair::coords[8]));

        infoFear = sSpellMgr.getSpellInfo(OnyxiasLair::AOE_FEAR);
        infoCleave = sSpellMgr.getSpellInfo(OnyxiasLair::CLEAVE);
        infoFBreath = sSpellMgr.getSpellInfo(OnyxiasLair::FLAME_BREATH);
        infoKAway = sSpellMgr.getSpellInfo(OnyxiasLair::KNOCK_AWAY);
        infoSFireball = sSpellMgr.getSpellInfo(OnyxiasLair::SCRIPTABLE_FIREBALL);
        infoWBuffet = sSpellMgr.getSpellInfo(OnyxiasLair::WING_BUFFET);
        infoDeepBreath = sSpellMgr.getSpellInfo(OnyxiasLair::DEEP_BREATH);

        if (!infoFear || !infoCleave || !infoFBreath
                || !infoKAway || !infoSFireball || !infoWBuffet || !infoDeepBreath)
            m_useSpell = false;

        // todo: add boundary
        //getCreature()->getAIInterface()->setOutOfCombatRange(200000);

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
        stopMovement();
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
        stopMovement();
        setWaypointToMove(1, 0);

        getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
        getCreature()->setMoveCanFly(false);
        getCreature()->setControlled(false, UNIT_STATE_ROOTED);
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

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        switch (iWaypointId)
        {
            case 1:
                {
                    setWaypointToMove(1, 2);
                    Fly();
                }
                break;
            case 2:
                {
                    setWaypointToMove(1, 3);
                }
                break;
            case 3:
                {
                    getCreature()->setControlled(true, UNIT_STATE_ROOTED);
                    getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
                    setAIAgent(AGENT_SPELL);
                    //_unit->m_pacified--;
                    stopMovement();
                    setWaypointToMove(1, 0);

                    getCreature()->setMoveHover(true);
                    m_currentWP = 3;
                }
                break;
            case 8:
                {
                    getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
                    setAIAgent(AGENT_NULL);
                    stopMovement();
                    setWaypointToMove(1, 0);
                    /*_unit->m_pacified--;
                    if (_unit->m_pacified > 0)
                        _unit->m_pacified--;*/
                    getCreature()->setMoveHover(false);
                    Land();
                }
                break;
            default:
                {
                    getCreature()->setControlled(true, UNIT_STATE_ROOTED);
                    getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
                    stopMovement();
                    setWaypointToMove(1, 0);

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

            getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
            //_unit->m_pacified++;
            getCreature()->stopMoving();
            setWaypointToMove(1, 1);

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
            getCreature()->setControlled(false, UNIT_STATE_ROOTED);
            getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
            //_unit->m_pacified++;
            getCreature()->stopMoving();
            setWaypointToMove(1, 8);

            return;
        }
        m_eFlamesCooldown--;
        if (!m_eFlamesCooldown && getCreature()->getThreatManager().getCurrentVictim())//_unit->getAttackTarget())
        {
            getCreature()->castSpell(getCreature()->getThreatManager().getCurrentVictim(), infoSFireball, false);//(_unit->getAttackTarget(),
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

                getCreature()->setControlled(false, UNIT_STATE_ROOTED);
                getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
                //_unit->m_pacified++;
                setWaypointToMove(1, m_currentWP);
                m_fCastCount = 5;
            }
            else if (val > 1000)//Move right
            {
                m_currentWP--;
                if (m_currentWP < 3)
                    m_currentWP = 7;

                getCreature()->setControlled(false, UNIT_STATE_ROOTED);
                getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
                //_unit->m_pacified++;
                setWaypointToMove(1, m_currentWP);
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
                cre = spawnCreature(11262, OnyxiasLair::whelpCoords[i].x, OnyxiasLair::whelpCoords[i].y, OnyxiasLair::whelpCoords[i].z, OnyxiasLair::whelpCoords[i].o);
                if (cre)
                {
                    cre->getAIInterface()->moveTo(14.161f, -177.874f, -85.649f);
                    cre->SetOrientation(0.23f);
                    // todo: add boundary
                    //cre->getAIInterface()->setOutOfCombatRange(100000);
                }
                cre = spawnCreature(11262, OnyxiasLair::whelpCoords[5 - i].x, OnyxiasLair::whelpCoords[5 - i].y, OnyxiasLair::whelpCoords[5 - i].z, OnyxiasLair::whelpCoords[5 - i].o);
                if (cre)
                {
                    cre->getAIInterface()->moveTo(27.133f, -232.030f, -84.188f);
                    cre->SetOrientation(0.44f);
                    // todo: add boundary
                    //cre->getAIInterface()->setOutOfCombatRange(100000);
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
                cre = spawnCreature(11262, OnyxiasLair::whelpCoords[i].x, OnyxiasLair::whelpCoords[i].y, OnyxiasLair::whelpCoords[i].z, OnyxiasLair::whelpCoords[i].o);
                if (cre)
                {
                    cre->getAIInterface()->moveTo(14.161f, -177.874f, -85.649f);
                    cre->SetOrientation(0.23f);
                    // todo: add boundary
                    //cre->getAIInterface()->setOutOfCombatRange(100000);
                }
                cre = spawnCreature(11262, OnyxiasLair::whelpCoords[5 - i].x, OnyxiasLair::whelpCoords[5 - i].y, OnyxiasLair::whelpCoords[5 - i].z, OnyxiasLair::whelpCoords[5 - i].o);
                if (cre)
                {
                    cre->getAIInterface()->moveTo(27.133f, -232.030f, -84.188f);
                    cre->SetOrientation(0.23f);
                    // todo: add boundary
                    //cre->getAIInterface()->setOutOfCombatRange(100000);
                }
            }
            m_whelpCooldown = 300;
        }
    }

    void Fly()
    {
        getCreature()->emote(EMOTE_ONESHOT_LIFTOFF);
        //Do we need hover really? Check it :D
        //_unit->SetHover(true);
        getCreature()->setMoveCanFly(true);
    }

    void Land()
    {
        getCreature()->emote(EMOTE_ONESHOT_LAND);
        //Do we need hover really? Check it :D
        //_unit->SetHover(false);
        getCreature()->setMoveCanFly(false);
    }

    void SpellCast(uint32_t val)
    {
        if (!getCreature()->isCastingSpell() && getCreature()->getThreatManager().getCurrentVictim())//_unit->getAttackTarget())
        {
            if (m_fBreath)
            {
                getCreature()->castSpell(getCreature(), infoFBreath, false);
                m_fBreath = false;
                return;
            }
            if (m_kAway)
            {
                getCreature()->castSpell(getCreature()->getThreatManager().getCurrentVictim(), infoKAway, false);
                m_kAway = false;
                return;
            }
            if (m_wBuffet)
            {
                getCreature()->castSpell(getCreature(), infoWBuffet, false);
                m_wBuffet = false;
                return;
            }
            if (m_Cleave)
            {
                getCreature()->castSpell(getCreature()->getThreatManager().getCurrentVictim(), infoCleave, false);
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
                //_unit->castSpell(_unit->getAIInterface()->GetNextTarget(), infoKAway, false);
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
                // _unit->castSpell(_unit->getAIInterface()->GetNextTarget(), infoCleave, false);
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
    mgr->register_creature_script(OnyxiasLair::CN_ONYXIA, &OnyxiaAI::Create);
}
