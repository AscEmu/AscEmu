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

// \ todo Finish Kritkhir Encounter, needs more blizzlike, may need InstanceScript | Anuburak | Add's AI and trash

#include "Setup.h"
#include "Instance_AzjolNerub.h"

class AzjolNerubInstanceScript : public InstanceScript
{
public:

    explicit AzjolNerubInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new AzjolNerubInstanceScript(pMapMgr); }


};

// Krikthir The Gatewatcher
class KrikthirAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KrikthirAI)
    explicit KrikthirAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (!_isHeroic())
        {
            addAISpell(KRIKTHIR_CURSEOFFATIGUE, 100.0f, TARGET_SELF, 0, 10);

            auto curseOfFattigue = addAISpell(KRIKTHIR_MINDFLAY, 100.0f, TARGET_RANDOM_SINGLE, 0, 7);
            curseOfFattigue->setMinMaxDistance(0.0f, 30.0f);
        }
        else
        {
            addAISpell(KRIKTHIR_CURSEOFFATIGUE_HC, 100.0f, TARGET_SELF, 0, 10);

            auto curseOfFattigue = addAISpell(KRIKTHIR_MINDFLAY_HC, 100.0f, TARGET_RANDOM_SINGLE, 0, 7);
            curseOfFattigue->setMinMaxDistance(0.0f, 30.0f);
        }

        mEnraged = false;

        addEmoteForEvent(Event_OnCombatStart, 3908);    // This kingdom belongs to the Scourge. Only the dead may enter!
        addEmoteForEvent(Event_OnTargetDied, 3910);     // As Anub'arak commands!
        addEmoteForEvent(Event_OnTargetDied, 3909);     // You were foolish to come.
        addEmoteForEvent(Event_OnDied, 3911);           // I should be grateful... but I long ago lost the capacity....
    }

    void AIUpdate() override
    {
        // case for scriptphase
        if (getCreature()->getHealthPct() <= 10 && mEnraged == false)
        {
            _applyAura(KRIKTHIR_ENRAGE);
            mEnraged = true;
        }
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        GameObject* Doors = getNearestGameObject(192395);
        if (Doors != nullptr)
            Doors->Despawn(0, 0);
    }
    bool mEnraged;
};

// Hadronox
class HadronoxAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HadronoxAI)
    explicit HadronoxAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (!_isHeroic())
        {
            addAISpell(HADRONOX_WEBGRAB, 22.0f, TARGET_RANDOM_SINGLE, 0, 14);

            auto leechpoison = addAISpell(HADRONOX_LEECHPOISON, 14.0f, TARGET_SELF, 0, 25);
            leechpoison->setMinMaxDistance(0.0f, 20.0f);

            auto acidcloud = addAISpell(HADRONOX_ACIDCLOUD, 18.0f, TARGET_RANDOM_SINGLE, 0, 20);
            acidcloud->setMinMaxDistance(0.0f, 60.0f);
        }
        else
        {
            addAISpell(HADRONOX_WEBGRAB_HC, 22.0f, TARGET_RANDOM_SINGLE, 0, 14);

            auto leechpoison = addAISpell(HADRONOX_LEECHPOISON_HC, 14.0f, TARGET_SELF, 0, 25);
            leechpoison->setMinMaxDistance(0.0f, 20.0f);

            auto acidcloud = addAISpell(HADRONOX_ACIDCLOUD_HC, 18.0f, TARGET_RANDOM_SINGLE, 0, 20);
            acidcloud->setMinMaxDistance(0.0f, 60.0f);
        }

        addAISpell(HADRONOX_PIERCEARMOR, 20.0f, TARGET_ATTACKING, 0, 5);
    }
};

// Watcher Gashra
class GashraAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GashraAI)
    explicit GashraAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(GASHRA_WEBWRAP, 22.0f, TARGET_RANDOM_SINGLE, 0, 35);
        addAISpell(GASHRA_INFECTEDBITE, 35.0f, TARGET_ATTACKING, 0, 12);
    }
};

// Watcher Narjil
class NarjilAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(NarjilAI)
    explicit NarjilAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(NARJIL_WEBWRAP, 22.0f, TARGET_RANDOM_SINGLE, 0, 35);
        addAISpell(NARJIL_INFECTEDBITE, 35.0f, TARGET_ATTACKING, 0, 12);
        addAISpell(NARJIL_BLINDINGWEBS, 16.0f, TARGET_ATTACKING, 0, 9);
    }
};

// Watcher Silthik
class SilthikAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SilthikAI)
    explicit SilthikAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(NARJIL_WEBWRAP, 22.0f, TARGET_RANDOM_SINGLE, 0, 35);
        addAISpell(NARJIL_INFECTEDBITE, 35.0f, TARGET_ATTACKING, 0, 12);
        addAISpell(SILTHIK_POISONSPRAY, 30.0f, TARGET_RANDOM_SINGLE, 0, 15);
    }
};

// Anub'ar Shadowcaster (anub shadowcaster)
class AnubShadowcasterAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AnubShadowcasterAI)
    explicit AnubShadowcasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHADOWCASTER_SHADOWBOLT, 36.0f, TARGET_RANDOM_SINGLE, 0, 8);
        addAISpell(SHADOWCASTER_SHADOW_NOVA, 22.0f, TARGET_SELF, 0, 15);
    }
};

void SetupAzjolNerub(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_AZJOL_NERUB, &AzjolNerubInstanceScript::Create);

    // Bosses
    mgr->register_creature_script(BOSS_KRIKTHIR, &KrikthirAI::Create);
    mgr->register_creature_script(BOSS_HADRONOX, &HadronoxAI::Create);

    // watchers
    mgr->register_creature_script(CN_GASHRA, &GashraAI::Create);
    mgr->register_creature_script(CN_NARJIL, &NarjilAI::Create);
    mgr->register_creature_script(CN_SILTHIK, &SilthikAI::Create);
}
