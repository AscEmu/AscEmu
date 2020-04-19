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

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Objects/Faction.h"

enum
{
    CN_JAINA_PROUDMOORE = 17772,

    CN_THRALL = 17852,

    CN_RAGE_WINTERCHILL = 17767,

    FROSTBOLT = 31249, // it's not correct spell for sure (not sure to others too :P)
    DEATCH_AND_DECAY = 31258,
    FROST_NOVA = 31250,
    FROST_ARMOR = 31256,

    CN_ANETHERON = 17808,

    //CARRION_SWARM = 31306,
    VAMPIRIC_AURA = 38196, // 31317
    INFERNO = 31299, // doesn't summon infernal - core bug
    SLEEP = 31298, // 12098
    BERSERK = 26662,

    CN_KAZROGAL = 17888,

    K_CLEAVE = 31345,
    WAR_STOMP = 31480,
    MARK_OF_KAZROGAL = 31447,
    MARK_OF_KAZROGAL2 = 31463, // should it be scripted to attack friends?

    CN_AZGALOR = 17842,

    CLEAVE = 31345,
    RAIN_OF_FIRE = 31340,
    HOWL_OF_AZGALOR = 31344,
    DOOM = 31347, // it's applied, but doesn't do anything more - should be scripted?

    CN_ARCHIMONDE_CHANNEL_TRIGGER = 30004,

    // Additional
    DRAIN_WORLD_TREE_VISUAL = 39140,
    DRAIN_WORLD_TREE_VISUAL2 = 39141,

    CN_DOOMFIRE = 18095,

    CN_ARCHIMONDE = 17968,

    FEAR = 33547,
    AIR_BURST = 32014,
    GRIP_OF_THE_LEGION = 31972,
    DOOMFIRE_STRIKE = 31903,
    //FINGER_OF_DEATH = 31984, // should be casted only when no one in melee range
    //HAND_OF_DEATH = 35354, // used if too close to Well of Eternity or if after 10 min caster has more than 10% hp
    //SOUL_CHARGER = 32053, // If player dies whole raid gets one of those 3 buffs
    //SOUL_CHARGEO = 32054,
    //SOUL_CHARGEG = 32057,
};

enum HyjalEvents
{
    HYJAL_EVENT_RAGE_WINTERCHILL,
    HYJAL_EVENT_ANETHERON,
    HYJAL_EVENT_KAZROGAL,
    HYJAL_EVENT_AZGALOR,
    HYJAL_EVENT_ARCHIMONDE
};

enum HyjalPhases
{
    HYJAL_PHASE_NOT_STARTED = 0,
    HYJAL_PHASE_RAGE_WINTERCHILL_COMPLETE,
    HYJAL_PHASE_ANETHERON_COMPLETE,
    HYJAL_PHASE_KAZROGAL_COMPLETE,
    HYJAL_PHASE_AZGALOR_COMPLETE,
    HYJAL_PHASE_ARCHIMONDE_COMPLETE,
};

enum HyjalType
{
    HYJAL_TYPE_BASIC = 0,
    HYJAL_TYPE_END
};

class MountHyjalScript : public InstanceScript
{
public:

    explicit MountHyjalScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        InstanceData[HYJAL_TYPE_BASIC][0] = HYJAL_PHASE_NOT_STARTED;
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new MountHyjalScript(pMapMgr); }

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
    ADD_CREATURE_FACTORY_FUNCTION(JainaProudmooreAI)
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
        if (pObject->GetMapMgr()->GetMapId() != MAP_HYJALPAST)//in case someone spawned this NPC in another map
            return;

        GossipMenu menu(pObject->getGuid(), 2);
        switch (static_cast<MountHyjalScript*>(pObject->GetMapMgr()->GetScript())->GetLocaleInstanceData(HYJAL_TYPE_BASIC, 0))
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
        if (pObject->GetMapMgr()->GetMapId() != MAP_HYJALPAST)//in case someone spawned this NPC in another map
            return;

        switch (static_cast<MountHyjalScript*>(pObject->GetMapMgr()->GetScript())->GetLocaleInstanceData(HYJAL_TYPE_BASIC, 0))
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
    ADD_CREATURE_FACTORY_FUNCTION(ThrallAI)
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
        if (pObject->GetMapMgr()->GetMapId() != MAP_HYJALPAST)//in case someone spawned this NPC in another map
            return;

        GossipMenu menu(pObject->getGuid(), 2);
        switch (static_cast<MountHyjalScript*>(pObject->GetMapMgr()->GetScript())->GetLocaleInstanceData(HYJAL_TYPE_BASIC, 0))
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
        if (pObject->GetMapMgr()->GetMapId() != MAP_HYJALPAST)//in case someone spawned this NPC in another map
            return;

        switch (static_cast<MountHyjalScript*>(pObject->GetMapMgr()->GetScript())->GetLocaleInstanceData(HYJAL_TYPE_BASIC, 0))
        {
            case HYJAL_PHASE_ANETHERON_COMPLETE:
            case HYJAL_PHASE_KAZROGAL_COMPLETE:
                break;
        }

        static_cast<Creature*>(pObject)->setNpcFlags(UNIT_NPC_FLAG_NONE);
    }
};

class RageWinterchillAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(RageWinterchillAI)
    explicit RageWinterchillAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto frostbold = addAISpell(FROSTBOLT, 8.0f, TARGET_RANDOM_SINGLE, 0, 10, false, true);
        frostbold->setAttackStopTimer(1000);
        frostbold->setMinMaxDistance(0.0f, 80.0f);

        auto deathAndDecay = addAISpell(DEATCH_AND_DECAY, 3.0f, TARGET_RANDOM_SINGLE, 0, 30);
        deathAndDecay->setAttackStopTimer(1000);
        deathAndDecay->setMinMaxDistance(0.0f, 30.0f);

        auto frostNova = addAISpell(FROST_NOVA, 5.0f, TARGET_RANDOM_SINGLE, 0, 15, false, true);
        frostNova->setAttackStopTimer(1000);
        frostNova->setMinMaxDistance(0.0f, 45.0f);

        auto frostArmor = addAISpell(FROST_ARMOR, 5.0f, TARGET_SELF, 0, 10, false, true);
        frostArmor->setAttackStopTimer(1000);
        frostArmor->setMinMaxDistance(0.0f, 45.0f);

        addEmoteForEvent(Event_OnCombatStart, 1590);    // The Legion's final conquest has begun! Once again the subjugation of this world is within our grasp. Let
        addEmoteForEvent(Event_OnTargetDied, 1586);     // Crumble and rot!
        addEmoteForEvent(Event_OnTargetDied, 1587);     // Ashes to ashes, dust to dust
        addEmoteForEvent(Event_OnTargetDied, 1584);     // All life must perish!
        addEmoteForEvent(Event_OnTargetDied, 1585);     // Victory to the Legion!
        addEmoteForEvent(Event_OnDied, 1583);           // You have won this battle, but not... the...war
    }
};

class AnetheronAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AnetheronAI)
    explicit AnetheronAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto vampiricAura = addAISpell(VAMPIRIC_AURA, 8.0f, TARGET_SELF, 0, 8, false, true);
        vampiricAura->setAttackStopTimer(1000);

        auto inferno = addAISpell(INFERNO, 6.0f, TARGET_RANDOM_DESTINATION, 0, 30);
        inferno->setAttackStopTimer(1000);
        inferno->setMinMaxDistance(0.0f, 60.0f);
        inferno->addEmote("Hit he, no time for a slow death", CHAT_MSG_MONSTER_YELL, 11039);

        auto sleep = addAISpell(SLEEP, 5.0f, TARGET_RANDOM_SINGLE, 0, 7, false, true);
        sleep->setAttackStopTimer(3000);
        sleep->setMinMaxDistance(0.0f, 30.0f);

        auto berserk = addAISpell(BERSERK, 5.0f, TARGET_SELF, 0, 600, false, true);
        berserk->setAttackStopTimer(3000);
        berserk->setMinMaxDistance(0.0f, 30.0f);

        addEmoteForEvent(Event_OnCombatStart, 1569);    // You are defenders of a doomed world. Flee here and perhaps you will prolong your pathetic lives!
        addEmoteForEvent(Event_OnTargetDied, 1560);     // Your hopes are lost.
        addEmoteForEvent(Event_OnTargetDied, 1561);     // Scream for me.
        addEmoteForEvent(Event_OnTargetDied, 1565);     // You look tired
        addEmoteForEvent(Event_OnDied, 1559);           // The clock... is still...ticking.
    }
};

class KazrogalAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KazrogalAI)
    explicit KazrogalAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto cleave = addAISpell(K_CLEAVE, 8.0f, TARGET_ATTACKING, 0, 10, false, true);
        cleave->setAttackStopTimer(1000);

        auto warStomp = addAISpell(WAR_STOMP, 6.0f, TARGET_VARIOUS, 0, 15, false, true);
        warStomp->setAttackStopTimer(1000);

        auto markOfKazrogal = addAISpell(MARK_OF_KAZROGAL, 4.0f, TARGET_VARIOUS, 0, 25, false, true);
        markOfKazrogal->setAttackStopTimer(2000);

        auto markOfKazrogal2 = addAISpell(MARK_OF_KAZROGAL2, 4.0f, TARGET_VARIOUS, 0, 25, false, true);
        markOfKazrogal2->setAttackStopTimer(2000);

        addEmoteForEvent(Event_OnCombatStart, 1582);     // Cry for mercy! Your meaningless lives will soon be forfeit.
        addEmoteForEvent(Event_OnTargetDied, 1580);     // Your death will be a painful one.
        addEmoteForEvent(Event_OnTargetDied, 1581);     // You... are marked
        addEmoteForEvent(Event_OnTargetDied, 1578);     // You... are nothing!
        addEmoteForEvent(Event_OnTargetDied, 1579);     // Miserable nuisance!
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        //\todo move this to db
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "hahahahaa aahaah");
        getCreature()->PlaySoundToSet(11018);
    }
};

class AzgalorAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AzgalorAI)
    explicit AzgalorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto cleave = addAISpell(CLEAVE, 8.0f, TARGET_ATTACKING, 0, 10, false, true);
        cleave->setAttackStopTimer(3000);

        auto rainOfFire = addAISpell(RAIN_OF_FIRE, 7.0f, TARGET_RANDOM_DESTINATION, 0, 15, false, true);
        rainOfFire->setAttackStopTimer(1000);
        rainOfFire->setMinMaxDistance(0.0f, 40.0f);

        auto howlOfAzgalor = addAISpell(HOWL_OF_AZGALOR, 5.0f, TARGET_VARIOUS, 0, 15, false, true);
        howlOfAzgalor->setAttackStopTimer(2000);

        auto doom = addAISpell(DOOM, 15.0f, TARGET_RANDOM_SINGLE, 0, 45, false, true);
        doom->setAttackStopTimer(1000);
        doom->setMinMaxDistance(0.0f, 50.0f);

        addEmoteForEvent(Event_OnCombatStart, 1576);    // Abandon all hope! The legion has returned to finish what was begun so many years ago. This time there will be no escape!
        addEmoteForEvent(Event_OnTargetDied, 1571);     // Reesh, hokta!
        addEmoteForEvent(Event_OnTargetDied, 1573);     // No one is going to save you!
        addEmoteForEvent(Event_OnTargetDied, 1572);     // Don't fight it
        addEmoteForEvent(Event_OnDied, 1570);           // Your time is almost... up!
    }
};

class ArchimondeTriggerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ArchimondeTriggerAI)
    explicit ArchimondeTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
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
    ADD_CREATURE_FACTORY_FUNCTION(DoomfireAI)
    explicit DoomfireAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
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
            if (getCreature()->GetAIInterface()->getUnitToFollow())
            {
                if (Util::getRandomUInt(3) == 1 || getCreature()->GetDistance2dSq(getCreature()->GetAIInterface()->getUnitToFollow()) <= 2.0f)
                {
                    getCreature()->GetAIInterface()->ResetUnitToFollow();
                    getCreature()->GetAIInterface()->SetUnitToFollowAngle(0.0f);
                }
            }

            if (!getCreature()->GetAIInterface()->getUnitToFollow())
            {
                if (Util::getRandomUInt(3) == 1)
                {
                    Unit* NewTarget = NULL;
                    NewTarget = FindTarget();
                    if (NewTarget)
                    {
                        getCreature()->GetAIInterface()->SetUnitToFollow(NewTarget);
                        getCreature()->GetAIInterface()->SetUnitToFollowAngle(2.0f);
                    }
                }

                if (!getCreature()->GetAIInterface()->getUnitToFollow())
                {
                    float movedist = 10.0f;
                    float x = 0.0f;
                    float y = 0.0f;

                    float xchange = Util::getRandomFloat(movedist);
                    float ychange = sqrt(movedist * movedist - xchange * xchange);

                    if (Util::getRandomUInt(2) == 1)
                        xchange *= -1;
                    if (Util::getRandomUInt(2) == 1)
                        ychange *= -1;

                    x = getCreature()->GetPositionX() + xchange;
                    y = getCreature()->GetPositionY() + ychange;

                    getCreature()->GetAIInterface()->MoveTo(x, y, getCreature()->GetPositionZ());
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

/* * Doomfire - Leaves a trail of fire on the ground, which does 2400 fire
        damage per second (occasionally feared people run into these and die) */

class ArchimondeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ArchimondeAI)
    explicit ArchimondeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto airBurst = addAISpell(AIR_BURST, 5.0f, TARGET_ATTACKING, 0, 25, false, true);
        airBurst->setAttackStopTimer(1000);

        auto gripOfTheLegion = addAISpell(GRIP_OF_THE_LEGION, 5.0f, TARGET_RANDOM_SINGLE, 0, 25, false, true);
        gripOfTheLegion->setAttackStopTimer(1000);
        gripOfTheLegion->setMinMaxDistance(0.0f, 60.0f);

        auto doomfireStrike = addAISpell(DOOMFIRE_STRIKE, 7.0f, TARGET_VARIOUS, 0, 25, false, true);
        doomfireStrike->setAttackStopTimer(2000);

        auto fear = addAISpell(FEAR, 2.0f, TARGET_VARIOUS, 0, 40, false, true);
        fear->setAttackStopTimer(2000);

        addEmoteForEvent(Event_OnCombatStart, 1591);    // Your resistance is insignificant.
        addEmoteForEvent(Event_OnTargetDied, 1597);     // Your soul will languish for eternity.
        addEmoteForEvent(Event_OnTargetDied, 1596);     // All creation will be devoured!
        addEmoteForEvent(Event_OnTargetDied, 1598);     // I am the coming of the end!
        addEmoteForEvent(Event_OnDied, 1600);           // No, it cannot be! Nooo!
    }
};

void SetupBattleOfMountHyjal(ScriptMgr* mgr)
{
    /*mgr->register_instance_script(MAP_HYJALPAST, &MountHyjalScript::Create);*/

    mgr->register_creature_gossip(CN_JAINA_PROUDMOORE, new JainaProudmooreGS());
    mgr->register_creature_script(CN_JAINA_PROUDMOORE, &JainaProudmooreAI::Create);

    mgr->register_creature_gossip(CN_THRALL, new ThrallGS());
    mgr->register_creature_script(CN_THRALL, &ThrallAI::Create);

    mgr->register_creature_script(CN_RAGE_WINTERCHILL, &RageWinterchillAI::Create);
    mgr->register_creature_script(CN_ANETHERON, &AnetheronAI::Create);
    mgr->register_creature_script(CN_KAZROGAL, &KazrogalAI::Create);
    mgr->register_creature_script(CN_AZGALOR, &AzgalorAI::Create);
    mgr->register_creature_script(CN_ARCHIMONDE_CHANNEL_TRIGGER, &ArchimondeTriggerAI::Create);
    mgr->register_creature_script(CN_DOOMFIRE, &DoomfireAI::Create);
    mgr->register_creature_script(CN_ARCHIMONDE, &ArchimondeAI::Create);
}
