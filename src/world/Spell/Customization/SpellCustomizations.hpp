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

        typedef std::unordered_map<uint32, OLD_SpellEntry> ServersideSpellContainer;

        void LoadServersideSpells();
        OLD_SpellEntry* GetServersideSpell(uint32 spell_id);
        ServersideSpellContainer* GetServersideSpellStore() { return &_serversideSpellContainerStore; }

        void StartSpellCustomization();

        // functions for setting up custom vars
        void LoadSpellRanks();
        void LoadSpellCustomAssign();
        void LoadSpellCustomCoefFlags();
        void LoadSpellProcs();

        void SetEffectAmplitude(OLD_SpellEntry* spell_entry);
        void SetAuraFactoryFunc(OLD_SpellEntry* spell_entry);

        void SetMeleeSpellBool(OLD_SpellEntry* spell_entry);
        void SetRangedSpellBool(OLD_SpellEntry* spell_entry);

        void SetMissingCIsFlags(OLD_SpellEntry* spell_entry);
        void SetCustomFlags(OLD_SpellEntry* spell_entry);
        void SetOnShapeshiftChange(OLD_SpellEntry* spell_entry);
        void SetAlwaysApply(OLD_SpellEntry* spell_entry);

        ServersideSpellContainer _serversideSpellContainerStore;
};

#define sSpellCustomizations SpellCustomizations::getSingleton()

#endif  // _SPELL_CUSTOMIZATIONSE_HPP
