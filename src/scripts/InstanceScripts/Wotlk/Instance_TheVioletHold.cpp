/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TheVioletHold.h"
#include "Server/Script/CreatureAIScript.h"

enum DataIndex
{
    TVH_PHASE_1 = 0, // main event
    TVH_PHASE_2 = 1, // 1. portal
    TVH_PHASE_3 = 2, // 2. portal
    TVH_PHASE_4 = 3, // 3. portal
    TVH_PHASE_5 = 4, // 4. portal
    TVH_PHASE_6 = 5, // 5. portal
    TVH_PHASE_DONE = 6, // 6. portal

    TVH_END = 7
};

//////////////////////////////////////////////////////////////////////////////////////////
// TheVioletHold Instance
class TheVioletHoldScript : public InstanceScript
{
friend class SinclariGossip; // Friendship forever ;-)
    uint32_t m_phaseData[TVH_END];
    uint32_t m_lastState = InvalidState;

    // NotStarted
    int32_t S0_SpawnIntroMobsTimer = 0;   // Spawn mobs every 15s

    // PreProgress
    int32_t S1_GuardFleeTimer = -1;       // Delay guards fleeing room for 2.5s (arbitrary)

public:
    explicit TheVioletHoldScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        for (uint8_t i = 0; i < TVH_END; ++i)
            m_phaseData[i] = NotStarted;

        setBossState(MAP_VIOLET_HOLD, NotStarted);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheVioletHoldScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }

    void UpdateEvent() override
    {
        auto state = getBossState(MAP_VIOLET_HOLD);

        if (state != m_lastState)
        {
            OnStateChange(m_lastState, state);
            m_lastState = state;
        }

        switch (state)
        {
            case NotStarted:
                S0_ReviveGuards();
                S0_SpawnIntroMobs();
                S0_RemoveDeadIntroMobs();
                break;
            case InProgress:
                S2_SpawnPortals();
                break;
            case Performed: printf("State: %s\n", "State_Finished"); break;
            case PreProgress:
                S1_ActivateCrystalFleeRoom();
                break;
        }
    }

    void S0_ReviveGuards()
    {
        auto guards = this->getCreatureSetForEntry(CN_VIOLET_HOLD_GUARD);
        for (auto guard : guards)
        {
            if (guard == nullptr || guard->isAlive())
            {
                continue;
            }

            guard->Despawn(VH_TIMER_GUARD_DESPAWN_TIME, VH_TIMER_GUARD_RESPAWN_TIME);
        }
    }

    void S0_RemoveDeadIntroMobs()
    {
        auto introMobs = this->getCreatureSetForEntries(std::vector<uint32_t> { CN_INTRO_AZURE_BINDER_ARCANE, CN_INTRO_AZURE_INVADER_ARMS, CN_INTRO_AZURE_MAGE_SLAYER_MELEE, CN_INTRO_AZURE_SPELLBREAKER_ARCANE });
        for (auto mob : introMobs)
        {
            if (mob == nullptr || mob->isAlive())
                continue;

            mob->Despawn(1500, 0);
        }
    }


    void S0_SpawnIntroMobs()
    {
        if (isTimerFinished(S0_SpawnIntroMobsTimer))
        {
            S0_SpawnIntroMobsTimer = 0; // This forces a new timer to be started below
            
            spawnCreature(GetRandomIntroMob(), IntroPortals[0].x, IntroPortals[0].y, IntroPortals[0].z, IntroPortals[0].o);
            spawnCreature(GetRandomIntroMob(), IntroPortals[1].x, IntroPortals[1].y, IntroPortals[1].z, IntroPortals[1].o);
            spawnCreature(GetRandomIntroMob(), IntroPortals[2].x, IntroPortals[2].y, IntroPortals[2].z, IntroPortals[2].o);
        }

        // Start another 15s timer
        if (getTimeForTimer(S0_SpawnIntroMobsTimer) <= 0)
        {
            S0_SpawnIntroMobsTimer = addTimer(VH_TIMER_SPAWN_INTRO_MOB);
        }
    }

    void S1_ActivateCrystalFleeRoom()
    {
        // TODO: activate crystal

        if (S1_GuardFleeTimer == -1)
        {
            S1_GuardFleeTimer = addTimer(VH_TIMER_GUARD_FLEE_DELAY); // arbitrary time

        }

        if (getTimeForTimer(S1_GuardFleeTimer) > 0)
        {
            return; // Wait for timer to finish
        }

        auto npcs = this->getCreatureSetForEntry(CN_VIOLET_HOLD_GUARD);
        for (auto guard : npcs)
        {
            if (!guard->IsInWorld())
                continue;
        }
    }

    static void S2_SpawnPortals()
    {

        // TODO: Spawn any portals that are missing
    }

    static void S2_SpawnMobs()
    {
        // TODO: Spawn any mobs that are missing
        // TODO: Move this logic to the portals
    }

    static void S2_UpdateDoorDamage()
    {
        // TODO: Update damage done to the door
        // TODO: Move this logic to the mobs
    }

    static void S3_UnlockDoorAndMoveNPCs()
    {
        // TODO: Open door
        // TODO: Move NPCs into room
    }

    static void S3_SpawnGuards()
    {
        // TODO: Respawn guards that are missing
    }

    static void ActivateCrystal()
    {
        // get all mobs
        // play GO anim
        // kill them after delay
        // TODO: Move this logic to the gameobjects
    }

    void DespawnIntroPortals()
    {
        auto portals = this->getCreatureSetForEntry(CN_PORTAL_INTRO);
        for (auto portal : portals)
        {
            portal->Despawn(VH_TIMER_INTRO_PORTAL_DESPAWN_TIME, 0);
        }
    }

    int GetRandomIntroMob() const
    {
        auto rnd = Util::getRandomFloat(100.0f);
        if (rnd < 25.0f)
            return CN_INTRO_AZURE_BINDER_ARCANE;
        if (rnd < 50.f)
            return CN_INTRO_AZURE_INVADER_ARMS;
        if (rnd < 75.0f)
            return CN_INTRO_AZURE_MAGE_SLAYER_MELEE;

        return CN_INTRO_AZURE_SPELLBREAKER_ARCANE;
    }

    void OnStateChange(uint32_t /*pLastState*/, uint32_t pNewState)
    {
        switch (pNewState)
        {
            case PreProgress:
                DespawnIntroPortals();
                break;
        }
    }

    void OnPlayerEnter(Player* pPlayer) override
    {
        TheVioletHoldScript* pInstance = (TheVioletHoldScript*)pPlayer->getWorldMap()->getScript();
        if (!pInstance)
            return;

        if (pInstance->getBossState(MAP_VIOLET_HOLD) == NotStarted)
        {
            setBossState(MAP_VIOLET_HOLD, PreProgress);
        }
    }
};

#define SINCLARI_SAY_1 "Prison guards, we are leaving! These adventurers are taking over! Go go go!"
#define SINCLARY_SAY_2 "I'm locking the door. Good luck, and thank you for doing this."

class SinclariAI : public CreatureAIScript
{
public:
    explicit SinclariAI(Creature* pCreature) : CreatureAIScript(pCreature) {}
    static CreatureAIScript* Create(Creature* creature) { return new SinclariAI(creature); }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        switch (iWaypointId)
        {
        case 2:
        {
            OnRescuePrisonGuards();
        } break;
        case 4:
        {
            getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, SINCLARY_SAY_2);
            getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
        } break;
        case 5:
        {
            TheVioletHoldScript* pInstance = (TheVioletHoldScript*)getCreature()->getWorldMap()->getScript();
            pInstance->setBossState(608, InProgress);
            GameObject* pVioletHoldDoor = pInstance->getClosestGameObjectForPosition(191723, 1822.59f, 803.93f, 44.36f);
            if (pVioletHoldDoor != nullptr)
                pVioletHoldDoor->setState(GO_STATE_CLOSED);
        } break;
        }
    }

    void OnRescuePrisonGuards()
    {
        getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, SINCLARI_SAY_1);

        TheVioletHoldScript* pInstance = (TheVioletHoldScript*)getCreature()->getWorldMap()->getScript();
        if (!pInstance)
            return;
    }
};

class SinclariGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* pPlayer) override
    {
        TheVioletHoldScript* pInstance = (TheVioletHoldScript*)pPlayer->getWorldMap()->getScript();
        if (!pInstance)
            return;

        //Page 1: Textid and first menu item
        if (pInstance->getBossState(608) == PreProgress)
        {
            GossipMenu menu(pObject->getGuid(), 13853, 0);
            menu.addItem(GOSSIP_ICON_CHAT, (600), 1);
            menu.sendGossipPacket(pPlayer);
        }

        //If VioletHold is started, Sinclari has this item for people who aould join.
        if (pInstance->getBossState(608) == InProgress)
        {
            GossipMenu menu(pObject->getGuid(), 13853, 0);
            menu.addItem(GOSSIP_ICON_CHAT, (602), 3);
            menu.sendGossipPacket(pPlayer);
        }
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        TheVioletHoldScript* pInstance = (TheVioletHoldScript*)pPlayer->getWorldMap()->getScript();
        if (!pInstance)
            return;

        if (!pObject->isCreature())
            return;

        switch (Id)
        {
            case 1:
            {
                GossipMenu menu(pObject->getGuid(), 13854, 0);
                menu.addItem(GOSSIP_ICON_CHAT, (601), 2);
                menu.sendGossipPacket(pPlayer);
            } break;
            case 2:
            {
                static_cast<Creature*>(pObject)->setNpcFlags(UNIT_NPC_FLAG_NONE);
            } break;
            case 3:
            {
                GossipMenu::senGossipComplete(pPlayer);
                pPlayer->safeTeleport(pPlayer->GetInstanceID(), 608, LocationVector(1830.531006f, 803.939758f, 44.340508f, 6.281611f));
            } break;
        }
    }
};

class VHGuardsAI : public CreatureAIScript
{
public:
    explicit VHGuardsAI(Creature* pCreature) : CreatureAIScript(pCreature) {}
    static CreatureAIScript* Create(Creature* creature) { return new VHGuardsAI(creature); }
};

//\TODO: Replace spell casting logic for all instances, this is temp
class VHCreatureAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VHCreatureAI(c); }
    explicit VHCreatureAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //this->createWaypoint(1, 0, 0, VH_DOOR_ATTACK_POSITION);
        //this->SetWaypointToMove(1);
        //this->moveTo(VH_DOOR_ATTACK_POSITION.x, VH_DOOR_ATTACK_POSITION.y, VH_DOOR_ATTACK_POSITION.z, true);
        //_unit->getAIInterface()->UpdateMove();
        for (int i = 1; i < 3; ++i)
        {
            addWaypoint(1, createWaypoint(i, 0, WAYPOINT_MOVE_TYPE_RUN, AttackerWP[i]));
        }
        getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "I am alive!");
    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        switch (iWaypointId)
        {
            case 1:
                getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Reached wp 1!");
                setWaypointToMove(1, 2);
                break;
            case 2:
            {
                if (m_isIntroMob)
                {
                    getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Reached wp 2!");
                    getCreature()->Despawn(500, 0);
                }
                else
                {
                        // TODO: Door attack code
                }
            }break;
        }
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        PutAllSpellsOnCooldown();
        RegisterAIUpdateEvent(getCreature()->getBaseAttackTime(MELEE));
    }

    void PutAllSpellsOnCooldown()
    {
        /*for (int i = 0; i < m_spellCount; i++)
        m_spells[i].casttime = m_spells[i].cooldown;*/
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        PutAllSpellsOnCooldown();
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        PutAllSpellsOnCooldown();
    }

    void AIUpdate() override
    {
        /*auto randomValue = Util::getRandomFloat(100.0f);
        SpellCast(randomValue);*/
    }
    
        /*void SpellCast(float randomValue)
        {
            if (!getCreature()->isCastingSpell() && getCreature()->getThreatManager().getCurrentVictim())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (int i = 0; i < m_spellCount; i++)
                {
                    m_spells[i].casttime--;

                    if (m_spellsEnabled[i])
                    {
                        if (!m_spells[i].instant)
                        {
                            this->stopMovement();
                        }

                        m_spells[i].casttime = m_spells[i].cooldown;
                        target = getCreature()->getThreatManager().getCurrentVictim();
                        switch (m_spells[i].targettype)
                        {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->castSpell(getCreature(), m_spells[i].info, m_spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->castSpell(target, m_spells[i].info, m_spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->castSpellLoc(target->GetPosition(), m_spells[i].info, m_spells[i].instant);
                            break;
                        }

                        if (m_spells[i].speech != "")
                        {
                            getCreature()->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, m_spells[i].speech.c_str());
                            getCreature()->PlaySoundToSet(m_spells[i].soundid);
                        }

                        m_spellsEnabled[i] = false;
                        return;
                    }

                    if ((randomValue > comulativeperc && randomValue <= (comulativeperc + m_spells[i].perctrigger)) || !m_spells[i].casttime)
                    {
                        getCreature()->setAttackTimer(m_spells[i].attackstoptimer, false);
                        m_spellsEnabled[i] = true;
                    }
                    comulativeperc += m_spells[i].perctrigger;
                }
            }
        }*/
protected:
    bool m_isIntroMob = false;
    int m_spellCount = 0;

    /* Warning
     * Using vectors here is theoretically dangerous as they don't guarantee order
     *   when elements are erased or moved, however it doesn't matter here as we
     *   aren't modifying the number of elements in each vector
     * 
     * TODO: Write a proper spell manager to handle this stuff */
    std::vector<bool> m_spellsEnabled;
    //std::vector<SP_AI_Spell> m_spells;
};

class VHIntroAzureBinder : VHCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VHIntroAzureBinder(c); }
    explicit VHIntroAzureBinder(Creature* pCreature) : VHCreatureAI(pCreature)
    {
        /*const int SPELL_ARCANE_BARRAGE = 58456;
        const int SPELL_ARCANE_EXPLOSION = 58455;

        m_isIntroMob = true;
        //m_spellCount = 2;
        for (int i = 0; i < m_spellCount; i++)
        {
            m_spellsEnabled.push_back(false);
        }*/

        /*auto spellArcaneBarrage = SP_AI_Spell();
        spellArcaneBarrage.info = sSpellMgr.getSpellInfo(SPELL_ARCANE_BARRAGE);
        spellArcaneBarrage.cooldown = 6;
        spellArcaneBarrage.targettype = TARGET_ATTACKING;
        spellArcaneBarrage.instant = true;
        spellArcaneBarrage.perctrigger = 50.0f;
        spellArcaneBarrage.attackstoptimer = 1000;
        m_spells.push_back(spellArcaneBarrage);
        m_spellsEnabled[0] = true;*/

        /*auto spellArcaneExplosion = SP_AI_Spell();
        spellArcaneExplosion.info = sSpellMgr.getSpellInfo(SPELL_ARCANE_EXPLOSION);
        spellArcaneExplosion.cooldown = 4;
        spellArcaneExplosion.targettype = TARGET_VARIOUS;
        spellArcaneExplosion.instant = true;
        spellArcaneExplosion.perctrigger = 50.0f;
        spellArcaneExplosion.attackstoptimer = 1000;
        m_spells.push_back(spellArcaneExplosion);
        m_spellsEnabled[1] = true;*/
    }
};

class VHIntroAzureInvader : VHCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VHIntroAzureInvader(c); }
    explicit VHIntroAzureInvader(Creature* pCreature) : VHCreatureAI(pCreature)
    { 
        /* const int SPELL_CLEAVE = 15496;
        const int SPELL_IMPALE = 58459;

        m_isIntroMob = true;
        m_spellCount = 2;
        for (int i = 0; i < m_spellCount; i++)
        {
            m_spellsEnabled.push_back(false);
        }

        auto spellCleave = SP_AI_Spell();
        spellCleave.info = sSpellMgr.getSpellInfo(SPELL_CLEAVE);
        spellCleave.cooldown = 6;
        spellCleave.targettype = TARGET_ATTACKING;
        spellCleave.instant = true;
        spellCleave.perctrigger = 50.0f;
        spellCleave.attackstoptimer = 1000;
        m_spells.push_back(spellCleave);
        m_spellsEnabled[0] = true;

        auto spellImpale = SP_AI_Spell();
        spellImpale.info = sSpellMgr.getSpellInfo(SPELL_IMPALE);
        spellImpale.cooldown = 8;
        spellImpale.targettype = TARGET_ATTACKING;
        spellImpale.instant = true;
        spellImpale.perctrigger = 20.0f;
        spellImpale.attackstoptimer = 1000;
        m_spells.push_back(spellImpale);
        m_spellsEnabled[1] = true;*/
    }
};

class VHIntroAzureMageSlayer : VHCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VHIntroAzureMageSlayer(c); }
    explicit VHIntroAzureMageSlayer(Creature* pCreature) : VHCreatureAI(pCreature)
    {
        /*const int SPELL_ARCANE_EMPOWERMENT = 58469;

        m_isIntroMob = true;
        /*m_spellCount = 1;
        for (int i = 0; i < m_spellCount; i++)
        {
            m_spellsEnabled.push_back(false);
        }

        auto spellArcaneEmpowerment = SP_AI_Spell();
        spellArcaneEmpowerment.info = sSpellMgr.getSpellInfo(SPELL_ARCANE_EMPOWERMENT);
        spellArcaneEmpowerment.cooldown = 8;
        spellArcaneEmpowerment.targettype = TARGET_SELF;
        spellArcaneEmpowerment.instant = true;
        spellArcaneEmpowerment.perctrigger = 50.0f;
        spellArcaneEmpowerment.attackstoptimer = 1000;
        m_spells.push_back(spellArcaneEmpowerment);
        m_spellsEnabled[0] = true;*/
    }
};

class VHIntroAzureSpellBreaker : VHCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VHIntroAzureSpellBreaker(c); }
    explicit VHIntroAzureSpellBreaker(Creature* pCreature) : VHCreatureAI(pCreature)
    {
        /*const int SPELL_ARCANE_BLAST = 58462;
        const int SPELL_SLOW = 25603;

        m_isIntroMob = true;
        m_spellCount = 2;
        for (int i = 0; i < m_spellCount; i++)
        {
            m_spellsEnabled.push_back(false);
        }

        auto spellArcaneBlast = SP_AI_Spell();
        spellArcaneBlast.info = sSpellMgr.getSpellInfo(SPELL_ARCANE_BLAST);
        spellArcaneBlast.cooldown = 3;
        spellArcaneBlast.targettype = TARGET_ATTACKING;
        spellArcaneBlast.instant = false;
        spellArcaneBlast.casttime = 2500;
        spellArcaneBlast.perctrigger = 60.0f;
        spellArcaneBlast.attackstoptimer = 1000;
        m_spells.push_back(spellArcaneBlast);
        m_spellsEnabled[0] = true;

        auto spellSlow = SP_AI_Spell();
        spellSlow.info = sSpellMgr.getSpellInfo(SPELL_SLOW);
        spellSlow.cooldown = 7;
        spellSlow.targettype = TARGET_ATTACKING;
        spellSlow.instant = true;
        spellSlow.perctrigger = 40.0f;
        spellSlow.attackstoptimer = 1000;
        m_spells.push_back(spellSlow);
        m_spellsEnabled[0] = true; */
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
//Boss: Erekem
//class ErekemAI : public CreatureAIScript

//////////////////////////////////////////////////////////////////////////////////////////
//Boss: Moragg
//class MoraggAI : public CreatureAIScript

class MoraggAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MoraggAI(c); }
    explicit MoraggAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //// Spells
        //if (_isHeroic())
        //{
        //    AddSpell(MORAGG_SPELL_RAY_OF_SUFFERING_H, TARGET_ATTACKING, 100, 0, 0, 0, 45);
        //    AddSpell(MORAGG_SPELL_RAY_OF_PAIN_H, TARGET_ATTACKING, 100, 0, 0, 0 , 45);
        //}
        //else
        //{
        //    AddSpell(MORAGG_SPELL_RAY_OF_SUFFERING, TARGET_SELF, 100, 0, 0, 0, 45);
        //    AddSpell(MORAGG_SPELL_RAY_OF_PAIN, TARGET_ATTACKING, 100, 0, 0, 0, 45);
        //}

        //AddSpell(MORAGG_SPELL_CORROSIVE_SALIVA, TARGET_ATTACKING, 100, 0, 10, 0 , 5);
        //AddSpell(MORAGG_SPELL_OPTIC_LINK, TARGET_ATTACKING, 100, 0, 15, 0, 50);

    }
};

//////////////////////////////////////////////////////////////////////////////////////////
//Boss: Ichoron
//class IchoronAI : public CreatureAIScript

//////////////////////////////////////////////////////////////////////////////////////////
//Boss: Xevozz
//class XevozzAI : public CreatureAIScript

//////////////////////////////////////////////////////////////////////////////////////////
//Boss: Lavanthos
//class LavanthosAI : public CreatureAIScript

//////////////////////////////////////////////////////////////////////////////////////////
//Boss: Zuramat the Obliterator
//class ZuramatTheObliteratorAI : public CreatureAIScript

//////////////////////////////////////////////////////////////////////////////////////////
//Final Boss: Cyanigosa
//class CyanigosaAI : public CreatureAIScript

void SetupTheVioletHold(ScriptMgr* mgr)
{
//    // Instance
    mgr->register_instance_script(MAP_VIOLET_HOLD, &TheVioletHoldScript::Create);
//
//    //Sinclari and Guards
    mgr->register_creature_script(CN_LIEUTNANT_SINCLARI, &SinclariAI::Create);
    mgr->register_creature_script(CN_VIOLET_HOLD_GUARD, &VHGuardsAI::Create);
//
//    // Intro trash
//    mgr->register_creature_script(CN_INTRO_AZURE_BINDER_ARCANE, &VHIntroAzureBinder::Create);
//    mgr->register_creature_script(CN_INTRO_AZURE_INVADER_ARMS, &VHIntroAzureInvader::Create);
//    mgr->register_creature_script(CN_INTRO_AZURE_MAGE_SLAYER_MELEE, &VHIntroAzureMageSlayer::Create);
//    mgr->register_creature_script(CN_INTRO_AZURE_SPELLBREAKER_ARCANE, &VHIntroAzureSpellBreaker::Create);
//
//    //Bosses
//    //mgr->register_creature_script(CN_EREKEM, &ErekemAI::Create);
//    mgr->register_creature_script(CN_MORAGG, &MoraggAI::Create);
//    //mgr->register_creature_script(CN_ICHORON, &IchoronAI::Create);
//    //mgr->register_creature_script(CN_XEVOZZ, &XevozzAI::Create);
//    //mgr->register_creature_script(CN_LAVANTHOR, &LavanthorAI::Create);
//    //mgr->register_creature_script(CN_TURAMAT_THE_OBLITERATOR, &ZuramatTheObliteratorAI::Create);
//    //mgr->register_creature_script(CN_CYANIGOSA, &CyanigosaAI::Create);
//
//    Script* GSinclari = new SinclariGossip();
//    mgr->register_creature_gossip(CN_LIEUTNANT_SINCLARI, GSinclari);
}
