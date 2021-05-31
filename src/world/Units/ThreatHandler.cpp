/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

void ThreatReference::addThreat(float amount)
{
    if (amount == 0.0f)
        return;
    _baseAmount = std::max<float>(_baseAmount + amount, 0.0f);

    _mgr.heapNotifyChanged();

    _mgr._needClientUpdate = true;
}

void ThreatReference::scaleThreat(float factor)
{
    if (factor == 1.0f)
        return;
    _baseAmount *= factor;

    _mgr.heapNotifyChanged();

    _mgr._needClientUpdate = true;
}

void ThreatReference::updateOffline()
{
    bool const shouldBeOff = shouldBeOffline();
    if (shouldBeOff == isOffline())
        return;

    if (shouldBeOff)
    {
        _online = ONLINE_STATE_OFFLINE;
        _mgr.heapNotifyChanged();
        _mgr.sendRemoveToClients(_victim);
    }
    else
    {
        _online = shouldBeSuppressed() ? ONLINE_STATE_SUPPRESSED : ONLINE_STATE_ONLINE;
        _mgr.heapNotifyChanged();
    }
}

bool ThreatReference::shouldBeOffline() const
{
    if (!_owner->canSee(_victim))
        return true;

    if (!_victim)
        return true;

    if (!_victim->isAlive())
        return true;

    if (!_victim->isInAccessiblePlaceFor(_owner))
        return true;

    // we cannot attack in evade mode
    if (_owner->isInEvadeMode())
        return true;

    // or if enemy is in evade mode
    if (_victim->GetTypeFromGUID() == TYPEID_UNIT && _victim->ToCreature()->isInEvadeMode())
        return true;

    if (_victim == _owner)
        return true;

    if (!flagsAllowFighting(_owner, _victim) || !flagsAllowFighting(_victim, _owner))
        return true;

    return false;
}

bool ThreatReference::shouldBeSuppressed() const
{
    if (isTaunting()) // a taunting victim can never be suppressed
        return false;
    
    if (_owner->isCreature())
        if (_victim->SchoolImmunityList[static_cast<Creature*>(_owner)->BaseAttackType] != 0)
            return true;
    
    // check if we have any aura that suppresses us
    for (uint32_t x = MAX_NEGATIVE_AURAS_EXTEDED_START; x < MAX_NEGATIVE_AURAS_EXTEDED_END; ++x)
    {
        if (_victim->m_auras[x])
        {
            for (uint8_t y = 0; y < 3; ++y)
            {
                switch (_victim->m_auras[x]->getSpellInfo()->getEffectApplyAuraName(y))
                {
                case SPELL_AURA_MOD_STUN:
                case SPELL_AURA_MOD_CONFUSE:
                    return true;
                    break;
                }
            }
        }
    }

    return false;
}

void ThreatReference::updateTauntState(TauntState state)
{
    // Check for SPELL_AURA_MOD_DETAUNT
    if (state < TAUNT_STATE_TAUNT && _victim->hasAuraWithAuraEffect(SPELL_AURA_MOD_DETAUNT))
        state = TAUNT_STATE_DETAUNT;

    if (state == _taunted)
        return;

    std::swap(state, _taunted);

    _mgr.heapNotifyChanged();

    _mgr._needClientUpdate = true;
}

/*static*/ bool ThreatReference::flagsAllowFighting(Unit const* a, Unit const* b)
{
    if (a->hasUnitFlags(UNIT_FLAG_PVP_ATTACKABLE))
    {
        if (b->hasUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT))
            return false;
    }
    else
    {
        if (b->hasUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT))
            return false;
    }
    return true;
}

void ThreatReference::clearThreat()
{
    _mgr.clearThreat(this);
}

void ThreatReference::unregisterAndFree()
{
    _owner->getThreatManager().purgeThreatListRef(_victim->getGuid());
    _victim->getThreatManager().purgeThreatenedByMeRef(_owner->getGuid());
    delete this;
}

ThreatManager::ThreatManager(Unit* owner) : _owner(owner), _ownerCanHaveThreatList(false), _needClientUpdate(false), _updateTimer(THREAT_UPDATE_INTERVAL), _currentVictimRef(nullptr), _fixateRef(nullptr)
{
    for (int8 i = 0; i < TOTAL_SPELL_SCHOOLS; ++i)
        _singleSchoolModifiers[i] = 1.0f;
}

ThreatManager::~ThreatManager()
{
    ASSERT(_myThreatListEntries.empty());
    ASSERT(_threatenedByMe.empty());
}

void ThreatManager::initialize()
{
    _ownerCanHaveThreatList = ThreatManager::canHaveThreatList(_owner);
}

/*static*/ bool ThreatManager::canHaveThreatList(Unit const* pUnit)
{
    // only creatures can have threat list
    if (!pUnit->isCreature())
        return false;

    // pets, totems cannot have threat list
    if (pUnit->isPet() || pUnit->isTotem())
        return false;

    // summons cannot have a threat list if they were summoned by a player
    if (pUnit->isSummon())
        if (Unit* summoner = pUnit->GetMapMgr()->GetUnit(WoWGuid::getGuidLowPartFromUInt64(pUnit->getSummonedByGuid())))
            if (summoner->isPlayer())
                return false;

   return true;
}

void ThreatManager::update(uint32_t tdiff)
{
    if (!canHaveThreatList() || isThreatListEmpty(true))
        return;

    if (_updateTimer <= tdiff)
    {
        updateVictim();
        _updateTimer = THREAT_UPDATE_INTERVAL;
    }
    else
        _updateTimer -= tdiff;
}

Unit* ThreatManager::getCurrentVictim()
{
    if (!_currentVictimRef || _currentVictimRef->shouldBeOffline())
        updateVictim();
    ASSERT(!_currentVictimRef || _currentVictimRef->isAvailable());
    return _currentVictimRef ? _currentVictimRef->getVictim() : nullptr;
}

Unit* ThreatManager::getSecondMostHated()
{
    if (getThreatListSize() >= 2)
    {
        ThreatReference* ref = (*std::next(_sortedThreatList.begin()));
        if (ref)
            return ref->getOwner();
        else
            return nullptr;
    }
    else
        return getCurrentVictim();
}

Unit* ThreatManager::getLastVictim() const
{
    if (_currentVictimRef && !_currentVictimRef->shouldBeOffline())
        return _currentVictimRef->getVictim();
    return nullptr;
}

Unit* ThreatManager::getAnyTarget() const
{
    for (ThreatReference const* ref : _sortedThreatList)
        if (!ref->isOffline())
            return ref->getVictim();
    return nullptr;
}

void ThreatManager::updateVictim()
{
    ThreatReference const* const newVictim = reselectVictim();
    bool const newHighest = newVictim && (newVictim != _currentVictimRef);

    _currentVictimRef = newVictim;
    if (newHighest || _needClientUpdate)
    {
        sendThreatListToClients(newHighest);
        _needClientUpdate = false;  
    }

    // Tell the AIInterface to chase our target because our Victim has changed
    if (newHighest)
        _owner->GetAIInterface()->updateVictim(_currentVictimRef->getVictim());
}

ThreatReference const* ThreatManager::reselectVictim()
{
    if (_sortedThreatList.empty())
        return nullptr;

    for (auto & pair : _myThreatListEntries)
        pair.second->updateOffline(); // AI notifies are processed in ::UpdateVictim caller

    // fixated target is always preferred
    if (_fixateRef && _fixateRef->isAvailable())
        return _fixateRef;

    ThreatReference const* oldVictimRef = _currentVictimRef;
    if (oldVictimRef && oldVictimRef->isOffline())
        oldVictimRef = nullptr;
    // in 99% of cases - we won't need to actually look at anything beyond the first element
    ThreatReference const* highest = _sortedThreatList.front();

    // if the highest reference is offline, the entire list is offline, and we indicate this
    if (!highest->isAvailable())
        return nullptr;
    // if we have no old victim, or old victim is still highest, then highest is our target and we're done
    if (!oldVictimRef || highest == oldVictimRef)
        return highest;
    // if highest threat doesn't break 110% of old victim, nothing below it is going to do so either; new victim = old victim and done
    if (!ThreatManager::compareReferencesLT(oldVictimRef, highest, 1.1f))
        return oldVictimRef;
    // if highest threat breaks 130%, it's our new target regardless of range (and we're done)
    if (ThreatManager::compareReferencesLT(oldVictimRef, highest, 1.3f))
        return highest;
    // if it doesn't break 130%, we need to check if it's melee - if yes, it breaks 110% (we checked earlier) and is our new target
    if (_owner->canReachWithAttack(highest->_victim))
        return highest;
    // If we get here, highest threat is ranged, but below 130% of current - there might be a melee that breaks 110% below us somewhere, so now we need to actually look at the next highest element
    // luckily, this is a heap, so getting the next highest element is O(log n), and we're just gonna do that repeatedly until we've seen enough targets (or find a target)
    auto it = _sortedThreatList.begin(), end = _sortedThreatList.end();
    while (it != end)
    {
        ThreatReference const* next = *it;
        // if we've found current victim, we're done (nothing above is higher, and nothing below can be higher)
        if (next == oldVictimRef)
            return next;
        // if next isn't above 110% threat, then nothing below it can be either - we're done, old victim stays
        if (!ThreatManager::compareReferencesLT(oldVictimRef, next, 1.1f))
            return oldVictimRef;
        // if next is melee, he's above 110% and our new victim
        if (_owner->canReachWithAttack(next->_victim))
            return next;
        // otherwise the next highest target may still be a melee above 110% and we need to look further
        ++it;
    }
    // we should have found the old victim at some point in the loop above, so execution should never get to this point
    return nullptr;
}

// returns true if a is LOWER on the threat list than b
/*static*/ bool ThreatManager::compareReferencesLT(ThreatReference const* a, ThreatReference const* b, float aWeight)
{
    if (a->_online != b->_online) // online state precedence (ONLINE > SUPPRESSED > OFFLINE)
        return a->_online < b->_online;
    if (a->_taunted != b->_taunted) // taunt state precedence (TAUNT > NONE > DETAUNT)
        return a->_taunted < b->_taunted;
    return (a->getThreat()*aWeight < b->getThreat());
}

void ThreatManager::fixateTarget(Unit* target)
{
    if (target)
    {
        auto it = _myThreatListEntries.find(target->getGuid());
        if (it != _myThreatListEntries.end())
        {
            _fixateRef = it->second;
            return;
        }
    }
    _fixateRef = nullptr;
}

Unit* ThreatManager::getFixateTarget() const
{
    if (_fixateRef)
        return _fixateRef->getVictim();
    else
        return nullptr;
}

void ThreatManager::updateRedirectInfo()
{
    _redirectInfo.clear();
    uint32_t totalPct = 0;
    for (auto const& pair : _redirectRegistry) // (spellid, victim -> pct)
        for (auto const& victimPair : pair.second) // (victim,pct)
        {
            uint32_t thisPct = std::min<uint32_t>(100 - totalPct, victimPair.second);
            if (thisPct > 0)
            {
                _redirectInfo.push_back({ victimPair.first, thisPct });
                totalPct += thisPct;
                ASSERT(totalPct <= 100);
                if (totalPct == 100)
                    return;
            }
        }
}

void ThreatManager::forwardThreatForAssistingMe(Unit* assistant, float baseAmount, SpellInfo const* spell, bool ignoreModifiers)
{
    if (spell && spell->getAttributesEx() == ATTRIBUTESEX_NO_INITIAL_AGGRO) // shortcut, none of the calls would do anything
        return;

    if (_threatenedByMe.empty())
        return;

    std::vector<Creature*> canBeThreatened, cannotBeThreatened;
    for (auto const& pair : _threatenedByMe)
    {
        Creature* owner = pair.second->getOwner();
        if (!owner->hasUnitFlags(UNIT_FLAG_PLAYER_CONTROLLED_CREATURE))
            canBeThreatened.push_back(owner);
        else
            cannotBeThreatened.push_back(owner);
    }

    if (!canBeThreatened.empty()) // targets under CC cannot gain assist threat - split evenly among the rest
    {
        float const perTarget = baseAmount / canBeThreatened.size();
        for (Creature* threatened : canBeThreatened)
            threatened->getThreatManager().addThreat(assistant, perTarget, spell, ignoreModifiers);
    }

    for (Creature* threatened : cannotBeThreatened)
        threatened->getThreatManager().addThreat(assistant, 0.0f, spell, true);
}

void ThreatManager::removeMeFromThreatLists()
{
    while (!_threatenedByMe.empty())
    {
        auto& ref = _threatenedByMe.begin()->second;
        ref->_mgr.clearThreat(_owner);
    }
}

/*static*/ float ThreatManager::calculateModifiedThreat(float threat, Unit* victim, SpellInfo const* spell)
{
    // modifiers by spell
    if (spell)
    {
        if (spell->custom_ThreatForSpell)
            if (spell->custom_ThreatForSpellCoef != 1.0f)
                threat *= spell->custom_ThreatForSpellCoef;
    }

    // modifiers by effect school
    ThreatManager const& victimMgr = victim->getThreatManager();
    SchoolMask const mask = spell ? SchoolMask(spell->getSchoolMask()) : SCHOOL_MASK_NORMAL;
    switch (mask)
    {
        case SCHOOL_MASK_NORMAL:
            threat *= victimMgr._singleSchoolModifiers[SCHOOL_NORMAL];
            break;
        case SCHOOL_MASK_HOLY:
            threat *= victimMgr._singleSchoolModifiers[SCHOOL_HOLY];
            break;
        case SCHOOL_MASK_FIRE:
            threat *= victimMgr._singleSchoolModifiers[SCHOOL_FIRE];
            break;
        case SCHOOL_MASK_NATURE:
            threat *= victimMgr._singleSchoolModifiers[SCHOOL_NATURE];
            break;
        case SCHOOL_MASK_FROST:
            threat *= victimMgr._singleSchoolModifiers[SCHOOL_FROST];
            break;
        case SCHOOL_MASK_SHADOW:
            threat *= victimMgr._singleSchoolModifiers[SCHOOL_SHADOW];
            break;
        case SCHOOL_MASK_ARCANE:
            threat *= victimMgr._singleSchoolModifiers[SCHOOL_ARCANE];
            break;
        default:
        {
            auto it = victimMgr._multiSchoolModifiers.find(mask);
            if (it != victimMgr._multiSchoolModifiers.end())
            {
                threat *= it->second;
                break;
            }
            float mod = static_cast<float>(victim->GetGeneratedThreatModifyer(mask));
            victimMgr._multiSchoolModifiers[mask] = mod;
            threat *= mod;
            break;
        }
    }
    return threat;
}

bool ThreatManager::isThreatenedBy(uint64_t const& who, bool includeOffline) const
{
    auto it = _myThreatListEntries.find(who);
    if (it == _myThreatListEntries.end())
        return false;
    return (includeOffline || it->second->isAvailable());
}
bool ThreatManager::isThreatenedBy(Unit const* who, bool includeOffline) const { return isThreatenedBy(who->getGuid(), includeOffline); }

bool ThreatManager::isThreatListEmpty(bool includeOffline) const
{
    if (includeOffline)
        return _sortedThreatList.empty();
    for (ThreatReference const* ref : _sortedThreatList)
        if (ref->isAvailable())
            return false;
    return true;
}

void ThreatManager::evaluateSuppressed(bool canExpire)
{
    for (auto const& pair : _threatenedByMe)
    {
        bool const shouldBeSuppressed = pair.second->shouldBeSuppressed();
        if (pair.second->isOnline() && shouldBeSuppressed)
        {
            pair.second->_online = ThreatReference::ONLINE_STATE_SUPPRESSED;
            heapNotifyChanged();
        }
        else if (canExpire && pair.second->isSuppressed() && !shouldBeSuppressed)
        {
            pair.second->_online = ThreatReference::ONLINE_STATE_ONLINE;
            heapNotifyChanged();
        }
    }
}

void ThreatManager::addThreat(Unit* target, float amount, SpellInfo const* spell, bool ignoreModifiers, bool ignoreRedirects)
{
    // Can we even have a Threat List if not return
    if (!canHaveThreatList())
        return;

    if (!getOwner()->isAlive())
        return;

    if (!target)
        return;

    if (spell)
    {
        if (spell->getAttributesEx() == ATTRIBUTESEX_NO_INITIAL_AGGRO)
            return;

        if (!_owner->isInCombat() && spell->getAttributesExC() & ATTRIBUTESEXC_UNK19)
            return;
    }

    // while riding a vehicle, all threat goes to the vehicle, not the pilot
    if (target->isVehicle())
    {
        // \ ToDo
    }

    // apply threat modifiers to the amount
    if (!ignoreModifiers)
        amount = calculateModifiedThreat(amount, target, spell);

    // if we're increasing threat, send some/all of it to redirection targets instead if applicable
    if (!ignoreRedirects && amount > 0.0f)
    {
        auto const& redirInfo = target->getThreatManager()._redirectInfo;
        if (!redirInfo.empty())
        {
            float const origAmount = amount;
            // intentional iteration by index - there's a nested AddThreat call further down that might cause AI calls which might modify redirect info through spells
            for (size_t i = 0; i < redirInfo.size(); ++i)
            {
                auto const pair = redirInfo[i]; // (victim,pct)
                Unit* redirTarget = nullptr;
                auto it = _myThreatListEntries.find(pair.first); // try to look it up in our threat list first (faster)
                if (it != _myThreatListEntries.end())
                    redirTarget = it->second->_victim;
                else
                    redirTarget = target->GetMapMgrUnit(pair.first);

                if (redirTarget)
                {                 
                    float amountRedirected = (origAmount * static_cast<float>(pair.second) / 100.0f);
                    addThreat(redirTarget, amountRedirected, spell, true, true);
                    amount -= amountRedirected;
                }
            }
        }
    }

    // ok, now we actually apply threat
    // check if we already have an entry - if we do, just increase threat for that entry and we're done
    auto it = _myThreatListEntries.find(target->getGuid());
    if (it != _myThreatListEntries.end())
    {
        ThreatReference* const ref = it->second;
        // SUPPRESSED threat states don't go back to ONLINE until threat is caused by them (retail behavior)
        if (ref->getOnlineState() == ThreatReference::ONLINE_STATE_SUPPRESSED)
            if (!ref->shouldBeSuppressed())
            {
                ref->_online = ThreatReference::ONLINE_STATE_ONLINE;
                heapNotifyChanged();
            }

        ref->updateOffline();
        if (ref->isOnline())
            ref->addThreat(amount);
        return;
    }

    // ok, we're now in combat - create the threat list reference and push it to the respective managers
    ThreatReference* ref = new ThreatReference(this, target);
    putThreatListRef(target->getGuid(), ref);
    target->getThreatManager().putThreatenedByMeRef(_owner->getGuid(), ref);

    ref->updateOffline();
    if (ref->isOnline()) // we only add the threat if the ref is currently available
        ref->addThreat(amount);

    if (!_currentVictimRef)
        updateVictim();
}

float ThreatManager::getThreat(Unit const* who, bool includeOffline) const
{
    auto it = _myThreatListEntries.find(who->getGuid());
    if (it == _myThreatListEntries.end())
        return 0.0f;
    return (includeOffline || it->second->isAvailable()) ? it->second->getThreat() : 0.0f;
}

std::vector<ThreatReference*> ThreatManager::getModifiableThreatList()
{
    std::vector<ThreatReference*> list;
    list.reserve(_myThreatListEntries.size());
    for (auto it = _sortedThreatList.begin(), end = _sortedThreatList.end(); it != end; ++it)
        list.push_back(const_cast<ThreatReference*>(*it));
    return list;
}

void ThreatManager::scaleThreat(Unit* target, float factor)
{
    auto it = _myThreatListEntries.find(target->getGuid());
    if (it != _myThreatListEntries.end())
        it->second->scaleThreat(std::max<float>(factor, 0.0f));
}

void ThreatManager::matchUnitThreatToHighestThreat(Unit* target)
{
    if (_sortedThreatList.empty())
        return;

    auto it = _sortedThreatList.begin(), end = _sortedThreatList.end();
    ThreatReference const* highest = *it;
    if (!highest->isAvailable())
        return;

    if (highest->isTaunting() && ((++it) != end)) // might need to skip this - max threat could be the preceding element (there is only one taunt element)
    {
        ThreatReference const* a = *it;
        if (a->isAvailable() && a->getThreat() > highest->getThreat())
            highest = a;
    }

    addThreat(target, highest->getThreat() - getThreat(target, true), nullptr, true, true);
}

void ThreatManager::tauntUpdate()
{
     _tauntEffects.clear();

    // check if we have any aura that taunts us
    for (uint32_t x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (_owner->m_auras[x])
        {
            for (uint8_t y = 0; y < MAX_SPELL_EFFECTS; ++y)
            {
                if (_owner->m_auras[x]->getSpellInfo()->getEffectApplyAuraName(y) == SPELL_AURA_MOD_TAUNT)
                {
                    _tauntEffects.push_back(_owner->m_auras[x]->getCasterGuid());
                }
            }
        }
    }

    uint32_t state = ThreatReference::TAUNT_STATE_TAUNT;
    std::unordered_map<uint64_t, ThreatReference::TauntState> tauntStates;
    // Only the last taunt effect applied by something still on our threat list is considered
    for (auto it = _tauntEffects.begin(), end = _tauntEffects.end(); it != end; ++it)
        tauntStates[(*it)] = ThreatReference::TauntState(state++);

    for (auto const& pair : _myThreatListEntries)
    {
        auto it = tauntStates.find(pair.first);
        if (it != tauntStates.end())
            pair.second->updateTauntState(it->second);
        else
            pair.second->updateTauntState();
    }
    
    // taunt aura update also re-evaluates all suppressed states (retail behavior)
    evaluateSuppressed(true);
}

void ThreatManager::resetAllThreat()
{
    for (auto const& pair : _myThreatListEntries)
        pair.second->scaleThreat(0.0f);
}

void ThreatManager::clearThreat(Unit* target)
{
    auto it = _myThreatListEntries.find(target->getGuid());
    if (it != _myThreatListEntries.end())
        clearThreat(it->second);
}

void ThreatManager::clearThreat(ThreatReference* ref)
{
    sendRemoveToClients(ref->_victim);
    ref->unregisterAndFree();
    if (!_currentVictimRef)
        updateVictim();
}

void ThreatManager::clearAllThreat()
{
    if (!_myThreatListEntries.empty())
    {
        sendClearAllThreatToClients();
        do
            _myThreatListEntries.begin()->second->unregisterAndFree();
        while (!_myThreatListEntries.empty());
    }
}

void ThreatManager::registerRedirectThreat(uint32_t spellId, uint64_t const& victim, uint32_t pct)
{
    _redirectRegistry[spellId][victim] = pct;
    updateRedirectInfo();
}

void ThreatManager::unregisterRedirectThreat(uint32_t spellId)
{
    auto it = _redirectRegistry.find(spellId);
    if (it == _redirectRegistry.end())
        return;
    _redirectRegistry.erase(it);
    updateRedirectInfo();
}

void ThreatManager::unregisterRedirectThreat(uint32_t spellId, uint64_t const& victim)
{
    auto it = _redirectRegistry.find(spellId);
    if (it == _redirectRegistry.end())
        return;
    auto& victimMap = it->second;
    auto it2 = victimMap.find(victim);
    if (it2 == victimMap.end())
        return;
    victimMap.erase(it2);
    updateRedirectInfo();
}

void ThreatManager::sendClearAllThreatToClients() const
{
    ByteBuffer packedGuidOwner;
    packedGuidOwner.appendPackGUID(_owner->getGuid());

    WorldPacket data(SMSG_THREAT_CLEAR, 8);
    data.append(packedGuidOwner);
    _owner->SendMessageToSet(&data, false);
}

void ThreatManager::sendRemoveToClients(Unit const* victim) const
{
    ByteBuffer packedGuidOwner;
    ByteBuffer packedGuidVictim;
    packedGuidOwner.appendPackGUID(_owner->getGuid());
    packedGuidVictim.appendPackGUID(victim->getGuid());

    WorldPacket data(SMSG_THREAT_REMOVE, 16);
    data.append(packedGuidOwner);
    data.append(packedGuidVictim);
    _owner->SendMessageToSet(&data, false);
}

void ThreatManager::sendThreatListToClients(bool newHighest) const
{
    if ((_owner == nullptr ) || (_currentVictimRef == nullptr))
        return;

    ByteBuffer packedGuidOwner;
    ByteBuffer packedGuidVictim1;
    packedGuidOwner.appendPackGUID(_owner->getGuid());
    packedGuidVictim1.appendPackGUID(_currentVictimRef->getVictim()->getGuid());

    WorldPacket data(static_cast<uint16_t>(newHighest ? SMSG_HIGHEST_THREAT_UPDATE : SMSG_THREAT_UPDATE), (_sortedThreatList.size() + 2) * 8); // guess
    data.append(packedGuidOwner);
    if (newHighest)
        data.append(packedGuidVictim1);
    size_t countPos = data.wpos();
    data << uint32_t(0); // placeholder
    uint32_t count = 0;
    for (ThreatReference const* ref : _sortedThreatList)
    {
        if (!ref->isAvailable())
            continue;
        ByteBuffer packedGuidVictim2;
        packedGuidVictim2.appendPackGUID(ref->getVictim()->getGuid());
        data.append(packedGuidVictim2);
        data << uint32_t(ref->getThreat() * 100);
        ++count;
    }
    data.put<uint32_t>(countPos, count);
    _owner->SendMessageToSet(&data, false);
}

void ThreatManager::putThreatListRef(uint64_t const& guid, ThreatReference* ref)
{
    _needClientUpdate = true;
    auto& inMap = _myThreatListEntries[guid];
    ASSERT(!inMap && "Duplicate threat reference being inserted - memory leak!");
    inMap = ref;
    _sortedThreatList.push_back(ref);
    heapNotifyChanged();
}

void ThreatManager::purgeThreatListRef(uint64_t const& guid)
{
    auto it = _myThreatListEntries.find(guid);
    if (it == _myThreatListEntries.end())
        return;
    ThreatReference* ref = it->second;
    _myThreatListEntries.erase(it);
    _sortedThreatList.remove(ref);
    heapNotifyChanged();

    if (_fixateRef == ref)
        _fixateRef = nullptr;
    if (_currentVictimRef == ref)
        _currentVictimRef = nullptr;
}

void ThreatManager::putThreatenedByMeRef(uint64_t const& guid, ThreatReference* ref)
{
    auto& inMap = _threatenedByMe[guid];
    ASSERT(!inMap && "Duplicate threatened-by-me reference being inserted - memory leak!");
    inMap = ref;
}

void ThreatManager::purgeThreatenedByMeRef(uint64_t const& guid)
{
    auto it = _threatenedByMe.find(guid);
    if (it != _threatenedByMe.end())
        _threatenedByMe.erase(it);
}

void ThreatManager::heapNotifyChanged()
{
    if (_sortedThreatList.size())
        _sortedThreatList.sort([](ThreatReference* a, ThreatReference* b) -> bool { return a->getThreat() > b->getThreat(); });
}
