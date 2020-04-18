/**
 * Summit MMORPG Server Software
 * Copyright (c) 2008 Summit Server Team
 * See COPYING for license details.
 */

#include <StdAfx.h>

enum
{
    // general defs
    HORDE 1,
    ALLIANCE 0,

    SILITHYST_SPELL = 29519,

    // gameobj
    SILITHYST_MOUND = 181597,
    SILITHYST_GEYSER = 181598,

    // buffs
    TRACES_OF_SILITHYST = 29534,
    CENARION_FAVOR = 30754,

    // how much we need to win?
    SILITHYST_MAX = 200,

    // areatriggers
    ALLIANCE_RETURN = 4162,
    HORDE_RETURN = 4168,

    // quests
    ALLIANCE_SILITHYST_QUEST = 17090,
    HORDE_SILITHYST_QUEST = 18199,

    // rewards
    REWARD_HONOR = 19,

    REWARD_REPUTATION_FACTION = 609,
    REWARD_REPUTATION_VALUE = 20,
};

int32_t silithyst_gathered[2] = { 0, 0 };

int32_t winners = -1;
bool locked = false;

void InitWorldStates(shared_ptr<MapMgr> pmgr)
{
    if(!pmgr)
        return;
    if(pmgr->GetMapId() == 1)  // Kalimdor
    {
        pmgr->GetStateManager().CreateWorldState(WORLDSTATE_SILITHUS_SILITHYST_MAX, SILITHYST_MAX);
        pmgr->GetStateManager().CreateWorldState(WORLDSTATE_SILITHUS_GATHERED_ALLIANCE, 0);
        pmgr->GetStateManager().CreateWorldState(WORLDSTATE_SILITHUS_GATHERED_HORDE, 0);
    }
}

void SilithusZoneHook(PlayerPointer plr, uint32_t Zone, uint32_t OldZone)
{
    if(!plr)
        return;

    if(Zone == ZONE_SILITHUS)
    {
        if(winners == plr->GetTeam())
            plr->CastSpell(plr, dbcSpell.LookupEntry(CENARION_FAVOR), true);
    }
    else if(OldZone == ZONE_SILITHUS)
    {
        if(winners == plr->GetTeam())
            plr->RemovePositiveAura(CENARION_FAVOR);
    }
}

void AreatriggerHook(PlayerPointer pPlayer, uint32_t triggerID)
{
    if(!pPlayer)
        return;

    if(triggerID == ALLIANCE_RETURN ||
            triggerID == HORDE_RETURN)
    {
        if(pPlayer->HasAura(SILITHYST_SPELL))     // if we doesn't have it, do nothing
        {
            pPlayer->RemoveAura(SILITHYST_SPELL);

            /* Rewards */
            pPlayer->CastSpell(pPlayer, TRACES_OF_SILITHYST, true);
            pPlayer->m_honorPoints += REWARD_HONOR;
            pPlayer->ModStanding(REWARD_REPUTATION_FACTION, REWARD_REPUTATION_VALUE);

            uint32_t quest = pPlayer->GetTeam() == ALLIANCE ? ALLIANCE_SILITHYST_QUEST : HORDE_SILITHYST_QUEST;
            QuestLogEntry* qle = pPlayer->GetQuestLogForEntry(quest);
            if(qle)
                qle->SendQuestComplete();

            if(locked)
                return;
            silithyst_gathered[ pPlayer->GetTeam() ]++;
            uint32_t worldstate = pPlayer->GetTeam() == ALLIANCE ? WORLDSTATE_SILITHUS_GATHERED_ALLIANCE : WORLDSTATE_SILITHUS_GATHERED_HORDE;
            pPlayer->GetMapMgr()->GetStateManager().UpdateWorldState(worldstate , silithyst_gathered[ pPlayer->GetTeam() ]);
            if(silithyst_gathered[ pPlayer->GetTeam() ] >= SILITHYST_MAX)
            {
                locked = true; // we win, so opposite team must wait for server restart
                winners = pPlayer->GetTeam();
                pPlayer->GetMapMgr()->CastSpellOnPlayers(winners, CENARION_FAVOR);
                pPlayer->GetMapMgr()->RemovePositiveAuraFromPlayers(winners == ALLIANCE ? HORDE : ALLIANCE, CENARION_FAVOR);
            }
        }
    }
}

class SilithystPickup : public GameObjectAIScript
{
public:

    SilithystPickup(GameObjectPointer goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObjectPointer  GO) { return new SilithystPickup(GO); }

    void OnActivate(PlayerPointer  pPlayer)
    {
        if(pPlayer && !pPlayer->HasAura(SILITHYST_SPELL))
            pPlayer->CastSpell(pPlayer, SILITHYST_SPELL, true);

        if(_gameobject)
            _gameobject->Despawn(0);
    }
};

void DropFlag(PlayerPointer  pPlayer, uint32_t spellID)
{
    if(!pPlayer || spellID != SILITHYST_SPELL)
        return;
    uint32_t triggerID = pPlayer->GetTeam() == ALLIANCE ? ALLIANCE_RETURN : HORDE_RETURN;
    // we have to use AreaTrigger.dbc here
    AreaTrigger* pAreaTrigger = AreaTriggerStorage.LookupEntry(triggerID);
    if(pAreaTrigger)
    {
        if(pPlayer->CalcDistance(pAreaTrigger->x, pAreaTrigger->y, pAreaTrigger->z) > 10.0f)
        {
            GameObjectPointer pGo = pPlayer->GetMapMgr()->GetInterface()->SpawnGameObject(SILITHYST_MOUND, pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 0, true, 0, 0);
            if(pGo == NULL)
            {
                pGo->Destructor();
                pGo = NULL;
            }
        };
    }
}

void SetupPvPSilithus(ScriptMgr* mgr)
{
    mgr->register_gameobject_script(SILITHYST_GEYSER, &SilithystPickup::Create);
    mgr->register_gameobject_script(SILITHYST_MOUND, &SilithystPickup::Create);

    mgr->register_hook(SERVER_HOOK_EVENT_ON_CONTINENT_CREATE, (void*)&InitWorldStates);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ZONE, (void*)&SilithusZoneHook);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_AREATRIGGER, (void*)&AreatriggerHook);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_AURA_REMOVE, (void*)&DropFlag);

    SpellEntry* sp = dbcSpell.LookupEntryForced(SILITHYST_SPELL);
    if(sp != NULL)
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN | AURA_INTERRUPT_ON_MOUNT;
}
