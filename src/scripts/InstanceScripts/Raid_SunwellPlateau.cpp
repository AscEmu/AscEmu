/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"

/*
//Kil'Jaeden sound IDs saved for future use
//Some of them are used anytime during the raid progression in the instance (no mechanism for this yet)
//Some others are used in the actual Kil'Jaeden encounter

12527 Sunwell Plateau - Kil Jaeden "Spawning"
12495 Sunwell Plateau - Kil Jaeden - "All my plans have led to this"
12496 Sunwell Plateau - Kil Jaeden - "Stay on task, do not waste time"
12497 Sunwell Plateau - Kil Jaeden - "I've waited long enough!"
12498 Sunwell Plateau - Kil Jaeden - "Fail me, and suffer for eternity!"
12499 Sunwell Plateau - Kil Jaeden - "Drain the girl, drain her power, untill there is nothing but an ...something... shell"
12500 Sunwell Plateau - Kil Jaeden - Very long thing, basiclly he tells us that he will take control over the Burning Legion.
12501 Sunwell Plateau - Kil Jaeden - "Another step towards destruction!"
*/

enum 
{
    //CN_SHADOWSWORD_ASSASSIN = 25484,
    SHADOWSWORD_ASSASSIN_ASSASSINS_MARK = 46459,
    SHADOWSWORD_ASSASSIN_AIMED_SHOT = 46460,
    SHADOWSWORD_ASSASSIN_SHADOWSTEP = 46463,
    SHADOWSWORD_ASSASSIN_GREATER_INVISIBILITY = 16380,


    //CN_SHADOWSWORD_COMMANDER = 25837,
    SHADOWSWORD_COMMANDER_SHIELD_SLAM = 46762,
    SHADOWSWORD_COMMANDER_BATTLESHOUT = 46763,

    CN_BRUTALLUS = 24882,
    BRUTALLUS_METEOR_SLASH = 45150,
    BRUTALLUS_BURN = 45141,
    BRUTALLUS_STOMP = 45185,
    BRUTALLUS_BERSERK = 26662,


    CN_FELMYST = 25038,
    FELMYST_CLEAVE = 19983,
    FELMYST_CORROSION = 45866,
    FELMYST_DEMONIC_VAPOR = 45402,
    FELMYST_GAS_NOVA = 45855,
    FELMYST_NOXIOUS_FUME = 47002,
    FELMYST_ENCAPSULATE = 45662,
    //FELMYST_FOG_OF_CORRUPTION = 45717,
    FELMYST_ENRAGE = 26662, // Using same as Brutallus for now, need to find actual spell id


    CN_LADY_SACROLASH = 25165,
    CN_GRAND_WARLOCK_ALYTHESS = 25166,
    LADY_SACROLASH_DARK_TOUCHED = 45347,
    LADY_SACROLASH_SHADOW_BLADES = 45248,
    LADY_SACROLASH_SHADOW_NOVA = 45329,
    LADY_SACROLASH_CONFOUNDING_BLOW = 45256,
    LADY_SACROLASH_ENRAGE = 26662, // Using same as Brutallus for now, need to find actual spell id


    GRAND_WARLOCK_ALYTHESS_PYROGENICS = 45230,
    GRAND_WARLOCK_ALYTHESS_FLAME_TOUCHED = 45348,
    GRAND_WARLOCK_ALYTHESS_CONFLAGRATION = 45342,
    GRAND_WARLOCK_ALYTHESS_BLAZE = 45235,
    GRAND_WARLOCK_ALYTHESS_FLAME_SEAR = 46771,
    GRAND_WARLOCK_ALYTHESS_ENRAGE = 26662, // Using same as Brutallus for now, need to find actual spell id


    CN_MURU = 25741,
    //CN_SHADOWSWORD_BERSERKER = 25798,
    //CN_SHADOWSWORD_FURY_MAGE = 25799,
    //CN_VOID_SENTINEL = 25772,
    MURU_NEGATIVE_ENERGY = 46008, // patch 2.4.2: this spell shouldn't cause casting pushback (to be fixed in core)
    MURU_DARKNESS = 45996,
    //MURU_SUMMON_BERSERKER = 46037,
    //MURU_SUMMON_FURY_MAGE = 46038,
    //MURU_SUMMON_VOID_SENTINEL = 45988,

    CN_SUNBLADE_PROTECTOR = 25507,
    SUNBLADE_PROTECTOR_FEL_LIGHTNING = 46480,
};

class SunbladeProtectorAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SunbladeProtectorAI);
    explicit SunbladeProtectorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto felLightning = addAISpell(SUNBLADE_PROTECTOR_FEL_LIGHTNING, 100.0f, TARGET_RANDOM_SINGLE, 0, 15);
        felLightning->setMinMaxDistance(0.0f, 60.0f);
    }
};

class ShadowswordAssassinAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowswordAssassinAI);
    explicit ShadowswordAssassinAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto assaMark = addAISpell(SHADOWSWORD_ASSASSIN_ASSASSINS_MARK, 100.0f, TARGET_RANDOM_SINGLE, 0, 15);
        assaMark->setMinMaxDistance(0.0f, 100.0f);

        auto aimedShot = addAISpell(SHADOWSWORD_ASSASSIN_AIMED_SHOT, 15.0f, TARGET_ATTACKING, 4, 6, false, true);
        aimedShot->setMinMaxDistance(5.0f, 35.0f);

        auto shadowstep = addAISpell(SHADOWSWORD_ASSASSIN_SHADOWSTEP, 15.0f, TARGET_RANDOM_SINGLE, 0, 50);
        shadowstep->setMinMaxDistance(0.0f, 40.0f);

        addAISpell(SHADOWSWORD_ASSASSIN_GREATER_INVISIBILITY, 5.0f, TARGET_SELF, 0, 180);
    }
};

class ShadowswordCommanderAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowswordCommanderAI);
    explicit ShadowswordCommanderAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHADOWSWORD_COMMANDER_SHIELD_SLAM, 10.0f, TARGET_ATTACKING, 0, 10);
        addAISpell(SHADOWSWORD_COMMANDER_BATTLESHOUT, 20.0f, TARGET_SELF, 0, 25);
    }
};

class BrutallusAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BrutallusAI);
    explicit BrutallusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BRUTALLUS_METEOR_SLASH, 100.0f, TARGET_SELF, 1, 12);
        addAISpell(BRUTALLUS_BURN, 50.0f, TARGET_RANDOM_SINGLE, 0, 20);
        addAISpell(BRUTALLUS_STOMP, 25.0f, TARGET_ATTACKING, 0, 30);

        // 6min Enrage
        mLocaleEnrageSpell = addAISpell(BRUTALLUS_BERSERK, 0.0f, TARGET_SELF);
        mLocaleEnrageSpell->addEmote("So much for a real challenge... Die!", CHAT_MSG_MONSTER_YELL, 12470);

        // Emotes
        addEmoteForEvent(Event_OnCombatStart, 8834);
        addEmoteForEvent(Event_OnTargetDied, 8835);
        addEmoteForEvent(Event_OnTargetDied, 8836);
        addEmoteForEvent(Event_OnTargetDied, 8837);
        addEmoteForEvent(Event_OnDied, 8838);
        addEmoteForEvent(Event_OnTaunt, 8839);
        addEmoteForEvent(Event_OnTaunt, 8840);
        addEmoteForEvent(Event_OnTaunt, 8841);

        mLocaleEnrageTimerId = 0;
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mLocaleEnrageTimerId))
        {
            _castAISpell(mLocaleEnrageSpell);
            _removeTimer(mLocaleEnrageTimerId);
        }
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mLocaleEnrageTimerId = _addTimer(360000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        _removeTimer(mLocaleEnrageTimerId);
    }

    CreatureAISpells* mLocaleEnrageSpell;
    uint32_t mLocaleEnrageTimerId;
};

class FelmystAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FelmystAI);
    explicit FelmystAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //Phase 1 spells
        auto cleave = addAISpell(FELMYST_CLEAVE, 6.0f, TARGET_ATTACKING, 0, 10);
            cleave->setAvailableForScriptPhase({ 1 });

        auto gasNova = addAISpell(FELMYST_GAS_NOVA, 25.0f, TARGET_SELF, 1, 18);
        gasNova->setAvailableForScriptPhase({ 1 });

        auto encapsulate = addAISpell(FELMYST_ENCAPSULATE, 25.0f, TARGET_RANDOM_SINGLE, 7, 30);
        encapsulate->setMinMaxDistance(0.0f, 30.0f);
        encapsulate->setAvailableForScriptPhase({ 1 });

        auto corrosion = addAISpell(FELMYST_CORROSION, 20.0f, TARGET_ATTACKING, 1, 35);
        corrosion->setMinMaxDistance(0.0f, 30.0f);
        corrosion->setAvailableForScriptPhase({ 1 });
        corrosion->addEmote("Choke on your final breath!", CHAT_MSG_MONSTER_YELL, 12478);

        //Phase 2 spells
        auto demonicVapor = addAISpell(FELMYST_DEMONIC_VAPOR, 10.0f, TARGET_RANDOM_SINGLE, 0, 20);
        demonicVapor->setAvailableForScriptPhase({ 2 });

        //Phase 3 spells
        //Fog of corruption is the actual breath Felmyst does during his second phase, probably we'll have to spawn it like a creature.

        //10min Enrage
        mLocaleEnrageSpell = addAISpell(FELMYST_ENRAGE, 0.0f, TARGET_SELF);
        mLocaleEnrageSpell->addEmote("No more hesitation! Your fates are written!", CHAT_MSG_MONSTER_YELL, 12482);

        //Emotes
        addEmoteForEvent(Event_OnCombatStart, 8842);
        addEmoteForEvent(Event_OnTargetDied, 8843);
        addEmoteForEvent(Event_OnTargetDied, 8844);
        addEmoteForEvent(Event_OnDied, 8845);
        addEmoteForEvent(Event_OnTaunt, 8846);

        mLocaleEnrageTimerId = 0;
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mLocaleEnrageTimerId))
        {
            _castAISpell(mLocaleEnrageSpell);
            _removeTimer(mLocaleEnrageTimerId);
        }
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mLocaleEnrageTimerId = _addTimer(600000);
        _applyAura(FELMYST_NOXIOUS_FUME);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        _removeTimer(mLocaleEnrageTimerId);
    }

    CreatureAISpells* mLocaleEnrageSpell;
    uint32_t mLocaleEnrageTimerId;
};

class LadySacrolashAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LadySacrolashAI);
    explicit LadySacrolashAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto darkTouch = addAISpell(LADY_SACROLASH_DARK_TOUCHED, 50.0f, TARGET_RANDOM_SINGLE, 0, 10);
        darkTouch->setMinMaxDistance(0.0f, 50.0f);

        auto shadowBlades = addAISpell(LADY_SACROLASH_SHADOW_BLADES, 25.0f, TARGET_ATTACKING, 2, 5);
        shadowBlades->setMinMaxDistance(0.0f, 50.0f);

        auto shadowNova = addAISpell(LADY_SACROLASH_SHADOW_NOVA, 15.0f, TARGET_RANDOM_SINGLE, 4, 20);
        shadowNova->addEmote("Shadow to the aid of fire!", CHAT_MSG_MONSTER_YELL, 12485);
        shadowNova->setMinMaxDistance(0.0f, 50.0f);

        auto confoundingBlow = addAISpell(LADY_SACROLASH_CONFOUNDING_BLOW, 10.0f, TARGET_RANDOM_SINGLE, 0, 15);
        confoundingBlow->setMinMaxDistance(0.0f, 50.0f);

        mLocaleEnrageSpell = addAISpell(LADY_SACROLASH_ENRAGE, 0.0f, TARGET_SELF);
        mLocaleEnrageSpell->addEmote("Time is a luxury you no longer possess!", CHAT_MSG_MONSTER_YELL, 0); // Wasn't able to find sound for this text

        // Emotes
        addEmoteForEvent(Event_OnTargetDied, 8847);
        addEmoteForEvent(Event_OnDied, 8848);

        mLocaleEnrageTimerId = 0;
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mLocaleEnrageTimerId))
        {
            _castAISpell(mLocaleEnrageSpell);
            _removeTimer(mLocaleEnrageTimerId);
        }
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mLocaleEnrageTimerId = _addTimer(360000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        _removeTimer(mLocaleEnrageTimerId);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        CreatureAIScript* mGrandWarlockAlythess = getNearestCreatureAI(CN_GRAND_WARLOCK_ALYTHESS);
        if (mGrandWarlockAlythess != nullptr && mGrandWarlockAlythess->isAlive())
        {
            mGrandWarlockAlythess->sendChatMessage(CHAT_MSG_MONSTER_YELL, 12492, "Sacrolash!");
        }
    }

    CreatureAISpells* mLocaleEnrageSpell;
    uint32_t mLocaleEnrageTimerId;
};

class GrandWarlockAlythessAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GrandWarlockAlythessAI);
    explicit GrandWarlockAlythessAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto pyrogenetics = addAISpell(GRAND_WARLOCK_ALYTHESS_PYROGENICS, 100.0f, TARGET_SELF, 0, 10);
        pyrogenetics->setMinMaxDistance(0.0f, 50.0f);

        auto flameTouched = addAISpell(GRAND_WARLOCK_ALYTHESS_FLAME_TOUCHED, 10.0f, TARGET_RANDOM_SINGLE, 0, 30);
        flameTouched->setMinMaxDistance(0.0f, 50.0f);

        auto conflagration = addAISpell(GRAND_WARLOCK_ALYTHESS_CONFLAGRATION, 15.0f, TARGET_RANDOM_SINGLE, 4, 25);
        conflagration->addEmote("Fire to the aid of shadow!", CHAT_MSG_MONSTER_YELL, 12489);
        conflagration->setMinMaxDistance(0.0f, 50.0f);

        auto blaze = addAISpell(GRAND_WARLOCK_ALYTHESS_BLAZE, 30.0f, TARGET_RANDOM_SINGLE, 3, 0);
        blaze->setMinMaxDistance(0.0f, 50.0f);

        auto flameFear = addAISpell(GRAND_WARLOCK_ALYTHESS_FLAME_SEAR, 20.0f, TARGET_RANDOM_SINGLE);
        flameFear->setMinMaxDistance(0.0f, 50.0f);

        mLocaleEnrageSpell = addAISpell(GRAND_WARLOCK_ALYTHESS_ENRAGE, 0.0f, TARGET_SELF);
        mLocaleEnrageSpell->addEmote("Your luck has run its course!", CHAT_MSG_MONSTER_YELL, 12493);
        mLocaleEnrageSpell->setMinMaxDistance(0.0f, 50.0f);

        // Emotes
        addEmoteForEvent(Event_OnTargetDied, 8849);
        addEmoteForEvent(Event_OnDied, 8850); // Wasn't able to find sound for this text

        mLocaleEnrageTimerId = 0;
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mLocaleEnrageTimerId))
        {
            _castAISpell(mLocaleEnrageSpell);
            _removeTimer(mLocaleEnrageTimerId);
        }
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mLocaleEnrageTimerId = _addTimer(360000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        _removeTimer(mLocaleEnrageTimerId);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        CreatureAIScript* mLadySacrolash = getNearestCreatureAI(CN_LADY_SACROLASH);
        if (mLadySacrolash != nullptr && mLadySacrolash->isAlive())
        {
            mLadySacrolash->sendChatMessage(CHAT_MSG_MONSTER_YELL, 12488, "Alythess! Your fire burns within me!");
        }
    }

    CreatureAISpells* mLocaleEnrageSpell;
    uint32_t mLocaleEnrageTimerId;
};

class MuruAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MuruAI);
    explicit MuruAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(MURU_NEGATIVE_ENERGY, 25.0f, TARGET_SELF);
        addAISpell(MURU_DARKNESS, 20.0f, TARGET_SELF, 0, 45);
    }
};

void SetupSunwellPlateau(ScriptMgr* pScriptMgr)
{
    pScriptMgr->register_creature_script(CN_SUNBLADE_PROTECTOR, &SunbladeProtectorAI::Create);
    pScriptMgr->register_creature_script(CN_BRUTALLUS, &BrutallusAI::Create);
    pScriptMgr->register_creature_script(CN_FELMYST, &FelmystAI::Create);
    pScriptMgr->register_creature_script(CN_LADY_SACROLASH, &LadySacrolashAI::Create);
    pScriptMgr->register_creature_script(CN_GRAND_WARLOCK_ALYTHESS, &GrandWarlockAlythessAI::Create);
    pScriptMgr->register_creature_script(CN_MURU, &MuruAI::Create);
}
