/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Instance_WailingCaverns.h"

class DevouringEctoplasmAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DevouringEctoplasmAI);
    explicit DevouringEctoplasmAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Summon Evolving Ectoplasm
        addAISpell(7952, 10.0f, TARGET_SELF, 0, 600);
    }
};

class DruidFangAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DruidFangAI);
    explicit DruidFangAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Serpent Form
        SerpentForm = addAISpell(8041, 5.0f, TARGET_SELF);
        SerpentForm->setMinMaxPercentHp(0, 50);

        // Healing Touch
        HealingTouch = addAISpell(5187, 5.0f, TARGET_SELF);
        HealingTouch->setMinMaxPercentHp(0, 5);

        // Lightning Bolt
        LightningBolt = addAISpell(9532, 30.0f, TARGET_ATTACKING, 3, 0);

        // Druid's Slumber
        DruidsSlumber = addAISpell(8040, 20.0f, TARGET_RANDOM_SINGLE, 3, 0);
    }

    CreatureAISpells* SerpentForm;
    CreatureAISpells* LightningBolt;
    CreatureAISpells* DruidsSlumber;
    CreatureAISpells* HealingTouch;
};

// BOSSES
class LadyAnacondraAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LadyAnacondraAI);
    explicit LadyAnacondraAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Lightning Bolt
        addAISpell(9532, 30.0f, TARGET_ATTACKING, 3, 0);
        // Sleep
        addAISpell(700, 10.0f, TARGET_RANDOM_SINGLE, 2, 20);

        addEmoteForEvent(Event_OnCombatStart, 8755);     // None can stand against the Serpent Lords
    }
};

class LordCobrahnAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LordCobrahnAI);
    explicit LordCobrahnAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Lightning Bolt
        LightningBolt = addAISpell(9532, 30.0f, TARGET_ATTACKING, 3, 0);
        mEnableLighningBolt = true;
        // Poison -- Spell ID Needs checked
        addAISpell(34969, 15.0f, TARGET_ATTACKING);
        // Cobrahn Serpent Form
        SerpentForm = addAISpell(7965, 0.0f, TARGET_SELF);
        mEnableSerpentForm = true;

        addEmoteForEvent(Event_OnCombatStart, 8756);     // You will never wake the dreamer!
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() <= 20 && mEnableSerpentForm == true)
        {
            _castAISpell(SerpentForm);
            mEnableSerpentForm = false;
            // Disable Lightning Bolt
            mEnableLighningBolt = false;
        }
        else if (_getHealthPercent() <= 20 && mEnableSerpentForm == false && !getCreature()->HasAura(7965))
        {
            // Enable Lightning Bolt
            mEnableLighningBolt = true;
        }
    }
    CreatureAISpells* LightningBolt;
    bool mEnableLighningBolt;
    CreatureAISpells* SerpentForm;
    bool mEnableSerpentForm;
};

class LordPythasAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LordPythasAI);
    explicit LordPythasAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Lightning Bolt
        addAISpell(9532, 30.0f, TARGET_ATTACKING, 3, 0);
        // Sleep
        addAISpell(700, 10.0f, TARGET_RANDOM_SINGLE, 2, 0);
        // Thunderclap
        addAISpell(8147, 20.0f, TARGET_SELF, 0, 5);

        addEmoteForEvent(Event_OnCombatStart, 8757);     // The coils of death... Will crush you!
    }
};

class LordSerpentisAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LordSerpentisAI);
    explicit LordSerpentisAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Lightning Bolt
        addAISpell(9532, 30.0f, TARGET_ATTACKING, 3, 0);
        // Sleep
        addAISpell(700, 10.0f, TARGET_RANDOM_SINGLE, 2, 0);

        addEmoteForEvent(Event_OnCombatStart, 8758);     // I am the serpent king, i can do anything!
    }
};

class VerdanEverlivingAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(VerdanEverlivingAI);
    explicit VerdanEverlivingAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Grasping Vines
        addAISpell(8142, 30.0f, TARGET_ATTACKING, 1, 0);
    }
};

class SkumAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SkumAI);
    explicit SkumAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Chained Bolt
        addAISpell(6254, 50.0f, TARGET_ATTACKING, 2, 0);
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() <= 10 && getAIAgent() != AGENT_FLEE)
        {
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Skum tries to run away in fear");
            setAIAgent(AGENT_FLEE);
            _setMeleeDisabled(false);
            _setRangedDisabled(true);
            _setCastDisabled(true);
            moveTo(-262.829742f, -299.363159f, -68.293579f, true);
        }
    }
};

class MutanusAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MutanusAI);
    explicit MutanusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Thundercrack
        addAISpell(8150, 15.0f, TARGET_SELF);
        // Terrify
        addAISpell(7399, 15.0f, TARGET_RANDOM_SINGLE, 0, 4);
    }
};

// Wailing Caverns Event
// Discipline of Naralex Gossip
static Movement::Location ToNaralex[] =
{
    {  },
    { -132.498077f, 125.888153f, -78.418182f, 0.244260f },
    { -123.892235f, 130.538422f, -78.808937f, 0.519935f },
    { -116.654480f, 142.935806f, -80.233383f, 1.149039f },
    { -111.656868f, 156.927307f, -79.880676f, 1.344603f },
    { -108.829506f, 169.213165f, -79.752487f, 1.344603f },
    { -107.592789f, 183.854782f, -79.735558f, 1.500112f },
    { -106.628258f, 197.477676f, -80.526184f, 1.500112f },
    { -109.725700f, 215.487885f, -85.336075f, 1.650287f },
    { -106.663147f, 225.879135f, -88.962914f, 0.215201f },
    { -90.607216f, 228.829071f, -91.022133f, 6.067203f },
    { -79.377800f, 219.999466f, -93.990906f, 5.482866f },
    { -69.134697f, 209.446045f, -93.404358f, 5.482866f },
    { -53.198994f, 204.919601f, -95.677971f, 6.071915f },
    { -38.501598f, 211.024460f, -96.222626f, 0.559205f },
    { -39.211544f, 197.527161f, -96.574646f, 4.658991f },
    { -40.258022f, 177.948105f, -96.374756f, 4.658991f },
    { -41.385948f, 156.845230f, -94.969429f, 4.658991f },
    { -49.557240f, 145.598206f, -93.284225f, 4.084079f },
    { -52.191185f, 133.269424f, -90.334198f, 4.501911f },
    { -53.070702f, 122.185814f, -89.757874f, 5.128569f },
    { -47.618214f, 115.986847f, -87.939827f, 5.562199f },
    { -36.105568f, 109.539597f, -87.755760f, 5.772686f },
    { -23.849794f, 109.712982f, -89.580704f, 0.014146f },
    { -15.070121f, 119.703346f, -89.904770f, 0.849840f },
    { -6.799855f, 134.471298f, -89.574089f, 1.060297f },
    { 1.530990f, 143.322433f, -89.091454f, 0.589058f },
    { 11.134405f, 149.739365f, -88.872955f, 0.589058f },
    { 21.220901f, 156.479080f, -89.180771f, 0.802686f },
    { 31.682161f, 167.308456f, -88.896530f, 0.802686f },
    { 43.933167f, 179.990555f, -88.922348f, 0.802686f },
    { 51.662514f, 195.831421f, -89.649101f, 1.116846f },
    { 70.554794f, 204.757950f, -92.880386f, 0.441403f },
    { 85.016724f, 211.591156f, -92.615730f, 0.441403f },
    { 99.523796f, 213.738951f, -96.214615f, 0.047919f },
    { 112.235191f, 214.378525f, -98.332832f, 0.679379f },
    { 118.665100f, 220.504974f, -98.305420f, 1.254290f },
    { 121.259758f, 228.493378f, -97.359711f, 1.632852f },
    { 116.031120f, 236.451187f, -96.007195f, 3.089230f }
};

class DofNaralexGossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Unit* Fanglord1 = pObject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-151.139008f, 414.367004f, -72.629402f, CN_LORD_COBRAHN);
        Unit* Fanglord2 = pObject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(36.807400f, -241.063995f, -79.498901f, CN_LORD_PYTHAS);
        Unit* Fanglord3 = pObject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-118.710999f, -24.990999f, -28.498501f, CN_LORD_SERPENTIS);
        Unit* Fanglord4 = pObject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-70.788902f, 120.072998f, -89.673599f, CN_LADY_ANACONDRA);

        if ((!Fanglord1 || !Fanglord1->isAlive()) && (!Fanglord2 || !Fanglord2->isAlive()) && (!Fanglord3 || !Fanglord3->isAlive()) && (!Fanglord4 || !Fanglord4->isAlive()))
        {
            Arcemu::Gossip::Menu menu(pObject->getGuid(), 699, 0);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(442), 1);     // Let's go!
            menu.Send(plr);
        }
        else
        {
            Arcemu::Gossip::Menu menu(pObject->getGuid(), 698, 0);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(443), 1);     // I will slay those Fanglords
            menu.Send(plr);
        }

    }
    void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Creature* pCreature = (pObject->isCreature()) ? static_cast<Creature*>(pObject) : nullptr;
        if (pCreature == nullptr)
        {
            return;
        }

        switch (Id)
        {
            case 1: // Disciple of Naralex Casts Mark of the Wild on players.
            {
                pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Take this! It will be useful for you. I'll be waiting here when you have slain the 4 Fanglords to awake Naralex!");
                pCreature->castSpell(Plr, 5232, true);
                pCreature->Emote(EMOTE_ONESHOT_CHEER);
            } break;
            case 2: // Start Event
            {
                pCreature->setNpcFlags(UNIT_NPC_FLAG_NONE);
                pCreature->GetAIInterface()->StopMovement(0);
                pCreature->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                pCreature->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                pCreature->GetAIInterface()->setWayPointToMove(2);
            } break;
            default:
                break;
        }
        Arcemu::Gossip::Menu::Complete(Plr);
    }
};

class DofNaralexAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DofNaralexAI);
    explicit DofNaralexAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        Mutanus = nullptr;

        for (uint8 i = 1; i < 39; ++i)
        {
            AddWaypoint(CreateWaypoint(i, 0, Movement::WP_MOVE_TYPE_RUN, ToNaralex[i]));
        }

        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);

        // Awakening Spell
        Awakening = addAISpell(6271, 0.0f, TARGET_SELF);
        Awakening->addEmote("Step back and be ready!, I'll try to Awake Naralex", CHAT_MSG_MONSTER_SAY, 0);

        SpawnTimer = 0;
    }

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        ForceWaypointMove(iWaypointId + 1);
        if (isScriptPhase(1) && GetCurrentWaypoint() == 39)
            setScriptPhase(2);
    }

    void OnScriptPhaseChange(uint32_t phaseId) override
    {
        switch (phaseId)
        {
            case 2:
                getCreature()->Emote(EMOTE_ONESHOT_TALK);
                _castAISpell(Awakening);
                SpawnTimer = _addTimer(100000);
                break;
            default:
                break;
        }
    }

    void AIUpdate() override
    {
        if (SpawnTimer && _isTimerFinished(SpawnTimer))
        {
            switch (getScriptPhase())
            {
                case 2:
                    Moccasin();
                    _resetTimer(SpawnTimer, 100000);
                    setScriptPhase(3);
                    break;
                case 3:
                    Ectoplasm();
                    _resetTimer(SpawnTimer, 100000);
                    setScriptPhase(4);
                    break;
                case 4:
                    BMutanus();
                    _resetTimer(SpawnTimer, 100000);
                    setScriptPhase(5);
                    break;
                default:
                    break;
            }
        }
        if (isScriptPhase(5) && (!Mutanus || !Mutanus->isAlive()))
        {
            CreatureAIScript* Naralex = getNearestCreatureAI(3679);
            if (Naralex && Naralex->isAlive())
            {
                _setDisplayId(17089);
                Naralex->_setDisplayId(17089);
                Naralex->sendChatMessage(CHAT_MSG_MONSTER_SAY, 5789, "I am awake... at last");
                Naralex->getCreature()->setStandState(STANDSTATE_STAND);
                setFlyMode(true);
                Naralex->setFlyMode(true);
                moveTo(-6.704030f, 200.308838f, -26.938824f);
                Naralex->moveTo(-6.704030f, 200.308838f, -26.938824f);
            }
            setScriptPhase(6);
        }
    }

    void Moccasin()
    {
        spawnCreature(5762, 134.249207f, 242.194839f, -98.375496f, 3.325373f);
        spawnCreature(5762, 124.917931f, 255.066635f, -97.796837f, 4.176745f);
        spawnCreature(5762, 113.077148f, 258.880157f, -97.190590f, 4.688039f);
    }

    void Ectoplasm()
    {
        spawnCreature(5763, 134.249207f, 242.194839f, -98.375496f, 3.325373f);
        spawnCreature(5763, 124.917931f, 255.066635f, -97.796837f, 4.176745f);
        spawnCreature(5763, 113.077148f, 258.880157f, -97.190590f, 4.688039f);
        spawnCreature(5763, 138.794693f, 228.224976f, -100.174332f, 2.471645f);
        spawnCreature(5763, 128.170364f, 225.190247f, -99.392830f, 2.411169f);
        spawnCreature(5763, 136.762009f, 242.685669f, -98.564545f, 3.344223f);
        spawnCreature(5763, 122.403961f, 259.438354f, -98.153984f, 4.366811f);
    }

    void BMutanus()
    {
        Mutanus = spawnCreature(CN_MUTANUS, 136.337006f, 263.769989f, -102.666000f, 4.002330f);
    }

    int32 SpawnTimer;
    CreatureAISpells* Awakening;
    Creature* Mutanus;
};


class DeviateMoccasinAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DeviateMoccasinAI);
    explicit DeviateMoccasinAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }
};

class EctoplasmAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EctoplasmAI);
    explicit EctoplasmAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }
};

class Naralex : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Naralex);
    explicit Naralex(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->setStandState(STANDSTATE_SLEEP);
    }
};

void SetupWailingCaverns(ScriptMgr* mgr)
{
    // Creatures
    mgr->register_creature_script(CN_DRUID_FANG, &DruidFangAI::Create);
    mgr->register_creature_script(CN_DEVOURING_ECTOPLASM, &DevouringEctoplasmAI::Create);
    mgr->register_creature_script(CN_LADY_ANACONDRA, &LadyAnacondraAI::Create);
    mgr->register_creature_script(CN_LORD_COBRAHN, &LordCobrahnAI::Create);
    mgr->register_creature_script(CN_LORD_PYTHAS, &LordPythasAI::Create);
    mgr->register_creature_script(CN_LORD_SERPENTIS, &LordSerpentisAI::Create);
    mgr->register_creature_script(CN_VERDAN_EVERLIVING, &VerdanEverlivingAI::Create);
    mgr->register_creature_script(CN_SKUM, &SkumAI::Create);
    mgr->register_creature_script(CN_MUTANUS, &MutanusAI::Create);

    Arcemu::Gossip::Script* DNaralex = new DofNaralexGossip();
    mgr->register_creature_gossip(CN_DIS_NARALEX, DNaralex);

    mgr->register_creature_script(CN_DIS_NARALEX, &DofNaralexAI::Create);
    mgr->register_creature_script(CN_NARALEX, &Naralex::Create);

    // Might be easier to merge this 2 into 1 since they are moving to the same locations..
    mgr->register_creature_script(5762, &DeviateMoccasinAI::Create);
    mgr->register_creature_script(5763, &EctoplasmAI::Create);
}
