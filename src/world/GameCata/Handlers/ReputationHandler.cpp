/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Map/MapMgr.h"
#include "Server/WorldSession.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Management/Group.h"
#include "Objects/ObjectMgr.h"

//\todo Rewrite for cata - after this all functions are copied from wotlk

enum FactionFlags
{
    FACTION_FLAG_VISIBLE = 0x01,
    FACTION_FLAG_AT_WAR = 0x02,
    FACTION_FLAG_HIDDEN = 0x04,
    FACTION_FLAG_FORCED_INVISIBLE = 0x08,     // if both ACTION_FLAG_VISIBLE and FACTION_FLAG_FORCED_INVISIBLE are set, client crashes!
    FACTION_FLAG_DISABLE_ATWAR = 0x10,     // disables AtWar button for client, but you can be in war with the faction
    FACTION_FLAG_INACTIVE = 0x20,
    FACTION_FLAG_RIVAL = 0x40      // only Scryers and Aldor have this flag
};

Standing Player::GetReputationRankFromStanding(int32 standing)
{
    if (standing >= 42000)
        return STANDING_EXALTED;
    else if (standing >= 21000)
        return STANDING_REVERED;
    else if (standing >= 9000)
        return STANDING_HONORED;
    else if (standing >= 3000)
        return STANDING_FRIENDLY;
    else if (standing >= 0)
        return STANDING_NEUTRAL;
    else if (standing > -3000)
        return STANDING_UNFRIENDLY;
    else if (standing > -6000)
        return STANDING_HOSTILE;
    return STANDING_HATED;
}

inline bool CanToggleAtWar(uint8 flag) { return (flag & FACTION_FLAG_DISABLE_ATWAR) == 0; }
inline bool AtWar(uint8 flag) { return (flag & FACTION_FLAG_AT_WAR) != 0; }
inline bool ForcedInvisible(uint8 flag) { return (flag & FACTION_FLAG_FORCED_INVISIBLE) != 0; }
inline bool Visible(uint8 flag) { return (flag & FACTION_FLAG_VISIBLE) != 0; }
inline bool Hidden(uint8 flag) { return (flag & FACTION_FLAG_HIDDEN) != 0; }
inline bool Inactive(uint8 flag) { return (flag & FACTION_FLAG_INACTIVE) != 0; }

inline bool SetFlagAtWar(uint8 & flag, bool set)
{
    if (set && !AtWar(flag))
        flag |= FACTION_FLAG_AT_WAR;
    else if (!set && AtWar(flag))
        flag &= ~FACTION_FLAG_AT_WAR;
    else
        return false;

    return true;
}

inline bool SetFlagVisible(uint8 & flag, bool set)
{
    if (ForcedInvisible(flag) || Hidden(flag))
        return false;
    else if (set && !Visible(flag))
        flag |= FACTION_FLAG_VISIBLE;
    else if (!set && Visible(flag))
        flag &= ~FACTION_FLAG_VISIBLE;
    else
        return false;

    return true;
}

inline bool SetFlagInactive(uint8 & flag, bool set)
{
    if (set && !Inactive(flag))
        flag |= FACTION_FLAG_INACTIVE;
    else if (!set && Inactive(flag))
        flag &= ~FACTION_FLAG_INACTIVE;
    else
        return false;

    return true;
}

inline bool RankChanged(int32 standing, int32 change)
{
    return (Player::GetReputationRankFromStanding(standing) != Player::GetReputationRankFromStanding(standing + change));
}

inline bool RankChangedFlat(int32 standing, int32 newStanding)
{
    return (Player::GetReputationRankFromStanding(standing) != Player::GetReputationRankFromStanding(newStanding));
}

void Player::smsg_InitialFactions()
{
    WorldPacket data(SMSG_INITIALIZE_FACTIONS, 764);
    data << uint32(128);
    FactionReputation* rep;
    for (uint8 i = 0; i < 128; ++i)
    {
        rep = reputationByListId[i];
        if (rep == nullptr)
        {
            data << uint8(0);
            data << uint32(0);
        }
        else
        {
            data << rep->flag;
            data << rep->CalcStanding();
        }
    }
    m_session->SendPacket(&data);
}

void Player::_InitialReputation()
{
    DBC::Structures::FactionEntry const* f;
    for (uint32 i = 0; i < sFactionStore.GetNumRows(); i++)
    {
        f = sFactionStore.LookupEntry(i);
        AddNewFaction(f, 0, true);
    }
}

int32 Player::GetStanding(uint32 Faction)
{
    ReputationMap::iterator itr = m_reputation.find(Faction);
    if (itr != m_reputation.end())
        return itr->second->standing;
    return 0;
}

int32 Player::GetBaseStanding(uint32 Faction)
{
    ReputationMap::iterator itr = m_reputation.find(Faction);
    if (itr != m_reputation.end())
        return itr->second->baseStanding;
    return 0;
}

void Player::SetStanding(uint32 Faction, int32 Value)
{
    const int32 minReputation = -42000;      //   0/36000 Hated
    const int32 exaltedReputation = 42000;   //   0/1000  Exalted
    const int32 maxReputation = 42999;       // 999/1000  Exalted
    int32 newValue = Value;
    DBC::Structures::FactionEntry const* f = sFactionStore.LookupEntry(Faction);
    if (f == nullptr || f->RepListId < 0)
        return;
    ReputationMap::iterator itr = m_reputation.find(Faction);

    if (newValue < minReputation)
        newValue = minReputation;
    else if (newValue > maxReputation)
        newValue = maxReputation;

    if (itr == m_reputation.end())
    {
        if (!AddNewFaction(f, newValue, false))
            return;

        itr = m_reputation.find(Faction);

        if (itr->second->standing >= 42000)   // check if we are exalted now
            m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION, 1, 0, 0);   // increment # of exalted

        m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION, f->ID, itr->second->standing, 0);
        UpdateInrangeSetsBasedOnReputation();
        OnModStanding(f, itr->second);
    }
    else
    {
        // Assign it.
        if (RankChangedFlat(itr->second->standing, newValue))
        {
            if (itr->second->standing - newValue >= exaltedReputation) // somehow we lost exalted status
                m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION, -1, 0, 0); // decrement # of exalted
            else if (newValue >= exaltedReputation) // check if we are exalted now
                m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION, 1, 0, 0); // increment # of exalted

            itr->second->standing = newValue;
            UpdateInrangeSetsBasedOnReputation();

            m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION, f->ID, Value, 0);
        }
        else
            itr->second->standing = newValue;

        OnModStanding(f, itr->second);
    }
}

Standing Player::GetStandingRank(uint32 Faction)
{
    return Standing(GetReputationRankFromStanding(GetStanding(Faction)));
}

bool Player::IsHostileBasedOnReputation(DBC::Structures::FactionEntry const* dbc)
{
    if (dbc->RepListId < 0 || dbc->RepListId >= 128)
        return false;

    FactionReputation* rep = reputationByListId[dbc->RepListId];
    if (rep == nullptr)
        return false;

    // forced reactions take precedence
    std::map<uint32, uint32>::iterator itr = m_forcedReactions.find(dbc->ID);
    if (itr != m_forcedReactions.end())
        return (itr->second <= STANDING_HOSTILE);

    return (AtWar(rep->flag) || GetReputationRankFromStanding(rep->standing) <= STANDING_HOSTILE);
}

void Player::ModStanding(uint32 Faction, int32 Value)
{
    const int32 minReputation = -42000;      //   0/36000 Hated
    const int32 exaltedReputation = 42000;   //   0/1000  Exalted
    const int32 maxReputation = 42999;       // 999/1000  Exalted

                                             // WE ARE THE CHAMPIONS MY FRIENDS! WE KEEP ON FIGHTING 'TILL THE END!
                                             //
                                             // If we are in a lvl80 instance or heroic, or raid and we have a championing tabard on,
                                             // we get reputation after the faction determined by the worn tabard.
    if ((GetMapMgr()->GetMapInfo()->minlevel == 80 || (GetMapMgr()->iInstanceMode == MODE_HEROIC && GetMapMgr()->GetMapInfo()->minlevel_heroic == 80)) && ChampioningFactionID != 0)
        Faction = ChampioningFactionID;

    DBC::Structures::FactionEntry const* f = sFactionStore.LookupEntry(Faction);
    int32 newValue = Value;
    if (f == nullptr || f->RepListId < 0)
        return;
    ReputationMap::iterator itr = m_reputation.find(Faction);

    if (itr == m_reputation.end())
    {
        if (newValue < minReputation)
            newValue = minReputation;
        else if (newValue > maxReputation)
            newValue = maxReputation;

        if (!AddNewFaction(f, newValue, false))
            return;

        itr = m_reputation.find(Faction);

        if (itr->second->standing >= 42000)   // check if we are exalted now
            m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION, 1, 0, 0);   // increment # of exalted

        m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION, f->ID, itr->second->standing, 0);

        UpdateInrangeSetsBasedOnReputation();
        OnModStanding(f, itr->second);
    }
    else
    {
        if (pctReputationMod > 0)
        {
            newValue = Value + (Value * pctReputationMod / 100);
        }
        // Increment it.
        if (RankChanged(itr->second->standing, newValue))
        {
            itr->second->standing += newValue;
            UpdateInrangeSetsBasedOnReputation();

            m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION, f->ID, itr->second->standing, 0);
            if (itr->second->standing >= exaltedReputation) // check if we are exalted now
                m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION, 1, 0, 0); // increment # of exalted
            else if (itr->second->standing - newValue >= exaltedReputation) // somehow we lost exalted status
                m_achievementMgr.UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION, -1, 0, 0); // decrement # of exalted
        }
        else
            itr->second->standing += newValue;

        if (itr->second->standing < minReputation)
            itr->second->standing = minReputation;
        else if (itr->second->standing > maxReputation)
            itr->second->standing = maxReputation;
        OnModStanding(f, itr->second);
    }
}

void Player::SetAtWar(uint32 Faction, bool Set)
{
    if (Faction >= 128)
        return;

    FactionReputation* rep = reputationByListId[Faction];
    if (rep == nullptr)
        return;

    if (GetReputationRankFromStanding(rep->standing) <= STANDING_HOSTILE && !Set)     // At this point we have to be at war.
        return;

    if (!CanToggleAtWar(rep->flag))
        return;

    if (SetFlagAtWar(rep->flag, Set))
    {
        UpdateInrangeSetsBasedOnReputation();
    }
}

void WorldSession::HandleSetAtWarOpcode(WorldPacket& recvData)
{
    uint32 id;
    uint8 state;

    recvData >> id;
    recvData >> state;

    _player->SetAtWar(id, (state == 1));
}

void Player::UpdateInrangeSetsBasedOnReputation()
{
    for (const auto& itr : getInRangeObjectsSet())
    {
        if (!itr || !itr->IsUnit())
            continue;

        Unit * pUnit = static_cast<Unit*>(itr);
        if (pUnit->m_factionDBC == nullptr || pUnit->m_factionDBC->RepListId < 0)
            continue;

        bool rep_value = IsHostileBasedOnReputation(pUnit->m_factionDBC);
        bool enemy_current = isObjectInInRangeOppositeFactionSet(pUnit);

        if (rep_value && !enemy_current)   // We are now enemies.
            addInRangeOppositeFaction(pUnit);
        else if (!rep_value && enemy_current)
            removeObjectFromInRangeOppositeFactionSet(pUnit);
    }
}

void Player::Reputation_OnKilledUnit(Unit* pUnit, bool InnerLoop)
{
    // add rep for on kill
    if (!pUnit->IsCreature() || pUnit->IsPet() || pUnit->isCritter())
        return;

    Group* m_Group = GetGroup();

    // Why would this be accessed if the group didn't exist?
    if (!InnerLoop && m_Group != nullptr)
    {
        /* loop the rep for group members */
        m_Group->getLock().Acquire();
        GroupMembersSet::iterator it;
        for (uint32 i = 0; i < m_Group->GetSubGroupCount(); i++)
        {
            for (it = m_Group->GetSubGroup(i)->GetGroupMembersBegin(); it != m_Group->GetSubGroup(i)->GetGroupMembersEnd(); ++it)
            {
                if ((*it)->m_loggedInPlayer && (*it)->m_loggedInPlayer->isInRange(this, 100.0f))
                    (*it)->m_loggedInPlayer->Reputation_OnKilledUnit(pUnit, true);
            }
        }
        m_Group->getLock().Release();
        return;
    }

    uint32 team = GetTeam();
    ReputationModifier* modifier = objmgr.GetReputationModifier(pUnit->GetEntry(), pUnit->m_factionDBC->ID);
    if (modifier != nullptr)
    {
        // Apply this data.
        for (std::vector<ReputationMod>::iterator itr = modifier->mods.begin(); itr != modifier->mods.end(); ++itr)
        {
            if (!(*itr).faction[team])
                continue;

            /* rep limit? */
            if (!IS_INSTANCE(GetMapId()) || (IS_INSTANCE(GetMapId()) && this->iInstanceType != MODE_HEROIC))
            {
                if ((*itr).replimit)
                {
                    if (GetStanding((*itr).faction[team]) >= (int32)(*itr).replimit)
                        continue;
                }
            }
            ModStanding(itr->faction[team], float2int32(itr->value * worldConfig.getFloatRate(RATE_KILLREPUTATION)));
        }
    }
    else
    {
        if (IS_INSTANCE(GetMapId()) && objmgr.HandleInstanceReputationModifiers(this, pUnit))
            return;

        if (pUnit->m_factionDBC->RepListId < 0)
            return;

        int32 change = int32(-5.0f * worldConfig.getFloatRate(RATE_KILLREPUTATION));
        ModStanding(pUnit->m_factionDBC->ID, change);
    }
}

void Player::Reputation_OnTalk(DBC::Structures::FactionEntry const* dbc)
{
    // set faction visible if not visible
    if (dbc == nullptr || dbc->RepListId < 0)
        return;

    FactionReputation* rep = reputationByListId[dbc->RepListId];
    if (rep == nullptr)
        return;

    if (SetFlagVisible(rep->flag, true) && IsInWorld())
    {
        m_session->OutPacket(SMSG_SET_FACTION_VISIBLE, 4, &dbc->RepListId);
    }
}

void Player::SetFactionInactive(uint32 faction, bool /*set*/)
{
    FactionReputation* rep = reputationByListId[faction];
    if (rep == nullptr)
        return;
}

void WorldSession::HandleSetFactionInactiveOpcode(WorldPacket& recvData)
{
    uint32 id;
    uint8 inactive;

    recvData >> id;
    recvData >> inactive;

    _player->SetFactionInactive(id, (inactive == 1));
}

bool Player::AddNewFaction(DBC::Structures::FactionEntry const* dbc, int32 standing, bool base)    // if (base) standing = baseRepValue
{
    if (dbc == nullptr || dbc->RepListId < 0)
        return false;

    uint32 RaceMask = getRaceMask();
    uint32 ClassMask = getClassMask();
    for (uint8 i = 0; i < 4; i++)
    {
        if ((dbc->RaceMask[i] & RaceMask || (dbc->RaceMask[i] == 0 && dbc->ClassMask[i] != 0)) && (dbc->ClassMask[i] & ClassMask || dbc->ClassMask[i] == 0))
        {
            FactionReputation* rep = new FactionReputation;
            rep->flag = static_cast<uint8>(dbc->repFlags[i]);
            rep->baseStanding = dbc->baseRepValue[i];
            rep->standing = (base) ? dbc->baseRepValue[i] : standing;
            m_reputation[dbc->ID] = rep;
            reputationByListId[dbc->RepListId] = rep;
            return true;
        }
    }
    return false;
}

void Player::OnModStanding(DBC::Structures::FactionEntry const* dbc, FactionReputation* rep)
{
    if (SetFlagVisible(rep->flag, true) && IsInWorld())
    {
        m_session->OutPacket(SMSG_SET_FACTION_VISIBLE, 4, &dbc->RepListId);
    }

    SetFlagAtWar(rep->flag, (GetReputationRankFromStanding(rep->standing) <= STANDING_HOSTILE));

    if (Visible(rep->flag) && IsInWorld())
    {
        WorldPacket data(SMSG_SET_FACTION_STANDING, 17);
        data << uint32(0);
        data << uint8(1);   //count
        data << uint32(rep->flag);
        data << dbc->RepListId;
        data << rep->CalcStanding();
        m_session->SendPacket(&data);
    }
}

uint32 Player::GetExaltedCount(void)
{
    const int32 exaltedReputation = 42000;
    uint32 ec = 0;

    ReputationMap::iterator itr = m_reputation.begin();
    while (itr != m_reputation.end())
    {
        if (itr->second->standing >= exaltedReputation)
            ++ec;
        ++itr;
    }
    return ec;
}
