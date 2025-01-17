/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/QuestLogEntry.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Movement/MovementManager.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Utilities/Random.hpp"

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
        getCreature()->getMovementManager()->moveCharge(pos.x, pos.y, pos.z);
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

        Creature* NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(5895, LocationVector(SSX, SSY + 1, SSZ, SSO), true, false, 0, 0);
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

//////////////////////////////////////////////////////////////////////////////////////////
// Lazy Peons
class PeonSleepingAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PeonSleepingAI(c); }
    explicit PeonSleepingAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(3000 + Util::getRandomUInt(180000));
    };

    void AIUpdate() override
    {
        getCreature()->castSpell(getCreature(), 17743, true);
        RemoveAIUpdateEvent();
    };
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
            Creature* NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(7729, LocationVector(getCreature()->GetPositionX() + 2, getCreature()->GetPositionY() + 2, getCreature()->GetPositionZ(), getCreature()->GetOrientation()), true, false, 0, 0);
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

        Creature* NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9526, LocationVector(getCreature()->GetPositionX() + Util::getRandomFloat(5.0f), getCreature()->GetPositionY() + Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()), true, false, 0, 0);
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);

        NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9526, LocationVector(getCreature()->GetPositionX() - Util::getRandomFloat(5.0f), getCreature()->GetPositionY() - Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()), true, false, 0, 0);
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

        Creature* NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9527, LocationVector(getCreature()->GetPositionX() + Util::getRandomFloat(5.0f), getCreature()->GetPositionY() + Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()), true, false, 0, 0);
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);

        NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9527, LocationVector(getCreature()->GetPositionX() - Util::getRandomFloat(5.0f), getCreature()->GetPositionY() - Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()), true, false, 0, 0);
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

        Creature* NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9297, LocationVector(getCreature()->GetPositionX() + Util::getRandomFloat(5.0f), getCreature()->GetPositionY() + Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()), true, false, 0, 0);
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);

        NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9297, LocationVector(getCreature()->GetPositionX() - Util::getRandomFloat(5.0f), getCreature()->GetPositionY() - Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()), true, false, 0, 0);
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

        Creature* NewCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(9521, LocationVector(getCreature()->GetPositionX() + Util::getRandomFloat(5.0f), getCreature()->GetPositionY() + Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()), true, false, 0, 0);
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);

        getCreature()->getWorldMap()->getInterface()->spawnCreature(9521, LocationVector(getCreature()->GetPositionX() - Util::getRandomFloat(5.0f), getCreature()->GetPositionY() - Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation()), true, false, 0, 0);
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

        Creature* SilithidGrub = getCreature()->getWorldMap()->getInterface()->spawnCreature(3251, LocationVector(SSX, SSY + 1, SSZ, SSO), true, false, 0, 0);
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
            if (auto* player = getCreature()->getWorldMap()->getPlayer(static_cast<uint32_t>(getCreature()->getTargetGuid())))
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

void SetupMiscCreatures(ScriptMgr* mgr)
{
    mgr->register_creature_script(16518, &NestlewoodOwlkin::Create);
    mgr->register_creature_script(11120, &CrimsonHammersmith::Create);
    mgr->register_creature_script(5894, &Corrupt_Minor_Manifestation_Water_Dead::Create);
    mgr->register_creature_script(3425, &SavannahProwler::Create);
    mgr->register_creature_script(10556, &PeonSleepingAI::Create);
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
    uint32_t GryphonMasterIds[] = { 352, 523, 931, 1571, 1572, 1573, 2299, 2409, 2432, 2835, 2859,
        2941,  4321, 6326, 6327, 7823, 8018, 8609,12596, 12617, 16822, 17209, 18809, 18931, 18939, 19181, 20234,
        21107, 24366, 23704, 26602, 26878, 26879, 23736, 23859, 24061, 26876, 26877, 226878, 26880,  29750, 
        41321, 41323, 41325,  41332, 42406, 42426, 42983, 43000, 43042, 43043, 43072, 43087, 43088, 43371, 
        43697, 43701, 43702, 44409, 44410, 46006, 47118, 47119, 47147, 47154, 52753, 0 };
    mgr->register_creature_script(GryphonMasterIds, &AllianceGryphon::Create);

    // Hippogryph Master
    uint32_t HippogryphMasterIds[] = { 3838, 3841, 4267, 4319, 4407, 6706, 8019, 9605, 10897, 11138, 11800, 12577, 12578,
        15177, 17554, 17555, 18785, 18788, 18789, 18937, 22485, 22935, 22936, 22937, 26881, 30271, 33253, 34374, 34378, 35137,
        40552, 40553, 41322, 43107, 47155, 47154, 47147, 0 };
    mgr->register_creature_script(HippogryphMasterIds, &AllianceHippogryph::Create);

    // Wyvern Master
    uint32_t WyvernMasterIds[] = { 1387, 2851, 2858, 2861, 2995, 3305, 3310, 3615, 4012, 4312, 4314, 4317, 6026, 6726,
        7824, 8020, 8610, 10378, 11139, 11899, 11900, 11901, 12616, 12740, 13177,
        15178, 16587, 18791, 18807, 18808, 18930, 18942, 18953, 19317, 19558, 20762, 22455, 
        24032, 25288, 26566, 26846, 26847, 26848, 26850, 26852, 26853, 29762, 31426, 35139, 39898, 40344, 41246, 43073, 0 };
    mgr->register_creature_script(WyvernMasterIds, &HordeWyvern::Create);

    // Bat Master
    uint32_t BatMasterIds[] = { 2226, 2389, 4551, 12636, 3575, 23816, 24155, 26844, 26845, 27344, 27842, 37915, 41142, 
    /*Rocket*/ 35141, 0 };

    mgr->register_creature_script(BatMasterIds, &HordeBat::Create);

    // Dragonhawk Masters
    uint32_t DragonhawkMasterIds[] = { 16189, 16192, 24795, 26560, 26851, 27046, 30269, 0 };
    mgr->register_creature_script(DragonhawkMasterIds, &DragonhawkMasters::Create);

    // Neutral Masters
    uint32_t NeutralMasterIds[] = { 10583, 11798, 16227, 18938, 18940, 19581, 19583, 20515,
        21766, 22216, 23415, 22931, 23413, 23612, 24851, 29480, 28037, 28195, 28196, 28197,
        28574, 28615, 28618, 28621, 28623, 28624, 28674, 29137, 29721, 29749, 29757, 29950,
        29951, 29952, 30314, 30433, 30569, 30869, 30870, 31069, 31078, 32571, 33849, 37888,
        40358, 43991, 44233, 44407, 0 };
    mgr->register_creature_script(NeutralMasterIds, &NeutralMasters::Create);

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
}
