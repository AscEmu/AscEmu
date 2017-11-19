/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "SpellMgr.h"
#include "SpellAuras.h"
#include "Customization/SpellCustomizations.hpp"

initialiseSingleton(SpellFactoryMgr);

void SpellFactoryMgr::AddSpellByEntry(SpellInfo* info, spell_factory_function spell_func)
{
    if (info != NULL)
        info->SpellFactoryFunc = (void * (*)) spell_func;
}

void SpellFactoryMgr::AddSpellById(uint32 spellId, spell_factory_function spell_func)
{
    AddSpellByEntry(sSpellCustomizations.GetSpellInfo(spellId), spell_func);
}

void SpellFactoryMgr::AddAuraByEntry(SpellInfo* info, aura_factory_function aura_func)
{
    if (info != NULL)
        info->AuraFactoryFunc = (void * (*)) aura_func;
}

void SpellFactoryMgr::AddAuraById(uint32 spellId, aura_factory_function aura_func)
{
    AddAuraByEntry(sSpellCustomizations.GetSpellInfo(spellId), aura_func);
}

SpellInfo* SpellFactoryMgr::GetSpellEntryByDifficulty(uint32 id, uint8 difficulty)
{
#if VERSION_STRING > TBC
    auto spell_difficulty = sSpellDifficultyStore.LookupEntry(id);
    if (spell_difficulty == nullptr)
        return NULL;

    if (spell_difficulty->SpellId[difficulty] <= 0)
        return NULL;

    return sSpellCustomizations.GetSpellInfo(spell_difficulty->SpellId[difficulty]);
#else
    return nullptr;
#endif
}

Spell* SpellFactoryMgr::NewSpell(Object* Caster, SpellInfo* info, bool triggered, Aura* aur)
{
    if (info->SpellFactoryFunc == NULL)
        return new Spell(Caster, info, triggered, aur);
    else
    {
        spell_factory_function ptr;
        ptr = *spell_factory_function(info->SpellFactoryFunc);
        return (*ptr)(Caster, info, triggered, aur);
    }
}

Aura* SpellFactoryMgr::NewAura(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary, Item* i_caster)
{
    if (proto->AuraFactoryFunc == NULL)
        return new Aura(proto, duration, caster, target, temporary, i_caster);
    else
    {
        aura_factory_function ptr;
        ptr = *aura_factory_function(proto->AuraFactoryFunc);
        return (*ptr)(proto, duration, caster, target, temporary, i_caster);
    }
}

void SpellFactoryMgr::LoadSpellAreas()
{
    mSpellAreaMap.clear();
    mSpellAreaForQuestMap.clear();
    mSpellAreaForActiveQuestMap.clear();
    mSpellAreaForQuestEndMap.clear();
    mSpellAreaForAuraMap.clear();

    //                                                  0     1         2              3               4           5          6        7       8
    QueryResult* result = WorldDatabase.Query("SELECT spell, area, quest_start, quest_start_active, quest_end, aura_spell, racemask, gender, autocast FROM spell_area");

    if (!result)
    {
        LOG_DETAIL("Loaded 0 spell area requirements. DB table `spell_area` is empty.");
        return;
    }

    uint32 pCount = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 spell = fields[0].GetUInt32();
        SpellArea spellArea;
        spellArea.spellId = spell;
        spellArea.areaId = fields[1].GetUInt32();
        spellArea.questStart = fields[2].GetUInt32();
        spellArea.questStartCanActive = fields[3].GetBool();
        spellArea.questEnd = fields[4].GetUInt32();
        spellArea.auraSpell = fields[5].GetInt32();
        spellArea.raceMask = fields[6].GetUInt32();
        spellArea.gender = Gender(fields[7].GetUInt8());
        spellArea.autocast = fields[8].GetBool();

        {
            bool ok = true;
            SpellAreaMapBounds sa_bounds = GetSpellAreaMapBounds(spellArea.spellId);
            for (SpellAreaMap::const_iterator itr = sa_bounds.first; itr != sa_bounds.second; ++itr)
            {
                if (spellArea.spellId != itr->second.spellId)
                    continue;
                if (spellArea.areaId != itr->second.areaId)
                    continue;
                if (spellArea.questStart != itr->second.questStart)
                    continue;
                if (spellArea.auraSpell != itr->second.auraSpell)
                    continue;
                if ((spellArea.raceMask & itr->second.raceMask) == 0)
                    continue;
                if (spellArea.gender != itr->second.gender)
                    continue;

                // duplicate by requirements
                ok = false;
                break;
            }

            if (!ok)
            {
                LOG_ERROR("Spell %u listed in `spell_area` already listed with similar requirements.", spell);
                continue;
            }
        }

        /*
        if (spellArea.areaId && !AreaTriggerStorage.LookupEntry(spellArea.areaId))
        {
        printf("Spell %u listed in `spell_area` have wrong area (%u) requirement \n", spell, spellArea.areaId);
        continue;
        }
        */

        if (spellArea.questStart && !sMySQLStore.getQuestProperties(spellArea.questStart))
        {
            LOG_ERROR("Spell %u listed in `spell_area` have wrong start quest (%u) requirement.", spell, spellArea.questStart);
            continue;
        }

        if (spellArea.questEnd)
        {
            if (!sMySQLStore.getQuestProperties(spellArea.questEnd))
            {
                LOG_ERROR("Spell %u listed in `spell_area` have wrong end quest (%u) requirement.", spell, spellArea.questEnd);
                continue;
            }

            if (spellArea.questEnd == spellArea.questStart && !spellArea.questStartCanActive)
            {
                LOG_ERROR("Spell %u listed in `spell_area` have quest (%u) requirement for start and end in same time.", spell, spellArea.questEnd);
                continue;
            }
        }

        if (spellArea.auraSpell)
        {
            SpellInfo const* spellInfo = sSpellCustomizations.GetSpellInfo(abs(spellArea.auraSpell));
            if (!spellInfo)
            {
                LOG_ERROR("Spell %u listed in `spell_area` have wrong aura spell (%u) requirement.", spell, abs(spellArea.auraSpell));
                continue;
            }

            if (uint32(abs(spellArea.auraSpell)) == spellArea.spellId)
            {
                LogDebugFlag(LF_SPELL, "Spell %u listed in `spell_area` have aura spell (%u) requirement for itself.", spell, abs(spellArea.auraSpell));
                continue;
            }

            // not allow autocast chains by auraSpell field (but allow use as alternative if not present)
            if (spellArea.autocast && spellArea.auraSpell > 0)
            {
                bool chain = false;
                SpellAreaForAuraMapBounds saBound = GetSpellAreaForAuraMapBounds(spellArea.spellId);
                for (SpellAreaForAuraMap::const_iterator itr = saBound.first; itr != saBound.second; ++itr)
                {
                    if (itr->second->autocast && itr->second->auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    LogDebugFlag(LF_SPELL, "Spell %u listed in `spell_area` have aura spell (%u) requirement that itself autocast from aura.", spell, spellArea.auraSpell);
                    continue;
                }

                SpellAreaMapBounds saBound2 = GetSpellAreaMapBounds(spellArea.auraSpell);
                for (SpellAreaMap::const_iterator itr2 = saBound2.first; itr2 != saBound2.second; ++itr2)
                {
                    if (itr2->second.autocast && itr2->second.auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    LogDebugFlag(LF_SPELL, "Spell %u listed in `spell_area` have aura spell (%u) requirement that itself autocast from aura.", spell, spellArea.auraSpell);
                    continue;
                }
            }
        }

        if (spellArea.raceMask && (spellArea.raceMask & RACEMASK_ALL_PLAYABLE) == 0)
        {
            LOG_ERROR("Spell %u listed in `spell_area` have wrong race mask (%u) requirement.", spell, spellArea.raceMask);
            continue;
        }

        if (spellArea.gender != GENDER_NONE && spellArea.gender != GENDER_FEMALE && spellArea.gender != GENDER_MALE)
        {
            LOG_ERROR("Spell %u listed in `spell_area` have wrong gender (%u) requirement.", spell, spellArea.gender);
            continue;
        }

        SpellArea const* sa = &mSpellAreaMap.insert(SpellAreaMap::value_type(spell, spellArea))->second;

        // for search by current zone/subzone at zone/subzone change
        if (spellArea.areaId)
            mSpellAreaForAreaMap.insert(SpellAreaForAreaMap::value_type(spellArea.areaId, sa));

        // for search at quest start/reward
        if (spellArea.questStart)
        {
            if (spellArea.questStartCanActive)
                mSpellAreaForActiveQuestMap.insert(SpellAreaForQuestMap::value_type(spellArea.questStart, sa));
            else
                mSpellAreaForQuestMap.insert(SpellAreaForQuestMap::value_type(spellArea.questStart, sa));
        }

        // for search at quest start/reward
        if (spellArea.questEnd)
            mSpellAreaForQuestEndMap.insert(SpellAreaForQuestMap::value_type(spellArea.questEnd, sa));

        // for search at aura apply
        if (spellArea.auraSpell)
            mSpellAreaForAuraMap.insert(SpellAreaForAuraMap::value_type(abs(spellArea.auraSpell), sa));

        ++pCount;
    } while (result->NextRow());
    delete result;

    LOG_DETAIL("Loaded %u spell area requirements.", pCount);
}

SpellAreaMapBounds SpellFactoryMgr::GetSpellAreaMapBounds(uint32 spell_id) const
{
    return SpellAreaMapBounds(mSpellAreaMap.lower_bound(spell_id), mSpellAreaMap.upper_bound(spell_id));
}

SpellAreaForQuestMapBounds SpellFactoryMgr::GetSpellAreaForQuestMapBounds(uint32 quest_id, bool active) const
{
    if (active)
        return SpellAreaForQuestMapBounds(mSpellAreaForActiveQuestMap.lower_bound(quest_id), mSpellAreaForActiveQuestMap.upper_bound(quest_id));
    else
        return SpellAreaForQuestMapBounds(mSpellAreaForQuestMap.lower_bound(quest_id), mSpellAreaForQuestMap.upper_bound(quest_id));
}

SpellAreaForQuestMapBounds SpellFactoryMgr::GetSpellAreaForQuestEndMapBounds(uint32 quest_id) const
{
    return SpellAreaForQuestMapBounds(mSpellAreaForQuestEndMap.lower_bound(quest_id), mSpellAreaForQuestEndMap.upper_bound(quest_id));
}

SpellAreaForAuraMapBounds SpellFactoryMgr::GetSpellAreaForAuraMapBounds(uint32 spell_id) const
{
    return SpellAreaForAuraMapBounds(mSpellAreaForAuraMap.lower_bound(spell_id), mSpellAreaForAuraMap.upper_bound(spell_id));
}

SpellAreaForAreaMapBounds SpellFactoryMgr::GetSpellAreaForAreaMapBounds(uint32 area_id) const
{
    return SpellAreaForAreaMapBounds(mSpellAreaForAreaMap.lower_bound(area_id), mSpellAreaForAreaMap.upper_bound(area_id));
}

bool SpellArea::IsFitToRequirements(Player* player, uint32 newZone, uint32 newArea) const
{
    if (gender != GENDER_NONE)                   // not in expected gender
        if (!player || gender != player->getGender())
            return false;

    if (raceMask)                                // not in expected race
        if (!player || !(raceMask & player->getRaceMask()))
            return false;

    if (areaId)                                  // not in expected zone
        if (newZone != areaId && newArea != areaId)
            return false;

    if (questStart)                              // not in expected required quest state
        if (!player || !player->HasQuest(questStart))
            return false;

    if (questEnd)                                // not in expected forbidden quest state
        if (!player || player->GetQuestRewardStatus(questEnd))
            return false;

    if (auraSpell)                               // not have expected aura
        if (!player || (auraSpell > 0 && !player->HasAura(auraSpell)) || (auraSpell < 0 && player->HasAura(-auraSpell)))
            return false;

    // Misc Conditions
    switch (spellId)
    {
        case 58600: //Restricted Flight Zone (Dalaran)
        {
            if (!player)
                return false;

            if (!player->HasAura(SPELL_AURA_ENABLE_FLIGHT2) && !player->HasAura(SPELL_AURA_FLY))
                return false;
            break;
        }
    }
    return true;
}

bool SpellInfo::CheckLocation(uint32_t /*map_id*/, uint32_t zone_id, uint32_t area_id, Player* player)
{
#if VERSION_STRING > TBC
    // normal case
    if (RequiresAreaId > 0)
    {
        bool found = false;
        auto area_group = sAreaGroupStore.LookupEntry(RequiresAreaId);
        while (area_group)
        {
            for (uint8 i = 0; i < 6; ++i)
                if (area_group->AreaId[i] == zone_id || area_group->AreaId[i] == area_id)
                    found = true;
            if (found || !area_group->next_group)
                break;

            //try next group
            area_group = sAreaGroupStore.LookupEntry(area_group->next_group);
        }

        if (!found)
            return false;
    }
#endif

    // DB base check (if non empty then must fit at least single for allow)
    SpellAreaMapBounds saBounds = sSpellFactoryMgr.GetSpellAreaMapBounds(Id);
    if (saBounds.first != saBounds.second)
    {
        for (SpellAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            if (itr->second.IsFitToRequirements(player, zone_id, area_id))
                return true;
        }
        return false;
    }

    return false;
}

void SpellFactoryMgr::Setup()
{
    SetupSpellClassScripts();
}
