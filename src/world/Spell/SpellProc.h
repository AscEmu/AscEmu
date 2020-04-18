/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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

#ifndef SPELL_PROC_H
#define SPELL_PROC_H

class SpellProc;
class Object;

#include "SpellInfo.hpp"

class Unit;

typedef SpellProc* (*spell_proc_factory_function)();

typedef std::unordered_map<uint32_t, spell_proc_factory_function>  SpellProcMap;

class SpellProc
{
    public:

        ~SpellProc()
        {
        }

        // Returns true if this spell can proc, false otherwise
        virtual bool CanProc(Unit* victim, SpellInfo const* CastingSpell);

        // Called when procFlags is to be compared.
        // Return true on success, false otherwise
        virtual bool CheckProcFlags(uint32_t flag);

        // Check if this object is identified by method arguments, so it can be deleted
        virtual bool CanDelete(uint32_t spellId, uint64_t casterGuid = 0, uint64_t misc = 0);

        // Called when is proccing from casting spell. It checks proc class mask with spell group type
        // Return true allow proc, false otherwise
        virtual bool CheckClassMask(Unit* victim, SpellInfo const* CastingSpell);

        // Called after proc chance is rolled
        // Return false so Unit::HandleProc execute subsequent statements
        // Return true if this handle everything, so Unit::HandleProc skips to next iteration
        virtual bool DoEffect(Unit* victim, SpellInfo const* CastingSpell, uint32_t flag, uint32_t dmg, uint32_t abs, int* dmg_overwrite, uint32_t weapon_damage_type);

        // Called just after this object is created. Usefull for initialize object members
        virtual void Init(Object* obj);

        virtual uint32_t CalcProcChance(Unit* victim, SpellInfo const* CastingSpell);

        // Called when trying to proc on a triggered spell
        // Return true allow proc, false otherwise
        virtual bool CanProcOnTriggered(Unit* victim, SpellInfo const* CastingSpell);

        // Cast proc spell
        virtual void CastSpell(Unit* victim, SpellInfo const* CastingSpell, int* dmg_overwrite);

        // Spell to proc
        SpellInfo const* mSpell;

        // Spell that created this proc
        SpellInfo const* mOrigSpell;

        // Unit 'owner' of this proc
        Unit* mTarget;

        // GUID of the caster of this proc
        uint64_t mCaster;

        uint32_t mProcChance;
        uint32_t mProcFlags;
        uint32_t mProcCharges;

        // Time of last time of proc
        uint32_t mLastTrigger;

        // Mask used to compare with casting spell group_type
        uint32_t mProcClassMask[3];

        // Mask used on spell effect
        uint32_t mGroupRelation[3];

        // Indicate that this object is deleted, and should be remove on next iteration
        bool mDeleted;
};

class SpellProcMgr
{
    private:

        SpellProcMgr() = default;
        ~SpellProcMgr() = default;

    public:

        static SpellProcMgr& getInstance()
        {
            static SpellProcMgr mInstance;
            return mInstance;
        }

        void initialize()
        {
            Setup();
        }

        SpellProcMgr(SpellProcMgr&&) = delete;
        SpellProcMgr(SpellProcMgr const&) = delete;
        SpellProcMgr& operator=(SpellProcMgr&&) = delete;
        SpellProcMgr& operator=(SpellProcMgr const&) = delete;

        SpellProc* NewSpellProc(Unit* target, uint32_t spell_id, uint32_t orig_spell_id, uint64_t caster, uint32_t procChance, uint32_t procFlags, uint32_t procCharges, uint32_t* groupRelation, uint32_t* procClassMask, Object* obj);

        SpellProc* NewSpellProc(Unit* target, SpellInfo const* spell, SpellInfo const* orig_spell, uint64_t caster, uint32_t procChance, uint32_t procFlags, uint32_t procCharges, uint32_t* groupRelation, uint32_t* procClassMask, Object* obj);

    private:

        SpellProcMap mSpellProc;

        void AddById(uint32_t spellId, spell_proc_factory_function spell_proc)
        {
            mSpellProc.insert(std::make_pair(spellId, spell_proc));
        }

        void AddById(uint32_t* spellId, spell_proc_factory_function spell_proc)
        {
            for (uint32_t y = 0; spellId[y] != 0; y++)
            {
                mSpellProc.insert(std::make_pair(spellId[y], spell_proc));
            }
        }

        void Setup();

        void SetupItems();
        void SetupSpellProcClassScripts();
};

#define sSpellProcMgr SpellProcMgr::getInstance()

#endif // _SPELL_PROC_H
