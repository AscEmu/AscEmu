/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

enum MiscSpells
{
    SPELL_GREATER_POLYMORPH     = 22274,
    SPELL_POLYMORPH             = 851,
};

// Polymorph spells used by creatures
class Polymorph : public SpellScript
{
public:
    Polymorph(bool requiresDismount, bool regeneratesHealth) : _dismount(requiresDismount), _health(regeneratesHealth) {}

    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getAuraEffectType() != SPELL_AURA_TRANSFORM)
            return SpellScriptExecuteState::EXECUTE_OK;

        if (apply)
        {
            if (_dismount)
            {
                // todo: creatures
                if (aur->getPlayerOwner() != nullptr)
                    aur->getPlayerOwner()->Dismount();
            }

            if (_health)
                aur->getOwner()->addUnitStateFlag(UNIT_STATE_POLYMORPHED);
        }
        else
        {
            if (_health)
                aur->getOwner()->removeUnitStateFlag(UNIT_STATE_POLYMORPHED);
        }

        return SpellScriptExecuteState::EXECUTE_OK;
    }

private:
    bool _dismount = false;
    bool _health = false;
};

void setupMiscSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyMiscSpellhandlers(mgr);

    mgr->register_spell_script(SPELL_GREATER_POLYMORPH, new Polymorph(false, true));
    mgr->register_spell_script(SPELL_POLYMORPH, new Polymorph(true, false));
}
