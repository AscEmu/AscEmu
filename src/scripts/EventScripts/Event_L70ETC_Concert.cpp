/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
//\details <b>At: Worlds End Tavern</b>\n
// event_properties entry: NA \n
// event_properties holiday: NA \n

//////////////////////////////////////////////////////////////////////////////////////////
//\details <b>L70ETC Concert</b>\n
// event_properties entry: 32 \n
// event_properties holiday: 0 \n
// Original: ArcScript/Moon++ \n

// L80etc band members
enum L80BandMembers
{
    SAMURO = 23625,
    BERGRISST = 23619,
    MAIKYL = 23624,
    SIGNICIOUS = 23626,
    THUNDERSKINS = 23623,
    TARGETGROUND = 48000,
    TARGETAIR = 48001,
    //UNDEAD = 48002,
    //UNDEAD2 = 48003,
    //UNDEAD3 = 48004,
    TRIGGER = 48005
};

// Spells
enum L80Spells
{
    SPELLFLARE = 42505,
    SPELLFIRE = 42501,
    SPOTLIGHT = 39312,
    SPELLEARTH = 42499,
    SPELLLLIGHTNING = 42510,
    SPELLLLIGHTNING2 = 42507,
    SPELLSTORM = 42500,
    CONSECRATION = 26573,
    SINGERSLIGHT = 42510
};

class SamAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SamAI(c); }
    explicit SamAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
        timer = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    { 
        RegisterAIUpdateEvent(1000);
    }

    void OnSpawn()
    {
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void OnDespawn() override
    {
        RemoveAIUpdateEvent();
    }

    void AIUpdate() override
    {
        switch (timer)
        {
            case 1:  getCreature()->PlaySoundToSet(11803); break;
            case 2:  getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 19:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL05); break;
            case 30:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 65:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 70:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 84:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL06); break;
            case 112: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SINGERSLIGHT), true); break;
            case 123: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 137: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL06); break;
            case 142: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 180: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 229: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL02); break;
            case 239: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL06); break;
            case 259: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL06); break;
            case 279: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 280: getCreature()->Despawn(1000, 300000); break;
        }
        timer++;
    }

protected:
    uint32_t timer;
};

class BerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BerAI(c); }
    explicit BerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
        timer = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnSpawn()
    {
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void OnDespawn() override
    {
        RemoveAIUpdateEvent();
    }

    void AIUpdate() override
    {
        switch (timer)
        {
            case 0: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 10:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 30:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 34:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 38:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 104: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 123: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 140: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPOTLIGHT), true); break;
            case 145: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 168: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 229: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL02); break;
            case 230: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL06); break;
            case 279: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 280: getCreature()->Despawn(1000, 300050); break;
        }
        timer++;
    }

protected:
    uint32_t timer;
};

class SigAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SigAI(c); }
    explicit SigAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        timer = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnSpawn()
    {
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void OnDespawn() override
    {
        RemoveAIUpdateEvent();
    }

    void AIUpdate() override
    {
        switch (timer)
        {
            case 0: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 10:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 30:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 34:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 38:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 70:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 85:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 123: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 140: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 165: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPOTLIGHT), true); break;
            case 166: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL02); break;
            case 168: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 180: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 193: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 229: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL06); break;
            case 259: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 279: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 280: getCreature()->Despawn(1000, 300050); break;
        }
        timer++;
    }

protected:
    uint32_t timer;
};

class MaiAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MaiAI(c); }
    explicit MaiAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        timer = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnSpawn()
    {
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void OnDespawn() override
    {
        RemoveAIUpdateEvent();
    }

    void AIUpdate() override
    {
        switch (timer)
        {
            case 0: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 10:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 30:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 45:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 70:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 85:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 95:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 102: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 115: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 123: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL02); break;
            case 165: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPOTLIGHT), true); break;
            case 192: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 203: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 229: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 279: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 280: getCreature()->Despawn(1000, 300050); break;
        }
        timer++;
    }

protected:
    uint32_t timer;
};

class ThuAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ThuAI(c); }
    explicit ThuAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        timer = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnSpawn()
    {
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void OnDespawn() override
    {
        RemoveAIUpdateEvent();
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void AIUpdate() override
    {
        switch (timer)
        {
            case 2: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 3:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 14:  getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "ARE YOU READY TO ROCK?!?!"); break;
            case 17:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 42:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 55:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 62:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 63:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 64:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 75:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 76:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 77:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 88:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 99:  getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 110: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 137: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL02); break;
            case 140: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 142: getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "WERE GANA ROCK YOU CRAAAAAAZY!!!"); break;
            case 313: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 194: getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 200:
            {
                getCreature()->emote(EMOTE_ONESHOT_CUSTOMSPELL01);
                getCreature()->setEmoteState(401);
                break;
            }
            case 279: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 281: getCreature()->Despawn(1000, 301000); break;
        }
        timer++;
    }

protected:
    uint32_t timer;
};

class UndeadAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UndeadAI(c); }
    explicit UndeadAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        timer = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnSpawn()
    {
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void OnDespawn() override
    {
        RemoveAIUpdateEvent();
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void AIUpdate() override
    {
        switch (timer)
        {
            case 2:  getCreature()->setEmoteState(EMOTE_STATE_DANCE); break;
            case 280: getCreature()->emote(EMOTE_ONESHOT_APPLAUD); break;
            case 281: getCreature()->Despawn(1000, 301000); break;
        }
        timer++;
    }

protected:
    uint32_t timer;
};

class Undead2AI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Undead2AI(c); }
    explicit Undead2AI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        timer = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnSpawn()
    {
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void OnDespawn() override
    {
        getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "THAT WAS GREAT!");
        RemoveAIUpdateEvent();
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void AIUpdate() override
    {
        switch (timer)
        {
            case 2:  getCreature()->setEmoteState(EMOTE_STATE_DANCE); break;
            case 280: getCreature()->emote(EMOTE_ONESHOT_CHEER); break;
            case 281: getCreature()->Despawn(1000, 301000); break;
        }
        timer++;
    }

protected:
    uint32_t timer;
};

class Undead3AI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Undead3AI(c); }
    explicit Undead3AI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        timer = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnSpawn()
    {
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void AIUpdate() override
    {
        switch (timer)
        {
            case 2:  getCreature()->setEmoteState(EMOTE_STATE_DANCE); break;
            case 279: getCreature()->emote(EMOTE_ONESHOT_CHEER); break;
            case 280: getCreature()->Despawn(1000, 301000); break;
        }
        timer++;
    }

protected:
    uint32_t timer;
};

class TriggerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TriggerAI(c); }
    explicit TriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        timer = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnSpawn()
    {
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void OnDespawn() override
    {
        getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "THAT WAS GREAT!");
        RemoveAIUpdateEvent();
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void AIUpdate() override
    {
        switch (timer)
        {
            case 1: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 8: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 15: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 21: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 28: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 35: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 41: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 48: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 55: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 62: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 69: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 76: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 81: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 89: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 96: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 101: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 108: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 115: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 121: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 128: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 135: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 141: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 148: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 155: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 162: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 169: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 176: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 181: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 189: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 196: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 201: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 208: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 215: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 221: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 228: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 235: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 241: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 248: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 255: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 262: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 269: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 276: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 281:
            {
                setAIAgent(AGENT_NULL);
                RemoveAIUpdateEvent();
                getCreature()->Despawn(1000, 301000); break;
            }
        }
        timer++;
    }
protected:
    uint32_t timer;
};

class Trigger2AI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Trigger2AI(c); }
    explicit Trigger2AI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        timer = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnSpawn()
    {
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void OnDespawn() override
    {
        getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "THAT WAS GREAT!");
        RemoveAIUpdateEvent();
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }
    void AIUpdate() override
    {
        switch (timer)
        {
            case 3: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 10: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 18: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 24: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 22: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 38: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 44: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 52: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 58: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 68: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 69: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 76: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 85: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 90: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 96: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 107: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 109: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 125: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 127: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 129: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 132: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 144: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 149: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 159: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 166: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 169: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 176: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 183: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 186: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 194: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 204: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 209: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 218: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 223: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 228: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 235: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 241: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 248: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 252: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 263: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 266: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 274: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(CONSECRATION), true); break;
            case 281: getCreature()->Despawn(1000, 301000); break;
        }
        timer++;
    }

protected:
    uint32_t timer;
};

class Effectsground : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Effectsground(c); }
    explicit Effectsground(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        timer = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnSpawn()
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        timer = 0;
    }

    void OnDespawn() override
    {
        RemoveAIUpdateEvent();
    }

    void AIUpdate() override
    {
        switch (timer)
        {
            case 2:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLLLIGHTNING), true);
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLLLIGHTNING2), true);
            }break;
            case 6:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLEARTH), true);
            }break;
            case 8:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFIRE), true);
            }break;
            case 72:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLLLIGHTNING), true);
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLLLIGHTNING2), true);
            }break;
            case 76:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLEARTH), true);
            }break;
            case 78:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFIRE), true);
            }break;
            case 125:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLLLIGHTNING), true);
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLLLIGHTNING2), true);
            }break;
            case 128:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLEARTH), true);
            }break;
            case 132:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFIRE), true);
            }break;
            case 232:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLLLIGHTNING), true);
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLLLIGHTNING2), true);
            }break;
            case 236:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLEARTH), true);
            }break;
            case 238:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFIRE), true);
            }break;
            case 245:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLLLIGHTNING), true);
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLLLIGHTNING2), true);
            }break;
            case 249:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLEARTH), true);
            }break;
            case 251:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFIRE), true);
            }break;
            case 279:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
            }break;
            case 280:
            {
                getCreature()->Despawn(1000, 300000);
            }break;

        }
        timer++;
    }

protected:
    uint32_t timer;
};

class Effectsair : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Effectsair(c); }
    explicit Effectsair(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        timer = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
    }

    void OnSpawn()
    {
        timer = 0;
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    }

    void OnDespawn() override
    {
        RemoveAIUpdateEvent();
    }

    void AIUpdate() override
    {
        switch (timer)
        {
            case 1:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLSTORM), true);
            }break;
            case 70:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLSTORM), true);
            }break;
            case 123:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLSTORM), true);
            }break;
            case 230:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLSTORM), true);
            }break;
            case 243:
            {
                getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLSTORM), true);
            }break;
            case 280:
            {
                getCreature()->Despawn(1000, 300000);
            }break;
        }
        timer++;
    }

protected:
    uint32_t timer;
};

void SetupL70ETC(ScriptMgr* mgr)
{
    mgr->register_creature_script(SAMURO, &SamAI::Create);
    mgr->register_creature_script(BERGRISST, &BerAI::Create);
    mgr->register_creature_script(MAIKYL, &MaiAI::Create);
    mgr->register_creature_script(SIGNICIOUS, &SigAI::Create);
    mgr->register_creature_script(THUNDERSKINS, &ThuAI::Create);
    mgr->register_creature_script(TARGETGROUND, Effectsground::Create);
    mgr->register_creature_script(TARGETAIR, Effectsair::Create);
    // mgr->register_creature_script(UNDEAD, &UndeadAI::Create);
    // mgr->register_creature_script(UNDEAD2, &Undead2AI::Create);
    // mgr->register_creature_script(UNDEAD3, &Undead3AI::Create);
    mgr->register_creature_script(TRIGGER, &TriggerAI::Create);
}
