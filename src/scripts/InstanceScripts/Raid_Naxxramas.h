/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2017 Sun++ Team <http://www.sunplusplus.info/>
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


#include "Objects/Faction.h"
#include <Units/Creatures/Pet.h>

class GrandWidowFaerlinaAI;
class AnubRekhanAI;
class DeathchargerSteedAI;
class PlaguedWarriorAI;
class PlaguedChampionAI;
class PlaguedGuardianAI;
class PlagueFissureGO;
class SporeAI;
class PortalOfShadowsAI;
class DeathKnightUnderstudyAI;

/////////////////////////////////////////////////////////////////////////////////
////// The Arachnid Quarter

/////////////////////////////////////////////////////////////////////////////////
////// Carrion Spinner
const uint32 CN_CARRION_SPINNER = 15975;
const uint32 CARRION_SPINNER_POISON_BOLT_NORMAL = 30043;
const uint32 CARRION_SPINNER_POISON_BOLT_HEROIC = 56032;
const uint32 CARRION_SPINNER_WEB_WRAP = 28618; //\todo  PULL EFFECT *FUN*

class CarrionSpinnerAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(CarrionSpinnerAI, MoonScriptCreatureAI);
    CarrionSpinnerAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Dread Creeper
const uint32 CN_DREAD_CREEPER = 15974;
const uint32 DREAD_CREEPER_VEIL_OF_SHADOW_NORMAL = 53803;
const uint32 DREAD_CREEPER_VEIL_OF_SHADOW_HEROIC = 28440;

class DreadCreeperAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DreadCreeperAI, MoonScriptCreatureAI);
    DreadCreeperAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Naxxramas Cultist
const uint32 CN_NAXXRAMAS_CULTIST = 15980;
const uint32 NAXXRAMAS_CULTIST_KNOCKBACK_NORMAL = 53850;
const uint32 NAXXRAMAS_CULTIST_KNOCKBACK_HEROIC = 53851;

class NaxxramasCultistAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(NaxxramasCultistAI, MoonScriptCreatureAI);
    NaxxramasCultistAI(Creature* pCreature);
};
//Necro Stalker AI - was it removed ?
/////////////////////////////////////////////////////////////////////////////////
////// Venom Stalker
const uint32 CN_VENOM_STALKER = 15976;
const uint32 VENOM_STALKER_POISON_CHARGE_NORMAL = 28431;
const uint32 VENOM_STALKER_POISON_CHARGE_HEROIC = 53809;

class VenomStalkerAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(VenomStalkerAI, MoonScriptCreatureAI);
    VenomStalkerAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Tomb Horror
const uint32 CN_TOMB_HORROR = 15979;
const uint32 TOMB_HORROR_SPIKE_VOLLEY = 28615;
const uint32 TOMB_HORROR_CRYPT_SCARAB_SWARM_NORMAL = 54313;
const uint32 TOMB_HORROR_CRYPT_SCARAB_SWARM_HEROIC = 54317;
const uint32 TOMB_HORROR_CRYPT_SCARABS_NORMAL = 54311;
const uint32 TOMB_HORROR_CRYPT_SCARABS_HEROIC = 54316;

class TombHorrorAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(TombHorrorAI, MoonScriptCreatureAI);
    TombHorrorAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Naxxramas Acolyte
const uint32 CN_NAXXRAMAS_ACOLYTE = 16368;
const uint32 NAXXRAMAS_ACOLYTE_SHADOW_BOLT_VOLLEY_NORMAL = 56064;
const uint32 NAXXRAMAS_ACOLYTE_SHADOW_BOLT_VOLLEY_HEROIC = 56065;
const uint32 NAXXRAMAS_ACOLYTE_ARCANE_EXPLOSION_NORMAL = 56063;
const uint32 NAXXRAMAS_ACOLYTE_ARCANE_EXPLOSION_HEROIC = 56067;
// To check: "total caster" + Explosion only when players are close ?
class NaxxramasAcolyteAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(NaxxramasAcolyteAI, MoonScriptCreatureAI);
    NaxxramasAcolyteAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Vigilant Shade
const uint32 CN_VIGILANT_SHADE = 30085;
const uint32 VIGILANT_SHADE_INVISIBILITY = 55848;
const uint32 VIGILANT_SHADE_SHADOW_BOLT_VOLLEY_NORMAL = 55850;
const uint32 VIGILANT_SHADE_SHADOW_BOLT_VOLLEY_HEROIC = 55851;
// Invisiblity should be removed OnCombatStart ?
class VigilantShadeAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(VigilantShadeAI, MoonScriptCreatureAI);
    VigilantShadeAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void OnCombatStop(Unit* pTarget);
};

/////////////////////////////////////////////////////////////////////////////////
////// Crypt Reaver
const uint32 CN_CRYPT_REAVER = 15978;
const uint32 CRYPT_REAVER_CLEAVE = 40504;
const uint32 CRYPT_REAVER_FRENZY = 56625;

class CryptReaverAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(CryptReaverAI, MoonScriptCreatureAI);
    CryptReaverAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Web Wrap
const uint32 CN_WEB_WRAP = 16486;

class WebWrapAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(WebWrapAI, MoonScriptCreatureAI);
    WebWrapAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void OnCombatStop(Unit* pTarget);
    void OnDied(Unit* pKiller);
    void AIUpdate();
    void Destroy();

    uint64    mPlayerGuid;
};

/////////////////////////////////////////////////////////////////////////////////
////// Maexxna Spiderling
const uint32 CN_MAEXXNA_SPIDERLING = 17055;
const uint32 MAEXXNA_SPIDERLING_NECROTIC_POISON_NORMAL = 54121;
const uint32 MAEXXNA_SPIDERLING_NECROTIC_POISON_HEROIC = 28776;

class MaexxnaSpiderlingAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(MaexxnaSpiderlingAI, MoonScriptCreatureAI);
    MaexxnaSpiderlingAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Maexxna
const uint32 CN_MAEXXNA = 15952;
const uint32 MAEXXNA_WEB_WRAP = 28622;
const uint32 MAEXXNA_WEB_SPRAY_NORMAL = 29484;
const uint32 MAEXXNA_WEB_SPRAY_HEROIC = 54125;
const uint32 MAEXXNA_POISON_SHOCK_NORMAL = 28741;
const uint32 MAEXXNA_POISON_SHOCK_HEROIC = 54122;
const uint32 MAEXXNA_NECROTIC_POISON_NORMAL = 54121;
const uint32 MAEXXNA_NECROTIC_POISON_HEROIC = 28776;
const uint32 MAEXXNA_FRENZY_NORMAL = 54123;
const uint32 MAEXXNA_FRENZY_HEROIC = 54124;

static Movement::Location WebWrapPos[] =
{
    // Left wall
    { 3515.307861f, -3837.076172f, 302.671753f, 4.388477f },
    { 3529.401123f, -3841.910889f, 300.766174f, 4.335070f },
    { 3541.990479f, -3851.308350f, 298.685272f, 3.804141f },
    // Right wall
    { 3497.262939f, -3949.542969f, 308.138916f, 1.433983f },
    { 3510.450928f, -3949.871582f, 309.523193f, 1.613839f },
    { 3523.486572f, -3946.144287f, 309.651611f, 1.973552f }
};

void SpellFunc_MaexxnaWebWrap(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);

class MaexxnaAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(MaexxnaAI, MoonScriptCreatureAI);
    MaexxnaAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void OnCombatStop(Unit* pTarget);
    void AIUpdate();

    SpellDesc* mWebWrapProc;
    bool mHasEnraged;
    bool mLeftWall;
    int32 mAddsSummonTimer;
    int32 mWebSprayTimer;
    uint32 mWebWrapTimer;
};

/////////////////////////////////////////////////////////////////////////////////
////// Naxxramas Worshipper
const uint32 CN_NAXXRAMAS_WORSHIPPER = 16506;
const uint32 NAXXRAMAS_WORSHIPPER_FIREBALL_NORMAL = 54095;
const uint32 NAXXRAMAS_WORSHIPPER_FIREBALL_HEROIC = 54096;
const uint32 NAXXRAMAS_WORSHIPPER_WIDOW_EMBRACE = 28732;
const uint32 NAXXRAMAS_WORSHIPPER_MIND_EXHAUSTION = 28727;

class NaxxramasWorshipperAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(NaxxramasWorshipperAI, MoonScriptCreatureAI);
    NaxxramasWorshipperAI(Creature* pCreature);
    friend class GrandWidowFaerlinaAI;

    void OnCastSpell(uint32 pSpellId);
    void OnDied(Unit* pKiller);
    void AIUpdate();
    void Destroy();

    GrandWidowFaerlinaAI* mGrandWidow;
    bool mPossessed;
};

/////////////////////////////////////////////////////////////////////////////////
////// Naxxramas Follower
const uint32 CN_NAXXRAMAS_FOLLOWER = 16505;
const uint32 NAXXRAMAS_FOLLOWER_BERSERKER_CHARGE_HEROIC = 56107;
const uint32 NAXXRAMAS_FOLLOWER_SILENCE_HEROIC = 54093;

void SpellFunc_NaxxramasFollowerCharge(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);

class NaxxramasFollowerAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(NaxxramasFollowerAI, MoonScriptCreatureAI);
    NaxxramasFollowerAI(Creature* pCreature);
    friend class GrandWidowFaerlinaAI;

    void Destroy();

    GrandWidowFaerlinaAI* mGrandWidow;
    SpellDesc* mCharge;
};

/////////////////////////////////////////////////////////////////////////////////
////// Grand Widow Faerlina
const uint32 CN_GRAND_WIDOW_FAERLINA = 15953;
const uint32 GRAND_WIDOW_FAERLINA_POISON_VOLLEY_BOLT_NORMAL = 28796;
const uint32 GRAND_WIDOW_FAERLINA_POISON_VOLLEY_BOLT_HEROIC = 54098;
const uint32 GRAND_WIDOW_FAERLINA_FRENZY_NORMAL = 28798;
const uint32 GRAND_WIDOW_FAERLINA_FRENZY_HEROIC = 54100;
const uint32 GRAND_WIDOW_RAIN_OF_FIRE_NORMAL = 39024;
const uint32 GRAND_WIDOW_RAIN_OF_FIRE_HEROIC = 58936;

static Movement::Location Worshippers[4] =
{
    { -3.0f, 0, 0, 0 },
    { -9.0f, 0, 0, 0 },
    { 3.0f, 0, 0, 0 },
    { 9.0f, 0, 0, 0 }
};

static Movement::Location Followers[2] =
{
    { -6.0f, 0, 0, 0 },
    { 6.0f, 0, 0, 0 }
};

class GrandWidowFaerlinaAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(GrandWidowFaerlinaAI, MoonScriptCreatureAI);
    GrandWidowFaerlinaAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void OnCombatStop(Unit* pTarget);
    void AIUpdate();
    void Destroy();

    std::set< NaxxramasWorshipperAI* > mWorshippers;
    std::set< NaxxramasFollowerAI* > mFollowers;
    SpellDesc* mFrenzy;
    SpellDesc* mPoisonVolleyBolt;
    int32 mFrenzyTimer;
    int32 mPoisonVolleyBoltTimer;
};

/////////////////////////////////////////////////////////////////////////////////
////// Crypt Guard
const uint32 CN_CRYPT_GUARD = 16573;
const uint32 CRYPT_GUARD_ACID_SPLIT_NORMAL = 28969;
const uint32 CRYPT_GUARD_ACID_SPLIT_HEROIC = 56098;
const uint32 CRYPT_GUARD_CLEAVE = 40504;
const uint32 CRYPT_GUARD_FRENZY = 8269;

class CryptGuardAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(CryptGuardAI, MoonScriptCreatureAI);
    CryptGuardAI(Creature* pCreature);
    friend class AnubRekhanAI;

    void OnCombatStart(Unit* pTarget);
    void AIUpdate();
    void Destroy();

    AnubRekhanAI* mAnubRekhanAI;
    bool mEnraged;
};

/////////////////////////////////////////////////////////////////////////////////
////// Corpse Scarab
const uint32 CN_CORPSE_SCARAB = 16698;

class CorpseScarabAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(CorpseScarabAI, MoonScriptCreatureAI);
    CorpseScarabAI(Creature* pCreature);
    friend class AnubRekhanAI;

    void Destroy();

    AnubRekhanAI* mAnubRekhanAI;
};

/////////////////////////////////////////////////////////////////////////////////
////// Anub'Rekhan
const uint32 CN_ANUBREKHAN = 15956;
const uint32 ANUBREKHAN_IMPALE_NORMAL = 28783;
const uint32 ANUBREKHAN_IMPALE_HEROIC = 56090;
const uint32 ANUBREKHAN_LOCUST_SWARM_NORMAL = 28785;
const uint32 ANUBREKHAN_LOCUST_SWARM_HEROIC = 54021;
const uint32 ANUBREKHAN_SUMMON_CORPSE_SCARABS_5 = 29105;
const uint32 ANUBREKHAN_SUMMON_CORPSE_SCARABS_10 = 28864;
const uint32 ANUBREKHAN_BERSERK = 26662;

static Movement::Location CryptGuards[] =
{
    { 3300.486572f, -3449.479492f, 287.077850f, 3.883793f },
    { 3300.568604f, -3503.060059f, 287.077850f, 2.367975f },
    { 3332.591797f, -3476.102539f, 287.073425f, 0.015707f }
};

void SpellFunc_AnubRekhanCorpseScarabsPlayer(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);
void SpellFunc_AnubRekhanCorpseScarabsCryptGuard(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);

class AnubRekhanAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(AnubRekhanAI, MoonScriptCreatureAI);
    AnubRekhanAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void OnCombatStop(Unit* pTarget);
    void AIUpdate();
    void Destroy();

    std::set< CorpseScarabAI* > mScarabs;
    std::set< CryptGuardAI* > mCryptGuards;
    std::set< uint32 > mUsedCorpseGuids;
    SpellDesc* mLocustSwarm;
    int32 mLocustSwarmTimer;
    uint32 mCryptSpawnTimer;
};

/////////////////////////////////////////////////////////////////////////////////
////// The Plague Quarter

/////////////////////////////////////////////////////////////////////////////////
////// Infectious Ghoul
const uint32 CN_INFECTIOUS_GHOUL = 16244;
const uint32 INFECTIOUS_GHOUL_FLESH_ROT = 54709;
const uint32 INFECTIOUS_GHOUL_REND_NORMAL = 54703;
const uint32 INFECTIOUS_GHOUL_REND_HEROIC = 54708;
const uint32 INFECTIOUS_GHOUL_FRENZY_NORMAL = 54701;
const uint32 INFECTIOUS_GHOUL_FRENZY_HEROIC = 24318;

class InfectiousGhoulAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(InfectiousGhoulAI, MoonScriptCreatureAI);
    InfectiousGhoulAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void AIUpdate();

    bool    mEnraged;
};

/////////////////////////////////////////////////////////////////////////////////
////// Stoneskin Gargoyle
const uint32 CN_STONESKIN_GARGOYLE = 16168;
const uint32 STONESKIN_GARGOYLE_ACID_VOLLEY_NORMAL = 29325;
const uint32 STONESKIN_GARGOYLE_ACID_VOLLEY_HEROIC = 54714;
const uint32 STONESKIN_GARGOYLE_STONESKIN_NORMAL = 28995;
const uint32 STONESKIN_GARGOYLE_STONESKIN_HEROIC = 54722;

class StoneskinGargoyleAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(StoneskinGargoyleAI, MoonScriptCreatureAI);
    StoneskinGargoyleAI(Creature* pCreature);

    bool HasStoneskin();
    void AIUpdate();

    SpellDesc* mStoneskin;
};

/////////////////////////////////////////////////////////////////////////////////
////// Frenzied Bat
const uint32 CN_FRENZIED_BAT = 16036;
const uint32 FRENZIED_BAT_FRENZIED_DIVE = 54781;

class FrenziedBatAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(FrenziedBatAI, MoonScriptCreatureAI);
    FrenziedBatAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Plague Beast
const uint32 CN_PLAGUE_BEAST = 16034;
const uint32 PLAGUE_BEAST_PLAGUE_SPLASH_NORMAL = 54780;
const uint32 PLAGUE_BEAST_PLAGUE_SPLASH_HEROIC = 56538;
const uint32 PLAGUE_BEAST_MUTATED_SPORES = 30110;
const uint32 PLAGUE_BEAST_TRAMPLE = 5568;

class PlagueBeastAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(PlagueBeastAI, MoonScriptCreatureAI);
    PlagueBeastAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void OnCombatStop(Unit* pTarget);
};

/////////////////////////////////////////////////////////////////////////////////
////// Eye Stalker
const uint32 CN_EYE_STALKER = 16236;
const uint32 EYE_STALKER_MIND_FLAY_NORMAL = 29407;
const uint32 EYE_STALKER_MIND_FLAY_HEROIC = 54805;

class EyeStalkerAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(EyeStalkerAI, MoonScriptCreatureAI);
    EyeStalkerAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void AIUpdate();
};

/////////////////////////////////////////////////////////////////////////////////
////// Noth the Plaguebringer
const uint32 CN_NOTH_THE_PLAGUEBRINGER = 15954;
const uint32 NOTH_THE_PLAGUEBRINGER_BLINK_HEROIC = 29208;
const uint32 NOTH_THE_PLAGUEBRINGER_CRIPLE_HEROIC = 29212;
const uint32 NOTH_THE_PLAGUEBRINGER_CURSE_OF_THE_PLAGUE_NORMAL = 29213;    // I must check if it's target-limited spell
const uint32 NOTH_THE_PLAGUEBRINGER_CURSE_OF_THE_PLAGUE_HEROIC = 54835;    // I must check if it's target-limited spell
const uint32 NOTH_THE_PLAGUEBRINGER_BERSERK = 47008;    // Guessed

static Movement::Location SkelPosPhase1[] =
{
    { 2660.175781f, -3473.315674f, 262.003967f, 5.252077f },
    { 2717.336426f, -3463.309326f, 262.044098f, 3.703270f },
    { 2718.720215f, -3524.452881f, 261.943176f, 2.760789f }
};

static Movement::Location SkelPosPhase2[] =
{
    { 2660.932129f, -3474.058105f, 262.004730f, 5.765719f },
    { 2706.175537f, -3465.862793f, 262.003510f, 4.488667f },
    { 2723.024170f, -3472.919434f, 262.102020f, 3.809298f },
    { 2717.878906f, -3518.062988f, 261.905945f, 3.177050f }
};

void SpellFunc_NothToBalconyPhaseSwitch(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);
void SpellFunc_NothFromBalconyPhaseSwitch(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);
void SpellFunc_NothCriple(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);
void SpellFunc_NothBlink(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);

class NothThePlaguebringerAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(NothThePlaguebringerAI, MoonScriptCreatureAI);
    NothThePlaguebringerAI(Creature* pCreature);
    friend class PlaguedWarriorAI;
    friend class PlaguedChampionAI;
    friend class PlaguedGuardianAI;

    void OnCombatStart(Unit* pTarget);
    void OnCombatStop(Unit* pTarget);
    void AIUpdate();
    void OnScriptPhaseChange(uint32_t phaseId);
    void Destroy();

    std::set<PlaguedWarriorAI*> mWarriors;
    std::set<PlaguedChampionAI*> mChampions;
    std::set<PlaguedGuardianAI*> mGuardians;
    SpellDesc* mCriple;
    SpellDesc* mBlink;
    SpellDesc* mToBalconySwitch;
    SpellDesc* mFromBalconySwitch;
    int32 mBlinkTimer;
    int32 mSkeletonTimer;
    int32 mPhaseSwitchTimer;
    uint32 mPhaseCounter;
};

/////////////////////////////////////////////////////////////////////////////////
////// Plagued Warrior
const uint32 CN_PLAGUED_WARRIOR = 16984;
const uint32 PLAGUED_WARRIOR_STRIKE = 12057;
const uint32 PLAGUED_WARRIOR_CLEAVE = 15496;

class PlaguedWarriorAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(PlaguedWarriorAI, MoonScriptCreatureAI);
    PlaguedWarriorAI(Creature* pCreature);

    void Destroy();

    NothThePlaguebringerAI* mNothAI;
};

/////////////////////////////////////////////////////////////////////////////////
////// Plagued Champion
const uint32 CN_PLAGUED_CHAMPION = 16983;
const uint32 PLAGUED_CHAMPION_MORTAL_STRIKE_NORMAL = 32736;
const uint32 PLAGUED_CHAMPION_MORTAL_STRIKE_HEROIC = 13737;
const uint32 PLAGUED_CHAMPION_SHADOW_SHOCK_NORMAL = 30138;
const uint32 PLAGUED_CHAMPION_SHADOW_SHOCK_HEROIC = 54889;

class PlaguedChampionAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(PlaguedChampionAI, MoonScriptCreatureAI);
    PlaguedChampionAI(Creature* pCreature);

    void Destroy();

    NothThePlaguebringerAI* mNothAI;
};

/////////////////////////////////////////////////////////////////////////////////
////// Plagued Guardian
const uint32 CN_PLAGUED_GUARDIAN = 16981;
const uint32 PLAGUED_GUARDIAN_ARCANE_EXPLOSION_NORMAL = 54890;
const uint32 PLAGUED_GUARDIAN_ARCANE_EXPLOSION_HEROIC = 54891;
const uint32 PLAGUED_GUARDIAN_BLINK = 29208;

class PlaguedGuardianAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(PlaguedGuardianAI, MoonScriptCreatureAI);
    PlaguedGuardianAI(Creature* pCreature);

    void Destroy();

    NothThePlaguebringerAI* mNothAI;
};

/////////////////////////////////////////////////////////////////////////////////
////// Heigan the Unclean
const uint32 CN_HEIGAN_THE_UNCLEAN = 15936;
const uint32 HEIGAN_THE_UNCLEAN_SPELL_DISRUPTION = 29310;
const uint32 HEIGAN_THE_UNCLEAN_DECREPIT_FEVER_NORMAL = 29998;
const uint32 HEIGAN_THE_UNCLEAN_DECREPIT_FEVER_HEROIC = 55011;
const uint32 HEIGAN_THE_UNCLEAN_PLAGUE_CLOUD_CHANNEL = 29350;
const uint32 HEIGAN_THE_UNCLEAN_PLAGUE_CLOUD_DAMAGE = 30122;
const uint32 HEIGAN_THE_UNCLEAN_TELEPORT = 34673;   // Guessed.
const uint32 FISSURE_TRIGGER_ERUPTION = 29371;

float HeiganPos[2] = { 2796, -3707 };
const float HeiganEruptionSlope[3] =
{
    (-3685 - HeiganPos[1]) / (2724 - HeiganPos[0]),
    (-3647 - HeiganPos[1]) / (2749 - HeiganPos[0]),
    (-3637 - HeiganPos[1]) / (2771 - HeiganPos[0]),
};

class HeiganTheUncleanAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(HeiganTheUncleanAI, MoonScriptCreatureAI);
    HeiganTheUncleanAI(Creature* pCreature);
    friend class PlagueFissureGO;

    uint32    CalculateTriggerArea(float pPosX, float pPosY);
    void    CallEruptionEvent(int32 pTimerId, int32 pNewTime);
    void    OnCombatStart(Unit* pTarget);
    void    OnCombatStop(Unit* pTarget);
    void    AIUpdate();
    void    Destroy();

    std::set<std::pair< uint32, PlagueFissureGO* > >        mFissures;
    int32                                        mPhaseSwitchTimer;
    int32                                        mEruptionTimer;
    int32                                        mEruptionPhase;
    bool                                        mClockWiseEruption;
};

/////////////////////////////////////////////////////////////////////////////////
////// Plague Fissure
const uint32 GO_PLAGUE_FISSURE = 181533;

class PlagueFissureGO : public GameObjectAIScript
{
    public:

    static GameObjectAIScript* Create(GameObject* pGameObject);
    PlagueFissureGO(GameObject* pGameObject);

    void DoErrupt();

    // I believe it's nowhere hooked in the core.
    void SetState(uint32 pState);
    void Destroy();
    void ResetHeiganAI() { mHeiganAI = NULL; }
    HeiganTheUncleanAI* mHeiganAI;
};

/////////////////////////////////////////////////////////////////////////////////
////// Loatheb
const uint32 CN_LOATHEB = 16011;
const uint32 LOATHEB_NECROTIC_AURA = 55593;
const uint32 LOATHEB_SUMMON_SPORE = 29234;
const uint32 LOATHEB_DEATHBLOOM_NORMAL = 29865;
const uint32 LOATHEB_DEATHBLOOM_HEROIC = 55053;
const uint32 LOATHEB_DEATHBLOOM_DAMAGE_NORMAL = 55594;
const uint32 LOATHEB_DEATHBLOOM_DAMAGE_HEROIC = 55601;
const uint32 LOATHEB_INEVITABLE_DOOM_NORMAL = 29204;
const uint32 LOATHEB_INEVITABLE_DOOM_HEROIC = 55052;
const uint32 LOATHEB_BERSERK = 26662;    // Unused

static Movement::Location Spores[] =
{
    { 2880.517334f, -4027.450684f, 273.680695f, 0.848522f },
    { 2938.914307f, -4027.245850f, 273.617340f, 2.419318f },
    { 2938.526611f, -3968.206543f, 273.524963f, 3.829108f },
    { 2879.754883f, -3968.288574f, 273.633698f, 5.525566f }
};

class LoathebAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(LoathebAI, MoonScriptCreatureAI);
    LoathebAI(Creature* pCreature);
    friend class SporeAI;

    void OnCombatStart(Unit* pTarget);
    void OnCombatStop(Unit* pTarget);
    void AIUpdate();
    void Destroy();

    std::set< SporeAI* >    mSpores;
    uint32            mDoomStaticTimer;
    int32            mSporeTimer;
    int32            mDoomTimer;
    int32            mDeathbloomTimer;
    bool            mDeathbloomDamagePhase;
};

/////////////////////////////////////////////////////////////////////////////////
////// Spore
const uint32 CN_SPORE = 16286;
const uint32 SPORE_FUNGAL_CREEP = 29232;

class SporeAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SporeAI, MoonScriptCreatureAI);
    SporeAI(Creature* pCreature);

    void OnDied(Unit* pKiller);
    void Destroy();

    LoathebAI* mLoathebAI;
};

/////////////////////////////////////////////////////////////////////////////////
////// The Military Quarter

/////////////////////////////////////////////////////////////////////////////////
////// Death Knight
const uint32 CN_DEATH_KNIGHT = 16146;
const uint32 DEATH_KNIGHT_BLOOD_PRESENCE = 55212;
const uint32 DEATH_KNIGHT_DEATH_COIL_NORMAL = 55209;
const uint32 DEATH_KNIGHT_DEATH_COIL_HEROIC = 55320;
const uint32 DEATH_KNIGHT_DEATH_COIL_HEAL = 55210;
const uint32 DEATH_KNIGHT_HYSTERIA = 55213;

class DeathKnightAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DeathKnightAI, MoonScriptCreatureAI);
    DeathKnightAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
};

/////////////////////////////////////////////////////////////////////////////////
////// Death Knight Captain
const uint32 CN_DEATH_KNIGHT_CAPTAIN = 16145;
const uint32 DEATH_KNIGHT_CAPTAIN_UNHOLY_PRESENCE = 55222;
const uint32 DEATH_KNIGHT_CAPTAIN_RAISE_DEAD = 28353;
const uint32 DEATH_KNIGHT_CAPTAIN_PLAGUE_STRIKE_NORMAL = 55255;
const uint32 DEATH_KNIGHT_CAPTAIN_PLAGUE_STRIKE_HEROIC = 55321;
const uint32 DEATH_KNIGHT_CAPTAIN_WHIRLWIND = 28335;

class DeathKnightCaptainAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DeathKnightCaptainAI, MoonScriptCreatureAI);
    DeathKnightCaptainAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
};

/////////////////////////////////////////////////////////////////////////////////
////// Skeletal Construct - wiki says he's in Naxx, but WoWHead claims him to be in Icecrown only

/////////////////////////////////////////////////////////////////////////////////
////// Ghost of Naxxramas
const uint32 CN_GHOST_OF_NAXXRAMAS = 16419;

class GhostOfNaxxramasAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(GhostOfNaxxramasAI, MoonScriptCreatureAI);
    GhostOfNaxxramasAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Shade of Naxxramas
const uint32 CN_SHADE_OF_NAXXRAMAS = 16164;
const uint32 SHADE_OF_NAXXRAMAS_PORTAL_OF_SHADOWS = 28383;
const uint32 SHADE_OF_NAXXRAMAS_SHADOW_BOLT_VOLLEY_NORMAL = 28407;
const uint32 SHADE_OF_NAXXRAMAS_SHADOW_BOLT_VOLLEY_HEROIC = 55323;

class ShadeOfNaxxramasAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(ShadeOfNaxxramasAI, MoonScriptCreatureAI);
    ShadeOfNaxxramasAI(Creature* pCreature);
    friend class PortalOfShadowsAI;

    void OnDied(Unit* pKiller);
    void Destroy();

    std::set< PortalOfShadowsAI* >    mPortals;
};

/////////////////////////////////////////////////////////////////////////////////
////// Portal of Shadows
const uint32 CN_PORTAL_OF_SHADOWS = 16420;

class PortalOfShadowsAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(PortalOfShadowsAI, MoonScriptCreatureAI);
    PortalOfShadowsAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void OnCombatStop(Unit* pTarget);
    void AIUpdate();
    void Destroy();

    ShadeOfNaxxramasAI* mShadeAI;
    int32 mSpawnTimer;
};

/////////////////////////////////////////////////////////////////////////////////
////// Necro Knight
const uint32 CN_NECRO_KNIGHT = 16165;
const uint32 NECRO_KNIGHT_ARCANE_EXPLOSION = 15453;
const uint32 NECRO_KNIGHT_BLAST_WAVE = 30092;
const uint32 NECRO_KNIGHT_BLINK = 28391;
const uint32 NECRO_KNIGHT_CONE_OF_COLD = 30095;
const uint32 NECRO_KNIGHT_FLAMESTRIKE = 30091;
const uint32 NECRO_KNIGHT_FROST_NOVA = 30094;

void SpellFunc_NecroKnightBlink(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);

class NecroKnightAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(NecroKnightAI, MoonScriptCreatureAI);
    NecroKnightAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Skeletal Smith
const uint32 CN_SKELETAL_SMITH = 16193;
const uint32 SKELETAL_SMITH_CRUSH_ARMOR = 33661;
const uint32 SKELETAL_SMITH_DISARM = 6713;
const uint32 SKELETAL_SMITH_THUNDERCLAP = 23931;
//const uint32 SKELETAL_SMITH_SUDDER_ARMOR = 24317;

class SkeletalSmithAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SkeletalSmithAI, MoonScriptCreatureAI);
    SkeletalSmithAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Death Knight Cavalier
const uint32 CN_DEATH_KNIGHT_CAVALIER = 16163;
const uint32 DEATH_KNIGHT_CAVALIER_BONE_ARMOR_NORMAL = 55315;
const uint32 DEATH_KNIGHT_CAVALIER_BONE_ARMOR_HEROIC = 55336;
const uint32 DEATH_KNIGHT_CAVALIER_DISMOUNT_DEATHCHARGER = 55294;
const uint32 DEATH_KNIGHT_CAVALIER_ICY_TOUCH_NORMAL = 55313;
const uint32 DEATH_KNIGHT_CAVALIER_ICY_TOUCH_HEROIC = 55331;
const uint32 DEATH_KNIGHT_CAVALIER_STRANGULATE_NORMAL = 55314;
const uint32 DEATH_KNIGHT_CAVALIER_STRANGULATE_HEROIC = 55334;
const uint32 DEATH_KNIGHT_CAVALIER_AURA_OF_AGONY = 28413;
const uint32 DEATH_KNIGHT_CAVALIER_CLEAVE = 15284;
const uint32 DEATH_KNIGHT_CAVALIER_DEATH_COIL = 28412;

class DeathKnightCavalierAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DeathKnightCavalierAI, MoonScriptCreatureAI);
    DeathKnightCavalierAI(Creature* pCreature);
    friend class DeathchargerSteedAI;

    void OnCombatStop(Unit* pTarget);
    void AIUpdate();
    void Destroy();

    DeathchargerSteedAI* mChargerAI;
    bool mIsMounted;
};

/////////////////////////////////////////////////////////////////////////////////
////// Deathcharger Steed
const uint32 CN_DEATHCHARGER_STEED = 29818;
const uint32 DEATHCHARGER_STEED_CHARGE = 55317;

void SpellFunc_DeathchargerSteedCharge(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);

class DeathchargerSteedAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DeathchargerSteedAI, MoonScriptCreatureAI);
    DeathchargerSteedAI(Creature* pCreature);
    friend class DeathKnightCavalierAI;

    void OnCombatStop(Unit* pTarget);
    void Destroy();

    DeathKnightCavalierAI* mDeathKnightAI;
    SpellDesc* mCharge;
};

/////////////////////////////////////////////////////////////////////////////////
////// Dark Touched Warrior
const uint32 CN_DARK_TOUCHED_WARRIOR = 16156;
const uint32 DARK_TOUCHED_WARRIOR_WHIRLWIND = 55267;
//const uint32 DARK_TOUCHED_WARRIOR_WHIRLWIND = 55266;    // This one disables mob's melee

class DarkTouchedWarriorAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DarkTouchedWarriorAI, MoonScriptCreatureAI);
    DarkTouchedWarriorAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void AIUpdate();

    int32    mResetHateTimer;
};

/////////////////////////////////////////////////////////////////////////////////
////// Risen Squire
const uint32 CN_RISEN_SQUIRE = 16154;
const uint32 RISEN_SQUIRE_PIERCE_ARMOR = 55318;

class RisenSquireAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(RisenSquireAI, MoonScriptCreatureAI);
    RisenSquireAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Unholy Axe
const uint32 CN_UNHOLY_AXE = 16194;
const uint32 UNHOLY_AXE_MORTAL_STRIKE_NORMAL = 16856;
const uint32 UNHOLY_AXE_MORTAL_STRIKE_HEROIC = 15708;
const uint32 UNHOLY_AXE_WHIRLWIND_NORMAL = 55463;
const uint32 UNHOLY_AXE_WHIRLWIND_HEROIC = 24236;

class UnholyAxeAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(UnholyAxeAI, MoonScriptCreatureAI);
    UnholyAxeAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Unholy Sword
const uint32 CN_UNHOLY_SWORD = 16216;
const uint32 UNHOLY_SWORD_CLEAVE_NORMAL = 15284;
const uint32 UNHOLY_SWORD_CLEAVE_HEROIC = 19632;

class UnholySwordAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(UnholySwordAI, MoonScriptCreatureAI);
    UnholySwordAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Unholy Staff
const uint32 CN_UNHOLY_STAFF = 16215;
const uint32 UNHOLY_STAFF_ARCANE_EXPLOSION_NORMAL = 28450;
const uint32 UNHOLY_STAFF_ARCANE_EXPLOSION_HEROIC = 55467;
const uint32 UNHOLY_STAFF_FROST_NOVA = 29849;
const uint32 UNHOLY_STAFF_POLYMORPH = 29848;

class UnholyStaffAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(UnholyStaffAI, MoonScriptCreatureAI);
    UnholyStaffAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Instructor Razuvious
const uint32 CN_INSTRUCTOR_RAZUVIOUS = 16061;
const uint32 INSTRUCTOR_RAZUVIOUS_DISRUPTING_SHOUT_NORMAL = 55543;
const uint32 INSTRUCTOR_RAZUVIOUS_DISRUPTING_SHOUT_HEROIC = 29107;
const uint32 INSTRUCTOR_RAZUVIOUS_JAGGED_KNIFE = 55550;
const uint32 INSTRUCTOR_RAZUVIOUS_UNBALANCING_STRIKE = 55470;
const uint32 INSTRUCTOR_RAZUVIOUS_HOPELESS = 29125;

class InstructorRazuviousAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(InstructorRazuviousAI, MoonScriptCreatureAI);
    InstructorRazuviousAI(Creature* pCreature);
    friend class DeathKnightUnderstudyAI;

    void OnCombatStart(Unit* pTarget);
    void OnCombatStop(Unit* pTarget);
    void OnDied(Unit* pKiller);
    void AIUpdate();

    std::set< DeathKnightUnderstudyAI* >    mStudents;
};

InstructorRazuviousAI::InstructorRazuviousAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
{
};

void InstructorRazuviousAI::OnCombatStart(Unit* pTarget)
{
    ParentClass::OnCombatStart(pTarget);
};

void InstructorRazuviousAI::OnCombatStop(Unit* pTarget)
{
    ParentClass::OnCombatStop(pTarget);
};

void InstructorRazuviousAI::OnDied(Unit* pKiller)
{
    ParentClass::OnDied(pKiller);
};

void InstructorRazuviousAI::AIUpdate()
{
    ParentClass::AIUpdate();
};

/////////////////////////////////////////////////////////////////////////////////
////// Death Knight Understudy
const uint32 CN_DEATH_KNIGHT_UNDERSTUDY = 16803;
const uint32 DEATH_KNIGHT_UNDERSTUDY_BLOOD_STRIKE = 61696;
const uint32 DEATH_KNIGHT_UNDERSTUDY_BONE_BARRIER = 29061;
const uint32 DEATH_KNIGHT_UNDERSTUDY_TAUNT = 29060;

class DeathKnightUnderstudyAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DeathKnightUnderstudyAI, MoonScriptCreatureAI);
    DeathKnightUnderstudyAI(Creature* pCreature);

    void Destroy();

    InstructorRazuviousAI* mRazuviousAI;
    bool mIsControlled;
};

DeathKnightUnderstudyAI::DeathKnightUnderstudyAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
{
    AddSpell(DEATH_KNIGHT_UNDERSTUDY_BLOOD_STRIKE, Target_Current, 10, 0, 4, 0, 8);
    AddSpell(DEATH_KNIGHT_UNDERSTUDY_BONE_BARRIER, Target_Self, 8, 0, 30);
    AddSpell(DEATH_KNIGHT_UNDERSTUDY_TAUNT, Target_Current, 8, 0, 10, 0, 8);

    // Blood Strike
    auto blood_strike_spell = new AI_Spell;
    blood_strike_spell->spell = sSpellCustomizations.GetSpellInfo(DEATH_KNIGHT_UNDERSTUDY_BLOOD_STRIKE);
    blood_strike_spell->agent = AGENT_SPELL;
    blood_strike_spell->entryId = getCreature()->GetEntry();
    blood_strike_spell->maxrange = GetMaxRange(sSpellRangeStore.LookupEntry(blood_strike_spell->spell->getRangeIndex()));
    blood_strike_spell->minrange = GetMinRange(sSpellRangeStore.LookupEntry(blood_strike_spell->spell->getRangeIndex()));
    blood_strike_spell->spelltargetType = TTYPE_SINGLETARGET;
    blood_strike_spell->spellType = STYPE_DAMAGE;
    blood_strike_spell->cooldown = objmgr.GetPetSpellCooldown(blood_strike_spell->spell->getId());
    blood_strike_spell->cooldowntime = 0;
    blood_strike_spell->autocast_type = AUTOCAST_EVENT_NONE;
    blood_strike_spell->floatMisc1 = 0;
    blood_strike_spell->Misc2 = 0;
    blood_strike_spell->procChance = 0;
    blood_strike_spell->procCount = 0;
    getCreature()->GetAIInterface()->addSpellToList(blood_strike_spell);
    delete blood_strike_spell;

    // Bone Barrier
    auto bone_barrier_spell = new AI_Spell;
    bone_barrier_spell->spell = sSpellCustomizations.GetSpellInfo(DEATH_KNIGHT_UNDERSTUDY_BONE_BARRIER);
    bone_barrier_spell->agent = AGENT_SPELL;
    bone_barrier_spell->entryId = getCreature()->GetEntry();
    bone_barrier_spell->maxrange = GetMaxRange(sSpellRangeStore.LookupEntry(bone_barrier_spell->spell->getRangeIndex()));
    bone_barrier_spell->minrange = GetMinRange(sSpellRangeStore.LookupEntry(bone_barrier_spell->spell->getRangeIndex()));
    bone_barrier_spell->spelltargetType = TTYPE_CASTER;
    bone_barrier_spell->spellType = STYPE_BUFF;
    bone_barrier_spell->cooldown = objmgr.GetPetSpellCooldown(bone_barrier_spell->spell->getId());
    bone_barrier_spell->cooldowntime = 0;
    bone_barrier_spell->autocast_type = AUTOCAST_EVENT_NONE;
    bone_barrier_spell->floatMisc1 = 0;
    bone_barrier_spell->Misc2 = 0;
    bone_barrier_spell->procChance = 0;
    bone_barrier_spell->procCount = 0;
    getCreature()->GetAIInterface()->addSpellToList(bone_barrier_spell);
    delete bone_barrier_spell;

    // Taunt
    auto understudy_taunt_spell = new AI_Spell;
    understudy_taunt_spell->spell = sSpellCustomizations.GetSpellInfo(DEATH_KNIGHT_UNDERSTUDY_TAUNT);
    understudy_taunt_spell->agent = AGENT_SPELL;
    understudy_taunt_spell->entryId = getCreature()->GetEntry();
    understudy_taunt_spell->maxrange = GetMaxRange(sSpellRangeStore.LookupEntry(understudy_taunt_spell->spell->getRangeIndex()));
    understudy_taunt_spell->minrange = GetMinRange(sSpellRangeStore.LookupEntry(understudy_taunt_spell->spell->getRangeIndex()));
    understudy_taunt_spell->spelltargetType = TTYPE_SINGLETARGET;
    understudy_taunt_spell->spellType = STYPE_BUFF;
    understudy_taunt_spell->cooldown = objmgr.GetPetSpellCooldown(understudy_taunt_spell->spell->getId());
    understudy_taunt_spell->cooldowntime = 0;
    understudy_taunt_spell->autocast_type = AUTOCAST_EVENT_NONE;
    understudy_taunt_spell->floatMisc1 = 0;
    understudy_taunt_spell->Misc2 = 0;
    understudy_taunt_spell->procChance = 0;
    understudy_taunt_spell->procCount = 0;
    getCreature()->GetAIInterface()->addSpellToList(understudy_taunt_spell);
    delete understudy_taunt_spell;

    mRazuviousAI = NULL;
    mIsControlled = false;
};

void DeathKnightUnderstudyAI::Destroy()
{
    if (mRazuviousAI != NULL)
    {
        std::set< DeathKnightUnderstudyAI* >::iterator Iter = mRazuviousAI->mStudents.find(this);
        if (Iter != mRazuviousAI->mStudents.end())
            mRazuviousAI->mStudents.erase(Iter);

        mRazuviousAI = NULL;
    };

    delete this;
};

/////////////////////////////////////////////////////////////////////////////////
////// The Construct Quarter

/////////////////////////////////////////////////////////////////////////////////
////// Patchwork Golem
const uint32 CN_PATCHWORK_GOLEM = 16017;
const uint32 PATCHWORK_GOLEM_CLEAVE = 27794;
const uint32 PATCHWORK_GOLEM_DISEASE_CLOUD_NORMAL = 27793;
const uint32 PATCHWORK_GOLEM_DISEASE_CLOUD_HEROIC = 28362;
const uint32 PATCHWORK_GOLEM_EXECUTE_NORMAL = 7160;
const uint32 PATCHWORK_GOLEM_EXECUTE_HEROIC = 56426;
const uint32 PATCHWORK_GOLEM_WAR_STOMP_NORMAL = 27758;
const uint32 PATCHWORK_GOLEM_WAR_STOMP_HEROIC = 56427;

class PatchworkGolemAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(PatchworkGolemAI, MoonScriptCreatureAI);
    PatchworkGolemAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void OnCombatStop(Unit* pTarget);
};

/////////////////////////////////////////////////////////////////////////////////
////// Bile Retcher
const uint32 CN_BILE_RETCHER = 16018;
const uint32 BILE_RETCHER_BILE_VOMIT_NORMAL = 27807;
const uint32 BILE_RETCHER_BILE_VOMIT_HEROIC = 54326;
const uint32 BILE_RETCHER_BILE_RETCHER_SLAM = 27862;

class BileRetcherAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(BileRetcherAI, MoonScriptCreatureAI);
    BileRetcherAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Sewage Slime
const uint32 CN_SEWAGE_SLIME = 16375;
const uint32 SEWAGE_SLIME_DISEASE_CLOUD = 28156;

class SewageSlimeAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SewageSlimeAI, MoonScriptCreatureAI);
    SewageSlimeAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void OnCombatStop(Unit* pTarget);
};

/////////////////////////////////////////////////////////////////////////////////
////// Embalming Slime
const uint32 CN_EMBALMING_SLIME = 16024;
const uint32 EMBALMING_SLIME_EMBALMING_CLOUD = 28322;

class EmbalmingSlimeAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(EmbalmingSlimeAI, MoonScriptCreatureAI);
    EmbalmingSlimeAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void OnCombatStop(Unit* pTarget);
};

/////////////////////////////////////////////////////////////////////////////////
////// Mad Scientist
const uint32 CN_MAD_SCIENTIST = 16020;
const uint32 MAD_SCIENTIST_GREAT_HEAL_NORMAL = 28306;
const uint32 MAD_SCIENTIST_GREAT_HEAL_HEROIC = 54337;
const uint32 MAD_SCIENTIST_MANA_BURN_NORMAL = 28301;
const uint32 MAD_SCIENTIST_MANA_BURN_HEROIC = 54338;

class MadScientistAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(MadScientistAI, MoonScriptCreatureAI);
    MadScientistAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Living Monstrosity
const uint32 CN_LIVING_MONSTROSITY = 16021;
const uint32 LIVING_MONSTROSITY_FEAR = 27990;
const uint32 LIVING_MONSTROSITY_LIGHTNING_TOTEM = 28294;
const uint32 LIVING_MONSTROSITY_CHAIN_LIGHTNING_NORMAL = 28293;
const uint32 LIVING_MONSTROSITY_CHAIN_LIGHTNING_HEROIC = 54334;

class LivingMonstrosityAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(LivingMonstrosityAI, MoonScriptCreatureAI);
    LivingMonstrosityAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Lightning Totem
const uint32 CN_LIGHTNING_TOTEM = 16385;
const uint32 LIGHTNING_TOTEM_SHOCK_NORMAL = 28297;
const uint32 LIGHTNING_TOTEM_SHOCK_HEROIC = 54333;

class LightningTotemAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(LightningTotemAI, MoonScriptCreatureAI);
    LightningTotemAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void AIUpdate();
};

/////////////////////////////////////////////////////////////////////////////////
////// Stitched Colossus
const uint32 CN_STITCHED_COLOSSUS = 30071;
const uint32 STITCHED_COLOSSUS_MASSIVE_STOMP_NORMAL = 55821;
const uint32 STITCHED_COLOSSUS_MASSIVE_STOMP_HEROIC = 55826;
const uint32 STITCHED_COLOSSUS_UNSTOPPABLE_ENRAGE = 54356;

class StitchedColossusAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(StitchedColossusAI, MoonScriptCreatureAI);
    StitchedColossusAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget);
    void AIUpdate();

    bool    mEnraged;
};

/////////////////////////////////////////////////////////////////////////////////
////// Marauding Geist
const uint32 CN_MARAUDING_GEIST = 30083;
const uint32 MARAUDING_GEIST_FRENZIED_LEAP = 56729;

class MaraudingGeistAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(MaraudingGeistAI, MoonScriptCreatureAI);
    MaraudingGeistAI(Creature* pCreature);
};

/////////////////////////////////////////////////////////////////////////////////
////// Patchwerk

void SpellFunc_PatchwerkHatefulStrike(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);

const uint32 CN_PATCHWERK = 16028;
const uint32 PATCHWERK_FRENZY = 28131;
const uint32 PATCHWERK_BERSERK = 26662;
const uint32 PATCHWERK_HATEFUL_STRIKE_10 = 41926;
const uint32 PATCHWERK_HATEFUL_STRIKE_25 = 59192;

class PatchwerkAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(PatchwerkAI, MoonScriptCreatureAI);
    PatchwerkAI(Creature* pCreature);

    void AIUpdate();

    bool    mEnraged;
};

// ---- Abomination Wing ----

// Stitched Spewer AI
const uint32 STICKED_SPEWER = 16025;

const uint32 CN_SLIME_BOLT = 32309;
const uint32 CN_UPPERCUT = 26007;

class StickedSpewerAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(StickedSpewerAI);
    SP_AI_Spell spells[2];
    bool m_spellcheck[2];

    StickedSpewerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // -- Number of spells to add --
        nrspells = 2;

        // --- Initialization ---
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
        }
        // ----------------------

        // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
        spells[0].info = sSpellCustomizations.GetSpellInfo(CN_SLIME_BOLT);
        spells[0].targettype = TARGET_VARIOUS;
        spells[0].instant = false;
        spells[0].perctrigger = 10.0f;
        spells[0].attackstoptimer = 2000; // 1sec

        spells[1].info = sSpellCustomizations.GetSpellInfo(CN_UPPERCUT);
        spells[1].targettype = TARGET_VARIOUS;
        spells[1].instant = false;
        spells[1].perctrigger = 10.0f;
        spells[1].attackstoptimer = 2000; // 1sec

    }

    void OnCombatStart(Unit* mTarget)
    {
        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        RemoveAIUpdateEvent();
    }

    void AIUpdate()
    {
        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                    }
                    m_spellcheck[i] = false;
                    return;
                }

                if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    protected:

    uint8 nrspells;
};



//Surgical Assistant AI
const uint32 CN_SURGICAL_ASSISTANT = 16022;

const uint32 MIND_FLAY = 28310;

class SurgicalAssistantAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(SurgicalAssistantAI);
    SP_AI_Spell spells[1];
    bool m_spellcheck[1];

    SurgicalAssistantAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // -- Number of spells to add --
        nrspells = 1;

        // --- Initialization ---
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
        }
        // ----------------------

        // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
        spells[0].info = sSpellCustomizations.GetSpellInfo(MIND_FLAY);
        spells[0].targettype = TARGET_ATTACKING;
        spells[0].instant = false;
        spells[0].perctrigger = 20.0f;
        spells[0].attackstoptimer = 6000; // 1sec

    }

    void OnCombatStart(Unit* mTarget)
    {
        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        RemoveAIUpdateEvent();
    }

    void AIUpdate()
    {
        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                    }
                    m_spellcheck[i] = false;
                    return;
                }

                if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    protected:

    uint8 nrspells;
};


// Sludge Belcher AI
const uint32 CN_SLUDGE_BELCHER = 16029;

const uint32 DISEISE_BUFFET = 27891;
const uint32 SUMMON_BILE_SLIMES = 27889; //\todo  GAWD :P

class SludgeBelcherAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(SludgeBelcherAI);
    SP_AI_Spell spells[2];
    bool m_spellcheck[2];

    SludgeBelcherAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // -- Number of spells to add --
        nrspells = 2;

        // --- Initialization ---
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
        }
        // ----------------------

        // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
        spells[0].info = sSpellCustomizations.GetSpellInfo(DISEISE_BUFFET);
        spells[0].targettype = TARGET_ATTACKING;
        spells[0].instant = false;
        spells[0].perctrigger = 20.0f;
        spells[0].attackstoptimer = 1000; // 1sec

        spells[1].info = sSpellCustomizations.GetSpellInfo(SUMMON_BILE_SLIMES);
        spells[1].targettype = TARGET_SELF;
        spells[1].instant = false;
        spells[1].perctrigger = 5.0f;
        spells[1].attackstoptimer = 3000; // 1sec

    }

    void OnCombatStart(Unit* mTarget)
    {
        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        RemoveAIUpdateEvent();
    }

    void AIUpdate()
    {
        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                    }
                    m_spellcheck[i] = false;
                    return;
                }

                if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    protected:

    uint8 nrspells;
};

// BOSS'S

// Patchwerk AI


// Grobbulus AI
const uint32 CN_GROBBULUS = 15931;

const uint32 POISON_CLOUD_GROB = 31259; // self
const uint32 SLIME_SPRAY = 28157; // various
const uint32 SUMMON_FALLOUT_SLIME = 28218; //\todo  cast on slime sprayd targets
const uint32 MUTATING_INJECTION = 28169; //\todo  DUMMY AURA

class GrobbulusAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(GrobbulusAI);
    SP_AI_Spell spells[2];
    bool m_spellcheck[2];

    GrobbulusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // -- Number of spells to add --
        nrspells = 2;

        // --- Initialization ---
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
        }
        // ----------------------

        // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
        spells[0].info = sSpellCustomizations.GetSpellInfo(POISON_CLOUD_GROB);
        spells[0].targettype = TARGET_SELF;
        spells[0].instant = false;
        spells[0].perctrigger = 0.0f;
        spells[0].attackstoptimer = 1000; // 1sec

        spells[1].info = sSpellCustomizations.GetSpellInfo(SLIME_SPRAY);
        spells[1].targettype = TARGET_VARIOUS;
        spells[1].instant = false;
        spells[1].perctrigger = 15.0f;
        spells[1].attackstoptimer = 3000; // 1sec

    }

    void OnCombatStart(Unit* mTarget)
    {
        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        getCreature()->CastSpell(getCreature(), spells[0].info, spells[0].instant);
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        RemoveAIUpdateEvent();
    }

    void AIUpdate()
    {
        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                    }
                    m_spellcheck[i] = false;
                    return;
                }

                if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    protected:

    uint8 nrspells;
};


// Gluth AI
const uint32 CN_GLUTH = 15932;

const uint32 MORTAL_WOUND = 28308; // target
const uint32 DECIMATE = 28374; //\todo  needs to be scripted
const uint32 TERRIFYING_ROAR = 37939;
const uint32 FRENZY = 38664; // self

class GluthAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(GluthAI);
    SP_AI_Spell spells[3];
    bool m_spellcheck[3];

    GluthAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // -- Number of spells to add --
        nrspells = 3;

        // --- Initialization ---
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
        }
        // ----------------------

        // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
        spells[0].info = sSpellCustomizations.GetSpellInfo(MORTAL_WOUND);
        spells[0].targettype = TARGET_ATTACKING;
        spells[0].instant = false;
        spells[0].perctrigger = 15.0f;
        spells[0].attackstoptimer = 2000; // 1sec

        spells[1].info = sSpellCustomizations.GetSpellInfo(TERRIFYING_ROAR);
        spells[1].targettype = TARGET_VARIOUS;
        spells[1].instant = false;
        spells[1].perctrigger = 5.0f;
        spells[1].attackstoptimer = 3000; // 1sec

        spells[2].info = sSpellCustomizations.GetSpellInfo(FRENZY);
        spells[2].targettype = TARGET_SELF;
        spells[2].instant = false;
        spells[2].perctrigger = 5.0f;
        spells[2].attackstoptimer = 1000; // 1sec

    }

    void OnCombatStart(Unit* mTarget)
    {
        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        RemoveAIUpdateEvent();
    }

    void OnTargetDied(Unit* mTarget)
    {
    }

    void AIUpdate()
    {
        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                    }
                    m_spellcheck[i] = false;
                    return;
                }

                if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    protected:

    uint8 nrspells;
};

// ---- Deathknight Wing ----

// Bony Construct AI
const uint32 CN_BONY_CONSTRUCT = 16167;

const uint32 SWEEPING_SLAM = 25322;
//\todo  Melee Chain Cleave - Strikes enemies in front of the Construct, chaining to anyone in melee range of the targets.

class BonyConstructAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(BonyConstructAI);
    SP_AI_Spell spells[1];
    bool m_spellcheck[1];

    BonyConstructAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // -- Number of spells to add --
        nrspells = 1;

        // --- Initialization ---
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
        }
        // ----------------------

        // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
        spells[0].info = sSpellCustomizations.GetSpellInfo(SWEEPING_SLAM);
        spells[0].targettype = TARGET_VARIOUS;
        spells[0].instant = false;
        spells[0].perctrigger = 15.0f;
        spells[0].attackstoptimer = 2000; // 1sec

    }

    void OnCombatStart(Unit* mTarget)
    {
        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        RemoveAIUpdateEvent();
    }

    void AIUpdate()
    {
        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                    }
                    m_spellcheck[i] = false;
                    return;
                }

                if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    protected:

    uint8 nrspells;
};



// Death Lord AI
const uint32 CN_DEATH_LORD = 16861;

const uint32 AURA_OF_AGONY = 28413;

class DeathLordAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(DeathLordAI);
    SP_AI_Spell spells[1];
    bool m_spellcheck[1];

    DeathLordAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // -- Number of spells to add --
        nrspells = 1;

        // --- Initialization ---
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
        }
        // ----------------------

        // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
        spells[0].info = sSpellCustomizations.GetSpellInfo(AURA_OF_AGONY);
        spells[0].targettype = TARGET_VARIOUS;
        spells[0].instant = false;
        spells[0].perctrigger = 15.0f;
        spells[0].attackstoptimer = 2000; // 1sec

    }

    void OnCombatStart(Unit* mTarget)
    {
        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        RemoveAIUpdateEvent();
    }

    void AIUpdate()
    {
        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                    }
                    m_spellcheck[i] = false;
                    return;
                }

                if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    protected:

    uint8 nrspells;
};

// -- BOSS'S --

// Instructor Razuvious AI
const uint32 UNBALANCING_STRIKE = 26613;
const uint32 DISRUPTING_SHOUT = 29107;

class RazuviousAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(RazuviousAI);
    SP_AI_Spell spells[2];
    bool m_spellcheck[2];

    RazuviousAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // -- Number of spells to add --
        nrspells = 2;

        // --- Initialization ---
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
        }
        // ----------------------

        // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
        spells[0].info = sSpellCustomizations.GetSpellInfo(DISRUPTING_SHOUT);
        spells[0].targettype = TARGET_VARIOUS;
        spells[0].instant = false;
        spells[0].perctrigger = 15.0f;
        spells[0].attackstoptimer = 3000; // 1sec

        spells[1].info = sSpellCustomizations.GetSpellInfo(UNBALANCING_STRIKE);
        spells[1].targettype = TARGET_ATTACKING;
        spells[1].instant = false;
        spells[1].perctrigger = 35.0f;
        spells[1].attackstoptimer = 1000; // 1sec

    }

    void OnCombatStart(Unit* mTarget)
    {
        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        RemoveAIUpdateEvent();
    }

    void OnTargetDied(Unit* mTarget)
    {
        if (Rand(50.0f))
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "You should've stayed home!");
            getCreature()->PlaySoundToSet(8862);
        }
        else
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "You disappoint me, students!");
            getCreature()->PlaySoundToSet(8863);
        }
    }

    void OnDied(Unit* mKiller)
    {
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "An honorable... death...");
        getCreature()->PlaySoundToSet(8860);
    }

    void AIUpdate()
    {
        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                    }
                    m_spellcheck[i] = false;
                    return;
                }

                if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    protected:

    uint8 nrspells;
};
// - The Four Horsemen: -

// Thane Korth'azz AI
const uint32 CN_THANE_KORTHAZZ = 16064;

const uint32 MARK_OF_KORTHAZZ = 28832;
const uint32 METEOR = 35181; // 1 target

class KorthazzAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(KorthazzAI);
    SP_AI_Spell spells[2];
    bool m_spellcheck[2];

    KorthazzAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // -- Number of spells to add --
        nrspells = 2;

        // --- Initialization ---
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
        }
        // ----------------------

        tountcooldown = 6;
        tountcont = 0;
        m_attackstart = false;

        // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
        spells[0].info = sSpellCustomizations.GetSpellInfo(MARK_OF_KORTHAZZ);
        spells[0].targettype = TARGET_VARIOUS;
        spells[0].instant = false;
        spells[0].perctrigger = 5.0f;
        spells[0].attackstoptimer = 1000; // 1sec

        spells[1].info = sSpellCustomizations.GetSpellInfo(METEOR);
        spells[1].targettype = TARGET_ATTACKING;
        spells[1].instant = false;
        spells[1].perctrigger = 15.0f;
        spells[1].attackstoptimer = 3000; // 1sec

    }

    void OnCombatStart(Unit* mTarget)
    {
        m_attackstart = true;
        sendDBChatMessage(4242);     // C'mon an' fight ye wee ninny!

        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* mTarget)
    {
        m_attackstart = false;
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        RemoveAIUpdateEvent();
    }

    void OnTargetDied(Unit* mTarget)
    {
        sendDBChatMessage(4247);     // Next time, bring more friends!
    }

    void OnDied(Unit* mKiller)
    {
        sendDBChatMessage(4248);     // What a bloody waste this is!
        getCreature()->CastSpell(getCreature(), spells[1].info, spells[1].instant);
    }

    void AIUpdate()
    {
        if (!m_attackstart)
        {
            if (!tountcooldown)
            {
                tountcooldown = 6;

                switch (tountcont)
                {
                    case 0:
                        sendDBChatMessage(4243);     // To arms, ye roustabouts! We've got company!
                        break;
                    case 1:
                        sendDBChatMessage(4244);     // I heard about enough of yer sniveling. Shut yer fly trap 'afore I shut it for ye!
                        break;
                    case 2:
                        sendDBChatMessage(4245);     // I'm gonna enjoy killin' these slack-jawed daffodils!
                        break;
                }
                tountcont++;
                if (tountcont >= 3)
                    tountcont = 0;
            }
            tountcooldown--;
        }

        if (m_spellcheck[0])
        {
            sendDBChatMessage(4246);     // I like my meat extra crispy!
        }

        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                    }
                    m_spellcheck[i] = false;
                    return;
                }

                if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    protected:

    bool m_attackstart;
    uint32 tountcooldown, tountcont;
    uint8 nrspells;
};

/*
8899 - Come out and fight ye wee ninny!
8900 - what a bloody waste this is!
8901 - next time. bring more friend
8902 - i like my meat extra crispy
8903 - To arms, ye roustabouts! We've got company!
8904 - I heard about enough of yer sniveling. Shut yer fly trap 'afore I shut it for ye!
8905 - I'm gonna enjoy killin' these slack-jawed daffodils!

8899, 'A_KOR_NAXX_AGGRO'
8900, 'A_KOR_NAXX_DEATH'
8901, 'A_KOR_NAXX_SLAY'
8902, 'A_KOR_NAXX_SPECIAL'
8903, 'A_KOR_NAXX_TAUNT01'
8904, 'A_KOR_NAXX_TAUNT02'
8905, 'A_KOR_NAXX_TAUNT03'
*/

// Baron Rivendare AI
const uint32 CN_Baron_Rivendare_4H = 30549; //4H not to confuse with Strat UD Side..
//Gief new Strat boss blizz or you make Stab a sad panda


// Lady Blaumeux AI
const uint32 CN_LADY_BLAUMEUX = 16065;

const uint32 MARK_OF_BLAUMEUX = 28833;
const uint32 VOID_ZONE = 28863; //\todo  DUMMY PART

class BlaumeuxAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(BlaumeuxAI);
    SP_AI_Spell spells[2];
    bool m_spellcheck[2];

    BlaumeuxAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // -- Number of spells to add --
        nrspells = 2;

        // --- Initialization ---
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
        }
        // ----------------------

        tountcooldown = 16;
        tountcont = 0;
        m_attackstart = false;

        // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
        spells[0].info = sSpellCustomizations.GetSpellInfo(MARK_OF_BLAUMEUX);
        spells[0].targettype = TARGET_VARIOUS;
        spells[0].instant = false;
        spells[0].perctrigger = 5.0f;
        spells[0].attackstoptimer = 1000; // 1sec

        spells[1].info = sSpellCustomizations.GetSpellInfo(VOID_ZONE);
        spells[1].targettype = TARGET_VARIOUS;
        spells[1].instant = false;
        spells[1].perctrigger = 15.0f;
        spells[1].attackstoptimer = 3000; // 1sec

    }

    void OnCombatStart(Unit* mTarget)
    {
        m_attackstart = true;
        sendDBChatMessage(4249);     // Defend yourself!

        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* mTarget)
    {
        m_attackstart = false;
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        RemoveAIUpdateEvent();
    }

    void OnTargetDied(Unit* mTarget)
    {
        sendDBChatMessage(4254);     // Who's next?
    }

    void OnDied(Unit* mKiller)
    {
        sendDBChatMessage(4255);     // Touche...
        getCreature()->CastSpell(getCreature(), spells[1].info, spells[1].instant);
    }

    void AIUpdate()
    {
        if (!m_attackstart)
        {
            if (!tountcooldown)
            {
                tountcooldown = 16;

                switch (tountcont)
                {
                    case 0:
                        sendDBChatMessage(4250);     // Come, Zeliek, do not drive them out. Not before we've had our fun!
                        break;
                    case 1:
                        sendDBChatMessage(4251);     // I do hope they stay alive long enough for me to... introduce myself.
                        break;
                    case 2:
                        sendDBChatMessage(4252);     // The first kill goes to me! Anyone care to wager?
                        break;
                }
                tountcont++;
                if (tountcont >= 3)
                    tountcont = 0;
            }
            tountcooldown--;
        }

        if (m_spellcheck[0])
        {
            sendDBChatMessage(4253);     // Your life is mine!
        }

        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                    }
                    m_spellcheck[i] = false;
                    return;
                }

                if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    protected:

    bool m_attackstart;
    uint32 tountcooldown, tountcont;
    uint8 nrspells;
};

/*
8892 - Defend yourself
8893 - Tou...che
8894 - Who's next?
8895 - Your life is mine!
8896 - Come, Zeliek, do not drive them out. Not before we've had our fun
8897 - I do hope they stay alive long enough for me to... introduce myself,
8898 - The first kill goes to me! Anyone care to wager?

8892, 'A_BLA_NAXX_AGGRO'
8893, 'A_BLA_NAXX_DEATH'
8894, 'A_BLA_NAXX_SLAY'
8895, 'A_BLA_NAXX_SPECIAL'
8896, 'A_BLA_NAXX_TAUNT01'
8897, 'A_BLA_NAXX_TAUNT02'
8898, 'A_BLA_NAXX_TAUNT03'
*/

// Sir Zeliek AI
const uint32 CN_SIR_ZELIEK = 16063;

const uint32 MARK_OF_ZELIEK = 28835;
const uint32 HOLY_WRATH = 32445; // 1target

class ZeliekAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(ZeliekAI);
    SP_AI_Spell spells[2];
    bool m_spellcheck[2];

    ZeliekAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // -- Number of spells to add --
        nrspells = 2;

        // --- Initialization ---
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
        }
        // ----------------------

        tountcooldown = 13;
        tountcont = 0;
        m_attackstart = false;

        // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
        spells[0].info = sSpellCustomizations.GetSpellInfo(MARK_OF_ZELIEK);
        spells[0].targettype = TARGET_VARIOUS;
        spells[0].instant = false;
        spells[0].perctrigger = 5.0f;
        spells[0].attackstoptimer = 1000; // 1sec

        spells[1].info = sSpellCustomizations.GetSpellInfo(HOLY_WRATH);
        spells[1].targettype = TARGET_ATTACKING;
        spells[1].instant = false;
        spells[1].perctrigger = 15.0f;
        spells[1].attackstoptimer = 1000; // 1sec

    }

    void OnCombatStart(Unit* mTarget)
    {
        m_attackstart = true;
        sendDBChatMessage(4266);     // Flee, before it's too late!

        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* mTarget)
    {
        m_attackstart = false;
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        RemoveAIUpdateEvent();
    }

    void OnTargetDied(Unit* mTarget)
    {
        sendDBChatMessage(4271);     // Forgive me!
    }

    void OnDied(Unit* mKiller)
    {
        sendDBChatMessage(4272);     // It is... as it should be.
        getCreature()->CastSpell(getCreature(), spells[1].info, spells[1].instant);
    }

    void AIUpdate()
    {
        if (!m_attackstart)
        {
            if (!tountcooldown)
            {
                tountcooldown = 13;

                switch (tountcont)
                {
                    case 0:
                        sendDBChatMessage(4267);     // Invaders, cease this foolish venture at once! Turn away while you still can!
                        break;
                    case 1:
                        sendDBChatMessage(4268);     // Perhaps they will come to their senses, and run away as fast as they can!");
                        break;
                    case 2:
                        sendDBChatMessage(4269);     // Do not continue! Turn back while there's still time!");
                        break;

                }
                tountcont++;
                if (tountcont >= 3)
                    tountcont = 0;
            }
            tountcooldown--;
        }

        if (m_spellcheck[0])
        {
            sendDBChatMessage(4270);     // I have no choice but to obey!
        }

        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                    }
                    m_spellcheck[i] = false;
                    return;
                }

                if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    protected:

    bool m_attackstart;
    uint32 tountcooldown, tountcont;
    uint8 nrspells;
};

// -- Sapphiron Encounter by M4ksiu -- //

// Settings

#define DISABLE_FROST_BREATH

// Frost Breath TrgiggerAI

const uint32 CN_FROST_BREATH_TRIGGER = 50010;   // Flying "ball" of frost energy
const uint32 CN_FROST_BREATH_TRIGGER2 = 50011;    // Explosion
const uint32 CN_FROST_BREATH_TRIGGER3 = 50012;    // Wing Buffet
const uint32 CN_CHILL_TRIGGER = 50013;    // Mass chill trigger (used, because we can't cast many aoe triggers on one character >_>)

// Air phase spells
const uint32 FROST_BREATH = 28524;
const uint32 FROST_BREATH_EFFECT = 30101;
const uint32 FROST_BREATH_DAMAGE = 29318;

// Additional spells
const uint32 SAPPHIRONS_WING_BUFFET = 29328;

struct Movement::Location PhaseTwoWP[] =
{
    {},
    { 3520.820068f, -5233.799805f, 137.626007f, 4.553010f }
};

struct Movement::Location IceBlocks[] =    // Those are not blizzlike pos, because those blocks are spawned randomly
{
    {},
    { 3580.986084f, -5241.330078f, 137.627304f, 3.006957f },
    { 3562.967285f, -5257.952148f, 137.860916f, 2.468959f },
    { 3569.620850f, -5276.108398f, 137.582733f, 2.480744f },
    { 3551.420410f, -5283.535156f, 137.731903f, 2.009505f },
    { 3535.933594f, -5294.710938f, 138.080002f, 1.823366f },
    { 3522.235107f, -5286.610352f, 138.115601f, 1.532768f },
    { 3503.184814f, -5296.418945f, 138.111252f, 1.222535f },
    { 3489.055664f, -5278.863770f, 138.119934f, 0.884814f },
    { 3473.002686f, -5277.641602f, 137.733414f, 0.680609f },
    { 3472.302734f, -5255.734863f, 137.755569f, 0.331107f },
    { 3458.193848f, -5241.013672f, 137.566147f, 0.111195f },
    { 3463.324463f, -5221.530273f, 137.634888f, 6.084152f },
    { 3467.574219f, -5200.617676f, 137.559662f, 5.860314f },
    { 3479.394775f, -5178.301758f, 140.904312f, 5.405583f },
    { 3507.219727f, -5180.725098f, 140.625473f, 4.431685f },
    { 3518.371338f, -5172.666504f, 142.269135f, 4.694800f },
    { 3542.516846f, -5184.699707f, 140.655182f, 4.470973f },
    { 3559.013916f, -5183.916016f, 140.899689f, 4.644558f },
    { 3559.006592f, -5183.923340f, 140.895554f, 3.952624f },
    { 3571.978760f, -5209.633301f, 137.671906f, 3.514374f }
};

class FrostBreathTriggerAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(FrostBreathTriggerAI);

    FrostBreathTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->MoveTo(PhaseTwoWP[1].x, PhaseTwoWP[1].y, PhaseTwoWP[1].z + 10.5f);
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->GetAIInterface()->setSplineFlying();
        getCreature()->m_noRespawn = true;
        getCreature()->Despawn(7000, 0);

        RegisterAIUpdateEvent(1000);

        AICounter = 7;
    }

    void OnCombatStart(Unit* mTarget) {}

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
    }

    void AIUpdate()
    {
        getCreature()->CastSpell(getCreature(), FROST_BREATH_EFFECT, true);

        AICounter--;
        if (AICounter == 6)
            getCreature()->GetAIInterface()->MoveTo(PhaseTwoWP[1].x, PhaseTwoWP[1].y, PhaseTwoWP[1].z + AICounter * 1.5f);
        else
            getCreature()->GetAIInterface()->MoveTo(PhaseTwoWP[1].x, PhaseTwoWP[1].y, PhaseTwoWP[1].z);
    }

    protected:

    int AICounter;
};

class FrostBreathTrigger2AI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(FrostBreathTrigger2AI);

    FrostBreathTrigger2AI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
#ifdef DISABLE_FROST_BREATH
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
#else
        _unit->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
#endif
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->m_canMove = false;
        getCreature()->m_noRespawn = true;
        getCreature()->Despawn(8000, 0);

        getCreature()->CastSpell(getCreature(), FROST_BREATH, false);
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
    }
};

class FrostBreathTrigger3AI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(FrostBreathTrigger3AI);

    FrostBreathTrigger3AI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
        getCreature()->CastSpell(getCreature(), SAPPHIRONS_WING_BUFFET, true);
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->m_canMove = false;
        getCreature()->m_noRespawn = true;

        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
    }

    void AIUpdate()
    {
        getCreature()->CastSpell(getCreature(), SAPPHIRONS_WING_BUFFET, true);
    }
};

class ChillTriggerAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(ChillTriggerAI);

    ChillTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->CastSpellAoF(getCreature()->GetPosition(), sSpellCustomizations.GetSpellInfo(28547), true);
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->m_canMove = false;
        getCreature()->m_noRespawn = true;
        getCreature()->Despawn(15000, 0);
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
    }
};

// SapphironAI
// Missing spawning effect with building skeleton of Sappiron
const uint32 CN_SAPPHIRON = 15989;

// Land phase spells
const uint32 LIFE_DRAIN = 28542;
const uint32 CHILL = 28547;
const uint32 FROST_AURA = 28531;

// Air phase spells
const uint32 ICEBOLT = 28522;

// Additional spells
const uint32 SAPPHIRON_DIES = 29357;
const uint32 BERSERK = 26662;    // 28498 - casts frostbolt (would be cool for Sapphiron), but every 2 sec for 5 min (~16k dmg per hit);
// 27680 - 10 mins instead 5 mins
// Researches
const uint32 SAPPHIRON_BIRTH = 181356;
const uint32 FROSTWYRM_WATERFALL_DOOR = 181225;
const uint32 ICE_BLOCK_GO = 181247;

// Immunities
const uint32 IMMUNITY_DISEASE = 6681;
const uint32 IMMUNITY_SHADOW = 7743;
const uint32 IMMUNITY_FROST = 7940;
const uint32 IMMUNITY_NATURE = 7941;
const uint32 IMMUNITY_FIRE = 7942;
const uint32 IMMUNITY_HOLY = 34182;
const uint32 IMMUNITY_ARCANE = 34184;
const uint32 IMMUNITY_PHYSICAL = 34310;

class SapphironAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(SapphironAI);
    SP_AI_Spell spells[4];
    bool m_spellcheck[4];

    SapphironAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        AddWaypoint(CreateWaypoint(1, 3000, Movement::WP_MOVE_TYPE_RUN, PhaseTwoWP[1]));

        nrspells = 2;
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
            spells[i].casttime = 0;
        }

        spells[0].info = sSpellCustomizations.GetSpellInfo(LIFE_DRAIN);
        spells[0].targettype = TARGET_VARIOUS;
        spells[0].instant = true;
        spells[0].perctrigger = 8.0f;
        spells[0].cooldown = 20;
        spells[0].attackstoptimer = 2000;

        spells[1].info = sSpellCustomizations.GetSpellInfo(CHILL);
        spells[1].targettype = TARGET_RANDOM_DESTINATION;
        spells[1].instant = true;
        spells[1].perctrigger = 10.0f;
        spells[1].cooldown = 15;
        spells[1].attackstoptimer = 1000;
        spells[1].mindist2cast = 0.0f;
        spells[1].maxdist2cast = 40.0f;
        spells[1].minhp2cast = 0;
        spells[1].maxhp2cast = 100;

        spells[2].info = sSpellCustomizations.GetSpellInfo(ICEBOLT);
        spells[2].targettype = TARGET_RANDOM_SINGLE;
        spells[2].instant = true;
        spells[2].perctrigger = 0.0f;
        spells[2].cooldown = 0;
        spells[2].attackstoptimer = 1000;
        spells[2].mindist2cast = 0.0f;
        spells[2].maxdist2cast = 70.0f;
        spells[2].minhp2cast = 0;
        spells[2].maxhp2cast = 100;

        spells[3].info = sSpellCustomizations.GetSpellInfo(BERSERK);
        spells[3].targettype = TARGET_SELF;
        spells[3].instant = true;
        spells[3].perctrigger = 0.0f;
        spells[3].cooldown = 900;
        spells[3].attackstoptimer = 1000;

        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
        getCreature()->GetAIInterface()->unsetSplineFlying();
        getCreature()->GetAIInterface()->m_canMove = true;
        getCreature()->CastSpell(getCreature(), IMMUNITY_FROST, true);

        getCreature()->setMoveHover(false);

        ChillTarget = NULL;
        FlightActions = 0;
        ChillCounter = 0;
        PhaseTimer = 0;
        m_phase = 1;
    }

    void OnCombatStart(Unit* mTarget)
    {
        for (uint8 i = 0; i < nrspells; i++)
            spells[i].casttime = 0;

        spells[3].casttime = (uint32)time(NULL) + spells[3].cooldown;

        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
        getCreature()->GetAIInterface()->unsetSplineFlying();
        getCreature()->GetAIInterface()->m_canMove = true;

        getCreature()->setMoveHover(false);

        GameObject* Waterfall = getNearestGameObject(3536.852783f, -5159.951172f, 143.636139f, FROSTWYRM_WATERFALL_DOOR);
        if (Waterfall != NULL)
        {
            Waterfall->SetState(GO_STATE_CLOSED);
        }

        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));

        PhaseTimer = (uint32)time(NULL) + 35;
        ChillTarget = NULL;
        FlightActions = 0;
        ChillCounter = 0;
        m_phase = 1;
    }

    void OnCombatStop(Unit* mTarget)
    {
        Creature* BreathTrigger = NULL;
        BreathTrigger = getNearestCreature(PhaseTwoWP[1].x, PhaseTwoWP[1].y, PhaseTwoWP[1].z, CN_FROST_BREATH_TRIGGER3);
        if (BreathTrigger != NULL)
            BreathTrigger->Despawn(0, 0);

        for (uint8 i = 1; i < 21; i++)
        {
            GameObject* IceBlock = getNearestGameObject(IceBlocks[i].x, IceBlocks[i].y, IceBlocks[i].z, ICE_BLOCK_GO);
            if (IceBlock != NULL)
            {
                IceBlock->Delete();
            }
        }

        GameObject* Waterfall = getNearestGameObject(3536.852783f, -5159.951172f, 143.636139f, FROSTWYRM_WATERFALL_DOOR);
        if (Waterfall != NULL)
        {
            Waterfall->SetState(GO_STATE_OPEN);
        }

        getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
        getCreature()->GetAIInterface()->unsetSplineFlying();
        getCreature()->GetAIInterface()->m_canMove = true;

        getCreature()->setMoveHover(false);

        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

        RemoveAIUpdateEvent();
    }

    void OnTargetDied(Unit* mTarget)
    {
    }

    void OnDied(Unit* mKiller)
    {
        getCreature()->CastSpell(getCreature(), SAPPHIRON_DIES, true);
    }

    void AIUpdate()
    {
        uint32 t = (uint32)time(NULL);
        if (t > spells[3].casttime && getCreature()->GetCurrentSpell() == NULL)
        {
            getCreature()->CastSpell(getCreature(), spells[3].info, spells[3].instant);

            spells[3].casttime = t + spells[3].cooldown;
        }

        switch (m_phase)
        {
            case 1:
                PhaseOne();
                break;
            case 2:
                PhaseTwo();
                break;
            default:
            {
            }
        }
    }

    void PhaseOne()
    {
        getCreature()->CastSpell(getCreature(), FROST_AURA, true);

        if (getCreature()->GetAIInterface()->getWaypointScriptType() == Movement::WP_MOVEMENT_SCRIPT_WANTEDWP)
            return;

        if (getCreature()->GetHealthPct() > 10)
        {
            uint32 t = (uint32)time(NULL);
            if (t > PhaseTimer)
            {
                if (getCreature()->GetCurrentSpell() != NULL)
                    getCreature()->GetCurrentSpell()->cancel();

                getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
                getCreature()->GetAIInterface()->StopMovement(0);
                getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(1);

                return;
            }
        }

        if (ChillCounter > 0)
        {
            ChillCounter--;
            if (ChillTarget != NULL)
            {
                spawnCreature(CN_CHILL_TRIGGER, ChillTarget->GetPosition());
            }

            if (ChillCounter == 0)
            {
                ChillTarget = NULL;
            }
        }

        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void PhaseTwo()
    {
        if (FlightActions == 0)
        {
            getCreature()->GetAIInterface()->m_canMove = false;
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
            setAIAgent(AGENT_SPELL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
            getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
            getCreature()->GetAIInterface()->setWayPointToMove(0);
        }

        if (FlightActions < 5)
        {
            if (getCreature()->GetCurrentSpell() == NULL)
            {
                if (getCreature()->GetAIInterface()->getNextTarget() != NULL)
                {
                    CastSpellOnRandomTarget(2, 0.0f, 40.0f, 0, 100);

                    FlightActions++;
                    if (FlightActions >= 5)
                    {
                        uint32 LastOne = 0;
                        for (uint8 i = 0; i < 2; i++)
                        {
                            uint32 Block = 0;
                            while (LastOne == Block)
                            {
                                Block = RandomUInt(5) + 15;
                            }

                            LastOne = Block;

                            GameObject* IceBlock = NULL;
                            IceBlock = getCreature()->GetMapMgr()->GetInterface()->SpawnGameObject(ICE_BLOCK_GO, IceBlocks[Block].x, IceBlocks[Block].y, IceBlocks[Block].z, IceBlocks[Block].o, true, 0, 0);
                            if (IceBlock != NULL)
                            {
                                IceBlock->SetFlags(GO_FLAG_NONSELECTABLE);
                            }
                        }

                        RemoveAIUpdateEvent();
                        RegisterAIUpdateEvent(3000);

                        FlightActions = 5;
                    }

                    if (FlightActions == 2)
                    {
                        for (uint8 i = 0; i < 2; i++)
                        {
                            uint32 Block = 0;
                            if (i == 0)
                                Block = RandomUInt(1, 3);
                            else
                                Block = RandomUInt(10, 13);

                            GameObject* IceBlock = NULL;
                            IceBlock = getCreature()->GetMapMgr()->GetInterface()->SpawnGameObject(ICE_BLOCK_GO, IceBlocks[Block].x, IceBlocks[Block].y, IceBlocks[Block].z, IceBlocks[Block].o, true, 0, 0);
                            if (IceBlock != NULL)
                            {
                                IceBlock->SetFlags(GO_FLAG_NONSELECTABLE);
                            }
                        }
                    }

                    if (FlightActions == 4)
                    {
                        for (uint8 i = 0; i < 2; i++)
                        {
                            uint32 Block = 0;
                            if (i == 0)
                                Block = RandomUInt(3) + 7;
                            else
                                Block = RandomUInt(7) + 13;

                            GameObject* IceBlock = NULL;
                            IceBlock = getCreature()->GetMapMgr()->GetInterface()->SpawnGameObject(ICE_BLOCK_GO, IceBlocks[Block].x, IceBlocks[Block].y, IceBlocks[Block].z, IceBlocks[Block].o, true, 0, 0);
                            if (IceBlock != NULL)
                            {
                                IceBlock->SetFlags(GO_FLAG_NONSELECTABLE);
                            }
                        }
                    }
                }
            }
        }

        else
        {
            if (FlightActions == 5)
            {
                Unit* FlyingFrostBreath = NULL;
                FlyingFrostBreath = spawnCreature(CN_FROST_BREATH_TRIGGER, PhaseTwoWP[1].x, PhaseTwoWP[1].y, PhaseTwoWP[1].z + 18.0f, getCreature()->GetOrientation());
                if (FlyingFrostBreath != NULL)
                {
                    FlyingFrostBreath->GetAIInterface()->MoveTo(PhaseTwoWP[1].x, PhaseTwoWP[1].y, PhaseTwoWP[1].z);
                }

                spawnCreature(CN_FROST_BREATH_TRIGGER2, PhaseTwoWP[1].x, PhaseTwoWP[1].y, PhaseTwoWP[1].z, getCreature()->GetOrientation());

                RemoveAIUpdateEvent();
                RegisterAIUpdateEvent(10000);

                Creature* BreathTrigger = NULL;
                BreathTrigger = getNearestCreature(PhaseTwoWP[1].x, PhaseTwoWP[1].y, PhaseTwoWP[1].z, CN_FROST_BREATH_TRIGGER3);
                if (BreathTrigger != NULL)
                    BreathTrigger->Despawn(0, 0);
            }

            if (FlightActions == 6)
            {
                for (uint8 i = 1; i < 21; i++)
                {
                    GameObject* IceBlock = getNearestGameObject(IceBlocks[i].x, IceBlocks[i].y, IceBlocks[i].z, ICE_BLOCK_GO);
                    if (IceBlock != NULL)
                    {
                        IceBlock->Delete();
                    }
                }

                getCreature()->GetAIInterface()->unsetSplineFlying();
                getCreature()->Emote(EMOTE_ONESHOT_LAND);

                getCreature()->setMoveHover(false);

                RemoveAIUpdateEvent();
                RegisterAIUpdateEvent(3000);
            }

            if (FlightActions == 7)
            {
                getCreature()->GetAIInterface()->m_canMove = true;
                getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
                setAIAgent(AGENT_NULL);
                getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                getCreature()->GetAIInterface()->setWayPointToMove(0);

                RemoveAIUpdateEvent();
                RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));

                PhaseTimer = (uint32)time(NULL) + 67;
                ChillTarget = NULL;
                FlightActions = 0;
                ChillCounter = 0;
                m_phase = 1;
            }

            FlightActions++;
        }
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget() != NULL)
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_RANDOM_FRIEND:
                        case TARGET_RANDOM_SINGLE:
                        case TARGET_RANDOM_DESTINATION:
                            CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                            break;
                    }

                    if (spells[i].speech != "")
                    {
                        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                        getCreature()->PlaySoundToSet(spells[i].soundid);
                    }

                    m_spellcheck[i] = false;
                    return;
                }

                uint32 t = (uint32)time(NULL);
                if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) && t > spells[i].casttime)
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    spells[i].casttime = t + spells[i].cooldown;
                    m_spellcheck[i] = true;
                }

                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
    {
        if (!maxdist2cast) maxdist2cast = 100.0f;
        if (!maxhp2cast) maxhp2cast = 100;

        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget() != NULL)
        {
            std::vector<Unit*> TargetTable;
            for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
            {
                if (isHostile(getCreature(), (*itr)) && (*itr) != getCreature() && (*itr)->IsUnit())
                {
                    Unit* RandomTarget = NULL;
                    RandomTarget = static_cast<Unit*>(*itr);

                    if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && getCreature()->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && getCreature()->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(getCreature(), RandomTarget))
                    {
                        TargetTable.push_back(RandomTarget);
                    }
                }
            }

            if (!TargetTable.size())
                return;

            auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
            auto random_target = TargetTable[random_index];

            if (random_target == nullptr)
                return;

            if (i == 1)
            {
                ChillCounter = RandomUInt(3) + 3;
                ChillTarget = random_target;
            }
            else
            {
                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        getCreature()->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        getCreature()->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }
            }

            TargetTable.clear();
        }
    }

    void OnReachWP(uint32 iWaypointId, bool bForwards)
    {
        if (iWaypointId == 1)
        {
            spawnCreature(CN_FROST_BREATH_TRIGGER3, PhaseTwoWP[1].x, PhaseTwoWP[1].y, PhaseTwoWP[1].z, getCreature()->GetOrientation());
            getCreature()->GetAIInterface()->setSplineFlying();
            getCreature()->Emote(EMOTE_ONESHOT_LIFTOFF);

            getCreature()->setMoveHover(true);

            RemoveAIUpdateEvent();
            RegisterAIUpdateEvent(3500);

            ChillTarget = NULL;
            FlightActions = 0;
            ChillCounter = 0;
            m_phase = 2;
        }
    }

    protected:

    Unit* ChillTarget;    // I don't like it >_>

    uint32 FlightActions;
    uint32 ChillCounter;
    uint32 PhaseTimer;
    uint32 m_phase;
    uint8 nrspells;
};

// -- Kel'thuzad Encounter by M4ksiu -- //

// Encounter mobs

const uint32 CN_THE_LICH_KING = 16980;
const uint32 CN_SOLDIER_OF_THE_FROZEN_WASTES = 16427;
const uint32 CN_UNSTOPPABLE_ABOMINATION = 16428;
const uint32 CN_SOUL_WEAVER = 16429;
const uint32 CN_GUARDIAN_OF_ICECROWN = 16441;

/*
  _____
  /  K  \
  /1     4\
  |       |
  |2     5|
  |       |
  \3     6/
  \_ 7 _/
  | |

  */

static Movement::Location SFrozenWastes[] =    // Soldier of the Frozen Wastes (no idea about those :|)
{
    { 3759.149902f, -5074.879883f, 143.175003f, 1.203640f },    // 1
    { 3762.959961f, -5067.399902f, 143.453003f, 0.893413f },
    { 3772.419922f, -5076.379883f, 143.466995f, 3.606970f },
    { 3779.699951f, -5078.180176f, 143.764008f, 4.038940f },
    { 3770.219971f, -5065.740234f, 143.477005f, 0.630304f },
    { 3765.709961f, -5060.799805f, 143.748001f, 1.608120f },
    { 3776.909912f, -5066.100098f, 143.550003f, 5.130640f },
    { 3782.659912f, -5069.529785f, 143.757004f, 5.150280f },
    { 3773.909912f, -5059.589844f, 143.774002f, 6.257680f },
    { 3780.260010f, -5061.580078f, 143.742996f, 5.169910f },
    { 3721.429932f, -5052.759766f, 143.442993f, 1.457330f },    // 2
    { 3732.149902f, -5051.589844f, 143.444000f, 1.017500f },
    { 3741.889893f, -5047.439941f, 143.886002f, 3.075250f },
    { 3726.229980f, -5043.410156f, 143.455994f, 6.150070f },
    { 3718.679932f, -5042.520020f, 143.768005f, 1.614410f },
    { 3733.060059f, -5040.979980f, 143.557007f, 1.669380f },
    { 3741.860107f, -5038.410156f, 143.917999f, 4.410420f },
    { 3736.189941f, -5032.810059f, 143.847000f, 5.026970f },
    { 3723.219971f, -5035.770020f, 143.764999f, 4.701020f },
    { 3728.760010f, -5031.759766f, 143.785995f, 3.723200f },
    { 3683.189941f, -5062.419922f, 143.175995f, 0.559623f },    // 3
    { 3687.739990f, -5057.779785f, 143.175995f, 1.345020f },
    { 3674.040039f, -5067.899902f, 143.524994f, 0.005909f },
    { 3688.340088f, -5052.009766f, 143.473007f, 2.299280f },
    { 3681.659912f, -5052.549805f, 143.263000f, 3.343860f },
    { 3678.840080f, -5055.529785f, 143.332993f, 3.512720f },
    { 3685.800049f, -5045.779785f, 143.615997f, 6.253750f },
    { 3673.479980f, -5053.350098f, 143.554993f, 1.945840f },
    { 3670.649902f, -5061.149902f, 143.820007f, 4.266700f },
    { 3688.018311f, -5061.541992f, 143.178223f, 0.512497f },
    { 3742.659912f, -5157.009766f, 143.171005f, 0.773261f },    // 4
    { 3750.310059f, -5153.779785f, 143.179993f, 5.603460f },
    { 3739.770020f, -5163.000000f, 143.662994f, 1.095270f },
    { 3746.649902f, -5164.560059f, 143.520004f, 1.970990f },
    { 3748.479980f, -5169.109863f, 143.649002f, 2.591460f },
    { 3744.560059f, -5173.520020f, 143.895996f, 1.370160f },
    { 3758.260010f, -5156.299805f, 143.570999f, 6.192510f },
    { 3762.620117f, -5159.149902f, 143.712997f, 4.433220f },
    { 3760.840088f, -5162.959961f, 143.649002f, 4.280060f },
    { 3756.580078f, -5170.060059f, 143.623001f, 3.031280f },
    { 3705.229980f, -5163.189941f, 143.266998f, 5.692200f },    // 5
    { 3696.949951f, -5163.370117f, 143.608002f, 0.901274f },
    { 3710.080078f, -5168.709961f, 143.585999f, 4.675110f },
    { 3697.030029f, -5170.220215f, 143.606003f, 0.343642f },
    { 3692.030029f, -5170.020020f, 143.815994f, 0.932691f },
    { 3710.320068f, -5175.319824f, 143.705002f, 4.639770f },
    { 3694.550049f, -5177.540039f, 143.839005f, 1.549230f },
    { 3705.830078f, -5179.120117f, 143.729996f, 3.956480f },
    { 3712.800049f, -5179.109863f, 143.891006f, 3.855950f },
    { 3701.669922f, -5181.859863f, 143.804001f, 1.380370f },
    { 3676.590088f, -5144.899902f, 143.186005f, 3.575550f },    // 6
    { 3670.649902f, -5142.629883f, 143.173996f, 4.313820f },
    { 3665.689941f, -5136.279785f, 143.177994f, 1.176150f },
    { 3664.870117f, -5132.330078f, 143.205002f, 3.249600f },
    { 3662.120117f, -5144.299805f, 143.320007f, 2.256080f },
    { 3658.979980f, -5139.490234f, 143.421997f, 6.077040f },
    { 3651.850098f, -5137.140137f, 143.764999f, 0.799161f },
    { 3654.689941f, -5144.009766f, 143.546997f, 2.519180f },
    { 3666.020020f, -5149.500000f, 143.587006f, 3.359560f },
    { 3667.959961f, -5153.120117f, 143.791000f, 4.015380f },
    { 3659.979980f, -5100.450195f, 143.453003f, 5.168254f },    // 7
    { 3662.800049f, -5088.189941f, 143.557999f, 4.409650f },
    { 3658.439941f, -5090.649902f, 143.470001f, 5.528840f },
    { 3652.810059f, -5090.450195f, 143.475998f, 1.362310f },
    { 3651.780029f, -5095.339844f, 143.388000f, 0.981387f },
    { 3652.629883f, -5098.970215f, 143.544998f, 2.799580f },
    { 3652.679932f, -5083.479980f, 143.774994f, 0.482659f },
    { 3647.189941f, -5085.490234f, 143.570999f, 1.919940f },
    { 3645.120117f, -5097.240234f, 143.487000f, 0.765403f },
    { 3646.360107f, -5101.200195f, 143.681000f, 2.909540f }
};

static Movement::Location Abomination[] =    // Unstoppable Abomination
{
    { 3776.229980f, -5081.439941f, 143.779999f, 4.043730f },    // 1
    { 3774.419922f, -5071.490234f, 143.423996f, 4.214940f },
    { 3759.850098f, -5064.479980f, 143.636002f, 6.255410f },
    { 3740.899902f, -5052.740234f, 143.785995f, 1.836760f },    // 2
    { 3726.919922f, -5040.020020f, 143.535995f, 2.481570f },
    { 3715.010010f, -5049.259766f, 143.632004f, 5.071810f },
    { 3695.060059f, -5052.160156f, 143.548004f, 3.792400f },    // 3
    { 3678.129883f, -5048.060059f, 143.436996f, 5.363980f },
    { 3676.120117f, -5061.359863f, 143.492004f, 5.726840f },
    { 3756.780029f, -5149.419922f, 143.460007f, 2.774530f },    // 4
    { 3752.262695f, -5164.782227f, 143.409119f, 0.383768f },
    { 3740.669922f, -5168.600098f, 143.873993f, 1.051360f },
    { 3714.020020f, -5171.129883f, 143.776993f, 1.742510f },    // 5
    { 3702.185303f, -5174.303711f, 143.532303f, 5.443298f },
    { 3693.739990f, -5162.149902f, 143.748001f, 5.696990f },
    { 3673.189941f, -5150.500000f, 143.751999f, 3.030570f },    // 6
    { 3658.570068f, -5147.799805f, 143.494003f, 1.230440f },
    { 3659.560059f, -5132.129883f, 143.677002f, 5.988380f },
    { 3651.130859f, -5104.800293f, 143.798248f, 5.374194f },    // 7
    { 3646.947021f, -5092.266113f, 143.305878f, 5.005841f },
    { 3658.618408f, -5083.832031f, 143.778641f, 5.951464f }
};

static Movement::Location SoulWeaver[] =    // Soul Weaver
{
    { 3768.540039f, -5075.140137f, 143.203995f, 5.096160f },
    { 3728.030029f, -5047.359863f, 143.306000f, 5.230460f },
    { 3682.929932f, -5055.819824f, 143.184006f, 5.368690f },
    { 3749.429932f, -5160.419922f, 143.283997f, 4.723090f },
    { 3706.120117f, -5169.250000f, 143.436996f, 2.682630f },
    { 3665.310059f, -5142.339844f, 143.220001f, 1.147180f },
    { 3656.365234f, -5094.724121f, 143.306641f, 6.203571f }
};

static Movement::Location Guardians[] =        // Guardians of Icecrown
{
    { 3778.371582f, -5065.141113f, 143.614639f, 3.700061f },
    { 3731.733398f, -5032.681152f, 143.775040f, 4.485459f },
    { 3758.592285f, -5170.157715f, 143.667297f, 2.144972f },
    { 3700.936279f, -5183.230469f, 143.858582f, 1.314648f }
};

static Movement::Location Waves[] =            // Spawn positions of units that attack circle
{
    { 3756.380615f, -5080.560059f, 142.906921f, 3.762599f },
    { 3726.448242f, -5058.546387f, 142.467331f, 4.262112f },
    { 3690.084229f, -5066.993164f, 142.705917f, 5.245427f },
    { 3742.711670f, -5146.786133f, 142.964890f, 2.178441f },
    { 3706.024902f, -5155.362793f, 142.655304f, 1.294868f },
    { 3676.363281f, -5133.007324f, 142.806168f, 0.615499f },
    { 3668.310303f, -5096.927246f, 142.307312f, 6.128994f }
};

// Kel'thuzad AI
// each ~10-20 sec new mob

const uint32 CN_KELTHUZAD = 15990;

const uint32 SFROSTBOLT = 28478;
const uint32 MFROSTBOLT = 28479;
const uint32 CHAINS_OF_KELTHUZAD = 28410;
const uint32 DETONATE_MANA = 27819;
const uint32 SHADOW_FISSURE = 27810;
const uint32 FROST_BLAST = 27808;

const uint32 KELTHUZAD_CHANNEL = 29423;

class KelthuzadAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(KelthuzadAI);
    SP_AI_Spell spells[7];
    bool m_spellcheck[7];
    bool FrozenWastes[7];
    bool Abominations[7];
    bool SoulWeavers[7];

    KelthuzadAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        nrspells = 6;

        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
            spells[i].casttime = 0;
        }

        for (uint8 i = 0; i < 7; i++)
        {
            FrozenWastes[i] = false;
            Abominations[i] = false;
            SoulWeavers[i] = false;
        }

        spells[0].info = sSpellCustomizations.GetSpellInfo(SFROSTBOLT);
        spells[0].targettype = TARGET_ATTACKING;
        spells[0].instant = false;
        spells[0].perctrigger = 7.0f;
        spells[0].cooldown = 0;
        spells[0].attackstoptimer = 2000;

        spells[1].info = sSpellCustomizations.GetSpellInfo(MFROSTBOLT);
        spells[1].targettype = TARGET_VARIOUS;
        spells[1].instant = true;
        spells[1].perctrigger = 0.1f;
        spells[1].cooldown = 15;            // it's casted after 15 sec anyway, so it does need additional perctrigger
        spells[1].attackstoptimer = 1000;

        spells[2].info = sSpellCustomizations.GetSpellInfo(CHAINS_OF_KELTHUZAD);
        spells[2].targettype = TARGET_RANDOM_SINGLE;
        spells[2].instant = true;
        spells[2].perctrigger = 5.0f;
        spells[2].cooldown = 20;
        spells[2].attackstoptimer = 1000;
        spells[2].mindist2cast = 0.0f;
        spells[2].maxdist2cast = 40.0f;
        spells[2].minhp2cast = 0;
        spells[2].maxhp2cast = 100;

        spells[3].info = sSpellCustomizations.GetSpellInfo(DETONATE_MANA);
        spells[3].targettype = TARGET_RANDOM_SINGLE;
        spells[3].instant = true;
        spells[3].perctrigger = 6.0f;
        spells[3].cooldown = 7;
        spells[3].attackstoptimer = 2000;
        spells[3].mindist2cast = 0.0f;
        spells[3].maxdist2cast = 40.0f;
        spells[3].minhp2cast = 0;
        spells[3].maxhp2cast = 100;

        spells[4].info = sSpellCustomizations.GetSpellInfo(SHADOW_FISSURE);
        spells[4].targettype = TARGET_ATTACKING;
        spells[4].instant = true;
        spells[4].perctrigger = 5.0f;
        spells[4].cooldown = 10;
        spells[4].attackstoptimer = 2000;

        spells[5].info = sSpellCustomizations.GetSpellInfo(FROST_BLAST);
        spells[5].targettype = TARGET_RANDOM_SINGLE;
        spells[5].instant = true;
        spells[5].perctrigger = 6.0f;
        spells[5].cooldown = 10;
        spells[5].attackstoptimer = 2000;
        spells[5].soundid = 8815;
        spells[5].speech = "I shall freeze the blood in your veins!";    // not sure if it's to this one or to one of bolt spells
        spells[5].mindist2cast = 0.0f;
        spells[5].maxdist2cast = 40.0f;
        spells[5].minhp2cast = 0;
        spells[5].maxhp2cast = 100;

        spells[6].info = sSpellCustomizations.GetSpellInfo(KELTHUZAD_CHANNEL);
        spells[6].targettype = TARGET_SELF;
        spells[6].instant = false;
        spells[6].perctrigger = 0.0f;
        spells[6].cooldown = 0;
        spells[6].attackstoptimer = 1000;

        _setMeleeDisabled(false);
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);
        getCreature()->GetAIInterface()->m_canMove = true;

        DespawnTrash = false;
        EventStart = false;
        SpawnCounter = 0;
        PhaseTimer = 310;
        SpawnTimer = 0;
        GCounter = 0;
        m_phase = 0;
        HelpDialog = 0;
        WaveTimer = 0;
    }

    void OnCombatStart(Unit* mTarget)
    {
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Minions, servants, soldiers of the cold dark, obey the call of Kel'Thuzad!");
        getCreature()->PlaySoundToSet(8819);

        Unit* TheLichKing = NULL;
        TheLichKing = getNearestCreature(3767.58f, -5117.15f, 174.49f, CN_THE_LICH_KING);
        if (TheLichKing != NULL)
        {
            getCreature()->SetChannelSpellTargetGUID(TheLichKing->GetGUID());
            getCreature()->SetChannelSpellId(29423);
        }

        GameObject* KelGate = getNearestGameObject(3635.44f, -5090.33f, 143.205f, 181228);

        if (KelGate)
            KelGate->SetState(GO_STATE_CLOSED);

        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        _setMeleeDisabled(true);
        getCreature()->GetAIInterface()->m_canMove = false;

        RegisterAIUpdateEvent(1000);
        CastTime();

        DespawnTrash = false;
        EventStart = true;
        SpawnCounter = 0;
        PhaseTimer = 310;
        SpawnTimer = 0;
        GCounter = 0;
        m_phase = 1;
    }

    void OnCombatStop(Unit* mTarget)
    {
        GameObject* KelGate = getNearestGameObject(3635.44f, -5090.33f, 143.205f, 181228);
        if (KelGate != NULL)
            KelGate->SetState(GO_STATE_OPEN);

        for (uint8 i = 0; i < 4; i++)
        {
            GameObject* WindowGate = getNearestGameObject(Guardians[i].x, Guardians[i].y, Guardians[i].z, 200002);
            if (WindowGate != NULL)
                WindowGate->SetState(GO_STATE_CLOSED);
        }

        getCreature()->SetChannelSpellTargetGUID(0);
        getCreature()->SetChannelSpellId(0);
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        _setMeleeDisabled(false);
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);
        getCreature()->GetAIInterface()->m_canMove = true;
        RemoveAIUpdateEvent();

        DespawnTrash = true;
        EventStart = false;
        SpawnCounter = 0;
        PhaseTimer = 310;
        SpawnTimer = 0;
        GCounter = 0;
        m_phase = 0;
    }

    void CastTime()
    {
        for (uint8 i = 0; i < nrspells; i++)
            spells[i].casttime = 0;
    }

    void OnTargetDied(Unit* mTarget)
    {
        if (getCreature()->GetHealthPct() == 0)
            return;

        switch (RandomUInt(1))
        {
            case 0:
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "The dark void awaits you!");
                getCreature()->PlaySoundToSet(8817);
                break;
            case 1:
                //_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "");    // no text?
                getCreature()->PlaySoundToSet(8818);
                break;
        }
    }

    void OnDied(Unit* mKiller)
    {
        GameObject* KelGate = getNearestGameObject(3635.44f, -5090.33f, 143.205f, 181228);
        if (KelGate != NULL)
            KelGate->SetState(GO_STATE_OPEN);

        for (uint8 i = 0; i < 4; i++)
        {
            GameObject* WindowGate = getNearestGameObject(Guardians[i].x, Guardians[i].y, Guardians[i].z, 200002);
            if (WindowGate != NULL)
                WindowGate->SetState(GO_STATE_CLOSED);
        }

        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Do not rejoice... your victory is a hollow one... for I shall return with powers beyond your imagining!");
        getCreature()->PlaySoundToSet(8814);

        _setMeleeDisabled(false);
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);
        getCreature()->GetAIInterface()->m_canMove = true;

        EventStart = false;
        SpawnCounter = 0;
        PhaseTimer = 310;
        SpawnTimer = 0;
        GCounter = 0;
        m_phase = 0;
    }

    void AIUpdate()
    {
        switch (m_phase)
        {
            case 1:
                PhaseOne();
                break;
            case 2:
                PhaseTwo();
                break;
            case 3:
                PhaseThree();
                break;
            default:
            {
            }
        }
    }

    void PhaseOne()
    {
        if (EventStart == true)
        {
            SpawnTimer++;
            if (SpawnTimer == 5 || SpawnTimer == 10)
            {
                for (uint8 i = 0; i < 7; i++)
                {
                    int Counter = 0;

                    while (Counter == 0)
                    {
                        if (FrozenWastes[i] == false && (RandomUInt(3) == 0 || SpawnCounter > 0))
                        {
                            for (uint8 x = 0; x < 10; x++)
                            {
                                uint32 SpawnID = 10 * i + x;
                                spawnCreature(CN_SOLDIER_OF_THE_FROZEN_WASTES, SFrozenWastes[SpawnID].x, SFrozenWastes[SpawnID].y, SFrozenWastes[SpawnID].z, SFrozenWastes[SpawnID].o);
                            }

                            FrozenWastes[i] = true;
                            Counter++;
                        }

                        if (Abominations[i] == false && (RandomUInt(3) == 0 || SpawnCounter > 0))
                        {
                            for (uint8 x = 0; x < 3; x++)
                            {
                                uint32 SpawnID = 3 * i + x;
                                spawnCreature(CN_UNSTOPPABLE_ABOMINATION, Abomination[SpawnID].x, Abomination[SpawnID].y, Abomination[SpawnID].z, Abomination[SpawnID].o);
                            }

                            Abominations[i] = true;
                            Counter++;
                        }

                        if (SoulWeavers[i] == false && ((RandomUInt(3) == 0 && Counter < 2) || Counter == 0 || SpawnCounter > 0))
                        {
                            uint32 SpawnID = i;
                            spawnCreature(CN_SOUL_WEAVER, SoulWeaver[SpawnID].x, SoulWeaver[SpawnID].y, SoulWeaver[SpawnID].z, SoulWeaver[SpawnID].o);

                            SoulWeavers[i] = true;
                            Counter++;
                        }

                        if (SoulWeavers[i] == true && Abominations[i] == true && FrozenWastes[i] == true)
                            Counter = 1;
                    }
                }

                SpawnCounter++;
            }

            if (SpawnCounter == 2)
            {
                for (uint8 i = 0; i < 7; i++)
                {
                    FrozenWastes[i] = false;
                    Abominations[i] = false;
                    SoulWeavers[i] = false;
                }

                WaveTimer = RandomUInt(10, 16);
                EventStart = false;
                SpawnCounter = 0;
                PhaseTimer = 310;
                SpawnTimer = 0;
            }
        }

        else
        {
            PhaseTimer--;
            WaveTimer--;

            if (PhaseTimer == 5)
            {
                switch (RandomUInt(2))
                {
                    case 0:
                        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Pray for mercy!");
                        getCreature()->PlaySoundToSet(8809);
                        break;
                    case 1:
                        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Scream your dying breath!");
                        getCreature()->PlaySoundToSet(8810);
                        break;
                    case 2:
                        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "The end is upon you!");
                        getCreature()->PlaySoundToSet(8811);
                        break;
                }
            }

            if (PhaseTimer == 3)
                DespawnTrash = true;

            if (!PhaseTimer)
            {
                getCreature()->SetChannelSpellTargetGUID(0);
                getCreature()->SetChannelSpellId(0);
                _setMeleeDisabled(false);
                getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);
                getCreature()->GetAIInterface()->m_canMove = true;

                DespawnTrash = false;
                HelpDialog = 0;
                GCounter = 0;
                m_phase = 2;
                return;
            }

            else if (!WaveTimer && PhaseTimer > 5)
            {
                uint32 SpawnPoint = RandomUInt(6);
                uint32 RandomSU = 0;
                if (PhaseTimer > 250)
                    RandomSU = RandomUInt(4);
                if (PhaseTimer <= 250 && PhaseTimer >= 150)
                    RandomSU = RandomUInt(5);
                if (PhaseTimer <= 150 && PhaseTimer > 100)
                    RandomSU = RandomUInt(6);
                if (PhaseTimer <= 100)
                    RandomSU = RandomUInt(7);

                uint32 UnitType;

                switch (RandomSU)
                {
                    case 0:
                    case 1:
                        UnitType = CN_SOLDIER_OF_THE_FROZEN_WASTES;
                        break;
                    case 2:
                    case 4:
                        UnitType = CN_SOUL_WEAVER;
                        break;
                    case 3:
                    case 5:
                    case 6:
                        UnitType = CN_UNSTOPPABLE_ABOMINATION;
                        break;
                    default:
                    {
                        UnitType = CN_UNSTOPPABLE_ABOMINATION;
                    }
                }

                spawnCreature(UnitType, Waves[SpawnPoint].x, Waves[SpawnPoint].y, Waves[SpawnPoint].z, Waves[SpawnPoint].o);
                WaveTimer = RandomUInt(11, 20);
            }
        }
    }

    void PhaseTwo()
    {
        if (getCreature()->GetHealthPct() <= 40)
        {
            HelpDialog++;
            if (HelpDialog == 1)
            {
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Master! I require aid!");
                getCreature()->PlaySoundToSet(8816);
            }

            if (HelpDialog == 4)
            {
                Unit* TheLichKing = NULL;
                TheLichKing = getNearestCreature(3767.58f, -5117.15f, 174.49f, CN_THE_LICH_KING);
                if (TheLichKing != NULL)
                {
                    TheLichKing->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Very well... warriors of the frozen wastes, rise up, I command you to fight, kill, and die for your master. Let none survive...");
                    TheLichKing->PlaySoundToSet(8824);
                }

                else
                {
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Very well... warriors of the frozen wastes, rise up, I command you to fight, kill, and die for your master. Let none survive...");
                    getCreature()->PlaySoundToSet(8824);
                }

                for (uint8 i = 0; i < 4; i++)
                {
                    GameObject* WindowGate = getNearestGameObject(Guardians[i].x, Guardians[i].y, Guardians[i].z, 200002);
                    if (WindowGate)
                        WindowGate->SetState(GO_STATE_OPEN);
                }
            }

            if (HelpDialog == 10 || HelpDialog == 12 || HelpDialog == 14 || HelpDialog == 16 || HelpDialog == 18)
            {
                Unit* Guardian = NULL;
                uint32 i = RandomUInt(3);
                Guardian = spawnCreature(CN_GUARDIAN_OF_ICECROWN, Guardians[i].x, Guardians[i].y, Guardians[i].z, Guardians[i].o);
                if (Guardian != NULL)
                {
                    if (Guardian->GetAIInterface()->getNextTarget() != NULL)
                        Guardian->GetAIInterface()->AttackReaction(Guardian->GetAIInterface()->getNextTarget(), 1, 0);
                }

                GCounter++;
                if (GCounter == 5)
                {
                    GCounter = 0;
                    m_phase = 3;
                }
            }
        }

        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void PhaseThree()
    {
        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();
                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_RANDOM_FRIEND:
                        case TARGET_RANDOM_SINGLE:
                        case TARGET_RANDOM_DESTINATION:
                            CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                            break;
                    }

                    if (i == 2)
                        ChainSound();

                    if (spells[i].speech != "")
                    {
                        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                        getCreature()->PlaySoundToSet(spells[i].soundid);
                    }

                    m_spellcheck[i] = false;
                    return;
                }

                uint32 t = (uint32)time(NULL);
                if (((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || i == 1) && t > spells[i].casttime)
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    spells[i].casttime = t + spells[i].cooldown;
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
    {
        if (!maxdist2cast) maxdist2cast = 100.0f;
        if (!maxhp2cast) maxhp2cast = 100;

        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            std::vector<Unit*> TargetTable;
            for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
            {
                if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(getCreature(), (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(getCreature(), (*itr)) && (*itr) != getCreature())) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                {
                    Unit* RandomTarget = NULL;
                    RandomTarget = static_cast<Unit*>(*itr);

                    if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && getCreature()->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (getCreature()->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(getCreature(), RandomTarget))))
                    {
                        TargetTable.push_back(RandomTarget);
                    }
                }
            }

            if (getCreature()->GetHealthPct() >= minhp2cast && getCreature()->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                TargetTable.push_back(getCreature());

            if (!TargetTable.size())
                return;

            auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
            auto random_target = TargetTable[random_index];

            if (random_target == nullptr)
                return;

            switch (spells[i].targettype)
            {
                case TARGET_RANDOM_FRIEND:
                case TARGET_RANDOM_SINGLE:
                    getCreature()->CastSpell(random_target, spells[i].info, spells[i].instant);
                    break;
                case TARGET_RANDOM_DESTINATION:
                    getCreature()->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                    break;
            }

            TargetTable.clear();
        }
    }

    void ChainSound()
    {
        switch (RandomUInt(1))
        {
            case 0:
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Your soul is bound to me now!");
                getCreature()->PlaySoundToSet(8812);
                break;
            case 1:
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "There will be no escape!");
                getCreature()->PlaySoundToSet(8813);
                break;
        }
    }

    bool GetDespawnTrash()
    {
        return DespawnTrash;
    }

    protected:

    bool DespawnTrash, EventStart;
    uint32 SpawnCounter;
    uint32 HelpDialog;
    uint32 SpawnTimer;
    uint32 PhaseTimer;
    uint32 WaveTimer;
    uint32 GCounter;
    uint32 m_phase;
    uint8 nrspells;
};

// The Lich KingAI

class TheLichKingAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(TheLichKingAI);

    TheLichKingAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }
};

// Soldier of the Frozen WastesAI

const uint32 DARK_BLAST = 28457; // 28458

class SoldierOfTheFrozenWastesAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(SoldierOfTheFrozenWastesAI);
    SoldierOfTheFrozenWastesAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->m_noRespawn = true;

        OnStart = false;

        LastPosX = 0;
        LastPosY = 0;
        LastPosZ = 0;

        newposx = 0;
        newposy = 0;

        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStart(Unit* mTarget)
    {
        LastPosX = getCreature()->GetPositionX();
        LastPosY = getCreature()->GetPositionY();
        LastPosZ = getCreature()->GetPositionZ();
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
    }

    void AIUpdate()
    {
        Creature* Kelthuzad = NULL;
        Kelthuzad = getNearestCreature(3749.950195f, -5113.451660f, 141.288635f, 15990);
        if (Kelthuzad == NULL || !Kelthuzad->isAlive())
        {
            getCreature()->Despawn(0, 0);
            return;
        }
        else if (Kelthuzad->GetScript())
        {
            CreatureAIScript* pScript = Kelthuzad->GetScript();
            if (static_cast< KelthuzadAI* >(pScript)->GetDespawnTrash())
            {
                getCreature()->Despawn(0, 0);
                return;
            }
        }
        if (getCreature()->GetPositionX() == LastPosX && getCreature()->GetPositionY() == LastPosY && getCreature()->GetPositionZ() == LastPosZ)
        {
            getCreature()->GetAIInterface()->MoveTo(newposx, newposy, 141.290451f);
        }
        if (OnStart == false)
        {
            for (uint8 i = 0; i < 7; i++)
            {
                if (getCreature()->GetPositionX() == Waves[i].x && getCreature()->GetPositionY() == Waves[i].y && getCreature()->GetPositionZ() == Waves[i].z)
                {
                    float xchange = RandomFloat(10.0f);
                    float distance = 10.0f;

                    float ychange = sqrt(distance * distance - xchange * xchange);

                    if (RandomUInt(1) == 1)
                        xchange *= -1;
                    if (RandomUInt(1) == 1)
                        ychange *= -1;

                    newposx = 3715.845703f + xchange;
                    newposy = -5106.928223f + ychange;

                    getCreature()->GetAIInterface()->MoveTo(newposx, newposy, 141.290451f);
                }
            }

            OnStart = true;
        }

        if (getCreature()->GetAIInterface()->getNextTarget() != NULL)
        {
            Unit* target = getCreature()->GetAIInterface()->getNextTarget();
            if (getCreature()->GetDistance2dSq(target) <= 49.0f)
                getCreature()->CastSpell(getCreature(), DARK_BLAST, true);
        }
    }

    protected:

    float LastPosX, LastPosY, LastPosZ;
    float newposx;
    float newposy;
    bool OnStart;
};

// Unstoppable Abomination AI

const uint32 UA_MORTAL_WOUND = 25646;    // 36814

class UnstoppableAbominationAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(UnstoppableAbominationAI);
    SP_AI_Spell spells[1];
    bool m_spellcheck[1];

    UnstoppableAbominationAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        nrspells = 1;
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
            spells[i].casttime = 0;
        }

        spells[0].info = sSpellCustomizations.GetSpellInfo(UA_MORTAL_WOUND);
        spells[0].targettype = TARGET_ATTACKING;
        spells[0].instant = true;
        spells[0].cooldown = 10;
        spells[0].perctrigger = 15.0f;
        spells[0].attackstoptimer = 1000;

        getCreature()->m_noRespawn = true;

        OnStart = false;

        LastPosX = 0;
        LastPosY = 0;
        LastPosZ = 0;
        newposx = 0;
        newposy = 0;

        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStart(Unit* mTarget)
    {
        LastPosX = getCreature()->GetPositionX();
        LastPosY = getCreature()->GetPositionY();
        LastPosZ = getCreature()->GetPositionZ();

        for (uint8 i = 0; i < nrspells; i++)
            spells[i].casttime = 0;
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
    }

    void AIUpdate()
    {
        Creature* Kelthuzad = NULL;
        Kelthuzad = getNearestCreature(3749.950195f, -5113.451660f, 141.288635f, 15990);
        if (Kelthuzad == NULL || !Kelthuzad->isAlive())
        {
            getCreature()->Despawn(0, 0);
            return;
        }
        else if (Kelthuzad->GetScript())
        {
            CreatureAIScript* pScript = Kelthuzad->GetScript();
            if (static_cast< KelthuzadAI* >(pScript)->GetDespawnTrash())
            {
                getCreature()->Despawn(0, 0);
                return;
            }
        }
        if (getCreature()->GetPositionX() == LastPosX && getCreature()->GetPositionY() == LastPosY && getCreature()->GetPositionZ() == LastPosZ)
        {
            getCreature()->GetAIInterface()->MoveTo(newposx, newposy, 141.290451f);
        }
        if (OnStart == false)
        {
            for (uint8 i = 0; i < 7; i++)
            {
                if (getCreature()->GetPositionX() == Waves[i].x && getCreature()->GetPositionY() == Waves[i].y && getCreature()->GetPositionZ() == Waves[i].z)
                {
                    float xchange = RandomFloat(10.0f);
                    float distance = 10.0f;

                    float ychange = sqrt(distance * distance - xchange * xchange);

                    if (RandomUInt(1) == 1)
                        xchange *= -1;
                    if (RandomUInt(1) == 1)
                        ychange *= -1;

                    newposx = 3715.845703f + xchange;
                    newposy = -5106.928223f + ychange;

                    getCreature()->GetAIInterface()->MoveTo(newposx, newposy, 141.290451f);
                }
            }

            OnStart = true;
        }

        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();

                    if (getCreature()->GetDistance2dSq(target) > 25.0f)
                        return;

                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                    }
                    m_spellcheck[i] = false;
                    return;
                }

                uint32 t = (uint32)time(NULL);
                if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    spells[i].casttime = t + spells[i].cooldown;
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    protected:

    float LastPosX, LastPosY, LastPosZ;
    float newposx;
    float newposy;
    bool OnStart;
    uint8 nrspells;
};

// Soul Weaver AI

const uint32 WAIL_OF_SOULS = 28459;

class SoulWeaverAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(SoulWeaverAI);
    SP_AI_Spell spells[1];
    bool m_spellcheck[1];

    SoulWeaverAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        nrspells = 1;
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
            spells[i].casttime = 0;
        }

        spells[0].info = sSpellCustomizations.GetSpellInfo(WAIL_OF_SOULS);
        spells[0].targettype = TARGET_VARIOUS;
        spells[0].instant = true;
        spells[0].cooldown = 10;
        spells[0].perctrigger = 15.0f;
        spells[0].attackstoptimer = 1000;

        getCreature()->m_noRespawn = true;

        OnStart = false;

        LastPosX = 0;
        LastPosY = 0;
        LastPosZ = 0;
        newposx = 0;
        newposy = 0;

        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStart(Unit* mTarget)
    {
        LastPosX = getCreature()->GetPositionX();
        LastPosY = getCreature()->GetPositionY();
        LastPosZ = getCreature()->GetPositionZ();

        //RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));

        for (uint8 i = 0; i < nrspells; i++)
            spells[i].casttime = 0;
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
    }

    void AIUpdate()
    {
        Creature* Kelthuzad = NULL;
        Kelthuzad = getNearestCreature(3749.950195f, -5113.451660f, 141.288635f, 15990);
        if (Kelthuzad == NULL || !Kelthuzad->isAlive())
        {
            getCreature()->Despawn(0, 0);
            return;
        }
        else if (Kelthuzad->GetScript())
        {
            CreatureAIScript* pScript = Kelthuzad->GetScript();
            if (static_cast< KelthuzadAI* >(pScript)->GetDespawnTrash())
            {
                getCreature()->Despawn(0, 0);
                return;
            }
        }
        if (getCreature()->GetPositionX() == LastPosX && getCreature()->GetPositionY() == LastPosY && getCreature()->GetPositionZ() == LastPosZ)
        {
            getCreature()->GetAIInterface()->MoveTo(newposx, newposy, 141.290451f);
        }
        if (OnStart == false)
        {
            for (uint8 i = 0; i < 7; i++)
            {
                if (getCreature()->GetPositionX() == Waves[i].x && getCreature()->GetPositionY() == Waves[i].y && getCreature()->GetPositionZ() == Waves[i].z)
                {
                    float xchange = RandomFloat(10.0f);
                    float distance = 10.0f;

                    float ychange = sqrt(distance * distance - xchange * xchange);

                    if (RandomUInt(1) == 1)
                        xchange *= -1;
                    if (RandomUInt(1) == 1)
                        ychange *= -1;

                    newposx = 3715.845703f + xchange;
                    newposy = -5106.928223f + ychange;

                    getCreature()->GetAIInterface()->MoveTo(newposx, newposy, 141.290451f);
                }
            }

            OnStart = true;
        }

        float val = RandomFloat(100.0f);
        SpellCast(val);
    }

    void SpellCast(float val)
    {
        if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())
        {
            float comulativeperc = 0;
            Unit* target = NULL;
            for (uint8 i = 0; i < nrspells; i++)
            {
                if (!spells[i].perctrigger) continue;

                if (m_spellcheck[i])
                {
                    target = getCreature()->GetAIInterface()->getNextTarget();

                    if (getCreature()->GetDistance2dSq(target) > 64.0f) // 8yards
                        return;

                    switch (spells[i].targettype)
                    {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                            break;
                    }
                    m_spellcheck[i] = false;
                    return;
                }

                uint32 t = (uint32)time(NULL);
                if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                {
                    getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                    spells[i].casttime = t + spells[i].cooldown;
                    m_spellcheck[i] = true;
                }
                comulativeperc += spells[i].perctrigger;
            }
        }
    }

    protected:

    float LastPosX, LastPosY, LastPosZ;
    float newposx;
    float newposy;
    bool OnStart;
    uint8 nrspells;
};

// Guardian of Icecrown AI

const uint32 BLOOD_TAP = 28459;

class GuardianOfIcecrownAI : public CreatureAIScript
{
    public:
    ADD_CREATURE_FACTORY_FUNCTION(GuardianOfIcecrownAI);
    SP_AI_Spell spells[1];
    bool m_spellcheck[1];

    GuardianOfIcecrownAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        nrspells = 1;
        for (uint8 i = 0; i < nrspells; i++)
        {
            m_spellcheck[i] = false;
        }

        spells[0].info = sSpellCustomizations.GetSpellInfo(BLOOD_TAP);
        spells[0].targettype = TARGET_SELF;
        spells[0].instant = true;
        spells[0].cooldown = 0;
        spells[0].perctrigger = 0.0f;
        spells[0].attackstoptimer = 1000;

        getCreature()->GetAIInterface()->setSplineRun();
        getCreature()->m_noRespawn = true;

        OnStart = false;

        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));

        LastPosX = 0;
        LastPosY = 0;
        LastPosZ = 0;
        newposx = 0;
        newposy = 0;
        LastTarget = NULL;
    }

    void OnCombatStart(Unit* mTarget)
    {
        if (getCreature()->GetAIInterface()->getNextTarget())
            LastTarget = getCreature()->GetAIInterface()->getNextTarget();

        LastPosX = getCreature()->GetPositionX();
        LastPosY = getCreature()->GetPositionY();
        LastPosZ = getCreature()->GetPositionZ();
    }

    void OnCombatStop(Unit* mTarget)
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
    }

    void AIUpdate()
    {
        Unit* Kelthuzad = NULL;
        Kelthuzad = getNearestCreature(3715.950195f, -5106.451660f, 141.288635f, 15990);
        if (Kelthuzad && !Kelthuzad->isAlive())
        {
            getCreature()->Despawn(0, 0);
            return;
        }
        if (getCreature()->GetPositionX() == LastPosX && getCreature()->GetPositionY() == LastPosY && getCreature()->GetPositionZ() == LastPosZ)
        {
            getCreature()->GetAIInterface()->MoveTo(newposx, newposy, 141.290451f);
        }
        if (OnStart == false)
        {
            for (uint8 i = 0; i < 4; i++)
            {
                if (getCreature()->GetPositionX() == Guardians[i].x && getCreature()->GetPositionY() == Guardians[i].y && getCreature()->GetPositionZ() == Guardians[i].z)
                {
                    float xchange = RandomFloat(10.0f);
                    float distance = 10.0f;

                    float ychange = sqrt(distance * distance - xchange * xchange);

                    if (RandomUInt(1) == 1)
                        xchange *= -1;
                    if (RandomUInt(1) == 1)
                        ychange *= -1;

                    newposx = 3715.845703f + xchange;
                    newposy = -5106.928223f + ychange;

                    getCreature()->GetAIInterface()->MoveTo(newposx, newposy, 141.290451f);
                }
            }

            OnStart = true;
        }

        if (getCreature()->GetAIInterface()->getNextTarget())
        {
            Unit* target = NULL;
            target = getCreature()->GetAIInterface()->getNextTarget();

            if (!LastTarget) { LastTarget = target; return; }

            if (LastTarget != target)
                getCreature()->CastSpell(getCreature(), spells[0].info, spells[0].instant);

            LastTarget = target;
        }
    }

    protected:

    float LastPosX, LastPosY, LastPosZ;
    Unit* LastTarget;
    float newposx;
    float newposy;
    bool OnStart;
    uint8 nrspells;
};

void SetupNaxxramas(ScriptMgr* pScriptMgr);
