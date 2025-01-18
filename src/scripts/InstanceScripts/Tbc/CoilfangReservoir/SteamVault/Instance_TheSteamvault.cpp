/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TheSteamvault.h"

#include "Movement/MovementManager.h"
#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Utilities/Random.hpp"

class TheSteamvaultInstanceScript : public InstanceScript
{
public:
    explicit TheSteamvaultInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheSteamvaultInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Boss AIs

class HydromancerThespiaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HydromancerThespiaAI(c); }
    explicit HydromancerThespiaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto envelopingWinds = addAISpell(ENVELOPING_WINDS, 9.0f, TARGET_RANDOM_SINGLE, 0, 10, false, true);
        envelopingWinds->setAttackStopTimer(1000);
        envelopingWinds->setMinMaxDistance(0.0f, 35.0f);

        auto lightningCloud = addAISpell(LIGHTNING_CLOUD, 8.0f, TARGET_RANDOM_DESTINATION, 0, 25, false, true);
        lightningCloud->setAttackStopTimer(1000);
        lightningCloud->setMinMaxDistance(0.0f, 30.0f);

        auto lungBurst = addAISpell(LUNG_BURST, 9.0f, TARGET_RANDOM_SINGLE, 0, 15, false, true);
        lungBurst->setAttackStopTimer(1000);
        lungBurst->setMinMaxDistance(0.0f, 40.0f);

        addEmoteForEvent(Event_OnCombatStart, SAY_HYDROMACER_THESPIA_02);
        addEmoteForEvent(Event_OnCombatStart, SAY_HYDROMACER_THESPIA_03);
        addEmoteForEvent(Event_OnCombatStart, SAY_HYDROMACER_THESPIA_04);
        addEmoteForEvent(Event_OnTargetDied, SAY_HYDROMACER_THESPIA_05);
        addEmoteForEvent(Event_OnTargetDied, SAY_HYDROMACER_THESPIA_06);
        addEmoteForEvent(Event_OnDied, SAY_HYDROMACER_THESPIA_07);
    }
};

static LocationVector SpawnCoords[] =
{
    { -300.037842f, -115.296227f, -7.865229f, 4.197916f },
    { -330.083008f, -121.505997f, -7.985120f, 5.061450f },
    { -346.530273f, -147.167892f, -6.703687f, 0.010135f }
};

// Should they really fight?
class SteamriggerMechanicAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SteamriggerMechanicAI(c); }
    explicit SteamriggerMechanicAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->m_noRespawn = true;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->getMovementManager()->clear();
        getCreature()->interruptSpell();
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
    }

    void OnTargetDied(Unit* mTarget) override
    {
        getCreature()->getThreatManager().clearThreat(mTarget);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getCreature()->getMovementManager()->clear();
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t /*fAmount*/) override
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
    }
};

// Must spawn 3 Steamrigger Mechanics when his health is on 75%, 50% and 25%
class MekgineerSteamriggerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MekgineerSteamriggerAI(c); }
    std::vector <Unit*> Gnomes;

    explicit MekgineerSteamriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto shrinkRay = addAISpell(SUPER_SHRINK_RAY, 9.0f, TARGET_RANDOM_SINGLE, 0, 20, false, true);
        shrinkRay->setAttackStopTimer(1000);
        shrinkRay->setMinMaxDistance(0.0f, 40.0f);

        auto sawBlade = addAISpell(SAW_BLADE, 10.0f, TARGET_RANDOM_SINGLE, 0, 15, false, true);
        sawBlade->setAttackStopTimer(1000);
        sawBlade->setMinMaxDistance(0.0f, 40.0f);

        auto electrifiedNet = addAISpell(ELECTRIFIED_NET, 8.0f, TARGET_RANDOM_SINGLE, 0, 15, false, true);
        electrifiedNet->setAttackStopTimer(1000);
        electrifiedNet->setMinMaxDistance(0.0f, 40.0f);

        auto enrage = addAISpell(ENRAGE, 8.0f, TARGET_SELF, 0, 300, false, true);
        enrage->setAttackStopTimer(1000);
        enrage->addDBEmote(SAY_MEKGINEER_STEAMRIGGER_08);

        GnomeCounter = 0;

        addEmoteForEvent(Event_OnCombatStart, SAY_MEKGINEER_STEAMRIGGER_02);
        addEmoteForEvent(Event_OnCombatStart, SAY_MEKGINEER_STEAMRIGGER_03);
        addEmoteForEvent(Event_OnCombatStart, SAY_MEKGINEER_STEAMRIGGER_04);
        addEmoteForEvent(Event_OnCombatStart, SAY_MEKGINEER_STEAMRIGGER_05);
        addEmoteForEvent(Event_OnTargetDied, SAY_MEKGINEER_STEAMRIGGER_06);
        addEmoteForEvent(Event_OnTargetDied, SAY_MEKGINEER_STEAMRIGGER_07);
        addEmoteForEvent(Event_OnDied, SAY_MEKGINEER_STEAMRIGGER_09);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        GnomeCounter = 0;

        Gnomes.clear();
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        for (size_t i = 0; i < Gnomes.size(); i++)
        {
            Creature* Gnome = NULL;
            Gnome = static_cast<Creature*>(Gnomes[i]);
            if (!Gnome)
                continue;

            if (!Gnome->IsInWorld() || !Gnome->isAlive())
                continue;

            Gnome->Despawn(0, 0);
        }

        Gnomes.clear();
    }

    void AIUpdate() override
    {
        if (Gnomes.size())
        {
            Unit* Gnome = NULL;
            for (std::vector<Unit*>::iterator itr = Gnomes.begin(); itr < Gnomes.end(); ++itr)
            {
                Gnome = *itr;
                if (!Gnome->isAlive() || !Gnome->IsInWorld())
                {
                    itr = Gnomes.erase(itr);
                    continue;
                }

                if (Gnome->getAIInterface()->getCurrentTarget())
                    continue;

                if (!Gnome->getAIInterface()->getUnitToFollow())
                {
                    Gnome->getMovementManager()->moveFollow(getCreature(), 15.0f, Gnome->getFollowAngle());
                }

                if (getCreature()->GetDistance2dSq(Gnome) > 250.0f)
                {
                    if (Gnome->isCastingSpell())
                        Gnome->interruptSpell();

                    continue;
                }

                if (!Gnome->isCastingSpell())
                {
                    Gnome->pauseMovement(1);
                    Gnome->castSpell(getCreature(), REPAIR, false);    // core problem? casted on self (and effect is applied on caster instead of _unit)
                }
            }
        }

        if ((getCreature()->getHealthPct() <= 75 && GnomeCounter == 0) || (getCreature()->getHealthPct() <= 50 && GnomeCounter == 1) || (getCreature()->getHealthPct() <= 25 && GnomeCounter == 2))
        {
            Unit* Gnome = NULL;
            for (uint8_t i = 0; i < 3; i++)
            {
                Gnome = spawnCreature(CN_STEAMRIGGER_MECHANIC, SpawnCoords[i].x, SpawnCoords[i].y, SpawnCoords[i].z, SpawnCoords[i].o, getCreature()->getFactionTemplate());
                if (Gnome)
                {
                    Gnome->getMovementManager()->moveFollow(getCreature(), 15.0f, 2.0f);
                    Gnomes.push_back(Gnome);
                }
            }

            sendDBChatMessage(SAY_MEKGINEER_STEAMRIGGER_01);

            GnomeCounter++;
        }
    }

protected:
    uint8_t GnomeCounter;
};

static LocationVector Distiller[] =
{
    {  },
    { -113.183952f, -488.599335f, 8.196310f, 6.134734f },
    {  -76.880989f, -489.164673f, 8.236189f, 2.103285f },
    {  -74.309753f, -520.418884f, 8.255078f, 4.612630f },
    { -116.220764f, -520.139771f, 8.198921f, 5.127069f }
};

static LocationVector DistillerMoveTo[] =
{
    {  },
    { -108.092949f, -491.747803f, 8.198845f,  0.621336f },
    {  -81.165871f, -492.459869f, 8.255936f,  6.531955f },
    {  -79.170982f, -518.544800f, 8.241381f, -2.281880f },
    { -112.033188f, -517.945190f, 8.205022f, -0.949258f }
};

class NagaDistillerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NagaDistillerAI(c); }
    explicit NagaDistillerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        _setMeleeDisabled(true);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getCreature()->setChannelObjectGuid(0);
        getCreature()->setChannelSpellId(0);
    }
};

class WarlordKalitreshAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WarlordKalitreshAI(c); }
    explicit WarlordKalitreshAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto impale = addAISpell(IMPALE, 8.0f, TARGET_RANDOM_SINGLE, 0, 10, false, true);
        impale->setAttackStopTimer(1000);
        impale->setMinMaxDistance(0.0f, 40.0f);

        auto crack = addAISpell(HEAD_CRACK, 8.0f, TARGET_RANDOM_SINGLE, 0, 10, false, true);
        crack->setAttackStopTimer(1000);
        crack->setMinMaxDistance(0.0f, 40.0f);

        auto reflection = addAISpell(SPELL_REFLECTION, 8.0f, TARGET_SELF, 0, 25, false, true);
        reflection->setAttackStopTimer(1000);

        auto rage = addAISpell(WARLORDS_RAGE, 2.0f, TARGET_SELF, 0, 240, false, true);
        rage->setAttackStopTimer(1000);
        rage->addEmote("This is not nearly over!", CHAT_MSG_MONSTER_YELL, 10391);

        DistillerNumber = 0;
        RagePhaseTimer = 0;
        EnrageTimer = 0;
        RagePhase = 0;

        addEmoteForEvent(Event_OnCombatStart, SAY_WARLORD_KALITRESH_03);
        addEmoteForEvent(Event_OnCombatStart, SAY_WARLORD_KALITRESH_04);
        addEmoteForEvent(Event_OnCombatStart, SAY_WARLORD_KALITRESH_05);
        addEmoteForEvent(Event_OnTargetDied, SAY_WARLORD_KALITRESH_06);
        addEmoteForEvent(Event_OnTargetDied, SAY_WARLORD_KALITRESH_07);
        addEmoteForEvent(Event_OnDied, SAY_WARLORD_KALITRESH_08);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RagePhaseTimer = (uint32_t)time(NULL) + Util::getRandomUInt(15) + 10;
        DistillerNumber = 0;
        EnrageTimer = 0;
        RagePhase = 0;

        GameObject* Gate = NULL;
        Gate = getNearestGameObject(-95.774361f, -439.608612f, 3.382976f, 183049);
        if (Gate)
            Gate->setState(GO_STATE_CLOSED);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        GameObject* Gate = getNearestGameObject(-95.774361f, -439.608612f, 3.382976f, 183049);
        if (Gate)
            Gate->setState(GO_STATE_OPEN);

        getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
        getCreature()->setControlled(false, UNIT_STATE_ROOTED);
        getCreature()->getMovementManager()->clear();

        if (getCreature()->getAuraWithId(37076))
            getCreature()->removeAllAurasById(37076);
        if (getCreature()->getAuraWithId(36453))
            getCreature()->removeAllAurasById(36453);

        Unit* pDistiller = NULL;
        pDistiller = GetClosestDistiller();
        if (pDistiller)
        {
            pDistiller->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
            pDistiller->setChannelObjectGuid(0);
            pDistiller->setChannelSpellId(0);
            pDistiller->getThreatManager().clearAllThreat();
            pDistiller->getThreatManager().removeMeFromThreatLists();
        }
    }

    void AIUpdate() override
    {
        uint32_t t = (uint32_t)time(NULL);
        if (t > RagePhaseTimer)
        {
            if (EnrageTimer != 0)
                EnrageTimer++;

            Unit* pDistiller = NULL;
            pDistiller = GetClosestDistiller();
            if (!pDistiller || (pDistiller->hasUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT) && RagePhase != 0))
            {
                getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
                getCreature()->setControlled(false, UNIT_STATE_ROOTED);
                getCreature()->getMovementManager()->clear();

                RagePhaseTimer = t + Util::getRandomUInt(15) + 20;
                EnrageTimer = 0;
                RagePhase = 0;

                if (getCreature()->getAuraWithId(31543))
                    getCreature()->removeAllAurasById(31543);
                if (getCreature()->getAuraWithId(37076))
                    getCreature()->removeAllAurasById(37076);
            }

            else
            {
                if (EnrageTimer == 0 && RagePhase == 0)
                {
                    getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
                    getCreature()->getMovementManager()->moveFollow(pDistiller, 10.0f, 2.0f);

                    getCreature()->stopMoving();
                    getCreature()->getAIInterface()->moveTo(DistillerMoveTo[DistillerNumber].x, DistillerMoveTo[DistillerNumber].y, DistillerMoveTo[DistillerNumber].z);

                    if (getCreature()->GetDistance2dSq(pDistiller) <= 100.0f)
                    {
                        pDistiller->removeUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
                        pDistiller->setChannelObjectGuid(getCreature()->getGuid());
                        pDistiller->setChannelSpellId(31543);

                        getCreature()->stopMoving();
                        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
                        sendDBChatMessage(SAY_WARLORD_KALITRESH_02);

                        if (!getCreature()->getAuraWithId(36453))
                            getCreature()->castSpell(getCreature(), 31543, true);

                        EnrageTimer = t + 3;
                        RagePhase = 1;
                    }
                }

                else if (t > EnrageTimer && RagePhase == 1)
                {
                    EnrageTimer = t + 6;    // 9
                    RagePhase = 2;
                }

                else if (t > EnrageTimer && RagePhase == 2)
                {
                    pDistiller->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
                    pDistiller->setChannelObjectGuid(0);
                    pDistiller->setChannelSpellId(0);
                    pDistiller->getThreatManager().clearAllThreat();
                    pDistiller->getThreatManager().removeMeFromThreatLists();

                    getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
                    getCreature()->setControlled(false, UNIT_STATE_ROOTED);
                    getCreature()->getMovementManager()->clear();
                    getCreature()->castSpell(getCreature(), 36453, true);

                    RagePhaseTimer = t + Util::getRandomUInt(15) + 20;
                    DistillerNumber = 0;
                    EnrageTimer = 0;
                    RagePhase = 0;
                }
            }
        }
    }

    Unit* GetClosestDistiller()
    {
        float distance = 50.0f;
        Unit* pDistiller = NULL;
        Unit* Unit2Check = NULL;

        for (uint8_t i = 1; i < 5; i++)
        {
            Unit2Check = getNearestCreature(Distiller[i].x, Distiller[i].y, Distiller[i].z, 17954);
            if (!Unit2Check)
                continue;

            if (!Unit2Check->isAlive())
                continue;

            float Dist2Unit = getCreature()->GetDistance2dSq(Unit2Check);
            if (Dist2Unit > distance * distance)
                continue;

            pDistiller = Unit2Check;
            distance = std::sqrt(Dist2Unit);
            DistillerNumber = i;
        }

        return pDistiller;
    }

protected:
    uint32_t DistillerNumber;
    uint32_t RagePhaseTimer;
    uint32_t EnrageTimer;
    uint32_t RagePhase;
};

/** \todo Check all spells/creatures and evenatually balance them (if needed!)
   also add spawns correctly (no core support for now and hacky Onyxia way does
   not work (in Onyxia also). Change target type when there will be more core
   support. Also very imporant thing is to force spawns to heal random or target
   you choosed. When that will work, I will add AI for mechanics who should
   heal one of instance bosses.*/
// Don't have infos about: Second Fragment Guardian (22891) | Dreghood Slave (17799 -
// should they really have enrage ? [8269]), Driller should use Warlord's Rage spell
// (31543) to force Warlord to enrage, but I need more infos about targeting target
// you want to.
void SetupTheSteamvault(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_CF_STEAMVAULT, &TheSteamvaultInstanceScript::Create);

    mgr->register_creature_script(CN_HYDROMANCER_THESPIA, &HydromancerThespiaAI::Create);
    mgr->register_creature_script(CN_STEAMRIGGER_MECHANIC, &SteamriggerMechanicAI::Create);
    mgr->register_creature_script(CN_MEKGINEER_STEAMRIGGER, &MekgineerSteamriggerAI::Create);
    mgr->register_creature_script(CN_NAGA_DISTILLER, &NagaDistillerAI::Create);
    mgr->register_creature_script(CN_WARLORD_KALITRESH, &WarlordKalitreshAI::Create);
}
