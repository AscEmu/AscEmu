/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_BloodFurnace.h"

#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

enum BloodFurnanceEncounter
{
    DATA_THE_MAKER = 0,
    DATA_BROGGOK = 1,
    DATA_KELIDAN_THE_BREAKER = 2
};

class BloodFurnaceInstanceScript : public InstanceScript
{
public:
    explicit BloodFurnaceInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new BloodFurnaceInstanceScript(pMapMgr); }

    void OnEncounterStateChange(uint32_t entry, uint32_t state) override
    {
        switch (entry)
        {
            case DATA_BROGGOK:
            {
                if (state == Performed)
                {
                    if (m_broggokDoorGUID)
                        if (GetGameObjectByGuid(m_broggokDoorGUID))
                            GetGameObjectByGuid(m_broggokDoorGUID)->setState(GO_STATE_OPEN);
                }
            } break;
            case DATA_KELIDAN_THE_BREAKER:
            {
                if (state == Performed)
                {
                    if (m_theMakerDoorGUID)
                        if (GetGameObjectByGuid(m_theMakerDoorGUID))
                            GetGameObjectByGuid(m_theMakerDoorGUID)->setState(GO_STATE_OPEN);
                }
            } break;
            default:
                break;
        }

    }

    void OnGameObjectPushToWorld(GameObject* pGameObject)
    {
        switch (pGameObject->getEntry())
        {
            case GO_BROGGOK:
                m_broggokDoorGUID = pGameObject->getGuidLow();
                break;
            case GO_THE_MAKER:
                m_theMakerDoorGUID = pGameObject->getGuidLow();
                break;
            default:
                break;
        }
    }

    uint32_t m_broggokDoorGUID = 0;
    uint32_t m_theMakerDoorGUID = 0;

};

class KelidanTheBreakerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KelidanTheBreakerAI(c); }
    explicit KelidanTheBreakerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (isHeroic())
        {
            mShadowBoltVolley = addAISpell(KELIDAN_SHADOW_BOLT_VOLLEY_H, 25.0f, TARGET_SELF, 0, 6);
            mFireNova = addAISpell(KELIDAN_FIRE_NOVA_H, 15.0f, TARGET_ATTACKING, 0, 12);
        }
        else
        {
            mShadowBoltVolley = addAISpell(KELIDAN_SHADOW_BOLT_VOLLEY, 25.0f, TARGET_SELF, 0, 6);
            mFireNova = addAISpell(KELIDAN_FIRE_NOVA, 15.0f, TARGET_SELF, 0, 12);
        }

        mBurningNova = addAISpell(KELIDAN_BURNING_NOVA, 0.0f, TARGET_SELF, 0, 0);
        mBurningNova->addEmote("Closer! Come closer... and burn!", CHAT_MSG_MONSTER_YELL);

        mVortex = addAISpell(KELIDAN_FIRE_NOVA, 0.0f, TARGET_SELF, 0, 0);
        addAISpell(KELIDAN_CORRUPTION, 15.0f, TARGET_ATTACKING, 0, 10);

        mBurningNovaTimerId = 0;
        SetAIUpdateFreq(800);

        addEmoteForEvent(Event_OnCombatStart, 4841);    // Who dares interrupt--What is this; what have you done? You'll ruin everything!
        addEmoteForEvent(Event_OnTargetDied, 4845);     // Just as you deserve!
        addEmoteForEvent(Event_OnTargetDied, 4846);     // Your friends will soon be joining you!
        addEmoteForEvent(Event_OnDied, 4848);           // Good...luck. You'll need it.
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mBurningNovaTimerId = _addTimer(15000);
    }

    void AIUpdate() override
    {
        if (getScriptPhase() == 1 && !_isCasting())
        {
            if (_isTimerFinished(mBurningNovaTimerId))
            {
                if (isHeroic())
                    _castAISpell(mVortex);

                _castAISpell(mBurningNova);

                _resetTimer(mBurningNovaTimerId, 30000);
            }
        }
    }

    CreatureAISpells* mShadowBoltVolley;
    CreatureAISpells* mFireNova;
    CreatureAISpells* mBurningNova;
    CreatureAISpells* mVortex;
    uint32_t mBurningNovaTimerId;
};

void SetupBloodFurnace(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HC_BLOOD_FURNANCE, &BloodFurnaceInstanceScript::Create);

    mgr->register_creature_script(CN_KELIDAN_THE_BREAKER, &KelidanTheBreakerAI::Create);
}
