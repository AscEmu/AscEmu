/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
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

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Raid_Naxxramas.h"
#include "Spell/SpellMgr.h"

/////////////////////////////////////////////////////////////////////////////////
////// Naxxramas Instance

const uint32 CN_THADDIUS = 15928;

class NaxxramasScript : public InstanceScript
{
    public:

        NaxxramasScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr) {}

        static InstanceScript* Create(MapMgr* pMapMgr) { return new NaxxramasScript(pMapMgr); }

        void OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/) override
        {
            //Creature* KelThuzad = NULL;
            switch (pVictim->GetEntry())
            {
                case 16998: // Kel thuzads cat
                    {
                        /* getCreatureBySpawnId not entry!
                        KelThuzad = getCreatureBySpawnId(CN_KELTHUZAD);
                        if (KelThuzad && KelThuzad->isAlive())
                            KelThuzad->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "No!!! A curse upon you, interlopers! The armies of the Lich King will hunt you down. You will not escape your fate...");
                        KelThuzad = NULL;*/
                    }
                    break;
                case CN_PATCHWERK:
                    setGameObjectStateForEntry(181123, GO_STATE_OPEN);
                    break;
                case CN_GLUTH:
                    {
                    setGameObjectStateForEntry(181120, GO_STATE_OPEN);
                    setGameObjectStateForEntry(181121, GO_STATE_OPEN);
                    }
                    break;
                case CN_ANUBREKHAN:
                    {
                    setGameObjectStateForEntry(181195, GO_STATE_OPEN);
                    setGameObjectStateForEntry(194022, GO_STATE_OPEN);
                    }
                    break;
                case CN_GRAND_WIDOW_FAERLINA:
                    setGameObjectStateForEntry(181209, GO_STATE_OPEN);
                    break;
            }
        }

};

/////////////////////////////////////////////////////////////////////////////////
////// The Arachnid Quarter

/////////////////////////////////////////////////////////////////////////////////
////// Carrion Spinner
CarrionSpinnerAI::CarrionSpinnerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(CARRION_SPINNER_POISON_BOLT_HEROIC, Target_Self, 15, 0, 15);
    else
        AddSpell(CARRION_SPINNER_POISON_BOLT_NORMAL, Target_Self, 15, 0, 15);

    // Does it work ?
    AddSpell(CARRION_SPINNER_WEB_WRAP, Target_RandomPlayer, 8, 0, 10, 0, 40);
}

/////////////////////////////////////////////////////////////////////////////////
////// Dread Creeper
DreadCreeperAI::DreadCreeperAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(DREAD_CREEPER_VEIL_OF_SHADOW_HEROIC, Target_Self, 15, 0, 10);
    else
        AddSpell(DREAD_CREEPER_VEIL_OF_SHADOW_NORMAL, Target_Self, 15, 0, 10);
}

/////////////////////////////////////////////////////////////////////////////////
////// Naxxramas Cultist
NaxxramasCultistAI::NaxxramasCultistAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(NAXXRAMAS_CULTIST_KNOCKBACK_HEROIC, Target_Destination, 10, 0, 10, 0, 8);
    else
        AddSpell(NAXXRAMAS_CULTIST_KNOCKBACK_NORMAL, Target_Destination, 10, 0, 10, 0, 8);
}

//Necro Stalker AI
/////////////////////////////////////////////////////////////////////////////////
////// Venom Stalker
VenomStalkerAI::VenomStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Do those really work ?
    if (_isHeroic())
        AddSpell(VENOM_STALKER_POISON_CHARGE_HEROIC, Target_RandomPlayer, 10, 0, 10, 0, 40);
    else
        AddSpell(VENOM_STALKER_POISON_CHARGE_NORMAL, Target_RandomPlayer, 10, 0, 10, 0, 40);
}

/////////////////////////////////////////////////////////////////////////////////
////// Tomb Horror
TombHorrorAI::TombHorrorAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
    {
        AddSpell(TOMB_HORROR_CRYPT_SCARAB_SWARM_HEROIC, Target_Self, 7, 3, 20);
        AddSpell(TOMB_HORROR_CRYPT_SCARABS_HEROIC, Target_RandomPlayer, 8, 2, 10, 0, 40);
    }
    else
    {
        AddSpell(TOMB_HORROR_CRYPT_SCARAB_SWARM_HEROIC, Target_Self, 7, 3, 20);
        AddSpell(TOMB_HORROR_CRYPT_SCARABS_NORMAL, Target_RandomPlayer, 10, 1.5, 10, 0, 40);
    }

    AddSpell(TOMB_HORROR_SPIKE_VOLLEY, Target_Self, 10, 0.5f, 15);
}

/////////////////////////////////////////////////////////////////////////////////
////// Naxxramas Acolyte
NaxxramasAcolyteAI::NaxxramasAcolyteAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
    {
        AddSpell(NAXXRAMAS_ACOLYTE_SHADOW_BOLT_VOLLEY_HEROIC, Target_RandomPlayerDestination, 10, 3, 5, 0, 30);
        AddSpell(NAXXRAMAS_ACOLYTE_ARCANE_EXPLOSION_HEROIC, Target_Self, 10, 2, 15);
    }
    else
    {
        AddSpell(NAXXRAMAS_ACOLYTE_SHADOW_BOLT_VOLLEY_NORMAL, Target_RandomPlayerDestination, 10, 3, 5, 0, 30);
        AddSpell(NAXXRAMAS_ACOLYTE_ARCANE_EXPLOSION_NORMAL, Target_Self, 10, 2, 15);
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// Vigilant Shade
VigilantShadeAI::VigilantShadeAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Does it really work ?
    if (_isHeroic())
        AddSpell(NAXXRAMAS_ACOLYTE_SHADOW_BOLT_VOLLEY_HEROIC, Target_RandomPlayerDestination, 10, 0, 15, 0, 30);
    else
        AddSpell(NAXXRAMAS_ACOLYTE_SHADOW_BOLT_VOLLEY_NORMAL, Target_RandomPlayerDestination, 10, 0, 15, 0, 30);

    _applyAura(VIGILANT_SHADE_INVISIBILITY);
}

void VigilantShadeAI::OnCombatStart(Unit* /*pTarget*/)
{
    _removeAura(VIGILANT_SHADE_INVISIBILITY);
}

void VigilantShadeAI::OnCombatStop(Unit* /*pTarget*/)
{
    _applyAura(VIGILANT_SHADE_INVISIBILITY);
}

/////////////////////////////////////////////////////////////////////////////////
////// Crypt Reaver
CryptReaverAI::CryptReaverAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddSpell(CRYPT_REAVER_CLEAVE, Target_Current, 10, 0, 5, 0, 8);
    AddSpell(CRYPT_REAVER_FRENZY, Target_Self, 7, 0, 40);
}

/////////////////////////////////////////////////////////////////////////////////
////// Web Wrap
WebWrapAI::WebWrapAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mPlayerGuid = 0;
}

void WebWrapAI::OnCombatStart(Unit* /*pTarget*/)
{
    _setMeleeDisabled(false);
    setRooted(true);
    stopMovement();
}

void WebWrapAI::OnCombatStop(Unit* /*pTarget*/)
{
    _setMeleeDisabled(true);
    setRooted(false);
}

void WebWrapAI::OnDied(Unit* /*pKiller*/)
{
    // Slower, but safer
    if (mPlayerGuid != 0)
    {
        Player* PlayerPtr = objmgr.GetPlayer(static_cast<uint32>(mPlayerGuid));
        if (PlayerPtr != NULL && PlayerPtr->HasAura(MAEXXNA_WEB_WRAP))
        {
            PlayerPtr->RemoveAura(MAEXXNA_WEB_WRAP);
            PlayerPtr->setMoveRoot(false);
        }

        mPlayerGuid = 0;
    }
}

void WebWrapAI::AIUpdate()
{
    if (mPlayerGuid != 0)
    {
        Player* PlayerPtr = objmgr.GetPlayer(static_cast<uint32>(mPlayerGuid));
        if (PlayerPtr == NULL || !PlayerPtr->isAlive() || !PlayerPtr->HasAura(MAEXXNA_WEB_WRAP))
        {
            mPlayerGuid = 0;
            RemoveAIUpdateEvent();
            despawn(1);
        }
    }
}

void WebWrapAI::Destroy()
{
    if (mPlayerGuid != 0)
    {
        Player* PlayerPtr = objmgr.GetPlayer(static_cast<uint32>(mPlayerGuid));
        if (PlayerPtr != NULL && PlayerPtr->HasAura(MAEXXNA_WEB_WRAP))
        {
            PlayerPtr->RemoveAura(MAEXXNA_WEB_WRAP);
            PlayerPtr->setMoveRoot(false);
        }

        mPlayerGuid = 0;
    }

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Maexxna Spiderling
MaexxnaSpiderlingAI::MaexxnaSpiderlingAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(MAEXXNA_SPIDERLING_NECROTIC_POISON_HEROIC, Target_Current, 10, 0, 20, 0, 8);
    else
        AddSpell(MAEXXNA_SPIDERLING_NECROTIC_POISON_NORMAL, Target_Current, 10, 0, 20, 0, 8);
}

/////////////////////////////////////////////////////////////////////////////////
////// Maexxna
MaexxnaAI::MaexxnaAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
    {
        AddSpell(MAEXXNA_POISON_SHOCK_HEROIC, Target_Self, 100, 0, 10);        // Target_Current with range ?
        AddSpell(MAEXXNA_NECROTIC_POISON_HEROIC, Target_Current, 10, 0, 15, 0, 8);
    }
    else
    {
        AddSpell(MAEXXNA_POISON_SHOCK_NORMAL, Target_Self, 100, 0, 10);        // Target_Current with range ?
        AddSpell(MAEXXNA_NECROTIC_POISON_NORMAL, Target_Current, 10, 0, 15, 0, 8);
    }

    mWebWrapProc = AddSpellFunc(&SpellFunc_MaexxnaWebWrap, Target_RandomPlayerNotCurrent, 0, 0, 0);
    mAddsSummonTimer = mWebWrapTimer = mWebSprayTimer = INVALIDATE_TIMER;
    mHasEnraged = false;
    mLeftWall = true;
}

void MaexxnaAI::OnCombatStart(Unit* /*pTarget*/)
{
    mAddsSummonTimer = _addTimer(30000);
    mWebSprayTimer = _addTimer(40000);
    mWebWrapTimer = _addTimer(20000);
    mHasEnraged = false;
    if (RandomUInt(1) == 1)
        mLeftWall = !mLeftWall;
}

void MaexxnaAI::OnCombatStop(Unit* /*pTarget*/)
{
    _removeTimer(mWebWrapTimer);
}

void MaexxnaAI::AIUpdate()
{
    if (_isTimerFinished(mAddsSummonTimer))
    {
        for (uint8 i = 0; i < 8; ++i)
        {
            CreatureAIScript* Spiderling = spawnCreatureAndGetAIScript(CN_MAEXXNA_SPIDERLING, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
            if (Spiderling != nullptr)
            {
                Spiderling->getCreature()->m_noRespawn = true;
                Spiderling->_setDespawnWhenInactive(true);
                static_cast<CreatureAIScript*>(Spiderling)->AggroRandomPlayer(1000);
            }
        }

        _resetTimer(mAddsSummonTimer, 40000);
    }

    if (!_isCasting())
    {
        if (!mHasEnraged && _getHealthPercent() <= 30)
        {
            if (_isHeroic())
                _applyAura(MAEXXNA_FRENZY_HEROIC);
            else
                _applyAura(MAEXXNA_FRENZY_NORMAL);

            mHasEnraged = true;
        }
        else if (_isTimerFinished(mWebSprayTimer))
        {
            if (_isHeroic())
                _applyAura(MAEXXNA_WEB_SPRAY_HEROIC);
            else
                _applyAura(MAEXXNA_WEB_SPRAY_NORMAL);

            _resetTimer(mWebSprayTimer, 40000);
        }
        else if (_isTimerFinished(mWebWrapTimer))
        {
            if (_isHeroic())
                CastSpellNowNoScheduling(mWebWrapProc);

            CastSpellNowNoScheduling(mWebWrapProc);
            _resetTimer(mWebWrapTimer, 40000);
        }
    }
}

void SpellFunc_MaexxnaWebWrap(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType /*pType*/)
{
    MaexxnaAI* Maexxna = (pCreatureAI != NULL) ? static_cast< MaexxnaAI* >(pCreatureAI) : NULL;
    if (Maexxna != NULL)
    {
        // Is target really added everytime and isn't this check redundant ?
        if (pTarget == NULL || !pTarget->IsPlayer() || pTarget->HasAura(MAEXXNA_WEB_WRAP) || Maexxna->getCreature() == NULL || Maexxna->getCreature()->GetMapMgr() == NULL)
            return;

        uint32 Id = RandomUInt(1);
        if (!Maexxna->mLeftWall)
            Id += 3;

        WebWrapAI* WebWrap = static_cast< WebWrapAI* >(Maexxna->spawnCreatureAndGetAIScript(CN_WEB_WRAP, WebWrapPos[Id].x, WebWrapPos[Id].y, WebWrapPos[Id].z, pTarget->GetOrientation()));
        if (WebWrap == NULL)
            return;

        WebWrap->getCreature()->m_noRespawn = true;
        WebWrap->RegisterAIUpdateEvent(5000);
        WebWrap->mPlayerGuid = static_cast<Player*>(pTarget)->GetGUID();

        if (pTarget->isCastingNonMeleeSpell())
            pTarget->interruptSpell();

        // Somewhy root does not apply at all
        static_cast<Player*>(pTarget)->SafeTeleport(Maexxna->getCreature()->GetMapId(), Maexxna->getCreature()->GetInstanceID(), LocationVector(WebWrapPos[Id].x, WebWrapPos[Id].y, WebWrapPos[Id].z));
        pTarget->CastSpell(pTarget, MAEXXNA_WEB_WRAP, true);
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// Naxxramas Worshipper
NaxxramasWorshipperAI::NaxxramasWorshipperAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(NAXXRAMAS_WORSHIPPER_FIREBALL_HEROIC, Target_Current, 10, 2.5, 0, 0, 45);
    else
        AddSpell(NAXXRAMAS_WORSHIPPER_FIREBALL_NORMAL, Target_Current, 10, 2.5, 0, 0, 45);

    mGrandWidow = NULL;
    mPossessed = false;
}

void NaxxramasWorshipperAI::OnCastSpell(uint32 /*pSpellId*/)
{
}

void NaxxramasWorshipperAI::OnDied(Unit* /*pKiller*/)
{
    if (mGrandWidow != NULL)   //&& !IsHeroic())
    {
        if (getRangeToObject(mGrandWidow->getCreature()) <= 15.0f)
        {
            for (std::set< NaxxramasWorshipperAI* >::iterator Iter = mGrandWidow->mWorshippers.begin(); Iter != mGrandWidow->mWorshippers.end(); ++Iter)
            {
                if (!(*Iter)->isAlive())
                    continue;

                // Must check if it does not crash if creature is already casting
                (*Iter)->_applyAura(NAXXRAMAS_WORSHIPPER_MIND_EXHAUSTION);
            }

            // Should be applied on Grand Widow, but is on the enemies - to script ?
            //ApplyAura(NAXXRAMAS_WORSHIPPER_WIDOW_EMBRACE);
            // I don't like the way we apply it
            Aura* WidowEmbrace = sSpellFactoryMgr.NewAura(sSpellCustomizations.GetSpellInfo(NAXXRAMAS_WORSHIPPER_WIDOW_EMBRACE), 30000, getCreature(), mGrandWidow->getCreature());
            getCreature()->AddAura(WidowEmbrace);

            // Not sure about new Frenzy Timer
            mGrandWidow->_resetTimer(mGrandWidow->mFrenzyTimer, 60000 + RandomUInt(20) * 1000);
            if (mGrandWidow->getCreature()->HasAura(GRAND_WIDOW_FAERLINA_FRENZY_NORMAL))
                mGrandWidow->getCreature()->RemoveAura(GRAND_WIDOW_FAERLINA_FRENZY_NORMAL);    // Really needed ?
            else if (mGrandWidow->getCreature()->HasAura(GRAND_WIDOW_FAERLINA_FRENZY_HEROIC))
                mGrandWidow->getCreature()->RemoveAura(GRAND_WIDOW_FAERLINA_FRENZY_HEROIC);    // Really needed ?
            else
            {
                mGrandWidow->_resetTimer(mGrandWidow->mPoisonVolleyBoltTimer, 30000);
                return;
            }

            mGrandWidow->_resetTimer(mGrandWidow->mPoisonVolleyBoltTimer, 60000);
        }
    }
}

void NaxxramasWorshipperAI::AIUpdate()
{
}

void NaxxramasWorshipperAI::Destroy()
{
    if (mGrandWidow != NULL)
    {
        std::set< NaxxramasWorshipperAI* >::iterator Iter = mGrandWidow->mWorshippers.find(this);
        if (Iter != mGrandWidow->mWorshippers.end())
            mGrandWidow->mWorshippers.erase(Iter);

        mGrandWidow = NULL;
    }

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Naxxramas Follower
NaxxramasFollowerAI::NaxxramasFollowerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mCharge = AddSpell(NAXXRAMAS_FOLLOWER_BERSERKER_CHARGE_HEROIC, Target_Current, 0, 0, 0);
    AddSpellFunc(&SpellFunc_NaxxramasFollowerCharge, Target_RandomPlayer, 8, 0, 20, 0, 40);
    AddSpell(NAXXRAMAS_FOLLOWER_SILENCE_HEROIC, Target_Self, 10, 0, 15);

    mGrandWidow = NULL;
}

void NaxxramasFollowerAI::Destroy()
{
    if (mGrandWidow != NULL)
    {
        std::set< NaxxramasFollowerAI* >::iterator Iter = mGrandWidow->mFollowers.find(this);
        if (Iter != mGrandWidow->mFollowers.end())
            mGrandWidow->mFollowers.erase(Iter);

        mGrandWidow = NULL;
    }

    delete this;
}

void SpellFunc_NaxxramasFollowerCharge(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType /*pType*/)
{
    NaxxramasFollowerAI* NaxxramasFollower = (pCreatureAI != NULL) ? static_cast< NaxxramasFollowerAI* >(pCreatureAI) : NULL;
    if (NaxxramasFollower != NULL)
    {
        Unit* CurrentTarget = NaxxramasFollower->getCreature()->GetAIInterface()->getNextTarget();
        if (CurrentTarget != NULL && CurrentTarget != pTarget)
        {
            NaxxramasFollower->getCreature()->GetAIInterface()->AttackReaction(pTarget, 500);
            NaxxramasFollower->getCreature()->GetAIInterface()->setNextTarget(pTarget);
            //NaxxramasFollower->GetUnit()->GetAIInterface()->RemoveThreatByPtr(CurrentTarget);
        }

        NaxxramasFollower->CastSpell(NaxxramasFollower->mCharge);
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// Grand Widow Faerlina
GrandWidowFaerlinaAI::GrandWidowFaerlinaAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    for (uint8 i = 0; i < 4; ++i)
    {
        CreatureAIScript* whorshipper = spawnCreatureAndGetAIScript(CN_NAXXRAMAS_WORSHIPPER, 3353.364502f + Worshippers[i].x, -3620.322998f, 260.996857f, 4.725017f);
        if (whorshipper != nullptr)
        {
            whorshipper->getCreature()->m_noRespawn = true;
            static_cast< NaxxramasWorshipperAI* >(whorshipper)->mGrandWidow = this;
            mWorshippers.insert(static_cast< NaxxramasWorshipperAI* >(whorshipper));
        }
    }

    if (_isHeroic())
    {
        for (uint8 i = 0; i < 2; ++i)
        {
            CreatureAIScript* follower = spawnCreatureAndGetAIScript(CN_NAXXRAMAS_FOLLOWER, 3353.364502f + Followers[i].x, -3620.322998f, 260.996857f, 4.725017f);
            if (follower != nullptr)
            {
                follower->getCreature()->m_noRespawn = true;
                static_cast< NaxxramasFollowerAI* >(follower)->mGrandWidow = this;
                mFollowers.insert(static_cast< NaxxramasFollowerAI* >(follower));
            }
        }

        mPoisonVolleyBolt = AddSpell(GRAND_WIDOW_FAERLINA_POISON_VOLLEY_BOLT_HEROIC, Target_Self, 0, 0, 0);
        mFrenzy = AddSpell(GRAND_WIDOW_FAERLINA_FRENZY_HEROIC, Target_Self, 0, 0, 0);
        AddSpell(GRAND_WIDOW_RAIN_OF_FIRE_HEROIC, Target_RandomPlayerDestination, 7, 0, 10, 0, 40);
    }
    else
    {
        mPoisonVolleyBolt = AddSpell(GRAND_WIDOW_FAERLINA_POISON_VOLLEY_BOLT_NORMAL, Target_Self, 0, 0, 0);
        mFrenzy = AddSpell(GRAND_WIDOW_FAERLINA_FRENZY_NORMAL, Target_Self, 0, 0, 0);
        AddSpell(GRAND_WIDOW_RAIN_OF_FIRE_NORMAL, Target_RandomPlayerDestination, 7, 0, 10, 0, 40);
    }

    sendDBChatMessage(8914);
    addEmoteForEvent(Event_OnCombatStart, 8915);
    addEmoteForEvent(Event_OnTargetDied, 8916);
    addEmoteForEvent(Event_OnTargetDied, 8917);
    addEmoteForEvent(Event_OnDied, 8918);
    mFrenzy->addEmote("You cannot hide from me!", CHAT_MSG_MONSTER_YELL, 8795);
    mFrenzy->addEmote("Kneel before me, worm!", CHAT_MSG_MONSTER_YELL, 8796);
    mFrenzy->addEmote("Run while you still can!", CHAT_MSG_MONSTER_YELL, 8797);
    mPoisonVolleyBoltTimer = mFrenzyTimer = INVALIDATE_TIMER;
}

void GrandWidowFaerlinaAI::OnCombatStart(Unit* /*pTarget*/)
{
    mPoisonVolleyBoltTimer = _addTimer(15000);
    mFrenzyTimer = _addTimer(60000 + RandomUInt(20) * 1000);

    GameObject* WebGate = getNearestGameObject(3318.65f, -3695.85f, 259.094f, 181235);
    if (WebGate != NULL)
        WebGate->SetState(GO_STATE_CLOSED);

    for (std::set< NaxxramasWorshipperAI* >::iterator Iter = mWorshippers.begin(); Iter != mWorshippers.end(); ++Iter)
    {
        (*Iter)->AggroNearestPlayer(200);
    }

    for (std::set< NaxxramasFollowerAI* >::iterator Iter = mFollowers.begin(); Iter != mFollowers.end(); ++Iter)
    {
        (*Iter)->AggroNearestPlayer(200);
    }
}

void GrandWidowFaerlinaAI::OnCombatStop(Unit* /*pTarget*/)
{
    mPoisonVolleyBoltTimer = mFrenzyTimer = INVALIDATE_TIMER;

    GameObject* WebGate = getNearestGameObject(3318.65f, -3695.85f, 259.094f, 181235);
    if (WebGate != NULL)
        WebGate->SetState(GO_STATE_OPEN);

    for (std::set< NaxxramasWorshipperAI* >::iterator Iter = mWorshippers.begin(); Iter != mWorshippers.end(); ++Iter)
    {
        (*Iter)->mGrandWidow = NULL;
        if (isAlive())
            (*Iter)->despawn();
    }

    mWorshippers.clear();

    for (std::set< NaxxramasFollowerAI* >::iterator Iter = mFollowers.begin(); Iter != mFollowers.end(); ++Iter)
    {
        (*Iter)->mGrandWidow = NULL;
        if (isAlive())
            (*Iter)->despawn();
    }

    mFollowers.clear();

    if (isAlive())
    {
        for (uint8 i = 0; i < 4; ++i)
        {
            CreatureAIScript* whorshipper = spawnCreatureAndGetAIScript(CN_NAXXRAMAS_WORSHIPPER, 3353.364502f + Worshippers[i].x, -3620.322998f, 260.996857f, 4.725017f);
            if (whorshipper != nullptr)
            {
                whorshipper->getCreature()->m_noRespawn = true;
                static_cast< NaxxramasWorshipperAI* >(whorshipper)->mGrandWidow = this;
                mWorshippers.insert(static_cast< NaxxramasWorshipperAI* >(whorshipper));
            }
        }

        if (_isHeroic())
        {
            for (uint8 i = 0; i < 2; ++i)
            {
                CreatureAIScript* follower = spawnCreatureAndGetAIScript(CN_NAXXRAMAS_FOLLOWER, 3353.364502f + Followers[i].x, -3620.322998f, 260.996857f, 4.725017f);
                if (follower != nullptr)
                {
                    follower->getCreature()->m_noRespawn = true;
                    static_cast< NaxxramasFollowerAI* >(follower)->mGrandWidow = this;
                    mFollowers.insert(static_cast< NaxxramasFollowerAI* >(follower));
                }
            }
        }
    }
}

void GrandWidowFaerlinaAI::AIUpdate()
{
    if (!_isCasting())
    {
        if (_isTimerFinished(mPoisonVolleyBoltTimer))
        {
            CastSpellNowNoScheduling(mPoisonVolleyBolt);
            _resetTimer(mPoisonVolleyBoltTimer, 15000);
        }
        else if (_isTimerFinished(mFrenzyTimer))
        {
            CastSpellNowNoScheduling(mFrenzy);
            _resetTimer(mFrenzyTimer, 60000 + RandomUInt(20) * 1000);
        }
    }
}

void GrandWidowFaerlinaAI::Destroy()
{
    for (std::set< NaxxramasWorshipperAI* >::iterator Iter = mWorshippers.begin(); Iter != mWorshippers.end(); ++Iter)
    {
        (*Iter)->mGrandWidow = NULL;
        (*Iter)->despawn();
    }

    mWorshippers.clear();

    for (std::set< NaxxramasFollowerAI* >::iterator Iter = mFollowers.begin(); Iter != mFollowers.end(); ++Iter)
    {
        (*Iter)->mGrandWidow = NULL;
        (*Iter)->despawn();
    }

    mFollowers.clear();

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Crypt Guard
CryptGuardAI::CryptGuardAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(CRYPT_GUARD_ACID_SPLIT_HEROIC, Target_RandomPlayer, 8, 0, 15, 0, 40);
    else
        AddSpell(CRYPT_GUARD_ACID_SPLIT_NORMAL, Target_RandomPlayer, 8, 0, 15, 0, 40);

    AddSpell(CRYPT_GUARD_CLEAVE, Target_Current, 10, 0, 10, 0, 8);
    mAnubRekhanAI = NULL;
    mEnraged = false;
}

void CryptGuardAI::OnCombatStart(Unit* /*pTarget*/)
{
    mEnraged = false;
}

void CryptGuardAI::AIUpdate()
{
    if (!mEnraged && _getHealthPercent() <= 50 && !_isCasting())
    {
        _applyAura(CRYPT_GUARD_FRENZY);
        mEnraged = true;
    }
}

void CryptGuardAI::Destroy()
{
    if (mAnubRekhanAI != NULL)
    {
        std::set< CryptGuardAI* >::iterator Iter = mAnubRekhanAI->mCryptGuards.find(this);
        if (Iter != mAnubRekhanAI->mCryptGuards.end())
            mAnubRekhanAI->mCryptGuards.erase(this);

        mAnubRekhanAI = NULL;
    }

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Corpse Scarab
CorpseScarabAI::CorpseScarabAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mAnubRekhanAI = NULL;
}

void CorpseScarabAI::Destroy()
{
    if (mAnubRekhanAI != NULL)
    {
        std::set< CorpseScarabAI* >::iterator Iter = mAnubRekhanAI->mScarabs.find(this);
        if (Iter != mAnubRekhanAI->mScarabs.end())
            mAnubRekhanAI->mScarabs.erase(this);

        mAnubRekhanAI = NULL;
    }

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Anub'Rekhan
AnubRekhanAI::AnubRekhanAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
    {
        for (uint8 i = 0; i < 2; ++i)
        {
            CryptGuardAI* CryptAI = static_cast< CryptGuardAI* >(spawnCreatureAndGetAIScript(CN_CRYPT_GUARD, CryptGuards[i].x, CryptGuards[i].y, CryptGuards[i].z, CryptGuards[i].o));
            if (CryptAI != nullptr)
            {
                CryptAI->getCreature()->m_noRespawn = true;
                CryptAI->mAnubRekhanAI = this;
                mCryptGuards.insert(CryptAI);
            }
        }

        AddSpell(ANUBREKHAN_IMPALE_HEROIC, Target_RandomPlayerDestination, 7, 1, 10, 0, 45);
        mLocustSwarm = AddSpell(ANUBREKHAN_LOCUST_SWARM_HEROIC, Target_Self, 0, 3, 0);
    }
    else
    {
        AddSpell(ANUBREKHAN_IMPALE_NORMAL, Target_RandomPlayerDestination, 7, 0, 10, 0, 45);
        mLocustSwarm = AddSpell(ANUBREKHAN_LOCUST_SWARM_NORMAL, Target_Self, 0, 3, 0);
    }

    AddSpellFunc(&SpellFunc_AnubRekhanCorpseScarabsPlayer, Target_Self, 8, 0, 20);
    AddSpellFunc(&SpellFunc_AnubRekhanCorpseScarabsCryptGuard, Target_Self, 8, 0, 20);
    sendDBChatMessage(8919);
    addEmoteForEvent(Event_OnCombatStart, 8920);
    addEmoteForEvent(Event_OnCombatStart, 8921);
    addEmoteForEvent(Event_OnCombatStart, 8922);
    addEmoteForEvent(Event_OnTargetDied, 8923);
    addEmoteForEvent(Event_OnTaunt, 8924);
    addEmoteForEvent(Event_OnTaunt, 8925);
    addEmoteForEvent(Event_OnTaunt, 8926);
    addEmoteForEvent(Event_OnTaunt, 8927);

    SetEnrageInfo(AddSpell(ANUBREKHAN_BERSERK, Target_Self, 0, 0, 0), 600000);
    mLocustSwarmTimer = mCryptSpawnTimer = 0;
}

void AnubRekhanAI::OnCombatStart(Unit* /*pTarget*/)
{
    mLocustSwarmTimer = _addTimer(70000 + RandomUInt(50) * 1000);

    if (_isHeroic())
    {
        for (std::set< CryptGuardAI* >::iterator Iter = mCryptGuards.begin(); Iter != mCryptGuards.end(); ++Iter)
        {
            (*Iter)->AggroRandomPlayer(200);
        }
    }
    else
        mCryptSpawnTimer = _addTimer(20000);
}

void AnubRekhanAI::OnCombatStop(Unit* /*pTarget*/)
{
    mLocustSwarmTimer = mCryptSpawnTimer = 0;

    for (std::set< CryptGuardAI* >::iterator Iter = mCryptGuards.begin(); Iter != mCryptGuards.end(); ++Iter)
    {
        (*Iter)->mAnubRekhanAI = NULL;
        if (isAlive())
            (*Iter)->despawn();
    }

    mCryptGuards.clear();

    for (std::set< CorpseScarabAI* >::iterator Iter = mScarabs.begin(); Iter != mScarabs.end(); ++Iter)
    {
        (*Iter)->mAnubRekhanAI = NULL;
        if (isAlive())
            (*Iter)->despawn();
    }

    mScarabs.clear();

    if (_isHeroic() && isAlive())
    {
        for (uint8 i = 0; i < 2; ++i)
        {
            CryptGuardAI* CryptAI = static_cast< CryptGuardAI* >(spawnCreatureAndGetAIScript(CN_CRYPT_GUARD, CryptGuards[i].x, CryptGuards[i].y, CryptGuards[i].z, CryptGuards[i].o));
            if (CryptAI != nullptr)
            {
                CryptAI->getCreature()->m_noRespawn = true;
                CryptAI->mAnubRekhanAI = this;
                mCryptGuards.insert(CryptAI);
            }
        }
    }
}

void AnubRekhanAI::AIUpdate()
{
    if (!_isCasting())
    {
        if (mCryptSpawnTimer != INVALIDATE_TIMER && _isTimerFinished(mCryptSpawnTimer))
        {
            _removeTimer(mCryptSpawnTimer);
            CryptGuardAI* CryptAI = static_cast< CryptGuardAI* >(spawnCreatureAndGetAIScript(CN_CRYPT_GUARD, CryptGuards[2].x, CryptGuards[2].y, CryptGuards[2].z, CryptGuards[2].o));
            if (CryptAI != nullptr)
            {
                CryptAI->getCreature()->m_noRespawn = true;
                CryptAI->mAnubRekhanAI = this;
                mCryptGuards.insert(CryptAI);
                CryptAI->AggroRandomPlayer(200);
            }
        }

        if (_isTimerFinished(mLocustSwarmTimer))
        {
            CryptGuardAI* CryptAI = static_cast< CryptGuardAI* >(spawnCreatureAndGetAIScript(CN_CRYPT_GUARD, CryptGuards[2].x, CryptGuards[2].y, CryptGuards[2].z, CryptGuards[2].o));
            if (CryptAI != nullptr)
            {
                CryptAI->getCreature()->m_noRespawn = true;
                CryptAI->mAnubRekhanAI = this;
                mCryptGuards.insert(CryptAI);
                CryptAI->AggroRandomPlayer(200);
            }

            CastSpellNowNoScheduling(mLocustSwarm);
            mLocustSwarmTimer = _addTimer(70000 + RandomUInt(50) * 1000);
        }
    }
}

void AnubRekhanAI::Destroy()
{
    for (std::set< CryptGuardAI* >::iterator Iter = mCryptGuards.begin(); Iter != mCryptGuards.end(); ++Iter)
    {
        (*Iter)->mAnubRekhanAI = NULL;
        (*Iter)->despawn();
    }

    mCryptGuards.clear();

    for (std::set< CorpseScarabAI* >::iterator Iter = mScarabs.begin(); Iter != mScarabs.end(); ++Iter)
    {
        (*Iter)->mAnubRekhanAI = NULL;
        (*Iter)->despawn();
    }

    mScarabs.clear();

    delete this;
}

void SpellFunc_AnubRekhanCorpseScarabsPlayer(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* /*pTarget*/, TargetType /*pType*/)
{
    AnubRekhanAI* AnubRekhan = (pCreatureAI != NULL) ? static_cast< AnubRekhanAI* >(pCreatureAI) : NULL;
    if (AnubRekhan != NULL)
    {
        std::vector<std::pair< Player* , Movement::Location > > PlayerCorpses;
        Player* PlayerPtr = NULL;
        LocationVector spawnLocation;
        for (std::set< Object* >::iterator Iter = AnubRekhan->getCreature()->GetInRangePlayerSetBegin(); Iter != AnubRekhan->getCreature()->GetInRangePlayerSetEnd(); ++Iter)
        {
            if ((*Iter) == NULL)
                continue;

            PlayerPtr = static_cast< Player* >(*Iter);
            std::set< uint32 >::iterator PlayerIter = AnubRekhan->mUsedCorpseGuids.find(static_cast<uint32>(PlayerPtr->GetGUID()));
            if (PlayerIter != AnubRekhan->mUsedCorpseGuids.end())
            {
                if (PlayerPtr->isAlive())
                    AnubRekhan->mUsedCorpseGuids.erase(PlayerIter);

                continue;
            }

            if (PlayerPtr->isAlive())
                continue;

            if (PlayerPtr->getDeathState() == JUST_DIED)
                spawnLocation = PlayerPtr->GetPosition();
            else if (PlayerPtr->getDeathState() == CORPSE)
            {
                Corpse* myCorpse = objmgr.GetCorpseByOwner(PlayerPtr->GetLowGUID());
                if (myCorpse == NULL || myCorpse->GetCorpseState() != CORPSE_STATE_BODY)
                    continue;

                spawnLocation = PlayerPtr->getMyCorpseLocation();
            }
            else
                continue;

            if (AnubRekhan->getCreature()->CalcDistance(spawnLocation) > 60.0f)
                continue;

            Movement::Location ObjectCoords;
            ObjectCoords.x = spawnLocation.x;
            ObjectCoords.y = spawnLocation.y;
            ObjectCoords.z = spawnLocation.z;
            ObjectCoords.o = spawnLocation.o;
            PlayerCorpses.push_back(std::make_pair(PlayerPtr, ObjectCoords));
        }

        if (PlayerCorpses.size() > 0)
        {
            uint32 Id = RandomUInt(static_cast<uint32>(PlayerCorpses.size() - 1));
            PlayerPtr = PlayerCorpses[Id].first;
            AnubRekhan->mUsedCorpseGuids.insert(static_cast<uint32>(PlayerPtr->GetGUID()));

            for (uint8 i = 0; i < 5; ++i)
            {
                CorpseScarabAI* ScarabAI = static_cast< CorpseScarabAI* >(AnubRekhan->spawnCreatureAndGetAIScript(CN_CORPSE_SCARAB, PlayerCorpses[Id].second.x, PlayerCorpses[Id].second.y, PlayerCorpses[Id].second.z, PlayerCorpses[Id].second.o));
                if (ScarabAI != nullptr)
                {
                    ScarabAI->getCreature()->m_noRespawn = true;
                    ScarabAI->mAnubRekhanAI = AnubRekhan;
                    AnubRekhan->mScarabs.insert(ScarabAI);
                    ScarabAI->AggroRandomPlayer(200);
                }
            }
        }
    }
}

void SpellFunc_AnubRekhanCorpseScarabsCryptGuard(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* /*pTarget*/, TargetType /*pType*/)
{
    AnubRekhanAI* AnubRekhan = (pCreatureAI != NULL) ? static_cast< AnubRekhanAI* >(pCreatureAI) : NULL;
    if (AnubRekhan != NULL)
    {
        std::vector< Creature* > CryptCorpses;
        Creature* CreaturePtr = NULL;
        for (std::set< Object* >::iterator Iter = AnubRekhan->getCreature()->GetInRangeSetBegin(); Iter != AnubRekhan->getCreature()->GetInRangeSetEnd(); ++Iter)
        {
            if ((*Iter) == NULL || !(*Iter)->IsCreature())
                continue;

            CreaturePtr = static_cast<Creature*>(*Iter);
            if (CreaturePtr->GetEntry() != CN_CRYPT_GUARD)
                continue;

            if (CreaturePtr->isAlive() || !CreaturePtr->IsInWorld())
                continue;

            if (AnubRekhan->getCreature()->CalcDistance(CreaturePtr) > 60.0f)
                continue;

            CryptCorpses.push_back(CreaturePtr);
        }

        if (CryptCorpses.size() > 0)
        {
            uint32 Id = RandomUInt(static_cast<uint32>(CryptCorpses.size() - 1));
            CreaturePtr = CryptCorpses[Id];

            float X, Y, Z, O;
            X = CreaturePtr->GetPositionX();
            Y = CreaturePtr->GetPositionY();
            Z = CreaturePtr->GetPositionZ();
            O = CreaturePtr->GetOrientation();

            for (uint8 i = 0; i < 10; ++i)
            {
                CorpseScarabAI* ScarabAI = static_cast< CorpseScarabAI* >(AnubRekhan->spawnCreatureAndGetAIScript(CN_CORPSE_SCARAB, X, Y, Z, O));
                if (ScarabAI != nullptr)
                {
                    ScarabAI->getCreature()->m_noRespawn = true;
                    ScarabAI->mAnubRekhanAI = AnubRekhan;
                    AnubRekhan->mScarabs.insert(ScarabAI);
                    ScarabAI->AggroRandomPlayer(200);
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// The Plague Quarter

/////////////////////////////////////////////////////////////////////////////////
////// Infectious Ghoul
InfectiousGhoulAI::InfectiousGhoulAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddSpell(INFECTIOUS_GHOUL_FLESH_ROT, Target_Current, 10, 0, 15, 0, 8);
    if (_isHeroic())
        AddSpell(INFECTIOUS_GHOUL_REND_HEROIC, Target_Current, 7, 0, 15, 0, 8);
    else
        AddSpell(INFECTIOUS_GHOUL_REND_NORMAL, Target_Current, 7, 0, 15, 0, 8);

    mEnraged = false;
}

void InfectiousGhoulAI::OnCombatStart(Unit* /*pTarget*/)
{
    mEnraged = false;
}

void InfectiousGhoulAI::AIUpdate()
{
    if (!mEnraged && !_isCasting() && _getHealthPercent() <= 50)
    {
        if (_isHeroic())
            _applyAura(INFECTIOUS_GHOUL_FRENZY_HEROIC);
        else
            _applyAura(INFECTIOUS_GHOUL_FRENZY_NORMAL);

        mEnraged = true;
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// Stoneskin Gargoyle
StoneskinGargoyleAI::StoneskinGargoyleAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
    {
        AddSpell(STONESKIN_GARGOYLE_ACID_VOLLEY_HEROIC, Target_Self, 10, 0, 5);
        mStoneskin = AddSpell(STONESKIN_GARGOYLE_STONESKIN_HEROIC, Target_Self, 0, 7, 0);
    }
    else
    {
        AddSpell(STONESKIN_GARGOYLE_ACID_VOLLEY_NORMAL, Target_Self, 10, 0, 5);
        mStoneskin = AddSpell(STONESKIN_GARGOYLE_STONESKIN_NORMAL, Target_Self, 0, 7, 0);
    }
}

bool StoneskinGargoyleAI::HasStoneskin()
{
    return (getCreature()->HasAura(STONESKIN_GARGOYLE_STONESKIN_NORMAL) || getCreature()->HasAura(STONESKIN_GARGOYLE_STONESKIN_HEROIC));
}

void StoneskinGargoyleAI::AIUpdate()
{
    bool HasAura = HasStoneskin();
    if (_isCasting() || HasAura)
        return;
    else if (getCreature()->GetEmoteState() == EMOTE_STATE_SUBMERGED)
        getCreature()->SetEmoteState(EMOTE_ONESHOT_NONE);

    if (!_isCasting() && _getHealthPercent() <= 30)
    {
        CastSpellNowNoScheduling(mStoneskin);
        getCreature()->SetEmoteState(EMOTE_STATE_SUBMERGED_NEW);
        setAIAgent(AGENT_SPELL);
        setRooted(true);
        stopMovement();
        return;
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// Frenzied Bat
FrenziedBatAI::FrenziedBatAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddSpell(FRENZIED_BAT_FRENZIED_DIVE, Target_Self, 10, 0, 15);
}

/////////////////////////////////////////////////////////////////////////////////
////// Plague Beast
PlagueBeastAI::PlagueBeastAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(PLAGUE_BEAST_PLAGUE_SPLASH_HEROIC, Target_RandomPlayerDestination, 8, 0, 15, 0, 50);
    else
        AddSpell(PLAGUE_BEAST_PLAGUE_SPLASH_NORMAL, Target_RandomPlayerDestination, 8, 0, 15, 0, 50);

    AddSpell(PLAGUE_BEAST_TRAMPLE, Target_Current, 10, 0, 10, 0, 8);
    _applyAura(PLAGUE_BEAST_MUTATED_SPORES);
}

void PlagueBeastAI::OnCombatStart(Unit* /*pTarget*/)
{
    _applyAura(PLAGUE_BEAST_MUTATED_SPORES);
}

void PlagueBeastAI::OnCombatStop(Unit* /*pTarget*/)
{
    _applyAura(PLAGUE_BEAST_MUTATED_SPORES);
}

/////////////////////////////////////////////////////////////////////////////////
////// Eye Stalker
EyeStalkerAI::EyeStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(EYE_STALKER_MIND_FLAY_HEROIC, Target_Current, 100, 6, 8, 0, 35);
    else
        AddSpell(EYE_STALKER_MIND_FLAY_NORMAL, Target_Current, 100, 6, 8, 0, 45);
}

void EyeStalkerAI::OnCombatStart(Unit* /*pTarget*/)
{
    setAIAgent(AGENT_SPELL);
    setRooted(true);
    stopMovement();
}

void EyeStalkerAI::AIUpdate()
{
    Unit* CurrentTarget = getCreature()->GetAIInterface()->getNextTarget();
    if (!_isCasting() && CurrentTarget != NULL)
    {
        float MaxRange = 45.0f;
        if (_isHeroic())
            MaxRange = 35.0f;

        if (getRangeToObject(CurrentTarget) > MaxRange)
        {
            Unit* NewTarget = GetBestUnitTarget(TargetFilter_Closest);
            if (NewTarget != NULL && getRangeToObject(NewTarget) <= MaxRange)
            {
                getCreature()->GetAIInterface()->setNextTarget(NewTarget);
                getCreature()->GetAIInterface()->AttackReaction(NewTarget, 200);
            }

            getCreature()->GetAIInterface()->RemoveThreatByPtr(CurrentTarget);
        }
    }

    // Meh, reset it in case something went wrong
    setAIAgent(AGENT_SPELL);
    setRooted(true);
    stopMovement();
}

/////////////////////////////////////////////////////////////////////////////////
////// Noth the Plaguebringer
NothThePlaguebringerAI::NothThePlaguebringerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
    {
        AddPhaseSpell(1, AddSpell(NOTH_THE_PLAGUEBRINGER_CURSE_OF_THE_PLAGUE_HEROIC, Target_Self, 10, 0, 20));
        mCriple = AddSpellFunc(&SpellFunc_NothCriple, Target_Self, 0, 0, 0);
        mBlink = AddSpellFunc(&SpellFunc_NothBlink, Target_RandomPlayer, 0, 0, 0);
    }
    else
    {
        AddPhaseSpell(1, AddSpell(NOTH_THE_PLAGUEBRINGER_CURSE_OF_THE_PLAGUE_NORMAL, Target_Self, 10, 0, 20));
        mCriple = NULL;
        mBlink = NULL;
    }

    mToBalconySwitch = AddSpellFunc(&SpellFunc_NothToBalconyPhaseSwitch, Target_Self, 0, 0, 0);
    mFromBalconySwitch = AddSpellFunc(&SpellFunc_NothFromBalconyPhaseSwitch, Target_RandomPlayer, 0, 0, 0);
    mBlinkTimer = mSkeletonTimer = mPhaseSwitchTimer = INVALIDATE_TIMER;
    mPhaseCounter = 0;

    addEmoteForEvent(Event_OnCombatStart, 8928);
    addEmoteForEvent(Event_OnCombatStart, 8929);
    addEmoteForEvent(Event_OnCombatStart, 8930);
    addEmoteForEvent(Event_OnDied, 8931);
    addEmoteForEvent(Event_OnTargetDied, 8932);
    addEmoteForEvent(Event_OnTargetDied, 8933);
}

void NothThePlaguebringerAI::OnCombatStart(Unit* /*pTarget*/)
{
    if (_isHeroic())
        mBlinkTimer = _addTimer(28000 + RandomUInt(12) * 1000);

    mPhaseSwitchTimer = _addTimer(110000);
    mSkeletonTimer = _addTimer(8000);
    mPhaseCounter = 0;

    if (getCreature()->GetMapMgr() != NULL && getCreature()->GetMapMgr()->GetInterface() != NULL)
    {
        GameObject* Gate = getNearestGameObject(2740.689209f, -3489.697266f, 262.117767f, 181200);
        if (Gate != NULL)
            Gate->SetState(GO_STATE_CLOSED);

        Gate = getNearestGameObject(2683.670654f, -3556.429688f, 261.823334f, 181201);
        if (Gate != NULL)
            Gate->SetState(GO_STATE_CLOSED);
    }
}

void NothThePlaguebringerAI::OnCombatStop(Unit* /*pTarget*/)
{
    if (getCreature()->GetMapMgr() != NULL && getCreature()->GetMapMgr()->GetInterface() != NULL)
    {
        GameObject* Gate = getNearestGameObject(2740.689209f, -3489.697266f, 262.117767f, 181200);
        if (Gate != NULL)
            Gate->SetState(GO_STATE_OPEN);

        Gate = getNearestGameObject(2683.670654f, -3556.429688f, 261.823334f, 181201);
        if (Gate != NULL)
            Gate->SetState(GO_STATE_OPEN);
    }

    for (std::set< PlaguedWarriorAI* >::iterator Iter = mWarriors.begin(); Iter != mWarriors.end(); ++Iter)
    {
        (*Iter)->mNothAI = NULL;
        (*Iter)->despawn();
    }

    mWarriors.clear();

    for (std::set< PlaguedChampionAI* >::iterator Iter = mChampions.begin(); Iter != mChampions.end(); ++Iter)
    {
        (*Iter)->mNothAI = NULL;
        (*Iter)->despawn();
    }

    mChampions.clear();

    for (std::set< PlaguedGuardianAI* >::iterator Iter = mGuardians.begin(); Iter != mGuardians.end(); ++Iter)
    {
        (*Iter)->mNothAI = NULL;
        (*Iter)->despawn();
    }

    mGuardians.clear();
}

void NothThePlaguebringerAI::AIUpdate()
{
    if (isScriptPhase(1))
    {
        if (!_isCasting())
        {
            if (mPhaseCounter < 3 && _isTimerFinished(mPhaseSwitchTimer))
            {
                setScriptPhase(2);
                _resetTimer(mPhaseSwitchTimer, 70000);
                _resetTimer(mSkeletonTimer, 0);
                ++mPhaseCounter;
                return;
            }

            if (mPhaseCounter == 3)
            {
                _applyAura(NOTH_THE_PLAGUEBRINGER_BERSERK);
                ++mPhaseCounter;
            }

            if (_isHeroic() && _isTimerFinished(mBlinkTimer))
            {
                CastSpellNowNoScheduling(mCriple);
                _resetTimer(mBlinkTimer, 28000 + (RandomUInt(1, 12) * 1000));
            }
        }

        if (_isTimerFinished(mSkeletonTimer))
        {
            uint32 SkelLimit = 2;
            if (_isHeroic())
                SkelLimit = 3;

            bool PosTaken[3];
            for (uint8 i = 0; i < 3; ++i)
            {
                PosTaken[i] = false;
            }

            uint32 Id = 0;
            for (uint8 i = 0; i < SkelLimit; ++i)
            {
                Id = RandomUInt(0, (SkelLimit - 1));    // SkellPosPhase1 is 0-indexed
                if (PosTaken[Id])
                {
                    for (uint32 j = 0; j < 3; ++j)
                    {
                        if (!PosTaken[j])
                        {
                            Id = j;
                            break;
                        }
                    }
                }

                PlaguedWarriorAI* WarriorAI = static_cast< PlaguedWarriorAI* >(spawnCreatureAndGetAIScript(CN_PLAGUED_WARRIOR, SkelPosPhase1[Id].x, SkelPosPhase1[Id].y, SkelPosPhase1[Id].z, SkelPosPhase1[Id].o));
                if (WarriorAI != nullptr)
                {
                    WarriorAI->getCreature()->m_noRespawn = true;
                    WarriorAI->AggroNearestPlayer(200);
                    WarriorAI->mNothAI = this;
                    mWarriors.insert(WarriorAI);
                }

                PosTaken[Id] = true;
            }

            sendChatMessage(CHAT_MSG_MONSTER_YELL, 8851, "Rise, my soldiers! Rise and fight once more!");
            _resetTimer(mSkeletonTimer, 30000);
            PosTaken[Id] = true;
        }
    }
    else
    {
        if (!_isCasting() && _isTimerFinished(mPhaseSwitchTimer))
        {
            setScriptPhase(1);
            _resetTimer(mPhaseSwitchTimer, 70000);
            _resetTimer(mSkeletonTimer, 8000);
            if (_isHeroic())
                _resetTimer(mBlinkTimer, 28000 + (RandomUInt(12)) * 1000);

            return;
        }

        if (_isTimerFinished(mSkeletonTimer))
        {
            uint32 SpawnLimit = 2;
            if (_isHeroic())
                ++SpawnLimit;

            uint32 Id = 0;
            uint32 Champions = 0;
            if (mPhaseCounter <= SpawnLimit + 1)
                Champions = SpawnLimit - mPhaseCounter + 1;

            bool PosTaken[4];
            for (uint8 i = 0; i < 4; ++i)
            {
                PosTaken[i] = false;
            }

            for (uint8 i = 0; i < Champions; ++i)
            {
                Id = RandomUInt(1, 3);
                if (PosTaken[Id])
                {
                    for (uint32 j = 0; j < 4; ++j)
                    {
                        if (!PosTaken[Id])
                        {
                            Id = j;
                            break;
                        }
                    }
                }

                PlaguedChampionAI* ChampionAI = static_cast< PlaguedChampionAI* >(spawnCreatureAndGetAIScript(CN_PLAGUED_CHAMPION, SkelPosPhase2[Id].x, SkelPosPhase2[Id].y, SkelPosPhase2[Id].z, SkelPosPhase2[Id].o));
                if (ChampionAI != nullptr)
                {
                    ChampionAI->getCreature()->m_noRespawn = true;
                    ChampionAI->AggroNearestPlayer(200);
                    ChampionAI->mNothAI = this;
                    mChampions.insert(ChampionAI);
                }

                PosTaken[Id] = true;
            }

            for (uint8 i = 0; i < SpawnLimit - Champions; ++i)
            {
                Id = RandomUInt(1, 3);
                if (PosTaken[Id])
                {
                    for (uint32 j = 0; j < 4; ++j)
                    {
                        if (!PosTaken[Id])
                        {
                            Id = j;
                            break;
                        }
                    }
                }

                PlaguedGuardianAI* GuardianAI = static_cast< PlaguedGuardianAI* >(spawnCreatureAndGetAIScript(CN_PLAGUED_GUARDIAN, SkelPosPhase2[Id].x, SkelPosPhase2[Id].y, SkelPosPhase2[Id].z, SkelPosPhase2[Id].o));
                if (GuardianAI != nullptr)
                {
                    GuardianAI->getCreature()->m_noRespawn = true;
                    GuardianAI->AggroNearestPlayer(200);
                    GuardianAI->mNothAI = this;
                    mGuardians.insert(GuardianAI);
                }
            }

            _resetTimer(mSkeletonTimer, 35000);
            PosTaken[Id] = true;
        }
    }

    if (isScriptPhase(2))
    {
        setAIAgent(AGENT_SPELL);
        setRooted(true);
        stopMovement();
    }
}

void NothThePlaguebringerAI::OnScriptPhaseChange(uint32_t phaseId)
{
    switch (phaseId)
    {
        case 1:
            CastSpellNowNoScheduling(mFromBalconySwitch);
            break;
        case 2:
            CastSpellNowNoScheduling(mToBalconySwitch);
            break;
        default:
            break;
    }
}

void NothThePlaguebringerAI::Destroy()
{
    for (std::set< PlaguedWarriorAI* >::iterator Iter = mWarriors.begin(); Iter != mWarriors.end(); ++Iter)
    {
        (*Iter)->mNothAI = NULL;
        (*Iter)->despawn();
    }

    mWarriors.clear();

    for (std::set< PlaguedChampionAI* >::iterator Iter = mChampions.begin(); Iter != mChampions.end(); ++Iter)
    {
        (*Iter)->mNothAI = NULL;
        (*Iter)->despawn();
    }

    mChampions.clear();

    for (std::set< PlaguedGuardianAI* >::iterator Iter = mGuardians.begin(); Iter != mGuardians.end(); ++Iter)
    {
        (*Iter)->mNothAI = NULL;
        (*Iter)->despawn();
    }

    mGuardians.clear();

    delete this;
}

void SpellFunc_NothToBalconyPhaseSwitch(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* /*pTarget*/, TargetType /*pType*/)
{
    NothThePlaguebringerAI* Noth = (pCreatureAI != NULL) ? static_cast< NothThePlaguebringerAI* >(pCreatureAI) : NULL;
    if (Noth != NULL)
    {
        // Are these coords correct ? Or maybe it should be just disappear / appear thing ? And is this spell correct ? I doubt it ...
        Noth->_applyAura(NOTH_THE_PLAGUEBRINGER_BLINK_HEROIC);
        Noth->getCreature()->SetPosition(2631.051025f, -3529.595703f, 274.037811f, 0.109163f);
        Noth->setAIAgent(AGENT_SPELL);
        Noth->setRooted(true);
        Noth->stopMovement();
    }
}

void SpellFunc_NothFromBalconyPhaseSwitch(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType /*pType*/)
{
    NothThePlaguebringerAI* Noth = (pCreatureAI != NULL) ? static_cast< NothThePlaguebringerAI* >(pCreatureAI) : NULL;
    if (Noth != NULL)
    {
        Noth->setAIAgent(AGENT_NULL);
        Noth->setRooted(false);
        Noth->_applyAura(NOTH_THE_PLAGUEBRINGER_BLINK_HEROIC);
        Noth->getCreature()->SetPosition(2684.620850f, -3502.447266f, 261.314880f, 0.098174f);

        if (pTarget != NULL)
            Noth->getCreature()->GetAIInterface()->AttackReaction(pTarget, 200);

        Noth->getCreature()->GetAIInterface()->setNextTarget(pTarget);
    }
}

void SpellFunc_NothCriple(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* /*pTarget*/, TargetType /*pType*/)
{
    NothThePlaguebringerAI* Noth = (pCreatureAI != NULL) ? static_cast< NothThePlaguebringerAI* >(pCreatureAI) : NULL;
    if (Noth != NULL)
    {
        // Dunno if target count that should be affected is correct
        Noth->_applyAura(NOTH_THE_PLAGUEBRINGER_CRIPLE_HEROIC);
        Noth->CastSpell(Noth->mBlink);
    }
}

void SpellFunc_NothBlink(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType /*pType*/)
{
    NothThePlaguebringerAI* Noth = (pCreatureAI != NULL) ? static_cast< NothThePlaguebringerAI* >(pCreatureAI) : NULL;
    if (Noth != NULL)
    {
        Noth->_applyAura(NOTH_THE_PLAGUEBRINGER_BLINK_HEROIC);
        float Angle = Noth->getCreature()->GetOrientation();
        float NewX = Noth->getCreature()->GetPositionX() + 20.0f * cosf(Angle);
        float NewY = Noth->getCreature()->GetPositionY() + 20.0f * sinf(Angle);
        Noth->getCreature()->SetPosition(NewX, NewY, Noth->getCreature()->GetPositionZ(), Angle);
        Noth->_clearHateList();
        if (pTarget != NULL)
            Noth->getCreature()->GetAIInterface()->AttackReaction(pTarget, 500);

        Noth->getCreature()->GetAIInterface()->setNextTarget(pTarget);
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// Plagued Warrior
PlaguedWarriorAI::PlaguedWarriorAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddSpell(PLAGUED_WARRIOR_STRIKE, Target_Current, 10, 0, 5, 0, 8);
    AddSpell(PLAGUED_WARRIOR_CLEAVE, Target_Current, 10, 0, 10, 0, 8);

    mNothAI = NULL;
}

void PlaguedWarriorAI::Destroy()
{
    if (mNothAI != NULL)
    {
        std::set< PlaguedWarriorAI* >::iterator Iter = mNothAI->mWarriors.find(this);
        if (Iter != mNothAI->mWarriors.end())
            mNothAI->mWarriors.erase(Iter);
    }

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Plagued Champion
PlaguedChampionAI::PlaguedChampionAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
    {
        AddSpell(PLAGUED_CHAMPION_MORTAL_STRIKE_HEROIC, Target_Current, 8, 0, 10, 0, 8);
        AddSpell(PLAGUED_CHAMPION_SHADOW_SHOCK_HEROIC, Target_Self, 10, 0, 10);
    }
    else
    {
        AddSpell(PLAGUED_CHAMPION_MORTAL_STRIKE_NORMAL, Target_Current, 8, 0, 10, 0, 8);
        AddSpell(PLAGUED_CHAMPION_SHADOW_SHOCK_NORMAL, Target_Self, 10, 0, 10);
    }

    mNothAI = NULL;
}

void PlaguedChampionAI::Destroy()
{
    if (mNothAI != NULL)
    {
        std::set< PlaguedChampionAI* >::iterator Iter = mNothAI->mChampions.find(this);
        if (Iter != mNothAI->mChampions.end())
            mNothAI->mChampions.erase(Iter);
    }

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Plagued Guardian
PlaguedGuardianAI::PlaguedGuardianAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(PLAGUED_GUARDIAN_ARCANE_EXPLOSION_HEROIC, Target_Self, 10, 1.5, 10);
    else
        AddSpell(PLAGUED_GUARDIAN_ARCANE_EXPLOSION_NORMAL, Target_Self, 10, 1.5, 10);

    mNothAI = NULL;
}

void PlaguedGuardianAI::Destroy()
{
    if (mNothAI != NULL)
    {
        std::set< PlaguedGuardianAI* >::iterator Iter = mNothAI->mGuardians.find(this);
        if (Iter != mNothAI->mGuardians.end())
            mNothAI->mGuardians.erase(Iter);
    }

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Heigan the Unclean
HeiganTheUncleanAI::HeiganTheUncleanAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddPhaseSpell(1, AddSpell(HEIGAN_THE_UNCLEAN_SPELL_DISRUPTION, Target_Self, 100, 0, 5));
    if (_isHeroic())
        AddPhaseSpell(1, AddSpell(HEIGAN_THE_UNCLEAN_DECREPIT_FEVER_HEROIC, Target_Self, 100, 0, 20));
    else
        AddPhaseSpell(1, AddSpell(HEIGAN_THE_UNCLEAN_DECREPIT_FEVER_NORMAL, Target_Self, 100, 0, 20));

    AddPhaseSpell(2, AddSpell(HEIGAN_THE_UNCLEAN_PLAGUE_CLOUD_DAMAGE, Target_Self, 100, 0, 2));

    addEmoteForEvent(Event_OnCombatStart, 8938);
    addEmoteForEvent(Event_OnCombatStart, 8939);
    addEmoteForEvent(Event_OnCombatStart, 8940);
    addEmoteForEvent(Event_OnDied, 8941);
    addEmoteForEvent(Event_OnTargetDied, 8942);
    addEmoteForEvent(Event_OnTaunt, 8943);
    addEmoteForEvent(Event_OnTaunt, 8944);
    addEmoteForEvent(Event_OnTaunt, 8945);
    addEmoteForEvent(Event_OnTaunt, 8946);
    mPhaseSwitchTimer = mEruptionTimer = INVALIDATE_TIMER;
    mEruptionPhase = 3;
    mClockWiseEruption = true;
}

uint32 HeiganTheUncleanAI::CalculateTriggerArea(float pPosX, float pPosY)
{
    pPosY -= HeiganPos[1];
    if (pPosY < 1.0f)
        return 0;

    pPosX -= HeiganPos[0];
    if (pPosX > -1.0f)
        return 3;

    float slope = pPosY / pPosX;
    for (uint8 i = 0; i < 3; ++i)
        if (slope > HeiganEruptionSlope[i])
            return i;

    return 3;
}

void HeiganTheUncleanAI::CallEruptionEvent(int32 pTimerId, int32 pNewTime)
{
    if (pTimerId < 0)
        return;

    for (std::set< std::pair< uint32, PlagueFissureGO* > >::iterator Iter = mFissures.begin(); Iter != mFissures.end(); ++Iter)
    {
        if ((int)(*Iter).first != mEruptionPhase)
        {
            (*Iter).second->DoErrupt();
        }
    }

    _resetTimer(pTimerId, pNewTime);
    if (mEruptionPhase == 0)
        mClockWiseEruption = false;
    else if (mEruptionPhase == 3)
        mClockWiseEruption = true;

    if (mClockWiseEruption)
        --mEruptionPhase;
    else
        ++mEruptionPhase;
}

void HeiganTheUncleanAI::OnCombatStart(Unit* /*pTarget*/)
{
    mPhaseSwitchTimer = _addTimer(90000);
    mEruptionTimer = _addTimer(8000);
    mEruptionPhase = 3;
    mClockWiseEruption = true;

    if (getCreature()->GetMapMgr() != NULL && getCreature()->GetMapMgr()->GetInterface() != NULL)
    {
        GameObject* Gate = getNearestGameObject(2790.709961f, -3708.669922f, 276.584991f, 181202);
        if (Gate != NULL)
            Gate->SetState(GO_STATE_CLOSED);

        Gate = getNearestGameObject(2771.718506f, -3739.965820f, 273.616211f, 181203);
        if (Gate != NULL)
            Gate->SetState(GO_STATE_CLOSED);

        if (mFissures.size() == 0)
        {
            GameObject* Fissure = NULL;
            PlagueFissureGO* FissureGO = NULL;
            for (std::set< Object* >::iterator Iter = getCreature()->GetInRangeSetBegin(); Iter != getCreature()->GetInRangeSetEnd(); ++Iter)
            {
                if ((*Iter) == NULL || !(*Iter)->IsGameObject())
                    continue;

                Fissure = static_cast< GameObject* >(*Iter);

                if (Fissure->GetGameObjectProperties() == nullptr)
                    continue;

                if (Fissure->getUInt32Value(GAMEOBJECT_DISPLAYID) != 6785 && Fissure->getUInt32Value(GAMEOBJECT_DISPLAYID) != 1287)
                    continue;

                if (Fissure->GetScript() == NULL)
                    continue;

                uint32 AreaId = CalculateTriggerArea(Fissure->GetPositionX(), Fissure->GetPositionY());
                FissureGO = static_cast< PlagueFissureGO* >(Fissure->GetScript());
                mFissures.insert(std::make_pair(AreaId, FissureGO));
                FissureGO->mHeiganAI = this;
                FissureGO->SetState(GO_STATE_CLOSED);
            }
        }
    }
}

void HeiganTheUncleanAI::OnCombatStop(Unit* /*pTarget*/)
{
    _unsetTargetToChannel();
    if (getCreature()->GetMapMgr() != NULL && getCreature()->GetMapMgr()->GetInterface() != NULL)
    {
        GameObject* Gate = getNearestGameObject(2790.709961f, -3708.669922f, 276.584991f, 181202);
        if (Gate != NULL)
            Gate->SetState(GO_STATE_OPEN);

        Gate = getNearestGameObject(2771.718506f, -3739.965820f, 273.616211f, 181203);
        if (Gate != NULL)
            Gate->SetState(GO_STATE_OPEN);
    }

    mFissures.clear();
}

void HeiganTheUncleanAI::AIUpdate()
{
    if (isScriptPhase(1))
    {
        if (!_isCasting() && _isTimerFinished(mPhaseSwitchTimer))
        {
            _applyAura(HEIGAN_THE_UNCLEAN_TELEPORT);
            sendChatMessage(CHAT_MSG_MONSTER_YELL, 8833, "The end is uppon you!");
            getCreature()->SetPosition(2794.235596f, -3707.067627f, 276.545746f, 2.407245f);
            _setTargetToChannel(getCreature(), HEIGAN_THE_UNCLEAN_PLAGUE_CLOUD_CHANNEL);
            setAIAgent(AGENT_SPELL);
            setRooted(true);
            stopMovement();
            setScriptPhase(2);
            mEruptionPhase = 3;
            _resetTimer(mPhaseSwitchTimer, 45000);
            _resetTimer(mEruptionTimer, 3000);
            return;
        }

        if (_isTimerFinished(mEruptionTimer))
            CallEruptionEvent(mEruptionTimer, 8000);
    }
    else
    {
        if (!_isCasting() && _isTimerFinished(mPhaseSwitchTimer))
        {
            _unsetTargetToChannel();
            setAIAgent(AGENT_NULL);
            setRooted(false);
            setScriptPhase(1);
            _resetTimer(mPhaseSwitchTimer, 90000);
            return;
        }

        if (_isTimerFinished(mEruptionTimer))
            CallEruptionEvent(mEruptionTimer, 3000);
    }

    if (isScriptPhase(2))
    {
        setAIAgent(AGENT_SPELL);
        setRooted(true);
        stopMovement();
    }
}

void HeiganTheUncleanAI::Destroy()
{
    for (std::set< std::pair< uint32, PlagueFissureGO* > >::iterator itr = mFissures.begin(); itr != mFissures.end(); ++itr)
    {
        (*itr).second->ResetHeiganAI();
    }
    mFissures.clear();
    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Plague Fissure
GameObjectAIScript* PlagueFissureGO::Create(GameObject* pGameObject)
{
    return new PlagueFissureGO(pGameObject);
}

PlagueFissureGO::PlagueFissureGO(GameObject* pGameObject) : GameObjectAIScript(pGameObject)
{
    mHeiganAI = NULL;
}

void PlagueFissureGO::SetState(uint8_t pState)
{
    _gameobject->SetState(pState);
}

void PlagueFissureGO::DoErrupt()
{
    _gameobject->SetCustomAnim();

    Creature* pFissureTrigger = _gameobject->GetMapMgr()->GetInterface()->SpawnCreature(15384, _gameobject->GetPositionX(), _gameobject->GetPositionY(), _gameobject->GetPositionZ(), _gameobject->GetOrientation(), true, false, 0, 0, 1);
    if (!pFissureTrigger)
        return;

    pFissureTrigger->CastSpell(pFissureTrigger, FISSURE_TRIGGER_ERUPTION, true);
    pFissureTrigger->Despawn(2000, 0);
}

void PlagueFissureGO::Destroy()
{
    if (mHeiganAI != NULL)
    {
        for (std::set< std::pair< uint32, PlagueFissureGO* > >::iterator Iter = mHeiganAI->mFissures.begin(); Iter != mHeiganAI->mFissures.end(); ++Iter)
        {
            if ((*Iter).second == this)
            {
                mHeiganAI->mFissures.erase(Iter);
                break;
            }
        }

        mHeiganAI = NULL;
    }

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Loatheb
LoathebAI::LoathebAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddSpell(LOATHEB_NECROTIC_AURA, Target_Self, 100, 0, 20);
    if (_isHeroic())
        AddSpell(LOATHEB_DEATHBLOOM_HEROIC, Target_Self, 100, 0, 30);
    else
        AddSpell(LOATHEB_DEATHBLOOM_NORMAL, Target_Self, 100, 0, 30);

    mSporeTimer = mDoomTimer = mDeathbloomTimer = INVALIDATE_TIMER;
    mDoomStaticTimer = 120000;
    mDeathbloomDamagePhase = false;
}

void LoathebAI::OnCombatStart(Unit* /*pTarget*/)
{
    mDoomStaticTimer = 120000;
    mSporeTimer = _addTimer(30000);
    mDoomTimer = _addTimer(mDoomStaticTimer);
    mDeathbloomTimer = _addTimer(30000);
    mDeathbloomDamagePhase = false;
}

void LoathebAI::OnCombatStop(Unit* /*pTarget*/)
{
    for (std::set< SporeAI* >::iterator Iter = mSpores.begin(); Iter != mSpores.end(); ++Iter)
    {
        (*Iter)->mLoathebAI = NULL;
        (*Iter)->despawn();
    }

    mSpores.clear();
}

void LoathebAI::AIUpdate()
{
    if (_isTimerFinished(mSporeTimer))
    {
        bool PosTaken[4];
        for (uint8 i = 0; i < 4; ++i)
        {
            PosTaken[i] = false;
        }

        uint32 Id = 0;
        for (uint8 i = 0; i < 3; ++i)
        {
            Id = RandomUInt(3);
            if (PosTaken[Id])
            {
                for (uint32 j = 0; j < 4; ++j)
                {
                    if (!PosTaken[j])
                    {
                        Id = j;
                        break;
                    }
                }
            }

            SporeAI* Spore = static_cast< SporeAI* >(spawnCreatureAndGetAIScript(CN_SPORE, Spores[Id].x, Spores[Id].y, Spores[Id].z, Spores[Id].o));
            if (Spore != nullptr)
            {
                Spore->getCreature()->m_noRespawn = true;
                Spore->AggroRandomPlayer(200);
                Spore->mLoathebAI = this;
                mSpores.insert(Spore);
            }
        }

        _resetTimer(mSporeTimer, 30000);
        PosTaken[Id] = true;
    }

    if (!_isCasting())
    {
        if (_isTimerFinished(mDoomTimer))
        {
            if (_isHeroic())
                _applyAura(LOATHEB_INEVITABLE_DOOM_HEROIC);
            else
                _applyAura(LOATHEB_INEVITABLE_DOOM_NORMAL);

            if (mDoomStaticTimer > 36000)
            {
                if (mDoomStaticTimer <= 36000)
                    mDoomStaticTimer = 15000;
                else
                    mDoomStaticTimer -= 21000;
            }

            _resetTimer(mDoomTimer, mDoomStaticTimer);
        }
        else if (_isTimerFinished(mDeathbloomTimer))
        {
            if (mDeathbloomDamagePhase)
            {
                Player* PlayerPtr = NULL;
                for (std::set< Object* >::iterator Iter = getCreature()->GetInRangePlayerSetBegin(); Iter != getCreature()->GetInRangePlayerSetEnd(); ++Iter)
                {
                    if ((*Iter) == NULL)
                        continue;

                    PlayerPtr = static_cast< Player* >(*Iter);
                    if (!PlayerPtr->isAlive())
                        continue;

                    if (!PlayerPtr->HasAura(LOATHEB_DEATHBLOOM_NORMAL) && !PlayerPtr->HasAura(LOATHEB_DEATHBLOOM_HEROIC))
                        continue;

                    if (_isHeroic())
                        PlayerPtr->CastSpell(PlayerPtr, LOATHEB_DEATHBLOOM_DAMAGE_HEROIC, true);
                    else
                        PlayerPtr->CastSpell(PlayerPtr, LOATHEB_DEATHBLOOM_DAMAGE_NORMAL, true);
                }

                _resetTimer(mDeathbloomTimer, 25000);
            }
            else
            {
                if (_isHeroic())
                    _applyAura(LOATHEB_DEATHBLOOM_HEROIC);
                else
                    _applyAura(LOATHEB_DEATHBLOOM_NORMAL);

                _resetTimer(mDeathbloomTimer, 5000);
            }

            mDeathbloomDamagePhase = !mDeathbloomDamagePhase;
        }
    }
}

void LoathebAI::Destroy()
{
    for (std::set< SporeAI* >::iterator Iter = mSpores.begin(); Iter != mSpores.end(); ++Iter)
    {
        (*Iter)->mLoathebAI = NULL;
        (*Iter)->despawn();
    }

    mSpores.clear();

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Spore
SporeAI::SporeAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    despawn(90000);
    mLoathebAI = NULL;
}

void SporeAI::OnDied(Unit* /*pKiller*/)
{
    _applyAura(SPORE_FUNGAL_CREEP);
}

void SporeAI::Destroy()
{
    if (mLoathebAI != NULL)
    {
        std::set< SporeAI* >::iterator Iter = mLoathebAI->mSpores.find(this);
        if (Iter != mLoathebAI->mSpores.end())
            mLoathebAI->mSpores.erase(Iter);

        mLoathebAI = NULL;
    }

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// The Military Quarter

/////////////////////////////////////////////////////////////////////////////////
////// Death Knight
DeathKnightAI::DeathKnightAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(DEATH_KNIGHT_DEATH_COIL_HEROIC, Target_RandomPlayer, 9, 0, 15, 0, 45);
    else
        AddSpell(DEATH_KNIGHT_DEATH_COIL_NORMAL, Target_RandomPlayer, 9, 0, 15, 0, 45);

    AddSpell(DEATH_KNIGHT_DEATH_COIL_HEAL, Target_WoundedFriendly, 6, 0, 15, 0, 45);
    AddSpell(DEATH_KNIGHT_HYSTERIA, Target_RandomFriendly, 7, 0, 30, 0, 45);
}

void DeathKnightAI::OnCombatStart(Unit* /*pTarget*/)
{
    _applyAura(DEATH_KNIGHT_BLOOD_PRESENCE);
}

/////////////////////////////////////////////////////////////////////////////////
////// Death Knight Captain
DeathKnightCaptainAI::DeathKnightCaptainAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(DEATH_KNIGHT_CAPTAIN_PLAGUE_STRIKE_HEROIC, Target_Current, 8, 0, 20, 0, 8);
    else
        AddSpell(DEATH_KNIGHT_CAPTAIN_PLAGUE_STRIKE_NORMAL, Target_Current, 8, 0, 20, 0, 8);

    AddSpell(DEATH_KNIGHT_CAPTAIN_RAISE_DEAD, Target_RandomPlayerDestination, 6, 1.5, 20);
    AddSpell(DEATH_KNIGHT_CAPTAIN_WHIRLWIND, Target_Self, 10, 0, 10);
}

void DeathKnightCaptainAI::OnCombatStart(Unit* /*pTarget*/)
{
    _applyAura(DEATH_KNIGHT_CAPTAIN_UNHOLY_PRESENCE);
}

/////////////////////////////////////////////////////////////////////////////////
////// Skeletal Construct - wiki says he's in Naxx, but WoWHead claims him to be in Icecrown only

/////////////////////////////////////////////////////////////////////////////////
////// Ghost of Naxxramas
GhostOfNaxxramasAI::GhostOfNaxxramasAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
}

/////////////////////////////////////////////////////////////////////////////////
////// Shade of Naxxramas
ShadeOfNaxxramasAI::ShadeOfNaxxramasAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(SHADE_OF_NAXXRAMAS_SHADOW_BOLT_VOLLEY_HEROIC, Target_Self, 10, 0, 10);
    else
        AddSpell(SHADE_OF_NAXXRAMAS_SHADOW_BOLT_VOLLEY_NORMAL, Target_Self, 10, 0, 10);

    AddSpell(SHADE_OF_NAXXRAMAS_PORTAL_OF_SHADOWS, Target_Self, 8, 0, 60);
}

void ShadeOfNaxxramasAI::OnDied(Unit* /*pKiller*/)
{
    CreatureAIScript* Ghost = spawnCreatureAndGetAIScript(CN_GHOST_OF_NAXXRAMAS, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), getCreature()->GetFaction());
    if (Ghost != nullptr)
    {
        Ghost->getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
        Ghost->_setDespawnWhenInactive(true);
        static_cast<CreatureAIScript*>(Ghost)->AggroNearestPlayer(200);
    }

    for (std::set< PortalOfShadowsAI* >::iterator Iter = mPortals.begin(); Iter != mPortals.end(); ++Iter)
    {
        (*Iter)->mShadeAI = NULL;
        (*Iter)->despawn();
    }

    mPortals.clear();
}

void ShadeOfNaxxramasAI::Destroy()
{
    for (std::set< PortalOfShadowsAI* >::iterator Iter = mPortals.begin(); Iter != mPortals.end(); ++Iter)
    {
        (*Iter)->mShadeAI = NULL;
        (*Iter)->despawn();
    }

    mPortals.clear();

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Portal of Shadows - timer value is a wild guess
PortalOfShadowsAI::PortalOfShadowsAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    RegisterAIUpdateEvent(1000);
    mSpawnTimer = _addTimer(15000);
    mShadeAI = NULL;

    // We do not consider using a spell that summons these portals by anyone else than Shade of Naxxramas.
    // I must figure out why it's often not added if only one Shade is on the battlefield.
    // I don't like this method anyway.
    if (getCreature()->GetSummonedByGUID() != 0 && getCreature()->GetMapMgr() != NULL && getCreature()->GetMapMgr()->GetInterface() != NULL)
    {
        //mShadeAI = static_cast< ShadeOfNaxxramasAI* >(GetNearestCreature(CN_SHADE_OF_NAXXRAMAS));
        Creature* UnitPtr = getNearestCreature(CN_SHADE_OF_NAXXRAMAS);
        if (UnitPtr != NULL)
        {
            mShadeAI = static_cast< ShadeOfNaxxramasAI* >(UnitPtr->GetScript());
            if (mShadeAI != NULL)
                mShadeAI->mPortals.insert(this);
        }
    }
}

void PortalOfShadowsAI::OnCombatStart(Unit* /*pTarget*/)
{
    setAIAgent(AGENT_SPELL);
    setRooted(true);
    stopMovement();
}

void PortalOfShadowsAI::OnCombatStop(Unit* /*pTarget*/)
{
    CancelAllSpells();
    _cancelAllTimers();
    setRooted(false);
    setAIAgent(AGENT_NULL);
}

void PortalOfShadowsAI::AIUpdate()
{
    if (mShadeAI != nullptr && mShadeAI->getCreature()->GetAIInterface()->getNextTarget() != nullptr)
    {
        if (_isTimerFinished(mSpawnTimer))
        {
            CreatureAIScript* Ghost = spawnCreatureAndGetAIScript(CN_GHOST_OF_NAXXRAMAS, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), getCreature()->GetFaction());
            if (Ghost != nullptr)
            {
                Ghost->getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                Ghost->_setDespawnWhenInactive(true);
                static_cast<CreatureAIScript*>(Ghost)->AggroNearestPlayer(200);
            }

            _resetTimer(mSpawnTimer, 15000);
        }
    }
    else
    {
        RemoveAIUpdateEvent();
        despawn();
        return;
    }

    setAIAgent(AGENT_SPELL);
    setRooted(true);
    stopMovement();
}

void PortalOfShadowsAI::Destroy()
{
    if (mShadeAI != nullptr)
    {
        std::set< PortalOfShadowsAI* >::iterator Iter = mShadeAI->mPortals.find(this);
        if (Iter != mShadeAI->mPortals.end())
            mShadeAI->mPortals.erase(Iter);

        mShadeAI = nullptr;
    }

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Necro Knight
NecroKnightAI::NecroKnightAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddSpell(NECRO_KNIGHT_ARCANE_EXPLOSION, Target_Self, 8, 1.5, 5);
    AddSpell(NECRO_KNIGHT_BLAST_WAVE, Target_Self, 7, 0, 5);
    AddSpell(NECRO_KNIGHT_CONE_OF_COLD, Target_Current, 7, 0, 5, 0, 10);
    AddSpell(NECRO_KNIGHT_FLAMESTRIKE, Target_RandomPlayerDestination, 8, 2, 10, 0, 30);
    AddSpell(NECRO_KNIGHT_FROST_NOVA, Target_Self, 8, 0, 10);
    AddSpellFunc(&SpellFunc_NecroKnightBlink, Target_RandomPlayerNotCurrent, 5, 0, 20, 0, 30);
}

void SpellFunc_NecroKnightBlink(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType /*pType*/)
{
    NecroKnightAI* NecroKnight = (pCreatureAI != NULL) ? static_cast< NecroKnightAI* >(pCreatureAI) : NULL;
    if (NecroKnight != NULL && pTarget != NULL)
    {
        NecroKnight->_applyAura(NECRO_KNIGHT_BLINK);
        NecroKnight->getCreature()->SetPosition(pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), NecroKnight->getCreature()->GetOrientation());
        NecroKnight->getCreature()->GetAIInterface()->AttackReaction(pTarget, 500);
        NecroKnight->getCreature()->GetAIInterface()->setNextTarget(pTarget);
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// Skeletal Smith
SkeletalSmithAI::SkeletalSmithAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddSpell(SKELETAL_SMITH_CRUSH_ARMOR, Target_Current, 10, 0, 10, 0, 8);
    AddSpell(SKELETAL_SMITH_DISARM, Target_Current, 10, 0, 15, 0, 8);
    AddSpell(SKELETAL_SMITH_THUNDERCLAP, Target_Self, 8, 0, 15);
    //AddSpell(SKELETAL_SMITH_SUDDER_ARMOR, Target_Current, 8, 0, 10, 0, 8);
}

/////////////////////////////////////////////////////////////////////////////////
////// Death Knight Cavalier
DeathKnightCavalierAI::DeathKnightCavalierAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
    {
        AddSpell(DEATH_KNIGHT_CAVALIER_BONE_ARMOR_HEROIC, Target_Self, 7, 0, 30);
        AddSpell(DEATH_KNIGHT_CAVALIER_ICY_TOUCH_HEROIC, Target_RandomPlayer, 10, 0, 15, 0, 20);    // Target_Current ?
        AddSpell(DEATH_KNIGHT_CAVALIER_STRANGULATE_HEROIC, Target_RandomPlayer, 7, 1, 15, 0, 50);
    }
    else
    {
        AddSpell(DEATH_KNIGHT_CAVALIER_BONE_ARMOR_NORMAL, Target_Self, 7, 0, 30);
        AddSpell(DEATH_KNIGHT_CAVALIER_ICY_TOUCH_NORMAL, Target_RandomPlayer, 10, 0, 15, 0, 20);    // Target_Current ?
        AddSpell(DEATH_KNIGHT_CAVALIER_STRANGULATE_NORMAL, Target_RandomPlayer, 7, 1, 15, 0, 50);
    }

    AddSpell(DEATH_KNIGHT_CAVALIER_AURA_OF_AGONY, Target_RandomPlayer, 10, 0, 5, 0, 30);
    AddSpell(DEATH_KNIGHT_CAVALIER_CLEAVE, Target_Current, 10, 0, 10, 0, 8);
    AddSpell(DEATH_KNIGHT_CAVALIER_DEATH_COIL, Target_RandomPlayer, 7, 0, 10, 0, 30);
    getCreature()->setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 25278);
    mChargerAI = NULL;
    mIsMounted = true;
}

void DeathKnightCavalierAI::OnCombatStop(Unit* /*pTarget*/)
{
    if (mChargerAI != NULL)
    {
        if (isAlive() && getCreature()->GetMount() == 0)
            getCreature()->setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 25278);

        mChargerAI->mDeathKnightAI = NULL;
        mChargerAI->despawn();
        mChargerAI = NULL;
    }

    mIsMounted = true;
}

void DeathKnightCavalierAI::AIUpdate()
{
    if (mIsMounted && getCreature()->GetMount() == 0)
        getCreature()->setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 25278);
    if (mIsMounted && RandomUInt(99) < 2)
    {
        getCreature()->setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
        _applyAura(DEATH_KNIGHT_CAVALIER_DISMOUNT_DEATHCHARGER);
        mIsMounted = false;
    }
}

void DeathKnightCavalierAI::Destroy()
{
    if (mChargerAI != NULL)
    {
        mChargerAI->mDeathKnightAI = NULL;
        mChargerAI->despawn();
        mChargerAI = NULL;
    }

    delete this;
}

/////////////////////////////////////////////////////////////////////////////////
////// Deathcharger Steed
DeathchargerSteedAI::DeathchargerSteedAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mDeathKnightAI = NULL;
    mCharge = AddSpell(DEATHCHARGER_STEED_CHARGE, Target_Current, 0, 0, 0, 5, 45);
    //AddSpellFunc(&SpellFunc_DeathchargerSteedCharge, Target_RandomPlayer, 8, 0, 15, 5, 45);

    // We do not consider using a spell that summons this unit by anyone else than Death Knight Cavalier.
    // I don't like this method anyway.
    if (getCreature()->GetSummonedByGUID() != 0 && getCreature()->GetMapMgr() != NULL && getCreature()->GetMapMgr()->GetInterface() != NULL)
    {
        mDeathKnightAI = static_cast< DeathKnightCavalierAI* >(getNearestCreatureAI(CN_DEATH_KNIGHT_CAVALIER));
        if (mDeathKnightAI != NULL && mDeathKnightAI->mChargerAI == NULL)
            mDeathKnightAI->mChargerAI = this;
    }
}

void DeathchargerSteedAI::OnCombatStop(Unit* /*pTarget*/)
{
    if (mDeathKnightAI != NULL)
    {
        if (mDeathKnightAI->_isInCombat())
            AggroNearestUnit(200);
        else
        {
            mDeathKnightAI->mChargerAI = NULL;
            mDeathKnightAI = NULL;
            despawn(1);
        }
    }
}

void DeathchargerSteedAI::Destroy()
{
    if (mDeathKnightAI != NULL)
    {
        mDeathKnightAI->mChargerAI = NULL;
        mDeathKnightAI = NULL;
    }

    delete this;
}

void SpellFunc_DeathchargerSteedCharge(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType /*pType*/)
{
    DeathchargerSteedAI* Deathcharger = (pCreatureAI != NULL) ? static_cast< DeathchargerSteedAI* >(pCreatureAI) : NULL;
    if (Deathcharger != NULL)
    {
        Unit* CurrentTarget = Deathcharger->getCreature()->GetAIInterface()->getNextTarget();
        if (CurrentTarget != NULL && CurrentTarget != pTarget)
        {
            Deathcharger->getCreature()->GetAIInterface()->AttackReaction(pTarget, 500);
            Deathcharger->getCreature()->GetAIInterface()->setNextTarget(pTarget);
            //Deathcharger->GetUnit()->GetAIInterface()->RemoveThreatByPtr(CurrentTarget);
        }

        Deathcharger->CastSpellNowNoScheduling(Deathcharger->mCharge);
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// Dark Touched Warrior
DarkTouchedWarriorAI::DarkTouchedWarriorAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddSpell(DARK_TOUCHED_WARRIOR_WHIRLWIND, Target_Self, 10, 0, 15);

    mResetHateTimer = INVALIDATE_TIMER;
}

void DarkTouchedWarriorAI::OnCombatStart(Unit* /*pTarget*/)
{
    mResetHateTimer = _addTimer(8000 + RandomUInt(7) * 1000);
}

void DarkTouchedWarriorAI::AIUpdate()
{
    if (!_isCasting() && _isTimerFinished(mResetHateTimer))
    {
        _clearHateList();
        mResetHateTimer = _addTimer(8000 + RandomUInt(7) * 1000);
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// Risen Squire
RisenSquireAI::RisenSquireAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddSpell(RISEN_SQUIRE_PIERCE_ARMOR, Target_Current, 10, 0, 15, 0, 8);
}

/////////////////////////////////////////////////////////////////////////////////
////// Unholy Axe
UnholyAxeAI::UnholyAxeAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
    {
        AddSpell(UNHOLY_AXE_MORTAL_STRIKE_HEROIC, Target_Current, 10, 0, 10, 0, 8);
        AddSpell(UNHOLY_AXE_WHIRLWIND_HEROIC, Target_Self, 8, 2, 15);
    }
    else
    {
        AddSpell(UNHOLY_AXE_MORTAL_STRIKE_NORMAL, Target_Current, 10, 0, 10, 0, 8);
        AddSpell(UNHOLY_AXE_WHIRLWIND_NORMAL, Target_Self, 8, 2, 15);
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// Unholy Sword
UnholySwordAI::UnholySwordAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(UNHOLY_SWORD_CLEAVE_HEROIC, Target_Current, 10, 0, 15, 0, 8);
    else
        AddSpell(UNHOLY_SWORD_CLEAVE_NORMAL, Target_Current, 10, 0, 15, 0, 8);
}

/////////////////////////////////////////////////////////////////////////////////
////// Unholy Staff
UnholyStaffAI::UnholyStaffAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(UNHOLY_STAFF_ARCANE_EXPLOSION_HEROIC, Target_Self, 8, 0.5, 15);
    else
        AddSpell(UNHOLY_STAFF_ARCANE_EXPLOSION_NORMAL, Target_Self, 8, 0.5, 15);

    AddSpell(UNHOLY_STAFF_FROST_NOVA, Target_Self, 10, 0, 15);
    AddSpell(UNHOLY_STAFF_POLYMORPH, Target_RandomPlayer, 8, 0, 20, 0, 45);    // NotCurrent ?
}

/////////////////////////////////////////////////////////////////////////////////
////// The Construct Quarter

/////////////////////////////////////////////////////////////////////////////////
////// Patchwork Golem
PatchworkGolemAI::PatchworkGolemAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddSpell(PATCHWORK_GOLEM_CLEAVE, Target_Current, 10, 0, 10, 0, 8);
    if (_isHeroic())
    {
        AddSpell(PATCHWORK_GOLEM_EXECUTE_HEROIC, Target_Current, 8, 0, 10, 0, 8);
        AddSpell(PATCHWORK_GOLEM_WAR_STOMP_HEROIC, Target_Self, 8, 0, 15);
        _applyAura(PATCHWORK_GOLEM_DISEASE_CLOUD_HEROIC);
    }
    else
    {
        AddSpell(PATCHWORK_GOLEM_EXECUTE_NORMAL, Target_Current, 8, 0, 10, 0, 8);
        AddSpell(PATCHWORK_GOLEM_WAR_STOMP_NORMAL, Target_Self, 8, 0, 15);
        _applyAura(PATCHWORK_GOLEM_DISEASE_CLOUD_NORMAL);
    }
}

void PatchworkGolemAI::OnCombatStart(Unit* /*pTarget*/)
{
    if (_isHeroic())
        _applyAura(PATCHWORK_GOLEM_DISEASE_CLOUD_HEROIC);
    else
        _applyAura(PATCHWORK_GOLEM_DISEASE_CLOUD_NORMAL);
}

void PatchworkGolemAI::OnCombatStop(Unit* /*pTarget*/)
{
    if (isAlive())
    {
        if (_isHeroic())
            _applyAura(PATCHWORK_GOLEM_DISEASE_CLOUD_HEROIC);
        else
            _applyAura(PATCHWORK_GOLEM_DISEASE_CLOUD_NORMAL);
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// Bile Retcher
BileRetcherAI::BileRetcherAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(BILE_RETCHER_BILE_VOMIT_NORMAL, Target_Destination, 10, 0, 10, 0, 20);
    else
        AddSpell(BILE_RETCHER_BILE_VOMIT_HEROIC, Target_Destination, 10, 0, 10, 0, 20);

    AddSpell(BILE_RETCHER_BILE_RETCHER_SLAM, Target_Destination, 8, 0, 10, 0, 10);
}

/////////////////////////////////////////////////////////////////////////////////
////// Sewage Slime
SewageSlimeAI::SewageSlimeAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    _applyAura(SEWAGE_SLIME_DISEASE_CLOUD);
}

void SewageSlimeAI::OnCombatStart(Unit* /*pTarget*/)
{
    _applyAura(SEWAGE_SLIME_DISEASE_CLOUD);
}

void SewageSlimeAI::OnCombatStop(Unit* /*pTarget*/)
{
    _applyAura(SEWAGE_SLIME_DISEASE_CLOUD);
}

/////////////////////////////////////////////////////////////////////////////////
////// Embalming Slime
EmbalmingSlimeAI::EmbalmingSlimeAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    _applyAura(EMBALMING_SLIME_EMBALMING_CLOUD);
}

void EmbalmingSlimeAI::OnCombatStart(Unit* /*pTarget*/)
{
    _applyAura(EMBALMING_SLIME_EMBALMING_CLOUD);
}

void EmbalmingSlimeAI::OnCombatStop(Unit* /*pTarget*/)
{
    _applyAura(EMBALMING_SLIME_EMBALMING_CLOUD);
}

/////////////////////////////////////////////////////////////////////////////////
////// Mad Scientist
MadScientistAI::MadScientistAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
    {
        AddSpell(MAD_SCIENTIST_GREAT_HEAL_HEROIC, Target_WoundedFriendly, 8, 2, 15, 0, 40);
        AddSpell(MAD_SCIENTIST_MANA_BURN_HEROIC, Target_RandomPlayer, 10, 2, 10, 0, 40);
    }
    else
    {
        AddSpell(MAD_SCIENTIST_GREAT_HEAL_NORMAL, Target_WoundedFriendly, 8, 2, 15, 0, 40);
        AddSpell(MAD_SCIENTIST_MANA_BURN_NORMAL, Target_RandomPlayer, 10, 2, 10, 0, 40);
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// Living Monstrosity
LivingMonstrosityAI::LivingMonstrosityAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddSpell(LIVING_MONSTROSITY_FEAR, Target_Self, 8, 1, 20);
    AddSpell(LIVING_MONSTROSITY_LIGHTNING_TOTEM, Target_Self, 8, 0.5, 25);
    if (_isHeroic())
        AddSpell(LIVING_MONSTROSITY_CHAIN_LIGHTNING_HEROIC, Target_RandomPlayer, 10, 1.5, 10, 0, 45);
    else
        AddSpell(LIVING_MONSTROSITY_CHAIN_LIGHTNING_NORMAL, Target_RandomPlayer, 10, 1.5, 10, 0, 45);
}

/////////////////////////////////////////////////////////////////////////////////
////// Lightning Totem
LightningTotemAI::LightningTotemAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(LIGHTNING_TOTEM_SHOCK_HEROIC, Target_Self, 100, 0.5, 2);
    else
        AddSpell(LIGHTNING_TOTEM_SHOCK_NORMAL, Target_Self, 100, 0.5, 2);

    getCreature()->m_noRespawn = true;
    despawn(60000);
}

void LightningTotemAI::OnCombatStart(Unit* /*pTarget*/)
{
    setAIAgent(AGENT_SPELL);
    setRooted(true);
    stopMovement();
}

void LightningTotemAI::AIUpdate()
{
    // Meh, reset it in case something went wrong
    setAIAgent(AGENT_SPELL);
    setRooted(true);
    stopMovement();
}

/////////////////////////////////////////////////////////////////////////////////
////// Stitched Colossus
StitchedColossusAI::StitchedColossusAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    if (_isHeroic())
        AddSpell(STITCHED_COLOSSUS_MASSIVE_STOMP_HEROIC, Target_Self, 8, 0, 15);
    else
        AddSpell(STITCHED_COLOSSUS_MASSIVE_STOMP_NORMAL, Target_Self, 8, 0, 15);

    mEnraged = false;
}

void StitchedColossusAI::OnCombatStart(Unit* /*pTarget*/)
{
}

void StitchedColossusAI::AIUpdate()
{
    if (!mEnraged && RandomUInt(99) == 0)
    {
        _applyAura(STITCHED_COLOSSUS_UNSTOPPABLE_ENRAGE);
        mEnraged = true;
    }
}

/////////////////////////////////////////////////////////////////////////////////
////// Marauding Geist
MaraudingGeistAI::MaraudingGeistAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddSpell(MARAUDING_GEIST_FRENZIED_LEAP, Target_RandomPlayer, 8, 0, 10);
}

/////////////////////////////////////////////////////////////////////////////////
////// Patchwerk

void SpellFunc_PatchwerkHatefulStrike(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* /*pTarget*/, TargetType /*pType*/)
{
    if (!pThis || !pCreatureAI)
        return;

    uint32 _mostHP = 0;
    Player* pBestTarget = NULL;

    for (std::set< Object* >::iterator PlayerIter = pCreatureAI->getCreature()->GetInRangePlayerSetBegin();
            PlayerIter != pCreatureAI->getCreature()->GetInRangePlayerSetEnd(); ++PlayerIter)
    {
        if ((*PlayerIter) && (static_cast< Player* >(*PlayerIter))->isAlive() && (*PlayerIter)->GetDistance2dSq(pCreatureAI->getCreature()) <= 5.0f
                && (*PlayerIter)->getUInt32Value(UNIT_FIELD_HEALTH) > _mostHP)
        {
            _mostHP = (*PlayerIter)->getUInt32Value(UNIT_FIELD_HEALTH);
            pBestTarget = static_cast< Player* >(*PlayerIter);
        }
    }

    if (pBestTarget == NULL || !pBestTarget->isAlive())
        return;

    if (pCreatureAI->_isHeroic())
        pCreatureAI->getCreature()->CastSpell(pBestTarget, PATCHWERK_HATEFUL_STRIKE_25, true);
    else
        pCreatureAI->getCreature()->CastSpell(pBestTarget, PATCHWERK_HATEFUL_STRIKE_10, true);
}

PatchwerkAI::PatchwerkAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    AddSpellFunc(&SpellFunc_PatchwerkHatefulStrike, Target_Self, 50, 0, 3);
    SetEnrageInfo(AddSpell(PATCHWERK_BERSERK, Target_Self, 0, 0, 0), 360000);

    addEmoteForEvent(Event_OnCombatStart, 8934);
    addEmoteForEvent(Event_OnCombatStart, 8935);
    addEmoteForEvent(Event_OnTargetDied, 8936);
    addEmoteForEvent(Event_OnDied, 8937);
    mEnraged = false;
}

void PatchwerkAI::AIUpdate()
{
    if (mEnraged == false && _getHealthPercent() <= 5)
    {
        _applyAura(PATCHWERK_FRENZY);
        getCreature()->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, "Patchwerk goes into a frenzy!");
        mEnraged = true;
    }
}

void SetupNaxxramas(ScriptMgr* pScriptMgr)
{
#ifndef UseNewMapScriptsProject
    pScriptMgr->register_instance_script(MAP_NAXXRAMAS, &NaxxramasScript::Create);
#endif
    /////////////////////////////////////////////////////////////////////////////////
    ////// The Arachnid Quarter
    // ---- Trash ----
    pScriptMgr->register_creature_script(CN_CARRION_SPINNER, &CarrionSpinnerAI::Create);
    pScriptMgr->register_creature_script(CN_DREAD_CREEPER, &DreadCreeperAI::Create);
    pScriptMgr->register_creature_script(CN_NAXXRAMAS_CULTIST, &NaxxramasCultistAI::Create);
    pScriptMgr->register_creature_script(CN_VENOM_STALKER, &VenomStalkerAI::Create);
    pScriptMgr->register_creature_script(CN_TOMB_HORROR, &TombHorrorAI::Create);
    pScriptMgr->register_creature_script(CN_NAXXRAMAS_ACOLYTE, &NaxxramasAcolyteAI::Create);
    pScriptMgr->register_creature_script(CN_VIGILANT_SHADE, &VigilantShadeAI::Create);
    pScriptMgr->register_creature_script(CN_CRYPT_REAVER, &CryptReaverAI::Create);
    pScriptMgr->register_creature_script(CN_WEB_WRAP, &WebWrapAI::Create);
    pScriptMgr->register_creature_script(CN_MAEXXNA_SPIDERLING, &MaexxnaSpiderlingAI::Create);
    pScriptMgr->register_creature_script(CN_NAXXRAMAS_WORSHIPPER, &NaxxramasWorshipperAI::Create);
    pScriptMgr->register_creature_script(CN_NAXXRAMAS_FOLLOWER, &NaxxramasFollowerAI::Create);
    pScriptMgr->register_creature_script(CN_CRYPT_GUARD, &CryptGuardAI::Create);
    pScriptMgr->register_creature_script(CN_CORPSE_SCARAB, &CorpseScarabAI::Create);
    // ---- Bosses ----
    pScriptMgr->register_creature_script(CN_MAEXXNA, &MaexxnaAI::Create);
    pScriptMgr->register_creature_script(CN_GRAND_WIDOW_FAERLINA, &GrandWidowFaerlinaAI::Create);
    pScriptMgr->register_creature_script(CN_ANUBREKHAN, &AnubRekhanAI::Create);

    /////////////////////////////////////////////////////////////////////////////////
    ////// The Plague Quarter
    // ---- Trash ----
    pScriptMgr->register_creature_script(CN_INFECTIOUS_GHOUL, &InfectiousGhoulAI::Create);
    pScriptMgr->register_creature_script(CN_STONESKIN_GARGOYLE, &StoneskinGargoyleAI::Create);
    pScriptMgr->register_creature_script(CN_FRENZIED_BAT, &FrenziedBatAI::Create);
    pScriptMgr->register_creature_script(CN_PLAGUE_BEAST, &PlagueBeastAI::Create);
    pScriptMgr->register_creature_script(CN_EYE_STALKER, &EyeStalkerAI::Create);
    pScriptMgr->register_creature_script(CN_PLAGUED_WARRIOR, &PlaguedWarriorAI::Create);
    pScriptMgr->register_creature_script(CN_PLAGUED_CHAMPION, &PlaguedChampionAI::Create);
    pScriptMgr->register_creature_script(CN_PLAGUED_GUARDIAN, &PlaguedGuardianAI::Create);
    pScriptMgr->register_creature_script(CN_SPORE, &SporeAI::Create);
    for (uint32 Id = 181510; Id < 181553; ++Id)
    {
        pScriptMgr->register_gameobject_script(Id, &PlagueFissureGO::Create);
    }

    for (uint32 Id = 181676; Id < 181679; ++Id)
    {
        pScriptMgr->register_gameobject_script(Id, &PlagueFissureGO::Create);
    }
    pScriptMgr->register_gameobject_script(181695, &PlagueFissureGO::Create);
    // ---- Bosses ----
    pScriptMgr->register_creature_script(CN_NOTH_THE_PLAGUEBRINGER, &NothThePlaguebringerAI::Create);
    pScriptMgr->register_creature_script(CN_HEIGAN_THE_UNCLEAN, &HeiganTheUncleanAI::Create);
    pScriptMgr->register_creature_script(CN_LOATHEB, &LoathebAI::Create);

    /////////////////////////////////////////////////////////////////////////////////
    ////// The Military Quarter
    // ---- Trash ----
    pScriptMgr->register_creature_script(CN_DEATH_KNIGHT, &DeathKnightAI::Create);
    pScriptMgr->register_creature_script(CN_DEATH_KNIGHT_CAPTAIN, &DeathKnightCaptainAI::Create);
    pScriptMgr->register_creature_script(CN_SHADE_OF_NAXXRAMAS, &ShadeOfNaxxramasAI::Create);
    pScriptMgr->register_creature_script(CN_PORTAL_OF_SHADOWS, &PortalOfShadowsAI::Create);
    pScriptMgr->register_creature_script(CN_NECRO_KNIGHT, &NecroKnightAI::Create);
    pScriptMgr->register_creature_script(CN_SKELETAL_SMITH, &SkeletalSmithAI::Create);
    pScriptMgr->register_creature_script(CN_DEATH_KNIGHT_CAVALIER, &DeathKnightCavalierAI::Create);
    pScriptMgr->register_creature_script(CN_DEATHCHARGER_STEED, &DeathchargerSteedAI::Create);
    pScriptMgr->register_creature_script(CN_DARK_TOUCHED_WARRIOR, &DarkTouchedWarriorAI::Create);
    pScriptMgr->register_creature_script(CN_RISEN_SQUIRE, &RisenSquireAI::Create);
    pScriptMgr->register_creature_script(CN_UNHOLY_AXE, &UnholyAxeAI::Create);
    pScriptMgr->register_creature_script(CN_UNHOLY_SWORD, &UnholySwordAI::Create);
    pScriptMgr->register_creature_script(CN_UNHOLY_STAFF, &UnholyStaffAI::Create);
    pScriptMgr->register_creature_script(CN_DEATH_KNIGHT_UNDERSTUDY, &DeathKnightUnderstudyAI::Create);
    // ---- Bosses ----
    pScriptMgr->register_creature_script(CN_INSTRUCTOR_RAZUVIOUS, &InstructorRazuviousAI::Create);

    /////////////////////////////////////////////////////////////////////////////////
    ////// The Construct Quarter
    // ---- Trash ----
    pScriptMgr->register_creature_script(CN_PATCHWORK_GOLEM, &PatchworkGolemAI::Create);
    pScriptMgr->register_creature_script(CN_BILE_RETCHER, &BileRetcherAI::Create);
    pScriptMgr->register_creature_script(CN_SEWAGE_SLIME, &SewageSlimeAI::Create);
    pScriptMgr->register_creature_script(CN_EMBALMING_SLIME, &EmbalmingSlimeAI::Create);
    pScriptMgr->register_creature_script(CN_MAD_SCIENTIST, &MadScientistAI::Create);
    pScriptMgr->register_creature_script(CN_LIVING_MONSTROSITY, &LivingMonstrosityAI::Create);
    pScriptMgr->register_creature_script(CN_LIGHTNING_TOTEM, &LightningTotemAI::Create);
    pScriptMgr->register_creature_script(CN_STITCHED_COLOSSUS, &StitchedColossusAI::Create);
    pScriptMgr->register_creature_script(CN_MARAUDING_GEIST, &MaraudingGeistAI::Create);

    // ---- Abomination Wing ----
    pScriptMgr->register_creature_script(STICKED_SPEWER, &StickedSpewerAI::Create);
    pScriptMgr->register_creature_script(CN_SURGICAL_ASSISTANT, &SurgicalAssistantAI::Create);
    pScriptMgr->register_creature_script(CN_SLUDGE_BELCHER, &StickedSpewerAI::Create);
    // BOSS'S
    pScriptMgr->register_creature_script(CN_PATCHWERK, &PatchwerkAI::Create);
    pScriptMgr->register_creature_script(CN_GROBBULUS, &GrobbulusAI::Create);
    pScriptMgr->register_creature_script(CN_GLUTH, &GluthAI::Create);

    // ---- Deathknight Wing ----
    pScriptMgr->register_creature_script(CN_BONY_CONSTRUCT, &BonyConstructAI::Create);
    pScriptMgr->register_creature_script(CN_DEATH_LORD, &DeathLordAI::Create);
    // BOSS'S
    //pScriptMgr->register_creature_script(CN_INSTRUCTOR_RAZUVIOUS, &RazuviousAI::Create);
    // The Four Horsemen:
    pScriptMgr->register_creature_script(CN_THANE_KORTHAZZ, &KorthazzAI::Create);
    pScriptMgr->register_creature_script(CN_LADY_BLAUMEUX, &BlaumeuxAI::Create);
    pScriptMgr->register_creature_script(CN_SIR_ZELIEK, &ZeliekAI::Create);
    //pScriptMgr->register_creature_script(CN_Baron_Rivendare_4H, &RivendareAI::Create);

    // ---- Frostwyrm Lair ---- > Sapphiron Encounter:
    pScriptMgr->register_creature_script(CN_FROST_BREATH_TRIGGER, &FrostBreathTriggerAI::Create);
    pScriptMgr->register_creature_script(CN_FROST_BREATH_TRIGGER2, &FrostBreathTrigger2AI::Create);
    pScriptMgr->register_creature_script(CN_FROST_BREATH_TRIGGER3, &FrostBreathTrigger3AI::Create);
    pScriptMgr->register_creature_script(CN_CHILL_TRIGGER, &ChillTriggerAI::Create);
    pScriptMgr->register_creature_script(CN_SAPPHIRON, &SapphironAI::Create);

    // ---- Frostwyrm Lair ---- > Kel'thuzad Encounter:
    pScriptMgr->register_creature_script(CN_THE_LICH_KING , &TheLichKingAI::Create);
    pScriptMgr->register_creature_script(CN_SOLDIER_OF_THE_FROZEN_WASTES , &SoldierOfTheFrozenWastesAI::Create);
    pScriptMgr->register_creature_script(CN_UNSTOPPABLE_ABOMINATION , &UnstoppableAbominationAI::Create);
    pScriptMgr->register_creature_script(CN_SOUL_WEAVER , &SoulWeaverAI::Create);
    pScriptMgr->register_creature_script(CN_GUARDIAN_OF_ICECROWN , &GuardianOfIcecrownAI::Create);
    pScriptMgr->register_creature_script(CN_KELTHUZAD, &KelthuzadAI::Create);

    // ---- Go Scripts ---- >
}
