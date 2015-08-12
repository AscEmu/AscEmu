/*
  * AscEmu Framework based on ArcEmu MMORPG Server
  * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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


#include "Setup.h"
#include "Raid_IceCrownCitadel.h"

//////////////////////////////////////////////////////////////////////////////////////////
//ICC zone: 4812
//Prepared creature entry:
//
//#define CN_DEATHBRINGER_SAURFANG    37813
//#define CN_FESTERGUT                36626
//#define CN_ROTFACE                  36627
//#define CN_PROFESSOR_PUTRICIDE      36678
//#define CN_PRINCE_VALANAR           37970
//#define CN_BLOOD_QUEEN_LANATHEL     37955
//#define CN_SINDRAGOSA               36853
//#define CN_THE_LICHKING             36597
//
///\todo  start boss scripts
//////////////////////////////////////////////////////////////////////////////////////////
//Event: GunshipBattle
//
//Affects:
//Available teleports. If GunshipBattle done -> Teleportlocation 4 available.
//
//Devnotes:
//Far away from implementing this :(
//////////////////////////////////////////////////////////////////////////////////////////
enum IceCrown_Encounters
{
    DATA_LORD_MARROWGAR,
    DATA_COLDFLAME,
    DATA_BONE_SPIKE,
    DATA_LADY_DEATHWHISPER,
    DATA_VALITHRIA_DREAM,

    ICC_DATA_END
};


//////////////////////////////////////////////////////////////////////////////////////////
//IceCrownCitadel Instance
class IceCrownCitadelScript : public MoonInstanceScript
{
    friend class ICCTeleporterAI; // Friendship forever ;-)

    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(IceCrownCitadelScript, MoonInstanceScript);
        IceCrownCitadelScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            // Way to select bosses
            BuildEncounterMap();
            if (mEncounters.size() == 0)
                return;

            for (EncounterMap::iterator Iter = mEncounters.begin(); Iter != mEncounters.end(); ++Iter)
            {
                if ((*Iter).second.mState != State_Finished)
                    continue;

                switch ((*Iter).first)
                {
                    case CN_LORD_MARROWGAR:
                        AddGameObjectStateByEntry(GO_MARROWGAR_ICEWALL_1, State_Inactive);    // Icewall 1
                        AddGameObjectStateByEntry(GO_MARROWGAR_ICEWALL_2, State_Inactive);    // Icewall 2
                        AddGameObjectStateByEntry(GO_MARROWGAR_DOOR, State_Inactive);         // Door
                        break;
                    default:
                        continue;
                };
            };
        }

        void OnGameObjectPushToWorld(GameObject* pGameObject)
        {
            // Gos which are not visible by killing a boss needs a second check...
            if (GetInstanceData(Data_EncounterState, CN_LORD_MARROWGAR) == State_Finished)
            {
                AddGameObjectStateByEntry(GO_MARROWGAR_ICEWALL_1, State_Active);    // Icewall 1
                AddGameObjectStateByEntry(GO_MARROWGAR_ICEWALL_2, State_Active);    // Icewall 2
                AddGameObjectStateByEntry(GO_MARROWGAR_DOOR, State_Active);         // Door
            }
        }

        void SetInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = (EncounterState)pData;
        };

        uint32 GetInstanceData(uint32 pType, uint32 pIndex)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return 0;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return 0;

            return (*Iter).second.mState;
        };

        void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
        {
            EncounterMap::iterator Iter = mEncounters.find(pCreature->GetEntry());
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = State_Finished;

            switch (pCreature->GetEntry())
            {
                case CN_LORD_MARROWGAR:
                {
                    AddGameObjectStateByEntry(GO_MARROWGAR_ICEWALL_1, State_Active);    // Icewall 1
                    AddGameObjectStateByEntry(GO_MARROWGAR_ICEWALL_2, State_Active);    // Icewall 2
                    AddGameObjectStateByEntry(GO_MARROWGAR_DOOR, State_Active);         // Door
                }break;
                default:
                    break;
            }
            return;
        }

        void OnPlayerEnter(Player* player)
        {
            // setup only the npcs with the correct team...
            switch (player->GetTeam())
            {
                case TEAM_ALLIANCE:
                    for (uint32 i = 0; i < 13; i++)
                        PushCreature(AllySpawns[i].entry, AllySpawns[i].x, AllySpawns[i].y, AllySpawns[i].z, AllySpawns[i].o, AllySpawns[i].faction);
                    break;
                case TEAM_HORDE:
                    for (uint32 i = 0; i < 13; i++)
                        PushCreature(HordeSpawns[i].entry, HordeSpawns[i].x, HordeSpawns[i].y, HordeSpawns[i].z, HordeSpawns[i].o, HordeSpawns[i].faction);
                    break;
            }
        }

};

//////////////////////////////////////////////////////////////////////////////////////////
// IceCrown Teleporter
class ICCTeleporterAI : public GameObjectAIScript
{
public:

    ICCTeleporterAI(GameObject* go) : GameObjectAIScript(go){}

    ~ICCTeleporterAI(){}

    static GameObjectAIScript* Create(GameObject* go) { return new ICCTeleporterAI(go); }

    void OnActivate(Player* player)
    {
        IceCrownCitadelScript* pInstance = (IceCrownCitadelScript*)player->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        GossipMenu* menu = NULL;
        objmgr.CreateGossipMenuForPlayer(&menu, _gameobject->GetGUID(), 15221, player);
        menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(515), 0);     // Teleport to Light's Hammer.

        if (pInstance->GetInstanceData(Data_EncounterState, CN_LORD_MARROWGAR) == State_Finished)
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(516), 1);      // Teleport to Oratory of The Damned.

        if (pInstance->GetInstanceData(Data_EncounterState, CN_LADY_DEATHWHISPER) == State_Finished)
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(517), 2);      // Teleport to Rampart of Skulls.

        // GunshipBattle has to be finished...
        //menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(518), 3);        // Teleport to Deathbringer's Rise.

        if (pInstance->GetInstanceData(Data_EncounterState, CN_VALITHRIA_DREAMWALKER) == State_Finished)
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(519), 4);      // Teleport to the Upper Spire.

        if (pInstance->GetInstanceData(Data_EncounterState, CN_COLDFLAME) == State_Finished)
            menu->AddItem(ICON_CHAT, player->GetSession()->LocalizedGossipOption(520), 5);      // Teleport to Sindragosa's Lair.

        menu->SendTo(player);
    }

};

class ICCTeleporterGossip : public GossipScript
{
public:
    ICCTeleporterGossip() : GossipScript(){}

    void OnSelectOption(Object* object, Player* player, uint32 Id, const char* enteredcode)
    {
        Arcemu::Gossip::Menu::Complete(player);

        if (Id >= 6)
            return;

        switch (Id)
        {
            case 0:
                player->CastSpell(player, 70781, true);     // Light's Hammer
                break;
            case 1:
                player->CastSpell(player, 70856, true);     // Oratory of The Damned
                break;
            case 2:
                player->CastSpell(player, 70857, true);     // Rampart of Skulls
                break;
            case 3:
                player->CastSpell(player, 70858, true);     // Deathbringer's Rise
                break;
            case 4:
                player->CastSpell(player, 70859, true);     // Upper Spire
                break;
            case 5:
                player->CastSpell(player, 70861, true);     // Sindragosa's Lair
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Boss: Lord Marrowgar
#define LM_BERSERK 47008
#define BONE_SLICE 69055
#define BONE_SPIKE 69057        // Not shure about this
#define BONE_STORM 69076
#define SOUL_FEAST 71203        // Needs a script

class LordMarrowgarAI : public MoonScriptBossAI
{
    public:

        bool m_spellcheck[4];
        SP_AI_Spell spells[4];

        MOONSCRIPT_FACTORY_FUNCTION(LordMarrowgarAI, MoonScriptBossAI);
        LordMarrowgarAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            _unit->SendScriptTextChatMessage(922);      // This is the beginning AND the end, mortals. None may enter the master's sanctum!

            nrspells = 4;

            spells[0].info = dbcSpell.LookupEntry(BONE_SLICE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 10000;

            spells[1].info = dbcSpell.LookupEntry(BONE_STORM);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 60000;
            spells[1].perctrigger = 75.0f;
            spells[1].attackstoptimer = 60000;

            spells[2].info = dbcSpell.LookupEntry(LM_BERSERK);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = 15;
            spells[2].perctrigger = 50.0f;
            spells[2].attackstoptimer = 10000;

            spells[3].info = dbcSpell.LookupEntry(SOUL_FEAST);
            spells[3].targettype = TARGET_RANDOM_SINGLE;
            spells[3].instant = true;
            spells[3].cooldown = 20;
            spells[3].perctrigger = 50.0f;
            spells[3].attackstoptimer = 12000;
        }

        void AIUpdate()
        {
            switch (RandomUInt(1))
            {
                case 0:
                {
                    float val = RandomFloat(100.0f);
                    SpellCast(val);
                }break;
                case 1:
                    BoneSpike();
                    break;
            }
        }

        void OnCombatStart(Unit* pTarget)
        {
            _unit->SendScriptTextChatMessage(923);      // The Scourge will wash over this world as a swarm of death and destruction!
            RegisterAIUpdateEvent(60000);
        }

        void BoneSpike()
        {
            switch (RandomUInt(2))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(925);      // Bound by bone!
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(926);      // Stick around!
                    break;
                case 2:
                    _unit->SendScriptTextChatMessage(927);      // The only escape is death!
                    break;
            }

            std::vector<Player*> TargetTable;
            set<Object*>::iterator itr = _unit->GetInRangePlayerSetBegin();

            for (; itr != _unit->GetInRangePlayerSetEnd(); ++itr)
            {
                if (isHostile(_unit, (*itr)))
                {
                    Player* RandomTarget = NULL;
                    RandomTarget = TO<Player*>(*itr);
                    if (RandomTarget && RandomTarget->isAlive() && isHostile(_unit, RandomTarget))
                        TargetTable.push_back(RandomTarget);
                }
            }

            if (!TargetTable.size())
                return;

            size_t RandTarget = rand() % TargetTable.size();
            Unit* RTarget = TargetTable[RandTarget];

            if (!RTarget)
                return;

            _unit->CastSpell(RTarget, dbcSpell.LookupEntry(BONE_SPIKE), false);

            TargetTable.clear();

            float dcX = RTarget->GetPositionX();
            float dcY = RTarget->GetPositionY();
            float dcZ = RTarget->GetPositionZ();

            _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_BONE_SPIKE, dcX, dcY, dcZ, 0, true, false, 0, 0);

            TargetTable.clear();
        }

        void OnTargetDied(Unit* pTarget)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(928);      // More bones for the offering!
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(929);      // Languish in damnation!
                    break;
            }
        }

        void OnDied(Unit* pTarget)
        {
            _unit->SendScriptTextChatMessage(930);      // I see... Only darkness.
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (int i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger)
                        continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                _unit->CastSpell(_unit, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_ATTACKING:
                                _unit->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }

                RemoveAIUpdateEvent();
                RegisterAIUpdateEvent(50000);
            }
        }

    protected:

        int nrspells;
};

#define IMPALED 69065

class BoneSpikeAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(BoneSpikeAI, MoonScriptBossAI);
        BoneSpikeAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            _unit->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);  // On wowhead they said "kill them not just looking at them".
            _unit->Despawn(8000, 0);
        }

};

//////////////////////////////////////////////////////////////////////////////////////////
// Boss: Lady Deathwhisper
// MANA_BARRIER = 70842
// DEATH_AND_DECAY = 71001
// TOUCH_OF_INSIGNIFICANCE = 71204
// SHADOW_BOLT = 71254
// DOMINATE_MIND_H = 71289
// SUMMON_SHADE = 71363
// FROSTBOLT = 71420
// FROSTBOLT_VOLLEY = 72905
// ...

///////////////////////////////////////////////////////
// Boss: Valithria Dreamwalker
// ...
//
//
//
//

///////////////////////////////////////////////////////
// Boss: Cold Flame
// ...
//
//
//
//

void SetupICC(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_ICECROWNCITADEL, &IceCrownCitadelScript::Create);

    //Teleporters
    mgr->register_gameobject_script(GO_TELE_1, &ICCTeleporterAI::Create);
    mgr->register_go_gossip_script(GO_TELE_1, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_2, &ICCTeleporterAI::Create);
    mgr->register_go_gossip_script(GO_TELE_2, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_3, &ICCTeleporterAI::Create);
    mgr->register_go_gossip_script(GO_TELE_3, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_4, &ICCTeleporterAI::Create);
    mgr->register_go_gossip_script(GO_TELE_4, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_5, &ICCTeleporterAI::Create);
    mgr->register_go_gossip_script(GO_TELE_5, new ICCTeleporterGossip());

    //Bosses
    mgr->register_creature_script(CN_LORD_MARROWGAR, &LordMarrowgarAI::Create);
    //mgr->register_creature_script(CN_LADY_DEATHWHISPER, &LadyDeathwhisperAI::Create);
    //mgr->register_creature_script(CN_VALITHRIA_DREAMWALKER, &ValithriaDreamwalkerAI::Create);
    //mgr->register_creature_script(CN_COLDFLAME, &ColdFlameAI::Create);

    //Misc
    mgr->register_creature_script(CN_BONE_SPIKE, &BoneSpikeAI::Create);
}
