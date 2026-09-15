/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/QuestLogEntry.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Movement/MovementManager.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>

//////////////////////////////////////////////////////////////////////////////////////////
// Explosive Sheep (Summoned by ItemID: 4384)
class ExplosiveSheep : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ExplosiveSheep(c); }
    explicit ExplosiveSheep(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->Despawn(180000, 0); // "Lasts for 3 minutes or until it explodes."
    }

    void OnCombatStart(Unit* mTarget) override
    // Summons an Explosive Sheep which will charge at a nearby enemy and explode for 135 - 165 damage.
    {
        const auto pos = mTarget->GetPosition();
        getCreature()->getMovementManager()->moveCharge(pos);
        getCreature()->castSpell(getCreature(), 4050, true);
        getCreature()->Despawn(1000, 0); // Despawn since we "exploded"
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Crimson Hammersmith
class CrimsonHammersmith : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CrimsonHammersmith(c); }
    explicit CrimsonHammersmith(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Who Dares Disturb Me");
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Corrupt Minor Manifestation Water Dead
class Corrupt_Minor_Manifestation_Water_Dead : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Corrupt_Minor_Manifestation_Water_Dead(c); }
    explicit Corrupt_Minor_Manifestation_Water_Dead(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* /*mKiller*/) override
    {
        float SSX = getCreature()->GetPositionX();
        float SSY = getCreature()->GetPositionY();
        float SSZ = getCreature()->GetPositionZ();
        float SSO = getCreature()->GetOrientation();

        Creature* NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(5895, LocationVector(SSX, SSY + 1, SSZ, SSO));
        if(NewCreature != NULL)
            NewCreature->Despawn(600000, 0);
    }
};

class SavannahProwler : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SavannahProwler(c); }
    explicit SavannahProwler(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        uint32_t chance = Util::getRandomUInt(3);

        if(chance == 1)
            getCreature()->setStandState(STANDSTATE_SLEEP);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        if(getCreature()->getStandState() == STANDSTATE_SLEEP)
            getCreature()->setStandState(STANDSTATE_STAND);
    }
};

class KirithAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KirithAI(c); }
    explicit KirithAI(Creature* pCreature) : CreatureAIScript(pCreature)  {}

    void OnDied(Unit* mKiller) override
    {
        if(mKiller->isPlayer())
        {
            Creature* NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(7729, LocationVector(getCreature()->GetPositionX() + 2, getCreature()->GetPositionY() + 2, getCreature()->GetPositionZ(), getCreature()->GetOrientation()));
            if(NewCreature != NULL)
                NewCreature->Despawn(3 * 6 * 1000, 0);
        }
    }
};

class AllianceGryphon : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AllianceGryphon(c); }
    explicit AllianceGryphon(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* mTarget) override
    {
        if(!mTarget->isPlayer())
            return;

        Creature* NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9526, LocationVector(getCreature()->GetPositionX() + Util::getRandomFloat(5.0f), getCreature()->GetPositionY() + Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()));
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);

        NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9526, LocationVector(getCreature()->GetPositionX() - Util::getRandomFloat(5.0f), getCreature()->GetPositionY() - Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()));
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);
    }
};

class AllianceHippogryph : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AllianceHippogryph(c); }
    explicit AllianceHippogryph(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* mTarget) override
    {
        if(!mTarget->isPlayer())
            return;

        Creature* NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9527, LocationVector(getCreature()->GetPositionX() + Util::getRandomFloat(5.0f), getCreature()->GetPositionY() + Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()));
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);

        NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9527, LocationVector(getCreature()->GetPositionX() - Util::getRandomFloat(5.0f), getCreature()->GetPositionY() - Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()));
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);
    }
};

class HordeWyvern : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HordeWyvern(c); }
    explicit HordeWyvern(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* mTarget) override
    {
        if(!mTarget->isPlayer())
            return;

        Creature* NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9297, LocationVector(getCreature()->GetPositionX() + Util::getRandomFloat(5.0f), getCreature()->GetPositionY() + Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()));
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);

        NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9297, LocationVector(getCreature()->GetPositionX() - Util::getRandomFloat(5.0f), getCreature()->GetPositionY() - Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()));
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);
    }
};

class HordeBat : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HordeBat(c); }
    explicit HordeBat(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* mTarget) override
    {
        if(!mTarget->isPlayer())
            return;

        Creature* NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9521, LocationVector(getCreature()->GetPositionX() + Util::getRandomFloat(5.0f), getCreature()->GetPositionY() + Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()));
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);

        getCreature()->getWorldMap()->getInterface()->spawnCreature(9521, LocationVector(getCreature()->GetPositionX() - Util::getRandomFloat(5.0f), getCreature()->GetPositionY() - Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()));
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);
    }
};

class DragonhawkMasters : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DragonhawkMasters(c); }
    explicit DragonhawkMasters(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        LocationVector vect(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
        for (uint8_t i = 0; i < 2; ++i)
        {
            vect.x += Util::getRandomFloat(2.0f);
            vect.y += Util::getRandomFloat(2.0f);
        }
    }
};

class NeutralMasters : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeutralMasters(c); }
    explicit NeutralMasters(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        LocationVector vect(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
        for (uint8_t i = 0; i < 2; ++i)
        {
            vect.x += Util::getRandomFloat(2.0f);
            vect.y += Util::getRandomFloat(2.0f);
        }
    }
};

class TyrandeWhisperwind : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TyrandeWhisperwind(c); }
    explicit TyrandeWhisperwind(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->PlaySoundToSet(5885);
    }
};

class ProphetVelen : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ProphetVelen(c); }
    explicit ProphetVelen(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->PlaySoundToSet(10155);
    }
};

class KingMagniBronzebeard : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KingMagniBronzebeard(c); }
    explicit KingMagniBronzebeard(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->PlaySoundToSet(5896);
    }
};

class Thrall : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Thrall(c); }
    explicit Thrall(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->PlaySoundToSet(5880);
    }
};

class CairneBloodhoof : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CairneBloodhoof(c); }
    explicit CairneBloodhoof(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->PlaySoundToSet(5884);
    }
};

class LadySylvanasWindrunner : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LadySylvanasWindrunner(c); }
    explicit LadySylvanasWindrunner(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->PlaySoundToSet(5886);
    }
};

class TrollRoofStalker : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TrollRoofStalker(c); }
    explicit TrollRoofStalker(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->castSpell(getCreature(), 30991, true);
    };
};

//////////////////////////////////////////////////////////////////////////////////////////
//D.I.S.C.O AI Script ( entry 27989 )
//"Dancer's Integrated Sonic Celebration Oscillator"
//
//Behavior
//  On spawn it casts 2 spells, which
//  - summon the dancefloor
//  - applies a periodic aura that plays the music
//
//
//////////////////////////////////////////////////////////////////////////////////////////
class DISCO : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DISCO(c); }
    explicit DISCO(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->castSpell(getCreature(), 50487, false);   // summon disco dancefloor
        getCreature()->castSpell(getCreature(), 50314, false);   // play the music
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Silithid Creeper Egg
class SilithidCreeperEgg : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SilithidCreeperEgg(c); }
    explicit SilithidCreeperEgg(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pCreature->getAIInterface()->setAllowedToEnterCombat(false);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        float SSX = getCreature()->GetPositionX();
        float SSY = getCreature()->GetPositionY();
        float SSZ = getCreature()->GetPositionZ();
        float SSO = getCreature()->GetOrientation();

        Creature* SilithidGrub = getCreature()->getWorldMap()->getInterface()->spawnCreature(3251, LocationVector(SSX, SSY + 1, SSZ, SSO));
        if(SilithidGrub != NULL)
            SilithidGrub->Despawn(600000, 0);
    }
};

class DraeneiSurvivor : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DraeneiSurvivor(c); }
    explicit DraeneiSurvivor(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setHealth(getCreature()->getMaxHealth() / 2);
    }
};

class GuardRoberts : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GuardRoberts(c); }
    explicit GuardRoberts(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setHealth(100);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getCreature()->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Argh, the pain. Will it ever leave me?");
    }
};

class SotaAntiPersonnalCannon : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SotaAntiPersonnalCannon(c); }
    explicit SotaAntiPersonnalCannon(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setMoveRoot(true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nestlewood Owlkin - Quest 9303
class NestlewoodOwlkin : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NestlewoodOwlkin(c); }
    explicit NestlewoodOwlkin(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        respawn = false;
        reset = false;
    }

    void AIUpdate() override
    {
        if (!reset)
        {
            if (getCreature()->hasAurasWithId(29528) && !respawn)
            {
                reset = true;
                getCreature()->setMoveRoot(true);
                RemoveAIUpdateEvent();
                GiveKillCredit();
            }
        }
        else
        {
            respawn = true;
            getCreature()->setMoveRoot(false);
            getCreature()->Despawn(0, 10000);   // respawn delay 10 seconds
        }
    }

    void GiveKillCredit()
    {
        if (getCreature()->hasAurasWithId(29528))
        {
            if (auto* player = getCreature()->getWorldMapPlayer(getCreature()->getTargetGuid()))
            {
                if (!player->hasQuestInQuestLog(9303) || player->hasQuestFinished(9303))
                    return;

                if (auto* questLog = player->getQuestLogByQuestId(9303))
                {
                    if (questLog->getMobCountByIndex(0) < 6)
                    {
                        questLog->incrementMobCountForIndex(0);
                        questLog->sendUpdateAddKill(0);
                        questLog->updatePlayerFields();

                        RegisterAIUpdateEvent(240000);  // update after 4 mins
                    }
                }
            }
        }
    }

    void OnLoad() override
    {
        RegisterAIUpdateEvent(4000);
        reset = false;
        respawn = false;
    }

private:
    bool reset;
    bool respawn;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sparring creatures: npcs that fight each other endlessly (guards vs. invaders, mock battles).
// Damage dealt by other creatures never takes them below their health limit, players (and their pets)
// are not limited and can kill them. Once a sparring partner dies the survivor loses its target and evades.
// sorted by entry
constexpr uint32_t sparringCreatureIds[] =
{
    98, 123, 124, 125, 449, 452, 453, 501, 589, 594, 1036, 1726,
    2694, 2952, 2991, 3374, 3375, 4294, 4295, 4298, 4300, 4301, 6190, 6195,
    7275, 7604, 7605, 7606, 7607, 7608, 7787, 7788, 7789, 7796, 7855, 7856,
    7858, 7939, 8876, 8877, 11858, 11910, 11911, 11912, 11913, 33082, 34486, 34511,
    34689, 34696, 34835, 34850, 34876, 34877, 34878, 34884, 34913, 34916, 34957, 34958,
    34959, 35118, 35203, 35204, 35232, 35333, 35334, 35504, 35627, 35839, 35872, 35893,
    35894, 35915, 36057, 36103, 36104, 36115, 36117, 36140, 36211, 36236, 36454, 36455,
    36464, 36488, 36491, 36492, 36602, 36634, 36653, 36671, 36690, 36752, 36795, 36809,
    36810, 36815, 36816, 36830, 36882, 36925, 36942, 36943, 36954, 36988, 37067, 37070,
    37078, 37161, 37170, 37175, 37177, 37178, 37216, 37499, 37580, 37581, 37591, 37592,
    37692, 37699, 37700, 37701, 37716, 37733, 37735, 37784, 37884, 37885, 37914, 37946,
    38022, 38027, 38037, 38038, 38196, 38242, 38243, 38244, 38245, 38246, 38247, 38268,
    38272, 38278, 38279, 38280, 38281, 38282, 38300, 38324, 38326, 38345, 38624, 38646,
    38648, 38649, 38650, 38658, 38661, 38664, 38719, 38823, 38824, 38857, 38896, 38902,
    38913, 38915, 38917, 38926, 38934, 38951, 38952, 38997, 38998, 39028, 39044, 39065,
    39066, 39068, 39128, 39140, 39147, 39153, 39157, 39226, 39294, 39339, 39344, 39390,
    39410, 39411, 39582, 39589, 39591, 39594, 39595, 39596, 39608, 39637, 39641,
    39645, 39650, 39749, 39828, 39867, 39869, 39871, 39874, 39877, 39882, 39895, 39915,
    39916, 39923, 39924, 39925, 39939, 39946, 39947, 39949, 39951, 39963, 39972, 39992,
    39993, 40025, 40065, 40080, 40108, 40123, 40139, 40147, 40148, 40149, 40150,
    40178, 40185, 40227, 40229, 40230, 40238, 40278, 40313, 40360, 40398, 40463,
    40482, 40504, 40535, 40537, 40583, 40590, 40593, 40632, 40635, 40636, 40642, 40643,
    40688, 40689, 40702, 40705, 40709, 40729, 40730, 40731, 40732, 40733, 40734, 40735,
    40736, 40737, 40738, 40739, 40740, 40741, 40742, 40744, 40746, 40755, 40767, 40775,
    40780, 40816, 40833, 40841, 40843, 40882, 40934, 40960, 40972, 40978, 41008, 41015,
    41035, 41046, 41047, 41049, 41050, 41095, 41115, 41138, 41146, 41158, 41175, 41181,
    41182, 41237, 41243, 41256, 41281, 41360, 41367, 41368, 41369, 41370, 41392, 41407,
    41421, 41431, 41439, 41444, 41446, 41447, 41448, 41452, 41455, 41456, 41457, 41509,
    41523, 41524, 41541, 41553, 41554, 41556, 41558, 41559, 41560, 41562, 41586, 41588,
    41589, 41592, 41593, 41601, 41611, 41642, 41763, 41769, 41771, 41772, 41773, 41774,
    41775, 41777, 41779, 41783, 41784, 41793, 41794, 41796, 41797, 41798, 41799, 41800,
    41897, 41898, 41899, 41902, 41909, 41911, 41912, 41914, 41915, 41924, 41980, 41985,
    41995, 41999, 42008, 42010, 42012, 42048, 42049, 42053, 42065, 42066, 42068, 42071,
    42072, 42073, 42074, 42075, 42160, 42284, 42295, 42307, 42320, 42359, 42367, 42369,
    42370, 42397, 42398, 42407, 42516, 42518, 42519, 42536, 42554, 42555, 42561, 42564,
    42566, 42567, 42568, 42571, 42574, 42577, 42608, 42618, 42619, 42653, 42730, 42731,
    42788, 42801, 42806, 42820, 42821, 42847, 42848, 42885, 42946, 42964, 43007, 43016,
    43018, 43026, 43089, 43092, 43100, 43134, 43138, 43153, 43169, 43170, 43174, 43184,
    43213, 43215, 43218, 43228, 43232, 43233, 43234, 43237, 43248, 43249, 43250, 43276,
    43427, 44708, 44751, 49869, 49871, 50471, 50474, 54371, 54372, 54373
};

class SparringCreature : public CreatureAIScript
{
    static constexpr float SPARRING_HEALTH_LIMIT_PCT = 83.33f;

public:
    explicit SparringCreature(Creature* pCreature) : CreatureAIScript(pCreature) {}
    static CreatureAIScript* Create(Creature* c) { return new SparringCreature(c); }

    void DamageTaken(Unit* attacker, uint32_t* damage) override
    {
        if (attacker == nullptr || attacker->getPlayerOwnerOrSelf() != nullptr)
            return;

        const uint32_t minHealth = static_cast<uint32_t>(std::ceil(getCreature()->getMaxHealth() * SPARRING_HEALTH_LIMIT_PCT / 100.0f));
        const uint32_t health = getCreature()->getHealth();
        *damage = health > minHealth ? std::min(*damage, health - minHealth) : 0;
    }
};

void SetupMiscCreatures(ScriptMgr* mgr)
{
    mgr->register_creature_script(16518, &NestlewoodOwlkin::Create);
    mgr->register_creature_script(11120, &CrimsonHammersmith::Create);
    mgr->register_creature_script(5894, &Corrupt_Minor_Manifestation_Water_Dead::Create);
    mgr->register_creature_script(3425, &SavannahProwler::Create);
    mgr->register_creature_script(7728, &KirithAI::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // https://wowwiki.fandom.com/wiki/Flight_master
    //
    //  Alliance
    //  |-<Gryphon Master>
    //  |--<Hippogryph Master>
    //  |---<Darnassus Flight Master>
    // 
    //  Horde
    //  |-<Bat Handler>
    //  |--<Wind Rider Master>
    //  |---<Dragonhawk Master>
    //  |----<Thunder Bluff Flight Master>
    //
    //  Neutral
    //  |-<Flight Master>
    //  |--<Flightmaster>
    //  |---<Gryphon Master>
    //  |----<Emerald Circle Flight Master>
    //  |-----<Spectral Gryphon Master>
    //  |------<Dragonhawk Master>
    //
    //////////////////////////////////////////////////////////////////////////////////////////

    // Gryphon Master
    constexpr uint32_t gryphonMasterIds[] = {352, 523, 931, 1571, 1572, 1573, 2299, 2409, 2432, 2835, 2859,
        2941, 4321, 6326, 6327, 7823, 8018, 8609, 12596, 12617, 16822, 17209, 18809, 18931, 18939, 19181, 20234,
        21107, 24366, 23704, 26602, 26878, 26879, 23736, 23859, 24061, 26876, 26877, 226878, 26880, 29750,
        41321, 41323, 41325, 41332, 42406, 42426, 42983, 43000, 43042, 43043, 43072, 43087, 43088, 43371,
        43697, 43701, 43702, 44409, 44410, 46006, 47118, 47119, 47147, 47154, 52753};
    mgr->registerCreatureScript(gryphonMasterIds, &AllianceGryphon::Create);

    // Hippogryph Master
    constexpr uint32_t hippogryphMasterIds[] = {3838, 3841, 4267, 4319, 4407, 6706, 8019, 9605, 10897, 11138, 11800, 12577, 12578,
        15177, 17554, 17555, 18785, 18788, 18789, 18937, 22485, 22935, 22936, 22937, 26881, 30271, 33253, 34374, 34378, 35137,
        40552, 40553, 41322, 43107, 47155};
    mgr->registerCreatureScript(hippogryphMasterIds, &AllianceHippogryph::Create);

    // Wyvern Master
    constexpr uint32_t wyvernMasterIds[] = {1387, 2851, 2858, 2861, 2995, 3305, 3310, 3615, 4012, 4312, 4314, 4317, 6026, 6726,
        7824, 8020, 8610, 10378, 11139, 11899, 11900, 11901, 12616, 12740, 13177,
        15178, 16587, 18791, 18807, 18808, 18930, 18942, 18953, 19317, 19558, 20762, 22455,
        24032, 25288, 26566, 26846, 26847, 26848, 26850, 26852, 26853, 29762, 31426, 35139, 39898, 40344, 41246, 43073};
    mgr->registerCreatureScript(wyvernMasterIds, &HordeWyvern::Create);

    // Bat Master
    constexpr uint32_t batMasterIds[] = {2226, 2389, 4551, 12636, 3575, 23816, 24155, 26844, 26845, 27344, 27842, 37915, 41142,
        /*Rocket*/ 35141};
    mgr->registerCreatureScript(batMasterIds, &HordeBat::Create);

    // Dragonhawk Masters
    constexpr uint32_t dragonhawkMasterIds[] = {16189, 16192, 24795, 26560, 26851, 27046, 30269};
    mgr->registerCreatureScript(dragonhawkMasterIds, &DragonhawkMasters::Create);

    // Neutral Masters
    constexpr uint32_t neutralMasterIds[] = { 10583, 11798, 16227, 18938, 18940, 19581, 19583, 20515,
        21766, 22216, 23415, 22931, 23413, 23612, 24851, 29480, 28037, 28195, 28196, 28197,
        28574, 28615, 28618, 28621, 28623, 28624, 28674, 29137, 29721, 29749, 29757, 29950,
        29951, 29952, 30314, 30433, 30569, 30869, 30870, 31069, 31078, 32571, 33849, 37888,
        40358, 43991, 44233, 44407};
    mgr->registerCreatureScript(neutralMasterIds, &NeutralMasters::Create);

    mgr->register_creature_script(7999, &TyrandeWhisperwind::Create);
    mgr->register_creature_script(17468, &ProphetVelen::Create);
    mgr->register_creature_script(2784, &KingMagniBronzebeard::Create);
    mgr->register_creature_script(4949, &Thrall::Create);
    mgr->register_creature_script(3057, &CairneBloodhoof::Create);
    mgr->register_creature_script(10181, &LadySylvanasWindrunner::Create);

    mgr->register_creature_script(23090, &TrollRoofStalker::Create);

    mgr->register_creature_script(27989, &DISCO::Create);

    mgr->register_creature_script(5781, &SilithidCreeperEgg::Create);

    mgr->register_creature_script(16483, &DraeneiSurvivor::Create);
    mgr->register_creature_script(12423, &GuardRoberts::Create);

    mgr->register_creature_script(2675, &ExplosiveSheep::Create);

    mgr->register_creature_script(27894, &SotaAntiPersonnalCannon::Create);

    mgr->registerCreatureScript(sparringCreatureIds, &SparringCreature::Create);
}
