/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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

#include "Singleton.h"
#include "Storage/DBC/DBCStores.h"

#ifndef _SPELL_CUSTOMIZATIONS_HPP
#define _SPELL_CUSTOMIZATIONS_HPP


class SERVER_DECL SpellCustomizations : public Singleton <SpellCustomizations>
{
    public:

        SpellCustomizations();
        ~SpellCustomizations();

        typedef std::unordered_map<uint32, SpellInfo> SpellInfoContainer;

        void LoadSpellInfoData();
        SpellInfo* GetSpellInfo(uint32 spell_id);
        SpellInfoContainer* GetSpellInfoStore() { return &_spellInfoContainerStore; }

        void StartSpellCustomization();

        // functions for setting up custom vars
        void LoadSpellRanks();
        void LoadSpellCustomAssign();
        void LoadSpellCustomCoefFlags();
        void LoadSpellProcs();

        void SetEffectAmplitude(SpellInfo* spell_entry);
        void SetAuraFactoryFunc(SpellInfo* spell_entry);

        void SetMeleeSpellBool(SpellInfo* spell_entry);
        void SetRangedSpellBool(SpellInfo* spell_entry);

        void SetMissingCIsFlags(SpellInfo* spell_entry);
        void SetCustomFlags(SpellInfo* spell_entry);
        void SetOnShapeshiftChange(SpellInfo* spell_entry);
        void SetAlwaysApply(SpellInfo* spell_entry);

        SpellInfoContainer _spellInfoContainerStore;
};

#define sSpellCustomizations SpellCustomizations::getSingleton()

#endif  // _SPELL_CUSTOMIZATIONSE_HPP
