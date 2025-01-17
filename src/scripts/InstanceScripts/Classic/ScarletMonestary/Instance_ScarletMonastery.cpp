/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_ScarletMonastery.h"

#include "Setup.h"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

class ScarletMonasteryInstanceScript : public InstanceScript
{
public:
    explicit ScarletMonasteryInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new ScarletMonasteryInstanceScript(pMapMgr); }
};

class VishasAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VishasAI(c); }
    explicit VishasAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_uiSay = 0;
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        m_uiSay = 0;
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() <= 75 && m_uiSay == 0)
        {
            sendDBChatMessage(2111);     // Naughty secrets!
            m_uiSay = 1;
        }

        if (_getHealthPercent() <= 25 && m_uiSay == 1)
        {
            sendDBChatMessage(2112);     // I'll rip the secrets from your flesh!
            m_uiSay = 2;
        }
    }

private:
    uint8_t m_uiSay;
};

class ThalnosAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ThalnosAI(c); }
    explicit ThalnosAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ScarletMonastery::SP_THALNOS_SHADOW_BOLT, 20.0f, TARGET_RANDOM_SINGLE, 3, 2);
        addAISpell(ScarletMonastery::SP_THALNOS_FLAME_SPIKE, 20.0f, TARGET_RANDOM_DESTINATION, 3, 14);

        m_bEmoted = false;

        addEmoteForEvent(Event_OnCombatStart, 2107);    // We hunger for vengeance.
        addEmoteForEvent(Event_OnTargetDied, 2109);     // More... More souls!
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        m_bEmoted = false;
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() <= 50 && m_bEmoted == false)
        {
            sendDBChatMessage(2108);     // No rest... for the angry dead!
            m_bEmoted = true;
        } 
    }

private:
    bool m_bEmoted;
};

class DoanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DoanAI(c); }
    explicit DoanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_bShielded = false;

        addEmoteForEvent(Event_OnCombatStart, 2099);     // You will not defile these mysteries!
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t /*fAmount*/) override
    {
        if (_getHealthPercent() <= 50 && !m_bShielded)
            Shield();
    }

    void Shield()
    {
        getCreature()->castSpell(getCreature(), ScarletMonastery::SP_DOAN_SHIELD, true);
        sendDBChatMessage(2100);     // Burn in righteous fire!
        getCreature()->castSpell(getCreature(), ScarletMonastery::SP_DOAN_NOVA, false);
        m_bShielded = true;
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        m_bShielded = false;
    }

private:
    bool m_bShielded;
};

// Armory

class HerodAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HerodAI(c); }
    explicit HerodAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_bEnraged = false;

        addEmoteForEvent(Event_OnCombatStart, 2094);     // Ah - I've been waiting for a real challenge!
        addEmoteForEvent(Event_OnTargetDied, 2087);     // Is that all?
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        m_bEnraged = false;
        _removeAura(ScarletMonastery::SP_HEROD_ENRAGESPELL); 
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() <= 40 && m_bEnraged == false)
        {
            sendDBChatMessage(2090);     // Light, give me strength!
            _applyAura(ScarletMonastery::SP_HEROD_ENRAGESPELL);
        }
    }

    bool m_bEnraged;
};

// Cathedral
class MograineAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MograineAI(c); }
    explicit MograineAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addEmoteForEvent(Event_OnCombatStart, ScarletMonastery::SAY_MORGRAINE_01);
        addEmoteForEvent(Event_OnTargetDied, ScarletMonastery::SAY_MORGRAINE_02);
        addEmoteForEvent(Event_OnDied, ScarletMonastery::SAY_MORGRAINE_03);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        GameObject* pDoor = getNearestGameObject(1173.01f, 1389.91f, 31.9723f, ScarletMonastery::GO_INQUISITORS_DOOR);
        if (pDoor != nullptr)
            pDoor->setState(GO_STATE_OPEN);
    }
};

class WhitemaneAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WhitemaneAI(c); }
    explicit WhitemaneAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        sleep = addAISpell(ScarletMonastery::SP_WHITEMANE_SLEEP, 0.0f, TARGET_ATTACKING, 0, 0, false, true);
        sleep->setAttackStopTimer(1000);

        resurrection = addAISpell(ScarletMonastery::SP_WHITEMANE_RESURRECTION, 0.0f, TARGET_VARIOUS, 0, 0, false, true);
        resurrection->setAttackStopTimer(1000);
        resurrection->addEmote("Arise, my champion!", CHAT_MSG_MONSTER_YELL, ScarletMonastery::SAY_SOUND_RESTALK2);

        addEmoteForEvent(Event_OnCombatStart, ScarletMonastery::SAY_WHITEMANE_01);
        addEmoteForEvent(Event_OnTargetDied, ScarletMonastery::SAY_WHITEMANE_02);
        addEmoteForEvent(Event_OnDied, ScarletMonastery::SAY_WHITEMANE_03);
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t fAmount) override
    {
        if (fAmount < 5)
            return;

        if (getCreature()->getHealthPct() <= 50 && getScriptPhase() == 1)
            ChangeToPhase1();
    }

    void ChangeToPhase1()
    {
        setScriptPhase(2);

        sendDBChatMessage(2106);
        CastSleep();
        CastRes();
    }

    void CastSleep()
    {
        getCreature()->castSpell(getCreature(), sleep->mSpellInfo, true);
    }

    void CastRes()
    {
        auto morgrain = getNearestCreature(ScarletMonastery::CN_COMMANDER_MOGRAINE);
        if (morgrain != nullptr)
            getCreature()->castSpell(morgrain, resurrection->mSpellInfo, true);
    }

protected:
    CreatureAISpells* sleep;
    CreatureAISpells* resurrection;
};

class ScarletTorch : public GameObjectAIScript
{
public:
    explicit ScarletTorch(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new ScarletTorch(GO); }

    void OnActivate(Player* pPlayer) override
    {
        GameObject* SecretDoor = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(1167.79f, 1347.26f, 31.5494f, ScarletMonastery::GO_SCARLET_SECRET_DOOR);
        if (SecretDoor != nullptr)
        {
            if (SecretDoor->getState() == GO_STATE_CLOSED)
                SecretDoor->setState(GO_STATE_OPEN);
            else
                SecretDoor->setState(GO_STATE_CLOSED);
        }
    }
};

class ArmoryLever : public GameObjectAIScript
{
public:
    explicit ArmoryLever(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new ArmoryLever(GO); }

    void OnActivate(Player* pPlayer) override
    {
        GameObject* ArmoryDoor = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(2886.31f, -827.261f, 160.336f, ScarletMonastery::GO_ARMORY_DOOR);
        if (ArmoryDoor != nullptr)
        {
            if (ArmoryDoor->getState() == GO_STATE_CLOSED)
                ArmoryDoor->setState(GO_STATE_OPEN);
            else
                ArmoryDoor->setState(GO_STATE_CLOSED);
        }
    }
};

class CathedralLever : public GameObjectAIScript
{
public:
    explicit CathedralLever(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new CathedralLever(GO); }

    void OnActivate(Player* pPlayer) override
    {
        GameObject* CathedralDoor = pPlayer->getWorldMap()->getInterface()->getGameObjectNearestCoords(2908.18f, -818.203f, 160.332f, ScarletMonastery::GO_CATHEDRAL_DOOR);
        if (CathedralDoor != nullptr)
        {
            if (CathedralDoor->getState() == GO_STATE_CLOSED)
                CathedralDoor->setState(GO_STATE_OPEN);
            else
                CathedralDoor->setState(GO_STATE_CLOSED);
        }
    }
};

void SetupScarletMonastery(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SCARLET_MONASTERY, &ScarletMonasteryInstanceScript::Create);

    //Bosses
    mgr->register_creature_script(ScarletMonastery::CN_VISHAS, &VishasAI::Create);
    mgr->register_creature_script(ScarletMonastery::CN_THALNOS, &ThalnosAI::Create);
    mgr->register_creature_script(ScarletMonastery::CN_COMMANDER_MOGRAINE, &MograineAI::Create);
    mgr->register_creature_script(ScarletMonastery::CN_WHITEMANE, &WhitemaneAI::Create);
    mgr->register_creature_script(ScarletMonastery::CN_HEROD, &HerodAI::Create);
    mgr->register_creature_script(ScarletMonastery::CN_DOAN, &DoanAI::Create);

    //Gameobjects
    mgr->register_gameobject_script(ScarletMonastery::GO_SCARLET_TORCH, &ScarletTorch::Create);
    mgr->register_gameobject_script(ScarletMonastery::GO_CATHEDRAL_LEVER, &CathedralLever::Create);
    mgr->register_gameobject_script(ScarletMonastery::GO_ARMORY_LEVER, &ArmoryLever::Create);
}
