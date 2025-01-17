/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Objects/Units/Unit.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/SpellProc.hpp"
#include "Spell/SpellScript.hpp"
#include "Spell/Definitions/ProcFlags.hpp"

enum WarriorSpells
{
    SPELL_FLURRY_DUMMY_R1   = 12319,
    SPELL_FLURRY_PROC_R1    = 12966,
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
    mgr->register_spell_script(SPELL_FLURRY_PROC_R1, new Flurry);
    mgr->register_spell_script(SPELL_FLURRY_DUMMY_R1, new FlurryDummy);
#endif
}
