/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Raid_CoT_BattleOfMountHyjal.h"

#include "Setup.h"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Movement/MovementManager.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Utilities/Random.hpp"

class MountHyjalScript : public InstanceScript
{
public:
    explicit MountHyjalScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        InstanceData[HYJAL_TYPE_BASIC][0] = HYJAL_PHASE_NOT_STARTED;
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new MountHyjalScript(pMapMgr); }

    void SetLocaleInstanceData(uint32_t pType, uint32_t pIndex, uint32_t pData)
    {
        if (pType >= HYJAL_TYPE_END || pIndex >= 10)
            return;

        InstanceData[pType][pIndex] = pData;
    }

    uint32_t GetLocaleInstanceData(uint32_t pType, uint32_t pIndex)
    {
        if (pType >= HYJAL_TYPE_END || pIndex >= 10)
            return 0;

        return InstanceData[pType][pIndex];
    }

private:
    uint32_t InstanceData[HYJAL_TYPE_END][10]; // Expand this to fit your needs.
    // Type 0 = Basic Data;
    //   Index 0 = Current Phase;
};

class JainaProudmooreAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new JainaProudmooreAI(c); }
    explicit JainaProudmooreAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }
};

class JainaProudmooreGS : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        if (pObject->getWorldMap()->getBaseMap()->getMapId() != MAP_HYJALPAST)//in case someone spawned this NPC in another map
            return;

        GossipMenu menu(pObject->getGuid(), 2);
        switch (static_cast<MountHyjalScript*>(pObject->getWorldMap()->getScript())->GetLocaleInstanceData(HYJAL_TYPE_BASIC, 0))
        {
            case HYJAL_PHASE_NOT_STARTED:
                menu.addItem(GOSSIP_ICON_CHAT, 435, 1);     // We are ready to defend the Alliance base.
                break;
            case HYJAL_PHASE_RAGE_WINTERCHILL_COMPLETE:
                menu.addItem(GOSSIP_ICON_CHAT, 435, 1);     // We are ready to defend the Alliance base.
                break;
            case HYJAL_PHASE_ANETHERON_COMPLETE:
                menu.addItem(GOSSIP_ICON_CHAT, 436, 1);     // The defenses are holding up: we can continue.
                break;
        }

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* /*Plr*/, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        if (pObject->getWorldMap()->getBaseMap()->getMapId() != MAP_HYJALPAST)//in case someone spawned this NPC in another map
            return;

        switch (static_cast<MountHyjalScript*>(pObject->getWorldMap()->getScript())->GetLocaleInstanceData(HYJAL_TYPE_BASIC, 0))
        {
            case HYJAL_PHASE_NOT_STARTED:
            case HYJAL_PHASE_RAGE_WINTERCHILL_COMPLETE:
            case HYJAL_PHASE_ANETHERON_COMPLETE:
                break;
        }

        static_cast<Creature*>(pObject)->setNpcFlags(UNIT_NPC_FLAG_NONE);
    }
};

class ThrallAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ThrallAI(c); }
    explicit ThrallAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }
};

class ThrallGS : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        if (pObject->getWorldMap()->getBaseMap()->getMapId() != MAP_HYJALPAST)//in case someone spawned this NPC in another map
            return;

        GossipMenu menu(pObject->getGuid(), 2);
        switch (static_cast<MountHyjalScript*>(pObject->getWorldMap()->getScript())->GetLocaleInstanceData(HYJAL_TYPE_BASIC, 0))
        {
            case HYJAL_PHASE_ANETHERON_COMPLETE:
                menu.addItem(GOSSIP_ICON_CHAT, 437, 1);     // We're here to help! The Alliance are overrun.
                break;
            case HYJAL_PHASE_KAZROGAL_COMPLETE:
                menu.addItem(GOSSIP_ICON_CHAT, 438, 1);     // We're okay so far. Let's do this!
                break;
        }

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* /*Plr*/, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        if (pObject->getWorldMap()->getBaseMap()->getMapId() != MAP_HYJALPAST)//in case someone spawned this NPC in another map
            return;

        switch (static_cast<MountHyjalScript*>(pObject->getWorldMap()->getScript())->GetLocaleInstanceData(HYJAL_TYPE_BASIC, 0))
        {
            case HYJAL_PHASE_ANETHERON_COMPLETE:
            case HYJAL_PHASE_KAZROGAL_COMPLETE:
                break;
        }

        static_cast<Creature*>(pObject)->setNpcFlags(UNIT_NPC_FLAG_NONE);
    }
};


class ArchimondeTriggerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ArchimondeTriggerAI(c); }
    explicit ArchimondeTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->m_noRespawn = true;

        Unit* Archimonde = getNearestCreature(5598.629883f, -3447.719971f, 1576.650024f, 17968);
        if (Archimonde)
        {
            getCreature()->setChannelObjectGuid(Archimonde->getGuid());
            getCreature()->setChannelSpellId(DRAIN_WORLD_TREE_VISUAL2);
        }
    }
};

class DoomfireAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DoomfireAI(c); }
    explicit DoomfireAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->m_noRespawn = true;

        RegisterAIUpdateEvent(1000);

        DespawnTimer = 0;
        DirChange = 0;
    }

    void AIUpdate() override
    {
        DespawnTimer++;
        if (DespawnTimer >= 27)
        {
            getCreature()->Despawn(0, 0);
            return;
        }
        // After 4 sec of last direction change, doomfire has 33% chance to change direction
        DirChange++;
        if (DirChange == 4 && Util::getRandomUInt(3) == 1 || DirChange >= 5)
        {
            if (getCreature()->getAIInterface()->getUnitToFollow())
            {
                if (Util::getRandomUInt(3) == 1 || getCreature()->GetDistance2dSq(getCreature()->getAIInterface()->getUnitToFollow()) <= 2.0f)
                {
                    getCreature()->getAIInterface()->setUnitToFollow(nullptr);
                }
            }

            if (!getCreature()->getAIInterface()->getUnitToFollow())
            {
                if (Util::getRandomUInt(3) == 1)
                {
                    Unit* NewTarget = NULL;
                    NewTarget = FindTarget();
                    if (NewTarget)
                    {
                        getCreature()->getAIInterface()->setUnitToFollow(NewTarget);
                        getCreature()->getMovementManager()->moveFollow(NewTarget, 2.0f, 0.0f);
                    }
                }

                if (!getCreature()->getAIInterface()->getUnitToFollow())
                {
                    float movedist = 10.0f;
                    float x = 0.0f;
                    float y = 0.0f;

                    float xchange = Util::getRandomFloat(movedist);
                    float ychange = std::sqrt(movedist * movedist - xchange * xchange);

                    if (Util::getRandomUInt(2) == 1)
                        xchange *= -1;
                    if (Util::getRandomUInt(2) == 1)
                        ychange *= -1;

                    x = getCreature()->GetPositionX() + xchange;
                    y = getCreature()->GetPositionY() + ychange;

                    getCreature()->getAIInterface()->moveTo(x, y, getCreature()->GetPositionZ());
                }
            }

            DirChange = 0;
        }
    }
    // A bit rewritten FindTarget function
    Unit* FindTarget()
    {
        Unit* target = NULL;
        float distance = 15.0f;
        float z_diff;

        Unit* pUnit;
        float dist;

        for (const auto& itr : getCreature()->getInRangeOppositeFactionSet())
        {
            if (!itr || !itr->isCreatureOrPlayer())
                continue;

            pUnit = static_cast<Unit*>(itr);

            if (pUnit->hasUnitFlags(UNIT_FLAG_FEIGN_DEATH))
                continue;

            z_diff = fabs(getCreature()->GetPositionZ() - pUnit->GetPositionZ());
            if (z_diff > 2.5f)
                continue;

            if (!pUnit->isAlive() || getCreature() == pUnit)
                continue;

            dist = getCreature()->GetDistance2dSq(pUnit);

            if (dist > distance * distance)
                continue;

            if (dist < 3.0f)
                continue;

            distance = dist;
            target = pUnit;
        }

        return target;
    }

protected:
    uint32_t DespawnTimer;
    uint32_t DirChange;
};

void SetupBattleOfMountHyjal(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HYJALPAST, &MountHyjalScript::Create);

    mgr->register_creature_gossip(CN_JAINA_PROUDMOORE, new JainaProudmooreGS());
    mgr->register_creature_script(CN_JAINA_PROUDMOORE, &JainaProudmooreAI::Create);

    mgr->register_creature_gossip(CN_THRALL, new ThrallGS());
    mgr->register_creature_script(CN_THRALL, &ThrallAI::Create);

    mgr->register_creature_script(CN_ARCHIMONDE_CHANNEL_TRIGGER, &ArchimondeTriggerAI::Create);
    mgr->register_creature_script(CN_DOOMFIRE, &DoomfireAI::Create);
}
