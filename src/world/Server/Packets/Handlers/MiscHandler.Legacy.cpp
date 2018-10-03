/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#include "Management/Item.h"
#include "Management/WeatherMgr.h"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.h"
#include "Server/WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Server/MainServerDefines.h"
#include "zlib.h"
#include "Map/MapMgr.h"
#include "Spell/SpellMgr.h"
#include "Map/WorldCreator.h"
#include "Spell/Definitions/LockTypes.h"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Server/Packets/SmsgLootRemoved.h"
#include "Server/Packets/CmsgAutostoreLootItem.h"
#include "Server/Packets/CmsgLootMasterGive.h"
#include "Server/Packets/CmsgLootRoll.h"
#include "Server/Packets/CmsgOpenItem.h"
#include "Management/GuildMgr.h"
#include "Server/Packets/CmsgGameobjUse.h"
#include "Server/Packets/SmsgStandstateUpdate.h"
#include "WoWGuid.h"
#include "Server/Packets/CmsgLoot.h"
#include "Server/Packets/SmsgLootMasterList.h"
#include "Server/Packets/SmsgLootMoneyNotify.h"
#include "Server/Packets/CmsgLootRelease.h"
#include "Server/Packets/SmsgLootReleaseResponse.h"
#include "Server/Packets/CmsgWhoIs.h"
#include "Server/Packets/CmsgBug.h"
#include "Server/Packets/CmsgReclaimCorpse.h"
#include "Server/Packets/SmsgResurrectFailed.h"
#include "Server/Packets/CmsgAlterAppearance.h"
#include "Server/Packets/SmsgBarberShopResult.h"
#include "Server/Packets/CmsgInspect.h"
#include "Server/Packets/CmsgSummonResponse.h"
#include "Server/Packets/CmsgRemoveGlyph.h"

using namespace AscEmu::Packets;

void WorldSession::HandleRepopRequestOpcode(WorldPacket& /*recvData*/)
{
    CHECK_INWORLD_RETURN

    LOG_DEBUG("WORLD: Recvd CMSG_REPOP_REQUEST Message");
    if (_player->getDeathState() != JUST_DIED)
        return;
#if VERSION_STRING != Cata
    if (_player->obj_movement_info.isOnTransport())
#else
    if (!_player->obj_movement_info.getTransportGuid().IsEmpty())
#endif
    {
        auto transport = _player->GetTransport();
        if (transport != nullptr)
        {
            transport->RemovePassenger(_player);
        }
    }

    GetPlayer()->RepopRequestedPlayer();
}

// LOOT related
//////////////////////////////////////////////////////////////////////////////////////////
// MISC
void WorldSession::HandleWhoIsOpcode(WorldPacket& recv_data)
{
    CmsgWhoIs srlPacket;
    if (!srlPacket.deserialise(recv_data))
        return;

    if (!GetPlayer()->GetSession()->CanUseCommand('3'))
    {
        SendNotification("You do not have permission to perform that function.");
        return;
    }

    if (srlPacket.characterName.empty())
    {
        SendNotification("You did not enter a character name!");
        return;
    }

    QueryResult* result_acctID = CharacterDatabase.Query("SELECT acct FROM characters WHERE name = '%s'", srlPacket.characterName.c_str());
    if (!result_acctID)
    {
        SendNotification("%s does not exit!", srlPacket.characterName.c_str());
        delete result_acctID;
        return;
    }

    Field* fields_acctID = result_acctID->Fetch();
    uint32 accid = fields_acctID[0].GetUInt32();
    delete result_acctID;

    //TODO - this will not work! no table accounts in character_db!!!
    QueryResult* result = CharacterDatabase.Query("SELECT acct, login, gm, email, lastip, muted FROM accounts WHERE acct = %u", accid);

    if (!result)
    {
        SendNotification("Account information for %s not found!", srlPacket.characterName.c_str());
        delete result;
        return;
    }

    Field* fields = result->Fetch();
    std::string acctID = fields[0].GetString();
    if (acctID.empty())
        acctID = "Unknown";

    std::string acctName = fields[1].GetString();
    if (acctName.empty())
        acctName = "Unknown";

    std::string acctPerms = fields[2].GetString();
    if (acctPerms.empty())
        acctPerms = "Unknown";

    std::string acctEmail = fields[3].GetString();
    if (acctEmail.empty())
        acctEmail = "Unknown";

    std::string acctIP = fields[4].GetString();
    if (acctIP.empty())
        acctIP = "Unknown";

    std::string acctMuted = fields[5].GetString();
    if (acctMuted.empty())
        acctMuted = "Unknown";

    delete result;

    std::string msg = srlPacket.characterName + "'s " + "account information: acctID: " + acctID + ", Name: " + acctName + ", Permissions: " + acctPerms + ", E-Mail: " + acctEmail + ", lastIP: " + acctIP + ", Muted: " + acctMuted;

    WorldPacket data(SMSG_WHOIS, msg.size() + 1);
    data << msg;
    SendPacket(&data);
    LogDebugFlag(LF_OPCODE, "Received WHOIS command from player %s for character %s", GetPlayer()->getName().c_str(), srlPacket.characterName.c_str());
}

void WorldSession::HandleRequestAccountData(WorldPacket& recv_data)
{
    LOG_DETAIL("WORLD: Received CMSG_REQUEST_ACCOUNT_DATA");

    uint32 id;
    if (!worldConfig.server.useAccountData)
        return;
    recv_data >> id;

    if (id > 8)
    {
        // Shit..
        LOG_ERROR("WARNING: Accountdata > 8 (%d) was requested by %s of account %d!", id, GetPlayer()->getName().c_str(), this->GetAccountId());
        return;
    }

    AccountDataEntry* res = GetAccountData(id);
    WorldPacket data;
    data.SetOpcode(SMSG_UPDATE_ACCOUNT_DATA);
    data << id;
    // if red does not exists if ID == 7 and if there is no data send 0
    if (!res || !res->data)  // if error, send a NOTHING packet
    {
        data << uint32(0);
    }
    else
    {
        data << res->sz;
        uLongf destsize;
        if (res->sz > 200)
        {
            data.resize(res->sz + 800);  // give us plenty of room to work with..

            if ((compress(const_cast<uint8*>(data.contents()) + (sizeof(uint32) * 2), &destsize, (const uint8*)res->data, res->sz)) != Z_OK)
            {
                LOG_ERROR("Error while compressing ACCOUNT_DATA");
                return;
            }

            data.resize(destsize + 8);
        }
        else
            data.append(res->data, res->sz);
    }

    SendPacket(&data);
}

void WorldSession::HandleAmmoSetOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint32 ammoId;
    recv_data >> ammoId;

    if (!ammoId)
        return;

    ItemProperties const* xproto = sMySQLStore.getItemProperties(ammoId);
    if (!xproto)
        return;

    if (xproto->Class != ITEM_CLASS_PROJECTILE || GetPlayer()->GetItemInterface()->GetItemCount(ammoId) == 0)
    {
        sCheatLog.writefromsession(GetPlayer()->GetSession(), "Definitely cheating. tried to add %u as ammo.", ammoId);
        GetPlayer()->GetSession()->Disconnect();
        return;
    }

    if (xproto->RequiredLevel)
    {
        if (GetPlayer()->getLevel() < xproto->RequiredLevel)
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_ITEM_RANK_NOT_ENOUGH);
            _player->SetAmmoId(0);
            _player->CalcDamage();
            return;
        }
    }
    if (xproto->RequiredSkill)
    {
        if (!GetPlayer()->_HasSkillLine(xproto->RequiredSkill))
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_ITEM_RANK_NOT_ENOUGH);
            _player->SetAmmoId(0);
            _player->CalcDamage();
            return;
        }

        if (xproto->RequiredSkillRank)
        {
            if (_player->_GetSkillLineCurrent(xproto->RequiredSkill, false) < xproto->RequiredSkillRank)
            {
                GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_ITEM_RANK_NOT_ENOUGH);
                _player->SetAmmoId(0);
                _player->CalcDamage();
                return;
            }
        }
    }
    switch (_player->getClass())
    {
        case PRIEST:  // allowing priest, warlock, mage to equip ammo will mess up wand shoot. stop it.
        case WARLOCK:
        case MAGE:
        case SHAMAN: // these don't get messed up since they don't use wands, but they don't get to use bows/guns/crossbows anyways
        case DRUID:  // we wouldn't want them cheating extra stats from ammo, would we?
        case PALADIN:
#if VERSION_STRING > TBC
        case DEATHKNIGHT:
#endif
            _player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM); // good error message?
            _player->SetAmmoId(0);
            _player->CalcDamage();
            return;
        default:
            _player->SetAmmoId(ammoId);
            _player->CalcDamage();
            break;
    }
}

#define OPEN_CHEST 11437

#if VERSION_STRING > TBC

namespace BarberShopResult
{
    enum
    {
        Ok = 0,
        NoMoney = 1
    };
}

void WorldSession::HandleBarberShopResult(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CmsgAlterAppearance srlPacket;
    if (!srlPacket.deserialise(recv_data))
        return;

    LOG_DEBUG("WORLD: CMSG_ALTER_APPEARANCE ");

    uint32 oldhair = _player->getHairStyle();
    uint32 oldhaircolor = _player->getHairColor();
    uint32 oldfacial = _player->getFacialFeatures();
    // uint32 oldskincolor = _player->getSkinColor();

    uint32 newhair, newhaircolor, newfacial;

    uint32 cost = 0;

    auto barberShopHair = sBarberShopStyleStore.LookupEntry(srlPacket.hair);
    if (!barberShopHair)
        return;
    newhair = barberShopHair->hair_id;

    newhaircolor = srlPacket.hairColor;

    auto barberShopFacial = sBarberShopStyleStore.LookupEntry(srlPacket.facialHairOrPiercing);
    if (!barberShopFacial)
        return;
    newfacial = barberShopFacial->hair_id;

    auto barberShopSkinColor = sBarberShopStyleStore.LookupEntry(srlPacket.skinColor);
    if (barberShopSkinColor && barberShopSkinColor->race != _player->getRace())
        return;

    uint32 level = _player->getLevel();
    if (level >= 100)
        level = 100;
    auto cutcosts = sBarberShopCostBaseStore.LookupEntry(level - 1);
    if (!cutcosts)
        return;

    // hair style cost = cutcosts
    // hair color cost = cutcosts * 0.5 or free if hair style changed
    // facial hair cost = cutcosts * 0.75
    if (newhair != oldhair)
    {
        cost += (uint32)cutcosts->cost;
    }
    else if (newhaircolor != oldhaircolor)
    {
        cost += (uint32)(cutcosts->cost) >> 1;
    }
    if (newfacial != oldfacial)
    {
        cost += (uint32)(cutcosts->cost * 0.75f);
    }

    if (!_player->HasGold(cost))
    {
        SendPacket(SmsgBarberShopResult(BarberShopResult::NoMoney).serialise().get());
        return;
    }

    SendPacket(SmsgBarberShopResult(BarberShopResult::Ok).serialise().get());

    _player->setHairStyle(static_cast<uint8>(newhair));
    _player->setHairColor(static_cast<uint8>(newhaircolor));
    _player->setFacialFeatures(static_cast<uint8>(newfacial));
    if (barberShopSkinColor)
        _player->setSkinColor(static_cast<uint8>(barberShopSkinColor->hair_id));
    _player->ModGold(-(int32)cost);

    _player->setStandState(STANDSTATE_STAND);                              // stand up
    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP, 1, 0, 0);
    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER, cost, 0, 0);
}
#endif

void WorldSession::HandleGameObjectUse(WorldPacket& recv_data)
{
    CmsgGameobjUse recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    LOG_DEBUG("WORLD: CMSG_GAMEOBJ_USE: [GUID %d]", recv_packet.guid.getGuidLowPart());

    GameObject* obj = _player->GetMapMgr()->GetGameObject(recv_packet.guid.getGuidLowPart());
    if (!obj)
        return;
    auto gameobject_info = obj->GetGameObjectProperties();
    if (!gameobject_info)
        return;

    Player* plyr = GetPlayer();

    //Event Scripts
    objmgr.CheckforScripts(plyr, obj->GetGameObjectProperties()->raw.parameter_9);

    CALL_GO_SCRIPT_EVENT(obj, OnActivate)(_player);
    CALL_INSTANCE_SCRIPT_EVENT(_player->GetMapMgr(), OnGameObjectActivate)(obj, _player);

    _player->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH); // cebernic:RemoveStealth due to GO was using. Blizzlike

    SpellCastTargets targets;
    Spell* spell = nullptr;
    SpellInfo* spellInfo = nullptr;

    uint32 type = obj->getGoType();
    switch (type)
    {
        case GAMEOBJECT_TYPE_CHAIR:
        {
            plyr->SafeTeleport(plyr->GetMapId(), plyr->GetInstanceID(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());
            plyr->setStandState(STANDSTATE_SIT_MEDIUM_CHAIR);
            SendPacket(SmsgStandstateUpdate(STANDSTATE_SIT_MEDIUM_CHAIR).serialise().get());

            plyr->UpdateSpeed();
        }
        break;
#if VERSION_STRING > TBC
        case GAMEOBJECT_TYPE_BARBER_CHAIR:
        {
            plyr->SafeTeleport(plyr->GetMapId(), plyr->GetInstanceID(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());
            plyr->UpdateSpeed();
            //send barber shop menu to player
            WorldPacket data(SMSG_ENABLE_BARBER_SHOP, 0);
            SendPacket(&data);
            plyr->setStandState(STANDSTATE_SIT_HIGH_CHAIR);
        }
        break;
#endif
        case GAMEOBJECT_TYPE_CHEST:     //cast da spell
        {
            spellInfo = sSpellCustomizations.GetSpellInfo(OPEN_CHEST);
            spell = sSpellFactoryMgr.NewSpell(plyr, spellInfo, true, NULL);
            targets.m_unitTarget = obj->getGuid();
            spell->prepare(&targets);
        }
        break;
        case GAMEOBJECT_TYPE_FISHINGNODE:
        {
            sEventMgr.RemoveEvents(_player, EVENT_STOP_CHANNELING);

            GameObject_FishingNode* fn = static_cast<GameObject_FishingNode*>(obj);

            bool success = fn->UseNode();

            uint32 zone = 0;
            MySQLStructure::FishingZones const* entry = nullptr;

            if (success)
            {
                zone = plyr->GetAreaID();

                if (zone == 0)                  // If the player's area ID is 0, use the zone ID instead
                    zone = plyr->GetZoneId();

                entry = sMySQLStore.getFishingZone(zone);
                if (entry == nullptr)
                {
                    LogError("ERROR: Fishing zone information for zone %d not found!", zone);
                    fn->EndFishing(true);
                    success = false;
                }
            }

            if (success)
            {
                uint32 maxskill = entry->maxSkill;
                uint32 minskill = entry->minSkill;

                if (plyr->_GetSkillLineCurrent(SKILL_FISHING, false) < maxskill)
                    plyr->_AdvanceSkillLine(SKILL_FISHING, float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE)));

                GameObject_FishingHole* school = nullptr;

                GameObject* go = fn->GetMapMgr()->FindNearestGoWithType(fn, GAMEOBJECT_TYPE_FISHINGHOLE);
                if (go != nullptr)
                {
                    school = dynamic_cast<GameObject_FishingHole*>(go);

                    if (!fn->isInRange(school, static_cast<float>(school->GetGameObjectProperties()->fishinghole.radius)))
                        school = nullptr;
                }

                if (school != nullptr)
                {
                    if (school->GetMapMgr() != nullptr)
                        lootmgr.FillGOLoot(&school->loot, school->GetGameObjectProperties()->raw.parameter_1, school->GetMapMgr()->iInstanceMode);
                    else
                        lootmgr.FillGOLoot(&school->loot, school->GetGameObjectProperties()->raw.parameter_1, 0);

                    plyr->SendLoot(school->getGuid(), LOOT_FISHING, school->GetMapId());
                    fn->EndFishing(false);
                    school->CatchFish();

                }
                else if (maxskill != 0 && Rand(((plyr->_GetSkillLineCurrent(SKILL_FISHING, true) - minskill) * 100) / maxskill))
                {
                    lootmgr.FillFishingLoot(&fn->loot, zone);
                    plyr->SendLoot(fn->getGuid(), LOOT_FISHING, fn->GetMapId());
                    fn->EndFishing(false);
                }
                else
                {
                    plyr->GetSession()->OutPacket(SMSG_FISH_ESCAPED);
                    fn->EndFishing(true);
                }
            }
            else
            {
                plyr->GetSession()->OutPacket(SMSG_FISH_NOT_HOOKED);
            }

            // Fishing is channeled spell
            auto channelledSpell = plyr->getCurrentSpell(CURRENT_CHANNELED_SPELL);
            if (channelledSpell != nullptr)
            {
                if (success)
                {
                    channelledSpell->SendChannelUpdate(0);
                    channelledSpell->finish(true);
                }
                else
                {
                    channelledSpell->SendChannelUpdate(0);
                    channelledSpell->finish(false);
                }
            }
        }
        break;
        case GAMEOBJECT_TYPE_DOOR:
        {
            obj->Use(plyr->getGuid());
        }
        break;
        case GAMEOBJECT_TYPE_BUTTON:
        {
            obj->Use(plyr->getGuid());
        }
        break;
        case GAMEOBJECT_TYPE_FLAGSTAND:
        {
            // battleground/warsong gulch flag
            if (plyr->m_bg)
                plyr->m_bg->HookFlagStand(plyr, obj);
        }
        break;
        case GAMEOBJECT_TYPE_FLAGDROP:
        {
            // Dropped flag
            if (plyr->m_bg)
                plyr->m_bg->HookFlagDrop(plyr, obj);
        }
        break;
        case GAMEOBJECT_TYPE_QUESTGIVER:
        {
            GameObject_QuestGiver* go_quest_giver = static_cast<GameObject_QuestGiver*>(obj);
            // Questgiver
            if (go_quest_giver->HasQuests())
            {
                sQuestMgr.OnActivateQuestGiver(obj, plyr);
            }
        }
        break;
        case GAMEOBJECT_TYPE_SPELLCASTER:
        {
            if (obj->GetGameObjectProperties()->spell_caster.party_only != 0)
            {
                if (obj->m_summoner != NULL && obj->m_summoner->isPlayer())
                {
                    Player* summoner = static_cast<Player*>(obj->m_summoner);

                    if (summoner->getGuid() != plyr->getGuid())
                    {
                        if (!plyr->InGroup())
                            return;

                        if (plyr->GetGroup() != summoner->GetGroup())
                            return;
                    }
                }
            }

            obj->Use(plyr->getGuid());
        }
        break;
        case GAMEOBJECT_TYPE_RITUAL:
        {
            // store the members in the ritual, cast sacrifice spell, and summon.
            GameObject_Ritual* ritual_obj = static_cast<GameObject_Ritual*>(obj);
            if (ritual_obj->GetRitual()->IsFinished() || ritual_obj->GetRitual()->GetCasterGUID() == 0)
                return;

            // If we clicked on the ritual we are already in, remove us, otherwise add us as a ritual member
            if (ritual_obj->GetRitual()->HasMember(plyr->getGuidLow()))
            {
                ritual_obj->GetRitual()->RemoveMember(plyr->getGuidLow());
                plyr->setChannelSpellId(0);
                plyr->setChannelObjectGuid(0);
                return;
            }
            else
            {
                ritual_obj->GetRitual()->AddMember(plyr->getGuidLow());
                plyr->setChannelSpellId(ritual_obj->GetRitual()->GetSpellID());
                plyr->setChannelObjectGuid(ritual_obj->getGuid());
            }

            // If we were the last required member, proceed with the ritual!
            if (!ritual_obj->GetRitual()->HasFreeSlots())
            {
                ritual_obj->GetRitual()->Finish();
                Player* plr = nullptr;

                unsigned long MaxMembers = ritual_obj->GetRitual()->GetMaxMembers();
                for (unsigned long i = 0; i < MaxMembers; i++)
                {
                    plr = plyr->GetMapMgr()->GetPlayer(ritual_obj->GetRitual()->GetMemberGUIDBySlot(i));
                    if (plr != nullptr)
                    {
                        plr->setChannelObjectGuid(0);
                        plr->setChannelSpellId(0);
                    }
                }

                SpellInfo* info = nullptr;
                if (gameobject_info->entry == 36727 || gameobject_info->entry == 194108)   // summon portal
                {
                    if (!ritual_obj->GetRitual()->GetTargetGUID() == 0)
                        return;

                    info = sSpellCustomizations.GetSpellInfo(gameobject_info->summoning_ritual.spell_id);
                    if (info == nullptr)
                        break;

                    Player* target = objmgr.GetPlayer(ritual_obj->GetRitual()->GetTargetGUID());
                    if (target == nullptr || !target->IsInWorld())
                        return;

                    spell = sSpellFactoryMgr.NewSpell(_player->GetMapMgr()->GetPlayer(ritual_obj->GetRitual()->GetCasterGUID()), info, true, NULL);
                    targets.m_unitTarget = target->getGuid();
                    spell->prepare(&targets);
                }
                else if (gameobject_info->entry == 177193)    // doom portal
                {
                    Player* psacrifice = nullptr;

                    uint32 victimid = Util::getRandomUInt(ritual_obj->GetRitual()->GetMaxMembers() - 1);

                    // kill the sacrifice player
                    psacrifice = _player->GetMapMgr()->GetPlayer(ritual_obj->GetRitual()->GetMemberGUIDBySlot(victimid));
                    Player* pCaster = obj->GetMapMgr()->GetPlayer(ritual_obj->GetRitual()->GetCasterGUID());
                    if (!psacrifice || !pCaster)
                        return;

                    info = sSpellCustomizations.GetSpellInfo(gameobject_info->summoning_ritual.caster_target_spell);
                    if (!info)
                        break;

                    spell = sSpellFactoryMgr.NewSpell(psacrifice, info, true, NULL);
                    targets.m_unitTarget = psacrifice->getGuid();
                    spell->prepare(&targets);

                    // summons demon
                    info = sSpellCustomizations.GetSpellInfo(gameobject_info->summoning_ritual.spell_id);
                    spell = sSpellFactoryMgr.NewSpell(pCaster, info, true, NULL);
                    SpellCastTargets targets2;
                    targets2.m_unitTarget = pCaster->getGuid();
                    spell->prepare(&targets2);
                }
                else if (gameobject_info->entry == 179944)    // Summoning portal for meeting stones
                {
                    plr = _player->GetMapMgr()->GetPlayer(ritual_obj->GetRitual()->GetTargetGUID());
                    if (!plr)
                        return;

                    Player* pleader = _player->GetMapMgr()->GetPlayer(ritual_obj->GetRitual()->GetCasterGUID());
                    if (!pleader)
                        return;

                    info = sSpellCustomizations.GetSpellInfo(gameobject_info->summoning_ritual.spell_id);
                    spell = sSpellFactoryMgr.NewSpell(pleader, info, true, NULL);
                    SpellCastTargets targets2(plr->getGuid());
                    spell->prepare(&targets2);

                    /* expire the gameobject */
                    ritual_obj->ExpireAndDelete();
                }
                else if (gameobject_info->entry == 186811 || gameobject_info->entry == 181622)
                {
                    info = sSpellCustomizations.GetSpellInfo(gameobject_info->summoning_ritual.spell_id);
                    if (info == NULL)
                        return;

                    spell = sSpellFactoryMgr.NewSpell(_player->GetMapMgr()->GetPlayer(ritual_obj->GetRitual()->GetCasterGUID()), info, true, NULL);
                    SpellCastTargets targets2(ritual_obj->GetRitual()->GetCasterGUID());
                    spell->prepare(&targets2);
                    ritual_obj->ExpireAndDelete();
                }
            }
        }
        break;
        case GAMEOBJECT_TYPE_GOOBER:
        {
            obj->Use(plyr->getGuid());

            plyr->CastSpell(recv_packet.guid.GetOldGuid(), gameobject_info->goober.spell_id, false);

            // show page
            if (gameobject_info->goober.page_id)
            {
                WorldPacket data(SMSG_GAMEOBJECT_PAGETEXT, 8);
                data << obj->getGuid();
                plyr->GetSession()->SendPacket(&data);
            }
        }
        break;
        case GAMEOBJECT_TYPE_CAMERA://eye of azora
        {
            if (gameobject_info->camera.cinematic_id != 0)
            {
                plyr->GetSession()->OutPacket(SMSG_TRIGGER_CINEMATIC, 4, &gameobject_info->camera.cinematic_id);
            }
        }
        break;
        case GAMEOBJECT_TYPE_MEETINGSTONE:	// Meeting Stone
        {
            /* Use selection */
            Player* pPlayer = objmgr.GetPlayer((uint32)_player->GetSelection());
            if (pPlayer == nullptr)
                return;

            // If we are not in a group we can't summon anyone
            if (!_player->InGroup())
                return;

            // We can only summon someone if they are in our raid/group
            if (_player->GetGroup() != pPlayer->GetGroup())
                return;

            // We can't summon ourselves!
            if (pPlayer->getGuid() == _player->getGuid())
                return;

            // Create the summoning portal
            GameObject* pGo = _player->GetMapMgr()->CreateGameObject(179944);
            if (pGo == nullptr)
                return;

            GameObject_Ritual* rGo = static_cast<GameObject_Ritual*>(pGo);

            rGo->CreateFromProto(179944, _player->GetMapId(), _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), 0);
            rGo->GetRitual()->Setup(_player->getGuidLow(), pPlayer->getGuidLow(), 18540);
            rGo->PushToWorld(_player->GetMapMgr());

            _player->setChannelObjectGuid(rGo->getGuid());
            _player->setChannelSpellId(rGo->GetRitual()->GetSpellID());

            // expire after 2mins
            sEventMgr.AddEvent(pGo, &GameObject::_Expire, EVENT_GAMEOBJECT_EXPIRE, 120000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
        break;
    }
}

void WorldSession::HandleInspectOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN;

    CmsgInspect srlPacket;
    if (!srlPacket.deserialise(recv_data))
        return;

    ByteBuffer m_Packed_GUID;
    Player* player = _player->GetMapMgr()->GetPlayer((uint32)srlPacket.guid);

    if (player == NULL)
    {
        LOG_ERROR("HandleInspectOpcode: guid was null");
        return;
    }

    _player->setTargetGuid(srlPacket.guid);
    _player->SetSelection(srlPacket.guid);

    if (_player->m_comboPoints)
        _player->UpdateComboPoints();

    WorldPacket data(SMSG_INSPECT_TALENT, 1000);
    m_Packed_GUID.appendPackGUID(player->getGuid());
    data.append(m_Packed_GUID);

    //data.appendPackGUID(guid);
    //data.appendPackGUID(player->getGuid());
    //data << player->GetNewGUID();
#ifdef SAVE_BANDWIDTH
    PlayerSpec *currSpec = &player->getActiveSpec();
    data << uint32(currSpec->GetTP());
    data << uint8(1) << uint8(0);
    data << uint8(currSpec->talents.size()); //fake value, will be overwritten at the end
    for (std::map<uint32, uint8>::iterator itr = currSpec->talents.begin(); itr != currSpec->talents.end(); itr++)
        data << itr->first << itr->second;
    data << uint8(0); // Send Glyph info
#else
    data << uint32(player->getActiveSpec().GetTP());
    data << uint8(player->m_talentSpecsCount);
    data << uint8(player->m_talentActiveSpec);
    for (uint8 s = 0; s < player->m_talentSpecsCount; s++)
    {
#ifdef FT_DUAL_SPEC
        PlayerSpec spec = player->m_specs[s];
#else
        PlayerSpec spec = player->m_spec;
#endif

        int32 talent_max_rank;
        uint32 const* talent_tab_ids;

        uint8 talent_count = 0;
        size_t pos = data.wpos();
        data << uint8(talent_count); //fake value, will be overwritten at the end

        talent_tab_ids = getTalentTabPages(player->getClass());

        for (uint8 i = 0; i < 3; ++i)
        {
            uint32 talent_tab_id = talent_tab_ids[i];

            for (uint32 j = 0; j < sTalentStore.GetNumRows(); ++j)
            {
                auto talent_info = sTalentStore.LookupEntry(j);
                if (talent_info == nullptr)
                    continue;

                if (talent_info->TalentTree != talent_tab_id)
                    continue;

                talent_max_rank = -1;
                for (int32 k = 4; k > -1; --k)
                {
                    //LOG_DEBUG("HandleInspectOpcode: k(%i) RankID(%i) HasSpell(%i) TalentTree(%i) Tab(%i)", k, talent_info->RankID[k - 1], player->HasSpell(talent_info->RankID[k - 1]), talent_info->TalentTree, talent_tab_id);
                    if (talent_info->RankID[k] != 0 && player->HasSpell(talent_info->RankID[k]))
                    {
                        talent_max_rank = k;
                        break;
                    }
                }

                //LOG_DEBUG("HandleInspectOpcode: RankID(%i) talent_max_rank(%i)", talent_info->RankID[talent_max_rank-1], talent_max_rank);

                if (talent_max_rank < 0)
                    continue;

                data << uint32(talent_info->TalentID);
                data << uint8(talent_max_rank);

                ++talent_count;

                //LOG_DEBUG("HandleInspectOpcode: talent(%i) talent_max_rank(%i) rank_id(%i) talent_index(%i) talent_tab_pos(%i) rank_index(%i) rank_slot(%i) rank_offset(%i) mask(%i)", talent_info->TalentID, talent_max_rank, talent_info->RankID[talent_max_rank-1], talent_index, talent_tab_pos, rank_index, rank_slot, rank_offset , mask);
            }
        }

        data.put<uint8>(pos, talent_count);

#ifdef FT_GLYPHS
        // Send Glyph info
        data << uint8(GLYPHS_COUNT);
        for (uint8 i = 0; i < GLYPHS_COUNT; i++)
            data << uint16(spec.glyphs[i]);
#endif

    }
#endif

    // ----[ Build the item list with their enchantments ]----
    uint32 slot_mask = 0;
    size_t slot_mask_pos = data.wpos();
    data << uint32(slot_mask);   // VLack: 3.1, this is a mask field, if we send 0 we can skip implementing this for now; here should come the player's enchantments from its items (the ones you would see on the character sheet).

    ItemInterface* iif = player->GetItemInterface();

    for (uint32 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)   // Ideally this goes from 0 to 18 (EQUIPMENT_SLOT_END is 19 at the moment)
    {
        Item* item = iif->GetInventoryItem(static_cast<uint16>(i));

        if (!item)
            continue;

        slot_mask |= (1 << i);

        data << uint32(item->getEntry());

        uint16 enchant_mask = 0;
        size_t enchant_mask_pos = data.wpos();

        data << uint16(enchant_mask);

        for (uint8_t Slot = 0; Slot < MAX_ENCHANTMENT_SLOT; ++Slot) // In UpdateFields.h we have ITEM_FIELD_ENCHANTMENT_1_1 to ITEM_FIELD_ENCHANTMENT_12_1, iterate on them...
        {
            uint32 enchantId = item->getEnchantmentId(Slot);   // This calculation has to be in sync with Item.cpp line ~614, at the moment it is:    uint32 EnchantBase = Slot * 3 + ITEM_FIELD_ENCHANTMENT_1_1;

            if (!enchantId)
                continue;

            enchant_mask |= (1 << Slot);
            data << uint16(enchantId);
        }

        data.put<uint16>(enchant_mask_pos, enchant_mask);

        data << uint16(0);   // UNKNOWN
        FastGUIDPack(data, item->getCreatorGuid());  // Usually 0 will do, but if your friend created that item for you, then it is nice to display it when you get inspected.
        data << uint32(0);   // UNKNOWN
    }
    data.put<uint32>(slot_mask_pos, slot_mask);

#if VERSION_STRING == Cata
    if (Guild* guild = sGuildMgr.getGuildById(player->getGuildId()))
    {
        data << guild->getGUID();
        data << uint32(guild->getLevel());
        data << uint64(guild->getExperience());
        data << uint32(guild->getMembersCount());
    }
#endif

    SendPacket(&data);
}

#if VERSION_STRING != Cata
void WorldSession::HandleAcknowledgementOpcodes(WorldPacket& recv_data)
{
    LogDebugFlag(LF_OPCODE, "Opcode %s (%u) received. This opcode is not known/implemented right now!", getOpcodeName(recv_data.GetOpcode()).c_str(), recv_data.GetOpcode());
}
#endif

#if VERSION_STRING > TBC
void WorldSession::HandleRemoveGlyph(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CmsgRemoveGlyph srlPacket;
    if (!srlPacket.deserialise(recv_data))
        return;

    if (srlPacket.glyphNumber > 5)
        return; // Glyph doesn't exist

    // Get info
    uint32 glyphId = _player->GetGlyph(srlPacket.glyphNumber);
    if (glyphId == 0)
        return;

    auto glyph_properties = sGlyphPropertiesStore.LookupEntry(glyphId);
    if (!glyph_properties)
        return;

    _player->SetGlyph(srlPacket.glyphNumber, 0);
    _player->removeAllAurasById(glyph_properties->SpellID);
    _player->m_specs[_player->m_talentActiveSpec].glyphs[srlPacket.glyphNumber] = 0;
    _player->smsg_TalentsInfo(false);
}
#endif
