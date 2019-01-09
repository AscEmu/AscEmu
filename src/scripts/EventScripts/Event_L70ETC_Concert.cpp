/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

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
    ADD_CREATURE_FACTORY_FUNCTION(SamAI);
    explicit SamAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->GetAIInterface()->m_canMove = false;
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
            case 19:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL05); break;
            case 30:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 65:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 70:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 84:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL06); break;
            case 112: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SINGERSLIGHT), true); break;
            case 123: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 137: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL06); break;
            case 142: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 180: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 229: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL02); break;
            case 239: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL06); break;
            case 259: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL06); break;
            case 279: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 280: getCreature()->Despawn(1000, 300000); break;
        }
        timer++;
    }

protected:

    uint32 timer;
};

class BerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BerAI);
    explicit BerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->GetAIInterface()->m_canMove = false;
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
            case 10:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 30:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 34:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 38:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 104: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 123: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 140: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPOTLIGHT), true); break;
            case 145: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 168: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 229: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL02); break;
            case 230: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL06); break;
            case 279: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 280: getCreature()->Despawn(1000, 300050); break;
        }
        timer++;
    }

protected:
    uint32 timer;

};

class SigAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SigAI);
    explicit SigAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->GetAIInterface()->m_canMove = false;
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
            case 10:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 30:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 34:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 38:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 70:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 85:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 123: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 140: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 165: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPOTLIGHT), true); break;
            case 166: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL02); break;
            case 168: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 180: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 193: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 229: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL06); break;
            case 259: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 279: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 280: getCreature()->Despawn(1000, 300050); break;
        }
        timer++;
    }

protected:

    uint32 timer;
};

class MaiAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MaiAI);
    explicit MaiAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->GetAIInterface()->m_canMove = false;
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
            case 10:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 30:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 45:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 70:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 85:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 95:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 102: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 115: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01); break;
            case 123: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL02); break;
            case 165: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPOTLIGHT), true); break;
            case 192: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 203: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 229: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 279: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 280: getCreature()->Despawn(1000, 300050); break;
        }
        timer++;
    }

protected:

    uint32 timer;
};


class ThuAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ThuAI);
    explicit ThuAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->GetAIInterface()->m_canMove = false;
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
            case 3:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 14:  getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "ARE YOU READY TO ROCK?!?!"); break;
            case 17:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 42:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 55:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL03); break;
            case 62:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 63:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 64:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 75:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 76:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 77:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 88:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 99:  getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 110: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 137: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL02); break;
            case 140: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 142: getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "WERE GANA ROCK YOU CRAAAAAAZY!!!"); break;
            case 313: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 194: getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL04); break;
            case 200:
            {
                getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL01);
                getCreature()->setEmoteState(401);
                break;
            }
            case 279: getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true); break;
            case 281: getCreature()->Despawn(1000, 301000); break;
        }
        timer++;
    }

protected:

    uint32 timer;
};

class UndeadAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(UndeadAI);
    explicit UndeadAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->GetAIInterface()->m_canMove = false;
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
            case 280: getCreature()->Emote(EMOTE_ONESHOT_APPLAUD); break;
            case 281: getCreature()->Despawn(1000, 301000); break;
        }
        timer++;
    }

protected:

    uint32 timer;
};

class Undead2AI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Undead2AI);
    explicit Undead2AI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->GetAIInterface()->m_canMove = false;
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
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "THAT WAS GREAT!");
        RemoveAIUpdateEvent();
        getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(SPELLFLARE), true);
    }

    void AIUpdate() override
    {
        switch (timer)
        {
            case 2:  getCreature()->setEmoteState(EMOTE_STATE_DANCE); break;
            case 280: getCreature()->Emote(EMOTE_ONESHOT_CHEER); break;
            case 281: getCreature()->Despawn(1000, 301000); break;
        }
        timer++;
    }

protected:

    uint32 timer;
};

class Undead3AI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Undead3AI);
    explicit Undead3AI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->GetAIInterface()->m_canMove = false;
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
            case 279: getCreature()->Emote(EMOTE_ONESHOT_CHEER); break;
            case 280: getCreature()->Despawn(1000, 301000); break;
        }
        timer++;
    }

protected:

    uint32 timer;
};

class TriggerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TriggerAI);
    explicit TriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->GetAIInterface()->m_canMove = false;
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
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "THAT WAS GREAT!");
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
                getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
                RemoveAIUpdateEvent();
                getCreature()->Despawn(1000, 301000); break;
            }
        }
        timer++;
    }
protected:

    uint32 timer;
};

class Trigger2AI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Trigger2AI);
    explicit Trigger2AI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->GetAIInterface()->m_canMove = false;
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
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "THAT WAS GREAT!");
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

    uint32 timer;
};

class Effectsground : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Effectsground);
    explicit Effectsground(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->GetAIInterface()->m_canMove = false;
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

    uint32 timer;
};

class Effectsair : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Effectsair);
    explicit Effectsair(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->GetAIInterface()->m_canMove = false;
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

    uint32 timer;
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
