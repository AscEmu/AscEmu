/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Objects/Units/Players/Player.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellScript.hpp"

enum MiscSpells
{
    SPELL_FEIGN_DEATH               = 51329,
    SPELL_GREATER_POLYMORPH         = 22274,
    SPELL_PERMANENT_FEIGN_DEATH_1   = 29266,
    SPELL_PERMANENT_FEIGN_DEATH_2   = 57685,
    SPELL_PERMANENT_FEIGN_DEATH_3   = 58951,
    SPELL_PERMANENT_FEIGN_DEATH_4   = 70592,
    SPELL_PERMANENT_FEIGN_DEATH_5   = 70628,
    SPELL_PERMANENT_FEIGN_DEATH_6   = 74490,
    SPELL_POLYMORPH                 = 851,
};

// Generic feign death spells, can be added with/without dynamic flag and prevent chat emote flag
class GenericFeignDeath : public SpellScript
{
public:
    GenericFeignDeath(bool preventChatEmotes = true, bool withDynamicFlag = true) : _preventChatEmotes(preventChatEmotes), _withDynamicFlag(withDynamicFlag) {}

    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        if (apply)
        {
#if VERSION_STRING >= TBC
            aur->getOwner()->addUnitFlags2(UNIT_FLAG2_FEIGN_DEATH);
#endif
            if (_withDynamicFlag)
                aur->getOwner()->addDynamicFlags(U_DYN_FLAG_DEAD);
            if (_preventChatEmotes)
                aur->getOwner()->addUnitFlags(UNIT_FLAG_FEIGN_DEATH);
        }
        else
        {
#if VERSION_STRING >= TBC
            aur->getOwner()->removeUnitFlags2(UNIT_FLAG2_FEIGN_DEATH);
#endif
            if (_withDynamicFlag)
                aur->getOwner()->removeDynamicFlags(U_DYN_FLAG_DEAD);
            if (_preventChatEmotes)
                aur->getOwner()->removeUnitFlags(UNIT_FLAG_FEIGN_DEATH);
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }

private:
    bool _preventChatEmotes = true;
    bool _withDynamicFlag = true;
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
                    aur->getPlayerOwner()->dismount();
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

#if VERSION_STRING >= WotLK
    mgr->register_spell_script(SPELL_FEIGN_DEATH, new GenericFeignDeath(true, false));
#endif

    mgr->register_spell_script(SPELL_GREATER_POLYMORPH, new Polymorph(false, true));

    mgr->register_spell_script(SPELL_PERMANENT_FEIGN_DEATH_1, new GenericFeignDeath);
#if VERSION_STRING >= WotLK
    mgr->register_spell_script(SPELL_PERMANENT_FEIGN_DEATH_2, new GenericFeignDeath);
    mgr->register_spell_script(SPELL_PERMANENT_FEIGN_DEATH_3, new GenericFeignDeath(false));
    mgr->register_spell_script(SPELL_PERMANENT_FEIGN_DEATH_4, new GenericFeignDeath);
    mgr->register_spell_script(SPELL_PERMANENT_FEIGN_DEATH_5, new GenericFeignDeath);
    mgr->register_spell_script(SPELL_PERMANENT_FEIGN_DEATH_6, new GenericFeignDeath);
#endif

    mgr->register_spell_script(SPELL_POLYMORPH, new Polymorph(true, false));
}
