/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Spell/SpellMgr.hpp"

enum WarriorSpells
{
    SPELL_FLURRY_DUMMY_R1   = 12319,
    SPELL_FLURRY_DUMMY_R2   = 12971,
    SPELL_FLURRY_DUMMY_R3   = 12972,
    SPELL_FLURRY_DUMMY_R4   = 12973,
    SPELL_FLURRY_DUMMY_R5   = 12974,
    SPELL_FLURRY_PROC_R1    = 12966,
    SPELL_FLURRY_PROC_R2    = 12967,
    SPELL_FLURRY_PROC_R3    = 12968,
    SPELL_FLURRY_PROC_R4    = 12969,
    SPELL_FLURRY_PROC_R5    = 12970,
};

#if VERSION_STRING < Mop
class Flurry : public SpellScript
{
public:
    void onCreateSpellProc(SpellProc* proc, Object* /*obj*/) override
    {
        proc->setExtraProcFlags(EXTRA_PROC_ON_CRIT_ONLY);
    }

    void onAuraApply(Aura* aur) override
    {
        // Make the proc aura proc its mother spell
        // This is only needed to consume proc charges from this aura on melee attacks
        // DBC data also have PROC_ON_DONE_MELEE_HIT flag set so this is not probably far-fetched
        motherAuraId = aur->pSpellId;
        spellProc = aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(motherAuraId), aur, aur->getCasterGuid());
        if (spellProc != nullptr)
            spellProc->skipOnNextHandleProc(true);
    }

    void onAuraRefreshOrGainNewStack(Aura* /*aur*/, uint32_t /*newStacks*/, uint32_t /*oldStacks*/) override
    {
        // If this proc is not skipped in next ::handleProc event,
        // the same attack, that created this aura, will consume one proc charge
        if (spellProc != nullptr)
            spellProc->skipOnNextHandleProc(true);
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeProcTriggerSpell(motherAuraId, aur->getCasterGuid());
        motherAuraId = 0;
        spellProc = nullptr;
    }

private:
    uint32_t motherAuraId = 0;
    SpellProc* spellProc = nullptr;
};

class FlurryDummy : public SpellScript
{
    SpellScriptExecuteState onCastProcSpell(SpellProc* /*spellProc*/, Unit* /*caster*/, Unit* /*victim*/, Spell* /*spellToProc*/) override
    {
        // Do not cast the spell, this is just needed to consume the proc charges from Flurry aura
        // If the cast is prevented earlier in onDoProcEffect, the proc system assumes the proc did not happen
        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};
#endif

void setupWarriorSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyWarriorSpells(mgr);

#if VERSION_STRING < Mop
    uint32_t flurryIds[] =
    {
        SPELL_FLURRY_PROC_R1,
        SPELL_FLURRY_PROC_R2,
        SPELL_FLURRY_PROC_R3,
#if VERSION_STRING < Cata
        SPELL_FLURRY_PROC_R4,
        SPELL_FLURRY_PROC_R5,
#endif
        0
    };
    mgr->register_spell_script(flurryIds, new Flurry);

    uint32_t flurryDummyIds[] =
    {
        SPELL_FLURRY_DUMMY_R1,
        SPELL_FLURRY_DUMMY_R2,
        SPELL_FLURRY_DUMMY_R3,
#if VERSION_STRING < Cata
        SPELL_FLURRY_DUMMY_R4,
        SPELL_FLURRY_DUMMY_R5,
#endif
        0
    };
    mgr->register_spell_script(flurryDummyIds, new FlurryDummy);
#endif
}
