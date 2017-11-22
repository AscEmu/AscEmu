/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "Creature.h"
#include "Units/Unit.h"
#include "Objects/DynamicObject.h"
#include "Server/Packets/Handlers/HonorHandler.h"
#include "Units/Stats.h"
#include "Management/Battleground/Battleground.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Spell/SpellMgr.h"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/ProcFlags.h"
#include <Spell/Definitions/AuraInterruptFlags.h>
#include "Spell/Definitions/PowerType.h"
#include "Spell/Definitions/SpellEffectTarget.h"
#include "Pet.h"

#define WATER_ELEMENTAL         510
#define WATER_ELEMENTAL_NEW     37994
#define PET_IMP                 416
#define PET_VOIDWALKER          1860
#define PET_SUCCUBUS            1863
#define PET_FELHUNTER           417
#define PET_FELGUARD            17252
#define SHADOWFIEND             19668
#define SPIRITWOLF              29264
#define DANCINGRUNEWEAPON       27893

uint32 Pet::GetAutoCastTypeForSpell(SpellInfo* ent)
{
    switch (ent->getId())
    {
        //////////////////////////////////////////////////////////////////////////////////////////
        // Warlock Pet Spells

        //SPELL_HASH_BLOOD_PACT
        case 6307:
        case 7804:
        case 7805:
        case 11766:
        case 11767:
        case 27268:
        case 47982:
        //SPELL_HASH_FEL_INTELLIGENCE
        case 54424:
        case 57564:
        case 57565:
        case 57566:
        case 57567:
        //SPELL_HASH_AVOIDANCE
        case 32233:
        case 32234:
        case 32600:
        case 62137:
        case 63623:
        case 65220:
        //SPELL_HASH_PARANOIA
        case 19481:
        case 20435:
        case 41002:
            return AUTOCAST_EVENT_ON_SPAWN;
            break;

        //SPELL_HASH_FIRE_SHIELD
        case 134:
        case 2947:
        case 2949:
        case 8316:
        case 8317:
        case 8318:
        case 8319:
        case 11350:
        case 11351:
        case 11770:
        case 11771:
        case 11772:
        case 11773:
        case 11968:
        case 13376:
        case 18968:
        case 19627:
        case 20322:
        case 20323:
        case 20324:
        case 20326:
        case 20327:
        case 27269:
        case 27486:
        case 27489:
        case 30513:
        case 30514:
        case 32749:
        case 32751:
        case 35265:
        case 35266:
        case 36907:
        case 37282:
        case 37283:
        case 37318:
        case 37434:
        case 38732:
        case 38733:
        case 38855:
        case 38893:
        case 38901:
        case 38902:
        case 38933:
        case 38934:
        case 47983:
        case 47998:
        case 61144:
        case 63778:
        case 63779:
        case 71514:
        case 71515:
            return AUTOCAST_EVENT_OWNER_ATTACKED;
            break;

        //SPELL_HASH_PHASE_SHIFT            // Phase Shift
        case 4511:
        case 4630:
        case 8611:
        case 8612:
        case 20329:
        case 29309:
        case 29315:
        //SPELL_HASH_CONSUME_SHADOWS
        case 17767:
        case 17776:
        case 17850:
        case 17851:
        case 17852:
        case 17853:
        case 17854:
        case 17855:
        case 17856:
        case 17857:
        case 17859:
        case 17860:
        case 20387:
        case 20388:
        case 20389:
        case 20390:
        case 20391:
        case 20392:
        case 27272:
        case 27491:
        case 36472:
        case 47987:
        case 47988:
        case 48003:
        case 48004:
        case 49739:
        case 54501:
        //SPELL_HASH_LESSER_INVISIBILITY
        case 3680:
        case 7870:
        case 7880:
        case 12845:
        case 20408:
            return AUTOCAST_EVENT_LEAVE_COMBAT;
            break;

        // SPELL_HASH_WAR_STOMP             // Doomguard spell
        case 45:
        case 11876:
        case 15593:
        case 16727:
        case 16740:
        case 19482:
        case 20549:
        case 24375:
        case 25188:
        case 27758:
        case 28125:
        case 28725:
        case 31408:
        case 31480:
        case 31755:
        case 33707:
        case 35238:
        case 36835:
        case 38682:
        case 38750:
        case 38911:
        case 39313:
        case 40936:
        case 41534:
        case 46026:
        case 56427:
        case 59705:
        case 60960:
        case 61065:
        //SPELL_HASH_SACRIFICE              // We don't want auto sacrifice :P
        case 1050:
        case 7812:
        case 7885:
        case 19438:
        case 19439:
        case 19440:
        case 19441:
        case 19442:
        case 19443:
        case 19444:
        case 19445:
        case 19446:
        case 19447:
        case 20381:
        case 20382:
        case 20383:
        case 20384:
        case 20385:
        case 20386:
        case 22651:
        case 27273:
        case 27492:
        case 30115:
        case 33587:
        case 34661:
        case 47985:
        case 47986:
        case 48001:
        case 48002:
            return AUTOCAST_EVENT_NONE;
            break;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Hunter Pet Spells

        // SPELL_HASH_PROWL
        case 5215:
        case 6783:
        case 8152:
        case 9913:
        case 24450:
        case 24451:
        case 24452:
        case 24453:
        case 24454:
        case 24455:
        case 42932:
        //SPELL_HASH_THUNDERSTOMP           // Thunderstomp
        case 26094:
        case 26189:
        case 26190:
        case 27366:
        case 34388:
        case 61580:
        case 63900:
        //SPELL_HASH_FURIOUS_HOWL           // Furious Howl
        case 3149:
        case 24604:
        case 30636:
        case 35942:
        case 50728:
        case 59274:
        case 64491:
        case 64492:
        case 64493:
        case 64494:
        case 64495:
        //SPELL_HASH_DASH                   // Dash
        case 1850:
        case 9821:
        case 33357:
        case 36589:
        case 43317:
        case 44029:
        case 44531:
        case 61684:
        //SPELL_HASH_DIVE                   // Dive
        case 23145:
        case 23146:
        case 23149:
        case 23150:
        case 29903:
        case 37156:
        case 37588:
        case 40279:
        case 43187:
        //SPELL_HASH_SHELL_SHIELD           // Shell Shield
        case 26064:
        case 26065:
        case 40087:
        case 46327:
            return AUTOCAST_EVENT_NONE;
            break;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Mage Pet Spells

        //SPELL_HASH_WATERBOLT              // Waterbolt
        case 31707:
        case 72898:
            return AUTOCAST_EVENT_ATTACK;
            break;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Shaman Pet Spells

        //SPELL_HASH_SPIRIT_HUNT
        case 58877:
        case 58879:
            return AUTOCAST_EVENT_ON_SPAWN;
            break;
        //SPELL_HASH_TWIN_HOWL
        case 58857:
        // SPELL_HASH_BASH
        case 5211:
        case 6798:
        case 8983:
        case 25515:
        case 43612:
        case 57094:
        case 58861:
            return AUTOCAST_EVENT_ATTACK;
            break;
    }
    return AUTOCAST_EVENT_ATTACK;
}

void Pet::SetNameForEntry(uint32 entry)
{
    switch (entry)
    {
        case PET_IMP:
        case PET_VOIDWALKER:
        case PET_SUCCUBUS:
        case PET_FELHUNTER:
        case PET_FELGUARD:
        {
            QueryResult* result = CharacterDatabase.Query("SELECT `name` FROM `playersummons` WHERE `ownerguid`=%u AND `entry`=%d", m_Owner->GetLowGUID(), entry);
            if (result)
            {
                m_name = result->Fetch()->GetString();
                delete result;
            }
            else // no name found, generate one and save it
            {
                m_name = generateName();
                CharacterDatabase.Execute("INSERT INTO playersummons VALUES(%u, %u, '%s')", m_Owner->GetLowGUID(), entry, m_name.data());
            }
        }
        break;
        default:
            m_name = generateName();
    }
}

bool Pet::CreateAsSummon(uint32 entry, CreatureProperties const* ci, Creature* created_from_creature, Player* owner, SpellInfo* created_by_spell, uint32 type, uint32 expiretime, LocationVector* Vec, bool dismiss_old_pet)
{
    if (ci == nullptr || owner == nullptr)
    {
        return false;//the caller will delete us.
    }

    if (dismiss_old_pet)
    {
        owner->DismissActivePets();
    }

    m_Owner = owner;
    m_OwnerGuid = m_Owner->GetGUID();
    m_phase = m_Owner->GetPhase();
    m_PetNumber = m_Owner->GeneratePetNumber();
    creature_properties = ci;
    myFamily = sCreatureFamilyStore.LookupEntry(ci->Family);

    float x, y, z;
    if (Vec)
    {
        x = Vec->x;
        y = Vec->y;
        z = Vec->z;
    }
    else
    {
        x = owner->GetPositionX() + 2;
        y = owner->GetPositionY() + 2;
        z = owner->GetPositionZ();
    }

    // Create ourself
    Create(owner->GetMapId(), x, y, z, owner->GetOrientation());

    // Hunter pet should be max 5 levels below owner
    uint32 level = owner->getLevel();
    if (type & 0x2 && created_from_creature != NULL)
        level = created_from_creature->getLevel() < (level - 5) ? level - 5 : created_from_creature->getLevel();

    SetEntry(entry);
    setLevel(level);
    SetDisplayId(ci->Male_DisplayID);
    SetNativeDisplayId(ci->Male_DisplayID);
    EventModelChange();
    SetSummonedByGUID(owner->GetGUID());
    SetCreatedByGUID(owner->GetGUID());

    setUInt32Value(UNIT_FIELD_BYTES_0, 2048 | (0 << 24));

    SetBaseAttackTime(MELEE, 2000);
    SetBaseAttackTime(OFFHAND, 2000);
    SetFaction(owner->GetFaction());
    SetCastSpeedMod(1.0f);    // better set this one

    if (type == 1)
        Summon = true;

    if (created_from_creature == NULL)
    {
        m_name.assign(creature_properties->Name);

        if (created_by_spell != NULL)
        {
            if (created_by_spell->HasEffect(SPELL_EFFECT_SUMMON_PET) ||
                created_by_spell->HasEffect(SPELL_EFFECT_TAME_CREATURE) ||
                created_by_spell->HasEffect(SPELL_EFFECT_TAMECREATURE))
                SetNameForEntry(entry);

            SetCreatedBySpell(created_by_spell->getId());
        }

        setUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
        setUInt32Value(UNIT_FIELD_BYTES_2, (0x01 | (0x28 << 8) | (0x2 << 24)));
        SetBoundingRadius(0.5f);
        SetCombatReach(0.75f);
        SetPowerType(POWER_TYPE_MANA);
    }
    else // Hunter pet
    {
        if (myFamily == nullptr)
            m_name = "Pet";
        else
#if VERSION_STRING != Cata
            m_name.assign(myFamily->name[0]);
#else
            m_name.assign(myFamily->name);
#endif

        SetBoundingRadius(created_from_creature->GetBoundingRadius());
        SetCombatReach(created_from_creature->GetCombatReach());

        setUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE | UNIT_FLAG_COMBAT);  // why combat ??
        SetPower(POWER_TYPE_HAPPINESS, PET_HAPPINESS_UPDATE_VALUE >> 1);                //happiness
        SetMaxPower(POWER_TYPE_HAPPINESS, 1000000);
        setUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
        setUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, GetNextLevelXP(level));
        SetPower(POWER_TYPE_FOCUS, 100);                                                // Focus
        SetMaxPower(POWER_TYPE_FOCUS, 100);
        setUInt32Value(UNIT_FIELD_BYTES_2, 1  /* | (0x28 << 8) */ | (PET_RENAME_ALLOWED << 16));  // 0x3 -> Enable pet rename.
        SetPowerType(POWER_TYPE_FOCUS);
    }
    SetFaction(owner->GetFaction());

    if (owner->IsPvPFlagged())
        this->SetPvPFlag();
    else
        this->RemovePvPFlag();

    if (owner->IsFFAPvPFlagged())
        this->SetFFAPvPFlag();
    else
        this->RemoveFFAPvPFlag();

    if (owner->IsSanctuaryFlagged())
        this->SetSanctuaryFlag();
    else
        this->RemoveSanctuaryFlag();

    BaseDamage[0] = 0;
    BaseDamage[1] = 0;
    BaseOffhandDamage[0] = 0;
    BaseOffhandDamage[1] = 0;
    BaseRangedDamage[0] = 0;
    BaseRangedDamage[1] = 0;

    setSpeedForType(TYPE_WALK, owner->getSpeedForType(TYPE_WALK, true), true);
    setSpeedForType(TYPE_RUN, owner->getSpeedForType(TYPE_RUN, true), true);
    setSpeedForType(TYPE_FLY, owner->getSpeedForType(TYPE_FLY, true), true);
    resetCurrentSpeed();

    ApplyStatsForLevel();

    m_ExpireTime = expiretime;
    bExpires = m_ExpireTime > 0 ? true : false;

    if (!bExpires && owner->IsPlayer())
    {
        // Create PlayerPet struct (Rest done by UpdatePetInfo)
        PlayerPet* pp = new PlayerPet;
        pp->number = m_PetNumber;
        pp->stablestate = STABLE_STATE_ACTIVE;
        pp->spellid = created_by_spell ? created_by_spell->getId() : 0;
        pp->alive = true;

        if (owner->getClass() == HUNTER)
            pp->type = HUNTERPET;
        else
            pp->type = WARLOCKPET;

        mPi = pp;
        owner->AddPlayerPet(pp, pp->number);
    }

    InitializeMe(true);
    return true;
}

Pet::Pet(uint64 guid) : Creature(guid)
{
    Summon = false;
    memset(ActionBar, 0, sizeof(uint32) * 10);
    ScheduledForDeletion = false;

    m_AutoCombatSpell = 0;

    m_HappinessTimer = PET_HAPPINESS_UPDATE_TIMER;
    m_PetNumber = 0;

    m_State = PET_STATE_DEFENSIVE;
    m_Action = PET_ACTION_FOLLOW;
    m_ExpireTime = 0;
    bExpires = false;
    m_Diet = 0;
    reset_time = 0;
    reset_cost = 0;

    for (uint8 i = 0; i < AUTOCAST_EVENT_COUNT; i++)
        m_autoCastSpells[i].clear();

    m_AISpellStore.clear();
    mSpells.clear();
    mPi = NULL;
    m_Owner = NULL;
    m_OwnerGuid = 0;
}

Pet::~Pet()
{
    for (std::map<uint32, AI_Spell*>::iterator itr = m_AISpellStore.begin(); itr != m_AISpellStore.end(); ++itr)
        delete itr->second;
    m_AISpellStore.clear();

    for (uint8 i = 0; i < AUTOCAST_EVENT_COUNT; i++)
        m_autoCastSpells[i].clear();

    mSpells.clear();
}

void Pet::Update(unsigned long time_passed)
{
    if (Summon)
        Creature::Update(time_passed);  // passthrough
    else
    {
        Unit::Update(time_passed);      //Dead Hunter's Pets should be despawned only if the Owner logs out or goes out of range.
        if (m_corpseEvent)
        {
            sEventMgr.RemoveEvents(this);
            m_corpseEvent = false;
        }
    }

    if (!Summon && !bExpires && isAlive())
    {
        //ApplyPetLevelAbilities();
        //Happiness
        if (m_HappinessTimer == 0)
        {
            int32 burn = 1042;          //Based on WoWWiki pet looses 50 happiness over 6 min => 1042 every 7.5 s
            if (CombatStatus.IsInCombat())
                burn >>= 1;             //in combat reduce burn by half (guessed)
            ModPower(POWER_TYPE_HAPPINESS, -burn);
            m_HappinessTimer = PET_HAPPINESS_UPDATE_TIMER;  // reset timer
        }
        else if (!IsInBg())
        {
            if (time_passed > m_HappinessTimer)
                m_HappinessTimer = 0;
            else
                m_HappinessTimer -= time_passed;
        }
    }

    if (bExpires)
    {
        if (m_ExpireTime == 0)
        {
            Remove(false, true);        // remove
            return;
        }
        else if (time_passed > m_ExpireTime)
            m_ExpireTime = 0;
        else
            m_ExpireTime -= time_passed;
    }
}

void Pet::BuildPetSpellList(WorldPacket& data)
{
    data << uint64(GetGUID());

    if (myFamily != NULL)
        data << uint16(myFamily->ID);
    else
        data << uint16(0);

    data << uint32(0);
    data << uint8(GetPetState());       // 0x0 = passive, 0x1 = defensive, 0x2 = aggressive
    data << uint8(GetPetAction());      // 0x0 = stay, 0x1 = follow, 0x2 = attack
    data << uint16(0);                  // flags: 0xFF = disabled pet bar (eg. when pet stunned)

    // Send the actionbar
    for (uint8 i = 0; i < 10; i++)
    {
        if (ActionBar[i] & 0x4000000)   // Commands
            data << uint32(ActionBar[i]);
        else
        {
            if (ActionBar[i])
            {
                data << uint16(ActionBar[i]);
                data << GetSpellState(ActionBar[i]);
            }
            else
            {
                data << uint16(0);
                data << uint8(0);
                data << uint8(i + 5);
            }
        }
    }

    // we don't send spells for the water elemental so it doesn't show up in the spellbook
    if (m_ExpireTime == 0)
    {
        // Send the rest of the spells.
        data << uint8(mSpells.size());
        for (PetSpellMap::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
        {
            data << uint16(itr->first->getId());
            data << uint16(itr->second);
        }
    }

    data << uint8(0);
}

void Pet::SendSpellsToOwner()
{
    if (m_Owner == NULL)
        return;

    uint16 packetsize;
    if (m_ExpireTime == 0)
        packetsize = static_cast<uint16>(mSpells.size() * 4 + 59);
    else
        packetsize = 62;

    WorldPacket data(SMSG_PET_SPELLS, packetsize);

    BuildPetSpellList(data);

    m_Owner->SendPacket(&data);
}

void Pet::SendTalentsToOwner()
{
#if VERSION_STRING > TBC
    if (m_Owner == NULL)
        return;

    WorldPacket data(SMSG_TALENTS_INFO, 50);
    data << uint8(1);                   // Pet talent packet identificator
    data << uint32(GetTPs());           // Unspent talent points

    uint8 count = 0;
    size_t pos = data.wpos();
    data << uint8(0);                   // Amount of known talents (will be filled later)

    DBC::Structures::CreatureFamilyEntry const* cfe = sCreatureFamilyStore.LookupEntry(GetCreatureProperties()->Family);
    if (!cfe || static_cast<int32>(cfe->talenttree) < 0)
        return;

    // go through talent trees
    for (uint32 tte_id = PET_TALENT_TREE_START; tte_id <= PET_TALENT_TREE_END; tte_id++)
    {
        auto talent_tab = sTalentTabStore.LookupEntry(tte_id);
        if (talent_tab == nullptr)
            continue;

        // check if we match talent tab
        if (!(talent_tab->PetTalentMask & (1 << cfe->talenttree)))
            continue;


        for (uint32 t_id = 1; t_id < sTalentStore.GetNumRows(); t_id++)
        {
            // get talent entries for our talent tree
            auto talent = sTalentStore.LookupEntry(t_id);
            if (talent == nullptr)
                continue;

            if (talent->TalentTree != tte_id)
                continue;

            // check our spells
            for (uint8 j = 0; j < 5; j++)
                if (talent->RankID[j] > 0 && HasSpell(talent->RankID[j]))
                {
                    // if we have the spell, include it in packet
                    data << talent->TalentID;       // Talent ID
                    data << j;                  // Rank
                    ++count;
                }
        }
        // tab loaded, we can exit
        break;
    }
    // fill count of talents
    data.put< uint8 >(pos, count);

    // send the packet to owner
    if (m_Owner->GetSession() != NULL)
        m_Owner->GetSession()->SendPacket(&data);
#endif
}

void Pet::SendCastFailed(uint32 spellid, uint8 fail)
{
    if (m_Owner == NULL || m_Owner->GetSession() == NULL)
        return;

    WorldPacket data(SMSG_PET_CAST_FAILED, 6);
    data << uint8(0);
    data << uint32(spellid);
    data << uint8(fail);
    m_Owner->GetSession()->SendPacket(&data);
}

void Pet::SendActionFeedback(PetActionFeedback value)
{
    if (m_Owner == NULL || m_Owner->GetSession() == NULL)
        return;
    m_Owner->GetSession()->OutPacket(SMSG_PET_ACTION_FEEDBACK, 1, &value);
}

void Pet::InitializeSpells()
{
    for (PetSpellMap::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
    {
        SpellInfo* info = itr->first;

        // Check that the spell isn't passive
        if (info->IsPassive())
        {
            // Cast on self..
            Spell* sp = sSpellFactoryMgr.NewSpell(this, info, true, NULL);
            SpellCastTargets targets(this->GetGUID());
            sp->prepare(&targets);

            continue;
        }

        AI_Spell* sp = CreateAISpell(info);
        if (itr->second == AUTOCAST_SPELL_STATE)
            SetAutoCast(sp, true);
        else
            SetAutoCast(sp, false);
    }
}

AI_Spell* Pet::CreateAISpell(SpellInfo* info)
{
    ARCEMU_ASSERT(info != NULL);

    // Create an AI_Spell
    std::map<uint32, AI_Spell*>::iterator itr = m_AISpellStore.find(info->getId());
    if (itr != m_AISpellStore.end())
        return itr->second;

    AI_Spell* sp = new AI_Spell;
    sp->agent = AGENT_SPELL;
    sp->entryId = GetEntry();
    sp->floatMisc1 = 0;
    sp->maxrange = GetMaxRange(sSpellRangeStore.LookupEntry(info->getRangeIndex()));
    if (sp->maxrange < sqrt(info->custom_base_range_or_radius_sqr))
        sp->maxrange = sqrt(info->custom_base_range_or_radius_sqr);
    sp->minrange = GetMinRange(sSpellRangeStore.LookupEntry(info->getRangeIndex()));
    sp->Misc2 = 0;
    sp->procChance = 0;
    sp->spell = info;
    sp->cooldown = objmgr.GetPetSpellCooldown(info->getId());
    if (sp->cooldown == 0)
        sp->cooldown = info->getRecoveryTime();          //still 0 ?
    if (sp->cooldown == 0)
        sp->cooldown = info->getCategoryRecoveryTime();
    if (sp->cooldown == 0)
        sp->cooldown = info->getStartRecoveryTime();     //avoid spell spamming
    if (sp->cooldown == 0)
        sp->cooldown = PET_SPELL_SPAM_COOLDOWN;     //omg, avoid spamming at least
    sp->cooldowntime = 0;

    if (/* info->Effect[0] == SPELL_EFFECT_APPLY_AURA || */
        info->getEffect(0) == SPELL_EFFECT_APPLY_GROUP_AREA_AURA
        || info->getEffect(0) == SPELL_EFFECT_APPLY_RAID_AREA_AURA
        || info->getEffectImplicitTargetA(0) == EFF_TARGET_PET_MASTER
        || info->getEffectImplicitTargetA(0) == EFF_TARGET_PARTY_MEMBER)
        sp->spellType = STYPE_BUFF;
    else
        sp->spellType = STYPE_DAMAGE;

    sp->spelltargetType = static_cast<uint8>(info->ai_target_type);
    sp->autocast_type = GetAutoCastTypeForSpell(info);
    sp->procCount = 0;
    m_AISpellStore[info->getId()] = sp;
    return sp;
}

void Pet::LoadFromDB(Player* owner, PlayerPet* pi)
{
    m_Owner = owner;
    m_OwnerGuid = m_Owner->GetGUID();
    m_phase = m_Owner->GetPhase();
    mPi = pi;
    creature_properties = sMySQLStore.getCreatureProperties(mPi->entry);
    if (creature_properties == nullptr)
        return;

    myFamily = sCreatureFamilyStore.LookupEntry(creature_properties->Family);

    Create(owner->GetMapId(), owner->GetPositionX() + 2, owner->GetPositionY() + 2, owner->GetPositionZ(), owner->GetOrientation());

    m_PetNumber = mPi->number;
    m_name = mPi->name;
    Summon = false;
    SetEntry(mPi->entry);
    setLevel(mPi->level);


    m_HappinessTimer = mPi->happinessupdate;
    reset_time = mPi->reset_time;
    reset_cost = mPi->reset_cost;
    m_State = mPi->petstate;

    bExpires = false;

    // Setup actionbar
    if (mPi->actionbar.size() > 2)
    {
        char* ab = strdup(mPi->actionbar.c_str());
        char* p = strchr(ab, ',');
        char* q = ab;
        uint32 spellid;
        uint32 spstate;
        uint8 i = 0;

        while (p && i < 10)
        {
            *p = 0;

            if (sscanf(q, "%u %u", &spellid, &spstate) != 2)
                break;

            ActionBar[i] = spellid;
            //SetSpellState(sSpellCustomizations.GetSpellInfo(spellid), spstate);
            if (!(ActionBar[i] & 0x4000000) && spellid)
                mSpells[sSpellCustomizations.GetSpellInfo(spellid)] = static_cast<unsigned short>(spstate);

            i++;

            q = p + 1;
            p = strchr(q, ',');
        }

        free(ab);
    }

    //Preventing overbuffs
    SetAttackPower(0);
    SetAttackPowerMods(0);
    SetBaseAttackTime(MELEE, 2000);
    SetBaseAttackTime(OFFHAND, 2000);
    SetCastSpeedMod(1.0f);          // better set this one

    setUInt32Value(UNIT_FIELD_BYTES_0, 2048 | (0 << 24));

    if (pi->type == WARLOCKPET)
    {
        SetNameForEntry(mPi->entry);
        setUInt64Value(UNIT_CREATED_BY_SPELL, mPi->spellid);
        setUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
        setUInt32Value(UNIT_FIELD_BYTES_2, (0x01 | (0x28 << 8) | (0x2 << 24)));
        SetBoundingRadius(0.5f);
        SetCombatReach(0.75f);
        SetPowerType(POWER_TYPE_MANA);
    }
    else
    {
        SetBoundingRadius(creature_properties->BoundingRadius);
        SetCombatReach(creature_properties->CombatReach);
        setUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE | UNIT_FLAG_COMBAT);      // why combat ??
        SetPower(POWER_TYPE_HAPPINESS, PET_HAPPINESS_UPDATE_VALUE >> 1);                    //happiness
        SetMaxPower(POWER_TYPE_HAPPINESS, 1000000);
        setUInt32Value(UNIT_FIELD_PETEXPERIENCE, mPi->xp);
        setUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, GetNextLevelXP(mPi->level));
        setUInt32Value(UNIT_FIELD_BYTES_2, 1);
        SetPower(POWER_TYPE_FOCUS, 100);                                                    // Focus
        SetMaxPower(POWER_TYPE_FOCUS, 100);
        SetPowerType(POWER_TYPE_FOCUS);
    }

    BaseDamage[0] = 0;
    BaseDamage[1] = 0;
    BaseOffhandDamage[0] = 0;
    BaseOffhandDamage[1] = 0;
    BaseRangedDamage[0] = 0;
    BaseRangedDamage[1] = 0;

    setSpeedForType(TYPE_WALK, owner->getSpeedForType(TYPE_WALK, true), true);
    setSpeedForType(TYPE_RUN, owner->getSpeedForType(TYPE_RUN, true), true);
    setSpeedForType(TYPE_FLY, owner->getSpeedForType(TYPE_FLY, true), true);
    resetCurrentSpeed();

    setLevel(mPi->level);

    SetDisplayId(creature_properties->Male_DisplayID);
    SetNativeDisplayId(creature_properties->Male_DisplayID);

    EventModelChange();

    SetSummonedByGUID(owner->GetGUID());
    SetCreatedByGUID(owner->GetGUID());
    SetCreatedBySpell(mPi->spellid);
    SetFaction(owner->GetFaction());

    ApplyStatsForLevel();

    SetTPs(static_cast<uint8>(mPi->talentpoints));
    SetPower(GetPowerType(), mPi->current_power);
    SetHealth(mPi->current_hp);
    SetPower(POWER_TYPE_HAPPINESS, mPi->current_happiness);

    if (mPi->renamable == 0)
        setByteValue(UNIT_FIELD_BYTES_2, 2, PET_RENAME_NOT_ALLOWED);
    else
        setByteValue(UNIT_FIELD_BYTES_2, 2, PET_RENAME_ALLOWED);

    //if pet was dead on logout then it should be dead now too.//we could use mPi->alive but this will break backward compatibility
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DEAD))   //LoadFromDB() (called by Player::SpawnPet()) now always revive the Pet if it was dead.
        //This is because now we call SpawnPet() only if it's alive or we wanna revive it.
    {
        setUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
        SetHealth(GetMaxHealth());              //this is modified (if required) in Spell::SpellEffectSummonDeadPet()
        setDeathState(ALIVE);
    }

    InitializeMe(false);
}

void Pet::OnPushToWorld()
{
    //Pets MUST always have an owner
    ARCEMU_ASSERT(m_Owner != NULL);
    //before we initialize pet spells so we can apply spell mods on them
    m_Owner->EventSummonPet(this);

    Creature::OnPushToWorld();
}

void Pet::InitializeMe(bool first)
{
    GetAIInterface()->Init(this, AI_SCRIPT_PET, Movement::WP_MOVEMENT_SCRIPT_NONE, m_Owner);
    GetAIInterface()->SetUnitToFollow(m_Owner);
    GetAIInterface()->SetFollowDistance(3.0f);

    creature_properties = sMySQLStore.getCreatureProperties(GetEntry());
    if (creature_properties == nullptr)
        return;

    m_Owner->AddSummon(this);
    m_Owner->SetSummonedUnitGUID(GetGUID());

    setUInt32Value(UNIT_FIELD_PETNUMBER, GetUIdFromGUID());
    setUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, (uint32)UNIXTIME);

    myFamily = sCreatureFamilyStore.LookupEntry(creature_properties->Family);

    SetPetDiet();
    _setFaction();

    // Load our spells
    if (Summon)         // Summons - always
    {
        // Adds parent +frost spell damage
        if (GetEntry() == WATER_ELEMENTAL || GetEntry() == WATER_ELEMENTAL_NEW)
        {
            // According to WoWWiki and ElitistJerks, Water Elemental should inherit 33% of owner's frost spell power.
            // And don't freak out about Waterbolt damage, it is supposed to do 601-673 base damage.
            float parentfrost = static_cast<float>(m_Owner->GetDamageDoneMod(SCHOOL_FROST) * 0.33f);
            ModDamageDone[SCHOOL_FROST] = (uint32)parentfrost;
        }
        else if (GetEntry() == PET_IMP)
            m_aiInterface->setMeleeDisabled(true);
        else if (GetEntry() == PET_FELGUARD)
            SetEquippedItem(MELEE, 12784);

    }
    else if (first)     // Hunter pets - after taming
    {
        SetTPs(GetTPsForLevel(getLevel()));    // set talent points
    }
    else                // Hunter pets - load from db
    {
        // Pull from database... :/
        QueryResult* query = CharacterDatabase.Query("SELECT * FROM playerpetspells WHERE ownerguid = %u AND petnumber = %u", m_Owner->GetLowGUID(), m_PetNumber);
        if (query)
        {
            do
            {
                Field* f = query->Fetch();
                SpellInfo* spell = sSpellCustomizations.GetSpellInfo(f[2].GetUInt32());
                uint16 flags = f[3].GetUInt16();
                if (spell != NULL && mSpells.find(spell) == mSpells.end())
                    mSpells.insert(std::make_pair(spell, flags));

            }
            while (query->NextRow());
        }
        delete query;
    }

    PushToWorld(m_Owner->GetMapMgr());
    ARCEMU_ASSERT(IsInWorld());     //we MUST be sure Pet was pushed to world.

    InitializeSpells();

    if (first)
    {
        SetDefaultActionbar();      // Set up default actionbar
    }

    UpdateSpellList(false);
    SendSpellsToOwner();
    if (!Summon)
        SendTalentsToOwner();

    // set to active
    if (!bExpires)
        UpdatePetInfo(false);

    sEventMgr.AddEvent(this, &Pet::HandleAutoCastEvent, AUTOCAST_EVENT_ON_SPAWN, EVENT_UNK, 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    sEventMgr.AddEvent(this, &Pet::HandleAutoCastEvent, AUTOCAST_EVENT_LEAVE_COMBAT, EVENT_UNK, 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}


void Pet::UpdatePetInfo(bool bSetToOffline)
{
    if (bExpires || m_Owner == NULL)     // don't update expiring pets
        return;

    auto player_pet = m_Owner->GetPlayerPet(m_PetNumber);
    if (player_pet == nullptr)
        return;

    player_pet->active = !bSetToOffline;
    player_pet->entry = GetEntry();
    std::stringstream ss;

    player_pet->name = GetName();
    player_pet->number = m_PetNumber;
    player_pet->xp = GetXP();
    player_pet->level = getLevel();
    player_pet->happinessupdate = m_HappinessTimer;

    // save actionbar
    ss.rdbuf()->str("");
    for (uint8 i = 0; i < 10; ++i)
    {
        if (ActionBar[i] & 0x4000000)
            ss << ActionBar[i] << " 0";
        else if (ActionBar[i])
            ss << ActionBar[i] << " " << uint32(GetSpellState(ActionBar[i]));
        else
            ss << "0 0";

        ss << ",";
    }

    player_pet->actionbar = ss.str();
    player_pet->reset_cost = reset_cost;
    player_pet->reset_time = reset_time;
    player_pet->petstate = m_State;
    player_pet->alive = isAlive();
    player_pet->current_power = GetPower(GetPowerType());
    player_pet->talentpoints = GetTPs();
    player_pet->current_hp = GetHealth();
    player_pet->current_happiness = GetPower(POWER_TYPE_HAPPINESS);

    uint32 renamable = getByteValue(UNIT_FIELD_BYTES_2, 2);

    if (renamable == PET_RENAME_ALLOWED)
        player_pet->renamable = 1;
    else
        player_pet->renamable = 0;
}

void Pet::Dismiss()     //Abandon pet
{
    if (m_Owner && !bExpires)
        m_Owner->RemovePlayerPet(m_PetNumber);   // find playerpet entry and delete it

    Remove(false, true);
}

void Pet::Remove(bool bUpdate, bool bSetOffline)
{
    if (ScheduledForDeletion)
        return;

    ScheduledForDeletion = true;
    PrepareForRemove(bUpdate, bSetOffline);
    m_Owner->AddGroupUpdateFlag(GROUP_UPDATE_PET);

    if (IsInWorld())
        Unit::RemoveFromWorld(true);

    SafeDelete();
}

void Pet::RemoveFromWorld(bool free_guid)
{
    if (IsSummonedPet())
        PrepareForRemove(false, true);
    else
        PrepareForRemove(true, false);
    Unit::RemoveFromWorld(free_guid);
}

void Pet::OnRemoveFromWorld()
{
    std::list<Pet*> ownerSummons = m_Owner->GetSummons();
    std::list<Pet*>::iterator itr;
    for (itr = ownerSummons.begin(); itr != ownerSummons.end(); ++itr)
    {
        //m_Owner MUST NOT have a reference to us anymore
        ARCEMU_ASSERT((*itr)->GetGUID() != GetGUID());
    }
}

void Pet::Despawn(uint32 delay, uint32 /*respawntime*/)
{
    bool delayed = (delay != 0);
    DelayedRemove(delayed, true, delay);
}

void Pet::SafeDelete()
{
    sEventMgr.RemoveEvents(this);

    m_Owner->AddGarbagePet(this);
}

void Pet::DelayedRemove(bool bTime, bool dismiss, uint32 delay)
{
    if (ScheduledForDeletion)
        return;

    // called when pet has died
    if (bTime)
    {
        if (Summon || dismiss)
            Dismiss();          // remove us..
        else
            Remove(true, false);
    }
    else
        sEventMgr.AddEvent(this, &Pet::DelayedRemove, true, dismiss, uint32(0), EVENT_PET_DELAYED_REMOVE, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Pet::PrepareForRemove(bool bUpdate, bool bSetOffline)
{
    RemoveAllAuras();           // Prevent pet overbuffing
    m_Owner->EventDismissPet();

    if (bUpdate)
    {
        if (!bExpires)
            UpdatePetInfo(bSetOffline);
        if (!IsSummonedPet())
            m_Owner->_SavePet(NULL);
    }

    bool main_summon = m_Owner->GetSummon() == this;
    m_Owner->RemoveSummon(this);

    if (m_Owner->GetSummon() == NULL)   //we have no more summons, required by spells summoning more than 1.
    {
        m_Owner->SetSummonedUnitGUID(0);
        m_Owner->SendEmptyPetSpellList();
    }
    else if (main_summon)               //we just removed the summon displayed in the portrait so we need to update it with another one.
    {
        m_Owner->SetSummonedUnitGUID(m_Owner->GetSummon()->GetGUID());      //set the summon still alive
        m_Owner->GetSummon()->SendSpellsToOwner();
    }

    if (IsInWorld() && IsActive())
        Deactivate(m_mapMgr);
}

void Pet::setDeathState(DeathState s)
{
    Creature::setDeathState(s);
    if (s == JUST_DIED && IsSummonedPet())
    {
        //we can't dimiss the summon now now since it's still needed in DealDamage()
        sEventMgr.AddEvent(this, &Pet::Dismiss, EVENT_UNK, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
    if (mPi != NULL)
        mPi->alive = isAlive();
}

bool Pet::CanGainXP()
{
    // only hunter pets which are below owner level can gain experience
    if (Summon || !m_Owner || getLevel() >= m_Owner->getLevel())
        return false;
    else
        return true;
}

void Pet::GiveXP(uint32 xp)
{
    xp += GetXP();
    uint32 nxp = m_uint32Values[UNIT_FIELD_PETNEXTLEVELEXP];

    if (xp >= nxp)
    {
        SetTPs(GetTPsForLevel(getLevel() + 1) - GetSpentTPs());
        modLevel(1);
        xp -= nxp;
        nxp = GetNextLevelXP(getLevel());
        setUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, nxp);
        ApplyStatsForLevel();
        UpdateSpellList();
        SendTalentsToOwner();
    }

    setUInt32Value(UNIT_FIELD_PETEXPERIENCE, xp);
}

uint32 Pet::GetNextLevelXP(uint32 level)
{
    // Pets need only 5% of xp to level up compared to players
    uint32 nextLvlXP = 0;
    nextLvlXP = sMySQLStore.getPlayerXPForLevel(level);
    return nextLvlXP / 20;
}

void Pet::UpdateSpellList(bool showLearnSpells)
{
    uint32 s = 0;       // SkillLine 1
    uint32 s2 = 0;      // SkillLine 2

    if (creature_properties->spelldataid != 0)
    {
        auto creature_spell_data = sCreatureSpellDataStore.LookupEntry(creature_properties->spelldataid);

        for (uint8 i = 0; i < 3; ++i)
        {
            if (creature_spell_data == nullptr)
                continue;

            uint32 spellid = creature_spell_data->Spells[i];

            if (spellid != 0)
            {
                SpellInfo* sp = sSpellCustomizations.GetSpellInfo(spellid);
                if (sp != NULL)
                    AddSpell(sp, true, showLearnSpells);
            }
        }
    }

    for (uint8 i = 0; i < 4; ++i)
    {
        uint32 spellid = creature_properties->AISpells[i];
        if (spellid != 0)
        {
            SpellInfo* sp = sSpellCustomizations.GetSpellInfo(spellid);
            if (sp != NULL)
                AddSpell(sp, true, showLearnSpells);
        }
    }

    if (GetCreatureProperties()->Family == 0 && Summon)
    {
        std::map<uint32, std::set<uint32> >::iterator it1;
        std::set<uint32>::iterator it2;
        it1 = m_Owner->SummonSpells.find(GetEntry());       // Get spells from the owner
        if (it1 != m_Owner->SummonSpells.end())
        {
            it2 = it1->second.begin();
            for (; it2 != it1->second.end(); ++it2)
            {
                AddSpell(sSpellCustomizations.GetSpellInfo(*it2), true, showLearnSpells);
            }
        }
        return;
    }
    else
    {
        // Get Creature family from DB (table creature_names, field family), load the skill line from CreatureFamily.dbc for use with SkillLineAbiliby.dbc entry
        DBC::Structures::CreatureFamilyEntry const* f = sCreatureFamilyStore.LookupEntry(GetCreatureProperties()->Family);
        if (f)
        {
            s = f->skilline;
            s2 = f->tameable;
        }
    }

    if (s || s2)
    {
        SpellInfo* sp;
        for (uint32 idx = 0; idx < sSkillLineAbilityStore.GetNumRows(); ++idx)
        {
            auto skill_line_ability = sSkillLineAbilityStore.LookupEntry(idx);
            if (skill_line_ability == nullptr)
                continue;

            // Update existing spell, or add new "automatic-acquired" spell
            if ((skill_line_ability->skilline == s || skill_line_ability->skilline == s2) && skill_line_ability->acquireMethod == 2)
            {
                sp = sSpellCustomizations.GetSpellInfo(skill_line_ability->spell);
                if (sp && getLevel() >= sp->getBaseLevel())
                {
                    // Pet is able to learn this spell; now check if it already has it, or a higher rank of it
                    bool addThisSpell = true;
                    for (PetSpellMap::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
                    {
                        if ((itr->first->custom_NameHash == sp->custom_NameHash) && (itr->first->custom_RankNumber >= sp->custom_RankNumber))
                        {
                            // Pet already has this spell, or a higher rank. Don't add it.
                            addThisSpell = false;
                        }
                    }
                    if (addThisSpell)
                    {
                        AddSpell(sp, true, showLearnSpells);
                    }
                }
            }
        }
    }
}

void Pet::AddSpell(SpellInfo* sp, bool learning, bool showLearnSpell)
{
    if (sp == NULL)
        return;

    if (sp->IsPassive())        // Cast on self if we're a passive spell
    {
        if (IsInWorld())
        {
            Spell* spell = sSpellFactoryMgr.NewSpell(this, sp, true, NULL);
            SpellCastTargets targets(this->GetGUID());
            spell->prepare(&targets);
            mSpells[sp] = 0x0100;
        }
    }
    else
    {
        bool ab_replace = false;                    // Active spell add to the actionbar.

        bool done = false;
        if (learning)
        {
            for (PetSpellMap::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
            {
                if (sp->custom_NameHash == itr->first->custom_NameHash)
                {
                    // replace the action bar
                    for (uint8 i = 0; i < 10; ++i)
                    {
                        if (ActionBar[i] == itr->first->getId())
                        {
                            ActionBar[i] = sp->getId();
                            ab_replace = true;
                            break;
                        }
                    }

                    // Create the AI_Spell
                    AI_Spell* asp = CreateAISpell(sp);

                    // apply the spell state
                    uint16 ss = GetSpellState(itr->first);
                    mSpells[sp] = ss;
                    if (ss == AUTOCAST_SPELL_STATE)
                        SetAutoCast(asp, true);

                    if (asp->autocast_type == AUTOCAST_EVENT_ON_SPAWN)
                        CastSpell(this, sp, false);

                    RemoveSpell(itr->first, showLearnSpell);
                    done = true;
                    break;
                }
            }
        }

        if (!ab_replace)
        {
            bool has = false;
            for (uint8 i = 0; i < 10; ++i)
            {
                if (ActionBar[i] == sp->getId())
                {
                    has = true;
                    break;
                }
            }

            if (!has)
            {
                for (uint8 i = 0; i < 10; ++i)
                {
                    if (ActionBar[i] == 0)
                    {
                        ActionBar[i] = sp->getId();
                        break;
                    }
                }
            }
        }

        if (done == false)
        {
            if (mSpells.find(sp) != mSpells.end())
                return;

            if (learning)
            {
                AI_Spell* asp = CreateAISpell(sp);
                uint16 ss = (asp->autocast_type > 0) ? AUTOCAST_SPELL_STATE : DEFAULT_SPELL_STATE;
                mSpells[sp] = ss;
                if (ss == AUTOCAST_SPELL_STATE)
                    SetAutoCast(asp, true);

                // Phase shift gets cast on spawn, right?
                if (asp->autocast_type == AUTOCAST_EVENT_ON_SPAWN)
                    CastSpell(this, sp, false);

                switch (asp->spell->getId())
                {
                    //SPELL_HASH_PHASE_SHIFT
                    case 4511:
                    case 4630:
                    case 8611:
                    case 8612:
                    case 20329:
                    case 29309:
                    case 29315:
                        CastSpell(this, sp, false);
                        break;
                }
            }
            else
                mSpells[sp] = DEFAULT_SPELL_STATE;
        }
    }

#if VERSION_STRING > TBC
    if (showLearnSpell && m_Owner && m_Owner->GetSession() && !(sp->getAttributes() & ATTRIBUTES_NO_CAST))
    {
        auto id = sp->getId();
        m_Owner->GetSession()->OutPacket(SMSG_PET_LEARNED_SPELL, 2, &id);
    }
#endif
    if (IsInWorld())
        SendSpellsToOwner();
}

void Pet::SetSpellState(SpellInfo* sp, uint16 State)
{
    PetSpellMap::iterator itr = mSpells.find(sp);
    if (itr == mSpells.end())
        return;

    uint16 oldstate = itr->second;
    itr->second = State;

    if (State == AUTOCAST_SPELL_STATE || oldstate == AUTOCAST_SPELL_STATE)
    {
        AI_Spell* sp2 = GetAISpellForSpellId(sp->getId());
        if (sp2)
        {
            if (State == AUTOCAST_SPELL_STATE)
                SetAutoCast(sp2, true);
            else
                SetAutoCast(sp2, false);
        }
    }
}

uint16 Pet::GetSpellState(SpellInfo* sp)
{
    PetSpellMap::iterator itr = mSpells.find(sp);
    if (itr == mSpells.end())
        return DEFAULT_SPELL_STATE;

    return itr->second;
}

void Pet::SetDefaultActionbar()
{
    // Set up the default actionbar.
    ActionBar[0] = PET_SPELL_ATTACK;
    ActionBar[1] = PET_SPELL_FOLLOW;
    ActionBar[2] = PET_SPELL_STAY;

    // Fill up 4 slots with our spells
    if (mSpells.size() > 0)
    {
        PetSpellMap::iterator itr = mSpells.begin();
        uint32 pos = 0;
        for (; itr != mSpells.end() && pos < 4; ++itr, ++pos)
            ActionBar[3 + pos] = itr->first->getId();
    }

    ActionBar[7] = PET_SPELL_AGRESSIVE;
    ActionBar[8] = PET_SPELL_DEFENSIVE;
    ActionBar[9] = PET_SPELL_PASSIVE;
}

void Pet::WipeTalents()
{
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); i++)
    {
        auto talent = sTalentStore.LookupEntry(i);
        if (talent == nullptr)
            continue;

        if (talent->TalentTree < PET_TALENT_TREE_START || talent->TalentTree > PET_TALENT_TREE_END)   // 409-Tenacity, 410-Ferocity, 411-Cunning
            continue;

        for (uint8 j = 0; j < 5; j++)
            if (talent->RankID[j] != 0 && HasSpell(talent->RankID[j]))
                RemoveSpell(talent->RankID[j]);
    }
    SendSpellsToOwner();
}

void Pet::RemoveSpell(SpellInfo* sp, bool showUnlearnSpell)
{
    mSpells.erase(sp);
    std::map<uint32, AI_Spell*>::iterator itr = m_AISpellStore.find(sp->getId());
    if (itr != m_AISpellStore.end())
    {
        if (itr->second->autocast_type != AUTOCAST_EVENT_NONE)
        {
            std::list<AI_Spell*>::iterator it3;
            for (std::list<AI_Spell*>::iterator it2 = m_autoCastSpells[itr->second->autocast_type].begin(); it2 != m_autoCastSpells[itr->second->autocast_type].end();)
            {
                it3 = it2++;
                if ((*it3) == itr->second)
                {
                    m_autoCastSpells[itr->second->autocast_type].erase(it3);
                }
            }
        }
        for (std::list<AI_Spell*>::iterator it = m_aiInterface->m_spells.begin(); it != m_aiInterface->m_spells.end(); ++it)
        {
            if ((*it) == itr->second)
            {
                m_aiInterface->m_spells.erase(it);
                m_aiInterface->CheckNextSpell(itr->second);
                break;
            }
        }

        delete itr->second;
        m_AISpellStore.erase(itr);
    }
    else
    {
        for (std::list<AI_Spell*>::iterator it = m_aiInterface->m_spells.begin(); it != m_aiInterface->m_spells.end(); ++it)
        {
            if ((*it)->spell == sp)
            {
                // woot?
                AI_Spell* spe = *it;
                m_aiInterface->m_spells.erase(it);
                delete spe;
                break;
            }
        }
    }
    //Remove spell from action bar as well
    for (uint32 pos = 0; pos < 10; pos++)
    {
        if (ActionBar[pos] == sp->getId())
            ActionBar[pos] = 0;
    }

#if VERSION_STRING > TBC
    if (showUnlearnSpell && m_Owner && m_Owner->GetSession())
    {
        auto id = sp->getId();
        m_Owner->GetSession()->OutPacket(SMSG_PET_UNLEARNED_SPELL, 4, &id);
    }
#endif
}

void Pet::Rename(std::string NewName)
{
    m_name = NewName;
    // update petinfo
    UpdatePetInfo(false);

    // update timestamp to force a re-query
    setUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, (uint32)UNIXTIME);

    // save new summoned name to db (.pet renamepet)
    if (m_Owner->getClass() == WARLOCK)
    {
        CharacterDatabase.Execute("UPDATE `playersummons` SET `name`='%s' WHERE `ownerguid`=%u AND `entry`=%u", m_name.data(), m_Owner->GetLowGUID(), GetEntry());
    }

    m_Owner->AddGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_NAME);
}

void Pet::ApplySummonLevelAbilities()
{
    uint32 level = getLevel();
    double pet_level = (double)level;

    int stat_index = -1;        // Determine our stat index.
    //float scale = 1;
    bool has_mana = true;

    switch (GetEntry())
    {
        case PET_IMP:
            stat_index = 0;
            m_aiInterface->setMeleeDisabled(true);
            break;
        case PET_VOIDWALKER:
            stat_index = 1;
            break;
        case PET_SUCCUBUS:
            stat_index = 2;
            break;
        case PET_FELHUNTER:
            stat_index = 3;
            break;
        case 11859:         // Doomguard
        case 89:            // Infernal
        case PET_FELGUARD:
            stat_index = 4;
            break;
        case WATER_ELEMENTAL:
        case WATER_ELEMENTAL_NEW:
            stat_index = 5;
            m_aiInterface->setMeleeDisabled(true);
            break;
        case SHADOWFIEND:
            stat_index = 5;
            break;
        case 26125:
            stat_index = 4;
            break;
        case 29264:
            stat_index = 5;
            break;
        case 27893:
            stat_index = 4;
            break;
    }
    if (GetEntry() == 89)
        has_mana = false;

    if (stat_index < 0)
    {
        LOG_ERROR("PETSTAT: No stat index found for entry %u, `%s`! Using 5 as a default.", GetEntry(), GetCreatureProperties()->Name.c_str());
        stat_index = 5;
    }

    static double R_base_str[6] = { 18.1884058, -15, -15, -15, -15, -15 };
    static double R_mod_str[6] = { 1.811594203, 2.4, 2.4, 2.4, 2.4, 2.4 };
    static double R_base_agi[6] = { 19.72463768, -1.25, -1.25, -1.25, -1.25, -1.25 };
    static double R_mod_agi[6] = { 0.275362319, 1.575, 1.575, 1.575, 1.575, 1.575 };
    static double R_base_sta[6] = { 18.82608695, -3.5, -17.75, -17.75, -17.75, 0 };
    static double R_mod_sta[6] = { 1.173913043, 4.05, 4.525, 4.525, 4.525, 4.044 };
    static double R_base_int[6] = { 19.44927536, 12.75, 12.75, 12.75, 12.75, 20 };
    static double R_mod_int[6] = { 4.550724638, 1.875, 1.875, 1.875, 1.875, 2.8276 };
    static double R_base_spr[6] = { 19.52173913, -2.25, -2.25, -2.25, -2.25, 20.5 };
    static double R_mod_spr[6] = { 3.47826087, 1.775, 1.775, 1.775, 1.775, 3.5 };
    static double R_base_pwr[6] = { 7.202898551, -101, -101, -101, -101, -101 };
    static double R_mod_pwr[6] = { 2.797101449, 6.5, 6.5, 6.5, 6.5, 6.5 };
    static double R_base_armor[6] = { -11.69565217, -702, -929.4, -1841.25, -1157.55, 0 };
    static double R_mod_armor[6] = { 31.69565217, 139.6, 74.62, 89.175, 101.1316667, 20 };
    static double R_pet_sta_to_hp[6] = { 4.5, 15.0, 7.5, 10.0, 10.6, 7.5 };
    static double R_pet_int_to_mana[6] = { 15.0, 15.0, 15.0, 15.0, 15.0, 5.0 };
    static double R_base_min_dmg[6] = { 0.550724638, 4.566666667, 26.82, 29.15, 20.17888889, 20 };
    static double R_mod_min_dmg[6] = { 1.449275362, 1.433333333, 2.18, 1.85, 1.821111111, 1 };
    static double R_base_max_dmg[6] = { 1.028985507, 7.133333333, 36.16, 39.6, 27.63111111, 20 };
    static double R_mod_max_dmg[6] = { 1.971014493, 1.866666667, 2.84, 2.4, 2.368888889, 1.1 };

    double base_str = R_base_str[stat_index];
    double mod_str = R_mod_str[stat_index];
    double base_agi = R_base_agi[stat_index];
    double mod_agi = R_mod_agi[stat_index];
    double base_sta = R_base_sta[stat_index];
    double mod_sta = R_mod_sta[stat_index];
    double base_int = R_base_int[stat_index];
    double mod_int = R_mod_int[stat_index];
    double base_spr = R_base_spr[stat_index];
    double mod_spr = R_mod_spr[stat_index];
    double base_pwr = R_base_pwr[stat_index];
    double mod_pwr = R_mod_pwr[stat_index];
    double base_armor = R_base_armor[stat_index];
    double mod_armor = R_mod_armor[stat_index];
    double base_min_dmg = R_base_min_dmg[stat_index];
    double mod_min_dmg = R_mod_min_dmg[stat_index];
    double base_max_dmg = R_base_max_dmg[stat_index];
    double mod_max_dmg = R_mod_max_dmg[stat_index];
    double pet_sta_to_hp = R_pet_sta_to_hp[stat_index];
    double pet_int_to_mana = R_pet_int_to_mana[stat_index];

    // Calculate bonuses
    double pet_str = base_str + pet_level * mod_str;
    double pet_agi = base_agi + pet_level * mod_agi;
    double pet_sta = base_sta + pet_level * mod_sta;
    double pet_int = base_int + pet_level * mod_int;
    double pet_spr = base_spr + pet_level * mod_spr;
    double pet_pwr = base_pwr + pet_level * mod_pwr;
    double pet_arm = base_armor + pet_level * mod_armor;

    // Calculate values
    BaseStats[STAT_STRENGTH] = (uint32)(pet_str);
    BaseStats[STAT_AGILITY] = (uint32)(pet_agi);
    BaseStats[STAT_STAMINA] = (uint32)(pet_sta);
    BaseStats[STAT_INTELLECT] = (uint32)(pet_int);
    BaseStats[STAT_SPIRIT] = (uint32)(pet_spr);

    double pet_min_dmg = base_min_dmg + pet_level * mod_min_dmg;
    double pet_max_dmg = base_max_dmg + pet_level * mod_max_dmg;
    BaseDamage[0] = float(pet_min_dmg);
    BaseDamage[1] = float(pet_max_dmg);

    // Apply attack power.
    SetAttackPower((uint32)(pet_pwr));

    BaseResistance[0] = (uint32)(pet_arm);
    CalcResistance(0);

    // Calculate health / mana
    double health = pet_sta * pet_sta_to_hp;
    double mana = has_mana ? (pet_int * pet_int_to_mana) : 0.0;
    if (health == 0)
    {
        LOG_ERROR("Pet with entry %u has 0 health !!", GetEntry());
        health = 100;
    }
    SetBaseHealth((uint32)(health));
    SetMaxHealth((uint32)(health));
    SetBaseMana((uint32)(mana));
    SetMaxPower(POWER_TYPE_MANA, (uint32)(mana));

    for (uint16 x = 0; x < 5; ++x)
        CalcStat(x);
}

void Pet::ApplyPetLevelAbilities()
{
    uint32 pet_family = GetCreatureProperties()->Family;
    uint32 level = getLevel();

    if (level > worldConfig.player.playerLevelCap)
        level = worldConfig.player.playerLevelCap;
    else if (level < 1)
        level = 1;

    static uint32 family_aura[47] = { 0   /*0*/,
        17223 /*1*/, 17210 /*2*/, 17129 /*3*/, 17208 /*4*/, 7000  /*5*/, 17212 /*6*/, 17209 /*7*/, 17211 /*8*/, 17214 /*9*/, 0    /*10*/,
        17217/*11*/, 17220/*12*/, 0       /*13*/, 0    /*14*/, 0    /*15*/, 0    /*16*/, 0    /*17*/, 0    /*18*/, 0    /*19*/, 17218/*20*/,
        17221/*21*/, 0    /*22*/, 0       /*23*/, 17206/*24*/, 17215/*25*/, 17216/*26*/, 17222/*27*/, 0    /*28*/, 0    /*29*/, 34887/*30*/,
        35257/*31*/, 35254/*32*/, 35258/*33*/, 35253/*34*/, 35386/*35*/, 0    /*36*/, 50297/*37*/, 54642/*38*/, 54676/*39*/, 0    /*40*/,
        55192/*41*/, 55729/*42*/, 56634/*43*/, 56635/*44*/, 58598/*45*/, 61199/*46*/
    };

    if (pet_family < 47)
        RemoveAura(family_aura[pet_family]);  //If the pet gained a level, we need to remove the auras to re-calculate everything.

    LoadPetAuras(-1);//These too

    MySQLStructure::PetLevelAbilities const* pet_abilities = sMySQLStore.getPetLevelAbilities(level);
    if (pet_abilities == nullptr)
    {
        LOG_ERROR("No abilities for level %u in table pet_level_abilities! Auto apply abilities of level 80!", level);
        pet_abilities = sMySQLStore.getPetLevelAbilities(DBC_PLAYER_LEVEL_CAP);
    }

    BaseResistance[0] = pet_abilities->armor;
    BaseStats[0] = pet_abilities->strength;
    BaseStats[1] = pet_abilities->agility;
    BaseStats[2] = pet_abilities->stamina;
    BaseStats[3] = pet_abilities->intellect;
    BaseStats[4] = pet_abilities->spirit;

    SetBaseHealth(pet_abilities->health);
    setUInt32Value(UNIT_FIELD_MAXHEALTH, pet_abilities->health);

    //Family Aura
    if (pet_family > 46)
        LOG_ERROR("PETSTAT: Creature family %i [%s] has missing data.", pet_family, myFamily->name);
    else if (family_aura[pet_family] != 0)
        this->CastSpell(this, family_aura[pet_family], true);

    for (uint16 x = 0; x < 5; ++x)
        CalcStat(x);

    LoadPetAuras(-2);//Load all BM auras
}

void Pet::ApplyStatsForLevel()
{
    if (Summon)
        ApplySummonLevelAbilities();
    else
        ApplyPetLevelAbilities();

    // Apply common stuff
    // Apply scale for this family.
    // Hunter pets' size scaling is affected by level of the pet.
    // http://www.wowwiki.com/Hunter_pet#Size
    if (myFamily != NULL && myFamily->minsize > 0.0f)
    {
        float pet_level = float(getLevel());
        float level_diff = float(myFamily->maxlevel - myFamily->minlevel);
        float scale_diff = float(myFamily->maxsize - myFamily->minsize);
        float factor = scale_diff / level_diff;
        float scale = factor * pet_level + myFamily->minsize;
        if (myFamily->ID == 23) // Imps have strange values set into CreatureFamily.dbc,
            SetScale(1.0f);    // they always will be set to be 0.5f. But that's not right.
        else
            SetScale(scale);
    }

    // Apply health fields.
    SetHealth(m_uint32Values[UNIT_FIELD_MAXHEALTH]);
    SetPower(POWER_TYPE_MANA, GetMaxPower(POWER_TYPE_MANA));   // mana
    SetPower(POWER_TYPE_FOCUS, GetMaxPower(POWER_TYPE_FOCUS));   // focus
}

void Pet::LoadPetAuras(int32 id)
{
    /*
       Talent               Aura Id
       Unleashed Fury            8875
       Thick Hide                19580
       Endurance Training        19581
       Bestial Swiftness        19582
       Bestial Discipline        19589
       Ferocity                19591
       Animal Handler            34666
       Catlike Reflexes        34667
       Serpent's Swiftness        34675
       */

    static uint32 mod_auras[9] = { 8875, 19580, 19581, 19582, 19589, 19591, 34666, 34667, 34675 };      //Beastmastery Talent's auras.
    InheritSMMods(m_Owner);

    if (id == -1)           //unload all
    {
        for (uint32 x = 0; x < 9; ++x)
            RemoveAura(mod_auras[x]);
    }
    else if (id == -2)      //load all
    {
        for (uint32 x = 0; x < 9; ++x)
            CastSpell(this, mod_auras[x], true);
    }
    else if (mod_auras[id])  //reload one
    {
        RemoveAura(mod_auras[id]);
        CastSpell(this, mod_auras[id], true);
    }

    m_Owner->AddGroupUpdateFlag(GROUP_UPDATE_PET);
}

void Pet::UpdateAP()
{
    // Only hunter pets
    if (Summon)
        return;

    uint32 str = GetStat(STAT_STRENGTH);
    uint32 AP = (str * 2 - 20);
    if (m_Owner)
        AP += m_Owner->GetRAP() * 22 / 100;
    if (static_cast<int32>(AP) < 0) AP = 0;
    SetAttackPower(AP);
}

uint32 Pet::CanLearnSpell(SpellInfo* sp)
{
    // level requirement
    if (getLevel() < sp->getSpellLevel())
        return SPELL_FAILED_LEVEL_REQUIREMENT;

    return 0;
}
HappinessState Pet::GetHappinessState()
{
    //gets happiness state from happiness points
    uint32 pts = getUInt32Value(UNIT_FIELD_POWER5);
    if (pts < PET_HAPPINESS_UPDATE_VALUE)
        return UNHAPPY;
    else if (pts >= PET_HAPPINESS_UPDATE_VALUE << 1)
        return HAPPY;
    else
        return CONTENT;
}

AI_Spell* Pet::HandleAutoCastEvent()
{
    std::list<AI_Spell*>::iterator itr, itr2;
    bool chance = true;
    uint32 size = 0;

    for (itr2 = m_autoCastSpells[AUTOCAST_EVENT_ATTACK].begin(); itr2 != m_autoCastSpells[AUTOCAST_EVENT_ATTACK].end();)
    {
        itr = itr2;
        ++itr2;
        size = (uint32)m_autoCastSpells[AUTOCAST_EVENT_ATTACK].size();
        if (size > 1)
            chance = Rand(100.0f / size);

        if ((*itr)->autocast_type == AUTOCAST_EVENT_ATTACK)
        {
            // spells still spammed, I think the cooldowntime is being set incorrectly somewhere else
            if (chance && (*itr)->spell &&Util::getMSTime() >= (*itr)->cooldowntime && GetPower(static_cast<uint16_t>((*itr)->spell->getPowerType())) >= (*itr)->spell->getManaCost())
            {
                return *itr;
            }
        }
        else    // bad pointers somehow end up here :S
        {
            LOG_ERROR("Bad AI_Spell detected in AutoCastEvent!");
            m_autoCastSpells[AUTOCAST_EVENT_ATTACK].erase(itr);
        }
    }

    return NULL;
}

void Pet::HandleAutoCastEvent(AutoCastEvents Type)
{
    std::list<AI_Spell*>::iterator itr, it2;
    AI_Spell* sp;
    if (m_Owner == NULL)
        return;

    if (Type == AUTOCAST_EVENT_ATTACK)
    {
        if (m_autoCastSpells[AUTOCAST_EVENT_ATTACK].size() > 1)
        {
            for (itr = m_autoCastSpells[AUTOCAST_EVENT_ATTACK].begin(); itr != m_autoCastSpells[AUTOCAST_EVENT_ATTACK].end(); ++itr)
            {
                if (itr == m_autoCastSpells[AUTOCAST_EVENT_ATTACK].end())
                {
                    if (Util::getMSTime() >= (*itr)->cooldowntime)
                        m_aiInterface->SetNextSpell(*itr);
                    else
                        return;
                    break;
                }
                else
                {
                    if ((*itr)->cooldowntime >Util::getMSTime())
                        continue;

                    m_aiInterface->SetNextSpell(*itr);
                }
            }
        }
        else if (m_autoCastSpells[AUTOCAST_EVENT_ATTACK].size())
        {
            sp = *m_autoCastSpells[AUTOCAST_EVENT_ATTACK].begin();
            if (sp->cooldown &&Util::getMSTime() < sp->cooldowntime)
                return;

            m_aiInterface->SetNextSpell(sp);
        }

        return;
    }

    for (itr = m_autoCastSpells[Type].begin(); itr != m_autoCastSpells[Type].end();)
    {
        it2 = itr++;
        sp = *it2;

        if (sp->spell == NULL)
        {
            LOG_ERROR("Found corrupted spell at m_autoCastSpells, skipping");
            continue;
        }
        else if (sp->autocast_type != static_cast<uint32>(Type))
        {
            LOG_ERROR("Found corrupted spell (%lu) at m_autoCastSpells, skipping", sp->entryId);
            continue;
        }

        if (sp->spelltargetType == TTYPE_OWNER)
        {
            if (!m_Owner->HasAura(sp->spell->getId()))
                CastSpell(m_Owner, sp->spell, false);
        }
        else
        {
            //modified by Zack: Spell targeting will be generated in the castspell function now.You cannot force to target self all the time
            CastSpell(static_cast<Unit*>(NULL), sp->spell, false);
        }
    }
}

void Pet::SetAutoCast(AI_Spell* sp, bool on)
{
    ARCEMU_ASSERT(sp != NULL);
    ARCEMU_ASSERT(sp->spell != NULL);

    if (sp->autocast_type > 0)
    {
        if (!on)
        {
            for (std::list<AI_Spell*>::iterator itr = m_autoCastSpells[sp->autocast_type].begin(); itr != m_autoCastSpells[sp->autocast_type].end(); ++itr)
            {
                if ((*itr) == sp)
                {
                    m_autoCastSpells[sp->autocast_type].erase(itr);
                    break;
                }
            }
        }
        else
        {
            for (std::list<AI_Spell*>::iterator itr = m_autoCastSpells[sp->autocast_type].begin(); itr != m_autoCastSpells[sp->autocast_type].end(); ++itr)
            {
                if ((*itr) == sp)
                    return;
            }

            m_autoCastSpells[sp->autocast_type].push_back(sp);
        }
    }
}

uint32 Pet::GetUntrainCost()
{
    uint32 days = (uint32)(UNIXTIME - reset_time) / 60 * 60 * 24;

    if (reset_cost < 1000 || days > 0)
        reset_cost = 1000;
    else if (reset_cost < 5000)
        reset_cost = 5000;
    else if (reset_cost < 10000)
        reset_cost = 10000;
    else
        reset_cost = reset_cost + 10000 > 100000 ? 100000 : reset_cost + 10000;

    return reset_cost;
}

Group* Pet::GetGroup()
{
    if (m_Owner)
        return m_Owner->GetGroup();
    return NULL;
}

void Pet::DealDamage(Unit* pVictim, uint32 damage, uint32 /*targetEvent*/, uint32 /*unitEvent*/, uint32 spellId, bool no_remove_auras)
{
    if (!pVictim || !pVictim->isAlive() || !pVictim->IsInWorld() || !IsInWorld())
        return;
    if (pVictim->IsPlayer() && static_cast< Player* >(pVictim)->GodModeCheat == true)
        return;
    if (pVictim->bInvincible)
        return;
    if (pVictim->IsCreature() && static_cast<Creature*>(pVictim)->isSpiritHealer())
        return;

    if (pVictim != this)
        CombatStatus.OnDamageDealt(pVictim);

    pVictim->SetStandState(STANDSTATE_STAND);

    if (pVictim->IsPvPFlagged())
    {
        if (!IsPvPFlagged())
            m_Owner->PvPToggle();

        m_Owner->AggroPvPGuards();
    }

    // Bg dmg counter
    if (pVictim != this)
    {
        if (m_Owner->m_bg != NULL && GetMapMgr() == pVictim->GetMapMgr())
        {
            m_Owner->m_bgScore.DamageDone += damage;
            m_Owner->m_bg->UpdatePvPData();
        }
    }

    // Duel
    if (pVictim->IsPlayer() && m_Owner->DuelingWith != NULL && m_Owner->DuelingWith->GetGUID() == pVictim->GetGUID())
    {
        if (pVictim->GetHealth() <= damage)
        {
            uint32 NewHP = pVictim->GetMaxHealth() / 100;

            if (NewHP < 5)
                NewHP = 5;

            pVictim->SetHealth(NewHP);
            m_Owner->EndDuel(DUEL_WINNER_KNOCKOUT);
            pVictim->Emote(EMOTE_ONESHOT_BEG);
            return;
        }
    }

    if (pVictim->GetHealth() <= damage)
    {
        if (pVictim->isTrainingDummy())
        {
            pVictim->SetHealth(1);
            return;
        }

        if (m_Owner->m_bg != NULL)
        {
            m_Owner->m_bg->HookOnUnitKill(m_Owner, pVictim);

            if (pVictim->IsPlayer())
                m_Owner->m_bg->HookOnPlayerKill(m_Owner, static_cast< Player* >(pVictim));
        }

        if (pVictim->IsPlayer())
        {

            Player* playerVictim = static_cast<Player*>(pVictim);
            sHookInterface.OnKillPlayer(m_Owner, playerVictim);

            bool setAurastateFlag = false;

            if (m_Owner->getLevel() >= (pVictim->getLevel() - 8) && (GetGUID() != pVictim->GetGUID()))
            {
#if VERSION_STRING > TBC
                m_Owner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA, m_Owner->GetAreaID(), 1, 0);
                m_Owner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL, 1, 0, 0);
#endif
                HonorHandler::OnPlayerKilled(m_Owner, playerVictim);
                setAurastateFlag = true;

            }

            if (setAurastateFlag)
            {
                SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_LASTKILLWITHHONOR);

                if (!sEventMgr.HasEvent(m_Owner, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
                    sEventMgr.AddEvent(static_cast< Unit* >(m_Owner), &Unit::EventAurastateExpire, static_cast<uint32>(AURASTATE_FLAG_LASTKILLWITHHONOR), EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000, 1, 0);
                else
                    sEventMgr.ModifyEventTimeLeft(m_Owner, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000);

            }
        }
        else
        {
            if (pVictim->IsCreature())
            {
                m_Owner->Reputation_OnKilledUnit(pVictim, false);
#if VERSION_STRING > TBC
                m_Owner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLING_BLOW, GetMapId(), 0, 0);
#endif
            }
        }

        if (pVictim->IsPvPFlagged())
        {
            uint32 team = m_Owner->GetTeam();

            if (team == TEAM_ALLIANCE)
                team = TEAM_HORDE;
            else
                team = TEAM_ALLIANCE;

            auto area = pVictim->GetArea();
            sWorld.sendZoneUnderAttackMessage(area ? area->id : GetZoneId(), static_cast<uint8>(team));
        }

        pVictim->Die(this, damage, spellId);

        //////////////////////////////////////////////////////////////////////////////////////////
        //Loot
        if (pVictim->isLootable())
        {
            Player* tagger = GetMapMgr()->GetPlayer(Arcemu::Util::GUID_LOPART(pVictim->GetTaggerGUID()));

            // Tagger might have left the map so we need to check
            if (tagger != NULL)
            {
                if (tagger->InGroup())
                    tagger->GetGroup()->SendLootUpdates(pVictim);
                else
                    tagger->SendLootUpdate(pVictim);
            }
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        //Experience
        if (pVictim->GetCreatedByGUID() == 0 && !pVictim->IsPet() && pVictim->IsTagged())
        {
            auto unit_tagger = pVictim->GetMapMgr()->GetUnit(pVictim->GetTaggerGUID());

            if (unit_tagger != nullptr)
            {
                Player* player_tagger = nullptr;

                if (unit_tagger->IsPlayer())
                    player_tagger = static_cast<Player*>(unit_tagger);

                if ((unit_tagger->IsPet() || unit_tagger->IsSummon()) && unit_tagger->GetPlayerOwner())
                    player_tagger = static_cast<Player*>(unit_tagger->GetPlayerOwner());

                if (player_tagger != nullptr)
                {

                    if (player_tagger->InGroup())
                    {
                        player_tagger->GiveGroupXP(pVictim, player_tagger);
                    }
                    else if (IsUnit())
                    {
                        uint32 xp = CalculateXpToGive(pVictim, unit_tagger);

                        if (xp > 0)
                        {
                            player_tagger->GiveXP(xp, pVictim->GetGUID(), true);

                            SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_LASTKILLWITHHONOR);

                            if (!sEventMgr.HasEvent(this, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
                                sEventMgr.AddEvent(static_cast<Unit*>(this), &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_LASTKILLWITHHONOR, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                            else
                                sEventMgr.ModifyEventTimeLeft(this, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000);

                            // let's give the pet some experience too
                            if (player_tagger->GetSummon() && player_tagger->GetSummon()->CanGainXP())
                            {
                                xp = CalculateXpToGive(pVictim, player_tagger->GetSummon());

                                if (xp > 0)
                                    player_tagger->GetSummon()->GiveXP(xp);
                            }
                        }
                        //////////////////////////////////////////////////////////////////////////////////////////

                        if (pVictim->IsCreature())
                        {
                            sQuestMgr.OnPlayerKill(player_tagger, static_cast<Creature*>(pVictim), true);

#if VERSION_STRING > TBC
                            //////////////////////////////////////////////////////////////////////////////////////////
                            //Kill creature/creature type Achievements
                            if (player_tagger->InGroup())
                            {
                                auto player_group = player_tagger->GetGroup();

                                player_group->UpdateAchievementCriteriaForInrange(pVictim, ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->GetEntry(), 1, 0);
                                player_group->UpdateAchievementCriteriaForInrange(pVictim, ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, GetHighGUID(), GetLowGUID(), 0);

                            }
                            else
                            {
                                player_tagger->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->GetEntry(), 1, 0);
                                player_tagger->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, GetHighGUID(), GetLowGUID(), 0);
                            }
#endif
                        }
                    }
                }
            }
        }

#if VERSION_STRING > TBC
        if (pVictim->isCritter())
        {
            m_Owner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->GetEntry(), 1, 0);
            m_Owner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, GetHighGUID(), GetLowGUID(), 0);
        }
#endif
    }
    else
    {
        pVictim->TakeDamage(this, damage, spellId, no_remove_auras);
    }
}

void Pet::TakeDamage(Unit* pAttacker, uint32 damage, uint32 spellid, bool no_remove_auras)
{
    if (!no_remove_auras)
    {
        //zack 2007 04 24 : root should not remove self (and also other unknown spells)
        if (spellid)
        {
            RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN, spellid);
            if (Rand(35.0f))
                RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_UNUSED2, spellid);
        }
        else
        {
            RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);
            if (Rand(35.0f))
                RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_UNUSED2);
        }
    }

    GetAIInterface()->AttackReaction(pAttacker, damage, spellid);

    ModHealth(-1 * static_cast<int32>(damage));
}

void Pet::Die(Unit* pAttacker, uint32 /*damage*/, uint32 spellid)
{
    //general hook for die
    if (!sHookInterface.OnPreUnitDie(pAttacker, this))
        return;

    // on die and an target die proc
    {
        SpellInfo* killerspell;
        if (spellid)
            killerspell = sSpellCustomizations.GetSpellInfo(spellid);
        else killerspell = NULL;

        HandleProc(PROC_ON_DIE, this, killerspell);
        m_procCounter = 0;
        pAttacker->HandleProc(PROC_ON_TARGET_DIE, this, killerspell);
        pAttacker->m_procCounter = 0;
    }

    setDeathState(JUST_DIED);
    GetAIInterface()->HandleEvent(EVENT_LEAVECOMBAT, this, 0);

    if (GetChannelSpellTargetGUID() != 0)
    {

        Spell* spl = getCurrentSpell(CURRENT_CHANNELED_SPELL);

        if (spl != nullptr)
        {

            for (uint8 i = 0; i < 3; i++)
            {
                if (spl->GetSpellInfo()->getEffect(i) == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                {
                    uint64 guid = GetChannelSpellTargetGUID();
                    DynamicObject* dObj = GetMapMgr()->GetDynamicObject(Arcemu::Util::GUID_LOPART(guid));
                    if (!dObj)
                        return;

                    dObj->Remove();
                }
            }

            if (spl->GetSpellInfo()->getChannelInterruptFlags() == 48140) interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);
        }
    }

    //Stop players from casting
    for (std::set< Object* >::iterator itr = GetInRangePlayerSetBegin(); itr != GetInRangePlayerSetEnd(); ++itr)
    {
        Unit* attacker = static_cast< Unit* >(*itr);

        if (attacker->isCastingNonMeleeSpell())
        {
            for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
            {
                Spell* curSpell = attacker->getCurrentSpell(CurrentSpellType(i));
                if (curSpell != nullptr && curSpell->m_targets.m_unitTarget == GetGUID())
                    attacker->interruptSpellWithSpellType(CurrentSpellType(i));
            }
        }
    }

    smsg_AttackStop(this);
    SetHealth(0);

    // Wipe our attacker set on death
    CombatStatus.Vanished();

    CALL_SCRIPT_EVENT(pAttacker, OnTargetDied)(this);
    pAttacker->smsg_AttackStop(this);

    GetAIInterface()->OnDeath(pAttacker);

    {
        //////////////////////////////////////////////////////////////////////////////////////////
        //PET DEATH HANDLING
        Pet* pPet = this;

        // dying pet looses 1 happiness level (not in BG)
        if (!pPet->IsSummonedPet() && !pPet->IsInBg())
        {
            pPet->ModPower(POWER_TYPE_HAPPINESS, -PET_HAPPINESS_UPDATE_VALUE);
        }
        pPet->DelayedRemove(false);
    }   //////////////////////////////////////////////////////////////////////////////////////////

    if (m_mapMgr->m_battleground != NULL)
        m_mapMgr->m_battleground->HookOnUnitDied(this);
}

Object* Pet::GetPlayerOwner()
{
    return m_Owner;
}
