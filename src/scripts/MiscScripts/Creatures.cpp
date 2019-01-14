/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Explosive Sheep (Summoned by ItemID: 4384)
class ExplosiveSheep : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ExplosiveSheep);
    explicit ExplosiveSheep(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad()
    {
        getCreature()->Despawn(180000, 0); // "Lasts for 3 minutes or until it explodes."
    }

    void OnCombatStart(Unit* mTarget) // Summons an Explosive Sheep which will charge at a nearby enemy and explode for 135 - 165 damage.
    {
        getCreature()->GetAIInterface()->splineMoveCharge(mTarget);
        getCreature()->castSpell(getCreature(), 4050, true);
        getCreature()->Despawn(1000, 0); // Despawn since we "exploded"
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Crimson Hammersmith
class CrimsonHammersmith : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CrimsonHammersmith);
    explicit CrimsonHammersmith(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Who Dares Disturb Me");
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Corrupt Minor Manifestation Water Dead
class Corrupt_Minor_Manifestation_Water_Dead : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Corrupt_Minor_Manifestation_Water_Dead);
    explicit Corrupt_Minor_Manifestation_Water_Dead(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* /*mKiller*/) override
    {
        float SSX = getCreature()->GetPositionX();
        float SSY = getCreature()->GetPositionY();
        float SSZ = getCreature()->GetPositionZ();
        float SSO = getCreature()->GetOrientation();

        Creature* NewCreature = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(5895, SSX, SSY + 1, SSZ, SSO, true, false, 0, 0);
        if(NewCreature != NULL)
            NewCreature->Despawn(600000, 0);
    }
};

class SavannahProwler : public CreatureAIScript
{
public:

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

    static CreatureAIScript* Create(Creature* c) { return new SavannahProwler(c); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lazy Peons
class PeonSleepingAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(PeonSleepingAI);
    explicit PeonSleepingAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        RegisterAIUpdateEvent(3000 + Util::getRandomUInt(180000));
    };

    void AIUpdate()
    {
        getCreature()->castSpell(getCreature(), 17743, true);
        RemoveAIUpdateEvent();
    };
};

class KirithAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KirithAI);
    explicit KirithAI(Creature* pCreature) : CreatureAIScript(pCreature)  {}

    void OnDied(Unit* mKiller)
    {
        if(mKiller->isPlayer())
        {
            Creature* NewCreature = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(7729, getCreature()->GetPositionX() + 2, getCreature()->GetPositionY() + 2, getCreature()->GetPositionZ(), getCreature()->GetOrientation(), true, false, 0, 0);
            if(NewCreature != NULL)
                NewCreature->Despawn(3 * 6 * 1000, 0);
        }
    }
};

class AllianceGryphon : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AllianceGryphon);

    explicit AllianceGryphon(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* mTarget)
    {
        if(!mTarget->isPlayer())
            return;

        Creature* NewCreature = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(9526, getCreature()->GetPositionX() + Util::getRandomFloat(5.0f), getCreature()->GetPositionY() + Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), true, false, 0, 0);
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);

        NewCreature = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(9526, getCreature()->GetPositionX() - Util::getRandomFloat(5.0f), getCreature()->GetPositionY() - Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), true, false, 0, 0);
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);
    }
};

class AllianceHippogryph : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AllianceHippogryph);

    explicit AllianceHippogryph(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* mTarget)
    {
        if(!mTarget->isPlayer())
            return;

        Creature* NewCreature = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(9527, getCreature()->GetPositionX() + Util::getRandomFloat(5.0f), getCreature()->GetPositionY() + Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), true, false, 0, 0);
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);

        NewCreature = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(9527, getCreature()->GetPositionX() - Util::getRandomFloat(5.0f), getCreature()->GetPositionY() - Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), true, false, 0, 0);
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);
    }
};

class HordeWyvern : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HordeWyvern);
    explicit HordeWyvern(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* mTarget)
    {
        if(!mTarget->isPlayer())
            return;

        Creature* NewCreature = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(9297, getCreature()->GetPositionX() + Util::getRandomFloat(5.0f), getCreature()->GetPositionY() + Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), true, false, 0, 0);
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);

        NewCreature = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(9297, getCreature()->GetPositionX() - Util::getRandomFloat(5.0f), getCreature()->GetPositionY() - Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), true, false, 0, 0);
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);
    }
};

class HordeBat : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HordeBat);
    explicit HordeBat(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* mTarget)
    {
        if(!mTarget->isPlayer())
            return;

        Creature* NewCreature = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(9521, getCreature()->GetPositionX() + Util::getRandomFloat(5.0f), getCreature()->GetPositionY() + Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), true, false, 0, 0);
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);

        getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(9521, getCreature()->GetPositionX() - Util::getRandomFloat(5.0f), getCreature()->GetPositionY() - Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), true, false, 0, 0);
        if(NewCreature != NULL)
            NewCreature->Despawn(360000, 0);
    }
};

class DragonhawkMasters : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DragonhawkMasters)
    explicit DragonhawkMasters(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        LocationVector vect(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
        for (uint8 i = 0; i < 2; ++i)
        {
            vect.x += Util::getRandomFloat(2.0f);
            vect.y += Util::getRandomFloat(2.0f);
        }
    }
};

class NeutralMasters : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(NeutralMasters)
    explicit NeutralMasters(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        LocationVector vect(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
        for (uint8 i = 0; i < 2; ++i)
        {
            vect.x += Util::getRandomFloat(2.0f);
            vect.y += Util::getRandomFloat(2.0f);
        }
    }
};

class TyrandeWhisperwind : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TyrandeWhisperwind);
    explicit TyrandeWhisperwind(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->PlaySoundToSet(5885);
    }
};

class ProphetVelen : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ProphetVelen);
    explicit ProphetVelen(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->PlaySoundToSet(10155);
    }
};

class KingMagniBronzebeard : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KingMagniBronzebeard);
    explicit KingMagniBronzebeard(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->PlaySoundToSet(5896);
    }
};

class Thrall : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Thrall);
    explicit Thrall(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->PlaySoundToSet(5880);
    }
};

class CairneBloodhoof : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CairneBloodhoof);
    explicit CairneBloodhoof(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->PlaySoundToSet(5884);
    }
};

class LadySylvanasWindrunner : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LadySylvanasWindrunner);
    explicit LadySylvanasWindrunner(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->PlaySoundToSet(5886);
    }
};

class TrollRoofStalker : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TrollRoofStalker);
    explicit TrollRoofStalker(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad()
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
    ADD_CREATURE_FACTORY_FUNCTION(DISCO);
    explicit DISCO(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad()
    {
        getCreature()->castSpell(getCreature(), 50487, false);   // summon disco dancefloor
        getCreature()->castSpell(getCreature(), 50314, false);   // play the music
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
//Silithid Creeper Egg
class SilithidCreeperEgg : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SilithidCreeperEgg);
    explicit SilithidCreeperEgg(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pCreature->GetAIInterface()->SetAllowedToEnterCombat(false);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        float SSX = getCreature()->GetPositionX();
        float SSY = getCreature()->GetPositionY();
        float SSZ = getCreature()->GetPositionZ();
        float SSO = getCreature()->GetOrientation();

        Creature* SilithidGrub = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(3251, SSX, SSY + 1, SSZ, SSO, true, false, 0, 0);
        if(SilithidGrub != NULL)
            SilithidGrub->Despawn(600000, 0);
    }
};

class DraeneiSurvivor : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DraeneiSurvivor);
    explicit DraeneiSurvivor(Creature* pCreature) : CreatureAIScript(pCreature)
    { }

    void OnLoad()
    {
        getCreature()->setHealth(getCreature()->getMaxHealth() / 2);
    }
};

class GuardRoberts : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GuardRoberts);
    explicit GuardRoberts(Creature* pCreature) : CreatureAIScript(pCreature)
    { }

    void OnLoad() override
    {
        getCreature()->setHealth(100);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Argh, the pain. Will it ever leave me?");
    }
};

class SotaAntiPersonnalCannon : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SotaAntiPersonnalCannon);
    explicit SotaAntiPersonnalCannon(Creature* pCreature) : CreatureAIScript(pCreature)
    { }

    void OnLoad()
    {
        getCreature()->setMoveRoot(true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nestlewood Owlkin - Quest 9303
class NestlewoodOwlkin : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(NestlewoodOwlkin);
    explicit NestlewoodOwlkin(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        respawn = false;
        reset = false;
    }

    void AIUpdate()
    {
        if (!reset)
        {
            if (getCreature()->HasAura(29528) && !respawn)
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
        if (getCreature()->HasAura(29528))
        {
            Player* player = getCreature()->GetMapMgr()->GetPlayer(static_cast<uint32>(getCreature()->getTargetGuid()));
            if (player != nullptr)
            {
                if (!player->HasQuest(9303) || player->HasFinishedQuest(9303))
                    return;

                QuestLogEntry* quest_entry = player->GetQuestLogForEntry(9303);
                if (quest_entry == nullptr)
                    return;

                if (quest_entry->GetMobCount(0) < 6)
                {
                    quest_entry->IncrementMobCount(0);
                    quest_entry->SendUpdateAddKill(0);
                    quest_entry->UpdatePlayerFields();

                    RegisterAIUpdateEvent(240000);  // update after 4 mins
                }
            }
        }
    }

    void OnLoad()
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

    // Gryphon Master
    uint32 GryphonMasterIds[] = { 352, 523, 931, 1571, 1572, 1573, 2299, 2409, 2432, 2835, 2859,
        2941, 4321, 7823, 8018, 8609, 12596, 12617, 16822, 17209, 18809, 18931, 18939, 19181, 20234,
        21107, 6326, 6327, 24366, 23704, 26879, 23736, 23859, 24061, 26876, 26877, 226878, 26880, 0 };

    mgr->register_creature_script(GryphonMasterIds, &AllianceGryphon::Create);

    // Hippogryph Master
    uint32 HippogryphMasterIds[] = { 3838, 3841, 4267, 4319, 4407, 6706, 8019, 10897, 11138, 11800, 12577, 12578,
        15177, 17554, 17555, 18785, 18788, 18789, 18937, 22485, 22935, 22936, 22937, 26881, 30271, 0 };
    mgr->register_creature_script(HippogryphMasterIds, &AllianceHippogryph::Create);

    // Wyvern Master
    uint32 WyvernMasterIds[] = { 1387, 2851, 2858, 2861, 2995, 3305, 3310, 3615, 4312, 4314, 4317, 6026, 6726,
        7824, 8020, 8610, 10378, 11139, 11899, 11900, 11901, 12616, 12740, 13177,
        15178, 16587, 18791, 18807, 18808, 18930, 18942, 18953, 19317, 19558, 20762,
        24032, 25288, 26566, 26846, 26847, 26848, 26850, 26852, 26853, 29762, 31426, 0 };
    mgr->register_creature_script(WyvernMasterIds, &HordeWyvern::Create);

    // Bat Master
    uint32 BatMasterIds[] = { 2226, 2389, 4551, 12636, 3575, 23816, 24155, 26844, 26845, 27344, 27842, 37915, 0 };
    mgr->register_creature_script(BatMasterIds, &HordeBat::Create);

    // Dragonhawk Masters
    uint32 DragonhawkMasterIds[] = { 16189, 16192, 26560, 30269, 0 };
    mgr->register_creature_script(DragonhawkMasterIds, &DragonhawkMasters::Create);

    // Neutral Masters
    uint32 NeutralMasterIds[] = { 10583, 11798, 16227, 18938, 18940, 19581, 19583, 20515,
        21766, 22216, 22455, 22931, 23612, 24851, 29480, 26851, 27046, 28037, 28195, 28196, 28197,
        28574, 28615, 28618, 28621, 28623, 28624, 28674, 29137, 29721, 29749, 29750, 29757, 29950,
        29951, 29952, 30314, 30433, 30569, 30869, 30870, 31069, 31078, 32571, 33849, 37888, 0 };
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
