/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GunshipBattle.h"
#include "Management/Faction.h"
#include "Objects/Units/Creatures/Summons/Summon.h"
#include <Management/ObjectMgr.h>
#include <Management/TransporterHandler.h>
#include <Objects/Transporter.h>
#include "Movement/MovementGenerators/PointMovementGenerator.h"
#include "Server/Script/CreatureAIScript.h"

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Gunship Battle Alliance
class MuradinGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 14500);
        if (!plr->isGroupLeader())
            menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_NOT_LEADER, 1);

        menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_ALLIANCE_RDY, 2);
        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        IceCrownCitadelScript* pInstance = (IceCrownCitadelScript*)pPlayer->getWorldMap()->getScript();
        if (!pInstance)
            return;

        switch (Id)
        {
        case 1:
            
            break;
        case 2:
            // Instance Start Gunship
            pInstance->DoAction(ACTION_INTRO_START);
            // Muradin Intro
            static_cast<Creature*>(pObject)->GetScript()->DoAction(ACTION_INTRO_START);
            // Clear NPC FLAGS
            static_cast<Creature*>(pObject)->removeUnitFlags(UNIT_NPC_FLAG_GOSSIP);
            break;
        }
        GossipMenu::senGossipComplete(pPlayer);
    }
};

class MuradinAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MuradinAI(c); }
    explicit MuradinAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();

        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }

    void AIUpdate() override
    {
        if (getCreature()->getAIInterface()->getAiState() == AI_STATE_CASTING)
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
                case EVENT_INTRO_ALLIANCE_1:
                {
                    sendDBChatMessage(SAY_INTRO_ALLIANCE_0);
                    break;
                }
                case EVENT_INTRO_ALLIANCE_2:
                {
                    sendDBChatMessage(SAY_INTRO_ALLIANCE_1);
                    break;
                }
                case EVENT_INTRO_ALLIANCE_3:
                {
                    sendDBChatMessage(SAY_INTRO_ALLIANCE_2);
                    mInstance->SpawnEnemyGunship();
                    break;
                }
                case EVENT_INTRO_ALLIANCE_4:
                {
                    sendDBChatMessage(SAY_INTRO_ALLIANCE_3);
                    break;
                }
                case EVENT_INTRO_ALLIANCE_5:
                {
                    sendDBChatMessage(SAY_INTRO_ALLIANCE_4);
                    break;
                }
                case EVENT_INTRO_ALLIANCE_6:
                {
                    sendDBChatMessage(SAY_INTRO_ALLIANCE_5);
                    break;
                }
                case EVENT_INTRO_ALLIANCE_7:
                {
                    if (Creature* pSaurfang = mInstance->getLocalCreatureData(DATA_GB_HIGH_OVERLORD_SAURFANG))
                        pSaurfang->GetScript()->sendDBChatMessage(SAY_BOARDING_SKYBREAKER_0);
                    break;
                }
                case EVENT_INTRO_ALLIANCE_8:
                {
                    sendDBChatMessage(SAY_INTRO_ALLIANCE_7);
                    break;
                }
                case EVENT_KEEP_PLAYER_IN_COMBAT:
                {
                    if (mInstance->getBossState(DATA_ICECROWN_GUNSHIP_BATTLE) == InProgress)
                    {
                        //SPELL_LOCK_PLAYERS_AND_TAP_CHEST maybe not needed to cast it but prepared
                        scriptEvents.addEvent(EVENT_KEEP_PLAYER_IN_COMBAT, 5000);
                    }
                    break;
                }
                case EVENT_SUMMON_MAGE:
                {
                    break;
                }
                case EVENT_ADDS:
                {
                    break;
                }
                case EVENT_ADDS_BOARD_YELL:
                {
                    break;
                }
                case EVENT_CHECK_RIFLEMAN:
                {
                    break;
                }
                case EVENT_CHECK_MORTAR:
                {
                    break;
                }
                case EVENT_CLEAVE:
                {
                    break;
                }
                default:
                    break;
            }
        }
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
            case ACTION_INTRO_START:
            {
                scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_1, 5000);
                scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_2, 10000);
                scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_3, 28000);
                scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_4, 33000);
                scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_5, 39000);
                scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_6, 45000);
                break;
            }
            case ACTION_BATTLE_EVENT:
            {
                scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_7, 5000);
                scriptEvents.addEvent(EVENT_INTRO_ALLIANCE_8, 11000);
                scriptEvents.addEvent(EVENT_KEEP_PLAYER_IN_COMBAT, 1);

                mInstance->setZoneMusic(4812, 17289);

                // Combat starts now
                if (Creature* orgrimsHammer = mInstance->getLocalCreatureData(DATA_ORGRIMMAR_HAMMER_BOSS))
                    mInstance->sendUnitEncounter(EncounterFrameEngage, orgrimsHammer, 1);

                if (Creature* skybreaker = mInstance->getLocalCreatureData(DATA_SKYBREAKER_BOSS))
                    mInstance->sendUnitEncounter(EncounterFrameEngage, skybreaker, 2);

                break;
            }
            case ACTION_SPAWN_MAGE:
            {
                // ToDo
                break;
            }
            default:
                break;
        }     
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Gunship Battle Horde
class SaurfangGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 14500);
        if (!plr->isGroupLeader())
            menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_NOT_LEADER, 1);

        menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_HORDE_RDY, 2);
        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        IceCrownCitadelScript* pInstance = (IceCrownCitadelScript*)pPlayer->getWorldMap()->getScript();
        if (!pInstance)
            return;

        switch (Id)
        {
        case 1:

            break;
        case 2:
            // Instance Start Gunship
            pInstance->DoAction(ACTION_INTRO_START);
            // Saurfang Intro
            static_cast<Creature*>(pObject)->GetScript()->DoAction(ACTION_INTRO_START);
            // Clear NPC FLAGS
            static_cast<Creature*>(pObject)->removeUnitFlags(UNIT_NPC_FLAG_GOSSIP);
            break;
        }
        GossipMenu::senGossipComplete(pPlayer);
    }
};

class SaurfangAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SaurfangAI(c); }
    explicit SaurfangAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();

        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }

    void AIUpdate() override
    {
        if (getCreature()->getAIInterface()->getAiState() == AI_STATE_CASTING)
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
                case EVENT_INTRO_HORDE_1:
                {
                    sendDBChatMessage(SAY_INTRO_HORDE_0);
                    break;
                }
                case EVENT_INTRO_HORDE_1_1:
                {
                    sendDBChatMessage(SAY_INTRO_HORDE_0_1);
                    break;
                }
                case EVENT_INTRO_HORDE_2:
                {
                    sendDBChatMessage(SAY_INTRO_HORDE_1);
                    mInstance->SpawnEnemyGunship();
                    break;
                }
                case EVENT_INTRO_HORDE_3:
                {
                    sendDBChatMessage(SAY_INTRO_HORDE_2);
                    break;
                }
                case EVENT_INTRO_HORDE_4:
                {
                    sendDBChatMessage(SAY_BOARDING_SKYBREAKER_0);
                    if (Creature* pMuradin = mInstance->getLocalCreatureData(DATA_GB_MURADIN_BRONZEBEARD))
                        pMuradin->GetScript()->sendDBChatMessage(SAY_INTRO_ALLIANCE_7);
                    break;
                }
                case EVENT_INTRO_HORDE_5:
                {
                    sendDBChatMessage(SAY_INTRO_HORDE_4);
                    break;
                }
                case EVENT_KEEP_PLAYER_IN_COMBAT:
                {
                    if (mInstance->getBossState(DATA_ICECROWN_GUNSHIP_BATTLE) == InProgress)
                    {
                        //SPELL_LOCK_PLAYERS_AND_TAP_CHEST maybe not needed to cast it but prepared
                        scriptEvents.addEvent(EVENT_KEEP_PLAYER_IN_COMBAT, 5000);
                    }
                    break;
                }
                case EVENT_SUMMON_MAGE:
                {
                    break;
                }
                case EVENT_ADDS:
                {
                    break;
                }
                case EVENT_ADDS_BOARD_YELL:
                {
                    break;
                }
                case EVENT_CHECK_RIFLEMAN:
                {
                    break;
                }
                case EVENT_CHECK_MORTAR:
                {
                    break;
                }
                case EVENT_CLEAVE:
                {
                    break;
                }
                default:
                    break;
            }
        }
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
            case ACTION_INTRO_START:
            {
                scriptEvents.addEvent(EVENT_INTRO_HORDE_1, 5000);
                scriptEvents.addEvent(EVENT_INTRO_HORDE_1_1, 16000);
                scriptEvents.addEvent(EVENT_INTRO_HORDE_2, 24600);
                scriptEvents.addEvent(EVENT_INTRO_HORDE_3, 29600);
                scriptEvents.addEvent(EVENT_INTRO_HORDE_4, 39200);
                break;
            }
            case ACTION_BATTLE_EVENT:
            {
                scriptEvents.addEvent(EVENT_INTRO_HORDE_5, 5000);

                mInstance->setZoneMusic(4812, 17289);

                // Combat starts now
                if (Creature* skybreaker = mInstance->getLocalCreatureData(DATA_SKYBREAKER_BOSS))
                    mInstance->sendUnitEncounter(EncounterFrameEngage, skybreaker, 1);

                if (Creature* orgrimsHammer = mInstance->getLocalCreatureData(DATA_ORGRIMMAR_HAMMER_BOSS))
                    mInstance->sendUnitEncounter(EncounterFrameEngage, orgrimsHammer, 2);

                break;
            }
            case ACTION_SPAWN_MAGE:
            {
                // ToDo
                break;
            }
            default:
                break;
        }
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;
};

class ZafodBoomboxGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        pInstance = (IceCrownCitadelScript*)plr->getWorldMap()->getScript();

        GossipMenu menu(pObject->getGuid(), 14500);
        menu.addItem(GOSSIP_ICON_CHAT, GOSSIP_OPTION_JETPACK, 1);
        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* /*pObject*/, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
        case 1:
            pPlayer->getItemInterface()->AddItemById(ITEM_GOBLIN_ROCKET_PACK, 1, 0);
            break;
        }
        GossipMenu::senGossipComplete(pPlayer);
    }

protected:
    IceCrownCitadelScript* pInstance;
};

class ZafodBoomboxAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ZafodBoomboxAI(c); }
    explicit ZafodBoomboxAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();
        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;
};

class GunshipAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GunshipAI(c); }
    explicit GunshipAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();

        _teamInInstance = mInstance->getInstance()->getTeamIdInInstance();
        _summonedFirstMage = false;
        _died = false;
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
    }

    void DamageTaken(Unit* /*_attacker*/, uint32_t* damage) override
    {
        if (*damage >= getCreature()->getHealth())
        {
            OnDied(nullptr);
            *damage = getCreature()->getHealth() - 1;
            return;
        }

        if (_summonedFirstMage)
            return;

        if (getCreature()->GetTransport()->getEntry() != uint32_t(_teamInInstance == TEAM_HORDE ? GO_THE_SKYBREAKER_HORDE_ICC : GO_ORGRIM_S_HAMMER_ALLIANCE_ICC))
            return;

        if (getCreature()->getHealthPct() > 90)
            return;

        _summonedFirstMage = true;
        if (Creature* captain = mInstance->getLocalCreatureData(_teamInInstance == TEAM_HORDE ? DATA_GB_MURADIN_BRONZEBEARD : DATA_GB_HIGH_OVERLORD_SAURFANG))
            captain->GetScript()->DoAction(ACTION_SPAWN_MAGE);
    }

    void OnDied(Unit* /*pTarget*/) override
    {
        if (_died)
            return;

        _died = true;

        // Victory??
        bool isVictory = getCreature()->GetTransport()->getEntry() == GO_THE_SKYBREAKER_HORDE_ICC || getCreature()->GetTransport()->getEntry() == GO_ORGRIM_S_HAMMER_ALLIANCE_ICC;
        mInstance->setBossState(DATA_ICECROWN_GUNSHIP_BATTLE, isVictory ? Performed : Failed);

        // Disangege
        if (Creature* creature = mInstance->getLocalCreatureData(getCreature()->getEntry() == NPC_GB_ORGRIMS_HAMMER ? DATA_SKYBREAKER_BOSS : DATA_ORGRIMMAR_HAMMER_BOSS))
        {
            mInstance->sendUnitEncounter(EncounterFrameDisengaged, creature);
        }

        mInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        // Stopp Playing Music
        mInstance->setZoneMusic(4812, 0);

        std::list<Creature*> creatures;
        // Eject All Passengers
        GetCreatureListWithEntryInGrid(creatures, _teamInInstance == TEAM_HORDE ? NPC_GB_HORDE_CANON : NPC_GB_ALLIANCE_CANON, 900.0f);
        for (auto itr = creatures.begin(); itr != creatures.end(); ++itr)
        {
            Creature* cannon = *itr;
            if (isVictory)
                cannon->GetScript()->DoAction(ACTION_BATTLE_DONE);
            else
                cannon->GetScript()->DoAction(ACTION_FAIL);
        }
        creatures.clear();
        // Explosion
        GetCreatureListWithEntryInGrid(creatures, NPC_GB_GUNSHIP_HULL, 900.0f);
        for (auto itr = creatures.begin(); itr != creatures.end(); ++itr)
        {
            Creature* hull = *itr;
            if (hull->GetTransport() != getCreature()->GetTransport())
                continue;

            if (isVictory)
                hull->GetScript()->DoAction(ACTION_BATTLE_DONE);
            else
                hull->GetScript()->DoAction(ACTION_FAIL);
        }
        creatures.clear();

        if (isVictory)
            mInstance->DoAction(ACTION_BATTLE_DONE);
        else
            mInstance->DoAction(ACTION_FAIL);
    }

private:
    uint32_t _teamInInstance;
    std::map<uint64_t, uint32_t> _shipVisits;
    bool _summonedFirstMage;
    bool _died;

protected:
    // Common
    IceCrownCitadelScript* mInstance;
};

class GunshipHullAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GunshipHullAI(c); }
    explicit GunshipHullAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);

        // Scripted Spells not autocastet
        ExplosionVictory = addAISpell(SPELL_EXPLOSION_VICTORY, 0.0f, TARGET_SELF);
        ExplosionWipe = addAISpell(SPELL_EXPLOSION_WIPE, 0.0f, TARGET_SELF);
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
            case ACTION_BATTLE_DONE:    // Enemy Ship Explodes
                _castAISpell(ExplosionVictory);
                break;
            case ACTION_FAIL:   // Our Ship Explodes
                _castAISpell(ExplosionWipe);
                break;
            default:
                break;
        }
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;

    // Spells
    CreatureAISpells* ExplosionVictory;
    CreatureAISpells* ExplosionWipe;
};

class GunshipCanonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GunshipCanonAI(c); }
    explicit GunshipCanonAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = (IceCrownCitadelScript*)getInstanceScript();

        getCreature()->setControlled(true, UNIT_STATE_ROOTED);

        EjectBelowZero = addAISpell(SPELL_EJECT_ALL_PASSENGERS_BELOW_ZERO, 0.0f, TARGET_SELF, 0, 0, 0, true);
        EcectWipe = addAISpell(SPELL_EJECT_ALL_PASSENGERS_WIPE, 0.0f, TARGET_SELF, 0, 0, 0, true);
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
            case ACTION_BATTLE_DONE:    // Enemy Ship Explodes
                _castAISpell(EjectBelowZero);
                break;
            case ACTION_FAIL:           // Our Ship Explodes
                _castAISpell(EcectWipe);
                break;
            default:
                break;
        }
    }

protected:
    // Common
    IceCrownCitadelScript* mInstance;

    // Spells
    CreatureAISpells* EjectBelowZero;
    CreatureAISpells* EcectWipe;
};
