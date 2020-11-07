/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Objects/Faction.h"
#include "Raid_Magtheridons_Lair.h"

class MagtheridonsLairInstanceScript : public InstanceScript
{
public:

    explicit MagtheridonsLairInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new MagtheridonsLairInstanceScript(pMapMgr); }


};

class MagtheridonTriggerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MagtheridonTriggerAI)
    std::vector<Unit*> ChannelersTable;    // Vector "list" of Channelers
    bool KilledChanneler[5];            // Bool that says if channeler died or not

    explicit MagtheridonTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // We set default value for KillerdChanneler array to avoid any unexpected situations
        for (uint8_t i = 0; i < 5; i++)
        {
            KilledChanneler[i] = false;
        }
        // Variable initialization
        YellTimer = YELL_TIMER;
        EventStarted = false;
        PhaseOneTimer = 0;
        Phase = 0;
        // Trigger settings
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        RegisterAIUpdateEvent(1000);
    }

    void AIUpdate() override
    {
        // Magtheridon yells when raid progresses, but during fight with himself
        YellTimer--;
        if (Phase <= 1 && !YellTimer)
        {
            // If Magtheridon can be found we let him yell one of six different texts
            Unit* Magtheridon = NULL;
            Magtheridon = getNearestCreature(-22.657900f, 2.159050f, -0.345542f, 17257);
            if (Magtheridon)
            {
                switch (Util::getRandomUInt(6))
                {
                    case 1:
                        sendDBChatMessage(8740);     // Wretched, meddling insects! Release me, and perhaps I will grant you a merciful death!
                        break;
                    case 2:
                        sendDBChatMessage(8741);     // Vermin! Leeches! Take my blood and choke on it!
                        break;
                    case 3:
                        sendDBChatMessage(8742);     // Illidan is an arrogant fool! I will crush him and reclaim Outland as my own!
                        break;
                    case 4:
                        sendDBChatMessage(8743);     // Away, you mindless parasites! My blood is my own!
                        break;
                    case 5:
                        sendDBChatMessage(8744);     // How long do you believe your pathetic sorcery can hold me?
                        break;
                    case 6:
                        sendDBChatMessage(8745);     // My blood will be the end of you!
                        break;
                    default:
                        sendDBChatMessage(8746);     // My blood will be the end of you!
                }
            }
            // We reset YellTimer to default value to let Pit Lord say something again and again
            YellTimer = YELL_TIMER;
        }
        // If our channeler "list" has unexpected size we try to recreate it
        if (ChannelersTable.size() < 5 && !EventStarted)
        {
            // We clear old "list"
            ChannelersTable.clear();
            // In order to recreate channeler "list" we need ot look for them in hardcoded spawn positions
            for (uint8_t i = 0; i < 5; i++)
            {
                Unit* Channeler = getNearestCreature(Channelers[i].x, Channelers[i].y, Channelers[i].z, 17256);
                if (!Channeler)
                    continue;
                // If Channeler was found we push him at the end of our "list"
                ChannelersTable.push_back(Channeler);
                // If Magtheridon is spawned we tell channeler to cast spell on Pit Lord
                Unit* Magtheridon = NULL;
                Magtheridon = getNearestCreature(-22.657900f, 2.159050f, -0.345542f, 17257);
                if (Magtheridon && Channeler->isAlive() && !Channeler->GetAIInterface()->getNextTarget())
                {
                    Channeler->setChannelObjectGuid(Magtheridon->getGuid());
                    Channeler->setChannelSpellId(SHADOW_GRASP);
                }
            }
        }
        // If ChannelersTable is not empty we check list to find if none of channelers died
        if (ChannelersTable.size() > 0)
        {
            // We look through list
            size_t Counter = 0;
            for (size_t i = 0; i < ChannelersTable.size(); i++)
            {
                // If channeler was already dead we count him as already dead one and go to others
                if (KilledChanneler[i])
                {
                    Counter++;
                    continue;
                }

                // Safe check to prevent memory corruptions
                Unit* Channeler = ChannelersTable[i];
                if (Channeler && !Channeler->IsInWorld())
                {
                    ChannelersTable[i] = NULL;
                    Channeler = NULL;
                    continue;
                }

                // If channeler wasn't dead before and now is not alive we buff other alive channelers
                if (Channeler && !Channeler->isAlive() && Channeler->IsInWorld())
                {
                    // We look through list of channelers to find alive ones and buff them
                    Unit* BuffedChanneler;
                    for (size_t x = 0; x < ChannelersTable.size(); x++)
                    {
                        // Safe check to prevent memory corruption
                        BuffedChanneler = ChannelersTable[x];
                        if (BuffedChanneler && !BuffedChanneler->IsInWorld())
                        {
                            ChannelersTable[x] = NULL;
                            BuffedChanneler = NULL;
                            continue;
                        }

                        // If channeler is found, alive and is not channeler we checked before if he's dead we move on
                        if (BuffedChanneler && BuffedChanneler != Channeler && BuffedChanneler->isAlive())
                        {
                            // We apply Soul Transfer Aura to channeler who should be buffed
                            Aura* aura = sSpellMgr.newAura(sSpellMgr.getSpellInfo(SOUL_TRANSFER), (uint32_t) - 1, BuffedChanneler, BuffedChanneler);
                            BuffedChanneler->addAura(aura);
                        }
                    }
                    // We count channeler which died between last and this trigger as dead and count him as dead one
                    KilledChanneler[i] = true;
                    Counter++;
                }
            }
            // If only one channeler is alive we can clear list, because we won't need other channelers to be buffed
            /*if (Counter >= ChannelersTable.size() - 1)
                ChannelersTable.clear();*/
        }
        // If table is empty (0 channelers spawned) we remove banish and go to phase 2 at once
        if (!ChannelersTable.size() && !Phase)
        {
            Unit* Magtheridon = NULL;
            Magtheridon = getNearestCreature(-22.657900f, 2.159050f, -0.345542f, 17257);
            if (Magtheridon)
            {
                Magtheridon->GetAIInterface()->SetAllowedToEnterCombat(true);
                Magtheridon->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                Magtheridon->RemoveAura(BANISHMENT);
                Magtheridon->RemoveAura(BANISH);
            }

            Phase = 2;
        }
        // If Event wasn't started yet we check if it shouldn't be marked as started
        if (!EventStarted)
        {
            // We look for channeler that may be In Combat or was killed by that unit enemy
            Unit* Channeler = NULL;
            Unit* UnitTarget = NULL;
            for (size_t i = 0; i < ChannelersTable.size(); i++)
            {
                // Safe check to prevent memory corruptions
                Channeler = ChannelersTable[i];
                if (Channeler && !Channeler->IsInWorld())
                {
                    ChannelersTable[i] = NULL;
                    Channeler = NULL;
                    continue;
                }

                // If dead or channeler In Combat is found we check if we have already copied target
                if (Channeler && Channeler->isAlive() && Channeler->GetAIInterface()->getNextTarget())
                {
                    // If channeler is In Combat and we haven't copied any target yet we copy it
                    if (Channeler->GetAIInterface()->getNextTarget() && !UnitTarget)
                    {
                        UnitTarget = Channeler->GetAIInterface()->getNextTarget();
                    }
                    // We switch phase and mark event as started
                    EventStarted = true;
                    Phase = 1;
                }
            }
            // Immediately after phase switching we check if channelers are In Combat
            if (EventStarted)
            {
                // We look through all channelers if they are In Combat and have targets
                for (size_t i = 0; i < ChannelersTable.size(); i++)
                {
                    // Safe check to prevent memory corruption
                    Channeler = ChannelersTable[i];
                    if (Channeler && !Channeler->IsInWorld())
                    {
                        ChannelersTable[i] = NULL;
                        Channeler = NULL;
                        continue;
                    }

                    // If channeler is not In Combat we force him to attack target we copied before
                    if (Channeler && !Channeler->GetAIInterface()->getNextTarget() && UnitTarget)
                    {
                        Channeler->GetAIInterface()->SetAllowedToEnterCombat(true);
                        Channeler->GetAIInterface()->AttackReaction(UnitTarget, 1, 0);
                    }
                }
                // If Magtheridon is found we remove Banish aura from him
                Unit* Magtheridon = NULL;
                Magtheridon = getNearestCreature(-22.657900f, 2.159050f, -0.345542f, 17257);
                if (Magtheridon)
                    Magtheridon->RemoveAura(BANISH);

                // If Gate is found we close it
                GameObject* Gate = getNearestGameObject(-72.5866f, 1.559f, 0.0f, 183847);
                if (Gate)
                    Gate->setState(GO_STATE_CLOSED);
            }
        }
        // We use different functions for each phase
        switch (Phase)
        {
            case 1:
                // If we are about to release Magtheridon we remove his Banishment aura, change his flag and use emotes that should be used
                PhaseOneTimer++;
                if (PhaseOneTimer == BANISH_TIMER - 2)
                {
                    Creature* Magtheridon = NULL;
                    Magtheridon = getNearestCreature(-22.657900f, 2.159050f, -0.345542f, 17257);
                    if (Magtheridon)
                    {
                        if (Util::getRandomUInt(4) == 1)
                        {
                            // on movies I saw only another text, but Magtheridon may use this one rarely too, so added here
                            Magtheridon->SendScriptTextChatMessage(8747);    // Thank you for releasing me. Now... die!
                        }
                        else
                        {
                            Magtheridon->SendScriptTextChatMessage(8748);    // I... am... unleashed!
                        }

                        Magtheridon->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
                        Magtheridon->Emote(EMOTE_ONESHOT_CREATURE_SPECIAL);
                        Magtheridon->RemoveAura(BANISHMENT);
                    }
                }
                // Time runs out, phase switches and Magtheridon can get In Combat
                if (PhaseOneTimer == BANISH_TIMER)
                {
                    Unit* Magtheridon = NULL;
                    Magtheridon = getNearestCreature(-22.657900f, 2.159050f, -0.345542f, 17257);
                    if (Magtheridon)
                    {
                        Magtheridon->GetAIInterface()->SetAllowedToEnterCombat(true);
                        Magtheridon->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
                    }

                    Phase = 2;
                }
                // Setting checks to reset encounter
                if (PhaseOneTimer == 115 || PhaseOneTimer == 105 || PhaseOneTimer == 90 || PhaseOneTimer == 75 ||
                        PhaseOneTimer == 60 || PhaseOneTimer == 45 || PhaseOneTimer == 30 || PhaseOneTimer == 15)
                {
                    // We check if any of channelers casts banish spell on Magtheridon and then we reset timer and setting
                    Unit* Magtheridon = NULL;
                    Magtheridon = getNearestCreature(-22.657900f, 2.159050f, -0.345542f, 17257);
                    if (Magtheridon)
                    {
                        Aura* aura = Magtheridon->getAuraWithId(BANISH);
                        if (aura)
                        {
                            EventStarted = false;
                            PhaseOneTimer = 0;
                            Phase = 0;
                        }
                    }
                    // Script creates vector "list" of alive channelers and counts those In Combat
                    Unit* Channeler = NULL;
                    size_t AliveInCombat = 0;
                    std::vector <Unit*> AliveChannelers;
                    for (size_t i = 0; i < ChannelersTable.size(); i++)
                    {
                        Channeler = ChannelersTable[i];
                        if (Channeler && !Channeler->IsInWorld())
                        {
                            ChannelersTable[i] = NULL;
                            Channeler = NULL;
                            continue;
                        }

                        if (Channeler && Channeler->isAlive())
                        {
                            AliveChannelers.push_back(Channeler);
                            if (Channeler->GetAIInterface()->getNextTarget())
                                AliveInCombat++;
                        }
                    }
                    // If less than half of alive channelers is out of combat we open Magtheridon's gate
                    if (AliveInCombat < AliveChannelers.size() / 2)
                    {
                        GameObject* Gate = getNearestGameObject(-72.5866f, 1.559f, 0.0f, 183847);
                        if (Gate)
                            Gate->setState(GO_STATE_OPEN);
                    }
                    // After doing our job we can clear temporary channeler list
                    AliveChannelers.clear();
                }
                break;
            default:
                {
                }
        }
    }

protected:

    bool EventStarted;

    uint32_t PhaseOneTimer;
    uint32_t YellTimer;
    uint32_t Phase;
};

class ManticronCubeGO : public GameObjectAIScript
{
public:

    explicit ManticronCubeGO(GameObject* pGameObject) : GameObjectAIScript(pGameObject)
    {
        CubeTrigger = NULL;
        MagYell = false;
        x = 0;
        y = 0;
        z = 0;
        Magtheridon = 0;
        Channeler = 0;
    }

    void OnActivate(Player* pPlayer) override
    {
        // We check if player has aura that prevents anyone from using this GO
        Aura* aura = pPlayer->getAuraWithId(MIND_EXHAUSTION);
        if (aura)
            return;

        // If we don't have Magtheridon we try to find it (with normal "getting creature" it was NOT working mostly).
        if (!Magtheridon)
        {
            for (uint8_t i = 0; i < 6; i++)
            {
                if (Magtheridon)
                    continue;

                Magtheridon = _gameobject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(CaveInPos[i].x, CaveInPos[i].y, CaveInPos[i].z, 17257);
            }
        }

        // We check if after trying to find Magtheridon we have found it at least
        if (!Magtheridon)
            return;

        // We check if Magtheridon is in world, is alive, has correct flag and so on
        if (!Magtheridon->isAlive() || !Magtheridon->GetAIInterface()->getNextTarget())
            return;

        // If we haven't "filled" pointer already we do that now
        if (!CubeTrigger)
                CubeTrigger = _gameobject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_gameobject->GetPositionX(), _gameobject->GetPositionY(), _gameobject->GetPositionZ(), 17376);

        // We check if Cube Trigger we want to use exists and if is alive
        if (!CubeTrigger || (CubeTrigger && !CubeTrigger->isAlive()))
            return;

        // We check if Cube Trigger is not in use
        if (CubeTrigger && CubeTrigger->getChannelSpellId() == SHADOW_GRASP && CubeTrigger->getChannelObjectGuid() == Magtheridon->getGuid())
            return;

        // We set player to channel spell "on Cube"
        pPlayer->castSpell(pPlayer, sSpellMgr.getSpellInfo(SHADOW_GRASP2), false);

        // We trigger channeling spell on Magtheridon for Cube Trigger
        CubeTrigger->setChannelObjectGuid(Magtheridon->getGuid());
        CubeTrigger->setChannelSpellId(SHADOW_GRASP);

        // We save player data in pointer as well as his position for further use
        x = pPlayer->GetPositionX();
        y = pPlayer->GetPositionY();
        z = pPlayer->GetPositionZ();
        Channeler = pPlayer;

        // We save/initialize vars
        MagYell = false;

        // We register AI to check if GO is still being used
        RegisterAIUpdateEvent(1000);
    }

    void AIUpdate() override
    {
        // Channeler settings check
        // We check if pointer has Channeler data and if so we check if that channeler is alive, in world and if channels Cube
        if (Channeler && (!Channeler->isAlive() || !Channeler->IsInWorld()))
        {
            CubeTrigger->setChannelObjectGuid(0);
            CubeTrigger->setChannelSpellId(0);

            Channeler = NULL;
        }

        // If player still exists (is in world, alive and so on) we check if he has "channeling aura"
        Aura* aura = NULL;
        if (Channeler)
            aura = Channeler->getAuraWithId(SHADOW_GRASP2);

        // If player doesn't have aura we interrupt channeling
        if (Channeler && (!aura || !Channeler->getChannelObjectGuid()))
        {
            CubeTrigger->setChannelObjectGuid(0);
            CubeTrigger->setChannelSpellId(0);

            // If player's channeling went over (and he was hit before) aura won't be removed when channeling ends - core bug
            Channeler->RemoveAura(SHADOW_GRASP2);

            Channeler = NULL;
        }

        // Safe check to prevent crashes when Channeler was nulled
        if (!Channeler)
        {
            uint32_t Counter = 0;
            for (uint8_t i = 0; i < 5; i++)
            {
                Unit* GlobalCubeTrigger = NULL;
                GlobalCubeTrigger = _gameobject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(CubeTriggers[i].x, CubeTriggers[i].y, CubeTriggers[i].z, 17376);
                if (Magtheridon != nullptr)
                    if (GlobalCubeTrigger && GlobalCubeTrigger->getChannelSpellId() == SHADOW_GRASP && CubeTrigger->getChannelObjectGuid() == Magtheridon->getGuid())
                        Counter++;
            }

            if (!Counter)
            {
                if (Magtheridon && Magtheridon->isAlive())
                    Magtheridon->RemoveAura(BANISH);

                MagYell = true;
            }

            return;
        }

        // We check if Magtheridon is spawned, is in world and so on
        if (!Magtheridon || (Magtheridon && (!Magtheridon->isAlive() || !Magtheridon->IsInWorld() || !Magtheridon->GetAIInterface()->getNextTarget())))
        {
            CubeTrigger->setChannelObjectGuid(0);
            CubeTrigger->setChannelSpellId(0);
        }

        // We count Cubes that channel spell on Magtheridon
        uint32_t Counter = 0;
        for (uint8_t i = 0; i < 5; i++)
        {
            Unit* GlobalCubeTrigger = NULL;
            GlobalCubeTrigger = _gameobject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(CubeTriggers[i].x, CubeTriggers[i].y, CubeTriggers[i].z, 17376);
            if (Magtheridon != nullptr)
                if (GlobalCubeTrigger && GlobalCubeTrigger->getChannelSpellId() == SHADOW_GRASP && CubeTrigger->getChannelObjectGuid() == Magtheridon->getGuid())
            Counter++;
        }

        // If it's the first and the only one Cube triggering spell we use Magtheridon's yell
        if (Counter == 1 && !MagYell)
        {
            Magtheridon->SendScriptTextChatMessage(8749);       // "Not again... NOT AGAIN!

            MagYell = true;
        }

        // If we have all req. Cubes active we may banish Magtheridon
        if (Counter >= ACTIVE_CUBES_TO_BANISH && Magtheridon && Magtheridon->isAlive())
        {
            Magtheridon->castSpell(Magtheridon, sSpellMgr.getSpellInfo(BANISH), true);
            Magtheridon->GetAIInterface()->StopMovement(3000);
            Magtheridon->setAttackTimer(MELEE, 3000);

            if (Magtheridon->isCastingSpell())
                Magtheridon->interruptSpell();

            // We add channeling player aura that does not allow that go to be used again in 1.3 min
            Aura* auraT = sSpellMgr.newAura(sSpellMgr.getSpellInfo(MIND_EXHAUSTION), (int32_t)78000, Magtheridon, Channeler);
            Channeler->addAura(auraT);

            MagYell = true;

            return;
        }

        // If not enough Cubes are active we eventually Banish from Magtheridon
        if (Counter < ACTIVE_CUBES_TO_BANISH && Magtheridon && Magtheridon->isAlive())
        {
            Magtheridon->RemoveAura(BANISH);

            MagYell = true;
        }

    }

    static GameObjectAIScript* Create(GameObject* GO) { return new ManticronCubeGO(GO); }

protected:

    bool MagYell;
    float x, y, z;
    Creature* Magtheridon;
    Player* Channeler;
    Unit* CubeTrigger;
};

class CubeTriggerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CubeTriggerAI)
    explicit CubeTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
    }
};

class HellfireWarderAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HellfireWarderAI)
    explicit HellfireWarderAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto shadowBoltVolley = addAISpell(HW_SHADOW_BOLT_VOLLEY, 15.0f, TARGET_VARIOUS, 0, 5, false, true);
        shadowBoltVolley->setAttackStopTimer(1000);

        auto wordPain = addAISpell(SHADOW_WORD_PAIN, 6.0f, TARGET_RANDOM_SINGLE, 0, 7, false, true);
        wordPain->setAttackStopTimer(1000);
        wordPain->setMinMaxDistance(0.0f, 30.0f);

        auto unstableAfflection = addAISpell(UNSTABLE_AFFLICTION, 6.0f, TARGET_RANDOM_SINGLE, 0, 7, false, true);
        unstableAfflection->setAttackStopTimer(1000);
        unstableAfflection->setMinMaxDistance(0.0f, 30.0f);

        auto deathCoil = addAISpell(DEATH_COIL, 5.0f, TARGET_RANDOM_SINGLE, 0, 8, false, true);
        deathCoil->setAttackStopTimer(1000);
        deathCoil->setMinMaxDistance(0.0f, 30.0f);

        auto rainOfFire = addAISpell(RAIN_OF_FIRE, 5.0f, TARGET_RANDOM_DESTINATION, 0, 6, false, true);
        rainOfFire->setAttackStopTimer(1000);
        rainOfFire->setMinMaxDistance(0.0f, 30.0f);

        auto fear = addAISpell(HW_FEAR, 4.0f, TARGET_RANDOM_SINGLE, 0, 10, false, true);
        fear->setAttackStopTimer(1000);
        fear->setMinMaxDistance(0.0f, 30.0f);

        auto shadowBurst = addAISpell(SHADOW_BURST, 4.0f, TARGET_VARIOUS, 0, 8);
        shadowBurst->setAttackStopTimer(1000);
        shadowBurst->setMinMaxDistance(0.0f, 30.0f);
    }
};

class HellfireChannelerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HellfireChannelerAI)
    explicit HellfireChannelerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto shadowBoltVolley = addAISpell(SHADOW_BOLT_VOLLEY, 10.0f, TARGET_VARIOUS, 0, 5);
        shadowBoltVolley->setAttackStopTimer(1000);

        auto fear = addAISpell(FEAR, 7.0f, TARGET_RANDOM_SINGLE, 0, 10);
        fear->setAttackStopTimer(1000);
        fear->setMinMaxDistance(0.0f, 30.0f);

        auto darkMending = addAISpell(DARK_MENDING, 8.0f, TARGET_RANDOM_FRIEND, 0, 10);
        darkMending->setAttackStopTimer(1000);
        darkMending->setMinMaxDistance(0.0f, 40.0f);
        darkMending->setMinMaxPercentHp(0, 70);

        auto burningAbyssal = addAISpell(BURNING_ABYSSAL, 6.0f, TARGET_RANDOM_SINGLE, 0, 30, false, true);
        burningAbyssal->setAttackStopTimer(1000);
        burningAbyssal->setMinMaxDistance(0.0f, 30.0f);

        auto soulTransfer = addAISpell(SOUL_TRANSFER, 2.0f, TARGET_VARIOUS, 0, 10, false, true);
        soulTransfer->setAttackStopTimer(1000);
        soulTransfer->setMinMaxDistance(0.0f, 30.0f);

        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->setChannelObjectGuid(0);
        getCreature()->setChannelSpellId(0);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        if (isAlive())
        {

            Unit* Magtheridon = NULL;
            Magtheridon = getNearestCreature(-22.657900f, 2.159050f, -0.345542f, 17257);
            if (Magtheridon && Magtheridon->hasUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT))
            {
                getCreature()->setChannelObjectGuid(Magtheridon->getGuid());
                getCreature()->setChannelSpellId(SHADOW_GRASP);

                Magtheridon->castSpell(Magtheridon, sSpellMgr.getSpellInfo(BANISH), true);
            }
        }
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t /*fAmount*/) override
    {
        if (!getCreature()->GetAIInterface()->GetAllowedToEnterCombat())
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getCreature()->setChannelObjectGuid(0);
        getCreature()->setChannelSpellId(0);
    }
};

class BurningAbyssalAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BurningAbyssalAI)
    explicit BurningAbyssalAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto fireBlast = addAISpell(FIRE_BLAST, 8.0f, TARGET_RANDOM_SINGLE, 0, 10, false, true);
        fireBlast->setAttackStopTimer(1000);
        fireBlast->setMinMaxDistance(0.0f, 20.0f);

        getCreature()->m_noRespawn = true;
    }
};

class MagtheridonAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MagtheridonAI)
    explicit MagtheridonAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto cleave = addAISpell(CLEAVE, 6.0f, TARGET_ATTACKING, 0, 15, false, true);
        cleave->setAttackStopTimer(1000);

        auto conflagration = addAISpell(CONFLAGRATION, 7.0f, TARGET_RANDOM_SINGLE, 0, 35, false, true);
        conflagration->setAttackStopTimer(1000);
        conflagration->setMinMaxDistance(0.0f, 30.0f);

        quake1 = addAISpell(QUAKE1, 0.0f, TARGET_VARIOUS);
        quake2 = addAISpell(QUAKE2, 0.0f, TARGET_VARIOUS);
        caveIn = addAISpell(CAVE_IN, 0.0f, TARGET_VARIOUS);

        getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);

        Aura* aura = sSpellMgr.newAura(sSpellMgr.getSpellInfo(BANISHMENT), (uint32_t) - 1, getCreature(), getCreature());
        getCreature()->addAura(aura);

        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(BANISH), true);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->setSheathType(SHEATH_STATE_MELEE);

        timer_quake = timer_enrage = timer_blastNova = timer_caveIn = 0;
        PhaseSwitch = false;

        addEmoteForEvent(Event_OnTargetDied, 8751);     // Did you think me weak? Soft? Who is the weak one now?!
        addEmoteForEvent(Event_OnDied, 8750);           // The Legion... will consume you... all....
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        timer_quake = timer_enrage = timer_blastNova = timer_caveIn = 0;
        PhaseSwitch = false;

        RegisterAIUpdateEvent(getCreature()->getBaseAttackTime(MELEE));

        GameObject* Gate = getNearestGameObject(-72.5866f, 1.559f, 0.0f, 183847);
        if (Gate)
            Gate->setState(GO_STATE_CLOSED);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        if (getCreature()->hasUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT) || getCreature()->hasUnitFlags(UNIT_FLAG_NON_ATTACKABLE))
            return;

        GameObject* Gate = NULL;
        for (uint8_t i = 0; i < 6; i++)
        {
            Gate = getNearestGameObject(Columns[i].x, Columns[i].y, Columns[i].z, 184634 + i);
            if (Gate)
                Gate->setState(GO_STATE_CLOSED);
        }

        Gate = getNearestGameObject(0.0f, 0.0f, 0.0f, 184653);
        if (Gate)
            Gate->setState(GO_STATE_CLOSED);

        Gate = getNearestGameObject(-72.5866f, 1.559f, 0.0f, 183847);
        if (Gate)
            Gate->setState(GO_STATE_OPEN);
    }

    void AIUpdate() override
    {
        if (PhaseSwitch)
            PhaseThree();
        else PhaseTwo();
    }

    void PhaseTwo()
    {
        Aura* aura = getCreature()->getAuraWithId(BANISH);

        if (getCreature()->getHealthPct() <= 30)
        {
            timer_caveIn = 1;
            PhaseSwitch = true;
        }

        timer_quake++;
        timer_enrage++;
        timer_blastNova++;

        if (timer_quake > 27)
        {
            if (timer_quake < 33)
            {
                getCreature()->castSpell(getCreature(), quake1->mSpellInfo, true);

                for (uint8_t i = 0; i < 6; i++)
                {
                    Unit* Trigger = getNearestCreature(CaveInPos[i].x, CaveInPos[i].y, CaveInPos[i].z, 17474);
                    if (Trigger)
                    {
                        Trigger->castSpell(Trigger, quake2->mSpellInfo, true);
                    }
                }
            }

            if (timer_quake == 32)
                timer_quake = 0;
        }

        if (timer_blastNova == 33)
        {
            getCreature()->SendChatMessageAlternateEntry(17257, CHAT_MSG_EMOTE, LANG_UNIVERSAL, " begins to cast Blast Nova!");
        }
        if (timer_blastNova > 33 && !getCreature()->isCastingSpell() && !aura)
        {
            getCreature()->GetAIInterface()->StopMovement(3000);
            getCreature()->setAttackTimer(MELEE, 3000);

            getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(BLAST_NOVA), false);

            timer_blastNova = 0;
            timer_quake = 0;
            return;
        }

        if (timer_enrage > 667 && !getCreature()->isCastingSpell() && !aura)
        {
            getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(ENRAGE), true);

            timer_enrage = 0;
        }
    }

    void PhaseThree()
    {
        Aura* aura = getCreature()->getAuraWithId(BANISH);

        timer_quake++;
        timer_enrage++;
        timer_blastNova++;

        if (timer_quake > 27)
        {
            if (timer_quake < 33)
            {
                getCreature()->castSpell(getCreature(), quake1->mSpellInfo, true);

                for (uint8_t i = 0; i < 6; i++)
                {
                    Unit* Trigger = getNearestCreature(CaveInPos[i].x, CaveInPos[i].y, CaveInPos[i].z, 17474);
                    if (Trigger)
                    {
                        Trigger->castSpell(Trigger, quake2->mSpellInfo, true);
                    }
                }
            }

            if (timer_quake == 32)
                timer_quake = 0;
        }

        if (timer_blastNova == 33)
        {
            getCreature()->SendChatMessageAlternateEntry(17257, CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, " begins to cast Blast Nova!");
        }
        if (timer_blastNova > 33 && !getCreature()->isCastingSpell() && !aura)
        {
            getCreature()->GetAIInterface()->StopMovement(3000);
            getCreature()->setAttackTimer(MELEE, 3000);

            getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(BLAST_NOVA), false);

            timer_blastNova = 0;
            timer_quake = 0;
            return;
        }

        if (timer_caveIn && (timer_caveIn != 1 || (!getCreature()->isCastingSpell() && timer_caveIn == 1 && !aura)))
        {
            timer_caveIn++;
            if (timer_caveIn == 2)
            {
                sendDBChatMessage(8752);     // I will not be taken so easily. Let the walls of this prison tremble... and FALL!!!

                getCreature()->GetAIInterface()->StopMovement(2000);
                getCreature()->setAttackTimer(MELEE, 2000);

                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CAMERA_SHAKE), true);
                return;
            }

            if (timer_caveIn == 3)
            {
                GameObject* Gate = NULL;
                for (uint8_t i = 0; i < 6; i++)
                {
                    Gate = getNearestGameObject(Columns[i].x, Columns[i].y, Columns[i].z, 184634 + i);
                    if (Gate)
                        Gate->setState(GO_STATE_OPEN);
                }

                Gate = getNearestGameObject(0.0f, 0.0f, 0.0f, 184653);
                if (Gate)
                    Gate->setState(GO_STATE_OPEN);
            }

            if (timer_caveIn == 5)
            {
                for (uint8_t i = 0; i < 6; i++)
                {
                    Unit* Trigger = getNearestCreature(CaveInPos[i].x, CaveInPos[i].y, CaveInPos[i].z, 17474);
                    if (Trigger)
                    {
                        Trigger->castSpellLoc(LocationVector(CaveInPos[i].x, CaveInPos[i].y, CaveInPos[i].z), caveIn->mSpellInfo, true);
                    }
                }

                timer_caveIn = 0;
            }
        }

        if (timer_enrage > 667 && !getCreature()->isCastingSpell() && !aura)
        {
            getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(ENRAGE), true);

            timer_enrage = 0;
        }
    }

protected:

    int timer_quake;
    int timer_enrage;
    int timer_caveIn;
    int timer_blastNova;

    bool PhaseSwitch;

    CreatureAISpells* quake1;
    CreatureAISpells* quake2;
    //CreatureAISpells* blastNova;
    CreatureAISpells* caveIn;
    //CreatureAISpells* enrage;
};

void SetupMagtheridonsLair(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MAGTHERIDONS_LAIR, &MagtheridonsLairInstanceScript::Create);

    mgr->register_gameobject_script(MANTICRON_CUBE, &ManticronCubeGO::Create);

    mgr->register_creature_script(CN_MAGTHERIDON_TRIGGER, &MagtheridonTriggerAI::Create);
    mgr->register_creature_script(CN_CUBE_TRIGGER, &CubeTriggerAI::Create);
    mgr->register_creature_script(CN_HELLFIRE_WARDER, &HellfireWarderAI::Create);
    mgr->register_creature_script(CN_HELLFIRE_CHANNELER, &HellfireChannelerAI::Create);
    mgr->register_creature_script(CN_BURNING_ABYSSAL, &BurningAbyssalAI::Create);
    mgr->register_creature_script(CN_MAGTHERIDON, &MagtheridonAI::Create);
}
