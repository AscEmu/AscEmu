/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
 */

#ifndef SPELLMGR_H
#define SPELLMGR_H

#include "Management/Item.h"
#include "Units/Players/PlayerDefines.hpp"

struct SpellArea
{
	uint32 spellId;
	uint32 areaId;              // zone/subzone/or 0 is not limited to zone
	uint32 questStart;          // quest start (quest must be active or rewarded for spell apply)
	uint32 questEnd;            // quest end (quest must not be rewarded for spell apply)
	int32 auraSpell;           // spell aura must be applied for spell apply)if possitive) and it must not be applied in other case
	uint32 raceMask;            // can be applied only to races
	Gender gender;              // can be applied only to gender
	bool questStartCanActive;   // if true then quest start can be active (not only rewarded)
	bool autocast;              // if true then auto applied at area enter, in other case just allowed to cast

	// helpers
	bool IsFitToRequirements(Player* player, uint32 newZone, uint32 newArea) const;
};

typedef std::multimap<uint32, SpellArea> SpellAreaMap;
typedef std::multimap<uint32, SpellArea const*> SpellAreaForQuestMap;
typedef std::multimap<uint32, SpellArea const*> SpellAreaForAuraMap;
typedef std::multimap<uint32, SpellArea const*> SpellAreaForAreaMap;
typedef std::pair<SpellAreaMap::const_iterator, SpellAreaMap::const_iterator> SpellAreaMapBounds;
typedef std::pair<SpellAreaForQuestMap::const_iterator, SpellAreaForQuestMap::const_iterator> SpellAreaForQuestMapBounds;
typedef std::pair<SpellAreaForAuraMap::const_iterator, SpellAreaForAuraMap::const_iterator> SpellAreaForAuraMapBounds;
typedef std::pair<SpellAreaForAreaMap::const_iterator, SpellAreaForAreaMap::const_iterator> SpellAreaForAreaMapBounds;

class Aura;

typedef Spell* (*spell_factory_function)(Object* Caster, SpellInfo* info, bool triggered, Aura* aur);
typedef Aura* (*aura_factory_function)(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary, Item* i_caster);

class SERVER_DECL SpellFactoryMgr: public Singleton < SpellFactoryMgr >
{
	public:

		SpellFactoryMgr()
		{
			Setup();
		}

		~SpellFactoryMgr()
		{
		}

		SpellInfo* GetSpellEntryByDifficulty(uint32 id, uint8 difficulty);
		Spell* NewSpell(Object* Caster, SpellInfo* info, bool triggered, Aura* aur);
		Aura* NewAura(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = NULL);

		// Spell area
		void LoadSpellAreas();

		SpellAreaMapBounds GetSpellAreaMapBounds(uint32 spell_id) const;
		SpellAreaForQuestMapBounds GetSpellAreaForQuestMapBounds(uint32 quest_id, bool active) const;
		SpellAreaForQuestMapBounds GetSpellAreaForQuestEndMapBounds(uint32 quest_id) const;
		SpellAreaForAuraMapBounds GetSpellAreaForAuraMapBounds(uint32 spell_id) const;
		SpellAreaForAreaMapBounds GetSpellAreaForAreaMapBounds(uint32 area_id) const;

	private:

		void AddSpellByEntry(SpellInfo* info, spell_factory_function spell_func);
		void AddSpellById(uint32 spellId, spell_factory_function spell_func);

		void AddAuraByEntry(SpellInfo* info, aura_factory_function aura_func);
		void AddAuraById(uint32 spellId, aura_factory_function aura_func);

		void Setup();

        void SetupSpellClassScripts();

		SpellAreaMap mSpellAreaMap;
		SpellAreaForQuestMap mSpellAreaForQuestMap;
		SpellAreaForQuestMap mSpellAreaForActiveQuestMap;
		SpellAreaForQuestMap mSpellAreaForQuestEndMap;
		SpellAreaForAuraMap mSpellAreaForAuraMap;
		SpellAreaForAreaMap mSpellAreaForAreaMap;
};

#define sSpellFactoryMgr SpellFactoryMgr::getSingleton()

#endif // SPELLMGR_H
