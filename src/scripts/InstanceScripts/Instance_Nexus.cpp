/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
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

// \todo Last boss needs to be finished
#include "Setup.h"
#include "Instance_Nexus.h"

#define GO_FLAG_UNCLICKABLE 0x00000010

//////////////////////////////////////////////////////////////////////////////////////////
///Anomalus
class AnomalusAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AnomalusAI);
        AnomalusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            mInstance = getInstanceScript();

            if (_isHeroic())
                AddSpell(SPARK_HC, Target_RandomPlayer, 80, 0, 3);
            else
                AddSpell(SPARK, Target_RandomPlayer, 80, 0, 3);

            mSummon = 0;
            mRift = false;
            mSummonTimer = 0;

            // new
            addEmoteForEvent(Event_OnCombatStart, 4317);     // Chaos beckons.
            addEmoteForEvent(Event_OnDied, 4318);     // Of course.
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            mSummon = 0;
            mRift = false;
            mSummonTimer = _addTimer(_isHeroic() ? 14000 : 18000);   // check heroic
        }

        void AIUpdate() override
        {
            if ((_getHealthPercent() <= 50 && mSummon == 0))
                mSummon += 1;

            if (mSummon == 1)
                ChargeRift();

            if (_isTimerFinished(mSummonTimer) && mRift == false)
            {
                SummonRift(false);
                _resetTimer(mSummonTimer, _isHeroic() ? 14000 : 18000);
            }

            if (mRift == true && (GetLinkedCreature() == NULL || !GetLinkedCreature()->isAlive()))
            {
                _removeAura(47748);
                mRift = false;
            }
        }

        void SummonRift(bool bToCharge)
        {
            if (!bToCharge)
                sendDBChatMessage(4319);     // Reality... unwoven.

            sendAnnouncement("Anomalus opens a Chaotic Rift!");
            //we are linked with CN_CHAOTIC_RIFT.
            CreatureAIScript* chaoticRift = spawnCreatureAndGetAIScript(CN_CHAOTIC_RIFT, getCreature()->GetPositionX() + 13.5f, getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
            if (chaoticRift != nullptr)
            {
                SetLinkedCreature(chaoticRift);
                chaoticRift->SetLinkedCreature(this);
            }
        }

        void ChargeRift()
        {
            SummonRift(true);
            sendDBChatMessage(4320);     // Indestructible.
            sendAnnouncement("Anomalus shields himself and diverts his power to the rifts!");
            _applyAura(47748);   // me immune
            setRooted(true);

            mRift = true;
            mSummon += 1;
        }

        void OnDied(Unit* /*pKiller*/) override
        {
            if (mInstance)
            {
                GameObjectSet sphereSet = mInstance->getGameObjectsSetForEntry(ANOMALUS_CS);
                for (auto goSphere : sphereSet)
                {
                    if (goSphere != nullptr)
                        goSphere->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);
                }
            }
        }

    private:

        int32 mSummonTimer;
        uint8 mSummon;
        bool mRift;
        InstanceScript* mInstance;
};

class ChaoticRiftAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ChaoticRiftAI);
        ChaoticRiftAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            auto spell_mana_wrath = sSpellCustomizations.GetSpellInfo(SUMMON_MANA_WRAITH);
            if (spell_mana_wrath != nullptr)
                AddSpell(SUMMON_MANA_WRAITH, Target_Self, 30, 0, spell_mana_wrath->getRecoveryTime());

            auto spell_energy_burst = sSpellCustomizations.GetSpellInfo(CHAOTIC_ENERGY_BURST);
            if (spell_energy_burst != nullptr)
                AddSpell(CHAOTIC_ENERGY_BURST, Target_RandomPlayer, 30, 0, spell_energy_burst->getRecoveryTime());
        }

        void OnLoad() override
        {
            _applyAura(CHAOTIC_RIFT_AURA);
            despawn(40000, 0);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            despawn(2000, 0);
        }

        void OnCombatStop(Unit* /*pTarget*/) override
        {
            despawn(2000, 0);
        }
};

class CraziedManaWrathAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(CraziedManaWrathAI);
        CraziedManaWrathAI(Creature* pCreature) : CreatureAIScript(pCreature) {};

        void OnCombatStop(Unit* /*pTarget*/) override
        {
            despawn(2000, 0);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            despawn(2000, 0);
        }
};


//////////////////////////////////////////////////////////////////////////////////////////
/// Grand Magus Telestra
static Movement::Location FormSpawns[] =
{
    { 494.726990f, 89.128799f, -15.941300f, 6.021390f },
    { 495.006012f, 89.328102f, -16.124609f, 0.027486f },
    { 504.798431f, 102.248375f, -16.124609f, 4.629921f }
};


class TelestraBossAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TelestraBossAI);
        TelestraBossAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            mInstance = getInstanceScript();

            if (_isHeroic())
            {
                AddSpell(ICE_NOVA_HC, Target_Self, 25, 2.0, 15);
                AddSpell(FIREBOMB_HC, Target_RandomPlayer, 35, 1.5, 5);
                AddSpell(GRAVITY_WELL, Target_Self, 15, 0.5, 20);
            }
            else
            {
                AddSpell(ICE_NOVA, Target_Self, 25, 2.0, 15);
                AddSpell(FIREBOMB, Target_RandomPlayer, 35, 1.5, 5);
                AddSpell(GRAVITY_WELL, Target_Self, 15, 0.5, 20);
            }

            SetAIUpdateFreq(1000);

            mAddCount = 0;
            mPhaseRepeat = 2;

            mAddArray[0] = mAddArray[1] = mAddArray[2] = NULL;

            // new
            addEmoteForEvent(Event_OnCombatStart, 4326);      // You know what they say about curiosity....
            addEmoteForEvent(Event_OnTargetDied, 4327);      // Death becomes you.
            addEmoteForEvent(Event_OnDied, 4328);      // Damn the... luck.
        }

        void AIUpdate() override
        {
            if (isScriptPhase(1) && _getHealthPercent() <= (mPhaseRepeat * 25))
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        sendDBChatMessage(4330);      // There's plenty of me to go around.
                        break;
                    case 1:
                        sendDBChatMessage(4331);      // I'll give you more than you can handle!
                        break;
                }

                setScriptPhase(2);
                setRooted(true);
                _setRangedDisabled(true);
                _setCastDisabled(true);
                _setTargetingDisabled(true);
                _applyAura(60191);

                for (uint8 i = 0; i < 3; ++i)
                {
                    mAddArray[i] = spawnCreature(CN_TELESTRA_FIRE + i, FormSpawns[i].x, FormSpawns[i].y, FormSpawns[i].z, FormSpawns[i].o);
                    if (mAddArray[i] != NULL)
                        ++mAddCount;
                }
            }

            if (isScriptPhase(2))
            {
                for (uint8 i = 0; i < 3; ++i)
                {
                    if (mAddArray[i] != NULL)
                    {
                        mAddArray[i]->Despawn(1000, 0);
                        mAddArray[i] = NULL;
                        --mAddCount;
                    }
                }

                if (mAddCount != 0)
                    return;

                sendChatMessage(CHAT_MSG_MONSTER_YELL, 13323, "Now to finish the job!");

                _removeAura(60191);
                setRooted(false);
                mPhaseRepeat = 1;
                setScriptPhase(_isHeroic() ? 1 : 3);   //3 disables p2
            }
        }

        void OnCombatStop(Unit* /*pTarget*/) override
        {
            for (uint8 i = 0; i < 3; ++i)
            {
                if (mAddArray[i] != NULL)
                {
                    mAddArray[i]->Despawn(1000, 0);
                    mAddArray[i] = NULL;
                }
            }
        }

        void OnDied(Unit* /*pKiller*/) override
        {
            for (uint8 i = 0; i < 3; ++i)
            {
                if (mAddArray[i] != NULL)
                {
                    mAddArray[i]->Despawn(1000, 0);
                    mAddArray[i] = NULL;
                }
            }

            if (mInstance)
            {
                mInstance->setData(NEXUS_TELESTRA, Finished);
                GameObjectSet sphereSet = mInstance->getGameObjectsSetForEntry(TELESTRA_CS);
                for (auto goSphere : sphereSet)
                {
                    if (goSphere != nullptr)
                        goSphere->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);
                }
            }
        }

    private:

        Creature* mAddArray[3];

        int32 mPhaseRepeat;
        int32 mAddCount;

        InstanceScript* mInstance;
};

class TelestraFireAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TelestraFireAI);
        TelestraFireAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            if (_isHeroic())
            {
                AddSpell(FIRE_BLAST_HC, Target_RandomPlayer, 30, 0, 14);
                AddSpell(SCORCH_HC, Target_Current, 100, 1, 3);
            }
            else
            {
                AddSpell(FIRE_BLAST, Target_RandomPlayer, 30, 0, 14);
                AddSpell(SCORCH, Target_Current, 100, 1, 3);
            }
        }

        void OnLoad() override
        {
            AggroNearestUnit();
        }
};

class TelestraFrostAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TelestraFrostAI);
        TelestraFrostAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            if (_isHeroic())
            {
                AddSpell(BLIZZARD_HC, Target_RandomPlayerDestination, 20, 0, 20);
                AddSpell(ICE_BARB_HC, Target_RandomPlayer, 25, 0.5, 6);
            }
            else
            {
                AddSpell(BLIZZARD, Target_RandomPlayerDestination, 20, 0, 20);
                AddSpell(ICE_BARB, Target_RandomPlayer, 25, 0.5, 6);
            }
        }

        void OnLoad() override
        {
            AggroNearestUnit();
        }
};

class TelestraArcaneAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TelestraArcaneAI);
        TelestraArcaneAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(TIME_STOP, Target_Self, 30, 1.5, 30);
            AddSpell(CRITTER, Target_RandomPlayer, 25, 0, 20);
        }

        void OnLoad() override
        {
            AggroNearestUnit();
        }
};


//////////////////////////////////////////////////////////////////////////////////////////
/// Ormorok the Tree-Shaper
class OrmorokAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(OrmorokAI);
    OrmorokAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mInstance = getInstanceScript();

        if (_isHeroic())
            AddSpell(TRAMPLE_H, Target_Current, 30, 0, 9);
        else
            AddSpell(TRAMPLE, Target_Current, 30, 0, 9);

        AddSpell(SPELL_REFLECTION, Target_Self, 35, 2.0f, 15);
        mCrystalSpikes = AddSpell(CRYSTAL_SPIKES, Target_Self, 25, 0, 12);
        mCrystalSpikes->addEmote("Bleed!", CHAT_MSG_MONSTER_YELL, 13332);

        mEnraged = false;

        // new
        addEmoteForEvent(Event_OnCombatStart, 1943);     // Noo!
        addEmoteForEvent(Event_OnDied, 1944);     // Aaggh!
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnraged = false;
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() <= 25 && mEnraged == false)
        {
            _applyAura(FRENZY);
            sendAnnouncement("Ormorok the Tree-Shaper goes into a frenzy!");
            mEnraged = true;
        }
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (mInstance)
        {
            GameObjectSet sphereSet = mInstance->getGameObjectsSetForEntry(ORMOROK_CS);
            for (auto goSphere : sphereSet)
            {
                if (goSphere != nullptr)
                    goSphere->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);
            }
        }
    }

    private:

        SpellDesc* mCrystalSpikes;
        bool mEnraged;

        InstanceScript* mInstance;
};

class CrystalSpikeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CrystalSpikeAI);
    CrystalSpikeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_part = 0;
    }

    void OnLoad() override
    {
        setCanEnterCombat(false);
        setRooted(true);
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        despawn(4500, 0);
        RegisterAIUpdateEvent(500);
    }

    void AIUpdate() override
    {
        m_part += 1;

        if (m_part == 1)
        {
            getCreature()->CastSpell(getCreature(), SPELL_CRYSTAL_SPIKE_VISUAL, true);
        }
        else if (m_part == 5)
        {
            if (_isHeroic())
            {
                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(SPELL_CRYSTAL_SPIKE_H), true);
            }
            else
            {
                getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(SPELL_CRYSTAL_SPIKE), true);
            }
        }
    }

    private:

        int m_part;
};


//////////////////////////////////////////////////////////////////////////////////////////
/// Keristrasza
/// \todo currently unfinished
class KeristraszaAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KeristraszaAI);
    KeristraszaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (_isHeroic())
            AddSpell(CRYSTALFIRE_BREATH_HC, Target_Self, 30, 1, 14);
        else
            AddSpell(CRYSTALFIRE_BREATH, Target_Self, 30, 1, 14);

        AddSpell(CRYSTAL_CHAINS, Target_RandomPlayer, 30, 0, 12);
        AddSpell(TAIL_SWEEP, Target_Self, 40, 0, 8);

        mCrystalize = AddSpell(CRYSTALLIZE, Target_Self, 25, 0, 22);
        mCrystalize->addEmote("Stay. Enjoy your final moments.", CHAT_MSG_MONSTER_YELL, 13451);

        mEnraged = false;
        setCanEnterCombat(false);

        // new
        addEmoteForEvent(Event_OnCombatStart, 4321);     // Preserve? Why? There's no truth in it. No no no... only in the taking! I see that now!
        addEmoteForEvent(Event_OnTargetDied, 4322);     // Now we've come to the truth! )
        addEmoteForEvent(Event_OnDied, 4324);     // Dragonqueen... Life-Binder... preserve... me.
    }

    void OnLoad() override
    {
        _applyAura(47543);   // frozen prison
    }

    void AIUpdate() override
    {
        if (mEnraged == false && _getHealthPercent() <= 25)
        {
            _applyAura(ENRAGE);
            mEnraged = true;
        }
    }

    void Release()
    {
        setCanEnterCombat(true);
        _removeAura(47543);
        _applyAura(INTENSE_COLD);
    }

    private:

        bool mEnraged;
        SpellDesc* mCrystalize;
};

// Nexus Instance script
class NexusScript : public InstanceScript
{
    public:

        uint32 mAnomalusGUID;
        uint32 mTelestraGUID;
        uint32 mOrmorokGUID;
        uint32 mKeristraszaGUID;

        uint8 mCSCount;

        uint32 m_uiEncounters[NEXUS_END];

        NexusScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
        {
            mAnomalusGUID = 0;
            mTelestraGUID = 0;
            mOrmorokGUID = 0;
            mKeristraszaGUID = 0;

            mCSCount = 0;

            for (uint8 i = 0; i < NEXUS_END; ++i)
                m_uiEncounters[i] = NotStarted;

            PrepareGameObjectsForState();
        }

        static InstanceScript* Create(MapMgr* pMapMgr) { return new NexusScript(pMapMgr); }

        void PrepareGameObjectsForState()
        {
            if (getData(NEXUS_ANOMALUS) == Finished)
            {
                GameObjectSet sphereSet = getGameObjectsSetForEntry(ANOMALUS_CS);
                for (auto goSphere : sphereSet)
                {
                    if (goSphere != nullptr)
                        goSphere->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);
                }
            }

            if (getData(NEXUS_TELESTRA) == Finished)
            {
                GameObjectSet sphereSet = getGameObjectsSetForEntry(TELESTRA_CS);
                for (auto goSphere : sphereSet)
                {
                    if (goSphere != nullptr)
                        goSphere->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);
                }
            }

            if (getData(NEXUS_ORMOROK) == Finished)
            {
                GameObjectSet sphereSet = getGameObjectsSetForEntry(ORMOROK_CS);
                for (auto goSphere : sphereSet)
                {
                    if (goSphere != nullptr)
                        goSphere->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNCLICKABLE);
                }
            }
        }

        void OnCreaturePushToWorld(Creature* pCreature) override
        {
            switch (pCreature->GetEntry())
            {
                case CN_KERISTRASZA:
                    mKeristraszaGUID = pCreature->GetLowGUID();
                    break;
                case CN_ANOMALUS:
                    mAnomalusGUID = pCreature->GetLowGUID();
                    break;
                case CN_TELESTRA:
                    mTelestraGUID = pCreature->GetLowGUID();
                    break;
                case CN_ORMOROK:
                    mOrmorokGUID = pCreature->GetLowGUID();
                    break;
            }
        }

        void OnGameObjectPushToWorld(GameObject* pGameObject) override
        {
            switch (pGameObject->GetEntry())
            {
                case ANOMALUS_CS:
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    break;
                case TELESTRA_CS:
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    break;
                case ORMOROK_CS:
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    break;
            }
        }

        void OnGameObjectActivate(GameObject* pGameObject, Player* /*pPlayer*/) override
        {
            switch (pGameObject->GetEntry())
            {
                case ANOMALUS_CS:
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    ++mCSCount;
                    break;
                case TELESTRA_CS:
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    ++mCSCount;
                    break;
                case ORMOROK_CS:
                    pGameObject->SetFlags(GO_FLAG_UNCLICKABLE);
                    ++mCSCount;
                    break;
                default:
                    return;
            }

            if (mCSCount == 3)   // release last boss
            {
                Creature* pKeristrasza = GetCreatureByGuid(mKeristraszaGUID);
                if (pKeristrasza == NULL)
                    return;

                KeristraszaAI* pKeristraszaAI = static_cast< KeristraszaAI* >(pKeristrasza->GetScript());
                if (pKeristraszaAI == NULL)
                    return;

                pKeristraszaAI->Release();
            }
        }

        void OnPlayerEnter(Player* player) override
        {
            if (!spawnsCreated())
            {
                // team spawns
                if (player->GetTeam() == TEAM_ALLIANCE)
                {
                    for (uint8 i = 0; i < 18; i++)
                        spawnCreature(TrashHordeSpawns[i].entry, TrashHordeSpawns[i].x, TrashHordeSpawns[i].y, TrashHordeSpawns[i].z, TrashHordeSpawns[i].o, TrashHordeSpawns[i].faction);
                }
                else
                {
                    for (uint8 i = 0; i < 18; i++)
                        spawnCreature(TrashAllySpawns[i].entry, TrashAllySpawns[i].x, TrashAllySpawns[i].y, TrashAllySpawns[i].z, TrashAllySpawns[i].o, TrashAllySpawns[i].faction);
                }

                // difficulty spawns
                if (player->GetDungeonDifficulty() == MODE_NORMAL)
                {
                    switch (player->GetTeam())
                    {
                        case TEAM_ALLIANCE:
                            spawnCreature(CN_HORDE_COMMANDER, 425.39f, 185.82f, -35.01f, -1.57f, 14);
                            break;
                        case TEAM_HORDE:
                            spawnCreature(CN_ALLIANCE_COMMANDER, 425.39f, 185.82f, -35.01f, -1.57f, 14);
                            break;
                    }
                }
                else    // MODE_HEROIC
                {
                    switch (player->GetTeam())
                    {
                        case TEAM_ALLIANCE:
                            spawnCreature(H_CN_HORDE_COMMANDER, 425.39f, 185.82f, -35.01f, -1.57f, 14);
                            break;
                        case TEAM_HORDE:
                            spawnCreature(H_CN_ALLIANCE_COMMANDER, 425.39f, 185.82f, -35.01f, -1.57f, 14);
                            break;
                    }
                }

                setSpawnsCreated();
            }
        }
};

void SetupNexus(ScriptMgr* mgr)
{
    // Anomalus Encounter
    mgr->register_creature_script(CN_ANOMALUS, &AnomalusAI::Create);
    mgr->register_creature_script(CN_CHAOTIC_RIFT, &ChaoticRiftAI::Create);
    mgr->register_creature_script(CN_CRAZED_MANA_WRAITH, &CraziedManaWrathAI::Create);

    // Grand Magus Telestra Encounter
    mgr->register_creature_script(CN_TELESTRA, &TelestraBossAI::Create);
    mgr->register_creature_script(CN_TELESTRA_ARCANE, &TelestraArcaneAI::Create);
    mgr->register_creature_script(CN_TELESTRA_FROST, &TelestraFrostAI::Create);
    mgr->register_creature_script(CN_TELESTRA_FIRE, &TelestraFireAI::Create);

    // Ormorok the Tree-Shaper Encounter
    mgr->register_creature_script(CN_ORMOROK, &OrmorokAI::Create);
    mgr->register_creature_script(CN_CRYSTAL_SPIKE, &CrystalSpikeAI::Create);


    // Keristrasza Encounter
    mgr->register_creature_script(CN_KERISTRASZA, &KeristraszaAI::Create);

#ifndef UseNewMapScriptsProject
    mgr->register_instance_script(MAP_NEXUS, &NexusScript::Create);
#endif
}
