/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
 */

#include "Setup.h"
#include "Management/QuestLogEntry.hpp"
#include "Management/Skill.h"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/MapMgr.h"
#include "Map/MapScriptInterface.h"
#include "Spell/SpellMgr.h"
#include "Spell/SpellAuras.h"
#include <Spell/Customization/SpellCustomizations.hpp>

bool FrostWarding(uint8_t /*effectIndex*/, Spell* s)
{
    Unit* unitTarget = s->GetUnitTarget();

    if (!unitTarget)
        return false;

    uint32 spellId = s->GetSpellInfo()->getId();

    unitTarget->RemoveReflect(spellId, true);

    ReflectSpellSchool* rss = new ReflectSpellSchool;

    rss->chance = s->GetSpellInfo()->getProcChance();
    rss->spellId = s->GetSpellInfo()->getId();
    rss->school = SCHOOL_FROST;
    rss->infront = false;
    rss->charges = 0;

    unitTarget->m_reflectSpellSchool.push_back(rss);

    return true;
}

bool MoltenShields(uint8_t /*effectIndex*/, Spell* s)
{
    Unit* unitTarget = s->GetUnitTarget();

    if (!unitTarget)
        return false;

    unitTarget->RemoveReflect(s->GetSpellInfo()->getId(), true);

    ReflectSpellSchool* rss = new ReflectSpellSchool;

    rss->chance = s->GetSpellInfo()->getEffectBasePoints(0);
    rss->spellId = s->GetSpellInfo()->getId();
    rss->school = SCHOOL_FIRE;
    rss->infront = false;
    rss->charges = 0;

    unitTarget->m_reflectSpellSchool.push_back(rss);

    return true;
}

bool Cannibalize(uint8_t effectIndex, Spell* s)
{
    if (!s->p_caster)
        return false;

    bool check = false;
    float rad = s->GetRadius(effectIndex);
    rad *= rad;

    for (Object::InRangeSet::iterator itr = s->p_caster->GetInRangeSetBegin(); itr != s->p_caster->GetInRangeSetEnd(); ++itr)
    {
        if ((*itr)->IsCreature())
        {
            if (static_cast<Creature*>((*itr))->getDeathState() == CORPSE)
            {
                CreatureProperties const* cn = static_cast<Creature*>((*itr))->GetCreatureProperties();
                if (cn->Type == UNIT_TYPE_HUMANOID || cn->Type == UNIT_TYPE_UNDEAD)
                {
                    if (s->p_caster->GetDistance2dSq((*itr)) < rad)
                    {
                        check = true;
                        break;
                    }
                }
            }
        }
    }

    if (check)
    {
        s->p_caster->cannibalize = true;
        s->p_caster->cannibalizeCount = 0;
        sEventMgr.AddEvent(s->p_caster, &Player::EventCannibalize, uint32(7), EVENT_CANNIBALIZE, 2000, 5, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        s->p_caster->setUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_CANNIBALIZE);
    }

    return true;
}

bool ArcaniteDragonLing(uint8_t /*effectIndex*/, Spell* s)
{
    s->u_caster->CastSpell(s->u_caster, 19804, true);
    return true;
}

bool MithrilMechanicalDragonLing(uint8_t /*effectIndex*/, Spell* s)
{
    s->u_caster->CastSpell(s->u_caster, 12749, true);
    return true;
}

bool MechanicalDragonLing(uint8_t /*effectIndex*/, Spell* s)
{
    s->u_caster->CastSpell(s->u_caster, 4073, true);
    return true;
}

bool GnomishBattleChicken(uint8_t /*effectIndex*/, Spell* s)
{
    s->u_caster->CastSpell(s->u_caster, 13166, true);
    return true;
}

bool GiftOfLife(uint8_t /*effectIndex*/, Spell* s)
{
    Player* playerTarget = s->GetPlayerTarget();

    if (!playerTarget)
        return false;

    SpellCastTargets tgt;
    tgt.m_unitTarget = playerTarget->GetGUID();
    SpellInfo* inf = sSpellCustomizations.GetSpellInfo(23782);
    Spell* spe = sSpellFactoryMgr.NewSpell(s->u_caster, inf, true, NULL);
    spe->prepare(&tgt);

    return true;
}

bool Give5kGold(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->GetPlayerTarget() != NULL)
    {
        if (worldConfig.player.isGoldCapEnabled && (s->GetPlayerTarget()->GetGold() + 50000000) > worldConfig.player.limitGoldAmount)
        {
            s->GetPlayerTarget()->SetGold(worldConfig.player.limitGoldAmount);
            s->GetPlayerTarget()->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_TOO_MUCH_GOLD);
        }
        else
        {
            s->GetPlayerTarget()->ModGold(50000000);
        }
    }
    else
        return false;

    return true;
}

bool NorthRendInscriptionResearch(uint8_t /*effectIndex*/, Spell* s)
{
    // http://www.wowwiki.com/Minor_Inscription_Research :
    // Minor Inscription Research is taught at 75 skill in Inscription.
    // When you perform this research, you have a very high chance of learning a new minor glyph recipe.
    // The chance to discover a new minor glyph is independent of your level, Inscription skill, and how many minor glyphs you already know.
    // The recipe has a 20-hour cooldown, similar to alchemical transmutes.

    // What is a "very high chance" ?  90% ?
    float chance = 90.0f;
    if (Rand(chance))
    {
        // Type 0 = Major, 1 = Minor
        uint32 glyphType = (s->GetSpellInfo()->getId() == 61177) ? 0 : 1;

        std::vector<uint32> discoverableGlyphs;

        // how many of these are the right type (minor/major) of glyph, and learnable by the player
        for (uint32 idx = 0; idx < sSkillLineAbilityStore.GetNumRows(); ++idx)
        {
            auto skill_line_ability = sSkillLineAbilityStore.LookupEntry(idx);
            if (skill_line_ability == nullptr)
                continue;

            if (skill_line_ability->skilline == SKILL_INSCRIPTION && skill_line_ability->next == 0)
            {
                SpellInfo* se1 = sSpellCustomizations.GetSpellInfo(skill_line_ability->spell);
                if (se1 && se1->getEffect(0) == SPELL_EFFECT_CREATE_ITEM)
                {
                    ItemProperties const* itm = sMySQLStore.getItemProperties(se1->getEffectItemType(0));
                    if (itm && (itm->Spells[0].Id != 0))
                    {
                        SpellInfo* se2 = sSpellCustomizations.GetSpellInfo(itm->Spells[0].Id);
                        if (se2 && se2->getEffect(0) == SPELL_EFFECT_USE_GLYPH)
                        {
#if VERSION_STRING > TBC
                            auto glyph_properties = sGlyphPropertiesStore.LookupEntry(se2->getEffectMiscValue(0));
                            if (glyph_properties)
                            {
                                if (glyph_properties->Type == glyphType)
                                {
                                    if (!s->p_caster->HasSpell(skill_line_ability->spell))
                                    {
                                        discoverableGlyphs.push_back(skill_line_ability->spell);
                                    }
                                }
                            }
#endif
                        }
                    }
                }
            }
        }

        if (discoverableGlyphs.size() > 0)
        {
            uint32 newGlyph = discoverableGlyphs.at(RandomUInt(static_cast<uint32>(discoverableGlyphs.size() - 1)));
            s->p_caster->addSpell(newGlyph);
        }
    }

    return true;
}

bool DeadlyThrowInterrupt(uint8_t /*effectIndex*/, Aura* a, bool apply)
{

    if (!apply)
        return true;

    Unit* m_target = a->GetTarget();

    // Interrupt target's current casted spell (either channeled or generic spell with cast time)
    if (m_target->isCastingNonMeleeSpell(true, false, true))
    {
        uint32_t school = 0;

        if (m_target->getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr && m_target->getCurrentSpell(CURRENT_CHANNELED_SPELL)->getCastTimeLeft() > 0)
        {
            school = m_target->getCurrentSpell(CURRENT_CHANNELED_SPELL)->GetSpellInfo()->getSchool();
            m_target->interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);
        }
        // No need to check cast time for generic spells, checked already in Object::isCastingNonMeleeSpell()
        else if (m_target->getCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr)
        {
            school = m_target->getCurrentSpell(CURRENT_GENERIC_SPELL)->GetSpellInfo()->getSchool();
            m_target->interruptSpellWithSpellType(CURRENT_GENERIC_SPELL);
        }

        m_target->SchoolCastPrevent[school] = 3000 + Util::getMSTime();
    }

    return true;
}

bool WaitingToResurrect(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* u_target = a->GetTarget();

    if (!u_target->IsPlayer())
        return true;

    Player* p_target = static_cast<Player*>(u_target);

    if (apply)        // already applied in opcode handler
        return true;

    uint64 crtguid = p_target->m_areaSpiritHealer_guid;

    Creature* pCreature = p_target->IsInWorld() ? p_target->GetMapMgr()->GetCreature(GET_LOWGUID_PART(crtguid)) : NULL;

    if (pCreature == NULL || p_target->m_bg == NULL)
        return true;

    p_target->m_bg->RemovePlayerFromResurrect(p_target, pCreature);

    return true;
}

bool NegativeCrap(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    if (apply)
        a->SetNegative();

    return true;
}

bool DecayFlash(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    if (apply && pAura->GetTarget()->IsPlayer())
    {
        Player* p_target = static_cast<Player*>(pAura->GetTarget());
        p_target->SetShapeShift(10);  //Tharon'ja Skeleton
        p_target->SetDisplayId(9784);
    }
    return true;
}

bool ReturnFlash(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    if (apply && pAura->GetTarget()->IsPlayer())
    {
        Player* p_target = static_cast<Player*>(pAura->GetTarget());
        p_target->SetDisplayId(p_target->GetNativeDisplayId());
        p_target->m_ShapeShifted = 0;
        p_target->SetShapeShift(0);
    }
    return true;
}

bool EatenRecently(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    if (pAura == nullptr)
        return true;

    auto unit_caster = pAura->GetUnitCaster();
    if (unit_caster == nullptr || unit_caster->IsPlayer())
        return true;

    Creature* NetherDrake = static_cast<Creature*>(unit_caster);

    if (apply)
    {
        NetherDrake->GetAIInterface()->SetAllowedToEnterCombat(false);
        NetherDrake->Emote(EMOTE_ONESHOT_EAT);
    }
    else
    {
        NetherDrake->GetAIInterface()->SetAllowedToEnterCombat(true);
        NetherDrake->GetAIInterface()->setSplineFlying();
        NetherDrake->GetAIInterface()->MoveTo(NetherDrake->GetSpawnX(), NetherDrake->GetSpawnY(), NetherDrake->GetSpawnZ());
    }
    return true;
}

bool Temper(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->u_caster == NULL)
        return true;

    Unit* pHated = pSpell->u_caster->GetAIInterface()->GetMostHated();

    MapScriptInterface* pMap = pSpell->u_caster->GetMapMgr()->GetInterface();
    Creature* pCreature1 = pMap->SpawnCreature(28695, 1335.296265f, -89.237503f, 56.717800f, 1.994538f, true, true, 0, 0, 1);
    if (pCreature1)
        pCreature1->GetAIInterface()->AttackReaction(pHated, 1);

    Creature* pCreature2 = pMap->SpawnCreature(28695, 1340.615234f, -89.083313f, 56.717800f, 0.028982f, true, true, 0, 0, 1);
    if (pCreature2)
        pCreature2->GetAIInterface()->AttackReaction(pHated, 1);

    return true;
};

//Chaos blast dummy effect
bool ChaosBlast(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->u_caster == NULL)
        return true;

    pSpell->u_caster->CastSpell(pSpell->GetUnitTarget(), 37675, true);
    return true;
}

bool Dummy_Solarian_WrathOfTheAstromancer(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* Caster = pSpell->u_caster;
    if (!Caster)
        return true;

    Unit* Target = Caster->GetAIInterface()->getNextTarget();
    if (!Target)
        return true;

    SpellInfo* SpellInfo = sSpellCustomizations.GetSpellInfo(42787);
    if (!SpellInfo)
        return true;

    //Explode bomb after 6sec
    sEventMgr.AddEvent(Target, &Unit::EventCastSpell, Target, SpellInfo, EVENT_UNK, 6000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    return true;
}

bool PreparationForBattle(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->p_caster == NULL)
        return true;

    Player* pPlayer = pSpell->p_caster;

    pPlayer->AddQuestKill(12842, 0, 0);

    return true;
};

#define CN_CRYSTAL_SPIKE    27099
#define CRYSTAL_SPIKES      47958
#define CRYSTAL_SPIKES_H    57082

bool CrystalSpikes(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (pSpell->u_caster == NULL)
        return true;

    Unit* pCaster = pSpell->u_caster;

    for (uint8 i = 1; i < 6; ++i)
    {
        pCaster->GetMapMgr()->GetInterface()->SpawnCreature(CN_CRYSTAL_SPIKE, pCaster->GetPositionX() + (3 * i) + RandomUInt(2), pCaster->GetPositionY() + (3 * i) + RandomUInt(2), pCaster->GetPositionZ(), pCaster->GetOrientation(), true, false, 0, 0);
    }

    for (uint8 i = 1; i < 6; ++i)
    {
        pCaster->GetMapMgr()->GetInterface()->SpawnCreature(CN_CRYSTAL_SPIKE, pCaster->GetPositionX() - (3 * i) - RandomUInt(2), pCaster->GetPositionY() + (3 * i) + RandomUInt(2), pCaster->GetPositionZ(), pCaster->GetOrientation(), true, false, 0, 0);
    }

    for (uint8 i = 1; i < 6; ++i)
    {
        pCaster->GetMapMgr()->GetInterface()->SpawnCreature(CN_CRYSTAL_SPIKE, pCaster->GetPositionX() + (3 * i) + RandomUInt(2), pCaster->GetPositionY() - (3 * i) - RandomUInt(2), pCaster->GetPositionZ(), pCaster->GetOrientation(), true, false, 0, 0);
    }

    for (uint8 i = 1; i < 6; ++i)
    {
        pCaster->GetMapMgr()->GetInterface()->SpawnCreature(CN_CRYSTAL_SPIKE, pCaster->GetPositionX() - (3 * i) - RandomUInt(2), pCaster->GetPositionY() - (3 * i) - RandomUInt(2), pCaster->GetPositionZ(), pCaster->GetOrientation(), true, false, 0, 0);
    }

    return true;
}


////////////////////////////////////////////////////////////////
/// bool Listening To Music scripted spell effect (SpellId 50499)
///
/// \brief
///  Casted by Player. Makes the player cast "Listening to Music"
///
////////////////////////////////////////////////////////////////
bool ListeningToMusicParent(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->p_caster == NULL)
        return true;

    s->p_caster->CastSpell(s->p_caster, 50493, true);

    return true;
}

////////////////////////////////////////////////////////////////
//TeleportToCoordinates scripted spell effect
//Default handler for spells:
//SELECT id, NAME, Effect_1 FROM dbc_spell WHERE Effect_1 = 77 AND
//(NAME LIKE "%Translocate%" OR NAME LIKE "%Portal to%" OR NAME LIKE
//"%Portal Effect%" OR NAME LIKE "%Teleport%") AND EffectBasePoints_1 = 0;
//
//Precondition(s)
//  Casted by Player
//
//Effect(s)
//  Teleports the caster to the location stored in the teleport_coords table of the Database
//
//
////////////////////////////////////////////////////////////////
bool TeleportToCoordinates(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->p_caster == nullptr)
        return true;

    TeleportCoords const* teleport_coord = sMySQLStore.getTeleportCoord(s->GetSpellInfo()->getId());
    if (teleport_coord == nullptr)
    {
        LogError("Spell %u ( %s ) has a TeleportToCoordinates scripted effect, but has no coordinates to teleport to. ", s->GetSpellInfo()->getId(), s->GetSpellInfo()->getName().c_str());
        return true;
    }

    s->HandleTeleport(teleport_coord->x, teleport_coord->y, teleport_coord->z, teleport_coord->mapId, s->p_caster);
    return true;
}


static float IOCTeleInLocations[6][4] =
{
    { 399.66f, -798.63f, 49.06f, 4.01f },     // Alliance front gate in
    { 313.64f, -775.43f, 49.04f, 4.93f },     // Alliance west gate in
    { 323.01f, -888.61f, 48.91f, 4.66f },     // Alliance east gate in
    { 1234.51f, -684.55f, 49.32f, 5.01f },    // Horde west gate in
    { 1161.82f, -748.87f, 48.62f, 0.34f },    // Horde front gate in
    { 1196.06f, -842.70f, 49.13f, 0.30f },    // Horde east gate in
};

static float IOCTeleOutLocations[6][4] =
{
    { 429.79f, -800.825f, 49.03f, 3.23f },    // Alliance front gate out
    { 324.68f, -748.73f, 49.38f, 1.76f },     // Alliance west gate out
    { 316.22f, -914.65f, 48.87f, 1.69f },     // Alliance east gate out
    { 1196.72f, -664.84f, 48.57f, 1.71f },    // Horde west gate out
    { 1140.19f, -780.74f, 48.69f, 2.93f },    // Horde front gate out
    { 1196.47f, -861.29f, 49.17f, 4.04f },    // Horde east gate out
};


bool IOCTeleporterIn(uint8_t /*effectIndex*/, Spell* s)
{
    Player* p = s->GetPlayerTarget();
    if (p == NULL)
        return true;

    // recently used the teleporter
    if (p->HasAura(66550) || p->HasAura(66551))
        return true;

    // Let's not teleport in/out before the battle starts
    if ((p->m_bg != NULL) && !p->m_bg->HasStarted())
        return true;

    uint32 j;
    for (j = 0; j < 6; j++)
    {
        if (p->getDistanceSq(IOCTeleOutLocations[j][0], IOCTeleOutLocations[j][1], IOCTeleOutLocations[j][2]) <= 20.0f)
            break;
    }

    // We are not in range of any portal coords
    if (j == 6)
        return true;

    LocationVector v(IOCTeleInLocations[j][0], IOCTeleInLocations[j][1], IOCTeleInLocations[j][2]);
    p->SafeTeleport(p->GetMapId(), p->GetInstanceID(), v);

    return true;
}

bool IOCTeleporterOut(uint8_t /*effectIndex*/, Spell* s)
{
    Player* p = s->GetPlayerTarget();
    if (p == NULL)
        return true;

    // recently used the teleporter
    if (p->HasAura(66550) || p->HasAura(66551))
        return true;

    // Let's not teleport in/out before the battle starts
    if ((p->m_bg != NULL) && !p->m_bg->HasStarted())
        return true;

    uint32 j;
    for (j = 0; j < 6; j++)
    {
        if (p->getDistanceSq(IOCTeleInLocations[j][0], IOCTeleInLocations[j][1], IOCTeleInLocations[j][2]) <= 20.0f)
            break;
    }

    // We are not in range of any portal coords
    if (j == 6)
        return true;

    LocationVector v(IOCTeleOutLocations[j][0], IOCTeleOutLocations[j][1], IOCTeleOutLocations[j][2]);
    p->SafeTeleport(p->GetMapId(), p->GetInstanceID(), v);

    return true;
}

const float sotaTransDest[5][4] =
{
    { 1388.94f, 103.067f, 34.49f, 5.4571f },
    { 1043.69f, -87.95f, 87.12f, 0.003f },
    { 1441.0411f, -240.974f, 35.264f, 0.949f },
    { 1228.342f, -235.234f, 60.03f, 0.4584f },
    { 1193.857f, 69.9f, 58.046f, 5.7245f },
};

// 54640
bool SOTATeleporter(uint8_t /*effectIndex*/, Spell* s)
{
    Player* plr = s->GetPlayerTarget();
    if (plr == NULL)
        return true;

    LocationVector dest;
    uint32 closest_platform = 0;

    for (uint8 i = 0; i < 5; i++)
    {
        float distance = plr->getDistanceSq(sotaTransDest[i][0], sotaTransDest[i][1], sotaTransDest[i][2]);

        if (distance < 75)
        {
            closest_platform = i;
            break;
        }
    }

    dest.ChangeCoords(sotaTransDest[closest_platform][0], sotaTransDest[closest_platform][1], sotaTransDest[closest_platform][2], sotaTransDest[closest_platform][3]);

    plr->SafeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest);
    return true;
}

// 51892 - Eye of Acherus Visual
bool EyeOfAcherusVisual(uint8_t /*effectIndex*/, Spell* spell)
{
    Player* player = spell->GetPlayerTarget();
    if (player == nullptr)
        return true;

    if (player->HasAura(51892))
        player->removeAllAurasById(51892);
    return true;
}

// 52694 - Recall Eye of Acherus
bool RecallEyeOfAcherus(uint8_t /*effectIndex*/, Spell* spell)
{
    Player* player = spell->GetPlayerTarget();
    if (player == nullptr)
        return true;

    player->removeAllAurasById(51852);
    return true;
}

bool GeneralDummyAura(uint8_t /*effectIndex*/, Aura* /*pAura*/, bool /*apply*/)
{
    // This handler is being used to apply visual effect.
    return true;
}

bool GeneralDummyEffect(uint8_t /*effectIndex*/, Spell* /*pSpell*/)
{
    // This applies the dummy effect (nothing more needed for this spell)
    return true;
}

void SetupMiscSpellhandlers(ScriptMgr* mgr)
{
    mgr->register_dummy_spell(51892, &EyeOfAcherusVisual);
    mgr->register_script_effect(52694, &RecallEyeOfAcherus);

    mgr->register_dummy_spell(54640, &SOTATeleporter);

    mgr->register_dummy_spell(66550, &IOCTeleporterOut);
    mgr->register_dummy_spell(66551, &IOCTeleporterIn);

    uint32 SpellTeleports[] =
    {
        // ICCTeleports
        70781,
        70856,
        70857,
        70858,
        70859,
        70861,
        // Ulduar Teleports
        64014,
        64032,
        64028,
        64031,
        64030,
        64029,
        64024,
        64025,
        65042,
        0
    };
    mgr->register_dummy_spell(SpellTeleports, &TeleportToCoordinates);

    mgr->register_dummy_spell(11189, &FrostWarding);
    mgr->register_dummy_spell(28332, &FrostWarding);

    mgr->register_dummy_spell(11094, &MoltenShields);
    mgr->register_dummy_spell(13043, &MoltenShields);

    mgr->register_dummy_spell(20577, &Cannibalize);

    mgr->register_dummy_spell(23074, &ArcaniteDragonLing);

    mgr->register_dummy_spell(23075, &MithrilMechanicalDragonLing);

    mgr->register_dummy_spell(23076, &MechanicalDragonLing);

    mgr->register_dummy_spell(23133, &GnomishBattleChicken);

    mgr->register_dummy_spell(23725, &GiftOfLife);

    mgr->register_script_effect(46642, &Give5kGold);

    mgr->register_script_effect(61288, &NorthRendInscriptionResearch);

    mgr->register_script_effect(61177, &NorthRendInscriptionResearch);

    mgr->register_dummy_aura(32748, &DeadlyThrowInterrupt);

    mgr->register_dummy_aura(2584, &WaitingToResurrect);

    uint32 negativecrapids[] =
    {
        26013,
        41425,
        0
    };
    mgr->register_dummy_aura(negativecrapids, &NegativeCrap);

    mgr->register_dummy_aura(49356, &DecayFlash);

    mgr->register_dummy_aura(53463, &ReturnFlash);

    mgr->register_dummy_aura(38502, &EatenRecently);

    mgr->register_dummy_spell(52238, &Temper);

    mgr->register_dummy_spell(37674, &ChaosBlast);

    mgr->register_dummy_spell(42783, &Dummy_Solarian_WrathOfTheAstromancer);

    mgr->register_dummy_spell(53341, &PreparationForBattle);
    mgr->register_dummy_spell(53343, &PreparationForBattle);

    mgr->register_script_effect(CRYSTAL_SPIKES, &CrystalSpikes);
    mgr->register_script_effect(CRYSTAL_SPIKES_H, &CrystalSpikes);

    mgr->register_script_effect(50499, &ListeningToMusicParent);

    uint32 teleportToCoordinates[] =
    {
        25140,
        25143,
        25650,
        25652,
        29128,
        29129,
        35376,
        35727,
        54620,
        58622,
        0
    };
    mgr->register_script_effect(teleportToCoordinates, &TeleportToCoordinates);

    uint32 auraWithoutNeededEffect[] =
    {
        71764,      // DiseasedWolf just apply GFX
        33209,      // Gossip NPC Periodic - Despawn (Aura hidden, Cast time hidden, no clue what it should do)
        57764,      // Hover (Anim Override) just apply GFX (not walking or swimming...)
        35357,      // Spawn Effect, Serverside (Aura hidden, Cast time hidden)
        45948,      ///\todo units with this aura are not allowed to fly (never seen it on a player)
        46011,      // See ^
        0
    };
    mgr->register_dummy_aura(auraWithoutNeededEffect, &GeneralDummyAura);

    uint32 spellWithoutNeededEffect[] =
    {
        29403,      // Holiday Breath of Fire, Effect (NPC) Triggered by 29421 Apply Aura 29402 (Aura is hidden)
        52124,      // Sky Darkener Assault. Triggered by 52147 (Apply Aura: Periodically trigger spell) (Aura is hidden)
        53274,      // Icebound Visage (Aura is hidden)
        53275,      // See ^
        0
    };
    mgr->register_dummy_spell(spellWithoutNeededEffect, &GeneralDummyEffect);
}
