/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
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
#include "Spell/SpellMgr.h"


// \todo Fire Magtheridon throws around the battlefield.

// Encounter Settings
const uint32 BANISH_TIMER = 120;       // 2 minutes
const uint32 YELL_TIMER = 260;       // 4 minutes and 20 seconds
const uint32 ACTIVE_CUBES_TO_BANISH = 5;        // 5 cubes

// Channelers Coords is list of spawn points of all 5 channelers casting spell on Magtheridon
static Movement::Location Channelers[] =
{
    { -55.638000f,   1.869050f, 0.630946f },
    { -31.861300f, -35.919399f, 0.630945f },
    {  10.469200f, -19.894800f, 0.630910f },
    {  10.477100f,  24.445499f, 0.630891f },
    { -32.171600f,  39.926800f, 0.630921f }
};

// Columns coords used for Cave In spell to give "collapse" effect
static Movement::Location Columns[] =
{
    {  17.7522f,  34.5464f,  0.144816f },
    {  19.0966f, -29.2772f,  0.133036f },
    { -30.8852f,  46.6723f, -0.497104f },
    { -60.2491f,  27.9361f, -0.018443f },
    { -60.8202f, -21.9269f, -0.030260f },
    { -29.7699f, -43.4445f, -0.522461f }
};

// Cave In Target Triggers coords
static Movement::Location CaveInPos[] =
{
    { -37.183399f, -19.491400f,  0.312451f },
    { -11.374900f, -29.121401f,  0.312463f },
    {  13.133100f,   2.757930f, -0.312492f },
    {  -3.110930f,  29.142401f, -0.312490f },
    { -29.691000f,  29.643000f, -0.034676f },
    { -12.111600f,   1.011050f, -0.303638f }
};

// Cube Triggers coords
static Movement::Location CubeTriggers[] =
{
    { -54.277199f,   2.343740f, 2.404560f },
    { -31.471001f, -34.155998f, 2.335100f },
    {   8.797220f, -19.480101f, 2.536460f },
    {   9.358900f,  23.228600f, 2.348950f },
    { -31.891800f,  38.430302f, 2.286470f }
};

// Magtheridon Trigger AI - Creature AI Class
const uint32 CN_MAGTHERIDON_TRIGGER = 133338;

// Spell macros used in whole script
const uint32 BANISH = 30231;    // 31797
const uint32 BANISHMENT = 40825;
const uint32 SOUL_TRANSFER = 30531;
const uint32 SHADOW_GRASP = 30166;   //30207
const uint32 SHADOW_GRASP2 = 30410;

class MagtheridonTriggerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MagtheridonTriggerAI);
        std::vector<Unit*> ChannelersTable;    // Vector "list" of Channelers
        bool KilledChanneler[5];            // Bool that says if channeler died or not

        MagtheridonTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // We set default value for KillerdChanneler array to avoid any unexpected situations
            for (uint8 i = 0; i < 5; i++)
            {
                KilledChanneler[i] = false;
            }
            // Variable initialization
            YellTimer = YELL_TIMER;
            EventStarted = false;
            PhaseOneTimer = 0;
            Phase = 0;
            // Trigger settings
            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
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
                for (uint8 i = 0; i < 5; i++)
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
                        Channeler->SetChannelSpellTargetGUID(Magtheridon->GetGUID());
                        Channeler->SetChannelSpellId(SHADOW_GRASP);
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
                                Aura* aura = sSpellFactoryMgr.NewAura(sSpellCustomizations.GetSpellInfo(SOUL_TRANSFER), (uint32) - 1, BuffedChanneler, BuffedChanneler);
                                BuffedChanneler->AddAura(aura);
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
                    Magtheridon->setUInt64Value(UNIT_FIELD_FLAGS, 0);
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
                        Gate->SetState(GO_STATE_CLOSED);
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

                            Magtheridon->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
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
                            Magtheridon->setUInt64Value(UNIT_FIELD_FLAGS, 0);
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
                                Gate->SetState(GO_STATE_OPEN);
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

        uint32 PhaseOneTimer;
        uint32 YellTimer;
        uint32 Phase;
};

// Manticron Cube Gameobject

const uint32 MANTICRON_CUBE = 181713;

const uint32 MIND_EXHAUSTION = 44032;

class ManticronCubeGO : public GameObjectAIScript
{
    public:
        ManticronCubeGO(GameObject* pGameObject) : GameObjectAIScript(pGameObject)
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
                for (uint8 i = 0; i < 6; i++)
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
            if (CubeTrigger && CubeTrigger->GetChannelSpellId() == SHADOW_GRASP && CubeTrigger->getUInt64Value(UNIT_FIELD_CHANNEL_OBJECT) == Magtheridon->GetGUID())
                return;

            // We set player to channel spell "on Cube"
            pPlayer->CastSpell(pPlayer, sSpellCustomizations.GetSpellInfo(SHADOW_GRASP2), false);

            // We trigger channeling spell on Magtheridon for Cube Trigger
            CubeTrigger->SetChannelSpellTargetGUID(Magtheridon->GetGUID());
            CubeTrigger->SetChannelSpellId(SHADOW_GRASP);

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
                CubeTrigger->SetChannelSpellTargetGUID(0);
                CubeTrigger->SetChannelSpellId(0);

                Channeler = NULL;
            }

            // If player still exists (is in world, alive and so on) we check if he has "channeling aura"
            Aura* aura = NULL;
            if (Channeler)
                aura = Channeler->getAuraWithId(SHADOW_GRASP2);

            // If player doesn't have aura we interrupt channeling
            if (Channeler && (!aura || !Channeler->getUInt64Value(UNIT_FIELD_CHANNEL_OBJECT)))
            {
                CubeTrigger->SetChannelSpellTargetGUID(0);
                CubeTrigger->SetChannelSpellId(0);

                // If player's channeling went over (and he was hit before) aura won't be removed when channeling ends - core bug
                Channeler->RemoveAura(SHADOW_GRASP2);

                Channeler = NULL;
            }

            // Safe check to prevent crashes when Channeler was nulled
            if (!Channeler)
            {
                uint32 Counter = 0;
                for (uint8 i = 0; i < 5; i++)
                {
                    Unit* GlobalCubeTrigger = NULL;
                    GlobalCubeTrigger = _gameobject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(CubeTriggers[i].x, CubeTriggers[i].y, CubeTriggers[i].z, 17376);
                    if (Magtheridon != nullptr)
                        if (GlobalCubeTrigger && GlobalCubeTrigger->GetChannelSpellId() == SHADOW_GRASP && CubeTrigger->getUInt64Value(UNIT_FIELD_CHANNEL_OBJECT) == Magtheridon->GetGUID())
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
                CubeTrigger->SetChannelSpellTargetGUID(0);
                CubeTrigger->SetChannelSpellId(0);
            }

            // We count Cubes that channel spell on Magtheridon
            uint32 Counter = 0;
            for (uint8 i = 0; i < 5; i++)
            {
                Unit* GlobalCubeTrigger = NULL;
                GlobalCubeTrigger = _gameobject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(CubeTriggers[i].x, CubeTriggers[i].y, CubeTriggers[i].z, 17376);
                if (Magtheridon != nullptr)
                    if (GlobalCubeTrigger && GlobalCubeTrigger->GetChannelSpellId() == SHADOW_GRASP && CubeTrigger->getUInt64Value(UNIT_FIELD_CHANNEL_OBJECT) == Magtheridon->GetGUID())
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
                Magtheridon->CastSpell(Magtheridon, sSpellCustomizations.GetSpellInfo(BANISH), true);
                Magtheridon->GetAIInterface()->StopMovement(3000);
                Magtheridon->setAttackTimer(3000, false);

                if (Magtheridon->isCastingNonMeleeSpell())
                    Magtheridon->interruptSpell();

                // We add channeling player aura that does not allow that go to be used again in 1.3 min
                Aura* auraT = sSpellFactoryMgr.NewAura(sSpellCustomizations.GetSpellInfo(MIND_EXHAUSTION), (int32)78000, Magtheridon, Channeler);
                Channeler->AddAura(auraT);

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

const uint32 CN_CUBE_TRIGGER = 17376;

class CubeTriggerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CubeTriggerAI);
        CubeTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        }
};

// Hellfire Warder
const uint32 CN_HELLFIRE_WARDER = 18829;

const uint32 HW_SHADOW_BOLT_VOLLEY = 36275;   // 39175
const uint32 SHADOW_WORD_PAIN = 34441;
const uint32 UNSTABLE_AFFLICTION = 35183;
const uint32 DEATH_COIL = 33130;
const uint32 RAIN_OF_FIRE = 34435;
const uint32 HW_FEAR = 34259;    // this one is probably wrong
const uint32 SHADOW_BURST = 34436;

class HellfireWarderAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HellfireWarderAI);
        HellfireWarderAI(Creature* pCreature) : CreatureAIScript(pCreature)
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

// Hellfire Channeler
const uint32 CN_HELLFIRE_CHANNELER = 17256;

const uint32 SHADOW_BOLT_VOLLEY = 30510;    // 39175
const uint32 FEAR = 30615;   // not sure
const uint32 DARK_MENDING = 30528;
const uint32 BURNING_ABYSSAL = 30511;

class HellfireChannelerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HellfireChannelerAI);
        HellfireChannelerAI(Creature* pCreature) : CreatureAIScript(pCreature)
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
            getCreature()->SetChannelSpellTargetGUID(0);
            getCreature()->SetChannelSpellId(0);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            if (isAlive())
            {

                Unit* Magtheridon = NULL;
                Magtheridon = getNearestCreature(-22.657900f, 2.159050f, -0.345542f, 17257);
                if (Magtheridon && Magtheridon->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IGNORE_PLAYER_COMBAT))
                {
                    getCreature()->SetChannelSpellTargetGUID(Magtheridon->GetGUID());
                    getCreature()->SetChannelSpellId(SHADOW_GRASP);

                    Magtheridon->CastSpell(Magtheridon, sSpellCustomizations.GetSpellInfo(BANISH), true);
                }
            }
        }

        void OnDamageTaken(Unit* /*mAttacker*/, uint32 /*fAmount*/) override
        {
            if (!getCreature()->GetAIInterface()->GetAllowedToEnterCombat())
                getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            getCreature()->SetChannelSpellTargetGUID(0);
            getCreature()->SetChannelSpellId(0);
        }
};


const uint32 CN_BURNING_ABYSSAL = 17454;

const uint32 FIRE_BLAST = 37110;

class BurningAbyssalAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(BurningAbyssalAI);
        BurningAbyssalAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto fireBlast = addAISpell(FIRE_BLAST, 8.0f, TARGET_RANDOM_SINGLE, 0, 10, false, true);
            fireBlast->setAttackStopTimer(1000);
            fireBlast->setMinMaxDistance(0.0f, 20.0f);

            getCreature()->m_noRespawn = true;
        }
};

// Magtheridon
const uint32 CN_MAGTHERIDON = 17257;

// Normal Casts
const uint32 CLEAVE = 31345;    // not sure; should be better, but not sure if it gives 8k dmg
const uint32 CONFLAGRATION = 23023;   // 35840 - this one was affecting caster; not sure - it's not right spell, but better than nothing for now

// Timed Casts
const uint32 QUAKE1 = 30571;   // Each 40 sec after Phase 2 starts
const uint32 QUAKE2 = 30658;    // Effect
const uint32 BLAST_NOVA = 30616;    // Each 60 sec after Phase 2 starts
const uint32 CAVE_IN = 36240;    // don't know correct timer
const uint32 CAMERA_SHAKE = 36455;    // is used when Magtheridon uses Cave In
const uint32 ENRAGE = 34624;    // dunno if it's correct spell    -- 20 min after Phase 2 starts

class MagtheridonAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MagtheridonAI);
        MagtheridonAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto cleave = addAISpell(CLEAVE, 6.0f, TARGET_ATTACKING, 0, 15, false, true);
            cleave->setAttackStopTimer(1000);

            auto conflagration = addAISpell(CONFLAGRATION, 7.0f, TARGET_RANDOM_SINGLE, 0, 35, false, true);
            conflagration->setAttackStopTimer(1000);
            conflagration->setMinMaxDistance(0.0f, 30.0f);

            quake1 = addAISpell(QUAKE1, 0.0f, TARGET_VARIOUS);
            quake2 = addAISpell(QUAKE2, 0.0f, TARGET_VARIOUS);
            caveIn = addAISpell(CAVE_IN, 0.0f, TARGET_VARIOUS);

            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_IGNORE_PLAYER_COMBAT);

            Aura* aura = sSpellFactoryMgr.NewAura(sSpellCustomizations.GetSpellInfo(BANISHMENT), (uint32) - 1, getCreature(), getCreature());
            getCreature()->AddAura(aura);

            getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(BANISH), true);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            getCreature()->setUInt32Value(UNIT_FIELD_BYTES_2, 1);

            timer_quake = timer_enrage = timer_blastNova = timer_caveIn = 0;
            PhaseSwitch = false;

            addEmoteForEvent(Event_OnTargetDied, 8751);     // Did you think me weak? Soft? Who is the weak one now?!
            addEmoteForEvent(Event_OnDied, 8750);           // The Legion... will consume you... all....
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            timer_quake = timer_enrage = timer_blastNova = timer_caveIn = 0;
            PhaseSwitch = false;

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));

            GameObject* Gate = getNearestGameObject(-72.5866f, 1.559f, 0.0f, 183847);
            if (Gate)
                Gate->SetState(GO_STATE_CLOSED);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            if (getCreature()->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IGNORE_PLAYER_COMBAT) || getCreature()->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2))
                return;

            GameObject* Gate = NULL;
            for (uint8 i = 0; i < 6; i++)
            {
                Gate = getNearestGameObject(Columns[i].x, Columns[i].y, Columns[i].z, 184634 + i);
                if (Gate)
                    Gate->SetState(GO_STATE_CLOSED);
            }

            Gate = getNearestGameObject(0.0f, 0.0f, 0.0f, 184653);
            if (Gate)
                Gate->SetState(GO_STATE_CLOSED);

            Gate = getNearestGameObject(-72.5866f, 1.559f, 0.0f, 183847);
            if (Gate)
                Gate->SetState(GO_STATE_OPEN);
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

            if (getCreature()->GetHealthPct() <= 30)
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
                    getCreature()->CastSpell(getCreature(), quake1->mSpellInfo, true);

                    for (uint8 i = 0; i < 6; i++)
                    {
                        Unit* Trigger = getNearestCreature(CaveInPos[i].x, CaveInPos[i].y, CaveInPos[i].z, 17474);
                        if (Trigger)
                        {
                            Trigger->CastSpell(Trigger, quake2->mSpellInfo, true);
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
            if (timer_blastNova > 33 && !getCreature()->isCastingNonMeleeSpell() && !aura)
            {
                getCreature()->GetAIInterface()->StopMovement(3000);
                getCreature()->setAttackTimer(3000, false);

                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(BLAST_NOVA), false);

                timer_blastNova = 0;
                timer_quake = 0;
                return;
            }

            if (timer_enrage > 667 && !getCreature()->isCastingNonMeleeSpell() && !aura)
            {
                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(ENRAGE), true);

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
                    getCreature()->CastSpell(getCreature(), quake1->mSpellInfo, true);

                    for (uint8 i = 0; i < 6; i++)
                    {
                        Unit* Trigger = getNearestCreature(CaveInPos[i].x, CaveInPos[i].y, CaveInPos[i].z, 17474);
                        if (Trigger)
                        {
                            Trigger->CastSpell(Trigger, quake2->mSpellInfo, true);
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
            if (timer_blastNova > 33 && !getCreature()->isCastingNonMeleeSpell() && !aura)
            {
                getCreature()->GetAIInterface()->StopMovement(3000);
                getCreature()->setAttackTimer(3000, false);

                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(BLAST_NOVA), false);

                timer_blastNova = 0;
                timer_quake = 0;
                return;
            }

            if (timer_caveIn && (timer_caveIn != 1 || (!getCreature()->isCastingNonMeleeSpell() && timer_caveIn == 1 && !aura)))
            {
                timer_caveIn++;
                if (timer_caveIn == 2)
                {
                    sendDBChatMessage(8752);     // I will not be taken so easily. Let the walls of this prison tremble... and FALL!!!

                    getCreature()->GetAIInterface()->StopMovement(2000);
                    getCreature()->setAttackTimer(2000, false);

                    getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(CAMERA_SHAKE), true);
                    return;
                }

                if (timer_caveIn == 3)
                {
                    GameObject* Gate = NULL;
                    for (uint8 i = 0; i < 6; i++)
                    {
                        Gate = getNearestGameObject(Columns[i].x, Columns[i].y, Columns[i].z, 184634 + i);
                        if (Gate)
                            Gate->SetState(GO_STATE_OPEN);
                    }

                    Gate = getNearestGameObject(0.0f, 0.0f, 0.0f, 184653);
                    if (Gate)
                        Gate->SetState(GO_STATE_OPEN);
                }

                if (timer_caveIn == 5)
                {
                    for (uint8 i = 0; i < 6; i++)
                    {
                        Unit* Trigger = getNearestCreature(CaveInPos[i].x, CaveInPos[i].y, CaveInPos[i].z, 17474);
                        if (Trigger)
                        {
                            Trigger->CastSpellAoF(LocationVector(CaveInPos[i].x, CaveInPos[i].y, CaveInPos[i].z), caveIn->mSpellInfo, true);
                        }
                    }

                    timer_caveIn = 0;
                }
            }

            if (timer_enrage > 667 && !getCreature()->isCastingNonMeleeSpell() && !aura)
            {
                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(ENRAGE), true);

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
    mgr->register_gameobject_script(MANTICRON_CUBE, &ManticronCubeGO::Create);

    mgr->register_creature_script(CN_MAGTHERIDON_TRIGGER, &MagtheridonTriggerAI::Create);
    mgr->register_creature_script(CN_CUBE_TRIGGER, &CubeTriggerAI::Create);
    mgr->register_creature_script(CN_HELLFIRE_WARDER, &HellfireWarderAI::Create);
    mgr->register_creature_script(CN_HELLFIRE_CHANNELER, &HellfireChannelerAI::Create);
    mgr->register_creature_script(CN_BURNING_ABYSSAL, &BurningAbyssalAI::Create);
    mgr->register_creature_script(CN_MAGTHERIDON, &MagtheridonAI::Create);
}
